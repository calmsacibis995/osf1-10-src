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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/06/04 09:19:06 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0


#

VPATH			= ../dump

PROGRAMS		= restore rrestore
MSGHDRS			= restore_msg.h
CATFILES		= restore.cat

ILIST			= restore rrestore
IDIR			= /usr/sbin/

CCTYPE			= writable_strings
CFLAGS			= -DDECOSF
dirs.o_CFLAGS		= ${CFLAGS} -D_BSD 
rtape.o_CFLAGS		= ${CFLAGS} -DREMOTE
dumprmt.o_CFLAGS	= ${CFLAGS} -DREMOTE -DRESTORE
dumprmt.o_INCFLAGS	= -I../dump
HFILES			= restore.h restore_msg.h
OBJS			= main.o interactive.o restore.o dirs.o symtab.o \
			  utilities.o chfl.o restoreextern.o
restore_OFILES		= ${OBJS} tape.o
rrestore_OFILES		= ${OBJS} rtape.o dumprmt.o
GARBAGE			= rtape.c dumprmt.c

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

rtape.c: tape.c
	${RM} ${_RMFLAGS_} rtape.c
	${CP} tape.c rtape.c
