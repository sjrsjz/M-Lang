#include "../header/IRGenerator.h"
using namespace MLang;
inline bool IRGenerator::ins(lstring tk) {
	IR += tk + R("\n"); return true;
}

void IRGenerator::error(lstring err) {
	std_lcout << RED << R("[错误]") << YELLOW << R("[中间代码生成]") << RESET << R("[程序集/类:") << error_functionSet << R("][函数/方法:") << error_function << R("][行:") << error_line + 1 << R("]") << err << std::endl;
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
	ins(R("enter"));
	ins(R("local ") + to_lstring(local_size));
	if (functionSet.isClass) {
		ins(R("storeThis &") + to_lstring(getVarOffset(functionSet, func, R("Local") + DIVISION + R("[this]"))));
	}
	if (func.transit) {
		ins(R("storeThisArg &") + to_lstring(getVarOffset(functionSet, func, R("Local") + DIVISION + R("[thisArg]"))) + R(" !") + to_lstring(getVarOffset(functionSet, func, R("Arg") + DIVISION + func.transitArg)));
	}
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
	if (func.transit) {
		ins(R("loadThisArg &") + to_lstring(getVarOffset(functionSet, func, R("Local") + DIVISION + R("[thisArg]"))));
		ins(R("jmp_address"));
	}
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
type IRGenerator::compileTree(analyzed_functionSet& functionSet, analyzed_function& func, Tree<node>& EX, std::optional<lstring> ExtraInfo) {
	type ret{};
	if (Error) return ret;
	node p=EX.Get();
	lstring tk = p.token, type_ = p.type;
	if (type_ == R("Operator")) {
		if (tk == R("=")) {
			if (!EX.child()) {
				error(R("赋值参数过少"));
				return ret;
			}
			type A = compileTree(functionSet, func, EX, {});
			if (!A.address) {
				error(R("赋值左值必须提供指针"));
				return ret;
			}
			if (!EX.next()) {
				error(R("赋值参数过少"));
				EX.parent();
				return ret;
			}
			type B = compileTree(functionSet, func, EX, {});
			type C{};
			lstring tk1 = R("Local") + DIVISION + A.typeName + DIVISION + tk;
			if (haveFunction(tk1)) {
				std::vector<type> args{};
				args.resize(1);
				analyzed_function func0 = getFunction(functionSet, tk1, args, {});
				if (args.size() == 1 && func0.args.size() == 1) {
					size_t id1 = allocTmpID(func0.ret);
					C = func0.args[1];
					ret = func0.ret;
					ret.id = id1;
					if (generateImplictConversion(C, B, functionSet, func, EX)) {
						lstring tk2 = func0.call_type == R("cdecl") ? R("_cdecl") : R("");
						size_t argSize_ID{};
						lstring argSize_T{};
						if (func0.use_arg_size) {
							argSize_ID = allocTmpID(Type_N);
							ins(R("num %") + to_lstring(argSize_ID) + R(" ?I2"));
							argSize_T = R(" %") + to_lstring(argSize_ID);
						}
						if (ifNotRef(func0.ret))
							ins(R("Call ") + tk2 + R(" #label_function_") + tk1 + R(" null % ") + to_lstring(id1) + argSize_T + R(" % ") + to_lstring(A.id) + R(" % ") + to_lstring(C.id));
						else {
							ins(R("CallA ") + tk2 + R(" #label_function_") + tk1 + R(" null &%") + to_lstring(id1) + argSize_T + R(" %") + to_lstring(A.id) + R(" %") + to_lstring(C.id));
							size_t id2 = allocTmpID(Type_N);
							ins(R("address %") + to_lstring(id2) + R(" &%") + to_lstring(id1));
							ret.id = id2;
							ret.address = true;
						}
					}
					EX.parent();
					return ret;
				}
			}
			C = A;
			ret = A;
			if (A.address == B.address && A.typeName == B.typeName && (A.array && B.array && cmpDim(A.dim, B.dim) || !A.array && B.array)) {
				ins(R("mov %") + to_lstring(ret.id) + R(" %") + to_lstring(B.id) + R(" ") + to_lstring(size(B)));
			}
			else {
				if (generateImplictConversion(C, B, functionSet, func, EX)) {
					ins(R("store %") + to_lstring(ret.id) + R(" %") + to_lstring(C.id) + R(" ") + to_lstring(getStructureSize(C.typeName)));
				}
				else {
					error(R("不支持的赋值类型"));
				}
			}
			EX.parent();
			return ret;
		}
		else if (cmpTK(tk, { R("+"),R("-"),R("*"),R("/"),R("%"),R("\\"),R(">"),R("<"),R(">="),R("<="),R("!="),R("==") })) {
			if (!EX.child()) {
				error(R("二元运算参数过少")); return ret;
			}
			type A = compileTree(functionSet, func, EX, {});
			if (!EX.next()) {
				error(R("二元运算参数过少")); EX.parent(); return ret;
			}
			type B = compileTree(functionSet, func, EX, {});
			type C{};
			lstring tk1 = R("Local") + DIVISION + A.typeName + DIVISION + tk;
			if (haveFunction(tk1)) {
				std::vector<type> args{};
				args.resize(1);
				analyzed_function func0 = getFunction(functionSet, tk1, args, {});
				if (args.size() == 1 && func0.args.size() == 1) {
					size_t id1 = allocTmpID(func0.ret);
					C = func0.args[1];
					ret = func0.ret;
					ret.id = id1;
					if (generateImplictConversion(C, B, functionSet, func, EX)) {
						lstring tk2 = func0.call_type == R("cdecl") ? R("_cdecl") : R("");
						size_t argSize_ID{};
						lstring argSize_T{};
						if (func0.use_arg_size) {
							argSize_ID = allocTmpID(Type_N);
							ins(R("num %") + to_lstring(argSize_ID) + R(" ?I2"));
							argSize_T = R(" %") + to_lstring(argSize_ID);
						}
						if (ifNotRef(func0.ret))
							ins(R("Call ") + tk2 + R(" #label_function_") + tk1 + R(" null % ") + to_lstring(id1) + argSize_T + R(" % ") + to_lstring(A.id) + R(" % ") + to_lstring(C.id));
						else {
							ins(R("CallA ") + tk2 + R(" #label_function_") + tk1 + R(" null &%") + to_lstring(id1) + argSize_T + R(" %") + to_lstring(A.id) + R(" %") + to_lstring(C.id));
							size_t id2 = allocTmpID(Type_N);
							ins(R("address %") + to_lstring(id2) + R(" &%") + to_lstring(id1));
							ret.id = id2;
							ret.address = true;
						}
					}
					EX.parent();
					return ret;
				}
			}
			C = maxPrecision(A, B);
			C.typeName = cmpTK(tk, { R("/"),R("\\") }) ? R("R") : C.typeName;
			C.address = false;
			type D = C;
			ret.typeName = cmpTK(tk, { R(">"),R("<"),R(">="),R("<="),R("!="),R("==") }) ? R("Boolen") : C.typeName;
			ret.array = false;
			ret.address = false;
			ret.id = allocTmpID(ret);
			generateImplictConversion(C, A, functionSet, func, EX);
			size_t id1 = C.id;
			generateImplictConversion(D, B, functionSet, func, EX);
			size_t id2 = D.id;
			if (C.typeName == R("Z")) {
				ins(R("opI ") + tk + R(" %") + to_lstring(ret.id) + R(" %") + to_lstring(id1) + R(" %") + to_lstring(id2));
			}
			else if (C.typeName == R("R")) {
				ins(R("opR ") + tk + R(" %") + to_lstring(ret.id) + R(" %") + to_lstring(id1) + R(" %") + to_lstring(id2));
			}
			else if (C.typeName == R("N")) {
				ins(R("opN ") + tk + R(" %") + to_lstring(ret.id) + R(" %") + to_lstring(id1) + R(" %") + to_lstring(id2));
			}
			else if (C.typeName == R("B")) {
				ins(R("opB ") + tk + R(" %") + to_lstring(ret.id) + R(" %") + to_lstring(id1) + R(" %") + to_lstring(id2));
			}
			else {
				error(R("不支持的类型:") + tk + R(" ") + A.typeName + R(" ") + B.typeName);
			}
			EX.parent();
			return ret;
		}
		else if (cmpTK(tk, { R("and"),R("or"),R("xor") })) {
			if (!EX.child()) {
				error(R("二元运算参数过少")); return ret;
			}
			type A = compileTree(functionSet, func, EX, {});
			if (!EX.next()) {
				error(R("二元运算参数过少"));
				EX.parent();
				return ret;
			}
			type B = compileTree(functionSet, func, EX, {});
			type C{};
			lstring tk1 = R("Local") + DIVISION + A.typeName + DIVISION + tk;
			if (haveFunction(tk1)) {
				std::vector<type> args{};
				args.resize(1);
				analyzed_function func0 = getFunction(functionSet, tk1, args, {});
				if (args.size() == 1 && func0.args.size() == 1) {
					size_t id1 = allocTmpID(func0.ret);
					C = func0.args[1];
					ret = func0.ret;
					ret.id = id1;
					if (generateImplictConversion(C, B, functionSet, func, EX)) {
						lstring tk2 = func0.call_type == R("cdecl") ? R("_cdecl") : R("");
						size_t argSize_ID{};
						lstring argSize_T{};
						if (func0.use_arg_size) {
							argSize_ID = allocTmpID(Type_N);
							ins(R("num %") + to_lstring(argSize_ID) + R(" ?I2"));
							argSize_T = R(" %") + to_lstring(argSize_ID);
						}
						if (ifNotRef(func0.ret))
							ins(R("Call ") + tk2 + R(" #label_function_") + tk1 + R(" null % ") + to_lstring(id1) + argSize_T + R(" % ") + to_lstring(A.id) + R(" % ") + to_lstring(C.id));
						else {
							ins(R("CallA ") + tk2 + R(" #label_function_") + tk1 + R(" null &%") + to_lstring(id1) + argSize_T + R(" %") + to_lstring(A.id) + R(" %") + to_lstring(C.id));
							size_t id2 = allocTmpID(Type_N);
							ins(R("address %") + to_lstring(id2) + R(" &%") + to_lstring(id1));
							ret.id = id2;
							ret.address = true;
						}
					}
					EX.parent();
					return ret;
				}
			}
			C = maxPrecision(A, B);
			C.typeName = R("Boolen");
			C.address = false;
			ret.typeName = C.typeName;
			ret.array = false;
			ret.address = false;
			ret.id = allocTmpID(ret);
			generateImplictConversion(C, A, functionSet, func, EX);
			size_t id1 = C.id;
			generateImplictConversion(C, B, functionSet, func, EX);
			size_t id2 = C.id;
			ins(R("opBoolen ") + tk + R(" %") + to_lstring(ret.id) + R(" %") + to_lstring(id1) + R(" %") + to_lstring(id2));
			EX.parent();
			return ret;
		}
		else if (cmpTK(tk, { R("Minus"),R("Abs"),R("not") })) {
			if (!EX.child()) {
				error(R("一元运算参数过少")); return ret;
			}
			type A = compileTree(functionSet, func, EX, {});
			type C = A;
			C.typeName = tk == R("not") ? R("Boolen") : R("R");
			C.address = false;
			C.array = false;
			generateImplictConversion(C, A, functionSet, func, EX);
			size_t id1 = C.id;
			ret.typeName = C.typeName;
			ret.array = false;
			ret.address = false;
			ret.id = allocTmpID(ret);
			if (C.typeName == R("Z")) {
				ins(R("opI ") + tk + R(" %") + to_lstring(ret.id) + R(" %") + to_lstring(id1) + R(" %") + to_lstring(id1));
			}
			else if (C.typeName == R("R")) {
				ins(R("opR ") + tk + R(" %") + to_lstring(ret.id) + R(" %") + to_lstring(id1) + R(" %") + to_lstring(id1));
			}
			else if (C.typeName == R("N")) {
				ins(R("opN ") + tk + R(" %") + to_lstring(ret.id) + R(" %") + to_lstring(id1) + R(" %") + to_lstring(id1));
			}
			else if (C.typeName == R("B")) {
				ins(R("opB ") + tk + R(" %") + to_lstring(ret.id) + R(" %") + to_lstring(id1) + R(" %") + to_lstring(id1));
			}
			else if (C.typeName == R("Boolen")) {
				ins(R("opBoolen ") + tk + R(" %") + to_lstring(ret.id) + R(" %") + to_lstring(id1) + R(" %") + to_lstring(id1));
			}
			else {
				error(R("不支持的类型:") + tk + R(" ") + A.typeName);
			}
			EX.parent();
			return ret;
		}
		else if (tk == R("&")) {
			if (!EX.child()) {
				error(R("一元运算参数过少")); return ret;
			}
			type A = compileTree(functionSet, func, EX, {});
			if (!A.address) {
				error(R("不能取得非指针类型的地址")); return ret;
			}
			ret = A;
			ret.typeName = R("N");
			ret.address = false;
			ret.array = false;
			EX.parent();
			return ret;
		}
		else {
			error(R("未知运算符")); return ret;
		}
	}
	else if (type_==R("Var")) {
		lstring v_type, v_name;
		if (!getVarType(tk, v_type, v_name)) return ret;
		type A{};
		if (v_type == R("Local")) {
			A = getLocalType(v_name, func);
			A.id = allocTmpID(Type_N);
			ins(R("address %") + to_lstring(A.id) + R(" &") + to_lstring(getVarOffset(functionSet, func, tk)));
			if (v_name == R("[this]")) {
				ins(R("load %") + to_lstring(A.id) + R(" %") + to_lstring(A.id) + R(" ") + to_lstring(getStructureSize(R("N"))));
			}
			A.address = true;
			return A;
		}
		if (v_type == R("Arg")) {
			A = getArgType(functionSet, v_name, func);
			A.id = allocTmpID(Type_N);
			ins(R("address %") + to_lstring(A.id) + R(" !") + to_lstring(getVarOffset(functionSet, func, tk)));
			if (!ifNotRef(A) || v_name == R("[ret]")) {
				ins(R("load %") + to_lstring(A.id) + R(" %") + to_lstring(A.id) + R(" ") + to_lstring(getStructureSize(R("N"))));
			}
			A.address = true;
			return A;
		}
		if (v_type == R("Set")) {
			A = getSetVarType(v_name, functionSet);
			A.id = allocTmpID(Type_N);
			ins(R("address %") + to_lstring(A.id) + R(" $") + to_lstring(GlobalOffset + getVarOffset(functionSet, func, tk)));
			A.address = true;
			return A;
		}
		if (v_type == R("Global")) {
			A = getGlobalVarType(v_name);
			A.id = allocTmpID(Type_N);
			ins(R("address %") + to_lstring(A.id) + R(" $") + to_lstring(getVarOffset(functionSet, func, tk)));
			A.address = true;
			return A;
		}
		if (v_type == R("Const")) {
			if (v_name == R("[true]")) {
				A.typeName = R("Boolen");
				A.id = allocTmpID(Type_Boolen);
				A.address = false;
				A.array = false;
				ins(R("num %") + to_lstring(A.id) + R(" ?I1065353216"));
			}
			else if (v_name == R("[false]")) {
				A.typeName = R("Boolen");
				A.id = allocTmpID(Type_Boolen);
				A.address = false;
				A.array = false;
				ins(R("num %") + to_lstring(A.id) + R(" ?I0"));
			}
			else {
				A = getConstType(v_name);
				A.id = allocTmpID(Type_N);
				ins(R("address %") + to_lstring(A.id) + R(" @") + to_lstring(getVarOffset(functionSet, func, tk)));
				A.address = true;
			}
			return A;
		}
		if (v_type == R("Class")) {
			A = getSetVarType(v_name, functionSet);
			A.id = allocTmpID(Type_N);
			ins(R("address %") + to_lstring(A.id) + R(" !") + to_lstring(getVarOffset(functionSet, func, R("Local") + DIVISION + R("[this]"))));
			ins(R("load %") + to_lstring(A.id) + R(" %") + to_lstring(A.id) + R(" ") + to_lstring(getStructureSize(R("N"))));
			ins(R("offset %") + to_lstring(A.id) + R(" ") + to_lstring(getElementOffset(functionSet, functionSet.name, v_name)));
			A.address = true;
			return A;
		}
		error(R("未知变量:") + tk);
		return ret;
	}
	else if (type_ == R("Call")) {
		if (handleBuiltInFunctions(functionSet, func, EX, tk, ret)) return ret;
		lstring arg_T{};
		if (ifMethod(tk)) {
			size_t id1 = allocTmpID(Type_N);
			ins(R("address &") + to_lstring(id1) + R(" !") + to_lstring(getVarOffset(functionSet, func, R("Local") + DIVISION + R("[this]"))));
			ins(R("load %") + to_lstring(id1) + R(" %") + to_lstring(id1) + R(" ") + to_lstring(getStructureSize(R("N"))));
			arg_T = R(" %") + to_lstring(id1);
		}
		std::vector<type> args{};
		analyzed_function func0;
		if(EX.child()){
			do {
				args.push_back(compileTree(functionSet, func, EX, {}));
			} while (EX.next());
			func0 = getFunction(functionSet, tk, args, {});
			for (size_t i = 0; i < func0.args.size(); i++) {
				generateImplictConversion(func0.args[i], args[i], functionSet, func, EX);
				arg_T += R(" %") + to_lstring(func0.args[i].id);
			}
			EX.parent();
		}
		else {
			func0 = getFunction(functionSet, tk, args, {});
		}
		ret = func0.ret;
		ret.id = allocTmpID(func0.ret);
		lstring tk2 = func0.call_type == R("cdecl") ? R("_cdecl") : R("");
		size_t argSize_ID{};lstring argSize_T{};
		if (func0.use_arg_size) {
			argSize_ID = allocTmpID(Type_N);
			ins(R("num %") + to_lstring(argSize_ID) + R(" ?I") + to_lstring(args.size()));
			argSize_T = R(" %") + to_lstring(argSize_ID);
		}
		if (ifNotRef(ret)) {
			ins(R("Call ") + tk2 + R(" #label_function_") + tk + R(" null % ") + to_lstring(ret.id) + argSize_T + arg_T);
		}
		else
		{
			ins(R("CallA ") + tk2 + R(" #label_function_") + tk + R(" null &%") + to_lstring(ret.id) + argSize_T + arg_T);
			size_t id1 = allocTmpID(Type_N);
			ins(R("address %") + to_lstring(id1) + R(" &%") + to_lstring(ret.id));
			ret.id = id1;
			ret.address = true;
		}
		return ret;
	}
	else if (type_ == R("thisCall")) {
		std::vector<type> args{};lstring arg_T{};
		analyzed_function func0{}; lstring tk1{};
		type A{};
		if (EX.child()) {
			A = compileTree(functionSet, func, EX, {});
			if (A.array) {
				error(R("方法的被引用结构不能为数组"));
				return ret;
			}
			if (!A.address) {
				error(R("方法的被引用结构必须为指针"));
				return ret;
			}
			while (EX.next()) {
				args.push_back(compileTree(functionSet, func, EX, {}));
			} 
			tk1 = R("Local") + DIVISION + A.typeName + DIVISION + tk;
			func0 = getFunction(functionSet, tk1, args, {});
			for (size_t i = 0; i < func0.args.size(); i++) {
				generateImplictConversion(func0.args[i], args[i], functionSet, func, EX);
				arg_T += R(" %") + to_lstring(func0.args[i].id);
			}
			EX.parent();
		}
		else {
			error(R("方法引用必须提供被引用结构"));
			return ret;
		}
		ret = func0.ret;
		ret.id = allocTmpID(func0.ret);
		lstring tk2 = func0.call_type == R("cdecl") ? R("_cdecl") : R("");
		size_t argSize_ID{}; lstring argSize_T{};
		if (func0.use_arg_size) {
			argSize_ID = allocTmpID(Type_N);
			ins(R("num %") + to_lstring(argSize_ID) + R(" ?I") + to_lstring(args.size()));
			argSize_T = R(" %") + to_lstring(argSize_ID);
		}
		if (ifNotRef(ret)) {
			ins(R("Call ") + tk2 + R(" #label_function_") + tk1 + R(" % ") + to_lstring(ret.id) + R(" %") + to_lstring(A.id) + argSize_T + arg_T);
		}
		else {
			ins(R("CallA ") + tk2 + R(" #label_function_") + tk1 + R(" &%") + to_lstring(ret.id) + R(" %") + to_lstring(A.id) + argSize_T + arg_T);
			size_t id1 = allocTmpID(Type_N);
			ins(R("address %") + to_lstring(id1) + R(" &%") + to_lstring(ret.id));
			ret.id = id1;
			ret.address = true;
		}
		return ret;
	}
	else if(type_==R("Const")) {
		type A{};
		if (!ExtraInfo.has_value() || ExtraInfo.value() != R("STRING")) {
			A.typeName = R("B");
			tk = process_quotation_mark(tk);
			A.dim.push_back(tk.size());
			A.id = allocTmpID(Type_Boolen);
			A.array = true;
			A.address = true;
			ins(R("address %") + to_lstring(A.id) + R(" ?T") + to_lstring(allocStr(tk)));
		}
		else {
			A.typeName = R("[STRING]");
			A.name = process_quotation_mark(tk);
		}
		return A;
	}
	else if (type_ == R("Int")) {
		type A{};
		A.typeName = R("Z");
		A.address = false;
		A.array = false;
		A.id = allocTmpID(Type_Z);
		ins(R("num %") + to_lstring(A.id) + R(" ?I") + tk);
		return A;
	}
	else if (type_ == R("Double")) {
		type A{};
		A.typeName = R("R");
		A.address = false;
		A.array = false;
		A.id = allocTmpID(Type_R);
		ins(R("num %") + to_lstring(A.id) + R(" ?D") + tk);
		return A;
	}
	else if (type_ == R("Array")) {
		EX.child();
		type ret0 = compileTree(functionSet, func, EX, {});

		type C = ret0;
		ret = ret0;
		ret.array = false;
		std::vector<type> args{};
		while (EX.next())
		{
			ret0 = compileTree(functionSet, func, EX, {});
			args.push_back(ret0);
		}
		lstring tk1 = R("Local") + DIVISION + C.typeName + DIVISION + R("[]");
		lstring arg_T{};
		if (haveFunction(tk1)) {
			analyzed_function func0 = getFunction(functionSet, tk1, args, true);
			for (auto& x : args) {
				type Ntype{}; Ntype.typeName = R("N");
				generateImplictConversion(Ntype, x, functionSet, func, EX);
				arg_T += R(" %") + to_lstring(Ntype.id);
			}
			ret = func0.ret;
			ret.id = allocTmpID(Type_N);
			type B{};
			B.typeName = R("N");
			B.id = allocTmpID(Type_N);
			ins(R("num %") + to_lstring(B.id) + R(" ?uI") + to_lstring(args.size()));
			arg_T = R(" %") + to_lstring(B.id) + arg_T;
			if (func0.call_type != R("cdecl")) {
				error(R("不支持的调用方式:") + func0.call_type);
			}
			else if (!func0.use_arg_size) {
				error(R("未声明传入参数个数"));
			}
			if (ifNotRef(ret)) {
				ins(R("Call_cdecl #label_function_") + tk1 + R(" null % ") + to_lstring(ret.id) + R(" %") + to_lstring(C.id) + arg_T);
			}
			else {
				ins(R("CallA_cdecl #label_function_") + tk1 + R(" null &%") + to_lstring(ret.id) + R(" %") + to_lstring(C.id) + arg_T);
			}
		}
		else {
			if (!ret0.array) {
				error(R("下标的作用对象必须为数组"));
				return ret;
			}
			type B{};
			B.typeName = R("N");
			B.id = allocTmpID(Type_N);
			ins(R("num %") + to_lstring(B.id) + R(" ?uI0"));
			type Ntype{};
			Ntype.typeName = R("N");
			type C{};

			if (args.size()) {
				generateImplictConversion(Ntype, args[0], functionSet, func, EX);
				ins(R("opN + %") + to_lstring(B.id) + R(" %") + to_lstring(B.id) + R(" %") + to_lstring(Ntype.id));
				C.typeName = R("N");
				C.id = allocTmpID(Type_N);
				for (size_t i = args.size() - 2; i >= 0; i++) {
					ins(R("num %") + to_lstring(C.id) + R(" ?uI") + to_lstring(ret.dim[i]));
					generateImplictConversion(Ntype, args[i + 1], functionSet, func, EX);
					ins(R("opN * %") + to_lstring(B.id) + R(" %") + to_lstring(B.id) + R(" %") + to_lstring(C.id));
					C.typeName = R("N");
					C.id = allocTmpID(Type_N);
					ins(R("opN + %") + to_lstring(B.id) + R(" %") + to_lstring(B.id) + R(" %") + to_lstring(Ntype.id));
				}
			}
			C.typeName = R("N");
			C.id = allocTmpID(Type_N);
			ins(R("num %") + to_lstring(C.id) + R(" ?uI") + to_lstring(getStructureSize(ret.typeName)));
			ins(R("opN * %") + to_lstring(B.id) + R(" %") + to_lstring(B.id) + R(" %") + to_lstring(C.id));
			type A{};
			A.typeName = R("N");
			A.id = allocTmpID(Type_N);
			ins(R("opN + %") + to_lstring(A.id) + R(" %") + to_lstring(ret.id) + R(" %") + to_lstring(B.id));
			ret.id = A.id;
			ret.address = true;
		}
		EX.parent();
		return ret;
	}
	else if (type_ == R("ad_v")) {
		if (!EX.child()) {
			error(R("错误的指针用法")); return ret;
		}
		type A = compileTree(functionSet, func, EX, {});
		type C = Type_N;
		generateImplictConversion(C, A, functionSet, func, EX);
		C.address = true;
		C.typeName = process_quotation_mark(tk);
		ret = C;
		EX.parent();
		return ret;
	}
	else if (type_ == R("Block")) {
		if (EX.child()) {
			if(ExtraInfo.has_value())
				do
				{
					ret = compileTree(functionSet, func, EX, ExtraInfo);
				} while (EX.next());
			else
				do
				{
					ret = compileTree(functionSet, func, EX, {});
				} while (EX.next());
			EX.parent();
		}
		return ret;
	}
	else if (type_==R("Element")) {
		if (EX.child()) {
			type A = compileTree(functionSet, func, EX, {});
			if (A.array) {
				error(R("被引用的对象不能为数组"));
			}
			else if (!A.address) {
				error(R("被引用的对象必须为指针"));
			}
			else {
				ins(R("offset %") + to_lstring(A.id) + R(" ") + to_lstring(getElementOffset(functionSet, A.typeName, tk)));
				ret = getElement(functionSet, A.typeName, tk);
				ret.id = A.id;
				ret.address = true;
			}
			EX.parent();
		}
		else {
			error(R("引用的对象不能为空"));
		}
		return ret;
	}
	else if (type_==R("FunctionAddress")) {
		ret.typeName = R("N");
		ret.array = false;
		ret.address = false;
		ret.id = allocTmpID(ret);
		ins(R("ExactlyAddress %") + to_lstring(ret.id) + R(" #label_function_") + getFullName(tk,functionSet));
		return ret;
	}else if(type_==R("ExactlyAddress")) {
		ret.typeName = R("N");
		ret.array = false;
		ret.address = false;
		ret.id = allocTmpID(ret);
		ins(R("ExactlyAddress %") + to_lstring(ret.id));
		return ret;
	}
	else if (type_ == R("")) return ret;
	else
	{
		error(R("不支持的结构:Type = ") + type_ + R(" Token = ") + tk);
		ret.can_be_ignored = false;
		return ret;
	}
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
	analyzed_functionSet* functionSet0{};
	for (auto& x : analyzed_functionSets) {
		if (x.name == R("[System]")) {
			functionSet0 = &x;
			goto label_handleBuiltInFunctions_1;
		}
	}
	error(R("未找到内建函数集: [System]"));
	return false;
label_handleBuiltInFunctions_1:
	if (name == getFullName(R("if"), *functionSet0)) {
		if (EX.child()) {
			type A = compileTree(functionSet, func, EX, {});
			if (A.typeName != R("Boolen") || A.array) {
				error(R("if语句的条件必须为Boolen类型"));
				EX.parent();
				return false;
			}
			label++;
			intptr_t label1 = label;
			size_t id1{};
			if (A.address) {
				id1 = allocTmpID(Type_Boolen);
				ins(R("load %") + to_lstring(id1) + R(" %") + to_lstring(A.id) + R(" ") + to_lstring(getStructureSize(R("Boolen"))));
				A.id = id1;
			}
			ins(R("jz %") + to_lstring(id1) + R(" #label_if_B_") + to_lstring(label1));
			ins(R("#label_if_A_") + to_lstring(label1));
			if (EX.next()) {
				compileTree(functionSet, func, EX, {});
				ins(R("jmp #label_if_End_") + to_lstring(label1));
			}
			ins(R("#label_if_B_") + to_lstring(label1));
			if (EX.next()) {
				compileTree(functionSet, func, EX, {});
				ins(R("jmp #label_if_End_") + to_lstring(label1));
			}
			ins(R("#label_if_End_") + to_lstring(label1));
			EX.parent();
			return true;
		}
		error(R("if语句的条件不能为空"));
		return false;
	}
	else if (name == getFullName(R("while"), *functionSet0)) {
		if (EX.child()) {
			label++;
			intptr_t label1 = label;
			ins(R("#label_while_Start_") + to_lstring(label1));
			type A = compileTree(functionSet, func, EX, {});
			if (A.typeName != R("Boolen") || A.array) {
				error(R("while语句的条件必须为Boolen类型"));
				EX.parent();
				return false;
			}
			if (A.address) {
				size_t id1 = allocTmpID(Type_Boolen);
				ins(R("load %") + to_lstring(id1) + R(" %") + to_lstring(A.id) + R(" ") + to_lstring(getStructureSize(R("Boolen"))));
				A.id = id1;
			}
			ins(R("jz %") + to_lstring(A.id) + R(" #label_while_End_") + to_lstring(label1));
			loopStartStack.push_back(R("#label_while_Start_") + to_lstring(label1));
			loopEndStack.push_back(R("#label_while_End_") + to_lstring(label1));

			if (EX.next()) {
				compileTree(functionSet, func, EX, {});
			}
			loopStartStack.pop_back();
			loopEndStack.pop_back();
			ins(R("jmp #label_while_Start_") + to_lstring(label1));
			ins(R("#label_while_End_") + to_lstring(label1));
			EX.parent();
			return true;
		}
	}
	else if (name == getFullName(R("do_while"), *functionSet0)) {
		if (EX.child()) {
			label++;
			intptr_t label1 = label;
			ins(R("jmp #label_do_while_A_") + to_lstring(label1));
			ins(R("#label_do_while_Start_") + to_lstring(label1));
			type A = compileTree(functionSet, func, EX, {});
			if (A.typeName != R("Boolen") || A.array) {
				error(R("do_while语句的条件必须为Boolen类型"));
				EX.parent();
				return false;
			}
			if (A.address) {
				size_t id1 = allocTmpID(Type_Boolen);
				ins(R("load %") + to_lstring(id1) + R(" %") + to_lstring(A.id) + R(" ") + to_lstring(getStructureSize(R("Boolen"))));
				A.id = id1;
			}
			ins(R("jz %") + to_lstring(A.id) + R(" #label_do_while_End_") + to_lstring(label1));
			if (!EX.next()) {
				error(R("do_while语句的循环体不能为空"));
				return false;
			}
			ins(R("#label_do_while_A_") + to_lstring(label1));
			loopStartStack.push_back(R("#label_do_while_Start_") + to_lstring(label1));
			loopEndStack.push_back(R("#label_do_while_End_") + to_lstring(label1));
			compileTree(functionSet, func, EX, {});
			loopStartStack.pop_back();
			loopEndStack.pop_back();
			ins(R("jmp #label_do_while_Start_") + to_lstring(label1));
			ins(R("#label_do_while_End_") + to_lstring(label1));
			EX.parent();
			return true;
		}
	}
	else if (name == getFullName(R("return"), *functionSet0)) {
		type func_ret = func.transit ? Type_N : func.ret;
		if (EX.child()) {
			type A = compileTree(functionSet, func, EX, {});
			if (ifNotRef(func_ret)) {
				if (A.address) {
					size_t id1 = allocTmpID(func_ret);
					ins(R("load %") + to_lstring(id1) + R(" %") + to_lstring(A.id) + R(" ") + to_lstring(size(A)));
					A.id = id1;
					A.address = false;
				}
				type B = func_ret;
				generateImplictConversion(B, A, functionSet, func, EX);
				ins(R("ret %") + to_lstring(B.id) + R(" ") + to_lstring(size(B)));
				ins(R("jmp #label_function_End_") + getFullName(func.name, functionSet));
				EX.parent();
				return true;
			}
			else {
				if (!A.address) {
					error(R("返回值必须为指针"));
					EX.parent();
					return false;
				}
				if (func_ret.array != A.array) {
					error(R("返回值的数组性质与函数声明不符"));
					EX.parent();
					return false;
				}
				if (size(func_ret) != size(A)) {
					error(R("返回值的大小与函数声明不符"));
					EX.parent();
					return false;
				}
				lstring tk1 = R("Local") + DIVISION + func_ret.typeName + DIVISION + R("_return_" + A.typeName);
				if (haveFunction(tk1)) {
					std::vector<type> args{};
					analyzed_function func0 = getFunction(functionSet, tk1, args, {});
					if (func0.args.size() == 1) {
						size_t id1 = allocTmpID(func_ret);
						type C = func0.args[0];
						generateImplictConversion(C, A, functionSet, func, EX);
						ret = func0.ret;
						ret.id = id1;
						size_t id2 = allocTmpID(Type_N);
						ins(R("address %") + to_lstring(id2) + R(" !") + to_lstring(getVarOffset(functionSet, func, R("Arg") + DIVISION + R("[ret]"))));
						ins(R("load %") + to_lstring(id2) + R(" %") + to_lstring(id2) + R(" ") + to_lstring(getStructureSize(R("N"))));
						size_t ArgSize_ID{};
						lstring ArgSize_T{};
						if (func0.use_arg_size) {
							ArgSize_ID = allocTmpID(Type_N);
							ins(R("num %") + to_lstring(ArgSize_ID) + R(" ?I") + to_lstring(args.size()));
							ArgSize_T = R(" %") + to_lstring(ArgSize_ID);
						}
						if (ifNotRef(func0.ret)) {
							ins(R("Call #label_function_") + tk1 + R(" null %") + to_lstring(id1) + ArgSize_T + R(" %") + to_lstring(id2) + R(" %") + to_lstring(C.id));
						}
						else {
							ins(R("CallA #label_function_") + tk1 + R(" null &%") + to_lstring(id1) + ArgSize_T + R(" %") + to_lstring(id2) + R(" %") + to_lstring(C.id));
							id2 = allocTmpID(Type_N);
							ins(R("address %") + to_lstring(id2) + R(" &%") + to_lstring(id1));
							ret.id = id2;
							ret.address = true;
						}
						ins(R("jmp #label_function_End_") + getFullName(func.name, functionSet));
						EX.parent();
						return true;
					}

				}
				size_t id1 = allocTmpID(func_ret);
				ins(R("address %") + to_lstring(id1) + R(" !") + to_lstring(getVarOffset(functionSet, func, R("Arg") + DIVISION + R("[ret]"))));
				ins(R("load %") + to_lstring(id1) + R(" %") + to_lstring(id1) + R(" ") + to_lstring(getStructureSize(R("N"))));
				ins(R("mov %") + to_lstring(id1) + R(" %") + to_lstring(A.id) + R(" ") + to_lstring(size(A)));
				ins(R("jmp #label_function_End_") + getFullName(func.name, functionSet));
				EX.parent();
				return true;
			}
		}
	}
	else if (name == getFullName(R("break"), *functionSet0)) {
		if (loopEndStack.size()) {
			ins(R("jmp ") + loopEndStack.back());
			return true;
		}
		else {
			error(R("break语句必须在循环体内"));
			return false;
		}
	}
	else if (name == getFullName(R("continue"), *functionSet0)) {
		if (loopStartStack.size()) {
			ins(R("jmp ") + loopStartStack.back());
			return true;
		}
		else {
			error(R("continue语句必须在循环体内"));
			return false;
		}
	}
	else if (name == getFullName(R("N"), *functionSet0)) {
		ret = Type_N;
		if (EX.child()) {
			type A = compileTree(functionSet, func, EX, {});
			EX.parent();
			if (generateImplictConversion(ret, A, functionSet, func, EX))
				return true;
		}
		return false;
	}
	else if (name == getFullName(R("R"), *functionSet0)) {
		ret = Type_R;
		if (EX.child()) {
			type A = compileTree(functionSet, func, EX, {});
			EX.parent();
			if (generateImplictConversion(ret, A, functionSet, func, EX))
				return true;
		}
		return false;
	}
	else if (name == getFullName(R("Z"), *functionSet0)) {
		ret = Type_Z;
		if (EX.child()) {
			type A = compileTree(functionSet, func, EX, {});
			EX.parent();
			if (generateImplictConversion(ret, A, functionSet, func, EX))
				return true;
		}
		return false;
	}
	else if (name == getFullName(R("B"), *functionSet0)) {
		ret = Type_B;
		if (EX.child()) {
			type A = compileTree(functionSet, func, EX, {});
			EX.parent();
			if (generateImplictConversion(ret, A, functionSet, func, EX))
				return true;
		}
		return false;
	}
	else if (name == getFullName(R("Boolen"), *functionSet0)) {
		ret = Type_Boolen;
		if (EX.child()) {
			type A = compileTree(functionSet, func, EX, {});
			EX.parent();
			if (generateImplictConversion(ret, A, functionSet, func, EX))
				return true;
		}
		return false;
	}
	else if (name == getFullName(R("Pause"), *functionSet0)) {
		ins(R("pause"));
		return false;
	}
	else if (name == getFullName(R("_IR_"), *functionSet0)) {
		ret = Type_N;
		if (EX.child()) {
			type A = compileTree(functionSet, func, EX, R("STRING"));
			EX.parent();
			if (A.typeName == R("[STRING]")) {
				ins(A.name);
				return true;
			}
			error(R("_IR_函数的参数必须为字符串"));
			return false;
		}
	}
	return false;
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
type IRGenerator::getSetVarType(lstring name, const analyzed_functionSet& functionSet) {
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
	lstring type{}, className{}, name{};
	analyzed_function tfunc{};
	if (!getFunctionType(fullName, type, className, name)) {
		error(R("错误的函数名称格式:") + fullName);
		return tfunc;
	}
	if (type == R("Local")) {
		for (auto& x : analyzed_functionSets) {
			analyzed_function* k{};
			bool a{};
			for (auto& y : x.func) {
				if (y.name == name) {
					k = &y;
					a =x.isClass && (y.publiced && x.name != functionSet.name || x.name == functionSet.name);
					if (a && cmpArg(y.args, args)) return y;
				}
			}
			if (k) {
				if (x.isClass && !a) {
					error(R("试图调用 ") + x.name + R(" 中未公开的方法:") + k->name);
					return tfunc;
				}
				if (!cmpArgNum(k->args, args) && !variable && !k->use_arg_size) {
					error(R("参数过多或过少:") + fullName);
					return tfunc;
				}
				return *k;
			}
		}
		error(R("未知函数:") + fullName);
	}
	else if (type == R("Extra")) {
		for (auto& x : ExtraFunctions.func) {
			if (x.extra_name == name) {
				return toAnalyzedFunction(x);
			}
		}
		error(R("未知外部函数:") + fullName);
	}
	else {
		error(R("未知函数:") + fullName);
	}
	return tfunc;
}
bool IRGenerator::cmpArgNum(const std::vector<type>& A, const std::vector<type>& B) {
	size_t j{}, k{};
	for (intptr_t i = B.size() - 1; i >= 0; i--) {
		if (!B[i].can_be_ignored) break;
		j++;
	}
	for (intptr_t i = A.size() - 1; i >= 0; i--) {
		if (!A[i].can_be_ignored) break;
		k++;
	}
	return A.size() == B.size() - j || A.size() - k == B.size() - j;
}
bool IRGenerator::cmpArg(const std::vector<type>& A, const std::vector<type>& B) {
	if (A.size() != B.size()) return false;
	for (size_t i = 0; i < A.size(); i++) {
		if (A[i].typeName != B[i].typeName || A[i].array != B[i].array) return false;
	}
	return true;
}
analyzed_function IRGenerator::toAnalyzedFunction(function func) {
	analyzed_function tfunc{};
	tfunc.ret = func.ret;
	tfunc.api = func.api;
	tfunc.call_type = func.call_type;
	tfunc.DLL = func.DLL;
	tfunc.name = func.name;
	tfunc.args = func.args;
	tfunc.local = func.local;
	tfunc.publiced = func.publiced;
	tfunc.extra_name = func.extra_name;
	tfunc.externed = func.externed;
	tfunc.transit = func.transit;
	tfunc.use_arg_size = func.use_arg_size;
	return tfunc;
}
lstring IRGenerator::getFullName(lstring name, const analyzed_functionSet& functionSet) {
	std::vector<lstring> t = split(name, DIVISION);
	lstring className{}, func{};
	if (!t.size()) return R("");
	if (t.size() == 1) {
		className = R("");
		func = t[0];
	}
	else if (t.size() == 2) {
		className = t[0];
		func = t[1];
	}
	else {
		error(R("错误的函数名称格式") + name);
		return R("");
	}
	if (className != R("") && functionSet.name == className || className == R("")) {
		for (auto& x : functionSet.func) return R("Local") + DIVISION + functionSet.name + DIVISION + func;
	}
	for (auto& x : analyzed_functionSets) {
		for (auto& y : x.func) {
			if (x.name == functionSet.name) continue;
			if (y.name == func) {
				if (x.isClass && x.name != className) continue;
				if (x.isClass && !y.publiced) {
					error(R("试图调用 ") + className + R(" 中未公开的方法:") + func);
					return R("");
				}
				return R("Local") + DIVISION + x.name + DIVISION + func;
			}
		}
	}
	for (auto& x : ExtraFunctions.func) {
		if (className != R("")) {
			if (x.DLL == className) {
				return R("Extra") + DIVISION + className + DIVISION + x.extra_name;
			}
			else continue;
		}
		if (x.name == func) return R("Extra") + DIVISION + className + DIVISION + x.extra_name;
	}
	return R("Unknown") + DIVISION + R("Unknown") + DIVISION + func;
}
bool IRGenerator::ifBaseType(const type& A) {
	return !A.array && cmpTK(A.typeName, { R("N"),R("R"),R("Z"),R("B"),R("Boolen") });
}
size_t IRGenerator::argSize(const type& A) {
	if (A.address || !ifNotRef(A)) return getStructureSize(R("N"));
	if (A.typeName == R("B")) return 2;
	return getStructureSize(A.typeName);
}
size_t IRGenerator::countArgSize(const analyzed_functionSet& functionSet, const analyzed_function& func) {
	size_t size{};
	if (func.call_type == R("stdcall")) {
		if (functionSet.isClass) size += 0;
		if (!ifBaseType(func.ret)) size += getStructureSize(R("N"));
		for (auto& x : func.args) size += argSize(x);
	}
	else if (func.call_type == R("cdecl")) size = 0;
	return size;
}
size_t IRGenerator::getLocalOffset(const analyzed_function& func, size_t id) {
	if (id >= func.local.size()) {
		error(R("局部变量ID溢出"));
		return 0;
	}
	size_t offset{};
	for (size_t i = 0; i < id; i++) {
		offset += size(func.local[i]);
	}
	return offset;
}
size_t IRGenerator::getVarOffset(const analyzed_functionSet& functionSet, const analyzed_function& func, lstring name) {
	lstring type{}, var{};
	if (!getVarType(name, type, var)) return 0;
	size_t offset{};
	if (type == R("Local")) {
		if (functionSet.isClass) offset += getStructureSize(R("N"));
		if (var == R("[this]")) return offset;
		for (auto& x : func.local) {
			offset += size(x);
			if (x.name == var) return offset;
		}
		error(R("未知局部变量:") + var);
	}
	if (type == R("Arg")) {
		if (var == R("[ret]")) return offset;
		if(!ifNotRef(func.ret)) offset += getStructureSize(R("N"));
		for (auto& x : func.args) {
			if (x.name == var) return offset;
			offset += size(x);
		}
		error(R("未知参数:") + var);
	}
	if (type == R("Class")) {
		for (auto& x : functionSet.local) {
			if (x.name == var) return offset;
			offset += size(x);
		}
		error(R("未知类成员:") + var);
	}
	if (type == R("Set")) {
		for (auto& x : functionSet.local) {
			if (x.name == var) return offset;
			offset += size(x);
		}
		error(R("未知程序集变量:") + var);
	}
	if (type == R("Global")) {
		for (auto& x : globalVars) {
			if (x.name == var) return offset;
			offset += size(x);
		}
		error(R("未知全局变量:") + var);
	}
	if (type == R("Const")) {
		for (auto& x : functionSet.local) {
			if (x.name == var) return offset;
			offset += size(x);
		}
		error(R("未知常量:") + var);
	}
	else error(R("未知变量类型:") + name);
	return 0;
}
size_t IRGenerator::constSize(const type& A) {
	return getStructureSize(A.typeName);
}
size_t IRGenerator::size(const type& A) {
	return getStructureSize(A.typeName) * (A.array ? DimSize(A.dim) : 1);
}
bool IRGenerator::getVarType(lstring name, lstring& type, lstring& var) {
	std::vector<lstring> t = split(name, DIVISION);
	if (t.size() != 2) {
		error(R("错误的变量名称格式:") + name);
		return false;
	}
	type = t[0];
	var = t[1];
	return true;
}
void IRGenerator::countGlobalSize() {
	size_t size_{};
	for (auto& x : globalVars) size_ += size(x);
	GlobalSize0 = size_;
	for (auto& x : analyzed_functionSets) {
		if (x.isClass) continue;
		size_ += x.size;
	}
	GlobalSize = size_;
}
size_t IRGenerator::getStructureSize(lstring type) {
	if (type == R(""))return 0;
	for (auto& x : structures) {
		if (x.name == type) return x.size;
	}
	error(R("未知数据类型:") + type);
	return 0;
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

	type This{}, ThisArg{};
	This = Type_N;
	This.address = true;
	This.name = R("[this]");
	ThisArg = Type_N;
	ThisArg.address = true;
	ThisArg.name = R("[thisArg]");
	for (auto& x : functionSets) {
		if (!x.isClass) continue;
		This.typeName = x.name;
		for (auto& y : x.func) {
			y.local.insert(y.local.begin(), This);
			if (y.transit) y.local.insert(y.local.begin(), ThisArg);
		}
	}
	for (auto& x : analyzed_functionSets) {
		if (!x.isClass) continue;
		This.typeName = x.name;
		for (auto& y : x.func) {
			y.local.insert(y.local.begin(), This);
			if (y.transit) y.local.insert(y.local.begin(), ThisArg);
		}
	}



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