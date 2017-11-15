#include "whale/basic/hos.h"
#include "whale/basic/hlog.h"
#include "whale/util/hstring.h"

#ifdef OS_LINUX
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <scsi/sg.h>
#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <string>
#include <fstream>
#endif

#pragma warning(disable : 4244)

#ifdef _MSC_VER
#include <Windows.h>
#include <VersionHelpers.h>

std::string GetSystemName()
{
	std::string osname;

	if (IsWindows8Point1OrGreater()) {
		osname = "Microsoft Windows 8 Point 1";
	}
	else if (IsWindows8OrGreater()) {
		osname = "Microsoft Windows 8";
	}
	else if (IsWindows7SP1OrGreater()) {
		osname = "Microsoft Windows 7 SP1";
	}
	else if (IsWindows7OrGreater()) {
		osname = "Microsoft Windows 7";
	}
	else if (IsWindowsVistaSP2OrGreater()) {
		osname = "Microsoft Windows Vista SP2";
	}
	else if (IsWindowsVistaSP1OrGreater()) {
		osname = "Microsoft Windows Vista SP1";
	}
	else if (IsWindowsVistaOrGreater()) {
		osname = "Microsoft Windows Vista";
	}
	else if (IsWindowsXPSP3OrGreater()) {
		osname = "Microsoft Windows XP SP3";
	}
	else if (IsWindowsXPSP2OrGreater()) {
		osname = "Microsoft Windows XP SP2";
	}
	else if (IsWindowsXPSP1OrGreater()) {
		osname = "Microsoft Windows XP SP1";
	}
	else if (IsWindowsXPOrGreater()) {
		osname = "Microsoft Windows XP";
	}

	if (IsWindowsServer())
	{
		osname += " Server";
	}

	return osname;
}
#else
#include <sys/utsname.h>
#endif



namespace Whale
{
	double power(unsigned int base, unsigned int expo)
	{
		return (expo == 0) ? 1 : base * power(base, expo - 1);
	}

	/* idea of this function is copied from top size scaling */
	std::string scaleSize(unsigned long size)
	{
		static char nextup[] = { 'B', 'K', 'M', 'G', 'T', 0 };
		char buf[BUFSIZ];
		int i;
		char *up;
		float base;

		base = 1024.0;

		/* human readable output */
		up = nextup;
		for (i = 1; up[0] != '0'; i++, up++) {
			switch (i) {
			case 1:
				if (4 >= snprintf(buf, sizeof(buf), "%ld%c", (long)size * 1024, *up))
					return buf;
				break;
			case 2:
			{
				if (4 >= snprintf(buf, sizeof(buf), "%ld%c", size, *up))
					return buf;
			}
			break;
			case 3:
			case 4:
			case 5:
				if (4 >=
					snprintf(buf, sizeof(buf), "%.1f%c",
					(float)(size / power(base, i - 2)), *up))
					return buf;
				if (4 >=
					snprintf(buf, sizeof(buf), "%ld%c",
					(long)(size / power(base, i - 2)), *up))
					return buf;
				break;
			case 6:
				break;
			}
		}
		/*
		 * On system where there is more than petabyte of memory or swap the
		 * output does not fit to column. For incoming few years this should
		 * not be a big problem (wrote at Apr, 2011).
		 */
		return std::string(buf);
	}

	bool startService(const std::string& app)
	{
#ifdef OS_WIN32
		std::string output = Whale::process("sc query %s", app.c_str());

		if (output.find("1060") != std::string::npos) {
			return true;
		}

		if (output.find("RUNNING") == std::string::npos) {
			output = Whale::process("sc start %s", app.c_str());
			log_info("Service start [%s] %s", app.c_str(), output.c_str());
			return true;
		}
#endif // OS_WIN32

#ifdef OS_LINUX
		std::string output = Whale::process("systemctl status %s | grep running", app.c_str());
		if (output.find("running") == std::string::npos) {
			output = Whale::process("systemctl start %s", app.c_str());
			log_info("Service [%s] %s", app.c_str(), output.c_str());
			return true;
		}
#endif // OS_LINUX

		return false;
	}

	bool stopService(const std::string& app)
	{
#ifdef OS_WIN32
		std::string output = Whale::process("sc query %s", app.c_str());

		if (output.find("1060") != std::string::npos) {
			return true;
		}

		if (output.find("RUNNING") != std::string::npos) {			
			output = Whale::process("sc stop %s", app.c_str());
			log_info("STOP: [%s], OUTPUT: %s.", app.c_str(), output.c_str());

			if (output.find("STOPPED") != std::string::npos) {
				return true;
			}			
		}

		Whale::process("tskill %s", app.c_str());

		output = Whale::process("sc query %s", app.c_str());
		log_info("STOP: [%s], OUTPUT: %s.", app.c_str(), output.c_str());

		if (output.find("STOPPED") != std::string::npos) {
			return true;
		}

#endif // OS_WIN32

#ifdef OS_LINUX
		std::string output = Whale::process("systemctl status %s | grep running", app.c_str());
		if (output.find("running") != std::string::npos) {
			Whale::process("systemctl stop %s", app.c_str());

			output = Whale::process("systemctl status %s | grep dead", app.c_str());
			log_info("STOP: [%s], OUTPUT: %s.", app.c_str(), output.c_str());

			if (output.find("dead") != std::string::npos) {
				return true;
			}
		}

		Whale::process("killall -9 %s", app.c_str());

		output = Whale::process("systemctl status %s | grep dead", app.c_str());
		log_info("STOP: [%s], OUTPUT: %s.", app.c_str(), output.c_str());

		if (output.find("dead") != std::string::npos) {
			return true;
		}
#endif // OS_LINUX

		return false;
	}

	bool littleEndian()
	{
		union {
			unsigned int  a;
			unsigned char b;
		} c;
		c.a = 1;
		return 1 == c.b;
	}

	uint16_t htons_(uint16_t A)
	{
		if (Whale::littleEndian()) {
			return ((((uint16_t)(A) & 0xff00) >> 8) | (((uint16_t)(A) & 0x00ff) << 8));
		}
		return A;
	}

	uint32_t htonl_(uint32_t A)
	{
		if (Whale::littleEndian()) {
			return ((((uint32_t)(A) & 0xff000000) >> 24) | (((uint32_t)(A) & 0x00ff0000) >> 8) | (((uint32_t)(A) & 0x0000ff00) << 8) | (((uint32_t)(A) & 0x000000ff) << 24));
		}
		return A;
	}

