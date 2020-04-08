#include<memlayout.h>

OUTPUT_ARCH(i386:x86-64)
SECTIONS
{
 . = P2V_WO(TEXT_OFFSET);
 .text : {
  _start = .;
  *(.head.text)
  *(.text .text.*)
  _etext = .;
 }
 .rodata : {
  *(.rodata .rodata.*)
 }
 . = ALIGN(0x1000);
 PROVIDE(data = .);
 .data : {
  *(.data .data.*)
  _edata = .;
 }
 .bss : {
  *(.bss .bss.*)
 }
 . = ALIGN(0x1000);
 PROVIDE(_end = .);
}
