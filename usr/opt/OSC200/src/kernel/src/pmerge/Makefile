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
# HISTORY
# 
#
# Makefile to build test programs
#
#
# NOTE: Since ODE cannot handle any "mcc" rules, we will define
# and do everyting by hand. Handles only mips and alpha.

OTHERS		= pmerge

MIPS_CC		= COMP_HOST_ROOT=/ COMP_TARGET_ROOT=/ mcc -DKERNEL
ALPHA_CC	= COMP_HOST_ROOT=/ COMP_TARGET_ROOT=/ cc
#INCLUDES= -I -I/usr/projects/alpha/wp/lib/_alpha.d/include \
#	    -I/usr/projects/alpha/wp/lib/_alpha.d/hostmips64/usr/include
MIPS_INCLUDES	= -I -I../../include -I../../include/sysV
ALPHA_INCLUDES	= -I../../include `genpath -I.`

COPTS_P=${CDEBUGOPTS} ${DEPENDS} ${ALLOPTS} ${GCC_OPTS}
COPTS=  ${COPTS_P} ${${@}_MIG?-Imach:}
CFLAGS= ${COPTS} ${CPP_ONLY}

INCFLAGS	= ${${TARGET_MACHINE}_INCLUDES}
#
#  Common assembly flags
#
SFLAGS=	${DEPENDS} ${ALLOPTS} ${GCC_OPTS} -DASSEMBLER
ASFLAGS=-c ${INCLUDES}

DDIR            = ../../bin
IDIR            = /isl/sas/

ILIST           = pmerge

PROGRAMS		= pmerge
CCTYPE			= host
USE_STATIC_LIBRARIES	=
OFILES			= main.o loader.o
HFILES			= pmerge.h

CFLAGS= ${${TARGET_MACHINE}_CFLAGS}

MIPS_LIBS = -lc
ALPHA_LIBS = -lmld

LIBS	= ${${TARGET_MACHINE}_LIBS}

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
