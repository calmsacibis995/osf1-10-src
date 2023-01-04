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
/*	
 *	@(#)$RCSfile: e.h,v $ $Revision: 4.2.3.4 $ (DEC) $Date: 1992/04/21 10:07:50 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*	e.h	4.2	83/02/12	*/

#include <stdio.h>

#define	FATAL	1
#define	ROM	'1'
#ifndef NEQN
#define	ITAL	'2'
#define	BLD	'3'
#else /* NEQN */
#define	ITAL	'1'
#define	BLD	'1'
#endif /* NEQN */

#ifndef NEQN
#define	VERT(n)	((((n)+1)/3)*3)
#else /* NEQN */
#define	VERT(n)	(20 * (n))
#endif /* NEQN */
#define	EFFPS(p)	((p) >= 6 ? (p) : 6)

#define YYSTYPE long

extern int	dbg;
extern int	ct;
extern YYSTYPE	lp[];
extern YYSTYPE	used[];	/* available registers */
extern int	ps;	/* dflt init pt size */
extern int	deltaps;	/* default change in ps */
extern int	gsize;	/* global size */
extern int	gfont;	/* global font */
extern int	ft;	/* dflt font */
extern FILE	*curfile;	/* current input file */
extern int	ifile;	/* input file number */
extern int	linect;	/* line number in current file */
extern int	eqline;	/* line where eqn started */
extern int	svargc;
extern char	**svargv;
extern int	eht[];
extern int	ebase[];
extern int	lfont[];
extern int	rfont[];
extern YYSTYPE	yyval;
extern YYSTYPE	*yypv;
extern YYSTYPE	yylval;
extern int	eqnreg, eqnht;
extern int	lefteq, righteq;
extern int	lastchar;	/* last character read by lex */
extern int	markline;	/* 1 if this EQ/EN contains mark or lineup */

typedef struct s_tbl {
	char	*name;
	char	*defn;
	struct s_tbl *next;
} tbl;
