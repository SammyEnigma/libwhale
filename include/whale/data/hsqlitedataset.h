/***************************************************************************************
* Copyright © 2008-2016, GuangDong CreateW Technology Incorporated Co., Ltd.
* All Rights Reserved.
*
* File Name			:	sqlitedataset.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef SQLITEDATASET_H
#define SQLITEDATASET_H

#include "whale/data/hsqlite.h"
#include "whale/basic/hlog.h"

#include <sqlite3.h>

namespace Whale 
{
	namespace Data
	{
		class HSqliteDataSet
		{
		public:
			explicit HSqliteDataSet(
				const std::weak_ptr<Whale::Data::HSqlite>& connection);
			virtual ~HSqliteDataSet();

			bool next();
			void close();
			void clear();
			
			int readInt32(int index);
			long long readInt64(int index);
			double readDouble(int index);
			std::string readString(int index);

			int columns(void);
			std::string columnName(int index);

			template<typename... Args>
			bool executeSelect(const char *formart, Args... args);

			template<typename... Args>
			bool executeNoneSelect(const char *formart, Args... args);

			template<typename... Args>
			bool prepare(const char *formart, Args... args);
			
			bool bind(int index, int val);
			bool bind(int index, double val);
			bool bind(int index, const std::string& val);
			
			bool excute();
			bool begin();
			bool rollback();
			bool commit();

		private:
			std::string script_ = "";
			mutable std::mutex locker_;
			sqlite3_stmt* sqlite3stmt_ = nullptr;
			bool isSelect_ = false;

			std::weak_ptr<Whale::Data::HSqlite> connection_;
		};

		template<typename... Args>
		bool HSqliteDataSet::executeSelect(const char *format, Args... args)
		{
			if (connection_.expired()) { 
				return false; 
			} 

			auto connObj = connection_.lock(); 

			if (!connObj) { 
				return false; 
			}

			assert(format != 0);
			size_t scriptLen = snprintf(nullptr, 0, format, args...);
			script_.reserve(scriptLen + 1);
			script_.resize(scriptLen);
			snprintf(&script_[0], scriptLen + 1, format, args...);

			isSelect_ = true;

			std::lock_guard <std::mutex> lock(locker_);

			sqlite3_finalize(sqlite3stmt_);

			bool prepareResult = false;

#ifdef  UNICODE 
			prepareResult = sqlite3_prepare16_v2(static_cast<sqlite3*>(connObj->connection()), script_.c_str(), -1, &sqlite3stmt_, NULL) == SQLITE_OK;
#else
			prepareResult = sqlite3_prepare_v2(static_cast<sqlite3*>(connObj->connection()), script_.c_str(), -1, &sqlite3stmt_, NULL) == SQLITE_OK;
#endif		

			if (!prepareResult)	{
				log_error("Err: %s, SqlScript: %s", sqlite3_errmsg(static_cast<sqlite3*>(connObj->connection())), script_.c_str());
			}

			return prepareResult;
		}

		template<typename... Args>
		bool HSqliteDataSet::executeNoneSelect(const char *format, Args... args)
		{
			if (connection_.expired()) { return false; } auto dbObj = connection_.lock(); if (!dbObj) { return false; }

			assert(format != 0);

			size_t scriptLen = snprintf(nullptr, 0, format, args...);
			script_.reserve(scriptLen + 1);
			script_.resize(scriptLen);
			snprintf(&script_[0], scriptLen + 1, format, args...);

			isSelect_ = false;

			std::lock_guard <std::mutex> lock(locker_);

			bool prepareResult = false;

#ifdef  UNICODE 
			prepareResult = sqlite3_prepare16_v2(dbObj->connection_, script_.c_str(), -1, &sqlite3stmt_, NULL) == SQLITE_OK;
#else
			prepareResult = sqlite3_prepare_v2(dbObj->connection_, script_.c_str(), -1, &sqlite3stmt_, NULL) == SQLITE_OK;
#endif		

			if (!prepareResult)	{
				log_error("Err: %s, SqlScript: %s", sqlite3_errmsg(dbObj->connection_), script_.c_str());
				return false;
			}

			sqlite3_step(sqlite3stmt_);

			return (sqlite3_finalize(sqlite3stmt_) == SQLITE_OK) ? true : false;
		}

		template<typename... Args>
		bool HSqliteDataSet::prepare(const char *format, Args... args)
		{
			if (connection_.expired()) { return false; }
			auto sqliteObject = connection_.lock();
			if (!sqliteObject) { return false; }

			assert(format != 0);
			size_t size = snprintf(nullptr, 0, format, args...);
			script_.reserve(size + 1);
			script_.resize(size);
			snprintf(&script_[0], size + 1, format, args...);

			std::lock_guard <std::mutex> lock(locker_);

			sqlite3_finalize(sqlite3stmt_);

			bool prepareResult = false;

#ifdef  UNICODE 
			prepareResult = sqlite3_prepare16_v2(sqliteObject->connection_, script_.c_str(), -1, &sqlite3stmt_, NULL) == SQLITE_OK;
#else
			prepareResult = sqlite3_prepare_v2(sqliteObject->connection_, script_.c_str(), -1, &sqlite3stmt_, NULL) == SQLITE_OK;
#endif		

			if (!prepareResult)	{
				log_error("Err: %s, SqlScript: %s", sqlite3_errmsg(sqliteObject->connection_), script_.c_str());
			}

			return prepareResult;
		}
	}
}

#endif
