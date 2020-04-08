#pragma once

#include <cdefs.h>

// Routines to let C code use special x86_64 instructions.

static inline uchar inb(ushort port) {
  uchar data;

  asm volatile("in %1,%0" : "=a"(data) : "d"(port));
  return data;
}

static inline void insl(int port, void *addr, int cnt) {
  asm volatile("cld; rep insl"
               : "=D"(addr), "=c"(cnt)
               : "d"(port), "0"(addr), "1"(cnt)
               : "memory", "cc");
}

static inline void outb(ushort port, uchar data) {
  asm volatile("out %0,%1" : : "a"(data), "d"(port));
}

static inline void outw(ushort port, ushort data) {
  asm volatile("out %0,%1" : : "a"(data), "d"(port));
}

static inline void outsl(int port, const void *addr, int cnt) {
  asm volatile("cld; rep outsl"
               : "=S"(addr), "=c"(cnt)
               : "d"(port), "0"(addr), "1"(cnt)
               : "cc");
}

static inline void stosb(void *addr, int data, int cnt) {
  asm volatile("cld; rep stosb"
               : "=D"(addr), "=c"(cnt)
               : "0"(addr), "1"(cnt), "a"(data)
               : "memory", "cc");
}

static inline void stosl(void *addr, int data, int cnt) {
  asm volatile("cld; rep stosl"
               : "=D"(addr), "=c"(cnt)
               : "0"(addr), "1"(cnt), "a"(data)
               : "memory", "cc");
}

struct segdesc;

static inline void lgdt(struct segdesc *p, int size) {
  volatile ushort pd[5];

  pd[0] = size - 1;
  pd[1] = (uint64_t)p;
  pd[2] = (uint64_t)p >> 16;
  pd[3] = (uint64_t)p >> 32;
  pd[4] = (uint64_t)p >> 48;

  asm volatile("lgdt (%0)" : : "r"(pd));
}

struct gatedesc;

static inline void lidt(struct gatedesc *p, int size) {
  volatile ushort pd[5];

  pd[0] = size - 1;
  pd[1] = (uint64_t)p;
  pd[2] = (uint64_t)p >> 16;
  pd[3] = (uint64_t)p >> 32;
  pd[4] = (uint64_t)p >> 48;

  asm volatile("lidt (%0)" : : "r"(pd));
}

static inline void ltr(ushort sel) { asm volatile("ltr %0" : : "r"(sel)); }

static inline uint64_t readeflags(void) {
  uint64_t eflags;
  asm volatile("pushf; pop %0" : "=r"(eflags));
  return eflags;
}

static inline void loadgs(ushort v) {
  asm volatile("movw %0, %%gs" : : "r"(v));
}

static inline void cli(void) { asm volatile("cli"); }

static inline void sti(void) { asm volatile("sti"); }

static inline uint xchg(volatile uint *addr, uint newval) {
  uint result;

  // The + in "+m" denotes a read-modify-write operand.
  asm volatile("lock; xchgl %0, %1"
               : "+m"(*addr), "=a"(result)
               : "1"(newval)
               : "cc");
  return result;
}

static inline uint64_t rcr2(void) {
  uint64_t val;
  asm volatile("mov %%cr2,%0" : "=r"(val));
  return val;
}

static inline void lcr3(uint64_t val) {
  asm volatile("mov %0,%%cr3" : : "r"(val));
}

static inline uint64_t rdmsr(uint32_t msr) {
  uint32_t lo, hi;

  asm volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
  return lo | ((uint64_t)hi << 32);
}

static inline void wrmsr(uint32_t msr, uint64_t val) {
  uint32_t lo = val & 0xffffffff, hi = val >> 32;

  asm volatile("wrmsr" : : "c"(msr), "a"(lo), "d"(hi) : "memory");
}

static inline void cpuid(uint32_t info, uint32_t *eaxp, uint32_t *ebxp,
                         uint32_t *ecxp, uint32_t *edxp) {
  uint32_t eax, ebx, ecx, edx;
  asm volatile("cpuid"
               : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
               : "a"(info));
  if (eaxp)
    *eaxp = eax;
  if (ebxp)
    *ebxp = ebx;
  if (ecxp)
    *ecxp = ecx;
  if (edxp)
    *edxp = edx;
}
