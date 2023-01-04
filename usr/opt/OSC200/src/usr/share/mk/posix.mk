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
# @(#)$RCSfile: posix.mk,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/08/25 13:35:41 $
#
# This file contains the default ("builtin") rules for the (XPG4/POSIX) make(1p)
# command.  It contains exactly what XPG4 X/Open CAE Specification (1992) 
# "Commands and Utilities, Issue 4 " says it must contain.  
# Do not make any changes to this file unless the standard or subsequent 
# standards require it.
#

#
# SUFFIXES AND MACROS
#

.SUFFIXES: .o .c .y .l .a .sh .f .c~ .y~ .l~ .sh~ .f~

MAKE=make
AR=ar
ARFLAGS=-rv
YACC=yacc
YFLAGS=
LEX=lex
LFLAGS=
LDFLAGS=
CC=cc
CFLAGS=-O
FC=fort77
FFLAGS=-O 1
GET=get
GFLAGS=
SCCSFLAGS=
SCCSGETFLAGS=-s

#
# SINGLE SUFFIX RULES
#

.c:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

.sh:
	cp $< $@
	chmod a+x $@

.f:
	$(FC) $(FFLAGS) $(LDFLAGS) -o $@ $<

.c~:
	$(GET) $(GFLAGS) -p $< > $*.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $*.c

.sh~:
	$(GET) $(GFLAGS) -p $< > $*.sh
	cp $*.sh $@
	chmod a+x $@

.f~:
	$(GET) $(GFLAGS) -p $< > $*.f
	$(FC) $(FFLAGS) $(LDFLAGS) -o $@ $*.f


#
# DOUBLE SUFFIX RULES
#

.c.o:
	$(CC) $(CFLAGS) -c $<

.y.o:
	$(YACC) $(YFLAGS) $<
	$(CC) $(CFLAGS) -c y.tab.c
	rm y.tab.c
	mv y.tab.o $@

.l.o:
	$(LEX) $(LFLAGS) $<
	$(CC) $(CFLAGS) -c lex.yy.c
	rm lex.yy.c
	mv lex.yy.o $@

.f.o:
	$(FC) $(FFLAGS) -c $<

.c~.o:
	$(GET) $(GFLAGS) -p $< > $*.c
	$(CC) $(CFLAGS) -c $*.c

.y~.o:
	$(GET) $(GFLAGS) -p $< > $*.y
	$(YACC) $(YFLAGS) $*.y
	$(CC) $(CFLAGS) -c y.tab.c
	rm -f y.tab.c
	mv y.tab.o $@

.l~.o:
	$(GET) $(GFLAGS) -p $< > $*.l
	$(LEX) $(LFLAGS) $*.l
	$(CC) $(CFLAGS) -c lex.yy.c
	rm -f lex.yy.c
	mv lex.yy.o $@

.f~.o:
	$(GET) $(GFLAGS) -p $< > $*.f
	$(FC) $(FFLAGS) -c $*.f

.y.c:
	$(YACC) $(YFLAGS) $<
	mv y.tab.c $@

.l.c:
	$(LEX) $(LFLAGS) $<
	mv lex.yy.c $@

.y~.c:
	$(GET) $(GFLAGS) -p $< > $*.y
	$(YACC) $(YFLAGS) $*.y
	mv y.tab.c $@

.l~.c:
	$(GET) $(GFLAGS) -p $< > $*.l
	$(LEX) $(LFLAGS) $*.l
	mv lex.yy.c $@

.c.a:
	$(CC) $(CFLAGS) -c $<
	$(AR) $(ARFLAGS) $@ $*.o
	rm -f $*.o

.f.a:
	$(FC) $(FFLAGS) -c $<
	$(AR) $(ARFLAGS) $@ $*.o
	rm -f $*.o

.SCCS_GET:
	sccs $(SCCSFLAGS) get $(SCCSGETFLAGS) $@

