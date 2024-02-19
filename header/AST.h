#pragma once
#include "Common.h"
#include "Struct.h"
namespace MLang {
	class AST {
	public:
		std::vector<lstring> libs{};
		std::vector<type> globalVars{};
		std::vector<functionSet> sets{};
		std::vector<structure> structures{};
		std::vector<analyzed_functionSet> analyzed_functionSets{};
		std::vector<type> constants{};
		functionSet ExtraFunctions{};
		bool Error{};
		intptr_t error_line{};
		lstring error_functionSet{};
		lstring error_function{};
		lstring error_type{};
		bool analyze(
			const std::vector<lstring>& libs_,
			const std::vector < type>& globalVars_,
			const std::vector <functionSet>& functionSets_,
			const std::vector <structure>& structures_,
			const functionSet& ExtraFunctions_,
			const std::vector <type>& constants_);
		bool getFunctionType(lstring fullname, lstring& type, lstring& super, lstring& name);

	private:
		std::vector<bool> list{};
		void analyzeStructureSize();
		size_t countStructureSize(lstring type);
		analyzed_functionSet analyzeFunctionSet(functionSet& functionSet_);
		analyzed_function analyzeFunction(functionSet& functionSet_, function& func);
		bool analyzeExper(functionSet& functionSet_, function& func, Tree<node>& EX, std::vector<lstring> tk);
		bool analyze_0(functionSet& functionSet_, function& func, Tree<node>& EX, std::vector<lstring> tk, int num);
		bool analyze_1(functionSet& functionSet_, function& func, Tree<node>& EX, std::vector<lstring> tk);
		bool analyze_2(functionSet& functionSet_, function& func, Tree<node>& EX, std::vector<lstring> tk);
		size_t size(type var);
		bool getVarType(lstring name, lstring& type, lstring& var);
		size_t getStructureSize(lstring type);
		bool analyzeVar(functionSet& functionSet_, function& func, std::vector<lstring> tk, type& var, Tree<node>& TREE);
		bool haveVar(functionSet functionSet_, function func, lstring name);
		lstring getVarFullName(functionSet functionSet_, function func, lstring name);
		bool analyzeArg(functionSet& functionSet_, function& func, Tree<node>& EX, std::vector<lstring> tk);
		size_t matchBracket(std::vector<lstring> tk, intptr_t start);
		intptr_t checkBracket(lstring tk);
		bool bracketIsMatched(lstring tk1, lstring tk2);
		lstring getFunctionFullName(lstring name, functionSet functionSet_);
		void error(lstring err);
		bool haveFunction(functionSet functionSet_, function func, lstring name);
		void analyzePolymorphism(functionSet* currFunctionSet, function* currFunc);
	};
}