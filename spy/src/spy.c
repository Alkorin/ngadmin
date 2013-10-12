
#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include <arpa/inet.h>

#include <nsdp/protocol.h>
#include <nsdp/attr.h>
#include <nsdp/net.h>


static const char* code_str (unsigned char code)
{
	switch (code) {
	
	case CODE_READ_REQ:
		return "read request";
	
	case CODE_READ_REP:
		return "read reply";
	
	case CODE_WRITE_REQ:
		return "write request";
	
	case CODE_WRITE_REP:
		return "write reply";
	
	default:
		return "unknown";
	}
}


static const char* error_str (unsigned char err)
{
	switch (err) {
	
	case 0:
		return "none";
	
	case ERROR_READONLY:
		return "read only";
	
	case ERROR_INVALID_VALUE:
		return "invalid value";
	
	case ERROR_DENIED:
		return "access denied";
	
	default:
		return "unknown";
	}
}


static void print_packet (const List *attr, const struct nsdp_cmd *nc)
{
	const ListNode *ln;
	const struct attr *at;
	
	
	printf("---------------------------------\n");
	printf("code = %s (%u)\n", code_str(nc->code), nc->code);
	printf("error = %s (%u)\n", error_str(nc->error), nc->error);
	if (nc->attr_error != 0)
		printf("erroneous attribute = %04X\n", nc->attr_error);
	printf("source address = %s:%u\n", inet_ntoa(nc->remote_addr.sin_addr), ntohs(nc->remote_addr.sin_port));
	printf("client MAC = %s\n", ether_ntoa(&nc->client_mac));
	printf("switch MAC = %s\n", ether_ntoa(&nc->switch_mac));
	printf("sequence number = %u\n\n", nc->seqnum);
	printf("received %d attribute(s)\n", attr->count);
	
	for (ln = attr->first; ln != NULL; ln = ln->next) {
		at = ln->data;
		printf("received attribute code = %04X, length = %d\n", at->attr, at->size);
	}
	
	printf("---------------------------------\n\n");
}


static void handler (int sig)
{
	(void)sig;
	printf("interrupt\n");
}


int main (void)
{
	int err = 0, sw_sock = -1, cl_sock = -1;
	List *attr;
	struct nsdp_cmd nc;
	struct sockaddr_in sw_local, cl_local;
	struct pollfd fds[2];
	struct sigaction sa;
	
	
	sw_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sw_sock < 0) {
		perror("socket");
		err = 1;
		goto end;
	};
	
	cl_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (cl_sock < 0) {
		perror("socket");
		err = 1;
		goto end;
	};
	
	
	memset(&sw_local, 0, sizeof(struct sockaddr_in));
	sw_local.sin_family = AF_INET;
	sw_local.sin_addr.s_addr = htonl(INADDR_ANY);
	sw_local.sin_port = htons(SWITCH_PORT);
	
	cl_local = sw_local;
	cl_local.sin_port = htons(CLIENT_PORT);
	
	if (bind(sw_sock, (struct sockaddr*)&sw_local, sizeof(struct sockaddr_in)) < 0) {
		perror("bind");
		err = 2;
		goto end;
	}
	
	if (bind(cl_sock, (struct sockaddr*)&cl_local, sizeof(struct sockaddr_in)) < 0) {
		perror("bind");
		err = 2;
		goto end;
	}
	
	fds[0].fd = sw_sock;
	fds[0].events = POLLIN;
	fds[0].revents = 0;
	fds[1].fd = cl_sock;
	fds[1].events = POLLIN;
	fds[1].revents = 0;
	
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = handler;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	
	attr = createEmptyList();
	
	while (1) {
		err = poll(fds, 2, -1);
		if (err < 0) {
			perror("poll");
			break;
		} else if (err == 0) {
			continue;
		}
		
		memset(&nc, 0, sizeof(struct nsdp_cmd));
		nc.remote_addr.sin_family = AF_INET;
		
		if (fds[0].revents & POLLIN) {
			nc.remote_addr.sin_port = htons(CLIENT_PORT);
			err = recvNsdpPacket(sw_sock, &nc, attr, NULL);
		} else {
			nc.remote_addr.sin_port = htons(SWITCH_PORT);
			err = recvNsdpPacket(cl_sock, &nc, attr, NULL);
		}
		
		if (err < 0)
			continue;
		
		print_packet(attr, &nc);
		
		clearList(attr, (void(*)(void*))freeAttr);
	}
	
	destroyList(attr, (void(*)(void*))freeAttr);
	
end:
	close(sw_sock);
	close(cl_sock);
	
	return err;
}

