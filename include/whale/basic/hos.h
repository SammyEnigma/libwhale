/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hos.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HOS_H
#define HOS_H

#include "hplatform.h"

namespace Whale
{
	bool startService(const std::string& app);
	bool stopService(const std::string& app);

	bool littleEndian();
	uint16_t htons_(uint16_t A);
	uint32_t htonl_(uint32_t A);

	uint32_t ipToInt(const std::string& val);
	std::string intToIp(uint32_t val);

	void sleep(int millisecs);
	const std::string appName();
	const std::string currentDir();

	template<typename... Args>
	std::string process(const char *fomart, Args... args);
	
	std::string hostName();	
	std::string diskSerial();
	std::string cpuId();
	std::vector<std::string> ipAddress();
	std::vector<std::string> macAddress();

	uint64_t tickCount();
	uint32_t cpuUsingRate();

	uint64_t diskFreeSize();
	uint64_t diskTotalSize();
	uint32_t diskUsingRate();

	uint64_t memoryFreeSize();
	uint64_t memoryTotalSize();
	uint32_t memoryUsingRate();

	std::string memoryFree();
	std::string memoryTotal();
	std::string memoryUsed();

	/**
	 * system up since
	 *
	 * @return string of system startup,
	 *         e.g. "2017-05-11 08:54:30"
	 */
	std::string since();

	/**
	 * Tell how long the system has been running.
	 * and show uptime in pretty format
	 *
	 * @return uptime string
	 */
	std::string uptime();

	// idea of this function is copied from top size scaling
	// using `kB' unit.
	std::string scaleSize(unsigned long size);

	std::string osname();
	std::string tempdir();
}

template<typename... Args>
std::string Whale::process(const char *fomart, Args... args)
{
	if (!fomart) { 
		return ""; 
	}

	std::string command = "";
	size_t len = snprintf(nullptr, 0, fomart, args...);
	command.reserve(len + 1);
	command.resize(len);
	snprintf(&command[0], len + 1, fomart, args...);

	char output[1024 * 64] = { 0 };
	FILE *pipofile = popen(command.c_str(), "r");
	fread(output, 1, sizeof(output) - 1, pipofile);
	pclose(pipofile);

	return std::string(output);
}

#endif

