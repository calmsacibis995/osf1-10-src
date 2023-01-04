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
#	@(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1992/08/25 09:13:52 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

# Generic kernel architecture library makefile. 

VPATH	= ${MAKETOP}/kernel/arch/alpha/hal

LIBR	= hal
XFLAGS  = -DHAL_LIBRARY

COBJECTS =  		\
	console.o	\
	cpuconf.o	\
	hal_sysinfo.o	\
	cpusw.o		\
	adu_cons.o	\
	ruby_cons.o	\
	cobra_cons.o	\
	autoconf.o	\
	cons_sw.o	\
	ka_adu.o	\
	kn7aa.o		\
	kn15aa.o	\
	hal_lmf.o	\
	kn430	.o	

all:  lib_hal.a 
include ${_FP_}/lib_forms.mk

-include hal.depend
