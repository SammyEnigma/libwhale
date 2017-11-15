/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
* All Rights Reserved.
*
* File Name			:	hsqlite.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HSQLITE_H
#define HSQLITE_H

#include "whale/basic/hthread.h"
#include "whale/basic/hdatetime.h"

struct sqlite3;

namespace Whale
{
	namespace Data
	{
		class HSqlite : public Whale::Basic::HThread
		{
			DECLARE_FRIEND_CLASS(HSqliteDataSet)

		public:
			explicit HSqlite(void);
			virtual ~HSqlite(void);

			virtual bool init(const std::string& filePath);
			virtual void stop();

			inline std::string filePath() const {
				return filePath_;
			}
			inline void filePath(std::string val) {
				filePath_ = val;
			}

		private:
			virtual void action();
			virtual void *connection();

			std::string filePath_;
			sqlite3* connection_ = 0;
		};		
	}
}
#endif
