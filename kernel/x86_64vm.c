#include <param.h>
#include <cdefs.h>
#include <defs.h>
#include <x86_64.h>
#include <memlayout.h>
#include <mmu.h>
#include <proc.h>
#include <segment.h>
#include <elf.h>
#include <msr.h>
#include <fs.h>
#include <file.h>

extern char data[];  // defined by kernel.ld
pml4e_t *kpml4;  // for use in scheduler()

// Set up CPU's kernel segment descriptors.
// Run once on entry on each CPU.
void
seginit(void)
{
  struct cpu *c = &cpus[cpunum()];

  uint64_t *gdt;
  uint *tss;
  uint64_t addr;

  gdt = (uint64_t*)c->gdt;
  tss = (uint*) &(c->ts);
  tss[16] = 0x00680000; // IO Map Base = End of TSS

  addr = (uint64_t) tss;
  gdt[0] = 0; // first entry is 0
  c->gdt[SEG_KCODE] = SEG64(STA_X, 0, 0, 1, 0);
  c->gdt[SEG_KDATA] = SEG64(STA_W, 0, 0, 0, 0);
  c->gdt[SEG_KCPU]  = SEG64(STA_W, &c->cpu, 8, 0, 0);
  c->gdt[SEG_UCODE] = SEG64(STA_X, 0, 0, 1, DPL_USER);
  c->gdt[SEG_UDATA] = SEG64(STA_W, 0, 0, 0, DPL_USER);
  c->gdt[SEG_TSS] = SEG16(STS_T64A, addr, sizeof(struct tss), DPL_USER);
  gdt[SEG_TSS+1] = (addr >> 32);

  lgdt((void*) gdt, 8 * sizeof(uint64_t));
  ltr(SEG_TSS << 3);

  loadgs(SEG_KCPU << 3);
  wrmsr(MSR_IA32_GS_BASE, (uint64_t)&c->cpu);
  wrmsr(MSR_IA32_KERNEL_GS_BASE, (uint64_t)&c->cpu);

  // Initialize cpu-local storage.
  c->cpu = c;
  c->proc = 0;
};


// Return the address of the PTE in page table pgdir
// that corresponds to virtual address va.  If alloc!=0,
// create any required page table pages.
pte_t *
walkpml4(pml4e_t *pml4, const void *va, int alloc)
{
  pml4e_t *pml4e;
  pdpte_t *pdpt, *pdpte;
  pde_t *pgdir, *pde;
  pte_t *pgtab;

  pml4e = &pml4[PML4_INDEX(va)];

  if (*pml4e & PTE_P) {
    pdpt = (pdpte_t*)P2V(PDPT_ADDR(*pml4e));
  } else {
    if(!alloc || (pdpt = (pdpte_t*)kalloc()) == 0)
      return 0;
    memset(pdpt, 0, PGSIZE);
    *pml4e = V2P(pdpt) | PTE_P | PTE_W | PTE_U;
  }

  pdpte = &pdpt[PDPT_INDEX(va)];

  if (*pdpte & PTE_P) {
    pgdir = (pde_t*)P2V(PDE_ADDR(*pdpte));
  } else {
    if(!alloc || (pgdir = (pde_t*)kalloc()) == 0)
      return 0;
    memset(pgdir, 0, PGSIZE);
    *pdpte = V2P(pgdir) | PTE_P | PTE_W | PTE_U;
  }

  pde = &pgdir[PD_INDEX(va)];

  if (*pde & PTE_P) {
    pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
  } else {
    if(!alloc || (pgtab = (pte_t*)kalloc()) == 0)
      return 0;
    memset(pgtab, 0, PGSIZE);
    *pde = V2P(pgtab) | PTE_P | PTE_W | PTE_U;
  }

  return &pgtab[PT_INDEX(va)];
}


// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa. va and size might not
// be page-aligned.
int
mappages(pml4e_t *pml4, uint64_t virt_pn, int num_page, uint64_t phy_pn, int perm, int kern)
{
  pte_t *pte;
  int i;

  for(i=0;i<num_page;i++){
    if((pte = walkpml4(pml4, (char*)(virt_pn << PT_SHIFT), 1)) == 0) {
      panic("not enough memory");
      return -1;
    }
    if(*pte & PTE_P)
      panic("remap");

    *pte = PTE(phy_pn << PT_SHIFT, perm);

    if (!kern)
      mark_user_mem(phy_pn << PT_SHIFT, virt_pn << PT_SHIFT);

    virt_pn ++;
    phy_pn ++;
  }
  return 0;
}


