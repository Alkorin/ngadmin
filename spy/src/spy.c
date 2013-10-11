
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <protocol.h>
#include <attr.h>


int main (void)
{
	char buffer[1500];
	struct ng_packet np;
	int err = 0, s, len;
	struct sockaddr_in local, remote;
	socklen_t slen = sizeof(struct sockaddr_in);
	unsigned char error;
	unsigned short attr_error;
	List *attr;
	ListNode *ln;
	struct attr *at;
	
	
	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s < 0) {
		perror("socket");
		err = 1;
		goto end;
	};
	
	
	memset(&local, 0, sizeof(struct sockaddr_in));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = htons(SWITCH_PORT);
	
	if (bind(s, (struct sockaddr*)&local, sizeof(struct sockaddr_in)) < 0) {
		perror("bind");
		err = 2;
		goto end;
	}
	
	while (1) {
		
		len = recvfrom(s, buffer, sizeof(buffer), 0, (struct sockaddr*)&remote, &slen);
		if (len < 0) {
			perror("recvfrom");
			err = 3;
			goto end;
		}
		
		printf("---------------------------------\n");
		
		np.buffer = buffer;
		np.maxlen = len;
		initNgPacket(&np);
		
		attr = createEmptyList();
		
		if (ntohs(remote.sin_port) != CLIENT_PORT ||
		    len < (int)sizeof(struct ng_header) ||
		    !validateNgHeader(np.nh, 0, NULL, NULL, 0) ||
		    extractPacketAttributes(&np, attr, 0) < 0) {
			printf("wrong packet\n");
			goto end;
		}
		
		printf("received %d attribute(s)\n", attr->count);
		
		for (ln = attr->first; ln != NULL; ln = ln->next) {
			at = ln->data;
			printf("received attribute code = %04X, length = %d\n", at->attr, at->size);
		}
		
		destroyList(attr, (void(*)(void*))freeAttr);
		
		printf("---------------------------------\n\n");
	}
	
	close(s);
	
end:
	return err;
}

