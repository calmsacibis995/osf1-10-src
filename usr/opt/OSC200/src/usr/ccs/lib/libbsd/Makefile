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
# (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
# HISTORY
# 11/11/91 - Fred Canter
#	Removed open routine from libbsd (breaks many programs).
#	Rlogind and telnetd are two examples.
#
# 10/03/91 - Tom Peterson
#	added open.o to OFILES line so the open function in open.c
#	would be included.
#
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.3.3.4 $ (DEC) $Date: 1992/11/24 10:16:57 $
#
# (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0.2
#

VPATH			= ../libc

LIBRARIES		= libbsd.a

EXPLIB_TARGETS		= export_libbsd.a

ILIST			= libbsd.a
IDIR			= /usr/ccs/lib/
IMODE			= 644

CFLAGS			= -D_BSD
OFILES			= ftime.o mktemp.o nice.o rand.o regex.o siginter.o \
			  signal.o timezone.o util.o valloc.o \
			  vtimes.o wait.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk
