#include "../header/AST.h"
using namespace MLang;

size_t AST::countStructureSize(lstring type) {
	if (type == R("")) return 0;
	size_t size{};
	for (size_t i = 0; i < structures.size(); i++) {
		if (structures[i].name == type) {
			if (structures[i].size > 0) return structures[i].size;
			if (list[i]) { error(R("发生循环定义")); return 0; }
		}
		list[i] = true;
		for (size_t j = 0; j < structures[i].elements.size(); j++)
			size += (structures[i].elements[j].array ? DimSize(structures[i].elements[j].dim) : 1) * countStructureSize(structures[i].elements[j].typeName);
		structures[i].size = size;
		return size;
	}
	error(R("未知数据类型:") + type);
	return 0;
}

size_t AST::analyzeStructureSize() {
	for (size_t i = 0; i < structures.size(); i++) {
		error_functionSet = structures[i].name;
		list.resize(structures.size(), 0);
		countStructureSize(structures[i].name);
	}
}
analyzed_functionSet AST::analyzeFunctionSet(functionSet functionSet_) {
	analyzed_functionSet set{};
	set.name = functionSet_.name;
	set.base = functionSet_.base;
	set.local = functionSet_.local;
	set.isClass = functionSet_.isClass;
	set.publiced = functionSet_.publiced;
	set.func.clear();
	for (size_t i = 0; i < functionSet_.func.size(); i++) 
		set.func.push_back(analyzeFunction(functionSet_, functionSet_.func[i]));
	return set;
}
analyzed_function AST::analyzeFunction(functionSet functionSet_, function func) {
	analyzed_function func0{};
	func0.codes.clear();
	size_t i{};
	while (i>=0 && i< func.codes.size())
	{
		i++;
		error_line = i;
		error_function = func.name;
		error_functionSet = functionSet_.name;
		std::vector<lstring> tk = func.codes[i].tokens;
		Tree<node> EX;
		EX.
	}
}
bool AST::analyzeExper(functionSet functionSet_, function func, Tree<node> EX, std::vector<lstring> tk) {}
bool AST::analyze_0(functionSet functionSet_, function func, Tree<node> EX, std::vector<lstring> tk, int num) {}
bool AST::analyze_1(functionSet functionSet_, function func, Tree<node> EX, std::vector<lstring> tk) {}
bool AST::analyze_2(functionSet functionSet_, function func, Tree<node> EX, std::vector<lstring> tk) {}
size_t AST::size(type var) {}
bool AST::getVarType(lstring name, lstring& type, lstring& var) {}
size_t AST::getStructureSize(lstring type) {}
bool AST::analyzeVar(std::vector<lstring> tk, type var) {}
bool AST::haveVar(functionSet functionSet_, function func, lstring name) {}
lstring AST::getVarFullName(functionSet functionSet_, function func, lstring name) {}
bool AST::analyzeArg(functionSet functionSet_, function func, lstring name) {}
bool AST::matchBracket(std::vector<lstring> tk, intptr_t start) {}
bool AST::checkBracket(lstring tk) {}
bool AST::bracketIsMatched(lstring tk1, lstring tk2) {}
lstring AST::getFunctionFullName(lstring name, functionSet functionSet_) {}
void AST::error(lstring err) {}
bool AST::analyze(
	std::vector<lstring> libs_,
	std::vector < type> globalVars_,
	std::vector <functionSet> functionSets_,
	std::vector <structure> structures_,
	functionSet ExtraFunctions_,
	std::vector <type> constants_) {
	libs = libs_;
	globalVars = globalVars_;
	constants = constants_;
	structures = structures_;
	sets = functionSets_;
	ExtraFunctions = ExtraFunctions_;
	Error = false;
	error_type = R("Struct");
	analyzeStructureSize();
	error_type = R("Code");
	for (size_t i = 0; i < sets.size(); i++) analyzed_functionSets.push_back(analyzeFunctionSet(sets[i]));
	return Error;
}
bool AST::getFunctionType(lstring fullname, lstring& type, lstring& super, lstring& name) {}
