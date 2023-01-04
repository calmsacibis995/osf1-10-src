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
@
 */
/*
 * HISTORY
 */
/* @(#)$RCSfile: ctags.h,v $ $Revision: 4.2.2.2 $ (OSF) $Date: 1993/08/25 22:26:19 $ */
/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * ctags.h     5.3 (Berkeley) 6/1/90
 */

#include <stdio.h>
#include <ctype.h>
#include "ctags_msg.h"

#define	bool	char

#define	YES		1
#define	NO		0
#define	EOS		'\0'

#define	ENDLINE		50		/* max length of pattern */
#define	MAXTOKEN	250		/* max size of single token */

#define	SETLINE		{++lineno;lineftell = ftell(inf);}
#define	GETC(op,exp)	((c = getc(inf)) op (int)exp)

#define	iswhite(arg)	(_wht[arg])	/* T if char is white */
#define	begtoken(arg)	(_btk[arg])	/* T if char can start token */
#define	intoken(arg)	(_itk[arg])	/* T if char can be in token */
#define	endtoken(arg)	(_etk[arg])	/* T if char ends tokens */
#define	isgood(arg)	(_gd[arg])	/* T if char can be after ')' */

typedef struct nd_st {			/* sorting structure */
	struct nd_st	*left,
			*right;		/* left and right sons */
	char	*entry,			/* function or type name */
		*file,			/* file name */
		*pat;			/* search pattern */
	int	lno;			/* for -x option */
	bool	been_warned;		/* set if noticed dup */
} NODE;

extern FILE	*inf;			/* ioptr for current input file */
extern long	lineftell;		/* ftell after getc( inf ) == '\n' */
extern int	lineno,			/* line number of current line */
		xflag;			/* -x: cxref style output */
extern bool	_wht[0177],_etk[0177],_itk[0177],_btk[0177],_gd[0177];
extern char	lbuf[BUFSIZ];

extern nl_catd catd;
#define	MSGSTR(num, str)	catgets(catd, MS_CTAGS, num, str)
