#pragma once
#include "../header/Common.h"
#include "../header/Tree.h"
#include "../header/ByteArray.h"
#include "../header/SectionManager.h"
#include "../header/AST.h"
#include "../header/Lexer.h"
using namespace MLang;
int main()
{
    std::vector<lstring> a = split(R("WTFWTFWTFF"), R("F"));
    std::cout << "" << std::endl;
    Lexer lex{};


    lex.analyze(R(
        "Main{\n"
        "main()->N:={\n"
        "print(&\"Hello World\")\n"
        "}\n"
        "}\n"
    ));
    AST ast{};

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
