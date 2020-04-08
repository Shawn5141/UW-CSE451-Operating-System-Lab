PROJECT		?= xk
ARCH		?= x86_64
O		?= out
NR_CPUS		?= 1

CFLAGS		+= -ffreestanding -MD -MP -mno-sse
CFLAGS		+= -Wall
CFLAGS		+= -g

TAR        = tar
TAROPTS    = czf
TURNINNAME = xkturnin.tar.gz

KERNEL_CFLAGS	+= $(CFLAGS) -DNR_CPUS=$(NR_CPUS) -fwrapv -I inc -mcmodel=kernel
USER_CFLAGS	+= $(CFLAGS) -I inc

MKDIR_P		:= mkdir -p
LN_S		:= ln -s
UNAME_S		:= $(shell uname -s)

KERNEL_LDS	:= $(O)/kernel/kernel.lds

all: $(PROJECT)

include user/Makefrag
include kernel/Makefrag

ifeq ($(UNAME_S),Darwin)
USE_CLANG	?= 1
TOOLPREFIX	?= $(ARCH)-linux-gnu-
endif

ifdef USE_CLANG
CC		:= clang -target $(ARCH)-linux-gnu
CFLAGS		+= -Wno-initializer-overrides
else
CC		:= $(TOOLPREFIX)gcc
endif
LD		:= $(TOOLPREFIX)ld
AR		:= $(TOOLPREFIX)ar
RANLIB		:= $(TOOLPREFIX)ranlib
NM		:= $(TOOLPREFIX)nm
OBJCOPY		:= $(TOOLPREFIX)objcopy
OBJDUMP		:= $(TOOLPREFIX)objdump
GDB		:= $(TOOLPREFIX)gdb

QEMU		:= qemu-system-$(ARCH)
QEMUOPTS	+= -serial mon:stdio
QEMUOPTS	+= -smp cpus=$(NR_CPUS)
ifdef QEMUEXTRA
QEMUOPTS	+= $(QEMUEXTRA)
endif

HOST_CC := gcc

all: $(PROJECT) $(O)/fs.img

qemu: $(PROJECT)-qemu

qemu-gdb: $(PROJECT)-qemu-gdb

gdb: $(PROJECT)-gdb

%.asm: %.elf
	$(QUIET_GEN)$(OBJDUMP) -S $< > $@

%.bin: %.elf
	$(QUIET_GEN)$(OBJCOPY) -O binary $< $@

%.map: %.elf
	$(QUIET_GEN)$(NM) -n $< > $@

$(O)/user/%.o: user/%.S
	$(Q)$(MKDIR_P) $(@D)
	$(QUIET_CC_USER)$(CC) -o $@ -c $(USER_CFLAGS) $<

$(O)/user/%.o: user/%.c
	$(Q)$(MKDIR_P) $(@D)
	$(QUIET_CC_USER)$(CC) -o $@ -c $(USER_CFLAGS) $<

$(O)/%.o: %.S
	$(Q)$(MKDIR_P) $(@D)
	$(QUIET_CC)$(CC) -o $@ -c $(KERNEL_CFLAGS) $<

$(O)/%.o: %.c
	$(Q)$(MKDIR_P) $(@D)
	$(QUIET_CC)$(CC) -o $@ -c $(KERNEL_CFLAGS) $<

$(O)/kernel/%.o: KERNEL_CFLAGS += -I $(dir $@)

$(O)/%assym.h: $(O)/%genassym.o
	$(QUIET_GEN)NM=$(NM) scripts/genassym.sh $< > $@

$(O)/%.ll: %.c
	$(Q)$(MKDIR_P) $(@D)
	$(QUIET_CC_IR)$(LLVM_CC) -o "$(subst .o,.ll,$@)" -c -S -emit-llvm $(KERNEL_CFLAGS) -Wno-initializer-overrides -O2 $<

$(O)/%.py: $(O)/%.ll irpy/compiler/irpy
	$(Q)$(MKDIR_P) $(@D)
	@touch $(join $(dir $@), __init__.py)
	$(QUIET_IRPY)./irpy/compiler/irpy "$<" > "$@"

$(O)/%.lds: %.lds.S
	$(Q)$(MKDIR_P) $(@D)
	$(QUIET_GEN)$(CPP) -o $@ -P $(KERNEL_CFLAGS) $<

$(O)/%.dtb: %.dts
	$(Q)$(MKDIR_P) $(@D)
	$(QUIET_GEN)dtc -o $@ -O dtb $<

clean:
	-rm -rf $(O) .gdbinit .gdbinit.tmpl1

turnin:
	$(TAR) $(TAROPTS) $(TURNINNAME) inc kernel user Makefile mkfs.c sign.pl *.txt *.pdf
