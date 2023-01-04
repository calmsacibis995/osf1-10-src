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
# HISTORY
# 
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
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement
# specifies the terms and conditions for use and redistribution.
#
# OSF/1 Release 1.0
######################################################################
#BEGIN	Machine dependent Makefile fragment for ALPHA
######################################################################
#

#
# To build a Mach-O kernel define MACHO in the config file
#
KCC=${_CC_EXEC_PREFIX}cc

RM=		rm -f
GPSIZE=		${MACHO?0:4}

ACC_ENDIAN	= 
MCC_ENDIAN	= -EL
GCC_ENDIAN	=

ENDIAN=${${CC_SUITE}_ENDIAN}

# mig generated c files require -Imach for gcc so that mach includes 
# can be found.  
# Makefile.template checks for $@_IMACH and adds a -Imach to the flags.
#
mach_user_internal.o_MIG =
exc_user.o_MIG = 
memory_object_default_user.o_MIG =
memory_object_user.o_MIG =

#
# COPTS is recognized by config and massaged for profiling kernels
#
# cc options for most kernel sources
# CCNFLAGS - normal files
# CCFFLAGS - normal files allowed to do floating point
# CCDFLAGS - device drivers
# CCSFLAGS - swap*.c files
# CCASFLAGS - *.s files
# CCASFFLAGS - *.s files allowed to do floating point
# CCPFLAGS - profiling *.c files
# CCPASFLAGS - profiling *.s files
#
# CCOPTS may be set from the config file
# CDEBUGOPTS may be set from the config file
# ASOPTS may be set from the config file
# ENDIAN may be set from the config file
#
CCPROFOPT=-Wb,-p2
CCOPTS= -DLANGUAGE_C

# need this for alpha
# afd add -v for debug
DEFINES= -D__alpha -Umips -UMIPS

CCNFLAGS= ${CPP_ONLY} -c -O2 -g3 ${ENDIAN} ${CCOPTS} ${COPTS} ${DEFINES}
CCFFLAGS= ${CPP_ONLY} -c -O2 -g3 ${ENDIAN} ${CCOPTS} ${CFOPTS} ${DEFINES}
CCDFLAGS= ${CPP_ONLY} -c -O0 -g3 ${ENDIAN} ${CCOPTS} ${COPTS} ${DEFINES}
CCSFLAGS= ${CPP_ONLY} -c -O2 -g3 ${ENDIAN} ${CCOPTS} ${COPTS} ${DEFINES}
CCASFLAGS= ${CPP_ONLY} -c -O0 -g3 ${ENDIAN} ${ASOPTS} ${COPTS} ${DEFINES} -DASSEMBLER
CCASFFLAGS= ${CPP_ONLY} -c -O0 -g3 -std ${ENDIAN} ${ASOPTS} ${CFOPTS} ${DEFINES} -DASSEMBLER
CCPFLAGS= -c -g3 ${ENDIAN} ${CCOPTS} ${PCOPTS} ${PROFOPTS} ${DEFINES}
CCPASFLAGS= -c -O0 -g3 ${ENDIAN} ${ASOPTS} ${PCOPTS} ${PROFOPTS} ${DEFINES} -DASSEMBLER

DCC_PFLAGS= -pic-names ${VMUNIX_PICNAMES} -c ${ENDIAN} ${CCOPTS} \
	${PCOPTS} ${PROFOPTS} ${DEFINES}
DCOPTS_P=${CDEBUGOPTS} -G 0 ${ALLOPTS} ${${CC_SUITE}_OPTS}
DCOPTS=  ${DCOPTS_P} ${${@}_MIG?-Imach:}
DCC_NFLAGS= -c -O2 ${ENDIAN} ${CCOPTS} ${DCOPTS} ${DEFINES}
DCC_DFLAGS= -c -O0 ${ENDIAN} ${CCOPTS} ${DCOPTS} ${DEFINES}

LDOBJS_PREFIX=scb.o entry.o
LDFLAGS= ${MACHO?:${ENDIAN} -G ${GPSIZE} -N} -w2 -O2 -T ${TEXTBASE} -e start ${LDOPTS}

# define volatile to be __volatile__ for gcc with -traditional
#
ACC_OPTS=-signed
MCC_OPTS=
GCC_EXTRAS=-signed
GCC_OPTS=${NO_GCC?:${GCC_EXTRAS}}

${SOBJS}: assym.s

#DEPENDS=${NO_GCC?-G ${GPSIZE}:-MD -G ${GPSIZE}}
DEPENDS=-G ${GPSIZE}

SYSDEPS_PREFIX=libvmunix


libvmunix: vmunix.sys


######################################################################
#END	Machine dependent Makefile fragment for Alpha
######################################################################
