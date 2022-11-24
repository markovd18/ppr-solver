#pragma once

#include <string>
#include <cstdint>
#include <filesystem>
#include <vector>

#include <platform.h>

struct SInput_Params {
	const std::filesystem::path file_path;
	const SPlatform platform;
};

struct CParse_Result {
public:
	bool Has_Error() const;
	const std::wstring& Error_Message() const;
	const SInput_Params& Input_Params() const;

	explicit CParse_Result(std::wstring error_message) : m_error_message(std::move(error_message)), m_result({}) {}
	CParse_Result(std::wstring error_message, SInput_Params result) 
		: m_error_message(std::move(error_message)), m_result(std::move(result)) {}
private:
	// If empty, parsing input was valid and result is populated. 
	// Otherwise, describes parsing error.
	const std::wstring m_error_message;
	const SInput_Params m_result;
};

CParse_Result parse_input_params(int argc, wchar_t** argv);