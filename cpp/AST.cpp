#include "../header/AST.h"
using namespace MLang;
static OP op[7];
const int OP_size = 7;
lstring root(size_t num) {
	lstring t{};
	for (size_t i = 0; i < num; i++) t += R("  ");
	return t + R("[]--");
}
void outputNodes(Tree<node>& EX, size_t c) {
	node p = EX.Get(); lstring t{}, l{};
	std_lcout << c << R(" ") << root(c) << R("Type=") << p.type << R("  Token=") << p.token << std::endl;
	if (!EX.child()) return;
	outputNodes(EX, c + 1);
	while (EX.next())
	{
		outputNodes(EX, c + 1);
	}
	EX.parent();
}
void cutTK(std::vector<lstring> tk, std::vector<lstring> op1, std::vector<lstring>& tk1, std::vector<lstring>& tk2, lstring& op2) {
	std::vector<lstring> t1 = tk, t2 = tk;
	intptr_t bracket{};
	bool a{};
	size_t k=0;
	for (size_t i = 1; i <= tk.size(); i++) {
		k++;
		if (tk[tk.size() - i] == R("(") || tk[tk.size() - i] == R("[") || tk[tk.size() - i] == R("{")) {
			bracket++; continue;
		}
		if (tk[tk.size() - i] == R(")") || tk[tk.size() - i] == R("]") || tk[tk.size() - i] == R("}")) {
			bracket--; continue;
		}
		if (bracket != 0) continue;
		for (size_t j = 1; j <= op1.size(); j++) {
			if (tk[tk.size() - i] == op1[j - 1]) {
				if (i == tk.size()) break;
				if (IsOperator(tk[tk.size() - i - 1], 1)) break;
				a = true;
				op2 = op1[j - 1];
				break;
			}
		}
		if (a) break;
	}
	if (!a) {
		tk2 = tk;
		return;
	}
	k = tk.size() - k;
	t1.erase(t1.begin() + k,t1.end());
	t2.erase(t2.begin(), t2.begin() + k + 1);
	tk1 = t1;
	tk2 = t2;
}
size_t AST::countStructureSize(lstring type) {
	if (type == R("")) return 0;
	size_t size{};
	for (size_t i = 0; i < structures.size(); i++) {
		if (structures[i].name == type) {
			if (structures[i].size > 0) return structures[i].size;
			if (list[i]) { error(R("发生循环定义")); return 0; }
			list[i] = true;
			for (size_t j = 0; j < structures[i].elements.size(); j++)
				size += (structures[i].elements[j].array ? DimSize(structures[i].elements[j].dim) : 1) * countStructureSize(structures[i].elements[j].typeName);
			structures[i].size = size;
			return size;
		}
	}
	error(R("未知数据类型:") + type);
	return 0;
}

void AST::analyzeStructureSize() {
	for (size_t i = 0; i < structures.size(); i++) {
		error_functionSet = structures[i].name;
		list.resize(structures.size(), 0);
		countStructureSize(structures[i].name);
	}

}
analyzed_functionSet AST::analyzeFunctionSet(functionSet& functionSet_) {
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
analyzed_function AST::analyzeFunction(functionSet& functionSet_, function& func) {
	analyzed_function func0{};
	func0.codes.clear();
	size_t i{};
	//std_lcout << std::endl << "[Function]" << func.name << std::endl ;
	while (i>=0 && i< func.codes.size())
	{
		error_line = i;
		error_function = func.name;
		error_functionSet = functionSet_.name;
		std::vector<lstring> tk = func.codes[i].tokens;
		Tree<node> EX;
		EX.clear();
		if (!tk.size()) { i++; continue; }
		analyzeExper(functionSet_, func, EX, tk);
		
		//print AST
		// 
		//outputNodes(EX, 0);

		func0.codes.push_back(EX);
		i++;
	}
	func0.ret = func.ret;
	func0.api = func.api;
	func0.DLL = func.DLL;
	func0.name = func.name;
	func0.args = func.args;
	func0.local = func.local;
	func0.publiced = func.publiced;
	func0.extra_name = func.extra_name;
	func0.call_type = func.call_type;
	func0.externed = func.externed;
	func0.transit = func.transit;
	func0.transitArg = func.transitArg;
	func0.use_arg_size = func.use_arg_size;
	return func0;
}
bool AST::analyzeExper(functionSet& functionSet_, function& func, Tree<node>& EX, std::vector<lstring> tk) {
	size_t size = tk.size();
	if (!size) return true;
	intptr_t final = search(tk, R(";"), 0, 0, 0);
	intptr_t end{};
	std::vector<lstring> ctk{};
	if (final == -1) {
		if (tk[0] == R("(") || tk[0] == R("{")) {
			end = matchBracket(tk, 1);
			if (!end) {
				error(R("括号不匹配")); return false;
			}
			SubTokens(tk, ctk, 2, end - 1);
			return analyzeExper(functionSet_, func, EX, ctk);
		}
		else {
			return analyze_0(functionSet_, func, EX, tk, OP_size);
		}
	}
	else {
		SubTokens(tk, ctk, 1, final - 1);
		bool p1 = analyzeExper(functionSet_, func, EX, ctk);
		SubTokens(tk, ctk, final + 1, tk.size());
		p1 &= analyzeExper(functionSet_, func, EX, ctk);
		return p1;
	}
}
bool AST::analyze_0(functionSet& functionSet_, function& func, Tree<node>& EX, std::vector<lstring> tk, int num) {
	node node{};
	std::vector<lstring> t1{}, t2{};
	lstring top{};
	cutTK(tk, op[num - 1].op, t1, t2, top);
	if (!t1.size()) {
		if (num == 1) return analyze_1(functionSet_, func, EX, tk);
		return analyze_0(functionSet_, func, EX, tk, num - 1);
	}
	node.token = top;
	node.type = R("Operator");
	EX.push_back(node);
	EX.ToChildrenEnd();
	bool p1 = analyze_0(functionSet_, func, EX, t1, num);
	bool p2 = analyze_0(functionSet_, func, EX, t2, num);
	EX.parent();
	return p1 && p2;
}
bool AST::analyze_1(functionSet& functionSet_, function& func, Tree<node>& EX, std::vector<lstring> tk) {
	node p{};
	std::vector<lstring> t1{};
	if (!tk.size()) return false;
	if (tk[0] == R("-") || tk[0] == R("+") || tk[0] == R("not")) {
		p.token = tk[0] == R("-") ? R("Minus") : (tk[0] == R("+") ? R("Abs") : R("not"));
		p.type = R("Operator");
		EX.push_back(p);
		EX.ToChildrenEnd();
		t1 = tk;
		t1.erase(t1.begin());
		bool a = analyze_1(functionSet_, func, EX, t1);
		EX.parent();
		return a;
	}
	if (tk.size() > 1 && tk[tk.size() - 2] == R("->")) {
		p.token = tk[tk.size() - 1];
		p.type = R("ad_v");
		EX.push_back(p);
		EX.ToChildrenEnd();
		t1 = tk;
		t1.erase(t1.end() - 2, t1.end());
		bool a = analyze_1(functionSet_, func, EX, t1);
		EX.parent();
		return a;
	}
	return analyze_2(functionSet_, func, EX, tk);
}
bool AST::analyze_2(functionSet& functionSet_, function& func, Tree<node>& EX, std::vector<lstring> tk) {
	node p{};
	std::vector<lstring> t1{};
	type var{};
	if(!tk.size()) return false;
	if (tk.size() == 1) {
		if (tk[0] == R("~")) {
			p.token = R("");
			p.type = R("ExactlyAddress");
			EX.push_back(p);
			return true;
		}
		if(isNum_(tk[0]) || tk[0].substr(0,1)==R("0")|| tk[0].substr(0, 1) == R("\"")|| tk[0].substr(0, 1) == R("-")){
			p.token = tk[0];
			if (isNum_(tk[0]) || tk[0].substr(0, 1) == R("0")) 
				p.type = (tk[0].find(R(".")) != std::string::npos) ? R("Double") : R("Int");
			else 
				p.type = R("Const");
			EX.push_back(p);
		}
		else {
			if (tk[0] == R("")) return true;
			p.token = getVarFullName(functionSet_, func, tk[0]);
			p.type = R("Var");
			EX.push_back(p);
		}
		return true;
	}
	if (tk[0] == R("&")) {
		p.token = R("&");
		p.type = R("Operator");
		EX.push_back(p);
		EX.ToChildrenEnd();
		t1 = tk;
		t1.erase(t1.begin());
		bool p1 = analyze_1(functionSet_, func, EX, t1);
		EX.parent();
		return p1;
	}
	if (tk[0] == R("~")) {
		if (tk.size() == 2) {
			p.token = process_quotation_mark(tk[1]);
			p.type = R("FunctionAddress");
			EX.push_back(p);
			return true;
		}
		else {
			error(R("取动态地址格式错误"));
		}
		return false;
	}
	if (tk.size() == 2) {
		if (tk[0] == R("(") && tk[1] == R(")") || tk[0] == R("[") && tk[1] == R("]")) {
			error(R("括号不匹配")); return false;
		}
	}
	if (analyzeVar(tk, var)) {
		if (var.typeName == R("sizeof")) {
			var.typeName = var.name;
			p.token = to_lstring(size(var));
			p.type = R("Int");
		}
		else {
			func.local.push_back(var);
			p.token = getVarFullName(functionSet_, func, var.name);
			p.type = R("Var");
		}
		EX.push_back(p);
		return true;
	}
	if (tk[tk.size() - 1] == R(")")) {
		intptr_t pos = search(tk, R("("), 1, {}, 0);
		if (pos == 1) {
			t1 = tk;
			t1.erase(t1.begin());
			t1.pop_back();
			return analyzeExper(functionSet_, func, EX, t1);
		}
		if (pos == 2) {
			p.token = getFunctionFullName(tk[pos - 2], functionSet_);
			p.type = R("Call");
			EX.push_back(p);
			EX.ToChildrenEnd();
			t1 = tk;
			t1.erase(t1.begin(), t1.begin() + 2);
			t1.pop_back();
			bool p1 = analyzeArg(functionSet_, func, EX, t1);
			EX.parent();
			return p1;
		}
		if (pos > 2 && tk[pos - 3] == R(".")) {
			p.token = tk[pos - 2];
			p.type = R("thisCall");
			EX.push_back(p);
			EX.ToChildrenEnd();
			t1 = tk;
			t1.erase(t1.begin() + pos - 1, t1.end());
			t1.erase(t1.end() - 2, t1.end());
			bool p1 = analyze_2(functionSet_, func, EX, t1);
			SubTokens(tk, t1, pos + 1, tk.size() - 1);
			p1 = analyzeArg(functionSet_, func, EX, t1) || p1;
			EX.parent();
			return p1;
		}
	}
	
	if (tk[tk.size() - 1] == R("]")) {
		std::vector<dim> dims{};
		intptr_t pos2 = tk.size();
		intptr_t pos = search(tk, R("["), 1, {}, 0);
		while (pos != -1) {
			t1 = tk;
			t1.erase(t1.begin()  + pos2 - 1, t1.end());
			t1.erase(t1.begin(), t1.begin() + pos);
			dim tdim;
			tdim.tk = t1;

			dims.push_back(tdim);
			pos2 = pos - 1;
			if (pos2 <= 0) {
				error(R("处理数组索引时出错"));
				return false;
			}
			if (tk[pos2 - 1] != R("]")) break;
			pos = search(tk, R("["), 1, pos2, 0);
		}
		p.token = R(""); p.type = R("Array");
		EX.push_back(p);
		EX.ToChildrenEnd();
		t1 = tk;
		t1.erase(t1.begin() + pos2, t1.end());
		bool p1 = analyzeExper(functionSet_, func, EX, t1);
		for (size_t i = 1; i <= dims.size(); i++) {
			p1 = analyzeExper(functionSet_, func, EX, dims[dims.size() - i].tk) || p1;
		}
		EX.parent();
		return p1;
	}
	if (tk[tk.size() - 2] == R(".")) {
		p.token = tk[tk.size() - 1];
		p.type = R("Element");
		EX.push_back(p);
		EX.ToChildrenEnd();
		t1 = tk;
		t1.erase(t1.end() - 2, t1.end());
		bool p1 = analyzeExper(functionSet_, func, EX, t1);
		EX.parent();
		return p1;
	}
	if (tk[tk.size() - 1] == R("}")) {
		intptr_t pos = search(tk, R("{"), 1, {}, 0);
		t1 = tk;
		t1.erase(t1.begin() + pos - 1, t1.end());
		bool p1 = analyze_2(functionSet_, func, EX, t1);
		bool p2{};
		if (p2 = EX.haveParent()) EX.ToChildrenEnd();
		p.token = R("");
		p.type = R("Block");
		EX.push_back(p);
		EX.ToChildrenEnd();
		t1 = tk;
		t1.erase(t1.begin(), t1.begin() + pos);
		t1.pop_back();
		p1 = analyzeExper(functionSet_, func, EX, t1) && p1;
		if (p2) EX.parent();
		EX.parent();
		return p1;
	}
	if (tk[tk.size() - 1] == R(".")) {
		t1 = tk;
		t1.pop_back();
		return analyze_2(functionSet_, func, EX, t1);
	}
	return false;
}
size_t AST::size(type var) {
	return getStructureSize(var.typeName) * (var.array ? var.dim.size() : 1);
}
bool AST::getVarType(lstring name, lstring& type, lstring& var){
	std::vector<lstring> t{};
	t = split(name, DIVISION);
	if (t.size() != 2) {
		error(R("错误的变量名称格式:") + name);
		return false;
	}
	type = t[1];
	var = t[2];
	return true;
}
size_t AST::getStructureSize(lstring type) {
	if (type == R("")) return 0;
	for (size_t i = 0; i < structures.size(); i++) 
		if (structures[i].name == type) return structures[i].size;
	error(R("未知数据类型:") + type);
	return 0;
}
bool AST::analyzeVar(std::vector<lstring> tk, type& var) {
	lstring head{}; size_t offset{}; std::vector<size_t> dim; intptr_t final;
	if (iftk(tk, R("["), 1) && iftk(tk, R("]"), 3)) {
		head = process_quotation_mark(tk[1]);
		offset = 3;
	}
	if (iftk(tk, R(":"), offset + 2) && tk.size() >= 3 + offset) {
		var.typeName = process_quotation_mark(tk[offset]);
		var.name = process_quotation_mark(tk[2 + offset]);
		var.array = analyze_dims(tk, dim, 4 + offset, final);
		var.dim = dim;
		if (final == tk.size() + 1) {
			if (head == R("Public")) var.publiced = true;
			else if (head == R("Private") || head == R("")) var.publiced = false;
			else { error(R("非法前缀:") + head); return false; }
			return true;
		}
	}
	return false;
}
bool AST::haveVar(functionSet functionSet_, function func, lstring name) {
	if (name == R("true") || name == R("false") || name == R("this")) return true;
	if (name.substr(0, 1) == R("#")) return true;
	for (size_t i = 0; i < func.local.size(); i++) if (func.local[i].name == name) return true;
	for (size_t i = 0; i < func.args.size(); i++) if (func.args[i].name == name) return true;
	for (size_t i = 0; i < functionSet_.local.size(); i++) if (functionSet_.local[i].name == name) return true;
	for (size_t i = 0; i < globalVars.size(); i++) if (globalVars[i].name == name) return true;
	return false;
}
lstring AST::getVarFullName(functionSet functionSet_, function func, lstring name) {
	if (name == R("this")) return R("Local") + DIVISION + R("[this]");
	if (name == R("true") || name == R("false")) return R("Const") + DIVISION + R("[") + name + R("]");
	if (name.substr(0, 1) == R("#")) return R("Const") + DIVISION + name.substr(1, name.length() - 1);
	for (size_t i = 0; i < func.local.size(); i++) if (func.local[i].name == name) return R("Local") + DIVISION + name;
	for (size_t i = 0; i < func.args.size(); i++) if (func.args[i].name == name) return R("Arg") + DIVISION + name;
	for (size_t i = 0; i < functionSet_.local.size(); i++) 
		if (functionSet_.local[i].name == name)
			return (functionSet_.isClass ? R("Class") : R("Set")) + DIVISION + name;
	for (size_t i = 0; i < globalVars.size(); i++) if (globalVars[i].name == name) return R("Global") + DIVISION + name;
	error(R("未知变量:") + name);
	return R("");
}
bool AST::analyzeArg(functionSet& functionSet_, function& func, Tree<node>& EX, std::vector<lstring> tk) {
	intptr_t pos{}, pos2{};
	std::vector<lstring> t1{};
	pos = search(tk, R(","), 0, pos, 0);
	bool a = true;
	if (pos != -1) {
		while (pos != -1) {
			SubTokens(tk, t1, pos2, pos - 1);
			if (false&&!t1.size()) {//<------------------------------------
				node p;
				p.token = R("");
				p.type = R("Blank");
				EX.push_back(p);
			}
			else {
				a = analyzeExper(functionSet_, func, EX, t1) && a;
			}
			pos2 = pos + 1;
			pos = search(tk, R(","), 0, pos, 0);
		}
	}
	SubTokens(tk, t1, pos2, tk.size());
	if (false&&!t1.size()) {
		node p;
		p.token = R("");
		p.type = R("Blank");
		EX.push_back(p);
	}
	else {
		a = analyzeExper(functionSet_, func, EX, t1) && a;
	}
	/*else {
		if (!tk.size()) a = analyzeExper(functionSet_, func, EX, tk) && a;
	}*/
	return a;
}
size_t AST::matchBracket(std::vector<lstring> tk, intptr_t start) {
	size_t size = tk.size();
	intptr_t k{};
	intptr_t i{};
	bool a{};
	if (start < 1 || size < start) return 0;
	do{
		k += checkBracket(tk[start + i - 1]);
		a = a || k;
		i++;
	} while (k && start + i <= size);
	i--;
	if (a && start + i <= size) {
		if (bracketIsMatched(tk[start - 1], tk[start + i - 1])) return start + i;
		else {
			error(R("括号不匹配"));
			return 0;
		}
	}
	return 0;

}
intptr_t AST::checkBracket(lstring tk) {
	if (tk == R("(") || tk == R("[") || tk == R("{")) return 1;
	if (tk == R(")") || tk == R("]") || tk == R("}")) return -1;
	return 0;
}
bool AST::bracketIsMatched(lstring tk1, lstring tk2) {
	return tk1 == R("(") && tk2 == R(")") || tk1 == R("[") && tk2 == R("]") || tk1 == R("{") && tk2 == R("}");
}
lstring AST::getFunctionFullName(lstring name, functionSet functionSet_) {
	std::vector<lstring> t{};
	lstring className{};
	lstring func{};
	t = split(name, R("\n"));
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
		error(R("错误的函数格式"));
		return R("");
	}
	if (className != R("") && functionSet_.name == className || className == R("")) {
		for (size_t i = 0; i < functionSet_.func.size(); i++)
			if (functionSet_.func[i].name == func) 
				return R("Local") + DIVISION + functionSet_.name + DIVISION + func;
	}
	for (size_t j = 0; j < sets.size(); j++) {
		if (sets[j].name == functionSet_.name) continue;
		for (size_t i = 0; i < sets[j].func.size(); i++) {
			if (sets[j].func[i].name == func) {
				if (sets[j].isClass && !sets[j].func[i].publiced) {
					error(R("试图调用 ") + className + R(" 中未公开的方法:" + func));
					return R("");
				}
				return R("Local") + DIVISION + sets[j].name + DIVISION + func;
			}
		}
	}
	for (size_t j = 0; j < ExtraFunctions.func.size(); j++) {
		if (className != R("")) {
			if (ExtraFunctions.func[j].DLL == className)
				return R("Extra") + DIVISION + className + DIVISION + ExtraFunctions.func[j].extra_name;
			else
				continue;
			if (ExtraFunctions.func[j].name == func) {
				return R("Extra") + DIVISION + ExtraFunctions.func[j].DLL + DIVISION + ExtraFunctions.func[j].extra_name;
			}
		}
	}
	return R("Unknown") + DIVISION + R("Unknown") + DIVISION + func;
}

