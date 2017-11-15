/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hmysql.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HMYSQL_H
#define HMYSQL_H

#include "whale/basic/hthread.h"
#include "whale/basic/hdatetime.h"

#ifdef OS_WIN32
#pragma warning(disable:4251)
#endif

namespace Whale 
{
	namespace Data
	{
		class HMysql : public Whale::Basic::HThread
		{
			DECLARE_FRIEND_CLASS(HMysqlDataSet)

		public:
			explicit HMysql();
			virtual ~HMysql();

			virtual bool init(const std::string& addr, int port, 
				const std::string& name, 
				const std::string& user, const std::string& passwd);
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

			inline std::string name() const {
				return name_;
			}
			inline void name(std::string val) {
				name_ = val;
			}

			inline std::string user() const {
				return user_;
			}
			inline void user(std::string val) {
				user_ = val;
			}

			inline std::string passwd() const {
				return passwd_;
			}
			inline void passwd(std::string val) {
				passwd_ = val;
			}

		private:
			virtual void action();
			virtual void *connection();
			virtual bool build();
			virtual bool release();

			bool initialized_;
			int port_;
			std::string addr_;
			std::string name_;
			std::string user_;
			std::string passwd_;

			mutable std::mutex locker_;
			std::map<std::thread::id, std::tuple<void*, Whale::Basic::HTimer>> connections_;
		};
	}
}

#endif
