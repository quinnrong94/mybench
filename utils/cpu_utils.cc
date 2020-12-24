#include "cpu_utils.h"
#include "cpu_info.h"

#include <stdio.h>

#define LOGD printf
#define LOGE printf
#define ARM82 1

#if defined(__ANDROID__) || defined(__linux__)
#include <stdint.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

#if defined(__ANDROID__) || defined(__linux__)
#include <sys/auxv.h>
#define AT_HWCAP 16
#define AT_HWCAP2 26
// from arch/arm64/include/uapi/asm/hwcap.h
#define HWCAP_FPHP (1 << 9)
#define HWCAP_ASIMDHP (1 << 10)
#endif  // __ANDROID__

#if defined(__APPLE__)
#include "TargetConditionals.h"
#include <sys/sysctl.h>
#if TARGET_OS_IPHONE
#include <mach/machine.h>
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
// A14
#ifndef CPUFAMILY_ARM_FIRESTORM_ICESTORM
#define CPUFAMILY_ARM_FIRESTORM_ICESTORM 0x1B588BB3
#endif
#elif TARGET_OS_OSX
#define __OSX__ 1
// M1
#ifndef CPUFAMILY_AARCH64_FIRESTORM_ICESTORM
#define CPUFAMILY_AARCH64_FIRESTORM_ICESTORM 0x1b588bb3
#endif
#endif  // TARGET_OS_IPHONE
#endif  // __APPLE__

bool CpuUtils::CpuSupportFp16() {
    bool fp16arith = false;

#if !ARM82
    LOGD("CpuUtils::CpuSupportFp16, ARM82 is off, fp16arith = 0.\n");
    return false;
#else

// ARM82_SIMU
#if defined(ARM82_SIMU)
    LOGD("CpuUtils::CpuSupportFp16, ARM82_SIMU is on, fp16arith = 1.\n");
    return true;

// IOS
#elif defined(__IOS__)
#ifdef __aarch64__
    unsigned int cpu_family = 0;
    size_t len              = sizeof(cpu_family);
    sysctlbyname("hw.cpufamily", &cpu_family, &len, NULL, 0);
    fp16arith = cpu_family == CPUFAMILY_ARM_MONSOON_MISTRAL || cpu_family == CPUFAMILY_ARM_VORTEX_TEMPEST ||
                cpu_family == CPUFAMILY_ARM_LIGHTNING_THUNDER || cpu_family == CPUFAMILY_ARM_FIRESTORM_ICESTORM;
    LOGD("CpuUtils::CpuSupportFp16, IOS and arm64, hw.cpufamily = %x, fp16arith = %d.\n", cpu_family, fp16arith);
    return fp16arith;
#else
    LOGD("CpuUtils::CpuSupportFp16, IOS and arm32, fp16arith = 0.\n");
    return false;
#endif

// ANDROID
#elif defined(__ANDROID__)
    cpuinfo_android_properties cpu_prop;
    cpuinfo_arm_linux_processor processor;
    cpuinfo_arm_linux_parse_proc_cpuinfo(cpu_prop.proc_cpuinfo_hardware, &processor);
    cpuinfo_arm_android_parse_properties(&cpu_prop);
    auto chipset = cpuinfo_arm_android_decode_chipset(&cpu_prop);
    LOGD("CpuUtils::CpuSupportFp16, ANDROID, vendor = %d, series = %d, model = %d.\n", chipset.vendor, chipset.series,
         chipset.model);
    if (chipset.series == cpuinfo_arm_chipset_series_samsung_exynos && chipset.model == 9810) {
        LOGD("Big cores of Exynos 9810 do not support FP16 compute, fp16arith = 0.\n");
        return false;
    }
#ifdef __aarch64__
    unsigned int hwcap = getauxval(AT_HWCAP);
    fp16arith          = hwcap & HWCAP_FPHP && hwcap & HWCAP_ASIMDHP;
    LOGD("CpuUtils::CpuSupportFp16, ANDROID and arm64, hwcap = %x, fp16arith = %d.\n", hwcap, fp16arith);
    return fp16arith;
#else
    switch (processor.midr & (CPUINFO_ARM_MIDR_IMPLEMENTER_MASK | CPUINFO_ARM_MIDR_PART_MASK)) {
        case UINT32_C(0x4100D050): /* Cortex-A55 */
        case UINT32_C(0x4100D060): /* Cortex-A65 */
        case UINT32_C(0x4100D0B0): /* Cortex-A76 */
        case UINT32_C(0x4100D0C0): /* Neoverse N1 */
        case UINT32_C(0x4100D0D0): /* Cortex-A77 */
        case UINT32_C(0x4100D0E0): /* Cortex-A76AE */
        case UINT32_C(0x4800D400): /* Cortex-A76 (HiSilicon) */
        case UINT32_C(0x51008020): /* Kryo 385 Gold (Cortex-A75) */
        case UINT32_C(0x51008030): /* Kryo 385 Silver (Cortex-A55) */
        case UINT32_C(0x51008040): /* Kryo 485 Gold (Cortex-A76) */
        case UINT32_C(0x51008050): /* Kryo 485 Silver (Cortex-A55) */
        case UINT32_C(0x53000030): /* Exynos M4 */
        case UINT32_C(0x53000040): /* Exynos M5 */
            fp16arith = true;
            break;
    }
    LOGD("CpuUtils::CpuSupportFp16, ANDROID and arm32, midr = %x, fp16arith = %d.\n", processor.midr, fp16arith);
    return fp16arith;
#endif

// linux
#elif defined(__linux__)
#ifdef __aarch64__
    unsigned int hwcap = getauxval(AT_HWCAP);
    fp16arith          = hwcap & HWCAP_FPHP && hwcap & HWCAP_ASIMDHP;
    LOGD("CpuUtils::CpuSupportFp16, linux and arm64, hwcap = %x, fp16arith = %d.\n", hwcap, fp16arith);
    return fp16arith;
#else
    LOGD("CpuUtils::CpuSupportFp16, linux and arm32, fp16arith = 0.\n");
    return false;
#endif

// OSX
#elif defined(__OSX__)
#ifdef __aarch64__
    unsigned int cpu_family = 0;
    size_t len              = sizeof(cpu_family);
    sysctlbyname("hw.cpufamily", &cpu_family, &len, NULL, 0);
    fp16arith = cpu_family == CPUFAMILY_AARCH64_FIRESTORM_ICESTORM;
    LOGD("CpuUtils::CpuSupportFp16, OSX and arm64, hw.cpufamily = %x, fp16arith = %d.\n", cpu_family, fp16arith);
    return fp16arith;
#else
    LOGD("CpuUtils::CpuSupportFp16, OSX and arm32, fp16arith = 0.\n");
    return false;
#endif

// unknown
#else
    LOGE("CpuUtils::CpuSupportFp16, unknown platform, fp16arith = 0.\n");
    return false;
#endif

#endif  // ARM82
}
