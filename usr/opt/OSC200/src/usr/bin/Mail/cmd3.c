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
static char	*sccsid = "@(#)$RCSfile: cmd3.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/09/29 13:26:47 $";
#endif 
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/* 
 * COMPONENT_NAME: CMDMAILX cmd3.c
 * 
 * FUNCTIONS: MSGSTR, Respond, _Respond, _respond, alternates, 
 *            bangexp, diction, dosh, echo, elsecmd, endifcmd, file, 
 *            getfilename, group, help, ifcmd, messize, null, 
 *            preserve, reedit, respond, rexit, schdir, set, shell, 
 *            sort, unread, unset 
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
 *	cmd3.c       5.3 (Berkeley) 9/15/85
 *
 * Modification History
 *
 * 01 gaudet Tue Nov 05 19:02:33 EST 1991
 *	Changed the local subroutine pipe() to mpipe() because it
 *	interfered with pipe(2). Caused shell variable evaluation to
 *	fail and Mail/mailx to exit. Fixes OSF_QAR #01665.
 */

#include "rcv.h"
#include <sys/stat.h>

#include "Mail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s) 

char *helpmsg[] = {
"Control Commands:",
"   q                       Quit - apply mailbox commands entered this session.",
"   x                       Quit - restore mailbox to original state.",
"   ! <cmd>                 Start a shell, run <cmd>, and return to mailbox.",
"   cd [<dir>]              Change directory to <dir> or $HOME.",
"Display Commands:",
"   t [<msg_list>]          Display messages in <msg_list> or current message.",
"   n                       Display next message.",
"   f [<msg_list>]          Display headings of messages.",
"   h [<num>]               Display headings of group containing message <num>.",
"Message Handling:",
"   e [<num>]               Edit message <num> (default editor is e).",
"   d [<msg_list>]          Delete messages in <msg_list> or current message.",
"   u [<msg_list>]          Recall deleted messages.",
"   s [<msg_list>] <file>   Append messages (with headings) to <file>.",
"   w [<msg_list>] <file>   Append messages (text only) to <file>.",
"   pre [<msg_list>]        Keep messages in system mailbox.",
"Creating New Mail:",
"   m <addrlist>            Create/send new message to addresses in <addrlist>.",
"   r [<msg_list>]          Send reply to senders and recipients of messages.",
"   R [<msg_list>]          Send reply only to senders of messages.",
"   a                       Display list of aliases and their addresses.",
"=============================== Mailbox Commands ==============================",
NULL
};

/*
 * Mail -- a mail program
 *
 * Still more user commands.
 */

/*
 * Process a shell escape by saving signals, ignoring signals,
 * and forking a sh -c
 */

shell(str)
	char *str;
{
	void (*sig[2])(); 
	int stat[1];
	register int t;
	char *Shell;
	char cmd[BUFSIZ];

	strcpy(cmd, str);
	if (bangexp(cmd) < 0)
		return(-1);
	if ((Shell = value("SHELL")) == NULLSTR)
		Shell = SHELL;
	for (t = 2; t < 4; t++)
		sig[t-2] = signal(t, SIG_IGN);
	t = vfork();
	if (t == 0) {
		sigchild();
		for (t = 2; t < 4; t++)
			if (sig[t-2] != SIG_IGN)
				signal(t, SIG_DFL);
		execl(Shell, Shell, "-c", cmd, (char *)0);
		perror(Shell);
		_exit(1);
	}
	while (wait(stat) != t)
		;
	if (t == -1)
		perror("fork");
	for (t = 2; t < 4; t++)
		signal(t, sig[t-2]);
	printf("!\n");
	return(0);
}

/*
 * Fork an interactive shell.
 */

