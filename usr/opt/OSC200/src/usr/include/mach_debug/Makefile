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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.3.10.4 $ (DEC) $Date: 1993/04/20 12:39:23 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

VPATH			= ${MAKETOP}/kernel/mach_debug

SUBDIRS			= ${target_machine}
EXPINC_SUBDIRS		= ${SUBDIRS}
MIG_DEFS		= mach_debug.defs
DATAFILES		= ${MIG_DEFS} callout_statistics.h ipc_statistics.h \
			  mach_debug_types.defs mach_debug_types.h zone_info.h
OTHERS			= machine mach_debug.h
MIG_HDRS		= ${MIG_DEFS:.defs=.h}
INCLUDES		= ${DATAFILES} ${OTHERS} ${MIG_HDRS}

ILIST			= ${MIG_HDRS}
IDIR			= /usr/sys/include/mach_debug/
EXPDIR			= /usr/include/mach_debug/
IMODE			= 644

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
include ${MAKEFILEPATH}/datafiles.mk
include ${MAKEFILEPATH}/objects.mk

${MIG_DEFS:.defs=.h}: $${@:.h=.defs}

machine: ${ALWAYS}
	${RM} ${_RMFLAGS_} machine
	ln -s ${target_machine} machine
