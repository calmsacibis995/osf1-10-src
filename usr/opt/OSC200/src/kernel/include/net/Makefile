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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.4.9.3 $ (DEC) $Date: 1993/05/08 10:44:50 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#

VPATH			= ${MAKETOP}/kernel/net

DATAFILES		= bpf.h ether_driver.h gw_screen.h \
			  if.h if_arp.h if_dl.h if_llc.h \
			  if_slvar.h if_trn_sr.h if_types.h \
			  net_globals.h net_malloc.h \
			  net_unixlock.h netisr.h pfilt.h pfiltdefs.h \
			  proto_net.h proto_uipc.h \
			  radix.h raw_cb.h route.h slcompress.h

ILIST   = ${DATAFILES}
IDIR    = /usr/sys/include/net/
IMODE   = 444
IGROUP  = bin
IOWNER  = bin


include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk

