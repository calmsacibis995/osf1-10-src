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
static char rcsid[] = "@(#)$RCSfile: bsd_termcap.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 20:15:01 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint

#endif not lint

#define	BUFSIZ		1024
#define MAXHOP		32	/* max number of tc= indirections */
#define	PBUFSIZ		512	/* max length of filename path */
#define	PVECSIZ		32	/* max number of names in path */
#define DEF_PATH	".termcap /etc/termcap"

/*
 * These defines are so BSD programs (which will include curses.h)
 * will see these routines instead of the SYSV termlib routines
 */
#define tfindent        _bsd_tfindent
#define tgetent         _bsd_tgetent
#define tgetflag        _bsd_tgetflag
#define tgetnum         _bsd_tgetnum
#define tgetstr         _bsd_tgetstr
#define tgetwinsize     _bsd_tgetwinsize
#define tnamatch        _bsd_tnamatch
#define tnchktc         _bsd_tnchktc

#include <ctype.h>
#include <sys/ioctl.h>		/* for TIOCGWINSZ */
/*
 * termcap - routines for dealing with the terminal capability data base
 *
 * BUG:		Should use a "last" pointer in tbuf, so that searching
 *		for capabilities alphabetically would not be a n**2/2
 *		process when large numbers of capabilities are given.
 * Note:	If we add a last pointer now we will screw up the
 *		tc capability. We really should compile termcap.
 *
 * Essentially all the work here is scanning and decoding escapes
 * in string capabilities.  We don't use stdio because the editor
 * doesn't, and because living w/o it is not hard.
 */

static	char *tbuf;
static	int hopcount;	/* detect infinite loops in termcap, init 0 */
static	char pathbuf[PBUFSIZ];		/* holds raw path of filenames */
static	char *pathvec[PVECSIZ];		/* to point to names in pathbuf */
static	char **pvec;			/* holds usable tail of path vector */
char	*tskip();
char	*tgetstr();
char	*tdecode();
char	*getenv();

/*
 * Get an entry for terminal name in buffer bp from the termcap file.
 */
tgetent(bp, name)
	char *bp, *name;
{
	register char *p;
	register char *cp;
	register int c;
	char *term, *home, *termpath;
	char **fname = pathvec;

	pvec = pathvec;
	tbuf = bp;
	p = pathbuf;
#ifndef V6
	cp = getenv("TERMCAP");
	/*
	 * TERMCAP can have one of two things in it. It can be the
	 * name of a file to use instead of /etc/termcap. In this
	 * case it better start with a "/". Or it can be an entry to
	 * use so we don't have to read the file. In this case it
	 * has to already have the newlines crunched out.  If TERMCAP
	 * does not hold a file name then a path of names is searched
	 * instead.  The path is found in the TERMPATH variable, or
	 * becomes "$HOME/.termcap /etc/termcap" if no TERMPATH exists.
	 */
	if (!cp || *cp != '/') {	/* no TERMCAP or it holds an entry */
		if (termpath = getenv("TERMPATH"))
			strncpy(pathbuf, termpath, PBUFSIZ);
		else {
			if (home = getenv("HOME")) {	/* set up default */
				p += strlen(home);	/* path, looking in */
				strcpy(pathbuf, home);	/* $HOME first */
				*p++ = '/';
			}	/* if no $HOME look in current directory */
			strncpy(p, DEF_PATH, PBUFSIZ - (p - pathbuf));
		}
	}
	else				/* user-defined name in TERMCAP */
		strncpy(pathbuf, cp, PBUFSIZ);	/* still can be tokenized */
#else
	strncpy(pathbuf, "/etc/termcap", PBUFSIZ);
#endif
	*fname++ = pathbuf;	/* tokenize path into vector of names */
	while (*++p)
		if (*p == ' ' || *p == ':') {
			*p = '\0';
			while (*++p)
				if (*p != ' ' && *p != ':')
					break;
			if (*p == '\0')
				break;
			*fname++ = p;
			if (fname >= pathvec + PVECSIZ) {
				fname--;
				break;
			}
		}
	*fname = (char *) 0;			/* mark end of vector */
	if (cp && *cp && *cp != '/') {
		tbuf = cp;
		c = tnamatch(name);
		tbuf = bp;
		if (c) {
			strcpy(bp,cp);
			return (tnchktc());
		}
	}
	return (tfindent(bp, name));	/* find terminal entry in path */
}

