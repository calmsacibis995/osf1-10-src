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
static char	*sccsid = "@(#)$RCSfile: tic.c,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1993/10/11 19:20:33 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * COMPONENT_NAME: (TERMINFO) Terminfo
 *
 * FUNCTIONS:   tic, compfile, tgetent, tnchkuse, tnamatch, tskip, tgetnum,
 *              tgetflag, tgetstr, tdecode, store, tgetstr, listlen, checkon
 *
 * ORIGINS: 3, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * tic.c  1.12  com/terminfo,3.1,9013 3/13/90 13:07:06
 *
 * History:
 *
 * 24-Jun-91  (gray)
 *
 *	Modified to use value of PATH_MAX (1024) for directory
 *      path of terminfo when specified in the TERMINFO
 *      environment variable. Previous value was 64.
 */

/*              include file for message texts          */
#ifndef _BLD
#include "tic_msg.h"
nl_catd  tic_catd;   /* Cat descriptor for scmc conversion */
#include <locale.h>
#define MSGSTR(n,s)   catgets(tic_catd, MS_tic, n, s)
#else
#define MSGSTR(n,s) s
#endif

#define CAPSIZ  8192

#define MAXHOP	32	/* max number of use= indirections */

#define COMMENT	'#'
#define SEPARATE	','
#define NUMBER	'#'
#define STRING	'='
#define CANCEL	'@'

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syslimits.h>

#ifndef E_TERMINFO
#define E_TERMINFO "terminfo.src"
#endif

#ifndef termpath
#define termpath "/usr/share/lib/terminfo/"                /* 004 */
#endif

/*
 * L_ctermid is only defined on USG.
 * We use it here since we don't include curses.h.
 */
#ifdef L_ctermid
#define index strchr
#endif

static	char *tbuf;
static	int hopcount;	/* detect infinite loops in terminfo, init 0 */
long	starttime;
int	verbose;
char	*terminfo;
char	*tskip();
char	*tgetstr();
char	*tdecode();
char	*getenv();

char *sourcefile = E_TERMINFO;

/*
 * NAME:        tic
 *
 * FUNCTION:
 *
 *      Program to compile a source terminfo file into object files.
 */

main(argc, argv)
char **argv;
{
	int i, exit_val = 0;
	char use_file [L_tmpnam];                /* 003 a */
	char use_line [256];                     /* 003 a */
	FILE *use_ptr;                           /* 003 a */

	/* open the message catalog file descriptor */

#ifndef	_BLD
	setlocale(LC_ALL, "");
	tic_catd = catopen(MF_TIC, NL_CAT_LOCALE);
#endif

	time(&starttime);
	while (argc > 1 && argv[1][0] == '-') {
		switch(argv[1][1]) {
		case 'v':
			if (argv[1][2])
				verbose = argv[1][2] - '0';
			else
				verbose++;
			break;
		default:
			fprintf(stderr,  MSGSTR(
				M_MSG_1, "usage: tic [-v[number]] [file...]\n") );
			exit(1);
		}
		argc--; argv++;
	}

	terminfo = getenv("TERMINFO");

	/* 003 (b) -- start */
	/* if reading from "stdin", make a copy of it for "use" operations */

	use_file [0] = 0;
	use_ptr = NULL;
	if (argc == 1) {
		tmpnam (use_file);
		use_ptr = fopen (use_file, "w");
		while (fgets (use_line, 256, stdin) != NULL)
			fputs (use_line, use_ptr);
		fclose (use_ptr);
		use_ptr = fopen (use_file, "r");
	}
	/* 003 (b) -- end */

	/* compile the terminfo source file(s), only return success if all source files 
	   compile successfully. paw: qar 4880 */
	if (argc == 1) {
 		exit_val = compfile(use_ptr, use_file);   /* 003 */
	} 
	else for (i=1; i<argc; i++) { 	
		if (exit_val == 0) 
			exit_val = compfile(fopen(argv[i], "r"), argv[i]); 
		else
			compfile(fopen(argv[i], "r"), argv[i]); 
	} 

	/* 003 (c) -- start */
	/* delete the spooled copy of "stdin" */

	if (use_file [0])
		unlink (use_file);
	/* 003 (c) -- end */

	exit (exit_val); 
}

/*
 * NAME:        compfile
 *
 * FUNCTION:
 *
 *      Compile a file.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This is very similar to the
 *      code in tgetstr but it passes through the whole file.
 */

