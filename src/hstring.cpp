#include "whale/util/hstring.h"
#include <iconv.h>

#ifdef OS_WIN32
#ifdef _DEBUG
#pragma comment(lib, "Z:/libwhale/deps/libiconv-1.15/lib/libiconvd.lib")
#else
#pragma comment(lib, "Z:/libwhale/deps/libiconv-1.15/lib/libiconv.lib")
#endif
#endif

namespace Whale
{
	namespace Util
	{
		namespace HString
		{
			std::string wstringToString(const std::wstring& val)
			{
				std::string curLocale = "";
				curLocale = setlocale(LC_ALL, 0);

				setlocale(LC_ALL, "chs");

				size_t size = 2 * val.size() + 1;
				const wchar_t* source = val.c_str();

				char *buff = new char[size];
				memset(buff, 0, size);

				wcstombs(buff, source, size);
				std::string result = buff;

				delete[]buff;
				setlocale(LC_ALL, curLocale.c_str());

				return result;
			}

			std::wstring stringToWString(const std::string& val)
			{
				setlocale(LC_ALL, "chs");
				const char* source = val.c_str();

				size_t wsize = val.size() + 1;
				wchar_t *wbuff = new wchar_t[wsize];
				wmemset(wbuff, 0, wsize);

				mbstowcs(wbuff, source, wsize);
				std::wstring result = wbuff;

				delete[]wbuff;
				setlocale(LC_ALL, "C");

				return result;
			}

			std::string wstringToUTF8(const std::wstring& val)
			{
#ifdef OS_WIN32
				std::wstring_convert<std::codecvt_utf8<wchar_t>> cv;
				return cv.to_bytes(val);
#endif // OS_WIN32
				return "";
			}

			std::wstring utf8ToWString(const std::string& val)
			{
#ifdef OS_WIN32
				std::wstring_convert<std::codecvt_utf8<wchar_t> > cv;
				return cv.from_bytes(val);
#endif // OS_WIN32

				return L"";
			}

			std::string stringToUTF8(const std::string& val)
			{
				std::wstring ws = stringToWString(val);
				std::string utf8 = wstringToUTF8(ws);
				return utf8;
			}

			std::string utf8ToString(const std::string& val)
			{
				std::wstring ws = utf8ToWString(val);
				std::string ss = wstringToString(ws);
				return ss;
			}

			Whale::Basic::HByteArray convert(const char *fromcode, const char *tocode, const Whale::Basic::HByteArray& in)
			{
				iconv_t iconvt = iconv_open(tocode, fromcode);
			
				if (iconvt == 0) {
					return Whale::Basic::HByteArray();
				}
				
				size_t inlen = in.size();
				size_t outlen = 3 * in.size();
				Whale::Basic::HByteArray out(outlen);
			
				char *outbuf = const_cast<char*>(out.data());
				char *inbuf = const_cast<char*>(in.data());
			
				size_t rc = iconv(iconvt, &inbuf, &inlen, &outbuf, &outlen);
				iconv_close(iconvt);
			
				if (rc == -1) {
					return Whale::Basic::HByteArray();
				}   
			
				return out.read(3 * in.size() - outlen);
			}

			std::string toUpper(const std::string &val)
			{
				std::string tmp = val;
				if (tmp.empty()) return tmp;
				transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
				return tmp;
			}

			std::string toLower(const std::string &val)
			{
				std::string tmp = val;
				if (tmp.empty()) return tmp;
				transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
				return tmp;
			}

			std::vector<std::string> split(const std::string& val, const std::string& pattern)
			{
				std::vector<std::string> vec;
				if (val.empty() || pattern.empty())
				{
					return vec;
				}

				size_t start = 0, index = val.find(pattern, 0);
				while (index != val.npos)
				{
					vec.push_back(val.substr(start, index - start));
					start = index + pattern.length();
					index = val.find(pattern, start);
				}

				vec.push_back(val.substr(start));

				return vec;
			}

			std::vector<std::string> cut(const std::string data, std::string pattern0,
				std::string pattern1, std::string pattern2, std::string pattern3,
				std::string pattern4, std::string pattern5, std::string pattern6,
				std::string pattern7, std::string pattern8, std::string pattern9)
			{
				std::vector<std::string> result;
				if (data.empty()) return result;

				size_t start = 0;
				size_t index = 0;
				if (!pattern0.empty() && (index = data.find(pattern0, start)) != data.npos)
				{
					if (start != index) result.push_back(data.substr(start, index - start));
					start = index + pattern0.length();
				}

				if (!pattern1.empty() && (index = data.find(pattern1, start)) != data.npos)
				{
					if (start != index) result.push_back(data.substr(start, index - start));
					start = index + pattern1.length();
				}

				if (!pattern2.empty() && (index = data.find(pattern2, start)) != data.npos)
				{
					if (start != index) result.push_back(data.substr(start, index - start));
					start = index + pattern2.length();
				}

				if (!pattern3.empty() && (index = data.find(pattern3, start)) != data.npos)
				{
					if (start != index) result.push_back(data.substr(start, index - start));
					start = index + pattern3.length();
				}

				if (!pattern4.empty() && (index = data.find(pattern4, start)) != data.npos)
				{
					if (start != index) result.push_back(data.substr(start, index - start));
					start = index + pattern4.length();
				}

				if (!pattern5.empty() && (index = data.find(pattern5, start)) != data.npos)
				{
					if (start != index) result.push_back(data.substr(start, index - start));
					start = index + pattern5.length();
				}

				if (!pattern6.empty() && (index = data.find(pattern6, start)) != data.npos)
				{
					if (start != index) result.push_back(data.substr(start, index - start));
					start = index + pattern6.length();
				}

				if (!pattern7.empty() && (index = data.find(pattern7, start)) != data.npos)
				{
					if (start != index) result.push_back(data.substr(start, index - start));
					start = index + pattern7.length();
				}

				if (!pattern8.empty() && (index = data.find(pattern8, start)) != data.npos)
				{
					if (start != index) result.push_back(data.substr(start, index - start));
					start = index + pattern8.length();
				}

				if (!pattern9.empty() && (index = data.find(pattern9, start)) != data.npos)
				{
					if (start != index) result.push_back(data.substr(start, index - start));
					start = index + pattern9.length();
				}

				if (!data.substr(start).empty())
				{
					result.push_back(data.substr(start));
				}

				return result;
			}

			std::string trim(std::string &val)
			{
				if (val.empty())
				{
					return val;
				}

				val.erase(0, val.find_first_not_of(" "));
				val.erase(val.find_last_not_of(" ") + 1);

				return val;
			}

			std::string replace(const std::string& str, const std::string& sub1, const std::string& sub2)
			{
				std::string val = str;
				std::string::size_type pos = 0;
				std::string::size_type srcLen = sub1.size();
				std::string::size_type dstLen = sub2.size();
				while ((pos = val.find(sub1, pos)) != std::string::npos)
				{
					val.replace(pos, srcLen, sub2);
					pos += dstLen;
				}

				return val;
			}

			std::string random(const std::string & seed, int size)
			{
				srand(static_cast<unsigned int>(time(0)));
				std::string result;

				for (int index = 0; index != size; ++index)
				{
					int charIndex = rand() % seed.size();
					result.push_back(seed[charIndex]);
				}

				return result;
			}
		}
	}
}
