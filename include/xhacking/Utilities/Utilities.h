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
// <summary>Contains the library utility funciontions</summary>

#ifndef _XHACKING_UTILITIES_H
#define _XHACKING_UTILITIES_H

#include <xhacking/xHacking.h>

#include <vector>
#include <algorithm>
#include <Psapi.h>
#pragma comment(lib, "Psapi.lib")

XHACKING_START_NAMESPACE

/// <summary>
/// Creates a console on the current process and redirects stdin, stdout, stderr
/// to that console
/// </summary>
/// <returns>true if succeeds, false otherwise</returns>
bool CreateConsole();

/// <summary>
/// Retrieves information of a module (DLL)
/// </summary>
/// <param name="module">Module (DLL) handle</param>
/// <returns>Struct containing module information, 0 if not found</returns>
MODULEINFO GetModuleInfo(HMODULE handle);

/// <summary>
/// Gets the address of a known pattern in the process memory
/// </summary>
/// <param name="haystack">Start address to search from</param>
/// <param name="hlen">Maximum length to search in</param>
/// <param name="needle">Byte array to search for</param>
/// <param name="mask">String contaning 'x' if the byte is known, or '?' if the byte is unknown. Must be equal length as `mask`</param>
/// <returns>Struct containing module information, 0 if not found</returns>
BYTE* FindPattern(BYTE* haystack, size_t hlen, BYTE* needle, const char* mask);

/// <sumary>
/// Unlinks a module (DLL) header from PEB
/// </sumary>
/// <param name="module">Module (DLL) to unlink from PEB</param>
void UnlinkModuleFromPEB(HMODULE module);

/// <sumary>
/// Links an unlinked module (DLL) header back to PEB
/// </sumary>
/// <param name="module">Module (DLL) to link to PEB</param>
void RelinkModuleToPEB(HMODULE module);

XHACKING_END_NAMESPACE

#endif
