#define _IN_MAIN_
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
    bool a = false;
    for (auto& x : functionSet.func) {
        if (x.name == name) {
            x.polymorphic = true;
            a = true;
        }
    }
    func.polymorphic = a;
    functionSet.func.push_back(func);
}
extern void cut_tokens(lstring code, std::vector<lstring>& tks);

void PrepareLexer(Lexer& lex) {
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
    //NewBuiltInFunction(builtIn, R("ErrMark"), args, ret, false);

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
}

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
    sm.Ins(R("Redirect table"), data.build(), {});
    mexe = sm.build();
    return true;
#endif
}

int test(int argn,char* argv[])
{
    Lexer lex{};

#ifdef G_UNICODE_
	std::wcout.imbue(std::locale("zh_CN"));
#endif 

    bool err = lex.analyze(R(R"ABC(

Extra:"user32.dll"{
    MessageBoxW(N:HWND,N:Text,N:Caption,N:Type)->N:=MessageBoxW;
}

_string_{
    R2Str(R:data)->string:={
        string:tmp;
        tmp.size=0;
        tmp.ad=new(512);
        R2T(data,tmp.ad);
        while(N(tmp.ad+tmp.size)->B!=0 or N(tmp.ad+tmp.size+1)->B!=0){tmp.size=tmp.size+2};
        tmp.size=tmp.size+2;
        return(tmp)
    }
}
Class:string{
    [Public]N:ad;[Public]N:size;
    _init_()->N:={
        ad=0;size=0
    }
    _destroy_()->N:={
        if(ad!=0){free(ad);ad=0};
        size=0;
    }
    _string_:={
        N:ad,N:size
    }
    [Public]"return(string)"(string:s)->N:={//prevent RAII destroys data
        _destroy_();
        ad=new(s.size);
        size=s.size;
        memcopy(ad,s.ad,size);
    }
    [Public]=(string:s)->string:={
        _destroy_();
        ad=new(s.size); 
        size=s.size;
        memcopy(ad,s.ad,s.size);
        return(this)
    }
    [Public]const(N:str)->N:={
        _destroy_();
        N:i=0;
        while(N(str+i)->B!=0 or N(str+i+1)->B!=0){i=i+2};
        size=i+2;
        ad=new(size);
        memcopy(ad,str,size);
    }
    [Public]+(string:s)->string:={
        string:tmp;
        if(size!=0){
            tmp.size=s.size+size-2;
            tmp.ad=new(tmp.size);
            memcopy(tmp.ad,ad,size);
            memcopy(tmp.ad+size-2,s.ad,s.size);
        }{
            tmp=s;
        };
        return(tmp)
    }
    [Public]*(N:times)->string:={
        string:tmp;
        N:i=0;
        while(i<times){
            tmp=tmp+this;
            i=i+1
        };
        return(tmp)
    }
    [Public]"R()"()->R:={
        return(T2R(ad))
    }
    [Public]==(string:s)->Boolen:={
        return(CmpStr(ad,s.ad))
    }
    [Public]!=(string:s)->Boolen:={
        return(not CmpStr(ad,s.ad))
    }
    [Public]"N()"()->N:={
		return(ad)
	}
}
Class:test{
    [Public]"sizeof()"(Z:arg)->N:={
        print("Z");

        return(arg)
    }
    [Public]"sizeof()"(R:arg)->N:={
        print("R");
        return(233)
    }
}
Main{
    string:A_G;
    f(B:s[0])->string:={
        string:A.const(&"test");
        return(A);
    }
	main()->N:={
        test:T;
        printN(sizeof:(T,114514.01));printN(sizeof:(T,114514));
        print("\n");
        string:A_Array[10];
        if(CmpStr(typeof:(1+2),"Z")==0){
            printN(sizeof:(A_Array));print(" True\n");
        };
		string:A.const("Hello ");
        string:B.const("World!");
        string:C.const("\n");
        string:D=f("Hello")+C+R2Str(0.5)+C+(A+B+C)*10;
        A_G=D;
        MessageBoxW(0,D,"Title",0);
		print(A_G,D);
	}
}

)ABC"));

    if (err) return 0;

    PrepareLexer(lex);

    AST ast{};
	
    ast.analyze(lex.importedLibs, lex.globals, lex.functionSets, lex.structures, lex.ExternFunctions, lex.constants);

	IRGenerator ir{};
    err = ir.analyze(ast.libs, ast.globalVars, ast.analyzed_functionSets, ast.sets, ast.structures, ast.ExtraFunctions, ast.constants);
    if (err) return 0;
#if _DEBUG
    std_lcout << ir.IR;
#endif
    ByteArray<unsigned char> mexe;
    err = !IR2MEXE(ir.IR, mexe);
    if (err) return 0;
#ifdef _WIN32
    x86Runner::LoadMEXE(mexe);
    x86Runner::run();
#endif // _WIN32
}

bool process_command(std::vector<lstring> args) {
    if (args.size() == 2 && args[0] == R("runIR")) {
        workPath = getDictionary(args[1]) + R("\\");
        lstring ir = readFileString(args[1]);
        ByteArray<unsigned char> mexe;
        if (!IR2MEXE(ir, mexe)) {
			return false;
		}
#ifdef _WIN32
        x86Runner::LoadMEXE(mexe);
        x86Runner::run();
#endif // _WIN32
        return true;
    }
    else if (args.size() == 2 && args[0] == R("run")) {
        workPath = getDictionary(args[1]) + R("\\");
        Lexer lex{};
		bool err = lex.analyze(readFileString(args[1]));
		if (err) return false;
		PrepareLexer(lex);
		AST ast{};
		err = ast.analyze(lex.importedLibs, lex.globals, lex.functionSets, lex.structures, lex.ExternFunctions, lex.constants);
		if (err) return false;
		IRGenerator ir{};
		err = ir.analyze(ast.libs, ast.globalVars, ast.analyzed_functionSets, ast.sets, ast.structures, ast.ExtraFunctions, ast.constants);
		if (err) return false;
		ByteArray<unsigned char> mexe;
		err = !IR2MEXE(ir.IR, mexe);
		if (err) return false;
#ifdef _WIN32
        x86Runner::LoadMEXE(mexe);
        x86Runner::run();
#endif // _WIN32
        return true;
    }
    else if (args.size() == 2 && args[0] == R("runMEXE")) {
        workPath = getDictionary(args[1]) + R("\\");
        ByteArray<unsigned char> mexe = readFileByteArray<unsigned char>(args[1]);
        #ifdef _WIN32
        x86Runner::LoadMEXE(mexe);
        x86Runner::run();
        #endif // _WIN32
        return true;
    }
    else if (args.size() == 3 && args[0] == R("buildIR")) {
        workPath = getDictionary(args[1]) + R("\\");
        Lexer lex{};
        bool err = lex.analyze(readFileString(args[1]));
        if (err) return false;
        PrepareLexer(lex);
        AST ast{};
        err = ast.analyze(lex.importedLibs, lex.globals, lex.functionSets, lex.structures, lex.ExternFunctions, lex.constants);
        if (err) return false;
        IRGenerator ir{};
        err = ir.analyze(ast.libs, ast.globalVars, ast.analyzed_functionSets, ast.sets, ast.structures, ast.ExtraFunctions, ast.constants);
        if (err) return false;
        writeFileString(args[2], ir.IR);
        return true;
    }
    else if (args.size() == 3 && args[0] == R("buildMEXE")) {
        workPath = getDictionary(args[1]) + R("\\");
        ByteArray<unsigned char> mexe;
		if (!IR2MEXE(readFileString(args[1]), mexe)) return false;
		writeFileByteArray(args[2], mexe);
		return true;	
    }
    else if (args.size() == 3 && args[0] == R("build")) {
        workPath = getDictionary(args[1]) + R("\\");
        Lexer lex{};
		bool err = lex.analyze(readFileString(args[1]));
		if (err) return false;
		PrepareLexer(lex);
		AST ast{};
		err = ast.analyze(lex.importedLibs, lex.globals, lex.functionSets, lex.structures, lex.ExternFunctions, lex.constants);
		if (err) return false;
		IRGenerator ir{};
		err = ir.analyze(ast.libs, ast.globalVars, ast.analyzed_functionSets, ast.sets, ast.structures, ast.ExtraFunctions, ast.constants);
		if (err) return false;
		ByteArray<unsigned char> mexe;
		err = !IR2MEXE(ir.IR, mexe);
		if (err) return false;
		writeFileByteArray(args[2], mexe);
		return true;
    }
    else if (args.size() == 1) {
        workPath = getDictionary(args[0]) + R("\\");
        Lexer lex{};
		bool err = lex.analyze(readFileString(args[0]));
		if (err) return false;
		PrepareLexer(lex);
		AST ast{};
		err = ast.analyze(lex.importedLibs, lex.globals, lex.functionSets, lex.structures, lex.ExternFunctions, lex.constants);
		if (err) return false;
		IRGenerator ir{};
		err = ir.analyze(ast.libs, ast.globalVars, ast.analyzed_functionSets, ast.sets, ast.structures, ast.ExtraFunctions, ast.constants);
		if (err) return false;
		ByteArray<unsigned char> mexe;
		err = !IR2MEXE(ir.IR, mexe);
        //std_lcout << ir.IR;
		if (err) return false;
#ifdef _WIN32
        x86Runner::LoadMEXE(mexe);
        x86Runner::run();
#endif // _WIN32
		return true;	
    }
}

int main(int argn, char* argv[]) {
#ifdef G_UNICODE_
    std::wcout.imbue(std::locale("zh_CN"));
#endif 
#if _DEBUG
    test(argn, argv);
#else
    if (argn == 1) {
        std::cout << "Usage:\n";
		std::cout << "runIR <IR file>\n";
		std::cout << "run <source file>\n";
		std::cout << "runMEXE <MEXE file>\n";
		std::cout << "buildIR <source file> <IR file>\n";
		std::cout << "buildMEXE <IR file> <MEXE file>\n";
		std::cout << "build <source file> <MEXE file>\n";
		return 0;
    }
    std::vector<lstring> args;
    for (int i = 1; i < argn; i++) {
        args.push_back(to_wide_string(argv[i]));
    }
    process_command(args);
#endif // _DEBUG
	return 0;
}