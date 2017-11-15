/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hconnection.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HCONNECTION_H
#define HCONNECTION_H

#include "whale/basic/hthread.h"
#include "whale/basic/hdatetime.h"

#define OTL_ODBC // Compile OTL 4/ODBC
#define OTL_STL

#ifdef OS_LINUX
#define OTL_ODBC_UNIX
#endif

#include <otlv4.h> // include the OTL 4 header file

namespace Whale 
{
	namespace Data
	{
		class HConnection : public Whale::Basic::HThread
		{
			DECLARE_FRIEND_CLASS(HDataSet)

		public:
			explicit HConnection();
			virtual ~HConnection();			

			inline std::string connStr() const {
				return connStr_;
			}

			inline bool initialized() const {
				return initialized_;
			}

			virtual bool init(const std::string& connStr);
			virtual void stop();

			virtual otl_connect *connect();
			virtual bool disconnect();

		private:
			virtual void action();

			bool initialized_;
			std::string connStr_;
			mutable std::mutex mutex_;
			std::map<std::thread::id, std::tuple<otl_connect*, Whale::Basic::HTimer>> connects_;
		};
	}
}

#endif
