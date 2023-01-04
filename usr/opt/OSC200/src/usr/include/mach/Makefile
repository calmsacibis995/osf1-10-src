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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.3.11.5 $ (DEC) $Date: 1993/09/23 17:01:26 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

VPATH			= ${MAKETOP}/kernel/mach

SUBDIRS			= ${target_machine}
EXPINC_SUBDIRS		= ${SUBDIRS}
MIG_DEFS		= exc.defs mach_host.defs monitor.defs
DATAFILES		= boolean.h device_types.h error.h exception.h \
			  host_info.h kern_return.h kernel_event.h \
			  events_info.h mach.defs mach_param.h \
			  mach_traps.h mach_types.defs mach_types.h \
			  machine.h memory_object.h message.h \
			  mig_errors.h msg_type.h netport.h notify.h \
			  policy.h port.h processor_info.h \
			  std_types.defs std_types.h syscall_sw.h \
			  task_info.h task_special_ports.h \
			  thread_info.h thread_special_ports.h \
			  thread_status.h thread_switch.h time_stamp.h \
			  time_value.h vm_attributes.h vm_inherit.h vm_param.h \
			  vm_prot.h vm_statistics.h ${MIG_DEFS}
MIG_HDRS		= ${MIG_DEFS:.defs=.h} mach_interface.h
OTHERS			= machine mach_interface.h exc.h
INCLUDES		= ${DATAFILES} ${OTHERS} ${MIG_HDRS}

ILIST			= ${OTHERS}
IDIR			= /usr/sys/include/mach/
EXPDIR			= /usr/include/mach/
IMODE			= 644

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
include ${MAKEFILEPATH}/datafiles.mk
include ${MAKEFILEPATH}/objects.mk

${MIG_DEFS:.defs=.h}: $${@:.h=.defs}

machine: ${ALWAYS}
	${RM} ${_RMFLAGS_} machine
	ln -s ${target_machine} machine

mach_interface.h: mach.defs std_types.defs mach_types.defs
	${MIG} ${_MIGFLAGS_} -D__${target_machine} -user /dev/null -server /dev/null -header $@ \
	mach.defs
