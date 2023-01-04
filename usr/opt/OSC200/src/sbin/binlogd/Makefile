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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/19 08:58:37 $ 
# 


VPATH			= ${MAKETOP}/usr/sbin/binlogd

SEC_PROGRAMS		= binlogd
PROGRAMS		= binlogd

SEC_ILIST		= binlogd
ILIST			= binlogd
IDIR			= /sbin/

USE_STATIC_LIBRARIES	=

INCFLAGS		= -I${MAKETOP}/usr/sbin/binlogd -I${MAKETOP}kernel
CFLAGS			= -D_BSD ${SEC_DEFS}
LIBS			= ${SEC_LIBS}
LDFLAGS			= ${LDSTRIP}
GLINE			=

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
