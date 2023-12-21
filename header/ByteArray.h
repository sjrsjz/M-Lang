#pragma once
#include "Common.h"
namespace MLang {
    template<typename class_T = char>
    class ByteArray {
    private:
        void Destroy() {
            if (!ptr) delete ptr;
            size = 0;
        }

    public:
        size_t size;
        char* ptr;

        ByteArray(const ByteArray& o) {
            size = o.size;
            ptr = new class_T[size];
            memcpy(ptr, o.ptr, size);
        }
        void operator =(const ByteArray& o) {
            Destroy();
            size = o.size;
            ptr = new class_T[size];
            memcpy(ptr, o.ptr, size);
        }
        ByteArray(ByteArray&& o) noexcept {
            size = o.size;
            ptr = o.ptr;
            o.size = 0;
            o.ptr = nullptr;
        }
        void operator =(ByteArray&& o) noexcept {
            size = o.size;
            ptr = o.ptr;
            o.size = 0;
            o.ptr = nullptr;
        }

        ByteArray() {
            ptr = nullptr;
            size = 0;
        }
        ByteArray(const size_t length) {
            size = length;
            assert((int)(size >= 0));
            ptr = new char[size];
        }
        ~ByteArray() {
            Destroy();
        }
        ByteArray Attach(const ByteArray& o) {
            ByteArray t(size + o.size);
            if (ptr) memcpy(t.ptr, ptr, size);
            if (o.ptr) memcpy((void*)((size_t)t.ptr + size), o.ptr, o.size);
            return t;
        }
        ByteArray Attach(const std::string& o) {
            ByteArray t(size + o.length());
            if (ptr) memcpy(t.ptr, ptr, size);
            if (o.c_str()) memcpy((void*)((size_t)t.ptr + size), o.c_str(), o.size());
            return t;
        }
        ByteArray Attach(const std::wstring& o) {
            ByteArray t(size + o.length());
            if (ptr) memcpy(t.ptr, ptr, size);
            if (o.c_str()) memcpy((void*)((size_t)t.ptr + size), o.c_str(), o.size() * sizeof(wchar_t));
            return t;
        }
        template<typename T>void push_back(const T& o) {
            *this = Attach(o);
        }
        template<typename T>ByteArray Attach(const T& o) {
            ByteArray t(size + sizeof(T));
            if (ptr) memcpy(t.ptr, ptr, size);
            memcpy((void*)((size_t)t.ptr + size), &o, sizeof(T));
            return t;
        }
        template<typename T>ByteArray operator +(const T& o) {
            return Attach(o);
        }
        template<typename T>void operator +=(const T& o) {
            push_back(o);
        }
        ByteArray& operator <<(unsigned char t) {
            push_back(t);
            return &(*this);
        }
        template<typename T> T& Get(size_t index) {
            return *((T*)((size_t)ptr + index));
        }
        template<typename T> void Set(size_t index, T& o) {
            memcpy((void*)((size_t)ptr + index), &o, sizeof(T));
        }
        class_T& operator [](size_t index) {
            return *((class_T*)((size_t)ptr + index));
        }
        template<typename T> T& operator [](size_t index) {
            return Get<T>(index);
        }
        ByteArray SubByteArray(size_t offset, size_t length) {
            ByteArray t(length);
            memcpy(t.ptr, (void*)((size_t)ptr + offset), t.size);
            return t;
        }
        ByteArray Replace(size_t offset, const ByteArray& o, std::optional<size_t> length) {
            size_t length0 = length.has_value() ? min_(o.size, length.value()) : o.size;
            ByteArray t(length + max_(0, offset + length0 - size));
            if (ptr) memcpy(t.ptr, ptr, size);
            if (o.ptr) memcpy(t.ptr + offset, o.ptr, length0);
            return t;
        }
        std::string ToString() {
            std::string t;
            t.resize(size);
            memcpy((void*)t.data(), (void*)ptr, size);
            return t;
        }
        std::wstring ToWString() {
            std::wstring t;
            t.resize(size);
            memcpy((void*)t.data(), (void*)ptr, size);
            return t;
        }
        lstring TolString() {
            lstring t;
            t.resize(size);
            memcpy((void*)t.data(), (void*)ptr, size);
            return t;
        }
        ByteArray insert(size_t offset, const ByteArray& o) {
			ByteArray t(size + o.size);
            if (ptr) {
                memcpy(t.ptr, ptr, offset);
                memcpy(t.ptr + o.size, ptr + offset, size - offset);
            }
			if (o.ptr) memcpy(t.ptr + offset, o.ptr, o.size);
			return t;
		}
        bool selfInsert(size_t offset, const ByteArray& o) {
            ByteArray t(size + o.size);
            if (ptr) {
                memcpy(t.ptr, ptr, offset);
                memcpy(t.ptr + o.size, ptr + offset, size - offset);
            }
            if (o.ptr) memcpy(t.ptr + offset, o.ptr, o.size);
            std::swap(*this, t);
            return true;
        }
    };
}
