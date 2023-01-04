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
static char rcsid[] = "@(#)$RCSfile: send.c,v $ $Revision: 4.2.9.2 $ (DEC) $Date: 1993/08/02 18:21:35 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0.1
 */
/* 
 * COMPONENT_NAME: CMDMAILX send.c
 * 
 * FUNCTIONS: MSGSTR, fixhead, fmt, headerp, infix, mail, mail1, 
 *            puthead, savemail, send, sendmail, statusput 
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
 *	send.c       5.2 (Berkeley) 6/21/85
 */

#include "rcv.h"
#include <sys/wait.h>
#include <ctype.h>
#include <sys/stat.h>
#include <time.h>

#include "Mail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s) 

#ifdef ASIAN_I18N
extern long cvtfile_size();
#endif /* ASIAN_I18N */

/*
 * Mail -- a mail program
 *
 * Mail to others.
 */

/*
 * Send message described by the passed pointer to the
 * passed output buffer.  Return -1 on error, but normally
 * the number of lines written.  Adjust the status: field
 * if need be.  If doign is set, suppress ignored header fields.
 */
#ifdef ASIAN_I18N
send(mailp, obuf, doign, cvtflg, charset_opt)
	struct message *mailp;
	FILE *obuf;
	int cvtflg;
	int charset_opt;
#else
send(mailp, obuf, doign)
	struct message *mailp;
	FILE *obuf;
