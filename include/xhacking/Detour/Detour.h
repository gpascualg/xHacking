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
// <summary>Contains the library Detour class</summary>

#ifndef _XHACKING_DETOUR_H
#define _XHACKING_DETOUR_H

#include <xhacking/xHacking.h>
#include <xhacking/Detour/Detour_i.h>

XHACKING_START_NAMESPACE

enum DETOUR_STATE
{
	DETOUR_NONE = 0,
	DETOUR_WAITING,
	DETOUR_READY
};

enum DETOUR_TYPE
{
	DETOUR_JMP = 0,
	DETOUR_JMP_EAX,
	DETOUR_RET,
	DETOUR_MEM
};

template<typename ReturnType, typename... Args>
class Detour
{
public:
	typedef ReturnType(WINAPI *raw_type)(Args...);
	typedef std::function<ReturnType(Args...)> complex_type;

	/// <summary>
	/// Creates a detour without specifying anything, must be called with Load
	/// </summary>
	Detour() :
		_state(DETOUR_NONE)
	{
		_detourer = new Detour_i(NULL, NULL, count_arg<complex_type>::value);
	}

	/// <summary>
	/// Creates a detour with source and destination, can be applied with Commit
	/// </summary>
	/// <param name="src">Detour source address.</param>
	/// <param name="dst">Detour destination address.</param>
	Detour(BYTE* src, BYTE* dst) :
		_state(DETOUR_READY)
	{
		_detourer = new Detour_i(src, dst, count_arg<complex_type>::value);
	}

	~Detour()
	{
		delete _detourer;
	}

#ifdef _XHACKING_LOADER_H
	/// <summary>
	/// Creates a Loader which automatically waits for the DLL to get loaded.
	/// It then automatically applies de detour
	/// </summary>
	/// <param name="module">Module (DLL) name.</param>
	/// <param name="function">Function name.</param>
	/// <param name="dst">Detour destination address.</param>
	void Wait(char* module, char* function, BYTE* dst)
	{
		_state = DETOUR_WAITING;
		_detourer->_dst = dst;
		Loader::Wait(module, function, std::bind1st(std::ptr_fun(Detour::Wait_i), this));
	}

	/// <summary>
	/// Creates a Loader which automatically loads the DLL.
	/// It then automatically applies de detour
	/// </summary>
	/// <param name="module">Module (DLL) name.</param>
	/// <param name="function">Function name.</param>
	/// <param name="dst">Detour destination address.</param>
	void Load(char* module, char* function, BYTE* dst)
	{
		_detourer->_dst = dst;

		Loader::Data* data =  Loader::Load(module, function);
		if (data)
		{
			_detourer->_src = data->Function;
			_state = DETOUR_READY;
			Commit();
			delete data;
		}
	}
#endif

	/// <summary>
	/// Calls the original (WINAPI!) function given a tuple of arguments
	/// </summary>
	/// <param name="Args...">Tuple of arguments to be passed to the function</param>
	/// <returns>Original function return value</returns>
	__forceinline ReturnType WINAPI operator()(Args... args)
	{
		return ((raw_type)(_detourer->_callee))(args...);
	}

	/// <summary>
	/// Gets the original function typecasted
	/// </summary>
	/// <return>Original function typecasted to T</returns>
	template<typename T>
	__forceinline T operator()()
	{
		return (T)_detourer->_callee;
	}

	/// <summary>
	/// Manually sets the detour length
	/// </summary>
	/// <param name="length">Length to set</param>
	/// <returns>(Same) Detour object</returns>
	Detour* Length(int length)
	{
		_detourer->_detourlen = length;
		return this;
	}

	/// <summary>
	/// Specifies if a trampoline is to be used
	/// </summary>
	/// <param name="state">true for trampoline, false for not</param>
	/// <returns>(Same) Detour object</returns>
	Detour* WithTrampoline(bool state)
	{
		_detourer->_withTrampoline = state;
		return this;
	}

	/// <summary>
	/// Specifies the detour type
	/// </summary>
	/// <param name="type">Type as stated in DETOUR_TYPE</param>
	/// <returns>(Same) Detour object</returns>
	Detour* Type(BYTE type)
	{
		_detourer->_type = type;
		return this;
	}

	/// <summary>
	/// Gets the current detour's state
	/// </summary>
	/// <returns>The current state</returns>
	inline BYTE State()
	{
		return _state;
	}

	/// <summary>
	/// Applies the detour to the target memory
	/// </summary>
	/// <returns>true if succeeds, false otherwise</returns>
	bool Commit()
	{
		if (_state != DETOUR_READY)
			return false;

		return _detourer->Commit();
	}

	/// <summary>
	/// Unapplies the detour on the target memory
	/// </summary>
	/// <returns>true if succeeds, false otherwise</returns>
	bool Restore()
	{
		return _detourer->Restore();
	}

#ifdef _64BITS_BUILD_
	/// <summary>
	/// Sets whether a call pool should be used or not (default: true)
	/// </summary>
	/// <param name="type">state, true for yes, false for not</param>
	/// <returns>(Same) Detour object</returns>
	Detour* WithPool(BYTE state)
	{
		_detourer->_withPool = state;
		return this;
	}

	/// <summary>
	/// Specifies the call pool type
	/// </summary>
	/// <param name="type">Type as stated in DETOUR_TYPE</param>
	/// <returns>(Same) Detour object</returns>
	Detour* PoolType(BYTE type)
	{
		_detourer->_pooltype = type;
		return this;
	}
#endif

private:
#ifdef _XHACKING_LOADER_H
	static void Wait_i(Detour* detour, Loader::Data* data)
	{
		if (NAMESPACE::GetLastError() == LOADER_TIMEOUT)
			return;

		detour->_detourer->_src = data->Function;
		detour->_state = DETOUR_READY;
		detour->Commit();
	}
#endif

private:
	Detour_i* _detourer;
	BYTE _state;
};

XHACKING_END_NAMESPACE

#endif
