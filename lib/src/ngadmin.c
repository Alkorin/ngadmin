
#include "lib.h"
#include "network.h"



static const struct timeval default_timeout={.tv_sec=4, .tv_usec=0};



// ---------------------------------------------
struct ngadmin* ngadmin_init (const char *iface) {
 
 struct ngadmin *nga;
 
 
 // allocate main structure
 nga=malloc(sizeof(struct ngadmin));
 memset(nga, 0, sizeof(struct ngadmin));
 
 strncpy(nga->iface, iface, IFNAMSIZ-1);
 
 if ( startNetwork(nga)<0 ) {
  free(nga);
  return NULL;
 }
 
 nga->timeout=default_timeout;
 if ( updateTimeout(nga)<0 ) {
  free(nga);
  return NULL;
 }
 
 
 return nga;
 
}



// ------------------------------------
int ngadmin_close (struct ngadmin *nga) {
 
 if ( nga==NULL ) {
  return ERR_INVARG;
 }
 
 
 stopNetwork(nga);
 free(nga->swi_tab);
 free(nga);
 
 
 return ERR_OK;
 
}



// ---------------------------------------------
int ngadmin_forceInterface (struct ngadmin *nga) {
 
 
 if ( nga==NULL ) {
  return ERR_INVARG;
 }
 
 
 if ( forceInterface(nga)!=0 ) {
  return ERR_NET;
 } else {
  return ERR_OK;
 }
 
}



// --------------------------------------------------------------
int ngadmin_setKeepBroadcasting (struct ngadmin *nga, bool value) {
 
 
 if ( nga==NULL ) {
  return ERR_INVARG;
 }
 
 
 nga->keepbroad=value;
 
 
 return ERR_OK;
 
}



// ------------------------------------------------------------
int ngadmin_setPassword (struct ngadmin *nga, const char *pass) {
 
 if ( nga==NULL ) {
  return ERR_INVARG;
 }
 
 
 strncpy(nga->password, pass, PASSWORD_MAX);
 
 
 return ERR_OK;
 
}



// -------------------------------------------------------------------
int ngadmin_setTimeout (struct ngadmin *nga, const struct timeval *tv) {
 
 int ret=ERR_OK;
 
 
 if ( nga==NULL || tv==NULL ) {
  return ERR_INVARG;
 }
 
 
 nga->timeout=*tv;
 if ( updateTimeout(nga)<0 ) {
  ret=ERR_NET;
 }
 
 
 return ret;
 
}



// -----------------------------------
int ngadmin_scan (struct ngadmin *nga) {
 
 int i;
 List *attr, *swiList;
 struct swi_attr *sa;
 /*
 sent by official win client:
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
 */
 static const unsigned short hello[]={
  ATTR_PRODUCT, 
  ATTR_NAME, 
  ATTR_MAC, 
  ATTR_IP, 
  ATTR_NETMASK, 
  ATTR_GATEWAY, 
  ATTR_DHCP, 
  ATTR_FIRM_VER, 
  ATTR_PORTS_COUNT, 
  ATTR_END
 };
 
 
 if ( nga==NULL ) {
  return ERR_INVARG;
 }
 
 free(nga->swi_tab);
 nga->swi_tab=NULL;
 nga->swi_count=0;
 nga->current=NULL;
 
 
 // create attributes for an "hello" request
 attr=createEmptyList();
 for (i=0; ; ++i) {
  pushBackList(attr, newEmptyAttr(hello[i]));
  if ( hello[i]==ATTR_END ) break;
 }
 
 // send request to all potential switches
 i=sendNgPacket(nga, CODE_READ_REQ, attr);
 clearList(attr, (void(*)(void*))freeAttr);
 if ( i<0 ) {
  return ERR_NET;
 }
 
 
 // try to receive any packets until timeout
 swiList=createEmptyList();
 while ( recvNgPacket(nga, CODE_READ_REP, NULL, NULL, attr)>=0 ) {
  sa=malloc(sizeof(struct swi_attr));
  extractSwitchAttributes(sa, attr);
  clearList(attr, (void(*)(void*))freeAttr);
  pushBackList(swiList, sa);
 }
 
 nga->swi_count=swiList->count;
 nga->swi_tab=convertToArray(swiList, sizeof(struct swi_attr));
 
 
 return ERR_OK;
 
}



