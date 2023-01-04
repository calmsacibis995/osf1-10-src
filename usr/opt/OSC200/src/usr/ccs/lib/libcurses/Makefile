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
# (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# @(#)$RCSfile: Makefile,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 20:25:05 $
#
#
# HISTORY
#
# OSF/1 Release 1.0

VPATH			= bsd

LIBRARIES		= libcurses.a
GARBAGE			= capnames.c termcap.c
MSGHDRS			= libcurses_msg.h
CATFILES		= libcurses.cat

EXPLIB_TARGETS		= export_libcurses.a

ILIST			= libcurses.a
IDIR			= /usr/ccs/lib/
EXPDIR			= /usr/ccs/lib/
IMODE			= 644

USE_STATIC_LIBRARIES	=

CFLAGS			= -DVIDEO -DKEYPAD -UNONSTANDARD -DWCHAR
INCFLAGS		= -I. -Ibsd

include libcurses-objs.mk
include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk

showstring.o setupterm.o : libcurses_msg.h

# modified make logic for tic, ntic, capnames.c, termcap.c and term.h to
# include the update logic in this make file and automate the update of
# term.h, termcap.c and capnames.c
#
# - caps controls making both term.h and capnames.c via the ed script
#   contained in maketerm.ex
# - term.h is generated here but is not used (removed).  It is made by the
#   /usr/include Makefile.
#
capnames.c term.h: caps maketerm.ex
	$(RM) -f caps.e
	$(CP) caps caps.e
	@ed < maketerm.ex
	$(RM) -f caps.e

# - caps also controls the creation of termcap.c via the ed script
# - contained in termcap.ed and the skeleton file termcap.form
#
termcap.c: caps termcap.form termcap.ed
	$(RM) -f caps.e termcap.form.e
	$(CP) caps caps.e
	$(CP) termcap.form termcap.form.e
	@ed < termcap.ed
	$(RM) -f caps.e termcap.form.e
