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
static char rcsid[] = "@(#)$RCSfile: aux.c,v $ $Revision: 4.2.9.3 $ (DEC) $Date: 1994/01/24 22:56:34 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0.1
 */
/* 
 * COMPONENT_NAME: CMDMAILX aux.c
 * 
 * FUNCTIONS: MSGSTR, _error, alter, anyof, argcount, blankline, 
 *            blockof, charcount, copyname, gethfield, hcontents, 
 *            hfield, icequal, index, isdir, ishfield, isign, 
 *            istrcpy, member, name1, nameof, nameto, numeric, offsetof, 
 *            panic, prs, rindex, savestr, skin, source, source1, 
 *            touch, unstack 
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
 *	aux.c        5.4 (Berkeley) 1/13/86
 */
/*
 * Modification History:
 *
 * 00 Todd Kaehler Thu Aug  8 1991
 *    converted from using OPEN_MAX to find maximum number of open files
 *    to MAX_NOFILE and getdtablesize(2) which tells the now configurable
 *    maximum number of open files.
 */

#include "rcv.h"
#include <sys/stat.h>
#include <ctype.h>

#include "Mail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s) 

/*
 * Mail -- a mail program
 *
 * Auxiliary functions.
 */

#ifdef ASIAN_I18N
extern char *convert_in_header();
#endif


/*
 * Return a pointer to a dynamic copy of the argument.
 */

char *
savestr(str)
	char *str;
{
	register char *cp, *cp2, *top;

	for (cp = str; *cp; cp++)
		;
	top = salloc(cp-str + 1);
	if (top == NULLSTR)
		return(NULLSTR);
	for (cp = str, cp2 = top; *cp; cp++)
		*cp2++ = *cp;
	*cp2 = 0;
	return(top);
}

/*
 * Copy the name from the passed header line into the passed
 * name buffer.  Null pad the name buffer.
 */

copyname(linebuf, nbuf)
	char *linebuf, *nbuf;
{
	register char *cp, *cp2;

	for (cp = linebuf + 5, cp2 = nbuf; *cp != ' ' && cp2-nbuf < 8; cp++)
		*cp2++ = *cp;
	while (cp2-nbuf < 8)
		*cp2++ = 0;
}

/*
 * Announce a fatal error and die.
 */

panic(str)
	char *str;
{
	char buf[NL_TEXTMAX];
	strcpy(buf,str);
	prs(MSGSTR(PANIC, "panic: ")); /*MSG*/
	prs(buf);
	prs("\n");
	exit(1);
}

/*
 * Catch stdio errors and report them more nicely.
 */

_error(str)
	char *str;
{
	prs(MSGSTR(STDIOERR, "Stdio Error: ")); /*MSG*/
	prs(str);
	prs("\n");
	abort();
}

/*
 * Print a string on diagnostic output.
 */

prs(str)
	char *str;
{
	register char *s;

	for (s = str; *s; s++)
		;
	write(2, str, s-str);
}

/*
 * Touch the named message by setting its MTOUCH flag.
 * Touched messages have the effect of not being sent
 * back to the system mailbox on exit.
 */

touch(mesg)
{
	register struct message *mp;

	if (mesg < 1 || mesg > msgCount)
		return;
	mp = &message[mesg-1];
	mp->m_flag |= MTOUCH;
	if ((mp->m_flag & MREAD) == 0)
		mp->m_flag |= MREAD|MSTATUS;
}

/*
 * Test to see if the passed file name is a directory.
 * Return true if it is.
 */

isdir(name)
	char name[];
{
	struct stat sbuf;

	if (stat(name, &sbuf) < 0)
		return(0);
	return((sbuf.st_mode & S_IFMT) == S_IFDIR);
}

/*
 * Count the number of arguments in the given string raw list.
 */

argcount(argv)
	char **argv;
{
	register char **ap;

	for (ap = argv; *ap != NULLSTR; ap++)
		;	
	return(ap-argv);
}

/*
 * Given a file address, determine the
 * block number it represents.
 */

blockof(off)
	off_t off;
{
	off_t a;

	a = off >> 9;
	a &= ~0;
	return((int) a);
}

/*
 * Take a file address, and determine
 * its offset in the current block.
 */

offsetof(off)
	off_t off;
{
	off_t a;

	a = off & 0777;
	return((int) a);
}

/*
 * Return the desired header line from the passed message
 * pointer (or NULLSTR if the desired header field is not available).
 */

char *
hfield(field, mp)
	char field[];
	struct message *mp;
{
	register FILE *ibuf;
	char linebuf[LINESIZE];
	register int lc;

#ifdef ASIAN_I18N
	int initpos;
#endif

	ibuf = setinput(mp);
#ifdef ASIAN_I18N
	initpos=ftell(ibuf);
#endif
	if ((lc = mp->m_lines) <= 0)
		return(NULLSTR);
	if (readline(ibuf, linebuf) < 0)
		return(NULLSTR);
	lc--;
	do {
		lc = gethfield(ibuf, linebuf, lc);
		if (lc == -1)
			return(NULLSTR);
		if (ishfield(linebuf, field))
#ifdef ASIAN_I18N
			return(savestr(hcontents(convert_in_header(ibuf, initpos, linebuf, mp-&message[0]+1))));
#else
			return(savestr(hcontents(linebuf)));
#endif
	} while (lc > 0);
	return(NULLSTR);
}

/*
 * Return the next header field found in the given message.
 * Return > 0 if something found, <= 0 elsewise.
 * Must deal with \ continuations & other such fraud.
 */

gethfield(f, linebuf, rem)
	register FILE *f;
	char linebuf[];
	register int rem;
{
	char line2[LINESIZE];
	long loc;
	register char *cp, *cp2;
	register int c;

#ifdef ASIAN_I18N
	int mb;
	wchar_t wc;
#endif

	for (;;) {
		if (rem <= 0)
			return(-1);
		if (readline(f, linebuf) < 0)
			return(-1);
		rem--;
		if (strlen(linebuf) == 0)
			return(-1);
#ifdef ASIAN_I18N
		if (ISSPACE(wc, linebuf, mb))
#else
		if (isspace(linebuf[0]))
#endif
			continue;
		if (linebuf[0] == '>')
			continue;
		cp = index(linebuf, ':');
		if (cp == NULLSTR)
			continue;
		for (cp2 = linebuf; cp2 < cp; cp2++)
#ifdef ASIAN_I18N
			if (isdigit(0xff & *cp2))
#else
			if (isdigit(*cp2))
#endif /* ASIAN_I18N */
				continue;
		
		/*
		 * I guess we got a headline.
		 * Handle wraparounding
		 */
		
		for (;;) {
			if (rem <= 0)
				break;
			loc = ftell(f);
			if (readline(f, line2) < 0)
				break;
			rem--;
#ifdef ASIAN_I18N
			if (!ISSPACE(wc, line2, mb)) {
#else
			if (!isspace(line2[0])) {
#endif
				fseek(f, loc, 0);
				rem++;
				break;
			}
			cp2 = line2;
#ifdef ASIAN_I18N
			for (cp2=line2; *cp2 && ISSPACE(wc, cp2, mb); cp2+=mb)
#else
			for (cp2 = line2; *cp2 != 0 && isspace(*cp2); cp2++)
#endif
				;
			if (strlen(linebuf) + strlen(cp2) >= LINESIZE-2)
				break;
			cp = &linebuf[strlen(linebuf)];
			while (cp > linebuf &&
#ifdef ASIAN_I18N
			    (ISSPACE(wc, cp-1, mb) || cp[-1] == '\\'))
				cp-=mb;
			*cp++ = ' ';
			for (cp2=line2; *cp2 && ISSPACE(wc, cp2, mb); cp2+=mb)
#else /* ASIAN_I18N */
			    (isspace(cp[-1]) || cp[-1] == '\\'))
				cp--;
			*cp++ = ' ';
			for (cp2 = line2; *cp2 != 0 && isspace(*cp2); cp2++)
#endif /* ASIAN_I18N */
				;
			strcpy(cp, cp2);
		}
		if ((c = strlen(linebuf)) > 0) {
			cp = &linebuf[c-1];
#ifdef ASIAN_I18N
			while (cp > linebuf && ISSPACE(wc, cp, mb))
				cp-=mb;
#else
			while (cp > linebuf && isspace(*cp))
				cp--;
#endif
			*++cp = 0;
		}
		return(rem);
	}
	/* NOTREACHED */
}

/*
 * Check whether the passed line is a header line of
 * the desired breed.
 */

ishfield(linebuf, field)
	char linebuf[], field[];
{
	register char *cp;
	register int c;

#ifdef ASIAN_I18N
	int mb;
	wchar_t wc;
#endif

	if ((cp = index(linebuf, ':')) == NULLSTR)
		return(0);
	if (cp == linebuf)
		return(0);
	cp--;
#ifdef ASIAN_I18N
	while (cp > linebuf && ISSPACE(wc, cp, mb))
		cp-=mb;
#else
	while (cp > linebuf && isspace(*cp))
		cp--;
#endif
	c = *++cp;
	*cp = 0;
	if (icequal(linebuf ,field)) {
		*cp = c;
		return(1);
	}
	*cp = c;
	return(0);
}