dosh(str)
	char *str;
{
	void (*sig[2])();
	int stat[1];
	register int t;
	char *Shell;
	if ((Shell = value("SHELL")) == NULLSTR)
		Shell = SHELL;
	for (t = 2; t < 4; t++)
		sig[t-2] = signal(t, SIG_IGN);
	t = vfork();
	if (t == 0) {
		sigchild();
		for (t = 2; t < 4; t++)
			if (sig[t-2] != SIG_IGN)
				signal(t, SIG_DFL);
		execl(Shell, Shell, (char *)0);
		perror(Shell);
		_exit(1);
	}
	while (wait(stat) != t)
		;
	if (t == -1)
		perror("fork");
	for (t = 2; t < 4; t++)
		signal(t, sig[t-2]);
	putchar('\n');
	return(0);
}

/*
 * Expand the shell escape by expanding unescaped !'s into the
 * last issued command where possible.
 */

char	lastbang[128];

bangexp(str)
	char *str;
{
	char bangbuf[BUFSIZ];
	register char *cp, *cp2;
	register int n;
	int changed = 0;
        int bangit = (value("bang")!=NULLSTR);	/* added for SVID-2 */

	cp = str;
	cp2 = bangbuf;
	n = BUFSIZ;
	while (*cp) {
		if (*cp == '!' && bangit) {	/* modified for SVID-2 */
			if (n < strlen(lastbang)) {
overf:
				printf(MSGSTR(OVER, "Command buffer overflow\n")); /*MSG*/
				return(-1);
			}
			changed++;
			strcpy(cp2, lastbang);
			cp2 += strlen(lastbang);
			n -= strlen(lastbang);
			cp++;
			continue;
		}
		if (*cp == '\\' && cp[1] == '!') {
			if (--n <= 1)
				goto overf;
			*cp2++ = '!';
			cp += 2;
			changed++;
		}
		if (--n <= 1)
			goto overf;
		*cp2++ = *cp++;
	}
	*cp2 = 0;
	if (changed) {
		printf("!%s\n", bangbuf);
		fflush(stdout);
	}
	strcpy(str, bangbuf);
	strncpy(lastbang, bangbuf, 128);
	lastbang[127] = 0;
	return(0);
}

/*
 * Print out a nice help message from some file or another.
 */

help()
{
    register int i;

    for (i = 0; helpmsg[i]; i++) 
	puts(MSGSTR(HELPMSG+i, helpmsg[i]));
    return(0);
}

/*
 * Change user's working directory.
 */

schdir(str)
	char *str;
{
	register char *cp;

	for (cp = str; *cp == ' '; cp++)
		;
	if (*cp == '\0')
		cp = homedir;
	else
		if ((cp = expand(cp)) == NULLSTR)
			return(1);
	if (chdir(cp) < 0) {
		perror(cp);
		return(1);
	}
	return(0);
}

respond(msgvec)
	int *msgvec;
{
	if ((value("flipr") == NULLSTR)
			&& (value("Replyall") == NULLSTR))
		return (_respond(msgvec));
	else
		return (_Respond(msgvec));
}

/*
 * Reply to a list of messages.  Extract each name from the
 * message header and send them off to mail1()
 */