compfile(tf, fname)
	FILE *tf;
	char *fname;
{
	register char *cp;
	register int c;
	register int i = 0, cnt = 0;
	char bp[CAPSIZ];
	char ibuf[CAPSIZ];
	char *cp2;
	int st_err, ret_val = 0;

	if (tf == (FILE *) NULL) {
		perror(fname);
		return (-1);
	}
	ibuf[0] = 0;
	for (;;) {
		tbuf = bp;
		strcpy(bp, ibuf);
		for (;;) {
			if (fgets(ibuf, sizeof ibuf, tf) == (char *) NULL) {
				fclose(tf);
				if (tnchkuse(fname)) {
					st_err = store(bp, fname);
					ret_val = (ret_val ? ret_val : st_err);
				}
				return ret_val;
			}
			/* comment or blank line */
			if (ibuf[0] == COMMENT || ibuf[0] == '\n')
				continue;
			cp = &ibuf[strlen(ibuf)-3];
			/* Allow and ignore old style backslashes */
			if (*cp == SEPARATE && cp[1] == '\\')
				cp[1] = 0;
			cp[2] = 0;	/* get rid of newline */
			/* lines with leading white space are continuation */
			if (!isspace(ibuf[0]) && *bp)
				break;
			if (strlen(bp) + strlen(ibuf) >= CAPSIZ) {
				fprintf(stderr,  MSGSTR(
				M_MSG_2, "Terminfo entry too long:\n") );
				fprintf(stderr, "%s", bp);
			}
			else {
				cp = ibuf;
				while (isspace(*cp))
					cp++;
				strcat(bp, cp);
			}
		}

		/*
		 * We have it, now do something with it.
		 */
		if (tnchkuse(fname)) {
			st_err = store(bp, fname);
			ret_val = (ret_val ? ret_val : st_err);
		}
	}
}

/*
 * NAME:        tgetent
 *
 * FUNCTION:
 *
 *      Get an entry for terminal name in buffer bp,
 *      from the terminfo file.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Parse is very rudimentary;
 *      we just notice escaped newlines.
 */

tgetent(bp, name, fname)
	char *bp, *name, *fname;
{
	register char *cp;
	register int c;
	register int i = 0, cnt = 0;
	char ibuf[CAPSIZ];
	char *cp2;
	FILE *tf;

	ibuf[0] = 0;
	tf = fopen(fname, "r");
	if (tf == (FILE *) NULL)
		return (-1);
	tbuf = bp;
	for (;;) {
		strcpy(bp, ibuf);
		for (;;) {
			if (fgets(ibuf, sizeof ibuf, tf) == (char *) NULL) {
				fclose(tf);
				if (tnamatch(name))
					return(tnchkuse(fname));
				return 0;
			}
			if (ibuf[0] == COMMENT) /* comment */
				continue;
			cp = &ibuf[strlen(ibuf)-3];
			/* Allow and ignore old style backslashes */
			if (*cp == SEPARATE && cp[1] == '\\')
				cp[1] = 0;
			cp[2] = 0;	/* get rid of newline */
			/* lines with leading white space are continuation */
			if (!isspace(ibuf[0]) && *bp)
				break;
			if (strlen(bp) + strlen(ibuf) >= CAPSIZ) {
				fprintf(stderr,  MSGSTR(
				M_MSG_4, "Terminfo entry too long:\n") );
				fprintf(stderr, "%s", bp);
			}
			else {
				cp = ibuf;
				while (isspace(*cp))
					cp++;
				strcat(bp, cp);
			}
		}

		/*
		 * The real work for the match.
		 */
		if (tnamatch(name)) {
			fclose(tf);
			return(tnchkuse(fname));
		}
	}
}

/*
 * NAME:        tnchkuse
 *
 * FUNCTION:
 *
 *      Check the last entry, see if it's use=xxx. If so,
 *      recursively find xxx and append that entry (minus the names)
 *      to take the place of the use=xxx entry.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This allows terminfo
 *      entries to say "like an HP2621 but doesn't turn on the labels".
 *      Note that this works because of the left to right scan.
 */

