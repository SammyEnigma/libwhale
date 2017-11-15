#include "whale/net/hhttpclient.h"

#include "whale/basic/hos.h"
#include "whale/util/hstring.h"
#include "whale/basic/hos.h"

namespace Whale
{
	namespace Net
	{
		HHttpClient::HHttpClient(void)
		{
			currentPath_ = Whale::currentDir();
		}

		HHttpClient::~HHttpClient(void)
		{
		}

		std::string HHttpClient::get(
			const std::string& url,
			const std::string& params,
			const std::string& charset,
			const std::string& user,
			const std::string& passwd,
			int millSeconds) {
			std::string auth = "";

			if (!user.empty()) {
				auth = passwd.empty() ?
					Whale::Util::HString::format("-u %s", user.c_str()) :
					Whale::Util::HString::format("-u %s:%s", user.c_str(), passwd.c_str());
			}

			std::string command = Whale::Util::HString::format(
				"%scurl %s -H \"Content-Type:application/json;charset=%s\" "\
				"-X GET --data \"%s\" \"%s\" --connect-timeout %d",
				currentPath_.c_str(),
				auth.c_str(),
#ifdef OS_WIN32
				"GBK",
#else
				"UTF-8",
#endif
				Whale::Util::HString::replace(params, "\"", "\\\"").c_str(),
				url.c_str(),
				millSeconds);

			log_info("%s", command.c_str());

#ifdef OS_WIN32
			if (Whale::Util::HString::toUpper(charset).compare("UTF-8") == 0) {
				return Whale::Util::HString::utf8ToString(Whale::process("%s", command.c_str()));
			}
#else
			if (Whale::Util::HString::toUpper(charset).compare("GBK") == 0) {
				return Whale::Util::HString::stringToUTF8(Whale::process("%s",
					Whale::Util::HString::utf8ToString(command).c_str()));
			}
#endif

			return Whale::process("%s", command.c_str());
		}

		std::string HHttpClient::post(
			const std::string& url,
			const std::string& params,
			const std::string& charset,
			const std::string& user,
			const std::string& passwd,
			int millSeconds) {
			std::string auth = "";

			if (!user.empty()) {
				auth = passwd.empty() ?
					Whale::Util::HString::format("-u %s", user.c_str()) :
					Whale::Util::HString::format("-u %s:%s", user.c_str(), passwd.c_str());
			}

			std::string command = Whale::Util::HString::format(
				"%scurl %s -H \"Content-Type:application/json;charset=%s\" "\
				"-X POST --data \"%s\" \"%s\" --connect-timeout %d",
				currentPath_.c_str(),
				auth.c_str(), 
#ifdef OS_WIN32
				"GBK",
#else
				"UTF-8",
#endif
				Whale::Util::HString::replace(params, "\"", "\\\"").c_str(),
				url.c_str(), 
				millSeconds);

			log_info("%s", command.c_str());

#ifdef OS_WIN32
			if (Whale::Util::HString::toUpper(charset).compare("UTF-8") == 0) {
				return Whale::Util::HString::utf8ToString(Whale::process("%s", command.c_str()));
			}
#else
			if (Whale::Util::HString::toUpper(charset).compare("GBK") == 0) {
				return Whale::Util::HString::stringToUTF8(Whale::process("%s", 
					Whale::Util::HString::utf8ToString(command).c_str()));
			}
#endif // OS_WIN32

			return Whale::process("%s", command.c_str());
		}

		std::string HHttpClient::upload(
			const std::string& url,
			const std::string& filePath,
			const std::string& charset,
			const std::string& user,
			const std::string& passwd,
			int millSeconds)
		{
			std::string auth = "";

			if (!user.empty()) {
				auth = passwd.empty() ?
					Whale::Util::HString::format("-u %s", user.c_str()) :
					Whale::Util::HString::format("-u %s:%s", user.c_str(), passwd.c_str());
			}

			std::string command = Whale::Util::HString::format(
				"%scurl %s -T \"%s\" \"%s\" --connect-timeout %d",
				currentPath_.c_str(), auth.c_str(), filePath.c_str(), url.c_str(), millSeconds);

			log_info("%s", command.c_str());

#ifdef OS_WIN32
			if (Whale::Util::HString::toUpper(charset).compare("UTF-8") == 0) {
				return Whale::Util::HString::utf8ToString(Whale::process("%s", command.c_str()));
			}
#else
			if (Whale::Util::HString::toUpper(charset).compare("GBK") == 0) {
				return Whale::Util::HString::stringToUTF8(Whale::process("%s", command.c_str()));
			}
#endif // OS_WIN32

			return Whale::process("%s", command.c_str());
		}

		int HHttpClient::download(
			const std::string& url,
			const std::string& filePath,
			const std::string& charset,
			const std::string& user,
			const std::string& passwd,
			int millSeconds)
		{
			std::string auth = "";

			if (!user.empty()) {
				auth = passwd.empty() ?
					Whale::Util::HString::format("-u %s", user.c_str()) :
					Whale::Util::HString::format("-u %s:%s", user.c_str(), passwd.c_str());
			}

			std::string command = Whale::Util::HString::format(
				"%scurl %s -o \"%s\" \"%s\" --connect-timeout %d",
				currentPath_.c_str(), auth.c_str(), filePath.c_str(), url.c_str(), millSeconds);

			log_info("%s", command.c_str());

			return system(command.c_str());
		}
	}
}
