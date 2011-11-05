
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <sys/ioctl.h>

#include "protocol.h"




int sock;



__attribute__((noreturn)) void handler (int sig) {
 
 
 if ( sig==SIGINT ) {
  printf("\033[1G");
 }
 
 
 close(sock);
 
 
 exit(0);
 
}



int main (int argc, char **argv) {
 
 char buffer[1024];
 socklen_t slen=sizeof(struct sockaddr_in);
 struct sockaddr_in local, client;
 struct ether_addr localmac;
 struct in_addr localip;
 struct ifreq ifr;
 int len, i;
 unsigned char ports=8;
 struct ng_packet np;
 struct attr *at;
 List *l;
 ListNode *ln;
 
 
 
 if ( argc<2 ) {
  printf("Usage: %s <interface>\n", argv[0]);
  return 1;
 }
 
 memset(&local, 0, sizeof(struct sockaddr_in));
 local.sin_family=AF_INET;
 client=local;
 local.sin_addr.s_addr=htonl(INADDR_ANY);
 local.sin_port=htons(SWITCH_PORT);
 
 memset(&ifr, 0, sizeof(struct ifreq));
 strncpy(ifr.ifr_name, argv[1], IFNAMSIZ-1);
 
 np.buffer=buffer;
 np.maxlen=sizeof(buffer);
 
 if ( (sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))<0 ) {
  perror("socket");
 } else if ( bind(sock, (struct sockaddr*)&local, sizeof(struct sockaddr_in))<0 ) {
  perror("bind");
 }
 
 i=1;
 if ( setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &i, sizeof(i))<0 ) {
  perror("setsockopt(SO_BROADCAST)");
 }
 
 /*
 i=0;
 if ( setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, &i, sizeof(i)) ) {
  perror("setsockopt(IP_MULTICAST_LOOP)");
 }
 */
 
 
 if ( ioctl(sock, SIOCGIFHWADDR, &ifr)<0 ) {
  perror("ioctl(SIOCGIFHWADDR)");
 }
 memcpy(&localmac, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
 
 if ( ioctl(sock, SIOCGIFADDR, &ifr)<0 ) {
  perror("ioctl(SIOCGIFADDR)");
 }
 localip=(*(struct sockaddr_in*)&ifr.ifr_addr).sin_addr;
 
 
 printf("server: ip %s, mac %s\n", inet_ntoa(localip), ether_ntoa(&localmac));
 
 signal(SIGINT, handler);
 signal(SIGTERM, handler);
 
 
 while ( 1 ) {
  
  if ( (len=recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&client, &slen))<0 ) {
   perror("recvfrom");
   break;
  }
  
  if ( len<(int)sizeof(struct ng_header) || ( memcmp(&np.nh->switch_mac, &nullMac, 6)!=0 && memcmp(&np.nh->switch_mac, &localmac, 6)!=0 ) ) {
   continue;
  }
  
  printf("received %d bytes from %s: \n", len, inet_ntoa(client.sin_addr));
  
  printf("code: %i\n", np.nh->code);
  printf("client mac: %s\n", ether_ntoa((struct ether_addr*)np.nh->client_mac));
  printf("seq num: %d\n", ntohl(np.nh->seqnum));
  printf("\n");
  
  memcpy(np.nh->switch_mac, &localmac, ETH_ALEN);
  
  initNgPacket(&np);
  l=extractPacketAttributes(&np);
  
  initNgPacket(&np);
  
  
  switch ( np.nh->code ) {
   
   case CODE_READ_REQ:
    
    for (ln=l->first; ln!=NULL; ln=ln->next) {
     at=ln->data;
     
     switch ( at->attr ) {
      
      case ATTR_PRODUCT:
       strcpy(np.ah->data, "NgEmu");
       addPacketAttr(&np, ATTR_PRODUCT, strlen(np.ah->data), NULL);
      break;
      
      case ATTR_UNK2:
       addPacketShortAttr(&np, ATTR_UNK2, 0);
      break;
      
      case ATTR_NAME:
       strcpy(np.ah->data, "netgear switch emulator");
       addPacketAttr(&np, ATTR_NAME, strlen(np.ah->data), NULL);
      break;
      
      case ATTR_MAC:
       memcpy(np.ah->data, &localmac, ETH_ALEN);
       addPacketAttr(&np, ATTR_MAC, ETH_ALEN, NULL);
      break;
      
      case ATTR_UNK5:
       memset(np.ah->data, 0, 64);
       addPacketAttr(&np, ATTR_UNK5, 64, NULL);
      break;
      
      case ATTR_IP:
       *(struct in_addr*)np.ah->data=localip;
       addPacketAttr(&np, ATTR_IP, 4, NULL);
      break;
      
      case ATTR_NETMASK:
       *(in_addr_t*)np.ah->data=inet_addr("255.255.255.0");
       addPacketAttr(&np, ATTR_NETMASK, 4, NULL);
      break;
      
      case ATTR_GATEWAY:
       *(in_addr_t*)np.ah->data=inet_addr("192.168.0.16");
       addPacketAttr(&np, ATTR_GATEWAY, 4, NULL);
      break;
      
      case ATTR_DHCP:
       addPacketShortAttr(&np, ATTR_DHCP, 1);
      break;
      
      case ATTR_UNK12:
       addPacketByteAttr(&np, ATTR_UNK12, 1);
      break;
      
      case ATTR_FIRM_VER:
       strcpy(np.ah->data, "NgAdmin 0.0.1");
       addPacketAttr(&np, ATTR_FIRM_VER, strlen(np.ah->data), NULL);
      break;
      
      case ATTR_UNK15:
       addPacketByteAttr(&np, ATTR_UNK15, 1);
      break;
      
      case ATTR_PORTS_NUMBER:
       addPacketByteAttr(&np, ATTR_PORTS_NUMBER, ports);
      break;
      
      case ATTR_PORT_STATUS:
       for (i=1; i<=ports; i++) {
        np.ah->data[0]=i;
        switch ( i&3 ) {
         case 0: np.ah->data[2]=0; np.ah->data[1]=0; break;
         case 1: np.ah->data[2]=1; np.ah->data[1]=SPEED_10; break;
         case 2: np.ah->data[2]=1; np.ah->data[1]=SPEED_100; break;
         case 3: np.ah->data[2]=1; np.ah->data[1]=SPEED_1000; 
        }
        addPacketAttr(&np, ATTR_PORT_STATUS, 3, NULL);
       }
      break;
      
      case ATTR_QOS_TYPE:
       addPacketByteAttr(&np, ATTR_QOS_TYPE, QOS_PORT);
      break;
      
      case ATTR_QOS_CONFIG:
       for (i=1; i<=(int)ports; i++) {
        np.ah->data[0]=i;
        np.ah->data[1]=(i&3)+1;
        addPacketAttr(&np, ATTR_QOS_CONFIG, 2, NULL);
       }
      break;
      
      case 0x5400:
       addPacketByteAttr(&np, 0x5400, 0);
      break;
      
      /*
      case 0x1000: // System -> Monitoring -> Port Statistics
       len=1+3*8;
       for (i=1; i<=(int)ports; i++) {
        memset(np.ah->data, 0, len);
        addPacketAttr(&np, 0x1000, len, NULL);
       }
      break;
      */
      
      case ATTR_MIRROR:
       np.ah->data[0]=ports;
       np.ah->data[1]=0;
       len=(ports>>3)+((ports&7)!=0);
       *(unsigned int*)&np.ah->data[2]=0;
       for (i=0; i<ports-1; i+=2) {
        np.ah->data[2+(i>>3)] |= 1<<(7-(i&7)) ;
       }
       addPacketAttr(&np, ATTR_MIRROR, 2+len, NULL);
      break;
      
      case ATTR_VLAN_TYPE:
       addPacketByteAttr(&np, ATTR_VLAN_TYPE, VLAN_DOT_ADV);
      break;
      
      /*
      case 0x7400:
      break
      */
      
      default:
       addPacketEmptyAttr(&np, at->attr);
      
     }
     
    }
    
    np.nh->code=CODE_READ_REP;
    len=getPacketTotalSize(&np);
    
   break;
   
   case CODE_WRITE_REQ:
    np.nh->code=CODE_WRITE_REP;
   break;
   
   default:
    printf("unkown code\n");
   
  }
  
  
  destroyList(l, (void(*)(void*))freeAttr);
  
  client.sin_addr.s_addr=htonl(INADDR_BROADCAST);
  client.sin_port=htons(CLIENT_PORT);
  if ( sendto(sock, buffer, len, 0, (struct sockaddr*)&client, sizeof(struct sockaddr))<0 ) {
   perror("sendto");
  }
  
  
 }
 
 handler(0);
 
}

