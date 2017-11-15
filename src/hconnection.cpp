#include "whale/data/hconnection.h"
#include "whale/basic/hlog.h"

namespace Whale
{
	namespace Data
	{
		HConnection::HConnection()
		{
			initialized_ = false;
			otl_connect::otl_initialize(); // initialize ODBC environment
		}

		HConnection::~HConnection()
		{	
		}

		void HConnection::action()
		{
			std::lock_guard <std::mutex> lock(mutex_);

			for (auto it : connects_) {
				if (std::get<0>(it.second) && std::get<1>(it.second).elapsed_minutes() > 30) {
					otl_connect* connection = std::get<0>(it.second);
					connection->commit();
					connection->logoff();
					delete connection; connection = NULL;
					connects_.erase(it.first);
				}
			}
		}

		bool HConnection::init(const std::string& connStr)
		{
			connStr_ = connStr;
			initialized_ = true;
			return initialized_;
		}

		void HConnection::stop()
		{
			initialized_ = false;

			std::lock_guard <std::mutex> lock(mutex_);

			for (auto it : connects_) {
				if (std::get<0>(it.second)){
					otl_connect* connection = std::get<0>(it.second);
					connection->commit();
					connection->logoff();
					delete connection; connection = NULL;
				}
			}

			connects_.clear();
		}

		otl_connect *HConnection::connect()
		{
			if (!initialized_) {
				throw std::runtime_error("Environment has not been initialized!!");
			}

			std::lock_guard <std::mutex> lock(mutex_);

			{
				auto it = connects_.find(std::this_thread::get_id());

				if (it != connects_.end() && std::get<0>(it->second)) {
					std::get<1>(it->second).reset();
					return std::get<0>(it->second);
				}
			}

			{
				otl_connect *connection = new otl_connect();
				connection->rlogon(connStr_.c_str(), 0);

				Whale::Basic::HTimer timer;
				connects_[std::this_thread::get_id()] = std::tie(connection, timer);

				log_info("[%s] Current connections: [%d]", connStr_.c_str(), connects_.size());

				return connection;
			}

			return NULL;
		}		

		bool HConnection::disconnect()
		{
			if (!initialized_) {
				throw std::runtime_error("Environment has not been initialized !!");
			}

			std::lock_guard <std::mutex> lock(mutex_);

			auto it = connects_.find(std::this_thread::get_id());

			if (it == connects_.end()) {
				return false;
			}

			if (std::get<0>(it->second)) {
				otl_connect* connection = std::get<0>(it->second);
				connection->commit();
				connection->logoff();
				delete connection; connection = NULL;
			}

			connects_.erase(std::this_thread::get_id());

			return true;
		}
	}
}
