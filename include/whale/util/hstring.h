/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hstring.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HSTRING_H
#define HSTRING_H

#include "whale/basic/hplatform.h"
#include "whale/basic/hbytearray.h"

namespace Whale
{
	namespace Util
	{
		namespace HString
		{
			std::string stringToUTF8(const std::string& val);
			std::string utf8ToString(const std::string& val);

			std::string wstringToString(const std::wstring& val);
			std::wstring stringToWString(const std::string& val);

			std::wstring utf8ToWString(const std::string& val);
			std::string wstringToUTF8(const std::wstring& val);			

			Whale::Basic::HByteArray convert(const char *fromcode, const char *tocode, const Whale::Basic::HByteArray& in);

			std::string toUpper(const std::string &val);
			std::string toLower(const std::string &val);			

			std::vector<std::string> split(const std::string& val, const std::string& pattern);
			std::vector<std::string> cut(const std::string data, std::string pattern0 = "",	std::string pattern1 = "", std::string pattern2 = "", std::string pattern3 = "",
				std::string pattern4 = "", std::string pattern5 = "", std::string pattern6 = "", std::string pattern7 = "", std::string pattern8 = "", std::string pattern9 = "");

			std::string trim(std::string &val);
			std::string replace(const std::string& val, const std::string& sub1, const std::string& sub2);

			std::string random(const std::string & seed, int length);

			template<class OType, class IType>
			OType convert(const IType & in)
			{				
				std::stringstream stream;
				stream << in;
				OType result = 0;
				stream >> result;
				return result;
			}

			template<typename... Args>
			std::string format(const char *format, Args... args)
			{
				std::string content = "";

				if (format)	{
					auto len = snprintf(nullptr, 0, format, args...);
					content.reserve(len + 1);
					content.resize(len);
					snprintf(&content[0], len + 1, format, args...);
				}

				return content;
			}
		}
	}
}

#endif

