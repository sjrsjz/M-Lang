#pragma once
#include "SectionManager.h"
#include "Struct.h"
#include "ByteArray.h"
namespace MLang {
	class x86Generator {
	private:
		void addBuiltInFunction(const lstring& name, std::vector<redirection>& redirections);
		void ins(ByteArray<unsigned char> bytes);
		void error(lstring err);
		void warning(lstring warn);
		size_t tmpSize(size_t id, const std::vector<size_t>& stack);
		size_t tmpOffset(size_t id, const std::vector<size_t>& stack);
	public:
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
		void generate(lstring IR);
	};
	namespace x86Runner {
		extern std::vector<lstring> strings{};
		extern unsigned char* GlobalAddress;
		extern int GlobalSize;
		extern std::vector<void*> DLL_h{};
		extern std::vector<lstring> sys_redirectTable{};
		extern std::vector<void*> sys_redirect{};
		void NewSysFunction();
		void NewSysFunction(const lstring& name, void* func);
		void LoadMEXE(const ByteArray<unsigned char>& mexe);
		void Load(const ByteArray<unsigned char>& mexe, const std::vector<lstring>& constStr, const std::vector<redirection>& redirections, size_t GlobalSize_, const std::vector<lstring> apiTable);
		void release();
	}
}