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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/01/08 15:31:16 $
#
#
# OSF/1 Release 1.0

VPATH			= ${MAKETOP}/usr/bin/cpio

SEC_PROGRAMS		= cpio
PROGRAMS		= cpio
MSGHDRS			= cpio_msg.h
CATFILES		= cpio.cat

SEC_ILIST		= cpio
ILIST			= cpio
IDIR			= /sbin/
SEC_IMODE		= 2${IMODE}
SEC_IGROUP		= sec

USE_STATIC_LIBRARIES	=

INCFLAGS		= -I${MAKETOP}/usr/bin/cpio
OFILES			= cpio.o
HFILES			= ${MSGHDRS}

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

