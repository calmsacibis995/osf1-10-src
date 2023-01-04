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
static char rcsid[] = "@(#)$RCSfile: cmd1.c,v $ $Revision: 4.2.9.3 $ (DEC) $Date: 1993/09/07 18:16:03 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0.1
 */
/* 
 * COMPONENT_NAME: CMDMAILX cmd1.c
 * 
 * FUNCTIONS: MSGSTR, More, Type, brokpipe, folders, from, headers, 
 *	    local, mboxit, more, pcmdlist, pdot, print, printhead, 
 *	    screensize, scroll, stouch, top, type, type1 
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
 *	cmd1.c       5.3 (Berkeley) 9/15/85
 *
 * Modification History
 *
 * 01 gaudet Tue Nov 05 18:50:55 EST 1991
 *	The display heading command, "h", would replace the null subject
 *	heading with the subject heading of the next message after a
 *	couple of iterations. The character array, "flatline", needed
 *	to be initialized. Fixes OSF_QAR #01857.
 */

#include "rcv.h"
#include <sys/stat.h>
#include <time.h>	/* GAG - LC_TIME support */
#include <nl_types.h>
#include <langinfo.h>

#include "Mail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s) 

#ifdef ASIAN_I18N
extern long cvtfile_size();
#endif /* ASIAN_I18N */

/*
 * Mail -- a mail program
 *
 * User commands.
 */

/*
 * Print the current active headings.
 * Don't change dot if invoker didn't give an argument.
 */

static int screen;
void brokpipe();

headers(msgvec)
	int *msgvec;
{
	register int n, mesg, flag;
	register struct message *mp;
	int size;

	size = screensize();
	n = msgvec[0];
	if (n != 0)
		screen = (n-1)/size;
	if (screen < 0)
		screen = 0;
	mp = &message[screen * size];
	if (mp >= &message[msgCount])
		mp = &message[msgCount - size];
	if (mp < &message[0])
		mp = &message[0];
	flag = 0;
	mesg = mp - &message[0];
	if (dot != &message[n-1])
		dot = mp;
	if (Hflag)			/* added for SVID-2 */
		mp = message;
	for (; mp < &message[msgCount]; mp++) {
		mesg++;
		if (mp->m_flag & MDELETED)
			continue;
		if (flag++ >= size && !Hflag)	/* changed for SVID-2 */
			break;
		printhead(mesg);
		sreset();
	}
	if (flag == 0) {
		printf(MSGSTR(NOMAIL, "No more mail.\n")); /*MSG*/
		return(1);
	}
	return(0);
}

/*
 * Set the list of alternate names for out host.
 */
local(namelist)
	char **namelist;
{
	register int c;
	register char **ap, **ap2, *cp;

	c = argcount(namelist) + 1;
	if (c == 1) {
		if (localnames == 0)
			return(0);
		for (ap = localnames; *ap; ap++)
			printf("%s ", *ap);
		printf("\n");
		return(0);
	}
	if (localnames != 0)
		cfree((char *) localnames);
	localnames = (char **) calloc(c, sizeof (char *));
	for (ap = namelist, ap2 = localnames; *ap; ap++, ap2++) {
		cp = (char *) calloc(strlen(*ap) + 1, sizeof (char));
		strcpy(cp, *ap);
		*ap2 = cp;
	}
	*ap2 = 0;
	return(0);
}

/*
 * Scroll to the next/previous screen
 */

scroll(arg)
	char arg[];
{
	register int s, size;
	int cur[1];

	cur[0] = 0;
	size = screensize();
	s = screen;
	switch (*arg) {
	case 0:
	case '+':
		s++;
		if (s * size > msgCount) {
			printf(MSGSTR(LAST, "On last screenful of messages\n")); /*MSG*/
			return(0);
		}
		screen = s;
		break;

	case '-':
		if (--s < 0) {
			printf(MSGSTR(FIRST, "On first screenful of messages\n")); /*MSG*/
			return(0);
		}
		screen = s;
		break;

	default:
		printf(MSGSTR(NOSCMD, "Unrecognized scrolling command \"%s\"\n"), arg); /*MSG*/
		return(1);
	}
	return(headers(cur));
}

/*
 * Compute what the screen size should be.
 * We use the following algorithm:
 *	If user specifies with screen option, use that.
 *	If baud rate < 1200, use  5
 *	If baud rate = 1200, use 10
 *	If baud rate > 1200, use 20
 */