// -----------------------------------------------------------------------
const struct swi_attr* ngadmin_getSwitchTab (struct ngadmin *nga, int *nb) {
 
 
 if ( nga==NULL || nb==NULL ) {
  return NULL;
 }
 
 
 *nb=nga->swi_count;
 
 
 return nga->swi_tab;
 
}



// ------------------------------------------------------------------
const struct swi_attr* ngadmin_getCurrentSwitch (struct ngadmin *nga) {
 
 
 if ( nga==NULL ) {
  return NULL;
 }
 
 
 return nga->current;
 
}



// --------------------------------------------
int ngadmin_login (struct ngadmin *nga, int id) {
 
 List *attr;
 int ret=ERR_OK;
 struct swi_attr *sa;
 
 
 if ( nga==NULL ) {
  return ERR_INVARG;
 } else if ( id<0 || id>=nga->swi_count ) {
  return ERR_BADID;
 }
 
 
 sa=&nga->swi_tab[id];
 nga->current=sa;
 
 attr=createEmptyList();
 pushBackList(attr, newAttr(ATTR_PASSWORD, strlen(nga->password), strdup(nga->password)));
 if ( (ret=readRequest(nga, attr))==ERR_OK ) {
  // login succeeded
  // TODO: if keep broadcasting is disabled, connect() the UDP socket so icmp errors messages (port unreachable, TTL exceeded in transit, ...)can be received
 } else {
  // login failed
  nga->current=NULL;
 }
 
 destroyList(attr, (void(*)(void*))freeAttr);
 
 
 return ret;
 
}



// -------------------------------------------------------------------
int ngadmin_getPortsStatus (struct ngadmin *nga, unsigned char *ports) {
 
 List *attr;
 ListNode *ln;
 struct attr *at;
 int ret=ERR_OK, i;
 char *p;
 
 
 if ( nga==NULL || ports==NULL ) {
  return ERR_INVARG;
 } else if ( nga->current==NULL ) {
  return ERR_NOTLOG;
 }
 
 
 attr=createEmptyList();
 pushBackList(attr, newEmptyAttr(ATTR_PORT_STATUS));
 if ( (ret=readRequest(nga, attr))!=ERR_OK ) {
  goto end;
 }
 
 for (ln=attr->first; ln!=NULL; ln=ln->next) {
  at=ln->data;
  p=at->data;
  if ( at->attr==ATTR_PORT_STATUS && at->size>=2 && (i=p[0]-1)>=0 && i<nga->current->ports ) {
   ports[i]=p[1];
  }
 }
 
 
 end:
 destroyList(attr, (void(*)(void*))freeAttr);
 
 return ret;
 
}



// --------------------------------------------------------
int ngadmin_setName (struct ngadmin *nga, const char *name) {
 
 List *attr;
 int ret=ERR_OK;
 
 
 if ( nga==NULL ) {
  return ERR_INVARG;
 } else if ( nga->current==NULL ) {
  return ERR_NOTLOG;
 }
 
 
 attr=createEmptyList();
 pushBackList(attr, name==NULL ? newEmptyAttr(ATTR_NAME) : newAttr(ATTR_NAME, strlen(name), strdup(name)) );
 if ( (ret=writeRequest(nga, attr))!=ERR_OK ) {
  goto end;
 }
 
 
 // successful, also update local name
 if ( name==NULL ) {
  nga->current->name[0]=0;
 } else {
  strncpy(nga->current->name, name, NAME_SIZE);
 }
 
 
 end:
 
 return ret;
 
}



// ------------------------------------------------------------------------
int ngadmin_getPortsStatistics (struct ngadmin *nga, struct port_stats *ps) {
 
 List *attr;
 ListNode *ln;
 struct attr *at;
 int ret=ERR_OK;
 int port;
 
 
 if ( nga==NULL || ps==NULL ) {
  return ERR_INVARG;
 } else if ( nga->current==NULL ) {
  return ERR_NOTLOG;
 }
 
 
 attr=createEmptyList();
 pushBackList(attr, newEmptyAttr(ATTR_PORT_STATISTICS));
 if ( (ret=readRequest(nga, attr))!=ERR_OK ) {
  goto end;
 }
 
 
 for (ln=attr->first; ln!=NULL; ln=ln->next) {
  at=ln->data;
  if ( at->attr==ATTR_PORT_STATISTICS && at->size>=49 && (port=(int)(*(char*)at->data)-1)>=0 && port<nga->current->ports ) {
   ps[port].recv=be64toh(*(unsigned long long*)(at->data+1+8*0));
   ps[port].sent=be64toh(*(unsigned long long*)(at->data+1+8*1));
   ps[port].crc=be64toh(*(unsigned long long*)(at->data+1+8*5));
   // all offsets between 2 and 4 inclusive are unknown values
  }
 }
 
 
 end:
 destroyList(attr, (void(*)(void*))freeAttr);
 
 return ret;
 
}



