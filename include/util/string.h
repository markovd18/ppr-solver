#pragma once

#include <string>
#include <codecvt>

/**
 * 
 * !IMPORTANT! works only with single byte chars. Created mainly for purposes of OCL device name lookups.
 * @brief Converts ascii (single-byte) char string to wstring equivalent.
 * @param source source string
 * @return wide-char string quivalent of the source
*/
std::wstring ascii_string_to_wstring(const std::string& source) {
    return std::wstring(source.begin(), source.end());
}