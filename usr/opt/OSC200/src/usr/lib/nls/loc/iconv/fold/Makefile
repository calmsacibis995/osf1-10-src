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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1993/10/29 19:03:27 $
#
# (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
# ALL RIGHTS RESERVED 
#
#
# OSF/1 1.2
#

#
# These aren't really libraries, but loadable 'shared objects'
# loaded by libiconv.a
#

SHARED_LIBRARIES	= ISO8859-1_ct ct_ISO8859-1

OTHERS			= ${SHARED_LIBRARIES} ${SYMLINKS}

USE_SHARED_LIBRARIES	=
INCFLAGS		= -I${MAKETOP}usr/ccs/lib/libiconv
LIBS			= -liconv -lc

ILIST			= ${SHARED_LIBRARIES} ${SYMLINKS}
IDIR			= /usr/lib/nls/loc/iconv/
IMODE			= 644

STDOBJS			= ${SHARED_LIBRARIES:=.o}
OFILES			= ${STDOBJS}

# Symbolic links

ISO8859-1_fold7_SYMLINK	= ISO8859-1_ct
ISO8859-1_fold8_SYMLINK	= ISO8859-1_ct
fold7_ISO8859-1_SYMLINK = ct_ISO8859-1
fold8_ISO8859-1_SYMLINK = ct_ISO8859-1
SYMLINKS		= ISO8859-1_fold7 ISO8859-1_fold8 fold7_ISO8859-1 \
			  fold8_ISO8859-1
_SYMLINK_		= ${${@}_SYMLINK}

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/others.mk
include ${MAKEFILEPATH}/objects.mk

${SYMLINKS}:	$${_SYMLINK_}
	${RM} ${_RMFLAGS_} $@
	umask 022 ; ln -s ${_SYMLINK_} $@

${SHARED_LIBRARIES}: ${${.TARGET}:=.o} fold.h

${STDOBJS}: $${@:.o=.c}
	${_CC_} -c ${_CCFLAGS_} $*.c

${SHARED_LIBRARIES}: $${@:=.o}
	${_LD_} ${_SHLDFLAGS_} -o $@.X ${@:=.o} ${LIBS}
	${MV} $@.X $@
