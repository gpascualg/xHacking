// <liscence>Affero GPL</liscence>
//
// Copyright (c) 2013-2014 All Right Reserved, http://youtube.com/user/bl1pi
// This source is subject to the Affero GPL License.
// Please see the LICENSE file for more information.
// All other rights reserved.
//
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// </copyright>
// <author>Guillem Pascual</author>
// <email>blipi.tube@gmail.com</email>
// <date>2014-02-05</date>
// <summary>Contains the library API class</summary>

#ifndef _XHACKING_API_H
#define _XHACKING_API_H

#include <xhacking/xHacking.h>

XHACKING_START_NAMESPACE

enum API_STATE
{
	API_NONE = 0,
	API_WAITING,
	API_READY
};

enum API_LOADER_MODE
{
	API_LOADER_WAIT = 0,
	API_LOADER_LOAD,
};

template<typename... Tp>
struct sizeof_pack___ { static const std::size_t value = sizeof...(Tp); };

template<std::size_t I = 0, typename... Tp>
typename std::enable_if<I == sizeof_pack___<Tp...>::value, BYTE*>::type
	push_arg(BYTE* jmp, std::tuple<Tp...> tpl)
{
	return jmp;
}

template<std::size_t I = 0, typename... Tp>
typename std::enable_if<I < sizeof_pack___<Tp...>::value, BYTE*>::type
	push_arg(BYTE* jmp, std::tuple<Tp...> tpl)
{
	*(BYTE*)(jmp + 0) = 0x68;
	*(DWORD*)(jmp + 1) = std::get<I>(tpl);
	jmp += 5;

	return push_arg<I + 1, Tp...>(jmp, tpl);
}


template<typename ReturnType, typename... Args>
class API
{
public:
	typedef ReturnType(WINAPI *raw_type)(Args...);
	typedef std::function<ReturnType(Args...)> complex_type;

	/// <summary>
	/// Creates an API caller without specifying its API
	/// </summary>
	API() :
		_state(API_NONE),
		_api(NULL),
		_jmp(NULL),
		_fake(NULL)
	{
	}

	/// <summary>
	/// Creates an API caller setting up the API
	/// </summary>
	/// <param name="api">API address.</param>
	/// <param name="dst">Detour destination address.</param>
	API(BYTE* api) :
		_state(API_READY),
		_loaderMode(API_LOADER_WAIT),
		_api(api),
		_jmp((DWORD)api),
		_fake(NULL)
	{
	}

	/// <summary>
	/// Calls the hidden API, faking the return address if it has been setup
	/// </summary>
	/// <param name="args">Arguments to call the API</param>
	/// <returns>API result. If it fails returns 0 and sets up an error</result>
	ReturnType __declspec(noinline) operator()(Args... args)
	{
		if (_state != API_READY)
		{
			NAMESPACE::SetLastError(API_NOT_READY);
			return 0;
		}

		if (_fake)
		{
			BYTE nargs = count_arg<complex_type>::value;
			BYTE* jmp = (BYTE*)malloc(nargs * 5 + 5 + 5);

			if (!jmp)
			{
				NAMESPACE::SetLastError(API_MALLOC_ERROR);
				return 0;
			}

			// Convert the args... to a tuple, so they can be individually accessed
			std::tuple<Args...> targs = std::forward_as_tuple(args...);

			// For some reason, using the attributes directly, would push it's offset (ie, 8)
			// instead of its value, even if indirection was used.
			DWORD fake = _fake;
			DWORD api = _jmp;
			DWORD retval = 0;
			DWORD to = (DWORD)jmp;

			// foreach args...: push args[i]
			push_arg(jmp, targs);

			// PUSH Custom Return
			*(BYTE*)(jmp + 0) = 0x68;
			*(DWORD*)(jmp + 1) = fake;
			jmp += 5;

			// JMP API
			*(BYTE*)(jmp + 0) = 0xE9;
			*(DWORD*)(jmp + 1) = (DWORD)(api - (DWORD)jmp) - 5;

			// Allow code execution
			DWORD old;
			VirtualProtect((LPVOID)to, nargs * 4 + 5 + 5, PAGE_EXECUTE_READ, &old);

			// Call code (Will return here if __fake__ has been correctly specified)
			__asm CALL to

			// Get return value
			__asm MOV retval, EAX

			// Unprotect
			VirtualProtect((LPVOID)to, nargs * 4 + 5 + 5, PAGE_READWRITE, &old);
			free((void*)to);

			return (ReturnType)retval;
		}

		return ((raw_type)(_api))(args...);
	}

