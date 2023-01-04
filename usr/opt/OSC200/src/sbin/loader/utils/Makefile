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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1992/12/07 16:19:12 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

LIBRARIES		= libutils.a

EXPLOADER_TARGETS	= ${LIBRARIES}

USE_STATIC_LIBRARIES	=

INCFLAGS		= -I../include -I../ldr_include
open_hash.o_INCFLAGS	= -I${MAKETOP}usr/include/loader ${INCFLAGS}
hash_string.o_INCFLAGS	= -I${MAKETOP}usr/include/loader ${INCFLAGS}
oh_nelem.o_INCFLAGS	= -I${MAKETOP}usr/include/loader ${INCFLAGS}
CFLAGS			= ${${TARGET_MACHINE}CFLAGS}
OFILES			= hash_string.o ldr_alloc.o open_hash.o oh_nelem.o \
			  chain_hash.o squeue.o dqueue.o ldr_sys_int.o \
			  ldr_windows.o ldr_auxv.o

# DO_NOT_USE_PROC_PTRS needs to be defined as long as lib_admin is built
# statically
chain_hash.o_CFLAGS	= -DDO_NOT_USE_PROC_PTRS
open_hash.o_CFLAGS	= -DDO_NOT_USE_PROC_PTRS

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk
-include ${TARGET_MACHINE}/machdep.mk
