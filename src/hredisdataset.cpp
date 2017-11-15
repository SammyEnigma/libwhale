#include "whale/data/hredisdataset.h"


namespace Whale
{
	namespace Data
	{
		HRedisDataSet::HRedisDataSet(const std::weak_ptr<Whale::Data::HRedis> &connection)
		{
			connection_ = connection;
			assert(!connection_.expired());
			auto connectionI = connection_.lock();
			assert(connectionI);
			encode_ = connectionI->encode();
		}

		HRedisDataSet::~HRedisDataSet()
		{
			release();
		}

		void HRedisDataSet::release()
		{
			if (dataset_ != 0)
			{
				freeReplyObject(dataset_);
				dataset_ = 0;
			}
		}

		long HRedisDataSet::count(int replyIndex)
		{
			if (!dataset_) {
				throw std::runtime_error("_dataset is null.");
			}

			if (replyIndex < 0)
			{
				return dataset_->elements;
			}

			if (replyIndex >= static_cast<int>(dataset_->elements)) {
				throw std::runtime_error(Whale::Util::HString::format("redis_reply_index %d is equal or greater than _dataset->elements %d.",
					replyIndex, dataset_->elements).c_str());
			}

			return dataset_->element[replyIndex]->elements;
		}

		std::string HRedisDataSet::readString(int index)
		{
			if (dataset_ == 0) {
				throw std::runtime_error("_dataset is null.");
			}

			if (index < 0)
			{
				return dataset_->str == 0 ? "" : dataset_->str;
			}

			if (index >= static_cast<int>(dataset_->elements)) {
				throw std::runtime_error(Whale::Util::HString::format("index %d is equal or greater than _dataset->elements %d.",
					index, dataset_->elements).c_str());
			}

			if (dataset_->type != REDIS_REPLY_ARRAY) {
				throw std::runtime_error("_dataset->type != REDIS_REPLY_ARRAY.");
			}

			return dataset_->element[index]->str == 0 ? "" : dataset_->element[index]->str;
		}

		std::string HRedisDataSet::readMultReplyString(int replyIndex, int index)
		{
			if (!dataset_) {
				throw std::runtime_error("_dataset is null.");
			}

			if (replyIndex < 0)
			{
				return dataset_->str == 0 ? "" : dataset_->str;
			}

			if (replyIndex >= static_cast<int>(dataset_->elements))	{
				throw std::runtime_error(Whale::Util::HString::format("redis_reply_index %d is equal or greater than _dataset->elements %d.",
					replyIndex, dataset_->elements).c_str());
			}

			if (dataset_->type != REDIS_REPLY_ARRAY) {
				throw std::runtime_error("_dataset->type != REDIS_REPLY_ARRAY.");
			}

			if (index < 0)
			{
				return dataset_->element[replyIndex]->str == 0 ? "" : dataset_->element[replyIndex]->str;
			}

			if (index >= static_cast<int>(dataset_->element[replyIndex]->elements))
			{
				throw std::runtime_error(Whale::Util::HString::format("index %d is equal or greater than _dataset->element[%d]->elements %d.",
					index, replyIndex, dataset_->element[replyIndex]->elements).c_str());
			}

			return dataset_->element[replyIndex]->element[index]->str == 0 ? "" : dataset_->element[replyIndex]->element[index]->str;
		}

		uint32_t HRedisDataSet::readUInt32(int index)
		{
			if (!dataset_) {
				throw std::runtime_error("_dataset is null.");
			}

			if (index < 0) {
				return dataset_->str == 0 ? 0 : strtoul(dataset_->str, NULL, 10);
			}

			if (index >= static_cast<int>(dataset_->elements)) {
				return static_cast<uint32_t>(dataset_->integer);
			}

			if (dataset_->type != REDIS_REPLY_ARRAY) {
				throw std::runtime_error("_dataset->type != REDIS_REPLY_ARRAY.");
			}

			return dataset_->element[index]->str == 0 ? 0 : strtoul(dataset_->element[index]->str, NULL, 10);
		}

		uint32_t HRedisDataSet::readMultReplyInt32(int replyIndex, int index)
		{		
			if (!dataset_) {
				throw std::runtime_error("_dataset is null.");
			}

			if (replyIndex < 0)
			{
				return dataset_->str == 0 ? 0 : strtoul(dataset_->str, NULL, 0);
			}

			if (replyIndex >= static_cast<int>(dataset_->elements))
			{
				throw std::runtime_error(Whale::Util::HString::format("redis_reply_index %d is equal or greater than _dataset->elements %d.",
					replyIndex, dataset_->elements).c_str());
			}

			if (dataset_->type != REDIS_REPLY_ARRAY) {
				throw std::runtime_error("_dataset->type != REDIS_REPLY_ARRAY.");
			}

			if (index < 0) {
				return dataset_->element[replyIndex]->str == 0 ? 0 : strtoul(dataset_->element[replyIndex]->str, NULL, 10);
			}

			if (index >= static_cast<int>(dataset_->element[replyIndex]->elements)) {
				throw std::runtime_error(Whale::Util::HString::format("index %d is equal or greater than _dataset->element[%d]->elements %d.",
					index, replyIndex, dataset_->element[replyIndex]->elements).c_str());
			}

			return dataset_->element[replyIndex]->element[index]->str == 0 ? 0 : strtoul(dataset_->element[replyIndex]->element[index]->str, NULL, 0);
		}

