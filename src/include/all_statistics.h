#pragma once

#include <vector>
#include <memory>

#include "calculator_factory.h"

namespace all {

    class CAll_Stats_Calculator : public CStatistics_Calculator {
    private:
        std::vector<std::unique_ptr< CStatistics_Calculator>> m_calculators;
        std::vector<std::unique_ptr< CStatistics_Calculator>>::iterator m_current_calculator;

    public:
        CAll_Stats_Calculator();
        virtual CStatistics Analyze_Vector(const std::vector<double>& data) override;
    };
};