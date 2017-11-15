/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hthread.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HTHREAD_H
#define HTHREAD_H

#include "whale/basic/hplatform.h"
#include "hevent.h"

namespace Whale
{
	namespace Basic
	{
		class HThread
		{
		public:
			explicit HThread();
			virtual ~HThread();

			inline int intervalMillSecs() const { 
				return interval_;
			}
			inline void intervalMillSecs(int val) {
				interval_ = val; 
			}

			inline int intervalMissions() const {
				return missions_;
			}
			inline void intervalMissions(int val) {
				missions_ = val;
			}

			inline bool onlyOnceRecyle() const {
				return singleRecyle_;
			}
			inline void onlyOnceRecyle(bool val) {
				singleRecyle_ = val;
			}			

			inline void quit() {
				working_ = false;
			}

			inline bool working() const {
				return working_; 
			}	

			inline std::thread::id threadid() const {
				return thread_->get_id(); 
			}

			inline std::string name() const {
				return name_;
			}
			inline void name(const std::string &val) {
				name_ = val;
			}

			virtual bool run();
			virtual void stop();

		protected:
			virtual void beforeRun() { }
			virtual void afterRun() { }

			virtual void action() = 0;
		private:
			bool working_;
			int  interval_ = 1;
			int  missions_ = 1;
			bool singleRecyle_ = false;
			std::string name_;
			
			void* pvoid_;
			std::thread* thread_;
			Whale::Basic::HEvent quitEvent_;
			Whale::Basic::HEvent confirmEvent_;

			static void work(void *);
			void routine();
		};
	}
}

#endif
