#include "whale/data/hmongodataset.h"
#include "whale/basic/hlog.h"

namespace Whale
{
	namespace Data
	{
		HMongoDataSet::HMongoDataSet(const std::weak_ptr<Whale::Data::HMongo> &connection,
			const std::string& db, const std::string& collection)
		{
			cursor_ = NULL;
			connection_ = connection;
			assert(!connection_.expired());

			auto connections = connection_.lock();
			assert(connections);

			dataset_ = mongoc_client_get_collection(
				static_cast<mongoc_client_t*>(connections->connection()), 
				db.c_str(), collection.c_str());
		}

		HMongoDataSet::~HMongoDataSet()
		{
			if (cursor_)
				mongoc_cursor_destroy(cursor_);

			if (dataset_) 
				mongoc_collection_destroy(dataset_);			
		}

		int HMongoDataSet::count()
		{
			return count_;
		}

		bool HMongoDataSet::next(Whale::Util::HJson& obj)
		{
			//log_info("ZZZZZZZZZZZZZZ 1");

			if (!cursor_ || 
				!mongoc_cursor_more(cursor_)) {
				return false;
			}

			char *str;
			const bson_t *doc;

			//log_info("ZZZZZZZZZZZZZZ 2");

			if (mongoc_cursor_next(cursor_, &doc)) {
				str = bson_as_json(doc, NULL);
				obj.decode(str);
				bson_free(str);
				//log_info("ZZZZZZZZZZZZZZ 3");
				return true;
			}
			

			return false;
		}

		bool HMongoDataSet::insert(Whale::Util::HJson& obj)
		{
			if (connection_.expired()) { return false; }
			auto connObj = connection_.lock();
			if (!connObj) { return false; }

			try	{
				bson_error_t berr;
				bson_t      *bson;

				bson = bson_new_from_json((const uint8_t *)obj.encode(obj).c_str(), -1, &berr);

				if (!bson) {
					log_error("%s", berr.message);
					return false;
				}
				
				if (!mongoc_collection_insert(dataset_, MONGOC_INSERT_NONE, bson, NULL, &berr)) {
					log_error("Err: %s", berr.message);
					return false;
				}

				bson_destroy(bson);

				return true;
			}
			catch (std::exception &e)
			{
				connObj->disconnect();
				throw std::runtime_error(e.what());
			}
			catch (...)
			{
				connObj->disconnect();
				throw std::runtime_error("UnKnown Err !!");
			}

			return false;
		}

		bool HMongoDataSet::insertBulk(std::vector<Whale::Util::HJson>& objs)
		{
			if (connection_.expired()) { return false; }
			auto connObj = connection_.lock();
			if (!connObj) { return false; }

			try {
				bson_error_t berr;
				std::vector<bson_t*>  bsons;

				for (auto& obj : objs) {
					bson_t* bson = bson_new_from_json((const uint8_t *)obj.encode(obj).c_str(), -1, &berr);
					bsons.push_back(bson);
				}				

				if (!mongoc_collection_insert_bulk(dataset_, MONGOC_INSERT_NONE, (const bson_t**)(bsons.data()), bsons.size(), NULL, &berr)) {
					log_error("Err: %s", berr.message);
					return false;
				}

				for (auto& bson : bsons) {
					bson_destroy(bson);
				}				

				return true;
			}
			catch (std::exception &e)
			{
				connObj->disconnect();
				throw std::runtime_error(e.what());
			}
			catch (...)
			{
				connObj->disconnect();
				throw std::runtime_error("UnKnown Err !!");
			}

			return false;
		}