		uint64_t HRedisDataSet::readUInt64(int index)
		{
			if (!dataset_) {
				throw std::runtime_error("_dataset is null.");
			}

			if (index < 0)
			{
				return dataset_->str == 0 ? 0 : strtoull(dataset_->str, NULL, 10);
			}

			if (index >= static_cast<int>(dataset_->elements)) {
				return static_cast<uint64_t>(dataset_->integer);
			}

			if (dataset_->type != REDIS_REPLY_ARRAY) {
				throw std::runtime_error("_dataset->type != REDIS_REPLY_ARRAY.");
			}

			return dataset_->element[index]->str == 0 ? 0 : strtoull(dataset_->element[index]->str, NULL, 0);
		}

		uint64_t HRedisDataSet::readMultReplyUInt64(int replyIndex, int index)
		{
			if (!dataset_) {
				throw std::runtime_error("_dataset is null.");
			}

			if (replyIndex < 0)
			{
				return dataset_->str == 0 ? 0 : strtoull(dataset_->str, NULL, 0);
			}

			if (replyIndex >= static_cast<int>(dataset_->elements)) {
				throw std::runtime_error(Whale::Util::HString::format("redis_reply_index %d is equal or greater than _dataset->elements %d.",
					replyIndex, dataset_->elements).c_str());
			}

			if (dataset_->type != REDIS_REPLY_ARRAY) {
				throw std::runtime_error("_dataset->type != REDIS_REPLY_ARRAY.");
			}

			if (index < 0)
			{
				return dataset_->element[replyIndex]->str == 0 ? 0 : strtoull(dataset_->element[replyIndex]->str, NULL, 0);
			}

			if (index >= static_cast<int>(dataset_->element[replyIndex]->elements))	{
				throw std::runtime_error(Whale::Util::HString::format("index %d is equal or greater than _dataset->element[%d]->elements %d.",
					index, replyIndex, dataset_->element[replyIndex]->elements).c_str());
			}

			return dataset_->element[replyIndex]->element[index]->str == 0 ? 0 : strtoull(dataset_->element[replyIndex]->element[index]->str, NULL, 0);
		}

		int HRedisDataSet::isHexDigit(char c)
		{
			return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
		}

		int HRedisDataSet::hexDigit2Int(char c)
		{
			switch (c)
			{
			case '0': return 0;
			case '1': return 1;
			case '2': return 2;
			case '3': return 3;
			case '4': return 4;
			case '5': return 5;
			case '6': return 6;
			case '7': return 7;
			case '8': return 8;
			case '9': return 9;
			case 'a': case 'A': return 10;
			case 'b': case 'B': return 11;
			case 'c': case 'C': return 12;
			case 'd': case 'D': return 13;
			case 'e': case 'E': return 14;
			case 'f': case 'F': return 15;
			default: return 0;
			}
		}

		bool HRedisDataSet::splitParam(const char *line, std::vector<std::string> &params)
		{
			const char *p = line;
			std::string argv = "";

			while (true) {
				while (*p && isspace(*p)) {
					p++;
				}

				if (*p)	{
					/* get a token */
					int inq = 0;  /* set to 1 if we are in "quotes" */
					int insq = 0; /* set to 1 if we are in 'single quotes' */
					int done = 0;

					while (!done) {
						if (inq) {
							if (*p == '\\' && *(p + 1) == 'x' && isHexDigit(*(p + 2)) && isHexDigit(*(p + 3))) {
								unsigned char bt;

								bt = (hexDigit2Int(*(p + 2)) * 16) + hexDigit2Int(*(p + 3));
								argv += (char*)&bt;
								p += 3;
							}
							else if (*p == '\\' && *(p + 1)) {
								char c;
								p++;
								switch (*p)	{
								case 'n': c = '\n'; break;
								case 'r': c = '\r'; break;
								case 't': c = '\t'; break;
								case 'b': c = '\b'; break;
								case 'a': c = '\a'; break;
								default: c = *p; break;
								}
								argv += &c;
							}
							else if (*p == '"')	{
								/* closing quote must be followed by a space or
								* nothing at all. */
								if (*(p + 1) && !isspace(*(p + 1))) return false;
								done = 1;
							}
							else if (!*p) {
								/* unterminated quotes */
								return false;
							}
							else {
								argv += p[0];
							}
						}
						else if (insq) {
							if (*p == '\\' && *(p + 1) == '\'')	{
								p++;
								argv += "'";
							}
							else if (*p == '\'') {
								/* closing quote must be followed by a space or
								* nothing at all. */
								if (*(p + 1) && !isspace(*(p + 1))) return false;
								done = 1;
							}
							else if (!*p) {
								/* unterminated quotes */
								return false;
							}
							else {
								argv += p[0];
							}
						}
						else {
							switch (*p)	{
							case ' ':
							case '\n':
							case '\r':
							case '\t':
							case '\0':
								done = 1;
								break;
							case '"':
								inq = 1;
								break;
							case '\'':
								insq = 1;
								break;
							default:
								argv += p[0];
								break;
							}
						}
						if (*p) p++;
					}

					/* add the token to the vector */
					params.push_back(argv);
					argv = "";
				}
				else {
					return true;
				}
			}

			return false;
		}
	}
}