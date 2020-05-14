# Lab 2 Design Doc: Multiprocessing

## Overview
The goal of this lab is to manage address spaces 

### Major Parts
  - sbrk: Allocates more memory at runtime for a process. Increases the user's heap space
   
  - Run shell commands: Load the shell after xk boots. Run commands `cat`, `echo`, `grep`, `ls`, `wc` alond with `|`

  - Grow user stack on-demand: Change to allocate only the memory needed at run-time. When a user process starts, the user stack only contains an initial page to store application arguments. Need to handle page faults by adding memory to the stack region then resume execution in the user program

  - Copy-on-Write fork: Optimize the current implementation of fork to allow multiple proccesses to share the same physical memory while still behaving as if the memory was copied

 

## In-depth Analysis and Implementation

### Sbrk:
- increment the heap by `n` bytes
- map memory into user's address space using `vregionaddmap` 
- return the previous heap limit or -1 if not enough space 

### Shell Commands: 
- modify `kernel/initcode.S`
- upon error, debug exec, pipe, and sbrk

### Grow user stack:
-
-
- Grow stack by 1 page 
- Limit of 10 pages total

### Copy-on_write fork: 
- 

### Modifications to vspacecopy
-
    

## Risk Analysis

### Unanswered Questions
-

### Staging of Work
Start this lab by ....


### Time Estimation

- sbrk:
- Shell commands:
- Grow user stack:
- Copy-on-Write fork:
