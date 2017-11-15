/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hloopbuffer.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HLOOPBUFFER_H
#define HLOOPBUFFER_H

#include "whale/basic/hplatform.h"

namespace Whale
{
	namespace Basic
	{
		class HLoopBuffer
		{
		public:
			explicit HLoopBuffer();
			virtual ~HLoopBuffer();

			bool init(int maxmium = 1024);				

			int size();

			int idle();

			int count();

			bool empty();

			void clear();

			int copy(char* data, int len);
			int read(char* data, int len);
			int write(const char* data, int len);

			unsigned char operator [] (int index);

		private:
			bool initialized_ = false;

			int readPos_ = 0;
			int writePos_ = 0;
			
			int size_ = 0;
			int count_ = 0;						

			mutable std::mutex locker_;
			char* buffer_ = nullptr;
		};
	}
}

#endif 
