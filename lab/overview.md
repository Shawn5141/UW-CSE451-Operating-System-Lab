# xk Overview

The overarching goal of xk (the experimental kernel) is to help students 
understand operating system implementation at an advanced level. 
The baseline code for xk provides a complete, but limited, 
working operating system, capable of running a single program
(loaded from disk) that can receive and print characters to a serial port 
(keyboard and display).  It also contains basic synchronization,
process time slicing, memory management, and a read-only file system.  

The assignments ask students to add basic UNIX-like functionality to
this baseline in a series of five projects: (i) implement file system
calls including managing file descriptors, (ii) support a user-level shell
by implementing UNIX fork, exec, wait, and pipe, (iii) add sbrk, dynamic
stack growth, and copy-on-write semantics, 
(iv) implement user-level paging, and (v) implement 
a crash-safe file system.  Except for the first assignment, 
assignments require student code to carefully consider data structure 
synchronization.

## Goals

### Radical simplicity
Operating systems are inherently complex: concurrent, 
performance-critical, feature-laden, and hard to change.  
It is no accident that commercial operating systems require tens of millions 
of lines of code. Although each started with a small core, Windows 
now has about 50 million lines of code while Linux clocks in at 15 million. 
We have intentionally designed xk to be extremely simple; as a very 
concrete example, xk has no kmalloc.  The baseline kernel is about 
4000 lines of code (and we are working on making it simpler). 

By stripping out less important features, our aim is for students 
understand and build a complete working system in a term. In this, 
we were inspired by the MIT xv6 project -- we started from xv6 as our 
original code base but we've made substantial changes since then. 

An alternative is to ask students to add small features to an existing 
open source operating system such as Linux or Android.  It is definitely 
useful to be able to add a system call to Linux or to write a checker 
for the on-disk format of the ext4 file system.  However, in our view, 
students learn more, and are more interested in, attempting to build 
key pieces of the operating system for themselves rather than learning 
how to cut and paste into a existing code base.

### Complete bootable code 
Despite being simple, xk is a working operating system that can 
load and run a user program, such as the canonical “hello world” program, 
on a modern 64-bit Intel server.  (To simplify debugging with gdb, 
students develop xk on a QEMU virtual machine rather than directly on 
hardware.) In particular, this means the baseline code provides 
machine-dependent code for context switching, synchronization, 
address translation, and disk operations.

Dating from Nachos, a standard approach for instructional operating systems 
is to build a complete kernel, and then to remove key pieces of code for 
students to re-implement.  (Nachos, Pintos, JOS, and OS161 all work this way.)
The problem with this is two-fold.  First, students never see a 
complete system until the very end of the term, making it harder to 
understand how the different pieces fit together.  Second, there is often 
only a single way to build some key part of the system, such as the 
context switch or thread synchronization code, which makes copying easier. 

### Challenging design problems with automated acceptance testing. 

Operating systems provide a unique opportunity in the CS curriculum 
for students to engage with open-ended and challenging design problems.  
In particular, it is essential that computer science students 
– especially those heading 
to careers in systems software – learn how to build non-trivial designs 
with complex data structure synchronization.  For example, a virtual 
memory system with read-write shared memory segments (due to copy-on-write 
or mmap sharing) requires careful treatment of lookup tables and core maps 
to provide atomicity and to avoid deadlock.  

At the same time, xk has a set of acceptance tests – automated scripts 
for validating and grading student solutions. 
One cautionary note: the current automated  tests are not complete, 
particularly in catching errors in student code for exception handling.
Since later assignments build on earlier ones, if you encounter a problem
you should at least consider whether the bug is an uncaught error from
a previous assignment.

The requirement for automated testing dovetails with xk’s other goals.
All tests are user programs running on 
top of the student kernel; we can do this, even from the first 
assignment, because our baseline code is complete enough to run user 
programs. This allows us to completely avoid complex kernel 
printf parsing.  We use a slightly larger system call interface than 
a minimal kernel. We use system calls instead of internal interfaces 
for almost all of our test code. (As an example, we have a system
call to crash after k disk operations, for configurable k; this lets
us test the file system for crash safety, but would not be in a real system.)
Students, of course, are strongly encouraged to write internal module 
tests; the test code is intended to be functional rather than diagnostic.

