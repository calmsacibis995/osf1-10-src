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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/03/15 20:49:08 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

VPATH			= ${target_machine}
PROGRAMS		= vmstat 
MSGHDRS			= vmstat_msg.h
CATFILES		= vmstat.cat

ILIST			= vmstat
IDIR			= /usr/bin/
IOWNER			= ${ROOT_OWNER}
IMODE			= ${SETUID_MODE}
CFLAGS			=  ${${TARGET_MACHINE}CFLAGS}
INCFLAGS		= -I${target_machine}

USE_STATIC_LIBRARIES	=

HFILES			= ${MSGHDRS}
OFILES 			= vmstat.o ${${TARGET_MACHINE}COBJS}
LIBS            	= -lmach

include ${target_machine}/machdep.mk
include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

tags:
	ctags *.c ${target_machine}/*.c
