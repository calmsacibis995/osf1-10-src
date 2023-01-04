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
static char rcsid[] = "@(#)$RCSfile: tty.c,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/08/02 18:21:43 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * COMPONENT_NAME: CMDMAILX tty.c
 * 
 * FUNCTIONS: MSGSTR, grabh, readtty, signull, ttycont 
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
 *	tty.c        5.2 (Berkeley) 6/21/85
 */

/*
 * Mail -- a mail program
 *
 * Generally useful tty stuff.
 */

#include "rcv.h"

#include "Mail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s) 

static	int	c_erase;		/* Current erase char */
static	int	c_kill;			/* Current kill char */
static	int	hadcont;		/* Saw continue signal */
static	jmp_buf	rewrite;		/* Place to go when continued */
#ifndef TIOCSTI
static	int	ttyset;			/* We must now do erase/kill */
static	int	c_lflag;		/* current line modes (SysV style) */
static	int	c_line;			/* Current line value */
static	int	eof, eol;		/* used as MIN and TIME when !ICANON */
#endif

extern int hadintr;			/* flag that we have rcvd one intr */
extern jmp_buf *Jmpbuf;			/* global ptr for signal handler */
extern void collrub();			/* SIGINT handler */
extern void intack();			/* SIGINT handler when we're ignoring */
extern int hf;				/* true iff "ignore" is set */


/*
 * Read all relevant header fields.
 */

grabh(hp, gflags)
	struct header *hp;
{
	struct sgttyb ttybuf;
	void ttycont(), signull();
#ifndef TIOCSTI
	void (*savesigs[2])();
#endif
	void (*savecont)();
	register int s;
	int errs;
	char templine[LINESIZE];

#ifdef SIGCONT
	savecont = signal(SIGCONT, signull);
#endif SIGCONT
	errs = 0;
#ifndef TIOCSTI
	ttyset = 0;
#endif
	if (gtty(fileno(stdin), &ttybuf) < 0) {
		perror("gtty");
		return(-1);
	}
	c_erase = ttybuf.sg_erase;
	c_kill = ttybuf.sg_kill;
#ifndef TIOCSTI
	c_lflag = ttybuf.c_lflag;
	c_line = ttybuf.c_line;
	eof = ttybuf.c_cc[4];
	eol = ttybuf.c_cc[5];
	ttybuf.sg_erase = ttybuf.sg_kill = 0;
	ttybuf.c_lflag &= ~ICANON;
	ttybuf.c_line = 0;
	ttybuf.c_cc[4] = ttybuf.c_cc[5] = 1;
	for (s = SIGINT; s <= SIGQUIT; s++)
		if ((savesigs[s-SIGINT] = signal(s, SIG_IGN)) == SIG_DFL)
			signal(s, SIG_DFL);
#endif

	if (gflags & GTO) {
#ifndef TIOCSTI
		if (!ttyset /* && hp->h_to != NULLSTR */ )
			ttyset++, stty(fileno(stdin), &ttybuf);
#endif
		hp->h_to = readtty("To: ", hp->h_to);
		if (hp->h_to != NULLSTR)
			hp->h_seq++;
	}
	if (gflags & GSUBJECT) {
#ifndef TIOCSTI
		if (!ttyset /* && hp->h_subject != NULLSTR */ )
			ttyset++, stty(fileno(stdin), &ttybuf);
#endif
		hp->h_subject = readtty("Subject: ", hp->h_subject);
		if (hp->h_subject != NULLSTR)
			{
#ifndef ASIAN_I18N
			strcpy (templine, hp->h_subject);
			NLflatstr (templine, hp->h_subject, LINESIZE);
#endif /* ASIAN_I18N */
			hp->h_seq++;
			}
	}
	if (gflags & GCC) {
#ifndef TIOCSTI
		if (!ttyset /* && hp->h_cc != NULLSTR */)
			ttyset++, stty(fileno(stdin), &ttybuf);
#endif
		hp->h_cc = readtty("Cc: ", hp->h_cc);
		if (hp->h_cc != NULLSTR)
			hp->h_seq++;
	}
	if (gflags & GBCC) {
#ifndef TIOCSTI
		if (!ttyset /* && hp->h_bcc != NULLSTR */)
			ttyset++, stty(fileno(stdin), &ttybuf);
#endif
		hp->h_bcc = readtty("Bcc: ", hp->h_bcc);
		if (hp->h_bcc != NULLSTR)
			hp->h_seq++;
	}
#ifdef SIGCONT
	signal(SIGCONT, savecont);
# endif SIGCONT
#ifndef TIOCSTI
	ttybuf.sg_erase = c_erase;
	ttybuf.sg_kill = c_kill;
	ttybuf.c_lflag = c_lflag;
	ttybuf.c_line = c_line;
	ttybuf.c_cc[4] = eof;
	ttybuf.c_cc[5] = eol;
	if (ttyset)
		stty(fileno(stdin), &ttybuf);
	for (s = SIGINT; s <= SIGQUIT; s++)
		signal(s, savesigs[s-SIGINT]);
#endif
	return(errs);
}

