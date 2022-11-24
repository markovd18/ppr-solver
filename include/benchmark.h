#pragma once

#include <functional>
#include <chrono>
#include <string>
#include <iostream>

namespace stats {
	void benchmark_function(const std::wstring& name, const std::function<void(void)>& function);
}