screensize()
{
	register char *cp;
	register int s;
#ifdef	TIOCGWINSZ
	struct winsize ws;
#endif

	if ((cp = value("screen")) != NULLSTR) {
		s = atoi(cp);
		if (s > 0)
			return(s);
	}
	if (baud < B1200)
		s = 5;
	else if (baud == B1200)
		s = 10;
#ifdef	TIOCGWINSZ
	else if (ioctl(fileno(stdout), TIOCGWINSZ, &ws) == 0 && ws.ws_row != 0)
		s = ws.ws_row - 4;
#endif
	else
		s = 20;
	return(s);
}

/*
 * Print out the headlines for each message
 * in the passed message list.
 */

from(msgvec)
	int *msgvec;
{
	register int *ip;

	for (ip = msgvec; *ip != NULL; ip++) {
		printhead(*ip);
		sreset();
	}
	if (--ip >= msgvec)
		dot = &message[*ip - 1];
	return(0);
}

/*
 * Print out the header of a specific message.
 * This is a slight improvement to the standard one.
 */

printhead(mesg)
{
	struct message *mp;
	FILE *ibuf;
	char headline[LINESIZE], wcount[LINESIZE], *subjline, dispc, curind;
	char *toline, *fromline;			/* for SVID-2 */
	char pbuf[BUFSIZ];
	int s;
	struct headline hl;
	register char *cp;
	int showto;				/* added for SVID-2 */
	char flatline[LINESIZE];

#ifdef ASIAN_I18N
	int mb;
	wchar_t wc;
#endif

	flatline[0] = 0;
	mp = &message[mesg-1];
	ibuf = setinput(mp);
	readline(ibuf, headline);
	toline = hfield("to", mp);		/* added for SVID-2 */
	subjline = hfield("subject", mp);
	if (subjline == NULLSTR)
		subjline = hfield("subj", mp);

#ifdef ASIAN_I18N
	if (subjline != NULLSTR && strlen(subjline) > 27)
		fix_len_header(&subjline, 0, 27, mesg);
#else /* ASIAN_I18N */
	/*
	 * Bletch!
	 */

	if (subjline != NULLSTR && strlen(subjline) > 27)
		subjline[28] = '\0';
#endif /* ASIAN_I18N */

	curind = (!Hflag && dot == mp) ? '>' : ' ';	/* changed for SVID-2 */
	dispc = ' ';
	showto = 0;				/* added for SVID-2 */
	if (mp->m_flag & MSAVED)
		dispc = '*';
	if (mp->m_flag & MPRESERVE)
		dispc = 'P';
	if ((mp->m_flag & (MREAD|MNEW)) == MNEW)
		dispc = 'N';
	if ((mp->m_flag & (MREAD|MNEW)) == 0)
		dispc = 'U';
	if (mp->m_flag & MBOX)
		dispc = 'M';
	parse(headline, &hl, pbuf);
#ifdef ASIAN_I18N
	sprintf(wcount, "%d/%ld", mp->m_lines, cvtfile_size(mp));
#else
	sprintf(wcount, "%d/%ld", mp->m_lines, mp->m_size);
#endif
	s = strlen(wcount);
	cp = wcount + s;
	while (s < 7)
		s++, *cp++ = ' ';
	*cp = '\0';
						/* added for SVID-2 */
	fromline = nameof(mp, 0);
	if (toline && value("showto")) {
		if (value("allnet")) {
			if ((cp = strrchr(fromline, '!'))==NULLSTR)
				cp = fromline;
			else
				cp++;
		} else
			cp = fromline;
		if (strcmp(cp, myname)==0) {
			showto = 1;
			fromline = toline;
#ifdef ASIAN_I18N
			while (*toline && !ISSPACE(wc, toline, mb))
				toline++;
#else
			while (*toline && !isspace(*toline))
				toline++;
#endif
			*toline = '\0';
			if ((cp = strrchr(fromline, '!'))==NULLSTR)
				cp = fromline;
			else while (cp > fromline) {
				if (*--cp=='!')
					break;
			}
			fromline = cp;
		}
	}
#if 0 /* modification from  Silver bl12, not sure will work or not */
/*
 *	The following routine makes a best effort to parse the date and time.
 *	We attempt to undo what binmail did with ctime(3).
 *	If this fails then we print what we had originally.
 */
	{
		struct tm tp;	/* strftime(3) needs time in this structure */
		char wday[4], month[4];
		int i;
		static char wday_name[7][3] = {
			"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
		};
		static char mon_name[12][3] = {
			"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
		};
		if (hl.l_date && sscanf(hl.l_date, "%3s %3s %2d %2d:%2d:%2d %4d", wday, month, &tp.tm_mday, &tp.tm_hour,
				&tp.tm_min, &tp.tm_sec, &tp.tm_year) != EOF) {
			for (i = 0; i < 12; i++) {
				if (!strncmp(mon_name[i], month, (size_t) 3)) {
					tp.tm_mon = i;
					break;
				}
			}
			if (i != 12) {
				for (i = 0; i < 7; i++) {
					if (!strncmp(wday_name[i], wday, (size_t) 3)) {
						tp.tm_wday = i;
						break;
					}
				}
				if (i != 7) {
					tp.tm_yday = 0;
					tp.tm_isdst = 0;
					strftime(hl.l_date, (size_t) 17, nl_langinfo(D_T_FMT), &tp);
				}
			}
		}
	}

#endif /* modification from Silver bl12, not sure will work or not */
#ifdef ASIAN_I18N
	if (showto) {			   /* added for SVID-2 */
		if (subjline != NULLSTR) {
			fix_len_header(&fromline, -15, 0, mesg);
			fix_len_header(&subjline, 0, 25, mesg);
			printf("%c%c%3d To %s %16.16s %4d/%-5d %s\n",
			       curind, dispc, mesg, fromline, hl.l_date,
			       mp->m_lines, cvtfile_size(mp), subjline);
		}
		else {
			fix_len_header(&fromline, -15, 0, mesg);
			printf("%c%c%3d To %s %16.16s %4d/%-5d\n",
			       curind, dispc, mesg, fromline, hl.l_date, 
			       mp->m_lines, cvtfile_size(mp));
		}
	}
	else				    /* changed for SVID-2 */
	{
		if (subjline != NULLSTR) {
			fromline=nameof(mp, 0);
			fix_len_header(&fromline, -16, 16, mesg);
			printf("%c%c%3d %s  %16.16s %8s \"%s\"\n",
			       curind, dispc, mesg, fromline, hl.l_date, 
			       wcount, subjline);
		}
		else {
			fromline=nameof(mp, 0);
			fix_len_header(&fromline, -16, 16, mesg);
			printf("%c%c%3d %s  %16.16s %8s\n",
			       curind, dispc, mesg, fromline, hl.l_date, 
			       wcount);
		}
	}
#else /* ASIAN_I18N */
	if (showto) {				/* added for SVID-2 */
		if (subjline != NULLSTR)
			printf("%c%c%3d To %-15s %16.16s %4d/%-5d %-.25s\n",
			    curind, dispc, mesg, fromline, hl.l_date,
			    mp->m_lines, mp->m_size, subjline);
		else
			printf("%c%c%3d To %-15s %16.16s %4d/%-5d\n", curind,
			    dispc, mesg,
			    fromline, hl.l_date, mp->m_lines, mp->m_size);
	}
	else					/* changed for SVID-2 */
	{
		if (subjline != NULLSTR)
			{
			NLflatstr (subjline, flatline, LINESIZE);
			printf("%c%c%3d %-16.16s  %16.16s %8s \"%s\"\n",
			      curind, dispc,
			      mesg, nameof(mp, 0), hl.l_date, wcount, flatline);
			}
		else
			printf("%c%c%3d %-16.16s  %16.16s %8s\n", curind,
			       dispc, mesg,
			       nameof(mp, 0), hl.l_date, wcount);
	}
#endif /* ASIAN_I18N */
}

