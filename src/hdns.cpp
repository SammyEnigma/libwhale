#include "whale/net/hdns.h"
#include "whale/basic/hplatform.h"
#include "whale/util/hstring.h"

#ifndef   NI_MAXHOST
#define   NI_MAXHOST 1024
#endif

namespace Whale
{
	namespace Net
	{
		namespace HDns
		{
			std::string mac(const std::string& ipaddr)
			{
				std::string macAddr = "ff-ff-ff-ff-ff-ff";
#ifdef OS_WIN32
				unsigned long macLen = 6;
				uint8_t macAddress[6] = { 0 };

				if (SendARP(inet_addr(ipaddr.c_str()), 
					(unsigned long)NULL, 
					(PULONG)&macAddress, &macLen) == NO_ERROR) {
					macAddr = Whale::Util::HString::format("%02x-%02x-%02x-%02x-%02x-%02x",
						macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
				}
#else

				int     sockfd;
				unsigned char *ptr;
				struct arpreq arpreq;
				struct sockaddr_in *sin;
				struct sockaddr_storage ss;
				char addr[INET_ADDRSTRLEN+1];

				memset(addr, 0, INET_ADDRSTRLEN+1);
				memset(&ss, 0, sizeof(ss));
				sockfd = socket(AF_INET, SOCK_DGRAM, 0);
				if (sockfd == -1) {
					return macAddr;
				}
				sin = (struct sockaddr_in *) &ss;
				sin->sin_family = AF_INET;
				if (inet_pton(AF_INET, ipaddr.c_str(), &(sin->sin_addr)) <= 0) {
					return macAddr;
				}
				sin = (struct sockaddr_in *) &arpreq.arp_pa;
				memcpy(sin, &ss, sizeof(struct sockaddr_in));
				strcpy(arpreq.arp_dev, "eth0");
				arpreq.arp_ha.sa_family = AF_UNSPEC;
				if (ioctl(sockfd, SIOCGARP, &arpreq) < 0) {
					return macAddr;
				}
				ptr = (unsigned char *)arpreq.arp_ha.sa_data;
				macAddr = Whale::Util::HString::format("%02x-%02x-%02x-%02x-%02x-%02x",
					*ptr, *(ptr + 1), *(ptr + 2), *(ptr + 3), *(ptr + 4), *(ptr + 5));
#endif // OS_WIN32
				return macAddr;
			}

			std::vector<std::string> addrs(const std::string& domain)
			{
				std::vector<std::string> addrs;

				struct addrinfo *dns;

				if (getaddrinfo(domain.c_str(), NULL, NULL, &dns) != 0)
				{
					return addrs;
				}

				char addr[NI_MAXHOST + 1] = { 0 };

				for (struct addrinfo * res = dns; res != NULL; res = res->ai_next)
				{
					memset(addr, 0, NI_MAXHOST + 1);

					switch (res->ai_addr->sa_family)
					{
					case AF_INET:
						inet_ntop(AF_INET, &(((struct sockaddr_in *)res->ai_addr)->sin_addr), addr, NI_MAXHOST);
						break;

					case AF_INET6:
						inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)res->ai_addr)->sin6_addr), addr, NI_MAXHOST);
						break;

					default:
						break;
					}

					if (strlen(addr) > 0)
					{
						addrs.push_back(addr);
					}
				}

				freeaddrinfo(dns);

				return addrs;
			}

			std::vector<std::string> hosts(const std::string& domain)
			{
				std::vector<std::string> hostnames;

				struct addrinfo *dns;

				if (getaddrinfo(domain.c_str(), NULL, NULL, &dns) != 0)
				{
					return hostnames;
				}

				for (struct addrinfo * res = dns; res != NULL; res = res->ai_next)
				{
					char hostname[NI_MAXHOST] = "";

					if (getnameinfo(res->ai_addr, res->ai_addrlen, hostname, NI_MAXHOST, NULL, 0, 0) != 0)
					{
						continue;
					}

					if (!std::string(hostname).empty())
					{
						hostnames.push_back(hostname);
					}
				}

				freeaddrinfo(dns);

				return hostnames;
			}
		}
	}
}
