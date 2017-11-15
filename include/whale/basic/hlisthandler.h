/***********************************************************************************************
* Copyright (c) 2014-2017, GuangDong CreateW Technology Incorporated Co., Ltd.
*
* File Name			:	hthreadsafelist.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
***********************************************************************************************/
#ifndef _HLISTHANDLER_H_
#define _HLISTHANDLER_H_

#include "whale/basic/hplatform.h"
#include "hevent.h"
#include "hthread.h"
#include "hthreadsafelist.h"

namespace Whale 
{
	namespace Basic
	{
		template <typename T>
		class HListHandler : public Whale::Basic::HThread
		{
		public:
			HListHandler() { }
			virtual ~HListHandler() { stop(); }
		public:
			// push data
			virtual void push(const T& t) {
				_list.push(t);
				_event.post();
			}

			virtual size_t size() {
				return _list.size();
			}

			virtual void stop() {
				_event.post();
				Whale::Basic::HThread::stop();
			}
	
		protected:
			virtual void action() {
				_event.wait();
				while (!_list.empty()) {
					T t;
					if (_list.pop(t)) {
						handle(t);
					}
				} // while
			}
			/**
			 * TODO::implement this func on sub-class
			 */
			virtual void handle(const T& t) = 0;
		
			Whale::Basic::HThreadSafeList<T> _list; /**< stream data record set */
			Whale::Basic::HEvent _event; /**< signal */
		};

	}
}

#endif
