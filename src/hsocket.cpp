#include <assert.h>
#include "whale/net/hsocket.h"
#include "whale/basic/hlog.h"
#include "whale/net/hdns.h"
#include "whale/basic/hplatform.h"

namespace Whale
{
	namespace Net
	{	
		HSocket::HSocket()
		{
#ifdef OS_WIN32	
			WSADATA  wsaData = { 0 };
			if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
				std::cout << "Init Windows Socket Failed !! Err: " << GetLastError() << std::endl;
			}
#endif
		}

		HSocket::~HSocket()
		{
			stop();
		}

		bool HSocket::config(int type, const std::string& addr, int port,
			std::function< void()> connectFunctional,
			std::function< void()> releaseFunctional)
		{
			type_ = type;
			std::vector<std::string> addrs = Whale::Net::HDns::addrs(addr);
			if (addrs.size() > 0)
				addr_ = addrs[0];
			else
				addr_ = "";
			port_ = port;
			connectFunctional_ = connectFunctional;
			releaseFunctional_ = releaseFunctional;
			inited_ = true;
			return inited_;		
		}		

		bool HSocket::setsockopt(int key, const void* val, int len)
		{
			if(sock_ == -1) return false;
			return ::setsockopt(sock_, SOL_SOCKET, key, (const char*)val, len) != -1;
		}

		bool HSocket::resizebuff(long val)
		{
			if(sock_ == -1) return false;
			return setsockopt(SO_SNDBUF, (const char*)&val, sizeof(val)) && setsockopt(SO_RCVBUF, (const char*)&val, sizeof(val));
		}

		bool HSocket::noneblock(long val)
		{
			if(sock_ == -1) return false;

#ifdef OS_WIN32
			return ::ioctlsocket(sock_, FIONBIO, (unsigned long*)&val) != -1;
#else
			return ::fcntl(sock_, F_SETFL, val == 1 ? O_NONBLOCK : ~O_NONBLOCK) != -1;
#endif

			return true;
		}

		bool HSocket::reuseaddr(long val)
		{
			if(sock_ == -1) return false;
			return setsockopt(SO_REUSEADDR, &val, sizeof(val));
		}

		bool HSocket::sendtimeval(long tv)
		{
			if(sock_ == -1) return false;
			struct timeval timeout = { 0, tv };
			return setsockopt(SO_SNDTIMEO, &timeout, sizeof(struct timeval));
		}

		bool HSocket::recvtimeval(long tv)
		{
			if(sock_ == -1) return false;
			struct timeval timeout = { 0, tv };
			return setsockopt(SO_RCVTIMEO, &timeout, sizeof(struct timeval));
		}

