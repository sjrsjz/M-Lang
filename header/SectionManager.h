#pragma once
#include "Common.h"
#include "ByteArray.h"
namespace MLang {
	class SectionManager {
	public:
		std::vector<lstring> names;
		std::vector<ByteArray<>> bins;
		SectionManager();
		~SectionManager();
		void Clear();
		void Ins(const lstring& name, const ByteArray<>& bin, std::optional<bool>different);
		void Delete(const lstring& name);
		ByteArray<>& Get(const lstring& name);
		ByteArray<> build();
		void translate( ByteArray<> bin);
	};
}