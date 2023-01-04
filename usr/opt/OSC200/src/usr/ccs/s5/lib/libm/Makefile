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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.3.5 $ (DEC) $Date: 1992/05/05 16:26:52 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

S5PATH			= /usr/opt/s5
VPATH			= ${TARGET_MACHINE}

LIBRARIES		= libm.a

EXPLIB_TARGETS		= export_libm.a

ILIST			= libm.a
IDIR			= ${S5PATH}/usr/lib/
IMODE			= 644

INCFLAGS		= -I${EXPORTBASE}/usr/opt/s5/usr/include
HFILES			= hdr.h
OFILES			= asin.o atan.o \
			  erf.o exp.o \
			  fabs.o floor.o fmod.o \
			  gamma.o hypot.o j0.o j1.o jn.o  log.o \
			  matherr.o modf.o pow.o sin.o sinh.o sqrt.o tanh.o \
			  tan.o isnan.o support.o

# Fix precision problems with bessel function.
jn.o_OPT_LEVEL		= -O1
j1.o_OPT_LEVEL		= -O1
j0.o_OPT_LEVEL		= -O1
fmod.o_OPT_LEVEL	= -O1

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk
-include ${TARGET_MACHINE}/machdep.mk

