#include <xhacking/Detour/Detour.h>

XHACKING_START_NAMESPACE

LONG WINAPI EHandler(EXCEPTION_POINTERS* /*ExceptionInfo*/);
bool EHApplied = false;
std::vector<Detour_i*> EHandlers;

Detour_i::Detour_i(BYTE* src, BYTE* dst, BYTE arguments) :
	_src(src),
	_dst(dst),
	_arguments(arguments),
	_withTrampoline(false),
	_detourlen(0),
	_type(DETOUR_JMP),
	_allocater(BlockAlloc::Instance())
#ifdef _64BITS_BUILD_
	, _withPool(true)
#endif
{}

Detour_i::~Detour_i()
{
	_allocater.reset();
}

#ifndef _64BITS_BUILD_

BYTE Detour_i::MinLength()
{
	switch (_type)
	{
		case DETOUR_JMP: return 5;
		case DETOUR_JMP_EAX: return 7;
		case DETOUR_RET: return 6;
		case DETOUR_MEM: return 1;
		default: return 0;
	}
}

BYTE Detour_i::FillByType(BYTE* src, BYTE* dst)
{
	switch (_type)
	{
		case DETOUR_JMP:
			*(BYTE*)(src + 0) = 0xE9;
			*(DWORD*)(src + 1) = (DWORD)(dst - src) - 5;
			break;

		case DETOUR_JMP_EAX:
			*(BYTE*)(src + 0) = 0xB8;
			*(DWORD*)(src + 1) = (DWORD)(dst);
			*(WORD*)(src + 5) = 0xE0FF;
			break;

		case DETOUR_RET:
			*(BYTE*)(src + 0) = 0x68;
			*(DWORD*)(src + 1) = (DWORD)(dst);
			*(BYTE*)(src + 5) = 0xC3;
			break;

		default:
			break;
	}

	return MinLength();
}

BYTE* Detour_i::CreateTrampoline()
{
	BYTE retlen = MinLength();
	BYTE* trampoline = (BYTE*)_allocater->Get(NULL, PAGE_EXECUTE_READ, 4 + (_arguments * 4) + 5 + 2 + _detourlen + retlen);
	if (!trampoline)
		return false;

	// PUSH EBP
	// MOV EBP, ESP
	// PUSHAD
	*(BYTE*)(trampoline + 0) = 0x55;
	*(WORD*)(trampoline + 1) = 0xEC8B;
	*(BYTE*)(trampoline + 3) = 0x60;
	trampoline += 4;

	BYTE offset = _arguments * 4 + 4; // +3 (mov eax, []) + 1 (push eax) = +4
	for (BYTE i = 0; i < _arguments; i++)
	{
		// MOV EAX, [EBP + (numarguments - i)*4]
		*(WORD*)(trampoline + 0) = 0x458B;
		*(BYTE*)(trampoline + 2) = offset;

		// PUSH EAX
		*(BYTE*)(trampoline + 3) = 0x50;

		trampoline += 4;
		offset -= 4;
	}

	// call dst
	*(BYTE*)(trampoline + 0) = 0xE8;
	*(DWORD*)(trampoline + 1) = (DWORD)(_dst - trampoline) - 5;
	trampoline += 5;

	// POPAD
	// POP EBP
	*(BYTE*)(trampoline + 0) = 0x61;
	*(BYTE*)(trampoline + 1) = 0x5D;
	trampoline += 2;

	// src bytes (detourlen)
	memcpy(trampoline, _src, _detourlen);
	trampoline += _detourlen;

	// jmp src + len (retlen)
	FillByType(trampoline, _src + _detourlen);

	return (trampoline - _detourlen - 2 - 5 - (_arguments * 4) - 4);
}

#endif

BYTE* Detour_i::CreateHook()
{
	BYTE retlen = MinLength();
	BYTE* jump = (BYTE*)_allocater->Get(NULL, PAGE_EXECUTE_READ, _detourlen + retlen);
	if (!jump)
		return NULL;

	memcpy(jump, _src, _detourlen);
	jump += _detourlen;

	// jmp back
	FillByType(jump, _src + _detourlen);

	return jump - _detourlen;
}

