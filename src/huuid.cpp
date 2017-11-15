#include "whale/util/huuid.h"
#include "whale/basic/hplatform.h"

#ifdef OS_WIN32
#include <objbase.h>
#else
/*
 * gen_uuid.c --- generate a DCE-compatible uuid
 *
 * Copyright (C) 1996, 1997, 1998, 1999 Theodore Ts'o.
 *
 * %Begin-Header%
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, and the entire permission notice in its entirety,
 *    including the disclaimer of warranties.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ALL OF
 * WHICH ARE HEREBY DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF NOT ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * %End-Header%
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

typedef unsigned char uuid_t[16];

struct uuid {
	uint32_t	time_low;
	uint16_t	time_mid;
	uint16_t	time_hi_and_version;
	uint16_t	clock_seq;
	uint8_t	node[6];
};

static const char *fmt_lower =
	"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x";

static const char *fmt_upper =
	"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X";

#ifdef UUID_UNPARSE_DEFAULT_UPPER
#define FMT_DEFAULT fmt_upper
#else
#define FMT_DEFAULT fmt_lower
#endif

/*
 * prototypes
 */
void uuid_pack(const struct uuid *uu, uuid_t ptr);
void uuid_unpack(const uuid_t in, struct uuid *uu);

/*
 * Get the ethernet hardware address, if we can find it...
 *
 * XXX for a windows version, probably should use GetAdaptersInfo:
 * http://www.codeguru.com/cpp/i-n/network/networkinformation/article.php/c5451
 * commenting out get_node_id just to get gen_uuid to compile under windows
 * is not the right way to go!
 */
static int get_node_id(unsigned char *node_id)
{
#ifdef HAVE_NET_IF_H
	int		sd;
	struct ifreq	ifr, *ifrp;
	struct ifconf	ifc;
	char buf[1024];
	int		n, i;
	unsigned char	*a;
#ifdef HAVE_NET_IF_DL_H
	struct sockaddr_dl *sdlp;
#endif

/*
 * BSD 4.4 defines the size of an ifreq to be
 * max(sizeof(ifreq), sizeof(ifreq.ifr_name)+ifreq.ifr_addr.sa_len
 * However, under earlier systems, sa_len isn't present, so the size is
 * just sizeof(struct ifreq)
 */
#ifdef HAVE_SA_LEN
#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif
#define ifreq_size(i) max(sizeof(struct ifreq),\
     sizeof((i).ifr_name)+(i).ifr_addr.sa_len)
#else
#define ifreq_size(i) sizeof(struct ifreq)
#endif /* HAVE_SA_LEN */

	sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sd < 0) {
		return -1;
	}
	memset(buf, 0, sizeof(buf));
	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if (ioctl (sd, SIOCGIFCONF, (char *)&ifc) < 0) {
		close(sd);
		return -1;
	}
	n = ifc.ifc_len;
	for (i = 0; i < n; i+= ifreq_size(*ifrp) ) {
		ifrp = (struct ifreq *)((char *) ifc.ifc_buf+i);
		strncpy(ifr.ifr_name, ifrp->ifr_name, IFNAMSIZ);
#ifdef SIOCGIFHWADDR
		if (ioctl(sd, SIOCGIFHWADDR, &ifr) < 0)
			continue;
		a = (unsigned char *) &ifr.ifr_hwaddr.sa_data;
#else
#ifdef SIOCGENADDR
		if (ioctl(sd, SIOCGENADDR, &ifr) < 0)
			continue;
		a = (unsigned char *) ifr.ifr_enaddr;
#else
#ifdef HAVE_NET_IF_DL_H
		sdlp = (struct sockaddr_dl *) &ifrp->ifr_addr;
		if ((sdlp->sdl_family != AF_LINK) || (sdlp->sdl_alen != 6))
			continue;
		a = (unsigned char *) &sdlp->sdl_data[sdlp->sdl_nlen];
#else
		/*
		 * XXX we don't have a way of getting the hardware
		 * address
		 */
		close(sd);
		return 0;
#endif /* HAVE_NET_IF_DL_H */
#endif /* SIOCGENADDR */
#endif /* SIOCGIFHWADDR */
		if (!a[0] && !a[1] && !a[2] && !a[3] && !a[4] && !a[5])
			continue;
		if (node_id) {
			memcpy(node_id, a, 6);
			close(sd);
			return 1;
		}
	}
	close(sd);
