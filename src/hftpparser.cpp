#include "whale/net/hftpparser.h"
#include "whale/net/hftpstatus.h"

namespace Whale
{
	namespace Net
	{

		tm& GmTime(tm& _tm, const time_t& time)
		{
#if _MSC_VER >= 1500
			gmtime_s(&_tm, &time);
#else
			_tm = *gmtime(&time);
#endif
			return _tm;
		}

#ifndef INSERT_TIME

#ifdef _DEBUG
		void InsertTime(HFtpFileStatus& ftpFileStatus)
		{
#if _MSC_VER >= 1500
			tm _tm = { 0 };
			if (ftpFileStatus.mtime() > 0)
			{
				char buffer[32] = { 0 };
				asctime_s(buffer, sizeof(buffer), &GmTime(_tm, ftpFileStatus.mtime()));
				ftpFileStatus.mtimeStr() = buffer;
			}
#else
			tm _tm = {0};
			if( ftpFileStatus.mtime()>0 )
				ftpFileStatus.mtimeStr() = HConvert::ConvertToTString(asctime(&GmTime(_tm, ftpFileStatus.mtime())));
#endif
		}

#define INSERT_TIME(ftpFileStatus) InsertTime(ftpFileStatus);
#else
#define INSERT_TIME(ftpFileStatus)
#endif
#endif

		const char* HFtpListParser::_months[12] = {
			("jan"), ("feb"), ("mar"), ("apr"), ("may"), ("jun"),
			("jul"), ("aug"), ("sep"), ("oct"), ("nov"), ("dec")
		};

		HFtpListParser::HFtpListParser() :
			_rurrentYear(-1)
		{
			_timeBase = 0;
			tm tm = { 0 };
			GmTime(tm, _timeBase);
			_timeBase = -(toTai(tm.tm_year + 1900, tm.tm_mon, tm.tm_mday) +
				tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec);
			// assumes the right time_t, counting seconds.
			// base may be slightly off if time_t counts non-leap seconds.
		}

		HFtpListParser::~HFtpListParser()
		{
		}

		bool HFtpListParser::checkMonth(const char* pszBuffer, const char* pszMonthName) const
		{
			if (tolower(pszBuffer[0]) != pszMonthName[0] ||
				tolower(pszBuffer[1]) != pszMonthName[1] ||
				tolower(pszBuffer[2]) != pszMonthName[2])
				return false;

			return true;
		}

		int HFtpListParser::getMonth(const char* pszBuffer, int iLength) const
		{
			if (iLength == 3)
			{
				for (int i = 0; i < 12; ++i)
				{
					if (checkMonth(pszBuffer, _months[i]))
						return i;
				}
			}
			return -1;
		}

		bool HFtpListParser::getLong(const char* pszLong, int iLength, long& lResult) const
		{
			std::string strLong(pszLong, iLength);

			char* pszEndPtr = NULL;
			lResult = strtoul(strLong.c_str(), &pszEndPtr, 10);

			if (pszEndPtr != NULL && pszEndPtr[0] != ('\0'))
				return false;

			return true;
		}

		long HFtpListParser::toTai(long lYear, long lMonth, long lMDay) const
		{
			if (lMonth >= 2)
			{
				lMonth -= 2;
			}
			else
			{
				lMonth += 10;
				--lYear;
			}

			long lResult = (lMDay - 1) * 10 + 5 + 306 * lMonth;
			lResult /= 10;

			if (lResult == 365)
			{
				lYear -= 3;
				lResult = 1460;
			}
			else
				lResult += 365 * (lYear % 4);

			lYear /= 4;

			lResult += 1461 * (lYear % 25);

			lYear /= 25;

			if (lResult == 36524)
			{
				lYear -= 3;
				lResult = 146096;
			}
			else
			{
				lResult += 36524 * (lYear % 4);
			}

			lYear /= 4;
			lResult += 146097 * (lYear - 5);
			lResult += 11017;

			return lResult * 86400;
		}

