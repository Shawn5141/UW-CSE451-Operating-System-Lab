#pragma once
#include <cdefs.h>

#define MSR_IA32_MTRRCAP 0x000000fe
#define MSR_IA32_MTRR_PHYSBASE(n) (0x200 + 2 * (n))
#define MSR_IA32_MTRR_PHYSMASK(n) (0x201 + 2 * (n))

/* MTRRCAP */
#define MTRRCAP_VCNT BITMASK64(7, 0)
#define MTRRCAP_FIX BIT64(8)
#define MTRRCAP_WC BIT64(10)
#define MTRRCAP_SMRR BIT64(11)

#define MTRR_PHYSBASE_PHYSBASE BITMASK64(39, 12)
#define MTRR_PHYSBASE_TYPE BITMASK64(7, 0)
#define MTRR_PHYSMASK_PHYSMASK BITMASK64(39, 12)
#define MTRR_PHYSMASK_VALID BIT64(11)

#define MTRR_UNCACHEABLE 0x00
#define MTRR_WRITE_COMBINING 0x01
#define MTRR_WRITE_THROUGH 0x04
#define MTRR_WRITE_PROTECTED 0x05
#define MTRR_WRITE_BACK 0x06

#define MSR_EFER 0xc0000080   /* extended features */
#define MSR_STAR 0xc0000081   /* syscall segments */
#define MSR_LSTAR 0xc0000082  /* long mode syscall target  */
#define MSR_CSTAR 0xc0000083  /* compatibility mode syscall target */
#define MSR_SFMASK 0xc0000084 /* syscall flags mask */

#define MSR_PLATFORM_INFO 0x000000ce

/* EFER bits */
#define EFER_SCE BIT32(0)   /* syscall/sysret enable */
#define EFER_LME BIT32(8)   /* long mode enable */
#define EFER_LMA BIT32(10)  /* long mode active */
#define EFER_NXE BIT32(11)  /* NX enable */
#define EFER_SVME BIT32(12) /* SVM enable */

/* FS and GS selector bases */
#define MSR_IA32_FS_BASE 0xc0000100
#define MSR_IA32_GS_BASE 0xc0000101
#define MSR_IA32_KERNEL_GS_BASE 0xc0000102

#define MSR_IA32_TSC_AUX 0xc0000103

#define MSR_IA32_FEATURE_CONTROL 0x0000003a

#define FEATURE_CONTROL_LOCK BIT64(0)
#define FEATURE_CONTROL_ENABLE_VMXON_INSIDE_SMX BIT64(1)
#define FEATURE_CONTROL_ENABLE_VMXON_OUTSIDE_SMX BIT64(2)

/* APIC */
#define MSR_IA32_APIC_BASE 0x0000001b
#define MSR_APIC_000 0x00000800

#define APIC_BASE_BSP BIT64(8)   /* processor is BSP */
#define APIC_BASE_EXTD BIT64(10) /* enable x2APIC mode */
#define APIC_BASE_EN BIT64(11)   /* xAPIC global enable/disable */
#define APIC_BASE_ADDRESS_MASK BITMASK64(35, 12)

/* SVM */
#define MSR_VM_CR 0xc0010114
#define MSR_VM_HSAVE_PA 0xc0010117

#define VM_CR_SVMDIS 0x10

/* VMX */
#define MSR_IA32_VMX_BASIC 0x480
#define MSR_IA32_VMX_PINBASED_CTLS 0x481
#define MSR_IA32_VMX_PROCBASED_CTLS 0x482
#define MSR_IA32_VMX_EXIT_CTLS 0x483
#define MSR_IA32_VMX_ENTRY_CTLS 0x484
#define MSR_IA32_VMX_MISC 0x485
#define MSR_IA32_VMX_CR0_FIXED0 0x486
#define MSR_IA32_VMX_CR0_FIXED1 0x487
#define MSR_IA32_VMX_CR4_FIXED0 0x488
#define MSR_IA32_VMX_CR4_FIXED1 0x489
#define MSR_IA32_VMX_PROCBASED_CTLS2 0x48b
#define MSR_IA32_VMX_EPT_VPID_CAP 0x48c
#define MSR_IA32_VMX_TRUE_PINBASED_CTLS 0x48d
#define MSR_IA32_VMX_TRUE_PROCBASED_CTLS 0x48e
#define MSR_IA32_VMX_TRUE_EXIT_CTLS 0x48f
#define MSR_IA32_VMX_TRUE_ENTRY_CTLS 0x490

/* MSR_IA32_VMX_BASIC */
#define VMX_BASIC_REVISION_ID BITMASK64(30, 0)
#define VMX_BASIC_REGION_SIZE BITMASK64(44, 32)
#define VMX_BASIC_MEMORY_TYPE BITMASK64(53, 50)
#define VMX_BASIC_TRUE_CTLS BIT64(55)

/* MSR_IA32_VMX_PINBASED_CTLS & MSR_IA32_VMX_TRUE_PINBASED_CTLS */
#define PINBASED_EXTINT_EXITING BIT32(0)
#define PINBASED_NMI_EXITING BIT32(3)
#define PINBASED_VIRTUAL_NMI BIT32(5)
#define PINBASED_PREMPTION_TIMER BIT32(6)
#define PINBASED_POSTED_INTERRUPT BIT32(7)

