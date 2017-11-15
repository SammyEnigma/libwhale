/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hlog.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HLOG_H
#define HLOG_H

#include "whale/basic/hplatform.h"
#include "whale/basic/hdatetime.h"
#include "whale/basic/hthread.h"
#include "whale/basic/hthreadsafelist.h"
#include "hlisthandler.h"
#include "whale/basic/hthreadpool.h"

#define	L_EMERG  1 // system is unusable
#define	L_ALERT  2 // action must be taken immediately
#define	L_CRIT   3 // critical conditions
#define	L_ERR    4 // error conditions
#define	L_WARN   5 // warning conditions
#define	L_NOTICE 6 // normal but significant conditions
#define	L_INFO   7 // informational
#define	L_DEBUG  8 // debug-level messages

namespace Whale
{
	namespace Basic
	{
		class HLogItem
		{
		public:
			explicit HLogItem(int level, int line,
							  const std::string& filename,
							  const std::string& datetime,
							  const std::string& location,
							  std::thread::id threadId,
							  const std::string& description);

			virtual ~HLogItem();

			const std::string what() const;

			inline int level() {
				return level_;
			}

			inline int line() {
				return line_;
			}

			inline std::string filename() {
				return fileName_;
			}

			inline std::string datetime() {
				return dateTime_;
			}

			inline std::string location() {
				return location_;
			}

			inline std::string description() {
				return description_;
			}

		private:
			explicit HLogItem() {};

			int line_ = 0;
			int level_ = 0;
			std::thread::id threadId_;

			std::string fileName_ = "";
			std::string dateTime_ = "";
			std::string location_ = "";
			std::string description_ = "";

			bool showLine_ = true;
			bool showLevel_ = true;
			bool showThreadId_ = true;
			bool showDateTime_ = true;
			bool showLocation_ = true;
			bool showFileName_ = true;
			bool showDescription_ = true;
		};

		class HLog
		{
			DECLARE_SINGLETON(HLog)

			public:
			explicit HLog();
			virtual ~HLog();

			virtual bool init(short level,
							  bool outputConsole = true,
							  bool outputFile = false,
							  const std::string &lpath = "",
							  bool rotate = true, 
							  std::function< void(const std::shared_ptr<HLogItem> &)> functional =
							  [](const std::shared_ptr<HLogItem> &)->void{}, int threadPools = 1);

			void level(short level) { level_ = level; }
			
			template<typename... Args>
			bool write(short level, const char *location, const char *fileName,
					   int lineNo, std::thread::id threadId, const char *format, Args... args);

		protected:
			bool outputToFile(const std::string &content);
			bool outputToConsole(const std::string &content);
			bool createLogFile();

		private:
			FILE *fp_;
			std::string fileName_;
			std::string directory_;
			std::string appName_;
			bool rotate_;
			std::function< void(const std::shared_ptr<HLogItem> &)> functional_;


			std::atomic_short level_;
			std::atomic_bool outputConsole_;
			std::atomic_bool outputFile_;
			Whale::Basic::HDateTime timeval_;
			mutable std::mutex locker_;
			std::shared_ptr<Whale::Basic::HThreadPool> threadPool_;			
		};

		template<typename... Args>
		bool HLog::write(short level, const char *location, const char *fileName,
						 int lineNo, std::thread::id threadId, const char *format, Args... args)
		{
			if (!location || !fileName || level > level_) {
				return false;
			}

			std::string content = "NULL";

			if (format)	{
				auto msgLen = snprintf(nullptr, 0, format, args...);
				content.reserve(msgLen + 1);
				content.resize(msgLen);
				snprintf(&content[0], msgLen + 1, format, args...);
			}

			std::shared_ptr<HLogItem> loginfo(
				new HLogItem(level,
							 lineNo,
							 fileName,
							 Whale::Basic::HDateTime().timestamp(),
							 location,
							 threadId,
							 content.c_str()));			

			if (outputConsole_)	{				
				outputToConsole(loginfo->what().c_str());
			}

			threadPool_->assign(
				std::bind([=](const std::shared_ptr<HLogItem> &loginfo) -> bool {

				if (functional_) {
					functional_(loginfo);
				}
	
				if (outputFile_) {
					outputToFile(loginfo->what().c_str());
				}

				return true;

			}, loginfo));
						
			return true;
		}
	}
}

#define log_init				Whale::Basic::HLog::instance()->init
#define log_level(lvl)          Whale::Basic::HLog::instance()->level((lvl))
#define log_emerg(format, ...)	Whale::Basic::HLog::instance()->write(L_EMERG, __FUNCTION__, __FILE__, __LINE__, std::this_thread::get_id(), format, ## __VA_ARGS__);
#define log_alert(format, ...)	Whale::Basic::HLog::instance()->write(L_ALERT, __FUNCTION__, __FILE__, __LINE__, std::this_thread::get_id(), format, ## __VA_ARGS__);
#define log_criti(format, ...)	Whale::Basic::HLog::instance()->write(L_CRIT, __FUNCTION__, __FILE__, __LINE__, std::this_thread::get_id(), format, ## __VA_ARGS__);
#define log_error(format, ...)	Whale::Basic::HLog::instance()->write(L_ERR, __FUNCTION__, __FILE__, __LINE__, std::this_thread::get_id(), format, ## __VA_ARGS__);
#define log_warn(format, ...)	Whale::Basic::HLog::instance()->write(L_WARN, __FUNCTION__, __FILE__, __LINE__, std::this_thread::get_id(), format, ## __VA_ARGS__);
#define log_notice(format, ...) Whale::Basic::HLog::instance()->write(L_NOTICE, __FUNCTION__, __FILE__, __LINE__, std::this_thread::get_id(), format, ## __VA_ARGS__);
#define log_info(format, ...)	Whale::Basic::HLog::instance()->write(L_INFO, __FUNCTION__, __FILE__, __LINE__, std::this_thread::get_id(), format, ## __VA_ARGS__);
#define log_debug(format, ...)	Whale::Basic::HLog::instance()->write(L_DEBUG, __FUNCTION__, __FILE__, __LINE__, std::this_thread::get_id(), format, ## __VA_ARGS__);
#endif
