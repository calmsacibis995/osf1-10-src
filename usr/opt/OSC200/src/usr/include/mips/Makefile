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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.5.3.7 $ (DEC) $Date: 1992/05/06 10:46:31 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#
# VPATH			= ${MAKETOP}/kernel/dec/machine/mips:${MAKETOP}/kernel/dec/machine/common

VPATH		= ${MAKETOP}/kernel/arch/mips:${MAKETOP}/kernel/arch/mips/hal:${MAKETOP}/kernel/io/common

DATAFILES		= addrconf.h asm.h clock.h cons.h cons_sw.h \
			  coreregs.h counters.h cpu.h \
			  endian.h entrypt.h fixpoint.h fpu.h frame.h \
			  fudge.xper.h handler.h hwconf.h inline_lock.h \
			  inst.h kn02ba.h kn02ca.h kn210.h kn220.h kn5800.h \
			  m_habitat.h machlimits.h machparam.h machtime.h \
			  mc146818clock.h mips_ptrace.h nexus.h \
			  pcb.h pmap.h psl.h reg.h regdef.h scb.h \
			  setjmp.h softfp.h ssc.h thread.h \
			  utsname.h varargs.h vm_tuning.h vmparam.h \
			  xpr.h devdriver.h cpuconf.h hal_sysinfo.h

INCLUDES		= ${DATAFILES}

#
# These are the header file(s) needed for the compiler bootstrap on ULTRIX
#

CMPLRS_DATAFILES        = endian.h

                                                                                
# See src/setup/postinstall.sh (search for HEADERS_SYMLINKS:)
#ILIST			= ${INCLUDES}
IDIR			= /usr/include/mips/
IMODE			= 644

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk

cmplrs_hdrs:            ${CMPLRS_DATAFILES}
