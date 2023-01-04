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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: getprm.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/09/07 16:05:38 $";
#endif
/* 
 * COMPONENT_NAME: UUCP getprm.c
 * 
 * FUNCTIONS: bal, charType, getprm, split 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
getprm.c	1.3  com/cmd/uucp,3.1,9013 10/10/89 13:39:32";
*/
/*	/sccs/src/cmd/uucp/s.getprm.c
	getprm.c	1.4	7/29/85 16:33:01
*/
#include "uucp.h"
/* VERSION( getprm.c	5.2 -  -  ); */

#define LQUOTE	'('
#define RQUOTE ')'

#define ZERO	0
#define WHITE	1
#define DELIMIT	2
#define QUOTE	3
#define OTHER	9


extern char *bal();

/*
 * get next parameter from s
 *	s	-> string to scan
 *	prm	-> pointer to use to return token
 * return:
 *	 s	-> pointer to next character 
 *		NULL at end
 */
char *
getprm(s, prm)
register char *s, *prm;
{
	register char *c;
	char rightq;		/* the right quote character */

	/* skip white space */
	while (charType(*s) == WHITE)
		s++;

	*prm = '\0';
	if (*s == '\0')
		return(NULL);

	if (charType(*s) == DELIMIT) {
		*prm++ = *s++;
		*prm = '\0';
		return(s);
	}

	/* look for quoted argument */
	if (charType(*s) == QUOTE) {
	    /* setup the right quote character */
	    if (*s == LQUOTE)
		rightq = RQUOTE;
	    else
		rightq = *s;

	    c = bal(s, rightq);
 	    (void) strncpy(prm, s, c-s+1);
	    prm[c-s+1] = '\0';
	    if ( *(s=c) == rightq) /* if end of string don't increment */
		s++;
	    return(s);
	}

	while (charType(*s) == OTHER)
		*prm++ = *s++;
	*prm = '\0';

	return(s);
}

/*
 * split name into system and file part
 *	name	-> string to scan
 *	sys	-> return area for system name
 *	rest	-> return area for remainder
 * return:
 *	0	-> no system prefix
 *	TRUE	-> system prefix return in sys
 */

split(name, sys, rest)
register char *name;
char *sys, *rest;
{
	register char *c, *n;

	*sys = '\0';
	if (*name == LQUOTE) {
		c = bal(name, RQUOTE);
		name++;
		(void) strncpy(rest, name, c-name);
		rest[c-name] = '\0';
		return(0);
	}

	for (n = name ;; n = c+1) { /* loop to get rid if initial Mynames */
	    if ((c = (char *)strchr(n, '!')) == NULL) {
		(void) strcpy(rest, n);
		return(0);
	    }

	    /*  ignore escaped '!' */
	    if ((c != n) && (*(c-1) == '\\')) {
		*(c-1) = '\0';
		(void) strcpy(rest, n);
		(void) strcat(rest, c);
		return(0);
	    }

	    *c = '\0';
	    if (EQUALS(n, Myname)) /* initial system is Myname */
		continue;

	    (void) strcpy(sys, n);
	    (void) strcpy(rest, ++c);
	    return(TRUE);
	}
	/*NOTREACHED*/
}


/*
 * bal - get balanced quoted string
 *
 * s - input string
 * r - right quote
 * Note: *s is the left quote
 * return:
 *  pointer to the end of the quoted string
 * Note:
 *	If the string is not balanced, it returns a pointer to the
 *	end of the string.
 */

char *
bal(s, r)
register char *s;
char r;
{
	short count = 1;
	char l;		/* left quote character */

	for (l = *s++; *s; s++) {
	    if (*s == r) {
		if (--count == 0)
		    break;	/* this is the balanced end */
	    }
	    else if (*s == l)
		count++;
	}
	return(s);
}


/*
 * charType - classify type of character , white space, delimiter, other
 *
 * s - character
 * return:
 *	WHITE - whitespace
 *	DELIMIT - delimiter
 *	QUOTE - some type of quote "'(`
 *	OTHER - other
 */

charType(s)
char s;
{
	switch(s) {
	case '>':
	case '<':
	case '|':
	case ';':
	case '&':
	case '^':
	case '\\':
		return(DELIMIT);

	case ' ':
	case '\t':
	case '\n':
		return(WHITE);

	case '\0':
		return(ZERO);


	case '"':
	case '\'':
	case '`':
	case LQUOTE:
		return(QUOTE);

	default:
		return(OTHER);
	}
}
