#pragma once
#include "Common.h"
#include "Struct.h"
#include "Tree.h"
#include "AST.h"
#include "base64.h"
#include "ByteArray.h"
namespace MLang {
	class IRGenerator {
	public:
		std::vector<lstring> libs{};
		std::vector<type> globalVars{};
		std::vector<functionSet> sets{};
		std::vector<structure> structures{};
		std::vector<analyzed_functionSet> analyzed_functionSets{};
		std::vector<functionSet> functionSets{};
		std::vector<type> constants{};
		
		ByteArray<> constantData{};
		functionSet ExtraFunctions{};
		bool Error{};
		intptr_t error_line{};
		lstring error_functionSet{};
		lstring error_function{};
		lstring error_type{};

		lstring IR{};
		size_t GlobalSize{};

		std::vector<lstring> strings{};
		bool analyze(
			std::vector<lstring> libs_,
			std::vector<type> globalVars_,
			std::vector<analyzed_functionSet> analyzed_functionSet_,
			std::vector<functionSet> functionSets_,
			std::vector<structure> structures_,
			functionSet ExtraFunctions_,
			std::vector<type> constants_
		);
	private:
		type Type_N{};
		type Type_Z{};
		type Type_R{};
		type Type_B{};
		type Type_Boolen{};

		size_t GlobalSize0{};
		intptr_t GlobalOffset{};
		size_t currLocalSize{};
		std::vector<size_t> tmpStack{};
		std::vector<lstring> destroyCode{};

		void error(lstring err);
		bool getFunctionType(lstring fullName,lstring& type,lstring& super,lstring& name);
		size_t countVarSize(const std::vector<type>&);
		void generateFunctionSet(analyzed_functionSet& functionSet);
		void generateFunction(analyzed_functionSet& functionSet, analyzed_function& func);
		void generateLine(analyzed_functionSet& functionSet, analyzed_function& func,Tree<node>& EX);
		void compileTree(analyzed_functionSet& functionSet, analyzed_function& func, Tree<node>& EX, std::optional<lstring> ExtraInfo);
		bool ifMethod(lstring FullName);
		size_t allocStr(lstring text);
		type getElement(analyzed_functionSet& functionSet, lstring struct_, lstring element);
		size_t getElementOffset(analyzed_functionSet& functionSet, lstring struct_, lstring element);
		void initGenerator(type local,size_t tmp,size_t offset,lstring tk,bool localMode);
		void destroyGenerator(type local, size_t tmp, size_t offset, lstring tk, bool localMode);
		void initSetVars(std::vector<type>& local,size_t tmp,size_t offset,lstring tk,bool localMode);
		void destroySetVars(std::vector<type>& local, size_t tmp, size_t offset, lstring tk, bool localMode);
		bool handleBuiltInFunctions(analyzed_functionSet& functionSet, analyzed_function& func, Tree<node>& EX, lstring name, type& ret);
		bool cmpDim(const std::vector<size_t>& tk1, const std::vector<size_t>& tk2);
		bool cmpTK(const lstring& tk1, const std::vector<lstring>& tk2);
		bool generateImplictConversion(type& A, type& B, analyzed_functionSet functionSet, analyzed_function func, Tree<node> EX);
		bool haveFunction(lstring name);
		bool ifNotRef(type A);
		size_t allocTmpID(type A);
		size_t tmpOffset(size_t id,const std::vector<size_t>& stack);
		bool setHasFunction(const analyzed_functionSet& functionSet, lstring name);
		type maxPrecision(const type& A,const type& B);
		int precisionLevel(const type& A);
		lstring precisionLevelToType(int level);
		type getLocalType(lstring name,const analyzed_function& func);
		type getConstType(lstring name);
		type getArgType(const analyzed_functionSet& functionSet, lstring name, const analyzed_function& func);
		structure getStructure(lstring name);
		type getSetVarType(lstring name, const analyzed_function& functionSet);
		type getGlobalVarType(lstring name);
		analyzed_function getFunction(const analyzed_functionSet& functionSet, lstring fullName, std::vector<type>& args, std::optional<bool> variable);
		bool cmpArgNum(const std::vector<type>& A, const std::vector<type>& B);
		bool cmpArg(const std::vector<type>& A, const std::vector<type>& B);
		analyzed_function toAnalyzedFunction(function func);
		lstring getFullName(lstring name, const analyzed_functionSet& functionSet);
		bool ifBaseType(const type& A);
		size_t argSize(const type& A);
		size_t countArgSize(const analyzed_functionSet& functionSet, const analyzed_function& func);
		size_t getLocalOffset(const analyzed_function& func, size_t id);
		size_t getVarOffset(const analyzed_function& functionSet, const analyzed_function& func, lstring name);
		size_t constSize(const type& A);
		size_t size(const type& A);
		bool getVarType(lstring name, lstring& type, lstring& var);
		size_t countGlobalSize();
		size_t getStructureSize(lstring type);
		bool ins(lstring tk);
	};
}