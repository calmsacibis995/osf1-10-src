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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:50:45 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

VPATH			= ${MAKETOP}/usr/bin/sh

PROGRAMS		= sh
MSGHDRS 		= sh_msg.h
CATFILES 		= sh.cat

ILIST			= sh
IDIR			= /sbin/
ILINKS			= ${IDIR}Rsh

USE_STATIC_LIBRARIES	=

INCFLAGS		= -I${MAKETOP}/usr/bin/sh
OFILES 			= main.o args.o blok.o cmd.o ctype.o defs.o \
			  echo.o error.o expand.o fault.o func.o \
			  hash.o hashserv.o io.o macro.o \
			  msg.o name.o nls.o print.o pwd.o \
			  service.o setbrk.o stak.o string.o \
			  test.o word.o xec.o

LDFLAGS			= ${LDSTRIP}
GLINE			=

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

sh:	sh_msg.h
defs.h: brkincr.h shctype.h mac.h mode.h name.h stak.h sh_msg.h
expand.o macro.o service.o: shctype.h
args.o blok.o cmd.o ctype.o dosetpath.o echo.o error.o expand.o test.o: defs.h
fault.o func.o io.o macro.o main.o msg.o name.o print.o service.o: defs.h
hash.o hashserv.o nls.o pwd.o setbrk.o stak.o string.o word.o xec.o: defs.h
io.o main.o: dup.h
hashserv.o xec.o: hash.h
pwd.o: mac.h
service.o: mode.h
defs.o: name.h
defs.o main.o stak.o: stak.h
cmd.o macro.o main.o msg.o name.o word.o xec.o: sym.h
fault.o main.o: timeout.h