// ---------------------------------------------------
int ngadmin_resetPortsStatistics (struct ngadmin *nga) {
 
 List *attr;
 
 
 attr=createEmptyList();
 pushBackList(attr, newByteAttr(ATTR_STATS_RESET, 1));
 
 
 return writeRequest(nga, attr);
 
}



// ---------------------------------------------------------------
int ngadmin_changePassword (struct ngadmin *nga, const char* pass) {
 
 List *attr;
 int ret=ERR_OK;
 
 
 if ( nga==NULL || pass==NULL ) {
  return ERR_INVARG;
 } else if ( nga->current==NULL ) {
  return ERR_NOTLOG;
 }
 
 
 attr=createEmptyList();
 pushBackList(attr, newAttr(ATTR_NEW_PASSWORD, strlen(pass), strdup(pass)));
 if ( (ret=writeRequest(nga, attr))!=ERR_OK ) {
  goto end;
 }
 
 
 // successful, also update local password
 strncpy(nga->password, pass, PASSWORD_MAX);
 
 
 end:
 
 return ret;
 
}



// ----------------------------------------------------------
int ngadmin_getStormFilterState (struct ngadmin *nga, int *s) {
 
 List *attr;
 ListNode *ln;
 struct attr *at;
 int ret=ERR_OK;
 
 
 if ( nga==NULL || s==NULL ) {
  return ERR_INVARG;
 } else if ( nga->current==NULL ) {
  return ERR_NOTLOG;
 }
 
 
 attr=createEmptyList();
 pushBackList(attr, newEmptyAttr(ATTR_STORM_ENABLE));
 if ( (ret=readRequest(nga, attr))!=ERR_OK ) {
  goto end;
 }
 
 
 for (ln=attr->first; ln!=NULL; ln=ln->next) {
  at=ln->data;
  if ( at->attr==ATTR_STORM_ENABLE && at->size>=1 ) {
   *s= *(char*)at->data!=0 ;
   break;
  }
 }
 
 
 end:
 destroyList(attr, (void(*)(void*))freeAttr);
 
 return ret;
 
}



// ---------------------------------------------------------
int ngadmin_setStormFilterState (struct ngadmin *nga, int s) {
 
 List *attr;
 
 
 attr=createEmptyList();
 pushBackList(attr, newByteAttr(ATTR_STORM_ENABLE, s!=0));
 
 
 return writeRequest(nga, attr);
 
}



// ---------------------------------------------------------------
int ngadmin_getStormFilterValues (struct ngadmin *nga, int *ports) {
 
 List *attr;
 ListNode *ln;
 struct attr *at;
 int ret=ERR_OK, i;
 
 
 if ( nga==NULL || ports==NULL ) {
  return ERR_INVARG;
 } else if ( nga->current==NULL ) {
  return ERR_NOTLOG;
 }
 
 
 attr=createEmptyList();
 pushBackList(attr, newEmptyAttr(ATTR_STORM_BITRATE));
 if ( (ret=readRequest(nga, attr))!=ERR_OK ) {
  goto end;
 }
 
 
 for (ln=attr->first; ln!=NULL; ln=ln->next) {
  at=ln->data;
  if ( at->attr==ATTR_STORM_BITRATE && at->size>=5 && (i=(int)*(char*)(at->data)-1)>=0 && i<nga->current->ports ) {
   ports[i]=ntohl(*(int*)(1+(char*)at->data));
  }
 }
 
 
 end:
 destroyList(attr, (void(*)(void*))freeAttr);
 
 return ret;
 
}



