
#include "network.h"




static struct timeval tv;
static struct ether_addr localmac;

static char buffer[1024];
static int sock;
static struct sockaddr_in local, remote;
static socklen_t slen=sizeof(struct sockaddr_in);




int startNetwork (const char *iface) {
 
 struct ifreq ifr;
 int ret;
 
 
 
 
 memset(&local, 0, sizeof(struct sockaddr_in));
 local.sin_family=AF_INET;
 remote=local;
 local.sin_port=htons(CLIENT_PORT);
 
 memset(&ifr, 0, sizeof(struct ifreq));
 strncpy(ifr.ifr_name, iface, IFNAMSIZ-1);
 
 if ( (sock=socket(AF_INET, SOCK_DGRAM, 0))<0 ) {
  perror("socket");
  return sock;
 } else if ( (ret=ioctl(sock, SIOCGIFADDR, &ifr))<0 ) {
  perror("ioctl(SIOCGIFADDR)");
  close(sock);
  return ret;
 }
 //local.sin_addr=(*(struct sockaddr_in*)&ifr.ifr_addr).sin_addr; // FIXME
 
 if ( (ret=ioctl(sock, SIOCGIFHWADDR, &ifr))<0 ) {
  perror("ioctl(SIOCGIFHWADDR)");
  close(sock);
  return ret;
 }
 memcpy(&localmac, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
 
 if ( (ret=bind(sock, (struct sockaddr*)&local, sizeof(struct sockaddr_in)))<0 ) {
  perror("bind");
  close(sock);
  return ret;
 }
 
 
 ret=1;
 if ( (ret=setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &ret, sizeof(ret)))<0 ) {
  perror("setsockopt(SO_BROADCAST)");
  return ret;
 }
 
 /*
 i=0;
 if ( setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, &i, sizeof(i)) ) {
  perror("setsockopt(IP_MULTICAST_LOOP)");
  goto end;
 }
 */
 
 
 return setTimeout(1.);
 
}


int stopNetwork (void) {
 
 return close(sock);
 
}




int setTimeout (double sec) {
 
 int ret;
 
 
 tv.tv_sec=(int)sec;
 tv.tv_usec=(int)((sec-(double)tv.tv_sec)*1e6);
 if ( (ret=setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)))<0 ) {
  perror("setsockopt(SO_RCVTIMEO)");
  return ret;
 }
 
 
 return 0;
 
}


int getTimeout (double *sec) {
 
 
 *sec=((double)tv.tv_sec)+1e-6*((double)tv.tv_usec);
 
 
 return 0;
 
}




int sendNgPacket (char code, const struct ether_addr *switch_mac, unsigned int seqnum, const List *attr) {
 
 struct ng_packet np;
 ListNode *ln;
 struct attr *at;
 int ret;
 
 
 
 np.buffer=buffer;
 np.maxlen=sizeof(buffer);
 initNgPacket(&np);
 initNgHeader(np.nh, code, &localmac, switch_mac, seqnum);
 
 if ( attr!=NULL ) {
  for (ln=attr->first; ln!=NULL; ln=ln->next) {
   at=ln->data;
   addPacketAttr(&np, at->attr, at->size, at->data);
  }
 }
 
 
 remote.sin_addr.s_addr=htonl(INADDR_BROADCAST);
 remote.sin_port=htons(SWITCH_PORT);
 if ( (ret=sendto(sock, buffer, getPacketTotalSize(&np), 0, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)))<0 ) {
  perror("sendto");
 }
 
 
 return ret;
 
}


List* recvNgPacket (char code, const struct ether_addr *switch_mac, unsigned int seqnum) {
 
 struct ng_packet np;
 List *l=NULL;
 int len;
 
 
 np.buffer=buffer;
 np.maxlen=sizeof(buffer);
 
 while ( (len=recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&remote, &slen))>=0 ) {
  if ( len>=(int)sizeof(struct ng_header) && validateNgHeader(np.nh, code, &localmac, switch_mac, seqnum) ) {
   initNgPacket(&np);
   l=extractPacketAttributes(&np);
   break;
  }
 }
 
 
 return l;
 
}


