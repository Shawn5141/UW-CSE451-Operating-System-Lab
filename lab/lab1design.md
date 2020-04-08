# Lab 1 Design Doc: Interrupts and System Calls

## Overview

The goal of this lab is to implement an interface for users to interact with persistent
media or with other I/O devices, without having to distinguish between them.

### Major Parts
File Interface: Provide an abstraction for the user that doesn't depend on the type of
"file". In user space, this will allow for seamless switching of "file" without large
changes in the code (ex: Reading input from a file vs reading from stdin, the method
for attaining bytes will be the same).

System Calls: The system call interface provides a barrier for the kernel to validate
user program input. This way, we can keep the I/O device state consistent. No user
program can directly affect the state of the kernel's structure. Furthermore, when we are
in the kernel code, we don't have to go through the syscall interface, which cuts down
superfluous error checking for trusted code.

## In-depth Analysis and Implementation

### File Interface

#### Bookkeeping
We need a structure to keep track of a logical file, things we need to keep track of:
- In memory reference count
- A reference to the inode
- Current offset
- Access permissions (readable or writable)

We will call this a `file_info` struct.

#### Kernel View
There will be a global array of all the open files on the system (bounded by `NFILE`) placed in
static memory.

#### Process View
Each process will have an array of open files (Bounded by `NOFILE`) in the process struct. We'll
use pointers to elements of the global open file table. The file descriptor will be the respective
index into the file table. (ex: stdin is typically file descriptor 0, so the corresponding file struct
will be the first element, a pointer into the global file table that is the stdin open file)

#### Functions

- `filewrite`, `fileread`:
  - Writing or reading of a "file". 
  - Will change the offset of the respective file_info struct.
  - Returns the number of bytes read/written or -1 on failure.
- `fileopen`:
  - Finds an open spot in the process open file table and has it point the global open file table entry .
  - Finds an open entry in the global open file table and allocates a new file_info struct
    for the process open file table spot to point to.
  - Will always open a device, and only open a file if permissions passed are `O_RDONLY`.
  - Returns the index into the process open file table as the file descriptor, or -1 on failure.
- `fileclose`:
  - Release the file from this process, will have to clean up if this is the last reference.
- `filedup`:
  - Will find an open spot in the process open file table and have it point to the same file_info struct
    of the first duped file. Will need to update the reference count of the file.
  - Returns the index of the newly found open spot in the process open file table, or -1 on failure.
- `filestat`:
  - Return statistics to the user about a file.

### System Calls

#### sys_open, sys_read, sys_write, sys_close, sys_dup, sys_fstat
Will need to parse arguments from the user and validate them (we never trust the user).
There are a few useful functions provided by xk:

All functions have `int n`, which will get the n'th argument. We need this because
we are direct reading the arguments from the registers. Returns 0 on success, -1 on failure

- `int argint(int n, int *ip)`: Gets an `int` argument
- `int argint64_t(int n, int64_t *ip)`: Gets a `int64_t` argument
- `int argptr(int n, char **pp, int size)`: Gets an array of `size`. Needs size
  to check array is within the bounds of the user's address space
- `int argstr(int n, char **pp)`: Tries to read a null terminated string.

Since all our system calls will be dealing with files, we think it will be useful to
add a function that validates a file descriptor:

- `int argfd(int n, int *fd)`: Will get the file descriptor, making sure it's a valid
  file descriptor (in the open file table for the process).

The main goals of the `sys_*` functions is to do argument parsing and then calling the
associated `file*` functions.

#### fs.c
We will need to use a number of functions in fs.c. These include:
- `namei`: Used to find the inode with the inputted path. Increments
    the reference count for the inode at the filesystem layer.
- `concurrent_readi`: Performs a read operation on the given inode.
- `concurrent_writei`: Performs a write operation on the given inode. 
- `concurrent_stati`: Grabs the stat struct for the given inode.
- `irelease`: Decrements the reference count for the inode.

## Risk Analysis

### Unanswered Questions

- What happens when two different process try to update data on the file?
- What happens when the user or the kernel has the maximum number of file's open?

### Staging of Work
First, the global file table will be implemented. Then the specific file functions. After, the user
open file table. Once the interface for files is complete, the system call portion will be scrubbing
the user input, validating it, and calling the respective file functions.

### Time Estimation

- File interface (__ hours)
  - Structures (__ hours)
  - Kernel portion (__ hours)
  - Process portion (__ hours)
- System calls (__ hours)
  - sys_open (__ hours)
  - sys_read (__ hours)
  - sys_write (__ hours)
  - sys_close (__ hours)
  - sys_dup (__ hours)
  - sys_fstat (__ hours)
- Edge cases and Error handling (__ hours)
