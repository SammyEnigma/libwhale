/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hdatetime.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HDATETIME_H
#define HDATETIME_H

#include "whale/basic/hplatform.h"
#include "whale/util/hstring.h"

namespace Whale
{
	namespace Basic
	{
		class HDateTime
		{
		public:
			explicit HDateTime();
			explicit HDateTime(const std::string& timep);
			virtual ~HDateTime();			

			inline operator std::string() const {
				return timestamp(); 
			}

			inline const std::chrono::system_clock::time_point& timepoint() {
				return timePoint_;
			}

			inline void timepoint(std::time_t val) {
				timePoint_ = std::chrono::system_clock::from_time_t(val);
			}

			inline void timepoint(std::chrono::system_clock::time_point val) {
				timePoint_ = val;
			}
			
			inline const uint64_t uptime() {
				return std::chrono::duration_cast<std::chrono::nanoseconds>(timePoint_ - Whale::Basic::HDateTime("1970-07-01 00:00:00.000").timepoint()).count();
			}

			const std::string timestamp(const std::string& format = "%Y-%m-%d %H:%M:%S.###") const;

		private:
			std::chrono::system_clock::time_point timePoint_;
		};

		class HTimer
		{
		public:
			HTimer() : begin_(std::chrono::high_resolution_clock::now())
			{
			}

			void reset()
			{
				begin_ = std::chrono::high_resolution_clock::now();
			}

			// default msecond
			int64_t elapsed() const
			{
				return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin_).count();
			}

			// msecond
			int64_t elapsed_micro() const
			{
				return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - begin_).count();
			}

			// nsecond
			int64_t elapsed_nano() const
			{
				return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - begin_).count();
			}

			// second
			int64_t elapsed_seconds() const
			{
				return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - begin_).count();
			}

			// minite
			int64_t elapsed_minutes() const
			{
				return std::chrono::duration_cast<std::chrono::minutes>(std::chrono::high_resolution_clock::now() - begin_).count();
			}

			// hour
			int64_t elapsed_hours() const
			{
				return std::chrono::duration_cast<std::chrono::hours>(std::chrono::high_resolution_clock::now() - begin_).count();
			}

		private:
			std::chrono::time_point<std::chrono::high_resolution_clock> begin_;
		};
	}
}

#endif

