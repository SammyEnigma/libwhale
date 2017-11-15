#include "whale/data/horacledataset.h"


namespace Whale
{
	namespace Data
	{
		HOracleDataSet::HOracleDataSet(const std::weak_ptr<Whale::Data::HOracle> &connection)
		{
			connection_ = connection;
			assert(!connection_.expired());
			auto connectionI = connection_.lock();
			assert(connectionI);

			OCI_Connection* conn = static_cast<OCI_Connection*>(connectionI->connection());
			statement_ = OCI_CreateStatement(conn);
		}

		HOracleDataSet::~HOracleDataSet()
		{
			release();
		}

		void HOracleDataSet::release()
		{
			if (dataset_)
			{
				OCI_ReleaseResultsets(statement_);
				dataset_ = 0;
			}

			if (statement_)
			{
				OCI_FreeStatement(statement_);
				statement_ = 0;
			}
		}

		long HOracleDataSet::count()
		{
			if (!dataset_) { return 0; }
			return OCI_GetFetchSize(statement_);
		}

		bool HOracleDataSet::next()
		{
			if (!dataset_) { return false; }
			return OCI_FetchNext(dataset_) == 1;
		}

		long HOracleDataSet::columns()
		{
			if (!dataset_) { return 0; }

			return OCI_GetColumnCount(dataset_);
		}

		std::string HOracleDataSet::columnName(int index)
		{
			if (!dataset_) { return ""; }

			return OCI_GetColumnName(OCI_GetColumn(dataset_, index));
		}

		std::string HOracleDataSet::readString(int index)
		{
			if (!dataset_) { return ""; }

			return OCI_GetString(dataset_, index);
		}

		std::string HOracleDataSet::readString(const char *clname)
		{
			if (!clname || !dataset_) { return ""; }

			return OCI_GetString2(dataset_, clname) == 0 ? "" : OCI_GetString2(dataset_, clname);
		}

		long HOracleDataSet::readInt32(int index)
		{
			if (!dataset_) { return 0; }

			return OCI_GetInt(dataset_, index);
		}

		long HOracleDataSet::readInt32(const char *clname)
		{
			if (!clname || !dataset_) { return 0; }

			return OCI_GetInt2(dataset_, clname);
		}

		long long HOracleDataSet::readInt64(int index)
		{
			if (!dataset_) { return 0; }

			return OCI_GetInt(dataset_, index);
		}

		long long HOracleDataSet::readInt64(const char *clname)
		{
			if (!clname || !dataset_) { return 0; }

			return OCI_GetInt2(dataset_, clname);
		}

		bool HOracleDataSet::execute()
		{
			if (connection_.expired()) {
				return false;
			}
			auto connection = connection_.lock();

			if (!connection) {
				return false;
			}

			if (!OCI_ExecuteStmt(statement_, script_.c_str())) {
				log_error("DML errors: %d, SQL: %s",
					OCI_GetBatchErrorCount(statement_), script_.c_str());
				OCI_Error *err = OCI_GetBatchError(statement_);
				while (err) {
					log_error("Error at row %d : %s", OCI_ErrorGetRow(err), OCI_ErrorGetString(err));
					err = OCI_GetBatchError(statement_);
				}
			}

			//log_debug("Affected rows: %d", OCI_GetAffectedRows(statement_));

			if (!OCI_Commit(static_cast<OCI_Connection *>(connection->connection()))) {
				release();
				connection->release();
				throw std::runtime_error("Connection cut down !!");
			}

			return true;
		}

		bool HOracleDataSet::executeStmt()
		{
			if (connection_.expired()) { 
				return false; 
			} 

			auto connection = connection_.lock(); 

			if (!connection) { 
				return false; 
			}
			
			if (!OCI_ExecuteStmt(statement_, script_.c_str())) {
				log_error("DML errors: %d, SQL: %s",
					OCI_GetBatchErrorCount(statement_), script_.c_str());
				OCI_Error *err = OCI_GetBatchError(statement_);
				while (err) {
					log_error("Error at row %d : %s", OCI_ErrorGetRow(err), OCI_ErrorGetString(err));
					err = OCI_GetBatchError(statement_);
				}
			}

			if (statement_ && isSelect_) {
				dataset_ = OCI_GetResultset(statement_);
			}

			//log_debug("Affected rows: %d", OCI_GetAffectedRows(statement_));

			if (!OCI_Commit(static_cast<OCI_Connection *>(connection->connection()))) {
				release();
				connection->release();
				throw std::runtime_error("Connection cut down !!");
			}

			return true;
		}

		bool HOracleDataSet::bind(const char *outKey, char *outValue, int maxmium)
		{
			return OCI_BindString(statement_, outKey, outValue, maxmium) == 1;
		}
	}
}
