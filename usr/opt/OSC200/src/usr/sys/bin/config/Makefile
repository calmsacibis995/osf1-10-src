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
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

DATAFILES	= config.h
VPATH		= ${MAKETOP}/kernel/src/config:${MAKETOP}/usr/ccs/lib/libAF
PROGRAMS	= config
ILIST		= config
IDIR		= /usr/sys/bin/
IMODE		= 555
IOWNER		= bin
IGROUP		= bin

SYMLINKS	= config
SYMLINKDIR1	= ../bin
SYMLINKDIR2	= /usr/sys/conf

USE_STATIC_LIBRARIES	=

INCFLAGS	= -I${MAKETOP}/kernel/src/config \
		  -I${MAKETOP}/kernel


OFILES		= y.tab.o lex.yy.o main.o mkioconf.o mkmakefile.o \
		  mkglue.o mkheaders.o mkswapconf.o mklocalconf.o \
		  AFclose.o AFgetatr.o AFgetent.o AFgetval.o      \
		  AFnxtatr.o AFnxtent.o AFnxtval.o AFopen.o       \
		  AFread.o AFrewind.o AFsetdflt.o

COMMON_CFLAGS	= -DCMU -D_NO_PROTO

ALPHA_CFLAGS	= -O0 ${COMMON_CFLAGS} -Dconst=

MIPS_CFLAGS	= ${COMMON_CFLAGS}

CFLAGS		= ${${TARGET_MACHINE}_CFLAGS}

LIBS		= -lln

I18N_FLAG	=

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/datafiles.mk

lex.yy.c: config.l
	lex config.l

y.tab.h y.tab.c: config.y
	yacc -d config.y

y.tab.o: config.h
lex.yy.o: y.tab.h config.h
main.o: y.tab.h config.h
mkioconf.o: y.tab.h config.h
mkmakefile.o: y.tab.h config.h
mkglue.o: config.h y.tab.h
mkheaders.o: config.h y.tab.h
mkswapconf.o: config.h
