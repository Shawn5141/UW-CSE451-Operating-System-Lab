# Lab 2 Design Doc: Multiprocessing

## Overview
The goal  of this lab is to add multiprocessing to xk. This will be done using classic 
UNIX system calls. This lab will also implement basic interprocess communication through pipes.


### Major Parts
  - Synchronization issues: Code from lab 1 should be modified to support
    system calls called concurrently. Locks on all critical sections.
          Not sure why lock is better to use (either spinlock / sleeplock /or sleep ). //Fix this line

  - fork: Duplicate a current process to childern process with same open file descriptor.
   
  - wait: Suspend execution until a child terminates (calls exit or is killed). 
          Returns process ID of that child process; hangs if no child process 
          has terminated (until one does); returns -1 on error. 
          A child is returned from wait only once.

  - exit: Return the allocated memory back to the kernel heap

  - exec:  Given a pathname for an executable file, sys_exec() runs that file 
           in the context of the current process (e.g., with the same open file 
           descriptors). arg1 is an array of strings; arg1[0] is the name of the 
           file; arg1[1] is the first argument; arg1[n] is '\0' signalling the
           end of the arguments.
  
  - pipes: Creates a pipe and two open file descriptors. The file descriptors
           are written to the array at arg0, with arg0[0] the read end of the 
           pipe and arg0[1] as the write end of the pipe.
 

## In-depth Analysis and Implementation

### Synchronization issues 
- Place locks around critical sections
- global file table will use a spinlock
- acquire lock before any file function called
- release lock 
- Create new spinlock on every allocated `pipe`

### fork: 
- A new entry in the process table must be created via `allocproc`
- User memory must be duplicated via `vspacecopy`
- The trapframe must be duplicated in the new process
- All the opened files must be duplicated in the new process (not as simple as a memory copy)
- Set the state of the new process to be `RUNNABLE`
- Return 0 in the child, while returning the child s pid in the parent
    
### wait:
- Scan through table looking for exited children 
- Suspend execution until a child calls `exit` or is killed 
- Return pid of child process or -1 if this process has no children

### exit:
- Mark the process as a `ZOMBIE` 
- Close all opened files within process via `syscall_close`
- Wake up parent or init process 
- Reclaims resources consumed by progam via `vspacefree` //Is this correct here?
- Does not return

### exec:
- Run the given file in the context of the current process
- overwrite current user's virtual address space 
- Read program and load it via `vspaceloadcode`
- Create deep copy of arguments from old address space to new one via `vscpacewriteova`
- Does not return on success. Returns -1 on error

### pipes:
- pipe_open(int fd1,int fd2): create two files descriptor in process file tablepointing to a pipe in global file info struct.
- pipe_write(int fd,char* buf, int offset): acquire a lock and write to pipe,  then release lock. Spin or sleep if pipe buffer is full.
- pipe_read(int fd,char* buf,int offset): acquire a lock and read from pipe, then release this lock. Spin or sleep if pipe is empty.       
-functions you need to modiy:
     (1) file_info strcut in file.h: add boolean type isPipe flag.
     (2) file_read() in file.c: if isPipe is true, use pipe_read function, otherwise use Lab1 file_read function.
     (3) file_write() in file.c: if isPipe is true, use pipe_write function, otherwise use Lab1 file_write function.
- corner cases:.
     (1) Empty and full state need to be handled. Will use circular buffer to determine whether it is full or empty. (If read is chased by write, then full. On the other hand, if write is chased by read, it is empty state)
- test plan:.
     (1) Transferring the data.
     (2) Full state and empty state.
     (3) Race competition
- file will be modified: .
     (1) file.h. 
     (2) file.c
   

## Risk Analysis

### Unanswered Questions
(1) write down everything you have not figured out yet
- When to acquire locks for parent/child process? 

### Staging of Work
(3) few sentences on how we plan to stage the work so that if our estimated 
turn out to be closer to the worst case, we will be able 
to jettison some lower priority features. 

### Time Estimation
(2) estimate average hours of implementation
    - best scenario
    - worst scenario

- Synchronization issues:
- sys_fork:
- sys_exit:
- sys_wait:
- sys_pipe:
- sys_exec:
