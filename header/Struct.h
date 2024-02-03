#pragma once
#include "Common.h"
#include "Tree.h"
struct code {
    std::vector<lstring> tokens;
};
struct OP {
    std::vector<lstring> op;
};

struct type {
    lstring name;
    lstring typeName;
    std::vector<size_t> dim;
    bool address;
    bool fixed;
    bool array;
    bool can_be_ignored;
    bool publiced;
    size_t id;
    lstring data;
};
struct function {
    lstring name;
    std::vector<code> codes;
    lstring extra_name;
    std::vector<type> args;
    std::vector<type> local;
    type ret;
    bool api;
    bool publiced;
    lstring DLL;
    bool externed;
    lstring call_type;
    bool transit;
    lstring transitArg;
    bool use_arg_size;
};
struct structure {
    lstring name;
    std::vector<type> elements;
    bool publiced;
    size_t size;
    bool isClass;
};
struct functionSet {
    lstring name;
    std::vector<type> local;
    std::vector<function> func;
    lstring base;
    bool publiced;
    bool isClass;
};
struct dim {
    std::vector<lstring> tk;
};
struct node {
    lstring token;
    lstring type;
};

struct analyzed_function {
    lstring name;
    std::vector<Tree<node>> codes;
    lstring extra_name;
    std::vector<type> args;
    std::vector<type> local;
    type ret;
    bool api;
    bool publiced;
    lstring DLL;
    bool externed;
    lstring call_type;
    bool transit;
    lstring transitArg;
    bool use_arg_size;
};
struct analyzed_functionSet {
    lstring name;
    std::vector<type> local;
    std::vector<analyzed_function> func;
    lstring base;
    bool publiced;
    bool isClass;
    size_t size;
};
struct redirection {
    size_t ip;
    intptr_t line;
    lstring name;
};