		bool HSocket::send(const Whale::Basic::HByteArray& bytes)
		{
			if(sock_ == -1) return false;		

			size_t index = 0;

			while (index < bytes.size()) {
				int sendLen = 0;

				if (type_ == Whale::Net::HSocket::TCP) {
					sendLen = ::send(sock_, &bytes[index], bytes.size() - index, 0);
				}
				else {
					sockaddr_in addr = { AF_INET, htons(port_) };
					addr.sin_addr.s_addr = inet_addr(addr_.c_str());
					sendLen = ::sendto(sock_, &bytes[index], bytes.size() - index, 0, (sockaddr*)&addr, sizeof(addr));
				}

				if (-1 == sendLen) {
#ifdef OS_WIN32
					switch (WSAGetLastError()) {
					case WSANOTINITIALISED:
						log_error("A successful WSAStartup call must occur before using this function.");
						return false;

					case WSAENETDOWN:
						log_error("The network subsystem has failed.");
						return false;

					case WSAEACCES:
						log_error("The requested address is a broadcast address, but the appropriate flag was not set.Call setsockopt with the SO_BROADCAST socket option to enable use of the broadcast address.");
						return false;

					case WSAEINTR:
						log_error("A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall.");
						return false;

					case WSAEINPROGRESS:
						log_error("A blocking Windows Sockets 1.1 call is in progress, or the supervisor provider is still processing a callback function.");
						return false;

					case WSAEFAULT:
						log_error("The buf parameter is not completely contained in a valid part of the user address space.");
						return false;

					case WSAENETRESET:
						log_error("The connection has been broken due to the keep - alive activity detecting a failure while the operation was in progress.");
						return false;

					case WSAENOBUFS:
						log_error("No buffer space is available.");
						return false;

					case WSAENOTCONN:
						log_error("The socket is not connected.");
						return false;

					case WSAENOTSOCK:
						log_error("The descriptor is not a socket.");
						return false;

					case WSAEOPNOTSUPP:
						log_error("MSG_OOB was specified, but the socket is not stream - style such as type SOCK_STREAM, OOB data is not supported in the filesupervisor domain associated with this socket, or the socket is unidirectional and supports only receive operations.");
						return false;

					case WSAESHUTDOWN:
						log_error("The socket has been shut down; it is not possible to send on a socket after shutdown has been invoked with how set to SD_SEND or SD_BOTH.");
						return false;

					case WSAEWOULDBLOCK:
						log_error("The socket is marked as nonblocking and the requested operation would block.");
						return false;

					case WSAEMSGSIZE:
						log_error("The socket is message oriented, and the message is larger than the maximum supported by the underlying transport.");
						return false;

					case WSAEHOSTUNREACH:
						log_error("The remote host cannot be reached from this host at this time.");
						return false;

					case WSAEINVAL:
						log_error("The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled.");
						return false;

					case WSAECONNABORTED:
						log_error("The virtual circuit was terminated due to a time - out or other failure.The application should close the socket as it is no longer usable.");
						return false;

					case WSAECONNRESET:
						log_error("The virtual circuit was reset by the remote side executing a hard or abortive close.For UDP sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a 'Port Unreachable' ICMP packet.The application should close the socket as it is no longer usable.");
						return false;

					case WSAETIMEDOUT:
						log_error("The connection has been dropped, because of a network failure or because the system on the other end went down without notice.");
						return false;

					default:
						log_error("Unknown err!!");
						return false;
					}
#else
					switch (errno) {
					case EBADF:
						log_error("Parameter socket is not a valid s processing code.");
						return false;

					case EFAULT:
						log_error("A pointer to the memory space that cannot be accessed.");
						return false;

					case ENOTSOCK:
						log_error("Parameter s is a file description word, non socket. ");
						return false;

					case EINTR:
						log_error("Interrupted by a signal.");
						return false;

					case EAGAIN:
						log_error("This operation will cause the process to block, but the parameter socket s is not blocked.");
						return false;

					case ENOBUFS:
						log_error("The buffer memory of the system is insufficient.");
						return false;

					case ENOMEM:
						log_error("Lack of core memory. ");
						return false;

					case EINVAL:
						log_error("The arguments passed to the system are incorrect.");
						return false;

					default:
						log_error("Unknown err!!");
						return false;
					}
#endif
				}

				index += sendLen;
			}

			return true;
		}

