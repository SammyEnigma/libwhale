/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hdns.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HDNS_H
#define HDNS_H

#include "whale/basic/hplatform.h"

namespace Whale 
{
	namespace Net
	{
		namespace HDns
		{
			std::string mac(const std::string& ipaddr);
			std::vector<std::string> addrs(const std::string& domain);
			std::vector<std::string> hosts(const std::string& domain);			
		}
	}
}

#endif
