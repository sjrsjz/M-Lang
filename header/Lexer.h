#pragma once
#include "Common.h"
#include "Struct.h"
namespace MLang {
    class Lexer {
        void preprocess(std::vector<lstring>& tk);
    public:
        std::vector<lstring> importedLibs{};
        std::vector<type> globals{};
        std::vector<functionSet> functionSets{};
        std::vector<structure> structures{};
        std::vector<type> constants{};
        functionSet ExternFunctions{};
        Lexer();
        ~Lexer();
        bool lexical_analyze(functionSet& set, lstring space, std::vector<lstring> tks);
        intptr_t analyze_struct(functionSet& set, lstring space, std::vector<lstring> tks, size_t ip);
        intptr_t analyze_function(functionSet& set, lstring space, std::vector<lstring> tks, size_t ip);
        intptr_t analyze_externedFunction(functionSet& set, lstring DLL, std::vector<lstring> tks, size_t ip);
        intptr_t analyze_setVars(functionSet& set, lstring space, std::vector<lstring> tks, size_t ip);
        bool analyze_vars(std::vector<lstring> tks, type& var);
        intptr_t analyze_externedFunctionSet(std::vector<lstring> tks, size_t ip);
        intptr_t analyze_functionSet(std::vector<lstring> tks, size_t ip);
        intptr_t analyze_globalVars(std::vector<lstring> tks, size_t ip);
        bool analyze(lstring code);
        bool analyze_type(std::vector<lstring>& tk, type var);
        void preprocesser(std::vector<lstring>& tk);
    };
}