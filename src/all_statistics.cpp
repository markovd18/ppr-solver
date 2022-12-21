#include <vector>
#include <memory>
#include <string>

#include "include/all_statistics.h"
#include "include/cpu/cpu_statistics.h"
#include "include/ocl/ocl_statistics.h"

all::CAll_Stats_Calculator::CAll_Stats_Calculator() {
    m_calculators.emplace_back(std::make_unique<cpu::CPar_Stats_Calculator>());
    std::vector<std::wstring> names;
    m_calculators.emplace_back(std::make_unique<ocl::COCL_Stats_Calculator>(names));

    m_current_calculator = m_calculators.begin();
}

CStatistics all::CAll_Stats_Calculator::Analyze_Vector(const std::vector<double>& data) {
    // selects in round-robin-like way next device to execute on
    // if we're at the end, returns to the beginning
    const auto& calculator = *m_current_calculator;
    m_current_calculator++;
    if (m_current_calculator == m_calculators.end()) {
        m_current_calculator = m_calculators.begin();
    }

    return calculator->Analyze_Vector(data);
}
