#pragma once

#include <vector>

#include <calculator_factory.h>
#include <statistics.h>

namespace cpu {

    class CSeq_Stats_Calculator : public CStatistics_Calculator {
    private:
        //double m_n = 0.0, m_M1 = 0.0, m_M2 = 0.0, m_M3 = 0.0, m_M4 = 0.0;
    public:
        virtual CStatistics Analyze_Vector(const std::vector<double>& data) override;
    };

    class CPar_Stats_Calculator : public CStatistics_Calculator {
    public:
        virtual CStatistics Analyze_Vector(const std::vector<double>& data) override;
    };

}
