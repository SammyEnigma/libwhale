#include "whale/basic/hloopbuffer.h"

namespace Whale
{
	namespace Basic
	{
		HLoopBuffer::HLoopBuffer()
		{
			writePos_ = 0;
			readPos_ = 0;
			count_ = 0;
			size_ = 0;
			buffer_ = nullptr;
			initialized_ = false;
		}

		HLoopBuffer::~HLoopBuffer()
		{
			std::lock_guard <std::mutex> lock(locker_);

			if (buffer_ != 0 || buffer_ != nullptr)
			{
				delete[] buffer_;
				buffer_ = nullptr;
			}
		}

		bool HLoopBuffer::init(int maxmium)
		{
			std::lock_guard <std::mutex> lock(locker_);

			if (!initialized_ && maxmium >= 0)
			{
				size_ = maxmium;
				buffer_ = new char[size_];
				initialized_ = true;
			}

			return initialized_;
		}

		int HLoopBuffer::idle()
		{
			std::lock_guard <std::mutex> lock(locker_);
			return size_ - count_;
		}

		int HLoopBuffer::size()
		{
			std::lock_guard <std::mutex> lock(locker_);
			return size_;
		}

		int HLoopBuffer::count()
		{
			std::lock_guard <std::mutex> lock(locker_);
			return count_;
		}	

		int HLoopBuffer::read(char* data, int len)
		{
			std::lock_guard <std::mutex> lock(locker_);

			len = count_ >= len ? len : count_;

			if (len <= 0)
			{
				return 0;
			}

			int readpos = readPos_;
			int writepos = writePos_;
			int size = count_;


			if (readPos_ > writePos_)
			{
				if (size_ - readPos_ > len)
				{
					memcpy(data, &buffer_[readPos_], len);
					readPos_ += len;
				}
				else
				{
					memcpy(data, &buffer_[readPos_], size_ - readPos_);
					memcpy(&data[size_ - readPos_], &buffer_[0], (len - (size_ - readPos_)));
					readPos_ = len - (size_ - readPos_);
				}
			}
			else if (readPos_ < writePos_)
			{
				memcpy(data, &buffer_[readPos_], len);
				readPos_ += len;
			}

			count_ -= len;

			return len;
		}

		int HLoopBuffer::copy(char* data, int len)
		{
			std::lock_guard <std::mutex> lock(locker_);

			len = count_ >= len ? len : count_;

			if (len <= 0)
			{
				return 0;
			}

			int readpos = readPos_;
			int writepos = writePos_;
			int size = count_;


			if (readPos_ > writePos_)
			{
				if (size_ - readPos_ > len)
				{
					memcpy(data, &buffer_[readPos_], len);
					readPos_ += len;
				}
				else
				{
					memcpy(data, &buffer_[readPos_], size_ - readPos_);
					memcpy(&data[size_ - readPos_], &buffer_[0], (len - (size_ - readPos_)));
					readPos_ = len - (size_ - readPos_);
				}
			}
			else if (readPos_ < writePos_)
			{
				memcpy(data, &buffer_[readPos_], len);
				readPos_ += len;
			}

			count_ -= len;

			readPos_ = readpos;
			writePos_ = writepos;
			count_ = size;

			return len;
		}

		unsigned char HLoopBuffer::operator [] (int index)
		{
			std::lock_guard <std::mutex> lock(locker_);
			char ch = 0x00;

			if (readPos_ + index >= size_)
			{
				ch = buffer_[index - (size_ - readPos_)];
			}
			else
			{
				ch = buffer_[readPos_ + index];
			}

			return ch;
		}


		int HLoopBuffer::write(const char* data, int len)
		{
			std::lock_guard <std::mutex> lock(locker_);

			int size = 0;
			size = (size_ - count_) >= len ? len : 0;

			if (size <= 0)
			{
				return 0;
			}

			if (readPos_ > writePos_)
			{
				memcpy(&buffer_[writePos_], data, size);
				writePos_ += size;
			}
			else if (readPos_ <= writePos_)
			{
				if (size_ - writePos_ > size)
				{
					memcpy(&buffer_[writePos_], data, size);
					writePos_ += size;
				}
				else
				{
					memcpy(&buffer_[writePos_], data, size_ - writePos_);
					memcpy(buffer_, &data[size_ - writePos_], size - (size_ - writePos_));
					writePos_ = size - (size_ - writePos_);
				}
			}

			count_ += size;

			return size;
		}

		bool HLoopBuffer::empty()
		{
			std::lock_guard <std::mutex> lock(locker_);
			return count_ <= 0 ? true : false;
		}

		void HLoopBuffer::clear()
		{
			std::lock_guard <std::mutex> lock(locker_);
			writePos_ = 0;
			readPos_ = 0;
			count_ = 0;
		}
	}
}