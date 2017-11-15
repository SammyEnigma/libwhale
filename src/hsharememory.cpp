#include "whale/basic/hsharememory.h"
#include "whale/basic/hlog.h"

namespace Whale
{
	namespace Basic
	{
		HShareMemory::HShareMemory()
		{

		}

		HShareMemory::~HShareMemory()
		{
			Whale::Basic::HThread::stop();
			release();
		}

		void HShareMemory::release()
		{
#ifdef OS_WIN32
			if (NULL != listenEvt_) CloseHandle(listenEvt_);
			if (NULL != notifyEvt_) CloseHandle(notifyEvt_);
			if (NULL != viewOfFile_) UnmapViewOfFile(viewOfFile_);
			if (NULL != fileMapping_) CloseHandle(fileMapping_);

			if (NULL != locker_)
			{
				ReleaseMutex(locker_);
				CloseHandle(locker_);
			}
#endif // OS_WIN32

#ifdef OS_LINUX

			if (isInitialOwner_)
			{
				sem_close(semRead_);
				sem_close(semWrite_);
			}

#endif // OS_LINUX

		}

		bool HShareMemory::init(bool isInitialOwner, const std::string& shmName, int smSize, 
			const std::string& lkName, const std::string& listenEvtName, const std::string& notifyEvtName)
		{
			intervalMillSecs(10);
			

			shmSize_ = smSize;
			isInitialOwner_ = isInitialOwner;
			shmName_ = shmName;

#ifdef OS_WIN32

			if (isInitialOwner_)
			{
				locker_ = CreateMutex(NULL, TRUE, lkName.c_str());

				if (NULL == locker_ || ERROR_ALREADY_EXISTS == GetLastError())
				{
					log_error("CreateMutex Fail: %d", GetLastError());
					release();
					return false;
				}

				fileMapping_ = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, smSize, shmName_.c_str());

				if (NULL == fileMapping_)
				{
					log_error("CreateFileMapping Fail: %d", GetLastError());
					release();
					return false;
				}				
			}	
			else
			{
				locker_ = OpenMutex(MUTEX_ALL_ACCESS, FALSE, lkName.c_str());

				if (NULL == locker_ || ERROR_FILE_NOT_FOUND == GetLastError())
				{
					log_error("CreateMutex Fail: %d", GetLastError());
					release();
					return false;
				}

				if (WaitForSingleObject(locker_, 5 * 1000) != WAIT_OBJECT_0)
				{
					log_error("WaitForSingleObject Fail: %d", GetLastError());
					release();
					return false;
				}

				fileMapping_ = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, shmName_.c_str());

				if (NULL == fileMapping_)
				{
					log_error("OpenFileMapping Fail: %d", GetLastError());
					release();
					return false;
				}
			}

			viewOfFile_ = MapViewOfFile(fileMapping_, FILE_MAP_ALL_ACCESS, 0, 0, 0);

			if (NULL == viewOfFile_)
			{
				log_error("MapViewOfFile Fail: %d", GetLastError());
				release();
				return false;
			}	

			memset((char*)viewOfFile_, 0, shmSize_);

			if (NULL != locker_)
			{
				ReleaseMutex(locker_);
			}

			listenEvt_ = CreateEvent(NULL, TRUE, FALSE, listenEvtName.c_str());

			if (NULL == listenEvt_)
			{
				log_error("CreateEvent [%s] Fail: %d", listenEvtName.c_str(), GetLastError());
				return false;
			}

			notifyEvt_ = CreateEvent(NULL, TRUE, FALSE, notifyEvtName.c_str());

			if (NULL == notifyEvt_)
			{
				log_error("CreateEvent [%s] Fail: %d", notifyEvtName.c_str(), GetLastError());
				return false;
			}

#endif

#ifdef OS_LINUX

			if ((semRead_ = sem_open(listenEvtName.c_str(), O_CREAT | O_RDWR , 0666, 0)) == SEM_FAILED)
			{
				log_error("sem_open error: %s.\n", strerror(errno));
				return false;
			}

			if ((semWrite_ = sem_open(notifyEvtName.c_str(), O_CREAT | O_RDWR, 0666, 1)) == SEM_FAILED)
			{
				log_error("sem_open error: %s.\n", strerror(errno));
				return false;
			}			

			key_t shmkey;

			if((shmkey = ftok(shmName_.c_str(), 0)) == -1)
			{
				log_error("ftok error: %s.\n", strerror(errno));
				return false;
			}

