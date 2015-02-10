#ifndef __DHCP_SERVER_H__
#define __DHCP_SERVER_H__ 1

#include "lwip/opt.h"
#include <stddef.h> /* for size_t */
#include "lwip/inet.h"
#include "lwip/err.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/ip_addr.h"
#include "lwip/api.h"
#include "lwip/dns.h"

/* members are in network byte order */
struct sockaddr_in {
  u8_t sin_len;
  u8_t sin_family;
  u16_t sin_port;
  struct in_addr sin_addr;
  char sin_zero[8];
};
/* Socket protocol types (TCP/UDP/RAW) */
#define SOCK_STREAM     1
#define SOCK_DGRAM      2
#define SOCK_RAW        3


#define AF_UNSPEC       0
#define AF_INET         2
#define PF_INET         AF_INET
#define PF_UNSPEC       AF_UNSPEC

struct ppp_addrs {
  ip_addr_t our_ipaddr, his_ipaddr, netmask, dns1, dns2;
};

#endif /* __DHCP_SERVER_H__ */

