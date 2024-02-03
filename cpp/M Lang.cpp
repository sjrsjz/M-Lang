#include "../header/Common.h"
#include "../header/Tree.h"
#include "../header/ByteArray.h"
#include "../header/SectionManager.h"
#include "../header/AST.h"
#include "../header/Lexer.h"
#include "../header/IRGenerator.h"

#ifdef WIN32
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
void NewBuiltInFunction(functionSet& functionSet, const lstring name, const std::vector<type>& args, const type& ret) {
    function func{};
    func.name = name;
    func.args = args;
    func.ret = ret;
    func.local.clear();
    func.codes.clear();
    func.publiced = true;
    func.use_arg_size = false;
    functionSet.func.push_back(func);
}
extern void cut_tokens(lstring code, std::vector<lstring>& tks);

bool IR2MEXE(const lstring& ir,ByteArray<unsigned char>& mexe) {
#ifdef WIN32
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
        DebugOutput(x.ip);
        data.Ins(x.name, tmp, {});
    }
    DebugOutput(data.build());
    //data.translate(data.build());
    sm.Ins(R("Redirect table"), data.build(), {});
    mexe = sm.build();
    return true;
#endif
}

int main()
{
    DebugOutput(base64_decode(base64_encode(R("我"))));
    Tree<int> t;
    Lexer lex{};

#ifdef G_UNICODE_
	std::wcout.imbue(std::locale("zh_CN"));
#endif 

    bool err = lex.analyze(R(R"(
Main{
	main()->N:={
        N:i=0;
        while(i<10){
            printN(i);
            print(&"
");
            //if(0.5){print(&"SS")}{print(&"BB")};
			i=i+1;
		};
        return(0);
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
    NewBuiltInFunction(builtIn, R("break"), args, ret);
    NewBuiltInFunction(builtIn, R("continue"), args, ret);
    NewBuiltInFunction(builtIn, R("Pause"), args, ret);
    NewBuiltInFunction(builtIn, R("_IR_"), args, ret);

    args.resize(1);
    args[0].typeName = R("Boolen");
    NewBuiltInFunction(builtIn, R("if"), args, ret);
    NewBuiltInFunction(builtIn, R("while"), args, ret);
    NewBuiltInFunction(builtIn, R("do_while"), args, ret);
    NewBuiltInFunction(builtIn, R("switch"), args, ret);
    args[0].typeName = R("N");
    NewBuiltInFunction(builtIn, R("return"), args, ret);
    NewBuiltInFunction(builtIn, R("print"), args, ret);
    NewBuiltInFunction(builtIn, R("printN"), args, ret);
    NewBuiltInFunction(builtIn, R("new"), args, ret);
    NewBuiltInFunction(builtIn, R("free"), args, ret);
    NewBuiltInFunction(builtIn, R("DebugOutput"), args, ret);
    NewBuiltInFunction(builtIn, R("ErrMark"), args, ret);

    args[0].typeName = R("R");
    NewBuiltInFunction(builtIn, R("printR"), args, ret);
    args[0].typeName = R("Z");
    NewBuiltInFunction(builtIn, R("printZ"), args, ret);
    args[0].typeName = R("B");
    NewBuiltInFunction(builtIn, R("printB"), args, ret);
    args[0].typeName = R("Boolen");
    NewBuiltInFunction(builtIn, R("printBoolen"), args, ret);

    ret.typeName = R("R");
    args[0].typeName = R("N");
    NewBuiltInFunction(builtIn, R("T2R"), args, ret);
    ret.typeName = R("N");
    NewBuiltInFunction(builtIn, R("N"), args, ret);
    ret.typeName = R("R");
    NewBuiltInFunction(builtIn, R("R"), args, ret);
    ret.typeName = R("B");
    NewBuiltInFunction(builtIn, R("B"), args, ret);
    ret.typeName = R("Z");
    NewBuiltInFunction(builtIn, R("Z"), args, ret);
    ret.typeName = R("Boolen");
    NewBuiltInFunction(builtIn, R("Boolen"), args, ret);

    args.resize(2);
    args[0].typeName = R("N");
    args[1].typeName = R("N");
    ret.typeName = R("Boolen");
    NewBuiltInFunction(builtIn, R("CmpStr"), args, ret);
    args[0].typeName = R("R");
    args[1].typeName = R("N");
    ret.typeName = R("N");
    NewBuiltInFunction(builtIn, R("R2T"), args, ret);
    args[0].typeName = R("N");
    args[1].typeName = R("N");
    ret.typeName = R("N");
    NewBuiltInFunction(builtIn, R("input"), args, ret);

    args.resize(3);
    args[0].typeName = R("N");
    args[1].typeName = R("N");
    args[2].typeName = R("N");
    ret.typeName = R("N");
    NewBuiltInFunction(builtIn, R("memcopy"), args, ret);
    ret.typeName = R("Boolen");
    NewBuiltInFunction(builtIn, R("CmpMem"), args, ret);



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
    ByteArray<unsigned char> mexe;
    err = !IR2MEXE(ir.IR, mexe);
    if (err) return 0;
#ifdef _WIN32
    x86Runner::LoadMEXE(mexe);
    x86Runner::run();
#endif // _WIN32
}
