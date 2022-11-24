#pragma once

#include <string>

namespace dist {

	enum EDistribution : std::uint8_t {
		UNIFORM = 0,
		EXPONENTIAL,
		POISSON,
		GAUSS
	};

	std::wstring get_distribution_name(const EDistribution distribution);

	double get_distribution_kurtosis(const EDistribution distribution, const double mean);	// TODO tady idealne intermedeate_result

	EDistribution evaluate_distribution(const double kurtosis, const double mean); // TODO tady idealne intermedeate_result
}
