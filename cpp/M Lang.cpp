#include "../header/Common.h"
#include "../header/Tree.h"
#include "../header/ByteArray.h"
#include "../header/SectionManager.h"
#include "../header/AST.h"
#include "../header/Lexer.h"
using namespace MLang;
int main()
{
    Tree<int> t;
    Lexer lex{};

#ifdef G_UNICODE_
	std::wcout.imbue(std::locale("zh_CN"));
#endif 

    lex.analyze(R(R"(

Class:Array{
	//N:ptr;N:DimSize;N:DimPtr;N:typeSize;N:init;N:destroy;N:TotalSize;
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
	[Transit]data_init(N:ptr)->N:={return(init)}
	[Transit]data_destroy(N:ptr)->N:={return(destroy)}
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
    AST ast{};
    ast.analyze(lex.importedLibs, lex.globals, lex.functionSets, lex.structures, lex.ExternFunctions, lex.constants);
    /*SectionManager s;
    Tree<int> t;
    t = 1;
    t.insert(2);
    t.insert(3);
    t.insert(4);

    lstring a = RemoveSpaceLR(R("  A B  c "));
    std::cout << t.Get() << " " << t[0] << " " << t[1] << std::endl;

    t.child();
    t.insert(10);
    std::cout << t.Get() << std::endl;
    t.child();
    std::cout << t.Get() << std::endl;
    t.parent();

    t.next();
    std::cout << t.Get() << std::endl;
    t.next();
    std::cout << t.Get() << std::endl;
    t.parent();
    std::cout << t.Get() << std::endl;
    t.parent();
    std::cout << t.Get() << std::endl << std::endl;


    ByteArray<> A, B;
    A = A + R("TestA") + R("TestA");
    B = B + R("TestB");
    //B = A;

//    B.Get<double>(0) = 2.1;


    s.Ins(R("A"), A, {});
    s.Ins(R("B"), B, {});
    B = s.Get(R("B"));
    s.translate(s.build());
    A = s.Get(R("A"));
    B = s.Get(R("B"));

    std::cout << A.ToString() << " " << B.ToString() << std::endl;

    //std::cout <<B.Get<double>(0) << " " << B.Get<double>(8) << " " << A.Get<double>(0) << " " << A.Get<double>(8) ;
    */

}
