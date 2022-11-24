#include <string>
#include <cstdint>

#include <platform.h>
#include <input_parser.h>

static const std::size_t REQUIRED_ARGS_COUNT = 3;
static const std::size_t INPUT_FILE_ARG_INDEX = 1;
static const std::size_t PLATFORM_TYPE_ARG_INDEX = 2;

static const wchar_t* PLATFORM_SMP = L"SMP";
static const wchar_t* PLATFORM_ALL = L"all";


std::wstring validate_input_params(const int argc, wchar_t** argv) {
	if (argc < REQUIRED_ARGS_COUNT) {
		return L"Not enough arguments passed.";
	}

	// TODO validace typu platformy?

	const std::filesystem::path input_file(argv[INPUT_FILE_ARG_INDEX]);
	if (!std::filesystem::exists(argv[INPUT_FILE_ARG_INDEX])) {
		return L"Input file does not exist.";
	}

	if (std::filesystem::is_directory(input_file)) {
		return L"Illegal input file. Input file has to be a binary.";
	}

	return L"";
}

std::vector<std::wstring> parse_platform_args(const int argc, wchar_t** argv) {
	// platform args (namely the OCL device names) start at the same index as platform type
	std::vector<std::wstring> args;
	for (std::size_t i = PLATFORM_TYPE_ARG_INDEX; i < argc; ++i) {
		args.push_back(argv[i]);
	}
	return args;
}

SPlatform parse_platform_info(const int argc, wchar_t** argv) {
	const std::wstring input = argv[PLATFORM_TYPE_ARG_INDEX];
	if (input == PLATFORM_SMP) {
		return { EPlatform_Type::SMP };
	}

	if (input == PLATFORM_ALL) {
		return { EPlatform_Type::ALL };
	}

	return { EPlatform_Type::OPEN_CL, parse_platform_args(argc, argv) };
}

std::filesystem::path parse_input_file_path(wchar_t** argv) {
	return std::filesystem::path(argv[INPUT_FILE_ARG_INDEX]);
}

CParse_Result parse_input_params(const int argc, wchar_t** argv) {
	const auto error_message = validate_input_params(argc, argv);
	if (!error_message.empty()) {
		return CParse_Result(error_message);
	}

	return CParse_Result(error_message, { parse_input_file_path(argv), parse_platform_info(argc, argv) });
}

bool CParse_Result::Has_Error() const {
	return !m_error_message.empty();
}

const std::wstring& CParse_Result::Error_Message() const {
	return m_error_message;
}

const SInput_Params& CParse_Result::Input_Params() const {
	return m_result;
}
