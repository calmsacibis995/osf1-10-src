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
static char	*sccsid = "@(#)$RCSfile: util.c,v $ $Revision: 4.2.3.4 $ (DEC) $Date: 1992/10/14 12:04:21 $";
#endif 
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
 * Copyright (c) 1983 Eric P. Allman
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
#ifndef lint

#endif 
*/
# include <stdio.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <sysexits.h>
# include <errno.h>
# include "sendmail.h"

bool catPrint = FALSE;		/* xputs: print strings for catenation */

/*
**  STRIPQUOTES -- Strip quotes & quote bits from a string.
**
**	Runs through a string and strips off unquoted quote
**	characters and quote bits.  This is done in place.
**
**	Parameters:
**		s -- the string to strip.
**		qf -- if set, remove actual `` " '' characters
**			as well as the quote bits.
**
**	Returns:
**		none.
**
**	Side Effects:
**		none.
**
**	Called By:
**		deliver
*/

stripquotes(s, qf)
	char *s;
	bool qf;
{
	register char *p;
	register char *q;
	register char c;

	if (s == NULL)
		return;

	for (p = q = s; (c = *p++) != '\0'; )
	{
		if (c != '"' || !qf)
# ifdef EIGHTBIT
				*q++ = Allow8bit ? c : c & 0177;
# else /* EIGHTBIT */
				*q++ = c & 0177;
# endif /* EIGHTBIT */
	}
	*q = '\0';
}
/*
**  QSTRLEN -- give me the string length assuming 0200 bits add a char
**
**	Parameters:
**		s -- the string to measure.
**
**	Reurns:
**		The length of s, including space for backslash escapes.
**
**	Side Effects:
**		none.
*/

qstrlen(s)
	register char *s;
{
	register int l = 0;
	register char c;

	while ((c = *s++) != '\0')
	{
# ifdef EIGHTBIT
		if (!Allow8bit)
# endif /* EIGHTBIT */
			if (bitset(0200, c))
				l++;
		l++;
	}
	return (l);
}
/*
**  CAPITALIZE -- return a copy of a string, properly capitalized.
**
**	Parameters:
**		s -- the string to capitalize.
**
**	Returns:
**		a pointer to a properly capitalized string.
**
**	Side Effects:
**		none.
*/

char *
capitalize(s)
	register char *s;
{
	static char buf[50];
	register char *p;

	p = buf;

	for (;;)
	{
		while (!isalpha(*s) && *s != '\0')
			*p++ = *s++;
		if (*s == '\0')
			break;
		*p++ = toupper(*s);
		s++;
		while (isalpha(*s))
			*p++ = *s++;
	}

	*p = '\0';
	return (buf);
}
/*
**  XALLOC -- Allocate memory and bitch wildly on failure.
**
**	THIS IS A CLUDGE.  This should be made to give a proper
**	error -- but after all, what can we do?
**
**	Parameters:
**		sz -- size of area to allocate.
**
**	Returns:
**		pointer to data region.
**
**	Side Effects:
**		Memory is allocated.
*/

char *
xalloc(sz)
	register int sz;
{
	register char *p;

	p = malloc((unsigned) sz);
	if (p == NULL)
	{
		syserr(MSGSTR(UT_MEM, "Out of memory!!"));
		abort();
		/* exit(EX_UNAVAILABLE); */
	}
	return (p);
}
/*
**  COPYPLIST -- copy list of pointers.
**
**	This routine is the equivalent of newstr for lists of
**	pointers.
**
**	Parameters:
**		list -- list of pointers to copy.
**			Must be NULL terminated.
**		copycont -- if TRUE, copy the contents of the vector
**			(which must be a string) also.
**
**	Returns:
**		a copy of 'list'.
**
**	Side Effects:
**		none.
*/