_respond(msgvec)
	int *msgvec;
{
	struct message *mp;
	char *cp, *cp2, *cp3, *rcv;
	char buf[2 * LINESIZE], **ap;
	struct name *np;
	struct header head;

	if (msgvec[1] != 0) {
		printf(MSGSTR(SORRY, "Sorry, can't reply to multiple messages at once\n")); /*MSG*/
		return(1);
	}
	mp = &message[msgvec[0] - 1];
	dot = mp;
	rcv = NULLSTR;
	cp = skin(nameof(mp, 1));
	if (cp != NULLSTR)
	    rcv = cp;
	if ((cp = skin(hfield("reply-to", mp))) == NULLSTR)
		cp = skin(hfield("from", mp));
	if (cp != NULLSTR)
	    rcv = cp;
	strcpy(buf, "");
	cp = skin(hfield("to", mp));
	if (cp != NULLSTR)
		strcpy(buf, cp);
	np = elide(extract(buf, GTO));
	mapf(np, rcv);
	/*
	 * Delete my name from the reply list,
	 * and with it, all my alternate names.
	 */
	np = delname(np, myname, icequal);
	if (altnames)
		for (ap = altnames; *ap; ap++)
			np = delname(np, *ap, icequal);
	head.h_seq = 1;
	cp = detract(np, 0);
	if (cp != NULLSTR) {
		strcpy(buf, cp);
		strcat(buf, " ");
		strcat(buf, rcv);
	}
	else {
		strcpy(buf, rcv);
	}
	head.h_to = buf;
	head.h_subject = hfield("subject", mp);
	if (head.h_subject == NULLSTR)
		head.h_subject = hfield("subj", mp);
	head.h_subject = reedit(head.h_subject);
	head.h_cc = NULLSTR;
	cp = hfield("cc", mp);
	if (cp != NULLSTR) {
		np = elide(extract(cp, GCC));
		mapf(np, rcv);
		np = delname(np, myname, icequal);
		if (altnames != 0)
			for (ap = altnames; *ap; ap++)
				np = delname(np, *ap, icequal);
		head.h_cc = detract(np, 0);
	}
	head.h_bcc = NULLSTR;
	mail1(&head, value("record"));
	return(0);
}


followup(msgvec)		/* added for SVID-2 */
int *msgvec;
{
	if (value("flipf") != NULLSTR) return(Resp1(msgvec, 1));
	else return(resp1(msgvec, 1));
}

resp1(msgvec, useauthor)		/* added for SVID-2 */
int *msgvec;
{
	char recfile[128];
	struct message *mp;
	char *cp, buf[2 * LINESIZE], *rcv, **ap;
	struct name *np;
	struct header head;
	char *netmap();
	char *unuucp();
	char mylocalname[100], mydomname[200];
	char myuucpname[100];
	extern char host[], domain[];
	char *fromline;

	if (msgvec[1] != 0) {
		printf(MSGSTR(SORRY,
		"Sorry, can't reply to multiple messages at once\n"));
		return(1);
	}

	strcpy(mylocalname, myname);
	strcat(mylocalname, "@");
	strcpy(mydomname, mylocalname);
	strcat(mylocalname, host);
	strcat(mydomname, domain);

	sprintf(myuucpname, "%s!%s", host, myname);

	mp = &message[msgvec[0] - 1];
	dot = mp;

	rcv = skin(nameof(mp,1));

	getrecf(rcv, recfile, useauthor);
	strcpy(buf, "");
	cp = skin(hfield("to", mp));
	if (cp != NULLSTR)
		strcpy(buf, cp);
	np = elide(extract(buf, GTO));
#ifdef	OPTIM
	/* rcv = rename(rcv); */
#endif	/* OPTIM */
	/*
	 * Delete my name from the reply list,
	 * and with it, all my alternate names.
	 */
	np = delname(np, myname, icequal); /*GAG*/
	np = delname(np, mylocalname, icequal); /*GAG*/
	np = delname(np, mydomname, icequal); /*GAG*/
	np = delname(np, myuucpname, icequal); /*GAG*/
	if (altnames != 0)
		for (ap = altnames; *ap; ap++)
			np = delname(np, *ap, icequal); /*GAG*/
	mapf(np, rcv);
	head.h_seq = 1;
	cp = detract(np, 0);
	if (cp != NULLSTR) {
		strcpy(buf, cp);
		strcat(buf, " ");
		strcat(buf, unuucp(rcv));
	}
	else {
		strcpy(buf, unuucp(rcv));
	}
	head.h_to = buf;
	head.h_subject = hfield("subject", mp);
	if (head.h_subject == NULLSTR)
		head.h_subject = hfield("subj", mp);
	head.h_subject = reedit(head.h_subject);
	head.h_cc = NULLSTR;
	cp = hfield("cc", mp);
	if (cp != NULLSTR) {
		np = elide(extract(cp, GCC));
		mapf(np, rcv);
		np = delname(np, myname, icequal); /*GAG*/
		np = delname(np, mylocalname, icequal); /*GAG*/
		np = delname(np, mydomname, icequal); /*GAG*/
		if (altnames != 0)
			for (ap = altnames; *ap; ap++)
				np = delname(np, *ap, icequal); /*GAG*/
		head.h_cc = detract(np, 0);
	}
	head.h_bcc = NULLSTR;
	mail1(&head, recfile);
	return(0);
}

