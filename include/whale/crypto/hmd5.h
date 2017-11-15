#ifndef __HMD5_H
#define __HMD5_H

#include <whale/basic/hbytearray.h>

namespace Whale
{
	namespace Crypto
	{
		namespace HMd5
		{
			std::string md5sum(const Whale::Basic::HByteArray& content);
			std::string md5sum(const std::string &fname);
			std::string md5sum(const char *buf, size_t len);
		}
	}
}

#endif // __HMD5_H