char **
copyplist(list, copycont)
	char **list;
	bool copycont;
{
	register char **vp;
	register char **newvp;

	for (vp = list; *vp != NULL; vp++)
		continue;

	vp++;

	newvp = (char **) xalloc((int) (vp - list) * sizeof *vp);
	bcopy((char *) list, (char *) newvp, (int) (vp - list) * sizeof *vp);

	if (copycont)
	{
		for (vp = newvp; *vp != NULL; vp++)
			*vp = newstr(*vp);
	}

	return (newvp);
}
/*
**  PRINTAV -- print argument vector.
**
**	Parameters:
**		av -- argument vector.
**
**	Returns:
**		none.
**
**	Side Effects:
**		prints av.
*/

printav(av)
	register char **av;
{
	while (*av != NULL)
	{
		if (tTd(0, 44))
			printf("\n\t%08x=", *av);
		else
			(void) putchar(' ');
		xputs(*av++);
	}
	(void) putchar('\n');
}
/*
**  LOWER -- turn letter into lower case.
**
**	Parameters:
**		c -- character to turn into lower case.
**
**	Returns:
**		c, in lower case.
**
**	Side Effects:
**		none.
*/

char
lower(c)
	register char c;
{
	return(isascii(c) && isupper(c) ? tolower(c) : c);
}
/*
**  XPUTS -- put string doing control escapes.
**
**	Parameters:
**		s -- string to put.
**
**	Returns:
**		none.
**
**	Side Effects:
**		output to stdout
*/

xputs(s)
	register char *s;
{
	register char c;
	register struct metamac *m;

	if (s == MACNULL)
	{
		printf("<macnull>");
		return;
	}
	if (s == NULL)
	{
		printf("<null>");
		return;
	}

	if (s[0] == MATCHREPL && isdigit(s[1]) && s[2] == '\0') {
	  printf("$%c", s[1]);
	  return;
	} else
	  for (m = MetaMacros; m->metaname != '\0'; m++)
	    if (m->metaval == *s) {
	      printf("$%c%s", m->metaname, &s[1]);
	      return;
	    }

	if (!catPrint)
		(void) putchar('"');
	while ((c = *s++) != '\0')
	{
# ifdef EIGHTBIT
		if (Allow8bit) 
		{
			/*
			** just do special things for C0 or C1 control chars
			*/
			if ((c < 0x20) || ((c >= 0x80) && (c <= 0xa0)))
			{
				(void) putchar('^');
				c &= 0177;
				c ^= 0100;
			}
			if (c == 0177)
			{
				(void) putchar('^');
				c = '?';
			}
		} 
		else 
		{
# endif /* EIGHTBIT */
			if (!isascii(c))
			{
				(void) putchar('\\');
				c &= 0177;
			}
			if (c < 040 || c >= 0177)
			{
				(void) putchar('^');
				c ^= 0100;
			}
# ifdef EIGHTBIT
		}
# endif /* EIGHTBIT */
		(void) putchar(c);
	}
	if (!catPrint)
		(void) putchar('"');
	(void) fflush(stdout);
}
/*
**  MAKELOWER -- Translate a line into lower case
**
**	Parameters:
**		p -- the string to translate.  If NULL, return is
**			immediate.
**
**	Returns:
**		none.
**
**	Side Effects:
**		String pointed to by p is translated to lower case.
**
**	Called By:
**		parse
*/

makelower(p)
	register char *p;
{
	register char c;
	register bool quoted_string = FALSE;

	if (p == NULL)
		return;
	for (; (c = *p) != '\0'; p++)
		if (c == '"')
			quoted_string = !quoted_string;
		else if (!quoted_string && isascii(c) && isupper(c))
			*p = tolower(c);
}
/*
**  BUILDFNAME -- build full name from gecos style entry.
**
**	This routine interprets the strange entry that would appear
**	in the GECOS field of the password file.
**
**	Parameters:
**		p -- name to build.
**		login -- the login name of this user (for &).
**		buf -- place to put the result.
**
**	Returns:
**		none.
**
**	Side Effects:
**		none.
*/

