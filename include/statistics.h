#pragma once
#include <vector>
#include <array>

#include <platform.h>

class RunningStats {
public:
    RunningStats();
    void Clear();
    void Push(double x);
    void Push_Vector(const std::vector<double>& data);
    double NumDataValues() const;
    double Mean() const;
    //double Variance() const;
    //double StandardDeviation() const;
    //double Skewness() const;
    double Kurtosis() const;

    friend RunningStats operator+(const RunningStats a, const RunningStats b);
    RunningStats& operator+=(const RunningStats& rhs);
//private:
    double n;
    double M1, M2, M3, M4;
};

class CStatistics {
public:
    CStatistics(double n, double M1, double M2, double M3, double M4);
    CStatistics() {};
    double Mean() const;
    double Kurtosis() const;

    friend CStatistics operator+(const CStatistics a, const CStatistics b);
private:
    double m_n = 0.0;
    double m_M1 = 0.0, m_M2 = 0.0, m_M3 = 0.0, m_M4 = 0.0;
};

class CRunning_Stats_SOA {
public:
    CRunning_Stats_SOA();
    void Clear();
    void Push_Vector(const std::vector<double>& data);

    CStatistics Merge_Statistics_Async() const;

    friend CRunning_Stats_SOA operator+(const CRunning_Stats_SOA a, const CRunning_Stats_SOA b);
    CRunning_Stats_SOA& operator+=(const CRunning_Stats_SOA& rhs);
private:
    static const std::size_t s_array_size = 64;
    std::array<double, s_array_size> m_n;   // double for vectorization purposes
    std::array<double, s_array_size> m_M1;
    std::array<double, s_array_size> m_M2;
    std::array<double, s_array_size> m_M3;
    std::array<double, s_array_size> m_M4;
};
