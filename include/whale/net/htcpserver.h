/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	htcpserver.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HTCPSERVER_H
#define HTCPSERVER_H

#ifndef WIN32
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#endif

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/thread.h>

#include "whale/basic/hplatform.h"
#include "whale/basic/hthread.h"
#include "whale/basic/hdatetime.h"
#include "whale/basic/hbytearray.h"
#include "whale/net/htcppacket.h"
#include "whale/basic/heventpool.h"
#include "whale/basic/hthreadsafelist.h"
#include "whale/basic/hos.h"
#include "whale/basic/hthreadpool.h"

namespace Whale
{
	namespace Net
	{
		class HTcpServer;

		class HTcpSession
		{
		public:
			explicit HTcpSession() = delete;
			explicit HTcpSession(Whale::Net::HTcpServer* tcpServer,
				struct bufferevent *bufferEvent);
			virtual ~HTcpSession();

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
				code_ = Whale::ipToInt(addr_);
			}

			inline std::string mac() const {
				return mac_;
			}
			inline void mac(const std::string& val) {
				mac_ = val;
			}

			inline void timeoutSeconds(int val) {
				struct timeval timeoutval = { val, 0 };
				bufferevent_set_timeouts(bufferEvent_, &timeoutval, 0);
			}
			inline int  timeoutSeconds() const {
				return timeoutSeconds_;
			}

			inline evutil_socket_t fd() const {
				return bufferevent_getfd(bufferEvent_);
			}

			inline Whale::Basic::HTimer& timer() {
				return timer_;
			}
			inline Whale::Basic::HDateTime& loginTime() {
				return loginTime_;
			}

			inline Whale::Basic::HByteArray& buffer() {
				return buffer_;
			}

			inline void id(uint32_t val) {
				id_ = val;
			}
			inline uint32_t id() const {
				return id_;
			}

			inline Whale::Net::HTcpServer* server() {
				return server_;
			}

			inline void bindParam(void* pvoid) {
				pvoid_ = pvoid;
			}
			inline void* bindParam() const {
				return pvoid_;
			}

			inline void code(uint32_t val) {
				code_ = val;
			}
			inline uint32_t code() const {
				return code_;
			}

			virtual int write(const Whale::Basic::HByteArray& byteArray);

		private:
			uint32_t port_;
			std::string addr_ = "";
			std::string mac_ = "";
			int timeoutSeconds_;
			uint32_t id_;
			uint32_t code_;

			Whale::Basic::HTimer timer_;
			Whale::Basic::HDateTime loginTime_;

			struct bufferevent *bufferEvent_;
			Whale::Net::HTcpServer* server_;

			void* pvoid_ = nullptr;
			Whale::Basic::HByteArray buffer_;
			mutable std::mutex locker_;
		};

		class HTcpServer : public Whale::Basic::HThread, public std::enable_shared_from_this<HTcpServer>
		{
		public:
			explicit HTcpServer();
			virtual ~HTcpServer();

			virtual bool create(int port, int threads = 8);
			virtual void stop();
			virtual uint32_t sessions();
			virtual void detach(uint32_t id);
			virtual bool invoke(uint32_t id, std::function< bool(Whale::Net::HTcpSession*)> __function__);
			virtual bool request(std::shared_ptr<Whale::Net::HTcpPacket>& packet, int millseconds);
			virtual bool response(std::shared_ptr<Whale::Net::HTcpPacket>& packet);

			inline void enableCustom(bool val) {
				defaultProtocol_ = !val;
			}

			inline bool enableCustom(void) {
				return !defaultProtocol_;
			}

			inline uint32_t maxConnections() {
				return maxConnections_;
			}
			inline void maxConnections(uint32_t val) {
				maxConnections_ = val;
			}

		protected:
			virtual bool signalAccept(Whale::Net::HTcpSession* session) {
				return true;
			}

			virtual bool signalRead(Whale::Net::HTcpSession* session) {
				return true;
			}

			virtual bool signalWrite(Whale::Net::HTcpSession* session) {
				return true;
			}

			virtual void signalClose(Whale::Net::HTcpSession* session) {
				return;
			}

			virtual bool signalPacket(std::shared_ptr<Whale::Net::HTcpPacket>& packet) {
				return true;
			}

			inline uint32_t maxPacketLen() {
				return maxPacketLen_;
			}
			inline void maxPacketLen(uint32_t val) {
				maxPacketLen_ = val;
			}

			inline std::shared_ptr<Whale::Basic::HThreadPool> threadPool() {
				return threadPool_;
			}


		private:
			virtual void action();
			virtual bool attach(Whale::Net::HTcpSession* session);

			mutable std::mutex locker_;
			std::list<Whale::Net::HTcpSession*> sessions_;

			uint32_t maxConnections_;
			uint32_t maxPacketLen_;
			bool defaultProtocol_ = true;
			int evPort_;
			struct event_base *evbase_;
			struct evconnlistener *listener_;
			struct event *interruptEvt_;
			struct sockaddr_in evaddrin_;

			std::shared_ptr<Whale::Basic::HEventPool> eventPool_;
			std::shared_ptr<Whale::Basic::HThreadPool> threadPool_;

			static void acceptCallback(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *addr, int sockLen, void *pvoid);
			static void readCallback(struct bufferevent *bev, void *pvoid);
			static void writeCallback(struct bufferevent *bev, void *pvoid);
			static void eventCallback(struct bufferevent *bev, short events, void *pvoid);
			static void errorCallback(struct evconnlistener *listener, void *pvoid);

			DECLARE_FRIEND_CLASS(Whale::Net::HTcpSession)
		};
	}
}

#endif

