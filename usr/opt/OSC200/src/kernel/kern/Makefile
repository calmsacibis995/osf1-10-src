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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.9.3 $ (DEC) $Date: 1994/01/19 19:16:35 $

# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#

VPATH			= ${MAKETOP}/kernel/kern

#DATAFILES		= zalloc.h lock.h xpr.h macro_help.h queue.h assert.h \
#			  thread.h event.h ast.h processor.h task.h timer.h \
#			  thread_modes.h sched.h kern_msg.h mach_param.h \
#			  kern_obj.h kern_set.h syscall_emulation.h \
#			  ipc_netport.h msg_queue.h threadcall.h parallel.h

DATAFILES		= server_loop.c

ILIST   = ${DATAFILES}
IDIR    = /usr/sys/include/kern/
IMODE   = 444
IGROUP  = bin
IOWNER  = bin


include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk

