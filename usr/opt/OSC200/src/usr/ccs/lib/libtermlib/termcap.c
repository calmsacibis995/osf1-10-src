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
static char	*sccsid = "@(#)$RCSfile: termcap.c,v $ $Revision: 4.3.9.3 $ (DEC) $Date: 1993/09/23 18:30:34 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.2
 */
/*
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: termcap.c,v $ $Revision: 4.3.9.3 $ (OSF) $Date: 1993/09/23 18:30:34 $";
#endif
 */
/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */
 /*** "termcap.c	1.5  com/lib/termcap,3.1,8943 9/11/89 08:59:34"; ***/
/*
 * COMPONENT_NAME: (LIBTERMC) Termcap Library 
 *
 * FUNCTIONS: tdecode, tgetent, tgetflag, tgetnum, tgetstr, tnamatch, tnchktc 
 *	      tskip
 *
 * ORIGINS: 26 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
 
/* Modification History
 *
 * 001 gws
 *	TERMCAPSIZ define added, to replace BUFSIZ, to define maximum
 *	  termcap entry size as 1024 bytes (including null terminator),
 *	  so termcap entry size is independent of BUFSIZ changes.  BUFSIZ is
 *	  defined in <stdio.h> and is subject to change, but applications
 *	  expect the maximum termcap entry to be 1024 bytes, as documented on
 *	  BSD systems), so they hardcoded tgetenf() buffer size to be 1024
 *	  bytes.
 *	 
 *	tnchktc: return(0) if termcap entry is too large, instead of
 *		q[BUFSIZ - (p-tbuf)] = 0;
 *	  which tried (incorrectly) to truncate it.
 *
 * 002 gws
 *	replaced all "write(2,MSGSTR(...,"..."), 80);" calls with fputs()
 *	calls of the form "fputs(MSGSTR(...,"..."),stderr);" because:
 *	  a. write() require "nbytes" equal strlen(message); , else
 *	     garbage data is written to the terminal
 *	  b. if message catalog is non-English, writes() would fail
 *	     if "nbytes" was a constant instead of strlen(message)
 *	  c. write() isn't thread-safe; fputs() is thread-safe
 *	  d. fputs() avoids needing to do "strlen(message)"
 *		
 * 003 gws
 *	"catd = " inserted in front of all catopen() calls so LANG and NLSPATH
 *	  are no longer ignored
 *
 */

#include <stdio.h> 
#include <ctype.h>

#define E_TERMCAP	"/etc/termcap"	/* where the termcap file lives */
#define MAXHOP	32	/* max number of tc= indirections */

#include "termc_msg.h"
#define MSGSTR(Num, Str) catgets(catd, MS_TERMC, Num, Str)

#define TERMCAPSIZ	1024	/* maximm size of termcap entry, including a
				 * null terminator.
				 */				/* 001 */

/*
 * termcap - routines for dealing with the terminal capability data base
 *
 * NOTE:	Should use a "last" pointer in tbuf, so that searching
 *		for capabilities alphabetically would not be a n**2/2
 *		process when large numbers of capabilities are given.
 *      	If we add a last pointer now we will screw up the
 *		tc capability. We really should compile termcap.
 *
 * Essentially all the work here is scanning and decoding escapes
 * in string capabilities.  We don't use stdio because the editor
 * doesn't, and because living w/o it is not hard.
 */

static	char *tbuf;
static	int hopcount;	/* detect infinite loops in termcap, init 0 */
static char	*tskip();
static char	*tdecode();
char	*tgetstr();
char	*getenv();

/*
 * Get an entry for terminal name in buffer bp,
 * from the termcap file.  Parse is very rudimentary;
 * we just notice escaped newlines.
 */
tgetent(bp, name)
char *bp;  /*  character buffer of size 1024 holds the terminals capabilities */
char *name;                           /*  name of terminal (ie terminal type) */
{
	char *cp;
	int c;
	int i = 0, cnt = 0;
	char ibuf[BUFSIZ];
	char *cp2;
	int tf;

	tbuf = bp;
	tf = 0;
	cp = getenv("TERMCAP");
	/*
	 * TERMCAP can have one of two things in it. It can be the
	 * name of a file to use instead of /etc/termcap. In this
	 * case it better start with a "/". Or it can be an entry to
	 * use so we don't have to read the file. In this case it
	 * has to already have the newlines crunched out.
	 */
	if (cp && *cp) {
		if (*cp!='/') {
			cp2 = getenv("TERM");
			if (cp2==(char *) 0 || strcmp(name,cp2)==0) {
				strcpy(bp,cp);
				return(tnchktc());
			} else {
				tf = open(E_TERMCAP, 0);
			}
		} else
			tf = open(cp, 0);
	}
	if (tf==0)
		tf = open(E_TERMCAP, 0);
	if (tf < 0)
		return (-1);
	for (;;) {
		cp = bp;
		for (;;) {
			if (i == cnt) {
				cnt = read(tf, ibuf, BUFSIZ);
				if (cnt <= 0) {
					close(tf);
					return (0);
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
			/* 004 begin change area */
			if (cp >= (bp+TERMCAPSIZ-1)) {	/* 001 */
				nl_catd catd;

				bp[TERMCAPSIZ-1] = 0;
				if (tnamatch(name)) {
				    catd = catopen(MF_TERMC, NL_CAT_LOCALE);  /* 003 */
								/* 002 */
				    fputs(MSGSTR(MS_TOOLONG,"Termcap entry too long\n"),stderr);
				    catclose(catd);
				    close(tf);
				    return(0);
				} else {  /* just passing over long one */
				    cp = bp;        /* to start of buffer */
				    *cp++ = '#';    /* force name check fail */
				}
				/*  004 end change area */
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
	char *p, *q;
	char tcname[16];	/* name of similar terminal */
	char tcbuf[TERMCAPSIZ];					/* 001 */
	char *holdtbuf = tbuf;
	int l;

	p = tbuf + strlen(tbuf) - 2;	/* before the last colon */
	while (*--p != ':')
		if (p<tbuf) {
			nl_catd catd;
			catd = catopen(MF_TERMC, NL_CAT_LOCALE);		/* 003 */
								/* 002 */
			fputs(MSGSTR(MS_BAD,"Bad termcap entry\n"),stderr);
			catclose(catd);
			return (0);
		}
	p++;
	/* p now points to beginning of last field */
	if (p[0] != 't' || p[1] != 'c')
		return(1);
	strcpy(tcname,p+3);
	q = tcname;
	while (q && *q != ':')
		q++;
	*q = 0;
	if (++hopcount > MAXHOP) {
		nl_catd catd;
		catd = catopen(MF_TERMC, NL_CAT_LOCALE);			/* 003 */
								/* 002 */
		fputs(MSGSTR(MS_INFINITE,"Infinite tc= loop\n"),stderr);
		catclose(catd);
		return (0);
	}
	if (tgetent(tcbuf, tcname) != 1)
		return(0);
	for (q=tcbuf; *q != ':'; q++)
		;
	l = p - holdtbuf + strlen(q);
	if (l >= TERMCAPSIZ) {				/* 001 && 004 */
		nl_catd catd;
		catd = catopen(MF_TERMC, NL_CAT_LOCALE);			/* 003 */
								/* 002 */
		fputs(MSGSTR(MS_TOOLONG,"Termcap entry too long\n"),stderr);
		catclose(catd);
		return(0);					/* 001 */
	}
	strcpy(p, q+1);
	tbuf = holdtbuf;
	return(1);
}

/*
 * Tnamatch deals with name matching.  The first field of the termcap
 * entry is a sequence of names separated by |'s, so we compare
 * against each such name.  The normal : terminator after the last
 * name (before the first field) stops us.
 */
tnamatch(np)
	char *np;                /* next buffer */
{
	char *Np, *Bp;

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
 * Skip to the next field.  
 * Note:  Termcap does not know about "\:", if necessary, :'s can be put
 * into the termcap file in octal.
 */
static char *
tskip(bp)
char *bp;   /* terminal capabilites buffer */
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
char *id;              /* two character capability id */
{
	int i, base;
	char *bp = tbuf;

	for (;;) {
		bp = tskip(bp);
		if (*bp == 0)         /* capability not found (end of buffer) */
			return (-1);
		if (*bp++ != id[0] || *bp == 0 || *bp++ != id[1])
			continue;
		if (*bp == '@') /* capability not supported for this termianl */
			return(-1);
		if (*bp != '#')                      /* numeric value is next */
			continue;
		bp++;
		base = 10;
		if (*bp == '0')
			base = 8;
		i = 0;
		while (isdigit(*bp))                     /* convert to number */
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
char *id;              /* two character capability id */      
{
	char *bp = tbuf;

	for (;;) {
		bp = tskip(bp);
		if (!*bp)                         /* end of capability buffer */
			return (0);
		if (*bp++ == id[0] && *bp != 0 && *bp++ == id[1]) {
			if (!*bp || *bp == ':')         /* boolean capability */
				return (1);
			else if (*bp == '@')      /* capability not supported */
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
char *id;               /* two character capability id */      
char **area;            /* pointer into bp (capability buffer) */
{
	char *bp = tbuf;

	for (;;) {
		bp = tskip(bp);
		if (!*bp)             /* if end of capability buffer */
			return (0);
		if (*bp++ != id[0] || *bp == 0 || *bp++ != id[1])
			continue;
		if (*bp == '@') /* capability not supported for this termianl */
			return(0);
		if (*bp != '=')                          /* string value next */
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
	char *str;         /* capbablitity buffer */
	char **area;       /* pointer into buffer */
{
	char *cp;
	int c;
	char *dp;
	int i;

	cp = *area;
	while ((c = *str++) && c != ':') {
		switch (c) {

		case '^':
			c = *str++ & 037;
			break;

		case '\\':    /* list of accepted escape sequences */
			dp = "E\033^^\\\\::n\nr\rt\tb\bf\f";
			c = *str++;       /* decode escape sequences */
nextc:
			if (*dp++ == c) {
				c = *dp++;
				break;
			}
			dp++;
			if (*dp)
				goto nextc;
			if (isdigit(c)) {        
				c -= '0'; 
				i = 2;
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
