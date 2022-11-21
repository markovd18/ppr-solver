#include <iostream>
#include <chrono>
#include <array>
#include <algorithm>
#include <future>

#include "../include/data_loader.h"
#include "../include/characteristics_calculator.h"

struct SIntermediate_Result {
	double M1 = 0;
	double M2 = 0;
	double M3 = 0;
	double M4 = 0;
	double n = 0;
};

SIntermediate_Result merge_intermadiate_results(const SIntermediate_Result a, const SIntermediate_Result b) {
	SIntermediate_Result result;

	result.n = a.n + b.n;

	const double delta = b.M1 - a.M1;
	const double delta2 = delta * delta;
	const double delta3 = delta * delta2;
	const double delta4 = delta2 * delta2;

	result.M1 = (a.n * a.M1 + b.n * b.M1) / result.n;

	result.M2 = a.M2 + b.M2 +
		delta2 * a.n * b.n / result.n;

	result.M3 = a.M3 + b.M3 +
		delta3 * a.n * b.n * (a.n - b.n) / (result.n * result.n);
	result.M3 += 3.0 * delta * (a.n * b.M2 - b.n * a.M2) / result.n;

	result.M4 = a.M4 + b.M4 + delta4 * a.n * b.n * (a.n * a.n - a.n * b.n + b.n * b.n) /
		(result.n * result.n * result.n);
	result.M4 += 6.0 * delta2 * (a.n * a.n * b.M2 + b.n * b.n * a.M2) / (result.n * result.n) +
		4.0 * delta * (a.n * b.M3 - b.n * a.M3) / result.n;

	return result;
}


