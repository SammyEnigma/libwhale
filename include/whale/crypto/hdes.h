#include <whale/basic/hbytearray.h>

namespace Whale
{
	namespace Crypto
	{
		namespace HDes
		{
			Whale::Basic::HByteArray encode(const Whale::Basic::HByteArray& plain, const Whale::Basic::HByteArray& key);
			Whale::Basic::HByteArray decode(const Whale::Basic::HByteArray& cipher, const Whale::Basic::HByteArray& key);
			bool encodeFile(const std::string& plainFile, const std::string& keyFile, const std::string& cipherFile);
			bool decodeFile(const std::string& cipherFile, const std::string& keyFile, const std::string& plainFile);

			Whale::Basic::HByteArray des3Encode(const Whale::Basic::HByteArray& plain, const Whale::Basic::HByteArray& key);
			Whale::Basic::HByteArray des3Decode(const Whale::Basic::HByteArray& cipher, const Whale::Basic::HByteArray& key);
			bool des3EncodeFile(const std::string& plainFile, const std::string& keyFile, const std::string& cipherFile);
			bool des3DecodeFile(const std::string& cipherFile, const std::string& keyFile, const std::string& plainFile);
		}
	}
}