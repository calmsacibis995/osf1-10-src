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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:50:08 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

VPATH			= ${MAKETOP}/usr/sbin/named

PROGRAMS		= named
MSGHDRS			= named_msg.h
CATFILES		= named.cat

ILIST			= named
IDIR			= /sbin/

USE_STATIC_LIBRARIES	=

INCFLAGS		= -I${MAKETOP}/usr/sbin/named
HFILES			= ${MSGHDRS}
LDFLAGS			= ${LDSTRIP}
GLINE			=

CCTYPE                  = writable_strings
#CFLAGS                 = -D_BSD  -D_SOCKADDR_LEN -DDEBUG -DSTATS
CFLAGS                  = -D_BSD  -D_SOCKADDR_LEN -UDEBUG -DSTATS
HFILES                  = ${MSGHDRS} db.h ns.h pathnames.h
#ALLOC                  = storage.o
named_OFILES            = db_dump.o db_load.o db_glue.o db_lookup.o \
                          db_reload.o db_save.o db_update.o ns_forw.o \
                          ns_init.o ns_main.o ns_maint.o ns_req.o ns_resp.o\
                          ns_sort.o ns_stats.o\
                          ${ALLOC} version.o
LIBS                    = -lbsd -lutil
GARBAGE                 = version version.c

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

version.c: newvers.sh Version.c version.ucb
	sh newvers.sh Version.c version.ucb
