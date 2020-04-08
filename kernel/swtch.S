# Context switch code. We only need to save callee save registers
# as all other registers are saved before we reach here.
# arg0 -- context (in struct process) for the old process
# arg1 -- context (in struct process) for the new process
#
# note that xk only switches to/from the scheduler process
# the scheduler process picks the next thread to run
#
.globl swtch
swtch:
  push %rbp
  push %rbx
  push %r11
  push %r12
  push %r13
  push %r14
  push %r15

  mov %rsp, (%rdi)
  mov %rsi, %rsp

  pop %r15
  pop %r14
  pop %r13
  pop %r12
  pop %r11
  pop %rbx
  pop %rbp

  ret
