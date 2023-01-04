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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.8.2 $ (DEC) $Date: 1993/10/14 18:33:57 $ 

PROGRAMS		= siainit

MSGHDRS                 = libc_msg.h

ILIST			= siainit
IDIR			= /usr/sbin/
IOWNER			= root
IMODE			= 500
HLIBC			= ${MAKETOP}usr/ccs/lib/libc
HLIBCSIA		= ${HLIBC}/SIA
MSGHDRS                 = libc_msg.h
CFLAGS			= ${SEC_DEFS} -DSV_SHLIBS
LINTFLAGS		= ${CFLAGS}
INCFLAGS		= -I${HLIBC} -I${HLIBCSIA}
OFILES			= siainit.o 
LIBS			= 

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
