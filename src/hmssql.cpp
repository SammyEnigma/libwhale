#include "whale/data/hmssql.h"
#include "whale/basic/hlog.h"

#include "whale/util/hstring.h"

//#ifdef OS_LINUX
//#include <sybfront.h>
//#include <sybdb.h>
//#endif // OS_LINUX

namespace Whale
{
	namespace Data
	{
		HMssql::HMssql()
		{
			initialized_ = false;
		}

		HMssql::~HMssql()
		{
			close();
		}

		bool HMssql::init(const std::string& addr, int port, 
			const std::string& name,
			const std::string& user, const std::string& passwd)
		{
#ifdef OS_WIN32
			HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
			if (hr != S_OK  && hr != S_FALSE && hr != RPC_E_CHANGED_MODE) {
				throw std::runtime_error("CoInitializeEx Err !!");
			}
#endif
			port_ = port;
			addr_ = addr;
			name_ = name;
			user_ = user;
			passwd_ = passwd;
			initialized_ = true;

//#ifdef OS_LINUX
//			initialized_ = (dbinit() != FAIL);
//#endif // OS_LINUX


			return initialized_;
		}

		void HMssql::close()
		{
			std::lock_guard <std::mutex> lock(_locker);

#ifdef OS_WIN32			

			for (auto it : connections_) {
				if ((*it.second)->State != adStateClosed) {
					(*it.second)->Close();
				}

				_ConnectionPtr* connection = it.second;
				delete connection;
				connection = 0;
			}

			connections_.clear();

			CoUninitialize();
#endif

//#ifdef OS_LINUX
//			for (auto it : connections_) {
//				dbclose(it->second);
//			}
//			connections_.clear();			
//#endif // OS_LINUX

			initialized_ = false;
		}

		void* HMssql::connection()
		{
#ifdef OS_WIN32
			if (!initialized_) {
				throw std::runtime_error("Environment has not been initialized!!");
			}

			{
				std::lock_guard <std::mutex> lock(_locker);

				auto it = connections_.find(std::this_thread::get_id());

				if (it != connections_.end() && it->second)	{
					return it->second;
				}
			}

			if (build())	{
				auto it = connections_.find(std::this_thread::get_id());

				if (it != connections_.end() && it->second)	{
					return it->second;
				}
			}
#endif

			return 0;
		}

		bool HMssql::build()
		{
			if (!initialized_) {
				throw std::runtime_error("Environment has not been initialized!!");
			}

			std::lock_guard <std::mutex> lock(_locker);

#ifdef OS_WIN32

			auto it = connections_.find(std::this_thread::get_id());
			if (it != connections_.end()) {
				return true;
			}

			try {
				_ConnectionPtr* conn = new _ConnectionPtr();

				if (FAILED(conn->CreateInstance("ADODB.Connection"))) {
					throw std::runtime_error("Create instance ADODB.Connection failure !!");
				}

				(*conn)->PutConnectionTimeout(15);

				char connstring[512] = { 0 };
				snprintf(connstring, 512, "Provider=SQLOLEDB.1;Password=%s;Persist Security Info=FALSE;User ID=%s;Initial Catalog=%s;Data Source=%s,%d", 
					passwd_.c_str(), user_.c_str(), name_.c_str(), addr_.c_str(), port_);

				if (FAILED((*conn)->Open((_bstr_t)connstring, "", "", adConnectUnspecified)) 
					|| FAILED((*conn)->put_CursorLocation(adUseClient))) {
					throw std::runtime_error(Whale::Util::HString::format("Open Err: %s", connstring).c_str());
				}

				connections_[std::this_thread::get_id()] = conn;

				log_info("MSSQL current connections: [%d]", connections_.size());
			}
			catch (_com_error& e) {
				throw std::runtime_error(Whale::Util::HString::format("ErrCode = %08lx, Meaning = %s, Description = %s",
					e.Error(), e.ErrorMessage(), (const char*)e.Description()).c_str());
			}
#endif

//#ifdef OS_LINUX
//			LOGINREC *login = dblogin();
//			DBSETLUSER(login, user_.c_str());
//			DBSETLPWD(login, passwd_.c_str());
//			std::string server = Whale::Util::HString::format("%s:%d", addr_.c_str(), port_);
//			DBPROCESS *connection = dbopen(login, server.c_str());
//			if (connection == FAIL) {
//				throw std::runtime_error("Function dbopen failure !!");
//			}
//
//			if (dbuse(connection, name_.c_str()) == FAIL) {
//				throw std::runtime_error("Function dbuse failure !!");
//			}
//
//			connections_[std::this_thread::get_id()] = connection;
//
//			log_info("MSSQL current connections: [%d]", connections_.size());
//			
//#endif // OS_LINUX
			return true;
		}

		bool HMssql::release()
		{
			if (initialized_ == false) {
				throw std::runtime_error("Environment has not been initialized !!");
			}

#ifdef OS_WIN32		

			std::lock_guard <std::mutex> lock(_locker);

			auto it = connections_.find(std::this_thread::get_id());

			if (it != connections_.end()) {
				if ((*it->second)->State != adStateClosed) {
					(*it->second)->Close();
				}

				delete it->second;
				it->second = 0;
				connections_.erase(std::this_thread::get_id());
			}

#endif

//#ifdef OS_LINUX		
//
//			std::lock_guard <std::mutex> lock(_locker);
//
//			auto it = connections_.find(std::this_thread::get_id());
//
//			if (it != connections_.end()) {
//				dbclose(it->second);
//				connections_.erase(std::this_thread::get_id());
//			}
//
//#endif

			return true;
		}
	}
}