/*
 * Extract the non label information from the given header field
 * and return it.
 */

char *
hcontents(hfield)
	char hfield[];
{
	register char *cp;

#ifdef ASIAN_I18N
	int mb;
	wchar_t wc;
#endif

	if ((cp = index(hfield, ':')) == NULLSTR)
		return(NULLSTR);
	cp++;
#ifdef ASIAN_I18N
	while (*cp && ISSPACE(wc, cp, mb))
		cp+=mb;
#else /* ASIAN_I8N */
	while (*cp && isspace(*cp))
		cp++;
#endif /* ASIAN_I18N */
	return(cp);
}

/*
 * Compare two strings, ignoring case.
 */

icequal(s1, s2)
	register char *s1, *s2;
{

	while (raise(*s1++) == raise(*s2))
		if (*s2++ == 0)
			return(1);
	return(0);
}

/*
 * Compare two strings for N chars, ignoring case.
 */

icnequal(s1, s2, n)
	register char *s1, *s2;
	register int n;
{

	if (s1 == s2 || n <= 0)
	    return(1);

	while (raise(*s1++) == raise(*s2))
		if (*s2++ == 0 || --n == 0)
			return(1);
	return(0);
}

/*
 * Copy a string, lowercasing it as we go.
 */
istrcpy(dest, src)
	char *dest, *src;
{
	register char *cp, *cp2;

	cp2 = dest;
	cp = src;
	do {
		*cp2++ = little(*cp);
	} while (*cp++ != 0);
}

/*
 * The following code deals with input stacking to do source
 * commands.  All but the current file pointer are saved on
 * the stack.
 */

static	int	ssp = -1;		/* Top of file stack */
struct sstack {
	FILE	*s_file;		/* File we were in. */
	int	s_cond;			/* Saved state of conditionals */
	int	s_loading;		/* Loading .mailrc, etc. */
} sstack[MAX_NOFILE];   /* 00 kaehler  */

/*
 * Pushdown current input file and switch to a new one.
 * Set the global flag "sourcing" so that others will realize
 * that they are no longer reading from a tty (in all probability).
 */

source(name)
	char name[];
{
	register FILE *fi;
	register char *cp;

	if ((cp = expand(name)) == NULLSTR)
		return(1);
	if ((fi = fopen(cp, "r")) == NULL) {
		perror(cp);
		return(1);
	}
	if (ssp >= getdtablesize() - 2) {     /* 00 kaehler  */
		printf(MSGSTR(TOOMUCH, "Too much \"sourcing\" going on.\n")); /*MSG*/
		fclose(fi);
		return(1);
	}
	sstack[++ssp].s_file = input;
	sstack[ssp].s_cond = cond;
	sstack[ssp].s_loading = loading;
	loading = 0;
	cond = CANY;
	input = fi;
	sourcing++;
	return(0);
}

/*
 * Source a file, but do nothing if the file cannot be opened.
 */

source1(name)
	char name[];
{
	register int f;

	if ((f = open(name, 0)) < 0)
		return(0);
	close(f);
	source(name);
}

/*
 * Pop the current input back to the previous level.
 * Update the "sourcing" flag as appropriate.
 */

unstack()
{
	if (ssp < 0) {
		printf(MSGSTR(OVRPOP, "\"Source\" stack over-pop.\n")); /*MSG*/
		sourcing = 0;
		return(1);
	}
	fclose(input);
	if (cond != CANY)
		printf(MSGSTR(UNMTCH, "Unmatched \"if\"\n")); /*MSG*/
	cond = sstack[ssp].s_cond;
	loading = sstack[ssp].s_loading;
	input = sstack[ssp--].s_file;
	if (ssp < 0)
		sourcing = loading;
	return(0);
}

/*
 * Touch the indicated file.
 * This is nifty for the shell.
 * If we have the utime() system call, this is better served
 * by using that, since it will work for empty files.
 * On non-utime systems, we must sleep a second, then read.
 */

alter(name)
	char name[];
{
#ifdef UTIME
	struct stat statb;
	long time();
	time_t time_p[2];
#else
	register int pid, f;
	char w;
#endif UTIME

#ifdef UTIME
	if (stat(name, &statb) < 0)
		return;
	time_p[0] = time((long *) 0) + 1;
	time_p[1] = statb.st_mtime;
	utime(name, time_p);
#else
	sleep(1);
	if ((f = open(name, 0)) < 0)
		return;
	read(f, &w, 1);
	exit(0);
#endif
}

/*
 * Examine the passed line buffer and
 * return true if it is all blanks and tabs.
 */

blankline(linebuf)
	char linebuf[];
{
	register char *cp;

	for (cp = linebuf; *cp; cp++)
		if (*cp != ' ' && *cp != '\t')
			return(0);
	return(1);
}

/*
 * Get recipient's name from this message.  If the message has
 * a bunch of arpanet stuff in it, we may have to skin the name
 * before returning it.
 */
char *
nameto(mp)
	register struct message *mp;
{
	register char *cp, *cp2;

	if ((cp = hfield("to", mp)) != NULLSTR)
	    cp = skin(cp);
	if (charcount(cp, '!') < 2)
		return(cp);
	cp2 = rindex(cp, '!');
	cp2--;
	while (cp2 > cp && *cp2 != '!')
		cp2--;
	if (*cp2 == '!')
		return(cp2 + 1);
	return(cp);
}

/*
 * Get sender's name from this message.  If the message has
 * a bunch of arpanet stuff in it, we may have to skin the name
 * before returning it.
 */
char *
nameof(mp, reptype)
	register struct message *mp;
{
	register char *cp, *cp2;
#ifdef ASIAN_I18N
	int initpos;
	FILE *ibuf;
#endif /* ASIAN_I18N */

	cp = skin(name1(mp, reptype));

#ifdef ASIAN_I18N
	initpos=ftell(ibuf=setinput(mp));
	cp=convert_in_header(ibuf, initpos, cp, mp-&message[0]+1);
#endif /* ASIAN_I18N */

	if (reptype != 0 || charcount(cp, '!') < 2)
		return(cp);
	cp2 = rindex(cp, '!');
	cp2--;
	while (cp2 > cp && *cp2 != '!')
		cp2--;
	if (*cp2 == '!')
		return(cp2 + 1);
	return(cp);
}

/*
 * Skin an arpa net address according to the RFC 822 interpretation
 * of "host-phrase."
 */
char *
skin(name)
	char *name;
{
	register int c;
	register char *cp, *cp2;
	char *bufend;
	int gotlt, lastsp;
	char nbuf[BUFSIZ];
	int nesting;

	if (name == NULLSTR)
		return(NULLSTR);
	if (index(name, '(') == NULLSTR && index(name, '<') == NULLSTR
	&& index(name, ' ') == NULLSTR)
		return(name);
	gotlt = 0;
	lastsp = 0;
	bufend = nbuf;
	for (cp = name, cp2 = bufend; c = *cp++; ) {
		switch (c) {
		case '(':
			/*
			 * Start of a "comment".
			 * Ignore it.
			 */
			nesting = 1;
			while ((c = *cp) != 0) {
				cp++;
				switch (c) {
				case '\\':
					if (*cp == 0)
						goto outcm;
					cp++;
					break;
				case '(':
					nesting++;
					break;

				case ')':
					--nesting;
					break;
				}

				if (nesting <= 0)
					break;
			}
		outcm:
			lastsp = 0;
			break;

		case '"':
			/*
			 * Start of a "quoted-string".
			 * Copy it in its entirety.
			 */
			while ((c = *cp) != 0) {
				cp++;
				switch (c) {
				case '\\':
					if ((c = *cp) == 0)
						goto outqs;
					cp++;
					break;
				case '"':
					goto outqs;
				}
				*cp2++ = c;
			}
		outqs:
			lastsp = 0;
			break;

		case ' ':
			if (cp[0] == 'a' && cp[1] == 't' && cp[2] == ' ')
				cp += 3, *cp2++ = '@';
			else
			if (cp[0] == '@' && cp[1] == ' ')
				cp += 2, *cp2++ = '@';
			else
				lastsp = 1;
			break;

		case '<':
			cp2 = bufend;
			gotlt++;
			lastsp = 0;
			break;

		case '>':
			if (gotlt) {
				gotlt = 0;
				while (*cp != ',' && *cp != 0)
					cp++;
				if (*cp == 0 )
					goto done;
				*cp2++ = ',';
				*cp2++ = ' ';
				bufend = cp2;
				break;
			}

			/* Fall into . . . */

		default:
			if (lastsp) {
				lastsp = 0;
				*cp2++ = ' ';
			}
			*cp2++ = c;
			break;
		}
	}
done:
	*cp2 = 0;

	return(savestr(nbuf));
}

/*
 * Fetch the sender's name from the passed message.
 * Reptype can be
 *	0 -- get sender's name for display purposes
 *	1 -- get sender's name for reply
 *	2 -- get sender's name for Reply
 */

char *
name1(mp, reptype)
	register struct message *mp;
{
	char namebuf[LINESIZE];
	char linebuf[LINESIZE];
	register char *cp, *cp2;
	register FILE *ibuf;
	int first = 1;
#ifdef ASIAN_I18N
	int mb;
#endif

	if ((cp = hfield("from", mp)) != NULLSTR)
		return(cp);
	if (reptype == 0 && (cp = hfield("sender", mp)) != NULLSTR)
		return(cp);
	ibuf = setinput(mp);
	copy("", namebuf);
	if (readline(ibuf, linebuf) <= 0)
		return(savestr(namebuf));
newname:
	for (cp = linebuf; *cp != ' '; cp++)
		;
#ifdef ASIAN_I18N
	while (mb_any(cp, " \t", &mb))
		cp+=mb;
	for (cp2 = &namebuf[strlen(namebuf)]; *cp && !mb_any(cp, " \t", &mb) &&
	    cp2-namebuf < LINESIZE-1; *cp2++ = *cp++)
#else
	while (any(*cp, " \t"))
		cp++;
	for (cp2 = &namebuf[strlen(namebuf)]; *cp && !any(*cp, " \t") &&
	    cp2-namebuf < LINESIZE-1; *cp2++ = *cp++)
#endif
		;
	*cp2 = '\0';
	if (readline(ibuf, linebuf) <= 0)
		return(savestr(namebuf));
	if ((cp = index(linebuf, 'F')) == NULL)
		return(savestr(namebuf));
	if (strncmp(cp, "From", 4) != 0)
		return(savestr(namebuf));
	while ((cp = index(cp, 'r')) != NULL) {
		if (strncmp(cp, "remote", 6) == 0) {
			if ((cp = index(cp, 'f')) == NULL)
				break;
			if (strncmp(cp, "from", 4) != 0)
				break;
			if ((cp = index(cp, ' ')) == NULL)
				break;
			cp++;
			if (first) {
				copy(cp, namebuf);
				first = 0;
			} else
				strcpy(rindex(namebuf, '!')+1, cp);
			strcat(namebuf, "!");
			goto newname;
		}
		cp++;
	}
	return(savestr(namebuf));
}

/*
 * Count the occurances of c in str
 */
charcount(str, c)
	char *str;
{
	register char *cp;
	register int i;

	for (i = 0, cp = str; *cp; cp++)
		if (*cp == c)
			i++;
	return(i);
}

/*
 * Find the rightmost pointer to an instance of the
 * character in the string and return it.
 */
char *
rindex(str, c)
	char str[];
	register int c;
{
	register char *cp, *cp2;

	for (cp = str, cp2 = NULLSTR; *cp; cp++)
		if (c == *cp)
			cp2 = cp;
	return(cp2);
}

/*
 * See if the string is a number.
 */

numeric(str)
	char str[];
{
	register char *cp = str;

	while (*cp)
#ifdef ASIAN_I18N
		if (!isdigit(0xff & *cp++))
#else
		if (!isdigit(*cp++))
#endif /* ASIAN_I18N */
			return(0);
	return(1);
}

/*
 * Are any of the characters in the two strings the same?
 */

anyof(s1, s2)
	register char *s1, *s2;
{
	register int c;
#ifdef ASIAN_I18N
	int mb;

	while (*s1)
		if (mb_any(s1, s2, &mb))
			return(1);
		else s1+=mb;
#else
	while (c = *s1++)
		if (any(c, s2))
			return(1);
#endif
	return(0);
}

/*
 * Determine the leftmost index of the character
 * in the string.
 */

char *
index(str, ch)
	char *str;
{
	register char *cp;
	register int c;

	for (c = ch, cp = str; *cp; cp++)
		if (*cp == c)
			return(cp);
	return(NULLSTR);
}

/*
 * See if the given header field is supposed to be ignored.
 */
isign(field)
	char *field;
{
	char realfld[BUFSIZ];

	/*
	 * Lower-case the string, so that "Status" and "status"
	 * will hash to the same place.
	 */
	istrcpy(realfld, field);

	if (nretained > 0)
		return (!member(realfld, retain));
	else
		return (member(realfld, ignore));
}

member(realfield, table)
	register char *realfield;
	register struct ignore **table;
{
	register struct ignore *igp;

	for (igp = table[hash(realfield)]; igp != 0; igp = igp->i_link)
		if (equal(igp->i_field, realfield))
			return (1);

	return (0);
}
