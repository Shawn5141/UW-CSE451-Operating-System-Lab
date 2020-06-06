#include <syscall.h>
	#include <trap.h>

	.globl start
start:
	  mov $init, %rdi
	  mov $argv, %rsi
	  mov $SYS_exec, %rax
	  int $TRAP_SYSCALL

exit:
	  mov $SYS_exit, %rax
	  int $TRAP_SYSCALL
	  jmp exit

init:
	.string "/lab3init\0"

	//.string "/init\0" 

	.p2align 2
argv:
	  .quad init
	  .quad 0
	