getrecf(buf, recfile, useauthor)		/* added for SVID-2 */
char *buf, *recfile;
{
	register char *bp, *cp;
	register char *recf = recfile;
	register int folderize = (value("outfolder")!=NULLSTR);

	if (useauthor) {
		if (folderize)
			*recf++ = '+';
		if (debug) fprintf(stderr, MSGSTR(BUF, "buf='%s'\n"), buf);
		for (bp=buf, cp=recf; bp && *bp && *bp!=' '; bp++)
			if (*bp=='!')
				cp = recf;
			else
				*cp++ = *bp;
		*cp = '\0';
		if (cp==recf)
			*recfile = '\0';
	} else {
		if (cp = value("record")) {
			if (folderize && *cp!='+' && *cp!='/')
				*recf++ = '+';
			strcpy(recf, cp);
		} else
			*recf = '\0';
	}
	if (debug) fprintf(stderr, MSGSTR(RECF,  "recfile='%s'\n"), recfile);
}



/*
 * Modify the subject we are replying to to begin with Re: if
 * it does not already.
 */

char *
reedit(subj)
	char *subj;
{
	char sbuf[10];
	register char *newsubj;

	if (subj == NULLSTR)
		return(NULLSTR);
	strncpy(sbuf, subj, 3);
	sbuf[3] = 0;
	if (icequal(sbuf, "re:"))
		return(subj);
	newsubj = salloc(strlen(subj) + 6);
	sprintf(newsubj, MSGSTR(RE, "Re:  %s"), subj); /*MSG*/
	return(newsubj);
}

/*
 * Preserve the named messages, so that they will be sent
 * back to the system mailbox.
 */

preserve(msgvec)
	int *msgvec;
{
	register struct message *mp;
	register int *ip, mesg;

	if (edit) {
		printf(MSGSTR(NOPRSRV, "Cannot \"preserve\" in edit mode\n")); /*MSG*/
		return(1);
	}
	for (ip = msgvec; *ip != NULL; ip++) {
		mesg = *ip;
		mp = &message[mesg-1];
		mp->m_flag |= MPRESERVE;
		mp->m_flag &= ~MBOX;
		dot = mp;
	}
	return(0);
}

/*
 * Mark all given messages as unread.
 */
unread(msgvec)
	int	msgvec[];
{
	register int *ip;

	for (ip = msgvec; *ip != NULL; ip++) {
		dot = &message[*ip-1];
		dot->m_flag &= ~(MREAD|MTOUCH);
		dot->m_flag |= MSTATUS;
	}
	return(0);
}

/*
 * Print the size of each message.
 */

messize(msgvec)
	int *msgvec;
{
	register struct message *mp;
	register int *ip, mesg;

	for (ip = msgvec; *ip != NULL; ip++) {
		mesg = *ip;
		mp = &message[mesg-1];
		printf("%d: %d/%ld\n", mesg, mp->m_lines, mp->m_size);
	}
	return(0);
}

/*
 * Quit quickly.  If we are sourcing, just pop the input level
 * by returning an error.
 */

rexit(e)
{
	if (sourcing)
		return(1);
	if (Tflag != NULLSTR)
		close(creat(Tflag, 0600));
	exit(e);
}

/*
 * Set or display a variable value.  Syntax is similar to that
 * of csh.
 */

set(arglist)
	char **arglist;
{
	register struct var *vp;
	register char *cp, *cp2;
	char varbuf[BUFSIZ], **ap, **p;
	int errs, h, s;

	if (argcount(arglist) == 0) {
		for (h = 0, s = 1; h < HSHSIZE; h++)
			for (vp = variables[h]; vp != NOVAR; vp = vp->v_link)
				s++;
		ap = (char **) salloc(s * sizeof *ap);
		for (h = 0, p = ap; h < HSHSIZE; h++)
			for (vp = variables[h]; vp != NOVAR; vp = vp->v_link)
				*p++ = vp->v_name;
		*p = NULLSTR;
		sort(ap);
		for (p = ap; *p != NULLSTR; p++)
			printf("%s\t%s\n", *p, value(*p));
		return(0);
	}
	errs = 0;
	for (ap = arglist; *ap != NULLSTR; ap++) {
		cp = *ap;
		cp2 = varbuf;
		while (*cp != '=' && *cp != '\0')
			*cp2++ = *cp++;
		*cp2 = '\0';
		if (*cp == '\0')
			cp = "";
		else
			cp++;
		if (equal(varbuf, "")) {
			printf(MSGSTR(NEWNM, "Non-null variable name required\n")); /*MSG*/
			errs++;
			continue;
		}
		assign(varbuf, cp);

		/* handle special case of setting debug var */
		if (! strcmp(varbuf, "debug"))
		    debug = 1;
	}
	return(errs);
}

/*
 * Unset a bunch of variable values.
 */

unset(arglist)
	char **arglist;
{
	register struct var *vp, *vp2;
	register char *cp;
	int errs, h;
	char **ap;

	errs = 0;
	for (ap = arglist; *ap != NULLSTR; ap++) {
		if ((vp2 = lookup(*ap)) == NOVAR) {
			if (!sourcing) {
				printf(MSGSTR(UNDEF, "\"%s\": undefined variable\n"), *ap); /*MSG*/
				errs++;
			}
			continue;
		}

		/* handle special case of resetting debug var */
		if (! strcmp(*ap, "debug"))
		    debug = 0;

		h = hash(*ap);
		if (vp2 == variables[h]) {
			variables[h] = variables[h]->v_link;
			vfree(vp2->v_name);
			vfree(vp2->v_value);
			cfree(vp2);
			continue;
		}
		for (vp = variables[h]; vp->v_link != vp2; vp = vp->v_link)
			;
		vp->v_link = vp2->v_link;
		vfree(vp2->v_name);
		vfree(vp2->v_value);
		cfree(vp2);
	}
	return(errs);
}

/*
 * Put add users to a group.
 */

group(argv)
	char **argv;
{
	register struct grouphead *gh;
	register struct group *gp;
	register int h;
	int s;
	char **ap, *gname, **p;

	if (argcount(argv) == 0) {
		for (h = 0, s = 1; h < HSHSIZE; h++)
			for (gh = groups[h]; gh != NOGRP; gh = gh->g_link)
				s++;
		ap = (char **) salloc(s * sizeof *ap);
		for (h = 0, p = ap; h < HSHSIZE; h++)
			for (gh = groups[h]; gh != NOGRP; gh = gh->g_link)
				*p++ = gh->g_name;
		*p = NULLSTR;
		sort(ap);
		for (p = ap; *p != NULLSTR; p++)
			printgroup(*p);
		return(0);
	}
	if (argcount(argv) == 1) {
		printgroup(*argv);
		return(0);
	}
	gname = *argv;
	h = hash(gname);
	if ((gh = findgroup(gname)) == NOGRP) {
		gh = (struct grouphead *) calloc(sizeof *gh, 1);
		gh->g_name = vcopy(gname);
		gh->g_list = NOGE;
		gh->g_link = groups[h];
		groups[h] = gh;
	}

	/*
	 * Insert names from the command list into the group.
	 * Who cares if there are duplicates?  They get tossed
	 * later anyway.
	 */

	for (ap = argv+1; *ap != NULLSTR; ap++) {
		gp = (struct group *) calloc(sizeof *gp, 1);
		gp->ge_name = vcopy(*ap);
		gp->ge_link = gh->g_list;
		gh->g_list = gp;
	}
	return(0);
}

