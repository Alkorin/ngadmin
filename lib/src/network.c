
#include "network.h"




// -----------------------------------
int startNetwork (struct ngadmin *nga) {
 
 struct ifreq ifr;
 int ret;
 
 
 // create socket
 if ( (nga->sock=socket(AF_INET, SOCK_DGRAM, 0))<0 ) {
  perror("socket");
  return nga->sock;
 }
 
 memset(&ifr, 0, sizeof(struct ifreq));
 strncpy(ifr.ifr_name, nga->iface, IFNAMSIZ-1);
 
 // get the interface broadcast address
 if ( (ret=ioctl(nga->sock, SIOCGIFBRDADDR, &ifr))<0 ) {
  perror("ioctl(SIOCGIFBRDADDR)");
  close(nga->sock);
  return ret;
 }
 nga->brd=(*(struct sockaddr_in*)&ifr.ifr_addr).sin_addr;
 
 // get the interface MAC address
 if ( (ret=ioctl(nga->sock, SIOCGIFHWADDR, &ifr))<0 ) {
  perror("ioctl(SIOCGIFHWADDR)");
  close(nga->sock);
  return ret;
 }
 memcpy(&nga->localmac, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
 
 // bind
 memset(&nga->local, 0, sizeof(struct sockaddr_in));
 nga->local.sin_family=AF_INET;
 nga->local.sin_port=htons(CLIENT_PORT);
 
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
 
 // prevent unicast packets from being routed by setting the TTL to 1
 ret=1;
 if ( (ret=setsockopt(nga->sock, IPPROTO_IP, IP_TTL, &ret, sizeof(ret)))<0 ) {
  perror("setsockopt(IP_TTL)");
  return ret;
 }
 
 
 return 0;
 
}



// ----------------------------------
int stopNetwork (struct ngadmin *nga) {
 
 return close(nga->sock);
 
}



// -------------------------------------
int forceInterface (struct ngadmin *nga) {
 
 int ret;
 
 
 /*
 As described bellow, when you have multiple interfaces, this forces the packet 
 to go to a particular interface. 
 */
 if ( (ret=setsockopt(nga->sock, SOL_SOCKET, SO_BINDTODEVICE, nga->iface, strlen(nga->iface)+1))<0 ) {
  perror("setsockopt(SO_BINDTODEVICE)");
  return ret;
 }
 
 /*
 If the switch's IP is not in your network range, for instance because you do 
 not have DHCP  enabled or you started the switch after it, this allows to 
 bypass the routing tables and consider every address is directly reachable on 
 the interface. 
 */
 ret=1;
 if ( (ret=setsockopt(nga->sock, SOL_SOCKET, SO_DONTROUTE, &ret, sizeof(ret)))<0 ) {
  perror("setsockopt(SO_DONTROUTE)");
  return ret;
 }
 
 
 return 0;
 
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



// ----------------------------------------------------------------
int sendNgPacket (struct ngadmin *nga, char code, const List *attr) {
 
 char buffer[1500];
 struct ng_packet np;
 ListNode *ln;
 struct attr *at;
 struct sockaddr_in remote;
 const struct swi_attr *sa=nga->current;
 int ret;
 
 
 np.buffer=buffer;
 np.maxlen=sizeof(buffer);
 initNgPacket(&np);
 initNgHeader(np.nh, code, &nga->localmac, sa==NULL ? NULL : &sa->mac , ++nga->seq);
 
 if ( attr!=NULL ) {
  for (ln=attr->first; ln!=NULL; ln=ln->next) {
   at=ln->data;
   addPacketAttr(&np, at->attr, at->size, at->data);
  }
 }
 
 memset(&remote, 0, sizeof(struct sockaddr_in));
 remote.sin_family=AF_INET;
 remote.sin_port=htons(SWITCH_PORT);
 
 if ( sa!=NULL && !nga->keepbroad ) remote.sin_addr=sa->nc.ip;
 else if ( nga->globalbroad ) remote.sin_addr.s_addr=htonl(INADDR_BROADCAST);
 else remote.sin_addr=nga->brd;
 
 
 if ( (ret=sendto(nga->sock, buffer, getPacketTotalSize(&np), 0, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)))<0 ) {
  perror("sendto");
 }
 
 
 return ret;
 
}



// ------------------------------------------------------------------------------------------------------------
int recvNgPacket (struct ngadmin *nga, char code, unsigned char *error, unsigned short *attr_error, List *attr) {
 
 char buffer[1500];
 struct ng_packet np;
 struct sockaddr_in remote;
 socklen_t slen=sizeof(struct sockaddr_in);
 const struct swi_attr *sa=nga->current;
 struct timeval rem;
 fd_set fs;
 int len=-1;
 
 
 np.buffer=buffer;
 
 memset(&remote, 0, sizeof(struct sockaddr_in));
 remote.sin_family=AF_INET;
 
 rem=nga->timeout;
 
 while ( 1 ) {
  
  FD_ZERO(&fs);
  FD_SET(nga->sock, &fs);
  select(nga->sock+1, &fs, NULL, NULL, &rem); // FIXME: non portable
  
  len=recvfrom(nga->sock, buffer, sizeof(buffer), MSG_DONTWAIT, (struct sockaddr*)&remote, &slen);
  
  if ( len<0 ) break;
  
  np.maxlen=len;
  initNgPacket(&np);
  
  if ( 
      ntohs(remote.sin_port)!=SWITCH_PORT || 
      len<(int)sizeof(struct ng_header) || 
      !validateNgHeader(np.nh, code, &nga->localmac, sa==NULL ? NULL : &sa->mac , nga->seq) || 
      extractPacketAttributes(&np, error, attr_error, attr)<0
     ) continue;
  
  len=0;
  break;
  
 }
 
 
 return len;
 
}



static int checkErrorCode (unsigned char err, unsigned short attr_error) {
 
 
 switch ( err ) {
  
  case ERROR_DENIED: return attr_error==ATTR_PASSWORD ? ERR_BADPASS : ERR_DENIED ;
  case ERROR_INVALID_VALUE: return ERR_INVARG;
  default: return ERR_OK;
  
 }
 
 
}



// ----------------------------------------------
int readRequest (struct ngadmin *nga, List *attr) {
 
 int i, ret=ERR_OK;
 unsigned char err;
 unsigned short attr_error;
 
 
 if ( nga==NULL ) {
  ret=ERR_INVARG;
  goto end;
 }
 
 // add end attribute to end
 pushBackList(attr, newEmptyAttr(ATTR_END));
 
 i=sendNgPacket(nga, CODE_READ_REQ, attr);
 
 // the list will be filled again by recvNgPacket
 clearList(attr, (void(*)(void*))freeAttr);
 
 if ( i<0 || (i=recvNgPacket(nga, CODE_READ_REP, &err, &attr_error, attr))<0 ) {
  ret= ( errno==EAGAIN || errno==EWOULDBLOCK ) ? ERR_TIMEOUT : ERR_NET ;
  goto end;
 }
 
 
 // check error code
 ret=checkErrorCode(err, attr_error);
 
 
 end:
 
 return ret;
 
}



// -----------------------------------------------
int writeRequest (struct ngadmin *nga, List *attr) {
 
 int i, ret=ERR_OK;
 unsigned char err;
 unsigned short attr_error;
 
 
 if ( nga==NULL ) {
  ret=ERR_INVARG;
  goto end;
 } else if ( nga->current==NULL ) {
  ret=ERR_NOTLOG;
  goto end;
 }
 
 
 if ( attr==NULL ) {
  attr=createEmptyList();
 }
 
 // add password attribute to start
 pushFrontList(attr, newAttr(ATTR_PASSWORD, strlen(nga->password), strdup(nga->password)));
 
 // add end attribute to end
 pushBackList(attr, newEmptyAttr(ATTR_END));
 
 i=sendNgPacket(nga, CODE_WRITE_REQ, attr);
 
 // the list will be filled again by recvNgPacket
 // but normally it will be still empty
 clearList(attr, (void(*)(void*))freeAttr);
 
 if ( i<0 || (i=recvNgPacket(nga, CODE_WRITE_REP, &err, &attr_error, attr))<0 ) {
  ret= ( errno==EAGAIN || errno==EWOULDBLOCK ) ? ERR_TIMEOUT : ERR_NET ;
  goto end;
 }
 
 // check error code
 ret=checkErrorCode(err, attr_error);
 
 
 end:
 // the switch replies to write request by just a header (no attributes), so the list can be destroyed
 destroyList(attr, (void(*)(void*))freeAttr);
 
 
 return ret;
 
}


