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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/09/07 21:25:20 $
#


S5PATH	= /usr/opt/s5
LIBS5PATH	= ${MAKETOP}usr/ccs/lib/libsys5
LIBS5RPATH	= ${MAKETOP}usr/ccs/lib/libsys5_r
VPATH		= ${LIBS5RPATH}:${LIBS5PATH}:${LIBS5PATH}/${target_machine}

USE_STATIC_LIBRARIES	=
LIBRARIES	= libsys5_r.a

EXPLIB_TARGETS	= export_libsys5_r.a

ILIST		= libsys5_r.a
IDIR		= ${S5PATH}/usr/lib/
IMODE		= 644

HFILES          = 
INCFLAGS	= -I${target_machine} -I../libc -I../libc/SIA
CFLAGS		= -DCOMPAT_43  -DLIBC_SCCS -D_THREAD_SAFE -D_REENTRANT \
		  ${${TARGET_MACHINE}CFLAGS} 


OFILES		= ${STDOBJS} 

STDOBJS	=	mknod.o mount.o umount.o ttyname.o mktemp.o \
	getcwd.o pipe.o mkfifo.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk

.c.o:
	${_CC_} -c ${_CCFLAGS_} $*.c
#	${LD} ${_LDFLAGS_} -x -r $*.o
#	${MV} -f a.out $*.o
