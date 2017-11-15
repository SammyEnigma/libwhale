#include "whale/data/horacle.h"
#include "whale/basic/hlog.h"


#include <ocilib.hpp>

#ifdef OS_WIN32
#if defined(OCI_CHARSET_WIDE)
#pragma comment(lib, "z:/libwhale/deps/ocilib-4.2.1/lib/ocilibw.lib")
#elif defined(OCI_CHARSET_ANSI)
#pragma comment(lib, "z:/libwhale/deps/ocilib-4.2.1/lib/ociliba.lib")
#endif
#endif

namespace Whale
{
	namespace Data
	{
		HOracle::HOracle()
		{
			initialized_ = false;
			OCI_Initialize(NULL, NULL, OCI_ENV_DEFAULT | OCI_ENV_THREADED);
			OCI_EnableWarnings(true);
		}

		HOracle::~HOracle()
		{			
			OCI_Cleanup();
		}

		void HOracle::action()
		{
			std::lock_guard <std::mutex> lock(locker_);

			for (auto it : connections_)
			{
				if (std::get<0>(it.second) && std::get<1>(it.second).elapsed_minutes() > 30)
				{
					OCI_Connection* conn = static_cast<OCI_Connection*>(std::get<0>(it.second));
					OCI_ConnectionFree(conn);
					connections_.erase(it.first);
				}
			}
		}

		bool HOracle::init(const std::string& addr, int port, 
			const std::string& name, const std::string& user, 
			const std::string& passwd)
		{
			port_ = port;
			addr_ = addr;
			name_ = name;
			user_ = user;
			passwd_ = passwd;
			initialized_ = true;

			return initialized_;
		}

		void HOracle::stop()
		{
			initialized_ = false;

			std::lock_guard <std::mutex> lock(locker_);

			for (auto session : connections_) {
				if (std::get<0>(session.second)){
					OCI_Connection* conn = static_cast<OCI_Connection*>(std::get<0>(session.second));
					OCI_ConnectionFree(conn);					
				}
			}

			connections_.clear();
		}

		void *HOracle::connection()
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

			if (build()){
				auto it = connections_.find(std::this_thread::get_id());

				if (it != connections_.end() && std::get<0>(it->second))	{
					return std::get<0>(it->second);
				}
			}

			return 0;
		}

		bool HOracle::build()
		{
			if (!initialized_) {
				throw std::runtime_error("Environment has not been initialized!!");
			}

			char conns[260] = { 0 };
			sprintf(conns, "%s:%d/%s", addr_.c_str(), port_, name_.c_str());

			OCI_Connection *connection = OCI_ConnectionCreate(conns, 
				user_.c_str(), passwd_.c_str(), ocilib::Environment::SessionDefault);
			if (!connection) {
				log_error("Connect Oracle Database %s/%s@%s Failure! Err %s", 
					user_.c_str(), passwd_.c_str(), conns, OCI_ErrorGetString(OCI_GetLastError()));
				return false;
			}		

			std::lock_guard <std::mutex> lock(locker_);
			Whale::Basic::HTimer timer;
			connections_[std::this_thread::get_id()] = std::tie(connection, timer);

			log_info("%s", OCI_GetVersionServer(connection));
			log_info("Oracle current connections: [%d]", connections_.size());

			return true;
		}

		bool HOracle::release()
		{
			if (!initialized_) {
				throw std::runtime_error("Environment has not been initialized !!");
			}

			std::lock_guard <std::mutex> lock(locker_);

			auto it = connections_.find(std::this_thread::get_id());

			if (it == connections_.end()) {
				return false;
			}

			if (std::get<0>(it->second))
			{
				OCI_Connection* conn = static_cast<OCI_Connection*>(std::get<0>(it->second));
				OCI_ConnectionFree(conn);
			}

			connections_.erase(std::this_thread::get_id());

			return true;
		}
	}
}
