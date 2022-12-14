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

#include "include/input_parser.h"
#include "include/data_loader.h"
#include "include/statistics.h"
#include "include/cpu/distribution.h"
#include "include/benchmark.h"
#include "include/calculator_factory.h"
#include "include/cpu/cpu_statistics.h"
#include "include/env.h"
#include "include/watchdog.h"

void print_all_ocl_devices() {
	try {
		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);
		cl::Platform plat;

		for (auto& platform : platforms) {
			std::vector<cl::Device> devices;
			platform.getDevices(CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_CPU, &devices);

			for (auto& device : devices) {
				auto desc = device.getInfo<CL_DEVICE_NAME>();
				std::cout << "Testing device " << desc << " on platform " << platform.getInfo<CL_PLATFORM_NAME>() << "...\n";
			}
		}
	} catch (const cl::Error& err) {
		std::cerr << "Error: " << err.what() << "(" << err.err() << ")\n";
	}
}

void print_usage(std::wostream& ostream) {
	if (!ostream) {
		return;
	}

	ostream << L"Usage: pprsolver.exe input_file_path (<PLATFORM_TYPE> | *<OPEN_CL_DEVICE_NAME>)\n"
		<< L"PLATFORM_TYPE:\n"
		<< L"\tSMP - performs computation on SMP only\n"
		<< L"\tall - performs computation on SMP and all available OpenCL devices\n"
		<< L"*<OPEN_CL_DEVICE_NAME> - list of OCL device names to compute on" << std::endl;
}

void print_errors(std::wostream& ostream, const std::vector<std::wstring>& errors) {
	if (!ostream) {
		return;
	}

	for (const auto& error : errors) {
		ostream << '\n' << error;
	}

	ostream << std::endl;
}

CStatistics collect_result(std::vector<std::future<CStatistics>>& futures) {
	std::vector<CStatistics> results;

	for (auto& future : futures) {
		results.push_back(future.get());
	}

	return std::reduce(std::execution::par, results.begin(), results.end());
}

// just a utility function for sequential run testing - to be deleted
CStatistics collect_result(std::vector<CStatistics>& results) {
	return std::reduce(std::execution::par, results.begin(), results.end());
}

int wmain(int argc, wchar_t** argv) {
	const auto parse_result = parse_input_params(argc, argv);
	if (parse_result.Has_Error()) {
		std::wcout << parse_result.Error_Message() << std::endl;
		print_usage(std::wcout);
		return EXIT_FAILURE;
	}

	const auto& input_params = parse_result.Input_Params();
	CStatistics result;
	
		Watchdog watchdog(std::chrono::milliseconds(1000));
		watchdog.Start();

		try {	
			const std::size_t chunk_size = env::s_stream_size * env::s_stream_size;

			CData_Loader data_loader(input_params.file_path, chunk_size);
			if (data_loader.Has_Error()) {
				print_errors(std::wcout, data_loader.Get_Errors());
				return EXIT_FAILURE;
			}

			std::vector<std::future<CStatistics>> futures;
			const auto calculator_ptr = create_statistics_calculator(input_params.platform);
			while (data_loader.Has_Next_Chunk()) {
				std::vector<double> buffer(chunk_size / sizeof(double));
				const auto loaded_doubles = data_loader.Load_Chunk(buffer);
				if (data_loader.Has_Error()) {
					print_errors(std::wcout, data_loader.Get_Errors());
					return EXIT_FAILURE;
				}

				// if we have less than 256 values, we ignore the input
				if (loaded_doubles >= env::s_stream_size) {
					futures.push_back(std::async(std::launch::async, [&calculator_ptr, &watchdog, buffer]() { 
						const auto result = calculator_ptr->Analyze_Vector(buffer); 
						watchdog.Kick(buffer.size());
						return result;
					}));
				}

				// vector size barrier, just so it does not grow infinitely
				if (futures.size() > env::s_stream_size) {
					result = result + collect_result(futures);
					futures.clear();
				}
			}

			result = result + collect_result(futures);
			watchdog.Stop();

			std::wcout << L"Calculated kurtosis: " << result.Kurtosis() << L"\n";
			std::wcout << L"Calculated mean: " << result.Mean() << L"\n";
			std::wcout << L"Calculated variance: " << result.Variance() << L"\n";

			const dist::EDistribution calculated_distribution = dist::evaluate_distribution(result.Kurtosis(), result.Mean());
			std::wcout << L"Determined distribution: " << dist::get_distribution_name(calculated_distribution) << std::endl;

		} catch (const std::exception& error) {
			std::wcout << error.what() << std::endl;
			return EXIT_FAILURE;
		}
		
	return EXIT_SUCCESS;
}
