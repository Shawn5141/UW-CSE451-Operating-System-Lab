#define ASM_FILE

#include <msr.h>
#include <segment.h>
#include <trap_support.h>
#include <memlayout.h>
#include <multiboot.h>

#define MULTIBOOT_HEADER_FLAGS	(MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_AOUT_KLUDGE)

.section .head.text
.balign	4
multiboot_header:
	.long   MULTIBOOT_HEADER_MAGIC
	.long	MULTIBOOT_HEADER_FLAGS
	.long	- (MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
	.long	V2P_WO(multiboot_header)
	.long	V2P_WO(_start)
	.long	V2P_WO(_edata)
	.long	V2P_WO(_end)
	.long	V2P_WO(start_multiboot)

/*
 * eax: magic value
 * ebx: physical address of the multiboot structure
 */
.code32
start_multiboot:
	/* check for multiboot bootloader magic */
	cmpl	$MULTIBOOT_BOOTLOADER_MAGIC, %eax
	jne	error_no_mb
	/* save multiboot pointer */
	movl	%ebx, V2P_WO(multiboot_info)
	jmp	start_bsp

start_bsp:
	/* check for the highest extended function */
	movl	$0x80000000, %eax
	cpuid
	cmpl	$0x80000001, %eax
	jb	error_no_lm

	/* check for lm (long mode) */
	movl	$0x80000001, %eax
	cpuid
	testl	$BIT32(29), %edx
	jz	error_no_lm

	# /* check for pdpe1gb (1G pages) */
	# testl	$BIT32(26), %edx
	# jz	error_no_pdpe1gb

	/* set BSP's cpunum to zero */
	movl	$MSR_IA32_TSC_AUX, %ecx
	movl	$0, %eax
	movl	$0, %edx
	wrmsr

	# /* set BSP's stack */
	movl	$(V2P_WO(STACK_TOP(0)) - TRAP_REGS_SIZE), %esp

.global	start_common
start_common:
 	/* CR4: enable PAE, PSE */
	movl	%cr4, %eax
	# orl	$(CR4_PAE|CR4_PSE|CR4_FSGSBASE), %eax
	# orl	$(CR4_PAE|CR4_PSE), %eax
	orl		$(CR4_PAE), %eax

	movl	%eax, %cr4

	/* CR3: load initial page table */
	# movl	$(0x1000 + V2P_WO(pagebase)), %eax
	movl    $(V2P_WO(kpml4_tmp)), %eax
	movl	%eax, %cr3

	/* MSR EFER: enable LME (and syscall) */
	movl	$MSR_EFER, %ecx
	rdmsr
	# orl	$(EFER_SCE|EFER_LME), %eax
	orl	$(EFER_LME), %eax
	wrmsr

	/* CR0: enable PG, WP */
	movl	%cr0, %eax
	orl	$(CR0_PG|CR0_WP), %eax
	movl	%eax, %cr0

	lgdt	V2P_WO(gdtdesc)
	movl	$GDT_DS, %eax
	movw	%ax, %ss
	movw	%ax, %ds
	movw	%ax, %es

	/* enter 64-bit mode */
	ljmp	$GDT_CS, $V2P_WO(start64)

error_no_mb:
	lea	V2P_WO(msg_no_mb), %esi
	jmp	error

error_no_lm:
	lea	V2P_WO(msg_no_lm), %esi
	jmp	error

error_no_pdpe1gb:
	lea	V2P_WO(msg_no_pdpe1gb), %esi
	jmp	error

error:
	/* write to COM1 */
	movl	$0x3f8, %edx
error_loop:
	lodsb
	test	%al, %al
	jz	error_end
	outb	%al, %dx
	jmp	error_loop
error_end:
	movl	$0x0a, %eax
	outb	%al, %dx
spin:
	hlt
	jmp	spin

.code64
start64:
  	movq $entry64high, %rax
  	jmp *%rax

.global _start
_start:
entry64high:
 	movq 	$0xFFFFFFFF80010000, %rax
  	movq 	%rax, %rsp
  	movq 	multiboot_info, %rax
  	movq    %rax, %rdi
	call	main
	jmp	spin

.section .rodata
msg_no_mb:
	.string	"no multiboot bootloader"
msg_no_lm:
	.string	"no long mode"
msg_no_pdpe1gb:
	.string	"no 1GB pages"

/* boot GDT (kernel segments only) */
gdtdesc:
	.word	gdtend - gdt - 1
	.long	V2P_WO(gdt)
gdt:
	.quad	0
	.quad	KERNEL_CS_DESC
	.quad	KERNEL_DS_DESC
gdtend:

.section .data
.balign	SZ_4K
.global	kpml4_tmp
kpml4_tmp:
	.quad	V2P_WO(kpml3low) + PTE_P + PTE_W
	.rept	512 - 2
		.quad	0
	.endr
	.quad	V2P_WO(kpml3high) + PTE_P + PTE_W

.balign	SZ_4K
kpml3low:
	.quad	V2P_WO(kpml2) + PTE_P + PTE_W
	.rept	512 - 1
	.quad	0
	.endr

.balign	SZ_4K
kpml3high:
	.rept	512 - 2
	.quad	0
	.endr
	.quad	V2P_WO(kpml2) + PTE_P + PTE_W
	.quad	0

.balign	SZ_4K
kpml2:
	index = 0
	.rept	512
	.quad	(index * SZ_2M) + PTE_P + PTE_W + PTE_PS
	index = index + 1
	.endr

.balign SZ_4K
pagebase:
    .space 10*PGSIZE

.section .bss
.balign	8
.global	multiboot_info
multiboot_info:
	.space	8
.global	multiboot2_addr
multiboot2_addr:
	.space	8
