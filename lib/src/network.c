
#include "network.h"




// -----------------------------------
int startNetwork (struct ngadmin *nga) {
 
 struct ifreq ifr;
 int ret;
 
 
 nga->local.sin_family=AF_INET;
 nga->remote=nga->local;
 nga->local.sin_port=htons(CLIENT_PORT);
 
 memset(&ifr, 0, sizeof(struct ifreq));
 strncpy(ifr.ifr_name, nga->iface, IFNAMSIZ-1);
 
 if ( (nga->sock=socket(AF_INET, SOCK_DGRAM, 0))<0 ) {
  perror("socket");
  return nga->sock;
 }
 
 // get the interface IP address
 if ( (ret=ioctl(nga->sock, SIOCGIFADDR, &ifr))<0 ) {
  perror("ioctl(SIOCGIFADDR)");
  close(nga->sock);
  return ret;
 }
 //local.sin_addr=(*(struct sockaddr_in*)&ifr.ifr_addr).sin_addr; // FIXME
 
 // get the interface MAC address
 if ( (ret=ioctl(nga->sock, SIOCGIFHWADDR, &ifr))<0 ) {
  perror("ioctl(SIOCGIFHWADDR)");
  close(nga->sock);
  return ret;
 }
 memcpy(&nga->localmac, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
 
 // bind
 if ( (ret=bind(nga->sock, (struct sockaddr*)&nga->local, sizeof(struct sockaddr_in)))<0 ) {
  perror("bind");
  close(nga->sock);
  return ret;
 }
 
 // allow broadcasting
 ret=1;
 if ( (ret=setsockopt(nga->sock, SOL_SOCKET, SO_BROADCAST, &ret, sizeof(ret)))<0 ) {
  perror("setsockopt(SO_BROADCAST)");
  return ret;
 }
 
 
 return 0;
 
}



// ----------------------------------
int stopNetwork (struct ngadmin *nga) {
 
 return close(nga->sock);
 
}



// ------------------------------------
int updateTimeout (struct ngadmin *nga) {
 
 int ret;
 
 
 // specify receive timeout
 if ( (ret=setsockopt(nga->sock, SOL_SOCKET, SO_RCVTIMEO, &nga->timeout, sizeof(struct timeval)))<0 ) {
  perror("setsockopt(SO_RCVTIMEO)");
  return ret;
 }
 
 
 return 0;
 
}



// --------------------------------------------------------------------------------------------------------------------------
int sendNgPacket (struct ngadmin *nga, char code, const struct ether_addr *switch_mac, unsigned int seqnum, const List *attr) {
 
 char buffer[1500];
 struct ng_packet np;
 ListNode *ln;
 struct attr *at;
 int ret;
 
 
 
 np.buffer=buffer;
 np.maxlen=sizeof(buffer);
 initNgPacket(&np);
 initNgHeader(np.nh, code, &nga->localmac, switch_mac, seqnum);
 
 if ( attr!=NULL ) {
  for (ln=attr->first; ln!=NULL; ln=ln->next) {
   at=ln->data;
   addPacketAttr(&np, at->attr, at->size, at->data);
  }
 }
 
 
 nga->remote.sin_addr.s_addr=htonl(INADDR_BROADCAST);
 nga->remote.sin_port=htons(SWITCH_PORT);
 if ( (ret=sendto(nga->sock, buffer, getPacketTotalSize(&np), 0, (struct sockaddr*)&nga->remote, sizeof(struct sockaddr_in)))<0 ) {
  perror("sendto");
 }
 
 
 return ret;
 
}



// ---------------------------------------------------------------------------------------------------------------------------------------------------
List* recvNgPacket (struct ngadmin *nga, char code, char *error, unsigned short *attr_error, const struct ether_addr *switch_mac, unsigned int seqnum) {
 
 char buffer[1500];
 struct ng_packet np;
 socklen_t slen=sizeof(struct sockaddr_in);
 List *l=NULL;
 int len;
 
 
 np.buffer=buffer;
 np.maxlen=sizeof(buffer);
 
 while ( (len=recvfrom(nga->sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&nga->remote, &slen))>=0 ) {
  if ( len>=(int)sizeof(struct ng_header) && validateNgHeader(np.nh, code, &nga->localmac, switch_mac, seqnum) ) {
   initNgPacket(&np);
   l=extractPacketAttributes(&np, error, attr_error);
   break;
  }
 }
 
 
 return l;
 
}


