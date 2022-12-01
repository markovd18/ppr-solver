#include <string>
#include <chrono>

#include "include/benchmark.h"

void stats::benchmark_function(const std::wstring& name, const std::function<void(void)>& function) {
	const auto start = std::chrono::high_resolution_clock::now();
	function();
	const auto end = std::chrono::high_resolution_clock::now();
	const auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	std::wcout << name << L" - duration: " << diff.count() << L"us." << std::endl;

}
