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
#	@(#)$RCSfile: template.mk,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:06:48 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# Copyright (c) 1988 Carnegie-Mellon University
# Copyright (c) 1987 Carnegie-Mellon University
# Copyright (c) 1986 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#  
#
# OSF/1 Release 1.0

######################################################################
#BEGIN	Machine dependent Makefile fragment for the Multimax
######################################################################

LDFLAGS=-e start memconf.mmax

FEATURES_EXTRA=mmax_*.h
LIBS=

#if	MACH
# LDOBJS_PREFIX=locore.o
#else	MACH
LDOBJS_LOCORE=	locopy_ns32k.o lodebug_ns32k.o loinit_ns32k.o \
		lomisc_ns32k.o lotrap_ns32k.o
LDOBJS_PREFIX=${LDOBJS_LOCORE}
#endif	MACH

LDDEPS_PREFIX=memconf.mmax

LDOBJS_SUFFIX=dbmon.o

CLEAN_EXTRA=mach kern

# mig generated c files require -Imach for gcc so that mach includes 
# can be found.  
# Makefile.template checks for $@_IMACH and adds a -Imach to the flags.
#
mach_user_internal.o_MIG =
exc_user.o_MIG = 
memory_object_default_user.o_MIG =
memory_object_user.o_MIG =

# To be consistent with the mips makefile, we define CCOPTS to determine what
# level of optimization we want.
CCPROFOPT=-pg
CCOPTS=${NO_GCC?-O:}

CCNFLAGS= -c ${CCOPTS} ${COPTS} ${DEFINES}
CCDFLAGS= -c ${CCOPTS} ${COPTS} ${DEFINES}
CCSFLAGS= -c ${CCOPTS} ${COPTS} ${DEFINES}
CCASFLAGS= -c ${ASOPTS} ${COPTS} ${DEFINES} -DASSEMBLER
CCPFLAGS= -c ${CCOPTS} ${PCOPTS} ${PROFOPTS} ${DEFINES}
CCPASFLAGS= -c ${ASOPTS} ${PCOPTS} ${PROFOPTS} ${DEFINES} -DASSEMBLER

# Extra for gcc. Define volatile to be __volatile for gcc with -traditional
#
GCC_EXTRAS=-Dvolatile=__volatile__ -Dsigned=__signed__ -traditional
GCC_OPTS=${NO_GCC?-Wc,-Z50 -q nosb -q volatile -q nopreload :${GCC_EXTRAS} -mnosb -mregparm}

DEPENDS=-MD

SYSDEPS_PREFIX=libvmunix

#
#  Compilation rules to generate .o from .c for normal files.
#  Multimax has special compiler switches, and sh doesn't understand
#  null commands between semicolons.
#
C_RULE_1A=${KCC} -c ${CCOPTS} ${COPTS} ${MMAX_COPTS}
C_RULE_1B=$*.c
C_RULE_2=
C_RULE_3=
C_RULE_4=

#
#  Compilation rules to generate .o from .c for profiling routine files.
#
C_RULE_1A_P=${KCC} -c ${CCOPTS} ${COPTS_P} ${MMAX_COPTS}
C_RULE_1B_P=$*.c
C_RULE_2_P=
C_RULE_3_P=
C_RULE_4_P=

#
# Compilation rules to generate .o from .c for driver files
#
C_RULE_1A_D=${C_RULE_1A}
C_RULE_1B_D=${C_RULE_1B}
C_RULE_2_D=${C_RULE_2}
C_RULE_3_D=${C_RULE_3}
C_RULE_4_D=${C_RULE_4}

#
#  Compilation rules to generate .o from .c for DYNAMIC OBJECT MODULES 
#
C_RULE_1A_DYN=${DCC} -pic-names ${VMUNIX_PICNAMES} -c ${CCOPTS} ${COPTS} \
	${MMAX_COPTS}
C_RULE_1B_DYN=$*.c
C_RULE_2_DYN=
C_RULE_3_DYN=
C_RULE_4_DYN=

#
#  Special rules to compile with symbols  These are inserted where
#	desired by hand editing the resulting Makefile.  It is also
#	necessary to remove the -x switch from the ld line to get symbols
#	into the image.
#
CG_RULE_1A=${KCC} -c -g ${COPTS} ${MMAX_COPTS}
CG_RULE_1A_D=${CG_RULE_1A}
CG_RULE_1A_P=${KCC} -c -g ${COPTS_P} ${MMAX_COPTS}

#
# Special for gcc - Compilation rules to generate .o from .s
# gcc only preprocesses assembler files if they are .S
#
S_RULE_1=${KCC} ${SFLAGS} -c $*.S
S_RULE_2=
S_RULE_3=


#
#  Special rules to generate the locore files.
#
LO_OBJ	= locopy_ns32k.o lodebug_ns32k.o loinit_ns32k.o lomisc_ns32k.o \
	  lotrap_ns32k.o dbmon.o

${LO_OBJ:.o=.S}: mmax/$${@:.S=.s}
	rm -f $@
	cp mmax/${@:.S=.s} $@

${LO_OBJ}: $${@:.o=.S} assym.s ${LOCORE_HEADS}
	${KCC} -c -DLOCORE -Immax ${SFLAGS} ${@:.o=.S}

#
#  Special rule to get memory configuration file (ld directives).
#
memconf.mmax: conf/memconf.mmax
	cp conf/memconf.mmax memconf.mmax

libvmunix: vmunix.sys
	echo ".text" > ${VMUNIX_LIB}.s
	${NM} -h -e -n vmunix.sys | \
	awk '-F|' '$$1 !~ /\./ {print $$1;}' | sort | uniq | \
	awk '{printf(".globl\t%s\n%s:\t.int 0\n",$$1,$$1);}' >> ${VMUNIX_LIB}.s
	${DAS} -o ${VMUNIX_LIB}.o ${VMUNIX_LIB}.s
	${DLD} -o ${VMUNIX_LIB}.so ${VMUNIX_LIB}.o -export kernel:
	rm -f ${VMUNIX_LIB}.o
	${NM} -h -e -n vmunix.sys | \
	awk '-F|' '$$1 !~ /\./ {print $$1;}' > ${VMUNIX_PICNAMES}

######################################################################
#END	Machine dependent Makefile fragment for the Multimax
######################################################################
