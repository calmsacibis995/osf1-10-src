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
static char rcsid[] = "@(#)$RCSfile: list.c,v $ $Revision: 4.2.9.2 $ (DEC) $Date: 1993/08/02 18:20:17 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0.1
 */
/* 
 * COMPONENT_NAME: CMDMAILX list.c
 * 
 * FUNCTIONS: MSGSTR, check, evalcol, first, getmsglist, getrawlist, 
 *            mark, markall, matchsubj, metamess, regret, scan, 
 *            scaninit, sender, unmark 
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
 *	list.c       5.4 (Berkeley) 11/2/85
 */

#include "rcv.h"
#include <ctype.h>

#include "Mail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s) 

/*
 * Mail -- a mail program
 *
 * Message list handling.
 */

/*
 * Convert the user string of message numbers and
 * store the numbers into vector.
 *
 * Returns the count of messages picked up or -1 on error.
 */

getmsglist(buf, vector, flags)
	char *buf;
	int *vector;
{
	register int *ip;
	register struct message *mp;

	if (markall(buf, flags) < 0)
		return(-1);
	ip = vector;
	for (mp = &message[0]; mp < &message[msgCount]; mp++)
		if (mp->m_flag & MMARK)
			*ip++ = mp - &message[0] + 1;
	*ip = NULL;
	return(ip - vector);
}

/*
 * Mark all messages that the user wanted from the command
 * line in the message structure.  Return 0 on success, -1
 * on error.
 */

/*
 * Bit values for colon modifiers.
 */

#define	CMNEW		01		/* New messages */
#define	CMOLD		02		/* Old messages */
#define	CMUNREAD	04		/* Unread messages */
#define	CMDELETED	010		/* Deleted messages */
#define	CMREAD		020		/* Read messages */

/*
 * The following table describes the letters which can follow
 * the colon and gives the corresponding modifier bit.
 */

struct coltab {
	char	co_char;		/* What to find past : */
	int	co_bit;			/* Associated modifier bit */
	int	co_mask;		/* m_status bits to mask */
	int	co_equal;		/* ... must equal this */
} coltab[] = {
	{ 'n',		CMNEW,		MNEW,		MNEW },
	{ 'o',		CMOLD,		MNEW,		0 },
	{ 'u',		CMUNREAD,	MREAD,		0 },
	{ 'd',		CMDELETED,	MDELETED,	MDELETED },
	{ 'r',		CMREAD,		MREAD,		MREAD },
	{ 0,		0,		0,		0 }
};

static	int	lastcolmod;

