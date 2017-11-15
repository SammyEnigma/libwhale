/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hmongo.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HMONGO_H
#define HMONGO_H

#include "whale/basic/hthread.h"
#include "whale/basic/hdatetime.h"

namespace Whale
{
	namespace Data
	{
		class HMongo : public Whale::Basic::HThread
		{
			DECLARE_FRIEND_CLASS(HMongoDataSet)

		public:
			explicit HMongo();
			virtual ~HMongo();

			//mongodb://127.0.0.1:27017
			virtual bool init(const std::string& connStr);
			virtual void stop();

			virtual void *connection();

		private:
			virtual void action();			
			virtual bool disconnect();

			bool initialized_;
			std::string connStr_;

			mutable std::mutex mutex_;
			std::map<std::thread::id, std::tuple<void*, Whale::Basic::HTimer>> connections_;
		};
	}
}

#endif
