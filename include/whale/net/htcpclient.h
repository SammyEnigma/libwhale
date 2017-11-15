/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	htcpclient.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Replace Version	:	1.0
* Original Author	:	Mr Li
* Finished Date		:	2017/05/05 09:58:00
****************************************************************************************/
#ifndef HTCPCLIENT_H
#define HTCPCLIENT_H

#include "whale/net/hsocket.h"
#include "whale/net/htcppacket.h"
#include "whale/basic/heventpool.h"
#include "whale/basic/hthreadsafelist.h"

namespace Whale
{
	namespace Net
	{
		class HTcpClient;

		class HDecodeThread : public Whale::Basic::HThread
		{
		public:
			explicit HDecodeThread(void);
			virtual ~HDecodeThread(void);

			virtual bool init(Whale::Net::HTcpClient* tcpClient);

		protected:
			virtual void action();

		private:
			Whale::Net::HTcpClient* tcpClient_;
		};

		class HTcpClient : public Whale::Net::HSocket
		{
			DECLARE_FRIEND_CLASS(HDecodeThread)

		public:
			explicit HTcpClient(void);
			virtual ~HTcpClient(void);

			virtual bool create(int type = Whale::Net::HSocket::TCP,
				const std::string& addr = "127.0.0.1",
				int port = 9999, uint32_t identity = 0, bool autoLogin = true);

			virtual bool request(std::shared_ptr<Whale::Net::HTcpPacket>& packet, int millseconds);
			virtual bool response(std::shared_ptr<Whale::Net::HTcpPacket>& packet);

			virtual void beforeRun();
			virtual void afterRun();

			inline void identity(uint32_t val) {
				identity_ = val;
			}

		protected:
			virtual bool signalPacket(std::shared_ptr<Whale::Net::HTcpPacket>& packet) = 0;

		private:			
			virtual bool signalRead(Whale::Basic::HByteArray& bytes);

			uint32_t identity_;
			std::shared_ptr<Whale::Basic::HEventPool> eventPool_;
			std::shared_ptr<Whale::Net::HDecodeThread> tcpThread_;
			Whale::Basic::HThreadSafeList<std::shared_ptr<Whale::Net::HTcpPacket>> packets_;
		};
	}
}

#endif

