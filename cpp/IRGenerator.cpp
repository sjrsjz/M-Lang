#include "../header/IRGenerator.h"
using namespace MLang;
inline bool IRGenerator::ins(lstring tk) {
	IR += tk + R("\n"); return true;
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
	size_t size_{};
	for (auto& x : var) size_ += size(x);
	return size_;
}
void IRGenerator::generateFunctionSet(analyzed_functionSet& functionSet) {
	if (Error) return;
	for (auto& x : functionSet.func) {
		error_function = x.name;
		generateFunction(functionSet, x);
	}
}
void IRGenerator::generateFunction(analyzed_functionSet& functionSet, analyzed_function& func) {
	if (Error) return;
	ins(R("\n\n#label_function_Local") + DIVISION + functionSet.name + DIVISION + func.name);
	size_t local_size{};
	for (auto& x : func.local) {
		ins(R(";") + x.typeName + R(" ") + x.name + (x.array ? R("[]") : R("")) + R(" size:") + to_lstring(size(x)));
		local_size += size(x);
	}
	currLocalSize = local_size;
	ins(R("enter\nlocal ") + to_lstring(local_size));
	tmpStack.clear();
	ins(R("tmpBegin"));
	size_t tmp = allocTmpID(Type_N);
	initSetVars(func.local, tmp, 0, R("&"), true);
	ins(R("tmpEnd"));
	for (size_t i = 0; i < func.codes.size(); i++) {
		error_line = i;
		generateLine(functionSet, func, func.codes[i]);
	}
	ins(R("#label_function_End_local") + DIVISION + functionSet.name + DIVISION + func.name);
	tmpStack.clear();
	ins(R("tmpBegin"));
	tmp = allocTmpID(Type_N);
	size_t tmp2 = allocTmpID(Type_R);
	ins(R("storeQ %") + to_lstring(tmp2));
	destroySetVars(func.local, tmp, 0, R("&"), true);
	ins(R("loadQ %") + to_lstring(tmp2));
	ins(R("tmpEnd"));
	if (func.transit)
		ins(R("jmp_address"));
	else
		ins(R("return ") + to_lstring(countArgSize(functionSet, func)));

}
void IRGenerator::generateLine(analyzed_functionSet& functionSet, analyzed_function& func, Tree<node>& EX) {
	ins(R("\n;") + functionSet.name + DIVISION + func.name + R(" Line:") + to_lstring(error_line));
	ins(R("tmpBegin"));
	destroyCode.clear();
	tmpStack.clear();
	compileTree(functionSet, func, EX, {});
	for (auto& x : destroyCode) ins(x);
	ins(R("tmpEnd"));
}
void IRGenerator::compileTree(analyzed_functionSet& functionSet, analyzed_function& func, Tree<node>& EX, std::optional<lstring> ExtraInfo) {

}
bool IRGenerator::ifMethod(lstring FullName) {
	lstring type, className, name;
	if (!getFunctionType(FullName, type, className, name)) {
		error(R("错误的函数名称格式:") + FullName);
		return false;
	}
	if (type == R("Local")) {
		for (auto& x : analyzed_functionSets) {
			if (x.name == className) return x.isClass;
		}
		error(R("未知函数:") + FullName);
	}
	return false;
}
size_t IRGenerator::allocStr(lstring text) {
	for (size_t i = 0; i < strings.size(); i++) {
		if (strings[i] == text) return i;
	}
	strings.push_back(text);
	ins(R("[string]") + to_lstring(strings.size()) + R(" ?T") + base64_encode(text));
	return strings.size();
}
type IRGenerator::getElement(analyzed_functionSet& functionSet, lstring struct_, lstring element) {
	for (auto& x : structures) {
		if (x.name == struct_) {
			for (auto& y : x.elements) {
				if (y.name != functionSet.name && x.isClass && !y.publiced) {
					error(R("试图在 ") + struct_ + R(" 中引用未公开的成员 ") + element);
				}
				return y;
			}
		}
	}
	error(R("试图在 ") + struct_ + R(" 中引用不存在的成员 ") + element);
	type ret{};
	return ret;
} 
size_t IRGenerator::getElementOffset(analyzed_functionSet& functionSet, lstring struct_, lstring element) {
	size_t k{};
	for (auto& x : structures) {
		if (x.name == struct_) {
			for (auto& y : x.elements) {
				if (y.name == element) {
					if (x.name != functionSet.name && x.isClass && !y.publiced) {
						error(R("试图在 ") + struct_ + R(" 中引用未公开的成员 ") + element);
						return 0;
					}
					return k;
				}
				k += size(y);
			}
		}
	}
	error(R("试图在 ") + struct_ + R(" 中引用不存在的成员 ") + element);
	return 0;
}
void IRGenerator::initGenerator(type local, size_t tmp, size_t offset, lstring tk, bool localMode) {
	structure struct0;
	for (auto& x : structures) {
		if (x.name == local.typeName) {
			struct0 = x;
			goto label_initGenerator_1;
		}
	}
	error(R("变量/成员 ") + local.name + R(" 的类型未知:") + local.typeName);
	return;
label_initGenerator_1:
	intptr_t localOffset{};
	for (auto& x : struct0.elements) {
		size_t localSize = getStructureSize(x.typeName);
		size_t dim = x.array ? x.dim.size() : 1;
		for (size_t i = 0; i < dim; i++) {
			initGenerator(x, tmp, localOffset + offset, tk, localMode);
			localOffset += localSize * (localMode ? -1 : 1);
		}
	}
	analyzed_functionSet* class_ {};
	for (auto& x : analyzed_functionSets) {
		if (x.name == local.typeName) {
			class_ = &x; break;
		}
	}
	if (class_ && setHasFunction(*class_, R("_init_"))) {
		ins(R("address %") + to_lstring(tmp) + R(" ") + tk + to_lstring(offset));
		ins(R("thisCall #label_function_Local") + DIVISION + class_->name + DIVISION + R("_init_ null %") + to_lstring(tmp));
	}
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

	for (auto& x : analyzed_functionSets) x.size = countVarSize(x.local);
	for (auto& x : constants) if (x.typeName == R("R")) constantData.Attach<double>(std::stod(x.data));
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
	for (auto& x : analyzed_functionSets) {
		if (!x.isClass) {
			initSetVars(x.local, tmp, j, R("$"), false);
			j += x.size;
		}
	}

	ins(R("loadQ %") + to_lstring(tmp2));
	ins(R("tmpEnd"));
	analyzed_functionSet* MainSet{};
	analyzed_function* mainFunc{};
	for(auto& x:analyzed_functionSets) {
		if (x.name == R("Main")) {
			MainSet = &x;
			for (auto& y:x.func) {
				if (y.name == R("main")) {
					mainFunc = &y;
				}
			}
			break;
		}
	}
	MainSet && mainFunc && ins(R("return ") + to_lstring(countArgSize(*MainSet, *mainFunc)));
	
	error_type = R("Code");
	
	GlobalOffset = GlobalSize0;
	for (auto& x : analyzed_functionSets) {
		if (x.name == R("[System]")) {
			for (auto& y : x.func) {
				ins(R("[System]#label_function_Local") + DIVISION + R("[System]") + DIVISION + y.name);
			}
			continue;
		}
		error_functionSet = x.name;
		generateFunctionSet(x);
		if (!x.isClass) GlobalOffset += x.size;
	}
	for (auto& x : ExtraFunctions.func) {
		ins(R("#label_function_Extra") + DIVISION + x.DLL + DIVISION + x.extra_name);
		ins(R("[API]") + base64_encode(x.DLL) + R(" ") + base64_encode(x.extra_name));
	}
	return Error;
}