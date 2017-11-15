/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hplatform.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HPLATFORM_H
#define HPLATFORM_H

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#pragma warning(disable : 4003)
#endif /* _MSC_VER */

#if defined(_WIN32) || defined(WIN32) || defined(__MINGW32__) || defined(__BORLANDC__)
#  ifndef OS_WIN32
#	define OS_WIN32
#	endif
#elif defined(__linux) || defined(OS_LINUX) || defined(linux) || defined(__linux__)
#	ifndef OS_LINUX
#	define OS_LINUX
#	endif
#elif defined(__unix) || defined(__unix__) || defined(unix)
#	ifndef OS_UNIX
#	define OS_UNIX
#	endif
#endif

#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

#include <map>
#include <list>
#include <vector>
#include <set>
#include <queue>
#include <string>
#include <atomic>
#include <memory>
#include <exception>

#include <thread>
#include <mutex>
#include <condition_variable>

#include <future>
#include <regex>
#include <algorithm>

#include <chrono>
#include <cinttypes>
#include <ctime>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <iostream>

#include <locale>

#ifdef OS_WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Iphlpapi.h>
#include <Iptypes.h>
#include <windows.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <direct.h>
#include <sys/timeb.h>
#include <time.h>
#include <io.h>
#include <codecvt>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")

#define	__S_ISTYPE(mode, mask)	(((mode) & _S_IFMT) == (mask))

#define	S_ISDIR(mode)	 __S_ISTYPE((mode), _S_IFDIR)
#define	S_ISCHR(mode)	 __S_ISTYPE((mode), _S_IFCHR)
#define	S_ISBLK(mode)	 __S_ISTYPE((mode), _S_IFBLK)
#define	S_ISREG(mode)	 __S_ISTYPE((mode), _S_IFREG)
#ifdef _S_IFIFO
# define S_ISFIFO(mode)	 __S_ISTYPE((mode), _S_IFIFO)
#endif
/* Values for the second argument to access.
   These may be OR'd together.  */
#define	R_OK	4		/* Test for read permission.  */
#define	W_OK	2		/* Test for write permission.  */
#define	X_OK	1		/* Test for execute permission.  */
#define	F_OK	0		/* Test for existence.  */
#endif

#ifdef OS_LINUX
#include <pthread.h>
#include <dirent.h>
#include <unistd.h>
#include <strings.h>
#include <stdarg.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <errno.h>
#include <netdb.h>
#include <mntent.h>
#include <termios.h>
#include <errno.h>
#include <libgen.h>
#include <sys/epoll.h>
#include <sys/resource.h>
#include <sys/queue.h>
#include <net/if.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/statfs.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <sys/msg.h>
#include <syslog.h>
#endif

#ifdef OS_LINUX
#define MAX_PATH			260
#define INFINITE			0xffffffff
#define closesocket			close
#endif

#ifdef OS_WIN32
#ifndef popen
#define popen _popen
#endif
#ifndef pclose
#define pclose _pclose
#endif
#ifndef stat
#define stat _stat
#endif
#ifndef fstat
#define fstat _fstat
#endif
#ifndef open
#define open _open
#endif
#ifndef close
#define close _close
#endif
#ifndef O_RDONLY
#define O_RDONLY _O_RDONLY
#endif
#ifndef access
#define access _access
#endif
#ifndef snprintf
#define snprintf _snprintf
#endif
#endif

#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#define MIN(a,b)            (((a) < (b)) ? (a) : (b))

/**
 * Description : Prohibit the use of copy construction and assignment function.
 * Examples	  :
 * class Test {  DISALLOW_COPY_AND_ASSIGN(Test) }
 */
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
private: \
    TypeName(const TypeName&); \
    void operator=(const TypeName&);

/**
 * Description : Declare single case.
 * Examples	  :
 * class Test {  DECLARE_SINGLETON(Test) }
 */
#define DECLARE_SINGLETON(TypeName) \
public: \
    static TypeName *instance(); \
private: \
    static TypeName *_instance;

/**
 * Description : Implement single case.
 * Examples	  :
 * class Test {  IMPLEMENT_SINGLETON(Test) }
 */
#define IMPLEMENT_SINGLETON(TypeName) \
TypeName *TypeName::_instance = 0; \
TypeName *TypeName::instance() { \
	if (_instance == 0) { \
		_instance = new TypeName(); \
	} \
	return _instance;\
}

/**
 * Description : Declare friend class.
 * Examples	  :
 * class Test {  DECLARE_FRIEND_CLASS(Test) }
 */
#define DECLARE_FRIEND_CLASS(TypeName) \
friend class TypeName;

/**
 * Description : Declare a interface.
 * Examples	  :
 * class Test {  DECLARE_INTERFACE_CLASS(Test) }
 */
#define DECLARE_INTERFACE_CLASS(TypeName) \
protected: \
	TypeName(){}; \
public: \
	virtual ~TypeName(){};

/**
 * Description : Declare a abstract class.
 * Examples	  :
 * class Test {  DECLARE_ABSTRACT_CLASS(Test) }
 */
#define DECLARE_ABSTRACT_CLASS(TypeName) \
public:\
	virtual ~TypeName(){}; \
protected: \
	explicit TypeName(){}; \
private:\


/**
 * Description : Declare a tool class.
 * Examples	  :
 * class Test {  DECLARE_TOOL_CLASS(Test) }
 */
#define DECLARE_TOOL_CLASS(TypeName) \
private: \
	TypeName(){}; \
	TypeName(const TypeName&); \
	void operator=(const TypeName&); \
public:

#endif
