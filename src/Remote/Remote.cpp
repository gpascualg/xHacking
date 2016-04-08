#include <xhacking/Remote/Remote.h>
#include <tlhelp32.h>

// Disable BOOL forcing to true/false
#pragma warning(disable: 4800)

XHACKING_START_NAMESPACE
XHACKING_REMOTE_START_NS

bool GrantPrivileges()
{
	HANDLE hToken = NULL;
	TOKEN_PRIVILEGES tokenPriv;
	LUID luidDebug;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
		if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luidDebug))
		{
			tokenPriv.PrivilegeCount = 1;
			tokenPriv.Privileges[0].Luid = luidDebug;
			tokenPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			bool result = AdjustTokenPrivileges(hToken, FALSE, &tokenPriv, 0, NULL, NULL);
			CloseHandle(hToken);

			return result;
		}
	}

	return false;
}

DWORD FindProcessId(const std::string& processName)
{
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);

	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (processesSnapshot == INVALID_HANDLE_VALUE)
		return 0;

	Process32First(processesSnapshot, &processInfo);
	if (!processName.compare(processInfo.szExeFile))
	{
		CloseHandle(processesSnapshot);
		return processInfo.th32ProcessID;
	}

	while (Process32Next(processesSnapshot, &processInfo))
	{
		if (!processName.compare(processInfo.szExeFile))
		{
			CloseHandle(processesSnapshot);
			return processInfo.th32ProcessID;
		}
	}

	CloseHandle(processesSnapshot);
	return 0;
}

HANDLE Open(char* process)
{
	DWORD pID = FindProcessId(std::string(process));

	if (!pID)
	{
		NAMESPACE::SetLastError(REMOTE_PROCESS_NOT_FOUND);
		return 0;
	}

	DWORD access = PROCESS_ALL_ACCESS;
	if (!GrantPrivileges())
	{
		NAMESPACE::SetLastError(REMOTE_GRANT_PRIVILEGES);
		access = PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE;
	}

	return OpenProcess(access, FALSE, pID);
}

XHACKING_REMOTE_END_NS
XHACKING_END_NAMESPACE