#endif
{
	register struct message *mp;
	register int t;
	long c;
	FILE *ibuf;
	char line[LINESIZE], field[BUFSIZ];
	int lc, ishead, infld, fline, dostat;
	char *cp, *cp2;
#ifdef ASIAN_I18N
	wchar_t wc;
	int mb;
#endif

	mp = mailp;
	ibuf = setinput(mp);
#ifdef ASIAN_I18N
	if (cvtflg) {
		convert_in_mail(&ibuf, mp-&message[0]+1);
		c = cvtfile_size(mp);
	}
	else c=mp->m_size;
#else
	c = mp->m_size;
#endif
	ishead = 1;
	dostat = 1;
	infld = 0;
	fline = 1;
	lc = 0;
	while (c > 0L) {
		fgets(line, LINESIZE, ibuf);
		c -= (long) strlen(line);
		lc++;
		if (ishead) {
			/* 
			 * First line is the From line, so no headers
			 * there to worry about
			 */
			if (fline) {
				fline = 0;
				goto writeit;
			}
			/*
			 * If line is blank, we've reached end of
			 * headers, so force out status: field
			 * and note that we are no longer in header
			 * fields
			 */
			if (line[0] == '\n') {
				if (dostat) {
					statusput(mailp, obuf, doign);
					dostat = 0;
				}
				ishead = 0;
				goto writeit;
			}
			/*
			 * If this line is a continuation (via space or tab)
			 * of a previous header field, just echo it
			 * (unless the field should be ignored).
			 */
#ifdef ASIAN_I18N
			if (infld && ISSPACE(wc, line, mb)) {
#else
			if (infld && (isspace(line[0]) || line[0] == '\t')) {
#endif
				if (doign && isign(field)) continue;
				goto writeit;
			}
			infld = 0;
			/*
			 * If we are no longer looking at real
			 * header lines, force out status:
			 * This happens in uucp style mail where
			 * there are no headers at all.
			 */
			if (!headerp(line)) {
				if (dostat) {
					statusput(mailp, obuf, doign);
					dostat = 0;
				}
				putc('\n', obuf);
				ishead = 0;
				goto writeit;
			}
			infld++;
			/*
			 * Pick up the header field.
			 * If it is an ignored field and
			 * we care about such things, skip it.
			 */
			cp = line;
			cp2 = field;
#ifdef ASIAN_I18N
			while (*cp && *cp != ':' && !ISSPACE(wc, cp, mb))
				*cp2++ = *cp++;
#else
			while (*cp && *cp != ':' && !isspace(*cp))
				*cp2++ = *cp++;
#endif
			*cp2 = 0;
			if (doign && isign(field))
				continue;
			/*
			 * If the field is "status," go compute and print the
			 * real Status: field
			 */
			if (icequal(field, "status")) {
				if (dostat) {
					statusput(mailp, obuf, doign);
					dostat = 0;
				}
				continue;
			}
#ifdef ASIAN_I18N
/* This is a lazy change on charset header because if we change the header
 * during conversion, we cannot guarantee the header to be appropriate in
 * all cases.  So we delay the change under here.
 */
			if (cvtflg && charset_opt)
				change_charset(line, mp-&message[0]+1, 
						charset_opt);
#endif /* ASIAN_I18N */

		}
writeit:
#ifdef ASIAN_I18N
		if (cvtflg && charset_opt)
			change_charset(line, mp-&message[0]+1, charset_opt);
#endif /* ASIAN_I18N */
		fputs(line, obuf);
		if (ferror(obuf))
			return(-1);
	}
	if (ferror(obuf))
		return(-1);
	if (ishead && (mailp->m_flag & MSTATUS))
		printf(MSGSTR(BADSTATUS, "failed to fix up status field\n")); /*MSG*/
	return(lc);
}

/*
 * Test if the passed line is a header line, RFC 733 style.
 */
headerp(line)
	register char *line;
{
	register char *cp = line;

#ifdef ASIAN_I18N
	int mb;
	wchar_t wc;

	while (*cp && !ISSPACE(wc, cp, mb) && *cp != ':')
		cp++;
	while (*cp && ISSPACE(wc, cp, mb))
		cp+=mb;
#else

	while (*cp && !isspace(*cp) && *cp != ':')
		cp++;
	while (*cp && isspace(*cp))
		cp++;
#endif
	return(*cp == ':');
}

/*
 * Output a reasonable looking status field.
 * But if "status" is ignored and doign, forget it.
 */
statusput(mp, obuf, doign)
	register struct message *mp;
	register FILE *obuf;
{
	char statout[3];

	if (doign && isign("status"))
		return;
	if ((mp->m_flag & (MNEW|MREAD)) == MNEW)
		return;
	if (mp->m_flag & MREAD)
		strcpy(statout, "R");
	else
		strcpy(statout, "");
	if ((mp->m_flag & MNEW) == 0)
		strcat(statout, "O");
	fprintf(obuf, MSGSTR(STATUS, "Status: %s\n"), statout); /*MSG*/
}


/*
 * Interface between the argument list and the mail1 routine
 * which does all the dirty work.
 */

mail(people)
	char **people;
{
	register char *cp2;
	register int s;
	char *buf, **ap;
	struct header head;
        char recfile[128];

	for (s = 0, ap = people; *ap != (char *) 0; ap++)
		s += strlen(*ap) + 1;
	buf = salloc(s+1);
	cp2 = buf;
	for (ap = people; *ap != (char *) 0; ap++) {
		cp2 = copy(*ap, cp2);
		*cp2++ = ' ';
	}
	if (cp2 != buf)
		cp2--;
	*cp2 = '\0';
	head.h_to = buf;
	if (Fflag) {				/* added for SVID-2 */
		char *tp, *bp, tbuf[128];
		struct name *to;
		/*
		 * peel off the first recipient and expand a possible alias
		 */
		for (tp=tbuf, bp=buf; *bp && *bp!=' '; bp++, tp++)
			*tp = *bp;
		*tp = '\0';
		if (debug)
			fprintf(stderr, MSGSTR(FIRREC,
			"First recipient is '%s'\n"), tbuf);
		if ((to = usermap(extract(tbuf, GTO)))==NIL) {
			printf(MSGSTR(NORECIPS, "No recipients specified\n"));
			return(1);
		}
		getrecf(to->n_name, recfile, Fflag);
	} else
		getrecf(buf, recfile, Fflag);
	head.h_subject = NULLSTR;
	head.h_cc = NULLSTR;
	head.h_bcc = NULLSTR;
	head.h_seq = 0;
	mail1(&head, recfile);
	return(0);
}


/*
 * Send mail to a bunch of user names.  The interface is through
 * the mail routine below.
 */

sendmail(str)
	char *str;
{
	register char **ap;
	char *bufp, recfile[128];	/* updated for SVID-2 */
	register int t;
	struct header head;

	if (blankline(str))
		head.h_to = NULLSTR;
	else
		head.h_to = str;
/*GAG*/	getrecf(head.h_to,recfile,0); /*
        getrecf(head.h_to,recfile,1);	/* updated for SVID-2 */
	head.h_subject = NULLSTR;
	head.h_cc = NULLSTR;
	head.h_bcc = NULLSTR;
	head.h_seq = 0;
	mail1(&head, recfile);
	return(0);
}

/*
 * Mail a message on standard input to the people indicated
 * in the passed header.  (Internal interface).
 */

mail1(hp, recfile)
	struct header *hp;
	char *recfile;			/* changed for SVID-2 */
{
	register char *cp;
	int pid, i, s, p, gotcha;
	char **namelist, *deliver, *ccp;
	struct name *to, *np;
	struct stat sbuf;
	FILE *mtf, *postage;
	int remote = rflag != NULLSTR || rmail;
	char **t;
	char buf[BUFSIZ];


	/*
	 * Collect user's mail from standard input.
	 * Get the result as mtf.
	 */

	pid = -1;
	if ((mtf = collect(hp)) == NULL)
		return(-1);
#ifdef ASIAN_I18N
	convert_out_mail(hp, &mtf);
#endif
	hp->h_seq = 1;
	if (hp->h_subject == NULLSTR)
		hp->h_subject = sflag;

	/*
	 * Now, take the user names from the combined
	 * to and cc lists and do all the alias
	 * processing.
	 */

	senderr = 0;
	to = usermap(cat(extract(hp->h_bcc, GBCC),
	    cat(extract(hp->h_to, GTO), extract(hp->h_cc, GCC))));
	if (to == NIL) {
		printf(MSGSTR(NORECIPS, "No recipients specified\n")); /*MSG*/
		goto topdog;
	}

	/*
	 * Look through the recipient list for names with /'s
	 * in them which we write to as files directly.
	 */

	to = outof(to, mtf, hp);
	rewind(mtf);
	to = verify(to);
	if (senderr && !remote) {
topdog:

		if (fsize(mtf) != 0) {
			remove(deadletter);
			exwrite(deadletter, mtf, 1);
			rewind(mtf);
		}
	}
	for (gotcha = 0, np = to; np != NIL; np = np->n_flink)
		if ((np->n_type & GDEL) == 0) {
			gotcha++;
			break;
		}
	if (!gotcha)
		goto out;
	to = elide(to);
	mechk(to);
	if (count(to) > 1)
		hp->h_seq++;
	if (hp->h_seq > 0 && !remote) {
		fixhead(hp, to);
		if (fsize(mtf) == 0)
		    if (hp->h_subject == NULLSTR)
			printf(MSGSTR(NOINFO, "No message, no subject; hope that's ok\n")); /*MSG*/
		    else
			printf(MSGSTR(NOTEXT, "Null message body; hope that's ok\n")); /*MSG*/
		if ((mtf = infix(hp, mtf)) == NULL) {
			fprintf(stderr, MSGSTR(LOST, ". . . message lost, sorry.\n")); /*MSG*/
			return(-1);
		}
	}
	namelist = unpack(to);
	if (debug) {
		printf(MSGSTR(RECIP, "Recipients of message:\n"));
		for (t = namelist; *t != NULLSTR; t++)
			printf(" \"%s\"", *t);
		printf("\n");
		fflush(stdout);
		return;
	}
        if (recfile != NULLSTR && *recfile)		/* for SVID-2 */
                savemail(expand(recfile), hp, mtf);

	/*
	 * Wait, to absorb a potential zombie, then
	 * fork, set up the temporary mail file as standard
	 * input for "mail" and exec with the user list we generated
	 * far above. Return the process id to caller in case he
	 * wants to await the completion of mail.
	 */

	while (wait3((union wait *)&s, WNOHANG, (struct rusage *)0) > 0)
		;
	rewind(mtf);
	pid = fork();
	if (pid == -1) {
		perror("fork");
		remove(deadletter);
		exwrite(deadletter, mtf, 1);
		goto out;
	}
	if (pid == 0) {
		sigchild();
#ifdef SIGTSTP
		if (remote == 0) {
			signal(SIGTSTP, SIG_IGN);
			signal(SIGTTIN, SIG_IGN);
			signal(SIGTTOU, SIG_IGN);
		}
#endif
		for (i = SIGHUP; i <= SIGQUIT; i++)
			signal(i, SIG_IGN);
		if (!stat(POSTAGE, &sbuf))
			if ((postage = fopen(POSTAGE, "a")) != NULL) {
				fprintf(postage, "%s %d %d\n", myname,
				    count(to), fsize(mtf));
				fclose(postage);
			}
		s = fileno(mtf);
		for (i = 3; i < 15; i++)
			if (i != s)
				close(i);
		close(0);
		dup(s);
		close(s);
#ifdef CC
		submit(getpid());
#endif /*CC*/
#ifdef SENDMAIL
		deliver = getenv ("sendmail");
		if (deliver == NULLSTR)
			deliver = getenv ("SENDMAIL");
		if (deliver == NULLSTR)
			deliver = value("sendmail");
		if (deliver == NULLSTR)
			deliver = SENDMAIL;
		execv (deliver, namelist);
		perror(deliver);
#endif /*SENDMAIL*/
		execv(MAIL, namelist);
		perror(MAIL);
		exit(1);
	}

out:
	if (remote || (value("verbose") != NULLSTR)) {
		while ((p = wait(&s)) != pid && p != -1)
			;
		if (s != 0)
			senderr++;
		pid = 0;
	}
	fclose(mtf);
	return(pid);
}

/*
 * Fix the header by glopping all of the expanded names from
 * the distribution list into the appropriate fields.
 * If there are any ARPA net recipients in the message,
 * we must insert commas, alas.
 */

fixhead(hp, tolist)
	struct header *hp;
	struct name *tolist;
{
	register struct name *nlist;
	register int f;
	register struct name *np;

	for (f = 0, np = tolist; np != NIL; np = np->n_flink)
		if (any('@', np->n_name)) {
			f |= GCOMMA;
			break;
		}

	if (debug && f & GCOMMA)
		fprintf(stderr, MSGSTR(SHDBCOM,
		"Should be inserting commas in recip lists\n"));
	hp->h_to = detract(tolist, GTO|f);
	hp->h_cc = detract(tolist, GCC|f);
}

/*
 * Prepend a header in front of the collected stuff
 * and return the new file.
 */

FILE *
infix(hp, fi)
	struct header *hp;
	FILE *fi;
{
	extern char tempMail[];
	register FILE *nfo, *nfi;
	register int c, ck;

	rewind(fi);
	if ((nfo = fopen(tempMail, "w")) == NULL) {
		perror(tempMail);
		return(fi);
	}
	if ((nfi = fopen(tempMail, "r")) == NULL) {
		perror(tempMail);
		fclose(nfo);
		return(fi);
	}
	remove(tempMail);
	puthead(hp, nfo, GTO|GSUBJECT|GCC|GNL);
	c = getc(fi);
	while (c != EOF) {
		ck = putc(c, nfo);
		if (ck == EOF) {
		perror(tempMail);
		fclose(nfo);
		fclose(nfi);
		return(fi);
		}
		c = getc(fi);
	}
	if (ferror(fi)) {
		perror("read");
		return(fi);
	}
	fflush(nfo);
	if (ferror(nfo)) {
		perror(tempMail);
		fclose(nfo);
		fclose(nfi);
		return(fi);
	}
	fclose(nfo);
	fclose(fi);
	rewind(nfi);
	return(nfi);
}

/*
 * Dump the to, subject, cc header on the
 * passed file buffer.
 */

puthead(hp, fo, w)
	struct header *hp;
	FILE *fo;
{
	register int gotcha;
	char templine[LINESIZE];

	gotcha = 0;
#ifdef ASIAN_I18N
	put_mime_header(fo);
#endif
	if (hp->h_to != NULLSTR && w & GTO)
		fmt("To: ", hp->h_to, fo), gotcha++;
	if (hp->h_subject != NULLSTR && w & GSUBJECT)
		{
#ifndef ASIAN_I18N
		strcpy (templine, hp->h_subject);
		NLflatstr (templine, hp->h_subject, LINESIZE);
#endif /* ASIAN_I18N */
		fprintf(fo, MSGSTR(SUBJECT, "Subject: %s\n"), hp->h_subject);
		gotcha++;
		}
	if (hp->h_cc != NULLSTR && w & GCC)
		fmt("Cc: ", hp->h_cc, fo), gotcha++;
	if (hp->h_bcc != NULLSTR && w & GBCC)
		fmt("Bcc: ", hp->h_bcc, fo), gotcha++;
	if (gotcha && w & GNL)
		putc('\n', fo);
	return(0);
}

/*
 * Format the given text to not exceed 72 characters.
 */

fmt(str, txt, fo)
	register char *str, *txt;
	register FILE *fo;
{
	register int col;
	register char *bg, *bl, *pt, ch;

#ifdef ASIAN_I18N
	int mb;
	wchar_t wc;
#endif

	col = strlen(str);
	if (col)
		fprintf(fo, "%s", str);
	pt = bg = txt;
	bl = 0;
	while (*bg) {
		pt++;
		if (++col >72) {
			if (!bl) {
				bl = bg;
#ifdef ASIAN_I18N
				while (*bl && !ISSPACE(wc, bl, mb))
#else
				while (*bl && !isspace(*bl))
#endif
					bl++;
			}
			if (!*bl)
				goto finish;
			ch = *bl;
			*bl = '\0';
			fprintf(fo, "%s\n    ", bg);
			col = 4;
			*bl = ch;
			pt = bg = ++bl;
			bl = 0;
		}
		if (!*pt) {
finish:
			fprintf(fo, "%s\n", bg);
			return;
		}
#ifdef ASIAN_I18N
		if (ISSPACE(wc, pt, mb))
#else
		if (isspace(*pt))
#endif
			bl = pt;
	}
}

/*
 * Save the outgoing mail on the passed file.
 */

savemail(name, hp, fi)
	char name[];
	struct header *hp;
	FILE *fi;
{
	register FILE *fo;
	register int c, ck;
	time_t now;
	char *n;

	if ((fo = fopen(name, "a")) == NULL) {
		perror(name);
		return(-1);
	}
	time(&now);
	n = rflag;
	if (n == NULLSTR)
		n = myname;
	fprintf(fo, MSGSTR(FROM, "From %s %s"), n, ctime(&now));
	rewind(fi);
	for (c = getc(fi); c != EOF; c = getc(fi)) {
		ck = putc(c, fo);
		if (ck == EOF) {
		perror(name);
		break;
		}
	}
	fprintf(fo, "\n");
	fflush(fo);
	if (ferror(fo))
		perror(name);
	fclose(fo);
	return(0);
}
