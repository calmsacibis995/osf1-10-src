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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/10 19:20:36 $
#
#
# HISTORY
#

PROGRAMS		= pax
MSGHDRS			= pax_msg.h
CATFILES		= pax.cat

ILIST			= ${PROGRAMS}
IDIR			= /usr/bin/
#ILINKS			= ${IDIR}tar ${IDIR}cpio

HFILES			= config.h func.h pax.h pax_msg.h

pax_OFILES		= pax.o append.o buffer.o charmap.o cpio.o \
			  create.o extract.o fileio.o\
			  hash.o link.o list.o mem.o namelist.o names.o \
			  pass.o pathname.o replace.o tar.o \
			  ttyio.o warn.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

charmap.o: charmap.h
