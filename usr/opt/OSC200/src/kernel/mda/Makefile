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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:36:34 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

#
# Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
#
# Property of Encore Computer Corporation.
# This software is made available solely pursuant to the terms of
# a software license agreement which governs its use. Unauthorized
# duplication, distribution or sale are strictly prohibited.
#

#SUBDIRS			= help
PROGRAMS		= mda

ILIST			= mda
IDIR			= /usr/sbin/

INCFLAGS		= -I../../usr/include -I../../kernel/XPC.ser -I..
IDENT			= -DMAXDSIZ="33554432" -DINET -DBSD44=1
CFLAGS			= -DKERNEL -D_KERNEL $(IDENT)
LDFLAGS			= -u _sbrk
OFILES			= mda.o commands.o map.o symtab.o display.o \
			  show.o symbols.o trace.o utils.o display_lock.o \
			  display_lists.o print.o history.o display_network.o \
			  display_buffer.o display_io_stuff.o display2.o \
			  display_xpr.o vers.o
LIBS			= -lmach

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

# For now, rely on a vers.o being built for the kernel.
#
vers.o:
	cp ../DEFAULT/vers.o .
