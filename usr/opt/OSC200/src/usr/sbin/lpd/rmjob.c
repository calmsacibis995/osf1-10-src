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
static char	*sccsid = "@(#)$RCSfile: rmjob.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:11:47 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * rmjob.c	5.3 (Berkeley) 6/30/88
 * rmjob.c	4.1 15:58:53 7/19/90 SecureWare 
 */


/*
 * rmjob - remove the specified jobs from the queue.
 */

#include "lp.h"

#if SEC_MAC
#include <sys/security.h>
#include <sys/access.h>

extern priv_t	*privvec();
#endif

/*
 * Stuff for handling lprm specifications
 */
extern char	*user[];		/* users to process */
extern int	users;			/* # of users in user array */
extern int	requ[];			/* job number of spool entries */
extern int	requests;		/* # of spool requests */
extern char	*person;		/* name of person doing lprm */

char	root[] = "root";
int	all = 0;		/* eliminate all files (root only) */
int	cur_daemon;		/* daemon's pid */
char	current[40];		/* active control file name */

int	iscf();

rmjob()
{
	register int i, nitems;
	int assasinated = 0;
	struct dirent **files;

	if ((i = pgetent(line, printer)) < 0)
		fatal(MSGSTR(RMJOB_1, "cannot open printer description file"));
	else if (i == 0)
		fatal(MSGSTR(RMJOB_2, "unknown printer"));
	if ((SD = pgetstr("sd", &bp)) == NULL)
		SD = DEFSPOOL;
	if ((LO = pgetstr("lo", &bp)) == NULL)
		LO = DEFLOCK;
	if ((LP = pgetstr("lp", &bp)) == NULL)
		LP = DEFDEVLP;
	if ((RP = pgetstr("rp", &bp)) == NULL)
		RP = DEFLP;
	RM = pgetstr("rm", &bp);

	/*
	 * If the format was `lprm -' and the user isn't the super-user,
	 *  then fake things to look like he said `lprm user'.
	 */
	if (users < 0) {
#if SEC_BASE
		/*
		 * Only allow to users with lp auth and when the
		 * request originated locally (from lprm, not lpd).
		 */
		if (from == host && hasrmauth())
#else /* !SEC_BASE */
		if (getuid() == 0)
#endif /* !SEC_BASE */
			all = 1;	/* all files in local queue */
		else {
			user[0] = person;
			users = 1;
		}
	}
	if (!strcmp(person, "-all")) {
		if (from == host)
			fatal(MSGSTR(RMJOB_3, "The login name \"-all\" is reserved"));
		all = 1;	/* all those from 'from' */
#if !SEC_BASE
		person = root;
#endif
	}

	if (chdir(SD) < 0)
		fatal(MSGSTR(RMJOB_4, "cannot chdir to spool directory"));
#if SEC_MAC
	/*
	 * If we are being called locally and we don't have the
	 * lp authorization, drop the MAC override privilege to
	 * allow iscf() to perform a meaningful access check.
	 */
	if (from == host && !hasrmauth()) {
		privvec_t	saveprivs;

		disableprivs(privvec(SEC_ALLOWMACACCESS, -1), saveprivs);
		if ((nitems = scandir(".", &files, iscf, NULL)) < 0)
			fatal("cannot access spool directory");
		seteffprivs(saveprivs, (priv_t *) 0);
	} else
#endif
	if ((nitems = scandir(".", &files, iscf, NULL)) < 0)
		fatal(MSGSTR(RMJOB_6, "cannot access spool directory"));

	if (nitems) {
		/*
		 * Check for an active printer daemon (in which case we
		 *  kill it if it is reading our file) then remove stuff
		 *  (after which we have to restart the daemon).
		 */
		if (lockchk(LO) && chk(current)) {
			assasinated = kill(cur_daemon, SIGINT) == 0;
			if (!assasinated)
				fatal(MSGSTR(RMJOB_7, "cannot kill printer daemon"));
			/* wait for daemon to die */
			for(i=5; i >= 0;i--)
			    if (kill(cur_daemon, 0) < 0)
				break;	/* no daemon */
			    else
				sleep(3);
			/* see if it is really wedged */
			if (kill(cur_daemon, 0) == 0)
				kill(cur_daemon, SIGKILL);
		}
		/*
		 * process the files
		 */
		for (i = 0; i < nitems; i++)
			process(files[i]->d_name);
	}
	chkremote();
	/*
	 * Restart the printer daemon if it was killed
	 */
	if (assasinated && !startdaemon(printer))
		fatal(MSGSTR(RMJOB_8, "cannot restart printer daemon\n"));
	exit(0);
}

