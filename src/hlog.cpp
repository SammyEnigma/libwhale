#include "whale/basic/hos.h"
#include "whale/basic/hlog.h"
#include "whale/util/hdir.h"
#include "whale/util/hstring.h"

namespace Whale
{
	namespace Basic
	{
		const static char *LEVEL_TEXT[] = {
			"- NULL  -", "- EMERG -", "- ALERT -",
			"- CRIT  -", "- ERROR -", "- WARN  -",
			"- NOTIC -", "- INFO  -", "- DEBUG -"
		};

		HLogItem::HLogItem(int level, int line, const std::string& filename, const std::string& datetime,
			const std::string& location, std::thread::id threadId, const std::string& description)
		{
			line_ = line;
			level_ = level;
			dateTime_ = datetime;
			fileName_ = filename;
			location_ = location;
			threadId_ = threadId;
			description_ = description;
		}

		HLogItem::~HLogItem()
		{
		}

		const std::string HLogItem::what() const
		{
			std::string logContent = "";

			if (showDateTime_)	{
				logContent += dateTime_ + std::string(" ");
			}

			if (showLevel_)	{
				logContent += LEVEL_TEXT[level_] + std::string(" ");
			}

			if (showFileName_)	{
#ifdef OS_WIN32
				char drivename[8] = { 0 };
				char directory[MAX_PATH] = { 0 };
				char filename[MAX_PATH] = { 0 };
				char extername[16] = { 0 };

				_splitpath(fileName_.c_str(), drivename, directory, filename, extername);
				logContent += std::string(filename) + std::string(extername);
#else
				logContent += basename(const_cast<char*>(fileName_.c_str()));
#endif

				if (showLine_) {
					logContent += Whale::Util::HString::format("(%d)", line_);
				}

				logContent += ": ";
			}

			if (showLocation_) {
#ifdef OS_WIN32
				logContent += Whale::Util::HString::format("%s(%ld) - ", location_.c_str(), threadId_);
#else
				logContent += Whale::Util::HString::format("%s(%llu) - ", location_.c_str(), threadId_);
#endif				
			}

			if (showDescription_) {
				logContent += description_;
			}

			logContent += "\n";

			return logContent;
		}

		IMPLEMENT_SINGLETON(HLog)

		HLog::HLog() : threadPool_(new Whale::Basic::HThreadPool()) {
			fp_ = NULL;
			level_ = L_DEBUG;

			outputFile_ = false;
			outputConsole_ = true;
			rotate_ = true;

			appName_ = Whale::appName();			
			functional_ = nullptr;
			
			try	{
				//run();
			} catch (...) {
				exit(-1);
			}
		}

		HLog::~HLog()
		{
			threadPool_->stop();
			//stop();			
		}

		bool HLog::outputToConsole(const std::string &content)
		{
			if (!outputConsole_) {
				return false;
			}

			std::string __cnt = content;
			__cnt = "*** " + appName_ + " *** " + __cnt;

			printf(__cnt.c_str());

#ifdef OS_WIN32
			OutputDebugString(__cnt.c_str());
#else
			syslog(LOG_INFO | LOG_USER, "%s", __cnt.c_str());
#endif

			return true;
		}

		bool HLog::outputToFile(const std::string &content)
		{
			std::lock_guard <std::mutex> lock(locker_);

			if (!createLogFile()) {
				return false;
			}

			fprintf(fp_, "%s", content.c_str());
			fflush(fp_); fclose(fp_);
			
			return true;
		}

		bool HLog::createLogFile()
		{
			if (directory_.empty()) {
				directory_ = Whale::Util::HString::format("%slog", Whale::currentDir().c_str());
			}
			
			if (access(directory_.c_str(), 0) != 0)	{
				if (!Whale::Util::HDir::mkdir(directory_)) {
					return false;
				}
			}

			if (rotate_) {
				timeval_ = Whale::Basic::HDateTime();;
				fileName_ = Whale::Util::HString::format("%s_%s.log", appName_.c_str(), timeval_.timestamp("%Y%m%d").c_str());
			} else {
				fileName_ = appName_ + ".log";
			}

			fp_ = fopen(Whale::Util::HString::format("%s/%s", directory_.c_str(), fileName_.c_str()).c_str(), "at");

			return fp_ != NULL;
		}

		bool HLog::init(short level,
			bool outputConsole,
			bool outputFile,
			const std::string &lpath,
			bool rotate,
			std::function< void(const std::shared_ptr<HLogItem> &)> functional, 
			int threadPools)
		{
			level_ = level;
			outputConsole_ = outputConsole;
			outputFile_ = outputFile;
			directory_ = lpath;
			rotate_ = rotate;
			functional_ = functional;

			return threadPool_->init(threadPools);
		}
	}
}
