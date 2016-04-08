#include <xhacking/xHacking.h>

XHACKING_START_NAMESPACE

WORD _error = ERROR_NONE;
void SetLastError(WORD error)
{
	_error = error;
}

WORD GetLastError()
{
	return _error;
}

XHACKING_END_NAMESPACE