// ---------------------------------------------------------------------
int ngadmin_setStormFilterValues (struct ngadmin *nga, const int *ports) {
 
 List *attr;
 int i;
 char *p;
 
 
 if ( nga==NULL || ports==NULL ) {
  return ERR_INVARG;
 } else if ( nga->current==NULL ) {
  return ERR_NOTLOG;
 }
 
 
 attr=createEmptyList();
 
 for (i=0; i<nga->current->ports; ++i) {
  if ( ports[i]>=0 && ports[i]<=11 ) {
   p=malloc(5);
   *p=i+1;
   *(int*)(p+1)=htonl(ports[i]);
   pushBackList(attr, newAttr(ATTR_STORM_BITRATE, 5, p));
  }
 }
 
 
 return writeRequest(nga, attr);
 
}



// -----------------------------------------------------------
int ngadmin_getBitrateLimits (struct ngadmin *nga, int *ports) {
 
 List *attr;
 ListNode *ln;
 struct attr *at;
 int ret=ERR_OK;
 struct {
  char port;
  int bitrate;
 } __attribute__((packed)) *p;
 
 
 if ( nga==NULL || ports==NULL ) {
  return ERR_INVARG;
 } else if ( nga->current==NULL ) {
  return ERR_NOTLOG;
 }
 
 
 attr=createEmptyList();
 pushBackList(attr, newEmptyAttr(ATTR_BITRATE_INPUT));
 pushBackList(attr, newEmptyAttr(ATTR_BITRATE_OUTPUT));
 if ( (ret=readRequest(nga, attr))!=ERR_OK ) {
  goto end;
 }
 
 
 for (ln=attr->first; ln!=NULL; ln=ln->next) {
  at=ln->data;
  p=at->data;
  if ( at->size<sizeof(*p) || p->port<1 || p->port>nga->current->ports ) continue;
  if ( at->attr==ATTR_BITRATE_INPUT ) {
   ports[(p->port-1)*2+0]=ntohl(p->bitrate);
  } else if ( at->attr==ATTR_BITRATE_OUTPUT ) {
   ports[(p->port-1)*2+1]=ntohl(p->bitrate);
  }
 }
 
 
 end:
 destroyList(attr, (void(*)(void*))freeAttr);
 
 return ret;
 
}



// -----------------------------------------------------------------
int ngadmin_setBitrateLimits (struct ngadmin *nga, const int *ports) {
 
 List *attr;
 int i;
 char *p;
 
 
 if ( nga==NULL || ports==NULL ) {
  return ERR_INVARG;
 } else if ( nga->current==NULL ) {
  return ERR_NOTLOG;
 }
 
 
 attr=createEmptyList();
 
 for (i=0; i<nga->current->ports; ++i) {
  if ( ports[2*i+0]>=0 && ports[2*i+0]<=11 ) {
   p=malloc(5);
   *p=i+1;
   *(int*)(p+1)=htonl(ports[2*i+0]);
   pushBackList(attr, newAttr(ATTR_BITRATE_INPUT, 5, p));
  }
  if ( ports[2*i+1]>=0 && ports[2*i+1]<=11 ) {
   p=malloc(5);
   *p=i+1;
   *(int*)(p+1)=htonl(ports[2*i+1]);
   pushBackList(attr, newAttr(ATTR_BITRATE_OUTPUT, 5, p));
  }
 }
 
 
 return writeRequest(nga, attr);
 
}



// -------------------------------------------------
int ngadmin_getQOSMode (struct ngadmin *nga, int *s) {
 
 List *attr;
 ListNode *ln;
 struct attr *at;
 int ret=ERR_OK;
 
 
 if ( nga==NULL || s==NULL ) {
  return ERR_INVARG;
 } else if ( nga->current==NULL ) {
  return ERR_NOTLOG;
 }
 
 
 attr=createEmptyList();
 pushBackList(attr, newEmptyAttr(ATTR_QOS_TYPE));
 if ( (ret=readRequest(nga, attr))!=ERR_OK ) {
  goto end;
 }
 
 
 for (ln=attr->first; ln!=NULL; ln=ln->next) {
  at=ln->data;
  if ( at->attr==ATTR_QOS_TYPE && at->size>=1 ) {
   *s= *(char*)at->data ;
   break;
  }
 }
 
 
 end:
 destroyList(attr, (void(*)(void*))freeAttr);
 
 return ret;
 
}


// ------------------------------------------------
int ngadmin_setQOSMode (struct ngadmin *nga, int s) {
 
 List *attr;
 
 
 if ( s<QOS_PORT || s>QOS_DOT ) {
  return ERR_INVARG;
 }
 
 attr=createEmptyList();
 pushBackList(attr, newByteAttr(ATTR_QOS_TYPE, s));
 
 
 return writeRequest(nga, attr);
 
}



