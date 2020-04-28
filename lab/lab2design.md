# Lab 2 Design Doc: Multiprocessing

## Overview
/* Some instruction from spec about overview
For each section, again, start with a few sentences describing goals. "Done"
Also, we need to write some description above following topics:
How the	different parts	of the design interact together.
  - Major data structures. "Not done for each fn"
  - Synchronization.       "Not done for each fn"
  - Major algorithms.      "Not done for each fn"
  - Major data structures. "Not done for each fn"
*/
//Most of description is from lab2.md. "Modified if needed." 

  - Synchronization issues.  "Not done"
          Not sure why lock is better to use (either spinlock / sleeplock /or sleep ).

  - fork: Duplicate a current process to childern process with same open file descriptor.
   
  - wait: Suspend execution until a child terminates (calls exit or is killed). 
          Returns process ID of that child process; hangs if no child process 
          has terminated (until one does); returns -1 on error. 
          A child is returned from wait only once.

  - exit: Return the allocated memory back to the kernel heap
    
  - pipes: Creates a pipe and two open file descriptors. The file descriptors
           are written to the array at arg0, with arg0[0] the read end of the 
           pipe and arg0[1] as the write end of the pipe.

  - exec:  Given a pathname for an executable file, sys_exec() runs that file 
           in the context of the current process (e.g., with the same open file 
           descriptors). arg1 is an array of strings; arg1[0] is the name of the 
           file; arg1[1] is the first argument; arg1[n] is '\0' signalling the
           end of the arguments.


## In-depth Analysis and Implementation

-
  - Synchronization issues  "Not done"
	(1) functions you need to implement

	(2) functions you need to modify

	(3) corner cases 

	(4) test plan

	(5) which file will be modified


*Example* 
### fork: 
- A new entry in the process table must be created via `allocproc`
- User memory must be duplicated via `vspacecopy`
- The trapframe must be duplicated in the new process
- All the opened files must be duplicated in the new process (not as simple as a memory copy)
- Set the state of the new process to be `RUNNABLE`
- Return 0 in the child, while returning the child s pid in the parent
    
   - wait: "Not done"
     
	(1) functions you need to implement.
	(2) functions you need to modify.
	(3) corner cases .
	(4) test plan.
	(5) which file will be modified.

   - exit: "Not done"

   - pipes: "Not done"

	(1) functions you need to implement:.
            pipe_open(int fd1,int fd2): create two files descriptor in process file tablepointing to a pipe in global file info struct.
            pipe_write(int fd,char* buf, int offset): acquire a lock and write to pipe,  then release lock. Spin or sleep if pipe buffer is full.
            pipe_read(int fd,char* buf,int offset): acquire a lock and read from pipe, then release this lock. Spin or sleep if pipe is empty. 
        (2) functions you need to modiy:.
            file_info strcut in file.h: add boolean type isPipe flag.
            file_read() in file.c: if isPipe is true, use pipe_read function, otherwise use Lab1 file_read function.
            file_write() in file.c: if isPipe is true, use pipe_write function, otherwise use Lab1 file_write function.
        (3) corner cases:.
            Empty and full state need to be handled. Will use circular buffer to determine whether it is full or empty. (If read is chased by write, then full. On the other hand, if write is chased by read, it is empty state)
        (4) test plan:.
             a. Transferring the data.
             b. Full state and empty state.
             c. Race competition
        (5) file will be modified: .
             file.h, file.c
   
   - exec: "Not done"

## Risk Analysis "Not done"
(1) write down everything you have not figured out yet
(2) estimate average hours of implementation
    - best scenario
    - worst scenario
(3) few sentences on how we plan to stage the work so that if our estimated turn out to be closer to the worst case, we will be able to jettison some lower priority features. 

