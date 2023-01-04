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
static char	*sccsid = "@(#)$RCSfile: input.c,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:11:43 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from input.c	2.1	(ULTRIX/OSF)	12/3/90";
 */

/*
 *
 *	UNIX debugger
 *
 */


#include <hal/kdb/defs.h>

char		line[LINSIZ];
char            oldline[LINSIZ];
char		*lp;
char		peekc,lastc = EOR;
short		eof;
short		echo = 1;

/* input routines */

eol(c)
char	c;
{
	return(c==EOR || c==';');
}

rdc()
{
	do {
		readchar();
	}
	while(	lastc==' ' || lastc=='\t'
	    ) ;
	return(lastc);
}

char erasec[] = {
	'\b', ' ', '\b'};

char *
editchar(l, p)
char *l;
register char *p;
{
	/* !echo means literalnext */
	if (echo == 0) {
		char printable;
		if (isprint(*p)) {
			write(1, erasec + 1, sizeof(erasec) - 1);
			write(1, p, 1);
		} else {
			printable = *p ^ 0100;
			write(1, "^", 1);
			write(1, &printable, 1);
		}
		echo = 1;
		return(++p);
	}
	switch (*p)
	{
	case 0177:
	case 'H'&077:
		if (p > l)
		{
			if (*p == 0177)
				write(1, erasec, sizeof(erasec));
			else
				write(1, erasec + 1, sizeof(erasec) - 1);
			p--; /* back up over previous character */
		}
		break;
	case 'U'&077:
		while (p > l)
		{
			write(1, erasec, sizeof(erasec));
			p--;	/* back up over this character */
		}
		/* p now points to beginning of input buffer */
		break;
	case 'R'&077:
		write(1, "^R\n", 3);
		while (p > l) {
			if (!isprint(*l)) {
				char printable = *l ^ 0100;
				write(1, "^", 1);
				write(1, &printable, 1);
			} else {
				write(1, l, 1);
			}
			l++;
		}
		/* p has not changed - ^R will be overwritten on next call */
		break;
	case 'V'&077:
		write(1, "^\b", 2);
		echo = 0;
		/* p has not changed - ^V will be overwritten on next call */
		break;
	case '\n':
		p++;
		break;
	default:
		/* Pass through all non-control characters */
		/* swallow all unknown (non-quoted) control characters */
		if (isprint(*p)) p++;
	}
	return(p);
}

readchar()
{
	if ( eof ) {
		lastc=0;
	} 
	else {
		if ( lp==0 ) {
			strncpy(oldline, line, sizeof oldline);
			lp=line;
			do {
				eof = read(0,lp,1)==0;
				lp = editchar(line, lp);
			}
			while( eof==0 && (lp == line || lp[-1]!=EOR) ) ;
			*lp=0;
			lp=line;
		}
		if ( lastc = peekc ) {
			peekc=0;
		} else if ( lastc = *lp ) {
			lp++;
		}
	}
	return(lastc);
}

nextchar()
{
	if ( eol(rdc()) ) {
		lp--;
		return(0);
	} else {
		return(lastc);
	}
}

quotchar()
{
	if ( readchar()=='\\' ) {
		return(readchar());
	} else if ( lastc=='\'' ) {
		return(0);
	} else {
		return(lastc);
	}
}

getformat(deformat)
string_t		deformat;
{
	register string_t	fptr;
	register char		quote;

	fptr=deformat;
	quote=FALSE;
	while ( (quote ? readchar()!=EOR : !eol(readchar())) ) {
		if ( (*fptr++ = lastc)=='"' ) {
			quote = ~quote;
		}
	}
	lp--;
	if ( fptr!=deformat ) {
		*fptr++ = '\0';
	}
}

re_input()
{
	strncpy(line, oldline, sizeof line);
	lp = line;
	eof = 0;
	peekc = ';';
}
