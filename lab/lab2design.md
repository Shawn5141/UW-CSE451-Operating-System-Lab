# Lab 2 Design Doc: Multiprocessing

## Overview

## In-depth Analysis and Implementation

*Example* 
### fork: 
- A new entry in the process table must be created via `allocproc`
- User memory must be duplicated via `vspacecopy`
- The trapframe must be duplicated in the new process
- All the opened files must be duplicated in the new process (not as simple as a memory copy)
- Set the state of the new process to be `RUNNABLE`
- Return 0 in the child, while returning the child's pid in the parent
    
## Risk Analysis
