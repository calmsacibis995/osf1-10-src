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
static char	*sccsid = "@(#)$RCSfile: printjob.c,v $ $Revision: 4.3.7.2 $ (DEC) $Date: 1993/10/19 18:14:26 $";
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
 * printjob.c	5.6 (Berkeley) 6/30/88
 * printjob.c	4.2 13:31:47 7/20/90 SecureWare 
 */


/*
 * printjob -- print jobs in the queue.
 *
 *	NOTE: the lock file is used to pass information to lpq and lprm.
 *	it does not need to be removed because file locks are dynamic.
 */

/*
 * In order to handle the standard error output from the lpscomm
 * network filter we have to do run an extra filter which
 * reads standard error from the output filter
 * The following table is used by open_output_filter() to select
 * the appropriate function according to enum printer_type_e
 */


static set_tty_modes(/* int pfd */);


/*
 * RESOURCE TABLE CODE
 */

char *CT_choices[] = {
        /* check this matches enum connection_type_e (connection.h) */
       "dev", "lat", "remote", "network", "tcp", "dqs", 0,
};




#include <sys/types.h>
#include <sys/ioctl.h>
#include <dec/lat/lat.h>
#include "lp.h"

#if SEC_MAC
#define	LPBANNER	"/usr/sbin/lpbanner"
#endif
#define DORETURN	0	/* absorb fork error */
#define DOABORT		1	/* abort if dofork fails */

/*
 * Error tokens
 */
#define REPRINT		-2
#define ERROR		-1
#define	OK		0
#define	FATALERR	1
#define	NOACCT		2
#define	FILTERERR	3
#define	ACCESS		4

char	title[80];		/* ``pr'' title */
FILE	*cfp;			/* control file */
int	pfd;			/* printer file descriptor */
int	ofd;			/* output filter file descriptor */
int	lfd;			/* lock file descriptor */
int	pid;			/* pid of lpd process */
int	prchild;		/* id of pr process */
int	child;			/* id of any filters */
int	ofilter;		/* id of output filter, if any */
int	tof;			/* true if at top of form */
int	remote;			/* true if sending files to remote */
dev_t	fdev;			/* device of file pointed to by symlink */
ino_t	fino;			/* inode of file pointed to by symlink */

char	fromhost[32];		/* user's host machine */
char	logname[32];		/* user's login name */
char    username[100];		/* user's full name */
char	jobname[100];		/* job or file name */
char	class[32];		/* classification field */
char	width[10] = "-w";	/* page width in characters */
char	length[10] = "-l";	/* page length in lines */
char	pxwidth[10] = "-x";	/* page width in pixels */
char	pxlength[10] = "-y";	/* page length in pixels */
char	indent[10] = "-i0";	/* indentation size in characters */
char	tmpfilename[] = "errsXXXXXX"; /* file name for filter output */
char	in_tray[32] = "-I";	/* input tray specifier */
char	out_bin[32] = "-o";	/* output bin specifier */
char	sides[32] = "-K";	/* number of sides specifier */
char	orient[32] = "-O";	/* orientation specifier */
char	number_up[32] = "-N";	/* number up specifier */
int	no_filterflg;		/* flag job as not requiring filter */
int	mailflg;		/* send mail on error or completion */
int	sendflg;		/* do a send on error or completion */
char	ERRORstr[100];		/* string for ERROR messages */
#if SEC_MAC
int	suppress_labels;	/* don't print internal page labels */
char	cfjobname[100];		/* sequence number/host */
int	lpbpid;
#endif

extern int	errno;
extern int	sys_nerr;
extern char	*sys_errlist[];
static char *itoa(p,n)
char *p;
unsigned n;
{
    if (n >= 10)
	p =itoa(p,n/10);
    *p++ = (n%10)+'0';
    return(p);
}

static char *errmsg(cod)
int cod;
{
	static char unk[100];		/* trust us */

	if (cod < 0) cod = errno;

	if((cod >= 0) && (cod < sys_nerr))
	    return(sys_errlist[cod]);

	strcpy(unk, MSGSTR(PRINTJOB_1, "Unknown error "));
	*itoa(&unk,cod) = '\0';

	return(unk);
}

printjob()
{
	struct stat stb;
	register struct queue *q, **qp;
	struct queue **queue;
	register int i, nitems, ndesc;
	long pidoff;
	int count = 0;
	extern void  abortpr();

	ndesc = getdtablesize();
	/* close random descriptors including flock'ed ones */
	for (i = ndesc; --i > 2; (void) close(i));
	init();					/* set up capabilities */
	(void) write(1, "", 1);			/* ack that daemon is started */
	/* at this point the network connection is never used */
	/* why not release the connection so that some busted tcp */
	/* implementations get potentially unbuffered data */
	close(1);
	i = open("/dev/null", O_WRONLY);
	if (i >= 0 && i != 1)
		dup2(i, 1);
	(void) close(2);			/* set up log file */
	/* be more careful about assigning logfile to stderr */
	if ((i = open(LF, O_WRONLY|O_APPEND, 0664)) < 0) {
		syslog(LOG_LPR | LOG_ERR, MSGSTR(PRINTJOB_2, "%s: cannot open %s: %m"),
			printer, LF);
		i = open("/dev/null", O_WRONLY);
	}
	if (i >= 0 && i != 2)
		dup2(i, 2);
	setgid(getegid());
	pid = getpid();				/* for use with lprm */
	setpgrp(0, pid);
	signal(SIGHUP, abortpr);
	signal(SIGINT, abortpr);
	signal(SIGQUIT, abortpr);
	signal(SIGTERM, abortpr);

	(void) mktemp(tmpfilename);

	/*
	 * uses short form file names
	 */
	if (chdir(SD) < 0) {
		syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_3, "%s: cannot chdir to %s: %m"), printer, SD);
		exit(1);
	}
	if (stat(LO, &stb) == 0 && (stb.st_mode & 0100))
		exit(0);		/* printing disabled */
	lfd = open(LO, O_WRONLY|O_CREAT, 0644);
	if (lfd < 0) {
		syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_4, "%s: cannot open lockfile %s: %m"), printer, LO);
		exit(1);
	}


	{ /* 2 min LAT timout wait on lock file */
	int lock_time_out;
	int max_time_out = 120; /* if locked for 2min were hung */

	for (lock_time_out = 0 ; 
		lock_time_out < max_time_out ; lock_time_out++ ) {

	  if (flock(lfd, LOCK_EX|LOCK_NB) < 0) {
		if (errno == EWOULDBLOCK) {	/* active deamon present */
			/* Try again after a pause in case it is exiting */
			sleep(1);
		}
	  }
	  else break;
	 } /* for loop timout */
	  if ( (lock_time_out == max_time_out) && (errno == EWOULDBLOCK) ){
			  /* 2 mins are up */
		exit (0); /*let other daemon thats hung take care of it*/
	  }
	  else if ( (lock_time_out == max_time_out) &&  
			(errno != EWOULDBLOCK) ) { 
			/* 2 min timeout and no other daemon so error exit */
		syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_5, 
			"%s: cannot lock file %s: %m"), printer, LO);
		exit(1);
	  }
	}
	/* Lock file is free in under 2 min */

	ftruncate(lfd, 0);
	/*
	 * write process id for others to know
	 */
	sprintf(line, "%u\n", pid);
	pidoff = i = strlen(line);
	if (write(lfd, line, i) != i) {
		syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_6, "%s: cannot write lockfile %s: %s"), printer, LO);
		exit(1);
	}
	/*
	 * search the spool directory for work and sort by queue order.
	 */
	if ((nitems = getq(&queue)) < 0) {
		syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_7, "%s: can't scan %s: %m"), printer, SD);
		exit(1);
	}
	if (nitems == 0)		/* no work to do */
		exit(0);
	if (stb.st_mode & 01) {		/* reset queue flag */
		if (fchmod(lfd, stb.st_mode & 0776) < 0)
			syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_9, "%s: cannot fchmod lockfile %s: %m"), printer, LO);
	}
#if !SEC_MAC
	openpr();			/* open printer or remote */
