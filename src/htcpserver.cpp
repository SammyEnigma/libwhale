#include "whale/net/htcpserver.h"
#include "whale/basic/hlog.h"
#include "whale/basic/hos.h"
#include "whale/util/hstring.h"
#include "whale/util/huuid.h"
#include "whale/net/hdns.h"
#include "whale/util/hrandom.h"
#include "whale/basic/hplatform.h"

#ifdef OS_WIN32
#pragma comment(lib, "z:/libwhale/deps/libevent-2.1.8-stable/lib/libevent.lib")
#pragma comment(lib, "z:/libwhale/deps/libevent-2.1.8-stable/lib/libevent_core.lib")
#pragma comment(lib, "z:/libwhale/deps/libevent-2.1.8-stable/lib/libevent_extras.lib")
#endif


namespace Whale
{
	namespace Net
	{
		HTcpSession::HTcpSession(Whale::Net::HTcpServer* tcpServer, struct bufferevent *bufferEvent)
		{
#ifdef OS_WIN32
			// Initialize the Winsock dll version 2.0
			WSADATA  wsaData = { 0 };
			if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
				std::cout << "Init Windows Socket Failed !! ERR: " << GetLastError() << std::endl;
			}
#endif
			assert(tcpServer);
			assert(bufferEvent);

			server_ = tcpServer;
			bufferEvent_ = bufferEvent;
			id_ = Whale::Util::HRandom::gen();
			code_ = 0;
		}

		HTcpSession::~HTcpSession()
		{
			if (bufferEvent_) {
				bufferevent_lock(bufferEvent_);
				bufferevent_free(bufferEvent_);
				bufferevent_unlock(bufferEvent_);
				bufferEvent_ = NULL;
			}
		}

		int HTcpSession::write(const Whale::Basic::HByteArray& byteArray)
		{
			if (!bufferEvent_)
				return -1;

			bufferevent_lock(bufferEvent_);
			int res = bufferevent_write(bufferEvent_, byteArray.data(), byteArray.size());
			bufferevent_unlock(bufferEvent_);

			return res;
		}

		HTcpServer::HTcpServer() : threadPool_(new Whale::Basic::HThreadPool())
		{
			maxConnections_ = 8192;
			maxPacketLen_ = 1024 * 16;
			eventPool_ = std::make_shared<Whale::Basic::HEventPool>();
		}

		HTcpServer::~HTcpServer()
		{
			threadPool_->stop();
		}

		bool HTcpServer::request(std::shared_ptr<Whale::Net::HTcpPacket>& packet, int millseconds)
		{
			packet->direction(0);

			if (!invoke(packet->to(),
				std::bind([&](Whale::Net::HTcpSession* session,
					std::shared_ptr<Whale::Net::HTcpPacket>& packet) -> bool {
				return session->write(packet->encode()) == 0;
			}, std::placeholders::_1, packet))) {
				std::cout << "WRITE ERROR: " << packet->to() << std::endl;
				return false;
			}

			auto eventObj = std::make_shared<Whale::Basic::HEvent>(packet->number());

			if (eventPool_->wait(eventObj, millseconds))
			{
				packet->result(eventObj->buffer());
				return true;
			}

			log_notice("Timeout");

			return false;
		}

		bool HTcpServer::response(std::shared_ptr<Whale::Net::HTcpPacket>& packet)
		{
			packet->direction(1);
			return  invoke(packet->session().id(), std::bind([&](Whale::Net::HTcpSession* session,
				std::shared_ptr<Whale::Net::HTcpPacket>& packet) -> bool {
				return session->write(packet->encode()) == 0;
			}, std::placeholders::_1, packet));
		}