## Baseline Code Walkthrough

We next walk through the code and main data structures. 
The lab1 doc has instructions for building and debugging xk. 
This document to help students navigate which functionality lives where.

### Directory structure 

kernel -- the operating system kernel and bootloader code. 

user -- user applications that run on xk.

inc -- include files, some of which are shared between user and kernel code
(such as the file defining which system call codes refer to which 
system calls), or between the bootloader and the kernel.

lab -- the lab descriptions. Note these are sometimes updated during 
the quarter.

A particularly useful file is inc/defs.h.  This lists every cross-module 
kernel data structure and function, along with which file implements 
the function. You will want to put new function declarations you want visible
to the kernel in here.

### Booting

The first instruction executed as an x86 PC boots is the BIOS. The BIOS
loads the bootloader from a fixed location on disk into a fixed
location in memory and jumps to it.  The bootloader 
in turn loads the operating system (xk) kernel from a fixed location on
disk into a fixed location in memory and jumps to it.  

xk assumes it is running on a 64 bit x86. 
The x86 is backwardly compatible with earlier operating systems
(e.g., 16 bit or 32 bit), so when it boots it starts out in a more
primitive mode.  Part of the task of the bootloader is to shift the 
processor to 64 bit mode.
gdb is not set up to be able to debug both 32 and 64 bit mode simultaneously,
so we start debugging at the beginning of xk, rather than from the
beginning of the bootloader.

For reference, the bootloader code is bootasm.S, bootmain.c, and multiboot.h.
Students can safely ignore these files.


### Initialization

initcode.S -- The bootloader starts executing the kernel at start.
start must be at a fixed known location in memory.  start in turn calls 
the C function main.

main.c -- initialization code

In main.c:main, the kernel needs to do several things as it starts up.
Note that the ordering very much matters -- for example, we need the ability
to allocate memory before we can build the kernel page table, etc. 

The first step is to figure out how much memory the machine has, and then
make that memory available for dynamic allocation.  (Different
machines are configured with different amounts of memory, but the kernel
code is the same regardless.)  

We list the files in the order they are referenced by main.

`e820.c` -- device specific code to read machine registers to determine
the physical addresses of DRAM and IO devices.

`kalloc.c` -- kernel code to build and manage a free list of available
kernel pages.  Unlike at application level where `malloc` can support arbitrary
size allocation units, the xk kernel only allocates memory in fixed 4KB chunks.
The free list of pages is managed as a list.

Once kernel memory can be allocated, the kernel can set up the
kernel page table.  A companion document describes the memory layout
in more detail, but for now you should understand that in xk, 
both user code and kernel code runs with translated (virtual) addresses.

Natively, the kernel has access to physical memory, so it could just
run using physical addresses.  However, by mapping we allow the kernel
to access both user addresses and kernel addresses at the same time:
the user code/data is in low addresses, while the kernel code/data 
has high addresses.  While the user mapping can be complex and partial
(e.g., for virtual memory paging), the kernel mapping is simple:
we simply add a (very large) constant to every physical address.

Note that this can be confusing! A user virtual address and a kernel
virtual address can refer to the same physical memory location, and it
is easy to forget that kernel memory can refer to either its location
in physical memory or its remapped kernel virtual address.

`vspace.c` -- Abstraction of the virtual memory management. Adds a level of
indirection so virtual memory actions can be architecture independent.

- `x86_64vm.c` -- code for managing memory (called exclusively through `vspace.c`).
`kvmalloc` allocates the page table the hardware uses to map kernel memory.
Protection bits are set in the page table so that user code cannot access
kernel memory, even though it is "mapped" into the user address space.

- `x86_64vm.c:seginit` initializes the per-CPU segment table. 

We'll have more to say about vm.c later.

`mp.c, lapic.c` -- code to setup multiprocessor execution.  At present xk
only runs in single processor mode, and so you can safely ignore those 
routines. (Note: we do allow time-slicing of kernel code, and so your 
code should be written to work correctly on a multiprocessor.)

`picirq.c, console.c, uart.c` -- IO initialization. All IO in xk is 
performed synchronously; we only take interrupts for time slice events.
The specific initialization code depends on the device (e.g., the specific
model number of the serial port).  Understanding the device code therefore 
requires reading manuals, something you can skip for now.

