#pragma once
#include "Common.h"
#include "ByteArray.h"

namespace MLang {
#define ARG_MAX_SIZE 16
	enum argType {
		ARG_INT,
		ARG_UINT,
		ARG_SHORT,
		ARG_USHORT,
		ARG_LONG,
		ARG_ULONG,
		ARG_LONGLONG,
		ARG_ULONGLONG,
		ARG_FLOAT,
		ARG_DOUBLE,
		ARG_CHAR,
		ARG_BOOL,
	};
	static size_t argTypeSize[] = {
		sizeof(int),
		sizeof(unsigned int),
		sizeof(short int),
		sizeof(unsigned short int),
		sizeof(long int),
		sizeof(unsigned long int),
		sizeof(long long int),
		sizeof(unsigned long long int),
		sizeof(float),
		sizeof(double),
		sizeof(char) * 2,
		sizeof(bool)
	};


	ByteArray<unsigned char>BuildVMInterfaceFunction(size_t uid, const void (*function)(size_t, void*, void*, const std::vector<enum argType>&, const enum argType),
			void* argBuffer, const void* retBuffer, const std::vector<argType>& argType, const enum argType retType, bool cdeclmode);
}

