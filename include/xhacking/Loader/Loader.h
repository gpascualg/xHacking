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
// <summary>Contains the library loader class</summary>

#ifndef _XHACKING_LOADER_H
#define _XHACKING_LOADER_H

#include <xhacking/xHacking.h>

XHACKING_START_NAMESPACE

	class Loader
	{
	public:
		struct Data
		{
			Data() :
				Module(0),
				Function(0)
			{}

			Data(HMODULE module, BYTE* function) :
				Module(module),
				Function(function)
			{}

			HMODULE Module;
			BYTE* Function;
		};

	private:
		struct Internal
		{
			Internal(char* module, char* function, std::function<void(Loader::Data*)> cb) :
				Module(module),
				Function(function),
				Cb(cb)
			{}

			char* Module;
			char* Function;
			std::function<void(Loader::Data*)> Cb;
		};

	public:
		/// <summary>
		/// Automatically waits for a DLL to be loaded (in a thread), and calls a function (callback)
		/// once the function address has ben obtained
		/// </summary>
		/// <param name="module">Module (DLL) name</param>
		/// <param name="function">Function name</param>
		/// <param name="cb">Callback function to be called when function is located</param>
		/// <returns>false if not callback is specified, true otherwise</returns>
		static bool Wait(char* module, char* function, std::function<void(Loader::Data*)> cb)
		{
			if (!_instance)
				_instance = new Loader();

			if (!cb)
			{
				SetLastError(LOADER_WAIT_WITHOUT_CALLBACK);
				return false;
			}

			_instance->wait_i(module, function, cb);
			return true;
		}

		/// <summary>
		/// Loads a DLL into the process and finds a function in it
		/// </summary>
		/// <param name="module">Module (DLL) name</param>
		/// <param name="function">Function name</param>
		/// <returns>NULL if module can not be loaded, structure cointaning information otherwise</returns>
		static Data* Load(char* module, char* function)
		{
			HMODULE handle = LoadLibraryA(module);
			if (!handle)
			{
				NAMESPACE::SetLastError(LOADER_LOAD_ERROR);
				return NULL;
			}

			Data* data = new Data(handle, (BYTE*)GetProcAddress(handle, function));
			return data;
		}

	private:
		void wait_i(char* module, char* function, std::function<void(Loader::Data*)> cb);

		static void Process()
		{
			_instance->process_i();
		}
		void process_i();

		inline bool Empty()
		{
			return _empty;
		}

		inline void Add(Internal* internal)
		{
			_queue.push_back(internal);
			_empty = false;
		}

	private:
		Loader();
		static Loader* _instance;

		CRITICAL_SECTION _mutex;
		std::vector<Internal*> _queue;
		bool _empty;
	};

XHACKING_END_NAMESPACE

#endif