#endif
again:
	/*
	 * we found something to do now do it --
	 *    write the name of the current control file into the lock file
	 *    so the spool queue program can tell what we're working on
	 */
	for (qp = queue; nitems--; free((char *) q)) {
		q = *qp++;
		if (stat(q->q_name, &stb) < 0)
			continue;
	restart:
		(void) lseek(lfd, pidoff, 0);
		(void) sprintf(line, "%s\n", q->q_name);
		i = strlen(line);
		if (write(lfd, line, i) != i)
			syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_10, "%s: cannot write to lockfile %s: %m"),
				printer, LO);
		/* clear fatal error string */
		ERRORstr[0] = '\0';
		if (!remote)
			i = printit(q->q_name);
		else
			i = sendit(q->q_name);
		/*
		 * Check to see if we are supposed to stop printing or
		 * if we are to rebuild the queue.
		 */
		if (fstat(lfd, &stb) == 0) {
			/* stop printing before starting next job? */
			if (stb.st_mode & 0100)
				goto done;
			/* rebuild queue (after lpc topq) */
			if (stb.st_mode & 01) {
				for (free((char *) q); nitems--; free((char *) q))
					q = *qp++;
				if (fchmod(lfd, stb.st_mode & 0776) < 0)
					syslog(LOG_LPR|LOG_WARNING, MSGSTR(PRINTJOB_9, "%s: cannot fchmod lockfile %s: %m"),
						printer, LO);
				break;
			}
		}
		if (i == OK)		/* file ok and printed */
			count++;
		else if (i == REPRINT) { /* try reprinting the job */
			syslog(LOG_LPR|LOG_INFO, MSGSTR(PRINTJOB_11, "restarting %s"),
				printer);
#if !SEC_MAC
			if (ofilter > 0) {
				kill(ofilter, SIGCONT);	/* to be sure */
				(void) close(ofd);
				while ((i = wait(0)) > 0 && i != ofilter)
					;
				ofilter = 0;
			}
			(void) close(pfd);	/* close printer */
#endif
			if (ftruncate(lfd, pidoff) < 0)
				syslog(LOG_LPR|LOG_WARNING, MSGSTR(PRINTJOB_12,
					"%s: cannot truncate lockfile %s: %m"),
					printer, LO);
#if !SEC_MAC
			openpr();		/* try to reopen printer */
#endif
			goto restart;
		}
	}
	free((char *) queue);
	/*
	 * search the spool directory for more work.
	 */
	if ((nitems = getq(&queue)) < 0) {
		syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_8, "%s: can't scan %s"), printer, SD);
		exit(1);
	}
	if (nitems == 0) {		/* no more work to do */
	done:
#if !SEC_MAC
		if (count > 0) {	/* Files actually printed */
			if (!SF && !tof)
				(void) write(ofd, FF, strlen(FF));
			if (TR != NULL)		/* output trailer */
				(void) write(ofd, TR, strlen(TR));
		}
#endif
		writeStatus("");	/* Clear out status file */
		(void) unlink(tmpfilename);
		exit(0);
	}
	goto again;
}

char	fonts[4][50];	/* fonts for troff */

char ifonts[4][18] = {
	"/usr/lib/vfont/R",
	"/usr/lib/vfont/I",
	"/usr/lib/vfont/B",
	"/usr/lib/vfont/S"
};

/*
 * The remaining part is the reading of the control file (cf)
 * and performing the various actions.
 */
printit(file)
	char *file;
{
	register int i;
	char *cp;
	int bombed = OK;

	/*
	 * open control file; ignore if no longer there.
	 */
	if ((cfp = fopen(file, "r")) == NULL) {
		syslog(LOG_LPR|LOG_INFO, MSGSTR(PRINTJOB_13, "%s: cannot open cf file %s: %s"),
			printer, file);
		return(OK);
	}
	/*
	 * Reset troff fonts.
	 */
	for (i = 0; i < 4; i++)
		strcpy(fonts[i], ifonts[i]);
	/*
	 * reset some internal variables so we don't make mistakes
	 */
	sprintf(&width[2], "%d", PW);
	sprintf(&length[2], "%d", PL);
	sprintf(&pxwidth[2], "%d", PX);
	sprintf(&pxlength[2], "%d", PY);
	strcpy(&indent[2], "0");
	orient[2]  = 0; /* set to use printer default */
	sides[2]   = 0; /* set to use printer default */
	in_tray[2] = 0; /* set to use printer default */
	out_bin[2] = 0; /* set to use printer default */
	number_up[2] = 0; /* set to use printer default */
	bzero(fromhost,sizeof(fromhost));
	bzero(logname,sizeof(logname));
	bzero(jobname,sizeof(jobname));
	bzero(class,sizeof(class));
	bzero(title,sizeof(title));
	sendflg = mailflg = 0;	/* no notification by default */
	no_filterflg = 0; /* pass through filter is not the default */
#if SEC_MAC
	{
		char *prefix = "cfA";

		for (i = 0; file[i] && prefix[i]; ++i)
			if (file[i] != prefix[i]) {
				i = 0;
				break;
			}
		strncpy(cfjobname, &file[i], sizeof cfjobname - 1);
	}
#endif

	/*
	 *      read the control file for work to do
	 *
	 *      file format -- first character in the line is a command
	 *      rest of the line is the argument.
	 *      valid commands are:
	 *
	 *		S -- "stat info" for symbolic link protection
	 *		J -- "job name" on banner page
	 *		C -- "class name" on banner page
	 *              L -- "literal" user's name to print on banner
	 *		T -- "title" for pr
	 *		H -- "host name" of machine where lpr was done
	 *              P -- "person" user's login name
	 *              I -- "indent" amount to indent output
	 *              f -- "file name" name of text file to print
	 *		x -- "file name" text file to print without filtering
	 *		l -- "file name" text file with control chars
	 *		p -- "file name" text file to print with pr(1)
	 *		t -- "file name" troff(1) file to print
	 *		n -- "file name" ditroff(1) file to print
	 *		d -- "file name" dvi file to print
	 *		g -- "file name" plot(1G) file to print
	 *		v -- "file name" plain raster file to print
	 *		c -- "file name" cifplot file to print
	 *		1 -- "R font file" for troff
	 *		2 -- "I font file" for troff
	 *		3 -- "B font file" for troff
	 *		4 -- "S font file" for troff
	 *		N -- "name" of file (used by lpq)
	 *              U -- "unlink" name of file to remove
	 *                    (after we print it. (Pass 2 only)).
	 *		M -- "mail" to user when done printing
	 *		Z -- "send" to user when done printing
	 *		V -- suppress labels
	 *		W -- "width" of page
	 *		< -- "input tray" to use
	 *		> -- "output tray" to use
	 *		O -- "page orientation"
	 *		G -- "number up" per page
	 *		K -- "sides" of page to use
	 *
	 *      getline reads a line and expands tabs to blanks
	 */

	/* pass 0: handle stuff for the normal pass 1 */

	while (getline(cfp)) {
 		switch (line[0]) {
		case 'M':
			mailflg++;
			continue;

		case 'Z':
			sendflg++;
			continue;
#if SEC_MAC
		case 'V':
			suppress_labels++;
			continue;
#endif
		}
	}
	fseek(cfp, 0L, 0);

	/* pass 1 */

	while (getline(cfp))
		switch (line[0]) {
		case 'G':	/* number up */
			strcpy(number_up+2, line+1, sizeof(number_up)-3);
			continue;

		case 'H':
			strcpy(fromhost, line+1);
			if (class[0] == '\0')
				strncpy(class, line+1, sizeof(class)-1);
			continue;

		case 'P':
			strncpy(logname, line+1, sizeof(logname)-1);
			if (RS) {			/* restricted */
				if (getpwnam(logname) == (struct passwd *)0) {
					bombed = NOACCT;
					/* we call send mail only once */
					goto pass2;
				}
			}
			continue;

		case 'S':
			cp = line+1;
			i = 0;
			while (*cp >= '0' && *cp <= '9')
				i = i * 10 + (*cp++ - '0');
			fdev = i;
			cp++;
			i = 0;
			while (*cp >= '0' && *cp <= '9')
				i = i * 10 + (*cp++ - '0');
			fino = i;
			continue;

		case 'J':
			if (line[1] != '\0')
				strncpy(jobname, line+1, sizeof(jobname)-1);
			else
				strcpy(jobname, " ");
			continue;

		case 'C':
			if (line[1] != '\0')
				strncpy(class, line+1, sizeof(class)-1);
			else if (class[0] == '\0')
				gethostname(class, sizeof(class));
			continue;

		case 'T':	/* header title for pr */
			strncpy(title, line+1, sizeof(title)-1);
			continue;

		case 'L':	/* identification line */
#if SEC_MAC
			/* banners are generated by lpbanner program */
#else /* !SEC_MAC */
			strncpy(username, line+1, sizeof(username)-1);
			if (!SH && !HL)
			    banner(logname, jobname);
/* math
				banner(line+1, jobname);
*/
#endif /* !SEC_MAC */
			continue;

		case '1':	/* troff fonts */
		case '2':
		case '3':
		case '4':
			if (line[1] != '\0')
				strcpy(fonts[line[0]-'1'], line+1);
			continue;

		case 'W':	/* page width */
			strncpy(width+2, line+1, sizeof(width)-3);
			continue;

		case 'O':	/* Orientation of the output on the paper */
			strncpy(orient+2, line+1, sizeof(orient)-3);
			continue;

		case 'K':	/* Number and style of sides */
			strncpy(sides+2, line+1, sizeof(sides)-3);
			continue;

		case '<':	/* Input tray selection */
			strncpy(in_tray+2, line+1, sizeof(in_tray)-3);
			continue;

		case '>':	/* Output tray selection */
			strncpy(out_bin+2, line+1, sizeof(out_bin)-3);
			continue;

		case 'I':	/* indent amount */
			strncpy(indent+2, line+1, sizeof(indent)-3);
			continue;

		default:	/* some file to print */
			/* give people and operators a feeling of progress */
			writeStatus(MSGSTR(PRINTJOB_14, "Attempting to print %s"), line+1);
			switch (i = print(line[0], line+1)) {
			case ERROR:
				syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_15, "%s: print error for %s\n"),
					printer, line+1);
				if (bombed == OK)
					bombed = FATALERR;
				break;
			case REPRINT:
				syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_16, "%s: reprint error for %s\n"),
					printer, line+1);
				(void) fclose(cfp);
				return(REPRINT);
			case FILTERERR:
			case ACCESS:
				syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_17, "%s: filter/access error for %s\n"),
					printer, line+1);
				bombed = i;
				/* we only call sendmail once */
			}
			title[0] = '\0';
			continue;

		case 'N':
		case 'U':
		case 'M':
		case 'Z':