bool Detour_i::Commit()
{
	BYTE minLength = MinLength();
	if (!minLength)
	{
		NAMESPACE::SetLastError(ERROR_NOT_IMPLEMENTED);
		return false;
	}

	// MEM Detour!
	if (_type == DETOUR_MEM)
	{
		DWORD old;
		MEMORY_BASIC_INFORMATION meminfo;
		VirtualQuery(_src, &meminfo, sizeof(MEMORY_BASIC_INFORMATION));
		if (!VirtualProtect(meminfo.BaseAddress, meminfo.RegionSize, meminfo.AllocationProtect | PAGE_GUARD, &old))
		{
			NAMESPACE::SetLastError(DETOUR_VIRTUAL_PROTECT_ERROR);
			return false;
		}

		if (!EHApplied)
		{
			if (!AddVectoredExceptionHandler(1, EHandler) && !SetUnhandledExceptionFilter(EHandler))
			{
				NAMESPACE::SetLastError(DETOUR_VEH_ERROR);
				return false;
			}

			EHApplied = true;
		}

		EHandlers.push_back(this);
		return true;
	}

#ifdef _64BITS_BUILD_
	if (_withPool && !_detourlen)
	{
		_detourlen = 5;
	}
	else
#endif
	// ASM Detour
	if (!_detourlen)
		_detourlen = minLength;
	else if (_detourlen < minLength)
	{
		NAMESPACE::SetLastError(DETOUR_LENGTH_ERROR);
		return false;
	}

	BYTE* hook = _withTrampoline ? CreateTrampoline() : CreateHook();
	if (!hook)
		return false;

	DWORD old;
	VirtualProtect(_src, _detourlen, PAGE_READWRITE, &old);
	memset(_src, 0x90, _detourlen);

	BYTE* dst = _dst;
	BYTE* src = _src;
	if (_withTrampoline)
		dst = hook;

#ifdef _64BITS_BUILD_
	if (_withPool)
	{
		BYTE* pool = CreateCallPool();
		if (!pool)
		{
			NAMESPACE::SetLastError(DETOUR_VIRTUAL_ALLOC_ERROR);
			VirtualProtect(_src, _detourlen, old, &old);
			return false;
		}

		src = pool;
	}
#endif

	FillByType(src, dst);

	// Allows calling in non trampoline environments, and code restoring
	_callee = hook;
	VirtualProtect(_src, _detourlen, old, &old);

	return true;
}

bool Detour_i::Restore()
{
	switch (_type)
	{
		case DETOUR_MEM:
		{
			DWORD old;
			MEMORY_BASIC_INFORMATION meminfo;
			VirtualQuery(_src, &meminfo, sizeof(MEMORY_BASIC_INFORMATION));
			if (!VirtualProtect(meminfo.BaseAddress, meminfo.RegionSize, meminfo.AllocationProtect & ~PAGE_GUARD, &old))
			{
				NAMESPACE::SetLastError(DETOUR_RESTORE_VP_ERROR);
				return false;
			}

			EHandlers.erase(std::find(EHandlers.begin(), EHandlers.end(), this));
			return true;
		}

		case DETOUR_JMP:
		case DETOUR_RET:
		{
			DWORD old;
			VirtualProtect(_src, _detourlen, PAGE_READWRITE, &old);

			BYTE* dst = _callee;
			if (_withTrampoline)
			{
				dst += 4 + (_arguments * 4) + 5 + 2;
			}

			memcpy(_src, dst, _detourlen);

			VirtualProtect(_src, _detourlen, old, &old);

			return true;
		}
	}

	return false;
}

#ifndef _64BITS_BUILD_

LONG WINAPI EHandler(EXCEPTION_POINTERS* ExceptionInfo)
{
	if (ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_GUARD_PAGE_VIOLATION) {
		std::vector<Detour_i*>::iterator it = EHandlers.begin();
		for (; it != EHandlers.end(); it++)
		{
			Detour_i* detour = *it;

			if ((DWORD)ExceptionInfo->ExceptionRecord->ExceptionAddress == (DWORD)detour->getSource())
			{
				// We are going to force a call to our "detour" function
				// That is, saving EBP (avoid messing up), pushing parameters, and calling

				// Inline asm requieres DWORDS for calls/movs
				DWORD offs = detour->getArguments() * 4 + 4;
				DWORD dest = (DWORD)detour->getDest();
				DWORD oEBP = 0;

				// Save EBP (just in case)
				__asm MOV oEBP, EBP

				// Push parameters
				for (int i = 0; i < detour->getArguments(); i++)
				{
					DWORD param = *(DWORD*)(ExceptionInfo->ContextRecord->Ebp + offs);
					offs -= 4;

					__asm PUSH param;
				}

				// Call dest
				__asm MOV EAX, dest
				__asm CALL EAX

				// Restore EBP
				__asm MOV EBP, oEBP
			}

			ExceptionInfo->ContextRecord->EFlags |= 0x100;
		}

		return EXCEPTION_CONTINUE_EXECUTION;
	}
	else if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP)
	{
		// TODO: We should avoid iterating all over the handlers, and use some kind of callback-structure to determine which address is to be protected
		// TODO: VirtualQuery should not be done everytime, it should be saved in a structure (alonside with above) on detour/breakpoint creation
		std::vector<Detour_i*>::iterator it = EHandlers.begin();
		for (; it != EHandlers.end(); it++)
		{
			Detour_i* detour = *it;
			DWORD old;
			MEMORY_BASIC_INFORMATION meminfo;
			VirtualQuery(detour->getSource(), &meminfo, sizeof(MEMORY_BASIC_INFORMATION));
			VirtualProtect(meminfo.BaseAddress, meminfo.RegionSize, meminfo.AllocationProtect | PAGE_GUARD, &old);
		}

		return EXCEPTION_CONTINUE_EXECUTION;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

#endif

XHACKING_END_NAMESPACE