/*
 * tfindent - reads through the list of files in pathvec as if they were one
 * continuous file searching for terminal entries along the way.  It will
 * participate in indirect recursion if the call to tnchktc() finds a tc=
 * field, which is only searched for in the current file and files ocurring
 * after it in pathvec.  The usable part of this vector is kept in the global
 * variable pvec.  Terminal entries may not be broken across files.  Parse is
 * very rudimentary; we just notice escaped newlines.
 */
tfindent(bp, name)
	char *bp, *name;
{
	register char *cp;
	register int c;
	register int i, cnt;
	char ibuf[BUFSIZ];
	int opencnt = 0;
	int tf;

	tbuf = bp;
nextfile:
	i = cnt = 0;
	while (*pvec && (tf = open(*pvec, 0)) < 0)
		pvec++;
	if (!*pvec)
		return (opencnt ? 0 : -1);
	opencnt++;
	for (;;) {
		cp = bp;
		for (;;) {
			if (i == cnt) {
				cnt = read(tf, ibuf, BUFSIZ);
				if (cnt <= 0) {
					close(tf);
					pvec++;
					goto nextfile;
				}
				i = 0;
			}
			c = ibuf[i++];
			if (c == '\n') {
				if (cp > bp && cp[-1] == '\\'){
					cp--;
					continue;
				}
				break;
			}
			if (cp >= bp+BUFSIZ) {
				write(2,"Termcap entry too long\n", 23);
				break;
			} else
				*cp++ = c;
		}
		*cp = 0;

		/*
		 * The real work for the match.
		 */
		if (tnamatch(name)) {
			close(tf);
			return(tnchktc());
		}
	}
}

/*
 * tnchktc: check the last entry, see if it's tc=xxx. If so,
 * recursively find xxx and append that entry (minus the names)
 * to take the place of the tc=xxx entry. This allows termcap
 * entries to say "like an HP2621 but doesn't turn on the labels".
 * Note that this works because of the left to right scan.
 */
tnchktc()
{
	register char *p, *q;
	char tcname[16];	/* name of similar terminal */
	char tcbuf[BUFSIZ];
	char *holdtbuf = tbuf;
	int l;

	p = tbuf + strlen(tbuf) - 2;	/* before the last colon */
	while (*--p != ':')
		if (p<tbuf) {
			write(2, "Bad termcap entry\n", 18);
			return (0);
		}
	p++;
	/* p now points to beginning of last field */
	if (p[0] != 't' || p[1] != 'c')
		return(1);
	strcpy(tcname,p+3);
	q = tcname;
	while (*q && *q != ':')
		q++;
	*q = 0;
	if (++hopcount > MAXHOP) {
		write(2, "Infinite tc= loop\n", 18);
		return (0);
	}
	if (tfindent(tcbuf, tcname) != 1) {
		hopcount = 0;		/* unwind recursion */
		return(0);
	}
	for (q=tcbuf; *q != ':'; q++)
		;
	l = p - holdtbuf + strlen(q);
	if (l > BUFSIZ) {
		write(2, "Termcap entry too long\n", 23);
		q[BUFSIZ - (p-tbuf)] = 0;
	}
	strcpy(p, q+1);
	tbuf = holdtbuf;
	hopcount = 0;			/* unwind recursion */
	return(1);
}

/*
 * Tnamatch deals with name matching.  The first field of the termcap
 * entry is a sequence of names separated by |'s, so we compare
 * against each such name.  The normal : terminator after the last
 * name (before the first field) stops us.
 */
tnamatch(np)
	char *np;
{
	register char *Np, *Bp;

	Bp = tbuf;
	if (*Bp == '#')
		return(0);
	for (;;) {
		for (Np = np; *Np && *Bp == *Np; Bp++, Np++)
			continue;
		if (*Np == 0 && (*Bp == '|' || *Bp == ':' || *Bp == 0))
			return (1);
		while (*Bp && *Bp != ':' && *Bp != '|')
			Bp++;
		if (*Bp == 0 || *Bp == ':')
			return (0);
		Bp++;
	}
}

/*
 * Skip to the next field.  Notice that this is very dumb, not
 * knowing about \: escapes or any such.  If necessary, :'s can be put
 * into the termcap file in octal.
 */
static char *
tskip(bp)
	register char *bp;
{

	while (*bp && *bp != ':')
		bp++;
	if (*bp == ':')
		bp++;
	return (bp);
}

/*
 * Return the (numeric) option id.
 * Numeric options look like
 *	li#80
 * i.e. the option string is separated from the numeric value by
 * a # character.  If the option is not found we return -1.
 * Note that we handle octal numbers beginning with 0.
 */
