#include "Detour.h"

XHACKING_START_NAMESPACE

#ifdef _64BITS_BUILD_

BYTE Detour_i::MinLength()
{
	switch (_type)
	{
		case DETOUR_JMP: return 0;
		case DETOUR_JMP_EAX: return 12;
		case DETOUR_RET: return 0;
		case DETOUR_MEM: return 0;
		default: return 0;
	}
}

BYTE Detour_i::FillByType(BYTE* src, BYTE* dst)
{
	switch (_type)
	{
		case DETOUR_JMP:
			break;

		// JMP RAX
		case DETOUR_JMP_EAX:
			// MOV RAX 64 bits
			*(BYTE*)(src + 0) = 0x48;
			*(BYTE*)(src + 1) = 0xB8;
			*(QWORD*)(src + 2) = (QWORD)(dst);

			// JMP RAX
			*(WORD*)(src + 10) = 0xE0FF;
			break;

		case DETOUR_RET:
			break;

		default:
			break;
	}

	return MinLength();
}

BYTE* Detour_i::CreateTrampoline()
{
	return NULL;
}

LONG WINAPI EHandler(EXCEPTION_POINTERS* ExceptionInfo)
{
	return EXCEPTION_CONTINUE_SEARCH;
}

#endif

XHACKING_END_NAMESPACE