#include "whale/data/hmssqldataset.h"

#include "whale/util/hstring.h"

namespace Whale
{
	namespace Data
	{
		HMssqlDataSet::HMssqlDataSet(
			const std::weak_ptr<Whale::Data::HMssql> &connection)
		{
			connection_ = connection;
			assert(!connection_.expired());
			auto connectionI = connection_.lock();
			assert(connectionI);
		}

		HMssqlDataSet::~HMssqlDataSet()
		{
#ifdef OS_WIN32
			if (isSelect_ && !dataset_ && (adStateOpen == dataset_->GetState())) {
				dataset_->Close();
			}
#endif
		}

		long HMssqlDataSet::count()
		{
#ifdef OS_WIN32
			if (!dataset_ || adStateOpen != dataset_->GetState()) {
				return 0;
			}

			return dataset_->RecordCount;
#endif
			return 0;
		}

		bool HMssqlDataSet::next()
		{
#ifdef OS_WIN32
			if (!isSelect_)	{
				return false;
			}

			if (dataset_ == 0 || adStateOpen != dataset_->GetState() || dataset_->adoEOF){
				return false;
			}

			if (isFirstMove_){
				isFirstMove_ = false;
			}
			else{
				dataset_->MoveNext();

				if (dataset_ == 0 || dataset_->adoEOF)	{
					return false;
				}
			}

#endif
			return true;
		}

		long HMssqlDataSet::columns()
		{
#ifdef OS_WIN32
			if (dataset_ == 0 || adStateOpen != dataset_->GetState() || dataset_->adoEOF)	{
				return 0;
			}

			return dataset_->GetFields().GetInterfacePtr()->GetCount();
#endif
			return 0;
		}

		std::string HMssqlDataSet::columnName(int index)
		{
#ifdef OS_WIN32
			if (dataset_ == 0 || adStateOpen != dataset_->GetState() || dataset_->adoEOF)	{
				return "";
			}

			if (index >= dataset_->GetFields().GetInterfacePtr()->GetCount() || index < 0) {
				throw std::runtime_error(Whale::Util::HString::format("index %d error!!", index).c_str());
			}

			return  (char*)_bstr_t(dataset_->GetFields()->GetItem((long)(index)).GetInterfacePtr()->GetName());
#endif
			return "";
		}

		std::string HMssqlDataSet::readString(int index)
		{
#ifdef OS_WIN32
			if (dataset_ == 0 || adStateOpen != dataset_->GetState() || dataset_->adoEOF)	{
				return "";
			}

			if (index >= dataset_->GetFields().GetInterfacePtr()->GetCount() || index < 0) {
				throw std::runtime_error(Whale::Util::HString::format("index %d error!!", index).c_str());
			}

			return  (char*)_bstr_t(dataset_->GetFields()->GetItem((long)(index)).GetInterfacePtr()->GetValue());
#endif
			return "";
		}

		std::string HMssqlDataSet::readString(const char *clname)
		{
#ifdef OS_WIN32
			if (0 == clname) {
				throw std::runtime_error("Column name is null !!");
			}

			std::string result = "";

			if (dataset_ == 0 || adStateOpen != dataset_->GetState() || dataset_->adoEOF)	{
				return result;
			}

			_variant_t tValue;

			tValue = dataset_->GetCollect(clname);

			if (tValue.vt != VT_EMPTY && tValue.vt != VT_NULL)	{
				result = (char*)_bstr_t(tValue);
			}
			else{
				result = "";
			}

			return result;
#endif
			return "";
		}

		long HMssqlDataSet::readInt32(int index)
		{
#ifdef OS_WIN32
			if (dataset_ == 0 || adStateOpen != dataset_->GetState() || dataset_->adoEOF){
				return 0;
			}

			if (index >= dataset_->GetFields().GetInterfacePtr()->GetCount() || index < 0) {
				throw std::runtime_error(Whale::Util::HString::format("index %d error!!", index).c_str());
			}

			return  atoi((char*)_bstr_t(dataset_->GetFields()->GetItem((long)(index)).GetInterfacePtr()->GetValue()));
#endif
			return 0;
		}

		long HMssqlDataSet::readInt32(const char *clname)
		{
#ifdef OS_WIN32
			if (0 == clname) {
				throw std::runtime_error("Column name is null !!");
			}

			if (dataset_ == 0 || adStateOpen != dataset_->GetState() || dataset_->adoEOF)	{
				return false;
			}

			std::string result;

			auto tValue = dataset_->GetCollect(clname);
			if (tValue.vt != VT_EMPTY && tValue.vt != VT_NULL)	{
				result = (char*)_bstr_t(tValue);
			}
			else{
				return 0;
			}

			if (result.empty())	{
				return 0;
			}

			return atoi(result.c_str());
#endif
			return 0;
		}

		long long HMssqlDataSet::readInt64(int index)
		{
#ifdef OS_WIN32
			if (dataset_ == 0 || adStateOpen != dataset_->GetState() || dataset_->adoEOF)	{
				return 0;
			}

			if (index >= dataset_->GetFields().GetInterfacePtr()->GetCount() || index < 0)	{
				throw std::runtime_error(Whale::Util::HString::format("index %d error!!", index).c_str());
			}

			return  atol((char*)_bstr_t(dataset_->GetFields()->GetItem((long)(index)).GetInterfacePtr()->GetValue()));
#endif
			return 0;
		}

		long long HMssqlDataSet::readInt64(const char *clname)
		{
#ifdef OS_WIN32
			if (0 == clname) {
				throw std::runtime_error("Column name is null !!");
			}

			if (dataset_ == 0 || adStateOpen != dataset_->GetState() || dataset_->adoEOF)	{
				return false;
			}

			std::string result;

			auto tValue = dataset_->GetCollect(clname);
			if (tValue.vt != VT_EMPTY && tValue.vt != VT_NULL)	{
				result = (char*)_bstr_t(tValue);
			}
			else{
				return 0;
			}

			if (result.empty())	{
				return 0;
			}

			return atol(result.c_str());
#endif
			return 0;
		}

		bool HMssqlDataSet::execute()
		{
#ifdef OS_WIN32
			if (connection_.expired()) {
				throw std::runtime_error("connection_ expired failed !!");
			}

			auto connection = connection_.lock();

			if (!connection) {
				throw std::runtime_error("conn lock object create failed !!");
			}

			try	{
				(*static_cast<_ConnectionPtr*>(connection->connection()))->put_CommandTimeout(30);
				_variant_t affectedRecords = 0;

				dataset_ = (*static_cast<_ConnectionPtr*>(connection->connection()))->Execute(script_.c_str(), &affectedRecords, adCmdText);

				if (!dataset_)	{
					_RecordsetPtr dataset_1 = dataset_;
					_RecordsetPtr dataset_2 = dataset_->NextRecordset(0);

					while (dataset_2){
						dataset_1->Close();
						dataset_1 = dataset_2;
						dataset_2 = dataset_->NextRecordset(0);

					}

					dataset_ = dataset_1;
				}

				return true;
			}
			catch (std::exception &e) {
				connection->release();
				throw std::runtime_error(e.what());
			}
			catch (_com_error& e){
				//if (e.Error() == 0X80004005 || e.Error() == 0X80040e31 || e.Error() == 0x800a0e78) {
				//	connection->release();
				//	connection->build();
				//	return executeAgain();
				//}
				connection->release();
				throw std::runtime_error(Whale::Util::HString::format("ErrCode = %08lx, Meaning = %s, Description = %s, Source = %s \n SQL: %s ",
					e.Error(), e.ErrorMessage(), (char *)e.Description(), (const char*)e.Source(), script_.c_str()).c_str());
			}
			catch (...)	{
				connection->release();
				throw std::runtime_error(Whale::Util::HString::format("Err: %d, SQL : %s ",
					GetLastError(), script_.c_str()).c_str());
			}
#endif

			return false;
		}

		bool HMssqlDataSet::executeProcedure(const std::string& procedure,
			const std::vector<std::tuple<std::string, std::string, int>> &iparams,
			std::vector<std::tuple<std::string, std::string, int>> &oparams)
		{
#ifdef OS_WIN32
			if (connection_.expired()) {
				throw std::runtime_error("connection_ expired failed !!");
			}

			auto connection = connection_.lock();

			if (!connection) {
				throw std::runtime_error("conn lock object create failed !!");
			}

			try {
				if (!connection->release() || !connection->build()) {
					throw std::runtime_error("reconnect database failure!");
				}

				HRESULT hr;
				_CommandPtr command;
					
				if ((FAILED(hr = command.CreateInstance(__uuidof(Command))))) {
					_com_issue_error(hr);
				}
								
				command->ActiveConnection = *static_cast<_ConnectionPtr*>(connection->connection());
				command->CommandType = adCmdStoredProc;
				command->CommandText = _bstr_t(procedure.c_str());

				for (auto param : iparams) {
					command->Parameters->Append(command->CreateParameter(
						_bstr_t(std::get<0>(param).c_str()), adVarChar, adParamInput, std::get<2>(param), std::get<1>(param).c_str()));
				}

				for (auto param : oparams) {
					command->Parameters->Append(command->CreateParameter(
						_bstr_t(std::get<0>(param).c_str()), adVarChar, adParamOutput, std::get<2>(param)));
				}				

				dataset_ = command->Execute(NULL, NULL, adCmdStoredProc);

				for (auto& param : oparams) {
					std::get<1>(param) = (const char*)(_bstr_t)command->Parameters->GetItem(std::get<0>(param).c_str())->GetValue();
				}	
				
				return true;
			}
			catch (std::exception &e) {
				connection->release();
				throw std::runtime_error(e.what());
			}
			catch (_com_error &e) {
				connection->release();
				throw std::runtime_error(Whale::Util::HString::format("ErrCode = %08lx, Meaning = %s, Description = %s, Source = %s. SQL: %s ",
					e.Error(), e.ErrorMessage(), (char *)e.Description(), (const char*)e.Source(), script_.c_str()).c_str());
			}
			catch (...) {
				connection->release();
				throw std::runtime_error(Whale::Util::HString::format("Err: %d, SQL : %s ",
					GetLastError(), script_.c_str()).c_str());
			}
#endif
		}
	}
}