tnchkuse(fname)
char *fname;
{
	register char *p, *q;
	char tcname[32];        /* name of similar terminal */
	char tcbuf[CAPSIZ];
	char restbuf[CAPSIZ];
	char *holdtbuf = tbuf;
	char *beg_use, *beg_next;
	char *index();
	int l;

	p = tbuf;
	if (++hopcount > MAXHOP) {
		fprintf(stderr,  MSGSTR( M_MSG_6,
		"Infinite use= loop '%s'\n") , tbuf);
		return (0);
	}
	for (;;) {
		p = index(p, 'u');
		if (p == (char *) NULL) {
			tbuf = holdtbuf;
			return 1;
		}
		beg_use = p;
		if (*++p != 's' || *++p != 'e' || *++p != '=')
			continue;
		strncpy(tcname, ++p, sizeof tcname);
		q = index(tcname, SEPARATE);
		if (q)
			*q = 0;
		/* try local file ... */
		if (tgetent(tcbuf, tcname, fname) != 1)	{
			/* ... and master */
			if (tgetent(tcbuf, tcname, E_TERMINFO) != 1) {
				fprintf(stderr,  MSGSTR(
				M_MSG_7, "Cannot find term %s\n") , tcname);
				return(0);
			}
		}

		/* Find the end of the use= spec */
		for(beg_next=beg_use;
		    *beg_next && *beg_next!=SEPARATE;
		    beg_next++)
			;
		beg_next++;
		while (isspace(*beg_next++))
			;
		beg_next--;
		
		/* Now shuffle string around. */
		strcpy(restbuf, beg_next);
		p = index(tcbuf, SEPARATE);
		if (p == (char *) NULL)
			p = tcbuf;
		else
			p++;
		strcpy(beg_use, p);
		p = index(beg_use, '\0');
		strcpy(p, restbuf);
	}
}

/*
 * NAME:        tnamatch
 *
 * FUNCTION:
 *
 *      Name matching.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      The first field of the terminfo
 *      entry is a sequence of names separated by |'s, so we compare
 *      against each such name.  The normal : terminator after the last
 *      name (before the first field) stops us.
 */

tnamatch(np)
	char *np;
{
	register char *Np, *Bp;

	Bp = tbuf;
	if (*Bp == COMMENT)
		return(0);
	for (;;) {
		for (Np = np; *Np && *Bp == *Np; Bp++, Np++)
			;
		if (*Np == 0 && (*Bp == '|' || *Bp == SEPARATE || *Bp == 0))
			return (1);
		while (*Bp && *Bp != SEPARATE && *Bp != '|')
			Bp++;
		if (*Bp == 0 || *Bp == SEPARATE)
			return (0);
		Bp++;
	}
}

/*
 * NAME:        tskip
 *
 * FUNCTION:
 *
 *      Skip to the next SEPARATE delimited field.
 */

static char *
tskip(bp)
	register char *bp;
{

	while (*bp && *bp != SEPARATE)
		bp++;
	if (*bp == 0)
		return bp;
	bp++;
	while (isspace(*bp) || *bp == SEPARATE)
		bp++;
	return (bp);
}

/*
 * NAME:        tgetnum
 *
 * FUNCTION:
 *
 *      Return the (numeric) option id.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Numeric options look like
 *              li#80
 *      i.e. the option string is separated from the numeric value by
 *      a # character.  If the option is not found we return -1.
 *      Note that we handle octal numbers beginning with 0.
 */

tgetnum(id)
	char *id;
{
	register int i, base;
	register char *bp = tbuf;
	int idl = strlen(id);
	int sign = 1;

	for (;;) {
		bp = tskip(bp);
		if (*bp == 0)
			return (-1);
		if (strncmp(id, bp, idl))
			continue;
		bp += idl;
		if (*bp == CANCEL)
			return(-1);
		if (*bp != NUMBER && *bp != STRING)
			continue;
		bp++;
		if (*bp == '-') {
			sign = -1;
			bp++;
		}
		base = 10;
		if (*bp == '0')
			base = 8;
		i = 0;
		while (isdigit(*bp))
			i *= base, i += *bp++ - '0';
		i *= sign;
		return (i);
	}
}

/*
 * NAME:        tgetflag
 *
 * FUNCTION:
 *
 *      Handle a flag option.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Flag options are given "naked", i.e. followed by a : or the end
 *      of the buffer.  Return 1 if we find the option, or 0 if it is
 *      not given.
 */

tgetflag(id)
	char *id;
{
	register char *bp = tbuf;
	int idl = strlen(id);

	for (;;) {
		bp = tskip(bp);
		if (!*bp)
			return (0);
		if (strncmp(bp, id, idl) == 0) {
			bp += idl;
			if (!*bp || *bp == SEPARATE)
				return (1);
			else if (*bp == CANCEL)
				return(0);
		}
	}
}

/*
 * NAME:        tgetstr
 *
 * FUNCTION:
 *
 *      Get a string valued option.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      These are given as
 *              cl=^Z
 *      Much decoding is done on the strings, and the strings are
 *      placed in area, which is a ref parameter which is updated.
 *      No checking on area overflow.
 */

char *
tgetstr(id, area)
	char *id, **area;
{
	register char *bp = tbuf;
	int idl = strlen(id);

	for (;;) {
		bp = tskip(bp);
		if (!*bp)
			return (0);
		if (strncmp(id, bp, idl))
			continue;
		bp += idl;
		if (*bp == CANCEL)
			return(0);
		if (*bp != STRING)
			continue;
		bp++;
		return (tdecode(bp, area));
	}
}

/*
 * NAME:        tdecode
 *
 * FUNCTION:
 *
 *      Tdecode does the grung work to decode the
 *      string capability escapes.
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
	while ((c = *str++) && c != SEPARATE) {
		switch (c) {

		case '^':
			c = *str++ & 037;
			break;

		case '\\':
			/*
			 * \x escapes understood:
			 *	\e	escape
			 *	\E	escape
			 *	\^	^
			 *	\\	\
			 *	\,	,
			 *	\:	:
			 *	\l	linefeed
			 *	\n	newline (=linefeed)
			 *	\r	return
			 *	\t	tab
			 *	\b	backspace
			 *	\f	formfeed
			 *	\s	space
			 *	\0	null
			 *	\###	octal ###
			 */
			dp = "e\033E\033^^\\\\,,::l\012n\nr\rt\tb\bf\fs ";
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
				do {
					if (!isdigit(*str))
						break;
					c <<= 3;
					c |= *str++ - '0';
				} while (--i);
				if (c == 0)
					c = 0200;	/* don't term. str. */
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

extern char *boolnames[], *numnames[], *strnames[];
extern char *boolcodes[], *numcodes[], *strcodes[];

char *malloc();
char *tgetstr();

#define TIMAGNUM 0432

/*
 * NAME:        store
 */

store(cap, fname)
char *cap, *fname;
{
	register char *cp;
	register int i;
	register char **pp, **np;
	char tcpbuf[CAPSIZ];
	char *tcp = tcpbuf;
	char *tname = cap;
	char *tnp;
	char tnbuf[256], names[256];
	char fnbuf[PATH_MAX], lnbuf[PATH_MAX];  /* was 64 */
	char strtab[CAPSIZ];              /* was 4096 */
	register char *strtabptr;
	FILE *fd;
	int sname, sbool;

	if (!*cap) {
		fprintf(stderr,  MSGSTR( M_MSG_9,
		"Warning: Terminfo entry empty: %s\n"), fname );
		return(1);
	}
	while (*cap != SEPARATE && *cap != '\0')	/* skip over names */
		cap++;
	*cap = 0;
	strcpy(tnbuf, tname);
	strcpy(names, tname);
	*cap = SEPARATE;

	for (tnp=tnbuf; *tnp && *tnp != '|' && *tnp != SEPARATE; tnp++)
		;
	if (*tnp)
		*tnp++ = 0;
	if (terminfo) {
		strcpy(fnbuf, terminfo);
		strcat(fnbuf, "/");
	} else {
		strcpy(fnbuf, termpath);         /* 004 */
	}
	strcat(fnbuf, tnbuf);
	checkon(fnbuf);
	if (verbose)
		fprintf(stderr,  MSGSTR( M_MSG_10, "create '%s'\n"), fnbuf );/*001*/
	fd = fopen(fnbuf, "w");
	if (fd == (FILE *) NULL) {
		perror(fnbuf);
		return(1);
	}

	putsh(TIMAGNUM, fd);
	sname = strlen(names)+1;
	putsh(sname, fd);
	sbool = listlen(boolcodes);
	putsh(sbool, fd);
	putsh(listlen(numcodes), fd);
	putsh(listlen(strcodes), fd);
	putsh(0, fd);			/* length of string table */

	/* Write out various terminal names to file, null terminated. */
	for (cp=names; *cp; cp++)
		putc(*cp, fd);
	putc(0, fd);

	/* Write out the booleans: flag */
	for (pp=boolnames, np=boolcodes; *np; pp++,np++) {
		i = tgetflag(*pp);
		putc(i, fd);
		if (verbose > 2)
			fprintf(stdout,  MSGSTR(
			M_MSG_11, "bool cap %s code %s val %d\n") ,
			*pp, *np, i);
	}
	if ((sname + sbool) & 1)
		putc(0, fd);

	/* Numbers: highbyte, lowbyte.  0377,0377 means -1 (missing) */
	for (pp=numnames, np=numcodes;   *np; pp++,np++) {
		i = tgetnum(*pp);
		putsh(i, fd);
		if (verbose > 1)
			fprintf(stdout,  MSGSTR(
			M_MSG_12, "num cap %s code %s val %d\n") ,
			*pp, *np, i);
	}

	/* Strings: offset into string table.  If cap is missing, -1 is used */
	strtabptr = strtab;
	for (pp=strnames, np=strcodes;   *np; pp++,np++) {
		cp = tgetstr(*pp, &tcp);
		if (verbose > 3)
			if (cp)
				fprintf(stdout,  MSGSTR(
				M_MSG_13, "str %s code %s val %s\n") ,
				*pp, *np, cp);
			else
				fprintf(stdout,  MSGSTR(
				M_MSG_14, "str %s code %s val NULL\n") ,
				*pp, *np);
		if (cp) {
			putsh(strtabptr-strtab, fd);
			while (*strtabptr++ = *cp++)
				;
		} else {
			putsh(-1, fd);
		}
	}
	fwrite(strtab, 1, strtabptr-strtab, fd);
	fseek(fd, 10L, 0);	/* Back to string table size in header */
	putsh(strtabptr-strtab, fd);
	fclose(fd);
	hopcount = 0;

	while (*tnp) {
		i = 0;
		for (tname=tnp; *tnp && *tnp != '|' && *tnp != SEPARATE;
								tnp++)
			if (isspace(*tnp))
				i = 1;
		if (*tnp)
			*tnp++ = 0;
		if (i)
			continue;
		if (terminfo) {
			strcpy(lnbuf, terminfo);
			strcat(lnbuf, "/");
		} else {
			strcpy(lnbuf, termpath);         /* 004 */
		}
		strcat(lnbuf, tname);
		checkon(lnbuf);
		link(fnbuf, lnbuf);
		if (verbose)
			fprintf(stdout,  MSGSTR(
			M_MSG_15, "link '%s' '%s'\n") , fnbuf, lnbuf);
	}
}

