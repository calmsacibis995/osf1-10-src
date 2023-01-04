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
# (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# @(#)$RCSfile: Makefile,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/10 15:05:57 $
#
#
# HISTORY
#

PROGRAMS		= sh
MSGHDRS 		= sh_msg.h
CATFILES 		= sh.cat

ILIST			= sh
IDIR			= /usr/bin/

sh_ILINKS		= ${IDIR}Rsh

expand.o_OPT_LEVEL	=
OFILES 			= main.o args.o blok.o cmd.o ctype.o defs.o \
			  echo.o error.o expand.o fault.o func.o \
			  hash.o hashserv.o io.o macro.o \
			  msg.o name.o nls.o print.o pwd.o \
			  service.o setbrk.o stak.o string.o \
			  test.o word.o xec.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

main.o args.o blok.o cmd.o ctype.o \
	echo.o error.o expand.o fault.o func.o \
	hash.o hashserv.o io.o macro.o \
	msg.o name.o nls.o print.o pwd.o \
	service.o setbrk.o stak.o string.o \
	test.o word.o xec.o : ${MAKETOP}usr/bin/sh/defs.h sh_msg.h \
	${MAKETOP}usr/bin/sh/shctype.h \
	${MAKETOP}usr/bin/sh/mac.h ${MAKETOP}usr/bin/sh/mode.h \
	${MAKETOP}usr/bin/sh/name.h ${MAKETOP}usr/bin/sh/stak.h 
fault.o main.o: ${MAKETOP}usr/bin/sh/timeout.h 
hashserv.o xec.o: ${MAKETOP}usr/bin/sh/hash.h
pwd.o: ${MAKETOP}usr/bin/sh/mac.h
defs.o: ${MAKETOP}usr/bin/sh/name.h ${MAKETOP}usr/bin/sh/stak.h
fault.o main.o: ${MAKETOP}usr/bin/sh/timeout.h 