However, note that `cprintf` assumes that the console and uart are initialized!
So don't try to print anything before this point.  (You can of course 
use gdb to set a breakpoint to examine the contents of memory.)

`proc.c` -- the code to manage the process table and which process
is running at any specific point in time.  `proc.c:pinit` just initializes
the lock providing mutual exclusion for the process table.

`proc.c:userinit`, called after traps and the disk are set up, sets up the first
process by copying the application code/data from a file on disk into memory.
It also sets up the page table so that the hardware will translate
user addresses to their corresponding physical addresses.
We don't immediately jump to to the user program, however.  
Rather, we mark it as "schedulable", for the scheduler to notice and run.

`proc.c:scheduler` is the last thing called by the init code, one time
on each processor.  You can think of this as the idle loop, what runs on
each processor when nothing else is running. It finds a runnable process
and switches to it.  When the process blocks or finishes, it switches
back to the scheduler, which then picks the next process to run.

`spinlock, sleeplock, swtch.S` -- code to acquire and release locks, as well
as to switch between processes.

We'll have more to say about `proc.c` and these other files later.

`trap.c, trapasm.S, vectors.S, syscall.c` -- code to handle interrupts, 
exceptions, and system calls.  Initially, we just need to set up
the interrupt table -- the set of vectors for where the hardware
should jump on different types of interrupts and system calls.

Again, ordering matters -- we need to set up the interrupt table
before we enable interrupts!

You will need to understand the trap code in detail, so we'll describe
that shortly.

`bio.c` -- the file system buffer cache.  To reduce I/O, the file system 
keeps a cache of recently read file blocks, the buffer cache.  You can
safely ignore that for now, although it will matter for lab 5.

`ide.c` -- device specific code for the disk device. As with the other
IO devices, the disk is managed synchronously, in that the kernel will
spin-wait until the disk operation completes before returning, rather
than suspending and running a different task.

`string.c` -- some useful C library routines

### Traps

The xk code for handling traps: interrupts, exceptions, and system
calls is spread over several files. You will find this discussion easier
to follow if you have already read OSPP Chapter 2.

`vectors.S` -- the interrupt table. More precisely, `trap.c:tvinit` sets
up a gate for each entry point in `vectors.S`. The gate specifies
what to do on each type of interrupt, e.g., including whether to take
the interrupt in kernel mode or not.

Some trap handlers push a value onto the stack, and some don't. This is 
because the x86 puts different information on the kernel/interrupt stack
for different types of interrupt (e.g., a page fault pushes the failing
virtual address, while a timer interrupt does not push anything).
For those that don't have a value, we push one to ensure that
the format of the stack is the same regardless of how we get there.

`trapasm.S` -- `trapasm.S:alltraps` is generic assembly code to 
handle traps, that is, interrupts, system calls, and exceptions. 
Because we could get here on an interrupt, we need to
be able to restore the entire state of the interrupted process.  So we first
save all the registers to the stack, move a pointer to the stack into
a register holding the first argument to a procedure, and then 
call into the C language `trap.c:trap`.

When we return in `trapasm.S:trapret`, we restore the state of the interrupted
process, by restoring its registers, popping the stack frame inserted 
by interrupt vector, and then returning from the interrupt with `iretq` (interrupt return).

Note that we can also get to `trapasm.S:trapret` in one other way. When
we start a new process (`proc.s:allocproc`), we have to be able to switch 
to it from the scheduler. We do this by making the switch code "return" to
trapret, so that trapret will "restore" the state of the registers
to the initial value expected by the process.

`trap.c` -- Generic trap handling code. Once we have saved the registers,  
`trapasm.S:alltraps` calls `trap.c:trap`.  This takes one argument, the pointer
to the processor register state saved by the hardware (as it takes the
interrupt and indirects through the vector table) and by software (in `alltraps`).

Depending on the type of interrupt, we do different things.  One
possibility is that this is a timer interrupt, in which case we need
to switch out of the current process and into the scheduler (so that the 
scheduler can pick a different task to run).  Alternately, this could
be a system call, in which case we need to jump to the generic system 
call handler in syscall.c.

