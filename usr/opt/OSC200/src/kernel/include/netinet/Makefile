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
#
# @(#)$RCSfile: Makefile,v $ $Revision: 4.4.7.3 $ (DEC) $Date: 1993/05/16 20:10:16 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#

VPATH			= ${MAKETOP}/kernel/netinet

#DATAFILES		= in.h in_systm.h in_var.h if_ether.h proto_inet.h \
#			  if_fddi.h

DATAFILES		= icmp_var.h if_ether.h if_fddi.h igmp.h igmp_var.h \
			  in.h in_pcb.h in_systm.h in_var.h inet_config.h \
			  ip.h ip_icmp.h ip_mroute.h \
			  ip_var.h proto_inet.h tcp.h \
			  tcp_debug.h tcp_fsm.h tcp_seq.h tcp_timer.h \
			  tcp_var.h tcpip.h udp.h udp_var.h \
			  if_trn.h if_trnstat.h
ILIST   = ${DATAFILES}
IDIR    = /usr/sys/include/netinet/
IMODE   = 444
IGROUP  = bin
IOWNER  = bin


include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk

