#pragma once
#include "SectionManager.h"
namespace MLang {
	class x64Generator {
	public:
		SectionManager generate(lstring IR);
	};
}