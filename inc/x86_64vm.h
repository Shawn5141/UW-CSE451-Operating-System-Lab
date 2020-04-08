#pragma once
#include <mmu.h>

void      seginit(void);
void      kvmalloc(void);
pml4e_t*  setupkvm(void);
int       mappages(pml4e_t *, uint64_t, int, uint64_t, int, int);
pte_t*		walkpml4(pml4e_t*, const void*, int);
int       allocuvm(pml4e_t*, char*, uint64_t, uint64_t);
int       deallocuvm(pml4e_t*, char*, uint64_t, uint64_t);
void      freevm_pdpt(pdpte_t *pdpt);
void      freevm(pml4e_t*);
