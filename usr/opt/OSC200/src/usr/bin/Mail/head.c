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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: head.c,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/08/02 18:19:55 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * COMPONENT_NAME: CMDMAILX head.c
 * 
 * FUNCTIONS: MSGSTR, any, cmatch, copy, copyin, fail, isalpha, 
 *            isdate, isdigit, ishead, nextword, parse, raise 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	head.c       5.2 (Berkeley) 6/21/85
 */

#include "rcv.h"

#include "Mail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s) 

/*
 * Mail -- a mail program
 *
 * Routines for processing and detecting headlines.
 */

/*
 * See if the passed line buffer is a mail header.
 * Return true if yes.  Note the extreme pains to
 * accomodate all funny formats.
 */

ishead(linebuf)
	char linebuf[];
{
	register char *cp;
	struct headline hl;
	char parbuf[BUFSIZ];

	cp = linebuf;
	if (strncmp("From ", cp, 5) != 0)
		return(0);
	parse(cp, &hl, parbuf);
	if (hl.l_from == NULLSTR || hl.l_date == NULLSTR) {
		fail(linebuf, MSGSTR(NOFDFIELD, "No from or date field")); /*MSG*/
		return(0);
	}
/* The parsing of the date field which follows was causing
   severe problems. With the national language constructs
   the number of date templates possible reached infinity.
   Our solution was to accept any format in this field
   as a date. This is not a bad assumption in that most
   of these 'From' lines will be written by bellmail.
   It also solves the timezone problem.
   We left the orginal Berkeley parsing routines for reference. */
	if (!isdate(hl.l_date)) {
		fail(linebuf, MSGSTR(ILLDATE, "Date field not legal date")); /*MSG*/
		return(0);
	}  
	
	/*
	 * I guess we got it!
	 */

	return(1);
}

fail(linebuf, reason)
	char linebuf[], reason[];
{
	char buf[NL_TEXTMAX];

	if (1 /*value("debug") == NULLSTR*/)
		return;
	strcpy(buf,reason);
	fprintf(stderr, MSGSTR(NOHEADER, "\"%s\"\nnot a header because %s\n"), linebuf, buf); /*MSG*/
}

/*
 * Split a headline into its useful components.
 * Copy the line into dynamic string space, then set
 * pointers into the copied line in the passed headline
 * structure.  Actually, it scans.
 */

parse(line, hl, pbuf)
	char line[], pbuf[];
	struct headline *hl;
{
	register char *cp, *dp;
	char *sp;
	char word[LINESIZE];

	hl->l_from = NULLSTR;
	hl->l_tty = NULLSTR;
	hl->l_date = NULLSTR;
	cp = line;
	sp = pbuf;

	/*
	 * Skip the first "word" of the line, which should be "From"
	 * anyway.
	 */

	cp = nextword(cp, word);
	dp = nextword(cp, word);
	if (strcmp(word, "SMTP") == 0)		/* this is an ugly hack */
		dp = nextword(dp, word);
	if (!equal(word, ""))
		hl->l_from = copyin(word, &sp);
	if (dp && strncmp(dp, "tty", 3) == 0) {
		cp = nextword(dp, word);
		hl->l_tty = copyin(word, &sp);
		if (cp != NULLSTR)
			hl->l_date = copyin(cp, &sp);
	}
	else
		if (dp != NULLSTR)
			hl->l_date = copyin(dp, &sp);
}

/*
 * Copy the string on the left into the string on the right
 * and bump the right (reference) string pointer by the length.
 * Thus, dynamically allocate space in the right string, copying
 * the left string into it.
 */

char *
copyin(src, space)
	char src[];
	char **space;
{
	register char *cp, *top;
	register int s;

	s = strlen(src);
	cp = *space;
	top = cp;
	strcpy(cp, src);
	cp += s + 1;
	*space = cp;
	return(top);
}

/*
 * Test to see if the passed string is a ctime(3) generated
 * date string as documented in the manual.  The template
 * below is used as the criterion of correctness.
 * Also, we check for a possible trailing time zone using
 * the auxtype template.
 */

#define	L	1		/* A lower case char */
#define	S	2		/* A space */
#define	D	3		/* A digit */
#define	O	4		/* An optional digit or space */
#define	C	5		/* A colon */
#define	N	6		/* A new line */
#define U	7		/* An upper case char */

