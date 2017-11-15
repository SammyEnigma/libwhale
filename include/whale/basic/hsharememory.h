/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hsharememory.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HSHAREMEMORY_H
#define HSHAREMEMORY_H

#include "whale/basic/hplatform.h"
#include "whale/basic/hthread.h"

namespace Whale
{
	namespace Basic
	{
		class HShareMemory : public Whale::Basic::HThread
		{
		public:
			explicit HShareMemory();
			virtual ~HShareMemory();
			
			virtual bool init(bool isInitialOwner, const std::string& shmName, int smSize,
				const std::string& lkName, const std::string& listenEvtName, const std::string& notifyEvtName);

			virtual bool read(int offset, char* data, int len);
			virtual bool read(int offset, std::string& data);

			virtual bool write(int offset, const char* data, int len);
			virtual bool write(int offset, const std::string& data);

			virtual void release();

			virtual void onSignal(const std::string& shmName, const std::string& msg) = 0;

		private:
			virtual void action();

			int shmSize_;			
			std::string shmName_;
			bool isInitialOwner_ = false;

#ifdef OS_WIN32
			HANDLE locker_ = NULL;
			HANDLE fileMapping_ = NULL;
			LPVOID viewOfFile_ = NULL;
			HANDLE listenEvt_;
			HANDLE notifyEvt_;
#endif

#ifdef OS_LINUX			
			int semid_;
			int shmid_;
			void *shmData_;
			char *buffer_;
			sem_t *semRead_;
			sem_t *semWrite_;		
#endif // OS_LINUX

		};
	}
}

#endif
