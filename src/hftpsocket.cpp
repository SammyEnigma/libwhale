#include "whale/net/hftpsocket.h"

#ifdef min
#undef min
#endif

namespace Whale
{
	namespace Net
	{

		IFtpSocket* HFtpSocket::instance() const
		{
			return new HFtpSocket();
		}

		HFtpSocket::HFtpSocket() :
			_socket((unsigned int)(~0))
		{
#ifdef OS_WIN32
			// Initialize the Winsock dll version 2.0
			WSADATA  wsaData = { 0 };
			WSAStartup(MAKEWORD(2, 0), &wsaData);
#endif
		}

		HFtpSocket::~HFtpSocket()
		{
			cleanUp();
#ifdef OS_WIN32
			WSACleanup();
#endif
		}

		void HFtpSocket::cleanUp()
		{
			// doesn't throw an exception because it's called in a catch block
			if (_socket == (unsigned int)(~0))
				return;

			closesocket(_socket);

			_socket = (unsigned int)(~0);
		}

		bool HFtpSocket::create(int nType /* = SOCK_STREAM */)
		{
			assert(_socket == (unsigned int)(~0));
			return (_socket = socket(AF_INET, nType, 0)) != (unsigned int)(~0);
		}

		bool HFtpSocket::bind(const struct sockaddr* psa) const
		{
			assert(psa != NULL);
			assert(_socket != (unsigned int)(~0));
			return ::bind(_socket, psa, sizeof(sockaddr)) != (-1);
		}

		bool HFtpSocket::listen() const
		{
			assert(_socket != (unsigned int)(~0));
			return ::listen(_socket, 5) != (-1);
		}

		bool HFtpSocket::accept(IFtpSocket& sConnect, sockaddr* psa) const
		{
			assert(psa != NULL);
			assert(_socket != (unsigned int)(~0));
			assert(sConnect.operator unsigned int() == (unsigned int)(~0));

			// ATTENTION: dynamic_cast would be better (and then checking against NULL)
			//            RTTI must be enabled to use dynamic_cast //+#
			HFtpSocket* pConnect = static_cast<HFtpSocket*>(&sConnect);

			socklen_t nLengthAddr = sizeof(sockaddr);
			pConnect->_socket = ::accept(_socket, psa, &nLengthAddr);

			return pConnect->operator unsigned int() != (unsigned int)(~0);
		}

		void HFtpSocket::close_()
		{
			if (_socket != (unsigned int)(~0)) {
				closesocket(_socket);
			}
			_socket = (unsigned int)(~0);
		}

		bool HFtpSocket::connect(const struct sockaddr* psa) const
		{
			assert(psa != NULL);
			assert(_socket != (unsigned int)(~0));
			// should timeout by itself
			return ::connect(_socket, psa, sizeof(sockaddr)) != (-1);
		}

		int HFtpSocket::write(const char* pch, int nSize, int nSecs) const
		{
			assert(pch != NULL);
			int         nBytesSent = 0;
			int         nBytesThisTime = 0;
			const char* pch1 = pch;

			do {
				nBytesThisTime = send(pch1, nSize - nBytesSent, nSecs);
				if (nBytesThisTime <= 0) {
					return nBytesThisTime;
				}
				nBytesSent += nBytesThisTime;
				pch1 += nBytesThisTime;
			} while (nBytesSent < nSize);

			return nBytesSent;
		}

		int HFtpSocket::send(const char* pch, int nSize, int nSecs) const
		{
			assert(pch != NULL);
			assert(_socket != (unsigned int)(~0));

			// returned value will be less than nSize if client cancels the reading
			fd_set fd;
			FD_ZERO(&fd);
			FD_SET(_socket, &fd);
			timeval tv = { nSecs, 0 };

			if (select(_socket + 1, NULL, &fd, NULL, &tv) == 0)
			{
				return -1;
			}

			const int nBytesSent = ::send(_socket, pch, nSize, 0);
			if (nBytesSent == (-1))
			{
				return -1;
			}

			return nBytesSent;
		}