		bool HTcpServer::create(int port, int threads)
		{
			evPort_ = port;

			onlyOnceRecyle(true);

			evbase_ = event_base_new();

			if (!evbase_) {
				return false;
			}

			memset(&evaddrin_, 0, sizeof(evaddrin_));
			evaddrin_.sin_family = AF_INET;
			evaddrin_.sin_port = htons(evPort_);

			listener_ = evconnlistener_new_bind(
				evbase_,
				HTcpServer::acceptCallback,
				this,
				LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE | LEV_OPT_THREADSAFE,
				-1,
				(struct sockaddr*)&evaddrin_,
				sizeof(evaddrin_));

			if (!listener_) {
				return false;
			}

			int enable = 1;
			struct linger ling = { 0, 0 };
			setsockopt(evconnlistener_get_fd(listener_), SOL_SOCKET, SO_REUSEADDR, (const char *)&enable, sizeof(enable));
			setsockopt(evconnlistener_get_fd(listener_), SOL_SOCKET, SO_KEEPALIVE, (const char *)&enable, sizeof(enable));
			setsockopt(evconnlistener_get_fd(listener_), SOL_SOCKET, SO_LINGER, (const char *)&ling, sizeof(ling));
			evutil_make_socket_nonblocking(evconnlistener_get_fd(listener_));
			evconnlistener_set_error_cb(listener_, HTcpServer::errorCallback);

			std::vector<std::string> addrs = Whale::ipAddress();

			if (addrs.empty())
			{
				log_notice("Can't catch local address, Please checking !!");
				exit(-1);
			}

			log_notice("Tcp server [%s:%d] started !!", addrs[0].c_str(), evPort_);

			return threadPool_->init(threads);
		}

		void HTcpServer::stop()
		{
			detach(0);
			event_base_loopexit(evbase_, NULL);
			evconnlistener_free(listener_);
			event_base_free(evbase_);
			Whale::Basic::HThread::stop();
			log_notice("Exited !!");
		};

		void HTcpServer::action()
		{
#ifdef OS_WIN32
			evthread_use_windows_threads();
#else
			evthread_use_pthreads();
#endif
			evthread_make_base_notifiable(evbase_);

			event_base_dispatch(evbase_);
		};

		bool HTcpServer::attach(Whale::Net::HTcpSession* session)
		{
			std::lock_guard <std::mutex> lock(locker_);

			if (sessions_.size() > maxConnections_) {
				log_notice("Maximum number of connections reached !! [%u].", maxConnections_);
				return false;
			}

			sessions_.push_back(session);

			log_notice("Attach [%s:%d][%d][%s], Current sessions: [%d].",
				session->addr().c_str(), session->port(), session->fd(),
				session->mac().c_str(), sessions_.size());

			return signalAccept(session);
		}

		uint32_t HTcpServer::sessions()
		{
			std::lock_guard <std::mutex> lock(locker_);
			return sessions_.size();
		}

		void HTcpServer::detach(uint32_t id)
		{
			std::lock_guard <std::mutex> lock(locker_);

			for (auto session : sessions_) {
				if (id < 0) {
					signalClose(session);
					sessions_.remove(session);

					log_notice("Detach [%s:%d][%d][%s], Current sessions: [%d].",
						session->addr().c_str(), session->port(), session->fd(),
						session->mac().c_str(), sessions_.size());

					delete session;
					session = 0;
					continue;
				}

				if (session->id() == id) {
					signalClose(session);
					sessions_.remove(session);

					log_notice("Detach [%s:%d][%d][%s], Current sessions: [%d].",
						session->addr().c_str(), session->port(), session->fd(),
						session->mac().c_str(), sessions_.size());

					delete session;
					return;
				}
			}
		}

		bool HTcpServer::invoke(uint32_t id, std::function< bool(Whale::Net::HTcpSession*)> __function__)
		{
			std::lock_guard <std::mutex> lock(locker_);

			for (auto session : sessions_) {
				if (id < 0) {
					if (__function__(session))
						continue;
					else
						return false;
				}

				if (session->id() == id) {
					return __function__(session);
				}
			}

			return id < 0 ? true : false;
		}

