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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.2.3 $ (DEC) $Date: 1992/12/11 14:39:37 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

PROGRAMS		= gprof
DATAFILES		= gprof.flat gprof.callg

ILIST			= $(PROGRAMS) $(DATAFILES)
IDIR			= /usr/sbin/
LIB_IDIR		= /usr/ccs/lib/
gprof.flat_IDIR		= ${LIB_IDIR}
gprof.callg_IDIR	= ${LIB_IDIR}
gprof.flat_IMODE	= 644
gprof.callg_IMODE	= 644

HFILES			= gprof.h
OFILES			= gprof.o arcs.o dfn.o lookup.o hertz.o \
			  printgprof.o printlist.o calls_${MACHINE}.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/datafiles.mk