	uint32_t ipToInt(const std::string& val)
	{
		if (val.empty()) {
			return 0;
		}

		std::vector<std::string> addrs = Whale::Util::HString::split(val, ".");

		if (addrs.size() != 4) {
			return 0;
		}

		uint8_t ips[4] = { 0 };
		ips[0] = Whale::Util::HString::convert<uint32_t>(addrs[0]);
		ips[1] = Whale::Util::HString::convert<uint32_t>(addrs[1]);
		ips[2] = Whale::Util::HString::convert<uint32_t>(addrs[2]);
		ips[3] = Whale::Util::HString::convert<uint32_t>(addrs[3]);

		return ips[0] << 24 | ips[1] << 16 | ips[2] << 8 | ips[3] << 0;
	}

	std::string intToIp(uint32_t val)
	{
		return Whale::Util::HString::format("%u.%u.%u.%u",
			uint8_t(val >> 24), uint8_t(val >> 16), uint8_t(val >> 8), uint8_t(val >> 0));
	}

	const std::string appName()
	{
		char nameBuff[MAX_PATH + 1] = { 0 };

#ifdef OS_WIN32

		GetModuleFileName(0, nameBuff, MAX_PATH);

		char *ptr = strrchr(nameBuff, '\\') + 1;
		*(strrchr(ptr, '.')) = 0;

		memcpy(nameBuff, ptr, strlen(ptr));
		nameBuff[strlen(ptr)] = '\0';

#else

		if (-1 == readlink("/proc/self/exe", nameBuff, MAX_PATH))
		{
			return "";
		}

		char *ptr = strrchr(nameBuff, '/') + 1;

		memcpy(nameBuff, ptr, strlen(ptr));
		nameBuff[strlen(ptr)] = '\0';

#endif

		return std::string(nameBuff);
	}

	const std::string currentDir()
	{
		char appname[MAX_PATH + 1] = { 0 };

#ifdef OS_WIN32

		GetModuleFileName(0, appname, MAX_PATH);

		*(strrchr(appname, '\\') + 1) = 0;

#else

		if (-1 == readlink("/proc/self/exe", appname, MAX_PATH))
		{
			return "";
		}

		*(strrchr(appname, '/') + 1) = 0;

#endif

		return Whale::Util::HString::replace(appname, "\\", "/");
	}

	void sleep(int millisecs)
	{
#ifdef OS_WIN32
		Sleep(millisecs);
#else
		usleep(millisecs * 1000);
#endif
	}

	std::string hostName()
	{
		char hostname[MAX_PATH + 1] = { 0 };
		gethostname(hostname, MAX_PATH);
		return std::string(hostname);

	}

	std::vector<std::string> ipAddress()
	{
		std::vector<std::string> addrs;

#ifdef OS_WIN32
		PHOSTENT hostinfo;
		char name[255] = { 0 };

		if (gethostname(name, sizeof(name)) == 0 && (hostinfo = gethostbyname(name)) != NULL)
		{
			for (int i = 0; hostinfo != NULL && hostinfo->h_addr_list[i] != NULL; ++i)
			{
				addrs.push_back(inet_ntoa(*(struct in_addr *)hostinfo->h_addr_list[i]));
			}
		}

#else
		int fd;
		int itr;
		int retn = 0;

		int  maxifr = 16;
		struct ifreq ifr[maxifr];

		struct ifconf ifc;

		if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
		{
			ifc.ifc_len = sizeof(ifr);
			ifc.ifc_buf = (caddr_t)ifr;

			if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc))
			{
				itr = ifc.ifc_len / sizeof(struct ifreq);

				while (itr-- > 0)
				{
					if (!(ioctl(fd, SIOCGIFADDR, (char *)&ifr[itr])))
					{
						addrs.push_back((inet_ntoa(((struct sockaddr_in*)(&ifr[itr].ifr_addr))->sin_addr)));
					}
				}
			}

			close(fd);
		}
#endif

		if (addrs.empty()) {
			addrs.push_back("127.0.0.1");
		}

		return addrs;
	}

	std::vector<std::string> macAddress()
	{
		std::vector<std::string> macs;
		char mac[256] = { 0 };

#ifdef OS_WIN32
		PIP_ADAPTER_INFO adapter;

		unsigned long adaptersize = sizeof(IP_ADAPTER_INFO);
		adapter = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));

		if (GetAdaptersInfo(adapter, &adaptersize) == ERROR_BUFFER_OVERFLOW)
		{
			free(adapter);
			adapter = (IP_ADAPTER_INFO *)malloc(adaptersize);
		}

		if (GetAdaptersInfo(adapter, &adaptersize) == NO_ERROR)
		{
			PIP_ADAPTER_INFO at = adapter;
			while (at) {
				macs.push_back(Whale::Util::HString::format("%02%02x%02x%02x%02x%02x",
					at->Address[0], at->Address[1], at->Address[2], at->Address[3], at->Address[4], at->Address[5]));
				at = at->Next;
			}
		}
#else

		unsigned int ifsize = 0;
		char buf[1024];
		char szbuff[16];
		struct sockaddr_in *pAddr;
		int i, j;
		int ok = 0;

		int fd = socket(AF_INET, SOCK_DGRAM, 0);

		if (fd > 0)
		{
			struct ifconf ifc;
			ifc.ifc_len = sizeof(buf);
			ifc.ifc_buf = buf;
			ioctl(fd, SIOCGIFCONF, &ifc);

			ifsize = ifc.ifc_len / sizeof(struct ifreq);

			struct ifreq* ift = ifc.ifc_req;

			for (i = 0; i < ifsize; i++, ift++)
			{
				struct ifreq ifr;
				strcpy(ifr.ifr_name, ift->ifr_name);

				if (ioctl(fd, SIOCGIFFLAGS, &ifr) == 0 && !(ifr.ifr_flags & IFF_LOOPBACK) && ioctl(fd, SIOCGIFHWADDR, &ifr) == 0)
				{
					macs.push_back(Whale::Util::HString::format("%02x%02x%02x%02x%02x%02x",
						(unsigned char)ifr.ifr_hwaddr.sa_data[0],
						(unsigned char)ifr.ifr_hwaddr.sa_data[1],
						(unsigned char)ifr.ifr_hwaddr.sa_data[2],
						(unsigned char)ifr.ifr_hwaddr.sa_data[3],
						(unsigned char)ifr.ifr_hwaddr.sa_data[4],
						(unsigned char)ifr.ifr_hwaddr.sa_data[5]));
				}
			}

			close(fd);
		}
#endif

		if (macs.empty()) {
			macs.push_back("000000000000");
		}

		return macs;
	}

