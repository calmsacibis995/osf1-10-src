/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: rules.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/12/10 13:25:05 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)
#endif
/*
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 *
 * rules.c	3.1";
 */

/*
 * DEFAULT RULES FOR UNIX
 *
 * These are the internal rules that "make" trucks around with it at
 * all times. One could completely delete this entire list and just
 * conventionally define a global "include" makefile which had these
 * rules in it. That would make the rules dynamically changeable
 * without recompiling make. This file may be modified to local
 * needs.
 */

#include "defs.h"


char _TARGET_MACHINE[32] = target_MACHINE;	/* patch me */
char _target_machine[32] = target_machine;	/* patch me */
char _CPUTYPE[32]	= this_CPUTYPE;	/* patch me */
char _cputype[32]	= this_cputype;	/* patch me */


char *builtin[] = {

	".SUFFIXES : .out .o .s .c .F .f .e .r .y .yr .ye .l .p .sh .csh .h",

	/*
	 * PRESET VARIABLES
	 */
	"MAKE=make",
	"AR=ar",
	"ARFLAGS=",
	"RANLIB=ranlib",
	"LD=ld",
	"LDFLAGS=",
	"LINT=lint",
	"LINTFLAGS=",
	"CO=co",
	"COFLAGS=-q",
	"CP=cp",
	"CPFLAGS=",
	"MV=mv",
	"MVFLAGS=",
	"RM=rm",
	"RMFLAGS=-f",
	"YACC=yacc",
	"YACCR=yacc -r",
	"YACCE=yacc -e",
	"YFLAGS=",
	"LEX=lex",
	"LFLAGS=",
	"CC=cc",
	"CFLAGS=",
	"AS=as",
	"ASFLAGS=",
	"PC=pc",
	"PFLAGS=",
	"RC=f77",
	"RFLAGS=",
	"EC=efl",
	"EFLAGS=",
	"FC=f77",
	"FFLAGS=",
	"LOADLIBES=",

	/* 001 - Moved definition from main.c */
	"MACHINE="
	  MACHINE,

	_TARGET_MACHINE,
	_target_machine,
	_CPUTYPE,
	_cputype,

	/*
	 * SINGLE SUFFIX RULES
	 */
	".s :",
	"\t$(AS) $(ASFLAGS) -o $@ $<",

	".c :",
	"\t$(CC) $(LDFLAGS) $(CFLAGS) $< $(LOADLIBES) -o $@",

	".F .f :",
	"\t$(FC) $(LDFLAGS) $(FFLAGS) $< $(LOADLIBES) -o $@",

	".e :",
	"\t$(EC) $(LDFLAGS) $(EFLAGS) $< $(LOADLIBES) -o $@",

	".r :",
	"\t$(RC) $(LDFLAGS) $(RFLAGS) $< $(LOADLIBES) -o $@",

	".p :",
	"\t$(PC) $(LDFLAGS) $(PFLAGS) $< $(LOADLIBES) -o $@",

	".y :",
	"\t$(YACC) $(YFLAGS) $<",
	"\t$(CC) $(LDFLAGS) $(CFLAGS) y.tab.c $(LOADLIBES) -ly -o $@",
	"\t$(RM) $(RMFLAGS) y.tab.c",

	".l :",
	"\t$(LEX) $(LFLAGS) $<",
	"\t$(CC) $(LDFLAGS) $(CFLAGS) lex.yy.c $(LOADLIBES) -ll -o $@",
	"\t$(RM) $(RMFLAGS) lex.yy.c",

	".sh :",
	"\t$(CP) $(CPFLAGS) $< $@",
	"\tchmod +x $@",

	".csh :",
	"\t$(CP) $(CPFLAGS) $< $@",
	"\tchmod +x $@",

	".CO :",
	"\t$(CO) $(COFLAGS) $< $@",

	".CLEANUP :",
	"\t$(RM) $(RMFLAGS) $?",

	/*
	 * DOUBLE SUFFIX RULES
	 */
	".s.o :",
	"\t$(AS) -o $@ $<",

	".c.o :",
	"\t$(CC) $(CFLAGS) -c $<",

	".F.o .f.o :",
	"\t$(FC) $(FFLAGS) -c $<",

	".e.o :",
	"\t$(EC) $(EFLAGS) -c $<",

	".r.o :",
	"\t$(RC) $(RFLAGS) -c $<",

	".y.o :",
	"\t$(YACC) $(YFLAGS) $<",
	"\t$(CC) $(CFLAGS) -c y.tab.c",
	"\t$(RM) $(RMFLAGS) y.tab.c",
	"\t$(MV) $(MVFLAGS) y.tab.o $@",

	".yr.o:",
	"\t$(YACCR) $(YFLAGS) $<",
	"\t$(RC) $(RFLAGS) -c y.tab.r",
	"\t$(RM) $(RMFLAGS) y.tab.r",
	"\t$(MV) $(MVFLAGS) y.tab.o $@",

	".ye.o :",
	"\t$(YACCE) $(YFLAGS) $<",
	"\t$(EC) $(EFLAGS) -c y.tab.e",
	"\t$(RM) $(RMFLAGS) y.tab.e",
	"\t$(MV) $(MVFLAGS) y.tab.o $@",

	".l.o :",
	"\t$(LEX) $(LFLAGS) $<",
	"\t$(CC) $(CFLAGS) -c lex.yy.c",
	"\t$(RM) $(RMFLAGS) lex.yy.c",
	"\t$(MV) $(MVFLAGS) lex.yy.o $@",

	".p.o :",
	"\t$(PC) $(PFLAGS) -c $<",

	".y.c :",
	"\t$(YACC) $(YFLAGS) $<",
	"\t$(MV) $(MVFLAGS) y.tab.c $@",

	".yr.r:",
	"\t$(YACCR) $(YFLAGS) $<",
	"\t$(MV) $(MVFLAGS) y.tab.r $@",

	".ye.e :",
	"\t$(YACCE) $(YFLAGS) $<",
	"\t$(MV) $(MVFLAGS) y.tab.e $@",

	".l.c :",
	"\t$(LEX) $(LFLAGS) $<",
	"\t$(MV) $(MVFLAGS) lex.yy.c $@",

	".o.out .s.out .c.out :",
	"\t$(CC) $(LDFLAGS) $(CFLAGS) $< $(LOADLIBES) -o $@",

	".F.out .f.out :",
	"\t$(FC) $(LDFLAGS) $(FFLAGS) $< $(LOADLIBES) -o $@",

	".e.out :",
	"\t$(EC) $(LDFLAGS) $(EFLAGS) $< $(LOADLIBES) -o $@",

	".r.out :",
	"\t$(RC) $(LDFLAGS) $(RFLAGS) $< $(LOADLIBES) -o $@",

	".y.out :",
	"\t$(YACC) $(YFLAGS) $<",
	"\t$(CC) $(LDFLAGS) $(CFLAGS) y.tab.c $(LOADLIBES) -ly -o $@",
	"\t$(RM) $(RMFLAGS) y.tab.c",

	".l.out :",
	"\t$(LEX) $(LFLAGS) $<",
	"\t$(CC) $(LDFLAGS) $(CFLAGS) lex.yy.c $(LOADLIBES) -ll -o $@",
	"\t$(RM) $(RMFLAGS) lex.yy.c",

	".p.out :",
	"\t$(PC) $(LDFLAGS) $(PFLAGS) $< $(LOADLIBES) -o $@",

	0
};
