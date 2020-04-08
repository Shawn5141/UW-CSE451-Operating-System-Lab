#pragma once

#include <cdefs.h>
#include <param.h>

#define DPL_USER 0x3 // User DPL

#define CR0_PE BIT32(0)  /* protected-mode enable */
#define CR0_MP BIT32(1)  /* monitor coprocessor */
#define CR0_EM BIT32(2)  /* emulate coprocessor */
#define CR0_TS BIT32(3)  /* task switched */
#define CR0_ET BIT32(4)  /* reserved to be 1 */
#define CR0_NE BIT32(5)  /* numeric error */
#define CR0_WP BIT32(16) /* write protect */
#define CR0_PG BIT32(31) /* paging enable */

#define CR4_PSE BIT32(4)
#define CR4_PAE BIT32(5)
#define CR4_MCE BIT32(6)
#define CR4_PGE BIT32(7)
#define CR4_OSFXSR BIT32(9)
#define CR4_OSXMMEXCPT BIT32(10)
#define CR4_VMXE BIT32(13)
#define CR4_FSGSBASE BIT32(16)

#define FLAGS_CF BIT64(0)    /* carry flag */
#define FLAGS_FIXED BIT64(1) /* always 1 */
#define FLAGS_PF BIT64(2)    /* parity flag */
/* 3 - reserved */
#define FLAGS_AF BIT64(4) /* adjust flag */
/* 5 - reserved */
#define FLAGS_ZF BIT64(6)         /* zero flag */
#define FLAGS_SF BIT64(7)         /* sign flag */
#define FLAGS_TF BIT64(8)         /* trap flag */
#define FLAGS_IF BIT64(9)         /* interrupt flag */
#define FLAGS_DF BIT64(10)        /* direction flag */
#define FLAGS_OF BIT64(11)        /* overflow flag */
#define FLAGS_IOPL BITS64(13, 12) /* I/O privilege level */
#define FLAGS_NT BIT64(14)        /* nested task flag */
/* 15 - reserved */
#define FLAGS_RF BIT64(16)  /* resume flag */
#define FLAGS_VM BIT64(17)  /* virtual 8086 mode flag */
#define FLAGS_AC BIT64(18)  /* alignment check */
#define FLAGS_VIF BIT64(19) /* virtual interrupt flag */
#define FLAGS_VIP BIT64(20) /* virtual interrupt pending */
#define FLAGS_ID BIT64(21)  /* can use CPUID */

#define PTE_P BIT64(0)   /* present */
#define PTE_W BIT64(1)   /* writable */
#define PTE_U BIT64(2)   /* user */
#define PTE_PWT BIT64(3) /* write through */
#define PTE_PCD BIT64(4) /* cache disable */
#define PTE_A BIT64(5)   /* accessed */
#define PTE_D BIT64(6)   /* dirty */
#define PTE_PS BIT64(7)  /* page size */
#define PTE_G BIT64(8)   /* global */
#define PTE_AVL BITMASK64(11, 9)
#define PTE_RO                                                                 \
  BIT64(11) /* read-only page, used to implement copy-on-write fork */

#define PTE_NX BIT64(63) /* execute disable */

#define PDPT_ADDR(pdpte) ((physaddr_t)(pdpte)&BITMASK64(51, 12))
#define PDE_ADDR(pde) ((physaddr_t)(pde)&BITMASK64(51, 12))
#define PTE_ADDR(pte) ((physaddr_t)(pte)&BITMASK64(51, 12))
#define PTE_FLAGS(pte) ((physaddr_t)(pte)&0xFFF)

#define PTE_PERM_MASK (PTE_P | PTE_W | PTE_U | PTE_AVL | PTE_NX)
#define PTE_PFN_SHIFT 12

#define PAGE_SHIFT 12
#define PAGE_SIZE (UINT64_C(1) << PAGE_SHIFT)

#define PML4_SHIFT 39
#define PTRS_PER_PML4 UINT64_C(512)
#define CAP_TYPE_X86_PML4 CAP_TYPE_PAGEMAP_L3

#define PDPT_SHIFT 30
#define PTRS_PER_PDPT UINT64_C(512)
#define CAP_TYPE_X86_PDPT CAP_TYPE_PAGEMAP_L2

