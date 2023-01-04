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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/06/03 14:06:34 $
#
#
# Platform specific library makefile for hal 

VPATH	= ${MAKETOP}/kernel/arch/mips/hal

LIBR	= hal
XFLAGS  = -DHAL_LIBRARY ${XFLAGS}

COBJECTS = \
	cktodr.o	\
	cnfg_delay.o	\
	inittodr.o	\
	resettodr.o	\
	microtime.o	\
	cpuconf.o	\
	cpusw.o		\
	cpusw_subr.o	\
	spl_rt.o	\
	kn_getspl_sub.o	\
	kn_whatspl.o	\
	flush_cache.o	\
	cln_icache.o	\
	cln_dcache.o	\
	page_iflush.o	\
	page_dflush.o	\
	parity.o	\
	mc146818clock.o	\
	cons_sw.o	\
	hal_sysinfo.o	\
	kn_hardclock.o	\
	kn220_hrdclck.o	\
	kn220_delay.o	\
	kn01.o		\
	kn02.o		\
	kn02ba.o	\
	kn02ca.o	\
	kn03.o		\
	kn220.o		\
	kn230.o		\
	ssc.o		\
	autoconf.o	\
	console.o	\
	dump.o		\
	mp_support.o	\
	mips_debug.o	\
	intr.o		\
	hal_lmf.o

ASSEMBLY_OBJECTS = \
	kn_wbflush.o	\
	kn_cache_ops.o	\
	spl.o		\
	kn02ba_spl.o	\
	iplis0.o	\
	issplsched.o	\
	kn_getspl.o	\
	clr_bev.o	\
	clr_cpe.o	\
	get_cause.o	\
	get_sr.o	\
	kn_delay.o	\
	kn230_copy.o	\
	console_entry.o	\
	clr_be_intr.o	\
	kn2xx.o

all:  lib_hal.a

include ${_FP_}/lib_forms.mk

-include ${PLATFORM_DEPEND}/${LIBR}.depend
