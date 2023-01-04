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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:42:51 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

VPATH			= ../eqn

PROGRAMS		= neqn

ILIST			= neqn
IDIR			= /usr/bin/

CFLAGS			= -DNEQN
YFLAGS			= -d
INCFLAGS		= -I../eqn
OFILES			= e.o diacrit.o eqnbox.o font.o fromto.o funny.o \
			  glob.o integral.o io.o lex.o lookup.o mark.o \
			  matrix.o move.o over.o paren.o pile.o shift.o \
			  size.o sqrt.o text.o
HFILES			= e.h e.def
GARBAGE			= e.c e.def y.tab.c y.tab.h

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

e.def: e.c
	-if [ -f y.tab.h ]; then ${MV} -f y.tab.h e.def; fi
