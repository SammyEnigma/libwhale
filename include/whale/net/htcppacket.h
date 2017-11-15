/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	htcppacket.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Replace Version	:	1.0
* Original Author	:	Mr Li
* Finished Date		:	2017/05/05 09:58:00
****************************************************************************************/
#ifndef HTCPPACKET_H
#define HTCPPACKET_H

#include <whale/basic/hbytearray.h>

namespace Whale
{
	namespace Net
	{
		class TcpSession {
		public:
			explicit TcpSession() {

			}

			virtual ~TcpSession() {

			}

			inline uint32_t  port() const {
				return port_;
			}
			inline void port(uint32_t val) {
				port_ = val;
			}

			inline std::string addr() const {
				return addr_;
			}
			inline void addr(const std::string& val) {
				addr_ = val;
			}

			inline std::string mac() const {
				return mac_;
			}
			inline void mac(const std::string& val) {
				mac_ = val;
			}

			inline void id(uint32_t val) {
				id_ = val;
			}
			inline uint32_t id() const {
				return id_;
			}

			inline void code(uint32_t val) {
				code_ = val;
			}
			inline uint32_t code() const {
				return code_;
			}

		private:
			uint32_t port_;
			std::string addr_ = "";
			std::string mac_ = "";
			uint32_t id_;
			uint32_t code_;
		};

		class HTcpPacket
		{
		public:
			explicit HTcpPacket(void);
			virtual ~HTcpPacket(void);

			inline TcpSession& session() {
				return session_;
			}
			inline void session(TcpSession val) {
				session_ = val;
			}

			inline uint8_t head() const {
				return head_; 
			}
			inline void head(uint8_t val) {
				head_ = val; 
			}

			inline uint8_t version() const {
				return version_;
			}
			inline void version(uint8_t val) {
				version_ = val;
			}

			inline uint16_t venderCode() const {
				return vender_;
			}
			inline void venderCode(uint16_t val) {
				vender_ = val;
			}

			inline uint32_t userCode() const {
				return user_;
			}
			inline void userCode(uint32_t val) {
				user_ = val;
			}			

			inline uint8_t type() const {
				return type_;
			}
			inline void type(uint8_t val) {
				type_ = val;
			}

			inline uint32_t from() const {
				return from_;
			}
			inline void from(uint32_t val) {
				from_ = val;
			}

			inline uint32_t to() const {
				return to_;
			}
			inline void to(uint32_t val) {
				to_ = val;
			}

			inline uint8_t direction() const {
				return direction_;
			}
			inline void direction(uint8_t val) {
				direction_ = val;
			}

			inline uint8_t priority() const {
				return priority_;
			}
			inline void priority(uint8_t val) {
				priority_ = val;
			}

			inline uint8_t charset() const {
				return charset_;
			}
			inline void charset(uint8_t val) {
				charset_ = val;
			}

			inline uint32_t number() const {
				return number_;
			}
			inline void number(uint32_t val) {
				number_ = val;
			}

			inline uint8_t compress() const {
				return compress_;
			}
			inline void compress(uint8_t val) {
				compress_ = val;
			}

			inline uint8_t encoded() const {
				return encoded_;
			}
			inline void encoded(uint8_t val) {
				encoded_ = val;
			}

			inline Whale::Basic::HByteArray buffer() const {
				return buffer_;
			}
			inline void buffer(Whale::Basic::HByteArray val) {
				buffer_ = val;
			}

			inline Whale::Basic::HByteArray result() const {
				return reply_;
			}
			inline void result(Whale::Basic::HByteArray val) {
				reply_ = val;
			}

			inline uint16_t crc16() const {
				return crc16_;
			}
			inline void crc16(uint16_t val) {
				crc16_ = val;
			}

			inline uint8_t tail() const {
				return tail_;
			}
			inline void tail(uint8_t val) {
				tail_ = val;
			}

			inline bool first() const {
				return first_;
			}
			inline void first(bool val) {
				first_ = val;
			}

			inline uint32_t maxmium() const {
				return maxmium_;
			}
			inline void maxmium(uint32_t val) {
				maxmium_ = val;
			}

			virtual uint32_t decode(Whale::Basic::HByteArray& bytes);
			virtual Whale::Basic::HByteArray encode();

		private:
			TcpSession session_;
			uint8_t head_;
			uint8_t version_;
			uint16_t vender_;
			uint32_t user_;			
			uint8_t type_;
			uint32_t from_;
			uint32_t to_;
			uint8_t direction_;
			uint8_t priority_;
			uint8_t charset_;
			uint32_t number_;
			uint8_t compress_;
			uint8_t encoded_;
			uint8_t reserve_[8]; // 预留8字节
			uint32_t dataLen_;
			uint16_t crc16_;
			uint8_t tail_;
			Whale::Basic::HByteArray buffer_;
			Whale::Basic::HByteArray reply_;			

			uint32_t lenPos_ = 0;
			uint32_t totalSize_ = 0;
			uint32_t maxmium_ = 0;
			bool first_ = false;
		};
	}
}

#endif
