#include <cpuid.h>
#include <defs.h>
#include <x86_64.h>

static const char *names[CPUID_BIT(CPUID_NR_FLAGS, 0)] = {
    // CPUID(1): EDX
    [CPUID_FEATURE_FPU] = "fpu",
    [CPUID_FEATURE_VME] = "vme",
    [CPUID_FEATURE_DE] = "de",
    [CPUID_FEATURE_PSE] = "pse",
    [CPUID_FEATURE_TSC] = "tsc",
    [CPUID_FEATURE_MSR] = "msr",
    [CPUID_FEATURE_PAE] = "pae",
    [CPUID_FEATURE_MCE] = "mce",
    [CPUID_FEATURE_CX8] = "cx8",
    [CPUID_FEATURE_APIC] = "apic",
    [CPUID_FEATURE_SEP] = "sep",
    [CPUID_FEATURE_MTRR] = "mtrr",
    [CPUID_FEATURE_PGE] = "pge",
    [CPUID_FEATURE_MCA] = "mca",
    [CPUID_FEATURE_CMOV] = "cmov",
    [CPUID_FEATURE_PAT] = "pat",
    [CPUID_FEATURE_PSE36] = "pse36",
    [CPUID_FEATURE_PN] = "pn",
    [CPUID_FEATURE_CLFLUSH] = "clflush",
    [CPUID_FEATURE_DS] = "ds",
    [CPUID_FEATURE_ACPI] = "acpi",
    [CPUID_FEATURE_MMX] = "mmx",
    [CPUID_FEATURE_FXSR] = "fxsr",
    [CPUID_FEATURE_SSE] = "sse",
    [CPUID_FEATURE_SSE2] = "sse2",
    [CPUID_FEATURE_SS] = "ss",
    [CPUID_FEATURE_HT] = "ht",
    [CPUID_FEATURE_TM] = "tm",
    [CPUID_FEATURE_IA64] = "ia64",
    [CPUID_FEATURE_PBE] = "pbe",

    // CPUID(1): ECX
    [CPUID_FEATURE_SSE3] = "sse3",
    [CPUID_FEATURE_PCLMULQDQ] = "pclmulqdq",
    [CPUID_FEATURE_DTES64] = "dtes64",
    [CPUID_FEATURE_MONITOR] = "monitor",
    [CPUID_FEATURE_DSCPL] = "ds_cpl",
    [CPUID_FEATURE_VMX] = "vmx",
    [CPUID_FEATURE_SMX] = "smx",
    [CPUID_FEATURE_EST] = "est",
    [CPUID_FEATURE_TM2] = "tm2",
    [CPUID_FEATURE_SSSE3] = "ssse3",
    [CPUID_FEATURE_CID] = "cid",
    [CPUID_FEATURE_SDBG] = "sdbg",
    [CPUID_FEATURE_FMA] = "fma",
    [CPUID_FEATURE_CX16] = "cx16",
    [CPUID_FEATURE_XTPR] = "xtpr",
    [CPUID_FEATURE_PDCM] = "pdcm",
    [CPUID_FEATURE_PCID] = "pcid",
    [CPUID_FEATURE_DCA] = "dca",
    [CPUID_FEATURE_SSE41] = "sse4.1",
    [CPUID_FEATURE_SSE42] = "sse4.2",
    [CPUID_FEATURE_X2APIC] = "x2apic",
    [CPUID_FEATURE_MOVBE] = "movbe",
    [CPUID_FEATURE_POPCNT] = "popcnt",
    [CPUID_FEATURE_TSC_DEADLINE] = "tsc-deadline",
    [CPUID_FEATURE_AES] = "aes",
    [CPUID_FEATURE_XSAVE] = "xsave",
    [CPUID_FEATURE_OSXSAVE] = "osxsave",
    [CPUID_FEATURE_AVX] = "avx",
    [CPUID_FEATURE_F16C] = "f16c",
    [CPUID_FEATURE_RDRAND] = "rdrand",
    [CPUID_FEATURE_HYPERVISOR] = "hypervisor",

    // CPUID(0x80000001): EDX
    [CPUID_FEATURE_SYSCALL] = "syscall",
    [CPUID_FEATURE_MP] = "mp",
    [CPUID_FEATURE_NX] = "nx",
    [CPUID_FEATURE_MMXEXT] = "mmxext",
    [CPUID_FEATURE_FXSR_OPT] = "fxsr_opt",
    [CPUID_FEATURE_PDPE1GB] = "pdpe1gb",
    [CPUID_FEATURE_RDTSCP] = "rdtscp",
    [CPUID_FEATURE_LM] = "lm",
    [CPUID_FEATURE_3DNOWEXT] = "3dnowext",
    [CPUID_FEATURE_3DNOW] = "3dnow",

    // CPUID(0x80000001): ECX
    [CPUID_FEATURE_LAHF_LM] = "lahf_lm",
    [CPUID_FEATURE_CMP_LEGACY] = "cmp_legacy",
    [CPUID_FEATURE_SVM] = "svm",
    [CPUID_FEATURE_EXTAPIC] = "extapic",
    [CPUID_FEATURE_CR8LEGACY] = "cr8legacy",
    [CPUID_FEATURE_ABM] = "abm",
    [CPUID_FEATURE_SSE4A] = "sse4a",
    [CPUID_FEATURE_MISALIGNSSE] = "misalignsse",
    [CPUID_FEATURE_3DNOWPREFETCH] = "3dnowprefetch",
    [CPUID_FEATURE_OSVW] = "osvw",
    [CPUID_FEATURE_IBS] = "ibs",
    [CPUID_FEATURE_XOP] = "xop",
    [CPUID_FEATURE_SKINIT] = "skinit",
    [CPUID_FEATURE_WDT] = "wdt",
    [CPUID_FEATURE_LWP] = "lwp",
    [CPUID_FEATURE_FMA4] = "fma4",
    [CPUID_FEATURE_TCE] = "tce",
    [CPUID_FEATURE_NODEID_MSR] = "nodeid_msr",
    [CPUID_FEATURE_TBM] = "tbm",
    [CPUID_FEATURE_TOPOEXT] = "topoext",
    [CPUID_FEATURE_PERFCTR_CORE] = "perfctr_core",
    [CPUID_FEATURE_PERFCTR_NB] = "perfctr_nb",
};

