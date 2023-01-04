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
#	@(#)$RCSfile: template.mk,v $ $Revision: 4.3.3.5 $ (DEC) $Date: 1992/06/03 12:01:02 $
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
#BEGIN	Machine dependent Makefile fragment for the MIPS
######################################################################
# ------------------------------------------------------------------
# | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights |
# | Reserved.  This software contains proprietary and confidential |
# | information of MIPS and its suppliers.  Use, disclosure or     |
# | reproduction is prohibited without the prior express written   |
# | consent of MIPS.                                               |
# ------------------------------------------------------------------
#
# Makefile for 4.3 bsd on MIPS Computer Systems Charleston architecture
#

#
# To build a Mach-O kernel define MACHO in the config file
#
KCC=${COFF_GCC_EXEC_PREFIX}cc

RM=		rm -f
GPSIZE=		${MACHO?0:18}

# don't pass -EL to gcc
#
ENDIAN=${NO_GCC?-EL:}

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
# CCDFLAGS - device drivers
# CCSFLAGS - swap*.c files
# CCASFLAGS - *.s files
# CCPFLAGS - profiling *.c files
# CCPASFLAGS - profiling *.s files
#
# CCOPTS may be set from the config file
# CDEBUGOPTS may be set from the config file
# ASOPTS may be set from the config file
# ENDIAN may be set from the config file
#
CCPROFOPT=-p
CCOPTS=-D__mips__=1 -D__MIPSEL__=1 -Dmips=1 -DMIPSEL=1

CCNFLAGS= -c -O2 ${ENDIAN} ${CCOPTS} ${COPTS} ${DEFINES}
CCDFLAGS= -c -O0 ${ENDIAN} ${CCOPTS} ${COPTS} ${DEFINES}
CCSFLAGS= -c -O2 ${ENDIAN} ${CCOPTS} ${COPTS} ${DEFINES}
# HACK: should this be ASOPTS in config file? Fred Canter -- 5/1/91
CCASFLAGS= -c -O0 ${ENDIAN} ${ASOPTS} ${COPTS} ${DEFINES} -DASSEMBLER -DMIPSEL=1 -Dmips=1
CCPFLAGS= -c ${ENDIAN} ${CCOPTS} ${PCOPTS} ${PROFOPTS} ${DEFINES}
CCPASFLAGS= -c ${ENDIAN} ${ASOPTS} ${PCOPTS} ${PROFOPTS} ${DEFINES} -DASSEMBLER

# flags for dynamic kernel module linking
#DCC_NFLAGS= -pic-names ${VMUNIX_PICNAMES} -c ${ENDIAN} ${CCOPTS} -MD -G 0 \
#	${CDEBUGOPTS} ${ALLOPTS} ${GCC_OPTS} ${INCLUDES} ${IDENT} \
#	${ALLDEFINES} ${DEFINES}
#DCC_DFLAGS= -pic-names ${VMUNIX_PICNAMES} -c ${ENDIAN} ${CCOPTS} -MD -G 0 \
#	${CDEBUGOPTS} ${ALLOPTS} ${GCC_OPTS} ${INCLUDES} ${IDENT} \
#	${ALLDEFINES} ${DEFINES}
DCC_PFLAGS= -pic-names ${VMUNIX_PICNAMES} -c ${ENDIAN} ${CCOPTS} \
	${PCOPTS} ${PROFOPTS} ${DEFINES}
DCOPTS_P=${CDEBUGOPTS} -G 0 ${ALLOPTS} ${GCC_OPTS}
DCOPTS=  ${DCOPTS_P} ${${@}_MIG?-Imach:}
DCC_NFLAGS= -c -O2 ${ENDIAN} ${CCOPTS} ${DCOPTS} ${DEFINES}
DCC_DFLAGS= -c -O0 ${ENDIAN} ${CCOPTS} ${DCOPTS} ${DEFINES}

#
LDOBJS_PREFIX=entry.o
LDFLAGS= ${MACHO?:${ENDIAN} -G ${GPSIZE} -N} -T ${TEXTBASE} -e start ${LDOPTS} -non_shared

# define volatile to be __volatile__ for gcc with -traditional
#
GCC_EXTRAS=-std0 -EL -signed
GCC_OPTS=${NO_GCC?:${GCC_EXTRAS}}

${SOBJS}: assym.s

#DEPENDS=${NO_GCC?-G ${GPSIZE}:-MD -G ${GPSIZE}}
DEPENDS=-G ${GPSIZE}

SYSDEPS_PREFIX=libvmunix


libvmunix: vmunix.sys


######################################################################
#END	Machine dependent Makefile fragment for the MIPS
######################################################################
