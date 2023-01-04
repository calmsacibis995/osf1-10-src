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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.4.14.2 $ (DEC) $Date: 1993/08/13 19:01:06 $

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

DATAFILES		= assert.h       ipc_tt.h          queue.h \
			  ast.h          kalloc.h          sched.h \
			  event.h        kdb_print.h       sched_prim.h \
			  host.h         kern_mon.h        sched_prim_macros.h \
			  ipc_basics.h   kern_msg.h        syscall_emulation.h \
			  ipc_cache.h    kern_obj.h        syscall_subr.h \
			  ipc_copyin.h   kern_port.h       syscall_sw.h \
			  ipc_copyout.h  kern_set.h        task.h \
			  ipc_globals.h  lock.h            thread.h \
			  ipc_hash.h     mach_param.h      thread_modes.h \
			  ipc_host.h     mach_redefines.h  thread_swap.h \
			  ipc_kmesg.h    macro_help.h      threadcall.h \
			  ipc_mports.h   mfs.h             timer.h \
			  ipc_netport.h  mig_user_hack.h   type_conversion.h \
			  ipc_pobj.h     msg_queue.h       xpr.h \
			  ipc_prims.h    parallel.h        zalloc.h \
			  ipc_ptraps.h   port_object.h     task_swap.h \
			  ipc_signal.h   processor.h 	   kd_thread_support.h

ILIST   = ${DATAFILES}
IDIR    = /usr/sys/include/kern/
IMODE   = 444
IGROUP  = bin
IOWNER  = bin


include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk

