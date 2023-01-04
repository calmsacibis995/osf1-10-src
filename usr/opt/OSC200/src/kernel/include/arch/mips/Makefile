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
#	@(#)$RCSfile: Makefile,v $ $Revision: 1.2.4.3 $ (DEC) $Date: 1992/06/03 12:30:18 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
# VPATH	= ${MAKETOP}/kernel/arch/${MACHINE}:${MAKETOP}/kernel/arch/${MACHINE}/hal
VPATH	= ${MAKETOP}/kernel/arch/${MACHINE}

DATAFILES		= addrconf.h asm.h coreregs.h counters.h cpu.h \
			  endian.h fixpoint.h fpu.h frame.h \
			  fudge.xper.h hwconf.h inline_lock.h inst.h \
			  m_habitat.h \
			  machlimits.h machparam.h machtime.h \
			  mips_ptrace.h nexus.h \
			  pcb.h pmap.h psl.h reg.h regdef.h \
			  setjmp.h softfp.h ssc.h thread.h \
			  utsname.h varargs.h vm_tuning.h vmparam.h xpr.h \
			  vm_ubc.h
ILIST	= ${DATAFILES}
IDIR	= /usr/sys/include/arch/mips/
IMODE	= 444
IGROUP	= bin
IOWNER	= bin

SUBDIRS			= hal

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
include ${MAKEFILEPATH}/subdirs.mk
