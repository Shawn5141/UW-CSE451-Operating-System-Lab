#include <defs.h>
#include <e820.h>
#include <multiboot.h>

struct e820_map e820_map;

static const char *e820_map_types[] = {
    "available", "reserved", "ACPI reclaimable", "ACPI NVS", "unusable",
};

static void print_e820_map_type(uint32_t type) {
  switch (type) {
  case 1 ... 5:
    cprintf((char *)e820_map_types[type - 1]);
    break;
  default:
    cprintf("type %u", type);
    break;
  }
}

void e820_print() {
  uint64_t i;

  cprintf("E820: physical memory map [mem 0x%x-0x%x]\n", mmap_addr,
          mmap_addr + mmap_length - 1);

  struct e820_entry *e = e820_map.entries;
  for (i = 0; i != e820_map.nr; ++i, ++e) {
    cprintf("  [mem 0x%x-0x%x] ", (uintptr_t)e->addr,
            (uintptr_t)(e->addr + e->len - 1));
    print_e820_map_type(e->type);
    cprintf("\n");
  }
}

// This function may ONLY be used during initialization,
// before page_init().
void e820_init(physaddr_t mbi_pa) {
  struct multiboot_info *mbi;
  uint64_t addr, addr_end, i;

  mbi = (struct multiboot_info *)mbi_pa;
  assert(mbi->flags & MULTIBOOT_INFO_MEM_MAP);

  mmap_addr = mbi->mmap_addr;
  mmap_length = mbi->mmap_length;

  addr = mbi->mmap_addr;
  addr_end = mbi->mmap_addr + mbi->mmap_length;
  for (i = 0; addr < addr_end; ++i) {
    struct multiboot_mmap_entry *e;

    // Print memory mapping.
    assert(addr_end - addr >= sizeof(*e));
    e = (struct multiboot_mmap_entry *)addr;

    // Save a copy.
    assert(i < E820_NR_MAX);
    e820_map.entries[i].addr = e->addr;
    e820_map.entries[i].len = e->len;
    e820_map.entries[i].type = e->type;

    addr += (e->size + 4);
  }
  e820_map.nr = i;
}
