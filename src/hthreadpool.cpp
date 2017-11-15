#include "whale/basic/hthreadpool.h"

#include "whale/basic/hlog.h"

namespace Whale
{
	namespace Basic
	{
		HThreadPool::HThreadPool() :
			enableAssign_{ false },
			maxPoolSize_{ 1 },
			stoped_{ false }
		{

		}

		HThreadPool::~HThreadPool()
		{
			stop();
		}

		bool HThreadPool::init(size_t maxPoolSize)
		{
			maxPoolSize_.store((maxPoolSize < 1 || maxPoolSize > 64) ? std::thread::hardware_concurrency() : maxPoolSize);

			std::lock_guard<std::mutex> lock{ poolLocker_ };

			for (size_t i = 0; i < maxPoolSize_.load(); ++i)
			{
				threadPool_.emplace_back(std::thread(&HThreadPool::schedual, this), false, Whale::Basic::HTimer());
			}

			return true;
		}

		void HThreadPool::stop()
		{
			while (taskCount() > 0) 
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
			};

			stoped_.store(true);

			conditional_.notify_all();

			std::lock_guard<std::mutex> lock{ poolLocker_ };

			for (auto& it : threadPool_)
			{
				std::get<0>(it).join();
			}

			threadPool_.clear();
		}

		void HThreadPool::updateStatus(const std::thread::id& id, bool busy)
		{
			std::lock_guard<std::mutex> lock{ poolLocker_ };

			for (auto& it : threadPool_)
			{
				if (std::get<0>(it).get_id() == id)
				{		
					std::get<1>(it) = busy;
					std::get<2>(it).reset();					
				}
			}
		}

		void HThreadPool::attach()
		{
			std::lock_guard<std::mutex> lock{ poolLocker_ };

			if (threadPool_.size() < maxPoolSize_.load())
			{
				threadPool_.emplace_back(std::thread(&HThreadPool::schedual, this), false, Whale::Basic::HTimer());
			}
		}

		bool HThreadPool::getTask(Task& task)
		{
			std::unique_lock<std::mutex> lock{ taskLocker_ };

			conditional_.wait(lock, [this]() {
				return !taskQueue_.empty() || stoped_.load();
			});

			if (!stoped_.load())
			{
				task = std::move(taskQueue_.front());
				taskQueue_.pop();
				updateStatus(std::this_thread::get_id(), true);
				return true;
			}

			return false;
		}

		void HThreadPool::schedual()
		{
			while (!stoped_.load())
			{
				Task task;

				if (getTask(task))
				{
					task();
					updateStatus(std::this_thread::get_id(), false);
				}
				else
				{
					std::this_thread::sleep_for(std::chrono::seconds(1));
				}
			}
		}
	}
}