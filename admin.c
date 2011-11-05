
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>



#include "protocol.h"
#include "command.h"
#include "controller.h"



#define MAXCOM	8
#define MAXSW	16


/*
getVlanStatus ()
setVlanMode (char mode)

makeCableTest (int n, const unsigned char* ports)

char speedIndex (const char* txt);
getBitrateStatus ()
setBiratePorts (int n, const unsigned char* ports, char index)

getBroadfiltStatus ()
disableBroadfilt ()
setBroadfiltPorts (int n, cons unsigned char* ports, char index)
*/


/*
sendPacket ()
receivePacket()
*/



void printErrCode (int err) {
 
 
 switch ( err ) {
  case ERR_OK: /*printf("ok\n");*/ break;
  case ERR_NOTLOG: printf("not switch selected\n"); break;
  case ERR_BADPASS: printf("wrong password\n"); break;
  default: printf("unknown status code (%i)\n", err);
 }
 
 
}


void displaySwitchList (const List *l) {
 
 ListNode *ln;
 struct swi_attr *sa;
 int i=0;
 
 
 
 if ( l->count==0 ) {
  printf("no switch found\n");
  return;
 }
 
 
 printf("Num\tMac\t\t\tProduct\t\tName\t\t\tIP/mask\n");
 
 for (ln=l->first; ln!=NULL; ln=ln->next) {
  sa=ln->data;
  printf("%i\t%s\t%s\t\t%s\t%s/", i, ether_ntoa(&sa->mac), sa->product, sa->name, inet_ntoa(sa->nc.ip));
  printf("%s\n", inet_ntoa(sa->nc.netmask));
  i++;
 }
 
 printf("\nfound %i switch(es)\n", l->count);
 
 
}




