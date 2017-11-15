/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hdir.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HDIR_H
#define HDIR_H

#include "whale/basic/hplatform.h"

namespace Whale
{
	namespace Util
	{
		namespace HDir
		{
			bool mkdir(const std::string& dir);
			bool traversal(const std::string& dir, std::list<std::string>& filelist, int depth = 0);
			bool traversalFolder(const std::string& dir, std::list<std::string>& dirlist, int depth = 0);
			bool remove(const std::string& dir);
		}
	}
}

#endif

