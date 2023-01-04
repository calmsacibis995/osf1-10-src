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
# OSF/1 Release 1.0
#
VPATH			= ${MAKETOP}/kernel/arch/alpha:${MAKETOP}/kernel/arch/alpha/hal:${MAKETOP}/kernel/io/common

DATAFILES		= addrconf.h adudefs.h alpha_debug.h alpha_ptrace.h \
			  asm.h clock.h cons_sw.h coreregs.h counters.h \
			  cpu.h endian.h entrypt.h fpu.h gentrap.h \
			  hal_sysinfo.h handler.h inline_lock.h inst.h \
			  ka_adu.h kn7aa.h kn15aa.h kn430.h \
			  m_habitat.h machlimits.h machparam.h machtime.h \
			  pal.h  pcb.h pmap.h prom.h psl.h pte.h \
			  reg.h regdef.h rpb.h \
			  scb.h  softfp.h thread.h trap.h va_list.h varargs.h \
			  vm_tuning.h vmparam.h xpr.h devdriver.h cpuconf.h

INCLUDES		= ${DATAFILES}

#ILIST			= ${INCLUDES}
IDIR			= /usr/include/alpha/
IMODE			= 644

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