markall(buf, f)
	char buf[];
{
	register char **np;
	register int i;
	register struct message *mp;
	char *namelist[NMLSIZE], *bufp;
	int tok, beg, mc, star, other, valdot, colmod, colresult;

	valdot = dot - &message[0] + 1;
	colmod = 0;
	for (i = 1; i <= msgCount; i++)
		unmark(i);
	bufp = buf;
	mc = 0;
	np = &namelist[0];
	scaninit();
	tok = scan(&bufp);
	star = 0;
	other = 0;
	beg = 0;
	while (tok != TEOL) {
		switch (tok) {
		case TNUMBER:
number:
			if (star) {
				printf(MSGSTR(NONUMS, "No numbers mixed with *\n")); /*MSG*/
				return(-1);
			}
			mc++;
			other++;
			if (beg != 0) {
				if (check(lexnumber, f))
					return(-1);
				for (i = beg; i <= lexnumber; i++)
					if ((message[i - 1].m_flag & MDELETED) == f)
						mark(i);
				beg = 0;
				break;
			}
			beg = lexnumber;
			if (check(beg, f))
				return(-1);
			tok = scan(&bufp);
			regret(tok);
			if (tok != TDASH) {
				mark(beg);
				beg = 0;
			}
			break;

		case TPLUS:
			if (beg != 0) {
				printf(MSGSTR(NONNUM, "Non-numeric second argument\n")); /*MSG*/
				return(-1);
			}
			i = valdot;
			do {
				i++;
				if (i > msgCount) {
					printf(MSGSTR(PASTEOF, "Referencing beyond EOF\n")); /*MSG*/
					return(-1);
				}
			} while ((message[i - 1].m_flag & MDELETED) != f);
			mark(i);
			break;

		case TDASH:
			if (beg == 0) {
				i = valdot;
				do {
					i--;
					if (i <= 0) {
						printf(MSGSTR(BEFORE1, "Referencing before 1\n")); /*MSG*/
						return(-1);
					}
				} while ((message[i - 1].m_flag & MDELETED) != f);
				mark(i);
			}
			break;

		case TSTRING:
			if (beg != 0) {
				printf(MSGSTR(NONNUM, "Non-numeric second argument\n")); /*MSG*/
				return(-1);
			}
			other++;
			if (lexstring[0] == ':') {
				colresult = evalcol(lexstring[1]);
				if (colresult == 0) {
					printf(MSGSTR(UNKCOLON, "Unknown colon modifier \"%s\"\n"), /*MSG*/
					    lexstring);
					return(-1);
				}
				colmod |= colresult;
			}
			else
				*np++ = savestr(lexstring);
			break;

		case TDOLLAR:
		case TUP:
		case TDOT:
			lexnumber = metamess(lexstring[0], f);
			if (lexnumber == -1)
				return(-1);
			goto number;

		case TSTAR:
			if (other) {
				printf(MSGSTR(NOMIX, "Can't mix \"*\" with anything\n")); /*MSG*/
				return(-1);
			}
			star++;
			break;
		}
		tok = scan(&bufp);
	}
	lastcolmod = colmod;
	*np = NULLSTR;
	mc = 0;
	if (star) {
		for (i = 0; i < msgCount; i++)
			if ((message[i].m_flag & MDELETED) == f) {
				mark(i+1);
				mc++;
			}
		if (mc == 0) {
			printf(MSGSTR(NOAPPMSGS, "No applicable messages.\n")); /*MSG*/
			return(-1);
		}
		return(0);
	}

	/*
	 * If no numbers were given, mark all of the messages,
	 * so that we can unmark any whose sender was not selected
	 * if any user names were given.
	 */

	if ((np > namelist || colmod != 0) && mc == 0)
		for (i = 1; i <= msgCount; i++)
			if ((message[i-1].m_flag & MDELETED) == f)
				mark(i);

	/*
	 * If any names were given, go through and eliminate any
	 * messages whose senders were not requested.
	 */

	if (np > namelist) {
		for (i = 1; i <= msgCount; i++) {
			for (mc = 0, np = &namelist[0]; *np != NULLSTR; np++)
				if (**np == '/') {
					if (matchsubj(*np, i)) {
						mc++;
						break;
					}
				}
				else {
					if (sender(*np, i)) {
						mc++;
						break;
					}
				}
			if (mc == 0)
				unmark(i);
		}

		/*
		 * Make sure we got some decent messages.
		 */

		mc = 0;
		for (i = 1; i <= msgCount; i++)
			if (message[i-1].m_flag & MMARK) {
				mc++;
				break;
			}
		if (mc == 0) {
			printf(MSGSTR(NOAPPMSGSA, "No applicable messages from {%s"), namelist[0]); /*MSG*/
			for (np = &namelist[1]; *np != NULLSTR; np++)
				printf(", %s", *np);
			printf("}\n");
			return(-1);
		}
	}

	/*
	 * If any colon modifiers were given, go through and
	 * unmark any messages which do not satisfy the modifiers.
	 */

	if (colmod != 0) {
		for (i = 1; i <= msgCount; i++) {
			register struct coltab *colp;

			mp = &message[i - 1];
			for (colp = &coltab[0]; colp->co_char; colp++)
				if (colp->co_bit & colmod)
					if ((mp->m_flag & colp->co_mask)
					    != colp->co_equal)
						unmark(i);
			
		}
		for (mp = &message[0]; mp < &message[msgCount]; mp++)
			if (mp->m_flag & MMARK)
				break;
		if (mp >= &message[msgCount]) {
			register struct coltab *colp;

			printf(MSGSTR(NOGOOD, "No messages satisfy")); /*MSG*/
			for (colp = &coltab[0]; colp->co_char; colp++)
				if (colp->co_bit & colmod)
					printf(" :%c", colp->co_char);
			printf("\n");
			return(-1);
		}
	}
	return(0);
}

/*
 * Turn the character after a colon modifier into a bit
 * value.
 */
evalcol(col)
{
	register struct coltab *colp;

	if (col == 0)
		return(lastcolmod);
	for (colp = &coltab[0]; colp->co_char; colp++)
		if (colp->co_char == col)
			return(colp->co_bit);
	return(0);
}

/*
 * Check the passed message number for legality and proper flags.
 */

check(mesg, f)
{
	register struct message *mp;

	if (mesg < 1 || mesg > msgCount) {
		printf(MSGSTR(INVMSGNO, "%d: Invalid message number\n"), mesg); /*MSG*/
		return(-1);
	}
	mp = &message[mesg-1];
	if ((mp->m_flag & MDELETED) != f) {
		printf(MSGSTR(BADMSG, "%d: Inappropriate message\n"), mesg); /*MSG*/
		return(-1);
	}
	return(0);
}

/*
 * Scan out the list of string arguments, shell style
 * for a RAWLIST.
 */

getrawlist(line, argv, argc)
	char line[];
	char **argv;
	int  argc;
{
	register char **ap, *cp, *cp2;
	char linebuf[BUFSIZ], quotec;
	register char **last;
#ifdef ASIAN_I18N
	int mb;
#endif

	ap = argv;
	cp = line;
	last = argv + argc - 1;
	while (*cp != '\0') {
#ifdef ASIAN_I18N
		while (mb_any(cp, " \t", &mb))
			cp+=mb;
#else
		while (any(*cp, " \t"))
			cp++;
#endif
		cp2 = linebuf;
		quotec = 0;
		/* replaced while loop to fix a bug with embedded " chars,
		 * the OSF/1 code only worked when a " char was the first
		 * char in the input string; i.e. set prompt="ls -l" would
		 * give 2 strings [prompt="ls] and [-l"]
		 * added in with SVID-2 changes
		 */
		while (*cp != '\0') {
			if (quotec) {
				if (*cp == quotec) {
					quotec=0;
					cp++;
				} else
					*cp2++ = *cp++;
			} else {
#ifdef ASIAN_I18N
				if (mb_any(cp, " \t", &mb))
#else
				if (any(*cp, " \t"))
#endif
					break;
				if (any(*cp, "'\""))
					quotec = *cp++;
				else
					*cp2++ = *cp++;
			}
		}
		*cp2 = '\0';
		if (cp2 == linebuf)
			break;
		if (ap >= last) {
			printf(MSGSTR(EXCESS, "Too many elements in the list; excess discarded\n")); /*MSG*/
			break;
		}
		*ap++ = savestr(linebuf);
	}

	*ap = NULLSTR;
	return(ap-argv);
}

/*
 * scan out a single lexical item and return its token number,
 * updating the string pointer passed **p.  Also, store the value
 * of the number or string scanned in lexnumber or lexstring as
 * appropriate.  In any event, store the scanned `thing' in lexstring.
 */

struct lex {
	char	l_char;
	char	l_token;
} singles[] = {
	{ '$',	TDOLLAR },
	{ '.',	TDOT },
	{ '^',	TUP },
	{ '*',	TSTAR },
	{ '-',	TDASH },
	{ '+',	TPLUS },
	{ '(',	TOPEN },
	{ ')',	TCLOSE },
	{ 0,	0 }
};

scan(sp)
	char **sp;
{
	register char *cp, *cp2;
	register int c;
	register struct lex *lp;
	int quotec;
#ifdef ASIAN_I18N
	int mb;
#endif

	if (regretp >= 0) {
		copy(stringstack[regretp], lexstring);
		lexnumber = numberstack[regretp];
		return(regretstack[regretp--]);
	}
	cp = *sp;
	cp2 = lexstring;
#ifdef ASIAN_I18N
	while (mb_any(cp, " \t", &mb))
		cp+=mb;
	c = *cp++;
#else	/* ASIAN_I18N */
	c = *cp++;

	/*
	 * strip away leading white space.
	 */

	while (any(c, " \t"))
		c = *cp++;
#endif	/* ASIAN_I18N */

	/*
	 * If no characters remain, we are at end of line,
	 * so report that.
	 */

	if (c == '\0') {
		*sp = --cp;
		return(TEOL);
	}

	/*
	 * If the leading character is a digit, scan
	 * the number and convert it on the fly.
	 * Return TNUMBER when done.
	 */

#ifdef ASIAN_I18N
	if (isdigit(0xff & c)) {
		lexnumber = 0;
		while (isdigit(0xff & c)) {
#else
	if (isdigit(c)) {
		lexnumber = 0;
		while (isdigit(c)) {
#endif
			lexnumber = lexnumber*10 + c - '0';
			*cp2++ = c;
			c = *cp++;
		}
		*cp2 = '\0';
		*sp = --cp;
		return(TNUMBER);
	}

	/*
	 * Check for single character tokens; return such
	 * if found.
	 */

	for (lp = &singles[0]; lp->l_char != 0; lp++)
		if (c == lp->l_char) {
			lexstring[0] = c;
			lexstring[1] = '\0';
			*sp = cp;
			return(lp->l_token);
		}

	/*
	 * We've got a string!  Copy all the characters
	 * of the string into lexstring, until we see
	 * a null, space, or tab.
	 * If the lead character is a " or ', save it
	 * and scan until you get another.
	 */

	quotec = 0;
	if (any(c, "'\"")) {
		quotec = c;
		c = *cp++;
	}
	while (c != '\0') {
		if (c == quotec)
			break;
		if (quotec == 0 && any(c, " \t"))
			break;
		if (cp2 - lexstring < STRINGLEN-1)
			*cp2++ = c;
		c = *cp++;
	}
	if (quotec && c == 0)
		fprintf(stderr, MSGSTR(MISSING, "Missing %c\n"), quotec); /*MSG*/
	*sp = --cp;
	*cp2 = '\0';
	return(TSTRING);
}

/*
 * Unscan the named token by pushing it onto the regret stack.
 */

regret(token)
{
	if (++regretp >= REGDEP)
		panic(MSGSTR(REGRETS, "Too many regrets")); /*MSG*/
	regretstack[regretp] = token;
	lexstring[STRINGLEN-1] = '\0';
	stringstack[regretp] = savestr(lexstring);
	numberstack[regretp] = lexnumber;
}

/*
 * Reset all the scanner global variables.
 */

scaninit()
{
	regretp = -1;
}

/*
 * Find the first message whose flags & m == f  and return
 * its message number.
 */

first(f, m)
{
	register int mesg;
	register struct message *mp;

	mesg = dot - &message[0] + 1;
	f &= MDELETED;
	m &= MDELETED;
	for (mp = dot; mp < &message[msgCount]; mp++) {
		if ((mp->m_flag & m) == f)
			return(mesg);
		mesg++;
	}
	mesg = dot - &message[0];
	for (mp = dot-1; mp >= &message[0]; mp--) {
		if ((mp->m_flag & m) == f)
			return(mesg);
		mesg--;
	}
	return(NULL);
}

/*
 * See if the passed name sent the passed message number.  Return true
 * if so.
 */

sender(str, mesg)
	char *str;
{
	register struct message *mp;
	register char *cp, *cp2, *backup;

	mp = &message[mesg-1];
	backup = cp2 = nameof(mp, 0);
	cp = str;

	/* added for SVID-2, merged from Sys V code */
	if (value("allnet")) {
		if( (cp2 = strrchr(backup, '!')) == NULLSTR)
			cp2 = backup;
		else
		{
			cp2++;		/* advance past ! */
			backup= cp2;	/* search starts after ! */
		}
	}

	while (*cp2) {
		if (*cp == 0)
			return(1);
		if (raise(*cp++) != raise(*cp2++)) {
			cp2 = ++backup;
			cp = str;
		}
	}

	return(*cp == 0);
}

/*
 * See if the given string matches inside the subject field of the
 * given message.  For the purpose of the scan, we ignore case differences.
 * If it does, return true.  The string search argument is assumed to
 * have the form "/search-string."  If it is of the form "/," we use the
 * previous search string.
 */

char lastscan[128];

matchsubj(str, mesg)
	char *str;
{
	register struct message *mp;
	register char *cp, *cp2, *backup;

	str++;
	if (strlen(str) == 0)
		str = lastscan;
	else
		strcpy(lastscan, str);
	mp = &message[mesg-1];
	
	/*
	 * Now look, ignoring case, for the word in the string.
	 */

	cp = str;
	cp2 = hfield("subject", mp);
	if (cp2 == NULLSTR)
		return(0);
	backup = cp2;
	while (*cp2) {
		if (*cp == 0)
			return(1);
		if (raise(*cp++) != raise(*cp2++)) {
			cp2 = ++backup;
			cp = str;
		}
	}
	return(*cp == 0);
}

/*
 * Mark the named message by setting its mark bit.
 */

mark(mesg)
{
	register int i;

	i = mesg;
	if (i < 1 || i > msgCount)
		panic(MSGSTR(MARK, "Bad message number to mark")); /*MSG*/
	message[i-1].m_flag |= MMARK;
}

/*
 * Unmark the named message.
 */

unmark(mesg)
{
	register int i;

	i = mesg;
	if (i < 1 || i > msgCount)
		panic(MSGSTR(UNMARK, "Bad message number to unmark")); /*MSG*/
	message[i-1].m_flag &= ~MMARK;
}

/*
 * Return the message number corresponding to the passed meta character.
 */

metamess(meta, f)
{
	register int c, m;
	register struct message *mp;

	c = meta;
	switch (c) {
	case '^':
		/*
		 * First 'good' message left.
		 */
		for (mp = &message[0]; mp < &message[msgCount]; mp++)
			if ((mp->m_flag & MDELETED) == f)
				return(mp - &message[0] + 1);
		printf(MSGSTR(NOAPPLY, "No applicable messages\n")); /*MSG*/
		return(-1);

	case '$':
		/*
		 * Last 'good message left.
		 */
		for (mp = &message[msgCount-1]; mp >= &message[0]; mp--)
			if ((mp->m_flag & MDELETED) == f)
				return(mp - &message[0] + 1);
		printf(MSGSTR(NOAPPLY, "No applicable messages\n")); /*MSG*/
		return(-1);

	case '.':
		/* 
		 * Current message.
		 */
		m = dot - &message[0] + 1;
		if ((dot->m_flag & MDELETED) != f) {
			printf(MSGSTR(BADMSG, "%d: Inappropriate message\n"), m); /*MSG*/
			return(-1);
		}
		return(m);

	default:
		printf(MSGSTR(UNKMETA, "Unknown metachar (%c)\n"), c); /*MSG*/
		return(-1);
	}
}