		bool HMongoDataSet::remove(Whale::Util::HJson& obj)
		{
			if (connection_.expired()) { return false; }
			auto connObj = connection_.lock();
			if (!connObj) { return false; }

			try	{
				bson_error_t berr;
				bson_t      *bson;

				bson = bson_new_from_json((const uint8_t *)obj.encode(obj).c_str(), -1, &berr);

				if (!bson) {
					log_error("%s", berr.message);
					return false;
				}

				if (!mongoc_collection_remove(dataset_, MONGOC_REMOVE_SINGLE_REMOVE, bson, NULL, &berr)) {
					log_error("Err: %s", berr.message);
					return false;
				}

				bson_destroy(bson);
				
				return true;
			}
			catch (std::exception &e) {
				connObj->disconnect();
				throw std::runtime_error(e.what());
			}
			catch (...)	{
				connObj->disconnect();
				throw std::runtime_error("UnKnown Err !!");
			}

			return false;
		}

		bool HMongoDataSet::update(Whale::Util::HJson& obj)
		{
			if (connection_.expired()) { return false; }
			auto connObj = connection_.lock();
			if (!connObj) { return false; }

			try	{
				bson_error_t berr;
				bson_t      *bson;

				bson = bson_new_from_json((const uint8_t *)obj.encode(obj).c_str(), -1, &berr);

				if (!bson) {
					log_error("%s", berr.message);
					return false;
				}

				if (!mongoc_collection_insert(dataset_, MONGOC_INSERT_NONE, bson, NULL, &berr))	{
					log_error("Err: %s", berr.message);
					return false;
				}

				bson_destroy(bson);
				
				return true;
			}
			catch (std::exception &e) {
				connObj->disconnect();
				throw std::runtime_error(e.what());
			}
			catch (...)	{
				connObj->disconnect();
				throw std::runtime_error("UnKnown Err !!");
			}

			return false;
		}

		bool HMongoDataSet::select(Whale::Util::HJson& obj)
		{
			if (connection_.expired()) { 
				return false; 
			}

			auto connection = connection_.lock();

			if (!connection) { 
				return false;
			}

			try	{
				bson_error_t berr;
				bson_t      *bson;

				bson = obj.empty() ? bson_new() : bson_new_from_json((const uint8_t *)obj.encode(obj).c_str(), -1, &berr);

				if (!bson) {
					log_error("%s", berr.message);
					return false;
				}

				cursor_ = mongoc_collection_find(dataset_, MONGOC_QUERY_NONE, 0, 0, 0, bson, NULL, NULL);

				count_ = static_cast<int>(mongoc_collection_count(dataset_, MONGOC_QUERY_NONE, bson, 0, 0, NULL, &berr));

				if (count_ < 0) {
					log_error("Err: %s", berr.message);
				}

				bson_destroy(bson);

				return true;
			}
			catch (std::exception &e) {
				connection->disconnect();
				throw std::runtime_error(e.what());
			}
			catch (...)	{
				connection->disconnect();
				throw std::runtime_error("UnKnown Err !!");
			}

			return false;
		}

		bool HMongoDataSet::execute(Whale::Util::HJson& obj)
		{
			if (connection_.expired()) { return false; } 
			auto connObj = connection_.lock(); 
			if (!connObj) { return false; }

			try	{
				char *str;
				bson_error_t berr;
				bson_t      *bson;
				bson_t      *reply = nullptr;

				bson = bson_new_from_json((const uint8_t *)obj.encode(obj).c_str(), -1, &berr);

				if (!bson) {
					log_error("%s", berr.message);
					return false;
				}
				
				if (!mongoc_collection_command_simple(dataset_, bson, NULL, reply, &berr)) 
				{
					log_error("Err: %s", berr.message);
					return false;					
				}

				str = bson_as_json(reply, NULL);
				log_notice("%s", str);
				bson_free(str);

				bson_destroy(bson);
				bson_destroy(reply);

				return true;
			}
			catch (std::exception &e) {
				connObj->disconnect();
				throw std::runtime_error(e.what());
			}
			catch (...)	{
				connObj->disconnect();
				throw std::runtime_error("UnKnown Err !!");
			}

			return false;
		}
	}
}
