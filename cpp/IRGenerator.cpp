#include "../header/IRGenerator.h"
using namespace MLang;
inline bool IRGenerator::ins(lstring tk) {
	IR += tk + R("\n"); return true;
}

void IRGenerator::error(lstring err) {
	lstring err_{};
	for (const auto& x : error_lineStack) {
		err_ += R("[行:") + to_lstring(x + 1) + R("]");
	}
	std_lcout << RED << R("[错误]") << CYAN << R("[中间代码生成]") << R("[程序集/类:") << error_functionSet << R("][函数/方法:") << error_function << R("]") << err_ << R("[行:") << error_line + 1 << R("]") << RESET << err << std::endl;
	Error = true;
}
void IRGenerator::warning(lstring warn) {
	if (!enableWarning) return;
	lstring err_{};
	for (const auto& x : error_lineStack) {
		err_ += R("[行:") + to_lstring(x + 1) + R("]");
	}
	std_lcout << YELLOW << R("[警告]") << CYAN << R("[中间代码生成]") << R("[程序集/类:") << error_functionSet << R("][函数/方法:") << error_function << R("]") << err_ << R("[行:") << error_line + 1 << R("]") << RESET << warn << std::endl;
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
	for (const auto& x : var) size_ += size(x);
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
	ins(R("\n\n#label_function_Local") + DIVISION + functionSet.name + DIVISION + func.name + buildFunctionTypeStr(func));
	size_t local_size{};
	for (const auto& x : func.local) {
		size_t size_ = x.name == R("[this]") ? size(Type_N) : size(x);
		ins(R(";") + x.typeName + R(" ") + x.name + (x.array ? R("[]") : R("")) + R(" size:") + to_lstring(size_));
		local_size += size_;
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
	tmpCodeStack.push_back(tmpCode);
	size_t curr = IR.length();
	tmpCode.clear();
	size_t tmp = allocTmpID(Type_N);
	initSetVars(func.local, tmp, 0, R("&"), true);
	IR.insert(curr,tmpCode);
	tmpCode = tmpCodeStack.back();
	tmpCodeStack.pop_back();
	ins(R("tmpEnd"));
	for (size_t i = 0; i < func.codes.size(); i++) {
		error_line = i;
		ins(R("\n;") + functionSet.name + DIVISION + func.name + R(" Line:") + to_lstring(error_line));
		ins(R("tmpBegin"));
		tmpStack.clear();
		generateLine(functionSet, func, func.codes[i]);
		ins(R("tmpEnd"));
	}
	ins(R("#label_function_End_Local") + DIVISION + functionSet.name + DIVISION + func.name + buildFunctionTypeStr(func));
	tmpStack.clear();
	ins(R("tmpBegin"));
	tmpCodeStack.push_back(tmpCode);
	curr = IR.length();
	tmpCode.clear();
	tmp = allocTmpID(Type_N);
	size_t tmp2 = allocTmpID(Type_R);
	ins(R("storeQ %") + to_lstring(tmp2));
	destroySetVars(func.local, tmp, 0, R("&"), true);
	ins(R("loadQ %") + to_lstring(tmp2));
	IR.insert(curr, tmpCode);
	tmpCode = tmpCodeStack.back();
	tmpCodeStack.pop_back();
	ins(R("tmpEnd"));
	if (func.transit) {
		ins(R("loadThisArg &") + to_lstring(getVarOffset(functionSet, func, R("Local") + DIVISION + R("[thisArg]"))));
		ins(R("transit"));
	}
	else
		ins(R("return ") + to_lstring(countArgSize(functionSet, func)));

}
void IRGenerator::generateLine(analyzed_functionSet& functionSet, analyzed_function& func, Tree<node>& EX) {
	need_return.push_back(false);
	return_id.push_back(0);
	tmpCodeStack.push_back(tmpCode);
	destroyCodeStack.push_back(destroyCode);
	initCodeStack.push_back(initCode);
	tmpCode.clear();
	destroyCode.clear();
	initCode.clear();
	size_t curr = IR.length();
	compileTree(functionSet, func, EX, {});
	IR.insert(curr, tmpCode + initCode);
	IR += destroyCode;
	tmpCode = tmpCodeStack.back();
	destroyCode = destroyCodeStack.back();
	initCode = initCodeStack.back();
	tmpCodeStack.pop_back();
	destroyCodeStack.pop_back();
	initCodeStack.pop_back();
	if (need_return.back()) {
		ins(R("loadQ %") + to_lstring(return_id.back()));
		ins(R("jmp #label_function_End_") + getFullName(func.name, functionSet) + buildFunctionTypeStr(func));
	}
	need_return.pop_back();
	return_id.pop_back();
	//for (auto& x : destroyCode) ins(x);
	
}
type IRGenerator::compileTree(analyzed_functionSet& functionSet, analyzed_function& func, Tree<node>& EX, std::optional<lstring> ExtraInfo) {
	//initCodeStack.push_back(initCode);
	//destroyCodeStack.push_back(destroyCode);

	//initCode.clear();
	//destroyCode.clear();

	size_t curr = IR.length();
	type ret{};
	node p=EX.Get();
	lstring tk = p.token, type_ = p.type;
	if (Error) goto RET;
	if (type_ == R("Operator")) {
		if (tk == R("=")) {
			if (!EX.child()) {
				error(R("赋值参数过少"));
				goto RET;
			} 
			type A = compileTree(functionSet, func, EX, {});
			if (!A.address) {
				error(R("赋值左值必须提供指针"));
				goto RET;
			}
			if (!EX.next()) {
				error(R("赋值参数过少"));
				EX.parent();
				goto RET;
			}
			type B = compileTree(functionSet, func, EX, {});
			type C{};
			lstring tk1 = R("Local") + DIVISION + A.typeName + DIVISION + tk;
			if (haveFunction(tk1)) {
				std::vector<type> args{};
				args.push_back(B);
				analyzed_function* func0 = getFunction(functionSet, tk1, args, {});
				if (func0 && args.size() == 1 && func0->args.size() == 1) {
					buildThisCall(functionSet, func, EX, tk1, *func0, A, args, ret);
					EX.parent();
					goto RET;
				}
			}
			C = A;
			ret = A;
			if (A.address == B.address && A.typeName == B.typeName && (A.array && B.array && cmpDim(A.dim, B.dim) || !A.array && !B.array)) {
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
			goto RET;
		}
		else if (cmpTK(tk, { R("+"),R("-"),R("*"),R("/"),R("%"),R("\\"),R(">"),R("<"),R(">="),R("<="),R("!="),R("==") })) {
			if (!EX.child()) {
				error(R("二元运算参数过少")); goto RET;
			}
			type A = compileTree(functionSet, func, EX, {});
			if (!EX.next()) {
				error(R("二元运算参数过少")); EX.parent(); goto RET;
			}
			type B = compileTree(functionSet, func, EX, {});
			type C{};
			lstring tk1 = R("Local") + DIVISION + A.typeName + DIVISION + tk;
			if (haveFunction(tk1)) {
				std::vector<type> args{};
				args.push_back(B);
				analyzed_function* func0 = getFunction(functionSet, tk1, args, {});
				if (func0 && args.size() == 1 && func0->args.size() == 1) {
					buildThisCall(functionSet, func, EX, tk1, *func0, A, args, ret);
					EX.parent();
					goto RET;
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
			goto RET;
		}
		else if (cmpTK(tk, { R("and"),R("or"),R("xor") })) {
			if (!EX.child()) {
				error(R("二元运算参数过少")); goto RET;
			}
			type A = compileTree(functionSet, func, EX, {});
			if (!EX.next()) {
				error(R("二元运算参数过少"));
				EX.parent();
				goto RET;
			}
			type B = compileTree(functionSet, func, EX, {});
			type C{};
			lstring tk1 = R("Local") + DIVISION + A.typeName + DIVISION + tk;
			if (haveFunction(tk1)) {
				std::vector<type> args{};
				args.push_back(B);
				analyzed_function* func0 = getFunction(functionSet, tk1, args, {});
				if (func0 && args.size() == 1 && func0->args.size() == 1) {
					buildThisCall(functionSet, func, EX, tk1, *func0, A, args, ret);
					EX.parent();
					goto RET;
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
			goto RET;
		}
		else if (cmpTK(tk, { R("Minus"),R("Abs"),R("not") })) {
			if (!EX.child()) {
				error(R("一元运算参数过少")); goto RET;
			}
			type A = compileTree(functionSet, func, EX, {});
			lstring tk1=R("Local") + DIVISION + A.typeName + DIVISION + tk;
			if (haveFunction(tk1)) {
				std::vector<type> args{};
				analyzed_function* func0 = getFunction(functionSet, tk1, {}, {});
				if (func0 && args.size() == 0 && func0->args.size() == 0) {
					buildThisCall(functionSet, func, EX, tk1, *func0, A, args, ret);
					EX.parent();
					goto RET;
				}
			}
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
			goto RET;
		}
		else if (tk == R("&")) {
			if (!EX.child()) {
				error(R("一元运算参数过少")); goto RET;
			}
			type A = compileTree(functionSet, func, EX, {});
			if (!A.address) {
				error(R("不能取得非指针类型的地址")); goto RET;
			}
			ret = A;
			ret.typeName = R("N");
			ret.address = false;
			ret.array = false;
			EX.parent();
			return ret;
		}
		else {
			error(R("未知运算符")); goto RET;
		}
	}
	else if (type_==R("Var")) {
		lstring v_type, v_name;
		if (!getVarType(tk, v_type, v_name)) goto RET;
		type A{};
		if (v_type == R("Local")) {
			A = getLocalType(v_name, func);
			A.id = allocTmpID(Type_N);
			ins(R("address %") + to_lstring(A.id) + R(" &") + to_lstring(getVarOffset(functionSet, func, tk)));
			if (v_name == R("[this]")) {
				ins(R("load %") + to_lstring(A.id) + R(" %") + to_lstring(A.id) + R(" ") + to_lstring(getStructureSize(R("N"))));
			}
			A.address = true;
			ret = A;
			goto RET;
		}
		if (v_type == R("Arg")) {
			A = getArgType(functionSet, v_name, func);
			A.id = allocTmpID(Type_N);
			ins(R("address %") + to_lstring(A.id) + R(" !") + to_lstring(getVarOffset(functionSet, func, tk)));
			if (!ifNotRef(A) || v_name == R("[ret]")) {
				ins(R("load %") + to_lstring(A.id) + R(" %") + to_lstring(A.id) + R(" ") + to_lstring(getStructureSize(R("N"))));
			}
			A.address = true;
			ret = A;
			goto RET;
		}
		if (v_type == R("Set")) {
			A = getSetVarType(v_name, functionSet);
			A.id = allocTmpID(Type_N);
			ins(R("address %") + to_lstring(A.id) + R(" $") + to_lstring(GlobalOffset + getVarOffset(functionSet, func, tk)));
			A.address = true;
			ret = A;
			goto RET;
		}
		if (v_type == R("Global")) {
			A = getGlobalVarType(v_name);
			A.id = allocTmpID(Type_N);
			ins(R("address %") + to_lstring(A.id) + R(" $") + to_lstring(getVarOffset(functionSet, func, tk)));
			A.address = true;
			ret = A;
			goto RET;
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
			ret = A;
			goto RET;
		}
		if (v_type == R("Class")) {
			A = getSetVarType(v_name, functionSet);
			A.id = allocTmpID(Type_N);
			ins(R("address %") + to_lstring(A.id) + R(" &") + to_lstring(getVarOffset(functionSet, func, R("Local") + DIVISION + R("[this]"))));
			ins(R("load %") + to_lstring(A.id) + R(" %") + to_lstring(A.id) + R(" ") + to_lstring(getStructureSize(R("N"))));
			ins(R("offset %") + to_lstring(A.id) + R(" ") + to_lstring(getElementOffset(functionSet, functionSet.name, v_name)));
			A.address = true;
			ret = A;
			goto RET;
		}
		error(R("未知变量:") + tk);
		goto RET;
	}
	else if (type_ == R("Call")) {
		if (handleBuiltInFunctions(functionSet, func, EX, tk, ret)) goto RET;
		lstring arg_T{};
		std::vector<type> args{};
		analyzed_function* func0;
 		if(EX.child()){
			do {
				args.push_back(compileTree(functionSet, func, EX, {}));
			} while (EX.next());

			type A{};
			if (ifMethod(tk)) {
				analyzed_function* func0 = getFunction(functionSet, tk, args, true);
				if (func0) {
					size_t id1 = allocTmpID(Type_N);
					ins(R("address %") + to_lstring(id1) + R(" &") + to_lstring(getVarOffset(functionSet, func, R("Local") + DIVISION + R("[this]"))));
					ins(R("load %") + to_lstring(id1) + R(" %") + to_lstring(id1) + R(" ") + to_lstring(getStructureSize(R("N"))));
					A.typeName = functionSet.name;
					A.address = true;
					A.array = false;
					A.id = id1;

					buildThisCall(functionSet, func, EX, tk, *func0, A, args, ret);
					EX.parent();
					goto RET;
				}
			}

			lstring tk1 = R("Local") + DIVISION + tk + DIVISION + R("_new_");

			if (haveFunction(tk1)) {
				analyzed_function* func0 = getFunction(functionSet, tk1, args, {});
				if (func0 && buildThisCall(functionSet, func, EX, tk1, *func0, A, args, ret)) {
					EX.parent();
					goto RET;
				}
			}

			func0 = getFunction(functionSet, tk, args, {});
			if (!func0) {
				error(R("找不到可以匹配的函数/方法"));
				goto RET;
			}
			size_t i = 0;
			for (; i < func0->args.size(); i++) {
				if (i >= args.size()) {
					error(R("函数调用参数过少"));
					goto RET;
				}
				generateImplictConversion(func0->args[i], args[i], functionSet, func, EX);
				arg_T += R(" %") + to_lstring(func0->args[i].id);
			}
			if (func0->use_arg_size) {
				while (i < args.size())
				{
					type tmp = func0->args[func0->args.size() - 1];
					generateImplictConversion(tmp, args[i], functionSet, func, EX);
					arg_T += R(" %") + to_lstring(tmp.id);
					i++;	
				}
			}
			EX.parent();
		}
		else {
			func0 = getFunction(functionSet, tk, args, {});
			if (!func0) {
				error(R("找不到可以匹配的函数/方法"));
				goto RET;
			}
		}
		ret = func0->ret;
		ret.id = allocTmpID(func0->ret);
		lstring tk2 = func0->call_type == R("cdecl") ? R("_cdecl") : R("");
		size_t argSize_ID{};lstring argSize_T{};
		if (func0->use_arg_size) {
			argSize_ID = allocTmpID(Type_N);
			ins(R("num %") + to_lstring(argSize_ID) + R(" ?I") + to_lstring(args.size()));
			argSize_T = R(" %") + to_lstring(argSize_ID);
		}
		if (ifNotRef(ret)) {
			ins(R("Call") + tk2 + R(" #label_function_") + tk + buildFunctionTypeStr(*func0) + R(" null %") + to_lstring(ret.id) + argSize_T + arg_T);
		}
		else
		{
			ins(R("CallA") + tk2 + R(" #label_function_") + tk + buildFunctionTypeStr(*func0) + R(" null &%") + to_lstring(ret.id) + argSize_T + arg_T);
			size_t id1 = allocTmpID(Type_N);
			ins(R("address %") + to_lstring(id1) + R(" &%") + to_lstring(ret.id));
			ret.id = id1;
			ret.address = true;
		}
		goto RET;
	}
	else if (type_ == R("thisCall")) {
		std::vector<type> args{};
		analyzed_function* func0{}; lstring tk1{};
		type A{};
		if (EX.child()) {
			A = compileTree(functionSet, func, EX, {});
			while (EX.next()) {
				args.push_back(compileTree(functionSet, func, EX, {}));
			} 
			tk1 = R("Local") + DIVISION + A.typeName + DIVISION + tk;
			func0 = getFunction(functionSet, tk1, args, {});
			if (!func0) {
				error(R("找不到可以匹配的方法"));
				goto RET;
			}
			buildThisCall(functionSet, func, EX, tk1, *func0, A, args, ret);
			EX.parent();
		}
		else {
			error(R("方法引用必须提供被引用结构"));
		}
		goto RET;
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
		ret = A;
		goto RET;
	}
	else if (type_ == R("typeof")) {
		if (!EX.child()) {
			error(R("typeof必须作用于表达式"));
			goto RET;
		}
		type A = compileTree(functionSet, func, EX, {});
		std::vector<type> args{};
		while (EX.next()) {
			args.push_back(compileTree(functionSet, func, EX, {}));
		}
		EX.parent();
		lstring tk1= R("Local") + DIVISION + A.typeName + DIVISION + R("typeof()");
		if (haveFunction(tk1)) {
			//thiscall
			analyzed_function* func0 = getFunction(functionSet, tk1, args, true);
			if (func0) {
				buildThisCall(functionSet, func, EX, tk1, *func0, A, args, ret);
				goto RET;
			}
		}
		lstring tk = A.typeName + (A.array ? R("[]") : R(""));
		ret.typeName = R("B");
		tk = process_quotation_mark(tk);
		ret.dim.push_back(tk.size());
		ret.id = allocTmpID(Type_Boolen);
		ret.array = true;
		ret.address = true;
		ins(R("address %") + to_lstring(ret.id) + R(" ?T") + to_lstring(allocStr(tk)));
		goto RET;
	}
	else if (type_ == R("sizeof")) {
		if (!EX.child()) {
			error(R("sizeof必须作用于表达式"));
			goto RET;
		}
		type A = compileTree(functionSet, func, EX, {});
		lstring tk1 = R("Local") + DIVISION + A.typeName + DIVISION + R("sizeof()");
		if (haveFunction(tk1)) {
			std::vector<type> args{};
			while (EX.next()) {
				args.push_back(compileTree(functionSet, func, EX, {}));
			}
			analyzed_function* func0 = getFunction(functionSet, tk1, args, {});
			if (func0) {
				buildThisCall(functionSet, func, EX, tk1, *func0, A, args, ret);
				EX.parent();
				goto RET;
			}
		}
		EX.parent();
		ret.typeName = R("N");
		ret.id = allocTmpID(Type_N);
		ret.address = false;
		ret.array = false;
		ins(R("num %") + to_lstring(ret.id) + R(" ?I") + to_lstring(size(A)));
		goto RET;
	}
	else if (type_ == R("Int")) {
		type A{};
		A.typeName = R("Z");
		A.address = false;
		A.array = false;
		A.id = allocTmpID(Type_Z);
		ins(R("num %") + to_lstring(A.id) + R(" ?I") + tk);
		ret = A;
		goto RET;
	}
	else if (type_ == R("Double")) {
		type A{};
		A.typeName = R("R");
		A.address = false;
		A.array = false;
		A.id = allocTmpID(Type_R);
		ins(R("num %") + to_lstring(A.id) + R(" ?D") + tk);
		ret = A;
		goto RET;
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
			args.push_back(compileTree(functionSet, func, EX, {}));
		}
		lstring tk1 = R("Local") + DIVISION + C.typeName + DIVISION + R("[]");
		lstring arg_T{};
		analyzed_function* func0{};
		if (haveFunction(tk1) && (func0 = getFunction(functionSet, tk1, args, true))) {
			for (auto& x : args) {
				type Ntype{}; Ntype.typeName = R("N");
				generateImplictConversion(Ntype, x, functionSet, func, EX);
				arg_T += R(" %") + to_lstring(Ntype.id);
			}
			ret = func0->ret;
			ret.id = allocTmpID(Type_N);
			type B{};
			B.typeName = R("N");
			B.id = allocTmpID(Type_N);
			ins(R("num %") + to_lstring(B.id) + R(" ?uI") + to_lstring(args.size()));
			arg_T = R(" %") + to_lstring(B.id) + arg_T;
			if (func0->call_type != R("cdecl")) {
				error(R("不支持的调用方式:") + func0->call_type);
			}
			else if (!func0->use_arg_size) {
				error(R("未声明传入参数个数"));
			}
			if (ifNotRef(ret)) {
				ins(R("Call_cdecl #label_function_") + tk1 + buildFunctionTypeStr(*func0) + R(" %") + to_lstring(C.id) + R(" %") + to_lstring(ret.id) + arg_T);
			}
			else {
				ins(R("CallA_cdecl #label_function_") + tk1 + buildFunctionTypeStr(*func0) + R(" %") + to_lstring(C.id) + R(" &%") + to_lstring(ret.id) + arg_T);
			}
		}
		else {
			if (!ret0.array) {
				error(R("下标的作用对象必须为数组"));
				goto RET;
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
				for (intptr_t i = args.size() - 2; i >= 0; i--) {
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
		goto RET;
	}
	else if (type_ == R("ad_v")) {
		if (!EX.child()) {
			error(R("错误的指针用法")); goto RET;
		}
		type A = compileTree(functionSet, func, EX, {});
		lstring tmp = process_quotation_mark(tk);
		lstring tk1 = R("Local") + DIVISION + A.typeName + DIVISION + R("->") + tmp;
		analyzed_function* func0{};
		std::vector<type> args{};
		if (haveFunction(tk1) && (func0 = getFunction(functionSet, tk1, args, {}))) {
			if (func0->args.size() != 1) {
				error(R("错误的指针用法"));
				goto RET;
			}
			buildThisCall(functionSet, func, EX, tk1, *func0, A, args, ret);
			EX.parent();
			goto RET;
		}
		else {
			type C = Type_N;
			generateImplictConversion(C, A, functionSet, func, EX);
			C.address = true;
			C.typeName = tmp;
			ret = C;
			EX.parent();
			goto RET;
		}
	}
	else if (type_ == R("Block")) {
		if (EX.child()) {
			error_lineStack.push_back(error_line);
			error_line = 0;
			if(ExtraInfo.has_value())
				do
				{
					generateLine(functionSet, func, EX);
					error_line++;
				} while (EX.next());
			else
				do
				{
					generateLine(functionSet, func, EX);
					error_line++;
				} while (EX.next());
			error_line = error_lineStack.back();
			error_lineStack.pop_back();
			EX.parent();
		}
		goto RET;
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
		goto RET;
	}
	else if (type_==R("FunctionAddress")) {
		ret.typeName = R("N");
		ret.array = false;
		ret.address = false;
		ret.id = allocTmpID(ret);
		ins(R("ExactlyAddress %") + to_lstring(ret.id) + R(" #label_function_") + getFullName(tk,functionSet));
		goto RET;
	}else if(type_==R("ExactlyAddress")) {
		ret.typeName = R("N");
		ret.array = false;
		ret.address = false;
		ret.id = allocTmpID(ret);
		ins(R("ExactlyAddress %") + to_lstring(ret.id));
		goto RET;
	}
	else if (type_ == R("")) return ret;
	else
	{
		error(R("不支持的结构:Type = ") + type_ + R(" Token = ") + tk);
		ret.can_be_ignored = false;
		goto RET;
	}

	RET:
	//IR.insert(curr, initCode);
	//IR += destroyCode;
	//initCode = initCodeStack.back();
	//destroyCode = destroyCodeStack.back();
	//initCodeStack.pop_back();
	//destroyCodeStack.pop_back();
	return ret;
}
bool IRGenerator::ifMethod(lstring FullName) {
	lstring type, className, name;
	if (!getFunctionType(FullName, type, className, name)) {
		error(R("错误的函数名称格式:") + FullName);
		return false;
	}
	if (type == R("Local")) {
		for (const auto& x : analyzed_functionSets) {
			if (x.name == className) return x.isClass;
		}
		error(R("未知函数:") + FullName);
	}
	return false;
}
size_t IRGenerator::allocStr(lstring text) {
	for (size_t i = 0; i < strings.size(); i++) {
		if (strings[i] == text) return i + 1;
	}
	strings.push_back(text);
	ins(R("[string] ") + to_lstring(strings.size()) + R(" ?T") + base64_encode(text));
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
	for (const auto& x : structures) {
		if (x.name == struct_) {
			for (const auto& y : x.elements) {
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
	if (local.name == R("[this]")) return;
	intptr_t localOffset{};
	for (auto& x : struct0.elements) {
		size_t localSize = getStructureSize(x.typeName);
		size_t dim = x.array ? DimSize(x.dim) : 1;
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
		ins(R("Call #label_function_Local") + DIVISION + class_->name + DIVISION + R("_init_ %") + to_lstring(tmp) + R(" null"));
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
	if (local.name == R("[this]")) return;
	intptr_t localOffset{};
	for (auto& x : struct0.elements) {
		size_t localSize = getStructureSize(x.typeName);
		size_t dim = x.array ? DimSize(x.dim) : 1;
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
		ins(R("Call #label_function_Local") + DIVISION + class_->name + DIVISION + R("_destroy_ %") + to_lstring(tmp) + R(" null"));
	}
}
void IRGenerator::initSetVars(std::vector<type>& vars, size_t tmp, size_t offset, lstring tk, bool localMode) {
	for (auto& x : vars) {
		size_t size = getStructureSize(x.name == R("[this]") ? R("N") : x.typeName);
		size_t dim = x.name != R("[this]") && x.array ? DimSize(x.dim) : 1;
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
		size_t size = getStructureSize(x.name == R("[this]") ? R("N") : x.typeName);
		size_t dim = x.name != R("[this]") && x.array ? DimSize(x.dim) : 1;
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
			id1 = A.id;
			if (A.address) {
				id1 = allocTmpID(Type_Boolen);
				ins(R("load %") + to_lstring(id1) + R(" %") + to_lstring(A.id) + R(" ") + to_lstring(getStructureSize(R("Boolen"))));
				A.id = id1;
			}
			ins(R("jz %") + to_lstring(id1) + R(" #label_if_B_") + to_lstring(label1));
			//ins(R("#label_if_A_") + to_lstring(label1));
			if (EX.next()) {
				generateLine(functionSet, func, EX);
				ins(R("jmp #label_if_End_") + to_lstring(label1));
			}
			ins(R("#label_if_B_") + to_lstring(label1));
			if (EX.next()) {
				generateLine(functionSet, func, EX);
				//ins(R("jmp #label_if_End_") + to_lstring(label1));
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
				generateLine(functionSet, func, EX);
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
			generateLine(functionSet, func, EX);
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
				return_id.back() = allocTmpID(Type_R);
				ins(R("storeQ %") + to_lstring(return_id.back()));
				need_return.back() = true;

				//ins(R("jmp #label_function_End_") + getFullName(func.name, functionSet) + buildFunctionTypeStr(func));
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
				lstring tk1 = R("Local") + DIVISION + func_ret.typeName + DIVISION + R("return(") + A.typeName + R(")");
				std::vector<type> args{};
				args.push_back(A);
				analyzed_function* func0{};
				if (haveFunction(tk1) && (func0 = getFunction(functionSet, tk1, args, {}))) {
					if (func0->args.size() == 1) {
						size_t id1 = allocTmpID(func0->ret);
						type C = func0->args[0];
						generateImplictConversion(C, A, functionSet, func, EX);
						ret = func0->ret;
						ret.id = id1;
						size_t id2 = allocTmpID(Type_N);
						ins(R("address %") + to_lstring(id2) + R(" !") + to_lstring(getVarOffset(functionSet, func, R("Arg") + DIVISION + R("[ret]"))));
						ins(R("load %") + to_lstring(id2) + R(" %") + to_lstring(id2) + R(" ") + to_lstring(getStructureSize(R("N"))));
						size_t ArgSize_ID{};
						lstring ArgSize_T{};
						if (func0->use_arg_size) {
							ArgSize_ID = allocTmpID(Type_N);
							ins(R("num %") + to_lstring(ArgSize_ID) + R(" ?I") + to_lstring(args.size()));
							ArgSize_T = R(" %") + to_lstring(ArgSize_ID);
						}
						if (ifNotRef(func0->ret)) {
							ins(R("Call #label_function_") + tk1 + buildFunctionTypeStr(*func0) + R(" %") + to_lstring(id2) + R(" %") + to_lstring(id1) + ArgSize_T  + R(" %") + to_lstring(C.id));
						}
						else {
							ins(R("CallA #label_function_") + tk1 + buildFunctionTypeStr(*func0) + R(" %") + to_lstring(id2) + R(" &%") + to_lstring(id1) + ArgSize_T  + R(" %") + to_lstring(C.id));
							id2 = allocTmpID(Type_N);
							ins(R("address %") + to_lstring(id2) + R(" &%") + to_lstring(id1));
							ret.id = id2;
							ret.address = true;
						}
						return_id.back() = allocTmpID(Type_R);
						ins(R("storeQ %") + to_lstring(return_id.back()));
						need_return.back() = true;
//						ins(R("jmp #label_function_End_") + getFullName(func.name, functionSet) + buildFunctionTypeStr(func));
						EX.parent();
						return true;
					}

				}
				size_t id1 = allocTmpID(func_ret);
				ins(R("address %") + to_lstring(id1) + R(" !") + to_lstring(getVarOffset(functionSet, func, R("Arg") + DIVISION + R("[ret]"))));
				ins(R("load %") + to_lstring(id1) + R(" %") + to_lstring(id1) + R(" ") + to_lstring(getStructureSize(R("N"))));
				ins(R("mov %") + to_lstring(id1) + R(" %") + to_lstring(A.id) + R(" ") + to_lstring(size(A)));
				return_id.back() = allocTmpID(Type_R);
				ins(R("storeQ %") + to_lstring(return_id.back()));
				need_return.back() = true;
				//ins(R("jmp #label_function_End_") + getFullName(func.name, functionSet));
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
		return true;
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
	lstring t = R("Local") + DIVISION + B.typeName + DIVISION + A.typeName + R("()");
	DebugOutput(t);
	size_t id;
	if (haveFunction(t)) {
		std::vector<type> args{}, args2{};
		analyzed_function* func0 = getFunction(functionSet, t, args, {});
		if (func0 && func0->args.size() == 0) {
			buildThisCall(functionSet, func, EX, t, *func0, B, args, A);
			return true;
		}
	}
	if ((!ifBaseType(A) || A.address == B.address) 
		&& A.typeName == B.typeName 
		&& (A.array && B.array && cmpDim(A.dim, B.dim)
		|| !A.array && !B.array)) {
		A.id = B.id;
		return true;
	}
	if (A.array && B.array && A.typeName == B.typeName) {
		warning(R("尝试对类型相同但性质不同的数组进行强制转换"));
		A.id = B.id;
		return true;
	}
	if (A.array && !B.array && B.typeName==R("N")) {
		warning(R("尝试将无符号整数/地址转换成数组"));
		A.id = B.id;
		return true;
	}
	if (!A.array && B.array && A.typeName == R("N")) {
		warning(R("尝试将数组转换成无符号整数/地址"));
		A.id = B.id;
		return true;
	}
	if (A.array || B.array) {
		error(R("非法强制转换:数组"));
		return false;
	}
	if (B.address && ifBaseType(B)) {
		if (size(Type_N) == size(B))
			id = B.id;
		else 
			id = allocTmpID(B);
		ins(R("load %") + to_lstring(id) + R(" %") + to_lstring(B.id) + R(" ") + to_lstring(getStructureSize(B.typeName)));
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
			ins(R("R2Z %") + to_lstring(id) + R(" %") + to_lstring(B.id));
			ins(R("Z2N %") + to_lstring(A.id) + R(" %") + to_lstring(id));
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
		error(R("不支持的强制转换类型:") + (B.typeName == R("") ? R("[Unknown]") : B.typeName) + R("->") + A.typeName);
		return false;
	}
	return true;
}
bool IRGenerator::haveFunction(lstring name) {
	for (const auto& x : analyzed_functionSets) {
		for (const auto& y : x.func) {
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
	tmpCode += R("tmp %") + to_lstring(id) + R(" ") + to_lstring(size_) + R("\n");
	analyzed_functionSet* class_{};
	for (auto& x : analyzed_functionSets) {
		if (x.name == A.typeName) class_ = &x;
	}
	if (class_ && setHasFunction(*class_, R("_init_"))) {
		initCode += (R("Call #label_function_Local") + DIVISION + A.typeName + DIVISION + R("_init_ &%") + to_lstring(id) + R(" null")) + R("\n");
	}
	if (class_ && setHasFunction(*class_, R("_destroy_"))) {
		destroyCode += (R("Call #label_function_Local") + DIVISION + A.typeName + DIVISION + R("_destroy_ &%") + to_lstring(id) + R(" null")) + R("\n");
	}
	return id;
}
size_t IRGenerator::tmpOffset(size_t id, const std::vector<size_t>& stack) {
	if (stack.size() <= id) {
		error(R("临时变量ID溢出"));
		return 0;
	}
	size_t offset{};
	for (const auto& x : stack) {
		offset += x;
	}
	return offset;
}
bool IRGenerator::setHasFunction(const analyzed_functionSet& functionSet, lstring name) {
	for (const auto& x : functionSet.func) {
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
		return 0;
	}
	if (A.typeName == R("Z")) {
		return 2;
	}
	if (A.typeName == R("R")) {
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
		t.address = true;
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
analyzed_function* IRGenerator::getFunction(const analyzed_functionSet& functionSet, lstring fullName, const std::optional<std::vector<type>>& args, std::optional<bool> variable) {
	lstring type{}, className{}, name{};
	analyzed_function tfunc{};
	if (!getFunctionType(fullName, type, className, name)) {
		error(R("错误的函数名称格式:") + fullName);
		return nullptr;
	}
	if (type == R("Local")) {
		lstring err{};
		for (auto& x : analyzed_functionSets) {
			analyzed_function* k{};
			bool a{};
			for (auto& y : x.func) {
				if (y.name == name) {
					k = &y;
					a =x.isClass && (y.publiced && x.name != functionSet.name || x.name == functionSet.name);
					if (a && cmpArg(y.args, args)) return &y;
				}
			}
			if (k) {
				if (x.isClass && !a) {
					err = R("试图调用 ") + x.name + R(" 中未公开的方法:") + k->name;
					continue;
				}
				if (!cmpArgNum(k->args, args) && !variable && !k->use_arg_size) {
					err = R("参数过多或过少:") + fullName;
					continue;
				}
				return k;
			}
		}
		error(err == R("") ? R("未知函数:") + fullName : err);
	}
	else if (type == R("Extra")) {
		for (auto& x : ExtraFunctions.func) {
			if (x.extra_name == name) {
				return &x;
			}
		}
		error(R("未知外部函数:") + fullName);
	}
	else {
		error(R("未知函数:") + fullName);
	}
	return nullptr;
}
bool IRGenerator::cmpArgNum(const std::vector<type>& A, const std::optional<std::vector<type>>& B) {
	size_t j{}, k{};
	std::vector<type> C{};
	if (B.has_value())
		C = B.value();
	for (intptr_t i = C.size() - 1; i >= 0; i--) {
		if (!C[i].can_be_ignored) break;
		j++;
	}
	for (intptr_t i = A.size() - 1; i >= 0; i--) {
		if (!A[i].can_be_ignored) break;
		k++;
	}
	return A.size() == C.size() - j || A.size() - k == C.size() - j;
}
bool IRGenerator::cmpArg(const std::vector<type>& A, const std::optional<std::vector<type>>& B) {
	std::vector<type> C{};
	if (B.has_value()) 
		C = B.value();
	if (A.size() != C.size()) return false;
	for (size_t i = 0; i < A.size(); i++) {
		if (A[i].typeName != C[i].typeName || A[i].array != C[i].array) return false;
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
lstring IRGenerator::getFullName(const lstring& name, const analyzed_functionSet& functionSet) {
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
		//if (functionSet.isClass) offset += getStructureSize(R("N"));
		//if (var == R("[this]")) return offset;
		for (auto& x : func.local) {
			if (x.name == R("[this]")) 
				offset += getStructureSize(R("N"));
			else
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
			offset += argSize(x);
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
	ExtraFunctions.func.clear();
	for (auto& x : ExtraFunctions_.func) {
		ExtraFunctions.func.push_back(toAnalyzedFunction(x));
	}
	constants = constants_;
	Error = false;
	strings.clear();

	Type_N.typeName = R("N");
	Type_N.array = false;
	Type_N.address = false;
	Type_R.typeName = R("R");
	Type_R.array = false;
	Type_R.address = false;
	Type_Z.typeName = R("Z");
	Type_Z.array = false;
	Type_Z.address = false;
	Type_B.typeName = R("B");
	Type_B.array = false;
	Type_B.address = false;
	Type_Boolen.typeName = R("Boolen");
	Type_Boolen.array = false;
	Type_Boolen.address = false;

	initCodeStack.clear();
	destroyCodeStack.clear();
	tmpStack.clear();
	initCode.clear();
	destroyCode.clear();
	tmpCode.clear();
	error_lineStack.clear();

	type This{}, ThisArg{};
	This = Type_N;
	This.address = true;
	This.name = R("[this]");
	ThisArg = Type_N;
	ThisArg.address = false;
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
	ins(R("[GlobalSize] ") + to_lstring(GlobalSize));
	ins(R(";Entry"));
	ins(R("enter"));
	ins(R("tmpBegin"));
	tmpCodeStack.push_back(tmpCode);
	tmpCode.clear();
	size_t curr = IR.length();
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
	ins(R("Call #label_function_Local$Main$main null null"));
	IR.insert(curr, tmpCode);
	tmpCode = tmpCodeStack.back();
	tmpCodeStack.pop_back();
	//ins(R("loadQ %") + to_lstring(tmp2));
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
				ins(R("[System] #label_function_Local") + DIVISION + R("[System]") + DIVISION + y.name + buildFunctionTypeStr(y));
			}
			continue;
		}
		error_functionSet = x.name;
		generateFunctionSet(x);
		if (!x.isClass) GlobalOffset += x.size;
	}
	for (auto& x : ExtraFunctions.func) {
		ins(R("#label_function_Extra") + DIVISION + x.DLL + DIVISION + x.extra_name + buildFunctionTypeStr(x));
		ins(R("[API] ") + base64_encode(x.DLL) + R(" ") + base64_encode(x.extra_name));
	}
	return Error;
}
bool IRGenerator::buildThisCall(analyzed_functionSet& functionSet, analyzed_function& func, Tree<node>& EX,const lstring& fullname, analyzed_function& callfunc
	, type& object, std::vector<type>& args, type& ret) {
	lstring arg_T{};
	if (object.array) {
		error(R("方法的被引用结构不能为数组"));
		return false;
	}
	if (!object.address) {
		error(R("方法的被引用结构必须为指针"));
		return false;
	}
	size_t i = 0;
	for (; i < callfunc.args.size(); i++) {
		if (i >= args.size()) {
			error(R("方法引用参数过少"));
			return false;
		}
		generateImplictConversion(callfunc.args[i], args[i], functionSet, func, EX);
		arg_T += R(" %") + to_lstring(callfunc.args[i].id);
	}
	if (callfunc.use_arg_size) {
		while (i < args.size())
		{
			type tmp = callfunc.args[callfunc.args.size() - 1];
			generateImplictConversion(tmp, args[i], functionSet, func, EX);
			arg_T += R(" %") + to_lstring(tmp.id);
			i++;
		}
	}

	ret = callfunc.ret;
	ret.id = allocTmpID(callfunc.ret);
	lstring tk2 = callfunc.call_type == R("cdecl") ? R("_cdecl") : R("");
	size_t argSize_ID{}; lstring argSize_T{};
	if (callfunc.use_arg_size) {
		argSize_ID = allocTmpID(Type_N);
		ins(R("num %") + to_lstring(argSize_ID) + R(" ?I") + to_lstring(args.size()));
		argSize_T = R(" %") + to_lstring(argSize_ID);
	}
	if (ifNotRef(ret)) {
		ins(R("Call") + tk2 + R(" #label_function_") + fullname + buildFunctionTypeStr(callfunc) + R(" %") + to_lstring(object.id) + R(" %") + to_lstring(ret.id) + argSize_T + arg_T);
	}
	else {
		ins(R("CallA") + tk2 + R(" #label_function_") + fullname + buildFunctionTypeStr(callfunc) + R(" %") + to_lstring(object.id) + R(" &%") + to_lstring(ret.id) + argSize_T + arg_T);
		size_t id1 = allocTmpID(Type_N);
		ins(R("address %") + to_lstring(id1) + R(" &%") + to_lstring(ret.id));
		ret.id = id1;
		ret.address = true;
	}
	return true;
}
lstring IRGenerator::buildFunctionTypeStr(const analyzed_function& func) {
	if (!func.polymorphic) return R("");
	lstring str{};
	for (const auto& x : func.args) {
		str += R("_") + base64_encode(x.typeName + (x.array ? R("[]") : R("")));
	}
	return str;
}