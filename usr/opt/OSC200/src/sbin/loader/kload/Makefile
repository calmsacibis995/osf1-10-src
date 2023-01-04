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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/03/19 09:15:45 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
# OSF/1 Release 1.0

VPATH			= ${TARGET_MACHINE}

SUBDIRS			= include
PROGRAMS		= kloadsrv

ILIST			= kloadsrv
IDIR			= /sbin/

USE_STATIC_LIBRARIES	=

LOADER_COMMON_OFILES	= ../loader/ldr_bootstrap.o \
			  ../loader/ldr_global_file.o \
			  ../loader/ldr_known_pkg.o \
			  ../loader/ldr_main.o \
			  ../loader/ldr_context.o \
			  ../loader/ldr_preload.o \
			  ../loader/ldr_symres.o \
			  ../loader/region_alloc.o \
			  ../loader/ldr_dyn_mgr.o \
			  ../loader/ldr_errno.o \
			  ../loader/ldr_lock.o \
			  ../loader/ldr_brk.o \
			  kls_switch.o \
			  ../utils/libutils.a

KERNEL_LOADER_OFILES	= kernel_region_alloc.o \
			  kls_vm.o \
			  ldr_kernel_bootstrap.o \
			  ldr_kernel_main.o \
			  ldr_kernel_syscalls.o \
			  kls_subr.o

IPC_OFILES		= kls_message.o kls_ipc.o

INCFLAGS		= -I../include -I../ldr_include -Iinclude -I.

CFLAGS			= ${${TARGET_MACHINE}CFLAGS}

include  ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
include  ${MAKEFILEPATH}/programs.mk
include  ${MAKEFILEPATH}/objects.mk
include ${TARGET_MACHINE}/machdep.mk

# We can't use this, because include files
# in . won't be found for the compile.
#
#kls_ipc.c: ${IPC_CFILE}
#	${RM} -f kls_ipc.c
#	${CP} ${IPC_CFILE} kls_ipc.c


# This is messy, but there doesn't seem to be a
# clean way in make to specify the basename of a
# file with a different extension.

kls_ipc.o: $${IPC_CFILE:.c=.o}
	${MV} `echo ${IPC_CFILE} | sed -e 's:.*/::' -e 's:c$$:o:'` $@
