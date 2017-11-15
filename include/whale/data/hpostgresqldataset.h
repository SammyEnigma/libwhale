/***************************************************************************************
* Copyright © 2008-2016, GuangDong CreateW Technology Incorporated Co., Ltd.
* All Rights Reserved.
*
* File Name			:	postgresqldataset.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef POSTGRESQLDATASET_H
#define POSTGRESQLDATASET_H

#include "whale/util/hstring.h"
#include "whale/basic/hlog.h"
#include "whale/data/hpostgresql.h"

namespace Whale 
{
	namespace Data
	{
		class HPostgreSqlDataSet
		{
		public:
			explicit HPostgreSqlDataSet(const std::weak_ptr<Whale::Data::HPostgreSql> &connObj);
			virtual ~HPostgreSqlDataSet();

			long count();
			bool next();

			long columns();
			std::string columnName(int index);			

			long readInt32(int index);
			long readInt32(const char *clname);

			long long readInt64(int index);
			long long readInt64(const char *clname);

			std::string readString(int index);
			std::string readString(const char *clname);

			template<typename... Args>
			bool executeSelect(char *sql, Args... args);

			template<typename... Args>
			bool executeNoneSelect(char *sql, Args... args);

		private:
			explicit HPostgreSqlDataSet();
			bool execute();

			bool isSelect_ = false;

			int index_ = -1;
			std::string script_ = "";

			pqxx::result dataset_;
			pqxx::connection* conn_ = 0;
			std::weak_ptr<Whale::Data::HPostgreSql> connection_;
		};

		template<typename... Args>
		bool HPostgreSqlDataSet::executeSelect(char *format, Args... args)
		{
			assert(format != 0);

			size_t size = snprintf(nullptr, 0, format, args...);
			script_.reserve(size + 1);
			script_.resize(size);
			snprintf(&script_[0], size + 1, format, args...);
#ifdef OS_WIN32
			if (Whale::Util::HString::toUpper(encode_).compare("UTF-8") == 0) {
				script_ = Whale::Util::HString::stringToUTF8(script_);
			}
#else
			if (Whale::Util::HString::toUpper(encode_).compare("GBK") == 0) {
				script_ = Whale::Util::HString::utf8ToString(script_);
			}
#endif // OS_WIN32
			isSelect_ = true;

			return execute();
		}

		template<typename... Args>
		bool HPostgreSqlDataSet::executeNoneSelect(char *format, Args... args)
		{
			assert(format != 0);

			size_t size = snprintf(nullptr, 0, format, args...);
			script_.reserve(size + 1);
			script_.resize(size);
			snprintf(&script_[0], size + 1, format, args...);
#ifdef OS_WIN32
			if (Whale::Util::HString::toUpper(encode_).compare("UTF-8") == 0) {
				script_ = Whale::Util::HString::stringToUTF8(script_);
			}
#else
			if (Whale::Util::HString::toUpper(encode_).compare("GBK") == 0) {
				script_ = Whale::Util::HString::utf8ToString(script_);
			}
#endif // OS_WIN32
			isSelect_ = false;

			return execute();
		}
	}
}

#endif