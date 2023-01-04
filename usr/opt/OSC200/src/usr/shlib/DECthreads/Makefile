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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.11.3 $ (DEC) $Date: 1993/06/03 21:56:07 $
#
#
# Library to build
#
LIBPTHREADS	= ${MAKETOP}usr/ccs/lib/DECthreads
VPATH           = ${LIBPTHREADS}/${target_machine}:${LIBPTHREADS}/COMMON:
OTHERS		= libpthreads.so
MSGHDRS		= dcethd_msg.h
CATFILES	= dcethd.cat
EXPSHLIB_TARGETS	= export_libpthreads.so
CFLAGS		= -D_CMA_NOWRAPPERS_ -Dultrix  
INCFLAGS	=  -ICOMMON -I${target_machine} -I${LIBPTHREADS}/${target_machine} -I${LIBPTHREADS}/COMMON
LDFLAGS		= -warning_unresolved

#
# For a debug version of DECthreads, start the build with the command:
#
#	build CC_OPT_LEVEL='-g'
#
# (that will override the following line)
#
CC_OPT_LEVEL	= -O -DNDEBUG

ILIST =			libpthreads.so
IMODE =			644
IDIR =			/usr/shlib/

#
# Objects that go into libpthreads.a
#

cma_tis_sup.o_CFLAGS=-D_CMA_SHLIB_

PMAX_OFILES	= cma_vmprot.o cma_vp_hdwr.o
alpha_OFILES	= cma_vmprot.o cma_vp_hdwr.o cma_host.o

HFILES	= ${MSGHDRS}

OFILES =	cma_alert.o		cma_assert.o \
	cma_attr.o		\
	cma_condition.o		cma_context.o \
	cma_debugger.o		cma_deb_core.o		cma_deb_event.o	\
	cma_defer.o		cma_dispatch.o		\
	cma_errno.o		cma_errors.o     	\
	exc_handling.o		cma_handle.o		\
	cma_init.o		cma_kernel.o		cma_message.o \
	cma_malloc.o		cma_mutex.o		cma_once.o \
	cma_pthread.o		cma_ptd_exc.o \
	cma_sched.o	\
	cma_signal.o		cma_stack.o \
	cma_tcb.o		cma_thread.o		cma_timer.o \
	cma_util.o		cma_ux.o		cma_vm.o \
	cma_vp.o		cma_assem.o		cma_tis_sup.o \
	${${target_machine}_OFILES}

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/datafiles.mk

${STDOBJS}: $${@:.o=.c}
	${_CC_} ${_CCFLAGS_} -c $*.c
 
.c.o:
	${_CC_} -c ${_CCFLAGS_} $*.c
 
libpthreads.so: ${_OFILES_}
	${_LD_} ${_SHLDFLAGS_} -o $@.X ${_OFILES_} ${_LIBS_}
	${MV} $@.X $@



