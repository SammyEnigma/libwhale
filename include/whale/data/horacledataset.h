/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	horacledataset.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HORACLEDATASET_H
#define HORACLEDATASET_H

#include "whale/basic/hlog.h"
#include "whale/data/horacle.h"
#include <ocilib.hpp>

namespace Whale 
{
	namespace Data
	{
		class HOracleDataSet
		{
		public:
			explicit HOracleDataSet(const std::weak_ptr<Whale::Data::HOracle> &connObj);
			virtual ~HOracleDataSet();

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
			bool executeSelect(const char *sql, Args... args);

			template<typename... Args>
			bool executeNoneSelect(const char *sql, Args... args);

			template<typename... Args>
			bool prepare(const char *sql, Args... args);

			bool bind(const char *outKey, char *outValue, int maxmium);
			bool execute();

		private:
			explicit HOracleDataSet();

			void release();
			bool executeStmt();

			bool isSelect_ = false;
			std::string script_ = "";
			std::string encode_;

			OCI_Resultset* dataset_ = 0;
			OCI_Statement* statement_ = 0;
			std::weak_ptr<Whale::Data::HOracle> connection_;
		};

		template<typename... Args>
		bool HOracleDataSet::executeSelect(const char *format, Args... args)
		{
			assert(format);

			size_t size = snprintf(nullptr, 0, format, args...);
			script_.reserve(size + 1);
			script_.resize(size);
			snprintf(&script_[0], size + 1, format, args...);
			isSelect_ = true;

			return executeStmt();
		}

		template<typename... Args>
		bool HOracleDataSet::executeNoneSelect(const char *format, Args... args)
		{
			assert(format);

			size_t size = snprintf(nullptr, 0, format, args...);
			script_.reserve(size + 1);
			script_.resize(size);
			snprintf(&script_[0], size + 1, format, args...);
			isSelect_ = false;

			return executeStmt();
		}

		template<typename... Args>
		bool HOracleDataSet::prepare(const char *format, Args... args)
		{
			assert(format);

			size_t size = snprintf(nullptr, 0, format, args...);
			script_.reserve(size + 1);
			script_.resize(size);
			snprintf(&script_[0], size + 1, format, args...);

			return OCI_Prepare(statement_, script_.c_str()) == TRUE;
		}
	}
}

#endif