// Set up kernel part of a page table.
pml4e_t*
setupkvm(void)
{
  pml4e_t *pml4;
  struct kmap *k;

  if((pml4 = (pml4e_t*)kalloc()) == 0)
    return 0;
  memset(pml4, 0, PGSIZE);

  struct kmap {
    void *virt;
    uint64_t phys_start;
    uint64_t phys_end;
    int perm;
  } kmap[] = {
    { (void*)KERNBASE, 0,             EXTMEM,    PTE_W}, // I/O space
    { (void*)KERNLINK, V2P(KERNLINK), V2P(data), 0},     // kern text+rodata
    { (void*)data,     V2P(data),     (uint64_t) npages * PGSIZE,   PTE_W}, // kern data+memory
    { (void*)DEVSPACE, 0xFE000000,    0x100000000,         PTE_W}, // more devices
  };

  for(k = kmap; k < &kmap[NELEM(kmap)]; k++) {
    if(mappages(pml4, (uint64_t)(k->virt) >> PT_SHIFT, (k->phys_end - k->phys_start) >> PT_SHIFT, k->phys_start >> PT_SHIFT, k->perm | PTE_P, 1) < 0)
      return 0;
  }
  return pml4;
}

uint64_t
find_next_possible_page(pml4e_t *pml4, uint64_t va)
{
  pml4e_t *pml4e;
  pdpte_t *pdpt, *pdpte;
  pde_t *pgdir, *pde;
  pml4e = &pml4[PML4_INDEX(va)];

  if (*pml4e & PTE_P) {
    pdpt = (pdpte_t*)P2V(PDPT_ADDR(*pml4e));
  } else {
    return PGADDR(PML4_INDEX(va) + 1, 0, 0, 0, 0) - PGSIZE;
  }

  pdpte = &pdpt[PDPT_INDEX(va)];

  if (*pdpte & PTE_P) {
    pgdir = (pde_t*)P2V(PDE_ADDR(*pdpte));
  } else {
    return PGADDR(PML4_INDEX(va), PDPT_INDEX(va) + 1, 0, 0, 0) - PGSIZE;
  }

  pde = &pgdir[PD_INDEX(va)];
  if (*pde & PTE_P) {
  } else {
    return PGADDR(PML4_INDEX(va), PDPT_INDEX(va), PD_INDEX(va) + 1, 0, 0) - PGSIZE;
  }

  panic("level is not missing");
}

// Deallocate user pages to bring the process size from oldsz to
// newsz.  oldsz and newsz need not be page-aligned, nor does newsz
// need to be less than oldsz.  oldsz can be larger than the actual
// process size.  Returns the new process size.
int
deallocuvm(pml4e_t *pml4, char* start, uint64_t oldsz, uint64_t newsz)
{
  pte_t *pte;
  uint64_t a, pa;

  if(newsz >= oldsz)
    return oldsz;

  a = PGROUNDUP((uint64_t)start + newsz);
  for(; a  < (uint64_t)start + oldsz; a += PGSIZE){
    pte = walkpml4(pml4, (char*)a, 0);
    if(!pte) {
      a = find_next_possible_page(pml4, a);
    }
    else if((*pte & PTE_P) != 0){
      pa = PTE_ADDR(*pte);
      if(pa == 0)
        panic("kfree");
      char *v = P2V(pa);
      kfree(v);
      *pte = 0;
    }
  }
  return newsz;
}

void
freevm_pgdir(pde_t *pgdir)
{
  uint i;
  for (i = 0; i < PTRS_PER_PD; i++) {
    if (pgdir[i] & PTE_P) {
      char *v = P2V(PTE_ADDR(pgdir[i]));
      kfree(v);
    }
  }
  kfree((char*) pgdir);
}

void
freevm_pdpt(pdpte_t *pdpt)
{
  uint i;
  for (i = 0; i < PTRS_PER_PDPT; i++) {
    if (pdpt[i] & PTE_P) {
      pde_t *pgdir = P2V(PDE_ADDR(pdpt[i]));
      freevm_pgdir(pgdir);
    }
  }
  kfree((char*) pdpt);
}


// Free a page table and all the physical memory pages
// in the user part.
void
freevm(pml4e_t *pml4)
{
  uint i;
  assertm(pml4, "freevm: no pml4");
  deallocuvm(pml4, 0, SZ_4G, 0);
  for(i = 0; i < PTRS_PER_PML4; i++){
    if(pml4[i] & PTE_P){
      pdpte_t *pdpt = P2V(PDPT_ADDR(pml4[i]));
      freevm_pdpt(pdpt);
    }
  }
  kfree((char*)pml4);
}
