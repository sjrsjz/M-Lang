#include "../header/caller.h"

using namespace MLang;

#define X64_MOV_RAX 0x48 << 0xB8
#define X64_MOV_RBX 0x48 << 0xBB
#define X64_MOV_RCX 0x48 << 0xB9
#define X64_MOV_RDX 0x48 << 0xBA
#define X64_MOV_R8 73 << 184
#define X64_MOV_R9 73 << 185
#define X64_MOV_PTR_RAX_RAX 0x48 << 0x89 << 0x00
#define X64_MOV_PTR_RAX_RBX 0x48 << 0x89 << 0x18
#define X64_MOV_PTR_RAX_RCX 0x48 << 0x89 << 0x08
#define X64_MOV_PTR_RAX_RDX 0x48 << 0x89 << 0x10
#define X64_MOV_PTR_RAX_R8 0x4C << 0x89 << 0x00
#define X64_MOV_PTR_RAX_R9 0x4C << 0x89 << 0x08
#define X64_MOV_PTR_RAX_XMM(id) 0x66 << 0x0F << 0x7E << id * 8
#define X64_MOV_RAX_STACKARG 72 << 137 << 132 << 36
#define X64_MOV_RBX_STACKARG 72 << 139 << 132 << 36
#define X64_PUSH_RAX 80
#define X86_64_CALL_RAX 255 << 16
#define X64_MOV_XMM_PTR_RAX(id) 0xF3 << 0x0F << 0x11 << id * 8
#define X64_MOV_RAX_PTR_RAX 0x48 << 0x8B << 0x00
#define X86_64_RET 195
#define X86_64_RETN 194
#define X86_PUSH 104
#define ARMv7_MOV_R0 0x04 << 0x00


ByteArray<unsigned char>
MLang::BuildVMInterfaceFunction(const void (*function)(void*, const void* , const std::vector<enum argType>&, const enum argType),
	void* argBuffer, const void* retBuffer, const std::vector<argType>& argType, const enum argType retType, bool cdeclmode) {
	
	ByteArray<unsigned char> ret{};
	size_t offset{};
	size_t arg_offset = (size_t)argBuffer;
	size_t arg_index{};
#ifdef _WIN32
	size_t argSize{};
	for (const auto& x : argType) argSize += argTypeSize[x];
	for (size_t i = 0; i < argSize; i++) {
		ret << 103 << 138 << 133; ret += (int)(i + sizeof(int)); ret << 136 << 4 << 37; ret += (int)(arg_offset + i);
	}
	ret << X86_PUSH; ret += (int)retType;
	ret << X86_PUSH; ret += (int)(size_t)&argType;
	ret << X86_PUSH; ret += (int)(size_t)retBuffer;
	ret << X86_PUSH; ret += (int)(size_t)argBuffer;
	ret << 184; ret += (int)(size_t)function;

#endif
#if defined(_WIN64)
	for (const auto& x : argType)
	{

#ifdef _WIN64
		arg_offset += ARG_MAX_SIZE;
		ret << X64_MOV_RAX;
		ret += (__int64)arg_offset;
		switch (x)
		{
		case argType::ARG_INT: case argType::ARG_BOOL: case argType::ARG_CHAR: case argType::ARG_LONG:
		case argType::ARG_LONGLONG: case argType::ARG_SHORT: case argType::ARG_UINT: case argType::ARG_ULONG:
		case argType::ARG_ULONGLONG: case argType::ARG_USHORT:
				switch (arg_index) {
					case 0:ret << X64_MOV_PTR_RAX_RCX; break; 
					case 1:ret << X64_MOV_PTR_RAX_RDX; break;
					case 2:ret << X64_MOV_PTR_RAX_R8; break;
					case 3:ret << X64_MOV_PTR_RAX_R9; break;
					default:ret << X64_MOV_RBX_STACKARG; ret += (__int64)(1 * sizeof(size_t) + offset); ret << X64_MOV_PTR_RAX_RBX;
						offset += argTypeSize[x];
						break;
				}
				break;
		case argType::ARG_FLOAT: case argType::ARG_DOUBLE:
				switch (arg_index) {
					case 0:ret << X64_MOV_PTR_RAX_XMM(0); break;
					case 1:ret << X64_MOV_PTR_RAX_XMM(1); break;
					case 2:ret << X64_MOV_PTR_RAX_XMM(2); break;
					case 3:ret << X64_MOV_PTR_RAX_XMM(3); break;
					default:ret << X64_MOV_RBX_STACKARG; ret += (__int64)(1 * sizeof(size_t) + offset); ret << X64_MOV_PTR_RAX_RBX;
						offset += argTypeSize[x];
						break;
				}
				break;
		}
#endif // _WIN64	
		arg_index++;
	}
#endif
#ifdef _WIN64
	ret << X64_MOV_RCX;
	ret += (__int64)argBuffer;
	ret << X64_MOV_RDX;
	ret += (__int64)retBuffer;
	ret << X64_MOV_R8;
	ret += (__int64)&argType;
	ret << X64_MOV_R9;
	ret += (__int64)retType;
	ret << X64_MOV_RAX;
	ret += (__int64)function;
	ret << X86_64_CALL_RAX;
	switch (retType)
	{
	case argType::ARG_INT: case argType::ARG_BOOL: case argType::ARG_CHAR: case argType::ARG_LONG:
	case argType::ARG_LONGLONG: case argType::ARG_SHORT: case argType::ARG_UINT: case argType::ARG_ULONG:
	case argType::ARG_ULONGLONG: case argType::ARG_USHORT:
		ret << X64_MOV_RAX;
		ret += (__int64)retBuffer;
		ret << X64_MOV_RAX_PTR_RAX;
		break;
	case argType::ARG_FLOAT: case argType::ARG_DOUBLE:
		ret << X64_MOV_RAX;
		ret += (__int64)retBuffer;
		ret << X64_MOV_XMM_PTR_RAX(0);
		break;
	default:
		break;
	}
	if (cdeclmode)
		ret << X86_64_RET;
	else {
		ret << X86_64_RETN;
		ret += (short)arg_offset;
	}
#endif

	return ret;
}