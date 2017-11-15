#include "whale/crypto/hbcd.h"

namespace Whale
{
	namespace Crypto
	{
		namespace HBcd
		{
			int decode(unsigned char *in, int size, unsigned char *out, int maxmium)
			{
				assert(in);
				assert(out);
				assert(size > 0);
				assert(maxmium > size * 2);

				for (int i = 0; i < size; i++)
				{
					out[2 * i] = ((unsigned char)(in[i] >> 4) + 0x30) > '9' ? ((unsigned char)(in[i] >> 4) + 0x30 + 7) : ((unsigned char)(in[i] >> 4) + 0x30);
					out[2 * i + 1] = (in[i] & 0x0F + 0x30) > '9' ? (in[i] & 0x0F + 0x30 + 7) : (in[i] & 0x0F + 0x30);
				}

				return size * 2;
			};

			int encode(unsigned char *in, int size, unsigned char *out, int maxmium)
			{
				assert(in);
				assert(out);
				assert(size > 0);
				assert(maxmium > size / 2);

				size = size / 2;

				for (int i = 0; i < size; i++)
				{
					unsigned char tmp;
					if (in[2 * i] >= '0' && in[2 * i] <= '9') tmp = in[2 * i] - 0x30;
					if (in[2 * i] >= 'A' && in[2 * i] <= 'F')  tmp = in[2 * i] - ('A' - 10);
					if (in[2 * i] >= 'a' && in[2 * i] <= 'f')  tmp = in[2 * i] - ('a' - 10);

					out[i] = tmp;
					if (in[2 * i + 1] >= '0' && in[2 * i + 1] <= '9') tmp = in[2 * i + 1] - 0x30;
					if (in[2 * i + 1] >= 'A' && in[2 * i + 1] <= 'F')  tmp = in[2 * i + 1] - ('A' - 10);
					if (in[2 * i + 1] >= 'a' && in[2 * i + 1] <= 'f')  tmp = in[2 * i + 1] - ('a' - 10);

					out[i] = out[i] << 4 | tmp;
				}

				return size;
			};
		};
	}
}