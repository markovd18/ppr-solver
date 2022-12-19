#pragma once

#include <vector>

#include "../../include/calculator_factory.h"
#include "../../include/statistics.h"

namespace cpu {

    class CSeq_Stats_Calculator : public CStatistics_Calculator {
    public:
        virtual CStatistics Analyze_Vector(const std::vector<double>& data) override;
    };

    class CPar_Stats_Calculator : public CStatistics_Calculator {
    public:
        virtual CStatistics Analyze_Vector(const std::vector<double>& data) override;
    };

}
