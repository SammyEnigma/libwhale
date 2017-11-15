#include "whale/data/hmysqldataset.h"
#include "whale/basic/hlog.h"


#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>

namespace Whale
{
	namespace Data
	{
		HMysqlDataSet::HMysqlDataSet(
			const std::weak_ptr<Whale::Data::HMysql> &connection)
		{
			connection_ = connection;
			assert(!connection_.expired());
			auto connectionI = connection_.lock();
			assert(connectionI);
			conn_ = static_cast<sql::Connection*>(connectionI->connection());
		}

		HMysqlDataSet::~HMysqlDataSet()
		{
			if (dataset_) {				
				delete dataset_;
				dataset_ = 0;
			}
		}

		long HMysqlDataSet::count()
		{
			return dataset_->rowsCount();
		}

		bool HMysqlDataSet::next()
		{
			if (!isSelect_) {
				return false;
			}

			if (dataset_ == 0){
				return false;
			}

			return dataset_->next();
		}

		long HMysqlDataSet::columns()
		{
			return 0;
		}

		std::string HMysqlDataSet::columnName(int index)
		{
			return "";
		}

		std::string HMysqlDataSet::readString(int index)
		{
			return dataset_->getString(index).c_str();
		}

		std::string HMysqlDataSet::readString(const char *clname)
		{
			if (!clname) {
				throw std::runtime_error("Column name is null !!");
			}

			return dataset_->getString(clname).c_str();
		}

		long HMysqlDataSet::readInt32(int index)
		{
			return dataset_->getInt(index);
		}

		long HMysqlDataSet::readInt32(const char *clname)
		{
			if (!clname) {
				throw std::runtime_error("Column name is null !!");
			}

			return dataset_->getInt(clname);
		}

		long long HMysqlDataSet::readInt64(int index)
		{
			return dataset_->getInt64(index);
		}

		long long HMysqlDataSet::readInt64(const char *clname)
		{
			if (!clname) {
				throw std::runtime_error("Column name is null !!");
			}

			return dataset_->getInt64(clname);
		}

		bool HMysqlDataSet::execute()
		{
			if (connection_.expired()) { 
				return false; 
			} 

			auto connection = connection_.lock(); 

			if (!connection) { 
				return false; 
			}

			try	{
				if (dataset_) {
					delete dataset_;
					dataset_ = 0;
				}

				sql::Statement* statement = conn_->createStatement();

				if (!statement) {
					throw std::runtime_error("mysql create statement failure!!");
				}

				if (statement->execute(script_.c_str())) {					
					do {
						if (dataset_) {
							std::cout << "Clear Dataset !!" << std::endl;
							delete dataset_; dataset_ = 0;
						}
						dataset_ = statement->getResultSet();						
					} while (statement->getMoreResults());
				}				

				if (statement) {
					delete statement;
					statement = 0;
				}

				conn_->commit();

				return true;
			}
			catch (sql::SQLException &e) {
				connection->release();
				throw std::runtime_error(Whale::Util::HString::format("ErrCode = %d, Description = %s, SQLState = %s, SQLString:%s",
					e.getErrorCode(), e.what(), e.getSQLStateCStr(), script_.c_str()).c_str());
			}
			catch (...)	{
				connection->release();
				throw std::runtime_error(Whale::Util::HString::format("Unknown Exception. SQL : %s ",
					script_.c_str()).c_str());
			}

			return false;
		}
	}
}
