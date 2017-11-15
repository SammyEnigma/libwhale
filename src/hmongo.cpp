#include "whale/data/hmongo.h"
#include "whale/basic/hlog.h"

#ifdef OS_WIN32
#pragma comment(lib, "z:/libwhale/deps/libbson-1.6.2/lib/bson-1.0.lib")
#pragma comment(lib, "z:/libwhale/deps/mongo-c-driver/lib/mongoc-1.0.lib")
#endif

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
		HMongo::HMongo()
		{
			initialized_ = false;
			mongoc_init();
		}

		HMongo::~HMongo()
		{
			mongoc_cleanup();
		}

		void HMongo::action()
		{
			std::lock_guard <std::mutex> lock(mutex_);

			for (auto it : connections_) {
				if (std::get<0>(it.second) && std::get<1>(it.second).elapsed_minutes() > 120) {
					mongoc_client_t* connection = static_cast<mongoc_client_t*>(std::get<0>(it.second));
					mongoc_client_destroy(connection);
					// delete connection; connection = nullptr;
					connections_.erase(it.first);
				}
			}
		}

		bool HMongo::init(const std::string& connStr)
		{
			connStr_ = connStr;
			initialized_ = true;
			return initialized_;
		}

		void HMongo::stop()
		{
			initialized_ = false;

			std::lock_guard <std::mutex> lock(mutex_);

			for (auto it : connections_) {
				mongoc_client_t* connection = static_cast<mongoc_client_t*>(std::get<0>(it.second));
				mongoc_client_destroy(connection); 
				// delete connection; connection = nullptr;
				connections_.erase(it.first);
			}
		}

		void *HMongo::connection()
		{
			if (!initialized_) {
				throw std::runtime_error("Environment has not been initialized!!");
			}

			std::lock_guard<std::mutex> lock(mutex_);

			auto it = connections_.find(std::this_thread::get_id());

			if (it != connections_.end() && std::get<0>(it->second)) {
				std::get<1>(it->second).reset();
				return std::get<0>(it->second);
			}

			mongoc_client_t* connection = mongoc_client_new(connStr_.c_str());

			Whale::Basic::HTimer timer;
			connections_[std::this_thread::get_id()] = std::tie(connection, timer);

			log_info("MongoDB current connections: [%d]", connections_.size());

			return connection;
		}

		bool HMongo::disconnect()
		{
			if (initialized_ == false) {
				throw std::runtime_error("Environment has not been initialized !!");
			}

			std::lock_guard <std::mutex> lock(mutex_);

			auto it = connections_.find(std::this_thread::get_id());

			if (it == connections_.end()) {
				return false;
			}				

			mongoc_client_t* connection = static_cast<mongoc_client_t*>(std::get<0>(it->second));
			mongoc_client_destroy(connection); 
			// delete connection; connection = nullptr;
			connections_.erase(std::this_thread::get_id());

			return true;
		}
	}
}