buildfname(p, login, buf)
	register char *p;
	char *login;
	char *buf;
{
	register char *bp = buf;

	if (*p == '*')
		p++;
	while (*p != '\0' && *p != ',' && *p != ';' && *p != '%')
	{
		if (*p == '&')
		{
			(void) strcpy(bp, login);
			*bp = toupper(*bp);
			while (*bp != '\0')
				bp++;
			p++;
		}
		else
			*bp++ = *p++;
	}
	*bp = '\0';
}
/*
**  SAFEFILE -- return true if a file exists and is safe for a user.
**
**	Parameters:
**		fn -- filename to check.
**		uid -- uid to compare against.
**		mode -- mode bits that must match.
**
**	Returns:
**		TRUE if fn exists, is owned by uid, and matches mode.
**		FALSE otherwise.
**
**	Side Effects:
**		none.
*/

bool
safefile(fn, uid, mode)
	char *fn;
	int uid;
	int mode;
{
	struct stat stbuf;

	if (stat(fn, &stbuf) >= 0 && stbuf.st_uid == uid &&
	    (stbuf.st_mode & mode) == mode)
		return (TRUE);
	errno = 0;
	return (FALSE);
}
/*
**  FIXCRLF -- fix <CR><LF> in line.
**
**	Looks for the <CR><LF> combination and turns it into the
**	UNIX canonical <NL> character.  It only takes one line,
**	i.e., it is assumed that the first <NL> found is the end
**	of the line.
**
**	Parameters:
**		line -- the line to fix.
**		stripnl -- if true, strip the newline also.
**
**	Returns:
**		none.
**
**	Side Effects:
**		line is changed in place.
*/

fixcrlf(line, stripnl)
	char *line;
	bool stripnl;
{
	register char *p;

	p = index(line, '\n');
	if (p == NULL)
		return;
	if (p > line && p[-1] == '\r')
		p--;
	if (!stripnl)
		*p++ = '\n';
	*p = '\0';
}
/*
**  DFOPEN -- determined file open
**
**	This routine has the semantics of fopen, except that it will
**	keep trying a few times to make this happen.  The idea is that
**	on very loaded systems, we may run out of resources (inodes,
**	whatever), so this tries to get around it.
*/

FILE *
dfopen(filename, mode)
	char *filename;
	char *mode;
{
	register int tries;
	register FILE *fp;

	for (tries = 0; tries < 10; tries++)
	{
		sleep((unsigned) (10 * tries));
		errno = 0;
		fp = fopen(filename, mode);
		if (fp != NULL)
			break;
		if (errno != ENFILE && errno != EINTR)
			break;
	}
	errno = 0;
	return (fp);
}
/*
**  PUTLINE -- put a line like fputs obeying SMTP conventions
**
**	This routine always guarantees outputing a newline (or CRLF,
**	as appropriate) at the end of the string.
**
**	Parameters:
**		l -- line to put.
**		fp -- file to put it onto.
**		m -- the mailer used to control output.
**
**	Returns:
**		none
**
**	Side Effects:
**		output of l to fp.
*/

# define SMTPLINELIM	990	/* maximum line length */

