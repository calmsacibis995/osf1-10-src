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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.3.11.3 $ (DEC) $Date: 1993/09/22 18:28:21 $

# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

SUBDIRS			= ${target_machine}
OTHERS			= machine
VPATH			= ${MAKETOP}/kernel/mach:${MAKETOP}/kernel/mach/${target_machine}
#DATAFILES		= boolean.h vm_statistics.h kern_return.h vm_prot.h \
#			  vm_inherit.h memory_object.h port.h policy.h \
#			  message.h time_value.h mach_param.h netport.h

DATAFILES		= boolean.h   memory_object.h   task_special_ports.h \
			  device_types.h    message.h      thread_info.h \
			  error.h     mig_errors.h   thread_special_ports.h \
			  events_info.h \
			  exception.h    msg_type.h     thread_status.h \
			  host_info.h    netport.h      thread_switch.h \
			  kern_return.h     notify.h    time_stamp.h \
			  kernel_event.h    policy.h    time_value.h \
			  port.h      vm_attributes.h \
			  mach_param.h   processor_info.h  vm_inherit.h \
			  mach_traps.h   std_types.h    vm_param.h \
			  mach_types.h   syscall_sw.h   vm_prot.h \
			  machine.h      task_info.h    vm_statistics.h \
			  mach.defs mach_types.defs std_types.defs exc.defs \
			  mach_host.defs monitor.defs

ILIST   = ${DATAFILES}
IDIR    = /usr/sys/include/mach/
IMODE   = 444
IGROUP  = bin
IOWNER  = bin
SYMLINKS		= ${target_machine}
SYMLINKDIR1 		= .
SYMLINKDIR2		= /usr/sys/include/mach
${target_machine}_SYMLINK 	= machine

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
include ${MAKEFILEPATH}/others.mk
include ${MAKEFILEPATH}/datafiles.mk

machine: ${ALWAYS}
	${RM} ${_RMFLAGS_} machine
	ln -s ${target_machine} machine
