#ifndef __HSHA_H
#define __HSHA_H

#include "whale/basic/hbytearray.h"

namespace Whale
{
	namespace Crypto
	{
		namespace HSha
		{
			std::string hash(const Whale::Basic::HByteArray& content);
			bool verify(const Whale::Basic::HByteArray& content, const std::string& hashVal);

			std::string hashFile(const std::string& filePath);
			bool verifyFile(const std::string& filePath, const std::string& hashVal);
		}
	}
}

#endif // __HSHA_H
