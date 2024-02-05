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
    sm.Ins(R("Redirect table"), data.build(), {});
    mexe = sm.build();
    return true;
#endif
}

int main(int argn,char* argv[])
{
    Lexer lex{};

#ifdef G_UNICODE_
	std::wcout.imbue(std::locale("zh_CN"));
#endif 
#if _DEBUG
    bool err = lex.analyze(R(R"(

Class:Array{
	N:ptr;
	N:DimSize;
	N:DimPtr;
	N:typeSize;
	N:init;
	N:destroy;
	N:TotalSize;
	_init_()->N:={
		init=0;destroy=0;TotalSize=0;
		DimSize=0;typeSize=0;DimPtr=0;ptr=0;
	}
	_destroy_()->N:={
		Destroy()
	}
	[Public]set_init_destroy(N:I,N:D)->N:={
		init=I;destroy=D
	}
	[Public][cdecl][ArgSize]ReDim(N:ArgSize,N:size)->Boolen:={
		Destroy();
		typeSize=size;
		DimPtr=new((ArgSize-1)*sizeof:N);
		N:i=0;N:size0=1;
		while(i<ArgSize-1){
			N:dim=(&size+(i+1)*sizeof:N)->N;
			(DimPtr+i*sizeof:N)->N=dim;
			size0=size0*dim;
			i=i+1;
		};
		TotalSize=size0;
		ptr=new(size0*size);
		i=0;
		if(init!=0 and ptr!=0){
			while(i<size0){
				data_init(ptr+i*typeSize);
                i=i+1	
			}
		};
		
		return(true)
	}
	[Transit][ptr]data_init(N:ptr)->N:={return(init)}
	[Transit][ptr]data_destroy(N:ptr)->N:={return(destroy)}
	[Public][cdecl][ArgSize]"[]"(N:ArgSize)->N:={		
        N:i=0;
        N:offset=0;
        while(i<ArgSize){
			N:index=(&ArgSize+(i+1)*sizeof:N)->N;
			offset=offset*(DimPtr+i*sizeof:N)->N+index;
			i=i+1;
		};
		return(ptr+offset*typeSize);
	}
	Destroy()->N:={
		N:i=0;
		if(destroy!=0 and ptr!=0){
			while(i<TotalSize){
				data_destroy(ptr+i*typeSize);i=i+1		
			}
		};
		DimSize=0;typeSize=0;
		if(DimPtr!=0){free(DimPtr)};
		if(ptr!=0){free(ptr)};
		DimPtr=0;ptr=0;
	}
}
Class:A{
	[Public]N:a;
	[Public]N:b;

	[Public]_init_()->N:={
        a=0;b=0;
		print(&"init ");printN(&this);print(&"
");
	}
	[Public]_destroy_()->N:={
		print(&"destroy ");printN(&this);print(&"
"); 
	}
}
Main{
    f()->A:={
		A:a;
        a.a=114;
		a.b=514;        
		return(a)
	}
	main()->N:={
		Array:A.set_init_destroy(~A$_init_,~A$_destroy_);
		A.ReDim(sizeof:A,2,3);
        A[0][0]->A=f();
        A[0][1]->A.a=2;
        A[0][2]->A.a=3;
        A[1][0]->A.a=4;
        A[1][1]->A.a=5;
        A[1][2]->A.a=6;
		printN(A[0][0]->A.a);
		printN(A[0][1]->A.b);
		printN(A[0][2]->A.a);
		printN(A[1][0]->A.b);
		printN(A[1][1]->A.a);
		printN(A[1][2]->A.b);
        print(&"
")
	}
}

)"));
#else
    if (argn != 2) return 0;
#ifdef G_UNICODE_
	lstring code = readFileString(to_wide_string(argv[1]));
#else
    lstring code = readFileString(argv[1]);
#endif
    bool err = lex.analyze(code);
#endif
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
