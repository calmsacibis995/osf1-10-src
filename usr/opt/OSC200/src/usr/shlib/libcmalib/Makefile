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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/03/16 21:59:44 $
#
# 
#
# Library to build
#
LIBPTHREADS	= ${MAKETOP}usr/ccs/lib/DECthreads
LIBCMALIB	= ${MAKETOP}usr/ccs/lib/libcmalib
VPATH           = ${LIBCMALIB}/${TARGET_MACHINE}:${LIBCMALIB}
OTHERS		= libcmalib.so
EXPSHLIB_TARGETS	= export_libcmalib.so
CFLAGS		= -D_CMA_NOWRAPPERS_ -Dultrix `genpath -I${TARGET_MACHINE}`
AFLAGS		= `genpath -I${TARGET_MACHINE}` ${AFLAGS}
INCFLAGS	=  -I${LIBCMALIB} -I${LIBCMALIB}/${TARGET_MACHINE} -I${LIBPTHREADS}/COMMON -I${LIBPTHREADS}/${TARGET_MACHINE}
LIBS		= -hidden -lpthreads -lc_r -lmach -lc

#
# For a debug version of DECthreads, start the build with the command:
#
#	build CC_OPT_LEVEL='-g'
#
# (that will override the following line)
#
CC_OPT_LEVEL	= -O -DNDEBUG

ILIST =			libcmalib.so
IMODE =			644
IDIR =			/usr/shlib/

#
# Objects that go into libcmalib.so
#

OFILES =	cmalib_assert.o		cmalib_aq_cre.o \
	cmalib_attr.o \
	cmalib_init.o		cmalib_queue.o \
	cmalib_seq.o		cmalib_aq_op.o	${${TARGET_MACHINE}_OFILES}

include ${MAKEFILEPATH}/standard.mk
#-include ${TARGET_MACHINE}/machdep.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/datafiles.mk

${STDOBJS}: $${@:.o=.c}
	${_CC_} ${_CCFLAGS_} -c $*.c
 
.c.o:
	${_CC_} -c ${_CCFLAGS_} $*.c
 
libcmalib.so: ${_OFILES_}
	${_LD_} ${_SHLDFLAGS_} -o $@.X ${_OFILES_} ${_LIBS_}
	${MV} $@.X $@







