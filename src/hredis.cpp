#include "whale/data/hredis.h"
#include "whale/basic/hlog.h"


#ifdef OS_WIN32
#ifdef _DEBUG
#pragma comment(lib,"z:/libwhale/deps/hiredis-3.0.0/lib/Debug/hiredis.lib")
#pragma comment(lib,"z:/libwhale/deps/hiredis-3.0.0/lib/Debug/Win32_Interop.lib")
#else
#pragma comment(lib,"z:/libwhale/deps/hiredis-3.0.0/lib/Release/hiredis.lib")
#pragma comment(lib,"z:/libwhale/deps/hiredis-3.0.0/lib/Release/Win32_Interop.lib")
#endif // _DEBUG

#endif // OS_WIN32

namespace Whale
{
	namespace Data
	{
		HRedis::HRedis()
		{
#ifdef OS_WIN32
			WSADATA  wsaData = { 0 };
			if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
				std::cout << "Init Windows Socket Failed !! Err: " << GetLastError() << std::endl;
			}
#endif
			initialized_ = false;
		}

		HRedis::~HRedis()
		{
		}

		void HRedis::action()
		{
			std::lock_guard <std::mutex> lock(locker_);

			for (auto it : connections_)
			{
				if (std::get<0>(it.second) && std::get<1>(it.second).elapsed_minutes() > 30)
				{
					redisFree(static_cast<redisContext*>(std::get<0>(it.second)));
					connections_.erase(it.first);
				}
			}
		}

		bool HRedis::init(const std::string& addr, 
			int port, const std::string& passwd,
			const std::string& encode)
		{
			addr_ = addr;
			port_ = port;
			passwd_ = passwd;
			encode_ = encode;

			initialized_ = true;
			return initialized_;
		}

		void HRedis::stop()
		{
			for (auto it = connections_.begin(); it != connections_.end(); ++it)
			{
				redisFree(static_cast<redisContext*>(std::get<0>(it->second)));
			}

			initialized_ = false;
		}

		void* HRedis::connection()
		{
			if (!initialized_) {				
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

		bool HRedis::build()
		{
			if (!initialized_) {				
				throw std::runtime_error("Environment has not been initialized!!");
			}

			struct timeval timeout = { 1, 500000 }; // 1.5 seconds
			redisContext* connection = redisConnectWithTimeout(addr_.c_str(), port_, timeout);

			if (connection == NULL)
			{
				log_error("Connection error: can't allocate redis context.");
				return false;
			}

			if (connection->err)
			{
				log_error("Connection error: %s", connection->errstr);
				redisFree(connection);
				return false;
			}

			if (!passwd_.empty()) {
				auto reply = (redisReply*)redisCommand(connection, "AUTH %s", passwd_.c_str());

				if (!reply) {
					log_error("Authentication failure, PASSWD: %s", passwd_.c_str());
					redisFree(connection);
					return false;
				}

				if (reply->type == REDIS_REPLY_ERROR) {
					log_error("Authentication failure, PASSWD: %s", passwd_.c_str());
					freeReplyObject(reply);
					redisFree(connection);
					return false;
				}

				freeReplyObject(reply);
			}

			std::lock_guard <std::mutex> lock(locker_);
			Whale::Basic::HTimer timer;
			connections_[std::this_thread::get_id()] = std::tie(connection, timer);

			log_info("Redis current connections: [%d]", connections_.size());

			return true;
		}

		bool HRedis::release()
		{
			if (!initialized_) {
				throw std::runtime_error("Environment has not been initialized !!");
			}

			std::lock_guard <std::mutex> lock(locker_);

			auto it = connections_.find(std::this_thread::get_id());

			if (it == connections_.end())
			{
				return false;
			}

			redisContext* connection = static_cast<redisContext*>(std::get<0>(it->second));

			if (connection != 0)
			{
				redisFree(connection);
			}

			connections_.erase(std::this_thread::get_id());

			connection = nullptr;

			return true;
		}
	}
}
