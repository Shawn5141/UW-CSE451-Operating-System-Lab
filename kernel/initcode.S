#include <syscall.h>
#include <trap.h>

.globl start
start:
  call main