		long HFtpListParser::getYear(time_t time) const
		{
			long lDay = static_cast<long>(time / 86400L);

			if ((time % 86400L) < 0)
				--lDay;

			lDay -= 11017;

			long lYear = 5 + lDay / 146097;

			lDay = lDay % 146097;
			if (lDay < 0)
			{
				lDay += 146097;
				--lYear;
			}
			lYear *= 4;

			if (lDay == 146096)
			{
				lYear += 3;
				lDay = 36524;
			}
			else
			{
				lYear += lDay / 36524;
				lDay %= 36524;
			}

			lYear *= 25;
			lYear += lDay / 1461;
			lDay %= 1461;
			lYear *= 4;

			if (lDay == 1460)
			{
				lYear += 3;
				lDay = 365;
			}
			else
			{
				lYear += lDay / 365;
				lDay %= 365;
			}

			lDay *= 10;
			if ((lDay + 5) / 306 >= 10)
				++lYear;

			return lYear;
		}

		/// UNIX ls does not show the year for dates in the last six months.
		/// So we have to guess the year.
		/// Apparently NetWare uses ``twelve months'' instead of ``six months''; ugh.
		/// Some versions of ls also fail to show the year for future dates.
		long HFtpListParser::guessTai(long lMonth, long lMDay)
		{
			///////////////////////////*/*/*/*/
			time_t now = time((time_t *)0) - _timeBase;

			if (_rurrentYear == -1)
			{
				_rurrentYear = getYear(now);
			}

			long lTAI = 0;
			for (long lYear = _rurrentYear - 1; lYear < _rurrentYear + 100; ++lYear)
			{
				lTAI = toTai(lYear, lMonth, lMDay);
				if (now - lTAI < 350 * 86400)
					return lTAI;
			}
			return lTAI;
		}

		/// Easily Parsed LIST Format (EPLF)
		/// see http://pobox.com/~djb/proto/eplf.txt
		/// "+i8388621.29609,m824255902,/,\tdev"
		/// "+i8388621.44468,m839956783,r,s10376,\tRFCEPLF"
		bool HFtpListParser::isEpls(const char* pszLine)
		{
			return pszLine && *pszLine == ('+');
		}

		bool HFtpListParser::parseEplf(HFtpFileStatus& ftpFileStatus, const char* pszLine, int iLength)
		{
			if (!isEpls(pszLine))
				return false;

			long lTemp = 0;
			int i = 1;
			for (int j = 1; j < iLength; ++j)
			{
				if (pszLine[j] == ('\t'))
				{
					ftpFileStatus.name() = pszLine + j + 1;
					return true;
				}

				if (pszLine[j] == (','))
				{
					switch (pszLine[i])
					{
					case ('/') :
						ftpFileStatus.cmdEnable() = true;
						break;
					case ('r') :
						ftpFileStatus.retryEnable() = true;
						break;
					case ('s') :
						ftpFileStatus.sizeType() = HFtpFileStatus::FTP_FILE_ENCODE_BINARY;
						if (!getLong(pszLine + i + 1, j - i - 1, ftpFileStatus.Size()))
							ftpFileStatus.Size() = -1;
						break;
					case ('m') :
						ftpFileStatus.mtimeType() = HFtpFileStatus::FTP_FILE_MTIME_LOCAL;
						getLong(pszLine + i + 1, j - i - 1, lTemp);
						ftpFileStatus.mtime() = _timeBase + lTemp;
						INSERT_TIME(ftpFileStatus);
						break;
					case ('i') :
						ftpFileStatus.idType() = HFtpFileStatus::FTP_FILE_IDTYPE_FULL;
						ftpFileStatus.id() = pszLine + i + 1;
						ftpFileStatus.id() = ftpFileStatus.id().substr(0, j - i - 1);
					}
					i = j + 1;
				}
			}
			return false;
		}

		/// UNIX-style listing, without inum and without blocks
		/// "-rw-r--r--   1 root     other        531 Jan 29 03:26 README"
		/// "dr-xr-xr-x   2 root     other        512 Apr  8  1994 etc"
		/// "dr-xr-xr-x   2 root     512 Apr  8  1994 etc"
		/// "lrwxrwxrwx   1 root     other          7 Jan 25 00:17 bin -> usr/bin"
		///
		/// Also produced by Microsoft's FTP servers for Windows:
		/// "----------   1 owner    group         1803128 Jul 10 10:18 ls-lR.Z"
		/// "d---------   1 owner    group               0 May  9 19:45 Softlib"
		///
		/// Also WFTPD for MSDOS:
		/// "-rwxrwxrwx   1 noone    nogroup      322 Aug 19  1996 message.ftp"
		///
		/// Also NetWare:
		/// "d [R----F--] supervisor            512       Jan 16 18:53    login"
		/// "- [R----F--] rhesus             214059       Oct 20 15:27    cx.exe"
		//
		/// Also NetPresenz for the Mac:
		/// "-------r--         326  1391972  1392298 Nov 22  1995 MegaPhone.sit"
		/// "drwxrwxr-x               folder        2 May 10  1996 network"
		bool HFtpListParser::isUnixStyleList(const char* pszLine)
		{
			if (pszLine == NULL)
				return false;

			switch (*pszLine)
			{
			case ('b') :
			case ('c') :
			case ('d') :
			case ('l') :
			case ('p') :
			case ('s') :
			case ('-') :
					   return true;
			}
			return false;
		}

