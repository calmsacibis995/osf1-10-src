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
# @(#)$RCSfile$ $Revision$ (DEC) $Date$
#
#
# HISTORY
# Removed '-update_registry ../so_locations' from LDFLAGS.
#

LIBCURSES		= ${MAKETOP}usr/ccs/lib/libcurses
VPATH			= ${LIBCURSES}:${LIBCURSES}/bsd
OTHERS			= libcurses.so
MSGHDRS			= libcurses_msg.h
CATFILES		= libcurses.cat

USE_SHARED_LIBRARIES	=
EXPORTS			=
SHLDFLAGS		= ${SHLDSTRIP}
EXPSHLIB_TARGETS	= export_libcurses.so
GARBAGE			= capnames.c termcap.c
LDFLAGS			=
EXPLIB_TARGETS		= export_libcurses.so

ILIST			= libcurses.so
IDIR			= /usr/shlib/
EXPDIR			= /usr/shlib/
IMODE			= 644

CFLAGS			= -DVIDEO -DKEYPAD -UNONSTANDARD -DWCHAR
INCFLAGS		= -I${LIBCURSES} -I${LIBCURSES}/bsd

include ${LIBCURSES}/libcurses-objs.mk
include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/others.mk
include ${MAKEFILEPATH}/objects.mk

#
# Order objects in the library with the smallest data sections first.
# This has a positive effect on the memory utilization, by keeping
# routines with large buffers out of the way of more frequently
# used routines.  Obviously, this is a rather unsophisticated
# heuristic, and should someday be replaced with feedback information
# from real running systems.
#
AWK			= awk
SIZE			= size
ORDERED_OFILES		= `${SIZE} ${_OFILES_} | ${AWK} \
			  '/^[0-9]/ { printf("%8d:%s\n", $$2+$$3,$$6);}'| \
			  ${SORT} | ${SED} -e 's/^.*://'`

showstring.o setupterm.o: libcurses_msg.h

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

.c.o:
	${_CC_} ${_CCFLAGS_} -c $*.c

libcurses.so: ${_OFILES_}
	${_LD_} ${_SHLDFLAGS_} -o $@.X ${ORDERED_OFILES} ${_LIBS_} -lc
	${MV} $@.X $@