/*
 * Read up a header from standard input.
 * The source string has the preliminary contents to
 * be read.
 *
 */

char *
readtty(pr, src)
	char *pr, *src;
{
	char ch, canonb[BUFSIZ];
	int c;
	register char *cp, *cp2;
	jmp_buf *oldbuf;
	void (*saveint)();  /* saves signal handler */
	void ttycont(), signull();

	fputs(pr, stdout);
	fflush(stdout);
	if (src != NULLSTR && strlen(src) > BUFSIZ - 2) {
		printf(MSGSTR(TOOLONG, "too long to edit\n")); /*MSG*/
		return(src);
	}
#ifndef TIOCSTI
	if (src != NULLSTR)
		cp = copy(src, canonb);
	else
		cp = copy("", canonb);
	fputs(canonb, stdout);
	fflush(stdout);
#else
	cp = src == NULLSTR ? "" : src;
	while (c = *cp++) {
		if (c == c_erase || c == c_kill) {
			ch = '\\';
			ioctl(0, TIOCSTI, &ch);
		}
		ch = c;
		ioctl(0, TIOCSTI, &ch);
	}
	cp = canonb;
	*cp = 0;
#endif
	cp2 = cp;
	while (cp2 < canonb + BUFSIZ)
		*cp2++ = 0;
	cp2 = cp;

	/* set up the context to restart on signals */
	if (setjmp(rewrite)) {
#ifdef TIOCSTI
		if (src)  /* save the original string for the retry */
		    strcpy(canonb, src);
#endif TIOCSTI
		goto redo;
	}
	oldbuf = Jmpbuf;  /* save ptr to old jmpbuf context */
	Jmpbuf = (jmp_buf *) rewrite;  /* tell sig hndlr to jump back here */
	saveint = signal(SIGINT, hf ? intack : collrub);
#ifdef SIGCONT
	signal(SIGCONT, ttycont);
#endif  SIGCONT
	clearerr(stdin);
	while (cp2 < canonb + BUFSIZ) {
		c = getc(stdin);
		hadintr = 0;  /* clear intr flag */
		if (c == EOF || c == '\n')
			break;
		if (c == c_erase) {
			if (cp2 == canonb) {
				fputc (' ', stdout);
				continue;
			}
			cp2--;
			if (*cp2 == '\\') {
				fprintf (stdout, "^%c", c + 0x40);
				fflush (stdout);
			} else {
				fprintf (stdout, " \b");
				fflush (stdout);
				*cp2 = '\0';
				continue;
			}
		}
		if (c == c_kill) {
			if (cp2 != canonb && cp2[-1] == '\\') {
				cp2--;
				fprintf (stdout, "\b^%c", c + 0x40);
				fflush (stdout);
			} else {
				cp2 = canonb;
				*cp2 = '\0';
				fprintf (stdout, "\n%s", pr);
				fflush (stdout);
				continue;
			}
		}
		if (c == 0x12) {	/* ^R (refresh ala BSD) */
			if (cp2[-1] == '\\') {
				cp2--;
				fprintf (stdout, "\b^%c", c + 0x40);
				fflush (stdout);
			} else {
				fprintf (stdout, "\n%s%s", pr, canonb);
				continue;
			}
		}
		*cp2++ = c;
		*cp2 = '\0';
	}

	*cp2 = 0;
#ifdef SIGCONT
	signal(SIGCONT, signull);
#endif SIGCONT
	if (c == EOF && ferror(stdin) && hadcont) {
redo:
		hadcont = 0;
		cp = strlen(canonb) > 0 ? canonb : NULLSTR;
		clearerr(stdin);
		return(readtty(pr, cp));
	}
	signal(SIGINT, saveint);  /* restore old handler */
	Jmpbuf = oldbuf;  /* reset old jmpbuf context */
	if (equal("", canonb))
		return(NULLSTR);
	return(savestr(canonb));
}

/*
 * Receipt continuation.
 */
void
ttycont(s)
{

	hadcont++;
	longjmp(rewrite, 1);
}

/*
 * Null routine to satisfy
 * silly system bug that denies us holding SIGCONT
 */
void
signull(s)
{}