putline(l, fp, m)
	register char *l;
	FILE *fp;
	MAILER *m;
{
	register char *p;
	char svchar;
	int	limitsflag,	xdotflag;

	limitsflag = bitnset(M_LIMITS, m->m_flags);
	/* strip out 0200 bits -- these can look like TELNET protocol */
# ifdef EIGHTBIT
	if (Allow8bit) 
	{
		if (!Allow8bitAlways && !bitnset(M_ALLOW8BIT, m->m_flags)) 
		{ 
			p = l;
			while ((*p++ &= ~0200) != 0)
				continue;
		}
	} 
	else 
	{
# endif /* EIGHTBIT */
		if (limitsflag)
		{
			p = l;
			while ((*p++ &= ~0200) != 0)
				continue;
		}
# ifdef EIGHTBIT
	}
# endif /* EIGHTBIT */
	xdotflag = bitnset(M_XDOT, m->m_flags);
	do
	{
		/* find the end of the line */
		p = index(l, '\n');
		if (p == NULL)
			p = &l[strlen(l)];

		/* check for line overflow */
		while (((p - l) > SMTPLINELIM) && limitsflag)
		{
			register char *q = &l[SMTPLINELIM - 1];

			if ((*l == '.') && xdotflag)
				(void) putc('.', fp);
			fprintf(fp, "%.*s!%s", SMTPLINELIM-1, l, m->m_eol);
			l = q;
		}

		/* output last part */
		if ((*l == '.') && xdotflag)
			(void) putc('.', fp);
		fprintf(fp, "%.*s%s", p-l, l, m->m_eol);
		l = p;
		if (*l == '\n')
			l++;
	} while (*l != '\0');
}
/*
**  XUNLINK -- unlink a file, doing logging as appropriate.
**
**	Parameters:
**		f -- name of file to unlink.
**
**	Returns:
**		none.
**
**	Side Effects:
**		f is unlinked.
*/

xunlink(f)
	char *f;
{
	register int i;

# ifdef LOG
	if (LogLevel > 20)
		syslog(LOG_DEBUG, "%s: unlink %s\n", CurEnv->e_id, f);
# endif /* LOG */

	i = unlink(f);
# ifdef LOG
	if (i < 0 && LogLevel > 21)
		syslog(LOG_DEBUG, "%s: unlink-fail %d", f, errno);
# endif /* LOG */
}
/*
**  SFGETS -- "safe" fgets -- times out and ignores random interrupts.
**
**	Parameters:
**		buf -- place to put the input line.
**		siz -- size of buf.
**		fp -- file to read from.
**
**	Returns:
**		NULL on error (including timeout).  This will also leave
**			buf containing a null string.
**		buf otherwise.
**
**	Side Effects:
**		none.
*/

static jmp_buf	CtxReadTimeout;

char *
sfgets(buf, siz, fp)
	char *buf;
	int siz;
	FILE *fp;
{
	register EVENT *ev = NULL;
	register char *p;
	extern readtimeout();

	/* set the timeout */
	if (ReadTimeout != 0)
	{
		if (setjmp(CtxReadTimeout) != 0)
		{
# ifdef LOG
			syslog(LOG_NOTICE,
			    MSGSTR(UT_TIME, "timeout waiting for input from %s\n"),
			    RealHostName ? RealHostName : "local");
# endif
			errno = 0;
			usrerr("451 timeout waiting for input");
			buf[0] = '\0';
			return (NULL);
		}
		ev = setevent((time_t) ReadTimeout, readtimeout, 0);
	}

	/* try to read */
	p = NULL;
	while (p == NULL && !feof(fp) && !ferror(fp))
	{
		errno = 0;
		p = fgets(buf, siz, fp);
		if (errno == EINTR)
			clearerr(fp);
	}

	/* clear the event if it has not sprung */
	clrevent(ev);

	/* clean up the books and exit */
	LineNumber++;
	if (p == NULL)
	{
		buf[0] = '\0';
		return (NULL);
	}
# ifdef EIGHTBIT
	if (!Allow8bit) 
# endif /* EIGHTBIT */
		for (p = buf; *p != '\0'; p++)
			*p &= ~0200;
	return (buf);
}

static
readtimeout()
{
	longjmp(CtxReadTimeout, 1);
}
/*
**  FGETFOLDED -- like fgets, but know about folded lines.
**
**	Parameters:
**		buf -- place to put result.
**		n -- bytes available.
**		f -- file to read from.
**
**	Returns:
**		buf on success, NULL on error or EOF.
**
**	Side Effects:
**		buf gets lines from f, with continuation lines (lines
**		with leading white space) appended.  CRLF's are mapped
**		into single newlines.  Any trailing NL is stripped.
*/

