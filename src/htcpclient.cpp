#include "whale/net/htcpclient.h"

namespace Whale
{
	namespace Net
	{
		class HTcpClient;

		HDecodeThread::HDecodeThread(void)
		{
			name("HTCThread");
		}

		HDecodeThread::~HDecodeThread(void)
		{

		}

		bool HDecodeThread::init(Whale::Net::HTcpClient* tcpClient)
		{
			tcpClient_ = tcpClient;
			return true;
		}

		void HDecodeThread::action()
		{
			if (!tcpClient_) {
				return;
			}

			std::shared_ptr<Whale::Net::HTcpPacket> packet;

			if (tcpClient_->packets_.pop(packet)) {
				tcpClient_->signalPacket(packet);
			}
		}

		HTcpClient::HTcpClient(void)
		{
			name("HTcpClient");
			eventPool_ = std::make_shared<Whale::Basic::HEventPool>();
			tcpThread_ = std::make_shared<Whale::Net::HDecodeThread>();
		}

		HTcpClient::~HTcpClient(void)
		{
			tcpThread_->stop();
		}

		void HTcpClient::beforeRun()
		{
			tcpThread_->init(this);
			tcpThread_->intervalMissions(64);
			tcpThread_->run();
		}

		void HTcpClient::afterRun()
		{
			tcpThread_->stop();
		}

		bool HTcpClient::create(int type, const std::string& addr, int port, uint32_t identity, bool autoLogin) {
			identity_ = identity;
			return autoLogin ? 
				config(type, addr, port, [this]() -> void {
				auto packet = std::make_shared<Whale::Net::HTcpPacket>();
				packet->from(identity_);
				request(packet, 10 * 1000);
			}) : config(type, addr, port, [this]() -> void {});
		}

		bool HTcpClient::request(std::shared_ptr<Whale::Net::HTcpPacket>& packet, int millseconds)
		{
			packet->direction(0);

			if (!Whale::Net::HSocket::send(packet->encode())) {
				return false;
			}

			auto event = std::make_shared<Whale::Basic::HEvent>(packet->number());

			if (eventPool_->wait(event, millseconds))
			{
				packet->result(event->buffer());
				return true;
			}

			log_notice("TIMEOUT");

			return false;
		}

		bool HTcpClient::response(std::shared_ptr<Whale::Net::HTcpPacket>& packet)
		{
			packet->direction(1);
			return Whale::Net::HSocket::send(packet->encode());
		}

		bool HTcpClient::signalRead(Whale::Basic::HByteArray& bytes)
		{
			std::shared_ptr<Whale::Net::HTcpPacket> packet(new Whale::Net::HTcpPacket);

			while (packet->decode(bytes) > 0) {
				if (packet->direction() == 0) {
					packets_.push(packet);
				} 
				else {
					eventPool_->post(packet->number(), packet->buffer());
				}				
			}

			return true;
		}
	}
}