/*
 * Process a lock file: collect the pid of the active
 *  daemon and the file name of the active spool entry.
 * Return boolean indicating existence of a lock file.
 */
lockchk(s)
	char *s;
{
	register FILE *fp;
	register int i, n;

	if ((fp = fopen(s, "r")) == NULL)
		if (errno == EACCES)
			fatal(MSGSTR(RMJOB_9, "can't access lock file"));
		else
			return(0);
	if (!getline(fp)) {
		(void) fclose(fp);
		return(0);		/* no daemon present */
	}
	cur_daemon = atoi(line);
	if (kill(cur_daemon, 0) < 0) {
		(void) fclose(fp);
		return(0);		/* no daemon present */
	}
	for (i = 1; (n = fread(current, sizeof(char), sizeof(current), fp)) <= 0; i++) {
		if (i > 5) {
			n = 1;
			break;
		}
		sleep(i);
	}
	current[n-1] = '\0';
	(void) fclose(fp);
	return(1);
}

/*
 * Process a control file.
 */
process(file)
	char *file;
{
	FILE *cfp;

	if (!chk(file))
		return;
	if ((cfp = fopen(file, "r")) == NULL)
		fatal(MSGSTR(RMJOB_10, "cannot open %s"), file);
	while (getline(cfp)) {
		switch (line[0]) {
		case 'U':  /* unlink associated files */
			if (from != host)
				printf("%s: ", host);
			if (unlink(line+1))
			    fprintf(stderr, 
			    MSGSTR(RMJOB_11, "cannot dequeue %s\n"), line+1);
			else
			    printf(MSGSTR(RMJOB_12, "%s dequeued\n"),
				   line+1);
		}
	}
	(void) fclose(cfp);
	if (from != host)
		printf("%s: ", host);
	if (unlink(file))
	    fprintf(stderr, MSGSTR(RMJOB_11, "cannot dequeue %s\n"), file);
	else
	    printf(MSGSTR(RMJOB_12, "%s dequeued\n"), file);
}

/*
 * Do the dirty work in checking
 */
chk(file)
	char *file;
{
	register int *r, n;
	register char **u, *cp;
	FILE *cfp;

	/*
	 * Check for valid cf file name (mostly checking current).
	 */
	if (strlen(file) < 7 || file[0] != 'c' || file[1] != 'f')
		return(0);

	if (isdigit(*(file+3)))
		cp = file + 6;
	else
		cp = file + 7;
	if (all && (from == host || !strcmp(from, cp)))
		return(1);

	/*
	 * get the owner's name from the control file.
	 */
	if ((cfp = fopen(file, "r")) == NULL)
		return(0);
	while (getline(cfp)) {
		if (line[0] == 'P')
			break;
	}
	(void) fclose(cfp);
	if (line[0] != 'P')
		return(0);

	if (users == 0 && requests == 0)
		return(!strcmp(file, current) && isowner(line+1, file));
	/*
	 * Check the request list
	 */
	if (isdigit(*(file+3)))
		cp = file+3;
	else
		cp = file+4;
	for (n = 0; isdigit(*cp); )
		n = n * 10 + (*cp++ - '0');
	for (r = requ; r < &requ[requests]; r++)
		if (*r == n && isowner(line+1, file))
			return(1);
	/*
	 * Check to see if it's in the user list
	 */
	for (u = user; u < &user[users]; u++)
		if (!strcmp(*u, line+1) && isowner(line+1, file))
			return(1);
	return(0);
}

