# Lab 2: Multiprocessing
## Design Due 04/28/2020 (Tuesday) at 11:59pm.
## Lab Due 05/08/2020 (Friday) at 11:59pm.

## Introduction
This lab adds multiprocessing to xk. As a first step,
before you start running multiple processes, you will need
to revisit your solution to lab 1 to make sure it is correct
even when system calls are called concurrently.  (Even without
multiple processors, the timer may expire during a system call,
causing a context switch to a new process.) Next, you will implement
UNIX `fork`, `wait`, and `exit`. Exit needs to cleanly return kernel allocated
pages back to the kernel heap. Wait allows a process to pause until
one of its child processes finishes executing. Fork creates
a copy of the current process, returning from the system call in each
context (but with different return values). Then, you will implement basic
interprocess communication through pipes, which will allow transfer of data
from one process to another. Finally, you will need to implement UNIX `exec`,
loading a new program onto an existing process.

From lab2 on, we will be asking you to write a small design document. You will
turn it in using a tag. Follow the guidelines in
[how to write a design document](designdoc.md). When finished with your design
document, tag your repo with:
```
git tag lab2_design
```

Don't forget to push the tags!

## Configuration
To pull the second lab tests and description, run the command
```
git pull upstream master
```
and merge.

### Exercise
After the merge, switch all function calls to inodes with thread safe 
versions. For example, switch `writei` to `concurrent_writei`.
If you read values from inode's directly, be sure to use `locki` and `unlocki`.
It is easier to fix this in `open` by using concurrent_stati instead of reading
inode values directly.
You must pass lab1tests before you continue.
This exercise does not require any new locks.


To test lab2 code, in `kernel/Makefrag`, replace `lab1test` with `lab2test` in
```
$(O)/initcode : kernel/initcode.S user/lab1test.c $(ULIB)
	$(CC) -g -nostdinc -I inc -c kernel/initcode.S -o $(O)/initcode.o
	$(CC) -g -ffreestanding -MD -MP -mno-sse -I inc -c user/lab1test.c -o $(O)/lab1test.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0 -o $(O)/initcode.out $(O)/initcode.o $(O)/lab1test.o $(ULIB)
	$(OBJCOPY) -S -O binary $(O)/initcode.out $(O)/initcode
	$(OBJDUMP) -S $(O)/initcode.out > $(O)/initcode.asm
```
This will create a new `initcode` binary which has lab2test in it.

## Part #1: Add synchronization to files

Once multiple processes are concurrently running on xk,
xk must ensure any shared data structures are properly protected.
Take a look at the definition of ptable in `kernel/proc.c`.

``` C
struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;
```

ptable is the table of user processes.
Notice there is a `struct spinlock lock` in the data structure.
This spinlock ensures mutual exclusion -- that at most one
process accesses the ptable at the same time. Whenever the kernel needs to read
or write the process table, it must acquire the lock first and release it when done.
This type of lock can be found protecting most shared xk data structures.

xk also has condition variables for spinlocks: `wakeup1` is the equivalent
of `cv::broadcast` (there is no signal -- all waiters are woken up).
`sleep` is the equivalent of `cv::wait`; it puts the process to sleep and releases
the spinlock, and re-acquires the lock on wakeup.  

In addition to spinlocks, xk also has partial support for sleeping
locks (locks where the process gives up the processor if it needs to
wait for the lock). It is your job to think critically about when to use spinlocks
and sleeplocks.

### Exercise
Fix your lab 1 code to be safe for multiple processes.  In particular,
you will need to use a lock to protect access to the shared file descriptor
table. In your final turn-in, please document and justify your locking scheme in
the design document. Specify what types of locks you use and what scenarios
they protect against.


## Part #2: Implement the fork, wait, and exit system calls
These system calls work together as a unit to support multiprocessing.
You can start by implementing fork, but one of the first tests will
have the new process call process exit.

xk fork duplicates the state of the user-level application.
This might seem a bit silly (why do we want to have
two copies of the same process?), but it actually makes writing a shell
program much simpler, as will become clear soon. The system call fork
returns twice, once in the parent, with the return value of the process
ID (pid) of the child, and once in the child, with the return value of 0.

The open files should be shared between both the parent in the child, e.g.
calling read in the parent should advance the offset in both the parent
and the child.

### Exercise
Implement `fork` in `kernel/proc.c`. 
Once a new process is created, xk will run it concurrently via the
process scheduler. A hardware device generates a timer interrupt on
fixed intervals. If another process is RUNNABLE, the scheduler will switch
to it, essentially causing the current process to yield the CPU.

### Question #1
Describe the relationship between `scheduler`, `sched`, `swtch` in `kernel/proc.c`.

### Question #2
Describe why the child process is able to return to user-level application where `fork` is called.

#### What to Implement
```c
/*
 * Creates a new process as a copy of the current process, with
 * the same open file descriptors.
 *
 * Returns twice on success: once in the parent with the process ID
 * of the child, once in the child with a value of 0.
 * Returns -1 on error.
 *
 * Errors:
 * kernel lacks space to create new process
 */
int
sys_fork(void);
```

## Synchronization between parent and child process
One of the first things our test cases will do is to have the forked child
call process `exit`. You need to implement exit to cleanly return the
allocated memory back to the kernel heap.  A complication is that the
process calling exit cannot free all of its memory, as it is running on
a stack in a process context, and e.g., the timer interrupt code will
assume that every interruptable process can be time sliced.

The textbook describes how to address this: when a process exits,
mark the process to be a `ZOMBIE`. This means that someone else (e.g.,
the process parent or the next process to run) is responsible for cleaning
up its memory.

The `wait` system call interacts with fork and exit: it is called by
the parent process to wait for a child. Note that a parent
can create multiple child processes. Wait does not have any argument.
Kernel finds one exited child and return the child's pid.
Further note that you need to be careful with synchronization here: the
parent may call wait before any child calls exit.  In that case, the wait
should stall (e.g., use a condition variable) until one child exits.

Note that the parent need not call wait; it can exit without waiting
for the child. The child's data structures must still be reclaimed
in this case when the child does eventually exit.

There are various ways that you can implement all of this, so you should
think through the various cases and design an implementation before you start to code.
If you choose to let the parent clean up its exited children,
you will need some way to reclaim the children's
data when the parent exits first. (Note that in xk as in UNIX,
the initial(`user/init.c`) process never exits.)

### Exercise
Implement `exit` and `wait`.

#### What to Implement
```c
/*
 * Halts program and reclaims resources consumed by program.
 * Does not return.
 */
void
sys_exit(void);

/*
 * No arguments. Suspend execution until a child terminates (calls exit
 * or is killed). Returns process ID of that child process; hangs if
 * no child process has terminated (until one does); returns -1 on error.
 * A child is returned from wait only once.
 *
 * Error conditions:
 * The calling process did not create any child processes, or all
 * child processes have been returned in previous calls to wait.
 */
int
sys_wait(void);
```

### Question #3
How does process `kill` work in xk? What is its relationship with `exit`?

## Part 3: Pipes

A pipe is a sequential communication 
channel between two endpoints, supporting writes on one end, and 
reads on the other. Reads and writes are asynchronous and buffered, 
up to some internally defined size. A read will block until there are 
bytes to read (it may return a partial read); a write will block if there is
no more room in the internal buffer. Pipes are a simple way to support interprocess 
communication, especially between processes started by the shell, with the system
calls you just implemented. 

### Exercise
Add support for pipes. In terms of implementation, pipes can use a bounded buffer 
described in Chapter 5; the pipe does
not specify how large the bounded buffer needs to be, but you may
find it convenient for it to be a single 4KB kernel page you can treat the
page as a pointer to your structure, the actual buffer may be smaller than
a page as you will need room for metadata about the pipe. You should be able
to write any number of bytes to a pipe. Concurrent operations can happen 
in any order, but each operation completes as an atomic unit. 

Since pipes support concurrency, your pipe implementation will
need to use spinlocks and condition variables.  Note that with
`dup` you may also have multiple readers and writers on the same pipe.

The `pipe` system call creates a pipe (a holding area for written
data) and opens two file descriptors, one for reading and one for writing.
A write to a pipe with no open read descriptors will return an error.
A read from a pipe with no open write descriptors will return any
remaining buffered data, and then 0 to indicate EOF.

#### What to Implement
```c
/*
 * arg0: int * [2] [pointer to an array of two file descriptors]
 *
 * Creates a pipe and two open file descriptors. The file descriptors
 * are written to the array at arg0, with arg0[0] the read end of the 
 * pipe and arg0[1] as the write end of the pipe.
 *
 * return 0 on success; returns -1 on error
 *
 * Errors:
 * Some address within [arg0, arg0+2*sizeof(int)] is invalid
 * kernel does not have space to create pipe
 * kernel does not have two available file descriptors
 */
int
sys_pipe(void);
```

## Part #4: Execute user program

