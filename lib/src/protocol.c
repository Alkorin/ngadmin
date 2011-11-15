
#include "protocol.h"





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



// ---------------------------------------------------------------------------------------------------------------------------------------------------------
bool validateNgHeader (const struct ng_header *nh, char code, const struct ether_addr *client_mac, const struct ether_addr *switch_mac, unsigned int seqnum) {
 
 
 if ( nh->unk1!=1 ) {
  return false;
 }
 
 if ( code>0 && nh->code!=code ) {
  return false;
 }
 
 if ( nh->unk2!=0 ) {
  return false;
 }
 
 if ( *(unsigned short*)nh->unk3!=0 ) {
  return false;
 }
 
 if ( client_mac!=NULL && memcmp(nh->client_mac, client_mac, ETH_ALEN)!=0 ) {
  return false;
 }
 
 if ( switch_mac!=NULL && memcmp(nh->switch_mac, switch_mac, ETH_ALEN)!=0 ) {
  return false;
 }
 
 if ( seqnum>0 && ntohl(nh->seqnum)!=seqnum ) {
  return false;
 }
 
 if ( *(unsigned int*)nh->unk4!=0 ) {
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
 return newAttr(attr, 0, NULL);
}



// ------------------------------------------------------------------------
struct attr* newAttr (unsigned short attr, unsigned short size, void *data) {
 
 struct attr *at;
 
 
 at=malloc(sizeof(struct attr));
 at->attr=attr;
 at->size=size;
 at->data=data;
 
 
 return at;
 
}



// ----------------------------------------------------------------
struct attr* newByteAttr (unsigned short attr, unsigned char value) {
 
 char *v=malloc(sizeof(char));
 
 *v=value;
 
 return newAttr(attr, 1, v);
 
}



// ----------------------------
void freeAttr (struct attr *at) {
 
 if ( at!=NULL ) {
  free(at->data);
  free(at);
 }
 
}



// ------------------------------------------------------------------------------------------
List* extractPacketAttributes (struct ng_packet *np, char *error, unsigned short *attr_error) {
 
 List *l;
 struct attr *at;
 
 
 if ( error!=NULL ) {
  *error=np->nh->error;
 }
 
 if ( attr_error!=NULL ) {
  *attr_error=ntohs(np->nh->attr);
 }
 
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



// --------------------------------------------------------------
void extractSwitchAttributes (struct swi_attr *sa, const List *l) {
 
 const ListNode *ln;
 const struct attr *at;
 int len;
 
 
 memset(sa, 0, sizeof(struct swi_attr));
 
 // FIXME: mutex lock ?
 
 for (ln=l->first; ln!=NULL; ln=ln->next) {
  at=ln->data;
  
  switch ( at->attr ) {
   
   case ATTR_PRODUCT:
    len=min(at->size, PRODUCT_SIZE);
    memcpy(sa->product, at->data, len);
    //trim(sa->product, len); // FIXME
   break;
   
   case ATTR_NAME:
    len=min(at->size, NAME_SIZE);
    memcpy(sa->name, at->data, len);
    //trim(sa->name, len); // FIXME
   break;
   
   case ATTR_MAC:
    memcpy(&sa->mac, at->data, ETH_ALEN);
   break;
   
   case ATTR_IP:
    sa->nc.ip=*(struct in_addr*)at->data;
   break;
   
   case ATTR_NETMASK:
    sa->nc.netmask=*(struct in_addr*)at->data;
   break;
   
   case ATTR_GATEWAY:
    sa->nc.gw=*(struct in_addr*)at->data;
   break;
   
   case ATTR_DHCP:
    sa->nc.dhcp=( ntohs(*(unsigned short*)at->data)==1 );
   break;
   
   case ATTR_FIRM_VER:
    len=min(at->size, FIRMWARE_SIZE-1);
    memcpy(sa->firmware, at->data, len);
    sa->firmware[len]=0;
   break;
   
   case ATTR_PORTS_COUNT:
    sa->ports=*(unsigned char*)at->data;
   break;
   
   case ATTR_END:
    return;
   
  }
  
 }
 
 
}



