#pragma once

#include <cdefs.h>

#ifndef __ASSEMBLER__

struct symbol {
  uint64_t address;
  uint8_t type;
  const char *name;
};

const struct symbol *symtable_lookup_by_address(uint64_t address);
const struct symbol *symtable_lookup_by_name(const char *name);

extern char _start[], _etext[], _edata[], _end[];

#endif /* !__ASSEMBLER__ */