// -------------------------------------------------------
int ngadmin_getQOSValues (struct ngadmin *nga, char *ports) {
 
 List *attr;
 ListNode *ln;
 struct attr *at;
 int ret=ERR_OK;
 char *p;
 
 
 if ( nga==NULL || ports==NULL ) {
  return ERR_INVARG;
 } else if ( nga->current==NULL ) {
  return ERR_NOTLOG;
 }
 
 
 attr=createEmptyList();
 pushBackList(attr, newEmptyAttr(ATTR_QOS_CONFIG));
 if ( (ret=readRequest(nga, attr))<0 ) {
  goto end;
 }
 
 for (ln=attr->first; ln!=NULL; ln=ln->next) {
  at=ln->data;
  p=at->data;
  if ( at->attr==ATTR_QOS_CONFIG && at->size>=2 && --p[0]>=0 && p[0]<nga->current->ports ) {
   ports[(int)p[0]]=p[1];
  }
 }
 
 
 end:
 destroyList(attr, (void(*)(void*))freeAttr);
 
 
 return ret;
 
}



// --------------------------------------------------------------
int ngadmin_setQOSValues (struct ngadmin *nga, const char *ports) {
 
 List *attr;
 int i;
 char *p;
 
 
 if ( nga==NULL || ports==NULL ) {
  return ERR_INVARG;
 } else if ( nga->current==NULL ) {
  return ERR_NOTLOG;
 }
 
 
 attr=createEmptyList();
 
 for (i=0; i<nga->current->ports; ++i) {
  if ( ports[i]>=PRIO_HIGH && ports[i]<=PRIO_LOW ) {
   p=malloc(2);
   p[0]=i+1;
   p[1]=ports[i];
   pushBackList(attr, newAttr(ATTR_QOS_CONFIG, 2, p));
  }
 }
 
 
 return writeRequest(nga, attr);
 
}



// --------------------------------------
int ngadmin_restart (struct ngadmin *nga) {
 
 List *attr;
 
 
 attr=createEmptyList();
 pushBackList(attr, newByteAttr(ATTR_RESTART, 1));
 
 
 return writeRequest(nga, attr);
 
}



// ---------------------------------------
int ngadmin_defaults (struct ngadmin *nga) {
 
 List *attr;
 int ret=ERR_OK;
 
 
 attr=createEmptyList();
 pushBackList(attr, newByteAttr(ATTR_DEFAULTS, 1));
 if ( (ret=writeRequest(nga, attr))!=ERR_OK ) {
  goto end;
 }
 
 
 // successful: delog and clean list
 free(nga->swi_tab);
 nga->swi_tab=NULL;
 nga->swi_count=0;
 nga->current=NULL;
 
 
 end:
 
 return ret;
 
}



// -----------------------------------------------------
int ngadmin_getMirror (struct ngadmin *nga, char *ports) {
 
 List *attr;
 ListNode *ln;
 struct attr *at;
 struct swi_attr *sa;
 int ret=ERR_OK, i;
 unsigned char *p;
 
 
 if ( nga==NULL || ports==NULL ) {
  return ERR_INVARG;
 } else if ( (sa=nga->current)==NULL ) {
  return ERR_NOTLOG;
 }
 
 
 attr=createEmptyList();
 pushBackList(attr, newEmptyAttr(ATTR_MIRROR));
 if ( (ret=readRequest(nga, attr))<0 ) {
  goto end;
 }
 
 for (ln=attr->first; ln!=NULL; ln=ln->next) {
  at=ln->data;
  p=at->data;
  if ( at->attr==ATTR_MIRROR && at->size>=3 && p[0]<=nga->current->ports ) {
   ports[0]=p[0];
   for (i=1; i<=sa->ports; ++i) { // FIXME: if ports>8
    ports[i]=(p[2]>>(sa->ports-i))&1;
   }
   break;
  }
 }
 
 
 end:
 destroyList(attr, (void(*)(void*))freeAttr);
 
 
 return ret;
 
}