#ifdef OS_LINUX
	static bool get_disk_name(std::string & disk_name)
	{
		disk_name.c_str();

		std::ifstream ifs("/etc/mtab", std::ios::binary);
		if (!ifs.is_open())
		{
			return(false);
		}

		char line[4096] = { 0 };
		while (!ifs.eof())
		{
			ifs.getline(line, sizeof(line));
			if (!ifs.good())
			{
				break;
			}

			const char * disk = line;
			while (isspace(disk[0]))
			{
				++disk;
			}

			const char * space = strchr(disk, ' ');
			if (NULL == space)
			{
				continue;
			}

			const char * mount = space + 1;
			while (isspace(mount[0]))
			{
				++mount;
			}
			if ('/' != mount[0] || ' ' != mount[1])
			{
				continue;
			}

			while (space > disk && isdigit(space[-1]))
			{
				--space;
			}

			if (space > disk)
			{
				std::string(disk, space).swap(disk_name);
				break;
			}
		}

		ifs.close();

		return(!disk_name.empty());
	}

	static void trim_serial(const void * serial, size_t serial_len, std::string & serial_no)
	{
		const char * serial_s = static_cast<const char *>(serial);
		const char * serial_e = serial_s + serial_len;
		while (serial_s < serial_e)
		{
			if (isspace(serial_s[0]))
			{
				++serial_s;
			}
			else if ('\0' == serial_e[-1] || isspace(serial_e[-1]))
			{
				--serial_e;
			}
			else
			{
				break;
			}
		}

		if (serial_s < serial_e)
		{
			std::string(serial_s, serial_e).swap(serial_no);
		}
	}

	static bool get_disk_serial_by_way_1(const std::string & disk_name, std::string & serial_no)
	{
		serial_no.clear();

		int fd = open(disk_name.c_str(), O_RDONLY);
		if (-1 == fd)
		{
			return(false);
		}

		struct hd_driveid drive = { 0 };
		if (0 == ioctl(fd, HDIO_GET_IDENTITY, &drive))
		{
			trim_serial(drive.serial_no, sizeof(drive.serial_no), serial_no);
		}

		close(fd);

		return(!serial_no.empty());
	}

	static bool scsi_io(
		int fd, unsigned char * cdb,
		unsigned char cdb_size, int xfer_dir,
		unsigned char * data, unsigned int data_size,
		unsigned char * sense, unsigned int sense_len
	)
	{
		sg_io_hdr_t io_hdr = { 0 };
		io_hdr.interface_id = 'S';
		io_hdr.cmdp = cdb;
		io_hdr.cmd_len = cdb_size;
		io_hdr.sbp = sense;
		io_hdr.mx_sb_len = sense_len;
		io_hdr.dxfer_direction = xfer_dir;
		io_hdr.dxferp = data;
		io_hdr.dxfer_len = data_size;
		io_hdr.timeout = 5000;

		if (ioctl(fd, SG_IO, &io_hdr) < 0)
		{
			return(false);
		}

		if (SG_INFO_OK != (io_hdr.info & SG_INFO_OK_MASK) && io_hdr.sb_len_wr > 0)
		{
			return(false);
		}

		if (io_hdr.masked_status || io_hdr.host_status || io_hdr.driver_status)
		{
			return(false);
		}

		return(true);
	}

	static bool get_disk_serial_by_way_2(const std::string & disk_name, std::string & serial_no)
	{
		serial_no.clear();

		int fd = open(disk_name.c_str(), O_RDONLY);
		if (-1 == fd)
		{
			return(false);
		}

		int version = 0;
		if (ioctl(fd, SG_GET_VERSION_NUM, &version) < 0 || version < 30000)
		{
			close(fd);
			return(false);
		}

		const unsigned int data_size = 0x00ff;
		unsigned char data[data_size] = { 0 };
		const unsigned int sense_len = 32;
		unsigned char sense[sense_len] = { 0 };
		unsigned char cdb[] = { 0x12, 0x01, 0x80, 0x00, 0x00, 0x00 };
		cdb[3] = (data_size >> 8) & 0xff;
		cdb[4] = (data_size & 0xff);

		if (scsi_io(fd, cdb, sizeof(cdb), SG_DXFER_FROM_DEV, data, data_size, sense, sense_len))
		{
			int page_len = data[3];
			trim_serial(data + 4, page_len, serial_no);
		}

		close(fd);

		return(!serial_no.empty());
	}

	static bool parse_serial(const char * line, int line_size, const char * match_words, std::string & serial_no)
	{
		const char * serial_s = strstr(line, match_words);
		if (NULL == serial_s)
		{
			return(false);
		}
		serial_s += strlen(match_words);
		while (isspace(serial_s[0]))
		{
			++serial_s;
		}

		const char * serial_e = line + line_size;
		const char * comma = strchr(serial_s, ',');
		if (NULL != comma)
		{
			serial_e = comma;
		}

		while (serial_e > serial_s && isspace(serial_e[-1]))
		{
			--serial_e;
		}

		if (serial_e <= serial_s)
		{
			return(false);
		}

		std::string(serial_s, serial_e).swap(serial_no);

		return(true);
	}

	static void get_serial(const char * file_name, const char * match_words, std::string & serial_no)
	{
		serial_no.c_str();

		std::ifstream ifs(file_name, std::ios::binary);
		if (!ifs.is_open())
		{
			return;
		}

		char line[4096] = { 0 };
		while (!ifs.eof())
		{
			ifs.getline(line, sizeof(line));
			if (!ifs.good())
			{
				break;
			}

			if (0 == ifs.gcount())
			{
				continue;
			}

			if (parse_serial(line, ifs.gcount() - 1, match_words, serial_no))
			{
				break;
			}
		}

		ifs.close();
	}

	static bool get_disk_serial_by_way_3(const std::string & disk_name, std::string & serial_no)
	{
		serial_no.c_str();

		const char * hdparm_result = ".hdparm_result.txt";
		char command[512] = { 0 };
		snprintf(command, sizeof(command), "hdparm -i %s | grep SerialNo > %s", disk_name.c_str(), hdparm_result);

		if (0 == system(command))
		{
			get_serial(hdparm_result, "SerialNo=", serial_no);
		}

		unlink(hdparm_result);

		return(!serial_no.empty());
	}

	static bool get_disk_serial_by_way_4(std::string & serial_no)
	{
		serial_no.c_str();

		const char * lshw_result = ".lshw_result.txt";
		char command[512] = { 0 };
		snprintf(command, sizeof(command), "lshw -class disk | grep serial > %s", lshw_result);

		if (0 == system(command))
		{
			get_serial(lshw_result, "serial:", serial_no);
		}

		unlink(lshw_result);

		return(!serial_no.empty());
	}

	static bool get_cpu_id_by_asm(std::string & cpu_id)
	{
		// cpu_id.clear();

		// unsigned int s1 = 0;
		// unsigned int s2 = 0;
		// asm volatile
		// 	(
		// 		"movl $0x01, %%eax; \n\t"
		// 		"xorl %%edx, %%edx; \n\t"
		// 		"cpuid; \n\t"
		// 		"movl %%edx, %0; \n\t"
		// 		"movl %%eax, %1; \n\t"
		// 		: "=m"(s1), "=m"(s2)
		// 		);

		// if (0 == s1 && 0 == s2)
		// {
		// 	return(false);
		// }

		// char cpu[32] = { 0 };
		// snprintf(cpu, sizeof(cpu), "%08X%08X", htonl(s2), htonl(s1));
		// std::string(cpu).swap(cpu_id);

		return(true);
	}

	static void parse_cpu_id(const char * file_name, const char * match_words, std::string & cpu_id)
	{
		cpu_id.c_str();

		std::ifstream ifs(file_name, std::ios::binary);
		if (!ifs.is_open())
		{
			return;
		}

		char line[4096] = { 0 };
		while (!ifs.eof())
		{
			ifs.getline(line, sizeof(line));
			if (!ifs.good())
			{
				break;
			}

			const char * cpu = strstr(line, match_words);
			if (NULL == cpu)
			{
				continue;
			}
			cpu += strlen(match_words);

			while ('\0' != cpu[0])
			{
				if (' ' != cpu[0])
				{
					cpu_id.push_back(cpu[0]);
				}
				++cpu;
			}

			if (!cpu_id.empty())
			{
				break;
			}
		}

		ifs.close();
	}

	static bool get_cpu_id_by_system(std::string & cpu_id)
	{
		cpu_id.c_str();

		const char * dmidecode_result = ".dmidecode_result.txt";
		char command[512] = { 0 };
		snprintf(command, sizeof(command), "dmidecode -t 4 | grep ID > %s", dmidecode_result);

		if (0 == system(command))
		{
			parse_cpu_id(dmidecode_result, "ID:", cpu_id);
		}

		unlink(dmidecode_result);

		return(!cpu_id.empty());
	}
