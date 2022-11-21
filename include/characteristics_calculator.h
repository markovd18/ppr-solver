#pragma once
#include <vector>

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

class RunningStatsSOA {
public:
    RunningStatsSOA();
    void Clear();
    void Push(double x);
    void Push_Vector(const std::vector<double>& data);
    double NumDataValues() const;
    double Mean() const;
    //double Variance() const;
    //double StandardDeviation() const;
    //double Skewness() const;
    double Kurtosis() const;

    friend RunningStatsSOA operator+(const RunningStatsSOA a, const RunningStatsSOA b);
    RunningStatsSOA& operator+=(const RunningStatsSOA& rhs);
//private:
    static const std::size_t m_array_size = 4;
    std::array<double, m_array_size> m_n;
    std::array<double, m_array_size> m_M1;
    std::array<double, m_array_size> m_M2;
    std::array<double, m_array_size> m_M3;
    std::array<double, m_array_size> m_M4;
};