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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/01/08 15:34:33 $
#
#
# OSF/1 Release 1.0

VPATH		= ${MAKETOP}/usr/ccs/bin/make

PROGRAMS	= make
MSGHDRS		= make_msg.h
CATFILES	= make.cat

ILIST		= make
IDIR		= /sbin/

USE_STATIC_LIBRARIES	=

INCFLAGS	= -I${MAKETOP}/usr/ccs/bin/make
OFILES		= main.o doname.o misc.o files.o dirs.o libs.o \
		  rules.o dosys.o gram.o dyndep.o \
		  ${ENVIRONMENT} ${REWINDDIR}
HFILES		= defs.h ${MSGHDRS}
LINTFLAGS	= -ax ${CFLAGS}
GARBAGE		= gram.c

libs.o_CFLAGS	= -D${ARCHIVE_FORMAT} ${CFLAGS}
main.o_CFLAGS	= -DMACHINE=\"${MACHINE}\"
rules.o_CFLAGS	= -Dtarget_MACHINE=\"TARGET_MACHINE=${TARGET_MACHINE}\" \
		  -Dtarget_machine=\"target_machine=${target_machine}\" \
		  -Dthis_CPUTYPE=\"CPUTYPE=${CPUTYPE}\" \
		  -Dthis_cputype=\"cputype=${cputype}\"

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

