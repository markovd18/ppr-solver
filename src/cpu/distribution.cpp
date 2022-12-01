#pragma once

#include <vector>
#include <algorithm>
#include <execution>
#include <unordered_map>
#include <string>

#include "../include/cpu/distribution.h"

std::wstring dist::get_distribution_name(const dist::EDistribution distribution) {
    switch (distribution) {
        case EXPONENTIAL: return L"exponential";
        case UNIFORM: return L"uniform";
        case POISSON: return L"poisson";
        case GAUSS: return L"normal/gaussian";
        default: return L"";
    }
}

double dist::get_distribution_kurtosis(const EDistribution distribution, const double mean) {
    switch (distribution) {
        case EXPONENTIAL: return 6.0;
        case UNIFORM: return -(6.0 / 5.0);
        case POISSON: return (1.0 / mean);
        case GAUSS: return 0.0;
        default: return -1.0; // TODO default error state??
    }
}

dist::EDistribution dist::evaluate_distribution(const double kurtosis, const double mean) {
    const std::vector<dist::EDistribution> distributions{ EXPONENTIAL, UNIFORM, POISSON, GAUSS };
    std::unordered_map<EDistribution, double> distribution_kurtoses;

    std::for_each(
        std::execution::par, 
        std::begin(distributions), 
        std::end(distributions), 
        [&distribution_kurtoses, mean](const EDistribution distribution) {
            distribution_kurtoses[distribution] = get_distribution_kurtosis(distribution, mean);
    });

    const EDistribution closest_distribution = 
        std::min_element(
            std::begin(distribution_kurtoses),
            std::end(distribution_kurtoses),
            [kurtosis](const std::pair<EDistribution, double>& first, const std::pair<EDistribution, double>& second) {
                const double diff_first = std::abs(first.second - kurtosis);
                const double diff_second = std::abs(second.second - kurtosis);
                return diff_first < diff_second;
            })->first;

    return closest_distribution;
}