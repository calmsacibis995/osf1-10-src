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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:21:04 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

VPATH	= ${MAKETOP}/kernel/mips:${MAKETOP}/kernel/mips/${TARGET_MACHINE}

DATAFILES		= addrconf.h asm.h clock.h cpu.h cpu_board.h \
			  dispatcher.h endian.h entrypt.h fixpoint.h \
			  fpu.h handler.h hwconf.h inline_lock.h \
			  inst.h machlimits.h machparam.h machtime.h \
			  mips_ptrace.h \
			  pcb.h pmap.h reg.h regdef.h setjmp.h \
			  softfp.h thread.h utsname.h varargs.h \
			  vm_tuning.h vmparam.h xpr.h \
			  ${${TARGET_MACHINE}_DATAFILES}

include ${TARGET_MACHINE}/machdep.mk
include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
