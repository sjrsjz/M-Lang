#include "../header/Lexer.h"
using namespace MLang;

bool Error{};

void error(std::vector<lstring>& tks, lstring err, size_t ip) {
    std_lcout << RED << R("[错误]") << YELLOW << R("[词法分析]") << RESET << err << std::endl;
}

bool Lexer::lexical_analyze(functionSet& set, lstring space, std::vector<lstring> tks) {
    for (size_t ip = 1; ip <= tks.size();) {
        size_t ip0 = ip;
        ip = analyze_struct(set, space, tks, ip);
        ip = analyze_setVars(set, space, tks, ip);
        ip = analyze_function(set, space, tks, ip);
        if (ip0 > ip) {
            error(tks, R("出现未知结构"), ip); return false;
        }
        if (ip0 == ip) ip++;
    }
    return true;
}
intptr_t Lexer::analyze_struct(functionSet& set, lstring space, std::vector<lstring> tks, size_t ip) {
    if (ip + 3 >= tks.size()) return ip;
    if (tks[ip] != R(":=") || tks[ip + 1] != R("{")) return ip;
    lstring name = space == R("") ? tks[ip] : space + DIVISION + tks[ip];
    intptr_t final = search(tks, R("}"), 0, ip + 3, -1);
    if (final == -1) {
        error(tks, R("括号不匹配"), ip + 2);
        return -1;
    }
    std::vector<lstring> tk0{}, tk1{};
    SubTokens(tks, tk0, ip + 3, final - 1);
    intptr_t ip0 = 1;
    intptr_t ip1 = search(tk0, R(","), 0, ip0, 0);
    structure struct0{};
    struct0.elements.resize(0);
    type element{};
    while (ip1 != -1)
    {
        SubTokens(tk0, tk1, ip0, ip1 - 1);
        if (analyze_vars(tk1, element))
            struct0.elements.push_back(element);
        else
            lexical_analyze(set, name, tk1);
        ip0 = ip1 + 1;
        ip1 = search(tk0, R(","), 0, ip0, 0);
    }
    SubTokens(tk0, tk1, ip0, tk0.size());
    if (analyze_vars(tk1, element))
        struct0.elements.push_back(element);
    else
        lexical_analyze(set, name, tk1);
    struct0.name = name;
    structures.push_back(struct0);
    return final + 1;
}
intptr_t Lexer::analyze_function(functionSet& set, lstring space, std::vector<lstring> tks, size_t ip) {
    intptr_t ip_ = ip;
    std::vector<lstring> head{};
    if (ip + 1 > tks.size()) return ip;
    while (iftk(tks, R("["), ip) && iftk(tks, R("]"), ip + 2))
    {
        head.push_back(process_quotation_mark(tks[ip]));
        ip += 3;
    }
    if (!iftk(tks, R("("), ip + 1)) return ip_;
    lstring t1; t1 = process_quotation_mark(tks[ip - 1]);
    lstring name; name = space == R("") ? t1 : space + DIVISION + t1;
    intptr_t final; final = search(tks, R(")"), 0, ip + 1, -1);
    if (final == -1) {
        error(tks, R("括号不匹配"), ip + 1); return -1;
    }
    if (!iftk(tks, R("->"), final + 1)) return ip_;
    std::vector<lstring> tk0{}, tk1{};
    SubTokens(tks, tk0, ip + 2, final - 1);
    intptr_t ip0 = 1, ip1; ip1 = search(tk0, R(","), 0, ip0, 0);
    function func{};
    func.args.resize(0);
    type arg{};
    while (ip1 != -1)
    {
        SubTokens(tk0, tk1, ip0, ip1 - 1);
        if (analyze_vars(tk1, arg))
            func.args.push_back(arg);
        else
            lexical_analyze(set, name, tk1);
        ip0 = ip1 + 1;
        ip1 = search(tk0, R(","), 0, ip0, 0);
    }
    SubTokens(tk0, tk1, ip0, tk0.size());
    if (analyze_vars(tk1, arg))
        func.args.push_back(arg);
    else
        lexical_analyze(set, name, tk1);
    ip0 = final + 2;
    final = search(tks, R("{"), 0, ip0, 1);
    if (final == -1 || tks[final - 2] != R(":=")) {
        error(tks, R("错误的函数定义"), ip0);
        return -1;
    }
    SubTokens(tks, tk0, ip0, final - 2);
    if (!analyze_type(tk0, func.ret)) {
        error(tks, R("返回值定义错误"), ip0);
        return -1;
    }
    ip = final;
    final = search(tks, R("}"), 0, ip, -1);
    if (final == -1) {
        error(tks, R("括号不匹配"), ip);
        return -1;
    }
    SubTokens(tks, tk0, ip + 1, final - 1);
    ip0 = 1;
    ip1 = search(tk0, R(";"), 0, ip0, 0);
    func.codes.resize(0);
    func.local.resize(0);
    code code0{};
    while (ip1 != -1) {
        SubTokens(tk0, tk1, ip0, ip1 - 1);
        code0.tokens = tk1;
        func.codes.push_back(code0);
        ip0 = ip1 + 1;
        ip1 = search(tk0, R(";"), 0, ip0, 0);
    }
    SubTokens(tk0, tk1, ip0, tk0.size());
    code0.tokens = tk1;
    func.codes.push_back(code0);
    func.name = name;
    func.call_type = R("stdcall");
    func.transit = false;
    func.use_arg_size = false;
    for (size_t i = 0; i < head.size(); i++) {
        if (head[i] == R("Public")) func.publiced = true;
        else if (head[i] == R("Private")) func.publiced = false;
        else if (head[i] == R("cdecl")) func.call_type = R("cdecl");
        else if (head[i] == R("stdcall")) func.call_type = R("stdcall");
        else if (head[i] == R("Transit")) { 
            func.transit = true; 
            if ((intptr_t)i >= (intptr_t)head.size() - 1) {
                error(tks, R("transit标识后必须具有用于传递this指针的参数名"), ip);
                return -1;
            }
            func.transitArg = head[i + 1];
            i++;
        }
        else if (head[i] == R("ArgSize")) func.use_arg_size = true;
        else { error(tks, R("非法前缀:") + head[i], ip); return -1; }
    }
    set.func.push_back(func);
    return final + 1;
}
intptr_t Lexer::analyze_externedFunction(functionSet& set, lstring DLL, std::vector<lstring> tks, size_t ip) {
    if (ip + 6 > tks.size() || tks[ip] != R("(")) return ip;
    std::vector<lstring> head{}, tk0{}, tk1{};
    while (iftk(tks, R("["), ip) && iftk(tks, R("]"), ip + 2)) {
        head.push_back(process_quotation_mark(tks[ip]));
        ip += 3;
    }
    lstring name = process_quotation_mark(tks[ip - 1]);
    intptr_t final = search(tks, R(")"), 0, ip + 1, -1);
    if (final == -1) {
        error(tks, R("括号不匹配"), ip + 1);
        return -1;
    }
    if (!iftk(tks, R("->"), final + 1)) return ip;
    SubTokens(tks, tk0, ip + 2, final - 1);
    intptr_t ip0 = 1, ip1; ip1 = search(tk0, R(","), 0, ip0, 0);
    function func{};
    func.args.resize(0);
    type arg{};
    while (ip1 != -1)
    {
        SubTokens(tk0, tk1, ip0, ip1 - 1);
        if (analyze_vars(tk1, arg))
            func.args.push_back(arg);
        else
            lexical_analyze(set, name, tk1);
        ip0 = ip1 + 1;
        ip1 = search(tk0, R(","), 0, ip0, 0);
    }
    SubTokens(tk0, tk1, ip0, tk0.size());
    if (analyze_vars(tk1, arg))
        func.args.push_back(arg);
    else
        lexical_analyze(set, name, tk1);

    ip0 = final + 2;
    final = search(tks, R(":="), 0, ip0, 0);
    if (final == -1 || final >= (intptr_t)tks.size()) {
        error(tks, R("错误的外部函数定义"), ip0);
        return -1;
    }
    SubTokens(tks, tk0, ip0, final - 1);
    if (!analyze_type(tk0, func.ret)) {
        error(tks, R("返回值定义错误"), ip0);
        return -1;
    }
    func.extra_name = process_quotation_mark(tks[final]);
    func.DLL = DLL;
    func.externed = true;
    func.name = name;
    func.call_type = R("stdcall");
    func.transit = false;
    func.use_arg_size = false;
    for (size_t i = 0; i < head.size(); i++) {
        if (head[i] == R("Public")) func.publiced = true;
        else if (head[i] == R("Private")) func.publiced = false;
        else if (head[i] == R("cdecl")) func.call_type = R("cdecl");
        else if (head[i] == R("stdcall")) func.call_type = R("stdcall");
        else if (head[i] == R("Transit")) func.transit = true;
        else if (head[i] == R("ArgSize")) func.use_arg_size = false;
        else { error(tks, R("非法前缀:") + head[i], ip); return -1; }
    }
    set.func.push_back(func);
    return final + 2;

}
intptr_t Lexer::analyze_setVars(functionSet& set, lstring space, std::vector<lstring> tks, size_t ip) {
    intptr_t final;
    type var{};
    std::vector<lstring> tk0{};
    if (iftk(tks, R("["), ip))
        final = search(tks, R(";"), 0, ip, -1);
    else
        final = search(tks, R(";"), 0, ip, 0);
    if (final == -1) final = tks.size() + 1;
    SubTokens(tks, tk0, ip, final - 1);
    if (analyze_vars(tk0, var)) {
        set.local.push_back(var); return final + 1;
    }
    else return ip;
}
bool Lexer::analyze_vars(std::vector<lstring> tks, type& var) {
    lstring head{};
    intptr_t final{}, offset{}, lp{};
    std::vector<size_t> dim{};
    if (iftk(tks, R("["), 1) && iftk(tks, R("]"), 3)) {
        head = process_quotation_mark(tks[1]);
        offset = 3;
    }
    if (iftk(tks, R(":"), offset + 2) && (intptr_t)tks.size() >= offset + 3) {
        var.typeName = process_quotation_mark(tks[offset]);
        var.name = process_quotation_mark(tks[offset + 2]);
        var.array = analyze_dims(tks, dim, offset + 4, final);
        var.dim = dim;
        lp = tks.size() + 1;
        if (final == lp) {
            if (head == R("Public")) var.publiced = true;
            else if (head == R("Private") || head == R("")) var.publiced = false;
            else { error(tks, R("非法前缀:") + head, 2); }
            return true;
        }
    }
    return false;
}
intptr_t Lexer::analyze_externedFunctionSet(std::vector<lstring> tks, size_t ip) {
    intptr_t ip0{}, ip1{}, start{}, final{};
    lstring Extra{}; std::vector<lstring> tk1{};
    if (iftk(tks, R("Extra"), ip) && iftk(tks, R(":"), ip + 1) && iftk(tks, R("{"), ip + 3)) {
        Extra = process_quotation_mark(tks[ip + 1]);
        final = search(tks, R("}"), 0, ip + 3, -1);
        if (final == -1) {
            error(tks, R("括号不匹配"), ip + 3);
            return -1;
        }
        SubTokens(tks, tk1, ip + 4, final - 1);
        while (ip1 <= (intptr_t)tk1.size()) {
            ip0 = ip1;
            ip1 = analyze_externedFunction(ExternFunctions, Extra, tk1, ip1);
            if (ip0 > ip1) {
                error(tks, R("出现未知结构"), ip1);
                return -1;
            }
            if (ip1 == ip0) ip1++;
        }
        return final + 1;
    }
    return ip;
}
intptr_t Lexer::analyze_functionSet(std::vector<lstring> tks, size_t ip) {
    intptr_t ip0{}, final{}, size{};
    std::vector<lstring> tk0{};
    functionSet set{};
    structure struct0{};
    size = tks.size();
    ip0 = ip;
    if (iftk(tks, R("Extra"), ip) && iftk(tks, R(":"), ip + 1)) return ip;
    if (iftk(tks, R("Class"), ip) && iftk(tks, R(":"), ip + 1)) {
        set.isClass = true;
        ip += 2;
    }
    if (iftk(tks, R("<<"), ip + 1) && ip + 2 <= (size_t)size) {
        set.base = process_quotation_mark(tks[ip + 1]); ip += 2;
    }
    if (iftk(tks, R("{"), ip + 1)) {
        final = search(tks, R("}"), 0, ip + 1, -1);
        if (final == -1) {
            error(tks, R("括号不匹配"), ip + 1);
            return ip;
        }
        SubTokens(tks, tk0, ip + 2, final - 1);
        set.name = process_quotation_mark(tks[ip - 1]);
        set.func.clear();
        set.local.clear();
        lexical_analyze(set, R(""), tk0);
        functionSets.push_back(set);
        if (set.isClass) {
            struct0.name = set.name;
            struct0.size = 0;
            struct0.publiced = set.publiced;
            struct0.elements = set.local;
            struct0.isClass = true;
            structures.push_back(struct0);
        }
        return final + 1;
    }
    return ip0;
}
intptr_t Lexer::analyze_globalVars(std::vector<lstring> tks, size_t ip) {
    intptr_t final{};
    std::vector<lstring> tk0{};
    type var{};
    functionSet set{};
    final = search(tks, R(":"), 0, ip + 1, 0);
    if (final == -1) final = tks.size() + 1;
    SubTokens(tks, tk0, ip, final - 1);
    if (analyze_vars(tk0, var)) {
        globals.push_back(var);
        return final + 1;
    }
    return ip;
}

extern void cut_tokens(lstring code,std::vector<lstring>& tks){
    intptr_t currentPos{}, length{}, i{};
    bool forbid{}, annotation{}, annotation_line{};
    lstring tmp_tk{}, curr_tk{}, tk{}, tmp_tk2{};
    tks.clear();
    forbid = false;
    i = 0;
    length = code.length();
    while (i < length)
    {
        i++;
        tmp_tk = code.substr(i - 1, 2);
        if (tmp_tk == R("//") && !forbid) annotation_line = true;
        if (tmp_tk == R("/*") && !forbid && !annotation_line) { annotation = true; i++; }
        if (tmp_tk == R("*/") && !forbid && !annotation_line) { annotation = false; i++; }
        if (code.substr(i - 1, 1) == R("\n") && !annotation && !forbid) annotation_line = false;
        if (annotation || annotation_line) continue;
        if (IsOperator(tmp_tk, 0) && !forbid && !annotation_line) {
            tks.push_back(RemoveSpaceLR(curr_tk));
            tks.push_back(tmp_tk);
            curr_tk = R("");
            i++;
            continue;
        }
        if (tmp_tk == R("\r\n") && !forbid) { annotation_line = false; continue; }
        tmp_tk = code.substr(i - 1, 1);
        if (tmp_tk == R("\r") && !forbid) { annotation_line = false; continue; }
        if (tmp_tk == R("\n") && !forbid) { annotation_line = false; continue; }
        if (IsOperator(tmp_tk, 0) && !forbid && !annotation_line) {
            tks.push_back(RemoveSpaceLR(curr_tk));
            tks.push_back(tmp_tk);
            curr_tk = R("");
            continue;
        }
        if (tmp_tk == R("\"")) forbid = !forbid;
        curr_tk += tmp_tk;

    }
    tks.push_back(RemoveSpaceLR(curr_tk));
    i = 0;
    while (i < tks.size())
    {
        i++;
        if (tks[i - 1] == R("") || tks[i - 1] == R("\n") || tks[i - 1] == R("\t")) {
            tks.erase(tks.begin() + i - 1); i--; continue;
        }
        if (tks[i - 1].substr(0, 1) != R("\"") && tks[i - 1].substr(tks[i - 1].length() - 1, 1) != R("\"")) {
            tmp_tk2 = tks[i - 1];
            tks[i - 1] = RemoveSpaceLR(subreplace(tks[i - 1], R("\t"), R(" ")));
            if (tmp_tk2 != tks[i - 1]) {
                i--; continue;
            }
        }
        tmp_tk2 = R("");
        while (tmp_tk2 != tks[i - 1])
        {
            tmp_tk2 = tks[i - 1];
            tks[i - 1] = subreplace(tks[i - 1], R("  "), R(" "));
        }
        if (tks[i - 1] == R(" ")) {
            tks.erase(tks.begin() + i - 1);
            i--;
        }
    }
    i = 0;
    while (i < tks.size())
    {
        i++;
        if (tks.size() >= (size_t)i + 2 && tks[i] == R(".") && (isNum(tks[i - 1]) || tks[i - 1] == R("-0"))) {
            tks[i - 1] += R(".") + tks[i + 1];
            tks.erase(tks.begin() + i);
            tks.erase(tks.begin() + i);
            continue;
        }
        if (tks.size() >= (size_t)i + 2 && tks[i] == R("-") && IsOperator(tks[i - 1], 1) && isNum(tks[i + 1])) {
            tks[i] = R("-") + tks[i + 1];
            tks.erase(tks.begin() + i + 1);
            continue;
        }
    }
}


