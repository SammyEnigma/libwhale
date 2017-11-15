#include "whale/data/hpostgresqldataset.h"
#include "whale/basic/hlog.h"

namespace Whale
{
	namespace Data
	{
		HPostgreSqlDataSet::HPostgreSqlDataSet(const std::weak_ptr<Whale::Data::HPostgreSql> &connection)
		{
			connection_ = connection;
			assert(!connection_.expired());
			auto connectionI = connection_.lock();
			assert(connectionI);
		}

		HPostgreSqlDataSet::~HPostgreSqlDataSet()
		{
			index_ = -1;
			dataset_.clear();
		}

		long HPostgreSqlDataSet::count()
		{
			return dataset_.size();
		}

		bool HPostgreSqlDataSet::next()
		{
			if (!isSelect_)
			{
				return false;
			}

			if (++index_ >= static_cast<int>(dataset_.size()))
			{
				return false;
			}

			return true;
		}

		long HPostgreSqlDataSet::columns()
		{
			try
			{
				return dataset_.columns();
			}
			catch (std::exception &e)
			{
				throw std::runtime_error(e.what());
			}
			catch (...)
			{
				throw std::runtime_error("Unknown Exception !!");
			}
		}

		std::string HPostgreSqlDataSet::columnName(int index)
		{
			try
			{
				return dataset_.column_name(index);
			}
			catch (std::exception &e)
			{
				throw std::runtime_error(e.what());
			}
			catch (...)
			{
				throw std::runtime_error("Unknown Exception !!");
			}
		}

		std::string HPostgreSqlDataSet::readString(int index)
		{
			try
			{
				return dataset_[index_][index].as<std::string>();
			}
			catch (std::exception &e)
			{
				throw std::runtime_error(e.what());
			}
			catch (...)
			{
				throw std::runtime_error("Unknown Exception !!");
			}
		}

		std::string HPostgreSqlDataSet::readString(const char *clname)
		{
			try
			{
				return dataset_[index_][dataset_.column_number(clname)].as<std::string>();
			}
			catch (std::exception &e)
			{
				throw std::runtime_error(e.what());
			}
			catch (...)
			{				
				throw std::runtime_error("Unknown Exception !!");
			}
		}

		long HPostgreSqlDataSet::readInt32(int index)
		{
			try
			{
				return dataset_[index_][index].as<long>();
			}
			catch (std::exception &e)
			{
				log_error("Exception %s", e.what());
				throw std::runtime_error(e.what());
			}
			catch (...)
			{
				
				throw std::runtime_error("Unknown Exception !!");
			}
		}

		long HPostgreSqlDataSet::readInt32(const char *clname)
		{
			try
			{
				return dataset_[index_][dataset_.column_number(clname)].as<long>();
			}
			catch (std::exception &e)
			{
				log_error("Exception %s", e.what());
				throw std::runtime_error(e.what());
			}
			catch (...)
			{
				
				throw std::runtime_error("Unknown Exception !!");
			}
		}

		long long HPostgreSqlDataSet::readInt64(int index)
		{
			try
			{
				return dataset_[index_][index].as<long long>();
			}
			catch (std::exception &e)
			{
				log_error("Exception %s", e.what());
				throw std::runtime_error(e.what());
			}
			catch (...)
			{
				
				throw std::runtime_error("Unknown Exception !!");
			}
		}

		long long HPostgreSqlDataSet::readInt64(const char *clname)
		{
			try
			{
				return dataset_[index_][dataset_.column_number(clname)].as<long long>();
			}
			catch (std::exception &e)
			{
				log_error("Exception %s", e.what());
				throw std::runtime_error(e.what());
			}
			catch (...)
			{
				
				throw std::runtime_error("Unknown Exception !!");
			}
		}

		bool HPostgreSqlDataSet::execute()
		{
			if (connection_.expired())
			{
				throw std::runtime_error("_database_interface expired failed !!");
			}

			auto connObj = connection_.lock();

			if (!connObj)
			{
				throw std::runtime_error("_database_interface lock object create failed !!");
			}

			try
			{
				dataset_.clear();
				pqxx::work w(*(pqxx::connection*)(connObj->connection()));
				dataset_ = w.exec(script_.c_str());
				w.commit();

				return true;
			}
			catch (std::exception &e)
			{
				connObj->release();
				throw std::runtime_error(Whale::Util::HString::format("Exception [%s] : %s", 
					script_.c_str(), e.what()).c_str());
			}
			catch (...)
			{
				connObj->release();
				throw std::runtime_error(Whale::Util::HString::format("Unknown Exception. SQL : %s ", 
					script_.c_str()).c_str());
			}

			return false;
		}
	}
}