// -----------------------------------------------------------
int ngadmin_setMirror (struct ngadmin *nga, const char *ports) {
 
 List *attr;
 int i;
 char *p;
 struct swi_attr *sa;
 
 
 if ( nga==NULL ) {
  return ERR_INVARG;
 } else if ( (sa=nga->current)==NULL ) {
  return ERR_NOTLOG;
 }
 
 
 p=malloc(3); // FIXME: if ports>8
 memset(p, 0, 3);
 
 if ( ports!=NULL && ports[0]>0 && ports[0]<=sa->ports ) {
  p[0]=ports[0];
  for (i=1; i<=sa->ports; ++i) {
   if ( i!=p[0] ) {
    p[2]|=(ports[i]&1)<<(sa->ports-i);
   }
  }
 }
 
 attr=createEmptyList();
 pushBackList(attr, newAttr(ATTR_MIRROR, 3, p));
 
 
 return writeRequest(nga, attr);
 
}



// ----------------------------------------------------------------
int ngadmin_getIGMPConf (struct ngadmin *nga, struct igmp_conf *ic) {
 
 List *attr;
 ListNode *ln;
 struct attr *at;
 struct swi_attr *sa;
 int ret=ERR_OK;
 unsigned char *p;
 unsigned short *s;
 
 
 if ( nga==NULL || ic==NULL ) {
  return ERR_INVARG;
 } else if ( (sa=nga->current)==NULL ) {
  return ERR_NOTLOG;
 }
 
 /*
 ATTR_IGMP_ENABLE_VLAN
 ATTR_IGMP_BLOCK_UNK
 ATTR_IGMP_VALID_V3
 
 Apparently, read-querying these attributes at the same time causes the switch to reply garbage. 
 Here we are forced to do like the official win app and send a separate request for each attribute. 
 */
 
 
 attr=createEmptyList();
 memset(ic, 0, sizeof(struct igmp_conf));
 
 
 pushBackList(attr, newEmptyAttr(ATTR_IGMP_ENABLE_VLAN));
 if ( (ret=readRequest(nga, attr))<0 ) {
  goto end;
 }
 
 for (ln=attr->first; ln!=NULL; ln=ln->next) {
  at=ln->data;
  s=at->data;
  if ( at->attr==ATTR_IGMP_ENABLE_VLAN && at->size>=4 ) {
   ic->enable= ntohs(s[0])!=0 ;
   ic->vlan=htons(s[1]);
   break;
  }
 }
 
 clearList(attr, (void(*)(void*))freeAttr);
 
 
 pushBackList(attr, newEmptyAttr(ATTR_IGMP_BLOCK_UNK));
 if ( (ret=readRequest(nga, attr))<0 ) {
  goto end;
 }
 
 for (ln=attr->first; ln!=NULL; ln=ln->next) {
  at=ln->data;
  p=at->data;
  if ( at->attr==ATTR_IGMP_BLOCK_UNK && at->size>=1 ) {
   ic->block= p[0]!=0 ;
   break;
  }
 }
 
 clearList(attr, (void(*)(void*))freeAttr);
 
 
 pushBackList(attr, newEmptyAttr(ATTR_IGMP_VALID_V3));
 if ( (ret=readRequest(nga, attr))<0 ) {
  goto end;
 }
 
 for (ln=attr->first; ln!=NULL; ln=ln->next) {
  at=ln->data;
  p=at->data;
  if ( at->attr==ATTR_IGMP_VALID_V3 && at->size>=1 ) {
   ic->validate= p[0]!=0 ;
   break;
  }
 }
 
 
 
 end:
 destroyList(attr, (void(*)(void*))freeAttr);
 
 
 return ret;
 
}



// ----------------------------------------------------------------------
int ngadmin_setIGMPConf (struct ngadmin *nga, const struct igmp_conf *ic) {
 
 List *attr;
 short *s;
 struct swi_attr *sa;
 
 
 if ( nga==NULL || ic==NULL ) {
  return ERR_INVARG;
 } else if ( (sa=nga->current)==NULL ) {
  return ERR_NOTLOG;
 }
 
 
 s=malloc(2*sizeof(short));
 s[0]=htons(ic->enable!=false);
 s[1]=htons(ic->vlan&0x0FFF);
 
 attr=createEmptyList();
 pushBackList(attr, newAttr(ATTR_IGMP_ENABLE_VLAN, 2*sizeof(short), s));
 pushBackList(attr, newByteAttr(ATTR_IGMP_BLOCK_UNK, ic->block!=false ));
 pushBackList(attr, newByteAttr(ATTR_IGMP_VALID_V3, ic->validate!=false ));
 
 
 return writeRequest(nga, attr);
 
}