/* MSR_IA32_VMX_PROCBASED_CTLS & MSR_IA32_VMX_TRUE_PROCBASED_CTLS */
#define PROCBASED_INT_WINDOW_EXITING BIT32(2)
#define PROCBASED_TSC_OFFSET BIT32(3)
#define PROCBASED_HLT_EXITING BIT32(7)
#define PROCBASED_INVLPG_EXITING BIT32(9)
#define PROCBASED_MWAIT_EXITING BIT32(10)
#define PROCBASED_RDPMC_EXITING BIT32(11)
#define PROCBASED_RDTSC_EXITING BIT32(12)
#define PROCBASED_CR3_LOAD_EXITING BIT32(15)
#define PROCBASED_CR3_STORE_EXITING BIT32(16)
#define PROCBASED_CR8_LOAD_EXITING BIT32(19)
#define PROCBASED_CR8_STORE_EXITING BIT32(20)
#define PROCBASED_USE_TPR_SHADOW BIT32(21)
#define PROCBASED_MOV_DR_EXITING BIT32(23)
#define PROCBASED_IO_EXITING BIT32(24)
#define PROCBASED_IO_BITMAPS BIT32(25)
#define PROCBASED_MTF BIT32(27)
#define PROCBASED_MSR_BITMAPS BIT32(28)
#define PROCBASED_MONITOR_EXITING BIT32(29)
#define PROCBASED_PAUSE_EXITING BIT32(30)
#define PROCBASED_SECONDARY_CONTROLS BIT32(31)

/* MSR_IA32_VMX_MISC */
#define VMX_MISC_PREEMPTION_TIMER_SCALE BITMASK64(4, 0)

/* MSR_IA32_VMX_PROCBASED_CTLS2 */
#define PROCBASED2_VIRTUALIZE_APIC_ACCESSES BIT32(0)
#define PROCBASED2_ENABLE_EPT BIT32(1)
#define PROCBASED2_DESC_TABLE_EXITING BIT32(2)
#define PROCBASED2_ENABLE_RDTSCP BIT32(3)
#define PROCBASED2_VIRTUALIZE_X2APIC_MODE BIT32(4)
#define PROCBASED2_ENABLE_VPID BIT32(5)
#define PROCBASED2_WBINVD_EXITING BIT32(6)
#define PROCBASED2_UNRESTRICTED_GUEST BIT32(7)
#define PROCBASED2_APIC_REGISTER_VIRTUALIZATION BIT32(8)
#define PROCBASED2_VIRTUAL_INTERRUPT_DELIVERY BIT32(9)
#define PROCBASED2_PAUSE_LOOP_EXITING BIT32(10)
#define PROCBASED2_ENABLE_INVPCID BIT32(12)

/* MSR_IA32_VMX_EPT_VPID_CAP */
#define VMX_EPT_EXECUTE_ONLY BIT64(0)
#define VMX_EPT_PAGE_WALK_4 BIT64(6)
#define VMX_EPT_MEMORY_UC BIT64(8)
#define VMX_EPT_MEMORY_WB BIT64(14)
#define VMX_EPT_PAGE_2MB BIT64(16)
#define VMX_EPT_PAGE_1GB BIT64(17)
#define VMX_EPT_AD BIT64(21)
#define VMX_INVVPID_SUPPORTED BIT64(32)
#define VMX_INVVPID_INDIVIDUAL_ADDRESS_SUPPORTED BIT64(40)
#define VMX_INVVPID_SINGLE_CONTEXT_SUPPORTED BIT64(41)
#define VMX_INVVPID_ALL_CONTEXT_SUPPORTED BIT64(42)
#define VMX_INVVPID_SINGLE_CONTEXT_RETAINING_GLOBALS_SUPPORTED BIT64(43)

/* VM-exit controls */
#define VM_EXIT_SAVE_DEBUG_CONTROLS BIT32(2)
#define VM_EXIT_HOST_LMA BIT32(9)
#define VM_EXIT_LOAD_PERF_GLOBAL_CTRL BIT32(12)
#define VM_EXIT_ACKNOWLEDGE_INTERRUPT BIT32(15)
#define VM_EXIT_SAVE_PAT BIT32(18)
#define VM_EXIT_LOAD_PAT BIT32(19)
#define VM_EXIT_SAVE_EFER BIT32(20)
#define VM_EXIT_LOAD_EFER BIT32(21)
#define VM_EXIT_SAVE_PREEMPTION_TIMER BIT32(22)

/* VM-entry controls */
#define VM_ENTRY_LOAD_DEBUG_CONTROLS BIT32(2)
#define VM_ENTRY_GUEST_LMA BIT32(9)
#define VM_ENTRY_INTO_SMM BIT32(10)
#define VM_ENTRY_DEACTIVATE_DUAL_MONITOR BIT32(11)
#define VM_ENTRY_LOAD_PERF_GLOBAL_CTRL BIT32(13)
#define VM_ENTRY_LOAD_PAT BIT32(14)
#define VM_ENTRY_LOAD_EFER BIT32(15)
