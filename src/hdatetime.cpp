/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
*
* File Name			:	hdatetime.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Replace Version   :   1.0
* Replace Author    :   developer
* Replace Date      :   2016-11-16 12:00:00
****************************************************************************************/
#include "whale/basic/hdatetime.h"

namespace Whale
{
	namespace Basic
	{
		HDateTime::HDateTime()
		{
			timePoint_ = std::chrono::system_clock::now();
		}

		HDateTime::HDateTime(const std::string& timep)
		{
			if (timep.empty()) {
				return;
			}

			std::vector<std::string> times;
			times = Whale::Util::HString::cut(timep, "-", "-", " ", ":", ":", ".");

			int milliseconds = 0;
			std::time_t tt = std::time(nullptr);
			std::tm timepoint = *localtime(&tt);

			timepoint.tm_isdst = -1;

			for (size_t index = 0; index < times.size(); ++index) {

				switch (index) {
				case 0:
					timepoint.tm_year = Whale::Util::HString::convert<int>(times[index]) - 1900;
					break;
				case 1:
					timepoint.tm_mon = Whale::Util::HString::convert<int>(times[index]) - 1;
					break;
				case 2:
					timepoint.tm_mday = Whale::Util::HString::convert<int>(times[index]);
					break;
				case 3:
					timepoint.tm_hour = Whale::Util::HString::convert<int>(times[index]);
					break;
				case 4:
					timepoint.tm_min = Whale::Util::HString::convert<int>(times[index]);
					break;
				case 5:
					timepoint.tm_sec = Whale::Util::HString::convert<int>(times[index]);
					break;
				case 6:
					milliseconds = Whale::Util::HString::convert<int>(times[index]);
					break;
				default:
					break;
				}
			}

			timePoint_ = std::chrono::system_clock::from_time_t(std::mktime(&timepoint));
			timePoint_ += std::chrono::milliseconds(milliseconds);
		}

		HDateTime::~HDateTime()
		{
		}

		const std::string HDateTime::timestamp(const std::string& format) const
		{
			// get milliseconds
			auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint_.time_since_epoch()) % 1000;

			auto currentTimePoint = std::chrono::system_clock::to_time_t(timePoint_);

			// output the time stamp
			std::ostringstream stream;

#ifdef OS_WIN32
			stream << std::put_time(std::localtime(&currentTimePoint), format.c_str());
#else
			char buffer[80];
			auto success = std::strftime(buffer, 80, format.c_str(), std::localtime(&currentTimePoint));
			assert(0 != success);
			stream << buffer;
#endif

			std::ostringstream millsecStream;
			millsecStream << std::setfill('0') << std::setw(3) << milliseconds.count();

			return Whale::Util::HString::replace(stream.str(), "###", millsecStream.str());
		}
	}
}


