/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hmssqldataset.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HMSSQLDATASET_H
#define HMSSQLDATASET_H

#include "whale/data/hmssql.h"
#include "whale/basic/hlog.h"

namespace Whale
{
	namespace Data
	{
		class HMssqlDataSet
		{
		public:
			explicit HMssqlDataSet(
				const std::weak_ptr<Whale::Data::HMssql> &connection);
			virtual ~HMssqlDataSet();

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
			bool executeSelect(const char *format, Args... args);

			template<typename... Args>
			bool executeNoneSelect(const char *format, Args... args);

			bool executeProcedure(const std::string& procedure,
				const std::vector<std::tuple<std::string, std::string, int>> &iparams,
				std::vector<std::tuple<std::string, std::string, int>> &oparams);

		private:
			explicit HMssqlDataSet();
			bool execute();

			bool isSelect_ = false;
			bool isFirstMove_ = false;
			std::string script_ = "";
			std::string encode_;

			void* cmdPtr_ = 0;
			std::weak_ptr<Whale::Data::HMssql> connection_;

#ifdef OS_WIN32
			_RecordsetPtr dataset_ = 0;
#endif	

//#ifdef OS_LINUX
//			DBPROCESS* conn_ = 0;
//#endif
		};

		template<typename... Args>
		bool HMssqlDataSet::executeSelect(const char *format, Args... args)
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
		bool HMssqlDataSet::executeNoneSelect(const char *format, Args... args)
		{
			assert(format != 0);

			size_t size = snprintf(nullptr, 0, format, args...);
			script_.reserve(size + 1);
			script_.resize(size);
			snprintf(&script_[0], size + 1, format, args...);

			isSelect_ = false;

			return execute();
		}
	}
}

#endif
