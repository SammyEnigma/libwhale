/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hredis.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HREDIS_H
#define HREDIS_H

#include "whale/basic/hthread.h"
#include "whale/basic/hdatetime.h"

#ifdef OS_WIN32
#pragma warning(disable:4099)
#endif

#include <hiredis/hiredis.h>

namespace Whale 
{
	namespace Data
	{
		class HRedis : public Whale::Basic::HThread
		{
			DECLARE_FRIEND_CLASS(HRedisDataSet)

		public:
			explicit HRedis();
			virtual ~HRedis();

			virtual bool init(const std::string& addr, 
				int port, const std::string& pwd,
				const std::string& encode = "UTF-8");
			virtual void stop();	

			inline std::string addr() const {
				return addr_;
			}
			inline void addr(std::string val) {
				addr_ = val;
			}

			inline int port() const {
				return port_;
			}
			inline void port(int val) {
				port_ = val;
			}

			inline std::string passwd() const {
				return passwd_;
			}
			inline void passwd(std::string val) {
				passwd_ = val;
			}

			inline std::string encode() const {
				return encode_;
			}
			inline void encode(std::string val) {
				encode_ = val;
			}

		private:
			virtual void action();
			virtual void *connection();
			virtual bool build();
			virtual bool release();

			bool initialized_;
			std::string addr_;
			int port_;
			std::string passwd_;
			std::string encode_;

			mutable std::mutex locker_;
			std::map<std::thread::id, std::tuple<void*, Whale::Basic::HTimer>> connections_;
		};
	}
}
#endif
