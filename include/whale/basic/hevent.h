/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hevent.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HEVENT_H
#define HEVENT_H

#include <random>
#include "whale/basic/hbytearray.h"

namespace Whale
{
	namespace Basic
	{
		class HEvent
		{
		public:
			inline HEvent() {  
				std::random_device rd; 
				id_ = rd(); 
				signal_ = false; 
			}

			inline HEvent(int id) : id_(id) {
				signal_ = false;
			}

			inline const int &id() const {
				return id_;
			}
			inline void id(int eventId) {
				id_ = eventId;
			}

			inline const HByteArray &buffer() const {
				return buffer_;
			}
			inline void buffer(const HByteArray &data) {
				buffer_ = data;
			}

			inline bool wait(unsigned long milliseconds = 0) {
				std::unique_lock<std::mutex> locker(locker_);

				if (signal_) {
					signal_ = false;
					return true;
				}
				
				if (milliseconds == 0) {
					condition_.wait(locker);
					signal_ = false;
					return true;
				}

				if (std::cv_status::no_timeout ==
					condition_.wait_for(locker, std::chrono::milliseconds(milliseconds))) {
					signal_ = false;
					return true;
				}
				
				return false;
			}

			inline void post()	{
				std::lock_guard <std::mutex> lock(locker_);
				signal_ = true;
				condition_.notify_one();
			}

		private:
			int id_;
			Whale::Basic::HByteArray buffer_;

			bool signal_;
			std::mutex locker_;
			std::condition_variable condition_;
		};
	}
}

#endif
