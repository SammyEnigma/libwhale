/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hmysqldataset.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HMYSQLDATASET_H
#define HMYSQLDATASET_H

#include "whale/basic/hlog.h"
#include "whale/data/hmysql.h"

#ifdef OS_WIN32
#pragma warning(disable:4251)
#endif

#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#include "whale/util/hstring.h"

namespace format
{
	class ResultSet;
	class Connection;
	class Statement;
}

namespace Whale
{
	namespace Data
	{
		class HMysqlDataSet
		{
		public:
			explicit HMysqlDataSet(
				const std::weak_ptr<Whale::Data::HMysql> &connection);
			virtual ~HMysqlDataSet();

			long count();
			bool next();

			long columns();
			std::string columnName(int index);			

			long readInt32(int index);
			long readInt32(const char *clName);

			long long readInt64(int index);
			long long readInt64(const char *clName);

			std::string readString(int index);
			std::string readString(const char *clName);

			template<typename... Args>
			bool executeSelect(const char *sql, Args... args);

			template<typename... Args>
			bool executeNoneSelect(const char *sql, Args... args);

		private:
			explicit HMysqlDataSet();
			bool execute();

			bool isSelect_ = false;
			std::string script_ = "";

			void* cmdPtr_ = 0;
			sql::ResultSet* dataset_ = 0;
			sql::Connection* conn_ = 0;
			std::weak_ptr<Whale::Data::HMysql> connection_;
		};

		template<typename... Args>
		bool HMysqlDataSet::executeSelect(const char *format, Args... args)
		{
			assert(format != 0);

			size_t size = snprintf(nullptr, 0, format, args...);
			script_.reserve(size + 1);
			script_.resize(size);
			snprintf(&script_[0], size + 1, format, args...);
			isSelect_ = true;

			return execute();
		}

		template<typename... Args>
		bool HMysqlDataSet::executeNoneSelect(const char *sql, Args... args)
		{
			assert(sql != 0);

			size_t size = snprintf(nullptr, 0, sql, args...);
			script_.reserve(size + 1);
			script_.resize(size);
			snprintf(&script_[0], size + 1, sql, args...);
			isSelect_ = false;

			return execute();
		}
	}
}

#endif