bool Lexer::analyze(lstring code) {
    Error = false;
    std::vector<lstring> tks{};
    cut_tokens(code,tks);
    preprocesser(tks);
    functionSets.clear();
    ExternFunctions.func.clear();
    size_t ip = 1;
    size_t ip0{};
    while (ip <= tks.size()) {
        ip0 = ip;
        ip = analyze_functionSet(tks, ip);
        ip = analyze_externedFunctionSet(tks, ip);
        ip = analyze_globalVars(tks, ip);
        if (ip0 > ip) { error(tks, R("出现未知结构"), ip); return Error; }
        if (ip0 == ip) ip++;
    }
    return Error;
}
bool Lexer::analyze_type(std::vector<lstring>& tk, type& var) {
    intptr_t final; std::vector<size_t> dim;
    if (tk.size() >= 1) {
        var.typeName = tk[0];
        var.array = analyze_dims(tk, dim, 2, final);
        var.dim = dim;
        if (final == tk.size() + 1) return true;
    }
    return false;
}
void Lexer::preprocesser(std::vector<lstring>& tk) {
    size_t i{}, size{};
    std::vector<lstring> ttk{};
    lstring t{}, t2{}, t3{}, path{}, file{}, a{}, b{};
    while (i<tk.size())
    {
        
        t2 = tk[i];
        i++;
        if (t2.substr(0, 1) == R("#")) {
            t = t2.substr(1, t2.size() - 1);
            if (t == R("include")) {
                if (i + 3 <= tk.size() && tk[i] == R("<") && tk[i + 2] == R(">")) {
                    t3 = tk[i + 1];
                    path = (t3.substr(0, 1) == R("@")) ? t3.substr(2, t3.size() - 3) : t3.substr(1, t3.size());
                    file = readFileString(path);
                    cut_tokens(file, ttk);
                    tk.insert(tk.end(),ttk.begin(),ttk.end());
                    tk.erase(tk.begin() + i - 1, tk.begin() + i + 4);
                    i--;
                }
                else {
                    error(tk, R("include用法错误"), i);
                }
            }
            else if (t == R("define")) {
                if (i + 2 > tk.size()) {
                    error(tk, R("define用法错误"), i); continue;
                }
                a = tk[i]; b = tk[i + 1];
                size = tk.size();
                for (size_t j = 0; j < size; j++) {
                    if (tk[j].substr(0, 1) != R("\"") || tk[j].substr(tk.size() - 1, 1) != R("\"")){
                        tk[j] = subreplace(tk[j], a, b);
                    }
                }
                tk.erase(tk.begin() + i - 1, tk.begin() + i + 3);
                i--;
            }
        }
    }
}
Lexer::Lexer() {}
Lexer::~Lexer() {}