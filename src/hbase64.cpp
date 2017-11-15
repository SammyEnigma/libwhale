#include "whale/crypto/hbase64.h"

namespace Whale
{
	namespace Crypto
	{
		namespace HBase64
		{
			static const std::string base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

			static inline bool isBase64(unsigned char c)
			{
				return (isalnum(c) || (c == '+') || (c == '/'));
			}

			std::string encode(const Whale::Basic::HByteArray& plain)
			{
				int i = 0;
				int j = 0;
				unsigned char char_array_3[3];
				unsigned char char_array_4[4];

				int index = 0;
				int size = plain.size();
				std::string cipher = "";

				while (size--)
				{
					char_array_3[i++] = plain[index++];

					if (i == 3)
					{
						char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
						char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
						char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
						char_array_4[3] = char_array_3[2] & 0x3f;

						for (i = 0; (i < 4); i++)
						{
							cipher += base64[char_array_4[i]];
						}

						i = 0;
					}
				}

				if (i)
				{
					for (j = i; j < 3; j++)
					{
						char_array_3[j] = '\0';
					}

					char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
					char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
					char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
					char_array_4[3] = char_array_3[2] & 0x3f;

					for (j = 0; (j < i + 1); j++)
					{
						cipher += base64[char_array_4[j]];
					}

					while ((i++ < 3))
					{
						cipher += '=';
					}
				}

				return cipher;
			}

			Whale::Basic::HByteArray decode(const std::string& cipher)
			{
				int i = 0;
				int j = 0;
				int in_ = 0;
				unsigned char char_array_4[4];
				unsigned char char_array_3[3];
				int in_len = cipher.size();
				Whale::Basic::HByteArray plain;

				while (in_len-- && (cipher[in_] != '=') && isBase64(cipher[in_]))
				{
					char_array_4[i++] = cipher[in_]; in_++;
					if (i == 4)
					{
						for (i = 0; i < 4; i++)
						{
							char_array_4[i] = static_cast<unsigned char>(base64.find(char_array_4[i]));
						}

						char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
						char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
						char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

						for (i = 0; (i < 3); i++)
						{
							plain.push_back(char_array_3[i]);
						}

						i = 0;
					}
				}

				if (i)
				{
					for (j = i; j < 4; j++)
					{
						char_array_4[j] = 0;
					}

					for (j = 0; j < 4; j++)
					{
						char_array_4[j] = static_cast<unsigned char>(base64.find(char_array_4[j]));
					}

					char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
					char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
					char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

					for (j = 0; (j < i - 1); j++)
					{
						plain.push_back(char_array_3[j]);
					}
				}

				return plain;
			}
		}
	}
}