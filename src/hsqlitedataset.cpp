#include "whale/data/hsqlitedataset.h"

#include "whale/basic/hlog.h"

namespace Whale
{
	namespace Data
	{
		HSqliteDataSet::HSqliteDataSet(
			const std::weak_ptr<Whale::Data::HSqlite>& connection)
		{
			connection_ = connection;
			assert(!connection_.expired());
			auto connectionI = connection_.lock();
			assert(connectionI);			
		}

		HSqliteDataSet::~HSqliteDataSet()
		{
			close();
		}

		bool HSqliteDataSet::next()
		{			
			std::lock_guard <std::mutex> lock(locker_);
			assert((sqlite3stmt_ != 0 && sqlite3stmt_ != nullptr));
			return sqlite3_step(sqlite3stmt_) == SQLITE_ROW;
		}

		void HSqliteDataSet::close()
		{
			std::lock_guard <std::mutex> lock(locker_);
			if (sqlite3stmt_ != 0 && sqlite3stmt_ != nullptr && isSelect_) { sqlite3_finalize(sqlite3stmt_); }			
		}

		int HSqliteDataSet::columns(void)
		{
			std::lock_guard <std::mutex> lock(locker_);
			assert((sqlite3stmt_ != 0 && sqlite3stmt_ != nullptr));
			return sqlite3_column_count(sqlite3stmt_);
		}

		std::string HSqliteDataSet::columnName(int index)
		{
			std::lock_guard <std::mutex> lock(locker_);
			assert((sqlite3stmt_ != 0 && sqlite3stmt_ != nullptr));
			return sqlite3_column_name(sqlite3stmt_, index);
		}

		std::string HSqliteDataSet::readString(int index)
		{
			std::lock_guard <std::mutex> lock(locker_);
			assert((sqlite3stmt_ != 0 && sqlite3stmt_ != nullptr));
			const unsigned char* value = sqlite3_column_text(sqlite3stmt_, index);
			return value ? (const char*)value : "";
		}

		int HSqliteDataSet::readInt32(int index)
		{
			std::lock_guard <std::mutex> lock(locker_);
			assert((sqlite3stmt_ != 0 && sqlite3stmt_ != nullptr));
			return sqlite3_column_int(sqlite3stmt_, index);
		}

		long long HSqliteDataSet::readInt64(int index)
		{
			std::lock_guard <std::mutex> lock(locker_);
			assert((sqlite3stmt_ != 0 && sqlite3stmt_ != nullptr));
			return sqlite3_column_int64(sqlite3stmt_, index);
		}

		double HSqliteDataSet::readDouble(int index)
		{
			std::lock_guard <std::mutex> lock(locker_);
			assert((sqlite3stmt_ != 0 && sqlite3stmt_ != nullptr));
			return sqlite3_column_double(sqlite3stmt_, index);
		}

		bool HSqliteDataSet::bind(int index, const std::string& value)
		{
			std::lock_guard <std::mutex> lock(locker_);
			assert((sqlite3stmt_ != 0 && sqlite3stmt_ != nullptr));
			return sqlite3_bind_text(sqlite3stmt_, index, value.c_str(), -1, SQLITE_TRANSIENT) == SQLITE_OK;
		}

		bool HSqliteDataSet::bind(int index, int value)
		{
			std::lock_guard <std::mutex> lock(locker_);
			assert((sqlite3stmt_ != 0 && sqlite3stmt_ != nullptr));
			return sqlite3_bind_int(sqlite3stmt_, index, value) == SQLITE_OK;
		}

		bool HSqliteDataSet::bind(int index, double value)
		{
			std::lock_guard <std::mutex> lock(locker_);
			assert((sqlite3stmt_ != 0 && sqlite3stmt_ != nullptr));
			return sqlite3_bind_double(sqlite3stmt_, index, value) == SQLITE_OK;
		}

		bool HSqliteDataSet::excute()
		{
			std::lock_guard <std::mutex> lock(locker_);
			sqlite3_finalize(sqlite3stmt_);
			return sqlite3_reset(sqlite3stmt_) == SQLITE_OK;
		}

		void HSqliteDataSet::clear()
		{
			std::lock_guard <std::mutex> lock(locker_);
			if (sqlite3stmt_) { sqlite3_finalize(sqlite3stmt_); }
		}

		bool HSqliteDataSet::begin()
		{
			if (connection_.expired()) { return false; }
			auto sqliteObject = connection_.lock();
			if (!sqliteObject) { return false; }
			
			char * errmsg = "";
			if (sqlite3_exec(sqliteObject->connection_, "BEGIN TRANSACTION;", 0, 0, &errmsg) != SQLITE_OK)
			{
				log_error("Err: %s!!", errmsg);
				return false;
			}

			return true;
		}

		bool HSqliteDataSet::commit()
		{
			if (connection_.expired()) { return false; }
			auto sqliteObject = connection_.lock();
			if (!sqliteObject) { return false; }

			char * errmsg = "";

			if (sqlite3_exec(sqliteObject->connection_, "COMMIT TRANSACTION;;", 0, 0, &errmsg) != SQLITE_OK)
			{
				log_error("Err: %s!!", errmsg);
				return false;
			}
			return true;
		}

		bool HSqliteDataSet::rollback()
		{
			if (connection_.expired()) { return false; }
			auto sqliteObject = connection_.lock();
			if (!sqliteObject) { return false; }

			char* errmsg = "";

			if (sqlite3_exec(sqliteObject->connection_, "ROLLBACK TRANSACTION;", 0, 0, &errmsg) != SQLITE_OK)
			{
				log_error("rollback failure %s!!", errmsg);
				return false;
			}

			return true;
		}
	}
}