#if	SEC_MAC
		case 'V':
#endif
			continue;
		}

	/* pass 2 */

pass2:
	fseek(cfp, 0L, 0);
	while (getline(cfp))
		switch (line[0]) {
		case 'L':	/* identification line */
#if SEC_MAC
			/* trailer pages are generated by lpbanner */
#else
			strncpy(username, line+1, sizeof(username)-1);
			if (!SH && HL)
			    banner(logname, jobname);
/* math
				banner(line+1, jobname);
*/
#endif
			continue;

		/* we only call sendmail once */
		case 'U':
			if (index(line+1, '/')){
				; /* unsecure attempt at unlinking a file */
				/* allow unlink only in spool directory */
			}
			else {
				(void) unlink(line+1);
			}
		}
	/*
	 * clean-up in case another control file exists
	 */
	writeStatus(MSGSTR(PRINTJOB_18, "Notifying user..."));
	sendmail(logname, bombed);
	(void) fclose(cfp);
	(void) unlink(file);
	/* give them a status message that goes with the cfile delete */
	writeStatus(MSGSTR(PRINTJOB_19, "%s is ready and printing"), printer);
	return(bombed == OK ? OK : ERROR);
}

/*
 * Print a file.
 * Set up the chain [ PR [ | {IF, OF} ] ] or {IF, RF, TF, NF, DF, CF, VF, XF}.
 * Return -1 if a non-recoverable error occured,
 * 2 if the filter detected some errors (but printed the job anyway),
 * 1 if we should try to reprint this job and
 * 0 if all is well.
 * Note: all filters take stdin as the file, stdout as the printer,
 * stderr as the log file, and must not ignore SIGINT.
 * Note: in the event the format is 'x' or the 'if' printcap field is not
 * defined and the format is 'l' or 'f' send the file direct to the printer
 */
print(format, file)
	int format;
	char *file;
{
	register int n, ndesc;
	register char *prog;
	int fi, fo;
	char *av[30], buf[BUFSIZ];
	int pid, p[2], stopped = 0;
	int  status;
	struct stat stb;
	FILE *fp;

	ndesc = getdtablesize();
	if (lstat(file, &stb) < 0 || (fi = open(file, O_RDONLY)) < 0)
	{
		sprintf(ERRORstr, MSGSTR(PRINTJOB_20, "lstat/open failed for %s: %s"), file,
			errmsg(-1));
		syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_21, "%s: lstat/open failed for %s: %m"),
			printer, file);
		return(ERROR);
 	}
	/*
	 * Check to see if data file is a symbolic link. If so, it should
	 * still point to the same file or someone is trying to print
	 * something he shouldn't.
	 */
	if ((stb.st_mode & S_IFMT) == S_IFLNK && fstat(fi, &stb) == 0 &&
	    (stb.st_dev != fdev || stb.st_ino != fino))
	{
		syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_22, "%s: invalid symbolic link %s"),
			printer, file);
		(void) close(fi);
		return(ACCESS);
	}
#if !SEC_MAC
	if (!SF && !tof) {		/* start on a fresh page */
		(void) write(ofd, FF, strlen(FF));
		tof = 1;
	}
#endif
	if(IF == NULL && (format == 'f' || format == 'l')) {
#if SEC_MAC
		openpr(1, file);
#endif
		tof = 0;
		while ((n = read(fi, buf, BUFSIZ)) > 0)
			if (write(ofd, buf, n) != n) {
				(void) close(fi);
#if SEC_MAC
				closepr();
#endif
				return(REPRINT);
			}
		(void) close(fi);
#if SEC_MAC
		closepr();
#endif
		return(OK);
	}
	switch (format) {
	case 'p':	/* print file using 'pr' */
		if (IF == NULL) {	/* use output filter */
#if SEC_MAC
			openpr(1, file);
#endif
			prog = PR;
			av[0] = "pr";
			av[1] = width;
			av[2] = length;
			av[3] = "-h";
			av[4] = *title ? title : " ";
			av[5] = 0;
			fo = ofd;
			goto start;
		}
		pipe(p);
		if ((prchild = dofork(DORETURN)) == 0) {	/* child */
			dup2(fi, 0);		/* file is stdin */
			dup2(p[1], 1);		/* pipe is stdout */
			for (n = 3; n < ndesc; n++)
				(void) close(n);
#if SEC_MAC
			/*
			 * If we are doing per-page labels, tell pr to use
			 * formfeeds to keep pages in sync.
			 */
			if (!suppress_labels)
				execl(PR, "pr", width, length, "-f",
					"-h", *title ? title : " ", 0);
			else
#endif
			execl(PR, "pr", width, length, "-h", *title ? title : " ", 0);
			syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_23, "%s: cannot execl %s: %m"),
				printer, PR);
			writeStatus("");
			exit(2);
		}
		(void) close(p[1]);		/* close output side */
		(void) close(fi);
		if (prchild < 0) {
			prchild = 0;
			sprintf(ERRORstr, MSGSTR(PRINTJOB_25, "PR filter failed: %s"), errmsg(-1));
			(void) close(p[0]);
			return(ERROR);
		}
		fi = p[0];			/* use pipe for input */
	case 'f':	/* print plain text file */
		prog = IF;
		av[1] = width;
		av[2] = length;
		av[3] = indent;
		n = 4;
		if(orient[2] != 0) av[n++] = orient;
		if(sides[2] != 0) av[n++] = sides;
		if(in_tray[2] != 0) av[n++] = in_tray;
		if(out_bin[2] != 0) av[n++] = out_bin;
		break;
	case 'l':	/* like 'f' but pass control characters */
		prog = IF;
		av[1] = "-c";
		av[2] = width;
		av[3] = length;
		av[4] = indent;
		n = 5;
		if(orient[2] != 0) av[n++] = orient;
		if(sides[2] != 0) av[n++] = sides;
		if(in_tray[2] != 0) av[n++] = in_tray;
		if(out_bin[2] != 0) av[n++] = out_bin;
		break;
	case 'x':	/* like 'l' but use the non-filter filter */
		prog = XF;
		n = 1;
		break;
	case 'r':	/* print a fortran text file */
		prog = RF;
		av[1] = width;
		av[2] = length;
		n = 3;
		break;
	case 't':	/* print troff output */
	case 'n':	/* print ditroff output */
	case 'd':	/* print tex output */
		(void) unlink(".railmag");
		if ((fo = creat(".railmag", FILMOD)) < 0) {
			syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_27, "%s: cannot create .railmag: %m"),
				printer);
			(void) unlink(".railmag");
		} else {
			for (n = 0; n < 4; n++) {
				if (fonts[n][0] != '/')
					(void) write(fo, "/usr/lib/vfont/", 15);
				(void) write(fo, fonts[n], strlen(fonts[n]));
				(void) write(fo, "\n", 1);
			}
			(void) close(fo);
		}
		prog = (format == 't') ? TF : (format == 'n') ? NF : DF;
		av[1] = pxwidth;
		av[2] = pxlength;
		n = 3;
		break;
	case 'c':	/* print cifplot output */
		prog = CF;
		av[1] = pxwidth;
		av[2] = pxlength;
		n = 3;
		break;
	case 'g':	/* print plot(1G) output */
		prog = GF;
		av[1] = pxwidth;
		av[2] = pxlength;
		n = 3;
		break;
	case 'v':	/* print raster output */
		prog = VF;
		av[1] = pxwidth;
		av[2] = pxlength;
		n = 3;
		break;
	default:
		(void) close(fi);
		syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_29, "%s: illegal format character '%c'"),
			printer, format);
		sprintf(ERRORstr, MSGSTR(PRINTJOB_30, "illegal format character '%c'"), format);
		return(ERROR);
	}
	if (!prog) {
		(void) close(fi);
		sprintf(ERRORstr, MSGSTR(PRINTJOB_31, "no filter for format '%c'"), format);
		syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_32, "%s: no filter for format '%c'"),
			printer, format);
		(void) unlink(".railmag");
		return(ERROR);
	}
	if ((av[0] = rindex(prog, '/')) != NULL)
		av[0]++;
	else
		av[0] = prog;
	av[n++] = "-n";
	av[n++] = logname;
	av[n++] = "-h";
	av[n++] = fromhost;
