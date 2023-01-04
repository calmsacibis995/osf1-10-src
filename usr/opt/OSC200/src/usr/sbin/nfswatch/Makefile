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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/08/31 17:53:58 $
#
PROGRAMS		= nfswatch nfslogsum
ILIST			= ${PROGRAMS}
IDIR			= /usr/sbin/

LIBS			= -lcurses -ltermcap -lm
CFLAGS                  = -DUSE_PFILT

nfswatch_OFILES		= dlpi.o logfile.o netaddr.o nfswatch.o nit.o \
			  pfilt.o pktfilter.o rpcfilter.o rpcutil.o \
			  screen.o snoop.o util.o xdr.o
nfslogsum_OFILES	= nfslogsum.o

dlpi.o:			nfswatch.h externs.h os.h
logfile.o:		nfswatch.h externs.h screen.h os.h
netaddr.o:		nfswatch.h externs.h os.h
nfslogsum.o:		nfswatch.h os.h
nfswatch.o:		nfswatch.h os.h
nit.o:			nfswatch.h externs.h os.h
pfilt.o:		nfswatch.h externs.h os.h
pktfilter.o:		nfswatch.h externs.h os.h
rpcfilter.o:		nfswatch.h externs.h rpcdefs.h os.h
rpcutil.o:		nfswatch.h externs.h rpcdefs.h os.h
screen.o:		nfswatch.h externs.h screen.h os.h
snoop.o:		nfswatch.h externs.h os.h
util.o:			nfswatch.h externs.h screen.h os.h
xdr.o:			nfswatch.h os.h

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
