#pragma once
#include "SectionManager.h"
#include "Struct.h"
#include "ByteArray.h"
namespace MLang {
	class x86Generator {
	private:
		std::vector<lstring> constStr{};
		std::vector<lstring> constStr2{};
		std::vector<redirection> linkTable{};
		std::vector<lstring> apiTable{};
		std::vector<lstring> builtInFunction{};
		lstring ErrorType{};
		size_t ErrorLine{};
		size_t globalSize{};
		bool Error{};
		ByteArray<unsigned char> codes{};
		void addBuiltInFunction(const lstring& name, std::vector<redirection>& redirections);
		void ins(ByteArray<unsigned char> bytes);
		void error(lstring err);
		void warning(lstring warn);
		size_t tmpSize(size_t id, const std::vector<size_t>& stack);
		size_t tmpOffset(size_t id, const std::vector<size_t>& stack);
	public:
		SectionManager generate(lstring IR);
	};
}