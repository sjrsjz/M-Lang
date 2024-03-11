#pragma once
#include "Common.h"
bool is_base64(lchar c);
lstring base64_encode(const lstring& input);
lstring base64_decode(const lstring& input);
lstring base64_encode_(const lstring& input);
lstring base64_decode_(const lstring& input);
