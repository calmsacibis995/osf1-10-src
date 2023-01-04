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
# 	@(#)$RCSfile: Makefile,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/01/30 09:16:38 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

PROGRAMS		= xntpd
CFLAGS			= -DDEBUG -DREFCLOCK -DLOCAL_CLOCK -DPST

ILIST			= xntpd
IDIR			= /usr/sbin/
OFILES			= ntp_config.o ntp_control.o ntp_io.o ntp_leap.o \
			  ntp_loopfilter.o ntp_monitor.o ntp_peer.o ntp_proto.o \
			  ntp_refclock.o ntp_request.o ntp_restrict.o ntp_timer.o \
			  ntp_unixclock.o ntp_util.o ntpd.o \
			  refclock_conf.o refclock_local.o refclock_pst.o \
			  version.o
 
LIBS			= -lntp

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
