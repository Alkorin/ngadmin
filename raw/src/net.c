
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <errno.h>
#include <poll.h>

#include <nsdp/net.h>
#include "encoding.h"


static inline void timespec_add (struct timespec *tsa, const struct timespec *tsb)
{
	tsa->tv_sec += tsb->tv_sec;
	tsa->tv_nsec += tsb->tv_nsec;
	while (tsa->tv_nsec >= 1000000000) {
		tsa->tv_nsec -= 1000000000;
		tsa->tv_sec++;
	}
}


static inline int timespec_diff_ms (const struct timespec *tsa, const struct timespec *tsb)
{
	int diff = (tsb->tv_sec - tsa->tv_sec) * 1000;
	
	if (tsb->tv_sec > tsa->tv_sec)
		diff += (tsb->tv_nsec - tsa->tv_nsec) / 1000000;
	else
		diff -= (tsb->tv_nsec - tsa->tv_nsec) / 1000000;
	
	return diff;
}


static inline void timeval_to_timespec (struct timespec *ts, const struct timeval *tv)
{
	ts->tv_sec = tv->tv_sec;
	ts->tv_nsec = tv->tv_usec * 1000;
}


int sendNsdpPacket (int sock, const struct nsdp_cmd *nc, const List *attr)
{
	unsigned char buffer[1500];
	struct nsdp_packet np;
	int ret;
	
	
	if (sock < 0 || nc == NULL)
		return -EINVAL;
	
	np.buffer = buffer;
	np.maxlen = sizeof(buffer);
	initNsdpPacket(&np);
	initNsdpHeader(np.nh, nc);
	
	ret = addPacketAttributes(&np, attr, nc->ports);
	if (ret < 0)
		return ret;
	
	ret = sendto(sock, buffer, getPacketTotalSize(&np), 0, (struct sockaddr*)&nc->remote_addr, sizeof(struct sockaddr_in));
	if (ret < 0)
		perror("sendto");
	
	
	return ret;
}


int recvNsdpPacket (int sock, struct nsdp_cmd *nc, List *attr, const struct timespec *timeout)
{
	unsigned char buffer[1500];
	struct nsdp_packet np;
	socklen_t slen = sizeof(struct sockaddr_in);
	int len = -1, timewait;
	struct sockaddr_in remote;
	struct timespec timecurrent, timeend;
#ifndef HAVE_CLOCK_GETTIME
	struct timeval tv;
#endif
	struct pollfd fds;
	
	
	if (sock < 0 || nc == NULL || attr == NULL)
		return -EINVAL;
	
	np.buffer = buffer;
	
	memset(&remote, 0, sizeof(struct sockaddr_in));
	remote.sin_family = AF_INET;
	
	if (timeout == NULL) {
		timewait = -1;
	} else {
#ifdef HAVE_CLOCK_GETTIME
		clock_gettime(CLOCK_MONOTONIC, &timeend);
#else
		gettimeofday(&tv, NULL);
		timeval_to_timespec(&timeend, &tv);
#endif
		timespec_add(&timeend, timeout);
	}
	fds.fd = sock;
	fds.events = POLLIN;
	fds.revents = 0;
	
	while (1) {
		if (timeout != NULL) {
#ifdef HAVE_CLOCK_GETTIME
			clock_gettime(CLOCK_MONOTONIC, &timecurrent);
#else
			gettimeofday(&tv, NULL);
			timeval_to_timespec(&timeend, &tv);
#endif
			timewait = timespec_diff_ms(&timecurrent, &timeend);
			if (timewait <= 0)
				break;
		}
		
		len = poll(&fds, 1, timewait);
		if (len < 0) {
			break;
		} else if (len == 0) {
			len = -ETIMEDOUT;
			break;
		}
		
		len = recvfrom(sock, buffer, sizeof(buffer), MSG_DONTWAIT, (struct sockaddr*)&remote, &slen);
		if (len < 0)
			break;
		
		np.maxlen = len;
		initNsdpPacket(&np);
		
		if ((nc->remote_addr.sin_addr.s_addr != 0 && remote.sin_addr.s_addr != nc->remote_addr.sin_addr.s_addr) ||
		    (nc->remote_addr.sin_port != 0 && remote.sin_port != nc->remote_addr.sin_port) ||
		    len < (int)sizeof(struct nsdp_header) ||
		    !extractNsdpHeader(np.nh, nc) ||
		    extractPacketAttributes(&np, attr, nc->ports) < 0)
			continue;
		
		nc->remote_addr = remote;
		
		len = 0;
		break;
	}
	
	
	return len;
}


