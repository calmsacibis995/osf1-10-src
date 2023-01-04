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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/15 11:16:55 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0



PROGRAMS		= dump rdump
MSGHDRS			= dump_msg.h
GARBAGE			= dumprmain.c dumproptr.c dumprtape.c

ILIST			= dump rdump
IDIR			= /usr/sbin/

CFLAGS			= -DDECOSF

dumprmain.o_CFLAGS	= -DREMOTE -DDECOSF
dumproptr.o_CFLAGS	= -DREMOTE -DDECOSF
dumprtape.o_CFLAGS	= -DREMOTE -DDECOSF
dumprmt.o_CFLAGS	= -DREMOTE -DDUMP -DDECOSF

HFILES			= ${MSGHDRS} dump.h

dump_OFILES		= \
			  dumpcheck.o		\
			  dumpextern.o		\
			  dumpitime.o		\
			  dumpmain.o		\
			  dumpoptr.o		\
			  dumptape.o		\
			  dumptraverse.o	\
			  unctime.o

rdump_OFILES		= \
			  dumpextern.o		\
			  dumpitime.o		\
			  dumprmain.o		\
			  dumprmt.o		\
			  dumproptr.o		\
			  dumprtape.o		\
			  dumptraverse.o	\
			  unctime.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

dumprmain.c: dumpmain.c
	${RM} -f dumprmain.c
	${CP} dumpmain.c dumprmain.c

dumproptr.c: dumpoptr.c
	${RM} -f dumproptr.c
	${CP} dumpoptr.c dumproptr.c

dumprtape.c: dumptape.c
	${RM} -f dumprtape.c
	${CP} dumptape.c dumprtape.c
