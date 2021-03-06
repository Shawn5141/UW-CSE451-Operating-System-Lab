#include <cdefs.h>
#include <defs.h>
#include <memlayout.h>
#include <mmu.h>
#include <param.h>
#include <proc.h>
#include <spinlock.h>
#include <trap.h>
#include <x86_64.h>

// Interrupt descriptor table (shared by all CPUs).
struct gate_desc idt[256];
extern void *vectors[]; // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

int num_page_faults = 0;

int growuserstack(void);

void tvinit(void) {
  int i;

  for (i = 0; i < 256; i++)
    set_gate_desc(&idt[i], 0, SEG_KCODE << 3, vectors[i], KERNEL_PL);
  set_gate_desc(&idt[TRAP_SYSCALL], 1, SEG_KCODE << 3, vectors[TRAP_SYSCALL],
                USER_PL);

  initlock(&tickslock, "time");
}

void idtinit(void) { lidt((void *)idt, sizeof(idt)); }

void trap(struct trap_frame *tf) {
  uint64_t addr;

  if (tf->trapno == TRAP_SYSCALL) {
    if (myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if (myproc()->killed)
      exit();
    return;
  }

  switch (tf->trapno) {
  case TRAP_IRQ0 + IRQ_TIMER:
    if (cpunum() == 0) {
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case TRAP_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case TRAP_IRQ0 + IRQ_IDE + 1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case TRAP_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case TRAP_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case TRAP_IRQ0 + 7:
  case TRAP_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n", cpunum(), tf->cs, tf->rip);
    lapiceoi();
    break;

  default:
    addr = rcr2();

    if (tf->trapno == TRAP_PF) { //if page fault
      num_page_faults += 1;
      
      // Generate vregion and vpage_info
      struct vregion *vr;
      struct vpage_info *vpi;
      if((vr= va2vregion(&myproc()->vspace,addr))!=0
        && (vpi =va2vpage_info(vr,addr))!=0 ){

      struct core_map_entry* entry = (struct core_map_entry *)pa2page(vpi->ppn<<PT_SHIFT);

      if(vpi->cow_page==true && entry->ref_count > 1 && vpi->writable==0){ //references to unwritable page 
        //allocate a page
        char* data = kalloc();
	if(!data){ // kalloc fails 
	  break;
        }

	memset(data, 0, PGSIZE);
        //copy the data from the copy-on-write page
        memmove(data, P2V(vpi->ppn << PT_SHIFT), PGSIZE);
	acquire_core_map_lock();
        entry->ref_count--;   	// ref count decrement   
	//        release_core_map_lock();
	vpi->used = 1; //page is in use
        vpi->writable = VPI_WRITABLE;//make vpi writable
	vpi->present = VPI_PRESENT; // in physical memory
        vpi->cow_page = false;         //make vpi non_cow_page
        //faulting process start writing to that freshly-allocated page
        vpi->ppn = PGNUM(V2P(data)); 
	release_core_map_lock();

	vspaceinvalidate(&myproc()->vspace);
	vspaceinstall(myproc());

        break;
      } else if(vpi->cow_page==true && entry->ref_count == 1 && vpi->writable==0){ //only reference to unwritable page 
	acquire_core_map_lock();
	vpi->writable = VPI_WRITABLE;//make vpi writable
        vpi->cow_page = false;         //make vpi non_cow_page
	release_core_map_lock();


	vspaceinvalidate(&myproc()->vspace);
	vspaceinstall(myproc());
	break;

      }
   }
      //GROW U STACK ON DEMAND
      // upon hardware exception, exception handler will add memory to the stack region and resume execution
      //check if addr > stack_base -10
      if(addr < SZ_2G && addr >= SZ_2G - 10 * PGSIZE) {
	if(growuserstack() != -1) //grow stack
	  break; // resume execution
	// else normal page fault - can't handle
      }
      
      if (myproc() == 0 || (tf->cs & 3) == 0) {
        // In kernel, it must be our mistake.
        cprintf("unexpected trap %d from cpu %d rip %lx (cr2=0x%x)\n",
                tf->trapno, cpunum(), tf->rip, addr);
        panic("trap");
      }
    }

    // Assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "rip 0x%lx addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno, tf->err, cpunum(),
            tf->rip, addr);
    myproc()->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if (myproc() && myproc()->killed && (tf->cs & 3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if (myproc() && myproc()->state == RUNNING &&
      tf->trapno == TRAP_IRQ0 + IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if (myproc() && myproc()->killed && (tf->cs & 3) == DPL_USER)
    exit();
}

int growuserstack(void) {
  // similar to sbrk, but growing stack not heap 
  int res = 0;
  struct proc *p = myproc();
  struct vregion * vr = &p->vspace.regions[VR_USTACK];

  uint64_t old_bound = vr->va_base - vr->size - PGSIZE;

  //if stack already at max 10 pages
  if(vr->size >= 10 * PGSIZE)
    return -1;

  // try to add new page to user stack
  if ((res = vregionaddmap(vr,old_bound, PGSIZE,VPI_PRESENT,VPI_WRITABLE))<0)
    return -1;

  vr->size += PGSIZE;

  vspaceinvalidate(&p->vspace);
  return old_bound;
}
