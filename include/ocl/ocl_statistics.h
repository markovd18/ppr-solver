#pragma once

#include <vector>

#include <ocl/opencl.h>
#include <calculator_factory.h>

namespace ocl {

    class COCL_Stats_Calculator : public CStatistics_Calculator {
    private:
        std::vector<cl::Device> m_devices;
    public:
        explicit COCL_Stats_Calculator(const std::vector<std::wstring>& device_names);
        CStatistics Analyze_Vector(const std::vector<double>& data) override;
    };

}