#endif

#ifdef OS_WIN32

	static char * flipAndCodeBytes(const char * str, int pos, int flip, char * buf)	{
		int i;
		int j = 0;
		int k = 0;

		buf[0] = '\0';
		if (pos <= 0)
			return buf;

		if (!j)
		{
			char p = 0;

			// First try to gather all characters representing hex digits only.
			j = 1;
			k = 0;
			buf[k] = 0;
			for (i = pos; j && str[i] != '\0'; ++i)
			{
				char c = tolower(str[i]);

				if (isspace(c))
					c = '0';

				++p;
				buf[k] <<= 4;

				if (c >= '0' && c <= '9')
					buf[k] |= (unsigned char)(c - '0');
				else if (c >= 'a' && c <= 'f')
					buf[k] |= (unsigned char)(c - 'a' + 10);
				else
				{
					j = 0;
					break;
				}

				if (p == 2)
				{
					if (buf[k] != '\0' && !isprint(buf[k]))
					{
						j = 0;
						break;
					}
					++k;
					p = 0;
					buf[k] = 0;
				}

			}
		}

		if (!j)
		{
			// There are non-digit characters, gather them as is.
			j = 1;
			k = 0;
			for (i = pos; j && str[i] != '\0'; ++i)
			{
				char c = str[i];

				if (!isprint(c))
				{
					j = 0;
					break;
				}

				buf[k++] = c;
			}
		}

		if (!j)
		{
			// The characters are not there or are not printable.
			k = 0;
		}

		buf[k] = '\0';

		if (flip)
			// Flip adjacent characters
			for (j = 0; j < k; j += 2)
			{
				char t = buf[j];
				buf[j] = buf[j + 1];
				buf[j + 1] = t;
			}

		// Trim any beginning and end space
		i = j = -1;
		for (k = 0; buf[k] != '\0'; ++k)
		{
			if (!isspace(buf[k]))
			{
				if (i < 0)
					i = k;
				j = k;
			}
		}

		if ((i >= 0) && (j >= 0))
		{
			for (k = i; (k <= j) && (buf[k] != '\0'); ++k)
				buf[k - i] = buf[k];
			buf[k - i] = '\0';
		}

		return buf;
	}

	static std::string ReadPhysicalDriveInNTWithZeroRights()
	{
		int done = FALSE;
		int drive = 0;
		char HardDriveSerialNumber[1024] = { 0 };

		for (drive = 0; drive < 16; drive++) {
			HANDLE hPhysicalDriveIOCTL = 0;

			//  Try to get a handle to PhysicalDrive IOCTL, report failure
			//  and exit if can't.
			char driveName[256];

			sprintf(driveName, "\\\\.\\PhysicalDrive%d", drive);

			//  Windows NT, Windows 2000, Windows XP - admin rights not required
			hPhysicalDriveIOCTL = CreateFile(driveName, 0,
				FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
				OPEN_EXISTING, 0, NULL);
			if (hPhysicalDriveIOCTL != INVALID_HANDLE_VALUE) {
				STORAGE_PROPERTY_QUERY query;
				DWORD cbBytesReturned = 0;
				char buffer[10000];

				memset((void *)& query, 0, sizeof(query));
				query.PropertyId = StorageDeviceProperty;
				query.QueryType = PropertyStandardQuery;

				memset(buffer, 0, sizeof(buffer));

				if (DeviceIoControl(hPhysicalDriveIOCTL, IOCTL_STORAGE_QUERY_PROPERTY,
					&query,
					sizeof(query),
					&buffer,
					sizeof(buffer),
					&cbBytesReturned, NULL)) {
					STORAGE_DEVICE_DESCRIPTOR * descrip = (STORAGE_DEVICE_DESCRIPTOR *)& buffer;
					char serialNumber[1000] = { 0 };
					char modelNumber[1000] = { 0 };
					char vendorId[1000] = { 0 };
					char productRevision[1000] = { 0 };

					flipAndCodeBytes(buffer,
						descrip->VendorIdOffset,
						0, vendorId);
					flipAndCodeBytes(buffer,
						descrip->ProductIdOffset,
						0, modelNumber);
					flipAndCodeBytes(buffer,
						descrip->ProductRevisionOffset,
						0, productRevision);
					flipAndCodeBytes(buffer,
						descrip->SerialNumberOffset,
						1, serialNumber);

					if (0 == HardDriveSerialNumber[0] &&
						(isalnum(serialNumber[0]) || isalnum(serialNumber[19]))) {
						strcpy(HardDriveSerialNumber, serialNumber);
					}

					// Get the disk drive geometry.
					memset(buffer, 0, sizeof(buffer));
					DeviceIoControl(hPhysicalDriveIOCTL,
						IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
						NULL,
						0,
						&buffer,
						sizeof(buffer),
						&cbBytesReturned,
						NULL);
				}

				CloseHandle(hPhysicalDriveIOCTL);
			}
		}

		return std::string(HardDriveSerialNumber);
	}
