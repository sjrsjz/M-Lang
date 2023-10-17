#pragma once
#pragma once
#include "Common.h"
bool is_base64(unsigned char c);
std::string base64_encode(const std::string& input);
std::string base64_decode(const std::string& input);