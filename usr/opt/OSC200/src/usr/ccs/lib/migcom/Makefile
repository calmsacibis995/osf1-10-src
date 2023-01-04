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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.2.4 $ (DEC) $Date: 1992/05/26 15:10:33 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

PROGRAMS		= migcom
EXPLIB_TARGETS		= export_migcom

ILIST			= migcom
IDIR			= /usr/ccs/lib/

CFLAGS			= -DALIGN_8
error.o_CFLAGS		= ${NO_VFPRINTF?-DNO_VFPRINTF:}
parser.c_YFLAGS		= -d
OFILES			= parser.o lexxer.o migcom.o \
			  error.o string.o type.o routine.o \
			  statement.o global.o \
			  header.o user.o server.o utils.o

LIBS			= -lln

GARBAGE			= parser.c parser.h

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

parser.h: parser.c
	-if [ -f y.tab.h ]; then ${MV} -f y.tab.h parser.h; fi

lexxer.o: parser.h
