#include "cpu_utils.h"

#include <iostream>

int main() {
    bool support_fp16 = CpuUtils::CpuSupportFp16();
    if (support_fp16) {
        std::cout << ">> support fp16\n\n";
    } else {
        std::cout << ">> do not support fp16\n\n";
    }
    return 0;
}
