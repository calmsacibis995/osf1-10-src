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
#	@(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/01/22 16:47:04 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

VPATH			= ${MAKETOP}/usr/sbin/routed
USE_STATIC_LIBRARIES	=

PROGRAMS		= routed
MSGHDRS			= routed_msg.h
CATFILES		= routed.cat

ILIST			= routed
IDIR			= /isl/

CFLAGS			= -D_BSD -D_SOCKADDR_LEN
HFILES			= ${MSGHDRS} defs.h af.h defs.h interface.h \
			  table.h trace.h
OFILES			= af.o if.o input.o main.o output.o startup.o\
			  tables.o timer.o trace.o inet.o
LIBS			= -lbsd -lutil

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
