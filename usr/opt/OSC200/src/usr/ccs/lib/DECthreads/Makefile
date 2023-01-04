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
# @(#)$RCSfile: Makefile,v $ $Revision: 4.2.13.3 $ (DEC) $Date: 1993/06/03 21:56:01 $
#
#  @OSF_COPYRIGHT@
# 
#
# Library to build
#
VPATH           = ${target_machine}:COMMON
LIBRARIES	= libpthreads.a
MSGHDRS		= dcethd_msg.h
CATFILES	= dcethd.cat
EXPLIB_TARGETS	= export_libpthreads.a
CFLAGS		= -D_CMA_NOWRAPPERS_ -Dultrix 
INCFLAGS	= -I${target_machine} -ICOMMON
#
# For a debug version of DECthreads, start the build with the command:
#
#	build OPT_LEVEL='-g'
#
# (that will override the following line)
#
OPT_LEVEL	= -O -DNDEBUG

# DATAFILES =	cma.h		cma_config.h		cma_ux.h \
#		cma_px.h        pthread.h		pthread_exc.h \
#	 	cma_sigwait.h	exc_handling.h

#
# DECthreads include files are exported by the makefile for /usr/include etc.
#
#INCLUDES =		${DATAFILES}

EXPDIR =		/usr/ccs/lib/

ILIST =			libpthreads.a
IMODE =			644
IDIR =			/usr/ccs/lib/

#
# Objects that go into libpthreads.a
#

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
	cma_sched.o		cma_signal.o		cma_stack.o \
	cma_tcb.o		cma_thread.o		cma_timer.o \
	cma_util.o		cma_ux.o		cma_vm.o \
	cma_vp.o		cma_assem.o		cma_tis_sup.o \
	${${target_machine}_OFILES}

include ${MAKEFILEPATH}/standard.mk
#-include ${target_machine}/machdep.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/datafiles.mk



