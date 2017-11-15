#ifndef __HFTPSOCKET_H
#define __HFTPSOCKET_H

#include "whale/basic/hplatform.h"

namespace Whale
{
	namespace Net
	{

		class HFtpSockAddr : public sockaddr_in
		{
		public:
			inline HFtpSockAddr() {
				sin_family = AF_INET;
				sin_port = 0;
				sin_addr.s_addr = 0;
			}

			inline HFtpSockAddr(const sockaddr& sa) {
				memcpy(this, &sa, sizeof(sockaddr));
			}

			inline HFtpSockAddr(const sockaddr_in& sin) {
				memcpy(this, &sin, sizeof(sockaddr_in));
			}

			inline HFtpSockAddr(unsigned long ulAddr, unsigned short ushPort = 0) {
				sin_family = AF_INET;
				sin_port = htons(ushPort);
				sin_addr.s_addr = htonl(ulAddr);
			}

			inline HFtpSockAddr(const std::string& pchIP, unsigned short ushPort = 0) {
				sin_family = AF_INET;
				sin_port = htons(ushPort);
				sin_addr.s_addr = inet_addr(pchIP.c_str());
			}

			inline std::string dottedDecimal() {
				return inet_ntoa(sin_addr);
			}

			inline unsigned short port() const{
				return ntohs(sin_port);
			}

			inline unsigned int ipAddr() const{
				return ntohl(sin_addr.s_addr);
			}

			inline HFtpSockAddr& operator=(const sockaddr& sa) {
				memcpy(this, &sa, sizeof(sockaddr));
				return *this;
			}

			inline HFtpSockAddr& operator=(const sockaddr_in& sin)	{
				memcpy(this, &sin, sizeof(sockaddr_in));
				return *this;
			}

			inline operator sockaddr(){
				return *((sockaddr*) this);
			}

			inline operator sockaddr*()	{
				return (sockaddr*) this;
			}

			inline operator sockaddr_in*() {
				return (sockaddr_in*) this;
			}
		};

		class IFtpSocket
		{
		public:
			virtual ~IFtpSocket() {}
			virtual IFtpSocket* instance() const = 0;
			virtual bool create(int nType = SOCK_STREAM) = 0;
			virtual bool connect(const struct sockaddr* psa) const = 0;
			virtual bool bind(const struct sockaddr* psa) const = 0;
			virtual bool listen() const = 0;
			virtual void cleanUp() = 0;
			virtual bool accept(IFtpSocket& s, sockaddr* psa) const = 0;
			virtual void close_() = 0;
			virtual int  write(const char* pch, int nSize, int nSecs) const = 0;
			virtual int  receive(char* pch, int nSize, int nSecs) const = 0;
			virtual bool peerAddr(sockaddr* psa) const = 0;
			virtual bool sockAddr(sockaddr* psa) const = 0;
			virtual operator unsigned int() const = 0;
			virtual bool readability() const = 0;
			virtual HFtpSockAddr   getHostByName(const char* pchName, unsigned short ushPort = 0) = 0;
			virtual const char* getHostByAddr(const struct sockaddr* psa) = 0;
		};

		class HFtpSocket : public IFtpSocket
		{
		public:
			explicit HFtpSocket();
			virtual ~HFtpSocket();

			virtual IFtpSocket* instance() const;

			void cleanUp();
			bool create(int nType = SOCK_STREAM);
			void close_();

			bool bind(const struct sockaddr* psa) const;
			bool listen() const;
			bool connect(const struct sockaddr* psa) const;
			bool accept(IFtpSocket& s, sockaddr* psa) const;
			int  send(const char* pch, int nSize, int nSecs) const;
			int  write(const char* pch, int nSize, int nSecs) const;
			int  receive(char* pch, int nSize, int nSecs) const;
			int  sendDatagram(const char* pch, int nSize, const struct sockaddr* psa, int nSecs) const;
			int  receiveDatagram(char* pch, int nSize, sockaddr* psa, int nSecs) const;
			bool peerAddr(sockaddr* psa) const;
			bool sockAddr(sockaddr* psa) const;
			bool readability() const;

			HFtpSockAddr   getHostByName(const char* pchName, unsigned short ushPort = 0);
			const char* getHostByAddr(const struct sockaddr* psa);

			operator unsigned int() const {
				return _socket;
			}

		private:
			unsigned int _socket;
		};


		class HFtpHttpSocket : public HFtpSocket
		{
		public:
			enum { nSizeRecv = 1000 };

			HFtpHttpSocket();
			~HFtpHttpSocket();

			int readHeader(char* pch, int nSize, int nSecs);
			int readResponse(char* pch, int nSize, int nSecs);

		private:
			char*  _readBuff; // read buffer
			int    _readBuffSize; // number of bytes in the read buffer
		};

	}
}

#endif // __HFTPSOCKET_H
