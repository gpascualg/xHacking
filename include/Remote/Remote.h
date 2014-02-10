#include "xHacking.h"

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