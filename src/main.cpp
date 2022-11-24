#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION  200


#include <iostream>
#include <chrono>
#include <array>
#include <algorithm>
#include <future>
#include <string>
#include <CL/opencl.hpp>
#include <cstdlib>
#include <execution>
#include <numeric>

#include <input_parser.h>
#include <data_loader.h>
#include <statistics.h>
#include <cpu/distribution.h>
#include <benchmark.h>
#include <calculator_factory.h>
#include <cpu/cpu_statistics.h>

int wmain(int argc, wchar_t** argv) {
	const auto parse_result = parse_input_params(argc, argv);
	if (parse_result.Has_Error()) {
		std::wcout << parse_result.Error_Message() << std::endl;
		return EXIT_FAILURE;
		// TODO print usage
	}

	const auto& input_params = parse_result.Input_Params();
	const auto numbers = load_data(input_params.file_path);
	// TODO vymazat nevalidni data pres std::fp_classify a udelat boolean flag, jestli muze/nemuze byt poisson

	const auto calculator_ptr = create_statistics_calculator(input_params.platform);
	const auto statistics = calculator_ptr->Analyze_Vector(numbers);

	if (input_params.platform.type == EPlatform_Type::OPEN_CL) {
		try {
			std::vector<cl::Platform> platforms;
			cl::Platform::get(&platforms);
			cl::Platform plat;

			for (auto& platform : platforms) {
				std::vector<cl::Device> devices;
				platform.getDevices(CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_CPU, &devices);

				//std::find_if(devices.begin(), devices.end(), [](const cl::Device& device) { return device.getInfo<CL_DEVICE_NAME>() == "ahoj";  });
				for (auto& device : devices) {
					auto desc = device.getInfo<CL_DEVICE_NAME>();
					std::cout << "Testuji zarizeni " << desc << " na platforme " << platform.getInfo<CL_PLATFORM_NAME>() << "...\n";
				}
			}
		} catch (const cl::Error& err) {
			std::cerr << "Chyba: " << err.what() << "(" << err.err() << ")\n";
			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
	}

	cpu::CSeq_Stats_Calculator seq_calc;
	cpu::CPar_Stats_Calculator par_calc;

	CStatistics seq_stats;
	CStatistics par_stats;

	stats::benchmark_function(L"Sequential algorithm", 
							  [&numbers, &seq_calc, &seq_stats]() { seq_stats = seq_calc.Analyze_Vector(numbers);  });
	std::wcout << L"Sequential kurtosis: " << seq_stats.Kurtosis() << L"\n";
	const dist::EDistribution calculated_distribution_seq = dist::evaluate_distribution(seq_stats.Kurtosis(), seq_stats.Mean());
	std::wcout << L"Determined distribution: " << dist::get_distribution_name(calculated_distribution_seq) << std::endl;

	std::wcout << std::endl;

	stats::benchmark_function(L"Parallel algorithm", 
							  [&numbers, &par_calc, &par_stats]() { par_stats = par_calc.Analyze_Vector(numbers);  });
	std::wcout << L"Parallel kurtosis: " << par_stats.Kurtosis() << L"\n";
	const dist::EDistribution calculated_distribution_par = dist::evaluate_distribution(par_stats.Kurtosis(), par_stats.Mean());
	std::wcout << L"Determined distribution: " << dist::get_distribution_name(calculated_distribution_par) << L"\n";

	std::wcout << std::endl;
	
	return EXIT_SUCCESS;
}
