/***************************************************************************************
* Copyright © 2008-2016, GuangDong CreateW Technology Incorporated Co., Ltd.
* All Rights Reserved.
*
* File Name			:	hpostgresql.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HPOSTGRESQL_H
#define HPOSTGRESQL_H

#undef min
#undef max
#include "pqxx/pqxx"
#undef NOMINMAX

#include "whale/basic/hthread.h"
#include "whale/basic/hdatetime.h"

namespace Whale 
{
	namespace Data
	{
		class HPostgreSql : public Whale::Basic::HThread
		{
			DECLARE_FRIEND_CLASS(HPostgreSqlDataSet)

		public:
			explicit HPostgreSql();
			virtual ~HPostgreSql();

			virtual bool init(
				const std::string& addr, int port, 
				const std::string& name, const std::string& user, 
				const std::string& passwd);
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

			int port_;
			std::string addr_;
			std::string name_;
			std::string user_;
			std::string passwd_;
			bool initialized_;

			mutable std::mutex locker_;
			std::map<std::thread::id, std::tuple<void*, Whale::Basic::HTimer>> connections_;
		};
	}
}

#endif