void AST::error(lstring err) {
	std_lcout << RED << R("[错误]") << CYAN << R("[语法分析]")  << R("[程序集/类:") << error_functionSet << R("][函数/方法:") << error_function << R("][行:") << error_line + 1 << R("]") << RESET << err << std::endl;
	Error = true;
}
bool AST::analyze(
	const std::vector<lstring>& libs_,
	const std::vector < type>& globalVars_,
	const std::vector <functionSet>& functionSets_,
	const std::vector <structure>& structures_,
	const functionSet& ExtraFunctions_,
	const std::vector <type>& constants_) {
	op[0].op = { R("*"),R("/"),R("<<"),R(">>") };
	op[1].op = { R("%"),R("\\") };
	op[2].op = { R("+"),R("-") };
	op[3].op = { R("=="),R(">"),R("<"),R(">="),R("<="),R("!=") };
	op[4].op = { R("and"),R("&&") };
	op[5].op = { R("xor"),R("or"),R("|"),R("||") };
	op[6].op = { R("=") };

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
bool AST::getFunctionType(lstring fullname, lstring& type, lstring& super, lstring& name) {
	std::vector<lstring> t = split(name, R("\n"));
	if (t.size() < 2 || t.size() > 3) {
		error(R("错误的函数全称"));
		return false;
	}
	if (t.size() == 3) {
		type = R("Local"); super = t[0]; name = t[1]; return true;
	}
	if (t.size() == 2) {
		type = t[0]; super = t[1]; name = t[2]; return true;
	}
	return false;
}
