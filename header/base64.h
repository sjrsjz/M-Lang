#pragma once
#include "Common.h"
bool is_base64(lchar c);
lstring base64_encode(const lstring& input);
lstring base64_decode(const lstring& input);