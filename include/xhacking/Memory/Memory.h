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
// <credits>Tsuda Kageyu for the �base? code</credits>
// <summary>Contains the worker class of Detour</summary>

#ifndef _XHACKING_MEMORY_H
#define _XHACKING_MEMORY_H

#include <xhacking/xHacking.h>
#include <iostream>
#include <vector>
#include <algorithm>

XHACKING_START_NAMESPACE

class BlockAlloc
{
	friend class Detour_i;

public:
	struct MEMORY_BLOCK
	{
		DWORD_PTR Start;
		DWORD_PTR End;
		DWORD_PTR Used;
		DWORD Protect;
	};

	static BlockAlloc* Instance()
	{
		if (!_instance)
			_instance = new BlockAlloc();

		return _instance;
	}

	~BlockAlloc();

	DWORD_PTR Get(DWORD_PTR from, DWORD protect, size_t size);

private:
	typedef ::std::vector<MEMORY_BLOCK> MEMORY_CONTAINER;
	typedef MEMORY_CONTAINER::iterator MEMORY_ITERATOR;

	const size_t BlockSize = 0x10000;

	BlockAlloc();

	MEMORY_BLOCK* Alloc(DWORD_PTR from, DWORD protect, size_t size);

private:
	static BlockAlloc* _instance;

	MEMORY_CONTAINER _memoryBlocks;
	DWORD_PTR _start;
	DWORD_PTR _end;

	DWORD _refcount;
};

XHACKING_END_NAMESPACE

#endif