#endif

	std::string diskSerial()
	{
		std::string serialNum = "";

#ifdef OS_WIN32
		OSVERSIONINFO version;

		memset(&version, 0, sizeof(version));
		version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&version);

		if (version.dwPlatformId == VER_PLATFORM_WIN32_NT) {
			serialNum = ReadPhysicalDriveInNTWithZeroRights();
		}
#endif

#ifdef OS_LINUX	

		if (0 != getuid()) {
			return serialNum;
		}

		std::string diskName = "";

		if (get_disk_name(diskName)) {
			if (get_disk_serial_by_way_1(diskName, serialNum)) {
				return serialNum;
			}
			if (get_disk_serial_by_way_2(diskName, serialNum)) {
				return serialNum;
			}
			if (get_disk_serial_by_way_3(diskName, serialNum)) {
				return serialNum;
			}
		}

		if (get_disk_serial_by_way_4(serialNum)) {
			return serialNum;
		}
#endif
		return serialNum;
	}

	std::string cpuId()
	{
		std::string cpuNum = "";
#ifdef OS_LINUX
		if (get_cpu_id_by_asm(cpuNum)) {
			return cpuNum;
		}
		if (0 == getuid()) {
			if (get_cpu_id_by_system(cpuNum)) {
				return cpuNum;
			}
		}
#endif
		return cpuNum;
	}

	uint32_t cpuUsingRate()
	{
#ifdef OS_WIN32
		HANDLE singleObjectEvent;
		FILETIME preidleTime;
		FILETIME prekernelTime;
		FILETIME preuserTime;
		FILETIME idleTime;
		FILETIME kernelTime;
		FILETIME userTime;

		GetSystemTimes(&idleTime, &kernelTime, &userTime);
		preidleTime = idleTime;
		prekernelTime = kernelTime;
		preuserTime = userTime;

		singleObjectEvent = CreateEventA(NULL, FALSE, FALSE, NULL);

		WaitForSingleObject(singleObjectEvent, 1000);

		GetSystemTimes(&idleTime, &kernelTime, &userTime);

		int64_t idle = ((int64_t)idleTime.dwHighDateTime << 32 | idleTime.dwLowDateTime) - ((int64_t)preidleTime.dwHighDateTime << 32 | preidleTime.dwLowDateTime);
		int64_t kernel = ((int64_t)kernelTime.dwHighDateTime << 32 | kernelTime.dwLowDateTime) - ((int64_t)prekernelTime.dwHighDateTime << 32 | prekernelTime.dwLowDateTime);
		int64_t user = ((int64_t)userTime.dwHighDateTime << 32 | userTime.dwLowDateTime) - ((int64_t)preuserTime.dwHighDateTime << 32 | preuserTime.dwLowDateTime);

		return static_cast<uint32_t>((kernel + user - idle) * 100 / (kernel + user));
#else
		char cpuUsingInfo[128] = { 0 };
		char cpu[8] = { 0 };
		int user, nice, sys, idle, iowait, irq, softirq;

		FILE *fp = fopen("/proc/stat", "r");
		if (fp == NULL) { return 0; }

		fgets(cpuUsingInfo, sizeof(cpuUsingInfo), fp);
		sscanf(cpuUsingInfo, "%s%d%d%d%d%d%d%d", cpu, &user, &nice, &sys, &idle, &iowait, &irq, &softirq);

		rewind(fp);
		fclose(fp);

		return (user + nice + sys + iowait + irq + softirq) * 100 / (user + nice + sys + idle + iowait + irq + softirq);
#endif
	}

	uint64_t diskFreeSize()
	{
#ifdef OS_WIN32
		int diskCount = 0;
		uint64_t diskInfo = GetLogicalDrives();
		while (diskInfo) { if (diskInfo & 1) { ++diskCount; } diskInfo = diskInfo >> 1; }

		char driveBuff[1024 + 1] = { 0 };
		int driveStringLength = GetLogicalDriveStrings(1024, (LPTSTR)driveBuff);

		uint64_t totalDiskSize = 0;
		uint64_t freeDiskSize = 0;

		for (int i = 0; i < driveStringLength / 4; ++i)
		{
			uint64_t i64FreeBytesToCaller = 0;
			uint64_t i64TotalBytes = 0;
			uint64_t i64FreeBytes = 0;

			if (GetDriveType(driveBuff + i * 4) == DRIVE_FIXED &&
				GetDiskFreeSpaceEx(&driveBuff[i * 4], (PULARGE_INTEGER)&i64FreeBytesToCaller, (PULARGE_INTEGER)&i64TotalBytes, (PULARGE_INTEGER)&i64FreeBytes))
			{
				totalDiskSize += i64TotalBytes;
				freeDiskSize += i64FreeBytesToCaller;
			}
		}

		return freeDiskSize >> 10;
#else
		struct statfs diskInfo;
		statfs("/", &diskInfo);
		uint64_t totalBlocks = diskInfo.f_bsize;
		uint64_t totalSize = totalBlocks * diskInfo.f_blocks;
		uint64_t freeDisk = diskInfo.f_bfree * totalBlocks;

		return freeDisk >> 10;
#endif
	}

	uint64_t diskTotalSize()
	{
#ifdef OS_WIN32
		int diskCount = 0;
		unsigned long diskInfo = GetLogicalDrives();
		while (diskInfo) { if (diskInfo & 1) { ++diskCount; } diskInfo = diskInfo >> 1; }

		char driveBuff[1024 + 1] = { 0 };
		int driveStringLength = GetLogicalDriveStrings(1024, (LPTSTR)driveBuff);

		uint64_t totalDiskSize = 0;
		uint64_t freeDiskSize = 0;

		for (int i = 0; i < driveStringLength / 4; ++i)
		{
			uint64_t i64FreeBytesToCaller = 0;
			uint64_t i64TotalBytes = 0;
			uint64_t i64FreeBytes = 0;

			if (GetDriveType(driveBuff + i * 4) == DRIVE_FIXED &&
				GetDiskFreeSpaceEx(&driveBuff[i * 4], (PULARGE_INTEGER)&i64FreeBytesToCaller, (PULARGE_INTEGER)&i64TotalBytes, (PULARGE_INTEGER)&i64FreeBytes))
			{
				totalDiskSize += i64TotalBytes;
				freeDiskSize += i64FreeBytesToCaller;
			}
		}

		return totalDiskSize >> 10;
#else
		struct statfs diskInfo;
		statfs("/", &diskInfo);
		uint64_t totalBlocks = diskInfo.f_bsize;
		uint64_t totalSize = totalBlocks * diskInfo.f_blocks;
		uint64_t freeDisk = diskInfo.f_bfree * totalBlocks;

		return totalSize >> 10;
#endif
	}

	uint32_t diskUsingRate()
	{
#ifdef OS_WIN32
		int diskCount = 0;
		unsigned long diskInfo = GetLogicalDrives();
		while (diskInfo) { if (diskInfo & 1) { ++diskCount; } diskInfo = diskInfo >> 1; }

		char driveBuff[1024 + 1] = { 0 };
		int driveStringLength = GetLogicalDriveStrings(1024, (LPTSTR)driveBuff);

		int64_t totalDiskSize = 0;
		int64_t freeDiskSize = 0;

		for (int i = 0; i < driveStringLength / 4; ++i)
		{
			int64_t i64FreeBytesToCaller = 0;
			int64_t i64TotalBytes = 0;
			int64_t i64FreeBytes = 0;

			if (GetDriveType(driveBuff + i * 4) == DRIVE_FIXED &&
				GetDiskFreeSpaceEx(&driveBuff[i * 4], (PULARGE_INTEGER)&i64FreeBytesToCaller, (PULARGE_INTEGER)&i64TotalBytes, (PULARGE_INTEGER)&i64FreeBytes))
			{
				totalDiskSize += i64TotalBytes;
				freeDiskSize += i64FreeBytesToCaller;
			}
		}

		return static_cast<uint32_t>((totalDiskSize - freeDiskSize) * 100 / totalDiskSize);
#else
		struct statfs diskInfo;
		statfs("/", &diskInfo);
		uint64_t totalBlocks = diskInfo.f_bsize;
		uint64_t totalSize = totalBlocks * diskInfo.f_blocks;
		uint64_t freeDisk = diskInfo.f_bfree * totalBlocks;

		return static_cast<int>((totalSize - freeDisk) * 100 / totalSize);
#endif
	}

	uint64_t memoryFreeSize()
	{
#ifdef OS_WIN32
		MEMORYSTATUS memoryStatus;
		GlobalMemoryStatus(&memoryStatus);
		return static_cast<uint64_t>(memoryStatus.dwAvailPhys / 1024 / 1024);
#else

		char *pLine = nullptr;
		if (system("free -m | awk '{print $2,$3}' > /tmp/mem2"));

		char meminfo[256 + 1] = { 0 };

		FILE* fd = fopen("/tmp/mem2", "rb");
		if (fd < 0) { return 0; }
		fread(meminfo, 1, 256, fd);
		fclose(fd);

		pLine = strstr(meminfo, "\n");
		int64_t totalMemory = atoi(pLine);
		pLine = strstr(pLine, " ");
		int64_t usingMemory = atoi(pLine);

		return static_cast<uint64_t>((totalMemory - usingMemory));
#endif
	}

	uint64_t memoryTotalSize()
	{
#ifdef OS_WIN32
		MEMORYSTATUS memoryStatus;
		GlobalMemoryStatus(&memoryStatus);
		return static_cast<uint64_t>(memoryStatus.dwTotalPhys / 1024 / 1024);
#else
		char *pLine = nullptr;

		if (system("free -m | awk '{print $2,$3}' > /tmp/mem2"));

		char meminfo[256 + 1] = { 0 };

		FILE* fd = fopen("/tmp/mem2", "rb");
		if (fd < 0) { return 0; }
		fread(meminfo, 1, 256, fd);
		fclose(fd);

		pLine = strstr(meminfo, "\n");
		int64_t totalMemory = atoi(pLine);
		pLine = strstr(pLine, " ");
		int64_t usingMemory = atoi(pLine);

		return static_cast<uint64_t>(totalMemory);
#endif
	}

	uint32_t memoryUsingRate()
	{
#ifdef OS_WIN32
		MEMORYSTATUS memoryStatus;
		GlobalMemoryStatus(&memoryStatus);
		return static_cast<uint32_t>(memoryStatus.dwMemoryLoad);
#else
		char *pLine = nullptr;

		if (system("free -m | awk '{print $2,$3}' > /tmp/mem2"));

		char meminfo[256 + 1] = { 0 };

		FILE* fd = fopen("/tmp/mem2", "rb");
		if (fd < 0) { return 0; }
		fread(meminfo, 1, 256, fd);
		fclose(fd);

		pLine = strstr(meminfo, "\n");
		int64_t totalMemory = atoi(pLine);
		pLine = strstr(pLine, " ");
		int64_t usingMemory = atoi(pLine);

		return static_cast<uint32_t>(usingMemory * 100 / totalMemory);
#endif
	}

	uint64_t tickCount()
	{
#ifdef OS_WIN32
		return static_cast<int>(GetTickCount() / 1000);
#else
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		return static_cast<uint64_t>((ts.tv_sec * 1000 + ts.tv_nsec / 1000000) / 1000);
#endif
	}

	// utils of linux system
#ifdef OS_LINUX
#define BAD_OPEN_MESSAGE											\
	"Error: /proc must be mounted\n"								\
	"  To mount /proc at boot you need an /etc/fstab line like:\n"	\
	"      proc   /proc   proc    defaults\n"						\
	"  In the meantime, run \"mount proc /proc -t proc\"\n"

#define UPTIME_FILE  "/proc/uptime"
	static int uptime_fd = -1;
#define MEMINFO_FILE "/proc/meminfo"
	static int meminfo_fd = -1;

	/* This macro opens filename only if necessary and seeks to 0 so
	 * that successive calls to the functions are more efficient.
	 * It also reads the current contents of the file into the global buf.
	 */
#define FILE_TO_BUF(filename, fd, buf) do{							\
		static int local_n;											\
		if (fd == -1 && (fd = open(filename, O_RDONLY)) == -1) {	\
			fputs(BAD_OPEN_MESSAGE, stderr);						\
			fflush(NULL);											\
			_exit(102);												\
		}															\
		lseek(fd, 0L, SEEK_SET);									\
		if ((local_n = read(fd, buf, sizeof buf - 1)) < 0) {		\
			perror(filename);										\
			fflush(NULL);											\
			_exit(103);												\
		}															\
		buf[local_n] = '\0';										\
	}while(0)

	 /* evals 'x' twice */
#define SET_IF_DESIRED(x,y) do{  if(x) *(x) = (y); }while(0)