char *
fgetfolded(buf, n, f)
	char *buf;
	register int n;
	FILE *f;
{
	register char *p = buf;
	register int i;

	n--;
	while ((i = getc(f)) != EOF)
	{
		if (i == '\r')
		{
			i = getc(f);
			if (i != '\n')
			{
				if (i != EOF)
					(void) ungetc(i, f);
				i = '\r';
			}
		}
		if (--n > 0)
			*p++ = i;
		if (i == '\n')
		{
			LineNumber++;
			i = getc(f);
			if (i != EOF)
				(void) ungetc(i, f);
			if (i != ' ' && i != '\t')
			{
				*--p = '\0';
				return (buf);
			}
		}
	}
	return (NULL);
}
/*
**  CURTIME -- return current time.
**
**	Parameters:
**		none.
**
**	Returns:
**		the current time.
**
**	Side Effects:
**		none.
*/

time_t
curtime()
{
	auto time_t t;

	(void) time(&t);
	return (t);
}
/*
**  ATOBOOL -- convert a string representation to boolean.
**
**	Defaults to "TRUE"
**
**	Parameters:
**		s -- string to convert.  Takes "tTyY" as true,
**			others as false.
**
**	Returns:
**		A boolean representation of the string.
**
**	Side Effects:
**		none.
*/

bool
atobool(s)
	register char *s;
{
	if (*s == '\0' || index("tTyY", *s) != NULL)
		return (TRUE);
	return (FALSE);
}
/*
**  ATOOCT -- convert a string representation to octal.
**
**	Parameters:
**		s -- string to convert.
**
**	Returns:
**		An integer representing the string interpreted as an
**		octal number.
**
**	Side Effects:
**		none.
*/

atooct(s)
	register char *s;
{
	register int i = 0;

	while (*s >= '0' && *s <= '7')
		i = (i << 3) | (*s++ - '0');
	return (i);
}
/*
**  WAITFOR -- wait for a particular process id.
**
**	Parameters:
**		pid -- process id to wait for.
**
**	Returns:
**		status of pid.
**		-1 if pid never shows up.
**
**	Side Effects:
**		none.
*/

waitfor(pid)
	int pid;
{
	auto int st;
	int i;

	do
	{
		errno = 0;
		i = wait(&st);
	} while ((i >= 0 || errno == EINTR) && i != pid);
	if (i < 0)
		st = -1;
	return (st);
}
/*
**  BITINTERSECT -- tell if two bitmaps intersect
**
**	Parameters:
**		a, b -- the bitmaps in question
**
**	Returns:
**		TRUE if they have a non-null intersection
**		FALSE otherwise
**
**	Side Effects:
**		none.
*/

bool
bitintersect(a, b)
	BITMAP a;
	BITMAP b;
{
	int i;

	for (i = BITMAPBYTES / sizeof (int); --i >= 0; )
		if ((a[i] & b[i]) != 0)
			return (TRUE);
	return (FALSE);
}
/*
**  BITZEROP -- tell if a bitmap is all zero
**
**	Parameters:
**		map -- the bit map to check
**
**	Returns:
**		TRUE if map is all zero.
**		FALSE if there are any bits set in map.
**
**	Side Effects:
**		none.
*/

bool
bitzerop(map)
	BITMAP map;
{
	int i;

	for (i = BITMAPBYTES / sizeof (int); --i >= 0; )
		if (map[i] != 0)
			return (FALSE);
	return (TRUE);
}
/*
**	PRINTCAV -- Print concatenated argument vector
**
**	Parameters:
**		av -- argument vector.
**
**	Returns:
**		none.
**
**	Side Effects:
**		prints av.
*/

printcav(av)
     register char **av;
{
  bool oldCatPrint = catPrint;

  catPrint = TRUE;
  printav(av);
  catPrint = oldCatPrint;
}