static void print_feature(uint32_t *feature) {
  int i, j;

  for (i = 0; i < CPUID_NR_FLAGS; ++i) {
    if (!feature[i])
      continue;
    cprintf(" ");
    for (j = 0; j < 32; ++j) {
      const char *name = names[CPUID_BIT(i, j)];

      if ((feature[i] & BIT32(j)) && name)
        cprintf(" %s", name);
    }
    cprintf("\n");
  }
}

static bool cpuid_has(uint32_t *feature, unsigned int bit) {
  return feature[bit / 32] & BIT32(bit % 32);
}

void cpuid_print(void) {
  uint32_t eax, brand[12], feature[CPUID_NR_FLAGS] = {0};

  cpuid(0x80000000, &eax, NULL, NULL, NULL);
  if (eax < 0x80000004)
    panic("CPU too old!");

  cpuid(0x80000002, &brand[0], &brand[1], &brand[2], &brand[3]);
  cpuid(0x80000003, &brand[4], &brand[5], &brand[6], &brand[7]);
  cpuid(0x80000004, &brand[8], &brand[9], &brand[10], &brand[11]);
  cprintf("CPU: %s\n", brand);

  cpuid(1, NULL, NULL, &feature[CPUID_1_ECX], &feature[CPUID_1_EDX]);
  cpuid(0x80000001, NULL, NULL, &feature[CPUID_80000001_ECX],
        &feature[CPUID_80000001_EDX]);
  print_feature(feature);
  // Check feature bits.
  assert(cpuid_has(feature, CPUID_FEATURE_PSE));
  assert(cpuid_has(feature, CPUID_FEATURE_APIC));
}
