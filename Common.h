#pragma once
#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <optional>
#include <vector>

#ifdef UNICODE
typedef std::wstring lstring;
typedef wchar_t lchar;
#define R(x) L##x
#else
typedef std::string lstring;
typedef char lchar;
#define R(x) x
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

}