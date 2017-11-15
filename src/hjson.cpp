
#include "whale/util/hjson.h"
#include "whale/util/hstring.h"

namespace Whale
{
	namespace Util
	{
		HJson::HJson(void) : document_(new rapidjson::Document())
		{
		}

		HJson::~HJson(void)
		{
			if (document_) {
				delete document_;
				document_ = 0;
			}
		}

		HJson::HJson(const HJson& json)
		{		
			try	{
				document_ = new rapidjson::Document();
				document_->Parse(const_cast<HJson&>(json).encode(const_cast<HJson&>(json)).c_str()).HasParseError();
			}
			catch (...) {
			}
		}


		HJson& HJson::operator=(const HJson& json)
		{
			if (this == &json) {
				return *this;
			}			

			try	{
				document_ = new rapidjson::Document();
				document_->Parse(const_cast<HJson&>(json).encode(const_cast<HJson&>(json)).c_str()).HasParseError();
			}
			catch (...) {
			}

			return *this;
		}

		bool HJson::empty()
		{
			if (document_->IsObject())
				return document_->ObjectEmpty();

			if (document_->IsArray())
				return document_->Empty();

			return true;
		}

		std::string HJson::encodePretty(rapidjson::Value& obj) {
			rapidjson::StringBuffer sb;
			rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
			obj.Accept(writer);
			return std::string(sb.GetString(), sb.GetSize());
		}

		std::string HJson::encode(rapidjson::Value& obj) {
			rapidjson::StringBuffer sb;
			rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
			obj.Accept(writer);
			return std::string(sb.GetString(), sb.GetSize());
		}

		bool HJson::decode(const std::string& jsonStr)
		{
			if (jsonStr.empty()) return false;

			try	{				
				return  !document_->Parse(jsonStr.c_str()).HasParseError();
			} 
			catch (...) {
			}

			return false;
		}

		bool HJson::saveToFile(const std::string& jsonFile)
		{
			if (jsonFile.empty()) return false;			

			try	{
				rapidjson::StringBuffer sb;
				rapidjson::PrettyWriter<rapidjson::StringBuffer> pw(sb);
				document_->Accept(pw);

				std::ofstream out(jsonFile.c_str());
				out << sb.GetString();
				return true;
			}
			catch (...) {				
			}

			return false;
		}

		bool HJson::loadFromFile(const std::string& jsonFile)
		{
			if (jsonFile.empty()) return false;

			try	{
				FILE *fJson = fopen(jsonFile.c_str(), "r");

				if (fJson == 0) { return false; }

				fseek(fJson, 0, SEEK_END);

				int jsize = ftell(fJson);
				fseek(fJson, 0, SEEK_SET);

				char *jsonPtr = (char *)malloc(jsize * sizeof(char) + 1);
				memset(jsonPtr, 0, jsize * sizeof(char) + 1);
				fread(jsonPtr, jsize, sizeof(char), fJson);
				fclose(fJson);

				bool parseOk = document_->Parse(jsonPtr).HasParseError();
				free(jsonPtr);

				return !parseOk;
			}
			catch (...)	{				
			}

			return false;
		}

		void HJson::insertObject(rapidjson::Value& obj, const std::string& key) {

			if (!obj.IsObject()) {
				obj.SetObject();
			}

			if (!obj.HasMember(key)) {
				rapidjson::Value valueObj(rapidjson::kObjectType);
				obj.AddMember(rapidjson::Value(key, document_->GetAllocator()).Move(), rapidjson::Value(valueObj, document_->GetAllocator()).Move(), document_->GetAllocator());
			}
		}

		void HJson::insertArray(rapidjson::Value& obj, const std::string& key) {

			if (!obj.IsObject()) {
				obj.SetObject();
			}

			if (!obj.HasMember(key)) {
				rapidjson::Value valueObj(rapidjson::kArrayType);
				obj.AddMember(rapidjson::Value(key, document_->GetAllocator()).Move(), rapidjson::Value(valueObj, document_->GetAllocator()).Move(), document_->GetAllocator());
			}
		}
	}
}