/*
 * Remove specified groups/aliases.
 */

unalias(argv)
	char **argv;
{
        register struct grouphead *gh, *prev;
        register struct group *gp;
        register int h;
        int s;
        char **ap, *gname, **p;

        if (argcount(argv) == 0) {
                for (h = 0, s = 1; h < HSHSIZE; h++)
                        for (gh = groups[h]; gh != NOGRP; gh = gh->g_link)
                                s++;
                ap = (char **) salloc(s * sizeof *ap);
                for (h = 0, p = ap; h < HSHSIZE; h++)
                        for (gh = groups[h]; gh != NOGRP; gh = gh->g_link)
                                *p++ = gh->g_name;
                *p = NULLSTR;
                sort(ap);

                for (p = ap; *p != NULLSTR; p++)
                        printgroup(*p);
                return(0);
        }
        for (ap = argv; *ap != NULLSTR; ap++) {
                h = hash(*ap);
                prev = groups[h];
                for (gh = groups[h]; gh != NOGRP; prev = gh, gh = gh->g_link)
                        if (equal(gh->g_name, *ap)) {
                                if (gh != groups[h])
                                        prev->g_link = gh->g_link;
                                else
                                        groups[h] = gh->g_link;
                                gh->g_link = NULL;
                        }
        }
        return(0);
}


/*
 * Sort the passed string vecotor into ascending dictionary
 * order.
 */

sort(list)
	char **list;
{
	register char **ap;
	int diction();

	for (ap = list; *ap != NULLSTR; ap++)
		;
	if (ap-list < 2)
		return;
	qsort(list, ap-list, sizeof *list, diction);
}

/*
 * Do a dictionary order comparison of the arguments from
 * qsort.
 */

diction(a, b)
	register char **a, **b;
{
	return(strcmp(*a, *b));
}

/*
 * The do nothing command for comments.
 */

null(e)
{
	return(0);
}

/*
 * Print out the current edit file, if we are editing.
 * Otherwise, print the name of the person who's mail
 * we are reading.
 */

file(argv)
	char **argv;
{
	register char *cp;
	char fname[BUFSIZ];
	int edit;

	if (argv[0] == NULLSTR) {
		newfileinfo();
		return(0);
	}

	/*
	 * Acker's!  Must switch to the new file.
	 * We use a funny interpretation --
	 *	# -- gets the previous file
	 *	% -- gets the invoker's post office box
	 *	%user -- gets someone else's post office box
	 *	& -- gets invoker's mbox file
	 *	string -- reads the given file
	 */

	cp = getfilename(argv[0], &edit);
	if (cp == NULLSTR)
		return(-1);
	if (setfile(cp, edit)) {
		perror(cp);
		return(-1);
	}
	announce(0);
}

/*
 * Evaluate the string given as a new mailbox name.
 * Ultimately, we want this to support a number of meta characters.
 * Possibly:
 *	% -- for my system mail box
 *	%user -- for user's system mail box
 *	# -- for previous file
 *	& -- get's invoker's mbox file
 *	file name -- for any other file
 */

char	prevfile[PATHSIZE];

