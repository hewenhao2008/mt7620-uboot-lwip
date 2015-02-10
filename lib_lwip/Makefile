#
# (C) Copyright 2015
# Mleaf_HeXi
# mail:350983773@qq.com
# www.hifirouter.com
#
include $(TOPDIR)/config.mk

# CFLAGS += -DET_DEBUG -DDEBUG
LIB	= liblwip.a

OBJS= api_lib.o api_msg.o err.o netbuf.o netdb.o netifapi.o sockets.o tcpip.o def.o dhcp.o dns.o init.o mem.o memp.o netif.o pbuf.o raw.o stats.o sys.o tcp.o tcp_in.o tcp_out.o timers.o udp.o autoip.o icmp.o igmp.o inet.o inet_chksum.o ip.o ip_addr.o ip_frag.o etharp.o ethernetif.o slipif.o
vpath %.c lwip/src/api/ lwip/src/core/ lwip/src/netif/ lwip/src/core/ipv4/
all:	$(LIB)

$(LIB):	$(START) $(OBJS)
	$(AR) crv $@ $(OBJS)
CFLAGS += -I../extra_include/
#########################################################################

.depend:	Makefile $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################