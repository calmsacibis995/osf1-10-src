#
# *****************************************************************
# *                                                               *
# *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
# *                                                               *
# *   All Rights Reserved.  Unpublished rights  reserved  under   *
# *   the copyright laws of the United States.                    *
# *                                                               *
# *   The software contained on this media  is  proprietary  to   *
# *   and  embodies  the  confidential  technology  of  Digital   *
# *   Equipment Corporation.  Possession, use,  duplication  or   *
# *   dissemination of the software and media is authorized only  *
# *   pursuant to a valid written license from Digital Equipment  *
# *   Corporation.                                                *
# *                                                               *
# *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
# *   by the U.S. Government is subject to restrictions  as  set  *
# *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
# *   or  in  FAR 52.227-19, as applicable.                       *
# *                                                               *
# *****************************************************************
#
#
# HISTORY
#
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/11/01 21:44:31 $
#
# Possible Options:
#  -DCSLIP	include support for kernel slip interface
#  -DPPP	include support for point to point protocol
#  -DIBMRTPC	enable the MERIT additions to the Stanford Enetfilter
#  -DNOVFPRINTF	simulate vfprintf() on systems that don't have it
#
# CSLIP and PPP work only under BPF.
#
#DEFINES = -DCSLIP -DPPP -DFDDI

PROGRAMS		= tcpdump tcpslice
ILIST                   = ${PROGRAMS}
IDIR                    = /usr/sbin/
GARBAGE			= lex.yy.c y.tab.c tokdefs.h tcpgram.c tcplex.c \
			  version.h version.c
YFLAGS			= -vd
CFLAGS			= -DPACKETFILTER -DFDDI -std

# TODO: -ldnet
LIBS			= -lc -ll

CSRC			= tcpdump.c addrtoname.c print-ether.c print-ip.c \
			  print-arp.c print-tcp.c print-udp.c print-atalk.c \
			  print-domain.c print-tftp.c print-bootp.c \
			  print-nfs.c print-icmp.c nametoaddr.c print-sl.c \
			  print-ppp.c print-rip.c print-snmp.c print-ntp.c \
			  print-null.c print-egp.c print-ospf.c print-fddi.c \
			  print-sunrpc.c savefile.c util.c etherent.c inet.c \
			  print-llc.c print-decnet.c print-isoclns.c gencode.c \
			  optimize.c bpf_dump.c bpf_filter.c bpf_image.c \
			  md-alpha.c os-osf1.c pcap-pf.c \
			  tcpslice.c gwtm2secs.c search.c

tcpdump_OFILES		= tcpdump.o addrtoname.o print-ether.o print-ip.o \
			  print-arp.o print-tcp.o print-udp.o print-atalk.o \
			  print-domain.o print-tftp.o print-bootp.o \
			  print-nfs.o print-icmp.o nametoaddr.o print-sl.o \
			  print-ppp.o print-rip.o print-snmp.o print-ntp.o \
			  print-null.o print-egp.o print-ospf.o print-fddi.o \
			  print-sunrpc.o savefile.o util.o etherent.o inet.o \
			  print-llc.o print-decnet.o print-isoclns.o gencode.o \
			  optimize.o bpf_dump.o bpf_filter.o bpf_image.o \
			  md-alpha.o os-osf1.o pcap-pf.o tcplex.o tcpgram.o \
			  version.o

tcpslice_OFILES		= tcpslice.o gwtm2secs.o search.o savefile.o \
			  bpf_filter.o md-alpha.o util.o

HFILES			= interface.h appletalk.h addrtoname.h bootp.h \
			  mib.h savefile.h etherproto.h nametoaddr.h \
			  etherent.h ntp.h gencode.h extract.h ospf.h \
			  fddi.h llc.h decnet.h md-alpha.h os-osf1.h

CFILES			= $(CSRC) tcplex.l

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

AWKS			= atime.awk packetdat.awk send-ack.awk stime.awk

# These files are automatically generated.
#GEN			= tokdefs.h tcpgram.c tcplex.c version.h version.c

md.h:	md-alpha.h
	-cp md-alpha.h md.h
os.h:	os-osf1.h
	-cp os-osf1.h os.h
