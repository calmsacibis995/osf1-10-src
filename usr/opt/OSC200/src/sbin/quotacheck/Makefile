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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:50:19 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

# preen.c comes to us from ufs_fsck
#
VPATH		= ${MAKETOP}/usr/sbin/quotacheck:${MAKETOP}/usr/sbin/ufs_fsck


SEC_PROGRAMS		= quotacheck
PROGRAMS		= quotacheck
MSGHDRS			= quotacheck_msg.h
CATFILES		= quotacheck.cat

SEC_ILIST		= quotacheck
ILIST			= quotacheck
IDIR			= /sbin/

USE_STATIC_LIBRARIES	=

CFLAGS			= ${SEC_DEFS}
HFILES			= ${MSGHDRS}
OFILES			= quotacheck.o preen.o
LIBS			= ${SEC_LIBS}
LDFLAGS			= ${LDSTRIP}
GLINE			=

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
