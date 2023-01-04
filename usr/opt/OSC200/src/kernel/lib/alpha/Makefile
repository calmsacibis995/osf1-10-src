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
#
#
# HISTORY
#
#
#	@(#)$RCSfile: Makefile,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/17 14:46:40 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

# Generic kernel architecture library makefile. 

VPATH	= ${MAKETOP}kernel/arch/alpha

LIBR	= alpha



COBJECTS =  		\
	alpha_debug.o		\
	alpha_exception.o	\
	alpha_init.o		\
	clock.o			\
	copy.o			\
	alpha_ptrace.o		\
	in_cksum.o		\
	super.o			\
	kernargs.o		\
	mem.o			\
	machdep.o		\
	crc.o			\
	ovbcopy.o		\
	md.o			\
	pcb.o			\
	pmap.o			\
	prom.o			\
	startup.o		\
	trap.o			\
	syscall_trap.o		\
	intr.o			\
	ul.o			\
	vm_machdep.o		

ASSEMBLY_OBJECTS =  		\
	context.o		\
	divide.o		\
	locore.o		\
	in_checksum.o		\
	byte_swap.o		\
	spl.o			\
	usercopy.o		\
	pal_lib.o		

all:  lib_alpha.a 
include ${_FP_}/lib_forms.mk

-include alpha.depend