// ----------------------------------------------------------------------
int ngadmin_cabletest (struct ngadmin *nga, struct cabletest *ct, int nb) {
 
 List *attr;
 ListNode *ln;
 struct attr *at;
 int i, ret=ERR_OK;
 struct swi_attr *sa;
 char *p;
 
 
 if ( nga==NULL || ct==NULL ) {
  return ERR_INVARG;
 } else if ( (sa=nga->current)==NULL ) {
  return ERR_NOTLOG;
 }
 
 
 attr=createEmptyList();
 
 for (i=0; i<nb; ++i) {
  if ( ct[i].port>=1 && ct[i].port<=sa->ports ) {
   
   p=malloc(2);
   p[0]=ct[i].port;
   p[1]=1;
   pushBackList(attr, newAttr(ATTR_CABLETEST_DO, 2, p));
   
   ret=writeRequest(nga, attr);
   attr=NULL;
   if ( ret<0 ) goto end;
   
   // the list is destroyed by writeRequest, so we need to recreate it
   attr=createEmptyList();
   pushBackList(attr, newByteAttr(ATTR_CABLETEST_RESULT, ct[i].port));
   
   if ( (ret=readRequest(nga, attr))<0 ) goto end;
   
   for (ln=attr->first; ln!=NULL; ln=ln->next) {
    at=ln->data;
    p=at->data;
    if ( at->attr==ATTR_CABLETEST_RESULT && at->size>=9 && p[0]==ct[i].port ) {
     ct[i].v1=ntohl(*(int*)&p[1]);
     ct[i].v2=ntohl(*(int*)&p[5]);
     break;
    }
   }
   
   // just empty the list, it will be used at next iteration
   clearList(attr, (void(*)(void*))freeAttr);
   
  }
 }
 
 
 end:
 destroyList(attr, (void(*)(void*))freeAttr);
 
 return ret;
 
}



// --------------------------------------------------------------------
int ngadmin_setNetConf (struct ngadmin *nga, const struct net_conf *nc) {
 
 List *attr;
 struct swi_attr *sa;
 int ret=ERR_OK;
 
 
 if ( nga==NULL || nc==NULL ) {
  return ERR_INVARG;
 } else if ( (sa=nga->current)==NULL ) {
  return ERR_NOTLOG;
 }
 
 
 attr=createEmptyList();
 
 if ( nc->dhcp ) {
  pushBackList(attr, newByteAttr(ATTR_DHCP, 1));
 } else {
  pushBackList(attr, newByteAttr(ATTR_DHCP, 0));
  if ( nc->ip.s_addr!=0 ) pushBackList(attr, newAddrAttr(ATTR_IP, nc->ip));
  if ( nc->netmask.s_addr!=0 ) pushBackList(attr, newAddrAttr(ATTR_NETMASK, nc->netmask));
  if ( nc->gw.s_addr!=0 ) pushBackList(attr, newAddrAttr(ATTR_GATEWAY, nc->gw));
 }
 
 if ( (ret=writeRequest(nga, attr))!=ERR_OK ) {
  goto end;
 }
 
 
 if ( nc->dhcp ) {
  sa->nc.dhcp=true;
 } else {
  memcpy(&sa->nc, nc, sizeof(struct net_conf));
 }
 
 
 end:
 
 return ret;
 
}



// --------------------------------------------------
int ngadmin_getVLANType (struct ngadmin *nga, int *t) {
 
 List *attr;
 ListNode *ln;
 struct attr *at;
 int ret=ERR_OK;
 
 
 if ( nga==NULL || t==NULL ) {
  return ERR_INVARG;
 } else if ( nga->current==NULL ) {
  return ERR_NOTLOG;
 }
 
 
 attr=createEmptyList();
 pushBackList(attr, newEmptyAttr(ATTR_VLAN_TYPE));
 if ( (ret=readRequest(nga, attr))!=ERR_OK ) {
  goto end;
 }
 
 
 for (ln=attr->first; ln!=NULL; ln=ln->next) {
  at=ln->data;
  if ( at->attr==ATTR_VLAN_TYPE && at->size>=1 ) {
   *t= (int)*(char*)at->data ;
   break;
  }
 }
 
 
 end:
 destroyList(attr, (void(*)(void*))freeAttr);
 
 return ret;
 
}



