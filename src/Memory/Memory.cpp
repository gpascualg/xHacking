#include <xhacking/Memory/Memory.h>

XHACKING_START_NAMESPACE

BlockAlloc* BlockAlloc::_instance = NULL;

bool operator <(size_t const l, BlockAlloc::MEMORY_BLOCK const& r)
{
	return l < r.Start;
}
bool operator <(BlockAlloc::MEMORY_BLOCK const& l, size_t const r)
{
	return l.Start < r;
}
bool operator <(BlockAlloc::MEMORY_BLOCK const& l, BlockAlloc::MEMORY_BLOCK const r)
{
	return l.Start < r.Start;
}

BlockAlloc::BlockAlloc()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	_start = (DWORD_PTR)(si.lpMinimumApplicationAddress);
	_end = (DWORD_PTR)(si.lpMaximumApplicationAddress);
}

BlockAlloc::~BlockAlloc()
{
	for (MEMORY_ITERATOR it = _memoryBlocks.begin(); it != _memoryBlocks.end(); it++)
	{
		VirtualFree((LPVOID)((*it).Start), BlockSize, MEM_RELEASE);
	}
}

DWORD_PTR BlockAlloc::Get(DWORD_PTR from, DWORD protect, size_t size)
{
	size = (size + TYPE_ALIGNMENT(void*) - 1) & ~(TYPE_ALIGNMENT(void*) - 1);
	MEMORY_BLOCK* block = Alloc(from, protect, size);
	if (block == NULL)
	{
		return NULL;
	}

	DWORD_PTR buffer = block->Start + block->Used;
	if (VirtualAlloc((LPVOID)buffer, size, MEM_COMMIT, block->Protect) == NULL)
	{
		return NULL;
	}

	DWORD oldProtect;
	// PAGE_EXECUTE_READ -> PAGE_EXECUTE_READWRITE, PAGE_READONLY -> PAGE_READWRITE
	if (!VirtualProtect((LPVOID)buffer, size, (block->Protect << 1), &oldProtect))
	{
		return NULL;
	}

	block->Used += size;
	return buffer;
}

BlockAlloc::MEMORY_BLOCK* BlockAlloc::Alloc(DWORD_PTR from, DWORD protect, size_t size)
{
	MEMORY_ITERATOR ib = _memoryBlocks.begin();
	MEMORY_ITERATOR ie = _memoryBlocks.end();

#ifdef _64BITS_BUILD_
	DWORD_PTR start = _start;
	DWORD_PTR end = _end;

	if (from)
	{
		// +- 500mb
		start = from - 0x20000000;
		end = from + 0x20000000;
	}

	ib = std::lower_bound(ib, ie, start);
	ie = std::upper_bound(ib, ie, end);
#endif

	// Let's search for a free block
	for (MEMORY_ITERATOR i = ib; i != ie; ++i)
	{
		if (i->Protect == protect && i->Used + size <= BlockSize)
		{
			return &(*i);
		}
	}

	// Not found, must allocate some memory
	DWORD_PTR alloc = NULL;

#ifdef _64BITS_BUILD_

	if (from)
	{
		DWORD_PTR min = start / BlockSize;
		DWORD_PTR max = end / BlockSize;
		int rel = 0;
		MEMORY_BASIC_INFORMATION mi = { 0 };
		for (int i = 0; i < (max - min + 1); ++i)
		{
			rel = -rel + (i & 1);
			void* query = reinterpret_cast<void*>(((min + max) / 2 + rel) * BlockSize);
			VirtualQuery(query, &mi, sizeof(mi));
			if (mi.State == MEM_FREE)
			{
				alloc = (DWORD_PTR)VirtualAlloc(query, BlockSize, MEM_RESERVE, protect);
				if (alloc != NULL)
				{
					break;
				}
			}
		}
	}
	else
#endif
	{
		alloc = (DWORD_PTR)VirtualAlloc(NULL, BlockSize, MEM_RESERVE, protect);
	}

	if (alloc)
	{
		MEMORY_BLOCK block = { 0 };
		block.Start = alloc;
		block.Protect = protect;

#if defined _64BITS_BUILD_
		MEMORY_ITERATOR i = std::lower_bound(_memoryBlocks.begin(), _memoryBlocks.end(), alloc);
#else
		MEMORY_ITERATOR i = _memoryBlocks.begin();
#endif
		i = _memoryBlocks.insert(i, block);

		return &(*i);
	}
	else
	{
		NAMESPACE::SetLastError(ERROR_BUFFER_ALLOC);
		return NULL;
	}
}

XHACKING_END_NAMESPACE
