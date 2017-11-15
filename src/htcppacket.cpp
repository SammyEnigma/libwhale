#include "whale/net/htcppacket.h"
#include "whale/basic/hos.h"
#include "whale/util/hrandom.h"
#include "whale/crypto/hcrc.h"
#include "whale/crypto/haes.h"
#include "whale/crypto/hzip.h"
#include "whale/util/hstring.h"

namespace Whale
{
	namespace Net
	{
		HTcpPacket::HTcpPacket()
		{
			head_ = 0xA7;							// 协议头部标识
			version_ = 0x00;						// 协议版本号
			vender_ = 0x0000;						// 厂商代码
			user_ = 0x00000000;						// 用户代码
			type_ = 0x00;							// 数据类型 0x00 普通记录 | 0x01 数据转发 | 0x02 数据广播
			from_ = 0x00000000;						// 原始地址
			to_ = 0x00000000;						// 目标地址
			direction_ = 0x00;						// 0x00 数据请求 | 0x01 数据响应
			priority_ = 0x00;						// 0x00 普通处理 | 0x01 优先处理
			charset_ = 0x00;						// 0x00 UTF-8编码 | 0x01 GBK编码
			number_ = Whale::Util::HRandom::gen();	// 数据包随机编号，请求响应一一对应
			compress_ = 0x00;						// 是否压缩 0x00 否 | 0x01 是
			encoded_ = 0x00;						// 是否加密 0x00 否 | 0x01 是
			memset(reserve_, 0, sizeof(reserve_));							// 保留字段
			dataLen_ = 0x00000000;					// 数据长度
			crc16_ =0x0000;							// 数据部分的CRC16校验值
			tail_ = 0x7A;							// 协议尾部标识

			lenPos_ = 0;
			lenPos_ += sizeof(head_);
			lenPos_ += sizeof(version_);
			lenPos_ += sizeof(vender_);
			lenPos_ += sizeof(user_);
			lenPos_ += sizeof(type_);
			lenPos_ += sizeof(from_);
			lenPos_ += sizeof(to_);
			lenPos_ += sizeof(direction_);
			lenPos_ += sizeof(priority_);
			lenPos_ += sizeof(charset_);
			lenPos_ += sizeof(number_);
			lenPos_ += sizeof(compress_);
			lenPos_ += sizeof(encoded_);
			lenPos_ += sizeof(reserve_);

			totalSize_ = 0;
			totalSize_ += sizeof(head_);
			totalSize_ += sizeof(version_);
			totalSize_ += sizeof(vender_);
			totalSize_ += sizeof(user_);
			totalSize_ += sizeof(type_);
			totalSize_ += sizeof(from_);
			totalSize_ += sizeof(to_);
			totalSize_ += sizeof(direction_);
			totalSize_ += sizeof(priority_);
			totalSize_ += sizeof(charset_);
			totalSize_ += sizeof(number_);
			totalSize_ += sizeof(dataLen_);
			totalSize_ += sizeof(crc16_);
			totalSize_ += sizeof(tail_);
			totalSize_ += sizeof(compress_);
			totalSize_ += sizeof(encoded_);
			totalSize_ += sizeof(reserve_);

			maxmium_ = 1024 * 512;
			first_ = false;
		}

		HTcpPacket::~HTcpPacket()
		{

		}

