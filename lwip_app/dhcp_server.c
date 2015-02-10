/*
I managed to cobble a very simple DHCP server as follows. I didn't have to modify LWIP.  
Note: this DHCP server is brain dead - it is designed for a box that has only 2 IP devices: 
the microcontroller with LWIP and another IP device, and it hands out a hard coded answer.  
It works with this particular IP device and probably no others.

But it serves to show how the UDP packets are handled.

-Mark
*/



//Minimal DHCP server. 
// 

#include "dhcp_server.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "linux/string.h"
//#include "linux/task.h"
//#include "linux/modem.h"

const int forever = 1;

#define IPPORT_DHCPS 67
#define IPPORT_DHCPC 68

#define DHCPDISCOVER                    1
#define DHCPOFFER                       2
#define DHCPREQUEST                     3
#define DHCPACK                         5

/* 32-bit structure containing 4-digit ip number */
struct id_struct
{
    u8_t is_ip_addrs[4];        /* IP address number */
};

typedef struct 
{
    u8_t  dp_op;                           /* packet opcode type */
    u8_t  dp_htype;                        /* hardware addr type */
    u8_t  dp_hlen;                         /* hardware addr length */
    u8_t  dp_hops;                         /* gateway hops */
    u32_t dp_xid;                          /* transaction ID */
    u16_t dp_secs;                         /* seconds since boot began */
    u16_t dp_flags;
    u8_t  dp_ciaddr[4];                    /* client IP address */
    u8_t  dp_yiaddr[4];                    /* 'your' IP address */
    u8_t  dp_siaddr[4];                    /* server IP address */
    u8_t  dp_giaddr[4];                    /* gateway IP address */
    u8_t  dp_chaddr[16];                   /* client hardware address */
    u8_t  dp_legacy[192];                   
    u8_t  dp_magic[4];                   
    u8_t  dp_options[275];                    /* options area */
                        /* as of RFC2131 it is variable length */
}DHCP_TYPE;

static int32_t  DHCPSocket = -1;
char magic_cookie[] = {0x63,0x82,0x53,0x63};

char OptionNewSubnet[]   = {1,4,255,255,255,0};
char OptionNewOffer[]    = {53,1,DHCPOFFER};
char OptionNewAck[]      = {53,1,DHCPACK};
u8_t OptionEnd[] = {0xFF};

typedef struct {
    u8_t mac[6];
    u8_t ip[4];
} DHCP_Entry_t;

#define NUM_ENTRIES 3
DHCP_Entry_t dhcpTable[NUM_ENTRIES] = {
    {{0},{192,168,0,16}},
    {{0},{192,168,0,17}},
    {{0},{192,168,0,18}}
};


DHCP_TYPE DHCP_Buffer;
u8_t zeros[6] = {0,0,0,0,0,0};
static int InventAddress(u8_t mac[], u8_t ip[]) {
	int i;
	int j;
    // search for existing
    for(i=0;i<NUM_ENTRIES;i++) {
        if (memcmp(mac,dhcpTable[i].mac,6) == 0) {
            memcpy(ip,dhcpTable[i].ip,4);
            return 0;
         }
    }
    // create new one
    for(j=0;j<NUM_ENTRIES;j++) {
        if (memcmp(dhcpTable[j].mac,zeros,6) == 0) {
            memcpy(dhcpTable[j].mac,mac,6);
            memcpy(ip,dhcpTable[j].ip,4);
            return 0;
         }
    }
    return -1;
}