		void HSocket::action()
		{
			if (!inited_) {
				return;
			}

			if (!connect()) {
				release();
				return std::this_thread::sleep_for(std::chrono::milliseconds(10 * 1000));
			}

			int buffsize = 0;
			socklen_t optLen = sizeof(socklen_t);
			::getsockopt(sock_, SOL_SOCKET, SO_RCVBUF, (char*)&buffsize, &optLen);

			sockaddr_in addr;
			int addrlen = sizeof(addr);

			char* buffer = new char[buffsize + 1];

			if (type_ == Whale::Net::HSocket::TCP) {
				buffsize = ::recv(sock_, buffer, buffsize, 0);
			}
			else {
#ifdef OS_WIN32
				buffsize = ::recvfrom(sock_, buffer, buffsize, 0, (sockaddr*)&addr, (int*)&addrlen);
#else
				buffsize = ::recvfrom(sock_, buffer, buffsize, 0, (sockaddr*)&addr, (socklen_t*)&addrlen);
#endif
			}

			if (buffsize > 0) {
				buffer_.write(Whale::Basic::HByteArray(buffer, buffsize));
			}

			delete[] buffer;

			if (buffsize > 0) {				
				signalRead(buffer_);
				return;
			}

			if (buffsize == -1) {
#ifdef OS_WIN32
				switch (WSAGetLastError()) {
				case WSANOTINITIALISED:
					log_error("A successful WSAStartup call must occur before using this function.");
					break;

				case WSAENETDOWN:
					log_error("The network subsystem has failed.");
					break;

				case WSAEFAULT:
					log_error("The buf parameter is not completely contained in a valid part of the user address space.");
					break;

				case WSAENOTCONN:
					log_error("The socket is not connected.");
					break;

				case WSAEINTR:
					log_error("The(blocking) call was canceled through WSACancelBlockingCall.");
					break;

				case WSAEINPROGRESS:
					log_error("A blocking Windows Sockets 1.1 call is in progress, or the supervisor provider is still processing a callback function.");
					break;

				case WSAENETRESET:
					log_error("For a connection - oriented socket, this error indicates that the connection has been broken due to keep - alive activity that detected a failure while the operation was in progress.For a datagram socket, this error indicates that the time to live has expired.");
					break;

				case WSAENOTSOCK:
					log_error("The descriptor is not a socket.");
					break;

				case WSAEOPNOTSUPP:
					log_error("MSG_OOB was specified, but the socket is not stream - style such as type SOCK_STREAM, OOB data is not supported in the filesupervisor domain associated with this socket, or the socket is unidirectional and supports only send operations.");
					break;

				case WSAESHUTDOWN:
					log_error("The socket has been shut down; it is not possible to receive on a socket after shutdown has been invoked with how set to SD_RECEIVE or SD_BOTH.");
					break;

				case WSAEWOULDBLOCK:
					log_error("The socket is marked as nonblocking and the receive operation would block.");
					break;

				case WSAEMSGSIZE:
					log_error("The message was too large to fit into the specified buffer and was truncated.");
					break;

				case WSAEINVAL:
					log_error("The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or(for unsigned char stream sockets only) len was zero or negative.");
					break;

				case WSAECONNABORTED:
					log_error("The virtual circuit was terminated due to a time - out or other failure.The application should close the socket as it is no longer usable.");
					break;

				case WSAETIMEDOUT:
					log_error("The connection has been dropped because of a network failure or because the peer system failed to respond.");
					break;

				case WSAECONNRESET:
					log_error("The virtual circuit was reset by the remote side executing a hard or abortive close.The application should close the socket as it is no longer usable.On a UDP - datagram socket, this error would indicate that a previous send operation resulted in an ICMP 'Port Unreachable' message.");
					break;

				default:
					log_error("Unknown err!!");
					break;
				}
#else
				switch (errno)
				{
				case EBADF:
					log_error("Parameter socket is not a valid s processing code.");
					break;

				case EFAULT:
					log_error("A pointer to the memory space that cannot be accessed.");
					break;

				case ENOTSOCK:
					log_error("Parameter s is a file description word, non socket. ");
					break;

				case EINTR:
					log_error("Interrupted by a signal.");
					break;

				case EAGAIN:
					log_error("This operation will cause the process to block, but the parameter socket s is not blocked.");
					break;

				case ENOBUFS:
					log_error("The buffer memory of the system is insufficient.");
					break;

				case ENOMEM:
					log_error("Lack of core memory. ");
					break;

				case EINVAL:
					log_error("The arguments passed to the system are incorrect.");
					break;

				default:
					log_error("Unknown err!!");
					break;
				}
#endif
			}

			if (buffsize == 0) {
				log_error("connection has been cut down !!");							
			}
			
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			release();
		}