char *
getfilename(name, aedit)
	char *name;
	int *aedit;
{
	register char *cp;
	char savename[BUFSIZ];
	char oldmailname[BUFSIZ];

	/*
	 * Assume we will be in "edit file" mode, until
	 * proven wrong.
	 */
	*aedit = 1;
	switch (*name) {
	case '%':
		*aedit = 0;
		strcpy(prevfile, mailname);
		if (name[1] != 0) {
			strcpy(savename, myname);
			strcpy(oldmailname, mailname);
			strncpy(myname, name+1, PATHSIZE-1);
			myname[PATHSIZE-1] = 0;
			findmail();
			cp = savestr(mailname);
			strcpy(myname, savename);
			strcpy(mailname, oldmailname);
			return(cp);
		}
		strcpy(oldmailname, mailname);
		findmail();
		cp = savestr(mailname);
		strcpy(mailname, oldmailname);
		return(cp);

	case '#':
		if (name[1] != 0)
			goto regular;
		if (prevfile[0] == 0) {
			printf(MSGSTR(NOPREV, "No previous file\n")); /*MSG*/
			return(NULLSTR);
		}
		cp = savestr(prevfile);
		strcpy(prevfile, mailname);
		return(cp);

	case '&':
		strcpy(prevfile, mailname);
		if (name[1] == 0)
			return(mbox);
		/* Fall into . . . */

	default:
regular:
		strcpy(prevfile, mailname);
		cp = expand(name);
		return(cp);
	}
}

/*
 * Expand file names like echo
 */

echo(argv)
	char **argv;
{
	register char **ap;
	register char *cp;

	for (ap = argv; *ap != NULLSTR; ap++) {
		cp = *ap;
		if ((cp = expand(cp)) != NULLSTR)
			printf("%s ", cp);
	}
	if (ap != argv)  /* print newline if we echoed anything */
		putchar('\n');
	return(0);
}

Respond(msgvec)
	int *msgvec;
{
	if ((value("flipr") == NULLSTR)
			&& (value("Replyall") == NULLSTR))
		return (_Respond(msgvec));
	else
		return (_respond(msgvec));
}

/*
 * Reply to a series of messages by simply mailing to the senders
 * and not messing around with the To: and Cc: lists as in normal
 * reply.
 */

_Respond(msgvec)
	int msgvec[];
{
	struct header head;
	struct message *mp;
	register int i, s, *ap;
	register char *cp, *cp2, *subject;

	for (s = 0, ap = msgvec; *ap != 0; ap++) {
		mp = &message[*ap - 1];
		dot = mp;
		if ((cp = skin(hfield("reply-to", mp))) == NULLSTR)
			cp = skin(hfield("from", mp));
		if (cp != NULLSTR)
		    s+= strlen(cp) + 1;
		else
		    s += strlen(skin(nameof(mp, 2))) + 1;
	}
	if (s == 0)
		return(0);
	cp = salloc(s + 2);
	head.h_to = cp;
	for (ap = msgvec; *ap != 0; ap++) {
		mp = &message[*ap - 1];
		if ((cp2 = skin(hfield("reply-to", mp))) == NULLSTR)
			cp2 = skin(hfield("from", mp));
		if (cp2 == NULLSTR)
		    cp2 = skin(nameof(mp, 2));
		cp = copy(cp2, cp);
		*cp++ = ' ';
	}
	*--cp = 0;
	mp = &message[msgvec[0] - 1];
	subject = hfield("subject", mp);
	head.h_seq = 0;
	if (subject == NULLSTR)
		subject = hfield("subj", mp);
	head.h_subject = reedit(subject);
	if (subject != NULLSTR)
		head.h_seq++;
	head.h_cc = NULLSTR;
	head.h_bcc = NULLSTR;
	mail1(&head, value("record"));
	return(0);
}

Followup(msgvec)		/* added for SVID-2 */
int *msgvec;
{
	if (value("flipf") != NULLSTR) return(resp1(msgvec, 1));
	else return(Resp1(msgvec, 1));
}

