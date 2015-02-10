#
# (C) Copyright 2015
# Mleaf_HeXi
# mail:350983773@qq.com
# www.hifirouter.com
#


include $(TOPDIR)/config.mk

# CFLAGS += -DET_DEBUG -DDEBUG

LIB	= liblwipapp.a

OBJS	= dhcp_server.o sys_arch.o
all:	$(LIB)

$(LIB):	$(START) $(OBJS)
	$(AR) crv $@ $(OBJS)
CFLAGS += -I../extra_include/
#########################################################################

.depend:	Makefile $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
