/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hbytearray.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HBYTEARRAY_H
#define HBYTEARRAY_H

#include "whale/basic/hplatform.h"

namespace Whale
{
	namespace Basic
	{
		class HByteArray : public std::vector<char>
		{
		public:
			HByteArray() = default;
			HByteArray(size_t size) : std::vector<char>(size) {	}
			HByteArray(const char *buffer, size_t size) : std::vector<char>(buffer, buffer + size) { }

			inline HByteArray(const std::string &str) : std::vector<char>(str.size()) {
				memcpy(data(), str.c_str(), str.size());
			}

			inline HByteArray copy(size_t pos, size_t size) {
				if (pos < this->size()) {
					return HByteArray(this->data() + pos, size > this->size() - pos ? this->size() - pos : size);
				}

				return HByteArray();
			}

			template<class T>
			inline T copy(size_t pos = 0) {
				T t;
				size_t sizeToRead = sizeof(T);
				if (pos < this->size()) {
					memcpy(&t, this->data() + pos, sizeToRead > this->size() - pos ? this->size() - pos : sizeToRead);
				}
				return t;
			}

			inline HByteArray read(size_t sizeToRead) {
				HByteArray byteTmp1(this->data(), sizeToRead >= this->size() ? this->size() : sizeToRead);

				if (sizeToRead < this->size()) {
					HByteArray byteTmp2(this->data() + sizeToRead, this->size() - sizeToRead);
					clear();
					concat(byteTmp2);
				}
				else {
					clear();
				}

				return byteTmp1;
			}

			template<class T>
			inline T read() {
				T t;
				size_t sizeToRead = sizeof(T);

				HByteArray byteTmp1(this->data(),
					sizeToRead >= this->size() ? this->size() : sizeToRead);

				if (sizeToRead < this->size()) {
					HByteArray byteTmp2(this->data() + sizeToRead, this->size() - sizeToRead);
					clear();
					concat(byteTmp2);
				}
				else {
					clear();
				}

				memcpy(&t, byteTmp1.data(), byteTmp1.size());

				return t;
			}

			template<class T>
			inline HByteArray& write(const T &value) {
				HByteArray byteTmp1((const char *)&value, sizeof(T));
				return concat(byteTmp1);
			}

			inline HByteArray& write(const HByteArray &buffer) {
				return concat(buffer);
			}

			inline HByteArray &concat(const HByteArray &buffer) {
				size_t oldSize = size();
				size_t newSize = oldSize + buffer.size();

				resize(newSize);
				memcpy(this->data() + oldSize, buffer.data(), buffer.size());

				return *this;
			}

			inline std::string toString() const {
				return std::string(this->cbegin(), this->cend());
			}

			inline std::string toHex() const {
				char hexEncoded[3] = { 0 };
				std::string hexTmp = "";

				for (auto ch : *this) {
					memset(hexEncoded, 0, 3);
					snprintf(hexEncoded, 3, "%02X", (uint8_t)ch);
					hexTmp += hexEncoded;
				}

				return hexTmp;
			}

			inline void fromHex(const std::string &str) {
				int buffSize = str.length();
				char hexBuff[3] = { 0 };

				for (auto i = 0; i < buffSize; ++i)
				{
					if (i % 2 == 0)
					{
						memcpy(hexBuff, &str[i], 2);
						push_back(strtoul(hexBuff, NULL, 16) & 0xFF);
					}
				}
			}

			inline HByteArray operator+(const HByteArray &buffer) const {
				HByteArray tmpBuffer(this->size() + buffer.size());
				memcpy(tmpBuffer.data(), this->data(), this->size());
				memcpy(tmpBuffer.data() + this->size(), buffer.data(), buffer.size());

				return tmpBuffer;
			}

			inline HByteArray &operator+= (const HByteArray &buffer) {
				return concat(buffer);
			}
		};
	}
}

#endif
