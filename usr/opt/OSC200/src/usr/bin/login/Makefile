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
# @(#)$RCSfile: Makefile,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1992/05/04 13:31:57 $
# 
# OSF/1 Release 1.0

SEC_PROGRAMS		= login
PROGRAMS		= login
MSGHDRS			= login_msg.h
CATFILES		= login.cat

SEC_ILIST		= login
ILIST			= login
IDIR			= /usr/bin/
IOWNER			= root
IMODE			= 4711

SYMLINKS                = ${SEC_LEVEL?login:}
SYMLINKDIR1             = ../../usr/bin
SYMLINKDIR2             = /tcb/lib

LINTFLAGS		= ${CFLAGS}
HFILES			= ${MSGHDRS} pathnames.h
OFILES			= login.o
LIBS			= -lutil

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk










