#pragma once

#include <vector>

#include "../../include/ocl/opencl.h"
#include "../../include/calculator_factory.h"

namespace ocl {

    class CDevice_Program {
    public:
        cl::Device device;
        cl::Program program;
        cl::Context context;
        explicit CDevice_Program(const cl::Device& device_ref);
        void Set_Program(const cl::Program& program_ref);
    };

    class COCL_Stats_Calculator : public CStatistics_Calculator {
    private:
        std::vector<CDevice_Program> m_device_programs;
        std::vector<CDevice_Program>::iterator m_current_device;
    public:
        explicit COCL_Stats_Calculator(const std::vector<std::wstring>& device_names);
        CStatistics Analyze_Vector(const std::vector<double>& data) override;
    };

}
