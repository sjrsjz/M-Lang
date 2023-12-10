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
		ins(R("Call #label_function_Local") + DIVISION + class_->name + DIVISION + R("_init_ null %") + to_lstring(tmp));
	}
}
void IRGenerator::destroyGenerator(type local, size_t tmp, size_t offset, lstring tk, bool localMode) {
	structure struct0;
	for (auto& x : structures) {
		if (x.name == local.typeName) {
			struct0 = x;
			goto label_destroyGenerator_1;
		}
	}
	error(R("变量/成员 ") + local.name + R(" 的类型未知:") + local.typeName);
	return;
label_destroyGenerator_1:
	intptr_t localOffset{};
	for (auto& x : struct0.elements) {
		size_t localSize = getStructureSize(x.typeName);
		size_t dim = x.array ? x.dim.size() : 1;
		for (size_t i = 0; i < dim; i++) {
			destroyGenerator(x, tmp, localOffset + offset, tk, localMode);
			localOffset += localSize * (localMode ? -1 : 1);
		}
	}
	analyzed_functionSet* class_{};
	for (auto& x : analyzed_functionSets) {
		if (x.name == local.typeName) {
			class_ = &x; break;
		}
	}
	if (class_ && setHasFunction(*class_, R("_destroy_"))) {
		ins(R("address %") + to_lstring(tmp) + R(" ") + tk + to_lstring(offset));
		ins(R("Call #label_function_Local") + DIVISION + class_->name + DIVISION + R("_destroy_ null %") + to_lstring(tmp));
	}
}
void IRGenerator::initSetVars(std::vector<type>& vars, size_t tmp, size_t offset, lstring tk, bool localMode) {
	for (auto& x : vars) {
		size_t size = getStructureSize(x.typeName);
		size_t dim = x.array ? x.dim.size() : 1;
		if (localMode) {
			for (size_t j = 0; j < dim; j++) {
				offset += size;
				initGenerator(x, tmp, offset, tk, true);
			}
		}
		else {
			for (size_t j = 0; j < dim; j++) {
				initGenerator(x, tmp, offset, tk, true);
				offset += size;
			}
		}
	}
}
void IRGenerator::destroySetVars(std::vector<type>& vars, size_t tmp, size_t offset, lstring tk, bool localMode) {
	for (auto& x : vars) {
		size_t size = getStructureSize(x.typeName);
		size_t dim = x.array ? x.dim.size() : 1;
		if (localMode) {
			for (size_t j = 0; j < dim; j++) {
				offset += size;
				destroyGenerator(x, tmp, offset, tk, true);
			}
		}
		else {
			for (size_t j = 0; j < dim; j++) {
				destroyGenerator(x, tmp, offset, tk, true);
				offset += size;
			}
		}
	}
}
bool IRGenerator::handleBuiltInFunctions(analyzed_functionSet& functionSet, analyzed_function& func, Tree<node>& EX, lstring name, type& ret) {

}
bool IRGenerator::cmpTK(const lstring& tk1, const std::vector<lstring>& tk2) {
	for (const auto& x : tk2) {
		if (tk1 == x) return true;
	}
	return false;
}
bool IRGenerator::cmpDim(const std::vector<size_t>& tk1, const std::vector<size_t>& tk2) {
	if (tk1.size() != tk2.size()) return false;
	for (size_t i = 0; i < tk1.size(); i++) {
		if (tk1[i] != tk2[i]) return false;
	}
	return true;
}
bool IRGenerator::generateImplictConversion(type& A, type& B, analyzed_functionSet functionSet, analyzed_function func, Tree<node> EX) {
	lstring t = R("Local") + DIVISION + A.typeName + DIVISION + R("To") + B.typeName;
	size_t id;
	if (haveFunction(t)) {
		std::vector<type> args{}, args2{};
		args.push_back(B);
		args2 = getFunction(functionSet, t, args, {}).args;
		if (cmpArg(args, args2)) {
			id = allocTmpID(A);
			ins(R("Call #label_function_") + t + R(" %") + to_lstring(id) + R(" %") + to_lstring(A.id) + R(" %") + to_lstring(B.id));
			A.id = id;
			return true;
		}
	}
	if ((!ifBaseType(A) || A.address == B.address) 
		&& A.typeName == B.typeName 
		&& A.array && B.array && cmpDim(A.dim, B.dim)
		|| !A.array && !B.array) {
		A.address = B.address;
		return true;
	}
	if (A.array || B.array) {
		error(R("非法强制转换:数组"));
		return false;
	}
	if (B.address && ifBaseType(B)) {
		id = allocTmpID(B);
		ins(R("load %") + to_lstring(id) + R(" %") + to_lstring(B.address) + R(" ") + to_lstring(getStructureSize(B.typeName)));
		B.id = id;
		A.address = false;
	}
	A.id = B.id;
	if (A.typeName == B.typeName) return true;
	if (A.typeName==R("Z")) {
		if (B.typeName == R("B")) {
			A.id = allocTmpID(A);
			ins(R("B2Z %") + to_lstring(A.id) + R(" %") + to_lstring(B.id));
		}
		else if (B.typeName == R("R")) {
			A.id = allocTmpID(A);
			ins(R("R2Z %") + to_lstring(A.id) + R(" %") + to_lstring(B.id));
		}
		else if (B.typeName == R("N")) {
			A.id = allocTmpID(A);
			ins(R("N2Z %") + to_lstring(A.id) + R(" %") + to_lstring(B.id));
		}
		else if (B.typeName == R("Boolen")) {
			ins(R("Boolen2Z %") + to_lstring(A.id) + R(" %") + to_lstring(B.id));
		}
		else goto Error;
	}
	else if (A.typeName == R("R")) {
		if (B.typeName == R("B")) {
			A.id = allocTmpID(A);
			id = allocTmpID(Type_Z);
			ins(R("B2Z %") + to_lstring(id) + R(" %") + to_lstring(B.id));
			ins(R("Z2R %") + to_lstring(A.id) + R(" %") + to_lstring(id));
		}
		else if (B.typeName == R("Z")) {
			A.id = allocTmpID(A);
			ins(R("Z2R %") + to_lstring(A.id) + R(" %") + to_lstring(B.id));
		}
		else if (B.typeName == R("N")) {
			A.id = allocTmpID(A);
			id = allocTmpID(Type_Z);
			ins(R("N2Z %") + to_lstring(id) + R(" %") + to_lstring(B.id));
			ins(R("Z2R %") + to_lstring(A.id) + R(" %") + to_lstring(id));
		}
		else if (B.typeName == R("Boolen")) {
			A.id = allocTmpID(A);
			ins(R("Boolen2R %") + to_lstring(A.id) + R(" %") + to_lstring(B.id));
		}
		else goto Error;
	}
	else if (A.typeName == R("Boolen")) {
		if (B.typeName == R("B")) {
			A.id = allocTmpID(A);
			id = allocTmpID(Type_Z);
			ins(R("B2Z %") + to_lstring(id) + R(" %") + to_lstring(B.id));
			ins(R("Z2Boolen %") + to_lstring(A.id) + R(" %") + to_lstring(id));
		}
		else if (B.typeName == R("Z")) {
			A.id = allocTmpID(A);
			ins(R("Z2Boolen %") + to_lstring(A.id) + R(" %") + to_lstring(B.id));
		}
		else if (B.typeName == R("N")) {
			A.id = allocTmpID(A);
			id = allocTmpID(Type_Z);
			ins(R("N2Z %") + to_lstring(id) + R(" %") + to_lstring(B.id));
			ins(R("Z2Boolen %") + to_lstring(A.id) + R(" %") + to_lstring(id));
		}
		else if (B.typeName == R("R")) {
			A.id = allocTmpID(A);
			ins(R("R2Boolen %") + to_lstring(A.id) + R(" %") + to_lstring(B.id));
		}
		else goto Error;
	}
	else if (A.typeName == R("N")) {
		if (B.typeName == R("B")) {
			A.id = allocTmpID(A);
			id = allocTmpID(Type_Z);
			ins(R("B2Z %") + to_lstring(id) + R(" %") + to_lstring(B.id));
			ins(R("Z2N %") + to_lstring(A.id) + R(" %") + to_lstring(id));
		}
		else if (B.typeName == R("Z")) {
			A.id = allocTmpID(A);
			ins(R("Z2N %") + to_lstring(A.id) + R(" %") + to_lstring(B.id));
		}
		else if (B.typeName == R("R")) {
			A.id = allocTmpID(A);
			id = allocTmpID(Type_Z);
			ins(R("B2Z %") + to_lstring(id) + R(" %") + to_lstring(B.id));
			ins(R("Z2R %") + to_lstring(A.id) + R(" %") + to_lstring(id));
		}
		else if (B.typeName == R("Boolen")) {
			A.id = allocTmpID(A);
			id = allocTmpID(Type_Z);
			ins(R("Boolen2Z %") + to_lstring(id) + R(" %") + to_lstring(B.id));
			ins(R("Z2N %") + to_lstring(A.id) + R(" %") + to_lstring(id));
		}
		else goto Error;
	}
	else if (A.typeName == R("B")) {
		if (B.typeName == R("Boolen")) {
			A.id = allocTmpID(A);
			id = allocTmpID(Type_Z);
			ins(R("Boolen2Z %") + to_lstring(id) + R(" %") + to_lstring(B.id));
			ins(R("Z2B %") + to_lstring(A.id) + R(" %") + to_lstring(id));
		}
		else if (B.typeName == R("Z")) {
			A.id = allocTmpID(A);
			ins(R("Z2B %") + to_lstring(A.id) + R(" %") + to_lstring(B.id));
		}
		else if (B.typeName == R("R")) {
			A.id = allocTmpID(A);
			id = allocTmpID(Type_Z);
			ins(R("R2Z %") + to_lstring(id) + R(" %") + to_lstring(B.id));
			ins(R("Z2B %") + to_lstring(A.id) + R(" %") + to_lstring(id));
		}
		else if (B.typeName == R("N")) {
			A.id = allocTmpID(A);
			id = allocTmpID(Type_Z);
			ins(R("N2Z %") + to_lstring(id) + R(" %") + to_lstring(B.id));
			ins(R("Z2B %") + to_lstring(A.id) + R(" %") + to_lstring(id));
		}
		else goto Error;
	}
	else {
		Error:
		error(R("不支持的强制转换类型:") + B.typeName + R("->") + A.typeName);
		return false;
	}
	return true;
}
bool IRGenerator::haveFunction(lstring name) {
	for (auto& x : analyzed_functionSets) {
		for (auto& y : x.func) {
			if (getFullName(y.name, x) == name) return true;
		}
	}
	return false;
}
bool IRGenerator::ifNotRef(type A) {
	return !A.array&&(cmpTK(A.typeName, { R("N"),R("R") ,R("Z") ,R("B") ,R("Boolen") }));
}
size_t IRGenerator::allocTmpID(type A) {
	size_t size_ = size(A);
	tmpStack.push_back(size_);
	size_t id = tmpStack.size();
	ins(R("tmp %") + to_lstring(id) + R(" ") + to_lstring(size_));
	analyzed_functionSet* class_{};
	for (auto& x : analyzed_functionSets) {
		if (x.name == A.typeName) class_ = &x;
	}
	if (class_ && setHasFunction(*class_, R("_init_"))) {
		ins(R("Call #label_function_Local") + DIVISION + A.typeName + DIVISION + R("_init_ null &%") + to_lstring(id));
	}
	if (class_ && setHasFunction(*class_, R("_destroy_"))) {
		destroyCode.push_back(R("Call #label_function_Local") + DIVISION + A.typeName + DIVISION + R("_destroy_ null &%") + to_lstring(id));
	}
	return id;
}
size_t IRGenerator::tmpOffset(size_t id, const std::vector<size_t>& stack) {
	if (stack.size() <= id) {
		error(R("临时变量ID溢出"));
		return 0;
	}
	size_t offset{};
	for (auto& x : stack) {
		offset += x;
	}
	return offset;
}
bool IRGenerator::setHasFunction(const analyzed_functionSet& functionSet, lstring name) {
	for (auto& x : functionSet.func) {
		if (x.name == name) return true;
	}
	return false;
}
type IRGenerator::maxPrecision(const type& A, const type& B) {
	type t{};
	t.typeName = precisionLevelToType(max_(precisionLevel(A), precisionLevel(B)));
	return t;
}
int IRGenerator::precisionLevel(const type& A) {
	if (A.typeName == R("B")) {
		return 2;
	}
	if (A.typeName == R("B")) {
		return 2;
	}
	if (A.typeName == R("B")) {
		return 5;
	}
	return 2;
}
lstring IRGenerator::precisionLevelToType(int level) {
	switch (level)
	{
		case 0:return R("B");
		case 2:return R("Z");
		case 5:return R("R");
		default:return R("[err]");
	}
}
type IRGenerator::getLocalType(lstring name, const analyzed_function& func) {
	for (auto& x : func.local) {
		if (x.name == name) return x;
	}
	type t{};
	error(R("未知局部变量:") + name);
	return t;
}
type IRGenerator::getConstType(lstring name) {
	for (auto& x : constants) {
		if (x.name == name) return x;
	}
	type t{};
	error(R("未知常量:") + name);
	return t;
}
type IRGenerator::getArgType(const analyzed_functionSet& functionSet, lstring name, const analyzed_function& func) {
	type t{};
	if (name == R("[this]")) {
		t.typeName = functionSet.name;
		t.address = false;
		t.array = false;
		return t;
	}
	for (auto& x : func.args) {
		if (x.name == name) return x;
	}
	error(R("未知参数:") + name);
	return t;
}
structure IRGenerator::getStructure(lstring name) {
	for (auto& x : structures) {
		if (x.name == name) return x;
	}
	structure t{};
	error(R("未知数据结构:") + name);
	return t;
}
type IRGenerator::getSetVarType(lstring name, const analyzed_function& functionSet) {
	for (auto& x : functionSet.local) {
		if (x.name == name) return x;
	}
	type t{};
	error(R("未知程序集变量/类成员:") + name);
	return t;
}
type IRGenerator::getGlobalVarType(lstring name) {
	for (auto& x : globalVars) {
		if (x.name == name) return x;
	}
	type t{};
	error(R("未知全局变量:") + name);
	return t;
}
analyzed_function IRGenerator::getFunction(const analyzed_functionSet& functionSet, lstring fullName, std::vector<type>& args, std::optional<bool> variable) {

}
bool IRGenerator::cmpArgNum(const std::vector<type>& A, const std::vector<type>& B) {

}
bool IRGenerator::cmpArg(const std::vector<type>& A, const std::vector<type>& B) {

}
analyzed_function IRGenerator::toAnalyzedFunction(function func) {

}
lstring IRGenerator::getFullName(lstring name, const analyzed_functionSet& functionSet) {

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