﻿#include "../header/base64.h"
static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

std::string base64_encode_(const std::string& input) {
    std::string encoded;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    for (const auto& char_to_encode : input) {
        char_array_3[i++] = char_to_encode;
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
                encoded += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; (j < i + 1); j++)
            encoded += base64_chars[char_array_4[j]];

        while ((i++ < 3))
            encoded += '=';
    }

    return encoded;
}

std::string base64_decode_(const std::string& input) {
    int in_len = input.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string decoded;

    while (in_len-- && (input[in_] != '=') && (is_base64(input[in_]))) {
        char_array_4[i++] = input[in_]; in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                decoded += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

        for (j = 0; (j < i - 1); j++) decoded += char_array_3[j];
    }

    return decoded;
}

bool is_base64(lchar c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}
lstring base64_encode(const lstring& input) {
#if G_UNICODE_
    std::string tmp{};
    tmp.resize(input.size() * sizeof(wchar_t));
    memcpy((void*)tmp.c_str(), (void*)input.c_str(), input.size() * sizeof(wchar_t));
    return MLang::to_wide_string(base64_encode_(tmp));
#else
    return base64_encode_(input);
#endif
}
lstring base64_decode(const lstring& input) {
#if G_UNICODE_
    std::string tmp = base64_decode_(MLang::to_byte_string(input));
    std::wstring tmp2{};
    tmp2.resize(tmp.size() / sizeof(wchar_t));
    memcpy((void*)tmp2.c_str(), (void*)tmp.c_str(), tmp.size());
    return tmp2;
#else
    return base64_decode_(input);
#endif
}