/*
 * NAME:        tgetstr
 *
 * FUNCTION:
 *
 *      Write a short out to the file in machine-independent format.
 */

putsh(val, fd)
register val;
FILE *fd;
{
	if (val != -1) {
		putc(val&0377, fd);
		putc((val>>8)&0377, fd);
	} else {
		/* Write -1 as two 0377's. */
		putc(0377, fd);
		putc(0377, fd);
	}
}

/*
 * NAME:        listlen
 *
 */

listlen(list)
register char **list;
{
	register int rv = 0;

	while (*list) {
		list++;
		rv++;
	}
	return rv;
}

/*
 * NAME:        checkon
 *
 * FUNCTION:
 *
 *      Do various processing on a file name to be created.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      If it already exists, and it's older than we started, unlink it.
 *      Also insert a / after the 2nd char of the tail, and make sure
 *      that directory exists.
 */

checkon(fn)
char *fn;
{
	struct stat stbuf;
	char *fp, *cp, *fp2;
	char nbuf[PATH_MAX];
	char cmdbuf[PATH_MAX];

	/* Find last / */
	for (cp=fn; *cp; cp++)
		if (*cp == '/')
			fp = cp;
	if (cp-fp > 2) {
		cp = fp + 2L;
		strcpy(nbuf, fp+1L);
		*cp = 0;
		if (stat(fn, &stbuf) < 0) {
			sprintf(cmdbuf, "mkdir %s", fn);
			if (verbose)
				fprintf(stdout, "%s\n", cmdbuf);
			if (mkdir (fn, 0777) != 0) {    /* 002 */
				fprintf(stderr,  MSGSTR(
				M_MSG_18, "Command '%s' failed\n"), cmdbuf);
				exit(1);
			}
		}
		*cp++ = '/';
		strcpy(cp, nbuf);

	}
	else
		fprintf(stderr,  MSGSTR( M_MSG_19,
		"%s: Terminal name too short\n") , fp + 1L);
	if (stat(fn, &stbuf) < 0)
		return;
	if (stbuf.st_mtime < starttime) {
		if (verbose > 1)
			fprintf(stdout,  MSGSTR(M_MSG_20, "unlink %s\n") , fn);
		unlink(fn);
	}
}



