
#include <stdio.h>
#include <errno.h>

#include <nsdp/net.h>
#include "encoding.h"


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


int recvNsdpPacket (int sock, struct nsdp_cmd *nc, List *attr, const struct timeval *timeout)
{
	unsigned char buffer[1500];
	struct nsdp_packet np;
	socklen_t slen = sizeof(struct sockaddr_in);
	struct timeval rem;
	fd_set fs;
	int len = -1;
	struct sockaddr_in remote;
	
	
	if (sock < 0 || nc == NULL || attr == NULL)
		return -EINVAL;
	
	np.buffer = buffer;
	
	if (timeout != NULL)
		rem = *timeout;
	
	memset(&remote, 0, sizeof(struct sockaddr_in));
	remote.sin_family = AF_INET;
	
	while (1) {
		FD_ZERO(&fs);
		FD_SET(sock, &fs);
		select(sock + 1, &fs, NULL, NULL, timeout == NULL ? NULL : &rem); /* FIXME: non portable */
		
		len = recvfrom(sock, buffer, sizeof(buffer), MSG_DONTWAIT, (struct sockaddr*)&remote, &slen);
		if (len < 0)
			break;
		
		np.maxlen = len;
		initNsdpPacket(&np);
		
		if ((nc->remote_addr.sin_addr.s_addr != 0 && remote.sin_addr.s_addr != nc->remote_addr.sin_addr.s_addr) ||
		    (nc->remote_addr.sin_port != 0 && remote.sin_port != nc->remote_addr.sin_port) ||
		    len < (int)sizeof(struct nsdp_header) ||
		    !validateNsdpHeader(np.nh, nc) ||
		    extractPacketAttributes(&np, attr, nc->ports) < 0)
			continue;
		
		nc->remote_addr = remote;
		
		nc->code = np.nh->code;
		nc->error = np.nh->error;
		nc->attr_error = ntohs(np.nh->attr);
		
		len = 0;
		break;
	}
	
	
	return len;
}


