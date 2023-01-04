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
# @(#)$RCSfile: Makefile,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1992/04/15 15:09:47 $
# 
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

# sec_export.c comes from seccmd
VPATH			= ${MAKETOP}/seccmd

SEC_PROGRAMS		= cpio
PROGRAMS		= cpio
MSGHDRS			= cpio_msg.h
CATFILES		= cpio.cat

SEC_ILIST		= cpio
ILIST			= cpio
IDIR			= /usr/bin/
SEC_IMODE		= 2${IMODE}
SEC_IGROUP		= sec

# sec_export.o_INCFLAGS	= -I${MAKETOP}/seccmd	# for sec_export.h
# CFLAGS		= ${SEC_DEFS}
# OFILES		= cpio.o ${SEC_DEFS?sec_export.o:}
# LIBS			= ${SEC_LIBS}

OFILES			= cpio.o
HFILES			= ${MSGHDRS}

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

# sec_export.o:	sec_export.h









