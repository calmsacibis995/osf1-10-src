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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/03/09 13:49:36 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

PROGRAMS		= bootpd

ILIST			= bootpd
IDIR			= /usr/sbin/

# The next few lines may be uncommented and changed to alter the default
# filenames bootpd uses for its configuration and dump files.
CONFFILE=-DCONFIG_FILE=\"/etc/bootptab\"
#DUMPFILE=-DBPDUMP_FILE=\"/usr/etc/bootpd.dump\"
FILESPECS=${CONFFILE} ${DUMPFILE}
BOOTPDOPT=-DSYSLOG -DDEBUG -DPRIVATE=static  ${FILESPECS}

CFLAGS=${BOOTPDOPT}
#CFLAGS			= -D_BSD -D_SOCKADDR_LEN
HFILES			= bootp.h bootpd.h hash.h
OFILES			= bootpd.o readfile.o hash.o version.o
LIBS			= -lbsd -lutil

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

version.c: newvers.sh Version.c
	sh newvers.sh Version.c
