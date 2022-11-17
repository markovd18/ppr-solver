#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "../../include/data_loader.h"

std::vector<double> load_data(const std::filesystem::path& path) {
	std::wcout << path;

	if (!std::filesystem::exists(path) || std::filesystem::is_directory(path)) {
		std::wcout << L"Soubor nenalezen nebo je to slozka" << std::endl;
		return std::vector<double>();
	}

	std::ifstream input_stream(path, std::ios::in | std::ios::binary);
	if (!input_stream) {
		std::wcout << L"Input stream se spatne otevrel" << std::endl;
		return std::vector<double>();
	}

	const auto file_size = std::filesystem::file_size(path);
	std::wcout << L"File size: " << file_size << std::endl;

	std::vector<double> numbers;
	const auto double_count = static_cast<std::size_t>(std::ceil(file_size / sizeof(double)));
	numbers.resize(double_count);
	input_stream.read(reinterpret_cast<char*>(numbers.data()), file_size);

	if (!input_stream) {
		std::wcout << L"Error pri cteni ze souboru" << std::endl;
		return std::vector<double>();
	}

	std::wcout << L"Read " << numbers.size() << L" doubles" << std::endl;
	for (std::size_t i = 0; i < file_size / sizeof(double); ++i) {
		std::wcout << numbers[i] << '\n';
	}
	std::wcout << std::endl;

	return numbers;
}