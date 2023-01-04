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
#	@(#)$RCSfile: libs.mk,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1992/02/28 19:05:54 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

#
# Definitions for build
#
_LIBRARIES_=${LIBRARIES?${LIBRARIES}:_LIBRARIES_}
_SHARED_LIBRARIES_=${SHARED_LIBRARIES?${SHARED_LIBRARIES}:_SHARED_LIBRARIES_}
_SET_ANY_LIBRARIES_=${LIBRARIES?_ANY_LIBRARIES_:${SHARED_LIBRARIES?_ANY_LIBRARIES_:}}
${_SET_ANY_LIBRARIES_}=

_LIB_OFILES_=${LIBRARIES?${_OFILES_}:}

#
#  Definitions for default .c.o rules used by programs or libraries
#
_LIBRARY_C_O_=${_ANY_LIBRARIES_?.c.o:_LIBRARY_C_O_}

#
#  Default double suffix compilation rules
#
${_LIBRARY_C_O_}:
	${_CC_} -c ${_CCFLAGS_} $*.c
#	${LD} ${_LDFLAGS_} -x -r $*.o
#	${MV} -f a.out $*.o

#
#  Build rules
#
${_LIBRARIES_}: ${DEPEND_LIST} ${_LIBRARIES_}($${_LIB_OFILES_})
	${_AR_} ${DEF_ARFLAGS} $@ $?
	${_RANLIB_} $@
	${RM} -f $?

${_SHARED_LIBRARIES_}: $${_OFILES_}
	${_LD_} ${_SHLDFLAGS_} -o $@.X ${_OFILES_} ${_LIBS_}
	${MV} $@.X $@
