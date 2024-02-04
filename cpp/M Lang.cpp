#include "../header/Common.h"
#include "../header/Tree.h"
#include "../header/ByteArray.h"
#include "../header/SectionManager.h"
#include "../header/AST.h"
#include "../header/Lexer.h"
#include "../header/IRGenerator.h"

#ifdef _WIN32
#include "../header/x86.h"
#endif // WIN32


using namespace MLang;

void NewType(std::vector<structure>& structure_, lstring name, bool publiced, size_t size) {
    structure tmp;
    tmp.elements.clear();
    tmp.isClass = false;
    tmp.name = name;
    tmp.publiced = publiced;
    tmp.size = size;
    structure_.push_back(tmp);
}
void NewBuiltInFunction(functionSet& functionSet, const lstring name, const std::vector<type>& args, const type& ret,bool cdecl_) {
    function func{};
    func.name = name;
    func.args = args;
    func.ret = ret;
    func.local.clear();
    func.codes.clear();
    func.publiced = true;
    func.use_arg_size = cdecl_;
    func.call_type = cdecl_ ? R("cdecl") : R("stdcall");
    functionSet.func.push_back(func);
}
extern void cut_tokens(lstring code, std::vector<lstring>& tks);

bool IR2MEXE(const lstring& ir,ByteArray<unsigned char>& mexe) {
#ifdef _WIN32
    x86Generator x86{};
    x86.generate(ir);
    if (x86.Error) {
		return false;
	}
	mexe = x86.codes;
    SectionManager sm{};
    sm.Ins(R("Code"), mexe, {});
    ByteArray tbin{};
    tbin = (unsigned int)x86.globalSize;
    sm.Ins(R("Global size"), tbin, {});
    SectionManager data{};
    data.Clear();
    for (auto& x : x86.apiTable) {
        data.Ins(x, ByteArray(), {});
    }
    sm.Ins(R("API table"), data.build(), {});
    data.Clear();
    for (auto& x : x86.constStr) {
		data.Ins(x, ByteArray(), {});
	}
    sm.Ins(R("Strings"), data.build(), {});

    data.Clear();
    for (auto& x : x86.linkTable) {
        ByteArray<unsigned char> tmp{};
        tmp = (int)x.ip;
        data.Ins(x.name, tmp, {});
    }
    //data.translate(data.build());
    sm.Ins(R("Redirect table"), data.build(), {});
    mexe = sm.build();
    return true;
#endif
}