// ------------------------------------------------------------------------------------------------------
int ngadmin_getVLANDotAllConf (struct ngadmin *nga, unsigned short *vlans, unsigned char *ports, int *nb) {
 
 List *attr;
 ListNode *ln;
 struct attr *at;
 struct swi_attr *sa;
 int ret=ERR_OK, total, i;
 char *p=NULL;
 
 
 if ( nga==NULL || vlans==NULL || ports==NULL || nb==NULL || *nb<=0 ) {
  return ERR_INVARG;
 } else if ( (sa=nga->current)==NULL ) {
  return ERR_NOTLOG;
 }
 
 
 total=*nb;
 *nb=0;
 
 attr=createEmptyList();
 pushBackList(attr, newEmptyAttr(ATTR_VLAN_DOT_CONF));
 if ( (ret=readRequest(nga, attr))!=ERR_OK ) {
  goto end;
 }
 
 
 for (ln=attr->first; ln!=NULL; ln=ln->next) {
  at=ln->data;
  p=at->data;
  if ( *nb>=sa->ports ) break; // no more room
  if ( at->attr==ATTR_VLAN_DOT_CONF && at->size>=4 ) {
   for (i=1; i<=sa->ports; ++i) {
    if ( (p[3]>>(sa->ports-i))&1 ) ports[i-1]=VLAN_TAGGED; // tagged
    else if ( (p[2]>>(sa->ports-i))&1 ) ports[i-1]=VLAN_UNTAGGED; // untagged
    else ports[i-1]=VLAN_NO;
   }
   *vlans++=ntohs(*(unsigned short*)p);
   ports+=sa->ports;
   ++*nb;
  }
 }
 
 
 end:
 destroyList(attr, (void(*)(void*))freeAttr);
 
 return ret;
 
}



// ----------------------------------------------------------------------------------------
int ngadmin_getVLANDotConf (struct ngadmin *nga, unsigned short vlan, unsigned char *ports) {
 
 List *attr;
 ListNode *ln;
 struct attr *at;
 struct swi_attr *sa;
 int ret=ERR_OK, i;
 char *p=NULL;
 
 
 if ( nga==NULL || ports==NULL ) {
  return ERR_INVARG;
 } else if ( (sa=nga->current)==NULL ) {
  return ERR_NOTLOG;
 }
 
 
 
 attr=createEmptyList();
 pushBackList(attr, newShortAttr(ATTR_VLAN_DOT_CONF, vlan));
 if ( (ret=readRequest(nga, attr))!=ERR_OK ) {
  goto end;
 }
 
 
 for (ln=attr->first; ln!=NULL; ln=ln->next) {
  at=ln->data;
  p=at->data;
  if ( at->attr==ATTR_VLAN_DOT_CONF && at->size>=4 ) {
   for (i=1; i<=sa->ports; ++i) {
    if ( (p[3]>>(sa->ports-i))&1 ) ports[i-1]=VLAN_TAGGED; // tagged
    else if ( (p[2]>>(sa->ports-i))&1 ) ports[i-1]=VLAN_UNTAGGED; // untagged
    else ports[i-1]=VLAN_NO;
   }
   break;
  }
 }
 
 
 end:
 destroyList(attr, (void(*)(void*))freeAttr);
 
 return ret;
 
}



// -------------------------------------------------------------
int ngadmin_getPVID (struct ngadmin *nga, unsigned short *ports) {
 
 List *attr;
 ListNode *ln;
 struct attr *at;
 struct swi_attr *sa;
 int ret=ERR_OK;
 char *p;
 
 
 if ( nga==NULL || ports==NULL ) {
  return ERR_INVARG;
 } else if ( (sa=nga->current)==NULL ) {
  return ERR_NOTLOG;
 }
 
 
 attr=createEmptyList();
 pushBackList(attr, newEmptyAttr(ATTR_VLAN_PVID));
 if ( (ret=readRequest(nga, attr))!=ERR_OK ) {
  goto end;
 }
 
 
 for (ln=attr->first; ln!=NULL; ln=ln->next) {
  at=ln->data;
  p=at->data;
  if ( at->attr==ATTR_VLAN_PVID && at->size>=3 && p[0]>=1 && p[0]<=sa->ports ) {
   ports[p[0]-1]=htons(*(unsigned short*)&p[1]);
  }
 }
 
 
 end:
 destroyList(attr, (void(*)(void*))freeAttr);
 
 return ret;
 
}




