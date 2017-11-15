/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hsocket.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HSOCKET_H
#define HSOCKET_H

#include "whale/basic/hplatform.h"
#include "whale/basic/hlog.h"
#include "whale/basic/hbytearray.h"

namespace Whale 
{
	namespace Net
	{
		class HSocket : public Whale::Basic::HThread
		{
		public:
			enum { TCP, UDP };

		public:
			explicit HSocket();
			virtual ~HSocket();

			virtual bool config(
				int type = Whale::Net::HSocket::TCP, 
				const std::string& addr = "127.0.0.1", 
				int port = 9999,
				std::function< void()> connectFunctional = [=]()->void {},
				std::function< void()> releaseFunctional = [=]()->void {});

			virtual bool connect();
			virtual bool send(const Whale::Basic::HByteArray& bytes);
			virtual bool broadcast(const Whale::Basic::HByteArray& bytes);			
			virtual bool signalRead(Whale::Basic::HByteArray& bytes) = 0;

			virtual bool noneblock(long val = 1);
			virtual bool reuseaddr(long val = 1);
			virtual bool resizebuff(long val = 1024 * 8);
			virtual bool sendtimeval(long tv = 1000 * 1);
			virtual bool recvtimeval(long tv = 1000 * 1);
			virtual bool setsockopt(int key, const void* val, int len);		
			virtual void release();
			virtual void stop();

		private:
			virtual void action();

			bool inited_ = false;
			bool connected_ = false;

			int sock_ = -1;
			int type_ = Whale::Net::HSocket::TCP;

			int port_;
			std::string addr_;

			Whale::Basic::HByteArray buffer_;

			std::function< void()> connectFunctional_;
			std::function< void()> releaseFunctional_;
		};
	}
}

#endif
