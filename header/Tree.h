#pragma once
#include "Common.h"
template<typename T>class Tree {
private:
    std::vector<Tree> nodes{};
    T data{};
    intptr_t pointer{};
    bool located;
    void error() {
#if UNICODE
        std::wcerr << RED << R("[错误]") << CYAN << R("[树损坏]") << RESET << __LINE__ << std::endl;
#else
        std::cerr << RED << R("[错误]") << CYAN << R("[树损坏]") << RESET << __LINE__ << std::endl;
#endif // UNICODE

	}
public:
    Tree() {
        located = false;
        pointer = -1;
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
    ~Tree(){}
    bool next() {
        Tree* c = LocateParentTree(nullptr);
        Tree* curr = LocateCurrentTree();
        c->pointer++;
        if (c->pointer >= (intptr_t)c->nodes.size()) {
            c->pointer = c->nodes.size() - 1; return false;
        }
        if(curr) curr->located = false;
        c->nodes[c->pointer].located = true;
        return true;
    }
    Tree* LocateCurrentTree() {
        if (located) return this;
        if (pointer >= (intptr_t)nodes.size() || pointer<0) return nullptr;
        return nodes[pointer].LocateCurrentTree();
    }
    Tree* LocateParentTree(Tree* parent) {
        if (located) return parent;
        if(!(pointer>=0 && pointer < (intptr_t)nodes.size())) {error(); return nullptr;}
        return nodes[pointer].LocateParentTree(this);
    }
    T& Get() {
        T tmp;
        Tree* c = LocateCurrentTree();
        if (!c) return tmp;
        return c->data;
    }
    void reset() { located = true; }
    bool parent() {
        Tree* c = LocateParentTree(nullptr);
        if (!c) return false;
        Tree* curr = LocateCurrentTree();
        if (curr) curr->located = false;
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
        if (!c || !c->size()) return false;
        c->pointer = 0;
        c->located = false;
        if (!(c->pointer >= 0 && c->pointer < (intptr_t)c->nodes.size())) { error(); return false; }
        c->nodes[c->pointer].located = true;
        return true;
    }
    bool insert(const T& o) {
        Tree* c = LocateCurrentTree();
        if (!c) { pointer = 0; data = o; located = true; return true; }
        if (c->pointer > (intptr_t)c->nodes.size()) return false;
        Tree t;
        t.data = o;
        c->nodes.insert((c->nodes.begin() + (c->pointer > 0 ? c->pointer : 0)), t);
        return true;
    }
    bool insert(const T& o, size_t index) {
        Tree* c = LocateCurrentTree();
        if (!c) { pointer = 0; data = o; located = true; return true; }
        if (index < 0 || index > c->nodes.size()) return false;
        Tree t;
        t.data = o;
        c->nodes.insert((c->nodes.begin() + index), t);
        return true;
    }
    bool insert(const Tree& o) {
        Tree* c = LocateCurrentTree();
        if (!c) { *this = o; pointer = 0; located = true; return true; }
        if (c->pointer > (intptr_t)c->nodes.size()) return false;
        c->nodes.insert((c->nodes.begin() + (c->pointer > 0 ? c->pointer : 0)), o);
        return true;
    }
    bool insert(const Tree& o, size_t index) {
        Tree* c = LocateCurrentTree();
        if (!c) { pointer = 0; data = o; located = true; return true; }
        if (index < 0 || index > c->nodes.size()) return false;
        if (!c) { pointer = 0; data = o; located = true; return true; }
        c->nodes.push_back((c->nodes.begin() + index), o);
        return true;
    }
    bool push_back(const T& o) {
        Tree* c = LocateCurrentTree();
        if (!c) { nodes.clear(); pointer = 0; data = o; located = true; return true; }
        Tree t;
        t.data = o;
        c->nodes.push_back(t);
        return true;
    }
    bool push_back(const T& o, size_t index) {
        Tree* c = LocateCurrentTree();
        if (!c) { nodes.clear(); pointer = 0; data = o; located = true; return true; }
        if (index < 0 || index > c->nodes.size()) return false;
        Tree t;
        t.data = o;
        c->nodes.push_back(t);
        return true;
    }
    bool push_back(const Tree& o) {
        Tree* c = LocateCurrentTree();
        if (!c) { pointer = 0; nodes = o.nodes; located = true; return true; }
        c->nodes.push_back(o);
        return true;
    }
    bool push_back(const Tree& o, size_t index) {
        Tree* c = LocateCurrentTree();
        if (!c) { pointer = 0; nodes = o.nodes; located = true; return true; }
        if (index < 0 || index > c->nodes.size()) return false;
        c->nodes.push_back(o);
        return true;
    }
    void operator = (const T& o) {
        LocateCurrentTree()->data = o;
    }
    T& operator [](size_t index) {
        Tree* c = LocateCurrentTree();
        if (index >= c->nodes.size()) { error(); return nullptr; }
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
    size_t getDepth() {
        Tree* c = LocateCurrentTree();
		size_t depth = 0;
        while (c->haveParent()) {
			c = c->LocateParentTree(nullptr);
			depth++;
		}
		return depth;
    }

    void LocateCurrentTree (std::vector<size_t>& list) {
        if (located) return;
        if (pointer >= (intptr_t)nodes.size() || pointer < 0) return;
        list.push_back(pointer);
        nodes[pointer].LocateCurrentTree(list);
    }

    std::vector<size_t> getLocationList() {
        std::vector<size_t> list;
        LocateCurrentTree(list);
		return list;
    }
    bool setLocationList(const std::vector<size_t>& list) {
		Tree* c = this;
        c->located = false;
        for (size_t i = 0; i < list.size(); i++) {
			if (list[i] >= c->size()) return false;
			c = &c->nodes[list[i]];
            c->pointer=list[i];
            c->located = false;
		}
		c->located = true;
		return true;
	}
};