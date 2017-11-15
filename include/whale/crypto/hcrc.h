/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
* All Rights Reserved.
*
* File Name			:	hcrc.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef __HCRC_H
#define __HCRC_H

#include "whale/basic/hplatform.h"

namespace Whale 
{
	namespace Crypto
	{
		namespace HCrc
		{
			uint8_t	 calcu08(uint8_t* buffer, uint32_t size);
			uint16_t calcu16(uint8_t* buffer, uint32_t size);
			uint32_t calcu32(uint8_t* buffer, uint32_t size);
		};
	}
}
#endif