		bool HFtpSocket::readability() const
		{
			assert(_socket != (unsigned int)(~0));

			fd_set fd;
			FD_ZERO(&fd);
			FD_SET(_socket, &fd);
			timeval tv = { 0, 0 };

			// static_cast is necessary to avoid compiler warning under WIN32;
			// This is no problem because the first parameter is included only
			// for compatibility with Berkeley sockets.
			const int iRet = select(_socket + 1, &fd, NULL, NULL, &tv);

			return iRet == 1;
		}

		int HFtpSocket::receive(char* pch, int nSize, int nSecs) const
		{
			assert(pch != NULL);
			assert(_socket != (unsigned int)(~0));

			fd_set fd;
			FD_ZERO(&fd);
			FD_SET(_socket, &fd);
			timeval tv = { nSecs, 0 };

			// static_cast is necessary to avoid compiler warning under WIN32;
			// This is no problem because the first parameter is included only
			// for compatibility with Berkeley sockets.
			if (select(_socket + 1, &fd, NULL, NULL, &tv) == 0)	{
				return -1;
			}

			const int nBytesReceived = recv(_socket, pch, nSize, 0);
			if (nBytesReceived == (-1))	{
				return -1;
			}

			return nBytesReceived;
		}

		int HFtpSocket::receiveDatagram(char* pch, int nSize, sockaddr* psa, int nSecs) const
		{
			assert(pch != NULL);
			assert(psa != NULL);
			assert(_socket != (unsigned int)(~0));

			fd_set fd;
			FD_ZERO(&fd);
			FD_SET(_socket, &fd);
			timeval tv = { nSecs, 0 };

			// static_cast is necessary to avoid compiler warning under WIN32;
			// This is no problem because the first parameter is included only
			// for compatibility with Berkeley sockets.
			if (select(_socket + 1, &fd, NULL, NULL, &tv) == 0)	{
				return -1;
			}

			// input buffer should be big enough for the entire datagram
			socklen_t nFromSize = sizeof(sockaddr);
			const int nBytesReceived = recvfrom(_socket, pch, nSize, 0, psa, &nFromSize);

			if (nBytesReceived == (-1))
			{
				return -1;
			}

			return nBytesReceived;
		}

		int HFtpSocket::sendDatagram(const char* pch, int nSize, const struct sockaddr* psa, int nSecs) const
		{
			assert(pch != NULL);
			assert(psa != NULL);
			assert(_socket != (unsigned int)(~0));

			fd_set fd;
			FD_ZERO(&fd);
			FD_SET(_socket, &fd);
			timeval tv = { nSecs, 0 };

			// static_cast is necessary to avoid compiler warning under WIN32;
			// This is no problem because the first parameter is included only
			// for compatibility with Berkeley sockets.
			if (select(_socket + 1, NULL, &fd, NULL, &tv) == 0)
			{
				return -1;
			}

			const int nBytesSent = sendto(_socket, pch, nSize, 0, psa, sizeof(sockaddr));
			if (nBytesSent == (-1))
			{
				return -1;
			}

			return nBytesSent;
		}

		bool HFtpSocket::peerAddr(sockaddr* psa) const
		{
			assert(psa != NULL);
			assert(_socket != (unsigned int)(~0));

			// gets the address of the socket at the other end
			socklen_t nLengthAddr = sizeof(sockaddr);
			return getpeername(_socket, psa, &nLengthAddr) != (-1);
		}

		bool HFtpSocket::sockAddr(sockaddr* psa) const
		{
			assert(psa != NULL);
			assert(_socket != (unsigned int)(~0));

			// gets the address of the socket at this end
			socklen_t nLengthAddr = sizeof(sockaddr);
			return getsockname(_socket, psa, &nLengthAddr) != (-1);
		}