/*
 * Print out the value of dot.
 */

pdot()
{
	printf("%d\n", dot - &message[0] + 1);
	return(0);
}

/*
 * Print out all the possible commands.
 */

pcmdlist()
{
	register struct cmd *cp;
	register int cc;
	extern struct cmd cmdtab[];

	printf(MSGSTR(CMDS, "Commands are:\n")); /*MSG*/
	for (cc = 0, cp = cmdtab; cp->c_name != NULL; cp++) {
		cc += strlen(cp->c_name) + 2;
		if (cc > 72) {
			printf("\n");
			cc = strlen(cp->c_name) + 2;
		}
		if ((cp+1)->c_name != NULLSTR)
			printf("%s, ", cp->c_name);
		else
			printf("%s\n", cp->c_name);
	}
	return(0);
}

/*
 * Paginate messages, honor ignored fields.
 */
more(msgvec)
	int *msgvec;
{
	return (type1(msgvec, 1, 1));
}

/*
 * Paginate messages, even printing ignored fields.
 */
More(msgvec)
	int *msgvec;
{

	return (type1(msgvec, 0, 1));
}

/*
 * Type out messages, honor ignored fields.
 */
type(msgvec)
	int *msgvec;
{

	return(type1(msgvec, 1, 0));
}

/*
 * Type out messages, even printing ignored fields.
 */
Type(msgvec)
	int *msgvec;
{

	return(type1(msgvec, 0, 0));
}

/*
 * Type out the messages requested.
 */
jmp_buf	pipestop;

