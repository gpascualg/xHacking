#include <Windows.h>
#include <stdio.h>

// INCLUDE ORDER IS, **IS**, IMPORTANT!
// If not done in this order, functions such as API::AutoLoad (depens on Loader) would not be available
#include <xhacking/xHacking.h>
#include <xhacking/Utilities/Utilities.h>
#include <xhacking/Loader/Loader.h>
#include <xhacking/API/api.h>
#pragma comment(lib, "xHacking.lib")
using namespace xHacking;

int main()
{
	// Normal use case of an API call
	GetAsyncKeyState(VK_F5);


	// Hidden api call (without using xHacking)
	typedef SHORT(WINAPI* get_t)(int);
	get_t getAsync = (get_t)GetProcAddress(GetModuleHandle("user32.dll"), "GetAsyncKeyState");
	getAsync(VK_F5);


	// Hidden api call using xHacking
	API<SHORT, int>* api = new API<SHORT, int>();
	api->AutoLoad("user32.dll", "GetAsyncKeyState");
	(*api)(VK_F5);
	// Autoload will use the `Loader` class and wait until the DLL is loaded to get its data
	// it can be used with the `->LoaderMode(API_LOADER_LOAD)` to force the DLL to be loaded (LoadLibrary).


	// Hidden api call using xHacking + Fake
	API<SHORT, int>* api = new API<SHORT, int>();
	api->AutoFake(NULL)->AutoLoad("user32.dll", "GetAsyncKeyState");
	(*api)(VK_F5);

	// AutoFake requieres the module name (ie. fake call from "user32.dll" should be done by ->AutoFake("user32.dll"))
	// NULL is to be used to fake call from the same module
	// Autofake will modify the stack as `fake` the caller of the function, bypassing most checks
	// The stack is also automatically cleaned up

	return 0;
}
