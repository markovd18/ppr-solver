#pragma once

#include "../include/statistics.h"
#include "../include/platform.h"


#include <vector>
#include <memory>

class CStatistics_Calculator {
public:
    virtual CStatistics Analyze_Vector(const std::vector<double>& data) = 0;
};

std::unique_ptr<CStatistics_Calculator> create_statistics_calculator(const SPlatform& platform);