	/**
	If we were to see the stack of the above function, it would be like:

		CALL_RETURN	<---------------
		arg[0]						|
		arg[1]						|
		...			----------------
		arg[i]		|
		FAKE_RETURN	<---------------
									|
		(JMP API)					|
			| ...					|
			| RET ------------------

	**/

	/// <summary>
	/// Setups the fake return address
	/// </summary>
	/// <param name="fake">Fake return address. MUST BE A RET (0xC3)</param>
	/// <returns>API object</result>
	API* Fake(DWORD fake)
	{
		_fake = fake;
		return this;
	}

	/// <summary>
	/// If using Loader methods to autoload the API or to autofake within a module,
	/// this function sets up the loader mode (*wait* for dll, or immediately *load* dll)
	/// </summary>
	/// <param name="mode">Loader mode as in API_LOADER_MODE</param>
	/// <returns>API object</result>
	API* LoaderMode(BYTE mode)
	{
		_loaderMode = mode;
		return this;
	}

#ifdef _XHACKING_UTILITIES_H
	/// <summary>
	/// Automatically finds a RET instruction to fake given an address and a range
	/// </summary>
	/// <param name="start">Addres from which to start searching</param>
	/// <param name="len">Length to search in</param>
	/// <returns>API object</result>
	API* AutoFakeRange(BYTE* start, int len)
	{
		_fake = (DWORD)NAMESPACE::FindPattern(start, len, (BYTE*)"\xC3", "x");
		if (!_fake)
			NAMESPACE::SetLastError(API_AUTOFAKE_NOT_FOUND);

		return this;
	}

	/// <summary>
	/// Automatically finds a RET instruction in a module and fakes the return address
	/// </summary>
	/// <param name="module">Module (DLL) handle to search in</param>
	/// <returns>API object</result>
	API* AutoFakeWithinModule(HMODULE module)
	{
		MODULEINFO info = NAMESPACE::GetModuleInfo(module);
		return AutoFakeRange((BYTE*)info.lpBaseOfDll, (int)info.SizeOfImage);
	}

#ifdef _XHACKING_LOADER_H
	/// <summary>
	/// Automatically loads (or waits for) a module (DLL), and then searches the fake adress in it
	/// </summary>
	/// <param name="module">Module (DLL) name</param>
	/// <returns>API object</result>
	API* AutoFake(char* module)
	{
		if (_loaderMode == API_LOADER_WAIT)
		{
			Loader::Wait(module, "", std::bind1st(std::ptr_fun(AutoFake_i), this));
		}
		else
		{
			Loader::Data* data = Loader::Load(module, "");
			if (data)
			{
				AutoFakeWithinModule(data->Module);
				delete data;
			}
		}

		return this;
	}

	/// <summary>
	/// Automatically loads or waits for an API to be available, and sets its as target
	/// </summary>
	/// <param name="module">Module (DLL) in which the API is</param>
	/// <param name="function">Funcion name</param>
	/// <returns>API object</result>
	API* AutoLoad(char* module, char* function)
	{
		if (_loaderMode == API_LOADER_WAIT)
		{
			Loader::Wait(module, function, std::bind1st(std::ptr_fun(AutoLoad_i), this));
		}
		else
		{
			Loader::Data* data = Loader::Load(module, function);
			if (data)
			{
				_api = data->Function;
				_jmp = (DWORD)data->Function;
				_state = API_READY;
				delete data;
			}
		}

		return this;
	}

private:
	static void AutoFake_i(API* api, Loader::Data* data)
	{
		if (NAMESPACE::GetLastError() == LOADER_TIMEOUT)
			return;

		api->AutoFakeWithinModule(data->Module);
	}

	static void AutoLoad_i(API* api, Loader::Data* data)
	{
		if (NAMESPACE::GetLastError() == LOADER_TIMEOUT)
			return;

		api->_api = data->Function;
		api->_jmp = (DWORD)data->Function;
		api->_state = API_READY;
	}
#endif
#endif

private:
	BYTE _state;
	BYTE _loaderMode;

	BYTE* _api;
	DWORD _jmp;
	DWORD _fake;
};

XHACKING_END_NAMESPACE

#endif
