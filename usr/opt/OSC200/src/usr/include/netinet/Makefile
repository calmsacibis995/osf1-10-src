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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.3.5.3 $ (DEC) $Date: 1993/07/30 17:26:47 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

VPATH			= ${MAKETOP}/kernel/netinet

DATAFILES		= inet_config.h \
			  icmp_var.h if_ether.h \
			  igmp.h igmp_var.h in.h in_pcb.h \
			  in_systm.h in_var.h ip.h ip_icmp.h \
			  ip_mroute.h ip_var.h \
			  tcp.h tcp_debug.h tcp_fsm.h tcp_seq.h \
			  tcp_timer.h tcp_var.h tcpip.h \
			  udp.h udp_var.h if_trn.h if_trnstat.h
INCLUDES		= ${DATAFILES}

# See src/setup/postinstall.sh (search for HEADERS_SYMLINKS:)
#ILIST			= ${INCLUDES}
IDIR			= /usr/include/netinet/
IMODE			= 644

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