		bool HSocket::connect()
		{
			if (connected_) {
				return connected_;
			}

			sock_ = ::socket(AF_INET,
				type_ == Whale::Net::HSocket::TCP ? SOCK_STREAM : SOCK_DGRAM, IPPROTO_IP);

#ifdef OS_WIN32
			
			if (sock_ == SOCKET_ERROR) {
				switch (WSAGetLastError()) {
				case WSANOTINITIALISED:
					log_error("A successful WSAStartup call must occur before using this function.");
					return false;

				case WSAENETDOWN:
					log_error("The network subsystem or the associated supervisor provider has failed.");
					return false;

				case WSAEAFNOSUPPORT:
					log_error("The specified address family is not supported.For example, an application tried to create a socket for the AF_IRDA address family but an infrared adapter and device driver is not installed on the local computer.");
					return false;

				case WSAEINPROGRESS:
					log_error("A blocking Windows Sockets 1.1 call is in progress, or the supervisor provider is still processing a callback function.");
					return false;

				case WSAEMFILE:
					log_error("No more socket descriptors are available.");
					return false;

				case WSAEINVAL:
					log_error("An invalid argument was supplied.This error is returned if the af parameter is set to AF_UNSPEC and the type and protocol parameter are unspecified.");
					return false;

				case WSAEINVALIDPROVIDER:
					log_error("The supervisor provider returned a version other than 2.2.");
					return false;

				case WSAEINVALIDPROCTABLE:
					log_error("The supervisor provider returned an invalid or incomplete procedure table to the WSPStartup.");
					return false;

				case WSAENOBUFS:
					log_error("No buffer space is available.The socket cannot be created.");
					return false;

				case WSAEPROTONOSUPPORT:
					log_error("The specified protocol is not supported.");
					return false;

				case WSAEPROTOTYPE:
					log_error("The supervisor provider failed to initialize.This error is returned if a layered supervisor provider(LSP) or namespace provider was improperly installed or the provider fails to operate correctly.");
					return false;

				case WSAEPROVIDERFAILEDINIT:
					log_error("The supervisor provider returned an invalid or incomplete procedure table to the WSPStartup.");
					return false;

				case WSAESOCKTNOSUPPORT:
					log_error("The specified socket type is not supported in this address family.");
					return false;

				default:
					log_error("Unknown err !!");
					return false;
				}
			}

#else
			 
			if (sock_ == -1) {
				switch (errno) {
				case ENFILE:
					log_error("Parameter domain specifies the type that does not support the parameter specified by type or protocol.");
					return false;

				case EMFILE:
					log_error("Parameter domain specifies the type that does not support the parameter specified by type or protocol. ");
					return false;

				case ENOMEM:
					log_error("Insufficient memory. ");
					return false;

				case ENOBUFS:
					log_error("Insufficient memory. ");
					return false;

				case EINVAL:
					log_error("Parameter domain/type/protocol is not valid. ");
					return false;

				default:
					log_error("Unknown err [%d]!!", errno);
					return false;
				}
			}
#endif 	
			log_notice("Create socket success [%d], Connect [%s:%d] !!", sock_, addr_.c_str(), port_);
			
			sockaddr_in sock = { AF_INET, htons(port_) };
			sock.sin_addr.s_addr = inet_addr(addr_.c_str());

			// connect timeout 3s
			sendtimeval(3000000);			
			
			if (::connect(sock_, (sockaddr*)&sock, sizeof(sock)) == -1)	{				
#ifdef OS_WIN32
				switch (WSAGetLastError())	{
				case WSANOTINITIALISED:
					log_error("A successful WSAStartup call must occur before using this function.");
					return false;

				case WSAENETDOWN:
					log_error("The network subsystem has failed.");
					return false;

				case WSAEADDRINUSE:
					log_error("The socket's local address is already in use and the socket was not marked to allow address reuse with SO_REUSEADDR. This error usually occurs when executing bind, but could be delayed until the connect function if the bind was to a wildcard address (INADDR_ANY or in6addr_any) for the local IP address. A specific address needs to be implicitly bound by the connect function.");
					return false;

				case WSAEINTR:
					log_error("The blocking Windows Socket 1.1 call was canceled through WSACancelBlockingCall.");
					return false;

				case WSAEINPROGRESS:
					log_error("A blocking Windows Sockets 1.1 call is in progress, or the supervisor provider is still processing a callback function.");
					return false;

				case WSAEALREADY:
					log_error("A nonblocking connect call is in progress on the specified socket.Note  In order to preserve backward compatibility, this error is reported as WSAEINVAL to Windows Sockets 1.1 applications that link to either Winsock.dll or Wsock32.dll.");
					return false;

				case WSAEADDRNOTAVAIL:
					log_error("The remote address is not a valid address(such as INADDR_ANY or in6addr_any) .");
					return false;

				case WSAEAFNOSUPPORT:
					log_error("Addresses in the specified family cannot be used with this socket.");
					return false;

				case WSAECONNREFUSED:
					log_error("The attempt to connect was forcefully rejected.");
					return false;

				case WSAEFAULT:
					log_error("The sockaddr structure pointed to by the name contains incorrect address format for the associated address family or the namelen parameter is too small.This error is also returned if the sockaddr structure pointed to by the name parameter with a length specified in the namelen parameter is not in a valid part of the user address space.");
					return false;

				case WSAEINVAL:
					log_error("The parameter s is a listening socket.");
					return false;

				case WSAEISCONN:
					log_error("The socket is already connected(connection - oriented sockets only).");
					return false;

				case WSAENETUNREACH:
					log_error("The network cannot be reached from this host at this time.");
					return false;

				case WSAEHOSTUNREACH:
					log_error("A socket operation was attempted to an unreachable host.");
					return false;

				case WSAENOBUFS:
					log_error("Note  No buffer space is available.The socket cannot be connected.");
					return false;

				case WSAENOTSOCK:
					log_error("The descriptor specified in the s parameter is not a socket.");
					return false;

				case WSAETIMEDOUT:
					log_error("An attempt to connect timed out without establishing a connection.");
					return false;

				case WSAEWOULDBLOCK:
					log_error("The socket is marked as nonblocking and the connection cannot be completed immediately.");
					return false;

				case WSAEACCES:
					log_error("An attempt to connect a datagram socket to broadcast address failed because setsockopt option SO_BROADCAST is not enabled.");
					return false;

				default:
					log_error("Unknown err !!");
					return false;
				}
#else
				switch (errno)
				{
				case EBADF:
					log_error("Parameter sockfd is not a valid socket processing code. ");
					return false;

				case EFAULT:
					log_error("Parameter serv_addr pointer to the memory space that cannot be accessed. ");
					return false;

				case ENOTSOCK:
					log_error("Parameter sockfd is a file description word, non socket.");
					return false;

				case EISCONN:
					log_error("Parameter socket sockfd is already in the connection state. ");
					return false;

				case ECONNREFUSED:
					log_error("Connection requests were rejected by the server side.");
					return false;

				case ETIMEDOUT:
					log_error("Attempts to connect to the line have not been in response to more than a limited period of time. ");
					return false;

				case ENETUNREACH:
					log_error("Unable to send data packets to the specified host.");
					return false;

				case EAFNOSUPPORT:
					log_error("sockaddr structure of the sa_family is not correct. ");
					return false;

				case EALREADY:
					log_error("Socket can not be blocked and the previous connection operation has not been completed.");
					return false;

				default:
					log_error("Unknown err !!");
					return false;
				}
#endif
			}

			log_notice("Connect [%s:%d] success!!", addr_.c_str(), port_);

			connected_ = true;
			connectFunctional_();

			return connected_;
		}

		bool HSocket::broadcast(const Whale::Basic::HByteArray& bytes)
		{
			if(sock_ == -1) return false;
			
			if (type_ != Whale::Net::HSocket::UDP)
				return false;

			bool optval = true;
			setsockopt(SO_BROADCAST, (char*)&optval, sizeof(optval));

			sockaddr_in addr;
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port_);
			addr.sin_addr.s_addr = INADDR_BROADCAST;

			size_t index = 0;

			while (index < bytes.size())
			{
				int sendLen = ::sendto(sock_, &bytes[index], bytes.size() - index, 0, (sockaddr*)&addr, sizeof(addr));

				if (-1 == sendLen) {
#ifdef OS_WIN32
					switch (WSAGetLastError()) {
					case WSANOTINITIALISED:
						log_error("A successful WSAStartup call must occur before using this function.");
						return false;

					case WSAENETDOWN:
						log_error("The network subsystem has failed.");
						return false;

					case WSAEACCES:
						log_error("The requested address is a broadcast address, but the appropriate flag was not set.Call setsockopt with the SO_BROADCAST parameter to allow the use of the broadcast address.");
						return false;

					case WSAEINVAL:
						log_error("An unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled.");
						return false;

					case WSAEINTR:
						log_error("A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall.");
						return false;

					case WSAEINPROGRESS:
						log_error("A blocking Windows Sockets 1.1 call is in progress, or the supervisor provider is still processing a callback function.");
						return false;

					case WSAEFAULT:
						log_error("The buf or to parameters are not part of the user address space, or the tolen parameter is too small.");
						return false;

					case WSAENETRESET:
						log_error("The connection has been broken due to keep - alive activity detecting a failure while the operation was in progress.");
						return false;

					case WSAENOBUFS:
						log_error("No buffer space is available.");
						return false;

					case WSAENOTCONN:
						log_error("The socket is not connected(connection - oriented sockets only).");
						return false;

					case WSAENOTSOCK:
						log_error("The descriptor is not a socket.");
						return false;

					case WSAEOPNOTSUPP:
						log_error("MSG_OOB was specified, but the socket is not stream - style such as type SOCK_STREAM, OOB data is not supported in the filesupervisor domain associated with this socket, or the socket is unidirectional and supports only receive operations.");
						return false;

					case WSAESHUTDOWN:
						log_error("The socket has been shut down; it is not possible to sendto on a socket after shutdown has been invoked with how set to SD_SEND or SD_BOTH.");
						return false;

					case WSAEWOULDBLOCK:
						log_error("The socket is marked as nonblocking and the requested operation would block.");
						return false;

					case WSAEMSGSIZE:
						log_error("The socket is message oriented, and the message is larger than the maximum supported by the underlying transport.");
						return false;

					case WSAEHOSTUNREACH:
						log_error("The remote host cannot be reached from this host at this time.");
						return false;

					case WSAECONNABORTED:
						log_error("The virtual circuit was terminated due to a time - out or other failure.The application should close the socket as it is no longer usable.");
						return false;

					case WSAECONNRESET:
						log_error("The virtual circuit was reset by the remote side executing a hard or abortive close.For UPD sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a 'Port Unreachable' ICMP packet.The application should close the socket as it is no longer usable.");
						return false;

					case WSAEADDRNOTAVAIL:
						log_error("The remote address is not a valid address, for example, ADDR_ANY.");
						return false;

					case WSAEAFNOSUPPORT:
						log_error("Addresses in the specified family cannot be used with this socket.");
						return false;

					case WSAEDESTADDRREQ:
						log_error("A destination address is required.");
						return false;

					case WSAENETUNREACH:
						log_error("A socket operation was attempted to an unreachable host.");
						return false;

					case WSAETIMEDOUT:
						log_error("The connection has been dropped, because of a network failure or because the system on the other end went down without notice.");
						return false;

					default:
						log_error("Unknown err!!");
						return false;
					}
#else
					switch (errno)	{
					case EBADF:
						log_error("Parameter socket is not a valid s processing code.");
						return false;

					case EFAULT:
						log_error("A pointer to the memory space that cannot be accessed.");
						return false;

					case ENOTSOCK:
						log_error("Parameter s is a file description word, non socket. ");
						return false;

					case EINTR:
						log_error("Interrupted by a signal.");
						return false;

					case EAGAIN:
						log_error("This operation will cause the process to block, but the parameter socket s is not blocked.");
						return false;

					case ENOBUFS:
						log_error("The buffer memory of the system is insufficient.");
						return false;

					case ENOMEM:
						log_error("Lack of core memory. ");
						return false;

					case EINVAL:
						log_error("The arguments passed to the system are incorrect.");
						return false;

					default:
						log_error("Unknown err!!");
						return false;
					}
#endif
				}

				index += sendLen;
			}

			log_notice("Socket [%d] broadcast success!!", sock_);

			return true;
		}

		//////////////////////////////////////////////////////////////////////////
		// SHUT_RD（0）：关闭sockfd上的读功能，此选项将不允许sockfd进行读操作。
		// SHUT_WR（1）：关闭sockfd的写功能，此选项将不允许sockfd进行写操作。
		// SHUT_RDWR（2）：关闭sockfd的读写功能。
		//////////////////////////////////////////////////////////////////////////
		void HSocket::release()
		{
			if (sock_ > 0) {
				shutdown(sock_, 2);
				closesocket(sock_);
				log_notice("Close socket [%d]!!", sock_);
				releaseFunctional_();
			}

			sock_ = -1;
			connected_ = false;
		}
		
		void HSocket::stop()
		{
			release();
			Whale::Basic::HThread::stop();
		}
	}
}
