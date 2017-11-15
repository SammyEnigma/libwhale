/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hzip.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HZIP_H
#define HZIP_H

#include "whale/basic/hplatform.h"
#include "whale/basic/hbytearray.h"

namespace Whale
{
	namespace Crypto
	{
		namespace HZip
		{
			/* 数据压缩 */
			Whale::Basic::HByteArray deflate(const Whale::Basic::HByteArray& content);

			/* 数据解压 */
			Whale::Basic::HByteArray inflate(const Whale::Basic::HByteArray& content);
		}
	}
}

#endif

