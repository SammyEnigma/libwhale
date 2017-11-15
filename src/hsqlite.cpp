#include "whale/data/hsqlite.h"
#include "whale/basic/hlog.h"


#include <sqlite3.h>

#ifdef OS_WIN32
#pragma comment(lib,"z:/libwhale/deps/sqlite-3.11.1/lib/x86/sqlite3.lib")
#endif

namespace Whale 
{
	namespace Data
	{
		HSqlite::HSqlite(void)
		{
		}

		HSqlite::~HSqlite(void)
		{
		}

		void HSqlite::action()
		{

		}

		void *HSqlite::connection()
		{
			return connection_;
		}

		bool HSqlite::init(
			const std::string& filePath)
		{
			if (filePath.empty()) {
				log_error("Param is NULL !!");
				return false;
			}

			filePath_ = filePath;

			int err = 0;
#ifdef  UNICODE
			err = sqlite3_open16(filePath.c_str(), &connection_);
#else
			err = sqlite3_open(filePath.c_str(), &connection_);
#endif
			if (err != SQLITE_OK) {
				log_error("sqlite3_open '%s': %s", filePath.c_str(), sqlite3_errstr(err));
			}				

			return (err == SQLITE_OK);
		}

		void HSqlite::stop()
		{
			sqlite3_close(connection_);
			connection_ = nullptr;
		}		
	}
}
