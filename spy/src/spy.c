
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <nsdp/protocol.h>
#include <nsdp/attr.h>
#include <nsdp/net.h>


int main (void)
{
	int err = 0, s;
	List *attr;
	ListNode *ln;
	struct attr *at;
	struct nsdp_cmd nc;
	struct sockaddr_in local;
	
	
	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s < 0) {
		perror("socket");
		err = 1;
		goto end;
	};
	
	
	memset(&nc, 0, sizeof(struct nsdp_cmd));
	nc.remote_addr.sin_family = AF_INET;
	nc.remote_addr.sin_port = htons(CLIENT_PORT);
	
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
		attr = createEmptyList();
		recvNsdpPacket(s, &nc, attr, NULL);
		
		printf("---------------------------------\n");
		
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

