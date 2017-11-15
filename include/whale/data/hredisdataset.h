/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hredisdataset.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HREDISDATASET_H
#define HREDISDATASET_H

#include "whale/data/hredis.h"
#include "whale/util/hstring.h"
#include "whale/basic/hlog.h"

namespace Whale 
{
	namespace Data
	{
		class HRedisDataSet
		{
		public:
			explicit HRedisDataSet(const std::weak_ptr<Whale::Data::HRedis> &connObj);
			virtual ~HRedisDataSet();

			long count(int replyIndex = -1);

			std::string readString(int index = -1);
			std::string readMultReplyString(int replyIndex, int index = -1);

			uint32_t readUInt32(int index = -1);
			uint32_t readMultReplyInt32(int replyIndex, int index = -1);

			uint64_t readUInt64(int index = -1);
			uint64_t readMultReplyUInt64(int replyIndex, int index = -1);

			template<typename... Args>
			bool execute(const char *format, Args... args);

		private:
			explicit HRedisDataSet();
			void release();

			bool isSelect_ = false;
			std::string script_ = "";
			std::string encode_;

			redisReply *dataset_ = 0;
			std::weak_ptr<Whale::Data::HRedis> connection_;

			int isHexDigit(char c);
			int hexDigit2Int(char c);
			bool splitParam(const char *line, std::vector<std::string> &params);
		};

		template<typename... Args>
		bool HRedisDataSet::execute(const char *format, Args... args)
		{
			if (format == 0)
				return false;

			size_t size = snprintf(nullptr, 0, format, args...);
			std::string script;
			script.reserve(size + 1);
			script.resize(size);
			snprintf(&script[0], size + 1, format, args...);

			if (connection_.expired())	{
				return false;
			}

			auto connection = connection_.lock();

			if (!connection){
				return false;
			}

			redisContext *context = static_cast<redisContext *>(connection->connection());

			if (!context) {
				return false;
			}				

			std::vector<std::string> params;

			if (!splitParam(script.c_str(), params)) {
				return false;
			}

			int index = 0;
			std::vector<size_t> param_sizes;
			std::vector<const char*> param_argvs;
			for (size_t i = 0; i < params.size(); ++i)	{
				param_sizes.push_back(params[i].length());
				param_argvs.push_back(params[i].c_str());
			}

			redisAppendCommandArgv(context, params.size(), 
				(const char**)&param_argvs[0], &param_sizes[0]);

			release();

			if (redisGetReply(context, (void**)&dataset_) != 0)	{
				connection->release();
				throw std::runtime_error(Whale::Util::HString::format("Function redisGetReply Err: %s", script.c_str()).c_str());
			}

			return true;
		}
	}
}

#endif
