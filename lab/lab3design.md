# Lab 3 Design Doc: Address Space Management

## Overview
The goal of this lab is to manage address spaces 

### Major Parts
  - sbrk: Allocates more memory at runtime for a process. Increases the user's heap space and supports memory allocation and deallocation at the byte-level
   
  - Run shell commands: Load the shell after xk boots. Run commands `cat`, `echo`, `grep`, `ls`, `wc` alond with `|`

  - Grow user stack on-demand: Change to allocate only the memory needed at run-time. When a user process starts, the user stack only contains an initial page to store application arguments. Need to handle page faults by adding memory to the stack region then resume execution in the user program

  - Copy-on-Write fork: Optimize the current implementation of fork to allow multiple proccesses to share the same physical memory while still behaving as if the memory was copied


## In-depth Analysis and Implementation

### Sbrk:
- increment the heap by `n` bytes as requested user
- check if there is unused space that already exists inside the user heap
- if there is not enough space, `kalloc` to get new pages
- map memory into user's address space using `vregionaddmap` 
- special case: `vregion[VR_HEAP].size = 0` 
- return the previous heap limit or -1 if not enough space 

### Shell Commands: 
- modify `kernel/initcode.S`
- upon error, debug exec, pipe, and sbrk

### Grow user stack:
- Check if page fault and address> stack_base -10 in 'trap.c` 
- Grow stack by 1 page using 'vspaceaddmap` 
- Limit of 10 pages total

### Copy-on-write fork: 
- Initialize same amount of Virtual space for child process. Pointing to corresponding pysical space. 
- Enable read-only for all page for both child and parent vspace.
- If write happend in either child/parent process page, page fault happend and kernel will allocate new physical address for that vpage using 'vspaceaddmap` . And change bit to writable.
- If ref count for pysical page becomes 1, change bit to writable.
    

## Risk Analysis

### Unanswered Questions
- How each function in vspace work and relationship between vspace, vregion, and page table


### Time Estimation

- sbrk:               10 hr
- Shell commands:     10 hr
- Grow user stack:    10 hr
- Copy-on-Write fork: 15 hr
- Debug:              10 hr
