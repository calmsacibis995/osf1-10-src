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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/08/20 21:13:50 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

VPATH		= ../../common

PROGRAMS	= lint2
MSGHDRS		= lint_msg.h ctools_msg.h
CATFILES	= lint.cat ctools.cat

ILIST		= lint2
IDIR		= /usr/ccs/lib/cmplrs/cc/

HFILES		= ${MSGHDRS}
INCFLAGS	= -I../../common
CFLAGS	= 		\
	-DHOSTIEEE	\
	-DLINT		\
	-DLINTP2
OFILES =	\
	hasher.o	\
	lint2.o		\
	main.o		\
	output.o	\
	reader.o	\
	storage.o	\
	tftn.o		\
	xdefs.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
