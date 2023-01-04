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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.3.3.4 $ (DEC) $Date: 1992/06/25 14:26:16 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

SEC_PROGRAMS		= lpd lpr lpq lprm lpc
PROGRAMS		= ${SEC_PROGRAMS} pac lp cancel lpstat lptest
MSGHDRS			= printer_msg.h
CATFILES		= printer.cat

SUBDIRS			= filterlib filters vfilters

SEC_ILIST		= lpd lpr lpq lprm lpc
ILIST			= ${SEC_ILIST} pac lp cancel lpstat lptest
IDIR			= /usr/sbin/
BIN_IDIR		= /usr/bin/
cancel_IDIR		= ${BIN_IDIR}
lp_IDIR			= ${BIN_IDIR}
lpq_IDIR		= ${BIN_IDIR}
lpr_IDIR		= ${BIN_IDIR}
lprm_IDIR		= ${BIN_IDIR}
lpstat_IDIR		= ${BIN_IDIR}
lpd_IDIR		= /usr/lbin/
lpc_IOWNER		= root
lpc_IMODE		= 4711
lpd_IOWNER		= root
lpd_IMODE		= 4711
lpq_IOWNER		= root
lpq_IMODE		= 4711
lpr_IOWNER		= root
lpr_IMODE		= 4711
lprm_IOWNER		= root
lprm_IMODE		= 4711

CFLAGS			= ${SEC_DEFS}
lpr.o_CFLAGS		= -D${OBJECT_FORMAT} ${CFLAGS}
HFILES			= ${MSGHDRS}
lpc_OFILES		= lpc.o cmds.o cmdtab.o startdaemon.o common.o \
			  printcap.o
lpd_OFILES		= lpd.o printjob.o recvjob.o displayq.o rmjob.o \
			  startdaemon.o lpdchar.o common.o printcap.o \
			  ${SEC_DEFS?lpd_sec.o:}
lpq_OFILES		= lpq.o printcap.o common.o displayq.o
lpq_LIBS		= -ltermlib ${LIBS}
lpr_OFILES		= lpr.o startdaemon.o common.o printcap.o
lprm_OFILES		= lprm.o rmjob.o startdaemon.o common.o printcap.o
pac_OFILES		= pac.o printcap.o
lpstat_OFILES		= lpstat.o printcap.o
LIBS			= ${SEC_LIBS}
lpd_LIBS		= -lutil ${SEC_LIBS}

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/subdirs.mk

lpd.o lpr.o lpq.o lprm.o pac.o: lp.h lp.local.h
recvjob.o printjob.o displayq.o rmjob.o common.o: lp.h lp.local.h
lp.o lpstat.o cancel.o: lp.h lp.local.h
startdaemon.o: lp.local.h
lpc.o cmdtab.o: lpc.h
cmds.o: lp.h lp.local.h