		bool HFtpListParser::parseUnixStyleList(HFtpFileStatus& ftpFileStatus, const char* pszLine, int iLength)
		{
			if (!isUnixStyleList(pszLine))
				return false;

			switch (*pszLine)
			{
			case ('d') : ftpFileStatus.cmdEnable() = true; break;
			case ('-') : ftpFileStatus.retryEnable() = true; break;
			case ('l') : ftpFileStatus.cmdEnable() = true;
				ftpFileStatus.retryEnable() = true;
			}

			int  iState = 1;
			int  i = 0;
			long lSize = 0;
			long lYear = 0;
			long lMonth = 0;
			long lMDay = 0;
			long lHour = 0;
			long lMinute = 0;

			for (int j = 1; j < iLength; ++j)
			{
				if (pszLine[j] == (' ') && pszLine[j - 1] != (' '))
				{
					switch (iState)
					{
					case 1: // skipping perm
						ftpFileStatus.attributes().assign(pszLine + i, j - i);
						iState = 2;
						break;
					case 2: // skipping nlink
						iState = 3;
						ftpFileStatus.link().assign(pszLine + i, j - i);
						if (j - i == 6 && pszLine[i] == ('f')) // for NetPresenz
							iState = 4;
						break;
					case 3: // skipping uid
						iState = 4;
						ftpFileStatus.uid().assign(pszLine + i, j - i);
						break;
					case 4: // getting tentative size
						if (!getLong(pszLine + i, j - i, lSize))
						{
							lSize = -1;
							ftpFileStatus.gid().assign(pszLine + i, j - i);
						}
						iState = 5;
						break;
					case 5: // searching for month, otherwise getting tentative size
						lMonth = getMonth(pszLine + i, j - i);
						if (lMonth >= 0)
							iState = 6;
						else
						{
							if (!getLong(pszLine + i, j - i, lSize))
								lSize = -1;
						}
						break;
					case 6: // have size and month
						getLong(pszLine + i, j - i, lMDay);
						iState = 7;
						break;
					case 7: // have size, month, mday
						if (j - i == 4 && pszLine[i + 1] == (':'))
						{
							getLong(pszLine + i, 1, lHour);
							getLong(pszLine + i + 2, 2, lMinute);
							ftpFileStatus.mtimeType() = HFtpFileStatus::FTP_FILE_MTIME_REMOTE_MINUTE;
							ftpFileStatus.mtime() = _timeBase + guessTai(lMonth, lMDay) + lHour * 3600 + lMinute * 60;
							INSERT_TIME(ftpFileStatus);
						}
						else if (j - i == 5 && pszLine[i + 2] == (':'))
						{
							getLong(pszLine + i, 2, lHour);
							getLong(pszLine + i + 3, 2, lMinute);
							ftpFileStatus.mtimeType() = HFtpFileStatus::FTP_FILE_MTIME_REMOTE_MINUTE;
							ftpFileStatus.mtime() = _timeBase + guessTai(lMonth, lMDay) + lHour * 3600 + lMinute * 60;
							INSERT_TIME(ftpFileStatus);
						}
						else if (j - i >= 4)
						{
							getLong(pszLine + i, j - i, lYear);
							ftpFileStatus.mtimeType() = HFtpFileStatus::FTP_FILE_MTIME_REMOTE_DAY;
							ftpFileStatus.mtime() = _timeBase + HFtpListParser::toTai(lYear, lMonth, lMDay);
							INSERT_TIME(ftpFileStatus);
						}
						else
							return false;

						ftpFileStatus.name() = pszLine + j + 1;
						iState = 8;
						break;
					case 8: // twiddling thumbs
						break;
					}

					i = j + 1;
					while (i < iLength && pszLine[i] == (' '))
						++i;
				}
			}

			if (iState != 8)
				return false;

			ftpFileStatus.Size() = lSize;
			ftpFileStatus.sizeType() = HFtpFileStatus::FTP_FILE_ENCODE_BINARY;

			// handle links
			if (pszLine[0] == ('l'))
			{
				std::string::size_type pos = ftpFileStatus.name().find((" -> "));
				if (pos != std::string::npos)
					ftpFileStatus.name() = ftpFileStatus.name().substr(0, pos);
			}

			// eliminate extra NetWare spaces
			if (pszLine[1] == (' ') || pszLine[1] == ('['))
			{
				if (ftpFileStatus.name().length() > 3 && ftpFileStatus.name().substr(0, 3) == ("   "))
					ftpFileStatus.name() = ftpFileStatus.name().substr(3);
			}

			return true;
		}

		/// MultiNet (some spaces removed from examples)
		/// "00README.TXT;1      2 30-DEC-1996 17:44 [SYSTEM] (RWED,RWED,RE,RE)"
		/// "CORE.DIR;1          1  8-SEP-1996 16:09 [SYSTEM] (RWE,RWE,RE,RE)"
		/// and non-MutliNet VMS:
		/// "CII-MANUAL.TEX;1  213/216  29-JAN-1996 03:33:12  [ANONYMOU,ANONYMOUS]   (RWED,RWED,,)"
		bool HFtpListParser::isMultiNetList(const char* pszLine)
		{
			return pszLine && strchr(pszLine, (';')) != NULL;
		}

		bool HFtpListParser::parseMultiNetList(HFtpFileStatus& ftpFileStatus, const char* pszLine, int iLength)
		{
			if (!isMultiNetList(pszLine))
				return false;

			// name lookup of `i' changed for new ISO `for' scoping
			int i = 0;
			for (; i < iLength; ++i)
				if (pszLine[i] == (';'))
					break;

			if (i < iLength)
			{
				ftpFileStatus.name() = pszLine;
				ftpFileStatus.name() = ftpFileStatus.name().substr(0, i);
				if (i > 4 && strncmp(pszLine + i - 4, (".DIR"), 4) == 0)
				{
					ftpFileStatus.name() = ftpFileStatus.name().substr(0, ftpFileStatus.name().length() - 4);
					ftpFileStatus.cmdEnable() = true;
				}
				else
					ftpFileStatus.retryEnable() = true;

				while (pszLine[i] != (' ')) if (++i == iLength) return false;
				while (pszLine[i] == (' ')) if (++i == iLength) return false;
				while (pszLine[i] != (' ')) if (++i == iLength) return false;
				while (pszLine[i] == (' ')) if (++i == iLength) return false;

				int j = i;
				while (pszLine[j] != ('-')) if (++j == iLength) return false;
				long lMDay = 0;
				getLong(pszLine + i, j - i, lMDay);

				while (pszLine[j] == ('-')) if (++j == iLength) return false;
				i = j;
				while (pszLine[j] != ('-')) if (++j == iLength) return false;
				long lMonth = getMonth(pszLine + i, j - i);
				if (lMonth < 0) return false;

				while (pszLine[j] == ('-')) if (++j == iLength) return false;
				i = j;
				while (pszLine[j] != (' ')) if (++j == iLength) return false;
				long lYear = 0;
				getLong(pszLine + i, j - i, lYear);

				while (pszLine[j] == (' ')) if (++j == iLength) return false;
				i = j;
				while (pszLine[j] != (':')) if (++j == iLength) return false;
				long lHour = 0;
				getLong(pszLine + i, j - i, lHour);

				while (pszLine[j] == (':')) if (++j == iLength) return false;
				i = j;
				while (pszLine[j] != (':') && pszLine[j] != (' ')) if (++j == iLength) return false;
				long lMinute = 0;
				getLong(pszLine + i, j - i, lMinute);

				ftpFileStatus.mtimeType() = HFtpFileStatus::FTP_FILE_MTIME_REMOTE_MINUTE;
				ftpFileStatus.mtime() = _timeBase + HFtpListParser::toTai(lYear, lMonth, lMDay) + lHour * 3600 + lMinute * 60;
				INSERT_TIME(ftpFileStatus);
			}

			return true;
		}

		/// MSDOS format
		/// 04-27-00  09:09PM       <DIR>          licensed
		/// 07-18-00  10:16AM       <DIR>          pub
		/// 04-14-00  03:47PM                  589 readme.htm
		bool HFtpListParser::isMsdosList(const char* pszLine)
		{
			return pszLine && isdigit(pszLine[0]);
		}

		bool HFtpListParser::parseMsdosList(HFtpFileStatus& ftpFileStatus, const char* pszLine, int iLength)
		{
			if (!isMsdosList(pszLine))
				return false;

			int i = 0;
			int j = 0;
			while (pszLine[j] != ('-')) if (++j == iLength) return false;
			long lMonth = 0;
			getLong(pszLine + i, j - i, lMonth); //+# -1

			while (pszLine[j] == ('-')) if (++j == iLength) return false;
			i = j;
			while (pszLine[j] != ('-')) if (++j == iLength) return false;
			long lMDay = 0;
			getLong(pszLine + i, j - i, lMDay);

			while (pszLine[j] == ('-')) if (++j == iLength) return false;
			i = j;
			while (pszLine[j] != (' ')) if (++j == iLength) return false;
			long lYear = 0;
			getLong(pszLine + i, j - i, lYear);

			if (lYear < 50)
				lYear += 2000;

			if (lYear < 1000)
				lYear += 1900;

			while (pszLine[j] == (' ')) if (++j == iLength) return false;
			i = j;
			while (pszLine[j] != (':')) if (++j == iLength) return false;
			long lHour = 0;
			getLong(pszLine + i, j - i, lHour);

			while (pszLine[j] == (':')) if (++j == iLength) return false;
			i = j;
			while ((pszLine[j] != ('A')) && (pszLine[j] != ('P'))) if (++j == iLength) return false;
			long lMinute = 0;
			getLong(pszLine + i, j - i, lMinute);

			if (lHour == 12)
				lHour = 0;

			if (pszLine[j] == ('A')) if (++j == iLength) return false;
			if (pszLine[j] == ('P')) { lHour += 12; if (++j == iLength) return false; }
			if (pszLine[j] == ('M')) if (++j == iLength) return false;

			while (pszLine[j] == (' ')) if (++j == iLength) return false;
			if (pszLine[j] == ('<'))
			{
				ftpFileStatus.cmdEnable() = true;
				while (pszLine[j] != (' ')) if (++j == iLength) return false;
			}
			else
			{
				i = j;
				while (pszLine[j] != (' ')) if (++j == iLength) return false;
				if (!getLong(pszLine + i, j - i, ftpFileStatus.Size()))
					ftpFileStatus.Size() = -1;
				ftpFileStatus.sizeType() = HFtpFileStatus::FTP_FILE_ENCODE_BINARY;
				ftpFileStatus.retryEnable() = true;
			}
			while (pszLine[j] == (' ')) if (++j == iLength) return false;

			ftpFileStatus.name() = pszLine + j;

			ftpFileStatus.mtimeType() = HFtpFileStatus::FTP_FILE_MTIME_REMOTE_MINUTE;
			ftpFileStatus.mtime() = _timeBase + HFtpListParser::toTai(lYear, lMonth, lMDay) + lHour * 3600 + lMinute * 60;
			INSERT_TIME(ftpFileStatus);

			return true;
		}

		bool HFtpListParser::parse(HFtpFileStatus& ftpFileStatus, const std::string& strLineToParse)
		{
			ftpFileStatus.reset();
			const char*   pszLine = strLineToParse.c_str();
			const int iLength = static_cast<int>(strLineToParse.length());

			if (iLength < 2) // an empty name in EPLF, with no info, could be 2 chars
				return false;

			if (isEpls(pszLine))
			{
				if (!parseEplf(ftpFileStatus, pszLine, iLength))
					return false;
			}
			else if (isUnixStyleList(pszLine))
			{
				if (!parseUnixStyleList(ftpFileStatus, pszLine, iLength))
					return false;
			}
			else if (isMultiNetList(pszLine))
			{
				if (!parseMultiNetList(ftpFileStatus, pszLine, iLength))
					return false;
			}
			else if (isMsdosList(pszLine))
			{
				if (!parseMsdosList(ftpFileStatus, pszLine, iLength))
					return false;
			}
			else
			{
				// Some useless lines, safely ignored:
				// "Total of 11 Files, 10966 Blocks." (VMS)
				// "total 14786" (UNIX)
				// "DISK$ANONFTP:[ANONYMOUS]" (VMS)
				// "Directory DISK$PCSA:[ANONYM]" (VMS)
				return false;
			}

			return true;
		}
	}
}