In the case of a system call, the values in registers, where we might
pass the arguments to the system call, will have been saved and overwritten
as part of going from `vectors.S` to `alltraps` to trap to syscall.  So we
store the trapframe (the saved user registers) in the process table,
and then access them in the system call handlers depending on which type
of handler we need to invoke.

Most of the rest of the complexity of `trap.c` is due to handling the
case where a process can be killed by another process (e.g., 
by "kill -9" on UNIX). We don't exercise this so you can skip by it.

`syscall.c` -- code to handle system calls.  Again, we have a table of
possible system calls, which we index using the value of the first
argument to the system call, inserted by the user-level library code
(see `user/usys.S`). Most of the code is to fetch user parameters 
(`syscall.c:argint` and `argptr`).  For example, `argptr` returns a pointer
that can be used from within the kernel to access a data structure
stored in user space (cf. `sys_sysinfo` which is a system call to fetch a 
set of system monitoring variables).

`sysfile.c, sysproc.c` -- implementations of the file system and process
system calls.  Most of the system calls are unimplemented stubs due
to be filled in by the next few labs.  We have provided implementations
of `sys_write` as a simple example (and to allow us to print
to the serial port even if your other system calls aren't working).

`inc/syscall.h` -- the assignment of system calls to system call numbers.
This needs to be the same for user programs and the kernel!

`user/usys.S` -- macros to generate the user library code for each system call.
We simply put the system call code into a caller save register and 
invoke the trap. The other parameters to the system call will be handled
normally.

For definitions of the UNIX system calls and parameters, 
chapter 0 of the xv6 book
(https://pdos.csail.mit.edu/6.828/2014/xv6/book-rev8.pdf)
is more thorough than the treatment in OSPP.  Note that xk does
not implement all of the xv6 system calls.

### Processes and synchronization

For the most part, you will be able to ignore the process and synchronization
code for the first lab.  We include a description for completeness (and
because you'll need it for lab 2).  This description will be easier
to follow if you have read Chapters 4 and 5 in OSPP.

The process abstraction is managed through the `proc.c:ptable`, 
a fixed size table of `proc.h:struct proc`.  

Each process has a page table, a set of memory regions, a stack to run in the 
kernel, a state (runnable, blocked, ...), a process ID (index into the process table),
the ID of the process that created this process (if any), the current trapframe if we are
executing in the kernel, and so forth.

`proc.c` has several interesting procedures.  One is `proc.c:allocproc`.
This finds an unused process table entry and fills it, e.g., by
allocating a kernel stack, etc. It sets the process to start running
in the kernel at proc.c:forket, but where the stack is set up so that
proc.c:forkret "returns" to proc.c:trapret, which "returns" to the
initial state of the process executing at user level.

Another interesting property of the implementation is that there
is no explicit list of runnable processes. Instead, the scheduler
code iterates over the process table (starting with the item just
after the previously scheduled process to avoid starvation) to find
the next process to run.

xk supports two kinds of locks to enforce mutual exclusion.  The difference
concerns what they do when the lock is busy. 

`spinlock.c` -- A spinlock spins in a loop until the lock is free.

`sleeplock.c` -- A sleeplock yields the processor until the lock is free.

A given data structure will generally be protected by either a spinlock 
or a sleeplock. There is no guarantee of mutual exclusion if they are mixed.

The low level routines of the kernel typically use spinlocks; for example,
the scheduler needs to use a spinlock as if the scheduler
is busy, you won't be able to put the current process to sleep and pick
another process to run, until the scheduler lock is free.

Similarly, it is generally a bad idea to try to acquire a 
sleeplock in an interrupt handler.  Most interrupt handlers
are structured to avoid accessing shared data as much as possible, but
where it is needed, they should use a spinlock. Care must be taken
that interrupts are disabled whenever a spinlock is held, or else
the interrupt handler could get into an infinite loop (waiting for
a spinlock that is held by the interrupted process).

One difference between xk and what is described
in the OSPP textbook is how xk handles condition variables.  
(This paragraph will be easier to understand once you read OSPP chapter 5.)
xk has two routines, sleep and wakeup, that roughly correspond
to condition variable wait and broadcast.  (Although wakeup1 sounds
like it should be equivalent to condition variable signal, it is just
a helper function, to acquire the lock on the process table.)

The xk routines are designed to work with spinlocks -- that is, 
sleep releases a spinlock and blocks the process until it is 
rescheduled by wakeup.  Similarly, wakeup 
has no effect if there are no waiting threads.

The implementation is particularly simple, however.  Instead
of allocating a queue for each condition variable, processes
sleep and wakeup on "channels" -- these are arbitrary values
(typically the address of a unique memory location specific
to a kernel module).  To sleep, the process marks in the 
process table that it is waiting on the channel; 
to wake up a process, the waker/signaller scans the process table
to see if any processes are waiting on that channel.  If so, it
marks them as runnable. Because of this implementation,
there is no need to pass the spinlock into wakeup.

Because there is no linked list, the implementation does NOT 
ensure that waiters are woken in the order in which they go to sleep; 
instead it wakes up everyone, and so all users of sleep/wakeup
must expect spurious wakeups.

A final bit of complexity in the process abstraction concerns the
context switch code (`swtch.S`).   You will need to read Chapter 4,
look at the code, trace the execution into and out of context switch,
and so forth, before you really understand it. For lab1, you can ignore
it for now.

### Memory management

Although the memory management code is complex (reflecting the
intracacies of the x86), the abstractions are fairly straightforward.
The key idea is that we have two sets of page tables for every process.
One is machine-dependent, with a structure defined by the particular
architecture.  For example, on a 64-bit x86, there are four levels of
page tables, a particular format for each page table entry, etc.

In addition, xk has a machine-independent page table, represented
as a set of `vregions` -- contiguous regions of the virtual address space
used for a specific purpose (such as to hold code or data). The reason for
this split is to (i) make xk more portable (not tied to a specific page
table format), and (ii) to allow extra information to be stored per
page as necessary (the reason for this will become obvious in later 
assignments). 

We have routines to generate the machine-dependent page table from
the machine-independent one on demand -- essentially, every time
you make a change to the machine-independent one, you'll need to 
invalidate (`vspace.c:vspaceinvalidate`) the old machine-dependent
page table, to get the system to generate a new (machine dependent) one.

Thus, a process has information about its virtual address space, 
its file descriptors, and its current execution state.

`vspace.h:vspace` describes the virtual address space of a process. 
It consists of a set of virtual memory regions (`vspace.h:vregion`),
and also a pointer to the x86 page table.

`vspace.h:vregion` is a 4KB page aligned region of virtual memory 
of indefinite page-aligned length. A set of `vregions` defines the virtual
addresses accessible to a process.  Addresses outside of a `vregion` are considered
invalid.

Operations on `vregions` include: extending the region, filling a 
region from a file, etc.

A `vregion` is represented as a linked table (`vpi_page`), where each table 
is 4KB in size, with `4KB/sizeof(vpage_info)` entries.
The last entry in the table is a pointer to the next table, 
in the case a region is too large to fit in a single table.

Note that a `vregion` can either grow up (if its a heap or code) or
down (if its a stack). This is the meaning of `vregion.dir` (direction).

Each page in the `vregion` is represented by a `vpage_info`, essentially
an abstract page table entry.

### File system

For the most part, you can ignore the details of the file system
until lab5. However, in lab1 we ask you to implement the system calls
relating to opening, closing, and reading files.  Since the file 
system code is provided as part of the baseline, this primarily 
means implementing the code to keep track of open file descriptors 
-- e.g., the current offset into the file for where you are reading.
Files can be opened by multiple processes (or even multiple times
by the same process!), so we'll need reference counts on the number 
of times a file is open. 

The on-disk data structure representing a file is called
an inode. A copy of the inode can be stored in memory, e.g., if the file
is open.  If so, there is a bit more information kept, e.g., the
reference count. Open loads an inode if it doesn't already exist
in memory; readi and writei are operations to read/write the inode file.

The file system locates the on-disk inode by doing a directory lookup.
This is simpler in xk than in regular UNIX as there is only one
"root" directory holding all files (no subdirectories). The directory contains
(filename, inumber) pairs, where the inumber is an index into a file
containing the inodes.  The inodes don't actually store the file data,
they simply record where on disk to find the file data for that file.

We provide a design document for lab 1, (i) to encourage
a design that will work better for later labs and (ii) to give you
a concrete example of what we mean by a design document. 
You will find it very very useful to write an explicit design 
document before each of the other labs.