#if SEC_MAC
	if (!suppress_labels) {
		av[n++] = "-V";
		av[n++] = file;
	}
	av[n++] = "-P";
	av[n++] = printer;
#else /* !SEC_MAC */
	if (FA) {
		av[n++] = "-A";
		av[n++] = FA;
	}
	if (HN) {
		av[n++] = "-i";
		av[n++] = HN;
	}
	av[n++] = AF;
#endif /* !SEC_MAC */
	av[n] = 0;
#if SEC_MAC
	openpr(0, file);
#endif
	fo = pfd;
#if !SEC_MAC
	if (ofilter > 0) {		/* stop output filter */
		write(ofd, "\031\1", 2);
		while ((pid = waitpid(-1, &status, WUNTRACED)) > 0 && pid != ofilter)
			;
		if (!WIFSTOPPED(status)) {
			(void) close(fi);
			syslog(LOG_LPR|LOG_WARNING,
			       MSGSTR(PRINTJOB_33, "%s: output filter died (sig=%d, err=%d)"),
			       printer,WTERMSIG(status), WEXITSTATUS(status));
			return(REPRINT);
		}
		stopped++;
	}
#endif /* !SEC_MAC */
start:
	if ((child = dofork(DORETURN)) == 0) {	/* child */
		dup2(fi, 0);
		dup2(fo, 1);
		n = open(tmpfilename, O_WRONLY|O_CREAT|O_TRUNC|O_APPEND, 0664);
		if (n >= 0)
			dup2(n, 2);
		for (n = 3; n < ndesc; n++)
			(void) close(n);
		execv(prog, av);
		syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_34, "%s: cannot execv %s: %m"),
			printer, prog);
		writeStatus("");
		exit(2);
	}
	(void) close(fi);
	if (child >= 0)
	    while ((pid = wait(&status)) > 0 && pid != child)
		;
#if SEC_MAC
	closepr();
#endif
	/* copy error file to log file */
	if (stat(tmpfilename, &stb) >= 0 && stb.st_size != 0 && (fp = fopen(tmpfilename, "r")) != NULL) {
		fputs("<<\n", stderr);
		ffilecopy(fp, stderr);
		fputs(">>\n", stderr);
		fprintf(stderr, MSGSTR(PRINTJOB_36, "Filter '%c' terminated (sig=%d,err=%d)\n"),
			format, WTERMSIG(status), WEXITSTATUS(status));
		fclose(fp);
	}
	child = 0;
	prchild = 0;
#if !SEC_MAC
	if (stopped) {		/* restart output filter */
		if (kill(ofilter, SIGCONT) < 0) {
			syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_37, "%s: cannot restart output filter: %m"),
				printer);
			writeStatus("");
			exit(1);
		}
	}
#endif
	tof = 0;
	if (!WIFEXITED(status)) {
		sprintf(ERRORstr, MSGSTR(PRINTJOB_39, "Filter '%c' terminated (sig=%d,err=%d)"),
			format, WTERMSIG(status), WEXITSTATUS(status));
		syslog(LOG_LPR|LOG_WARNING, MSGSTR(PRINTJOB_40, "%s: Daemon filter '%c' terminated (%d)"),
			printer, format, WTERMSIG(status));
		return(FILTERERR);
	}
	switch (WEXITSTATUS(status)) {
	case 0:
		tof = 1;
		return(OK);
	case 1:
		return(REPRINT);
	default:
		syslog(LOG_LPR|LOG_WARNING, MSGSTR(PRINTJOB_41, "%s: Daemon filter '%c' exited (%d)"),
			printer, format, WEXITSTATUS(status));
		sprintf(ERRORstr, MSGSTR(PRINTJOB_39, "Filter '%c' terminated (sig=%d,err=%d)"),
			format, WTERMSIG(status), WEXITSTATUS(status));
		return(FILTERERR);
	case 2:
		syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_41, "%s: Daemon filter '%c' exited (%d)"),
			printer, format, WEXITSTATUS(status));
		sprintf(ERRORstr,MSGSTR(PRINTJOB_39, "Filter '%c' terminated (sig=%d,err=%d)"),
			format, WTERMSIG(status), WEXITSTATUS(status));
		return(ERROR);
	}
}

/*
 * Send the daemon control file (cf) and any data files.
 * Return -1 if a non-recoverable error occured, 1 if a recoverable error and
 * 0 if all is well.
 */
sendit(file)
	char *file;
{
	register int i, err = OK;
	char *cp, last[BUFSIZ];

	/*
	 * open control file
	 */
	if ((cfp = fopen(file, "r")) == NULL)
		return(OK);
	/*
	 *      read the control file for work to do
	 *
	 *      file format -- first character in the line is a command
	 *      rest of the line is the argument.
	 *      commands of interest are:
	 *
	 *            a-z -- "file name" name of file to print
	 *              U -- "unlink" name of file to remove
	 *                    (after we print it. (Pass 2 only)).
	 */

	/*
	 * pass 1
	 */
	while (getline(cfp)) {
	again:
		if (line[0] == 'S') {
			cp = line+1;
			i = 0;
			while (*cp >= '0' && *cp <= '9')
				i = i * 10 + (*cp++ - '0');
			fdev = i;
			cp++;
			i = 0;
			while (*cp >= '0' && *cp <= '9')
				i = i * 10 + (*cp++ - '0');
			fino = i;
			continue;
		}
		if (line[0] >= 'a' && line[0] <= 'z') {
			strcpy(last, line);
			while (i = getline(cfp))
				if (strcmp(last, line))
					break;
			switch (sendfile('\3', last+1)) {
			case OK:
				if (i)
					goto again;
				break;
			case REPRINT:
				(void) fclose(cfp);
				return(REPRINT);
			case ACCESS:
				sendmail(logname, ACCESS);
			case ERROR:
				err = ERROR;
			}
			break;
		}
	}
	if (err == OK && sendfile('\2', file) > 0) {
		(void) fclose(cfp);
		return(REPRINT);
	}
	/*
	 * pass 2
	 */
	fseek(cfp, 0L, 0);
	while (getline(cfp))
		if (line[0] == 'U')
			(void) unlink(line+1);
	/*
	 * clean-up in case another control file exists
	 */
	(void) fclose(cfp);
	(void) unlink(file);
	return(err);
}

/*
 * Send a data file to the remote machine and spool it.
 * Return positive if we should try resending.
 */