#endif
	return 0;
}

static int get_random_fd(void)
{
	struct timeval	tv;
	static int	fd = -2;
	int		i;

	if (fd == -2) {
		gettimeofday(&tv, 0);

		fd = open("/dev/urandom", O_RDONLY);
		if (fd == -1)
			fd = open("/dev/random", O_RDONLY | O_NONBLOCK);
		if (fd >= 0) {
			i = fcntl(fd, F_GETFD);
			if (i >= 0)
				fcntl(fd, F_SETFD, i | FD_CLOEXEC);
		}

		srand((getpid() << 16) ^ getuid() ^ tv.tv_sec ^ tv.tv_usec);
	}
	/* Crank the random number generator a few times */
	gettimeofday(&tv, 0);
	for (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
		rand();
	return fd;
}

/*
 * Generate a series of random bytes.  Use /dev/urandom if possible,
 * and if not, use srandom/random.
 */
static void get_random_bytes(void *buf, int nbytes)
{
	int i, n = nbytes, fd = get_random_fd();
	int lose_counter = 0;
	unsigned char *cp = (unsigned char *) buf;

	if (fd >= 0) {
		while (n > 0) {
			i = read(fd, cp, n);
			if (i <= 0) {
				if (lose_counter++ > 16)
					break;
				continue;
			}
			n -= i;
			cp += i;
			lose_counter = 0;
		}
	}

	/*
	 * We do this all the time, but this is the only source of
	 * randomness if /dev/random/urandom is out to lunch.
	 */
	 for (cp = (uint8_t *)buf, i = 0; i < nbytes; i++)
		 *cp++ ^= (rand() >> 7) & 0xFF;

	return;
}

/* Assume that the gettimeofday() has microsecond granularity */
#define MAX_ADJUSTMENT 10

/*
 * Get clock from global sequence clock counter.
 *
 * Return -1 if the clock counter could not be opened/locked (in this case
 * pseudorandom value is returned in @ret_clock_seq), otherwise return 0.
 */
static int get_clock(uint32_t *clock_high, uint32_t *clock_low,
		     uint16_t *ret_clock_seq, int *num)
{
	static int		adjustment = 0;
	static struct timeval	last = {0, 0};
	static int		state_fd = -2;
	static FILE		*state_f;
	static uint16_t		clock_seq;
	struct timeval			tv;
	uint64_t			clock_reg;
	mode_t				save_umask;
	int				len;
	int				ret = 0;

	if (state_fd == -2) {
		save_umask = umask(0);
		state_fd = open("/var/lib/libuuid/clock.txt",
				O_RDWR|O_CREAT, 0660);
		(void) umask(save_umask);
		if (state_fd != -1) {
			state_f = fdopen(state_fd, "r+");
			if (!state_f) {
				close(state_fd);
				state_fd = -1;
				ret = -1;
			}
		}
		else
			ret = -1;
	}
	if (state_fd >= 0) {
		rewind(state_f);
		while (flock(state_fd, LOCK_EX) < 0) {
			if ((errno == EAGAIN) || (errno == EINTR))
				continue;
			fclose(state_f);
			close(state_fd);
			state_fd = -1;
			ret = -1;
			break;
		}
	}
	if (state_fd >= 0) {
		unsigned int cl;
		unsigned long tv1, tv2;
		int a;

		if (fscanf(state_f, "clock: %04x tv: %lu %lu adj: %d\n",
			   &cl, &tv1, &tv2, &a) == 4) {
			clock_seq = cl & 0x3FFF;
			last.tv_sec = tv1;
			last.tv_usec = tv2;
			adjustment = a;
		}
	}

	if ((last.tv_sec == 0) && (last.tv_usec == 0)) {
		get_random_bytes(&clock_seq, sizeof(clock_seq));
		clock_seq &= 0x3FFF;
		gettimeofday(&last, 0);
		last.tv_sec--;
	}

try_again:
	gettimeofday(&tv, 0);
	if ((tv.tv_sec < last.tv_sec) ||
	    ((tv.tv_sec == last.tv_sec) &&
	     (tv.tv_usec < last.tv_usec))) {
		clock_seq = (clock_seq+1) & 0x3FFF;
		adjustment = 0;
		last = tv;
	} else if ((tv.tv_sec == last.tv_sec) &&
	    (tv.tv_usec == last.tv_usec)) {
		if (adjustment >= MAX_ADJUSTMENT)
			goto try_again;
		adjustment++;
	} else {
		adjustment = 0;
		last = tv;
	}

	clock_reg = tv.tv_usec*10 + adjustment;
	clock_reg += ((uint64_t) tv.tv_sec)*10000000;
	clock_reg += (((uint64_t) 0x01B21DD2) << 32) + 0x13814000;

	if (num && (*num > 1)) {
		adjustment += *num - 1;
		last.tv_usec += adjustment / 10;
		adjustment = adjustment % 10;
		last.tv_sec += last.tv_usec / 1000000;
		last.tv_usec = last.tv_usec % 1000000;
	}

	if (state_fd >= 0) {
		rewind(state_f);
		len = fprintf(state_f,
			      "clock: %04x tv: %016lu %08lu adj: %08d\n",
			      clock_seq, last.tv_sec, last.tv_usec, adjustment);
		fflush(state_f);
		if (ftruncate(state_fd, len) < 0) {
			fprintf(state_f, "                   \n");
			fflush(state_f);
		}
		rewind(state_f);
		flock(state_fd, LOCK_UN);
	}

	*clock_high = clock_reg >> 32;
	*clock_low = clock_reg;
	*ret_clock_seq = clock_seq;
	return ret;
}


int __uuid_generate_time(uuid_t out, int *num)
{
	static unsigned char node_id[6];
	static int has_init = 0;
	struct uuid uu;
	uint32_t	clock_mid;
	int ret;

	if (!has_init) {
		if (get_node_id(node_id) <= 0) {
			get_random_bytes(node_id, 6);
			/*
			 * Set multicast bit, to prevent conflicts
			 * with IEEE 802 addresses obtained from
			 * network cards
			 */
			node_id[0] |= 0x01;
		}
		has_init = 1;
	}
	ret = get_clock(&clock_mid, &uu.time_low, &uu.clock_seq, num);
	uu.clock_seq |= 0x8000;
	uu.time_mid = (uint16_t) clock_mid;
	uu.time_hi_and_version = ((clock_mid >> 16) & 0x0FFF) | 0x1000;
	memcpy(uu.node, node_id, 6);
	uuid_pack(&uu, out);
	return ret;
}

/*
 * Generate time-based UUID and store it to @out
 *
 * Tries to guarantee uniqueness of the generated UUIDs by obtaining them from the uuidd daemon,
 * or, if uuidd is not usable, by using the global clock state counter (see get_clock()).
 * If neither of these is possible (e.g. because of insufficient permissions), it generates
 * the UUID anyway, but returns -1. Otherwise, returns 0.
 */
static int uuid_generate_time_generic(uuid_t out) {
	return __uuid_generate_time(out, 0);
}

/*
 * Generate time-based UUID and store it to @out.
 *
 * Discards return value from uuid_generate_time_generic()
 */
void uuid_generate_time(uuid_t out)
{
	(void)uuid_generate_time_generic(out);
}

void __uuid_generate_random(uuid_t out, int *num)
{
	uuid_t	buf;
	struct uuid uu;
	int i, n;

	if (!num || !*num)
		n = 1;
	else
		n = *num;

	for (i = 0; i < n; i++) {
		get_random_bytes(buf, sizeof(buf));
		uuid_unpack(buf, &uu);

		uu.clock_seq = (uu.clock_seq & 0x3FFF) | 0x8000;
		uu.time_hi_and_version = (uu.time_hi_and_version & 0x0FFF)
			| 0x4000;
		uuid_pack(&uu, out);
		out += sizeof(uuid_t);
	}
}

void uuid_generate_random(uuid_t out)
{
	int	num = 1;
	/* No real reason to use the daemon for random uuid's -- yet */

	__uuid_generate_random(out, &num);
}

/*
 * This is the generic front-end to uuid_generate_random and
 * uuid_generate_time.  It uses uuid_generate_random only if
 * /dev/urandom is available, since otherwise we won't have
 * high-quality randomness.
 */
void uuid_generate(uuid_t out)
{
	if (get_random_fd() >= 0) 
		uuid_generate_random(out);
	else 
		uuid_generate_time(out);
}

static void uuid_unparse_x(const uuid_t uu, char *out, const char *fmt)
{
	struct uuid uuid;

	uuid_unpack(uu, &uuid);
	sprintf(out, fmt,
		uuid.time_low, uuid.time_mid, uuid.time_hi_and_version,
		uuid.clock_seq >> 8, uuid.clock_seq & 0xFF,
		uuid.node[0], uuid.node[1], uuid.node[2],
		uuid.node[3], uuid.node[4], uuid.node[5]);
}

void uuid_unparse_lower(const uuid_t uu, char *out)
{
	uuid_unparse_x(uu, out,	fmt_lower);
}

void uuid_unparse_upper(const uuid_t uu, char *out)
{
	uuid_unparse_x(uu, out,	fmt_upper);
}

void uuid_unparse(const uuid_t uu, char *out)
{
	uuid_unparse_x(uu, out, FMT_DEFAULT);
}

void uuid_pack(const struct uuid *uu, uuid_t ptr)
{
	uint32_t	tmp;
	unsigned char	*out = ptr;

	tmp = uu->time_low;
	out[3] = (unsigned char) tmp;
	tmp >>= 8;
	out[2] = (unsigned char) tmp;
	tmp >>= 8;
	out[1] = (unsigned char) tmp;
	tmp >>= 8;
	out[0] = (unsigned char) tmp;

	tmp = uu->time_mid;
	out[5] = (unsigned char) tmp;
	tmp >>= 8;
	out[4] = (unsigned char) tmp;

	tmp = uu->time_hi_and_version;
	out[7] = (unsigned char) tmp;
	tmp >>= 8;
	out[6] = (unsigned char) tmp;

	tmp = uu->clock_seq;
	out[9] = (unsigned char) tmp;
	tmp >>= 8;
	out[8] = (unsigned char) tmp;

	memcpy(out+10, uu->node, 6);
}

void uuid_unpack(const uuid_t in, struct uuid *uu)
{
	const uint8_t	*ptr = in;
	uint32_t		tmp;

	tmp = *ptr++;
	tmp = (tmp << 8) | *ptr++;
	tmp = (tmp << 8) | *ptr++;
	tmp = (tmp << 8) | *ptr++;
	uu->time_low = tmp;

	tmp = *ptr++;
	tmp = (tmp << 8) | *ptr++;
	uu->time_mid = tmp;

	tmp = *ptr++;
	tmp = (tmp << 8) | *ptr++;
	uu->time_hi_and_version = tmp;

	tmp = *ptr++;
	tmp = (tmp << 8) | *ptr++;
	uu->clock_seq = tmp;

	memcpy(uu->node, ptr, 6);
}

#endif	// LINUX

namespace Whale
{
	namespace Util
	{
		namespace HUuid
		{
			std::string gen()
			{
#ifdef OS_WIN32
				GUID guid;
#else
				uuid_t guid;
#endif

				char buf[64] = { 0 };

#ifdef OS_WIN32
				CoCreateGuid(&guid);
#else
				uuid_generate(guid);
#endif

#ifdef OS_WIN32
				_snprintf(buf, sizeof(buf), "%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x",
					guid.Data1, guid.Data2, guid.Data3,
					guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
#else
				snprintf(buf, sizeof(buf), "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
					guid[0], guid[1], guid[2], guid[3], guid[4], guid[5], guid[6], guid[7],
					guid[8], guid[9], guid[10], guid[11], guid[12], guid[13], guid[14], guid[15]);
#endif

				return std::string(buf);
			}

			std::string gens()
			{
#ifdef OS_WIN32
				GUID guid;
#else
				uuid_t guid;
#endif

				char buf[64] = { 0 };

#ifdef OS_WIN32
				CoCreateGuid(&guid);
#else			
				uuid_generate(guid);
#endif

#ifdef OS_WIN32
				_snprintf(buf, sizeof(buf), "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
					guid.Data1, guid.Data2, guid.Data3,
					guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
#else
				snprintf(buf, sizeof(buf), "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
					guid[0], guid[1], guid[2], guid[3], guid[4], guid[5], guid[6], guid[7],
					guid[8], guid[9], guid[10], guid[11], guid[12], guid[13], guid[14], guid[15]);
#endif

				return std::string(buf);
			}
		}
	}
}
