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
// <summary>Contains the library main header file</summary>

#if !defined(_MSC_VER)
#error Support for non MSC compilers is not yet implemented
#else

#if (_MSC_VER < 1700) // < 2012
#error At least Visual Studio 2012 is required
#endif

#ifndef WARN
#define STRINGISE_IMPL(x) #x
#define STRINGISE(x) STRINGISE_IMPL(x)
#define FILE_LINE_LINK __FILE__ "(" STRINGISE(__LINE__) ") : "
#define WARN(exp) (FILE_LINE_LINK "WARNING: " exp)
#endif

#if (_MSC_VER < 1800) // < 2013
#pragma message WARN("Visual Studio 2013 is recommended")
#endif

// Define custom namespace macro
#ifndef XHACKING_START_NAMESPACE
	#define NAMESPACE xHacking
	#define XHACKING_START_NAMESPACE namespace xHacking {
	#define XHACKING_END_NAMESPACE }
#endif

// Disable warnings 
#pragma warning (disable: 4731)

// Check compiler bits mode
#if (defined(__x86_64__) || defined(__LP64__) || defined(_WIN64)) && !defined(_64BITS_BUILD_)
#pragma message WARN("Not all functions may be available on x64 yet")
#define _64BITS_BUILD_
#endif

#endif

#ifndef _XHACKING_MAIN_H
#define _XHACKING_MAIN_H

#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <vector>
#include <functional>
#include <typeinfo>

#ifdef _64BITS_BUILD_
typedef UINT64 QWORD;
#endif

XHACKING_START_NAMESPACE

class Loader;
template<typename R, typename... A> class Detour;

template<typename T>
struct count_arg;

template<typename R, typename... Args>
struct count_arg<std::function<R(Args...)>>
{
	static const size_t value = sizeof...(Args);
};

XHACKING_END_NAMESPACE


XHACKING_START_NAMESPACE

enum ERROR_CODES
{
	// GENERAL
	ERROR_NONE						= 0x0000,
	ERROR_CONSOLE_ALLOC				= 0x0001,
	ERROR_NOT_IMPLEMENTED			= 0x0003,
	ERROR_BUFFER_ALLOC				= 0x0004,

	// LOADER
	LOADER_WAIT_WITHOUT_CALLBACK	= 0x1001,
	LOADER_TIMEOUT					= 0x1002,
	LOADER_LOAD_ERROR				= 0x1003,

	// DETOUR
	DETOUR_VIRTUAL_PROTECT_ERROR	= 0x2001,
	DETOUR_VEH_ERROR				= 0x2002,
	DETOUR_LENGTH_ERROR				= 0x2003,
	DETOUR_MALLOC_ERROR				= 0x2004,
	DETOUR_RESTORE_VP_ERROR			= 0x2005,
	DETOUR_VIRTUAL_ALLOC_ERROR		= 0x2006,

	// API
	API_NOT_READY					= 0x3001,
	API_MALLOC_ERROR				= 0x3002,
	API_AUTOFAKE_NOT_FOUND			= 0x3003,

	// UTILITIES
	UTILITY_PEB_ALREADY_UNLIKED		= 0x4001,
	UTILITY_PEB_NOT_UNLIKED			= 0x4002,

	// REMOTE
	REMOTE_PROCESS_NOT_FOUND		= 0x5001,
	REMOTE_GRANT_PRIVILEGES			= 0x5002,
	REMOTE_READ_LENGTH_ERROR		= 0x5003,
	REMOTE_WRITE_LENGTH_ERROR		= 0x5004,
};

/// <summary>
/// Sets the last error
/// </summary>
/// <param name="error">The error code as stated in ERROR_CODES.</param>
void SetLastError(WORD error);

/// <summary>
/// Gets the last error
/// </summary>
/// <returns>The error code as stated in ERROR_CODES.</returns>
WORD GetLastError();

XHACKING_END_NAMESPACE

#endif
