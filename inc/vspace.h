#pragma once

#include <defs.h>
#include <mmu.h>

#define NREGIONS 3

enum {
  VR_CODE   = 0,
  VR_HEAP   = 1,
  VR_USTACK = 2,
};

#define VPI_PRESENT  ((short) 1)
#define VPI_WRITABLE ((short) 1)

struct vpage_info {
  short used;     // whether the page is in use
  uint64_t ppn;   // physical page number
  short present;  // whether the page is in physical memory
  short writable; // does the page have write permissions
  // user defined fields

};

#define VPIPPAGE ((PGSIZE/sizeof(struct vpage_info)) - 1)
#define VRTOP(r) \
  ((r)->dir == VRDIR_UP ? (r)->va_base + (r)->size : (r)->va_base)
#define VRBOT(r) \
  ((r)->dir == VRDIR_UP ? (r)->va_base : (r)->va_base - (r)->size)

struct vpi_page {
  struct vpage_info infos[VPIPPAGE];  // info struct for the given page
  struct vpi_page *next;              // the next page
};

enum vr_direction {
  VRDIR_UP,   // The code and heap "grow up"
  VRDIR_DOWN  // The stack "grows down"
};

struct vregion {
  enum vr_direction dir;  // direction of growth
  uint64_t va_base;       // base of the region
  uint64_t size;          // size of region in bytes
  struct vpi_page *pages;  // pointer to array of page_infos
};

struct vspace {
  struct vregion regions[NREGIONS]; // the regions for a process' virtual space
  pml4e_t* pgtbl;                   // process' page table
};

