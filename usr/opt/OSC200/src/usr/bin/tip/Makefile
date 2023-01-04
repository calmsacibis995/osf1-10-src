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

SUBDIRS			= aculib etcfiles
PROGRAMS		= tip
MSGHDRS			= tip_msg.h
CATFILES		= tip.cat


ILIST			= tip
IDIR			= /usr/bin/
IOWNER			= uucp
IGROUP			= uucp
IMODE			= 2755

CFLAGS			= -D_BSD -DDEFBR=1200 -DDEFFS=BUFSIZE -DACULOG -DPRISTINE -DGENACU
remcap.o_CFLAGS		= -DBUFSIZE=1024 ${CFLAGS}
acutab.o_CFLAGS		= -DV831 -DVENTEL -DHAYES -DCOURIER -DDECMOD ${CFLAGS}
HFILES			= tip_msg.h tip.h pathnames.h
OFILES			= acu.o cmds.o cmdtab.o cu.o hunt.o log.o \
			  partab.o remote.o tip.o tipout.o value.o \
			  vars.o acutab.o remcap.o uucplock.o gen.o
LIBS			= -lbsd aculib/aculib.a
LINTFLAGS		= -hbacvx

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

