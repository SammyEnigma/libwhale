/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	heventpool.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HEVENTPOOL_H
#define HEVENTPOOL_H

#include "whale/basic/hevent.h"

namespace Whale
{
	namespace Basic
	{
		class HEventPool
		{
		public:
			HEventPool(const HEventPool&) = delete;

			inline explicit HEventPool() {
			}

			inline virtual ~HEventPool() {
			}

			inline bool wait(std::shared_ptr<HEvent> eventObj, unsigned long milliseconds = 0) {
				{
					std::lock_guard <std::mutex> locker(locker_);
					eventList_.push_back(std::shared_ptr<HEvent>(eventObj));
				}

				bool result = eventObj->wait(milliseconds);

				{
					std::lock_guard <std::mutex> locker(locker_);
					eventList_.remove(eventObj);
				}

				return result;
			}

			inline void post(int id, const Whale::Basic::HByteArray& buffer) {

				std::lock_guard <std::mutex> locker(locker_);

				for (auto event : eventList_) {
					if (event->id() == id) {
						event->buffer(buffer);
						event->post();
					}
				}
			}

		private:
			std::mutex locker_;
			std::list< std::shared_ptr<HEvent> > eventList_;
		};
	}
}

#endif
