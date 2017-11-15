/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
* All Rights Reserved.
*
* File Name			:	hbase64.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef __HBASE64_H
#define __HBASE64_H

#include "whale/basic/hbytearray.h"

namespace Whale
{
	namespace Crypto
	{
		namespace HBase64
		{
			std::string encode(const Whale::Basic::HByteArray& plain);
			Whale::Basic::HByteArray decode(const std::string& cipher);
		}		
	}
}

#endif