Resp1(msgvec, useauthor)		/* added for SVID-2 */
int *msgvec;
{
	char recfile[128];
	struct header head;
	struct message *mp;
	register int s, *ap;
	register char *cp, *cp2, *subject;

	for (s = 0, ap = msgvec; *ap != 0; ap++) {
		mp = &message[*ap - 1];
		dot = mp;
/*
	ehr3

		if ((cp = skin(hfield("from",mp))) != NULLSTR)
			s += strlen(cp) + 1;
		else 
			s += strlen(nameof(mp, 2)) + 1;
*/
		if ((cp = skin(nameof(mp, 2))) != NULLSTR)
			s += strlen(cp) + 1;
	}
	if (s == 0)
		return(0);
	cp = salloc(s + 2);
	head.h_to = cp;
	for (ap = msgvec; *ap != 0; ap++) {
		mp = &message[*ap - 1];
/*
	ehr3

		if ((cp2 = skin(hfield("from",mp))) == NULLSTR)
			cp2 = skin(nameof(mp,2));
*/
		cp2 = skin(nameof(mp,2));

		cp = copy(cp2,cp);
		*cp++ = ' ';
	}
	*--cp = 0;
	getrecf(head.h_to, recfile, useauthor);
	mp = &message[msgvec[0] - 1];
	subject = hfield("subject", mp);
	head.h_seq = 0;
	if (subject == NULLSTR)
		subject = hfield("subj", mp);
	head.h_subject = reedit(subject);
	if (subject != NULLSTR)
		head.h_seq++;
	head.h_cc = NULLSTR;
	head.h_bcc = NULLSTR;
	mail1(&head, recfile);
	return(0);
}

/*
 * Conditional commands.  These allow one to parameterize one's
 * .mailrc and do some things if sending, others if receiving.
 */

ifcmd(argv)
	char **argv;
{
	register char *cp;

	if (cond != CANY) {
		printf(MSGSTR(NESTED, "Illegal nested \"if\"\n")); /*MSG*/
		return(1);
	}
	cond = CANY;
	cp = argv[0];
	switch (*cp) {
	case 'r': case 'R':
		cond = CRCV;
		break;

	case 's': case 'S':
		cond = CSEND;
		break;

	default:
		printf(MSGSTR(NOKEYWD, "Unrecognized if-keyword: \"%s\"\n"), cp); /*MSG*/
		return(1);
	}
	return(0);
}

/*
 * Implement 'else'.  This is pretty simple -- we just
 * flip over the conditional flag.
 */

elsecmd()
{

	switch (cond) {
	case CANY:
		printf(MSGSTR(NOMATCH, "\"Else\" without matching \"if\"\n")); /*MSG*/
		return(1);

	case CSEND:
		cond = CRCV;
		break;

	case CRCV:
		cond = CSEND;
		break;

	default:
		printf(MSGSTR(WRONG, "Mail's idea of conditions is screwed up\n")); /*MSG*/
		cond = CANY;
		break;
	}
	return(0);
}

/*
 * End of if statement.  Just set cond back to anything.
 */

endifcmd()
{

	if (cond == CANY) {
		printf(MSGSTR(NOMTCH, "\"Endif\" without matching \"if\"\n")); /*MSG*/
		return(1);
	}
	cond = CANY;
	return(0);
}

/*
 * Set the list of alternate names.
 */
alternates(namelist)
	char **namelist;
{
	register int c;
	register char **ap, **ap2, *cp;

	c = argcount(namelist) + 1;
	if (c == 1) {
		if (altnames == 0)
			return(0);
		for (ap = altnames; *ap; ap++)
			printf("%s ", *ap);
		printf("\n");
		return(0);
	}
	if (altnames != 0)
		cfree((char *) altnames);
	altnames = (char **) calloc(c, sizeof (char *));
	for (ap = namelist, ap2 = altnames; *ap; ap++, ap2++) {
		cp = (char *) calloc(strlen(*ap) + 1, sizeof (char));
		strcpy(cp, *ap);
		*ap2 = cp;
	}
	*ap2 = 0;
	return(0);
}