sendfile(type, file)
	char type, *file;
{
	register int f, i, amt;
	struct stat stb;
	char buf[BUFSIZ];
	int sizerr, resp;

#if SEC_MAC
	/* Remote submission not supported on systems with MAC */
	return ERROR;
#else /* !SEC_MAC { */
	if (lstat(file, &stb) < 0 || (f = open(file, O_RDONLY)) < 0)
	{
		sprintf(ERRORstr, MSGSTR(PRINTJOB_42,
			"lstat/open failed for file %s: %s"), file,
			errmsg(-1));
		syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_43,
			"%s: lstat/open failed for file %s: %m"),
			printer, file);
		return(ERROR);
	}
	/*
	 * Check to see if data file is a symbolic link. If so, it should
	 * still point to the same file or someone is trying to print something
	 * he shouldn't.
	 */
	if ((stb.st_mode & S_IFMT) == S_IFLNK && fstat(f, &stb) == 0 &&
	    (stb.st_dev != fdev || stb.st_ino != fino))
		return(ACCESS);
	(void) sprintf(buf, "%c%d %s\n", type, stb.st_size, file);
	amt = strlen(buf);
	for (i = 0;  ; i++) {
		if (write(pfd, buf, amt) != amt ||
		    (resp = response()) < 0 || resp == '\1') {
			(void) close(f);
			return(REPRINT);
		} else if (resp == '\0')
			break;
		if (i == 0)
			writeStatus(MSGSTR(PRINTJOB_44,
			"no space on remote; waiting for queue to drain"));
		if (i == 10)
			syslog(LOG_LPR|LOG_ALERT, MSGSTR(PRINTJOB_45,
				"%s: can't send to %s; queue full"),
				printer, RM);
		sleep(5 * 60);
	}
	if (i)
		writeStatus(MSGSTR(PRINTJOB_46,
			"sending files to %s to be printed"), RM);
	sizerr = 0;
	for (i = 0; i < stb.st_size; i += BUFSIZ) {
		amt = BUFSIZ;
		if (i + amt > stb.st_size)
			amt = stb.st_size - i;
		if (sizerr == 0 && read(f, buf, amt) != amt)
			sizerr = 1;
		if (write(pfd, buf, amt) != amt) {
			(void) close(f);
			return(REPRINT);
		}
	}
	(void) close(f);
	if (sizerr) {
		sprintf(ERRORstr,MSGSTR(PRINTJOB_47,
			"file %s changed size"), file);
		syslog(LOG_LPR|LOG_INFO, MSGSTR(PRINTJOB_48,
			"%s: %s: changed size"), printer, file);
		/* tell recvjob to ignore this file */
		(void) write(pfd, "\1", 1);
		return(ERROR);
	}
	if (write(pfd, "", 1) != 1 || response())
		return(REPRINT);
	writeStatus(MSGSTR(PRINTJOB_49,
		"done sending files to %s to be printed"), RM);
	return(OK);
#endif /* !SEC_MAC } */
}

#if !SEC_MAC /* { */
/*
 * Check to make sure there have been no errors and that both programs
 * are in sync with eachother.
 * Return non-zero if the connection was lost.
 */
response()
{
	char resp;

	if (netread(pfd, &resp, 1) != 1) {
		syslog(LOG_LPR|LOG_INFO, MSGSTR(PRINTJOB_50,
			"%s: lost connection: %m"), printer);
		return(-1);
	}
	return(resp);
}

/*
 * Banner printing stuff
 */
banner(name1, name2)
	char *name1, *name2;
{
	time_t tvec;
	extern char *ctime();

	time(&tvec);
	if (!SF && !tof)
		(void) write(ofd, FF, strlen(FF));
	if (SB) {	/* short banner only */
		if (class[0]) {
			(void) write(ofd, class, strlen(class));
			(void) write(ofd, ":", 1);
		}
		(void) write(ofd, name1, strlen(name1));
		(void) write(ofd, MSGSTR(PRINTJOB_52, "  Job: "), 7);
		(void) write(ofd, name2, strlen(name2));
		(void) write(ofd, MSGSTR(PRINTJOB_53, "  Date: "), 8);
		(void) write(ofd, ctime(&tvec), 24);
		(void) write(ofd, "\n", 1);
	} else {	/* normal banner */
		(void) write(ofd, "\n\n\n", 3);
		scan_out(ofd, name1, '\0');
		(void) write(ofd, "\n\n", 2);
		scan_out(ofd, name2, '\0');
		if (class[0]) {
			(void) write(ofd,"\n\n\n",3);
			scan_out(ofd, class, '\0');
		}
		(void) write(ofd, MSGSTR(PRINTJOB_54, "\n\n\n\n\t\t\t\t\tJob:  "), 15);
		(void) write(ofd, name2, strlen(name2));
		(void) write(ofd, MSGSTR(PRINTJOB_55, "\n\t\t\t\t\tUser:  "), 13);
		(void) write(ofd, username, strlen(username));
		(void) write(ofd, MSGSTR(PRINTJOB_56, "\n\t\t\t\t\tDate: "), 12);
		(void) write(ofd, ctime(&tvec), 24);
		(void) write(ofd, "\n", 1);
	}
	if (!SF)
		(void) write(ofd, FF, strlen(FF));
	tof = 1;
}

char *
scnline(key, p, c)
	register char key, *p;
	char c;
{
	register scnwidth;

	for (scnwidth = WIDTH; --scnwidth;) {
		key <<= 1;
		*p++ = key & 0200 ? c : BACKGND;
	}
	return (p);
}

#define TRC(q)	(((q)-' ')&0177)

scan_out(scfd, scsp, dlm)
	int scfd;
	char *scsp, dlm;
{
	register char *strp;
	register nchrs, j;
	char outbuf[LINELEN+1], *sp, c, cc;
	int d, scnhgt;
	extern char scnkey[][HEIGHT];	/* in lpdchar.c */

	for (scnhgt = 0; scnhgt++ < HEIGHT+DROP; ) {
		strp = &outbuf[0];
		sp = scsp;
		for (nchrs = 0; ; ) {
			d = dropit(c = TRC(cc = *sp++));
			if ((!d && scnhgt > HEIGHT) || (scnhgt <= DROP && d))
				for (j = WIDTH; --j;)
					*strp++ = BACKGND;
			else
				strp = scnline(scnkey[c][scnhgt-1-d], strp, cc);
			if (*sp == dlm || *sp == '\0' || nchrs++ >= PW/(WIDTH+1)-1)
				break;
			*strp++ = BACKGND;
			*strp++ = BACKGND;
		}
		while (*--strp == BACKGND && strp >= outbuf)
			;
		strp++;
		*strp++ = '\n';	
		(void) write(scfd, outbuf, strp-outbuf);
	}
}

dropit(c)
	char c;
{
	switch(c) {

	case TRC('_'):
	case TRC(';'):
	case TRC(','):
	case TRC('g'):
	case TRC('j'):
	case TRC('p'):
	case TRC('q'):
	case TRC('y'):
		return (DROP);

	default:
		return (0);
	}
}
#endif /* !SEC_MAC } */

/*
 * sendmail ---
 *   tell people about job completion
 */
sendmail(user, bombed)
	char *user;
	int bombed;
{
	register int i, ndesc;
	int p[2], s;
	register char *cp;
	char buf[100];
	struct stat stb;
	FILE *fp;

	ndesc = getdtablesize();
	if (*user == '\0')
		return;
	if (*fromhost == '\0')
		return;
	if (sendflg)
		sendsend(user, bombed);
	if (!mailflg)
		return;
	pipe(p);
	if ((s = dofork(DORETURN)) == 0) {		/* child */
		dup2(p[0], 0);
		for (i = 3; i < ndesc; i++)
			(void) close(i);
		if ((cp = rindex(MAIL, '/')) != NULL)
			cp++;
		else
			cp = MAIL;
		sprintf(buf, "%s@%s", user, fromhost);
		execl(MAIL, cp, "-s", MSGSTR(PRINTJOB_57, "printer job"),
			buf, 0);
		syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_58,
			"%s: can not execl %s: %m"), printer, MAIL);
		writeStatus("");
		exit(-1);
	} else if (s > 0) {				/* parent */
		dup2(p[1], 1);
		/* pass headers through command line */
		printf(MSGSTR(PRINTJOB_61, "Your printer job "));
		if (*jobname)
			printf("(%s) ", jobname);
		switch (bombed) {
		case OK:
			printf(MSGSTR(PRINTJOB_62, "\ncompleted successfully\n"));
			break;
		default:
		case FATALERR:
			printf(MSGSTR(PRINTJOB_63, "\ncould not be printed\n"));
			break;
		case NOACCT:
			printf(MSGSTR(PRINTJOB_64, "\ncould not be printed without an account on %s\n"), host);
			break;
		case FILTERERR:
			if (stat(tmpfilename, &stb) < 0 || stb.st_size == 0 ||
			    (fp = fopen(tmpfilename, "r")) == NULL) {
				printf(MSGSTR(PRINTJOB_65, "\nwas printed but had some errors\n"));
				break;
			}
			printf(MSGSTR(PRINTJOB_66, "\nwas printed but had the following errors:\n"));
			while ((i = getc(fp)) != EOF)
				putchar(i);
			(void) fclose(fp);
			break;
		case ACCESS:
			printf(MSGSTR(PRINTJOB_67, "\nwas not printed because it was not linked to the original file\n"));
		}
		fflush(stdout);
		(void) close(1);
	}
	(void) close(p[0]);
	(void) close(p[1]);
	wait(&s);
}

