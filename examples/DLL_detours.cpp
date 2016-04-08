// General
#include <stdio.h>
#include <Windows.h>

// xHacking library
// INCLUDE ORDER IS, **IS**, IMPORTANT!
// If not done in this order, functions such as Detour::Wait (depens on Loader) would not be available
#include <xhacking/xHacking.h>
#include <xhacking/Utilities/Utilities.h>
#include <xhacking/Loader/Loader.h>
#include <xhacking/Detour/Detour.h>
using namespace xHacking;

#ifdef _DEBUG
#pragma comment(lib, "xHacking_d.lib")
#else
#pragma comment(lib, "xHacking.lib")
#endif

// Variables which will hold the detours
Detour<int, int, char*, int, int>* recvDetour = NULL;
Detour<int, int, int, char*, int, int>* sendDetour = NULL;

// Commented example, not used
Detour<int, int, char*, int, int>* sendMemory = NULL;

int WINAPI nuestro_recv(SOCKET s, char *buf, int len, int flags)
{
	// Calling the original function
	int ret = (*recvDetour)(s, buf, len, flags);

	__asm PUSHAD;
	__asm PUSHFD;

	// Custom code

	__asm POPFD;
	__asm POPAD;

	return ret;
}

// NOTE: unk0 is due to detouring 5 bytes from function start (as seen below)
// and, probably, unk0 = EBP
int WINAPI nuestro_send(int unk0, SOCKET s, char *buf, int len, int flags)
{
	__asm PUSHAD;
	__asm PUSHFD;

	// Custom code

	__asm POPFD;
	__asm POPAD;

	return 0;
	// We are in a trampoline, it doesn't really matter what we return
}

// This is in case we want to do a memory detour (PAGE_GUARD)
int WINAPI nuestro_mem_send(int s, char* buf, int l, int f)
{
	__asm PUSHAD;
	__asm PUSHFD;

	__asm POPFD;
	__asm POPAD;

	return 0;
	// Again, it doesn't matter what we return, we are inside the VEH (error handler)
}

// This is asyncrouneusly (or sync if the DLL is already loaded) by the xHacking::Loader class
void hookSend(Loader::Data* data)
{
	// We are performing the detour on +5 bytes, including a trampoline, so that's why we are doing
	// it this way.
	// It is, somehow, an example of a mid-function hooking
	sendDetour = new Detour<int, int, int, char*, int, int>(data->Function + 5, (BYTE*)nuestro_send);
	sendDetour->WithTrampoline(true)->Commit();

	// A trampoline is basically some code to automatically return to the original program flow.

	// Alternatively, we could apply a memory breakpoint uncommenting the below lines
	// Both detours should not be used at the same time
	//sendMemory = new Detour<int, int, char*, int, int>(data->Function + 5, (BYTE*)nuestro_mem_send);
	//sendMemory->Type(DETOUR_MEM)->Commit();
}

void Hooks()
{
	// We first create the recv detour.
	// We want it to be automatically created whenever WSOCK32 is loaded (and not load it),
	// Thus, we use the `Wait` function in Detour.
	// If we wanted to force load the DLL, we could use `Load`

	// int WINAPI recv(int, char*, int, int);
	recvDetour = new Detour<int, int, char*, int, int>();
	recvDetour->Wait("WSOCK32.dll", "recv", (BYTE*)nuestro_recv);

	// We could specify the Detour type by calling `Type(DETOUR_X)` where DETOUR_X is defined in
	// DETOUR_TYPE. That call should be done BEFORE the wait call, unexpected behaviour may happen
	// otherwise

	// Here we use the Loader class to wait for the WS2_32 dll
	// Once it is loaded, the function `hookSend` will be called
	Loader::Wait("WS2_32.dll", "send", hookSend);
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, DWORD reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		// Use the xHacking::CreateConsole function
		CreateConsole();

		// Call our function
		Hooks();
	}

	return true;
}
