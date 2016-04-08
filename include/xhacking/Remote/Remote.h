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
// <summary>Contains the remote functions namespace</summary>

#ifndef _XHACKING_REMOTE_H
#define _XHACKING_REMOTE_H

#include <xhacking/xHacking.h>

#define XHACKING_REMOTE_START_NS namespace Remote {
#define XHACKING_REMOTE_END_NS }

XHACKING_START_NAMESPACE
XHACKING_REMOTE_START_NS

bool GrantPrivileges();
DWORD FindProcessId(const std::string& processName);
HANDLE Open(char* process);

template<typename T> T Read(HANDLE process, DWORD address)
{
	T value = 0;
	size_t read = 0;
	ReadProcessMemory(process, (LPCVOID)address, &value, sizeof(T), &read);

	if (sizeof(T) != read)
		NAMESPACE::SetLastError(REMOTE_READ_LENGTH_ERROR);

	return value;
}

template<typename T> bool Write(HANDLE process, DWORD address, T value)
{
	WriteProcessMemory(process, address, &value, sizeof(T), &written);

	if (written != sizeof(T))
	{
		NAMESPACE::SetLastError(REMOTE_WRITE_LENGTH_ERROR);
		return false;
	}

	return true;
}

XHACKING_REMOTE_END_NS
XHACKING_END_NAMESPACE

#endif
