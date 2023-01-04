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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.3.11.5 $ (DEC) $Date: 1994/01/11 19:59:03 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

#
# Make file for lint, lint1, lint2, ntvlint1, and lint libraries
#

VPATH			= ../common

SUBDIRS			= pass2
PROGRAMS		= lint1
OTHERS			= ntvlint1
SCRIPTS			= lint
MSGHDRS			= ctools_msg.h
CATFILES		= ctools.cat

ANSI_LIST = llib-lansi.ln
LLIB_LIST =	\
	llib-lc.ln	\
	llib-port.ln	\
	llib-lm.ln	\
	llib-lcrses.ln

DATAFILES		= ${ANSI_LIST} ${LLIB_LIST} ${LLIB_SRC} Mkf.native

ILIST			= lint lint1 ${DATAFILES}
IDIR			= /usr/ccs/lib/lint/
BIN_IDIR		= /usr/ccs/bin/
LIB_IDIR		= /usr/ccs/lib/
lint1_IDIR		= /usr/ccs/lib/cmplrs/cc/
lint_IDIR		= ${BIN_IDIR}

SYMLINKS	= lint1 lint2 
SYMLINKDIR1	= ../lib/cmplrs/cc
SYMLINKDIR2	= /usr/ccs/lib

llib-lansi.ln_IDIR=${LIB_IDIR}
llib-lc.ln_IDIR=${LIB_IDIR}
llib-port.ln_IDIR=${LIB_IDIR}
llib-lm.ln_IDIR=${LIB_IDIR}
llib-lcrses.ln_IDIR=${LIB_IDIR}


CFLAGS	= \
	-DNEWCALL	\
	-DHOSTIEEE	\
	-DSDBDEBUG	\
	-DTWOPASS	\
	-DSINGLE_PRECISION	\
	-D_COMPILER	\
	-DLINT

INCFLAGS		= -I../common -I../../bin/common 
HFILES			= ${MSGHDRS}
OFILES = \
	cgram.o	\
	code.o	\
	comm1.o	\
	local.o	\
	messages.o	\
	optim.o	\
	pftn.o	\
	scan.o	\
	tftn.o	\
	trees.o	\
	xdefs.o

LLIB_SRC =	\
	llib-lansi.c	\
	llib-lc.c	\
	llib-port.c	\
	llib-lm.c	\
	llib-lcrses.c


LLIB_CFLAGS = -Dlint -DLINT -w -E -C -I- -I. ${_CCFLAGS_}

NLINT1	= ./ntvlint1

#
#	This builds the lint libraries for OSF/1.  
#
#	First, build lint pass1 natively non_shared. 
ntvlint1:	$(OFILES)
	cc -o ntvlint1 -non_shared ${OFILES} -L -L/usr/lib -L/usr/ccs/lib

$(ANSI_LIST): $$(@:.ln=.c) $(OTHERS)
	$(RM) -f $@.X
	$(CC) -std ${LLIB_CFLAGS} $(@:.ln=.c) | $(NLINT1) -wAP -MA -L$@.X
	$(MV) $@.X $@


$(LLIB_LIST): $$(@:.ln=.c) $(OTHERS)
	$(RM) -f $@.X
	$(CC) -std0 ${LLIB_CFLAGS} $(@:.ln=.c) | $(NLINT1) -Nn2500 -Nd3000 -L$@.X
	$(MV) $@.X $@

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/scripts.mk
include ${MAKEFILEPATH}/datafiles.mk


check:
	@echo "Checking bootstrap of the lint"
	$(CMP) $(TARGETS) $(CHECKDIR)


