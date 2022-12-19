#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "../include/data_loader.h"

const bool is_valid_number(const double x) {
	switch (std::fpclassify(x)) {
		case FP_NORMAL:
		case FP_ZERO:
			return true;
		default:
			return false;
	}
}

void erase_invalid_numbers(std::vector<double>& vector) {
	auto iter = vector.begin();
	vector.erase(
		std::remove_if(vector.begin(), vector.end(), [](const double val) { return !is_valid_number(val); }), 
		vector.end()
	);
}

void trim_buffer(std::vector<double>& buffer, const std::size_t max_size) {
	if (buffer.size() > max_size) {
		buffer.resize(max_size);
	}
}

std::vector<double> load_data(const std::filesystem::path& path) {
	if (!std::filesystem::exists(path) || std::filesystem::is_directory(path)) {
		std::wcout << L"File not found or is a directory" << std::endl;
		return std::vector<double>();
	}

	std::ifstream input_stream(path, std::ios::in | std::ios::binary);
	if (!input_stream) {
		std::wcout << L"Error while openning input stream" << std::endl;
		return std::vector<double>();
	}

	const auto file_size = std::filesystem::file_size(path);

	std::vector<double> numbers;
	const auto double_count = static_cast<std::size_t>(std::ceil(file_size / sizeof(double)));
	numbers.resize(double_count);
	input_stream.read(reinterpret_cast<char*>(numbers.data()), file_size);

	if (!input_stream) {
		std::wcout << L"Error pri cteni ze souboru" << std::endl;
		return std::vector<double>();
	}

	std::wcout << L"Read " << numbers.size() << L" doubles" << std::endl;
	
	return numbers;
}

std::string load_text_file(const std::filesystem::path& path) {
	if (!std::filesystem::exists(path) || std::filesystem::is_directory(path)) {
		std::wcout << L"Soubor nenalezen nebo je to slozka" << std::endl;
		return std::string();
	}

	std::ifstream input_stream(path);
	if (!input_stream) {
		std::wcout << L"Input stream se spatne otevrel" << std::endl;
		return std::string();
	}

	const auto file_size = std::filesystem::file_size(path);
	
	std::vector<char> result;
	result.resize(file_size);
	input_stream.read(result.data(), file_size);

	return std::string(result.begin(), result.end());
}

CData_Loader::CData_Loader(const std::filesystem::path& path, const std::size_t chunk_size) 
	: m_input_stream(path, std::ios::in | std::ios::binary), m_chunk_size(chunk_size) {
	const bool file_exists = std::filesystem::exists(path);
	const bool is_directory = std::filesystem::is_directory(path);
	if (!file_exists) {
		m_errors.push_back(L"File does not exist");
	}

	if (is_directory) {
		m_errors.push_back(L"File is a directory");
	}

	if (file_exists && !is_directory && !m_input_stream) {
		m_errors.push_back(L"Unknown error while openning input stream");
	}

	if (!Has_Error()) {
		m_file_size = std::filesystem::file_size(path);
	}

	if (m_chunk_size == s_do_not_chunk) {
		m_chunk_size = m_file_size;
	}
}

bool CData_Loader::Has_Error() const {
	return m_input_stream.bad() || !m_errors.empty();
}

bool CData_Loader::Has_Next_Chunk() const {
	return !m_input_stream.eof();
}

const std::vector<std::wstring>& CData_Loader::Get_Errors() const {
	return m_errors;
}

std::size_t CData_Loader::Load_Chunk(std::vector<double>& buffer) {
	if (Has_Error() || !Has_Next_Chunk() || !m_input_stream.is_open()) {
		return 0;
	}

	if (m_valid_backup.empty()) {
		Load_Valid_Backup();
	}

	if (!Has_Next_Chunk()) {
		buffer = m_valid_backup;
		return m_valid_backup.size();
	}

	m_input_stream.read(reinterpret_cast<char*>(buffer.data()), m_chunk_size);
	if (m_input_stream.bad()) {
		m_errors.push_back(L"Error while loading from input file");
	}


	const auto loaded_doubles = m_input_stream.gcount() / sizeof(double);
	trim_buffer(buffer, loaded_doubles);

	erase_invalid_numbers(buffer);
	const auto buffer_size = m_chunk_size / sizeof(double);
	while ((buffer.size() != buffer_size) && !m_valid_backup.empty()) {
		buffer.push_back(m_valid_backup.back());
		m_valid_backup.pop_back();
	}

	return buffer.size();
}

void CData_Loader::Load_Valid_Backup() {
	const auto buffer_size = m_chunk_size / sizeof(double);
	m_valid_backup.resize(buffer_size);
	m_input_stream.read(reinterpret_cast<char*>(m_valid_backup.data()), m_chunk_size);
	if (m_input_stream.bad()) {
		m_errors.push_back(L"Error while loading from input file");
		return;
	}

	erase_invalid_numbers(m_valid_backup);
	const auto loaded_doubles = m_input_stream.gcount() / sizeof(double);
	trim_buffer(m_valid_backup, loaded_doubles);
}
