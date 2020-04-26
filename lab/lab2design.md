# Lab 2 Design Doc: Multiprocessing

## Overview
/* Some instruction from spec about overview
For each section, again, start with a few sentences describing goals.
Also, we need to write some description above following topics:
How the	different parts	of the design interact together.
  - Major data structures.
  - Synchronization.
  - Major algorithms.
  - Major data structures.
*/
//Some topic from suggested in spec
  - Synchronization issues

  - fork

  - wait/exit

  - exec




## In-depth Analysis and Implementation
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
    

## Risk Analysis
(1) write down everything you have not figured out yet
(2) estimate average hours of implementation
    - best scenario
    - worst scenario
(3) few sentences on how we plan to stage the work so that if our estimated turn out to be closer to the worst case, we will be able to jettison some lower priority features. 

