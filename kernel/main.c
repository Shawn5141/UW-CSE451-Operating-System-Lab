#include <cdefs.h>
#include <cpuid.h>
#include <defs.h>
#include <e820.h>
#include <memlayout.h>
#include <trap.h>

noreturn static void mpmain(void);
extern char _end[]; // first address after kernel loaded from ELF file

int main(uint64_t addr) {
  e820_init(addr);
  detect_memory();
  mem_init(_end); // phys page allocator
  vspacebootinit();
  mpinit();
  lapicinit();
  picinit();
  ioapicinit();
  consoleinit();
  uartinit(); // serial port
  cpuid_print();
  e820_print();
  cprintf("\ncpu%d: starting xk\n\n", cpunum());
  cprintf("free pages: %d\n", free_pages);
  pinit();
  tvinit();   // trap vectors
  binit();    // buffer cache
  ideinit();  // disk
  userinit(); // first user process
  mpmain();
  return 0;
}

// Common CPU setup code.
static void mpmain(void) {
  cprintf("cpu%d: starting\n", cpunum());
  idtinit(); // load idt register
  // xchg(&cpu->started, 1); // tell startothers() we're up
  scheduler(); // start running processes
}
