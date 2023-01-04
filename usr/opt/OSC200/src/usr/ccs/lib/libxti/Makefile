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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/05/12 19:40:18 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

VPATH			= ../libtli
MSGHDRS			= libtli_msg.h
CATFILES		= libtli.cat
HFILES			= ${MSGHDRS}

LIBRARIES		= libxti.a

EXPLIB_TARGETS		= export_libxti.a

ILIST			= libxti.a
IDIR			= /usr/ccs/lib/
IMODE			= 644

#CFLAGS			= -DXTI -DXTIDBG -DXTIBDBG
CFLAGS			= -DXTI
OFILES=		errno.o iostate.o taccept.o talloc.o tbind.o tblock.o \
		tclose.o tconnect.o tdbg.o terrno.o terrnolst.o \
		terror.o tfixup.o tfree.o tgetinfo.o tgetst.o \
		tlisten.o tlook.o tnblock.o topen.o toptmgmt.o trcv.o \
		trcvcon.o trcvdis.o trcvrel.o trudata.o truderr.o tsnd.o \
		tsnddis.o tsndrel.o tsudata.o tsync.o tunbind.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk
