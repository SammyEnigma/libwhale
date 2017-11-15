#include "whale/data/hmysql.h"
#include "whale/basic/hlog.h"


#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/driver.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <cppconn/metadata.h>
#include <cppconn/exception.h>

#ifdef OS_WIN32
#pragma comment(lib, "z:/libwhale/deps/mysqlcppconn-1.1.7/lib/mysqlcppconn.lib")
#endif

namespace Whale
{
	namespace Data
	{
		HMysql::HMysql()
		{
			initialized_ = false;
		}

		HMysql::~HMysql()
		{
			stop();
		}

		void HMysql::action()
		{
			std::lock_guard <std::mutex> lock(locker_);

			for (auto it : connections_){
				if (std::get<0>(it.second) && std::get<1>(it.second).elapsed_minutes() > 30){
					sql::Connection* connection = static_cast<sql::Connection*>(std::get<0>(it.second));
					delete connection; connection = nullptr;
					connections_.erase(it.first);
				}
			}
		}

		bool HMysql::init(const std::string& addr, int port, 
			const std::string& name, 
			const std::string& user, const std::string& passwd)
		{
			port_ = port;
			addr_ = addr;
			name_ = name;
			user_ = user;
			passwd_ = passwd;
			initialized_ = true;
			return initialized_;
		}

		void HMysql::stop()
		{
			initialized_ = false;

			std::lock_guard <std::mutex> lock(locker_);

			for (auto it : connections_) {
				sql::Connection* conn = static_cast<sql::Connection*>(std::get<0>(it.second));
				delete conn; conn = nullptr;				
			}

			connections_.clear();
		}

		void *HMysql::connection()
		{
			if (!initialized_)	{
				throw std::runtime_error("Environment has not been initialized!!");
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

			if (build()) {
				auto it = connections_.find(std::this_thread::get_id());

				if (it != connections_.end() && std::get<0>(it->second)){
					return std::get<0>(it->second);
				}
			}

			return 0;
		}

		bool HMysql::build()
		{
			if (!initialized_) {
				throw std::runtime_error("Environment has not been initialized!!");
			}

			sql::Connection* connection = nullptr;
			sql::Statement* statement = nullptr;

			try	{
				auto driver = sql::mysql::get_mysql_driver_instance();
	
				connection = driver->connect(
					Whale::Util::HString::format("tcp://%s:%d", addr_.c_str(), port_).c_str(),
					user_.c_str(), passwd_.c_str());

				if (!connection || connection->isClosed()) {
					return false;
				}

				connection->setAutoCommit(0);

				sql::DatabaseMetaData *meta = connection->getMetaData();  
  
   				log_info("MySQL product name: %s, Version: %s", 
					meta->getDatabaseProductName().c_str(),
					meta->getDatabaseProductVersion().c_str());  
  
   				log_info("MySQL driver name: %s, Version: %s",
					meta->getDriverName().c_str(),
					meta->getDriverVersion().c_str());  
  
    				log_info("MySQL maximum connections: %d, Row size: %lu", 
					meta->getMaxConnections(),
					meta->getMaxRowSize());  
  
				statement = connection->createStatement();

#ifdef OS_WIN32
				statement->execute("SET character_set_client=gbk");
				statement->execute("SET character_set_connection=gbk");
				statement->execute("SET character_set_results=gbk");
#else
				statement->execute("SET character_set_client=utf8");
				statement->execute("SET character_set_connection=utf8");
				statement->execute("SET character_set_results=utf8");
#endif // OS_WIN32			

				char sql[1024] = { 0 };
				sprintf(sql, "use %s", name_.c_str());
				statement->execute(sql);				

				std::lock_guard <std::mutex> lock(locker_);
				Whale::Basic::HTimer timer;
				connections_[std::this_thread::get_id()] = std::tie(connection, timer);

				log_info("MySQL current connections: [%d]", connections_.size());
			}
			catch (sql::SQLException &e){
				if (connection) 
					delete connection;
				if (statement) 
					delete statement;
				throw std::runtime_error(Whale::Util::HString::format("ErrCode = %d, Description = %s, SQLState = %s",
					e.getErrorCode(), e.what(), e.getSQLStateCStr()).c_str());
			}

			if (statement)
				delete statement;

			return true;
		}

		bool HMysql::release()
		{
			if (!initialized_)	{
				throw std::runtime_error("Environment has not been initialized !!");
			}

			std::lock_guard <std::mutex> lock(locker_);

			auto it = connections_.find(std::this_thread::get_id());

			if (it == connections_.end()) {
				return false;
			}

			sql::Connection* connection = static_cast<sql::Connection*>(std::get<0>(it->second));
			delete connection; connection = nullptr;
			connections_.erase(std::this_thread::get_id());

			return true;
		}
	}
}
