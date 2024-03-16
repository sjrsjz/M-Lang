#include "..\header\vm.h"
void MLang::VM::newVirtualFunction(size_t uid, size_t functionBaseOffset, const std::vector<enum argType>& argTypes, const enum argType& retType)
{
	size_t argBufferSize{}, retBufferSize{};
	for (const auto& x: argTypes)
	{
		argBufferSize += x;
	}
	retBufferSize = argTypeSize[retType];
	VirtualFunctionTable.push_back(virtualFunctionTable{ code + functionBaseOffset, uid, (unsigned char*)malloc(argBufferSize), (unsigned char*)malloc(retBufferSize), argTypes, retType});
	if (uid >= VirtualFunctionTableID.size()) {
		VirtualFunctionTableID.resize(uid + 1);
	}	
	VirtualFunctionTableID[uid] = VirtualFunctionTable.size() - 1;
}
//SHIT!!!
void MLang::VM::VMInterfaceFunction(size_t uid, void* argBuffer, void* retBuffer, const std::vector<argType>& argTypes, const enum argType retType)
{
	VM vm{};
	vm.code = code;
	vm.global = global;
	vm.this_ptr = NULL;
	for (const auto& x : argTypes) {
		size_t size = argTypeSize[x];
		vm.allocTmp(size);
	}

}