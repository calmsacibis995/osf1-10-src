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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:45:46 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
# OSF/1 Release 1.0

VPATH			= ${TARGET_MACHINE}

LIBVMUNIX_SO		= ${MAKETOP}../../export/${target_machine}/usr/shlib/libvmunix.so
VMUNIX_PICNAMES		= ${MAKETOP}vmunix.pic-names

PROGRAMS		= tohash tchash testdq testsq1 testsq2 tnmmap \
			  tload2 tralloc ktload2 kloadcall vm_region texport \
			  terrno

USE_STATIC_LIBRARIES	=

# Do not put ${KTLOAD2PROGS} into ${OTHERS} unless the kernel
#   object file is in place
TLOAD2PROGS		= 
KTLOAD2PROGS		= kmod1

tohash_OFILES		= tohash.o ../loader/ldr_errno.o ../utils/libutils.a

tchash_OFILES		= tchash.o ../loader/ldr_errno.o ../utils/libutils.a

testdq_OFILES		= testdq.o ../utils/libutils.a

testsq1_OFILES		= testsq1.o ../utils/libutils.a

testsq2_OFILES		= testsq2.o ../utils/libutils.a

tnmmap_OFILES		= tnmmap.o ../loader/ldr_errno.o ../utils/libutils.a

LOADER_COMMON_OFILES	= ../loader/ldr_bootstrap.o \
			  ../loader/ldr_global_file.o \
			  ../loader/ldr_main.o \
			  ../loader/ldr_known_pkg.o \
			  ../loader/ldr_context.o \
			  ../loader/ldr_symres.o \
			  ../loader/region_alloc.o \
			  ../loader/ldr_preload.o \
			  ../loader/ldr_dyn_mgr.o \
			  ../loader/ldr_errno.o \
			  ../loader/ldr_lock.o \
			  ../loader/ldr_brk.o \
			  ../utils/libutils.a

tload2_COMMON_OFILES	= tload2.o

tralloc_OFILES		= tralloc.o \
			  ../loader/region_alloc.o \
			  ../loader/ldr_errno.o \
			  ../utils/libutils.a

texport_COMMON_OFILES	= texport.o ../build/switch.o

terrno_OFILES		= terrno.o ../loader/ldr_errno.o ../utils/libutils.a

ktload2.o_CFLAGS	= -DKTLOAD2
ktload2_OFILES		= ktload2.o

kloadcall_LIBS		= -lmach

vm_region_LIBS		= -lmach

INCFLAGS		= -I../include -I../ldr_include

CFLAGS			= ${${TARGET_MACHINE}CFLAGS}

IMODE			= 664

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/others.mk
include ${TARGET_MACHINE}/machdep.mk

${TLOAD2PROGS}:	tload2 $$@.o

${KTLOAD2PROGS}: ${LIBVMUNIX_SO} ${VMUNIX_PICNAMES} $$@.o

ktload2.c: tload2.c
	${RM} -f ktload2.c
	${CP} tload2.c ktload2.c

${TLOAD2PROGS}:	$$@.o

OTHERS = tload2.so

tload2.so: tload2
	${CP} tload2 tload2.so

