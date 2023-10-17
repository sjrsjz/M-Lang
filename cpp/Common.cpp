#pragma once
#include "../header/Common.h"
size_t max_(size_t A, size_t B) {
	return A > B ? A : B;
}
size_t min_(size_t A, size_t B) {
	return A < B ? A : B;
}
lstring subreplace(lstring resource_str, lstring sub_str, lstring new_str)
{
    lstring dst_str = resource_str;
    lstring::size_type; intptr_t pos = 0;
    while ((pos = dst_str.find(sub_str)) != lstring::npos)   //替换所有指定子串
    {
        dst_str.replace(pos, sub_str.length(), new_str);
    }
    return dst_str;
}
bool IsOperator(lstring t, int type) {
    bool l{};
    l = t == R("+") || t == R("-") || t == R("*") || t == R("/") || t == R("\\") || t == R("%") || t == R("&") || t == R("!") || t == R("^") || t == R("~") || t == R("=") || t == R("==") || t == R(">") || t == R("<") || t == R("<=") || t == R(">=") || t == R("!=") || t == R("?=") || t == R("|") ||
        t == R("&&") || t == R(",") || t == R(".") || t == R("\n") || t == R(":") || t == R("->") || t == R("<<") || t == R(">>") || t == R("/*") || t == R("*/") || t == R(";") || t == R(" ") || t == R(":=");
    if (type == 0) {
        l |= t == R("(") || t == R(")") || t == R("[") || t == R("]") || t == R("{") || t == R("}");
    }
    return l;
}
lstring RemoveSpaceLR(lstring str) {
    int i = 0, j = str.length();
    while (i < j && (str.substr(i, 1) == R(" ") || str.substr(i, 1) == R("\t"))) i++;
    while (j > i && (str.substr(j, 1) == R(" ") || str.substr(j, 1) == R("\t"))) j--;
    return str.substr(i, j - i);
}
bool isNum(lstring str)
{
    lstringstream sin(str);
    double d;
    lchar c;
    if (!(sin >> d)) return false;
    if (sin >> c) return false;
    return true;
}

intptr_t search(std::vector<lstring>& tks, lstring str, int type, std::optional<intptr_t> begin, intptr_t offset) {
    if (!begin.has_value()) begin = type == 0 ? 0 : tks.size();
    intptr_t j{};
    for (int i = 1; i <= (type == 0 ? tks.size() - begin.value() : begin.value()); i++) {
        size_t p;
        if (type == 0)
            p = i + begin.value() - 1;
        else
            p = begin.value() - i ;
        if (tks[p] == R("(") || tks[p] == R("[") || tks[p] == R("{")) j++;
        if (tks[p] == R(")") || tks[p] == R("]") || tks[p] == R("}")) j--;
        if (tks[p] == str && j == offset) return p;
    }
    return -1;
}
void SubTokens(std::vector<lstring>& tks, std::vector<lstring>& output, intptr_t start, intptr_t end) {


}
bool iftk(std::vector<lstring>& tks, lstring tk, intptr_t i) {

}
lstring process_quotation_mark(lstring tk) {

}
bool analyze_dims(std::vector<lstring>& tks, std::vector<size_t>& dims, intptr_t ip, intptr_t & final) {


}

