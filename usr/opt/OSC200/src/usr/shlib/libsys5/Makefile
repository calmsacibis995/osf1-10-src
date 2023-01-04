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
# HISTORY 
#
#
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.15.5 $ (DEC) $Date: 1993/08/24 19:49:56 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

# If you need to modify the VPATH definition, modify the one in unenc.mk
# so that the change will be picked up for the encumbered tree as well.

S5PATH			= /usr/opt/s5
LIBSYS5			= ${MAKETOP}usr/ccs/lib/libsys5

OTHERS			= libsys5.so

USE_SHARED_LIBRARIES	=
LDFLAGS			= -init __sys5init_
LIBS			= -lc 

EXPSHLIB_TARGETS	= export_libsys5.so

ILIST			= libsys5.so
IDIR			= ${S5PATH}/usr/shlib/
IMODE			= 644
NOSTRIP			=

USE_STATIC_LIBRARIES	=

INCFLAGS		= -I${TARGET_MACHINE} -I../../ccs/lib/libc/${target_machine}
CFLAGS			= ${${TARGET_MACHINE}_UNENC_CFLAGS} -DH_S5
AFLAGS			= ${${TARGET_MACHINE}_UNENC_ASFLAGS} -DH_S5

# Do not edit OFILES in this makefile.  If you need to make changes to 
# OFILES, edit unenc.mk instead.  unenc.mk is included in both the 
# encumbered and unencumbered libsys5 Makefiles.  This convention
# allows the unencumbered changes to be picked up in the encumbered
# version of libsys5.
OFILES			= ${UNENC_OFILES}

-include ${target_machine}/machdep.mk

include ${MAKETOP}/usr/ccs/lib/libsys5/unenc.mk
include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk

${STDOBJS}: $${@:.o=.c}
	${_CC_} ${_CCFLAGS_} -c $*.c
#	${LD} -x -r $*.o
#	${MV} -f a.out $*.o

.c.o:
	${_CC_} -c ${_CCFLAGS_} $*.c
#	${LD} ${_LDFLAGS_} -x -r $*.o
#	${MV} -f a.out $*.o

libsys5.so: ${_OFILES_}
	${_LD_} ${_SHLDFLAGS_} -o $@.X ${_OFILES_} ${_LIBS_}
	${MV} $@.X $@
