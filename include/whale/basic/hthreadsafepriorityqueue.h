/***********************************************************************************************
* Copyright (c) 2014-2017, GuangDong CreateW Technology Incorporated Co., Ltd.
*
* File Name			:	hthreadsafepriorityqueue.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
***********************************************************************************************/
#ifndef HTHREADSAFEPRIORITYQUEUE_H
#define HTHREADSAFEPRIORITYQUEUE_H

#include "whale/basic/hplatform.h"

namespace Whale 
{
	namespace Basic
	{
		template<class T>
		class HThreadSafePriorityQueue
		{
		public:
			explicit HThreadSafePriorityQueue();
			virtual ~HThreadSafePriorityQueue();

			int  size();
			bool empty();
			void clear();
			bool push(const T& item);
			bool pop(T& item);

		private:
			std::mutex locker_;
			std::priority_queue<T> queue_;
		};

		template<class T>
		HThreadSafePriorityQueue<T>::HThreadSafePriorityQueue()
		{
		};

		template<class T>
		HThreadSafePriorityQueue<T>::~HThreadSafePriorityQueue()
		{
		};

		template<class T>
		bool HThreadSafePriorityQueue<T>::empty()
		{
			std::lock_guard <std::mutex> lock(locker_);
			return queue_.empty();
		};

		template<class T>
		bool HThreadSafePriorityQueue<T>::push(const T& item)
		{
			std::lock_guard <std::mutex> lock(locker_);

			queue_.push(item);

			return true;
		};

		template<class T>
		int HThreadSafePriorityQueue<T>::size()
		{
			std::lock_guard <std::mutex> lock(locker_);
			return queue_.size();
		};

		template<class T>
		void HThreadSafePriorityQueue<T>::clear()
		{
			std::lock_guard <std::mutex> lock(locker_);
			while (!queue_.empty())
			{
				queue_.pop();
			}
		};

		template<class T>
		bool HThreadSafePriorityQueue<T>::pop(T& item)
		{
			std::lock_guard <std::mutex> lock(locker_);
			if (queue_.empty())
			{
				return false;
			}

			item = queue_.top();
			queue_.pop();
			return true;
		};
	}
}

#endif
