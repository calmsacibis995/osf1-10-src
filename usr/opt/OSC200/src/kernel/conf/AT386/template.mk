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
#	@(#)$RCSfile: template.mk,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:06:44 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#  
#
# OSF/1 Release 1.0

###############################################################################
#BEGIN	Machine dependent Makefile fragment for the i386
###############################################################################

CC=GCC_EXEC_PREFIX=; CPATH=/usr/include; LPATH=/lib:/usr/lib; export GCC_EXEC_PREFIX CPATH LPATH; /bin/cc
KCC=${A_OUT_GCC_EXEC_PREFIX}gcc -traditional

#
# Special for gcc - Compilation rules to generate .o from .s
# gcc only preprocesses assembler files if they are .S
#
S_RULE_1=${KCC} -E ${CFLAGS} -D${TARGET_MACHINE} -DASSEMBLER $*.S > $*.pp
S_RULE_2=sed '/^\#/d' $*.pp > $*.s
S_RULE_3=${KCC} -c $*.s

mach_user_internal.o_MIG =
exc_user.o_MIG = 
memory_object_default_user.o_MIG =
memory_object_user.o_MIG =

LDOBJS_PREFIX= ${ORDERED} locore.o
LOCORE_DEPS=	assym.s i386/start.s \
		i386/locore.s i386/cswitch.s


LOCORE_HEADS= 


locore.S: ${LOCORE_DEPS}
	cat ${LOCORE_DEPS} >locore.S
	@echo The hack below should get fixed sometime.
	cp /dev/null ioconf.c

locore.o: locore.S
	${S_RULE_1}
	${S_RULE_2}
	${S_RULE_3}


LDFLAGS=-e pstart

OTHERS = FPU_end.o FPU_start.o arith.o dcode.o divmul.o lipsq.o reg.o \
         remsc.o round.o status.o store.o subadd.o trans.o 

${OTHERS}: i386/fp/$${@:=.uu}
	uudecode i386/fp/${@:=.uu}



###############################################################################
#END	Machine dependent Makefile fragment for the i386
###############################################################################
