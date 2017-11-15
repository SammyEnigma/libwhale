/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hjson.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HJSON_H
#define HJSON_H

#ifdef OS_WIN32
#pragma warning(disable:4003)
#endif

#define RAPIDJSON_HAS_STDSTRING 1

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/writer.h"
#include "rapidjson/allocators.h"

#include "whale/basic/hplatform.h"


namespace Whale
{
	namespace Util
	{
		class HJson
		{
		public:
			explicit HJson(void);
			explicit HJson(const HJson& json);
			virtual HJson& operator=(const HJson& json);
			virtual ~HJson(void);			

			bool empty();

			bool saveToFile(const std::string& jsonFile);
			bool loadFromFile(const std::string& jsonFile);

			bool decode(const std::string& jsonStr);

			std::string encode(rapidjson::Value& obj);
			std::string encodePretty(rapidjson::Value& obj);			

			template<typename T>
			void pushBackArray(rapidjson::Value& obj, T value);

			template<typename T>
			void writeObject(rapidjson::Value& obj, const std::string& key, T value);

			template<typename T>
			void writeArray(rapidjson::Value& obj, const std::string& key, std::vector<T> value);

			void insertObject(rapidjson::Value& obj, const std::string& key);

			void insertArray(rapidjson::Value& obj, const std::string& key);

			void clearArray(rapidjson::Value& obj) {
				if (obj.IsArray()) { obj.Clear(); }
			}

			void eraseValue(rapidjson::Value& obj, const char* key) {
				if (obj.IsObject() && obj.HasMember(key)) { obj.EraseMember(key); }
			}

			inline operator rapidjson::Value&() {
				return *document_;
			}

			inline rapidjson::Value& operator [](const std::string& key){
				return (*document_)[key];
			}

			inline rapidjson::Value& operator [](int index) {
				return (*document_)[index];
			}

			bool readBool(rapidjson::Value& obj, const std::string& key) {
				return isBool(obj, key) && (obj)[key].GetBool();
			}

			std::string readString(rapidjson::Value& obj, const std::string& key) {
				return isString(obj, key) ? (obj)[key].GetString() : "";
			}

			int readInt32(rapidjson::Value& obj, const std::string& key) {
				if (isInt32(obj, key)) {
					return (obj)[key].GetInt();
				}
				if (isString(obj, key)) {
					return strtol((obj)[key].GetString(), NULL, 10);
				}
				return 0;
			}

			unsigned int readUInt32(rapidjson::Value& obj, const std::string& key) {
				if (isUInt32(obj, key))	{
					return (obj)[key].GetUint();
				}

				if (isString(obj, key)) {
					return strtoul((obj)[key].GetString(), NULL, 10);
				}

				return 0;
			}

			long long readInt64(rapidjson::Value& obj, const std::string& key) {
				if (isInt64(obj, key)) {
					return (obj)[key].GetInt64();
				}

				if (isString(obj, key)) {
					return strtoll((obj)[key].GetString(), NULL, 10);
				}

				return 0;
			}

			uint64_t readUInt64(rapidjson::Value& obj, const std::string& key) {
				if (isUInt64(obj, key)) {
					return (obj)[key].GetUint64();
				}

				if (isString(obj, key)) {
					return strtoull((obj)[key].GetString(), NULL, 10);
				}

				return 0;
			}

			float readFloat(rapidjson::Value& obj, const std::string& key) {
				return isFloat(obj, key) ? (obj)[key].GetFloat() : 0;
			}

			double readDouble(rapidjson::Value& obj, const std::string& key) {
				return isDouble(obj, key) ? (obj)[key].GetDouble() : 0;
			}			

			bool isBool(rapidjson::Value& obj, const std::string& key) {
				return obj.IsObject() && obj.HasMember(key) && obj[key].IsBool();
			}
			bool isString(rapidjson::Value& obj, const std::string& key) {
				return obj.IsObject() && obj.HasMember(key) && obj[key].IsString();
			}

			bool isInt32(rapidjson::Value& obj, const std::string& key) {
				return obj.IsObject() && obj.HasMember(key) && obj[key].IsInt();
			}

			bool isUInt32(rapidjson::Value& obj, const std::string& key) {
				return obj.IsObject() && obj.HasMember(key) && obj[key].IsUint();
			}

			bool isInt64(rapidjson::Value& obj, const std::string& key) {
				return obj.IsObject() && obj.HasMember(key) && obj[key].IsInt64();
			}

			bool isUInt64(rapidjson::Value& obj, const std::string&  key) {
				return obj.IsObject() && obj.HasMember(key) && obj[key].IsUint64();
			}

			bool isFloat(rapidjson::Value& obj, const std::string& key) {
				return obj.IsObject() && obj.HasMember(key) && obj[key].IsFloat();
			}

			bool isDouble(rapidjson::Value& obj, const std::string& key) {
				return obj.IsObject() && obj.HasMember(key) && obj[key].IsDouble();
			}

			bool isNumber(rapidjson::Value& obj, const std::string& key) {
				return obj.IsObject() && obj.HasMember(key) && obj[key].IsNumber();
			}

			bool isArray(rapidjson::Value& obj, const std::string& key) {
				return obj.IsObject() && obj.HasMember(key) && obj[key].IsArray();
			}

			bool isObject(rapidjson::Value& obj, const std::string& key) {
				return obj.IsObject() && obj.HasMember(key) && obj[key].IsObject();
			}

		private:
			rapidjson::Document* document_;
		};

		template<typename T>
		void HJson::pushBackArray(rapidjson::Value& obj, T value) {

			if (!obj.IsArray()) {
				obj.SetArray();
			}

			if (obj.IsArray()) {
				rapidjson::Value valueObj(rapidjson::kObjectType);
				valueObj.Set(value, document_->GetAllocator());
				obj.PushBack(valueObj, document_->GetAllocator());
			}
		}

		template<typename T>
		void HJson::writeObject(rapidjson::Value& obj, const std::string& key, T value) {

			if (!obj.IsObject()) {
				obj.SetObject();
			}

			if (!obj.HasMember(key)) {
				rapidjson::Value valueObj(rapidjson::kObjectType);
				obj.AddMember(rapidjson::Value(key, document_->GetAllocator()).Move(), rapidjson::Value(valueObj, document_->GetAllocator()).Move(), document_->GetAllocator());
			}

			obj[key].Set(value, document_->GetAllocator());
		}

		template<typename T>
		void HJson::writeArray(rapidjson::Value& obj, const std::string& key, std::vector<T> value) {

			if (!obj.IsObject()) {
				obj.SetObject();
			}

			if (!obj.HasMember(key)) {
				rapidjson::Value valueObj(rapidjson::kArrayType);
				obj.AddMember(rapidjson::Value(key, document_->GetAllocator()).Move(), rapidjson::Value(valueObj, document_->GetAllocator()).Move(), document_->GetAllocator());
			}

			if (obj[key].IsArray()) {
				for (size_t i = 0; i < value.size(); i++) {
					pushBackArray(obj[key], value[i]);
				}
			}
		}
	}
}

#endif

