#pragma once

class CStatistics {
public:
    CStatistics(double n, double M1, double M2, double M3, double M4);
    CStatistics() {};
    double Mean() const;
    double Kurtosis() const;
    double Variance() const;
    double M3() const;
    double M4() const;

    friend CStatistics operator+(const CStatistics a, const CStatistics b);
private:
    double m_n = 0.0;
    double m_M1 = 0.0, m_M2 = 0.0, m_M3 = 0.0, m_M4 = 0.0;
};
