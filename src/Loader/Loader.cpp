#include <xhacking/Loader/Loader.h>

XHACKING_START_NAMESPACE

Loader* Loader::_instance = NULL;

Loader::Loader() :
	_empty(true)
{
	InitializeCriticalSection(&_mutex);
}

void Loader::wait_i(char* module, char* function, std::function<void(Loader::Data*)> cb)
{
	// Can we immediatelly return?
	HMODULE handle = GetModuleHandle(module);
	if (handle)
	{
		Data* data = new Data(handle, (BYTE*)GetProcAddress(handle, function));
		cb(data);
		delete data;
		return;
	}

	// We couldn't, let's create a thread or queue or request
	Internal* internal = new Internal(module, function, cb);

	EnterCriticalSection(&_mutex);
	bool start = Empty();
	Add(internal);

	if (start)
	{
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Loader::Process, 0, 0, 0);
	}
	LeaveCriticalSection(&_mutex);
}

void Loader::process_i()
{
	while (!Empty())
	{
		EnterCriticalSection(&_mutex);
		std::vector<Internal*>::iterator it = _queue.begin();
		Internal* internal = *it;
		_queue.erase(it);
		LeaveCriticalSection(&_mutex);

		// TODO: TIMEOUT, otherwise this could be an infinite loop
		Data* data = new Data();
		while (!data->Module)
		{
			data->Module = GetModuleHandle(internal->Module);
			Sleep(500);
		}
		data->Function = (BYTE*)GetProcAddress(data->Module, internal->Function);

		internal->Cb(data);
		delete internal;
		delete data;

		EnterCriticalSection(&_mutex);
		if (_queue.empty())
			_empty = true;
		LeaveCriticalSection(&_mutex);
	}
}

XHACKING_END_NAMESPACE