sendsend(user, bombed)
	char *user;
	int bombed;
{
	register int i, j, ndesc;
	int child, pid, status;
	register char *cp;
	char buf[100];
	struct stat stb;
	char msgbuf[256];
	char addr[80];
	FILE *fp;

	ndesc = getdtablesize();
	sprintf(addr, "%s@%s", user, fromhost);
	bzero(msgbuf, sizeof(msgbuf));
	sprintf(msgbuf, "[%s] ", printer);
	if (*jobname) {
		strcat(msgbuf, "(");
		strcat(msgbuf, jobname);
		strcat(msgbuf, ") ");
	}
	switch (bombed) {
	case OK:
		strcat(msgbuf, MSGSTR(PRINTJOB_68, "completed successfully"));
		break;
	default:
	case FATALERR:
		strcat(msgbuf, MSGSTR(PRINTJOB_69, "could not be printed: "));
		strcat(msgbuf, ERRORstr);
		break;
	case NOACCT:
		strcat(msgbuf, MSGSTR(PRINTJOB_70, "could not be printed without an account on "));
		strcat(msgbuf, host);
		break;
	case FILTERERR:
		if (stat(tmpfilename, &stb) < 0 || stb.st_size == 0 ||
		    (fp = fopen(tmpfilename, "r")) == NULL) {
			strcat(msgbuf, MSGSTR(PRINTJOB_71, "encountered problems during printing"));
			break;
		}
		strcat(msgbuf, MSGSTR(PRINTJOB_72, "encountered problems during printing: "));
		cp = &msgbuf[strlen(msgbuf)];
		while ((i = getc(fp)) != EOF && cp < &msgbuf[70])
			if (i <= ' ') {
			    if (*cp != ' ') *cp++ = ' ';
			}
			else
				*cp++ = i;
		*cp++ = '\0';
		if (i != EOF)
			strcat(msgbuf, "...");
		(void) fclose(fp);
		break;
	case ACCESS:
		strcat(msgbuf, MSGSTR(PRINTJOB_73, "was not printed because of access failure"));
	}
	if ((child = dofork(DORETURN)) == 0) {
/* make a two level fork until remote send problem resolved...hangs */
		if ((child = fork()) == 0) {
			for (i = 3; i < ndesc; i++)
				(void) close(i);
			if ((cp = rindex(SEND, '/')) != NULL)
				cp++;
			else
				cp = SEND;
			execl(SEND, cp, addr, "-all", msgbuf, 0);
			syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_58, "%s: can not execl %s: %m"), printer, SEND);
			writeStatus("");
			exit(-1);
		} else if (child < 0)
			syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_74, "%s: can not fork for send: %m"), printer);
		writeStatus("");
		exit(0);
	}
	else if (child > 0) {
		while ((pid = wait(&status)) > 0 && pid != child)
			;
	}
}

/*
 * dofork - fork with retries on failure
 */
dofork(action)
	int action;
{
	register int i, pid;

	for (i = 0; i < 20; i++) {
		if ((pid = fork()) < 0) {
			sleep((unsigned)(i*i));
			continue;
		}
#if !SEC_BASE
		/*
		 * Child should run as daemon instead of root
		 */
		if (pid == 0)
			setuid(DU);
#endif
		return(pid);
	}
	syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_75, "%s: can't fork"), printer);

	switch (action) {
	case DORETURN:
		return (-1);
	default:
		syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_77, "%s: bad action (%d) to dofork"), printer, action);
		/*FALL THRU*/
	case DOABORT:
		writeStatus("");
		exit(1);
	}
	/*NOTREACHED*/
}

/*
 * Kill child processes to abort current job.
 */
void abortpr()
{
	(void) unlink(tmpfilename);
	kill(0, SIGINT);
#if !SEC_MAC
	if (ofilter > 0)
		kill(ofilter, SIGCONT);
#endif
	while (wait(0) > 0)
		;
	writeStatus("");
	exit(0);
}

init()
{
	int status;

	if ((status = pgetent(line, printer)) < 0) {
		syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_79, "%s: can't open printer description file"), printer);
		writeStatus("");
		exit(1);
	} else if (status == 0) {
		syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_81, "unknown printer: %s"), printer);
		writeStatus("");
		exit(1);
	}
	if ((LP = pgetstr("lp", &bp)) == NULL)
		LP = DEFDEVLP;
	if ((RP = pgetstr("rp", &bp)) == NULL)
		RP = DEFLP;
	if ((LO = pgetstr("lo", &bp)) == NULL)
		LO = DEFLOCK;
	if ((ST = pgetstr("st", &bp)) == NULL)
		ST = DEFSTAT;
	if ((LF = pgetstr("lf", &bp)) == NULL)
		LF = DEFLOGF;
	if ((SD = pgetstr("sd", &bp)) == NULL)
		SD = DEFSPOOL;
	if ((CT = pgetstr("ct", &bp)) == NULL)
		CT = " "; 
	if ((OP = pgetstr("op", &bp)) == NULL)
		OP = " "; 
	if ((OS = pgetstr("os", &bp)) == NULL)
		OS = " ";
	if ((TS = pgetstr("ts", &bp)) == NULL)
		TS = " ";
	if ((DU = pgetnum("du")) < 0)
		DU = DEFUID;
	if ((FF = pgetstr("ff", &bp)) == NULL)
		FF = DEFFF;
	if ((PW = pgetnum("pw")) < 0)
		PW = DEFWIDTH;
	sprintf(&width[2], "%d", PW);
	if ((PL = pgetnum("pl")) < 0)
		PL = DEFLENGTH;
	sprintf(&length[2], "%d", PL);
	if ((PX = pgetnum("px")) < 0)
		PX = 0;
	sprintf(&pxwidth[2], "%d", PX);
	if ((PY = pgetnum("py")) < 0)
		PY = 0;
	sprintf(&pxlength[2], "%d", PY);
	RM = pgetstr("rm", &bp);
	/*
	 * Figure out whether the local machine is the same as the remote 
	 * machine entry (if it exists).  If not, then ignore the local
	 * queue information.
	 */
	 if (RM != (char *) NULL) {
		char name[256];
		struct hostent *hp;

		/* get the standard network name of the local host */
		gethostname(name, sizeof(name));
		name[sizeof(name)-1] = '\0';
		hp = gethostbyname(name);
		if (hp == (struct hostent *) NULL) {
		    syslog(LOG_LPR|LOG_ERR,
			MSGSTR(PRINTJOB_82, "%s: unable to get network name for local machine %s"),
			printer, name);
		    goto localcheck_done;
		} else strcpy(name, hp->h_name);

		/* get the standard network name of RM */
		hp = gethostbyname(RM);
		if (hp == (struct hostent *) NULL) {
		    syslog(LOG_LPR|LOG_ERR,
			MSGSTR(PRINTJOB_84, "%s: unable to get hostname for remote machine %s"), printer, RM);
		    goto localcheck_done;
		}

		/* if printer is not on local machine, ignore LP */
		if (strcmp(name, hp->h_name) != 0) *LP = '\0';
	}
