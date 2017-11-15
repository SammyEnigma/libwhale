/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	herror.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef __HERROR_H
#define __HERROR_H

#include "whale/basic/hplatform.h"

namespace Whale
{
	namespace Basic
	{
		namespace HError
		{
			static unsigned int num() {
#ifdef OS_WIN32
				return ::GetLastError();
#else
				return errno;
#endif
			}

			static std::string what(int errCode = num()) {
#ifdef OS_WIN32
				LPVOID msgBuf = NULL;
				FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL, errCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					reinterpret_cast<char*>(&msgBuf), 0, NULL);
				std::string errorDescription;
				if (msgBuf)	{
					errorDescription = reinterpret_cast<char*>(msgBuf);
					LocalFree(msgBuf);
				}
				return errorDescription;
#else
				return strerror(errCode);
#endif
			}
		}
	}
}

#endif