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
static char	*sccsid = "@(#)$RCSfile: recvjob.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/09/24 20:51:42 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
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
 * recvjob.c	5.7 (Berkeley) 6/30/88
 * recvjob.c	4.1 15:58:38 7/19/90 SecureWare 
 */


/*
 * Receive printer jobs from the network, queue them and
 * start the printer daemon.
 */

#include <sys/mount.h>
#include <sys/time.h>
#include "lp.h"

char	*sp = "";
#define ack()	(void) write(1, sp, 1);

char    tfname[40];		/* tmp copy of cf before linking */
char    dfname[40];		/* data files */
int	minfree;		/* keep at least minfree blocks available */

recvjob()
{
	struct stat stb;
	char *bp = pbuf;
	int status;
	void rcleanup();
	int i;

#if SEC_MAC
	frecverr("%s: remote job submission not supported", printer);
#else /* !SEC_MAC { */
	/*
	 * Perform lookup for printer name or abbreviation
	 */
	if ((status = pgetent(line, printer)) < 0)
		frecverr(MSGSTR(RECVJOB_1, "cannot open printer description file"));
	else if (status == 0)
		frecverr(MSGSTR(RECVJOB_2, "unknown printer %s"), printer);
	if ((LF = pgetstr("lf", &bp)) == NULL)
		LF = DEFLOGF;
	if ((SD = pgetstr("sd", &bp)) == NULL)
		SD = DEFSPOOL;
	if ((LO = pgetstr("lo", &bp)) == NULL)
		LO = DEFLOCK;

	if ((DU = pgetnum("du")) < 0)		/* get the daemon uid */
		DU = DEFUID;			/* 001 - plewa        */

	(void) close(2);			/* set up log file */
	/* be more careful about assigning logfile to stderr */
	if ((i = open(LF, O_WRONLY|O_APPEND, 0664)) < 0) {
		/* ignore log file open failures */
		if (errno != ENOENT)
			syslog(LOG_ERR, MSGSTR(RECVJOB_3, "%s: cannot open logfile %s: %m"),
				printer, LF);
		i = open("/dev/null", O_WRONLY);
	}
	if (i >= 0 && i != 2)
		dup2(i, 2);

	if (chdir(SD) < 0)
		frecverr(MSGSTR(RECVJOB_4, "%s: cannot chdir to %s: %m"), printer, SD);
	if (stat(LO, &stb) == 0)
		if (stb.st_mode & 010)		/* queue is disabled */
			frecverr(MSGSTR(RECVJOB_5, "%s: queue is disabled."), printer);
	minfree = read_number("minfree");

	signal(SIGTERM, rcleanup);
	signal(SIGPIPE, rcleanup);

	if (readjob())
		printjob();
#endif /* !SEC_MAC } */
}

/*
 * Read printer jobs sent by lpd and copy them to the spooling directory.
 * Return the number of jobs successfully transfered.
 */
readjob()
{
	register int size, nfiles;
	register char *cp;
	void rcleanup();

	ack();
	nfiles = 0;
	for (;;) {
		/*
		 * Read a command to tell us what to do
		 */
		cp = line;
		do {
			if ((size = netread(1, cp, 1)) != 1) {
				if (size < 0)
					frecverr(MSGSTR(RECVJOB_10, "%s: Lost connection"),printer);
				return(nfiles);
			}
		} while (*cp++ != '\n' && cp < &line[BUFSIZ]);
		/* terminate for end of line or overflow of buffer */

		*--cp = '\0';
		cp = line;
		switch (*cp++) {
		case '\1':	/* cleanup because data sent was bad */
			rcleanup();
			continue;

		case '\2':	/* read cf file */
			size = 0;
			while (*cp >= '0' && *cp <= '9')
				size = size * 10 + (*cp++ - '0');
			if (*cp++ != ' ')
				break;
			/*
			 * host name has been authenticated, we use our
			 * view of the host name since we may be passed
			 * something different than what gethostbyaddr()
			 * returns
			 */
			if (!isdigit(*(cp + 3)))
				strcpy(cp + 7, from);
			else
			strcpy(cp + 6, from);
			strcpy(tfname, cp);
			tfname[0] = 't';
			if(index(tfname, '/') || strlen(tfname) <= 6) {
			/* illegal control file - outside spool area */
				break;
			}
			if (!chksize(size)) {
				(void) write(1, "\2", 1);
				continue;
			}
			if (!readfile(tfname, size)) {
				rcleanup();
				continue;
			}

			(void) chown(tfname, DU, -1);	/* cf's must by owned by daemon */
							/* 001 - plewa                  */

			if (link(tfname, cp) < 0)
				frecverr(MSGSTR(RECVJOB_11, "%s: cannot link %s to %s: %m"),
					printer, tfname, cp);
			(void) unlink(tfname);
			tfname[0] = '\0';
			nfiles++;
			continue;

		case '\3':	/* read df file */
			size = 0;
			while (*cp >= '0' && *cp <= '9')
				size = size * 10 + (*cp++ - '0');
			if (*cp++ != ' ')
				break;
			if (!chksize(size)) {
				(void) write(1, "\2", 1);
				continue;
			}
			strcpy(dfname, cp);
			if(index(dfname, '/') || strlen(dfname) <= 6 ||
			   (dfname[1] == 'f' && (dfname[0] == 'c' ||
						 dfname[0] == 't' ) )   ) {
			/* illegal data file -- outside spool area or
                         * starts with illegal character combination (cf or df)
			 */
			  	break;
			}
			(void) readfile(dfname, size);
			continue;
		}
		frecverr(MSGSTR(RECVJOB_12, "protocol screwup"));
	}
}

