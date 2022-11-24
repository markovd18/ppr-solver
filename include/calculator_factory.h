#pragma once

#include <statistics.h>
#include <memory>

class CStatistics_Calculator {
public:
    virtual CStatistics Analyze_Vector(const std::vector<double>& data) = 0;
};

std::unique_ptr<CStatistics_Calculator> create_statistics_calculator(const SPlatform& platform);
