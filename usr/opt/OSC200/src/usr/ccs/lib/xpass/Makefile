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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/03/15 20:56:30 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

VPATH			= ../../bin/common:../../bin/cxref
SYMLINKS		= xpass

PROGRAMS		= xpass
MSGHDRS			= ctools_msg.h cxref_msg.h
CATFILES		= ctools.cat cxref.cat

ILIST			= xpass
IDIR			= /usr/ccs/lib/

SYMLINKDIR1		= ../ccs/lib
SYMLINKDIR2		= /usr/bin

INCFLAGS		= -I../../bin/common
CFLAGS	=	\
	-DNEWCALL	\
	-DHOSTIEEE	\
	-DSDBDEBUG	\
	-DTWOPASS	\
	-DSINGLE_PRECISION	\
	-D_COMPILER	\
	-DCXREF

xpass_OFILES	= xpass.o cgram.o code.o comm1.o local.o messages.o	\
		  optim.o pftn.o scan.o tftn.o trees.o xdefs.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

xpass.o:	cxref_msg.h

$(xpass_OFILES):	ctools_msg.h