		uint32_t HTcpPacket::decode(Whale::Basic::HByteArray& cipher)
		{			
			while (true) {
				// 检测数据包是否达到协议总长度
				if (cipher.size() < totalSize_) {
					return 0;
				}

				// 校验数据包包头
				if (static_cast<uint8_t>(cipher[0]) != head_) {
					cipher.read(1); continue; 
				}

				// 获取数据包数据部分长度
				uint32_t dataLen = Whale::htonl_(cipher.copy<decltype(dataLen)>(lenPos_));

				// 校验数据包是否超过系统允许包的最大长度
				if (dataLen + totalSize_ > maxmium_) { 
					cipher.read(1); continue;
				}

				// 校验数据包是否接收完毕
				if (cipher.size() < dataLen + totalSize_) { 
					return 0; 
				}

				// 数据部分CRC16校验
				if (Whale::htons_(cipher.copy<decltype(crc16_)>(dataLen + totalSize_ - sizeof(tail_) - sizeof(crc16_)))
					!= Whale::Crypto::HCrc::calcu16((uint8_t*)&cipher[sizeof(head_)],
						dataLen + totalSize_ - sizeof(tail_) - sizeof(crc16_) - sizeof(head_))) {
					cipher.read(1); continue;
				}

				// 校验数据包包尾
				if (static_cast<uint8_t>(cipher[dataLen + totalSize_ - sizeof(tail_)]) != tail_) { 
					cipher.read(1); continue; 
				}

				/*head_ = */cipher.read<decltype(head_)>();
				version_	= cipher.read<decltype(version_)>();
				vender_		= Whale::htons_(cipher.read<decltype(vender_)>());
				user_		= Whale::htonl_(cipher.read<decltype(user_)>());
				type_		= cipher.read<decltype(type_)>();
				from_		= Whale::htonl_(cipher.read<decltype(from_)>());
				to_			= Whale::htonl_(cipher.read<decltype(to_)>());
				direction_	= cipher.read<decltype(direction_)>();
				priority_	= cipher.read<decltype(priority_)>();
				charset_	= cipher.read<decltype(charset_)>();
				number_		= Whale::htonl_(cipher.read<decltype(number_)>());
				compress_	= cipher.read<decltype(compress_)>();
				encoded_	= cipher.read<decltype(encoded_)>();
				/*reserve_	= */cipher.read(sizeof(reserve_));
				dataLen		= Whale::htonl_(cipher.read<decltype(dataLen)>());	

				buffer_ = cipher.read(dataLen);				

				buffer_ = encoded_ ? Whale::Crypto::HAes::decode(buffer_,
					Whale::Basic::HByteArray("78c91ba01a3a5a37")) : buffer_;

				buffer_ = compress_ ? Whale::Crypto::HZip::inflate(buffer_) : buffer_;
				
				crc16_		= Whale::htons_(cipher.read<decltype(crc16_)>());

				/*tail_ = */cipher.read<decltype(tail_)>();			

#ifdef OS_WIN32
				// WINDOWS平台，数据编码为UTF-8需要转为GBK编码
				if (charset_ == 0x00) {
					buffer_ = Whale::Util::HString::convert("utf-8", "gb2312", buffer_);
				}
#else
				// LINUX平台，数据编码为GBK需要转为UTF-8编码
				if (charset_ == 0x01) {
					buffer_ = Whale::Util::HString::convert("gb2312", "utf-8", buffer_);
				}
#endif // OS_WIN32

				return dataLen + totalSize_;
			}			
		}

		Whale::Basic::HByteArray HTcpPacket::encode()
		{
			Whale::Basic::HByteArray cipher;

#ifdef OS_WIN32
			charset_ = 0x01;						// 0x00 UTF-8编码 | 0x01 GBK编码
#else
			charset_ = 0x00;						// 0x00 UTF-8编码 | 0x01 GBK编码
#endif

			cipher.write<decltype(head_)>(head_);
			cipher.write<decltype(version_)>(version_);
			cipher.write<decltype(vender_)>(Whale::htons_(vender_));
			cipher.write<decltype(user_)>(Whale::htonl_(user_));
			cipher.write<decltype(type_)>(type_);
			cipher.write<decltype(from_)>(Whale::htonl_(from_));
			cipher.write<decltype(to_)>(Whale::htonl_(to_));
			cipher.write<decltype(direction_)>(direction_);
			cipher.write<decltype(priority_)>(priority_);
			cipher.write<decltype(charset_)>(charset_);
			cipher.write<decltype(number_)>(Whale::htonl_(number_));
			cipher.write<decltype(compress_)>(compress_);
			cipher.write<decltype(encoded_)>(encoded_);
			cipher.write<decltype(reserve_)>(reserve_);

			buffer_ = compress_ ? Whale::Crypto::HZip::deflate(buffer_) : buffer_;

			buffer_ = encoded_ ? Whale::Crypto::HAes::encode(buffer_,
				Whale::Basic::HByteArray("78c91ba01a3a5a37")) : buffer_;						
			
			cipher.write<decltype(dataLen_)>(Whale::htonl_(buffer_.size()));
			cipher.write(buffer_);

			crc16_ = Whale::Crypto::HCrc::calcu16((uint8_t*)(&cipher[sizeof(head_)]), cipher.size() - sizeof(head_));
			cipher.write<decltype(crc16_)>(Whale::htons_(crc16_));
			cipher.write<decltype(tail_)>(tail_);	

			return cipher;
		}
	}
}
