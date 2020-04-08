#include <cdefs.h>
#include <multiboot.h>

struct multiboot_info *mbi;

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

static inline void stosb(void *addr, int data, int cnt) {
  asm volatile("cld; rep stosb"
               : "=D"(addr), "=c"(cnt)
               : "0"(addr), "1"(cnt), "a"(data)
               : "memory", "cc");
}

#define SECTSIZE 512

static void readseg(uchar *, uint, uint);

void bootmain(void) {
  uint32_t *elf;
  struct multiboot_header *hdr;
  void (*entry)(void);
  uint32_t n;

  elf = (uint32_t *)0x10000; // scratch space

  // Read 1st page off disk
  readseg((uchar *)elf, 8192, 0);

  for (n = 0; n < 8192 / 4; n++)
    if (elf[n] == MULTIBOOT_HEADER_MAGIC)
      // if ((elf[n] + elf[n+1] + elf[n+2]) == 0)
      goto found_it;

  // failure
  return;

found_it:
  hdr = (struct multiboot_header *)(elf + n);

  readseg((uchar *)hdr->load_addr, (hdr->load_end_addr - hdr->load_addr),
          (n * 4) - (hdr->header_addr - hdr->load_addr));

  // If too much RAM was allocated, then zero redundant RAM
  if (hdr->bss_end_addr > hdr->load_end_addr)
    stosb((void *)hdr->load_end_addr, 0,
          hdr->bss_end_addr - hdr->load_end_addr);

  // Call the entry point from the multiboot header.
  // Does not return!
  entry = (void (*)(void))(hdr->entry_addr);

  /* set up multiboot: fill struct multiboot_info */
  mbi->flags = MULTIBOOT_INFO_MEM_MAP;
  mbi->mmap_length = (uint32_t)mbi & (4096 - 1);
  mbi->mmap_addr = (uint32_t)mbi - mbi->mmap_length;

  asm volatile("\tmovl %0, %%eax\n"
               "\tmovl %1, %%ebx\n"
               "\tpush %2\n"
               "\tret\n"
               :
               : "r"(MULTIBOOT_BOOTLOADER_MAGIC), "r"(mbi), "r"(entry)
               : "eax", "ebx");
}

static void waitdisk(void) {
  // Wait for disk ready.
  while ((inb(0x1F7) & 0xC0) != 0x40)
    ;
}

// Read a single sector at offset into dst.
void readsect(void *dst, uint offset) {
  // Issue command.
  waitdisk();
  outb(0x1F2, 1); // count = 1
  outb(0x1F3, offset);
  outb(0x1F4, offset >> 8);
  outb(0x1F5, offset >> 16);
  outb(0x1F6, (offset >> 24) | 0xE0);
  outb(0x1F7, 0x20); // cmd 0x20 - read sectors

  // Read data.
  waitdisk();
  insl(0x1F0, dst, SECTSIZE / 4);
}

// Read 'count' bytes at 'offset' from kernel into physical address 'pa'.
// Might copy more than asked.
void readseg(uchar *pa, uint count, uint offset) {
  uchar *epa;

  epa = pa + count;

  // Round down to sector boundary.
  pa -= offset % SECTSIZE;

  // Translate from bytes to sectors; kernel starts at sector 1.
  offset = (offset / SECTSIZE) + 1;

  // If this is too slow, we could read lots of sectors at a time.
  // We'd write more to memory than asked, but it doesn't matter --
  // we load in increasing order.
  for (; pa < epa; pa += SECTSIZE, offset++)
    readsect(pa, offset);
}