localcheck_done:

	AF = pgetstr("af", &bp);
	OF = pgetstr("of", &bp);
	IF = pgetstr("if", &bp);
	XF = pgetstr("xf", &bp);
	RF = pgetstr("rf", &bp);
	TF = pgetstr("tf", &bp);
	NF = pgetstr("nf", &bp);
	DF = pgetstr("df", &bp);
	GF = pgetstr("gf", &bp);
	VF = pgetstr("vf", &bp);
	CF = pgetstr("cf", &bp);
	TR = pgetstr("tr", &bp);
	FA = pgetstr("fa", &bp);
	HN = pgetstr("hn", &bp);
	RS = pgetflag("rs");
	SF = pgetflag("sf");
	SH = pgetflag("sh");
	SB = pgetflag("sb");
	HL = pgetflag("hl");
	RW = pgetflag("rw");
	BR = pgetnum("br");
	if ((FC = pgetnum("fc")) < 0)
		FC = 0;
	if ((FS = pgetnum("fs")) < 0)
		FS = 0;
	if ((XC = pgetnum("xc")) < 0)
		XC = 0;
	if ((XS = pgetnum("xs")) < 0)
		XS = 0;
	tof = !pgetflag("fo");
#if SEC_MAC
	PS = pgetflag("ps");
#endif
}



struct bauds {
	int	baud;
	int	speed;
} bauds[] = {
	50,	B50,
	75,	B75,
	110,	B110,
	134,	B134,
	150,	B150,
	200,	B200,
	300,	B300,
	600,	B600,
	1200,	B1200,
	1800,	B1800,
	2400,	B2400,
	4800,	B4800,
	9600,	B9600,
	19200,	EXTA,
	38400,	EXTB,
	0,	0
};

/****************************************************************
 * lat_open(cxp) -- open a lat connection
 ****************************************************************/

static enum printer_type_e printer_type; /* type of printer */


/*********************************************************************/

static int
lat_open(cxp)
register CXP cxp;
{
        int i;
        int j = 0;
        int old_errno = 0;

        if (!(*LP && TS && (OP || OS))) {
                exit(1);
        }

        for (i = 1; ; i = i < 32 ? i << 1 : i) {
                cxp->cx_pr_fd = lat_conn(LP,TS,OP,OS);
                if (cxp->cx_pr_fd >= 0)
                    break;
                switch (errno) {
                    default:
                        exit(1);
                    case EADDRNOTAVAIL:
                    case ETIMEDOUT:
                        /* Try up to 30 times (~15 minutes) on time
outs */
                        if (++j > 30) {
                            exit(1);
                        }
                        break;
                    case EBUSY:
                    case EWOULDBLOCK:
                        break;
                }
                if ((i == 1) || (errno != old_errno)) {
		;
                }
                old_errno = errno;
                sleep(i);
        }

        set_tty_modes(cxp->cx_pr_fd);

        return 0;
}

/*************************************************************/

/*
 * cx_init -- initialise the object structure
 */
void
cx_init(cxp, connection_type)
register CXP cxp;
enum connection_type_e connection_type;
{
        cxp->cx_state = cxs_closed;
        cxp->cx_out_fd = cxp->cx_pr_fd = -1;
        cxp->cx_output_filter = NULL;
        cxp->cx_type = connection_type;
}

/*
 * cx_delete -- de-initialise the connection object
 */
void
cx_delete(cxp, on_heap)
CXP cxp;
int on_heap;
{
        if (cxp && on_heap) free(cxp);
}


/********************************************************************/


/*
 * set_tty_modes() -- do the relevant ioctls on a tty line
 */
static
set_tty_modes(pfd)
int pfd;
{
        struct sgttyb ttybuf;
        register struct bauds *bp;

        if (ioctl(pfd, TIOCGETP, (char *)&ttybuf) < 0) {
                exit(1);
        }
        if (BR > 0) {
                for (bp = bauds; bp->baud; bp++)
                    if (BR == bp->baud)
                        break;
                if (!bp->baud) {
                        exit(1);
                }
                ttybuf.sg_ispeed = ttybuf.sg_ospeed = bp->speed;
        }
        ttybuf.sg_flags &= ~FC;
        ttybuf.sg_flags |= FS;
        if (ioctl(pfd, TIOCSETP, (char *)&ttybuf) < 0) {
                exit(1);
        }
        if (XC) {
                if (ioctl(pfd, TIOCLBIC, &XC) < 0) {
                        exit(1);
                }
        }
        if (XS) {
                if (ioctl(pfd, TIOCLBIS, &XS) < 0) {
                        exit(1);
                }
        }
}

/********************************************************************
	END LAT OPEN ROUTINES
*********************************************************************/


/*
 * Acquire line printer or remote connection.
 */
#if SEC_MAC
openpr(startof, file)
	int	startof;
	char	*file;
#else
openpr()
#endif
{
	register int i, n, ndesc;
	int resp;
	CXP	cxp;
	enum connection_type_e lat;

	ndesc = getdtablesize();
	if (*LP) {
 /* check for lat first */
		if ( strcmp(CT, CT_choices[1]) == 0 ) 
		{
		cxp = (CXP)malloc(sizeof(struct connection));
		lat = 1;
		cx_init(cxp, lat);
		lat_open(cxp); /* we now have a new lat fd to printer
				  device called cxp->cx_pr_fd */
		pfd = cxp->cx_pr_fd;
		}
		else { /* non-lat printer do control */
		for (i = 1; ; i = i < 32 ? i << 1 : i) {
			pfd = open(LP, RW ? O_RDWR : O_WRONLY);
			if (pfd >= 0)
				break;
			if (errno == ENOENT) {
				syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_2, "%s: cannot open %s: %m"),
					printer, LP);
				writeStatus("");
				exit(1);
			}
			if (i == 1)
				writeStatus(MSGSTR(PRINTJOB_86, "waiting for %s to become ready (offline ?)"), printer);
			sleep(i);
		}
		if (isatty(pfd))
			setty();
		} /* end non-lat control */
#if SEC_MAC
		/*
		 * Spawn the lpbanner program to talk to the printer.
		 * It receives input from whatever other filter programs
		 * are called for, and handles the printing of standardized
		 * header and trailer pages with appropriate labels.
		 */
		{
			register int	status, n;
			register char	*cp;
			int	p[2];
			char	*argv[16];

			if ((status = pipe(p)) == 0 &&
			    (lpbpid = dofork(DORETURN)) == 0) {
				dup2(p[0], 0);	/* stdin is pipe from filter */
				dup2(pfd, 1);	/* stdout is printer */
				for (i = ndesc; --i > 2; )
					close(i);
				if ((cp = rindex(LPBANNER, '/')) == NULL)
					cp = LPBANNER;
				else
					++cp;
				n = 0;
				argv[n++] = cp;
				if (PS)	/* PostScript */
					argv[n++] = "-P";
				if (suppress_labels)
					argv[n++] = "-l";
				argv[n++] = "-u";
				argv[n++] = logname;
				argv[n++] = "-j";
				argv[n++] = cfjobname;
				argv[n++] = "-t";
				argv[n++] = jobname;
				argv[n++] = "-p";
				argv[n++] = printer;
				argv[n++] = file;
				argv[n] = NULL;
				execv(LPBANNER, argv);
				syslog(LOG_LPR|LOG_ERR, "%s: cannot exec %s: %m",
					printer, LPBANNER);
				writeStatus("");
				exit(1);
			}
			if (status == -1 || lpbpid == -1) {
				syslog(LOG_LPR|LOG_ERR, "%s: %s: %m", printer,
					status == -1
					    ? "can't create pipe to lpbanner"
					    : "can't fork for lpbanner");
				writeStatus("");
				exit(1);
			}
			close(p[0]);	/* child is using this for stdin */
			close(pfd);	/* child is using this for stdout */
			pfd = p[1];
		}
#endif
		writeStatus(MSGSTR(PRINTJOB_19, "%s is ready and printing"),
			printer);
#if !SEC_MAC
	} else if (RM != NULL) {
		for (i = 1; ; i = i < 256 ? i << 1 : i) {
			resp = -1;
			pfd = getport(RM);
			if (pfd >= 0) {
				(void) sprintf(line, "\2%s\n", RP);
				n = strlen(line);
				if (write(pfd, line, n) == n &&
				    (resp = response()) == '\0')
					break;
				(void) close(pfd);
			}
			if (i == 1) {
				if (resp < 0)
					writeStatus(MSGSTR(PRINTJOB_87,"waiting for %s to come up"), RM);
				else {
					writeStatus(MSGSTR(PRINTJOB_88, "waiting for queue to be enabled on %s"), RM);
					i = 256;
				}
			}
			sleep(i);
		}
		writeStatus(MSGSTR(PRINTJOB_89, "sending to %s"), RM);
		remote = 1;
#endif
	} else {
		syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_90, "%s: no line printer device or host name"),
			printer);
		writeStatus("");
		exit(1);
	}
	/*
	 * Start up an output filter, if needed.
	 */
