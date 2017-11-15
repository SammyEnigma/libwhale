#include "whale/basic/hlog.h"
#include "whale/basic/hthread.h"

#include "whale/basic/hos.h"

namespace Whale
{
	namespace Basic
	{
		void HThread::work(void *param)
		{
			HThread *pthread = (HThread *)param;

			if (pthread) {
				pthread->routine();
			}
		}

		void HThread::routine()
		{
			beforeRun();
			
			while (working_)
			{
				if (quitEvent_.wait(interval_)) {
					// log_debug("%s: quitEvent wait", name_.c_str());
				}

				for (auto i = 0; i < missions_ && working_; ++i) {
					action();
				}

				if (singleRecyle_) {
					working_ = false;
					break;
				}		
			}

			afterRun();

			// log_debug("%s: confirmEvent post", name_.c_str());
			confirmEvent_.post();
		}

		HThread::HThread()
		{
			thread_ = nullptr;
			pvoid_ = nullptr;
			working_ = false;
		}

		HThread::~HThread()
		{
			stop();
		}

		bool HThread::run()
		{
			working_ = true;
			thread_ = new std::thread(work, this);
			return working_;
		}

		void HThread::stop()
		{
			if (!working_) {
				return;
			}

			working_ = false;
			
			// do {
			quitEvent_.post();
			confirmEvent_.wait(3000);
			// } while (!_confirmEvent.wait(1000));
			// log_debug("%s: thread stop", name_.c_str());
			
			if (thread_) {
				//_thread->join();
				thread_->detach();

				delete thread_;
				thread_ = nullptr;
			}
		}
	}
}
