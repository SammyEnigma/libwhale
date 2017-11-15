/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hhttpclient.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Replace Version	:	1.0
* Original Author	:	Mr Li
* Finished Date		:	2017/05/05 09:58:00
****************************************************************************************/
#ifndef HHTTPCLIENT_H
#define HHTTPCLIENT_H

#include "whale.h"

namespace Whale
{
	namespace Net
	{
		class HHttpClient
		{
		public:
			explicit HHttpClient(void);
			virtual ~HHttpClient(void);

			virtual std::string get(
				const std::string& url, 
				const std::string& params,
				const std::string& charset,
				const std::string& user,
				const std::string& passwd,
				int millSeconds);

			virtual std::string post(
				const std::string& url,
				const std::string& params,
				const std::string& charset,
				const std::string& user,
				const std::string& passwd,
				int millSeconds);

			virtual std::string upload(
				const std::string& url,
				const std::string& filePath,
				const std::string& charset,
				const std::string& user,
				const std::string& passwd,
				int millSeconds);

			virtual int download(
				const std::string& url,
				const std::string& filePath,
				const std::string& charset = "UTF-8",
				const std::string& user = "",
				const std::string& passwd = "",
				int millSeconds = 75000);

		private:
			std::string currentPath_;
		};
	}
}

#endif

