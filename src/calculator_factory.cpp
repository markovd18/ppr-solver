#include <memory>

#include "include/platform.h"
#include "include/ocl/ocl_statistics.h"
#include "include/cpu/cpu_statistics.h"
#include "include/calculator_factory.h"

std::unique_ptr<CStatistics_Calculator> create_statistics_calculator(const SPlatform& platform) {
    if (platform.type == EPlatform_Type::OPEN_CL) {
        return std::make_unique<ocl::COCL_Stats_Calculator>(platform.args);
    }

    if (platform.type == EPlatform_Type::SMP) {
        return std::make_unique<cpu::CPar_Stats_Calculator>();
    }

    // TODO ALL
    return std::make_unique<ocl::COCL_Stats_Calculator>(platform.args);
}