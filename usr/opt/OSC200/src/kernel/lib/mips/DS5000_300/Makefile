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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/06/03 13:55:52 $
#
# Platform specific library makefile for DS5000_300 (3MAX+).

VPATH	= ${MAKETOP}/kernel/arch/mips/hal:${MAKETOP}/kernel/arch/mips

LIBR	= DS5000_300
CPU_DEF = -D${LIBR}

COBJECTS = 		\
	cpuconf.o	\
	kn_whatspl.o	\
	flush_cache.o	\
	cln_icache.o	\
	cln_dcache.o	\
	page_dflush.o	\
	page_iflush.o

ASSEMBLY_OBJECTS =  	\
	kn_wbflush.o	\
	kn_bcopy.o    	\
	kn_bzero.o	\
	kn_getspl.o	\
	kn_delay.o	\
	locore.o

all: lib_DS5000_300.O

include ${_FP_}/lib_forms.mk

-include ${PLATFORM_DEPEND}/${LIBR}.depend