int main (int argc, char **argv) {
 
 char buffer[512], *com[MAXCOM], cont=1;
 int n, i;
 //struct termios tr;
 const List *swi;
 
 
 
 if ( argc<2 ) {
  printf("Usage: %s <interface>\n", argv[0]);
  return 1;
 }
 
 
 memset(com, 0, sizeof(com));
 
 if ( startNetwork(argv[1])<0 ) {
  printf("networking error\n");
  goto end;
 } 
 
 startController();
 swi=getSwitchList();
 
 
 //printf("client: ip %s, mac %s\n", inet_ntoa(local.sin_addr), ether_ntoa(&localmac));
 
 while ( cont ) {
  
  printf("> ");
  fflush(stdout);
  
  if ( fgets(buffer, sizeof(buffer), stdin)==NULL ) {
   if ( !feof(stdin) ) {
    perror("fgets");
   }
   goto end;
  }
  
  trim(buffer, strlen(buffer));
  n=explode(buffer, com, MAXCOM);
  
  /*
  } else if ( strcmp(com[0], "password")==0 ) {
   // password: specify password to use to login
   
   printf("enter password: ");
   fflush(stdout);
   
   tcgetattr(STDIN_FILENO, &tr);
   tr.c_lflag &= ~ECHO ;
   tcsetattr(STDIN_FILENO, TCSANOW, &tr);
   
   if ( fgets(password, sizeof(password), stdin)==NULL ) {
    perror("fgets");
   }
   
   tr.c_lflag |= ECHO ;
   tcsetattr(STDIN_FILENO, TCSANOW, &tr);
   
   putchar('\n');
   
   trim(password, strlen(password));
   
   
  } else if ( strcmp(com[0], "ports")==0 ) {
   // ports: show ports status
   
   if ( current==NULL ) {
    printf("no switch selected for configuration\n");
    goto nxt;
   }
   
   initNgPacket(&np);
   initNgHeader(np.nh, CODE_READ_REQ, &localmac, &current->mac, ++seq);
   
   addPacketEmptyAttr(&np, ATTR_PORT_STATUS);
   addPacketEmptyAttr(&np, ATTR_END);
   
   len=getPacketTotalSize(&np);
   remote.sin_addr.s_addr=htonl(INADDR_BROADCAST);
   remote.sin_port=htons(SWITCH_PORT);
   if ( sendto(sock, buffer, len, 0, (struct sockaddr*)&remote, sizeof(struct sockaddr_in))<0 ) {
    perror("sendto");
    goto nxt;
   }
   
   while ( (len=recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&remote, &slen))>=0 && !validateNgHeader(np.nh, &localmac, &current->mac, seq) );
   
   if ( len<0 ) {
    printf("time out\n");
    goto nxt;
   }
   
   initNgPacket(&np);
   l=extractPacketAttributes(&np);
   
   for (ln=l->first; ln!=NULL; ln=ln->next) {
    at=ln->data;
    char *p=at->data;
    if ( at->attr==ATTR_PORT_STATUS ) {
     printf("port %i: ", p[0]);
     switch ( p[1] ) {
      case 0: printf("down"); break;
      case SPEED_10: printf("up, 10M"); break;
      case SPEED_100: printf("up, 100M"); break;
      case SPEED_1000: printf("up, 1000M"); break;
      default: printf("unknown (%i)", p[2]);
     }
     putchar('\n');
    }
   }
   
   destroyList(l, (void(*)(void*))freeAttr);
   
  } else if ( strcmp(com[0], "mirror")==0 ) {
   // 
   
   if ( current==NULL ) {
    printf("no switch selected for configuration\n");
    goto nxt;
   }
   
   if ( n==1 ) {
    
    initNgPacket(&np);
    initNgHeader(np.nh, CODE_READ_REQ, &localmac, &current->mac, ++seq);
    
    addPacketEmptyAttr(&np, ATTR_MIRROR);
    addPacketEmptyAttr(&np, ATTR_END);
    
    len=getPacketTotalSize(&np);
    remote.sin_addr.s_addr=htonl(INADDR_BROADCAST);
    remote.sin_port=htons(SWITCH_PORT);
    if ( sendto(sock, buffer, len, 0, (struct sockaddr*)&remote, sizeof(struct sockaddr_in))<0 ) {
     perror("sendto");
     goto nxt;
    }
    
    
    while ( (len=recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&remote, &slen))>=0 && !validateNgHeader(np.nh, &localmac, &current->mac, seq) );
    
    if ( len<0 ) {
     printf("time out\n");
     goto nxt;
    }
    
    initNgPacket(&np);
    l=extractPacketAttributes(&np);
    
    for (ln=l->first; ln!=NULL; ln=ln->next) {
     at=ln->data;
     char *p=at->data;
     if ( at->attr==ATTR_MIRROR ) {
      if ( p[0]==0 ) {
       printf("mirorring disabled\n");
      } else {
       printf("ports ");
       // FIXME
       printf("redirected to port %i\n", p[0]);
      }
     }
    }
    
    destroyList(l, (void(*)(void*))freeAttr);
    
    
   } else {
    
    initNgPacket(&np);
    
    strcpy(np.ah->data, password);
    addPacketAttr(&np, ATTR_PASSWORD, strlen(password), NULL);
    
    if ( strcmp(com[1], "disable")==0 ) {
     
    }
    
    addPacketEmptyAttr(&np, ATTR_END);
    
    initNgHeader(np.nh, CODE_WRITE_REQ, &localmac, &current->mac, ++seq);
    
    len=getPacketTotalSize(&np);
    remote.sin_addr.s_addr=htonl(INADDR_BROADCAST);
    remote.sin_port=htons(SWITCH_PORT);
    if ( sendto(sock, buffer, len, 0, (struct sockaddr*)&remote, sizeof(struct sockaddr_in))<0 ) {
     perror("sendto");
     goto nxt;
    }
    
    
   }
   
   
  } else if ( strcmp(com[0], "clear")==0 ) {
   // 
   
   printf("\e[H\e[J");
   
  } else {
   // unknown command
   
   printf("unknown command\n");
   
  }
  */
  
  
  if ( n==0 ) {
   // nothing: do nothing
   
  } else if ( strcmp(com[0], "quit")==0 ) {
   // quit: exits
   
   cont=0;
   
  } else if ( strcmp(com[0], "scan")==0 ) {
   // scan: scan for switches
   
   if ( (i=discoverSwitches())<0 ) {
    printErrCode(i);
    goto nxt;
   }
   
   displaySwitchList(swi);
   
  } else if ( strcmp(com[0], "list")==0 ) {
   // list: display last detected switches
   
   displaySwitchList(swi);
   
  } else if ( strcmp(com[0], "login")==0 ) {
   // login: selects a switch from the list
   
   printErrCode(i=login(strtol(com[1], NULL, 0)));
   
  } else if ( strcmp(com[0], "ports")==0 ) {
   // 
   
   if ( n==1 ) {
    
    printf("ports\n");
    printf("\tstate\n");
    printf("\tstats\n");
    printf("\t\tshow\n");
    printf("\t\treset\n");
    
   } else if ( strcmp(com[1], "state")==0 ) {
    
    unsigned char ports[16];
    if ( (i=getPortsStatus(ports, 16))<0 ) {
     printErrCode(i);
     goto nxt;
    }
    
    for (i=0; i<8; i++) { // FIXME
     printf("port %i: ", i+1);
     switch ( ports[i] ) {
      case 0: printf("down"); break;
      case SPEED_10: printf("up, 10M"); break;
      case SPEED_100: printf("up, 100M"); break;
      case SPEED_1000: printf("up, 1000M"); break;
      default: printf("unknown (%i)", ports[i]);
     }
     putchar('\n');
    }
    
   }
   
  } else if ( strcmp(com[0], "password")==0 ) {
   // 
   
   if ( n==1 ) {
    printf("usage: password set|change\n");
   } else if ( strcmp(com[1], "set")==0 ) {
    printf("password set\n");
   } else if ( strcmp(com[1], "change")==0 ) {
    printf("password change\n");
   } else {
    printf("unknown password subcommand\n");
   }
   
  } else if ( strcmp(com[0], "timeout")==0 ) {
   // 
   
   
   
  } else {
   // unknown command
   
   printf("unknown command\n");
   
  }
  
  
  nxt:
  for (i=0; i<MAXCOM; i++) {
   if ( com[i]!=NULL ) {
    free(com[i]);
    com[i]=NULL;
   }
  }
  
 }
 
 
 end:
 stopController();
 stopNetwork();
 
 
 
 return 0;
 
}