User programs are in the file system using “Executable and Linkable Format”
(i.e., ELF). Full information about this format is available in the
[ELF specification](https://courses.cs.washington.edu/courses/cse451/16au/readings/elf.pdf)
but you will not need to delve very deeply into the details of this format in this
class. We have provided functions (i.e., `vspaceloadcode` in `kernel/vspace.c`)
to read the program and load it into the passed in address space.

You need to setup all the required kernel data structures for the new program.
This should be slightly similar to `fork`, but the key differences are explained
below.

One thing special about `exec` compared to `fork` is the need to pass in the
arguments of the user program. In order to realize this functionality, you need to
first pull the arguments from the `exec`-ing program. With these arguments, you need to
carefully construct the user process stack and register state after loading the program
to give the `main` function in the loaded program its arguments. The arguments that
xk pulled from the exec-ing program and the arguments that xk is giving to the new
user-level program are both in user memory. However, simply copying the pointers is
not sufficient because the page tables of the two user-level programs are different.
You must create a deep copy of the argument from the old address space to the new address space
(see `kernel/vspace:vspacewritetova`).

`exec` has two arguments, a path to the executable ELF file and a pointer `argv` to
an array of strings (recall that a "string" in C is a `char *`). For example, let's say
we run:
```
$ cat a.txt b.txt
```
In the `exec` syscall, the first argument is the path to `cat` program. The second argument
is an array of `char *` where the first points to string `cat`, the second points to
string `a.txt`, the third points to string `b.txt` and the fourth element is `\0`
indicating the end of the array.

      argv
        |
        |     0   1   2   3
        |   +---+---+---+----+
        +-> | * | * | * | \0 |
            +---+---+---+----+
              |   |   |
              |   |   |
              |   |   +-> "b.txt"
              |   |
              |   +-> "a.txt"
              |
              +-> "cat"

When `exec` setups the user stack, we need to be careful. Note that every user program in
xk has the same definition of main (except the testing scripts, because we didn't load
testing scripts via `exec` yet).

``` C
int
main(int argc, char *argv[])
```

This means the first argument, `argc`, is the length of `argv`, where `argv` is a pointer
to a list of strings. In the previous example, this means you have to copy `cat`, `a.txt`
and `b.txt` to the user stack. Create an array on the user stack whose 0th index points
to `cat`, 1st index points to `a.txt`, 2nd index points to `b.txt` and 
3rd index element is the 0 pointer `\0`. Set the `%rdi` register (first argument in x86\_64 
calling convention) to be 3 (length of argv) and the `%rsi` register (second argument in
x86\_64 calling convention) to the `argv` array you created on the stack.

### Exercise
Implement `exec`.

#### What to Implement
```c
/*
 * arg0: char * [path to the executable file]
 * arg1: char * [] [array of strings for arguments]
 *
 * Given a pathname for an executable file, sys_exec() runs that file
 * in the context of the current process (e.g., with the same open file 
 * descriptors). arg1 is an array of strings; arg1[0] is the name of the 
 * file; arg1[1] is the first argument; arg1[n] is `\0' signalling the
 * end of the arguments.
 *
 * Does not return on success; returns -1 on error
 *
 * Errors:
 * arg0 points to an invalid or unmapped address
 * there is an invalid address before the end of the arg0 string
 * arg0 is not a valid executable file, or it cannot be opened
 * the kernel lacks space to execute the program
 * arg1 points to an invalid or unmapped address
 * there is an invalid address between arg1 and the first n st arg1[n] == `\0'
 * for any i < n, there is an invalid address between arg1[i] and the first `\0'
 */
int
sys_exec(void);
```

#### exec stack layout
Call `vspacedumpstack` with the newly created vspace to observe the stack contents. \
This is NOT the only possible stack format. \
Here is a solution code's stack layout for `ls`:
```
exectest: testing ls
dumping stack: base=80000000 size=4096
virtual address: 7ffffff8 data: 736c
virtual address: 7ffffff0 data: 0
virtual address: 7fffffe8 data: 7ffffff8
virtual address: 7fffffe0 data: 0
```
And for `echo` `echotest` `ok`:
```
exectest: test argument
dumping stack: base=80000000 size=4096
virtual address: 7ffffff8 data: 6b6f
virtual address: 7ffffff0 data: 0
virtual address: 7fffffe8 data: 747365746f686365
virtual address: 7fffffe0 data: 6f686365
virtual address: 7fffffd8 data: 0
virtual address: 7fffffd0 data: 7ffffff8
virtual address: 7fffffc8 data: 7fffffe8
virtual address: 7fffffc0 data: 7fffffe0
virtual address: 7fffffb8 data: 0
```

### lab2test expected output
lab2test output should be close to the following: 
```
forktest
forktest: ok
racetest: ok
fdesctest
fdesctest: ok
pipetest
pipetest: ok
extendedpipetest
extendedpipetest: ok
childpidtest
childpidtest: ok
childpidtest
exectest: testing ls
.              1 1 416
..             1 1 416
console        3 2 0
sh             2 3 19856
init           2 4 12216
cat            2 5 12416
echo           2 6 11880
grep           2 7 13672
kill           2 8 11888
ln             2 9 11888
ls             2 10 13632
rm             2 11 11944
stressfs       2 12 12320
wc             2 13 12720
zombie         2 14 11648
sysinfo        2 15 12216
lab1test       2 16 22192
lab2test       2 17 23256
lab3test       2 18 19352
lab3init       2 19 11992
lab4test       2 20 13936
lab5test_a     2 21 18160
lab5test_b     2 22 18024
lab5test_c     2 23 15408
small.txt      2 24 26
l2_share.txt   2 25 20
exectest: test argument
exectest: test output
echotest ok
exectest: ok
lab2 tests passed!!
```


## Testing and hand-in
After you implement the system calls described above. The kernel should be able to print `lab2 tests passed!`.

### Question #4
For each member of the project team, how many hours did you
spend on this lab?

Create a file `lab2.txt` in the top-level xk directory with
your answers to the questions listed above.

When you're finished, create a `end_lab2` git tag as described above.
