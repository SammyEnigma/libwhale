#ifndef __HAES_H
#define __HAES_H

#include <whale/basic/hbytearray.h>

namespace Whale
{
	namespace Crypto
	{
		namespace HAes 
		{
			const static int ECB = 0;
			const static int CBC = 1;
			const static int CFB = 2;

			Whale::Basic::HByteArray encode(
				const Whale::Basic::HByteArray& plain,
				const Whale::Basic::HByteArray& key,
				const Whale::Basic::HByteArray& chain = Whale::Basic::HByteArray("0123456789012345"),
				int mode = Whale::Crypto::HAes::ECB);

			Whale::Basic::HByteArray decode(
				const Whale::Basic::HByteArray& cipher,
				const Whale::Basic::HByteArray& key,
				const Whale::Basic::HByteArray& chain = Whale::Basic::HByteArray("0123456789012345"),
				int mode = Whale::Crypto::HAes::ECB);
		}
	}
}

#endif // __HAES_H

