#include <cmath>
#include <vector>
#include <future>
#include <array>

#include <statistics.h>

RunningStats::RunningStats() {
    Clear();
}

void RunningStats::Clear() {
    n = 0;
    M1 = M2 = M3 = M4 = 0.0;
}

void RunningStats::Push(const double x) {
    double delta, delta_n, delta_n2, term1;

    double n1 = n;
    n++;
    // vektorizovat urèitì tohle
    // teoreticky by šel každej krok rozdìlit na batche a dìlat ve forloopu po 4 a pøekladaè to snad pochopí
    delta = x - M1;
    delta_n = delta / n;
    delta_n2 = delta_n * delta_n;
    term1 = delta * delta_n * n1;

    M1 += delta_n;
    M4 += term1 * delta_n2 * (n * n - 3 * n + 3) + 6 * delta_n2 * M2 - 4 * delta_n * M3;
    M3 += term1 * delta_n * (n - 2) - 3 * delta_n * M2;
    M2 += term1;
}

void RunningStats::Push_Vector(const std::vector<double>& data) {
    for (std::size_t i = 0; i < data.size(); i += 4) {
        Push(data[i]);
        Push(data[i + 1]);
        Push(data[i + 2]);
        Push(data[i + 3]);
    }
}

double RunningStats::NumDataValues() const {
    return n;
}

double RunningStats::Mean() const {
    return M1;
}

//double RunningStats::Variance() const {
//    return M2 / (n - 1.0);
//}

//double RunningStats::StandardDeviation() const {
//    return sqrt(Variance());
//}

//double RunningStats::Skewness() const {
//    return sqrt(double(n)) * M3 / pow(M2, 1.5);
//}

double RunningStats::Kurtosis() const
{
    return double(n) * M4 / (M2 * M2) - 3.0;
}

RunningStats operator+(const RunningStats a, const RunningStats b) {
    RunningStats combined;

    combined.n = a.n + b.n;

    double delta = b.M1 - a.M1;
    double delta2 = delta * delta;
    double delta3 = delta * delta2;
    double delta4 = delta2 * delta2;

    combined.M1 = (a.n * a.M1 + b.n * b.M1) / combined.n;

    combined.M2 = a.M2 + b.M2 +
        delta2 * a.n * b.n / combined.n;

    combined.M3 = a.M3 + b.M3 +
        delta3 * a.n * b.n * (a.n - b.n) / (combined.n * combined.n);
    combined.M3 += 3.0 * delta * (a.n * b.M2 - b.n * a.M2) / combined.n;

    combined.M4 = a.M4 + b.M4 + delta4 * a.n * b.n * (a.n * a.n - a.n * b.n + b.n * b.n) /
        (combined.n * combined.n * combined.n);
    combined.M4 += 6.0 * delta2 * (a.n * a.n * b.M2 + b.n * b.n * a.M2) / (combined.n * combined.n) +
        4.0 * delta * (a.n * b.M3 - b.n * a.M3) / combined.n;

    return combined;
}

RunningStats& RunningStats::operator+=(const RunningStats& rhs) {
    RunningStats combined = *this + rhs;
    *this = combined;
    return *this;
}

CRunning_Stats_SOA::CRunning_Stats_SOA() {
    Clear();
}

void CRunning_Stats_SOA::Clear() {
    m_n.fill(0);
    m_M1.fill(0.0);
    m_M2.fill(0.0);
    m_M3.fill(0.0);
    m_M4.fill(0.0);
}

void CRunning_Stats_SOA::Push_Vector(const std::vector<double>& data) {
    // this takes up more memory but is faster than accessing attributes directly
    auto n = m_n;
    auto M1 = m_M1;
    auto M2 = m_M2;
    auto M3 = m_M3;
    auto M4 = m_M4;
    const auto count = s_array_size;

    for (std::size_t j = 0; (j + count) < data.size(); j += count) {
        for (std::size_t i = 0; i < count; ++i) {
            const auto n1 = m_n[i];
            const auto n2 = n1 + 1;

            const double delta = data[j + i] - M1[i];
            const double delta_n = delta / n2;
            const double delta_n2 = delta_n * delta_n;
            const double term1 = delta * delta_n * n1;

            M1[i] += delta_n;
            M4[i] += term1 * delta_n2 * (n2 * n2 - 3 * n2 + 3) + 6 * delta_n2 * M2[i] - 4 * delta_n * M3[i];
            M3[i] += term1 * delta_n * (n2 - 2) - 3 * delta_n * M2[i];
            M2[i] += term1;

            n[i] = n2;
        }
    }

    m_n = n;
    m_M1 = M1;
    m_M2 = M2;
    m_M3 = M3;
    m_M4 = M4;
}

CStatistics CRunning_Stats_SOA::Merge_Statistics_Async() const {
    std::vector<CStatistics> results;
    results.resize(s_array_size);
    for (std::size_t i = 0; i < s_array_size; i++) {
        // TODO 1305 - je to struktura a to se mu nelibi, asi nepujde zvektorizovat
        results[i] = CStatistics(m_n[i], m_M1[i], m_M2[i], m_M3[i], m_M4[i]);
    }

    std::vector<std::future<CStatistics>> futures;
    futures.reserve(s_array_size / 2);
    auto start_merge = std::chrono::high_resolution_clock::now();

    while (results.size() >= 2) {
        while (results.size() >= 2 && !results.empty()) {
            const auto& a = *(results.end() - 1);
            const auto& b = *(results.end() - 2);

            futures.emplace_back(std::async(std::launch::async, [&a, &b]() { return a + b;  }));
            results.pop_back();
            results.pop_back();
        }

        while (!futures.empty()) {
            auto& future = futures.back();
            results.emplace_back(std::move(future.get()));

            futures.pop_back();
        }
    }

    return results.back();
}

CRunning_Stats_SOA operator+(const CRunning_Stats_SOA a, const CRunning_Stats_SOA b) {
    CRunning_Stats_SOA combined;

    /*combined.m_n[0] = a.m_n[0] + b.n;

    double delta = b.M1 - a.M1;
    double delta2 = delta * delta;
    double delta3 = delta * delta2;
    double delta4 = delta2 * delta2;

    combined.M1 = (a.n * a.M1 + b.n * b.M1) / combined.n;

    combined.M2 = a.M2 + b.M2 +
        delta2 * a.n * b.n / combined.n;

    combined.M3 = a.M3 + b.M3 +
        delta3 * a.n * b.n * (a.n - b.n) / (combined.n * combined.n);
    combined.M3 += 3.0 * delta * (a.n * b.M2 - b.n * a.M2) / combined.n;

    combined.M4 = a.M4 + b.M4 + delta4 * a.n * b.n * (a.n * a.n - a.n * b.n + b.n * b.n) /
        (combined.n * combined.n * combined.n);
    combined.M4 += 6.0 * delta2 * (a.n * a.n * b.M2 + b.n * b.n * a.M2) / (combined.n * combined.n) +
        4.0 * delta * (a.n * b.M3 - b.n * a.M3) / combined.n;*/

    return combined;
}

CRunning_Stats_SOA& CRunning_Stats_SOA::operator+=(const CRunning_Stats_SOA& rhs) {
    CRunning_Stats_SOA combined = *this + rhs;
    *this = combined;
    return *this;
}

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