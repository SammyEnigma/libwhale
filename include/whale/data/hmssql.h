/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hmssql.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HMSSQL_H
#define HMSSQL_H

#include "whale/basic/hplatform.h"

#ifdef OS_WIN32
#pragma warning(disable:4146)
#import "z:/libwhale/deps/ado-1.0/msado15.dll" no_namespace rename("EOF", "adoEOF")
#endif

namespace Whale 
{
	namespace Data
	{
		class HMssql
		{
		public:
			explicit HMssql();
			virtual ~HMssql();

			virtual bool init(const std::string& addr, int port, 
				const std::string& name,
				const std::string& user, const std::string& passwd);
			virtual void close();
			virtual void* connection();
			virtual bool build();
			virtual bool release();

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
			bool initialized_;
			int port_;
			std::string addr_;			
			std::string name_;
			std::string user_;
			std::string passwd_;

			mutable std::mutex _locker;

#ifdef OS_WIN32
			std::map<std::thread::id, _ConnectionPtr*> connections_;
#endif

//#ifdef OS_LINUX
//			std::map<std::thread::id, DBPROCESS*> connections_;
//#endif
		};
	}
}

#endif