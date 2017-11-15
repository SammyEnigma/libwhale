/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hmongodataset.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HMONGODATASET_H
#define HMONGODATASET_H

#include "whale/data/hmongo.h"
#include "whale/basic/hlog.h"
#include "whale/util/hjson.h"

#ifdef OS_WIN32
#pragma warning(disable:4005)
#pragma warning(disable:4150)
#endif

#include <bson.h>
#include <bcon.h>
#include <mongoc.h>

namespace Whale
{
	namespace Data
	{
		class HMongoDataSet
		{
		public:
			explicit HMongoDataSet() = delete;
			explicit HMongoDataSet(const std::weak_ptr<Whale::Data::HMongo> &connection,
				const std::string& db, const std::string& collection);
			virtual ~HMongoDataSet();

			int	 count();
			bool next(Whale::Util::HJson& obj);

			bool insert(Whale::Util::HJson& obj);
			bool insertBulk(std::vector<Whale::Util::HJson>& objs);

			bool remove(Whale::Util::HJson& obj);
			bool update(Whale::Util::HJson& obj);
			bool select(Whale::Util::HJson& obj);
			bool execute(Whale::Util::HJson& obj);

		private:
			int count_;
			mongoc_cursor_t *cursor_;
			mongoc_collection_t *dataset_;
			
			std::weak_ptr<Whale::Data::HMongo> connection_;
		};
	}
}

#endif