#define PD_SHIFT 21
#define PTRS_PER_PD UINT64_C(512)
#define PD_SIZE (UINT64_C(1) << PD_SHIFT)
#define CAP_TYPE_X86_PD CAP_TYPE_PAGEMAP_L1

#define PT_SHIFT 12
#define PTRS_PER_PT UINT64_C(512)
#define CAP_TYPE_X86_PT CAP_TYPE_PAGEMAP_L0

#define PML4_INDEX(va) ((UINTPTR_T(va) >> PML4_SHIFT) & (PTRS_PER_PML4 - 1))

#define PDPT_INDEX(va) ((UINTPTR_T(va) >> PDPT_SHIFT) & (PTRS_PER_PDPT - 1))

#define PD_INDEX(va) ((UINTPTR_T(va) >> PD_SHIFT) & (PTRS_PER_PD - 1))

#define PT_INDEX(va) ((UINTPTR_T(va) >> PT_SHIFT) & (PTRS_PER_PT - 1))

#define PGADDR(l4, l3, d, t, o)                                                \
  ((uint64_t)((l4) << PML4_SHIFT | (l3) << PDPT_SHIFT | (d) << PD_SHIFT |      \
              (t) << PT_SHIFT | (o)))

#define PTE(pa, flags) ((uint64_t)((pa) | (flags)))

#define PGSIZE 4096 // bytes mapped by a page

#define PGNUM(la) (((uint64_t)(la)) >> PT_SHIFT)

#define PGROUNDUP(sz) (((sz) + PGSIZE - 1) & ~(PGSIZE - 1))
#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE - 1))

// various segment selectors.
#define SEG_KCODE 1 // kernel code
#define SEG_KDATA 2 // kernel data+stack
#define SEG_KCPU 3  // kernel per-cpu data
#define SEG_UCODE 4 // user code
#define SEG_UDATA 5 // user data+stack
#define SEG_TSS 6   // this process's task state

// cpu->gdt[NSEGS] holds the above segments.
#define NSEGS 8
#ifndef __ASSEMBLER__

struct segdesc {
  uint lim_15_0 : 16;  // Low bits of segment limit
  uint base_15_0 : 16; // Low bits of segment base address
  uint base_23_16 : 8; // Middle bits of segment base address
  uint type : 4;       // Segment type (see STS_ constants)
  uint s : 1;          // 0 = system, 1 = application
  uint dpl : 2;        // Descriptor Privilege Level
  uint p : 1;          // Present
  uint lim_19_16 : 4;  // High bits of segment limit
  uint avl : 1;        // Unused (available for software use)
  uint rsv1 : 1;       // Reserved
  uint db : 1;         // 0 = 16-bit segment, 1 = 32-bit segment
  uint g : 1;          // Granularity: limit scaled by 4K when set
  uint base_31_24 : 8; // High bits of segment base address
};

#define SEG64(type, base, lim, resv1, dpl)                                     \
  (struct segdesc) {                                                           \
    ((lim) >> 12) & 0xffff, (uint64_t)(base)&0xffff,                           \
        ((uint64_t)(base) >> 16) & 0xff, type, 1, dpl, 1,                      \
        (uint64_t)(lim) >> 28, 0, resv1, 0, 0, (uint64_t)(base) >> 24          \
  }

#define SEG16(type, base, lim, dpl)                                            \
  (struct segdesc) {                                                           \
    (lim) & 0xffff, (uint)(base)&0xffff, ((uint)(base) >> 16) & 0xff, type, 0, \
        dpl, 1, (uint)(lim) >> 16, 0, 0, 0, 0, (uint)(base) >> 24              \
  }

struct core_map_entry {
  int available;
  short user;   // 0 if kernel allocated memory, otherwise is user
  uint64_t va;  // if it is used by kernel only, this field is 0
};

#endif

// Application segment type bits
#define STA_X 0x8 // Executable segment
#define STA_E 0x4 // Expand down (non-executable segments)
#define STA_C 0x4 // Conforming code segment (executable only)
#define STA_W 0x2 // Writeable (non-executable segments)
#define STA_R 0x2 // Readable (executable segments)
#define STA_A 0x1 // Accessed