/***********************************************************************/
	int uptime(double * uptime_secs, double * idle_secs) {
		// As of 2.6.24 /proc/meminfo seems to need 888 on 64-bit,
		// and would need 1258 if the obsolete fields were there.
		char buf[2048] = { 0x00 };
		double up = 0, idle = 0;
		char *savelocale;

		FILE_TO_BUF(UPTIME_FILE, uptime_fd, buf);
		savelocale = strdup(setlocale(LC_NUMERIC, NULL));
		setlocale(LC_NUMERIC, "C");
		if (sscanf(buf, "%lf %lf", &up, &idle) < 2) {
			setlocale(LC_NUMERIC, savelocale);
			free(savelocale);
			fputs("bad data in " UPTIME_FILE "\n", stderr);
			return 0;
		}
		setlocale(LC_NUMERIC, savelocale);
		free(savelocale);
		SET_IF_DESIRED(uptime_secs, up);
		SET_IF_DESIRED(idle_secs, idle);
		return up;	/* assume never be zero seconds in practice */
	}

	/***********************************************************************/
	/*
	 * Copyright 1999 by Albert Cahalan; all rights reserved.
	 * This file may be used subject to the terms and conditions of the
	 * GNU Library General Public License Version 2, or any later version
	 * at your option, as published by the Free Software Foundation.
	 * This program is distributed in the hope that it will be useful,
	 * but WITHOUT ANY WARRANTY; without even the implied warranty of
	 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	 * GNU Library General Public License for more details.
	 */

	typedef struct mem_table_struct {
		const char *name;     /* memory type name */
		unsigned long *slot; /* slot in return struct */
	} mem_table_struct;

	static int compare_mem_table_structs(const void *a, const void *b) {
		return strcmp(((const mem_table_struct*)a)->name, ((const mem_table_struct*)b)->name);
	}

	/* example data, following junk, with comments added:
	 *
	 * MemTotal:        61768 kB    old
	 * MemFree:          1436 kB    old
	 * MemShared:           0 kB    old (now always zero; not calculated)
	 * Buffers:          1312 kB    old
	 * Cached:          20932 kB    old
	 * Active:          12464 kB    new
	 * Inact_dirty:      7772 kB    new
	 * Inact_clean:      2008 kB    new
	 * Inact_target:        0 kB    new
	 * Inact_laundry:       0 kB    new, and might be missing too
	 * HighTotal:           0 kB
	 * HighFree:            0 kB
	 * LowTotal:        61768 kB
	 * LowFree:          1436 kB
	 * SwapTotal:      122580 kB    old
	 * SwapFree:        60352 kB    old
	 * Inactive:        20420 kB    2.5.41+
	 * Dirty:               0 kB    2.5.41+
	 * Writeback:           0 kB    2.5.41+
	 * Mapped:           9792 kB    2.5.41+
	 * Shmem:              28 kB    2.6.32+
	 * Slab:             4564 kB    2.5.41+
	 * Committed_AS:     8440 kB    2.5.41+
	 * PageTables:        304 kB    2.5.41+
	 * ReverseMaps:      5738       2.5.41+
	 * SwapCached:          0 kB    2.5.??+
	 * HugePages_Total:   220       2.5.??+
	 * HugePages_Free:    138       2.5.??+
	 * Hugepagesize:     4096 kB    2.5.??+
	 */

	 /* obsolete since 2.6.x, but reused for shmem in 2.6.32+ */
	unsigned long kb_main_shared;
	/* old but still kicking -- the important stuff */
	unsigned long kb_main_buffers;
	unsigned long kb_main_cached;
	unsigned long kb_main_free;
	unsigned long kb_main_total;
	unsigned long kb_swap_free;
	unsigned long kb_swap_total;
	/* recently introduced */
	unsigned long kb_high_free;
	unsigned long kb_high_total;
	unsigned long kb_low_free;
	unsigned long kb_low_total;
	/* 2.4.xx era */
	unsigned long kb_active;
	unsigned long kb_inact_laundry;
	unsigned long kb_inact_dirty;
	unsigned long kb_inact_clean;
	unsigned long kb_inact_target;
	unsigned long kb_swap_cached;  /* late 2.4 and 2.6+ only */
	/* derived values */
	unsigned long kb_swap_used;
	unsigned long kb_main_used;
	/* 2.5.41+ */
	unsigned long kb_writeback;
	unsigned long kb_slab;
	unsigned long nr_reversemaps;
	unsigned long kb_committed_as;
	unsigned long kb_dirty;
	unsigned long kb_inactive;
	unsigned long kb_mapped;
	unsigned long kb_pagetables;
	// seen on a 2.6.x kernel:
	static unsigned long kb_vmalloc_chunk;
	static unsigned long kb_vmalloc_total;
	static unsigned long kb_vmalloc_used;
	// seen on 2.6.24-rc6-git12
	static unsigned long kb_anon_pages;
	static unsigned long kb_bounce;
	static unsigned long kb_commit_limit;
	static unsigned long kb_nfs_unstable;
	static unsigned long kb_swap_reclaimable;
	static unsigned long kb_swap_unreclaimable;

	void meminfo(void) {
		char namebuf[16]; /* big enough to hold any row name */
		mem_table_struct findme = { namebuf, NULL };
		mem_table_struct *found = NULL;
		char *head;
		char *tail;
		/*static const*/ mem_table_struct mem_table[] = {
		{"Active",       &kb_active},       // important
		{"AnonPages",    &kb_anon_pages},
		{"Bounce",       &kb_bounce},
		{"Buffers",      &kb_main_buffers}, // important
		{"Cached",       &kb_main_cached},  // important
		{"CommitLimit",  &kb_commit_limit},
		{"Committed_AS", &kb_committed_as},
		{"Dirty",        &kb_dirty},        // kB version of vmstat nr_dirty
		{"HighFree",     &kb_high_free},
		{"HighTotal",    &kb_high_total},
		{"Inact_clean",  &kb_inact_clean},
		{"Inact_dirty",  &kb_inact_dirty},
		{"Inact_laundry",&kb_inact_laundry},
		{"Inact_target", &kb_inact_target},
		{"Inactive",     &kb_inactive},     // important
		{"LowFree",      &kb_low_free},
		{"LowTotal",     &kb_low_total},
		{"Mapped",       &kb_mapped},       // kB version of vmstat nr_mapped
		{"MemFree",      &kb_main_free},    // important
		{"MemShared",    &kb_main_shared},  // obsolete since kernel 2.6! (sharing the variable with Shmem replacement)
		{"MemTotal",     &kb_main_total},   // important
		{"NFS_Unstable", &kb_nfs_unstable},
		{"PageTables",   &kb_pagetables},   // kB version of vmstat nr_page_table_pages
		{"ReverseMaps",  &nr_reversemaps},  // same as vmstat nr_page_table_pages
		{"SReclaimable", &kb_swap_reclaimable}, // "swap reclaimable" (dentry and inode structures)
		{"SUnreclaim",   &kb_swap_unreclaimable},
		{"Shmem",        &kb_main_shared},  // kernel 2.6 and later (sharing the output variable with obsolete MemShared)
		{"Slab",         &kb_slab},         // kB version of vmstat nr_slab
		{"SwapCached",   &kb_swap_cached},
		{"SwapFree",     &kb_swap_free},    // important
		{"SwapTotal",    &kb_swap_total},   // important
		{"VmallocChunk", &kb_vmalloc_chunk},
		{"VmallocTotal", &kb_vmalloc_total},
		{"VmallocUsed",  &kb_vmalloc_used},
		{"Writeback",    &kb_writeback},    // kB version of vmstat nr_writeback
		};
		const int mem_table_count = sizeof(mem_table) / sizeof(mem_table_struct);
		char buf[2048] = { 0x00 };
		FILE_TO_BUF(MEMINFO_FILE, meminfo_fd, buf);

		kb_inactive = ~0UL;

		head = buf;
		for (;;) {
			tail = strchr(head, ':');
			if (!tail) break;
			*tail = '\0';
			if (strlen(head) >= sizeof(namebuf)) {
				head = tail + 1;
				goto nextline;
			}
			strcpy(namebuf, head);
			// compile error on -std=c++11
			// found = bsearch(&findme, mem_table, mem_table_count,
			//     sizeof(mem_table_struct), compare_mem_table_structs
			// );
			for (int i = 0; i < mem_table_count; ++i) {
				if (0 == strcmp(findme.name, mem_table[i].name)) {
					found = &mem_table[i];
					break;
				}
			}
			head = tail + 1;
			if (!found) goto nextline;
			*(found->slot) = (unsigned long)strtoull(head, &tail, 10);
		nextline:
			tail = strchr(head, '\n');
			if (!tail) break;
			head = tail + 1;
		}
		if (!kb_low_total) {  /* low==main except with large-memory support */
			kb_low_total = kb_main_total;
			kb_low_free = kb_main_free;
		}
		if (kb_inactive == ~0UL) {
			kb_inactive = kb_inact_dirty + kb_inact_clean + kb_inact_laundry;
		}
		kb_swap_used = kb_swap_total - kb_swap_free;
		kb_main_used = kb_main_total - kb_main_free;
	}


