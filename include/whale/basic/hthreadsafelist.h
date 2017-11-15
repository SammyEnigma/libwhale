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
#ifndef HTHREADSAFELIST_H
#define HTHREADSAFELIST_H

#include "whale/basic/hplatform.h"

namespace Whale 
{
	namespace Basic
	{
		template<class T>
		class HThreadSafeList : public std::list<T>
		{
		public:	
			explicit HThreadSafeList();
			virtual ~HThreadSafeList();

			int size() const;
			bool empty() const;
			bool has(const T& item);
			void clear();
			void push(const T& item);
			bool pop(T& item);

		private:
			mutable std::mutex locker_;
		};

		template<class T>
		HThreadSafeList<T>::HThreadSafeList()
		{
		}

		template<class T>
		HThreadSafeList<T>::~HThreadSafeList()
		{
		}

		template<class T>
		bool HThreadSafeList<T>::empty() const
		{
			return std::list<T>::empty();
		}

		template<class T>
		bool HThreadSafeList<T>::has(const T& item)
		{
			std::lock_guard <std::mutex> lock(locker_); 

			auto it = std::find(std::list<T>::begin(), std::list<T>::end(), item);
			return it != std::list<T>::end();
		}

		template<class T>
		void HThreadSafeList<T>::push(const T& item)
		{
			std::lock_guard <std::mutex> lock(locker_); 

			std::list<T>::push_back(item);
		}		

		template<class T>
		int HThreadSafeList<T>::size() const
		{
			return std::list<T>::size();
		}

		template<class T>
		void HThreadSafeList<T>::clear()
		{
			std::lock_guard <std::mutex> lock(locker_); 
			std::list<T>::clear();
		}

		template<class T>
		bool HThreadSafeList<T>::pop(T& item)
		{
			std::lock_guard <std::mutex> lock(locker_); 
			if (!empty()) {
				item = std::list<T>::front();
				std::list<T>::pop_front();
				return true;
			}
			return false;
		}
	}
}

#endif
