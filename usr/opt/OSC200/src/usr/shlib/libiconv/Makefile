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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/08 01:18:22 $
#
LIBICONV		= ${MAKETOP}usr/ccs/lib/libiconv
VPATH			= ${LIBICONV}

USE_SHARED_LIBRARIES	=
OTHERS			= libiconv.so
EXPSHLIB_TARGETS	= export_libiconv.so

ILIST			= libiconv.so
IDIR			= /usr/shlib/
EXPDIR			= /usr/shlib/
IMODE			= 644

USE_SHARED_LIBRARIES	=
SHLDFLAGS		= ${SHLDSTRIP}
LIBS			=
EXPORTS			=
LDFLAGS			=

GLINE			=
INCFLAGS		= -I${LIBICONV} 

OFILES			= iconv.o ascii.o fcs.o
HFILES			= iconvTable.h iconv932.h iconvP.h iconv_local.h

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/others.mk
include ${MAKEFILEPATH}/objects.mk

fcs.o: fcs.h

.c.o:
	 ${_CC_} ${_CCFLAGS_} -c $*.c

libiconv.so: ${_OFILES_}
	${_LD_} ${_SHLDFLAGS_} -o $@.X ${_OFILES_} ${_LIBS_} -lc
	${MV} $@.X $@