#endif /* OS_LINUX */

	std::string uptime()
	{
#ifdef OS_WIN32
		return std::string("");
#else
		int upminutes, uphours, updays, upweeks, upyears, updecades;
		int pos;
		int comma;
		double uptime_secs, idle_secs;
		char buf[1024] = { 0x00 };

		/* read and calculate the amount of uptime */

		uptime(&uptime_secs, &idle_secs);

		updecades = (int)uptime_secs / (60 * 60 * 24 * 365 * 10);
		upyears = ((int)uptime_secs / (60 * 60 * 24 * 365)) % 10;
		upweeks = ((int)uptime_secs / (60 * 60 * 24 * 7)) % 52;
		updays = ((int)uptime_secs / (60 * 60 * 24)) % 7;

		strcat(buf, "up ");
		pos += 3;

		upminutes = (int)uptime_secs / 60;
		uphours = upminutes / 60;
		uphours = uphours % 24;
		upminutes = upminutes % 60;

		comma = 0;

		if (updecades) {
			pos += sprintf(buf + pos, "%d %s", updecades,
				updecades > 1 ? "decades" : "decade");
			comma += 1;
		}

		if (upyears) {
			pos += sprintf(buf + pos, "%s%d %s", comma > 0 ? ", " : "", upyears,
				upyears > 1 ? "years" : "year");
			comma += 1;
		}

		if (upweeks) {
			pos += sprintf(buf + pos, "%s%d %s", comma > 0 ? ", " : "", upweeks,
				upweeks > 1 ? "weeks" : "week");
			comma += 1;
		}

		if (updays) {
			pos += sprintf(buf + pos, "%s%d %s", comma > 0 ? ", " : "", updays,
				updays > 1 ? "days" : "day");
			comma += 1;
		}

		if (uphours) {
			pos += sprintf(buf + pos, "%s%d %s", comma > 0 ? ", " : "", uphours,
				uphours > 1 ? "hours" : "hour");
			comma += 1;
		}

		if (upminutes) {
			pos += sprintf(buf + pos, "%s%d %s", comma > 0 ? ", " : "", upminutes,
				upminutes > 1 ? "minutes" : "minute");
			comma += 1;
		}

		return std::string(buf);
#endif
	}

	std::string since()
	{
#ifdef OS_WIN32
		time_t now = time(NULL);
		time_t upSinceSecs = now - (GetTickCount() / 1000);
		struct tm *upSince = NULL;
		char buf[64] = { 0x00 };

		// Show this
		upSince = localtime(&upSinceSecs);
		snprintf(buf, sizeof(buf) - 1,
			"%04d-%02d-%02d %02d:%02d:%02d",
			upSince->tm_year + 1900, upSince->tm_mon + 1, upSince->tm_mday,
			upSince->tm_hour, upSince->tm_min, upSince->tm_sec);
		return std::string(buf);
#else
		double now, uptimeSecs, idleSecs;
		time_t upSinceSecs;
		struct tm *upSince;
		struct timeval tim;
		char buf[64] = { 0x00 };

		// Get the current time and convert it to a double
		gettimeofday(&tim, NULL);
		now = tim.tv_sec + (tim.tv_usec / 1000000.0);

		// Get the uptime and calculate when that was
		uptime(&uptimeSecs, &idleSecs);
		upSinceSecs = (time_t)((now - uptimeSecs) + 0.5);

		// Show this
		upSince = localtime(&upSinceSecs);
		snprintf(buf, sizeof(buf) - 1,
			"%04d-%02d-%02d %02d:%02d:%02d",
			upSince->tm_year + 1900, upSince->tm_mon + 1, upSince->tm_mday,
			upSince->tm_hour, upSince->tm_min, upSince->tm_sec);
		return std::string(buf);
#endif
	}

	std::string memoryFree()
	{
#ifdef OS_WIN32
		return std::string("");
#else
		meminfo();
		return std::string(scaleSize(kb_main_free));
#endif /* OS_WIN32 */
	}

	std::string memoryTotal()
	{
#ifdef OS_WIN32
		return std::string("");
#else
		meminfo();
		return std::string(scaleSize(kb_main_total));
#endif /* OS_WIN32 */
	}

	std::string memoryUsed()
	{
#ifdef OS_WIN32
		return std::string("");
#else
		meminfo();
		return std::string(scaleSize(kb_main_used));
#endif /* OS_WIN32 */
	}

	std::string osname()
	{
#ifdef OS_WIN32
		return GetSystemName();
#else
		struct utsname name;
		uname(&name);
		return std::string(name.release);
#endif
	}

	std::string tempdir()
	{
#ifdef OS_WIN32
		return getenv("temp");
#else
		return std::string("/tmp");
#endif /* OS_WIN32 */
	}
}