type1(msgvec, doign, page)
	int *msgvec;
{
	register *ip;
	register struct message *mp;
	register int mesg;
	register char *cp;
	int c, nlines;
	FILE *ibuf, *obuf;

	obuf = stdout;
	if (setjmp(pipestop)) {
		if (obuf != stdout) {
			pipef = NULL;
			pclose(obuf);
		}
		signal(SIGPIPE, SIG_DFL);
		return(0);
	}
	if (intty && outtty && (page || (cp = value("crt")) != NULLSTR)) {
		nlines = 0;
		if (!page) {
			for (ip = msgvec; *ip && ip-msgvec < msgCount; ip++)
				nlines += message[*ip - 1].m_lines;
		}
		if (page || nlines > atoi(cp)) {
			cp = value("PAGER");
			if (cp == NULL || *cp == '\0')
				cp = MORE;
			obuf = popen(cp, "w");
			if (obuf == NULL) {
				perror(cp);
				obuf = stdout;
			}
			else {
				pipef = obuf;
				signal(SIGPIPE, brokpipe);
			}
		}
	}
	for (ip = msgvec; *ip && ip-msgvec < msgCount; ip++) {
		mesg = *ip;
		touch(mesg);
		mp = &message[mesg-1];
		dot = mp;
		print(mp, obuf, doign);
	}
	if (obuf != stdout) {
		pipef = NULL;
		pclose(obuf);
	}
	signal(SIGPIPE, SIG_DFL);
	return(0);
}

/*
 * Respond to a broken pipe signal --
 * probably caused by using quitting more.
 */

void
brokpipe()
{
	longjmp(pipestop, 1);
}

/*
 * Print the indicated message on standard output.
 */

print(mp, obuf, doign)
	register struct message *mp;
	FILE *obuf;
{

	if (value("quiet") == NULLSTR)
		fprintf(obuf, MSGSTR(MESGNUM, "Message %2d:\n"), mp - &message[0] + 1); /*MSG*/
	touch(mp - &message[0] + 1);
#ifdef ASIAN_I18N
	send(mp, obuf, doign, 1, NOCHANGE);
#else
	send(mp, obuf, doign);
#endif
}

/*
 * Print the top so many lines of each desired message.
 * The number of lines is taken from the variable "toplines"
 * and defaults to 5.
 */

top(msgvec)
	int *msgvec;
{
	register int *ip;
	register struct message *mp;
	register int mesg;
	int c, topl, lines, lineb;
	char *valtop, linebuf[LINESIZE];
	FILE *ibuf;

	topl = 5;
	valtop = value("toplines");
	if (valtop != NULLSTR) {
		topl = atoi(valtop);
		if (topl < 0 || topl > 10000)
			topl = 5;
	}
	lineb = 1;
	for (ip = msgvec; *ip && ip-msgvec < msgCount; ip++) {
		mesg = *ip;
		touch(mesg);
		mp = &message[mesg-1];
		dot = mp;
		if (value("quiet") == NULLSTR)
			printf(MSGSTR(MESGNUM, "Message %2d:\n"), mesg); /*MSG*/
		ibuf = setinput(mp);
		c = mp->m_lines;
		if (!lineb)
			printf("\n");
		for (lines = 0; lines < c && lines < topl; lines++) {
			if (readline(ibuf, linebuf) <= 0)
				break;
			puts(linebuf);
			lineb = blankline(linebuf);
		}
	}
	return(0);
}

/*
 * Touch all the given messages so that they will
 * get mboxed.
 */

stouch(msgvec)
	int msgvec[];
{
	register int *ip;

	for (ip = msgvec; *ip != 0; ip++) {
		dot = &message[*ip-1];
		dot->m_flag |= MTOUCH;
		dot->m_flag &= ~MPRESERVE;
	}
	return(0);
}

/*
 * Make sure all passed messages get mboxed.
 */

mboxit(msgvec)
	int msgvec[];
{
	register int *ip;

	for (ip = msgvec; *ip != 0; ip++) {
		dot = &message[*ip-1];
		dot->m_flag |= MTOUCH|MBOX;
		dot->m_flag &= ~MPRESERVE;
	}
	return(0);
}

/*
 * List the folders the user currently has.
 */
folders()
{
	char dirname[BUFSIZ], cmd[BUFSIZ];
	int pid, s, e;

	if (getfold(dirname) < 0) {
		printf(MSGSTR(NOVALUE, "No value set for \"folder\"\n")); /*MSG*/
		return(-1);
	}

	/* change from fork/exec to system() using LS - for SVID-2 */

	sprintf(cmd, "%s %s", LS, dirname);	/* create cmd string */
	return(system(cmd));			/* execute & return status */
}