int wmain(int argc, wchar_t** argv) {

	const auto numbers = load_data("../referencni_rozdeleni/gauss_large");
	RunningStats stats;

	//auto start = std::chrono::high_resolution_clock::now();
	//for (const auto& number : numbers) {
	//	stats.Push(number);
	//}
	////auto end = std::chrono::high_resolution_clock::now();
	////auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	////std::wcout << L"Sequential duration: " << diff.count() << L"us\n";
	//std::wcout << L"Kurtosis: " << stats.Kurtosis() << std::endl;
	//std::wcout << L"Mean: " << stats.Mean() << std::endl;

	//stats.Clear();

	//start = std::chrono::high_resolution_clock::now();

	////stats.Push_Vector(numbers);

	//end = std::chrono::high_resolution_clock::now();
	//diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	//std::wcout << L"Batch duration: " << diff.count() << L"us\n";

	//RunningStats stats_1;
	//RunningStats stats_2;
	//RunningStats stats_3;
	//RunningStats stats_4;

	//start = std::chrono::high_resolution_clock::now();
	////for (std::size_t i = 0; i < numbers.size(); i += 4) {
	////	// TODO tady podle mì musí být velký forloop vnì
	////	// a vnoøený forloop += 4, který vektorizuje jen výpoèet delty 1
	////	// z 
	////	stats_1.Push(numbers[i]);
	////	stats_2.Push(numbers[i + 1]);
	////	stats_3.Push(numbers[i + 2]);
	////	stats_4.Push(numbers[i + 3]);
	////}

	//const auto result = stats_1 + stats_2 + stats_3 + stats_4;
	//end = std::chrono::high_resolution_clock::now();
	//diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	//std::wcout << L"Parallel duration: " << diff.count() << L"us\n";

	//for (const auto stats : { &stats_1, &stats_2, &stats_3, &stats_4 }) {
	//	stats->Clear();
	//}

	//

	////for (const double number : numbers) {
	////	double delta, delta_n, delta_n2, term1 = 0;
	////	const double n1 = stats_1.n;
	////	const double n2 = stats_1.n + 1;
	////	// vektorizovat urèitì tohle
	////	// teoreticky by šel každej krok rozdìlit na batche a dìlat ve forloopu po 4 a pøekladaè to snad pochopí
	////	delta = number - stats_1.M1;
	////	delta_n = delta / n2;
	////	delta_n2 = delta_n * delta_n;
	////	term1 = delta * delta_n * n1;
	////	stats_1.M1 += delta_n;
	////	stats_1.M4 += term1 * delta_n2 * (n2 * n2 - 3.0 * n2 + 3.0) + 6.0 * delta_n2 * stats_1.M2 - 4.0 * delta_n * stats_1.M3;
	////	stats_1.M3 += term1 * delta_n * (n2 - 2.0) - 3.0 * delta_n * stats_1.M2;
	////	stats_1.M2 += term1;

	////	stats_1.n = n2;
	////}

	//for (std::size_t i = 0; i < numbers.size(); i += 4) {
	//	std::array<double, 4> deltas;
	//	const auto modular = i % 4;
	//	deltas[modular] = numbers[i] - stats_1.M1;
	//	deltas[modular + 1] = numbers[i + 1] - stats_1.M1;
	//	deltas[modular + 2] = numbers[i + 2] - stats_1.M1;
	//	deltas[modular + 3] = numbers[i + 3] - stats_1.M1;
	//}

	///*std::vector<double> deltas;
	//deltas.resize(numbers.size());

	//const auto M1 = stats_1.M1;
	//for (std::size_t i = 0; i < numbers.size(); ++i) {
	//	deltas[i] = numbers[i] - M1;
	//}*/

	//std::vector<double> B, A;
	//B.resize(numbers.size());
	//A.resize(numbers.size());

	//for (int i = 0; i < 1000; ++i)
	//{
	//	const double x = B[i];
	//	A[i] = A[i] + x;
	//}

	RunningStatsSOA soa_stats;
	static const std::size_t count = 64;
	std::array<double, count> n{ 0 }/* = soa_stats.m_n*/;
	//n.resize(64);
	std::array<double, count> M1{ 0 }/* = soa_stats.m_M1*/;
	std::array<double, count> M2{ 0 }/* = soa_stats.m_M2*/;
	std::array<double, count> M3{ 0 } /*= soa_stats.m_M3*/;
	std::array<double, count> M4{ 0 } /*= soa_stats.m_M4*/;

	auto start = std::chrono::high_resolution_clock::now();

	for (std::size_t j = 0; j < numbers.size(); j += count) {

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

	std::vector<std::future<SIntermediate_Result>> futures;
	futures.reserve(count / 2);
	std::vector<SIntermediate_Result> results;
	results.reserve(count / 2);

	{
		// TODO jses kunda musis vytvorit vsechny a ne jen dva
		const SIntermediate_Result a{ M1[0], M2[0], M3[0], M4[0], n[0] };
		const SIntermediate_Result b{ M1[1], M2[1], M3[1], M4[1], n[1] };

		results.push_back(a);
		results.push_back(b);
	}

	while (results.size() >= 2) {	// TODO vzdycky po dvou, muze zbyt jeden, tak ten prohlasit za merged / finalni vysledek
		while (results.size() >= 2 && !results.empty()) {
			const auto a = results.back();
			results.pop_back();
			const auto b = results.back();
			results.pop_back();

			futures.push_back(std::async(std::launch::async, merge_intermadiate_results, a, b));
		}

		auto& future = futures.back();
		while (!futures.empty()) {
			const auto result = future.get();
			results.push_back(result);

			futures.pop_back();
		}
	}


	auto end = std::chrono::high_resolution_clock::now();
	auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	std::wcout << L"Vectorized 64 duration: " << diff.count() << L"us\n";

	stats.Clear();

	start = std::chrono::high_resolution_clock::now();

	for (const auto& number : numbers) {
		stats.Push(number);
	}

	end = std::chrono::high_resolution_clock::now();
	diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	std::wcout << L"Sequential 64 duration: " << diff.count() << L"us\n";
	std::wcout << L"Sequential 64 kurtosis: " << stats.Kurtosis() << L"\n";
	std::wcout << L"Sequential 64 kurtosis: " << stats.NumDataValues() << L"\n";


	return 0;
}