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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:34:11 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

CCTYPE			= traditional

OTHERS		= mach_server.c \
		  memory_object_server.c memory_object_default_server.c \
		  exc_server.c mach_host_server.c device_server.c \
		  monitor_server.c

INCFLAGS	= -I.. -I../..
MIGFLAGS	= -DKERNEL
mach_server.c_MIGFLAGS = ${MIGFLAGS} -DKERNEL_SERVER
mach_host_server.c_MIGFLAGS = ${MIGFLAGS} -DKERNEL_SERVER
monitor_server.c_MIGFLAGS = ${MIGFLAGS} -DKERNEL_SERVER
memory_object_server.c_MIGFLAGS = ${MIGFLAGS} -DKERNEL_USER
memory_object_default_server.c_MIGFLAGS = ${MIGFLAGS} -DKERNEL_USER

include ${MAKEFILEPATH}/standard.mk

mach_server.c: mach.defs
	${MIG} ${_MIGFLAGS_} -server mach_server.c -header mach_interface.h mach.defs
	-egrep 'Routine ' < mach_interface.h | sed -e 's/.*Routine \([a-z_]*\).*/#define \1 \1_EXTERNAL/' > mach_user_internal.h

memory_object_server.c: memory_object.defs
	${MIG} ${_MIGFLAGS_} -header memory_object_user.h -user memory_object_user.c -server memory_object_server.c memory_object.defs

memory_object_default_server.c: memory_object_default.defs
	${MIG} ${_MIGFLAGS_} -user memory_object_default_user.c -server memory_object_default_server.c memory_object_default.defs

exc_server.c: exc.defs
	${MIG} ${_MIGFLAGS_} -user exc_user.c -server exc_server.c exc.defs

mach_host_server.c: mach_host.defs
	${MIG} ${_MIGFLAGS_} -server mach_host_server.c mach_host.defs

device_server.c: device.defs
	${MIG} ${_MIGFLAGS_} -user device_user.c -server device_server.c device.defs

monitor_server.c: monitor.defs
	${MIG} ${_MIGFLAGS_} -server monitor_server.c monitor.defs
