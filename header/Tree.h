﻿#pragma once
#include "Common.h"
template<typename T>class Tree {
private:
    std::vector<Tree> nodes{};
    T data{};
    intptr_t pointer{};
    bool located;
public:
    Tree() {
        located = true;
    }
    Tree(const Tree& o) {
        data = o.data;
        nodes = o.nodes;
        pointer = o.pointer;
        located = o.located;
    }
    Tree(Tree&& o) noexcept {
        data = std::move(o.data);
        nodes = std::move(o.nodes);
        pointer = o.pointer;
        located = o.located;
    }
    void operator =(const Tree& o) {
        nodes = o.nodes;
        data = o.data;
        located = o.located;
        pointer = o.pointer;
    }
    void operator =(Tree&& o) noexcept {
        data = std::move(o.data);
        nodes = std::move(o.nodes);
        pointer = o.pointer;
        located = o.located;
    }
    ~Tree() { data.~T(); }
    bool next() {
        Tree* c = LocateParentTree(nullptr);
        c->pointer++;
        if (c->pointer >= (intptr_t)c->nodes.size()) {
            c->pointer = c->nodes.size() - 1; return false;
        }
        c->nodes[pointer].located = true;
        return true;
    }
    Tree* LocateCurrentTree() {
        if (located) return this;
        assert(pointer < (intptr_t)nodes.size());
        return nodes[pointer].LocateCurrentTree();
    }
    Tree* LocateParentTree(Tree* parent) {
        if (located) return parent;
        assert(pointer < (intptr_t)nodes.size());
        return nodes[pointer].LocateParentTree(this);
    }
    T& Get() {
        return LocateCurrentTree()->data;
    }
    void reset() { located = true; }
    bool parent() {
        Tree* c = LocateParentTree(nullptr);
        if (!c) return false;
        c->reset(); return true;
    }
    bool haveParent() {
        return (bool)LocateParentTree(nullptr);
    }
    size_t size() {
        return nodes.size();
    }
    bool child() {
        Tree* c = LocateCurrentTree();
        if (!c->size()) return false;
        c->pointer = 0;
        c->located = false;
        c->nodes[pointer].located = true;
        return true;
    }
    bool insert(const T& o) {
        Tree* c = LocateCurrentTree();
        if (c->pointer > (intptr_t)c->nodes.size()) return false;
        Tree t;
        t.data = o;
        c->nodes.insert((c->nodes.begin() + (c->pointer > 0 ? c->pointer : 0)), t);
        return true;
    }
    bool insert(const T& o, size_t index) {
        Tree* c = LocateCurrentTree();
        if (index < 0 || index > c->nodes.size()) return false;
        Tree t;
        t.data = o;
        c->nodes.insert((c->nodes.begin() + index), t);
        return true;
    }
    bool insert(const Tree& o) {
        Tree* c = LocateCurrentTree();
        if (c->pointer > (intptr_t)c->nodes.size()) return false;
        c->nodes.insert((c->nodes.begin() + (c->pointer > 0 ? c->pointer : 0)), o);
        return true;
    }
    bool insert(const Tree& o, size_t index) {
        Tree* c = LocateCurrentTree();
        if (index < 0 || index > c->nodes.size()) return false;
        c->nodes.push_back((c->nodes.begin() + index), o);
        return true;
    }
    bool push_back(const T& o) {
        Tree* c = LocateCurrentTree();
        if (c->pointer > (intptr_t)c->nodes.size()) return false;
        Tree t;
        t.data = o;
        c->nodes.push_back(t);
        return true;
    }
    bool push_back(const T& o, size_t index) {
        Tree* c = LocateCurrentTree();
        if (index < 0 || index > c->nodes.size()) return false;
        Tree t;
        t.data = o;
        c->nodes.push_back(t);
        return true;
    }
    bool push_back(const Tree& o) {
        Tree* c = LocateCurrentTree();
        if (c->pointer > (intptr_t)c->nodes.size()) return false;
        c->nodes.push_back(o);
        return true;
    }
    bool push_back(const Tree& o, size_t index) {
        Tree* c = LocateCurrentTree();
        if (index < 0 || index > c->nodes.size()) return false;
        c->nodes.push_back(o);
        return true;
    }
    void operator = (const T& o) {
        LocateCurrentTree()->data = o;
    }
    T& operator [](size_t index) {
        Tree* c = LocateCurrentTree();
        assert(index < c->nodes.size());
        return c->nodes[index].data;
    }
    bool remove() {
        Tree* c = LocateParentTree();
        if (!c) return false;
        if (c->pointer < 0 || c->pointer >= (intptr_t)c->nodes.size()) return false;
        c->nodes.erase(c->nodes.begin() + c->pointer);
        parent();
        return true;
    }
    bool clear() {
        nodes.clear();
        pointer = -1;
        return true;
    }
    bool ToChildrenEnd() {
        if (!child()) return false;
        while (next());
        return true;
    }
};