int main()
{
    Lexer lex{};

#ifdef G_UNICODE_
	std::wcout.imbue(std::locale("zh_CN"));
#endif 
    bool err = lex.analyze(R(R"(




Main{
A:={
    N:a,N:b,N:c
}
    f()->A:={
        A:tmp;tmp.a=666;tmp.b=666;tmp.c=666;
        return(tmp)
    }
	main()->N:={
        N:a[3];
        a[0]=0;
		a[1]=0;
		a[2]=0;
        A:A;
        A.a=100;
        A.b=100;
        A.c=100;
        N:b[3];
        b[0]=1;
		b[1]=1;
		b[2]=1;
        A=f();
		printN(A.a);
        print(&"
");
		printN(A.b);
		print(&"
");
		printN(A.c);
		print(&"
");
		printN(a[0]);
		print(&"
");
		printN(a[1]);
		print(&"
");
        printN(a[2]);
		print(&"
");
        printN(b[0]);
		print(&"
");
        printN(b[1]);
		print(&"
");
        printN(b[2]);
		print(&"
");

	}
}

)"));
    if (err) return 0;
    NewType(lex.structures, R("N"), false, sizeof(size_t));
    NewType(lex.structures, R("Z"), false, sizeof(int));
    NewType(lex.structures, R("R"), false, sizeof(double));
    NewType(lex.structures, R("B"), false, sizeof(unsigned char));
    NewType(lex.structures, R("Boolen"), false, sizeof(float));

    std::vector<type> args{};
    type ret{};
    functionSet builtIn{};
    ret.typeName = R("N");
    NewBuiltInFunction(builtIn, R("break"), args, ret, false);
    NewBuiltInFunction(builtIn, R("continue"), args, ret, false);
    NewBuiltInFunction(builtIn, R("Pause"), args, ret, false);
    NewBuiltInFunction(builtIn, R("_IR_"), args, ret, false);
    ret.typeName = R("R");
    NewBuiltInFunction(builtIn, R("rand"), args, ret, false);

    ret.typeName = R("N");
    args.resize(1);
    args[0].typeName = R("Boolen");
    NewBuiltInFunction(builtIn, R("if"), args, ret, false);
    NewBuiltInFunction(builtIn, R("while"), args, ret, false);
    NewBuiltInFunction(builtIn, R("do_while"), args, ret, false);
    NewBuiltInFunction(builtIn, R("switch"), args, ret, false);
    args[0].typeName = R("N");
    NewBuiltInFunction(builtIn, R("return"), args, ret, false);
    NewBuiltInFunction(builtIn, R("print"), args, ret, true);
    NewBuiltInFunction(builtIn, R("printN"), args, ret, false);
    NewBuiltInFunction(builtIn, R("srand"), args, ret, false);
    NewBuiltInFunction(builtIn, R("new"), args, ret, false);
    NewBuiltInFunction(builtIn, R("free"), args, ret, false);
    NewBuiltInFunction(builtIn, R("DebugOutput"), args, ret, false);
    NewBuiltInFunction(builtIn, R("ErrMark"), args, ret, false);

    args[0].typeName = R("R");
    NewBuiltInFunction(builtIn, R("printR"), args, ret, false);
    args[0].typeName = R("Z");
    NewBuiltInFunction(builtIn, R("printZ"), args, ret, false);
    args[0].typeName = R("B");
    NewBuiltInFunction(builtIn, R("printB"), args, ret, false);
    args[0].typeName = R("Boolen");
    NewBuiltInFunction(builtIn, R("printBoolen"), args, ret, false);

    ret.typeName = R("R");
    args[0].typeName = R("N");
    NewBuiltInFunction(builtIn, R("T2R"), args, ret, false);
    ret.typeName = R("N");
    NewBuiltInFunction(builtIn, R("N"), args, ret, false);
    ret.typeName = R("R");
    NewBuiltInFunction(builtIn, R("R"), args, ret, false);
    ret.typeName = R("B");
    NewBuiltInFunction(builtIn, R("B"), args, ret, false);
    ret.typeName = R("Z");
    NewBuiltInFunction(builtIn, R("Z"), args, ret, false);
    ret.typeName = R("Boolen");
    NewBuiltInFunction(builtIn, R("Boolen"), args, ret, false);

    args.resize(2);
    args[0].typeName = R("N");
    args[1].typeName = R("N");
    ret.typeName = R("Boolen");
    NewBuiltInFunction(builtIn, R("CmpStr"), args, ret, false);
    args[0].typeName = R("R");
    args[1].typeName = R("N");
    ret.typeName = R("N");
    NewBuiltInFunction(builtIn, R("R2T"), args, ret, false);
    args[0].typeName = R("N");
    args[1].typeName = R("N");
    ret.typeName = R("N");
    NewBuiltInFunction(builtIn, R("input"), args, ret, false);

    args.resize(3);
    args[0].typeName = R("N");
    args[1].typeName = R("N");
    args[2].typeName = R("N");
    ret.typeName = R("N");
    NewBuiltInFunction(builtIn, R("memcopy"), args, ret, false);
    ret.typeName = R("Boolen");
    NewBuiltInFunction(builtIn, R("CmpMem"), args, ret, false);



    Lexer lex2{};
    std::vector<lstring> lines = split(R(""), R("\n"));
    for (auto& x : lines) {
        std::vector<lstring> tk{};
        cut_tokens(x, tk);
        lex2.analyze_function(builtIn, R(""), tk, 1);
    }
    builtIn.local.clear();
    builtIn.name = R("[System]");
    lex.functionSets.push_back(builtIn);

    AST ast{};
	
    ast.analyze(lex.importedLibs, lex.globals, lex.functionSets, lex.structures, lex.ExternFunctions, lex.constants);

	IRGenerator ir{};
    err = ir.analyze(ast.libs, ast.globalVars, ast.analyzed_functionSets, ast.sets, ast.structures, ast.ExtraFunctions, ast.constants);
    if (err) return 0;
    std_lcout << ir.IR;
    ByteArray<unsigned char> mexe;
    err = !IR2MEXE(ir.IR, mexe);
    if (err) return 0;
#ifdef _WIN32
    x86Runner::LoadMEXE(mexe);
    x86Runner::run();
#endif // _WIN32
}
