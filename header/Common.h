﻿#pragma once
#ifndef _COMMON_HEAD_
#define _COMMON_HEAD_
#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <optional>
#include <vector>
#include <sstream>
#include <fstream>
#include <codecvt>

#define G_X64_ _WIN64
#define G_UNICODE_ UNICODE

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#endif // _WIN32 || _WIN64


#ifdef G_UNICODE_
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

#if G_X64_
#define st_size_t(x) std::stoull(x)
#define st_intptr_t(x) std::stoll(x)
#else
#define st_size_t(x) std::stoul(x)
#define st_intptr_t(x) std::stol(x)
#endif // X64
#endif // !_COMMON_HEAD_

#include "ByteArray.h"

#define foreach(x, y) for(auto y=x.begin();y<x.end();y++)

#define RESET   R("\033[0m")
#define BLACK   R("\033[30m")      /* Black */
#define RED     R("\033[31m")      /* Red */
#define GREEN   R("\033[32m")      /* Green */
#define YELLOW  R("\033[33m")      /* Yellow */
#define BLUE    R("\033[34m")      /* Blue */
#define MAGENTA R("\033[35m")      /* Magenta */
#define CYAN    R("\033[36m")      /* Cyan */
#define WHITE   R("\033[37m")      /* White */
#define BOLDBLACK   R("\033[1m\033[30m")      /* Bold Black */
#define BOLDRED     R("\033[1m\033[31m")      /* Bold Red */
#define BOLDGREEN   R("\033[1m\033[32m")      /* Bold Green */
#define BOLDYELLOW  R("\033[1m\033[33m")      /* Bold Yellow */
#define BOLDBLUE    R("\033[1m\033[34m")      /* Bold Blue */
#define BOLDMAGENTA R("\033[1m\033[35m")      /* Bold Magenta */
#define BOLDCYAN    R("\033[1m\033[36m")      /* Bold Cyan */
#define BOLDWHITE   R("\033[1m\033[37m")      /* Bold White */

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
   
    // 默认情况：is_vector为false
    template<typename T,typename... Types>
    struct is_vector : std::false_type {};

    // 特化：为所有vector类型，is_vector为true
    template<typename T, typename... Types>
    struct is_vector<const std::vector<T, Types...>&> : std::true_type {};

    template<typename T, typename... Types>
    lstringstream DebugOutputString(const T& data, const Types&... args) {
        lstringstream ss;
        //ss << "Type:" << typeid(data).name();
        if constexpr (std::is_same<decltype(data), const ByteArray<unsigned char>&>::value) {
            ss << "size:" << data.size << "{";
            for (size_t i = 0; i < data.size; i++) {
                ss << (i ? R(", ") : R("")) << to_lstring((unsigned char)data.ptr[i]);
            }
            ss << "}";
        }
        else if constexpr (is_vector<decltype(data)>::value) {
            ss << "size:" << data.size() << "{";
            for (size_t i = 0; i < data.size(); i++) {
                ss << (i ? R(", ") : R("")) << DebugOutputString(data[i]).str();
            }
            ss << "}";
        }
        else {
            if constexpr (std::is_same<decltype(data), const lstring&>::value) {
				ss << R("\"") << data << R("\"");
			}
            else {
				ss << data;
			}
        }
        if constexpr (sizeof...(args)) {
            ss << R(" | ") << DebugOutputString(args...).str();
        }
        return ss;
    }

    template<typename T,typename... Types>
    void DebugOutput(const T& data, const Types&... args) {
        lstring tmp = DebugOutputString(data, args...).str();
#if defined(_WIN32) || defined(_WIN64)
        OutputDebugString(tmp.c_str());
        OutputDebugString(R("\n"));
#else
        std::cout << tmp << std::endl;
#endif
    }
}