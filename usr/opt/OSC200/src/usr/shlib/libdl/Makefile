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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1992/05/07 10:36:10 $
#

OTHERS			= libdl.so

USE_SHARED_LIBRARIES	=
LDFLAGS			=
EXPORTS			=

EXPSHLIB_TARGETS	= export_libdl.so

ILIST			= libdl.so
IDIR			= /usr/shlib/
IMODE			= 644
NOSTRIP			=

GLINE			=

RLDCFLAGS		= -D_RLD_DBX_LINK -D_LIBDL_SUPPORT_
DEBUGFLAGS		=
MACHDEPFLAGS		=
CFLAGS  		= ${RLDCFLAGS} ${MACHDEPFLAGS} ${DEBUGFLAGS}

OFILES			= ${STDOBJS}
GARBAGE			= *.S *.as tags

STDOBJS			= libdl.o

-include ${TARGET_MACHINE}/machdep.mk
include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/others.mk
include ${MAKEFILEPATH}/objects.mk

${STDOBJS}: $${@:.o=.c}
	${_CC_} ${_CCFLAGS_} -c $*.c

.c.o:
	${_CC_} -c ${_CCFLAGS_} $*.c

libdl.so: ${_OFILES_}
	@echo 'Expect to see _rld_new_interface unresolved'
	${_LD_} ${_SHLDFLAGS_} -o $@.X ${_OFILES_} ${_LIBS_}
	${MV} $@.X $@
