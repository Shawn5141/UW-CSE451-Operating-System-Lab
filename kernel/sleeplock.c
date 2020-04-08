// Sleeping locks

#include <cdefs.h>
#include <defs.h>
#include <memlayout.h>
#include <mmu.h>
#include <param.h>
#include <proc.h>
#include <sleeplock.h>
#include <spinlock.h>
#include <x86_64.h>

void initsleeplock(struct sleeplock *lk, char *name) {
  initlock(&lk->lk, "sleep lock");
  lk->name = name;
  lk->locked = 0;
  lk->pid = 0;
}

// a sleeping lock relinquishes the processor if the lock is busy
// note mesa semantics: process can wakeup and find the lock still busy
void acquiresleep(struct sleeplock *lk) {
  acquire(&lk->lk);
  while (lk->locked) { 
    sleep(lk, &lk->lk);
  }
  lk->locked = 1;
  lk->pid = myproc()->pid;
  release(&lk->lk);
}

// a sleeping lock wakes up a waiting process, if any, on lock release
void releasesleep(struct sleeplock *lk) {
  acquire(&lk->lk);
  lk->locked = 0;
  lk->pid = 0;
  wakeup(lk); 
  release(&lk->lk);
}

int holdingsleep(struct sleeplock *lk) {
  int r;

  acquire(&lk->lk);
  r = lk->locked;
  release(&lk->lk);
  return r;
}
