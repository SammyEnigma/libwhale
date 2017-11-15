#include "whale/data/hpostgresql.h"
#include "whale/basic/hlog.h"

#ifdef OS_WIN32
#ifdef _DEBUG
#pragma comment(lib, "z:/libwhale/deps/libpqxx-4.0.1/lib/libpqxxD.lib")
#else
#pragma comment(lib, "z:/libwhale/deps/libpqxx-4.0.1/lib/libpqxx.lib")
#endif
#endif // OS_WIN32

namespace Whale
{
	namespace Data
	{
		HPostgreSql::HPostgreSql()
		{
			initialized_ = false;
		}

		HPostgreSql::~HPostgreSql()
		{
			
		}

		void HPostgreSql::action()
		{
			std::lock_guard <std::mutex> lock(locker_);

			for (auto it : connections_)
			{
				if (std::get<0>(it.second) && std::get<1>(it.second).elapsed_minutes() > 30)
				{
					pqxx::connection* connection = static_cast<pqxx::connection*>(std::get<0>(it.second));
					connection->disconnect();
					delete connection; connection = nullptr;
					connections_.erase(it.first);
				}
			}
		}

		bool HPostgreSql::init(const std::string& addr, int port, 
			const std::string& name, 
			const std::string& user, const std::string& passwd)
		{
			addr_ = addr;
			port_ = port;
			name_ = name;
			user_ = user;
			passwd_ = passwd;

			initialized_ = true;
			return initialized_;
		}

		void HPostgreSql::stop()
		{
			for (auto it = connections_.begin(); it != connections_.end(); ++it)
			{
				pqxx::connection* conn = static_cast<pqxx::connection*>(std::get<0>(it->second));
				conn->disconnect();
				delete conn; conn = nullptr;
				connections_.erase(it->first);
			}

			initialized_ = false;
		}

		void* HPostgreSql::connection()
		{
			if (!initialized_) {
				throw std::runtime_error("environment has not been initialized!!");
			}

			{
				std::lock_guard <std::mutex> lock(locker_);

				auto it = connections_.find(std::this_thread::get_id());

				if (it != connections_.end() && std::get<0>(it->second))
				{
					std::get<1>(it->second).reset();
					return std::get<0>(it->second);
				}
			}

			if (build())
			{
				auto it = connections_.find(std::this_thread::get_id());

				if (it != connections_.end() && std::get<0>(it->second))
				{
					return std::get<0>(it->second);
				}
			}

			return 0;
		}

		bool HPostgreSql::build()
		{
			if (!initialized_) {
				throw std::runtime_error("environment has not been initialized!!");
			}

			char conns[1024] = { 0 };
			snprintf(conns, 1024, "hostaddr=%s port=%d dbname=%s user=%s passwd=%s",
				addr_.c_str(), port_, name_.c_str(), user_.c_str(), passwd_.c_str());

			pqxx::connection* connection = new pqxx::connection(conns);

			if (!connection->is_open())
			{
				log_error("Open database failure: %s!!", conns);
				return false;
			}

#ifdef OS_WIN32
			connection->set_client_encoding("GBK");
#endif // OS_WIN32


			std::lock_guard <std::mutex> lock(locker_);
			Whale::Basic::HTimer timer;
			connections_[std::this_thread::get_id()] = std::tie(connection, timer);

			log_info("PostgreSQL current connections: [%d]", connections_.size());

			return true;
		}

		bool HPostgreSql::release()
		{
			if (!initialized_) {
				throw std::runtime_error("environment has not been initialized !!");
			}

			std::lock_guard <std::mutex> lock(locker_);

			auto it = connections_.find(std::this_thread::get_id());

			if (it == connections_.end())
			{
				return false;
			}

			pqxx::connection* connection = static_cast<pqxx::connection*>(std::get<0>(it->second));

			if (connection) {
				connection->disconnect();
				delete connection; connection = 0;
			}

			connections_.erase(std::this_thread::get_id());

			return true;
		}
	}
}
