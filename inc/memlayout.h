#pragma once

#include <mmu.h>
#include <symtable.h>

#define TEXT_OFFSET SZ_1M

/* kernel stack */
#define STACK_SIZE (2 * PAGE_SIZE)
#define STACK_TOP(n) (_end + (n)*STACK_SIZE + STACK_SIZE)

#define GDT_ENTRY_CS 1
#define GDT_ENTRY_DS 2
/* TSS selector takes up two slots */
#define GDT_ENTRY_TSS 3

#define GDT_CS (GDT_ENTRY_CS << 3)
#define GDT_DS (GDT_ENTRY_DS << 3)
#define GDT_TSS (GDT_ENTRY_TSS << 3)

#define AP_ENTRY 0x7000
#define AP_OFFSET_CPUNUM 4 /* [0x7000-4, 0x7000) */
#define AP_OFFSET_STACK 8  /* [0x7000-8, 0x7000-4) */

#define EXTMEM 0x100000             // Start of extended memory
#define DEVSPACE 0xFFFFFFFFFE000000 // Other devices are at high addresses
#define KERNBASE 0xFFFFFFFF80000000
#define DEVBASE 0xFFFFFFFF40000000
#define KERNLINK (KERNBASE + EXTMEM) // Address where kernel is linked

#define V2P(a) (((uint64_t)(a)) - KERNBASE)
#define P2V(a) (((void *)(a)) + KERNBASE)
#define IO2V(a) (((void *)(a)) + 0xFFFFFFFF00000000)

#define V2P_WO(x) ((x)-KERNBASE)   // same as V2P, but without casts
#define P2V_WO(x) ((x) + KERNBASE) // same as P2V, but without casts
