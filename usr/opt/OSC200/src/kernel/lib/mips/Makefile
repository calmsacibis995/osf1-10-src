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
#	@(#)$RCSfile: Makefile,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/06/03 14:04:06 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

# Generic kernel architecture library makefile. 

VPATH	= ${MAKETOP}kernel/arch/mips:${MAKETOP}kernel/arch/mips/usercopy.d

LIBR	= mips
XFLAGS	= -DMIPS_LIBRARY ${XFLAGS}

ASSEMBLY_OBJECTS =  	\
	in_checksum.o	\
	ns_checksum.o	\
	byte_swap.o	\
			\
	scanc.o		\
	ffs.o		\
	ffintr.o	\
			\
	addupc.o    	\
	bcmp.o    	\
	bcopy.o    	\
	bzero.o    	\
	clearseg.o    	\
	copyin.o    	\
	copyinstr.o    	\
	copyout.o    	\
	copyoutstr.o   	\
	copystr.o    	\
	fubyte.o    	\
	fuword.o    	\
	strlen.o    	\
	subyte.o    	\
	suword.o    	\
	uload_half.o    \
	uload_uhalf.o   \
	uload_word.o    \
	ustore_half.o   \
	ustore_word.o  	\
			\
	config_cache.o 	\
	size_cache.o 	\
			\
	adderr.o 	\
	cerror.o 	\
	cstrerror.o 	\
	fixade_error.o 	\
	uerror.o	\
	context.o	\
	coproc_cntrl.o	\
	emulate_instr.o	\
	softfp.o	\
	softfp_unusbl.o	\
	fp_intr.o	\
	locore.o	\
	badaddr.o	\
	tlb.o

COBJECTS =		\
	xtob.o		\
	in_cksum.o	\
	ns_cksum.o	\
	strcmp.o	\
	strncmp.o	\
	strcpy.o	\
	strncpy.o	\
	ovbcopy.o	\
	crc.o		\
	sr_masks.o	\
	mips_init.o	\
	cache.o		\
	hwconf.o	\
	kopt.o		\
	machdep.o	\
	mem.o		\
	mips_excptn.o	\
	mips_ptrace.o	\
	pmap.o		\
	pcb.o		\
	startup.o	\
	sys_machdep.o	\
	sysmips.o	\
	trap.o		\
	syscall_trap.o	\
	tlb_trap.o	\
	vm_machdep.o

all:  lib_mips.a 

include ${_FP_}/lib_forms.mk

-include ${MACHINE_DEPEND}/${LIBR}.depend


