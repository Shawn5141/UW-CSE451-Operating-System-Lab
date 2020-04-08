#pragma once

#include <cdefs.h>

#define TRAP_DE 0  /* divide error */
#define TRAP_DB 1  /* debug */
#define TRAP_NMI 2 /* non-maskable interrupt */
#define TRAP_BP 3  /* breakpoint */
#define TRAP_OF 4  /* overflow */
#define TRAP_BR 5  /* bound range exceeded */
#define TRAP_UD 6  /* invalid opcode */
#define TRAP_NM 7  /* device not available */
#define TRAP_DF 8  /* double fault */
/* 9	reserved */
#define TRAP_TS 10 /* invalid TSS */
#define TRAP_NP 11 /* segment not present */
#define TRAP_SS 12 /* stack exception */
#define TRAP_GP 13 /* general protection fault */
#define TRAP_PF 14 /* page fault */
/* 15	reserved */
#define TRAP_MF 16 /* x87 FPU floating point */
#define TRAP_AC 17 /* aligment check */
#define TRAP_MC 18 /* machine check */
#define TRAP_XF 19 /* SIMD floating point */
#define TRAP_VE 20 /* virtualization */
#define TRAP_VC 29 /* VMM communication */
#define TRAP_SX 30 /* security */

#define TRAP_IRQ0 32
#define TRAP_SYSCALL 64 // system call

#define IRQ_TIMER 0
#define IRQ_KBD 1
#define IRQ_COM1 4
#define IRQ_IDE 14
#define IRQ_ERROR 19
#define IRQ_SPURIOUS 31

#ifndef __ASSEMBLER__

struct trap_frame {
  uint64_t rax; // rax
  uint64_t rbx;
  uint64_t rcx;
  uint64_t rdx;
  uint64_t rbp;
  uint64_t rsi;
  uint64_t rdi;
  uint64_t r8;
  uint64_t r9;
  uint64_t r10;
  uint64_t r11;
  uint64_t r12;
  uint64_t r13;
  uint64_t r14;
  uint64_t r15;
  uint64_t trapno;
  /* error code, pushed by hardware or 0 by software */
  uint64_t err;
  uint64_t rip;
  uint64_t cs;
  uint64_t rflags;
  /* ss:rsp is always pushed in long mode */
  uint64_t rsp;
  uint64_t ss;
} __packed;

struct trap_regs {
  /* callee-saved; save only on context switching */
  register_t r15;
  register_t r14;
  register_t r13;
  register_t r12;
  register_t rbp;
  register_t rbx;
  /* caller-saved registers; always save them on kernel entry */
  register_t rax;
  register_t r11;
  register_t r10;
  register_t r9;
  register_t r8;
  register_t rcx;
  register_t rdx;
  register_t rsi;
  register_t rdi;
} __packed __aligned(16);

#endif /* !__ASSEMBLER__ */
