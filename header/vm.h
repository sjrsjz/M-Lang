#pragma once
#include <algorithm>
#include "Common.h"
#include "caller.h"
namespace MLang {
	class VM {
#define MAX_UNFREE_STACK_SIZE 0x1000
#define ADD 0x00
#define SUB 0x01
#define MUL 0x02
#define DIV 0x03
#define MOD 0x04
#define AND 0x05
#define OR 0x06
#define XOR 0x07
#define NOT 0x08
#define SHL 0x09
#define SHR 0x0A
#define CMP 0x0B
#define BIG 0x0C
#define BIGE 0x0D
#define SMALL 0x0E
#define SMALLE 0x0F
#define EQU 0x10
#define NEQU 0x11
#define ABS 0x12
#define MINUS 0x13
		struct virtualFunctionTable {
			void* functionAddress;
			size_t functionID;
			unsigned char* argBuffer;
			unsigned char* retBuffer;
			std::vector<enum argType> argTypes;
			enum argType retType;
		};
	private:
		unsigned char* stack{};
		unsigned char* code{};
		unsigned char* ip{};
		unsigned char* sp{};
		unsigned char* bp{};
		unsigned __int64 regA64{}, regB64{}, regC64{}, regD64{};
		unsigned __int32 regA32{}, regB32{}, regC32{}, regD32{};
		unsigned __int16 regA16{}, regB16{}, regC16{}, regD16{};
		unsigned __int8 regA8{}, regB8{}, regC8{}, regD8{};
		unsigned char** tmpStack{};
		size_t tmpStackSize{};
		size_t allocedTmpStackSize{};
		unsigned char* global{};
		size_t this_ptr{};
		std::vector<virtualFunctionTable> VirtualFunctionTable{};
		std::vector<size_t> VirtualFunctionTableID{};
	public:
		VM();
		~VM();
		void run();
#define allocStack(size) sp -= size
#define freeStack(size) sp += size
		void newVirtualFunction(size_t uid, size_t functionBaseOffset,const std::vector<enum argType>& argTypes, const enum argType& retType);

		inline unsigned char* getTmp(size_t tmpID) {
			return tmpStack[tmpID];
		}
		inline void num64(unsigned __int64 num, size_t tmpID) {
			*(unsigned __int64*)getTmp(tmpID) = num;
		}
		inline void num32(unsigned __int32 num, size_t tmpID) {
			*(unsigned __int32*)getTmp(tmpID) = num;
		}
		inline void num16(unsigned __int16 num, size_t tmpID) {
			*(unsigned __int16*)getTmp(tmpID) = num;
		}
		inline void num8(unsigned __int8 num, size_t tmpID) {
			*(unsigned __int8*)getTmp(tmpID) = num;
		}
		inline void tmpBegin() {
			if (tmpStack && allocedTmpStackSize >MAX_UNFREE_STACK_SIZE) {
				delete[] tmpStack;
				tmpStack = new unsigned char* [MAX_UNFREE_STACK_SIZE];
			}
			else if (!tmpStack) {
				tmpStack = new unsigned char* [MAX_UNFREE_STACK_SIZE];
			}
			tmpStackSize = 0;
			allocedTmpStackSize = MAX_UNFREE_STACK_SIZE;
		}
		inline void tmpEnd() {
		}
		inline void allocTmp(size_t size) {
			if (tmpStackSize >= allocedTmpStackSize) {
				unsigned char** newTmpStack = new unsigned char* [allocedTmpStackSize + MAX_UNFREE_STACK_SIZE];
				for (size_t i = 0; i < allocedTmpStackSize; i++) {
					newTmpStack[i] = tmpStack[i];
				}
				delete[] tmpStack;
				tmpStack = newTmpStack;
				allocedTmpStackSize += MAX_UNFREE_STACK_SIZE;
			}
			tmpStack[tmpStackSize] = new unsigned char[size];
			tmpStackSize++;
		}