char ctypes[] = {U,L,L,S,U,L,L,S,O,D,S,D,D,C,D,D,C,D,D,S,D,D,D,D,0};
char tmztypes[] = {U,L,L,S,U,L,L,S,O,D,S,D,D,C,D,D,C,D,D,S,U,U,U,S,D,D,D,D,0};
char ctypes_nosecs[] = {U,L,L,S,U,L,L,S,O,D,S,D,D,C,D,D,S,D,D,D,D,0};
char tmztypes_nosecs[] = {U,L,L,S,U,L,L,S,O,D,S,D,D,C,D,D,S,U,U,U,S,D,D,D,D,0};

isdate(date)
	char date[];
{
	register char *cp;

	cp = date;
	return(1);
}

/*
 * Match the given string against the given template.
 * Return 1 if they match, 0 if they don't
 */

cmatch(str, temp)
	char str[], temp[];
{
	register char *cp, *tp;
	register int c;

	cp = str;
	tp = temp;
	while (*cp != '\0' && *tp != 0) {
		c = *cp++;
		switch (*tp++) {
		case L:
			if (c < 'a' || c > 'z')
				return(0);
			break;

		case U:
			if (c < 'A' || c > 'Z')
				return(0);
			break;

		case S:
			if (c != ' ')
				return(0);
			break;

		case D:
#ifdef ASIAN_I18N
			if (!isdigit(0xff & c))
#else
			if (!isdigit(c))
#endif
				return(0);
			break;

		case O:
#ifdef ASIAN_I18N
			if (c != ' ' && !isdigit(0xff & c))
#else
			if (c != ' ' && !isdigit(c))
#endif
				return(0);
			break;

		case C:
			if (c != ':')
				return(0);
			break;

		case N:
			if (c != '\n')
				return(0);
			break;
		}
	}
	if (*cp != '\0' || *tp != 0)
		return(0);
	return(1);
}

/*
 * Collect a liberal (space, tab delimited) word into the word buffer
 * passed.  Also, return a pointer to the next word following that,
 * or NULLSTR if none follow.
 */

char *
nextword(wp, wbuf)
	char wp[], wbuf[];
{
	register char *cp, *cp2;
#ifdef ASIAN_I18N
	int mb;
#endif

	if ((cp = wp) == NULLSTR) {
		copy("", wbuf);
		return(NULLSTR);
	}
	cp2 = wbuf;
#ifdef ASIAN_I18N
	while (!mb_any(cp, " \t", &mb) && *cp != '\0')
#else
	while (!any(*cp, " \t") && *cp != '\0')
#endif
		if (*cp == '"') {
 			*cp2++ = *cp++;
 			while (*cp != '\0' && *cp != '"')
 				*cp2++ = *cp++;
 			if (*cp == '"')
 				*cp2++ = *cp++;
 		} else
 			*cp2++ = *cp++;
	*cp2 = '\0';
#ifdef ASIAN_I18N
	while (mb_any(cp, " \t", &mb))
		cp+=mb;
#else
	while (any(*cp, " \t"))
		cp++;
#endif
	if (*cp == '\0')
		return(NULLSTR);
	return(cp);
}

/*
 * Test to see if the character is an ascii alphabetic.
 */

isalpha(c)
{
	register int ch;

	ch = raise(c);
	return(ch >= 'A' && ch <= 'Z');
}

/*
 * Test to see if the character is an ascii digit.
 */

isdigit(c)
{
	return(c >= '0' && c <= '9');
}

/*
 * Copy str1 to str2, return pointer to null in str2.
 */

char *
copy(str1, str2)
	char *str1, *str2;
{
	register char *s1, *s2;

	s1 = str1;
	s2 = str2;
	while (*s1)
		*s2++ = *s1++;
	*s2 = 0;
	return(s2);
}

/*
 * Is ch any of the characters in str?
 */

any(ch, str)
	char *str;
{
	register char *f;
	register c;

	f = str;
	c = ch;
	while (*f)
		if (c == *f++)
			return(1);
	return(0);
}


#ifdef ASIAN_I18N
/*
 * Is *cp any of the characters in str?
 */
mb_any(char *cp, char *str, int *mbp)
{
	register char *f;
	wchar_t wc;

	*mbp=1;
	for (f=str; *f; f++)
		if (*cp==*f) return(1);

	/* try mb space case */
	if ((*mbp=wctomb(&wc, cp, MB_MAX)) != -1 &&
	    iswspace(wc) && *mbp>1) { 
		for (f=str; *f; f++)
			if (*f==' ') return(1);
	}
	*mbp=1;
	return(0);
}
#endif /* ASIAN_I18N */



/*
 * Convert lower case letters to upper case.
 */

raise(int c)
{
	if (c >= 'a' && c <= 'z')
		c += 'A' - 'a';
	return(c);
}
