#include <xhacking/Utilities/Utilities.h>

// Console
#include <io.h>
#include <fcntl.h>

XHACKING_START_NAMESPACE

MODULEINFO GetModuleInfo(HMODULE handle)
{
	MODULEINFO modinfo = { 0 };
	if (handle == 0) return modinfo;
	GetModuleInformation(GetCurrentProcess(), handle, &modinfo, sizeof(MODULEINFO));
	return modinfo;
}

typedef struct _PEB_LDR_DATA {
    UINT8 _PADDING_[12];
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _PEB {
    UINT8 _PADDING_[12];
    PEB_LDR_DATA* Ldr;
} PEB, *PPEB;

typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    VOID* DllBase;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

typedef struct _UNLINKED_MODULE
{
    HMODULE Module;
    PLIST_ENTRY RealInLoadOrderLinks;
    PLIST_ENTRY RealInMemoryOrderLinks;
    PLIST_ENTRY RealInInitializationOrderLinks;
    PLDR_DATA_TABLE_ENTRY Entry;
} UNLINKED_MODULE;

#define UNLINK(x)               \
    (x).Flink->Blink = (x).Blink;   \
    (x).Blink->Flink = (x).Flink;

#define RELINK(x, real)   \
    (x).Flink->Blink = (real);  \
    (x).Blink->Flink = (real);  \
    (real)->Blink = (x).Blink;  \
    (real)->Flink = (x).Flink;

std::vector<UNLINKED_MODULE> UnlinkedModules;

struct FindModuleHandle
{
    HMODULE _module;
    FindModuleHandle(HMODULE module) : _module(module)
    {
    }
    bool operator() (UNLINKED_MODULE const &Module) const
    {
        return (Module.Module == _module);
    }
};

void RelinkModuleToPEB(HMODULE module)
{
    std::vector<UNLINKED_MODULE>::iterator it = std::find_if(UnlinkedModules.begin(), UnlinkedModules.end(), FindModuleHandle(module));

    if (it == UnlinkedModules.end())
    {
		NAMESPACE::SetLastError(UTILITY_PEB_NOT_UNLIKED);
        return;
    }

    RELINK((*it).Entry->InLoadOrderLinks, (*it).RealInLoadOrderLinks);
    RELINK((*it).Entry->InInitializationOrderLinks, (*it).RealInInitializationOrderLinks);
    RELINK((*it).Entry->InMemoryOrderLinks, (*it).RealInMemoryOrderLinks);
    UnlinkedModules.erase(it);
}

void UnlinkModuleFromPEB(HMODULE module)
{
    std::vector<UNLINKED_MODULE>::iterator it = std::find_if(UnlinkedModules.begin(), UnlinkedModules.end(), FindModuleHandle(module));
    if (it != UnlinkedModules.end())
    {
        NAMESPACE::SetLastError(UTILITY_PEB_ALREADY_UNLIKED);
        return;
    }

#ifdef _64BITS_BUILD_
    PPEB pPEB = (PPEB)__readgsqword(0x60);
#else
    PPEB pPEB = (PPEB)__readfsdword(0x30);
#endif

    PLIST_ENTRY CurrentEntry = pPEB->Ldr->InLoadOrderModuleList.Flink;
    PLDR_DATA_TABLE_ENTRY Current = NULL;

    while (CurrentEntry != &pPEB->Ldr->InLoadOrderModuleList && CurrentEntry != NULL)
    {
        Current = CONTAINING_RECORD(CurrentEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
        if (Current->DllBase == module)
        {
            UNLINKED_MODULE CurrentModule = {0};
            CurrentModule.Module = module;
            CurrentModule.RealInLoadOrderLinks = Current->InLoadOrderLinks.Blink->Flink;
            CurrentModule.RealInInitializationOrderLinks = Current->InInitializationOrderLinks.Blink->Flink;
            CurrentModule.RealInMemoryOrderLinks = Current->InMemoryOrderLinks.Blink->Flink;
            CurrentModule.Entry = Current;
            UnlinkedModules.push_back(CurrentModule);

            UNLINK(Current->InLoadOrderLinks);
            UNLINK(Current->InInitializationOrderLinks);
            UNLINK(Current->InMemoryOrderLinks);

            break;
        }

        CurrentEntry = CurrentEntry->Flink;
    }
}

BYTE* FindPattern(BYTE* haystack, size_t hlen, BYTE* needle, const char* mask)
{
	size_t scan, nlen = strlen(mask);
	size_t bad_char_skip[256];

	for (scan = 0; scan < 256; scan++)
		bad_char_skip[scan] = nlen;

	size_t last = nlen - 1;

	for (scan = 0; scan < last; scan++)
		if (mask[scan] != '?')
			bad_char_skip[needle[scan]] = last - scan;

	while (hlen >= nlen)
	{
		for (scan = last; mask[scan] == '?' || haystack[scan] == needle[scan]; scan--)
		if (scan == 0)
			return (unsigned char*)haystack;

		hlen -= bad_char_skip[haystack[last]];
		haystack += bad_char_skip[haystack[last]];
	}

	return 0;
}

bool CreateConsole()
{
	if (AllocConsole())
	{
		long stdioHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
		int consoleHandleR = _open_osfhandle(stdioHandle, _O_TEXT);
		FILE* fptr = _fdopen(consoleHandleR, "r");
		*stdin = *fptr;
		setvbuf(stdin, NULL, _IONBF, 0);

		stdioHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
		int consoleHandleW = _open_osfhandle(stdioHandle, _O_TEXT);
		fptr = _fdopen(consoleHandleW, "w");
		*stdout = *fptr;
		setvbuf(stdout, NULL, _IONBF, 1);

		stdioHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
		*stderr = *fptr;
		setvbuf(stderr, NULL, _IONBF, 0);

		// Compatibility for older implementations
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);

		return true;
	}

	NAMESPACE::SetLastError(ERROR_CONSOLE_ALLOC);
	return false;
}

XHACKING_END_NAMESPACE
