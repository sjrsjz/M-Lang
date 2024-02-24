#pragma once
#include "Common.h"
#include "ByteArray.h"

namespace MLang {
	ByteArray<unsigned char> BuildCallFunction(const void* function, const std::vector<lstring>& argType);
}