		HFtpSockAddr HFtpSocket::getHostByName(const char* pchName, unsigned short ushPort /* = 0 */)
		{
			assert(pchName != NULL);
			hostent* pHostEnt = gethostbyname(pchName);

			if (pHostEnt == NULL) {
				return HFtpSockAddr();
			}

			unsigned int* pulAddr = (unsigned int*)pHostEnt->h_addr_list[0];
			sockaddr_in sockTemp;
			sockTemp.sin_family = AF_INET;
			sockTemp.sin_port = htons(ushPort);
			sockTemp.sin_addr.s_addr = *pulAddr; // address is already in network byte order
			return sockTemp;
		}

		const char* HFtpSocket::getHostByAddr(const struct sockaddr* psa)
		{
			assert(psa != NULL);
			hostent* pHostEnt = gethostbyaddr((char*)&((sockaddr_in*)psa)
				->sin_addr.s_addr, 4, PF_INET);

			if (pHostEnt == NULL) {
				return "";
			}

			return pHostEnt->h_name; // caller shouldn't delete this memory
		}

		///////////////////////////////////////////////////////////////////////////////////////
		//**************************** Class CHttpBlockingSocket ****************************//
		///////////////////////////////////////////////////////////////////////////////////////

		HFtpHttpSocket::HFtpHttpSocket()
		{
			_readBuff = new char[nSizeRecv];
			_readBuffSize = 0;
		}

		HFtpHttpSocket::~HFtpHttpSocket()
		{
			delete[] _readBuff;
		}

		int HFtpHttpSocket::readHeader(char* pch, int nSize, int nSecs)
			// reads an entire header line through CRLF (or socket close)
			// inserts zero string terminator, object maintains a buffer
		{
			int       nBytesThisTime = _readBuffSize;
			ptrdiff_t nLineLength = 0;
			char*     pch1 = _readBuff;
			char*     pch2 = NULL;

			do
			{
				// look for lf (assume preceded by cr)
				if ((pch2 = (char*)memchr(pch1, '\n', nBytesThisTime)) != NULL)
				{
					assert((pch2) > _readBuff);
					assert(*(pch2 - 1) == '\r');
					nLineLength = (pch2 - _readBuff) + 1;
					if (nLineLength >= nSize)
						nLineLength = nSize - 1;
					memcpy(pch, _readBuff, nLineLength); // copy the line to caller
					_readBuffSize -= static_cast<unsigned int>(nLineLength);
					memmove(_readBuff, pch2 + 1, _readBuffSize); // shift remaining characters left
					break;
				}
				pch1 += nBytesThisTime;
				nBytesThisTime = receive(_readBuff + _readBuffSize, nSizeRecv - _readBuffSize, nSecs);
				if (nBytesThisTime <= 0)
				{
					return nBytesThisTime;
				}
				_readBuffSize += nBytesThisTime;
			} while (true);

			*(pch + nLineLength) = ('\0');

			return static_cast<unsigned int>(nLineLength);
		}

		// reads remainder of a transmission through buffer full or socket close
		// (assume headers have been read already)
		int HFtpHttpSocket::readResponse(char* pch, int nSize, int nSecs)
		{
			int nBytesToRead, nBytesThisTime, nBytesRead = 0;

			if (_readBuffSize > 0)
			{ // copy anything already in the recv buffer
				memcpy(pch, _readBuff, _readBuffSize);
				pch += _readBuffSize;
				nBytesRead = _readBuffSize;
				_readBuffSize = 0;
			}
			do
			{ // now pass the rest of the data directly to the caller
				nBytesToRead = std::min(static_cast<int>(nSizeRecv), nSize - nBytesRead);
				nBytesThisTime = receive(pch, nBytesToRead, nSecs);
				if (nBytesThisTime <= 0)
					break; // sender closed the socket
				pch += nBytesThisTime;
				nBytesRead += nBytesThisTime;
			} while (nBytesRead <= nSize);

			return nBytesRead;
		}
	}
}
