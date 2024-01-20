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
    function func;
    func.name = name;
    func.args = args;
    func.ret = ret;
    func.local.clear();
    func.codes.clear();
    func.publiced = true;
    functionSet.func.push_back(func);
}
extern void cut_tokens(lstring code, std::vector<lstring>& tks);
int main()
{

    Tree<int> t;
    Lexer lex{};

#ifdef G_UNICODE_
	std::wcout.imbue(std::locale("zh_CN"));
#endif 

    lex.analyze(R(R"(

Class:Array{
	N:ptr;N:DimSize;N:DimPtr;N:typeSize;N:init;N:destroy;N:TotalSize;
	_init_()->N:={
		init=0;destroy=0;TotalSize=0;
		DimSize=0;typeSize=0;DimPtr=0;ptr=0;
	}.
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
		if(init!=0){
			while(i<size0){
				data_init(ptr+i*typeSize);i=i+1	
			}
		};
		
		return(true)
	}
	[Transit][ptr]data_init(N:ptr)->N:={return(init)}
	[Transit][ptr]data_destroy(N:ptr)->N:={return(destroy)}
	[Public][cdecl][ArgSize]"[]"(N:ArgSize)->N:={
		N:i=0;N:offset=0;
		while(i<ArgSize){
			N:index=(&ArgSize+(i+1)*sizeof:N)->N;
			offset=offset*(DimPtr+i*sizeof:N)->N+index;
			i=i+1;
		};printN(ptr);
		return(ptr+offset*typeSize);
	}
	Destroy()->N:={
		N:i=0;
		if(init!=0){
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
	[Public]_init_()->N:={
		print(&"init ");printN(&this);a=0;
	}
	[Public]_destroy_()->N:={
		print(&"destroy ");printN(&this);
	}
}
Main{
	
	main()->N:={
		Array:A.set_init_destroy(~A$_init_,~A$_destroy_);
		A.ReDim(sizeof:A,2,3);print(&"mmmmmmmm");
		A[1][2]->A.a=1;
		printN(A[1][2]->A.a);
	}
}

)"));

    NewType(lex.structures, R("N"), false, 8);
    NewType(lex.structures, R("Z"), false, 4);
    NewType(lex.structures, R("R"), false, 8);
    NewType(lex.structures, R("B"), false, 1);
    NewType(lex.structures, R("Boolen"), false, 4);

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
    if (!ir.analyze(ast.libs, ast.globalVars, ast.analyzed_functionSets, ast.sets, ast.structures, ast.ExtraFunctions, ast.constants)) {
        std_lcout << ir.IR << std::endl;
    }
#ifdef WIN32
    x86Generator x86{};
    x86.generate(ir.IR);

#endif // WIN32

}
