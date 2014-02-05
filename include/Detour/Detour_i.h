// <liscence>Affero GPL</liscence>
//
// Copyright (c) 2013 All Right Reserved, http://youtube.com/user/bl1pi
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
// <summary>Contains the worker class of Detour</summary>

#ifndef _XHACKING_DETOUR_I_H
#define _XHACKING_DETOUR_I_H

#include "xHacking.h"

XHACKING_START_NAMESPACE

	class Detour_i
	{
		template<typename R, typename... A> friend class Detour;

	public:
		BYTE* getSource()
		{
			return _src;
		}

		BYTE* getDest()
		{
			return _dst;
		}

		BYTE getArguments()
		{
			return _arguments;
		}

	private:
		Detour_i(BYTE* src, BYTE* dst, BYTE arguments);

		BYTE MinLength();
		BYTE FillByType(BYTE* src, BYTE* dst);

		BYTE* CreateTrampoline();
		BYTE* CreateHook();
		bool Commit();
		bool Restore();

		bool _withTrampoline;

		BYTE* _src;
		BYTE* _dst;
		BYTE* _callee;

		BYTE _arguments;
		BYTE _type;
		BYTE _detourlen;
	};
	
XHACKING_END_NAMESPACE

#endif