void XOS_TaskPrototype(void)
{
   struct sockaddr_in myAddr;
   struct sockaddr_in sourceAddr;
   struct sockaddr_in destinationAddr;
   struct ppp_addrs ppp_addr;
   int status;
   char * option_ptr;

   myAddr.sin_family      = AF_INET;
   myAddr.sin_port        = htons(IPPORT_DHCPS);
   myAddr.sin_addr.s_addr = INADDR_ANY;

   DHCPSocket = socket(AF_INET, SOCK_DGRAM, 0);
   status = bind(DHCPSocket, (struct sockaddr *)&myAddr,  sizeof (myAddr));

   printf("DHCP Server started\n");
   while ( forever )
   {
      status = recvfrom( DHCPSocket, (char *)&DHCP_Buffer, sizeof( DHCP_Buffer ), 0 , (struct sockaddr *)&sourceAddr, 0);

      printf("DHCP Server started\n");

      if ( status > 0 )
      {
         switch ( DHCP_Buffer.dp_options[2] )
         {
            case DHCPDISCOVER:
               printf("DHCP DISCOVER\n");
               if (InventAddress(DHCP_Buffer.dp_chaddr ,DHCP_Buffer.dp_yiaddr ) == 0) {
                   DHCP_Buffer.dp_op = 2;// reply
                   DHCP_Buffer.dp_secs = 0;
                   DHCP_Buffer.dp_flags = 0;
                   memset( &DHCP_Buffer.dp_options, 0, sizeof( DHCP_Buffer.dp_options ));
                   memcpy( DHCP_Buffer.dp_magic, magic_cookie, 4 );
                   
                   option_ptr = (char *)&DHCP_Buffer.dp_options;
                   memcpy( option_ptr, OptionNewOffer, sizeof(OptionNewOffer) );
                   option_ptr += sizeof(OptionNewOffer);
                   memcpy( option_ptr, OptionNewSubnet, sizeof(OptionNewSubnet) );
                   option_ptr += sizeof(OptionNewSubnet);
                   memcpy( option_ptr, OptionEnd, sizeof(OptionEnd) );
                   option_ptr += sizeof(OptionEnd);

                   destinationAddr.sin_port   = htons(IPPORT_DHCPC);
                   destinationAddr.sin_family = AF_INET;
                   destinationAddr.sin_addr.s_addr = INADDR_BROADCAST;
                   sendto( DHCPSocket, (char *)&DHCP_Buffer, sizeof( DHCP_Buffer ), 0 , (struct sockaddr *)&destinationAddr, sizeof( destinationAddr ));

                   printf("DHCPS: offer %02X:%02X:%02X:%02X:%02X:%02X -> %d.%d.%d.%d\n",
                          DHCP_Buffer.dp_chaddr[0],DHCP_Buffer.dp_chaddr[1],DHCP_Buffer.dp_chaddr[2],
                          DHCP_Buffer.dp_chaddr[3],DHCP_Buffer.dp_chaddr[4],DHCP_Buffer.dp_chaddr[5],
                          DHCP_Buffer.dp_yiaddr[0],DHCP_Buffer.dp_yiaddr[1],DHCP_Buffer.dp_yiaddr[2],DHCP_Buffer.dp_yiaddr[3]);
               } else {
                   printf("DHCPS: rejected discover, table full\n");
               }
               break;

            case DHCPREQUEST:
               printf("DHCP REQUEST\n");
               u8_t temp[4];
               if (InventAddress(DHCP_Buffer.dp_chaddr, temp ) == 0) {
                   DHCP_Buffer.dp_op = 2; // reply
                   DHCP_Buffer.dp_secs = 0;
                   DHCP_Buffer.dp_flags = 0;
                   option_ptr = (char *)&DHCP_Buffer.dp_options;
                   while (*option_ptr != 255) {
                       int len = option_ptr[1];
                       switch(*option_ptr) {
                       case 50:
                           // requested IP
                           if (len == 4) {
                               memcpy(DHCP_Buffer.dp_yiaddr,option_ptr+2,4);
                           }
                           break;
                       default:
                           break;
                       }
                       option_ptr += (len + 2);
                   }
                   if (memcmp(DHCP_Buffer.dp_yiaddr,temp,4)!=0) {
                       printf("ohoh, DHCP request did not match offer\n");
                   }
                   memcpy( DHCP_Buffer.dp_magic, magic_cookie, 4 );
                   // erase and create new options
                   memset( &DHCP_Buffer.dp_options, 0, sizeof( DHCP_Buffer.dp_options ));
                   option_ptr = (char *)&DHCP_Buffer.dp_options;
                   
                   // ACK message typ
                   *option_ptr++ = 53;
                   *option_ptr++ = 1;
                   *option_ptr++ = 5;

                   // renewal time
                   *option_ptr++ = 58;
                   *option_ptr++ = 4;
                   *option_ptr++ = 0;
                   *option_ptr++ = 0;
                   *option_ptr++ = 0x38;
                   *option_ptr++ = 0x40;

                  // rebinding time
                   *option_ptr++ = 59;
                   *option_ptr++ = 4;
                   *option_ptr++ = 0;
                   *option_ptr++ = 0;
                   *option_ptr++ = 0x62;
                   *option_ptr++ = 0x70;

                  // lease time (0x00010203 = 18 hours (66ksec)
                   *option_ptr++ = 51;
                   *option_ptr++ = 4;
                   *option_ptr++ = 0;
                   *option_ptr++ = 0;
                   *option_ptr++ = 0x70;
                   *option_ptr++ = 0x80;

                   // dhcp server identifier
                   *option_ptr++ = 54;
                   *option_ptr++ = 4;
                   *option_ptr++ = 192;
                   *option_ptr++ = 168;
                   *option_ptr++ = 0;
                   *option_ptr++ = 11;
                   
                   // subnet mask
                   *option_ptr++ = 1;
                   *option_ptr++ = 4;
                   *option_ptr++ = 0xFF;
                   *option_ptr++ = 0xFF;
                   *option_ptr++ = 0xFF;
                   *option_ptr++ = 0x00;
                   
                   *option_ptr++ = 15;
                   int len = sprintf((char*)option_ptr+1,"polecat.internal");
                   *option_ptr = (len + 1);
                   option_ptr += (len + 2);
                   
                   // router
                   *option_ptr++ = 3;
                   *option_ptr++ = 4;
                   *option_ptr++ = 192;
                   *option_ptr++ = 168;
                   *option_ptr++ = 0;
                   *option_ptr++ = 11;
                   
                   // domain name server (DNS)
                   *option_ptr++ = 6;
                   *option_ptr++ = 4;
                   if (ppp_addr.dns1.addr != 0) {
                       printf("Using DNS from ppp\n");
                       u8_t* addr = (u8_t*) &ppp_addr.dns1.addr;
                       *option_ptr++ = addr[0];
                       *option_ptr++ = addr[1];
                       *option_ptr++ = addr[2];
                       *option_ptr++ = addr[3];
                   } else {
                       *option_ptr++ = 192;
                       *option_ptr++ = 168;
                       *option_ptr++ = 0;
                       *option_ptr++ = 11;
                   }

                   // end
                   *option_ptr++ = 255;
                   

                   destinationAddr.sin_port        = htons(IPPORT_DHCPC);
                   destinationAddr.sin_family      = AF_INET;
                   destinationAddr.sin_addr.s_addr = INADDR_BROADCAST;
                   sendto( DHCPSocket, (char *)&DHCP_Buffer, sizeof( DHCP_Buffer ), 0 , (struct sockaddr *)&destinationAddr, sizeof( destinationAddr ));

                   printf("DHCPS: ack %02X:%02X:%02X:%02X:%02X:%02X -> %d.%d.%d.%d\n",
                          DHCP_Buffer.dp_chaddr[0],DHCP_Buffer.dp_chaddr[1],DHCP_Buffer.dp_chaddr[2],
                          DHCP_Buffer.dp_chaddr[3],DHCP_Buffer.dp_chaddr[4],DHCP_Buffer.dp_chaddr[5],
                          DHCP_Buffer.dp_yiaddr[0],DHCP_Buffer.dp_yiaddr[1],DHCP_Buffer.dp_yiaddr[2],DHCP_Buffer.dp_yiaddr[3]);
               } else {
                   printf("DHCPS: rejected Request, table full\n");
               }
               break;

            default:
               printf("DHCP unknown option %d\n", DHCP_Buffer.dp_options[2]);
               break;
         }
      } 
   }
}

/*
On 12/6/2012 10:51 AM, Louis Wells wrote:
Hello, 

I need to get a very simple dhcp server working on my board. All it is going to be used for is the ability to set up a connection if I need to directly plug a laptop into the board outside of a network. It will start up running dhcp client, and if it does not receive a response after sending 2 discover's it will switch to server mode. I just need to be able to reply to discover messages and send an offer and an ack after receiving a request.

I previously had everything running on uip, and am in the process of transitioning to lwip. I already have a dhcpd module that should be able to handle this, I just need to switch out the uip calls to lwip calls and get it to use the netif and the udp pcb in order to send packets and so on. 

Also for reference, I am using the raw api, and the tcpip thread.

It looks like I am going to have to modify udp_input to look for a packet that originated at DHCP_CLIENT_PORT and is addressed to DHCP_SERVER_PORT to pass the messages into my dhcpd module.

If i make these changes am I doing anything that will mess up lwip? Does it sound like what I am trying to do is the right idea ?

Also if anyone has experience getting this to work, any advice or recommendations would be appreciated !

Thanks

Louis
*/