/*
 * Read files send by lpd and copy them to the spooling directory.
 */
readfile(file, size)
	char *file;
	int size;
{
	register char *cp;
	char buf[BUFSIZ];
	register int i, j, amt;
	int fd, err;

	fd = open(file , O_WRONLY|O_CREAT|O_EXCL , FILMOD);
	if (fd < 0)
		frecverr(MSGSTR(RECVJOB_13, "%s: cannot open %s: %m"), printer, file);
	ack();
	err = 0;
	for (i = 0; i < size; i += BUFSIZ) {
		amt = BUFSIZ;
		cp = buf;
		if (i + amt > size)
			amt = size - i;
		do {
			j = netread(1, cp, amt);
			if (j <= 0)
				frecverr(MSGSTR(RECVJOB_14, "Lost connection"));
			amt -= j;
			cp += j;
		} while (amt > 0);
		amt = BUFSIZ;
		if (i + amt > size)
			amt = size - i;
		if (write(fd, buf, amt) != amt) {
			err++;
			break;
		}
	}
	(void) close(fd);
	if (err)
		frecverr(MSGSTR(RECVJOB_15, "%s: write error for %s: %m"), printer, file);
	if (noresponse()) {		/* file sent had bad data in it */
		(void) unlink(file);
		return(0);
	}
	ack();
	return(1);
}

noresponse()
{
	char resp;

	if (netread(1, &resp, 1) != 1)
		frecverr(MSGSTR(RECVJOB_17, "%s: Lost connection: %m"), printer);
	if (resp == '\0')
		return(0);
	return(1);
}

/*
 * Check to see if there is enough space on the disk for size bytes.
 * 1 == OK, 0 == Not OK.
 */
chksize(size)
	int size;
{
	struct statfs statfsbuf;

	if (statfs(SD, &statfsbuf, sizeof statfsbuf) < 0)
		frecverr(MSGSTR(RECVJOB_18, "%s: cannot statfs %s: %m"), printer, SD);

	size = (size + statfsbuf.f_fsize - 1) / statfsbuf.f_fsize;

	if (minfree + size > statfsbuf.f_bavail)
		return(0);
	else
		return(1);
}

read_number(fn)
	char *fn;
{
	char lin[80];
	register FILE *fp;

	if ((fp = fopen(fn, "r")) == NULL)
		return (0);
	if (fgets(lin, 80, fp) == NULL) {
		fclose(fp);
		return (0);
	}
	fclose(fp);
	return (atoi(lin));
}
#endif /* !SEC_MAC } */

/*
 * Remove all the files associated with the current job being transfered.
 */
void rcleanup()
{

	if (tfname[0])
		(void) unlink(tfname);
	if (dfname[0])
		do {
			do
				(void) unlink(dfname);
			while (dfname[2]-- != 'A');
			dfname[2] = 'z';
		} while (dfname[0]-- != 'd');
	dfname[0] = '\0';
}

frecverr(msg, a1, a2)
	char *msg;
        caddr_t a1, a2;
{
	rcleanup();
	syslog(LOG_ERR, msg, a1, a2);
	putchar('\1');		/* return error code */
	exit(1);
}

int netread(fd, buf, cnt)
int fd;
char *buf;
int cnt;
{
    int msk;
    struct timeval timout;
    /* this routine is designed not to block on an hung connection */
    msk = 1 << fd;
    timout.tv_sec = 60;
    timout.tv_usec = 0;
    if (select(30, &msk, 0, 0, &timout) < 0) {
	syslog(LOG_ERR,MSGSTR(RECVJOB_20, "netread failed fd=%d %m"),fd);
	return -1;
    }
    return(read(fd, buf, cnt));
}
