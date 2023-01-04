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
#	"@(#)$RCSfile: Makefile,v $ $Revision: 4.2.12.3 $ (DEC) $Date: 1993/10/05 18:14:49 $"
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
# OSF/1 Release 1.0
#
#	000	03-jun-1991	ccb
#	001	12-jul-1991	ccb
#		strip the results


AWKDIR			= usr/bin/awk

VPATH			= ${MAKETOP}/${AWKDIR}
PROGRAMS		= awk
MSGHDRS			= awk_msg.h
CATFILES		= awk.cat

ILIST			= awk
IDIR			= /isl/

USE_STATIC_LIBRARIES	=

CCTYPE			= traditional
#CFLAGS			= -D_NO_PROTO -Dvolatile=__volatile__
CFLAGS			= -DYYSTYPE=long
INCFLAGS		= -I${MAKETOP}/${AWKDIR}
YFLAGS			= -d
HFILES			= ${MSGHDRS}
FILES			= awk.lx.o b.o main.o token.o tran.o lib.o\
			  run.o parse.o proctab.o freeze.o
OFILES			= awk.g.o ${FILES}
LIBS			= -lm
LDFLAGS			= ${LDSTRIP}
GARBAGE			= awk.h token.c proc proc.o proctab.c\
			  y.tab.c y.tab.h yacc* temp

proc.o_CCTYPE		= host
proc.o_INCFLAGS		= -I.
#proc.o_CFLAGS		= -DPROC
proctoken.o_CCTYPE	= host
proctoken.o_INCFLAGS	= -I.
#proctoken.o_CFLAGS	= -DPROC
proc_CCTYPE		= host
proc_LIBS		=
proc_LDFLAGS		= # -non_shared

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

awk.g.o: awk.def

proc.o:	awk.h

y.tab.h: awk.g.o

awk.h: awk.g.o
	if [ -f y.tab.h ]; then ${MV} -f y.tab.h awk.h; else true; fi

${FILES}: awk.h awk.def

token.c: awk.h token.c.ucb
	sed -n -e '1,/= *{'\$$'/p' token.c.ucb > ./token.pre
	sed -e 's/# *define *//' \
	    -e 's/^/'\"'/' \
	    -e 's/ /'\"', /' \
	    -e 's/'\$$'/,/' awk.h > ./token.mid
	sed -n -e '/^};'\$$'/,'\$$'p' token.c.ucb > ./token.end
	cat ./token.pre ./token.mid ./token.end > $@.X
	${MV} $@.X $@

proctab.c: proc
	${RM} -f $@
	./proc > $@

proctoken.c: token.c
	${RM} -f $@
	${CP} token.c $@

proctoken.o: proctoken.c
	${_CC_} ${_CFLAGS_} -c proctoken.c

proc.o: awk.h
	${_CC_} ${_CFLAGS_} -I. -c proc.c

proc.o: awk.h

proc:	proc.o proctoken.o
	${_CC_} -o $@.X proc.o proctoken.o
	${MV} $@.X $@
