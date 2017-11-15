#include "whale/crypto/hzip.h"
#include "zlib.h"

#ifdef _MSC_VER
#pragma comment(lib, "Z:/libwhale/deps/zlib-1.2.11/lib/zlib.lib")
#endif

namespace Whale
{
	namespace Crypto
	{
		namespace HZip
		{

			void *myalloc(void * q, unsigned n, unsigned m)
			{
				(void)q;
				return calloc(n, m);
			}

			void myfree(void *q, void *p)
			{
				(void)q;
				free(p);
			}


			Whale::Basic::HByteArray deflate(const Whale::Basic::HByteArray& content)
			{
				if (content.empty()) {
					return content;
				}

				int err;

				uLong comprLen = 1024 * 1024 * sizeof(int);
				Byte *compr = (Byte*)calloc((uInt)comprLen, 1);

				err = compress(compr, &comprLen, (const Bytef*)content.data(), content.size());

				if (err != Z_OK) {
					return Whale::Basic::HByteArray();
				}

				Whale::Basic::HByteArray comp((const char*)compr, (size_t)comprLen);

				free(compr);

				return comp;

				

				//z_stream c_stream; /* compression stream */
				//int err;

				//uLong comprLen = 1024 * 1024 * sizeof(int);
				//Byte *compr = (Byte*)calloc((uInt)comprLen, 1);

				//c_stream.zalloc = myalloc;
				//c_stream.zfree = myfree;
				//c_stream.opaque = (voidpf)0;

				//err = deflateInit(&c_stream, Z_BEST_SPEED);

				//c_stream.next_out = compr;
				//c_stream.avail_out = (uInt)comprLen;

				///* At this point, uncompr is still mostly zeroes, so it should compress
				//* very well:
				//*/
				//c_stream.next_in = (Bytef*)content.data();
				//c_stream.avail_in = (uInt)content.size();
				//err = deflate(&c_stream, Z_NO_FLUSH);
				//if (c_stream.avail_in != 0) {
				//	fprintf(stderr, "deflate not greedy\n");
				//	exit(1);
				//}

				///* Feed in already compressed data and switch to no compression: */
				//deflateParams(&c_stream, Z_NO_COMPRESSION, Z_DEFAULT_STRATEGY);
				//c_stream.next_in = compr;
				//c_stream.avail_in = (uInt)comprLen / 2;
				//err = deflate(&c_stream, Z_NO_FLUSH);

				///* Switch back to compressing mode: */
				//deflateParams(&c_stream, Z_BEST_COMPRESSION, Z_FILTERED);
				//c_stream.next_in = (Bytef*)content.data();
				//c_stream.avail_in = (uInt)content.size();
				//err = deflate(&c_stream, Z_NO_FLUSH);

				//err = deflate(&c_stream, Z_FINISH);
				//if (err != Z_STREAM_END) {
				//	fprintf(stderr, "deflate should report Z_STREAM_END\n");
				//	exit(1);
				//}
				//err = deflateEnd(&c_stream);

				//Whale::Basic::HByteArray comp((const char*)compr, (size_t)comprLen);

				//free(compr);

				//return comp;
			}

			Whale::Basic::HByteArray inflate(const Whale::Basic::HByteArray& content)
			{
				if (content.empty()) {
					return content;
				}

				int err;

				uLong comprLen = 1024 * 1024 * sizeof(int);
				Byte *compr = (Byte*)calloc((uInt)comprLen, 1);

				strcpy((char*)compr, "garbage");
				err = uncompress(compr, &comprLen, (const Bytef*)content.data(), content.size());

				if (err != Z_OK) {
					return Whale::Basic::HByteArray();
				}

				Whale::Basic::HByteArray comp((const char*)compr, (size_t)comprLen);

				free(compr);

				return comp;

				//int err;
				//uLong comprLen = 1024 * 1024 * sizeof(int);
				//Byte *compr = (Byte*)calloc((uInt)comprLen, 1);
				//z_stream d_stream; /* decompression stream */

				//strcpy((char*)compr, "garbage");

				//d_stream.zalloc = myalloc;
				//d_stream.zfree = myfree;
				//d_stream.opaque = (voidpf)0;

				//d_stream.next_in = (Bytef*)content.data();
				//d_stream.avail_in = (uInt)content.size();

				//err = inflateInit(&d_stream);

				//for (;;) {
				//	d_stream.next_out = compr;            /* discard the output */
				//	d_stream.avail_out = comprLen;
				//	err = inflate(&d_stream, Z_NO_FLUSH);
				//	if (err == Z_STREAM_END) break;
				//}

				//err = inflateEnd(&d_stream);

				//Whale::Basic::HByteArray comp((const char*)compr, (size_t)comprLen);

				//free(compr);

				//return comp;
			}
		}
	}
}