#pragma once

#include <vector>
#include <string>
#include <cstdint>

enum EPlatform_Type : std::uint8_t {
	SMP,
	OPEN_CL,
	ALL
};

struct SPlatform {
	const EPlatform_Type type;
	const std::vector<std::wstring> args;
};