#tokdefs.h: tcpgram.o
#	-mv y.tab.h tokdefs.h

#tcplex.c: tcplex.l
#	rm -f $@
#	$(LEX) $<
#	mv -f lex.yy.c tcplex.c

tcpgram.c: tcpgram.y
	rm -f tcpgram.c tokdefs.h
	$(YACC) $(YFLAGS) $<
	mv y.tab.c tcpgram.c
	mv y.tab.h tokdefs.h

version.o: version.c
version.c version.h: VERSION
	rm -f version.c ; sed 's/.*/char version[] = "&";/' VERSION > version.c
	set `sed 's/\([0-9]*\)\.\([0-9]*\).*/\1 \2/' VERSION` ; \
		{ echo '#define VERSION_MAJOR' $$1 ; \
		  echo '#define VERSION_MINOR' $$2 ; } > version.h

savefile.o: version.h
tcpslice.o: version.h
tokdefs.h: tcpgram.c

# local file list from a make depend:
tcpdump.o:	interface.h os.h
tcpdump.o:	md.h savefile.h addrtoname.h
addrtoname.o:	interface.h os.h
addrtoname.o:	md.h addrtoname.h nametoaddr.h
addrtoname.o:	etherent.h llc.h
print-ether.o:	interface.h os.h md.h
print-ether.o:	addrtoname.h etherproto.h
print-ip.o:	interface.h os.h md.h addrtoname.h
print-arp.o:	md.h addrtoname.h
print-tcp.o:	interface.h os.h
print-tcp.o:	md.h addrtoname.h
print-udp.o:	addrtoname.h appletalk.h bootp.h
print-atalk.o:	interface.h os.h md.h
print-atalk.o:	addrtoname.h appletalk.h
print-atalk.o:	extract.h
print-domain.o:	interface.h os.h
print-domain.o:	md.h addrtoname.h
print-bootp.o:	interface.h os.h
print-bootp.o:	md.h addrtoname.h bootp.h
print-nfs.o:	interface.h os.h
print-nfs.o:	md.h
print-icmp.o:	interface.h os.h
print-icmp.o:	md.h addrtoname.h
nametoaddr.o:	md.h etherent.h nametoaddr.h
nametoaddr.o:	decnet.h etherproto.h
print-rip.o:	interface.h os.h
print-rip.o:	md.h addrtoname.h
print-snmp.o:	interface.h os.h
print-snmp.o:	md.h addrtoname.h mib.h
print-ntp.o:	interface.h os.h
print-ntp.o:	md.h addrtoname.h ntp.h
print-null.o:	interface.h os.h
print-null.o:	md.h addrtoname.h
print-egp.o:	interface.h os.h
print-egp.o:	md.h addrtoname.h
print-ospf.o:	ospf.h
print-ospf.o:	interface.h os.h md.h
print-ospf.o:	addrtoname.h
print-fddi.o:	interface.h os.h md.h
print-fddi.o:	addrtoname.h etherproto.h fddi.h
print-sunrpc.o:	interface.h
print-sunrpc.o:	os.h md.h addrtoname.h
print-sunrpc.o:	extract.h
savefile.o:	version.h savefile.h
util.o:		interface.h os.h
util.o:		md.h
etherent.o:	interface.h os.h
etherent.o:	md.h etherent.h
inet.o:		interface.h os.h
inet.o:		md.h
print-llc.o:	interface.h os.h
print-llc.o:	md.h addrtoname.h llc.h
print-decnet.o:	decnet.h
print-decnet.o:	interface.h os.h
print-decnet.o:	md.h addrtoname.h
print-isoclns.o:	md.h addrtoname.h etherproto.h
gencode.o:	interface.h os.h
gencode.o:	md.h gencode.h nametoaddr.h
gencode.o:	extract.h etherproto.h
optimize.o:	interface.h os.h
optimize.o:	md.h
optimize.o:	gencode.h
tcpslice.o:	savefile.h version.h
search.o:	interface.h os.h
search.o:	md.h savefile.h
tcpgram.o:	interface.h os.h
tcpgram.o:	md.h
tcpgram.o:	gencode.h
tcplex.o:	gencode.h tokdefs.h
