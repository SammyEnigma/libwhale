/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
* All Rights Reserved.
*
* File Name			:	hbcd.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef __HBCD_H
#define __HBCD_H

#include "whale/basic/hplatform.h"

namespace Whale
{
	namespace Crypto
	{
		namespace HBcd
		{
			int encode(unsigned char *in, int size, unsigned char *out, int maxmium);
			int decode(unsigned char *in, int size, unsigned char *out, int maxmium);
		}
	}
}

#endif