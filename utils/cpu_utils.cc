#include "cpu_utils.h"
#include "cpu_info.h"

#include <iostream>

#if defined(__ANDROID__) || defined(__linux__)
#include <stdint.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

#if defined(__ANDROID__) || defined(__linux__)
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

#ifdef __ANDROID__
    cpuinfo_android_properties cpu_prop;
    cpuinfo_arm_linux_processor processor;
    cpuinfo_arm_linux_parse_proc_cpuinfo(cpu_prop.proc_cpuinfo_hardware, &processor);
    printf("proc_cpuinfo_hardware: %s\n", cpu_prop.proc_cpuinfo_hardware);
    cpuinfo_arm_android_parse_properties(&cpu_prop);
    printf("ro_product_board: %s\n", cpu_prop.ro_product_board);
    printf("ro_board_platform: %s\n", cpu_prop.ro_board_platform);
    printf("ro_mediatek_platform: %s\n", cpu_prop.ro_mediatek_platform);
    printf("ro_arch: %s\n", cpu_prop.ro_arch);
    printf("ro_chipname: %s\n", cpu_prop.ro_chipname);
    printf("ro_hardware_chipname: %s\n", cpu_prop.ro_hardware_chipname);
    const struct cpuinfo_arm_chipset chipset = cpuinfo_arm_android_decode_chipset(&cpu_prop);
    printf("vender %d, %d, %d\n", chipset.vendor, chipset.series, chipset.model);
    if (chipset.series == cpuinfo_arm_chipset_series_samsung_exynos && chipset.model == 9810) {
        printf(">>>exynos 9810<<<\n");
        /* Exynos 9810 reports that it supports FP16 compute, but in fact only little cores do */
        return false;
    }
#endif  // __ANDROID__

#ifdef __aarch64__

#if defined(__ANDROID__) || defined(__linux__)
    unsigned int hwcap = getauxval(AT_HWCAP);
    fp16arith = hwcap & HWCAP_FPHP &&
                hwcap & HWCAP_ASIMDHP;
#endif  // __ANDROID__ || __linux__

#ifdef __IOS__
    unsigned int cpu_family = 0;
    size_t len = sizeof(cpu_family);
    sysctlbyname("hw.cpufamily", &cpu_family, &len, NULL, 0);
    fp16arith = cpu_family == CPUFAMILY_ARM_MONSOON_MISTRAL ||
                cpu_family == CPUFAMILY_ARM_VORTEX_TEMPEST ||
                cpu_family == CPUFAMILY_ARM_LIGHTNING_THUNDER;
#endif  // __IOS__

#else

#if defined(__arm__) && defined(__ANDROID__)
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
    printf(">> %x\n", processor.midr);
#endif  // __arm__ && __ANDROID__

#endif  // __aarch64__

    return fp16arith;
}

