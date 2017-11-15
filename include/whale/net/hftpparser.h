#ifndef __FTPLISTPARSER_H
#define __FTPLISTPARSER_H

#include "whale/basic/hplatform.h"
#include "whale/net/hftpclient.h"

namespace Whale
{
	namespace Net
	{

		class HFtpFileStatus;

		class IFtpFileListParser : public IFtpDataType
		{
		public:
			virtual ~IFtpFileListParser() {}
			virtual bool parse(HFtpFileStatus& ftpFileStatus, const std::string& strLineToParse) = 0;
		};

		class HFtpListParser : public IFtpFileListParser
		{
		public:
			HFtpListParser();
			virtual ~HFtpListParser();

		public:
			bool parse(HFtpFileStatus& fileStatus, const std::string& line);

		private:
			bool isEpls(const char* pszLine);
			bool parseEplf(HFtpFileStatus& ftpFileStatus, const char* pszLine, int iLength);

			bool isUnixStyleList(const char* pszLine);
			bool parseUnixStyleList(HFtpFileStatus& ftpFileStatus, const char* pszLine, int iLength);

			bool isMultiNetList(const char* pszLine);
			bool parseMultiNetList(HFtpFileStatus& ftpFileStatus, const char* pszLine, int iLength);

			bool isMsdosList(const char* pszLine);
			bool parseMsdosList(HFtpFileStatus& ftpFileStatus, const char* pszLine, int iLength);

			bool checkMonth(const char* pszBuffer, const char* pszMonthName) const;
			int  getMonth(const char* pszBuffer, int iLength) const;
			bool getLong(const char* pszLong, int iLength, long& lResult) const;
			long getYear(time_t time) const;

			long toTai(long lYear, long lMonth, long lMDay) const;
			long guessTai(long lMonth, long lMDay);

		private:
			time_t _timeBase; // time() value on this OS at the beginning of 1970 TAI
			long _rurrentYear; // approximation to current year
			static const char* _months[12];
		};
	}
}

#endif // __FTPLISTPARSER_H
