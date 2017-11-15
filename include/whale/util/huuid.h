/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	huuid.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HUUID_H
#define HUUID_H

#include "whale/basic/hplatform.h"

namespace Whale 
{
	namespace Util
	{
		namespace HUuid 
		{
			std::string gen();
			std::string gens();
		}
	}
}

#endif
