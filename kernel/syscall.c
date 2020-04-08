#include <cdefs.h>
#include <defs.h>
#include <memlayout.h>
#include <mmu.h>
#include <param.h>
#include <proc.h>
#include <syscall.h>
#include <sysinfo.h>
#include <trap.h>
#include <x86_64.h>
#include <vspace.h>

// User code makes a system call with INT T_SYSCALL.
// System call number in %eax.
// Arguments on the stack, from the user call to the C
// library system call function. The saved user %esp points
// to a saved program counter, and then the first argument.

#define syscall_gen_fetcher(type) \
  int \
  fetch ## type(uint64_t addr, type *ip) \
  { \
    struct vregion *r; \
    struct vspace *v; \
    v = &myproc()->vspace; \
    for (r = v->regions; r < &v->regions[NREGIONS]; r++) { \
      if (vregioncontains(r, addr, sizeof(type))) { \
        *ip = *(type *)(addr); \
        return 0; \
      } \
    } \
    return -1; \
  } \

// these create the functions:
//   - fetchint(uint64_t addr, int *p)
//       -> use to get a 4 byte integer
//
//   - fetchint64_t(uint64_t addr, uint64_t *p)
//       -> use to get an 8 byte integer
syscall_gen_fetcher(int)
syscall_gen_fetcher(int64_t)

// Fetch the nul-terminated string at addr from the current process.
// Doesn't actually copy the string - just sets *pp to point at it.
// Returns length of string, not including nul.
int
fetchstr(uint64_t addr, char **pp)
{
  struct vregion *r;
  struct vspace *v;
  char *s, *ep;

  v = &myproc()->vspace;
  for (r = v->regions; r < &v->regions[NREGIONS]; r++) {
    if (vregioncontains(r, addr, 0)) {
      *pp = (char*)addr;
      ep = (char *)VRTOP(r);
      for(s = *pp; s < ep; s++) {
        if(*s == 0)
          return s - *pp;
      }
    }
  }
  return -1;
}

static uint64_t fetcharg(int n) {
  switch (n) {
  case 0:
    return myproc()->tf->rdi;
  case 1:
    return myproc()->tf->rsi;
  case 2:
    return myproc()->tf->rdx;
  case 3:
    return myproc()->tf->rcx;
  case 4:
    return myproc()->tf->r8;
  case 5:
    return myproc()->tf->r9;
  }
  panic("more than 6 arguments for a syscall");
}

// Fetch the nth 32-bit system call argument.
int argint(int n, int *ip) {
  *ip = fetcharg(n);
  return 0;
}

// Fetch the nth 64-bit system call argument.
int argint64(int n, int64_t *ip) {
  *ip = fetcharg(n);
  return 0;
}

// Fetch the nth word-sized system call argument as a pointer
// to a block of memory of size bytes.  Check that the pointer
// lies within the process address space.
int argptr(int n, char **pp, int size) {
  int64_t i;
  struct vregion *r;
  struct vspace *v;

  if (argint64(n, &i) < 0)
    return -1;
  if (size < 0)
    return -1;

  v = &myproc()->vspace;
  for (r = v->regions; r < &v->regions[NREGIONS]; r++) {
    if (vregioncontains(r, i, size)) {
      *pp = (char*)i;
      return 0;
    }
  }
  return -1;
}

// Fetch the nth word-sized system call argument as a string pointer.
// Check that the pointer is valid and the string is nul-terminated.
// (There is no shared writable memory, so the string can't change
// between this check and being used by the kernel.)
int argstr(int n, char **pp) {
  int addr;
  if (argint(n, &addr) < 0)
    return -1;
  return fetchstr(addr, pp);
}

extern int sys_close(void);
extern int sys_dup(void);
extern int sys_exec(void);
extern int sys_exit(void);
extern int sys_fork(void);
extern int sys_fstat(void);
extern int sys_getpid(void);
extern int sys_kill(void);
extern int sys_open(void);
extern int sys_pipe(void);
extern int sys_read(void);
extern int sys_sbrk(void);
extern int sys_sleep(void);
extern int sys_wait(void);
extern int sys_write(void);
extern int sys_uptime(void);
extern int sys_sysinfo(void);
extern int sys_crashn(void);

static int (*syscalls[])(void) = {
    [SYS_fork] = sys_fork,       [SYS_exit] = sys_exit,
    [SYS_wait] = sys_wait,       [SYS_pipe] = sys_pipe,
    [SYS_read] = sys_read,       [SYS_kill] = sys_kill,
    [SYS_exec] = sys_exec,       [SYS_fstat] = sys_fstat,
    [SYS_dup] = sys_dup,         [SYS_getpid] = sys_getpid,
    [SYS_sbrk] = sys_sbrk,       [SYS_sleep] = sys_sleep,
    [SYS_uptime] = sys_uptime,   [SYS_open] = sys_open,
    [SYS_write] = sys_write,     [SYS_close] = sys_close,
    [SYS_sysinfo] = sys_sysinfo, [SYS_crashn] = sys_crashn,
};

void syscall(void) {
  int num;

  num = myproc()->tf->rax;
  if (num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    myproc()->tf->rax = syscalls[num]();
  } else {
    cprintf("%d %s: unknown sys call %d\n", myproc()->pid, myproc()->name, num);
    myproc()->tf->rax = -1;
  }
}

int sys_sysinfo(void) {
  struct sys_info *info;

  if (argptr(0, (void *)&info, sizeof(info)) < 0)
    return -1;

  info->pages_in_use = pages_in_use;
  info->pages_in_swap = pages_in_swap;
  info->free_pages = free_pages;
  info->num_page_faults = num_page_faults;
  info->num_disk_reads = num_disk_reads;

  return 0;
}
