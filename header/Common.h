#pragma once
#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <optional>
#include <vector>
#include <sstream>
#include <fstream>
#include <codecvt>

#define _UNICODE_ UNICODE

#ifdef _UNICODE_
typedef std::wstring lstring;
typedef std::wstringstream lstringstream;
typedef wchar_t lchar;
#define R(x) L##x
#define to_lstring(x) std::to_wstring(x)
#define std_lcout std::wcout

#else
typedef std::string lstring;
typedef char lchar;
typedef std::stringstream lstringstream;
#define R(x) x
#define to_lstring(x) std::to_string(x)
#define std_lcout std::cout
#endif //


namespace MLang {
    size_t max_(size_t A, size_t B);
    size_t min_(size_t A, size_t B);
    const lstring DIVISION = R("$");
    intptr_t search(std::vector<lstring>& tks, lstring str, int type, std::optional<intptr_t> begin, intptr_t offset);
    void SubTokens(std::vector<lstring>& tks, std::vector<lstring>& output, intptr_t start, intptr_t end);
    bool iftk(std::vector<lstring>& tks, lstring tk, intptr_t i);
    lstring process_quotation_mark(lstring tk);
    bool analyze_dims(std::vector<lstring>& tks, std::vector<size_t>& dims, intptr_t ip, intptr_t & final);
    bool IsOperator(lstring t, int type);
    lstring RemoveSpaceLR(lstring str);
    lstring subreplace(lstring resource_str, lstring sub_str, lstring new_str);
    bool isNum(lstring str);
    bool isNum_(lstring str);
    void output(std::vector<lstring> tk,lstring str,intptr_t pos);
    lstring gather(std::vector<lstring> tks, size_t c);
    lstring readFileString(lstring path);
    size_t DimSize(std::vector<size_t> dim);
    std::vector<lstring> split(lstring str, lstring str_0);
}