		void HTcpServer::acceptCallback(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *addr, int sockLen, void *pvoid)
		{
			assert(pvoid);
			Whale::Net::HTcpServer *server = static_cast<Whale::Net::HTcpServer*>(pvoid);

			int enable = 1;
			struct linger ling = { 0, 0 };
			setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&enable, sizeof(enable));
			setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (const char *)&enable, sizeof(enable));
			setsockopt(fd, SOL_SOCKET, SO_LINGER, (const char *)&ling, sizeof(ling));
			evutil_make_socket_nonblocking(fd);

			struct bufferevent *bufferEvent;

			bufferEvent = bufferevent_socket_new(server->evbase_, fd,
				BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE | BEV_OPT_UNLOCK_CALLBACKS | BEV_OPT_DEFER_CALLBACKS);

			if (!bufferEvent) {
				event_base_loopbreak(server->evbase_);
				log_error("Function bufferevent_socket_new failure !!");
				return;
			}

			Whale::Net::HTcpSession* session = new Whale::Net::HTcpSession(server, bufferEvent);

			session->port(((struct sockaddr_in *)addr)->sin_port);
			session->addr(inet_ntoa(((struct sockaddr_in *)addr)->sin_addr));
			session->mac(Whale::Net::HDns::mac(session->addr()));

			bufferevent_setcb(bufferEvent, HTcpServer::readCallback, HTcpServer::writeCallback, HTcpServer::eventCallback, session);
			bufferevent_enable(bufferEvent, EV_WRITE | EV_READ | EV_PERSIST);

			if (!server->attach(session)) {
				server->detach(session->id());
			}
		}

		void HTcpServer::writeCallback(struct bufferevent *bufferEvent, void *pvoid)
		{
			assert(pvoid);
			Whale::Net::HTcpSession* session = static_cast<Whale::Net::HTcpSession*>(pvoid);
			evbuffer_get_length(bufferevent_get_output(bufferEvent)) == 0 ?
				session->server()->signalWrite(session) : true;
		}

		void HTcpServer::readCallback(struct bufferevent *bufferEvent, void *pvoid)
		{
			bufferevent_lock(bufferEvent);

			assert(pvoid);
			Whale::Net::HTcpSession *session = static_cast<Whale::Net::HTcpSession *>(pvoid);

			int size = evbuffer_get_length(bufferevent_get_input(bufferEvent));

			if (size <= 0) {
				bufferevent_unlock(bufferEvent);
				session->server()->detach(session->id());
				return;
			}

			Whale::Basic::HByteArray buffer(size);
			size = bufferevent_read(bufferEvent, buffer.data(), size);
			session->buffer() += buffer;

			bufferevent_unlock(bufferEvent);

			auto p = std::make_shared<Whale::Net::HTcpPacket>();

			// 1、检测协议头部和默认协议是否一致
			// 2、检测是否启用了自定义协议， 若满足了1-2条件则进行自定义协议的匹配。
			if (session->buffer().copy<decltype(p->head())>() != p->head() &&
				!session->server()->defaultProtocol_) {

				bool result = session->server()->signalRead(session);

				if (!result) { 
					session->server()->detach(session->id());
				}

				return;
			}

			session->server()->threadPool()->assign(
				std::bind([=](Whale::Net::HTcpServer* tcpServer, uint32_t id) -> bool {

				std::list<std::shared_ptr<Whale::Net::HTcpPacket>> packets;

				tcpServer->invoke(id, std::bind([&](Whale::Net::HTcpSession* pSess) -> bool {
					auto packet = std::make_shared<Whale::Net::HTcpPacket>();

					while (packet->decode(pSess->buffer()) > 0) {

						if (packet->direction() == 0) {

							if (packet->from() != pSess->code()) {
								packet->first(true);
								pSess->code(packet->from());
							}

							packet->session().id(pSess->id());
							packet->session().addr(pSess->addr());
							packet->session().port(pSess->port());
							packet->session().mac(pSess->mac());
							packet->session().code(pSess->code());

							packets.push_back(packet);
							//tcpServer->signalPacket(packet);
						}
						else {
							tcpServer->eventPool_->post(packet->number(), packet->buffer());
						}
					}
					return true;
				}, std::placeholders::_1));

				for (auto packet : packets)	{
					tcpServer->signalPacket(packet);
				}

				return true;
			}, session->server(), session->id()));

			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////

			//bufferevent_lock(bufferEvent);
			//assert(pvoid);
			//Whale::Net::HTcpSession* session = static_cast<Whale::Net::HTcpSession*>(pvoid);			

			//int size = evbuffer_get_length(bufferevent_get_input(bufferEvent));
			//
			//if (size == 0) {
			//	bufferevent_unlock(bufferEvent);
			//	session->server()->detach(session->id());
			//	return;
			//}

			//Whale::Basic::HByteArray buffer(size);
			//size = bufferevent_read(bufferEvent, buffer.data(), size);
			//session->buffer() += buffer;

			//std::shared_ptr<Whale::Net::HTcpPacket> packet;
			//packet = std::make_shared<Whale::Net::HTcpPacket>();			

			//// 1、检测协议头部和默认协议是否一致
			//// 2、检测是否启用了自定义协议， 若满足了1-2条件则进行自定义协议的匹配。
			//if (session->buffer().copy<decltype(packet->head())>() != packet->head()
			//	&& !session->server()->defaultProtocol_) {
			//	// 自定义协议匹配
			//	int res = session->server()->signalRead(session);
			//	bufferevent_unlock(bufferEvent);
			//	if (!res) {					
			//		session->server()->detach(session->id());
			//	}				

			//	return;
			//}
			//
			//while (packet->decode(session->buffer()) > 0) {
			//	if (packet->direction() == 0) {

			//		// 判断是否为该连接的第一个包
			//		if (packet->from() != session->code()) {
			//			packet->first(true);
			//			session->code(packet->from());
			//		}					

			//		packet->session().id(session->id());
			//		packet->session().addr(session->addr());
			//		packet->session().port(session->port());
			//		packet->session().mac(session->mac());
			//		packet->session().code(session->code());
			//		
			//		session->server()->packets_.push(packet);
			//	} else {
			//		session->server()->eventPool_->post(packet->number(), packet->buffer());
			//	}
			//}

			//bufferevent_unlock(bufferEvent);
		}

		void HTcpServer::eventCallback(struct bufferevent *bufferEvent, short events, void *pvoid)
		{
			assert(pvoid);
			Whale::Net::HTcpSession* session = static_cast<Whale::Net::HTcpSession*>(pvoid);

			if ((events & BEV_EVENT_EOF) || (events & BEV_EVENT_ERROR) || (events & BEV_EVENT_READING)
				|| (events & BEV_EVENT_WRITING) || (events & BEV_EVENT_TIMEOUT)) {
				log_notice("EVENT [session://%s:%d][%d], INFO: [%d] %s %s %s %s %s",
					session->addr().c_str(),
					session->port(),
					session->fd(),
					EVUTIL_SOCKET_ERROR(),
					(events & BEV_EVENT_EOF) ? "BEV_EVENT_EOF" : "",
					(events & BEV_EVENT_ERROR) ? "BEV_EVENT_ERROR" : "",
					(events & BEV_EVENT_READING) ? "BEV_EVENT_READING" : "",
					(events & BEV_EVENT_WRITING) ? "BEV_EVENT_WRITING" : "",
					(events & BEV_EVENT_TIMEOUT) ? "BEV_EVENT_TIMEOUT" : "");

				return session->server()->detach(session->id());

				session->server()->threadPool()->assign(
				std::bind([=](
					Whale::Net::HTcpServer* tcpServer, 
					uint32_t id) -> void {
					return tcpServer->detach(id);
				}, session->server(), session->id()));
			}
		}

		void HTcpServer::errorCallback(struct evconnlistener *listener, void *pvoid)
		{
			assert(pvoid);
			Whale::Net::HTcpServer* serverObj = static_cast<Whale::Net::HTcpServer*>(pvoid);
			log_error("Err: %s", evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
			event_base_loopexit(evconnlistener_get_base(listener), NULL);
		}
	}
}
