/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hhttpserver.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HHTTPSERVER_H
#define HHTTPSERVER_H

#include <event2/event.h>
#include <event2/thread.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>
#include <event2/http_struct.h>

#include "whale/basic/hthread.h"
#include "whale/basic/hdatetime.h"
#include "whale/util/hjson.h"
#include "whale/basic/hthreadpool.h"

namespace Whale
{
	namespace Net
	{
		class HHttpServer;

		class HHttpSession
		{
		public:
			explicit HHttpSession(Whale::Net::HHttpServer* httpServer) { 
				httpServer_ = httpServer; 
			};
			virtual ~HHttpSession() {};
			
			inline int  port() const { 
				return port_; 
			}
			inline void port(int val) {
				port_ = val; 
			}

			inline std::string addr() const { 
				return addr_; 
			}
			inline void addr(const std::string& val) { 
				addr_ = val; 
			}

			inline std::string id() const { 
				return id_; 
			}
			inline void id(const std::string& val) { 
				id_ = val; 
			}

			inline Whale::Basic::HTimer& timer() {
				return timer_;
			}
			inline Whale::Basic::HDateTime& loginTime() {
				return loginTime_;
			}

			inline Whale::Util::HJson& request() { 
				return request_; 
			}

			inline Whale::Net::HHttpServer* httpServer() const { 
				return httpServer_; 
			}

		private:
			int port_;
			std::string addr_ = "";
			std::string id_ = "";

			Whale::Util::HJson request_;

			Whale::Basic::HTimer timer_;
			Whale::Basic::HDateTime loginTime_;

			Whale::Net::HHttpServer* httpServer_;
		};

		class HHttpServer : public Whale::Basic::HThread, public std::enable_shared_from_this<HHttpServer>
		{
		public:
			explicit HHttpServer();
			virtual ~HHttpServer();			

			inline int port() const {
				return port_; 
			}

			inline void addr(std::string val) {
				addr_ = val;
			}
			inline std::string addr() const {
				return addr_;
			}

			inline int timeoutSeconds() const {
				return timeoutSeconds_; 
			}
			inline void timeoutSeconds(int val) {
				timeoutSeconds_ = val; 
			}

			inline void docRoot(std::string val) {
				docRoot_ = val;
			}
			inline std::string docRoot() const {
				return docRoot_; 
			}

			inline void home(std::string val) {
				home_ = val;
			}
			inline std::string home() const {
				return home_; 
			}

			inline std::shared_ptr<Whale::Basic::HThreadPool> threadPool() {
				return threadPool_;
			}

			virtual bool create(int port, const std::string& docRoot = "/opt/webapps", const std::string& home = "index.html", int threads = 8);

			virtual void stop();

			virtual void signalRequest(std::shared_ptr<Whale::Net::HHttpSession>& session) = 0;

		private:
			virtual void action();

			int port_;
			std::string addr_;
			int timeoutSeconds_;			
			std::string docRoot_;
			std::string home_;

			struct event_base *evbase_;
			struct evhttp *evhttp_;
			struct evhttp_bound_socket *evsock_;
			std::shared_ptr<Whale::Basic::HThreadPool> threadPool_;

			static void generalCallback(struct evhttp_request *request, void *pvoid);
		};
	}
}

#endif

