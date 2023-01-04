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
# @(#)$RCSfile: Makefile,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1992/03/19 10:39:15 $
# 
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

# sec_export.c comes from seccmd
VPATH			= ${MAKETOP}/usr/bin/tar:${MAKETOP}/seccmd

SEC_PROGRAMS		= tar
PROGRAMS		= tar
MSGHDRS			= tar_msg.h
CATFILES		= tar.cat

SEC_ILIST		= tar
ILIST			= tar
IDIR			= /sbin/
SEC_IMODE		= 2${IMODE}
SEC_IGROUP		= sec

USE_STATIC_LIBRARIES	=

INCFLAGS		= -I${MAKETOP}/usr/bin/tar -I${MAKETOP}/seccmd
#CFLAGS			= ${SEC_DEFS}
HFILES			= ${MSGHDRS}
#OFILES			= tar.o ${SEC_DEFS?sec_export.o:}
OFILES			= tar.o
LIBS			= ${SEC_LIBS}
LDFLAGS			= ${LDSTRIP}
GLINE			=

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
