#include "cpu_utils.h"

#if defined(__ANDROID__) || defined(__linux__)
#include <stdint.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

#if defined(__ANDROID__)
#include <sys/auxv.h>
#define AT_HWCAP  16
#define AT_HWCAP2 26
// from arch/arm64/include/uapi/asm/hwcap.h
#define HWCAP_FPHP    (1 << 9)
#define HWCAP_ASIMDHP (1 << 10)
#endif  // __ANDROID__

#if defined(__APPLE__)
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE
#include <mach/machine.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#define __IOS__ 1
// A11
#ifndef CPUFAMILY_ARM_MONSOON_MISTRAL
#define CPUFAMILY_ARM_MONSOON_MISTRAL 0xe81e7ef6
#endif
// A12
#ifndef CPUFAMILY_ARM_VORTEX_TEMPEST
#define CPUFAMILY_ARM_VORTEX_TEMPEST 0x07d34b9f
#endif
// A13
#ifndef CPUFAMILY_ARM_LIGHTNING_THUNDER
#define CPUFAMILY_ARM_LIGHTNING_THUNDER 0x462504d2
#endif
#endif  // TARGET_OS_IPHONE
#endif  // __APPLE__

bool CpuUtils::CpuSupportFp16() {
    bool fp16arith = false;

#ifdef __aarch64__

#ifdef __ANDROID__
    unsigned int hwcap = getauxval(AT_HWCAP);
    fp16arith = hwcap & HWCAP_FPHP &&
                hwcap & HWCAP_ASIMDHP;
#endif  // __ANDROID__


#ifdef __IOS__
    unsigned int cpu_family = 0;
    size_t len = sizeof(cpu_family);
    sysctlbyname("hw.cpufamily", &cpu_family, &len, NULL, 0);
    fp16arith = cpu_family == CPUFAMILY_ARM_MONSOON_MISTRAL ||
                cpu_family == CPUFAMILY_ARM_VORTEX_TEMPEST ||
                cpu_family == CPUFAMILY_ARM_LIGHTNING_THUNDER;
#endif  // __IOS__

#endif  // __aarch64__

    return fp16arith;
}