		inline void mov(size_t dst, size_t src, size_t size) {
			for (size_t i = 0; i < size; i++) {
				*(getTmp(dst) + i) = *(getTmp(src) + i);
			}
		}
		inline size_t addressOfTmp(size_t tmpID) {
			return (size_t)getTmp(tmpID);
		}
		template<typename T>
		inline void opNum(size_t dst,size_t A,size_t B,int mode) {
			T* dst_ptr = (T*)getTmp(dst);
			T* A_ptr = (T*)getTmp(A);
			T* B_ptr = (T*)getTmp(B);
			switch (mode)
			{
			case ADD:
				*dst_ptr = *A_ptr + *B_ptr; break;
			case SUB:
				*dst_ptr = *A_ptr - *B_ptr; break;
			case MUL:
				*dst_ptr = *A_ptr * *B_ptr; break;
			case DIV:
				*dst_ptr = *A_ptr / *B_ptr; break;
			case SHL:
				*dst_ptr = *A_ptr << *B_ptr; break;
			case SHR:
				*dst_ptr = *A_ptr >> *B_ptr; break;
			case AND:
				*dst_ptr = *A_ptr & *B_ptr; break;
			case OR:
				*dst_ptr = *A_ptr | *B_ptr; break;
			case XOR:
				*dst_ptr = *A_ptr ^ *B_ptr; break;
			case NOT:
				*dst_ptr = ~*A_ptr; break;
			case MOD:
				*dst_ptr = *A_ptr % *B_ptr; break;
			case BIG:
				*dst_ptr = (T)(*A_ptr > *B_ptr); break;
			case BIGE:
				*dst_ptr = (T)(*A_ptr >= *B_ptr); break;
			case SMALL:
				*dst_ptr = (T)(*A_ptr < *B_ptr); break;
			case SMALLE:
				*dst_ptr = (T)(*A_ptr <= *B_ptr); break;
			case EQU:
				*dst_ptr = (T)(*A_ptr == *B_ptr); break;
			case NEQU:
				*dst_ptr = (T)(*A_ptr != *B_ptr); break;
			case ABS:
				*dst_ptr = (T)abs(*A_ptr); break;
			case MINUS:
				*dst_ptr = (T)-*A_ptr; break;
			default:
				break;
			}
		}
		inline void jmp(size_t offset) {
			ip = code + offset;
		}
		inline void jz(size_t A,size_t offset) {
			float data = *(float*)getTmp(A);
			if (data == 0.0f || data != 1.0f && data <= float(rand()) / RAND_MAX) {
				ip = code + offset;
			}
		}
		inline void load(size_t dst, size_t src, size_t size) {
			switch (size)
			{
			case 1:
				*(unsigned char*)getTmp(dst) = *((unsigned char*)*(size_t*)getTmp(src)); break;
			case 2:
				*(unsigned short*)getTmp(dst) = *((unsigned short*)*(size_t*)getTmp(src)); break;
			case 4:
				*(unsigned int*)getTmp(dst) = *((unsigned int*)*(size_t*)getTmp(src)); break;
			case 8:
				*(unsigned __int64*)getTmp(dst) = *((unsigned __int64*)*(size_t*)getTmp(src)); break;
			default:
				for (size_t i = 0; i < size; i++) {
					*(getTmp(dst) + i) = *((unsigned char*)*(size_t*)getTmp(dst) + i);
				}
				break;
			}			
		}
		inline void store(size_t dst, size_t src, size_t size) {
			switch (size)
			{
			case 1:
				*((unsigned char*)*(size_t*)getTmp(dst)) = *(unsigned char*)getTmp(src); break;
			case 2:
				*((unsigned short*)*(size_t*)getTmp(dst)) = *(unsigned short*)getTmp(src); break;
			case 4:
				*((unsigned int*)*(size_t*)getTmp(dst)) = *(unsigned int*)getTmp(src); break;
			case 8:
				*((unsigned __int64*)*(size_t*)getTmp(dst)) = *(unsigned __int64*)getTmp(src); break;
			default:
				for (size_t i = 0; i < size; i++) {
					*((unsigned char*)*(size_t*)getTmp(dst) + i) = *(getTmp(src) + i);
				}
				break;
			}
		}
		inline void addressOfLocalVar(size_t A,intptr_t offset) {
			*(size_t*)getTmp(A) = (size_t)(bp - offset);
		}
		inline void addressOfArg(size_t A, intptr_t offset) {
			*(size_t*)getTmp(A) = (size_t)(bp + offset + sizeof(size_t) * 2);
		}
		inline void call(intptr_t offset) {
			allocStack(sizeof(size_t));
			*(size_t*)sp = (size_t)ip;
			ip += offset;
		}
		inline void enter(intptr_t size) {
			allocStack(sizeof(size_t));
			*(size_t*)sp = (size_t)bp;
			bp = sp;
			allocStack(size);
		}
		inline void enter() {
			allocStack(sizeof(size_t));
			*(size_t*)sp = (size_t)bp;
			bp = sp;
		}
		inline void local(size_t size) {
			allocStack(size);
		}
		inline void leave() {
			sp = bp;
			bp = (unsigned char *)*(size_t*)sp;
			freeStack(sizeof(size_t));
		}
		inline void ret() {
			ip = (unsigned char*)*(size_t*)sp;
			freeStack(sizeof(size_t));
		}
		inline void ret(size_t size) {
			ip = (unsigned char*)*(size_t*)sp;
			freeStack(sizeof(size_t));
			freeStack(size);
		}
		inline void freeGlobal() {
			if (global) {
				delete[] global;
				global = nullptr;
			}
		}
		inline void allocGlobal(size_t size) {
			freeGlobal();
			global = new unsigned char[size];
		}
		inline void addressOfGlobal(size_t A,size_t offset) {
			*(size_t*)getTmp(A) = (size_t)global + offset;
		}
		inline void storeQ(size_t A) {
			regA64 = *(unsigned __int64*)getTmp(A);
		}
		inline void loadQ(size_t A) {
			*(unsigned __int64*)getTmp(A) = regA64;
		}
		inline void storeThisArg(size_t A, size_t argOffset) {
			*(size_t*)getTmp(A) = *(size_t*)(bp + argOffset);
		}
		inline void loadThis(size_t A) {
			*(size_t*)this_ptr = *(size_t*)getTmp(A);
		}
		inline void getVirtualFunctionAddress(size_t A, size_t FuncitonID) {
			
		}
		void VMInterfaceFunction(size_t uid, void*, void*, const std::vector<argType>&, const enum argType);
	};
}