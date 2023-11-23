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
    Tree<int> t;
    std::vector<lstring> a = split(R("WTFWTFWTFF"), R("F"));
    std::cout << "" << std::endl;
    Lexer lex{};


    lex.analyze(R(R"(
        _string_{
            R2Str(R:data)->string:= {
                string:tmp;
                tmp.size = 0;
                tmp.ad = new(64);
                R2T(data,tmp.ad);
                while (N(tmp.ad + tmp.size)->B != 0) { tmp.size = tmp.size + 1 };
                tmp.size = tmp.size + 1;
                return(tmp)
            }
        }
    Class:string{
        [Public] N:ad;
        [Public] N:size;
        _init_()->N:= {
            ad = 0; size = 0
        }
        _destroy_()->N := {
            if (ad != 0) { free(ad); ad = 0 };
            size = 0;
        }
        _string_: = {
            N:ad,N : size
        }
        [Public]_return_string(string : s)->N := {//prevent RAII destroys data
        ErrMark(&"String$_return_string");
            _destroy_();
            ad = new(s.size);
            size = s.size;
            memcopy(s.ad,ad,size)
        }
        [Public] = (string:s)->string:= {
        ErrMark(&"String$=");
            _destroy_();
            ad = new(s.size);
            size = s.size;
            memcopy(s.ad,ad,s.size);
            return(this)
        }
        [Public]const(N:str)->N:= {
        ErrMark(&"String$const");
            _destroy_();
            N:i = 0;
            while (N(str + i)->B != 0) { i = i + 1 };
            size = i + 1;
            ad = new(size);
            memcopy(str,ad,size);
        }
        [Public] + (string:s)->string:= {
        ErrMark(&"String$+");
            string:tmp;
            if (this.size != 0) {
                tmp.size = s.size + size - 1;
                tmp.ad = new(tmp.size);
                memcopy(ad,tmp.ad,size);
                memcopy(s.ad,tmp.ad + size - 1,s.size);
            } {
                tmp = s;
            };
            return(tmp)
        }
        [Public] * (N:times)->string:= {
        ErrMark(&"String$*");
            string:tmp;
            N:i = 0;
            while (i < times) {
                tmp = tmp + this;
                i = i + 1
            };
            return(tmp)
        }
        [Public]ToR()->R:= {
            return(T2R(ad))
        }
        [Public] == (string : s)->Boolen := {
            return(CmpStr(ad,s.ad))
        }
        [Public] != (string : s)->Boolen := {
            return(not CmpStr(ad,s.ad))
        }
                }
                    Main{
            N:test;
                [Transit] func0(string:str, N : n)->string:= { return(test) }
                    func1(string : str, N : n)->string := { return(str * n) }
                    main()->N := {
                        string:str1;
                        string:str2;
                        string:str3;
                        str1.const(&"Hello ");
                        str2.const(&"World");
                        str3.const(&"! ");
                        str2 = str1 + str2 + str3 + R2Str(100);
                        str3.const(&"Hello World! 100");
                        print(str2.ad);
                        if (str2 == str3) {
                            print(&" Equal");
                        } {
                            print(&" Different")
                        };
                        str1.const(&"
                ");
                        str3 = str1 + str3;
                        test = ~func1;
                        str3 = func0(str3,10);
                        print(str3.ad);

                        B:buffer[1024];

                        input(&buffer,1024);
                            }
            })"
    ));
    AST ast{};
    ast.analyze(lex.importedLibs, lex.globals, lex.functionSets, lex.structures, lex.ExternFunctions, lex.constants);
    std::cout << "s";
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