#if SEC_MAC
	if (OF && startof)
#else
	if (OF)
#endif
	{
		int p[2];
		char *cp;

		pipe(p);
		if ((ofilter = dofork(DOABORT)) == 0) {	/* child */
			dup2(p[0], 0);		/* pipe is std in */
			dup2(pfd, 1);		/* printer is std out */
			for (i = 3; i < ndesc; i++)
				(void) close(i);
			if ((cp = rindex(OF, '/')) == NULL)
				cp = OF;
			else
				cp++;
			execl(OF, cp, width, length, 0);
			syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_91, "%s: cannot exec %s: %m"), printer, OF);
			writeStatus("");
			exit(1);
		}
		(void) close(p[0]);		/* close input side */
		ofd = p[1];			/* use pipe for output */
	} else {
		ofd = pfd;
		ofilter = 0;
	}
}

/*
 * setup tty lines.
 */
setty()
{
	struct sgttyb ttybuf;
	register struct bauds *bp;

	if (ioctl(pfd, TIOCEXCL, (char *)0) < 0) {
		syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_92, "%s: ioctl(TIOCEXCL): %m"), printer);
		exit(1);
	}
	if (ioctl(pfd, TIOCGETP, (char *)&ttybuf) < 0) {
		syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_93, "%s: ioctl(TIOCGETP): %m"), printer);
		exit(1);
	}
	if (BR > 0) {
		for (bp = bauds; bp->baud; bp++)
			if (BR == bp->baud)
				break;
		if (!bp->baud) {
			syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_94, "%s: illegal baud rate %d"), printer, BR);
			exit(1);
		}
		ttybuf.sg_ispeed = ttybuf.sg_ospeed = bp->speed;
	}
	ttybuf.sg_flags &= ~FC;
	ttybuf.sg_flags |= FS;
	if (ioctl(pfd, TIOCSETP, (char *)&ttybuf) < 0) {
		syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_95, "%s: ioctl(TIOCSETP): %m"), printer);
		exit(1);
	}
	if (XC || XS) {
		int ldisc = NTTYDISC;

		if (ioctl(pfd, TIOCSETD, &ldisc) < 0) {
			syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_96, "%s: ioctl(TIOCSETD): %m"), printer);
			exit(1);
		}
	}
	if (XC) {
		if (ioctl(pfd, TIOCLBIC, &XC) < 0) {
			syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_97, "%s: ioctl(TIOCLBIC): %m"), printer);
			exit(1);
		}
	}
	if (XS) {
		if (ioctl(pfd, TIOCLBIS, &XS) < 0) {
			syslog(LOG_LPR|LOG_ERR, MSGSTR(PRINTJOB_98, "%s: ioctl(TIOCLBIS): %m"), printer);
			exit(1);
		}
	}
}

/*VARARGS1*/
writeStatus(msg, a1, a2, a3)
	char *msg;
        caddr_t a1, a2, a3;
{
	register int fd;
	char buf[BUFSIZ];
	long curtime;

	curtime = time(0);

	umask(0);
	fd = open(ST, O_WRONLY|O_CREAT, 0664);
	if (fd < 0 || flock(fd, LOCK_EX) < 0) {
		syslog(LOG_LPR|LOG_ERR, "%s: %s: %m", printer, ST);
		exit(1);
	}
	ftruncate(fd, 0);
	sprintf(buf, "%24.24s: ", ctime(&curtime));
	sprintf(buf+26, msg, a1, a2, a3);
	strcat(buf, "\n");
	(void) write(fd, buf, strlen(buf));
	(void) close(fd);
}

/*  ffilecopy  --  very fast buffered file copy
 *
 *  Usage:  i = ffilecopy (here,there)
 *	int i;
 *	FILE *here, *there;
 *
 *  Ffilecopy is a very fast routine to copy the rest of a buffered
 *  input file to a buffered output file.  Here and there are open
 *  buffers for reading and writing (respectively); ffilecopy
 *  performs a file-copy faster than you should expect to do it
 *  yourself.  Ffilecopy returns 0 if everything was OK; EOF if
 *  there was any error.  Normally, the input file will be left in
 *  EOF state (feof(here) will return TRUE), and the output file will be
 *  flushed (i.e. all data on the file rather in the core buffer).
 *  It is not necessary to flush the output file before ffilecopy.
 */

int filecopy();

int ffilecopy (here,there)
FILE *here, *there;
{
	register int i, herefile, therefile;

	herefile = fileno(here);
	therefile = fileno(there);

	if (fflush (there) == EOF)		/* flush pending output */
		return (EOF);

	if ((here->_cnt) > 0) {			/* flush buffered input */
		i = write (therefile, here->_ptr, here->_cnt);
		if (i != here->_cnt)  return (EOF);
		here->_ptr = here->_base;
		here->_cnt = 0;
	}

	i = filecopy (herefile, therefile);	/* fast file copy */
	if (i < 0)  return (EOF);

	(here->_flag) |= _IOEOF;		/* indicate EOF */

	return (0);
}

/*  filecopy  --  copy a file from here to there
 *
 *  Usage:  i = filecopy (here,there);
 *	int i, here, there;
 *
 *  Filecopy performs a fast copy of the file "here" to the
 *  file "there".  Here and there are both file descriptors of
 *  open files; here is open for input, and there for output.
 *  Filecopy returns 0 if all is OK; -1 on error.
 *
 *  I have performed some tests for possible improvements to filecopy.
 *  Using a buffer size of 10240 provides about a 1.5 times speedup
 *  over 512 for a file of about 200,000 bytes.  Of course, other
 *  buffer sized should also work; this is a rather arbitrary choice.
 *  I have also tried inserting special startup code to attempt
 *  to align either the input or the output file to lie on a
 *  physical (512-byte) block boundary prior to the big loop,
 *  but this presents only a small (about 5% speedup, so I've
 *  canned that code.  The simple thing seems to be good enough.
 */

#define BUFFERSIZE 10240

int filecopy (here,there)
int here,there;
{
	register int kount;
	char buffer[BUFFERSIZE];
	kount = 0;
	while (kount == 0 && (kount=read(here,buffer,BUFFERSIZE)) > 0)
		kount -= write (there,buffer,kount);
	return (kount ? -1 : 0);
}

#if SEC_MAC
/*
 * Wait for the lpbanner program to terminate.  It will do so as a result
 * of an EOF on its input pipe when the filter chain terminates.
 */
closepr()
{
	int	pid, status;

	if (ofilter > 0) {
		close(ofd);
		while ((pid = wait(&status)) > 0 && pid != ofilter)
			;
	}
	/*if ( CT == CT_choices[1] )
	lat_close(cxp);		
	else*/
	close(pfd);
	if (lpbpid > 0)
		while ((pid = wait(&status)) > 0 && pid != lpbpid)
			;
	ofilter = lpbpid = 0;
}
#endif




/********************************************************************
 *		l a t _ c o n n
 *
 * Solicit a connection between a LAT terminal and a terminal server port.
 *
 * Returns:		0 if success
 *			error code if not
 *
 * Inputs:
 *	subjport	= Name of subject port (ULTRIX LAT terminal)
 *	objnam		= Name of object (terminal server node)
 *	objport		= Name of object port
 *	objsrvc		= Name of object service
 *	
 ******************************************************************/

lat_conn(subjport,objnam,objport,objsrvc)
    char *subjport;
    char *objnam;
    char *objport;
    char *objsrvc;
{
    int fd;
    struct ltattyi ltattyi;

    fd = open("/dev/lat", O_RDWR|O_NONBLOCK|O_NOCTTY, 0);
    if (fd < 0)
	return -1;
    if (strlen(objnam) >= sizeof(ltattyi.li_host)) {
	errno = EMSGSIZE;
	return -1;
    }
    if (strlen(objport) >= sizeof(ltattyi.li_port)) {
	errno = EMSGSIZE;
	return -1;
    }
    if (strlen(objsrvc) >= sizeof(ltattyi.li_service)) {
	errno = EMSGSIZE;
	return -1;
    }
    strcpy(ltattyi.li_host, objnam);
    strcpy(ltattyi.li_port, objport);
    strcpy(ltattyi.li_service, objsrvc);
    if (ioctl(fd, LIOCCONN, &ltattyi) < 0) {
	int error = errno;
	(void) close(fd);
	errno = error;
        return -1;
    } 
    return fd;
}

