
#include "protocol.h"



const unsigned short helloRequest[]={
 ATTR_PRODUCT, 
 ATTR_UNK2, 
 ATTR_NAME, 
 ATTR_MAC, 
 ATTR_UNK5, 
 ATTR_IP, 
 ATTR_NETMASK, 
 ATTR_GATEWAY, 
 ATTR_DHCP, 
 ATTR_UNK12, 
 ATTR_FIRM_VER, 
 ATTR_UNK14, 
 ATTR_UNK15, 
 ATTR_END
};


const struct ether_addr nullMac={.ether_addr_octet={0, 0, 0, 0, 0, 0}};



// -------------------
int min (int a, int b) {
 return a<b ? a : b ;
}



// -----------------------------------------------------------------------------------------------------------------------------------------------
void initNgHeader (struct ng_header *nh, char code, const struct ether_addr *client_mac, const struct ether_addr *switch_mac, unsigned int seqnum) {
 
 
 memset(nh, 0, sizeof(struct ng_header));
 nh->unk1=1;
 nh->code=code;
 
 memcpy(nh->client_mac, client_mac, ETH_ALEN);
 
 if ( switch_mac!=NULL ) {
  memcpy(nh->switch_mac, switch_mac, ETH_ALEN);
 }
 
 nh->seqnum=htonl(seqnum);
 strcpy(nh->proto_id, "NSDP");
 
 
}



// ----------------------------------------------------------------------------------------------------------------------------------------------
bool validateNgHeader (const struct ng_header *nh, char code, const struct ether_addr *client_mac, const struct ether_addr *switch_mac, unsigned int seqnum) {
 
 int i;
 
 
 if ( nh->unk1!=1 ) {
  //printf("unk1 not 1\n");
  return false;
 }
 
 if ( code>0 && nh->code!=code ) {
  return false;
 }
 
 for (i=0; i<6; i++) {
  if ( nh->unk2[i]!=0 ) {
   //printf("unk2[%i] not 0\n", i);
   return false;
  }
 }
 
 if ( client_mac!=NULL && memcmp(nh->client_mac, client_mac, ETH_ALEN)!=0 ) {
  //printf("client_mac err\n");
  return false;
 }
 
 if ( switch_mac!=NULL && memcmp(nh->switch_mac, switch_mac, ETH_ALEN)!=0 ) {
  //printf("switch_mac err\n");
  return false;
 }
 
 if ( seqnum>0 && ntohl(nh->seqnum)!=seqnum ) {
  //printf("incorrect seqnum\n");
  return false;
 }
 
 if ( *(unsigned int*)nh->unk3!=0 ) {
  //printf("unk3 not 0\n");
  return false;
 }
 
 
 return true;
 
}



// -------------------------------------
void initNgPacket (struct ng_packet *np) {
 
 np->ah=(struct attr_header*)np->nh->data;
 
}



// --------------------------------------------------------------------------------------------
void addPacketAttr (struct ng_packet *np, unsigned short attr, unsigned short size, void* data) {
 
 struct attr_header *ah=np->ah;
 
 
 if ( (int)(getPacketTotalSize(np)+sizeof(struct attr_header)+size)>(np->maxlen) ) {
  return;
 }
 
 ah->attr=htons(attr);
 ah->size=htons(size);
 
 if ( size>0 && data!=NULL ) {
  memcpy(ah->data, data, size);
 }
 
 np->ah=(struct attr_header*)(ah->data+size);
 
}



// ----------------------------------------------------------------
void addPacketEmptyAttr (struct ng_packet *np, unsigned short attr) {
 addPacketAttr(np, attr, 0, NULL);
}



// -------------------------------------------------------------------------
void addPacketByteAttr (struct ng_packet *np, unsigned short attr, char val) {
 addPacketAttr(np, attr, 1, &val);
}



// ---------------------------------------------------------------------------
void addPacketShortAttr (struct ng_packet *np, unsigned short attr, short val) {
 
 short s=htons(val);
 
 
 addPacketAttr(np, attr, 2, &s);
 
}



// ------------------------------------------------
int getPacketTotalSize (const struct ng_packet *np) {
 return ((char*)np->ah)-np->buffer;
}


// --------------------------------------------
struct attr* newEmptyAttr (unsigned short attr) {
 
 struct attr *at;
 
 
 at=malloc(sizeof(struct attr));
 at->attr=attr;
 at->size=0;
 at->data=NULL;
 
 
 return at;
 
}


// ----------------------------
void freeAttr (struct attr *at) {
 
 if ( at!=NULL ) {
  free(at->data);
  free(at);
 }
 
}



// -------------------------------------------------
List* extractPacketAttributes (struct ng_packet *np) {
 
 List *l;
 struct attr *at;
 
 
 l=createEmptyList();
 
 while ( getPacketTotalSize(np)<np->maxlen ) {
  
  at=malloc(sizeof(struct attr));
  at->attr=ntohs(np->ah->attr);
  at->size=ntohs(np->ah->size);
  
  if ( getPacketTotalSize(np)+at->size>np->maxlen ) {
   free(at);
   break;
  }
  
  if ( at->size==0 ) {
   at->data=NULL;
  } else {
   at->data=malloc(at->size*sizeof(char));
   memcpy(at->data, np->ah->data, at->size);
  }
  
  pushBackList(l, at);
  
  if ( at->attr==ATTR_END ) {
   break;
  }
  
  np->ah=(struct attr_header*)(np->ah->data+at->size);
  
 }
 
 
 return l;
 
}


