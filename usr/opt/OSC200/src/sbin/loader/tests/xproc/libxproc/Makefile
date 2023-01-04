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
# @(#)$RCSfile: Makefile,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/03/15 20:47:31 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
# OSF/1 Release 1.0

#
#             Loader Cross Process Interfaces (-lxproc)
#
# This is an experimental implementation of the loader cross process
# interfaces that work on an arbitrary process, that is on a process
# other than ldr_my_process() and ldr_kernel_process().  To get a
# an appropriate ldr_process_t simply obtain a pid_t for the desired
# target process and cast that value to a ldr_process_t.
# Because changes need to be made to ldr_xcalls.o found in -lc
# to support this new functionality, -lxproc contains its own version
# of ldr_xcalls.o with the necessary changes.  Always link programs that
# link with -lxproc, with -lld and -lloader.  nlist() needs -lld and the
# cross process interfaces call the loader to do some of their work.
# If -lxproc is built with the USE_VM_CALLS set, also link programs
# that link with -lxproc, with -lmach.
#
# Configuration Options
#
#	-D_USE_VM_CALLS
#		Causes the cross process interfaces to use the Mach VM
#		calls, such as vm_read(), to read from the target process's
#		address space.  Note that a task_t for the target process
#		is obtained by task_by_unix_pid() and its access
#		restrictions are applicable.  Note also that when
#		USE_VM_CALLS is set, programs that link with libxproc.a
#		must also link with -lmach.
#
#	-D_USE_PTRACE
#		Causes the cross process interfaces to use the ptrace()
#		system call to read from the target process's address
#		space.  Note that with ptrace(), the target process must
#		be a child and must be traced (that is, it has called
#		ptrace(0)).
#

LIBS		=  -lloader -lld -lmach

INCFLAGS	= -I${MAKETOP}sbin/loader/include \
		  -I${MAKETOP}sbin/loader/include/${target_machine} \
		  -I${MAKETOP}sbin/loader/ldr_include

LIBRARIES		= libxproc.a

EXPLOADER_TARGETS	= export_libxproc.a
#EXPLIB_TARGETS		= export_libxproc.a

ILIST			= libxproc.a
IDIR			= /usr/ccs/lib/
IMODE			= 644

OFILES			= ldr_xcalls.o ldr_xproc.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk
-include ${target_machine}/machdep.mk

#CFLAGS		= -D_USE_VM_CALLS ${${target_machine}CFLAGS}
CFLAGS		= -D_CORE_PID=-2 -D_USE_PTRACE \
		  ${${target_machine}CFLAGS}
