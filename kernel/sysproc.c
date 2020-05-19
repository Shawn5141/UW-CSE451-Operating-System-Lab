#include <cdefs.h>
#include <date.h>
#include <defs.h>
#include <memlayout.h>
#include <mmu.h>
#include <param.h>
#include <proc.h>
#include <x86_64.h>

int sys_crashn(void) {
  int n;
  if (argint(0, &n) < 0)
    return -1;

  crashn_enable = 1;
  crashn = n;

  return 0;
}

int sys_fork(void) { return fork(); }

void halt(void) {
  while (1)
    ;
}

void sys_exit(void) {
  // LAB2
  exit();
}

int sys_wait(void) { return wait(); }

int sys_kill(void) {
  int pid;

  if (argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int sys_getpid(void) { return myproc()->pid; }

int sys_sbrk(void) {
  // LAB3
 /*
 *  arg0: integer value of amount of memory to be added to the heap. If arg0 < 0, treat it as 0.
 *   
 *  Adds arg0 to the current heap.
 *  Returns the previous heap limit address, or -1 on error.
 *  
 *  Error condition:
 *  * Insufficient space to allocate the heap.  Note that if some space
 *  * exists but that space is insufficient to handle the complete request, 
 *  * -1 should still be returned, and nothing should be added to the heap.
 */
 int n; 
 if(argint(0,&n)<0 )
    return -1;
 if(n<0)n=0;
 return sbrk(n);  
}

int sys_sleep(void) {
  int n;
  uint ticks0;

  if (argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (myproc()->killed) {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int sys_uptime(void) {
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
