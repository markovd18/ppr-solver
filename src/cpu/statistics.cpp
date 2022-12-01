
#include "../include/statistics.h"

CStatistics::CStatistics(double n, double M1, double M2, double M3, double M4) 
    : m_n(n), m_M1(M1), m_M2(M2), m_M3(M3), m_M4(M4) {}

double CStatistics::Mean() const {
    return m_M1;
}

double CStatistics::Kurtosis() const {
    return m_n * m_M4 / (m_M2 * m_M2) - 3.0;
}

CStatistics operator+(const CStatistics a, const CStatistics b) {
    CStatistics combined;

    combined.m_n = a.m_n + b.m_n;

    double delta = b.m_M1 - a.m_M1;
    double delta2 = delta * delta;
    double delta3 = delta * delta2;
    double delta4 = delta2 * delta2;

    combined.m_M1 = (a.m_n * a.m_M1 + b.m_n * b.m_M1) / combined.m_n;

    combined.m_M2 = a.m_M2 + b.m_M2 +
        delta2 * a.m_n * b.m_n / combined.m_n;

    combined.m_M3 = a.m_M3 + b.m_M3 +
        delta3 * a.m_n * b.m_n * (a.m_n - b.m_n) / (combined.m_n * combined.m_n);
    combined.m_M3 += 3.0 * delta * (a.m_n * b.m_M2 - b.m_n * a.m_M2) / combined.m_n;

    combined.m_M4 = a.m_M4 + b.m_M4 + delta4 * a.m_n * b.m_n * (a.m_n * a.m_n - a.m_n * b.m_n + b.m_n * b.m_n) /
        (combined.m_n * combined.m_n * combined.m_n);
    combined.m_M4 += 6.0 * delta2 * (a.m_n * a.m_n * b.m_M2 + b.m_n * b.m_n * a.m_M2) / (combined.m_n * combined.m_n) +
        4.0 * delta * (a.m_n * b.m_M3 - b.m_n * a.m_M3) / combined.m_n;

    return combined;
}