/*
 * If root is removing a file on the local machine, allow it.
 * If root is removing a file from a remote machine, only allow
 * files sent from the remote machine to be removed.
 * Normal users can only remove the file from where it was sent.
 */
isowner(owner, file)
	char *owner, *file;
{
	register char *cp;

	if (isdigit(*(file + 3)))
		cp = file + 6;
	else
		cp = file + 7;
#if SEC_BASE
	/*
	 * If we are being called locally by lprm (rather than by lpd),
	 * person and owner must match or the caller must have the
	 * lp authorization.
	 * If we are being called remotely by lpd, then the client
	 * host must match the filename and person must be "-all"
	 * or match owner.
	 */
	if (from == host) {
		if (!strcmp(person, owner) || hasrmauth())
			return 1;
	} else {
		if (!strcmp(from, cp) &&
		    (!strcmp(person, owner) || !strcmp(person, "-all")))
			return 1;
	}
#else
	if (!strcmp(person, root) && (from == host || !strcmp(from, cp)))
		return(1);
	if (!strcmp(person, owner) && !strcmp(from, cp))
		return(1);
#endif
	if (from != host)
		printf("%s: ", host);
	/* record these access failures */
	syslog(LOG_LPR | LOG_INFO,MSGSTR(RMJOB_13, "[%s] %s: permission denied\n"),host,file);
	fprintf(stderr, MSGSTR(RMJOB_14, "%s: Permission denied\n"), file);
	return(0);
}

/*
 * Check to see if we are sending files to a remote machine. If we are,
 * then try removing files on the remote machine.
 */
chkremote()
{
	register char *cp;
	register int i, rem;
	char buf[BUFSIZ];

	if (*LP || RM == NULL)
		return;	/* not sending to a remote machine */

	/*
	 * Flush stdout so the user can see what has been deleted
	 * while we wait (possibly) for the connection.
	 */
	fflush(stdout);

	sprintf(buf, "\5%s %s", RP, all ? "-all" : person);
	cp = buf;
	for (i = 0; i < users; i++) {
		cp += strlen(cp);
		*cp++ = ' ';
		strcpy(cp, user[i]);
	}
	for (i = 0; i < requests; i++) {
		cp += strlen(cp);
		(void) sprintf(cp, " %d", requ[i]);
	}
	strcat(cp, "\n");
	rem = getport(RM);
	if (rem < 0) {
		if (from != host)
			printf("%s: ", host);
		printf(MSGSTR(RMJOB_15, "connection to %s is down\n"), RM);
	} else {
		i = strlen(buf);
		if (write(rem, buf, i) != i)
			fatal(MSGSTR(RMJOB_16, "Lost connection"));
		while ((i = read(rem, buf, sizeof(buf))) > 0)
			(void) fwrite(buf, 1, i, stdout);
		(void) close(rem);
	}
}

/*
 * Return 1 if the filename begins with 'cf'
 */
iscf(d)
	struct dirent *d;
{
#if SEC_MAC
	/*
	 * If the user doesn't have lp authorization, select only
	 * jobs at the current sensitivity level.  We lowered all
	 * MAC override privileges before calling scandir, so an
	 * access check for read/write tells the story.
	 */
	if (from == host && !hasrmauth() && eaccess(d->d_name, R_OK|W_OK))
		return 0;
#endif
	return(d->d_name[0] == 'c' && d->d_name[1] == 'f');
}

#if SEC_BASE
/*
 * Check user's authorization to remove other users' jobs.
 * This function avoids multiple calls to authorized_user() to
 * prevent redundant auditing.
 */
hasrmauth()
{
	static int	hasauth = -1;

	if (hasauth == -1)
		hasauth = authorized_user("lp");
	return hasauth;
}
#endif
