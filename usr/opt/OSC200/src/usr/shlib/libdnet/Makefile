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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1992/12/04 17:26:15 $
#
#
# Makefile for shareable DECnet stub library -- libdnet_stub.so
#
#	Don Haney       22 October, 1992
#
VPATH			= ${MAKETOP}usr/ccs/lib/libdnet

OTHERS			= libdnet_stub.so

USE_SHARED_LIBRARIES	=

EXPSHLIB_TARGETS	= export_libdnet_stub.so
LDFLAGS			=
LIBS			= -hidden -lc


ILIST			= libdnet_stub.so
IDIR			= /usr/shlib/
IMODE			= 644
NOSTRIP			=

CFLAGS                  = -std1

DNETOBJS                = dnet_stub.o

OFILES			= ${DNETOBJS}

include ${MAKEFILEPATH}/standard.mk
# include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/others.mk

${DNETOBJS}: $${@:.o=.c}
	${_CC_} ${_CCFLAGS_} -c $*.c
 
.c.o:
	${_CC_} -c ${_CCFLAGS_} $*.c
 
libdnet_stub.so: ${_OFILES_}
	${_LD_} ${_SHLDFLAGS_} -o $@.X ${_OFILES_} ${_LIBS_}
	${MV} $@.X $@
