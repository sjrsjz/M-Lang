#pragma once
#include "../header/Common.h"

namespace MLang {
    void output(lstring str) {
        printf((char*)(str + R("\n")).c_str());
    }
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
        int i = 0, j = str.length() - 1;
        while (i < j && (str.substr(i, 1) == R(" ") || str.substr(i, 1) == R("\t"))) i++;
        while (j > i && (str.substr(j, 1) == R(" ") || str.substr(j, 1) == R("\t"))) j--;
        return str.substr(i, j - i + 1);
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
        if (!tks.size()) return -1;
        if (!begin.has_value()) begin = type == 0 ? 0 : tks.size();
        intptr_t j{};
        for (int i = 1; i <= (type == 0 ? tks.size() - begin.value() : begin.value()); i++) {
            size_t p;
            if (type == 0)
                p = i + begin.value() - 1;
            else
                p = begin.value() - i;
            if (tks[p] == R("(") || tks[p] == R("[") || tks[p] == R("{")) j++;
            if (tks[p] == R(")") || tks[p] == R("]") || tks[p] == R("}")) j--;
            if (tks[p] == str && j == offset) return p + 1;
        }
        return -1;
    }
    void SubTokens(std::vector<lstring>& tks, std::vector<lstring>& output, intptr_t start, intptr_t end) {
        std::vector<lstring> t;
        if (end < start) {
            output.clear(); return;
        }
        if (end > tks.size()) {
            end = tks.size();
        }
        if (start < 1) start = 1;
        t = tks;
        t.erase(t.begin() + end, t.end());
        if (start > 1) t.erase(t.begin(), t.begin() + start - 1);
        output = t;
    }
    bool iftk(std::vector<lstring>& tks, lstring tk, intptr_t i) {
        return i >= 1 && i <= tks.size() && tks[i - 1] == tk;
    }
    lstring process_quotation_mark(lstring tk) {
        lstring t = tk;
        if (t.substr(0, 1) == R("\"")) t = t.substr(1, t.length() - 1);
        if (t.substr(t.length() - 1, 1) == R("\"")) t = t.substr(0, t.length() - 1);
        return t;
    }
    bool analyze_dims(std::vector<lstring>& tks, std::vector<size_t>& dims, intptr_t ip, intptr_t & final) {
        dims.clear();
        while (iftk(tks, R("["), ip) && iftk(tks, R("]"), ip + 2))
        {
            dims.push_back(std::stoi(tks[ip]));
            ip += 3;
        }
        final = ip;
        return dims.size() > 0;
    }
    lstring gather(std::vector<lstring> tks, size_t layer) {
        size_t layer0 = layer;
        lstring head;
        return head;
    }

    //convert string to wstring
    inline std::wstring to_wide_string(const std::string& input)
    {
#pragma warning(suppress : 4996)
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
#pragma warning(suppress : 4996)
        return converter.from_bytes(input);
    }

    //convert wstring to string 
    inline std::string to_byte_string(const std::wstring& input)
    {
#pragma warning(suppress : 4996)
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
#pragma warning(suppress : 4996)
        return converter.to_bytes(input);
    }
    lstring readFileString(lstring path) {
        std::ifstream fin{};
        fin.open(path, std::ios::in);
        if (!fin.is_open()) return R("");
        std::string buf;
        lstring buf2{};
        while (std::getline(fin, buf)) {
#ifdef  UNICODE
            buf2 += to_wide_string(buf);
#else
            buf2 += buf;
#endif //  UNICODE
        }
        fin.close();
        return buf2;
    }
    size_t DimSize(std::vector<size_t> dim) {
        size_t j = 1;
        for (int i = 0; i < dim.size(); i++) j *= dim[i];
        return j;
    }
    std::vector<lstring> split(lstring str, lstring str_0) {
        std::vector<lstring> tmp{};
        size_t p{};
        size_t lp{};
        size_t off = str_0.length();
        while ((p = str.find(str_0, p)) != std::string::npos)
        {
            tmp.push_back(str.substr(lp, p - lp));
            p += off;
            lp = p;
        }
        tmp.push_back(str.substr(lp, str.size() - lp + 1));
        return tmp;
    }
}
