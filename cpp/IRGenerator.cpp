#include "../header/IRGenerator.h"
using namespace MLang;
void IRGenerator::ins(lstring tk) {
	IR += tk + R("\n");
}

void IRGenerator::error(lstring err) {
	std_lcout << RED << R("[错误][中间代码生成]") << RESET << R("[程序集/类:") << error_functionSet << R("][函数/方法:") << error_function << R("][行:") << error_line + 1 << R("]") << err << std::endl;
	Error = true;
}
bool IRGenerator::getFunctionType(lstring fullName, lstring& type, lstring& super, lstring& name) {
	std::vector<lstring> t = split(fullName, DIVISION);
	if (t.size() < 2 || t.size() > 3) {
		error(R("错误的函数全称")); return false;
	}
	if (t.size() == 2) {
		type = R("Local");
		super = t[0];
		name = t[1];
		return true;
	}
	if (t.size() == 3) {
		type = t[0];
		super = t[1];
		name = t[2];
		return true;
	}
	return false;
}
size_t IRGenerator::countVarSize(const std::vector<type>& var) {

}
void IRGenerator::generateFunctionSet(analyzed_functionSet& functionSet) {

}
void IRGenerator::generateFunction(analyzed_functionSet& functionSet, analyzed_function& func) {

}
void IRGenerator::generateLine(analyzed_functionSet& functionSet, analyzed_function& func, Tree<node>& EX) {

}
void IRGenerator::compileTree(analyzed_functionSet& functionSet, analyzed_function& func, Tree<node>& EX, lstring ExtraInfo) {

}
bool IRGenerator::ifMethod(lstring FullName) {

}
size_t IRGenerator::allocStr(lstring text) {

}
type IRGenerator::getElement(analyzed_functionSet& functionSet, lstring struct_, lstring element) {

}
type IRGenerator::getElementOffset(analyzed_functionSet& functionSet, lstring struct_, lstring element) {

}
void IRGenerator::initGenerator(type local, size_t tmp, size_t offset, lstring tk, bool localMode) {

}
void IRGenerator::destroyGenerator(type local, size_t tmp, size_t offset, lstring tk, bool localMode) {

}
void IRGenerator::initSetVars(std::vector<type>& local, size_t tmp, size_t offset, lstring tk, bool localMode) {

}
void IRGenerator::destroySetVars(std::vector<type>& local, size_t tmp, size_t offset, lstring tk, bool localMode) {

}
bool IRGenerator::handleBuiltInFunctions(analyzed_functionSet& functionSet, analyzed_function& func, Tree<node>& EX, lstring name, type& ret) {

}
bool IRGenerator::cmpTK(std::vector<size_t>& tk1, std::vector<size_t>& tk2) {

}
bool IRGenerator::generateImplictConversion(type& A, type& B, analyzed_functionSet functionSet, analyzed_function func, Tree<node> EX) {

}
bool IRGenerator::haveFunction(lstring name) {

}
bool IRGenerator::isObject(type A) {

}
size_t IRGenerator::allocTmpID(type A) {

}
size_t IRGenerator::tmpOffset(size_t id, const std::vector<size_t>& stack) {

}
bool IRGenerator::setHasFunction(const analyzed_functionSet& functionSet, lstring name) {

}
type IRGenerator::maxPrecision(const type& A, const type& B) {

}
int IRGenerator::precisionLevel(const type& A) {

}
lstring IRGenerator::precisionLevelToType(int level) {

}
type IRGenerator::getLocalType(lstring name, const analyzed_function& func) {

}
type IRGenerator::getConstType(lstring name) {

}
type IRGenerator::getArgType(const analyzed_functionSet& functionSet, lstring name, const analyzed_function& func) {

}
structure IRGenerator::getStructure(lstring name) {

}
type IRGenerator::getSetVarType(lstring name, const analyzed_function& functionSet) {

}
type IRGenerator::getGlobalVarType(lstring name) {

}
analyzed_function IRGenerator::getFunction(const analyzed_functionSet& functionSet, lstring fullName, std::vector<type>& args, std::optional<bool> variable) {

}
bool IRGenerator::cmpArgNum(const std::vector<type>& A, const std::vector<type>& B) {

}
bool IRGenerator::cmpArg(const std::vector<type>& A, const std::vector<type>& B) {

}
analyzed_function IRGenerator::toAnalyzedFunction(function func) {

}
lstring IRGenerator::getFullName(lstring name, const analyzed_function& functionSet) {

}
bool IRGenerator::ifBaseType(const type& A) {

}
size_t IRGenerator::argSize(const type& A) {

}
size_t IRGenerator::countArgSize(const analyzed_functionSet& functionSet, const analyzed_function& func) {

}
size_t IRGenerator::getLocalOffset(const analyzed_function& func, size_t id) {

}
size_t IRGenerator::getVarOffset(const analyzed_function& functionSet, const analyzed_function& func, lstring name) {

}
size_t IRGenerator::constSize(const type& A) {

}
size_t IRGenerator::size(const type& A) {

}
bool IRGenerator::getVarType(lstring name, lstring& type, lstring& var) {

}
size_t IRGenerator::countGlobalSize() {

}
size_t IRGenerator::getStructureSize(lstring type) {

}
bool IRGenerator::analyze(
	std::vector<lstring> libs_,
	std::vector<type> globalVars_,
	std::vector<analyzed_functionSet> analyzed_functionSet_,
	std::vector<functionSet> functionSets_,
	std::vector<structure> structures_,
	functionSet ExtraFunctions_,
	std::vector<type> constants_
) {
	IR = R("");
	libs = libs_;
	globalVars = globalVars_;
	analyzed_functionSets = analyzed_functionSet_;
	functionSets = functionSets_;
	structures = structures_;
	ExtraFunctions = ExtraFunctions_;
	constants = constants_;
	Error = false;
	strings.clear();
	
	Type_N.typeName = R("N");
	Type_N.array = false;
	Type_N.address = false;
	Type_R.typeName = R("N");
	Type_R.array = false;
	Type_R.address = false;
	Type_Z.typeName = R("N");
	Type_Z.array = false;
	Type_Z.address = false;
	Type_B.typeName = R("N");
	Type_B.array = false;
	Type_B.address = false;
	Type_Boolen.typeName = R("N");
	Type_Boolen.array = false;
	Type_Boolen.address = false;

	for (size_t i = 0; i < analyzed_functionSets.size(); i++) analyzed_functionSets[i].size = countVarSize(analyzed_functionSets[i].local);
	for (size_t i = 0; i < constants.size(); i++) {
		if (constants[i].typeName == R("R")) constantData.Attach<double>(std::stod(constants[i].data));
	}
	error_type = R("Struct");
	countGlobalSize();
	ins(R("[GlobalSize]") + to_lstring(GlobalSize));
	ins(R(";Entry"));
	ins(R("enter"));
	ins(R("tmpBegin"));
	size_t tmp = allocTmpID(Type_N);
	size_t tmp2 = allocTmpID(Type_R);
	initSetVars(globalVars, tmp, 0, R("$"), false);
	size_t j = countVarSize(globalVars);
	for (size_t i = 0; i < analyzed_functionSets.size(); i++) {
		if (!analyzed_functionSets[i].isClass) {
			initSetVars(analyzed_functionSets[i].local, tmp, j, R("$"), false);
			j += analyzed_functionSets[i].size;
		}
	}

	ins(R("loadQ %") + to_lstring(tmp2));
	ins(R("tmpEnd"));
	size_t MainSet{}, mainFunc{};
	for (size_t i = 0; i < analyzed_functionSets.size(); i++) {
		if (analyzed_functionSets[i].name == R("Main")) {
			MainSet = i + 1;
			for (size_t j = 0; j < analyzed_functionSets[i].func.size(); j++) {
				mainFunc = j + 1; break;
			}
			break;
		}
	}
	if (MainSet && mainFunc) {
		ins(R("return " + to_lstring(countArgSize(analyzed_functionSets[MainSet - 1], analyzed_functionSets[MainSet - 1].func[mainFunc]))));
	}
	error_type = R("Code");
	
}