			if ((shmid_ = shmget(shmkey/*IPC_PRIVATE*/, shmSize_, isInitialOwner_ ? (IPC_CREAT | SHM_R | SHM_W) : (SHM_R | SHM_W))) == -1)
			{
				log_error("shmget error: %s.\n", strerror(errno));
				return false;
			}

			if ((shmData_ = shmat(shmid_, 0, 0)) == (void *)-1)
			{
				log_error("shmat error: %s.\n", strerror(errno));
				return false;
			}

			buffer_ = (char*)shmData_;

			log_notice("ShareMemory Id: ** %d ** Semaphore Id: ** %d ** ", shmid_, semid_);

#endif // OS_LINUX

			return true;
		}


		void HShareMemory::action()
		{
#ifdef OS_WIN32
			DWORD dwRet = WaitForSingleObject(listenEvt_, INFINITE);

			if (dwRet != WAIT_TIMEOUT && dwRet != WAIT_FAILED)
			{
				if (locker_ != NULL && WaitForSingleObject(locker_, 5 * 1000) == WAIT_OBJECT_0)
				{
					std::string msg = (char*)viewOfFile_;
					ReleaseMutex(locker_);
					onSignal(shmName_, msg);
				}

				ResetEvent(listenEvt_);
			}
#endif // OS_WIN32			

#ifdef OS_LINUX
			sem_wait(semRead_);
			onSignal(shmName_, buffer_);
			sem_post(semWrite_);
#endif // OS_LINUX

		}

		bool HShareMemory::read(int offset, char* data, int len)
		{
#ifdef OS_WIN32
			if (locker_ != NULL && WaitForSingleObject(locker_, 5 * 1000) == WAIT_OBJECT_0)
			{
				if ((offset + len) < shmSize_)
				{
					memcpy(data, &((char*)viewOfFile_)[offset], len);
				}

				return ReleaseMutex(locker_) == TRUE;
			}
#endif // OS_WIN32


#ifdef OS_LINUX
			if ((offset + len) < shmSize_)
			{
				sem_wait(semRead_);
				memcpy(data, &buffer_[offset], shmSize_ - offset);
				sem_post(semWrite_);
			}
#endif // OS_LINUX

			return true;
		}

		bool HShareMemory::read(int offset, std::string& data)
		{
#ifdef OS_WIN32
			if (locker_ != NULL && WaitForSingleObject(locker_, 5 * 1000) == WAIT_OBJECT_0)
			{
				if ((offset) < shmSize_)
				{
					data = &((char*)viewOfFile_)[offset];
				}

				return ReleaseMutex(locker_) == TRUE;
			}
#endif // OS_WIN32

#ifdef OS_LINUX
			if (offset < shmSize_)
			{
				sem_wait(semRead_);
				data = std::string(&buffer_[offset], shmSize_ - offset);
				sem_post(semWrite_);
			}
#endif // OS_LINUX

			return true;
		}

		bool HShareMemory::write(int offset, const char* data, int len)
		{
#ifdef OS_WIN32
			if (locker_ != NULL && WaitForSingleObject(locker_, 5 * 1000) == WAIT_OBJECT_0)
			{
				if ((offset + len) < shmSize_)
				{
					memcpy(&((char*)viewOfFile_)[offset], data, len);
				}	

				SetEvent(notifyEvt_);
				
				return ReleaseMutex(locker_) == TRUE;
			}
#endif // OS_WIN32

#ifdef OS_LINUX
			sem_wait(semWrite_);
			memcpy(buffer_, data, len > shmSize_ ? shmSize_ : len);
			sem_post(semRead_);
#endif // OS_LINUX

			return false;
		}

		bool HShareMemory::write(int offset, const std::string& data)
		{
#ifdef OS_WIN32
			if (locker_ != NULL && WaitForSingleObject(locker_, 5 * 1000) == WAIT_OBJECT_0)
			{
				if ((offset + static_cast<int>(data.length())) < shmSize_)
				{
					memcpy(&((char*)viewOfFile_)[offset], data.c_str(), data.length());
				}

				SetEvent(notifyEvt_);

				return ReleaseMutex(locker_) == TRUE;
			}
#endif // OS_WIN32

#ifdef OS_LINUX
			sem_wait(semWrite_);
			memcpy(buffer_, data.c_str(), data.length() > shmSize_ ? shmSize_ : data.length());
			sem_post(semRead_);
#endif // OS_LINUX

			return false;
		}
	}
}
