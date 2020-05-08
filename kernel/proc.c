#include <cdefs.h>
#include <defs.h>
#include <file.h>
#include <fs.h>
#include <memlayout.h>
#include <mmu.h>
#include <param.h>
#include <proc.h>
#include <spinlock.h>
#include <trap.h>
#include <x86_64.h>
#include <fs.h>
#include <file.h>
#include <vspace.h>

// process table
struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;
int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

// to test crash safety in lab5, 
// we trigger restarts in the middle of file operations
void reboot(void) {
  uint8_t good = 0x02;
  while (good & 0x02)
    good = inb(0x64);
  outb(0x64, 0xFE);
loop:
  asm volatile("hlt");
  goto loop;
}

void pinit(void) { initlock(&ptable.lock, "ptable"); }

// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc *allocproc(void) {
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if (p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  p->killed = 0;

  release(&ptable.lock);

  // Allocate kernel stack.
  if ((p->kstack = kalloc()) == 0) {
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trap_frame *)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 8;
  *(uint64_t *)sp = (uint64_t)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context *)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->rip = (uint64_t)forkret;

  return p;
}

// Set up first user process.
void userinit(void) {
  struct proc *p;
  extern char _binary_out_initcode_start[], _binary_out_initcode_size[];

  p = allocproc();

  initproc = p;
  assertm(vspaceinit(&p->vspace) == 0, "error initializing process's virtual address descriptor");
  vspaceinitcode(&p->vspace, _binary_out_initcode_start, (int64_t)_binary_out_initcode_size);
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ss = (SEG_UDATA << 3) | DPL_USER;
  p->tf->rflags = FLAGS_IF;
  p->tf->rip = VRBOT(&p->vspace.regions[VR_CODE]);  // beginning of initcode.S
  p->tf->rsp = VRTOP(&p->vspace.regions[VR_USTACK]);

  safestrcpy(p->name, "initcode", sizeof(p->name));

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);
  p->state = RUNNABLE;
  release(&ptable.lock);
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int fork(void) {

   // A new entry in the process table must be created via `allocproc`
   struct proc *p;
   p=allocproc(); 
   if(p == 0) return -1;

   assertm(vspaceinit(&p->vspace) == 0, "error initializing process's virtual address descriptor");
   // User memory must be duplicated via `vspacecopy`
   vspacecopy(&p->vspace,&myproc()->vspace);

// The trapframe must be duplicated in the new process
   //copy parent trap frame to child
   memmove(p->tf, myproc()->tf, sizeof(*p->tf)); //sizeof trapframe struct?
   //copy parent proc file table and update ref count
   for (int i=0; i < NOFILE; i++) {
     acquire(&ptable.lock);
     if(myproc()->pftable[i] != NULL) {
       p->pftable[i] = &(*(myproc()->pftable[i]));
      //if(p->pftable[i]->isPipe) cprintf("\nbefore forking %d with ref num = %d\t ",i,p->pftable[i]->ref);
       p->pftable[i]->ref++;
      //if(p->pftable[i]->isPipe) cprintf("after forking %d with ref num = %d\n",i,p->pftable[i]->ref);
     }
     release(&ptable.lock);
   }

// Set the state of the new process to be `RUNNABLE`
  acquire(&ptable.lock);
  p->state = RUNNABLE;
  p->tf->rax = 0;
  p->parent = myproc(); //set the parent 
  release(&ptable.lock);   
  // your code here
  
  return p->pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void exit(void) {
  // your code here
 //cprintf("\n\n====  exit calleed =======\n\n\n"); 
  //close all files open in process
  for(int i=0; i < NOFILE; i++) {
    if(myproc()->pftable[i] != NULL) {
      fileclose(i);
    }
  }

  //set init to parent process if necessary 
  acquire(&ptable.lock);
  for(int i = 0; i < NPROC; i++) {
    if (ptable.proc[i].parent->pid == myproc()->pid) {
      ptable.proc[i].parent = initproc;
      if (initproc->state == SLEEPING)
        wakeup1(initproc);
    }
  }
  wakeup1(myproc()->parent); //wake up parent
  myproc()->state = ZOMBIE; // set child to zombie - regardless of parent status. Should no longer run 
 //do not try to free vspace - cleanup in wait()
  sched(); // because lock then changed process state
  release(&ptable.lock);

}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int wait(void) {
  // your code here
  // Scan through table looking for exited children.
  int hasActiveChildren = 0;
  //cprintf("NPROC =%d, PID =%d  lock =%d  \n",NPROC,myproc()->pid,ptable.lock);
  for(int i=0; i<NPROC; i++) {
    acquire(&ptable.lock);

    if(ptable.proc[i].parent == myproc() && ptable.proc[i].state != UNUSED) {
      hasActiveChildren = 1;
    }
    release(&ptable.lock);
  }
  //cprintf("PID %d ,leave for loop when i= %d has active child %d \n",myproc()->pid,tmp,hasActiveChildren);
  if(!hasActiveChildren) return -1; //no children 

  //Look for zombie child - do reaping 
  acquire(&ptable.lock);
  while(true){
    for(int i=0; i< NPROC; i++) {
       if(ptable.proc[i].parent ==myproc() && ptable.proc[i].state==ZOMBIE){
           int child_pid =ptable.proc[i].pid;
           ptable.proc[i].state = UNUSED;
           vspacefree(&ptable.proc[i].vspace);
           kfree(ptable.proc[i].kstack);
           ptable.proc[i].kstack = 0;
           ptable.proc[i].parent = 0;
           ptable.proc[i].pid = 0;
           ptable.proc[i].killed = 0;
           release(&ptable.lock);
           return child_pid;
         }
      }
    //cprintf("pid =%d call sleep on itself\n",myproc()->pid);
    sleep(myproc(),&ptable.lock); //suspend execution
  }
  

  return -1;
}

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void scheduler(void) {
  struct proc *p;

  for (;;) {
    // Enable interrupts on this processor.
    sti();

    //cprintf("In for loop ");
    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
      if (p->state != RUNNABLE)
        continue;

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      mycpu()->proc = p;
      vspaceinstall(p);
      p->state = RUNNING;
      swtch(&mycpu()->scheduler, p->context);
      vspaceinstallkern();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      mycpu()->proc = 0;
    }
    release(&ptable.lock);
  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void sched(void) {
  int intena;

  //cprintf("PID %d lock->locked =%d, mycpu %d lock cpu %d holding %d \n",myproc()->pid,&ptable.lock.locked,mycpu(),!holding(&ptable.lock));
  if (!holding(&ptable.lock)){
    cprintf("before panic %d", !holding(&ptable.lock));
    panic("sched ptable.lock");
 }
  if (mycpu()->ncli != 1) {
    cprintf("pid : %d\n", myproc()->pid);
    cprintf("ncli : %d\n", mycpu()->ncli);
    cprintf("intena : %d\n", mycpu()->intena);

    panic("sched locks");
  }
  if (myproc()->state == RUNNING)
    panic("sched running");
  if (readeflags() & FLAGS_IF)
    panic("sched interruptible");

  intena = mycpu()->intena;
  swtch(&myproc()->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void yield(void) {
  acquire(&ptable.lock); // DOC: yieldlock
  myproc()->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void forkret(void) {
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void sleep(void *chan, struct spinlock *lk) {
  if (myproc() == 0)
    panic("sleep");

  if (lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if (lk != &ptable.lock) { // DOC: sleeplock0
    acquire(&ptable.lock);  // DOC: sleeplock1
    release(lk);
  }

  // Go to sleep.
  myproc()->chan = chan;
  myproc()->state = SLEEPING;
  //TODO
  sched();

  // Tidy up.
  myproc()->chan = 0;

  // Reacquire original lock.
  if (lk != &ptable.lock) { // DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void wakeup1(void *chan) {
  struct proc *p;

  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if (p->state == SLEEPING && p->chan == chan){
      p->state = RUNNABLE;
    }
      
}

// Wake up all processes sleeping on chan.
void wakeup(void *chan) {
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int kill(int pid) {
  struct proc *p;

  acquire(&ptable.lock);
  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    if (p->pid == pid) {
      p->killed = 1;
      // Wake process from sleep if necessary.
      if (p->state == SLEEPING){
         p->state = RUNNABLE;
      }
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void procdump(void) {
  static char *states[] = {[UNUSED] = "unused",   [EMBRYO] = "embryo",
                           [SLEEPING] = "sleep ", [RUNNABLE] = "runble",
                           [RUNNING] = "run   ",  [ZOMBIE] = "zombie"};
  int i;
  struct proc *p;
  char *state;
  uint64_t pc[10];

  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    if (p->state == UNUSED)
      continue;
    if (p->state != 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if (p->state == SLEEPING) {
      getcallerpcs((uint64_t *)p->context->rbp, pc);
      for (i = 0; i < 10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}

struct proc *findproc(int pid) {
  struct proc *p;
  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    if (p->pid == pid)
      return p;
  }
  return 0;
}
