
#include "controller.h"




static char password[128];
static List *swiList=NULL;
static struct swi_attr *current=NULL;
static unsigned int seq=0;




// =============================================================================


// ------------------------
void startController (void) {
 
 password[0]=0;
 swiList=createEmptyList();
 
}


// -----------------------
void stopController (void) {
 
 destroyList(swiList, free);
 current=NULL;
 
}



// =============================================================================


static void getSwitchAttributes (struct swi_attr *sa, const List *l) {
 
 const ListNode *ln;
 const struct attr *at;
 int len;
 
 
 memset(sa, 0, sizeof(struct swi_attr));
 sa->ports=0;
 
 
 for (ln=l->first; ln!=NULL; ln=ln->next) {
  at=ln->data;
  
  switch ( at->attr ) {
   
   case ATTR_PRODUCT:
    len=min(at->size, PRODUCT_SIZE);
    memcpy(sa->product, at->data, len);
    trim(sa->product, len);
   break;
   
   case ATTR_NAME:
    len=min(at->size, NAME_SIZE);
    memcpy(sa->name, at->data, len);
    trim(sa->name, len);
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
   
   case ATTR_END:
    return;
   
  }
  
 }
 
 
}



// ------------------------
int discoverSwitches (void) {
 
 int i;
 List *attr;
 struct swi_attr *sa;
 
 
 clearList(swiList, free);
 current=NULL;
 
 attr=createEmptyList();
 
 i=0;
 do {
  pushBackList(attr, newEmptyAttr(helloRequest[i]));
 } while ( helloRequest[i++]!=ATTR_END );
 
 i=sendNgPacket(CODE_READ_REQ, NULL, ++seq, attr);
 destroyList(attr, (void(*)(void*))freeAttr);
 if ( i<0 ) {
  return ERR_NET;
 }
 
 
 while ( (attr=recvNgPacket (CODE_READ_REP, NULL, seq))!=NULL ) {
  sa=malloc(sizeof(struct swi_attr));
  getSwitchAttributes(sa, attr);
  pushBackList(swiList, sa);
 }
 
 destroyList(attr, (void(*)(void*))freeAttr);
 
 
 return ERR_OK;
 
}


// -----------------------------
const List* getSwitchList (void) {
 return swiList;
}


// ------------------------------------
const struct swi_attr* getCurrentSwitch (void) {
 return current;
}


// --------------
int login (int i) {
 
 List *attr;
 ListNode *ln;
 
 
 if ( i<0 || i>(int)swiList->count ) {
  return ERR_BADID;
 }
 
 
 for (ln=swiList->first; i-->0; ln=ln->next);
 current=ln->data;
 
 
 //attr=createEmptyList();
 
 
 
 return ERR_OK;
 
}


//int logout (void);


// --------------------------------
void setPassword (const char *pass) {
 
 strncpy(password, pass, sizeof(password));
 
}


//int changePassword (const char *pass);



// =============================================================================


//int getPortsNumber (void);

// ------------------------------------------------
int getPortsStatus (unsigned char *ports, int size) {
 
 List *attr;
 ListNode *ln;
 struct attr *at;
 char *p;
 int i;
 
 
 if ( ports==NULL || size<=0 ) {
  return ERR_INVARG;
 } else if ( current==NULL ) {
  return ERR_NOTLOG;
 }
 
 
 attr=createEmptyList();
 pushBackList(attr, newEmptyAttr(ATTR_PORT_STATUS));
 pushBackList(attr, newEmptyAttr(ATTR_END));
 
 
 i=sendNgPacket(CODE_READ_REQ, NULL, ++seq, attr);
 destroyList(attr, (void(*)(void*))freeAttr);
 if ( i<0 ) {
  return ERR_NET;
 }
 
 if ( (attr=recvNgPacket (CODE_READ_REP, NULL, seq))==NULL ) {
  return ERR_TIMEOUT;
 }
 
 for (ln=attr->first; ln!=NULL; ln=ln->next) {
  at=ln->data;
  p=at->data;
  if ( at->attr==ATTR_PORT_STATUS && p[0]-1<size ) {
   i=p[0]-1;
   ports[i]=p[1];
  }
 }
 
 destroyList(attr, (void(*)(void*))freeAttr);
 
 
 return ERR_OK;
 
}


//int getPortsStatistics (unsigned long long stats[][6], int size);

//int resetPortsStatistics (void);



