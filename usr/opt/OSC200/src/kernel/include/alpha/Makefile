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
#	@(#)Makefile	9.2	(ULTRIX/OSF)	10/24/91
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
# OSF/1 Release 1.0

VPATH	= ${MAKETOP}/kernel/alpha:${MAKETOP}/kernel/alpha/${TARGET_MACHINE}

DATAFILES		= addrconf.h adudefs.h alpha_debug.h asm.h clock.h \
			  cons_sw.h coreregs.h counters.h cpu.h \
			  dispatcher.h endian.h entrypt.h \
			  handler.h iic.h inline_lock.h inst.h ka_adu.h \
			  kn15aa.h kn16aa.h kn7aa.h kn430.h \
			  machlimits.h machparam.h machtime.h pal.h \
			  pcb.h pmap.h prom.h psl.h reg.h regdef.h
			  rpb.h scb.h softfp.h sysinfo.h thread.h trap.h \
			  varargs.h vm_tuning.h vmparam.h xpr.h \
			  ${${TARGET_MACHINE}_DATAFILES}

ILIST	= ${DATAFILES}
IDIR	= /usr/sys/include/alpha/
IMODE	= 444
IGROUP	= bin
IOWNER	= bin

include ${TARGET_MACHINE}/machdep.mk
include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
