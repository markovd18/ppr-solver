#include <array>
#include <vector>
#include <execution>
#include <numeric>

#include <cpu/cpu_statistics.h>
#include <statistics.h>
#include <benchmark.h>

namespace cpu {

	CStatistics CSeq_Stats_Calculator::Analyze_Vector(const std::vector<double>& data) {
		for (const auto& x : data) {
			double delta, delta_n, delta_n2, term1;

			double n1 = m_n;
			m_n++;

			delta = x - m_M1;
			delta_n = delta / m_n;
			delta_n2 = delta_n * delta_n;
			term1 = delta * delta_n * n1;

			m_M1 += delta_n;
			m_M4 += term1 * delta_n2 * (m_n * m_n - 3 * m_n + 3) + 6 * delta_n2 * m_M2 - 4 * delta_n * m_M3;
			m_M3 += term1 * delta_n * (m_n - 2) - 3 * delta_n * m_M2;
			m_M2 += term1;
		}

		return CStatistics(m_n, m_M1, m_M2, m_M3, m_M4);
	}

	CStatistics CPar_Stats_Calculator::Analyze_Vector(const std::vector<double>& numbers) {
		CStatistics result;

		static const std::size_t count = 64;
		std::array<double, count> n{ 0 };
		std::array<double, count> M1{ 0 };
		std::array<double, count> M2{ 0 };
		std::array<double, count> M3{ 0 };
		std::array<double, count> M4{ 0 };

		stats::benchmark_function(L"Vectorized algorithm", [&numbers, &n, &M1, &M2, &M3, &M4]() {
			for (std::size_t j = 0; (j + count) < numbers.size(); j += count) {

				for (std::size_t i = 0; i < count; ++i) {
					const auto n1 = n[i];
					const auto n2 = n1 + 1;

					const double delta = numbers[j + i] - M1[i];
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
								  });

		std::vector<CStatistics> results;
		results.resize(count);
		for (std::size_t i = 0; i < count; i++) {
			// TODO 1305 - je to struktura a to se mu nelibi, asi nepujde zvektorizovat
			results[i] = CStatistics(n[i], M1[i], M2[i], M3[i], M4[i]);
		}

		//std::vector<std::future<SIntermediate_Result>> futures;
		//futures.reserve(count / 2);

		//stats::benchmark_function(L"Vectorized merge", [&results, &futures]() {
		//	while (results.size() >= 2) {
		//		while (results.size() >= 2 && !results.empty()) {
		//			const auto& a = *(results.end() - 1);
		//			const auto& b = *(results.end() - 2);

		//			futures.emplace_back(std::async(std::launch::async, merge_intermadiate_results, a, b));
		//			results.pop_back();
		//			results.pop_back();
		//		}

		//		while (!futures.empty()) {
		//			auto& future = futures.back();
		//			results.emplace_back(std::move(future.get()));

		//			futures.pop_back();
		//		}
		//	}
		//});

		//result = results.front();

		//results.clear();
		//results.resize(count);
		//for (std::size_t i = 0; i < count; i++) {
		//	// TODO 1305 - je to struktura a to se mu nelibi, asi nepujde zvektorizovat
		//	results[i] = { M1[i], M2[i], M3[i], M4[i], n[i] };
		//}

		stats::benchmark_function(L"Paralel reduce merge", [&results, &result]() {
			result = std::reduce(std::execution::par, results.cbegin(), results.cend());
								  });

		return result;
	}


}
