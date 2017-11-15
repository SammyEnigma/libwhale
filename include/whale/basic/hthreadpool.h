/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hthreadpool.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HTHREADPOOL_H
#define HTHREADPOOL_H

#include "whale/basic/hdatetime.h"

namespace Whale
{
	namespace Basic
	{
		class HThreadPool
		{
			using Task = std::function<void()>;

		public:
			explicit HThreadPool();
			virtual ~HThreadPool();

			inline size_t maxPoolSize() const { 
				return maxPoolSize_.load();
			}

			inline void closeAssign(){ 
				this->enableAssign_.store(true); 
			}

			inline bool enableAssign() {
				return enableAssign_.load();
			}

			inline void openAssign() { 
				this->enableAssign_.store(false); 
			}

			inline int taskCount() {
				std::lock_guard<std::mutex> lock{ taskLocker_ };
				return taskQueue_.size();
			}

			inline int poolSize() {
				std::lock_guard<std::mutex> lock{ poolLocker_ };
				return threadPool_.size();
			}

			inline bool isIdle() {
				std::lock_guard<std::mutex> lock{ poolLocker_ };

				for (auto& it : threadPool_) {
					if (std::get<1>(it))
						return false;
				}

				return true;
			}

			virtual bool init(size_t maxPoolSize = 1);

			virtual void stop();

			template<class FUNC, class... Args>
			auto assign(FUNC&& func, Args&&... args)->std::future<decltype(func(args...))>;

		private:
			virtual void schedual();
			virtual bool getTask(Task& task);
			virtual void attach();
			virtual void updateStatus(const std::thread::id& id, bool busy);

			std::atomic<size_t> maxPoolSize_;

			std::mutex poolLocker_;
			std::list<std::tuple<std::thread, bool, Whale::Basic::HTimer>> threadPool_;

			std::mutex taskLocker_;
			std::condition_variable conditional_;
			std::queue<Task> taskQueue_;
			
			std::atomic<bool> enableAssign_;
			std::atomic<bool> stoped_;
		};

		template<class FUNC, class... Args>
		auto HThreadPool::assign(FUNC&& func, Args&&... args)->std::future<decltype(func(args...))>{

			if (enableAssign_.load()) {
				throw std::runtime_error("task pool have closed commit.");
			}

			using ResType = decltype(func(args...));

			auto task = std::make_shared<std::packaged_task<ResType()>>(
				std::bind(std::forward<FUNC>(func), std::forward<Args>(args)...));

			{
				std::lock_guard<std::mutex> lock{ taskLocker_ };
				taskQueue_.emplace([task](){(*task)(); });
			}

			conditional_.notify_all();
			std::future<ResType> future = task->get_future();

			return future;
		}
	}
}

#endif