tgetnum(id)
	char *id;
{
	register int i, base;
	register char *bp = tbuf;

	for (;;) {
		bp = tskip(bp);
		if (*bp == 0)
			return (-1);
		if (*bp++ != id[0] || *bp == 0 || *bp++ != id[1])
			continue;
		if (*bp == '@')
			return(-1);
		if (*bp != '#')
			continue;
#ifdef	TIOCGWINSZ
		/*
		 *	If we're running a window manager, and the user
		 *	asks for the lines or columns, get it from the
		 *	tty.
		 */
		if ((i = tgetwinsize(id)) != -1)
			return (i);
#endif
		bp++;
		base = 10;
		if (*bp == '0')
			base = 8;
		i = 0;
		while (isdigit(*bp))
			i *= base, i += *bp++ - '0';
		return (i);
	}
}

/*
 * Handle a flag option.
 * Flag options are given "naked", i.e. followed by a : or the end
 * of the buffer.  Return 1 if we find the option, or 0 if it is
 * not given.
 */
tgetflag(id)
	char *id;
{
	register char *bp = tbuf;

	for (;;) {
		bp = tskip(bp);
		if (!*bp)
			return (0);
		if (*bp++ == id[0] && *bp != 0 && *bp++ == id[1]) {
			if (!*bp || *bp == ':')
				return (1);
			else if (*bp == '@')
				return(0);
		}
	}
}

/*
 * Get a string valued option.
 * These are given as
 *	cl=^Z
 * Much decoding is done on the strings, and the strings are
 * placed in area, which is a ref parameter which is updated.
 * No checking on area overflow.
 */
char *
tgetstr(id, area)
	char *id, **area;
{
	register char *bp = tbuf;

	for (;;) {
		bp = tskip(bp);
		if (!*bp)
			return (0);
		if (*bp++ != id[0] || *bp == 0 || *bp++ != id[1])
			continue;
		if (*bp == '@')
			return(0);
		if (*bp != '=')
			continue;
		bp++;
		return (tdecode(bp, area));
	}
}

/*
 * Tdecode does the grung work to decode the
 * string capability escapes.
 */
static char *
tdecode(str, area)
	register char *str;
	char **area;
{
	register char *cp;
	register int c;
	register char *dp;
	int i;

	cp = *area;
	while ((c = *str++) && c != ':') {
		switch (c) {

		case '^':
			c = *str++ & 037;
			break;

		case '\\':
			dp = "E\033^^\\\\::n\nr\rt\tb\bf\f";
			c = *str++;
nextc:
			if (*dp++ == c) {
				c = *dp++;
				break;
			}
			dp++;
			if (*dp)
				goto nextc;
			if (isdigit(c)) {
				c -= '0', i = 2;
				do
					c <<= 3, c |= *str++ - '0';
				while (--i && isdigit(*str));
			}
			break;
		}
		*cp++ = c;
	}
	*cp++ = 0;
	str = *area;
	*area = cp;
	return (str);
}
#ifdef	TIOCGWINSZ
/*
 *	If we're running a window manager, and the user asks for the
 *	lines or columns, get it from the tty.
 *
 *	bp points to capability name
 *	returns -1 if invalid, otherwise lines/cols
 */
int	tgetwinsize(id)
	register char *id;
{
	register int lines, i, fd;
	struct winsize winsize;
	static int cols, rows = 0;

	if (id[0] == 'l' && id[1] == 'i')
		lines = 1;	/* lines */
	else if (id[0] == 'c' && id[1] == 'o')
		lines = 0;	/* columns */
	else
		return (-1);	/* neither */

	if (rows < 0)
		return (-1);	/* failed before */

	if (rows == 0) {
		for (i = 0; i < 3; i++)
			if (isatty(i))
				break;
		if (i == 3)
			fd = i = open("/dev/tty", 0);
		else
			fd = i;
		if (i >= 0) {
			i = ioctl(fd, TIOCGWINSZ, (char *)&winsize);
			if (fd >= 3)
				close(fd);
			if (i >= 0) {
				rows = winsize.ws_row;
				cols = winsize.ws_col;
				if (rows <= 0 || rows > 999 ||
				    cols <= 0 || cols > 999)
					i = -1;
			}
		}
		if (i < 0) {
			rows = -1;
			return (-1);
		}
	}

	if (lines)
		return (rows);
	return (cols);
}
#endif	/* TIOCGWINSZ */
