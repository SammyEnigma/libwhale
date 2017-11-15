/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hfile.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HFILE_H
#define HFILE_H

#include "whale/basic/hplatform.h"

namespace Whale
{
	namespace Util
	{
		namespace HFile
		{
			bool write(const char *filePath, const char* buffer, int size, bool append = false);
			bool move(const std::string& fp1, const std::string& fp2);
			bool remove(const std::string& filePath);
		}
	}
}

#endif

