#include <iostream>
#include <chrono>

#include "../include/data_loader.h"
#include "../include/characteristics_calculator.h"

int wmain(int argc, wchar_t **argv) {

	const auto numbers = load_data("../referencni_rozdeleni/gauss");
	RunningStats stats;

	auto start = std::chrono::high_resolution_clock::now();
	for (const auto& number : numbers) {
		stats.Push(number);
	}
	auto end = std::chrono::high_resolution_clock::now();
	auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	std::wcout << L"Sequential duration: " << diff.count() << L"us\n";
	std::wcout << L"Kurtosis: " << stats.Kurtosis() << std::endl;
	std::wcout << L"Mean: " << stats.Mean() << std::endl;

	stats.Clear();

	start = std::chrono::high_resolution_clock::now();

	stats.Push_Vector(numbers);

	end = std::chrono::high_resolution_clock::now();
	diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	std::wcout << L"Batch duration: " << diff.count() << L"us\n";

	RunningStats stats_1;
	RunningStats stats_2;
	RunningStats stats_3;
	RunningStats stats_4;

	start = std::chrono::high_resolution_clock::now();
	for (std::size_t i = 0; i < numbers.size(); i += 4) {
		// TODO tady podle mì musí být velký forloop vnì
		// a vnoøený forloop += 4, který vektorizuje jen výpoèet delty 1
		// z 
		stats_1.Push(numbers[i]);
		stats_2.Push(numbers[i + 1]);
		stats_3.Push(numbers[i + 2]);
		stats_4.Push(numbers[i + 3]);
	}

	const auto result = stats_1 + stats_2 + stats_3 + stats_4;
	end = std::chrono::high_resolution_clock::now();
	diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	std::wcout << L"Parallel duration: " << diff.count() << L"us\n";


	return 0;
}