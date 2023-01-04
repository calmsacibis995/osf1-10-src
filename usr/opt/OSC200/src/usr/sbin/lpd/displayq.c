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
static char	*sccsid = "@(#)$RCSfile: displayq.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/11/23 22:22:30 $";
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
 * displayq.c	5.8 (Berkeley) 6/30/88
 */


/*
 * Routines to display the state of the queue.
 */
#include "lp.h"


#define JOBCOL	40		/* column for job # in -l format */
#define OWNCOL	11		/* start of Owner column in normal */
#define SIZCOL	66		/* start of Size column in normal */
#define PRICOL	7		/* column for priority in normal */

/*
 * Stuff for handling job specifications
 */
extern char	*user[];	/* users to process */
extern int	users;		/* # of users in user array */
extern int	requ[];		/* job number of spool entries */
extern int	requests;	/* # of spool requests */

int	lflag;		/* long output option */
char	current[40];	/* current file being printed */
int	garbage;	/* # of garbage cf files */
int	rank;		/* order to be printed (-1=none, 0=active) */
long	totsize;	/* total print job size in bytes */
int	first;		/* first file in ``files'' column? */
int	col;		/* column on screen */
int	sendtorem;	/* are we sending to a remote? */
char	file[132];	/* print file name */

/*
 * Display the current state of the queue. Format = 1 if long format.
 */
int displayq(format)
	int format;
{
	register struct queue *q;
	register int i, nitems, fd;
	register char	*cp;
	struct queue **queue;
	struct stat statb;
	FILE *fp;
	char c;
	int trys;
	int stlockfd;
	int havedaemon;

	lflag = format;
	totsize = 0;
	rank = -1;

	if ((i = pgetent(line, printer)) < 0)
		fatal(MSGSTR(DISPLAYQ_1, "cannot open printer description file"));
	else if (i == 0)
		fatal(MSGSTR(DISPLAYQ_2, "%s: unknown printer"), printer);
	if ((LP = pgetstr("lp", &bp)) == NULL)
		LP = DEFDEVLP;
	if ((RP = pgetstr("rp", &bp)) == NULL)
		RP = DEFLP;
	if ((SD = pgetstr("sd", &bp)) == NULL)
		SD = DEFSPOOL;
	if ((LO = pgetstr("lo", &bp)) == NULL)
		LO = DEFLOCK;
	if ((ST = pgetstr("st", &bp)) == NULL)
		ST = DEFSTAT;
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
		    printf(MSGSTR(DISPLAYQ_3,
			"unable to get network name for local machine %s\n"),
			name);
		    goto localcheck_done;
		} else (void) strcpy(name, hp->h_name);

		/* get the network standard name of RM */
		hp = gethostbyname(RM);
		if (hp == (struct hostent *) NULL) {
		    printf(MSGSTR(DISPLAYQ_4,
			"unable to get hostname for remote machine %s\n"),
			RM);
		    goto localcheck_done;
		}

		/* if printer is not on local machine, ignore LP */
		if (strcmp(name, hp->h_name)) {
			*LP = '\0';
			++sendtorem;
		}
	}
localcheck_done:

	/*
	 * Print out local queue
	 * Find all the control files in the spooling directory
	 */
	if (chdir(SD) < 0)
		fatal(MSGSTR(DISPLAYQ_5, "cannot chdir to spooling directory"));
	/* do getq after the stat */
	havedaemon = 0;	/* we do NOT have a daemon */
	if (stat(LO, &statb) >= 0) {
		if (statb.st_mode & 0100) {
			if (sendtorem)
				printf("%s: ", host);
			printf(MSGSTR(DISPLAYQ_7, "Warning: %s is down:\n"), printer);
			/* we print the status message later */
			havedaemon++;	/* we don't need a daemon */
		}
		if (statb.st_mode & 010) {
			if (sendtorem)
				printf("%s: ", host);
			printf(MSGSTR(DISPLAYQ_9, "Warning: %s queue is turned off\n"), printer);
		}
	}

	/*
	 * anti-race condition code for queue display. what a hack!
	 */
	stlockfd = -1;
	nitems = 0;
	for(trys = 0;trys < 5;trys++) {
		if (trys > 0) {
		    /* sleep for a while and try again */
		    sleep(3);
		}
		/* free up the old queue we got (if we have one */
		if (nitems > 0) {
			for (i = 0; i < nitems; i++) {
				q = queue[i];
				free(q);
			}
			free(queue);
		}
		/*
		 * Lock the status file....It is a shared file lock...
		 * Side effect: will cause a status file up date by lpd
		 * to block which will eliminate the race condition problems
		 * in the "Warning: No Daemon present" message.
		 */
		if (stlockfd < 0) {
			stlockfd = open(ST, O_RDONLY);
			if (stlockfd >= 0)
				(void) flock(stlockfd, LOCK_SH);
		}
		/* get the queue */
		if ((nitems = getq(&queue)) < 0)
			fatal(MSGSTR(DISPLAYQ_6, "cannot examine spooling area\n"));
		/* if we have no items then break out */
		if (nitems <= 0)
			break;
#if SEC_BASE
		/*
		 * If we already know the daemon is not running because
		 * the printer has been stopped, just break out now
		 * without wasting any more time.
		 */
		else if (havedaemon)
			break;
#endif
		/* have items and we want to see if daemon is running */
		else {
			register char *cp;
			register int c;
			fp = fopen(LO, "r");
			if (fp == NULL) {
				continue;	/* no lock file. */
			}
			/* get daemon pid */
			cp = current;
			for(;;) {
				c = getc(fp);
				if (c == EOF || c == '\n') break;
				*cp++ = c;
			}
			*cp = '\0';
			i = atoi(current);
			if (i <= 1 || kill(i, 0) < 0) {
				(void) fclose(fp);
				continue;
			}
			/* read current file name */
			cp = current;
			for(;;) {
				c = getc(fp);
				if (c == EOF || c == '\n') break;
				*cp++ = c;
			}
			*cp = '\0';
			(void) fclose(fp);
			/* indicate we have the daemon */
			havedaemon++;
			/* and terminate loop */
			break;
		}
	}
	/* okay release lock on status file (now so lpd can run a bit) */
	if (stlockfd >= 0)
		close(stlockfd);
	stlockfd = -1;
	/* present warning if neccessary */
	if (nitems > 0 && !havedaemon)
	    warn();
	/*
	 * Print the status file.
	 */
	if (sendtorem)
		printf("%s: ", host);
	fd = open(ST, O_RDONLY);
	if (fd >= 0) {
		(void) flock(fd, LOCK_SH);
		while ((i = read(fd, line, sizeof(line))) > 0)
			(void) fwrite(line, 1, i, stdout);
		(void) close(fd);	/* unlocks as well */
	} else
		putchar('\n');
	if (nitems) {
		/*
		 * Now, examine the control files and print out the jobs to
		 * be done for each user.
		 */
		if (!lflag)
			header();
		for (i = 0; i < nitems; i++) {
			q = queue[i];
			inform(q->q_name);
			free(q);
		}
		free(queue);
	}
	if (!sendtorem) {
		if (nitems == 0)
			puts(MSGSTR(DISPLAYQ_10, "no entries"));
		return nitems > 0;
	}

	/*
	 * Print foreign queue
	 * Note that a file in transit may show up in either queue.
	 */
	if (nitems)
		putchar('\n');
	(void) sprintf(line, "%c%s", format + '\3', RP);
	cp = line;
	for (i = 0; i < requests; i++) {
		cp += strlen(cp);
		(void) sprintf(cp, " %d", requ[i]);
	}
	for (i = 0; i < users; i++) {
		cp += strlen(cp);
		*cp++ = ' ';
		(void) strcpy(cp, user[i]);
	}
	strcat(line, "\n");
	fd = getport(RM);
	if (fd < 0) {
		if (from != host)
			printf("%s: ", host);
		printf(MSGSTR(DISPLAYQ_11, "connection to %s is down\n"), RM);
	}
	else {
		i = strlen(line);
		if (write(fd, line, i) != i)
			fatal(MSGSTR(DISPLAYQ_12, "Lost connection"));
		nitems = 0;
		while ((i = read(fd, line, sizeof(line))) > 0)
		{
		    line[i] = '\0';
		    if (strstr (line, MSGSTR(DISPLAYQ_10, "no entries")) == NULL)
	 		nitems++;

		    (void) fwrite(line, 1, i, stdout);
		}
		(void) close(fd);
	}
    return(nitems);
}

/*
 * Print a warning message if there is no daemon present.
 */
warn()
{
	if (sendtorem)
		printf("\n%s: ", host);
	puts(MSGSTR(DISPLAYQ_13, "Warning: no daemon present"));
	current[0] = '\0';
}

/*
 * Print the header for the short listing format
 */
header()
{
	printf(MSGSTR(DISPLAYQ_14, "Rank   Pri Owner      Job  Files"));
	col = strlen(MSGSTR(DISPLAYQ_14, "Rank   Pri Owner      Job  Files"))+1;
	blankfill(SIZCOL);
	printf(MSGSTR(DISPLAYQ_15, "Total Size\n"));
}

inform(cf)
	char *cf;
{
	register int j, k;
	register char *cp;
	FILE *cfp;
	int seqoff, nice;

	/*
	 * There's a chance the control file has gone away
	 * in the meantime; if this is the case just keep going
	 */
	if ((cfp = fopen(cf, "r")) == NULL)
		return;

	seqoff = 3;
	if (!isdigit(*(cf+seqoff)))
		nice = *(cf+seqoff++) - NICELET;
	else
		nice = 0;
	if (rank < 0)
		rank = 0;
	if (sendtorem || garbage || strcmp(cf, current))
		rank++;
	j = 0;
	while (getline(cfp)) {
		switch (line[0]) {
		case 'P': /* Was this file specified in the user's list? */
			if (!inlist(line+1, cf)) {
				fclose(cfp);
				return;
			}
			if (lflag) {
				printf("\n%s: ", line+1);
				col = strlen(line+1) + 2;
				prank(rank);
				blankfill(JOBCOL);
				printf(MSGSTR(DISPLAYQ_16, " [job %s]\n"), cf+seqoff);
			} else {
				col = 0;
				prank(rank);
				blankfill(PRICOL);
				printf("%-3d ",nice);
				col += 4;
				blankfill(OWNCOL);
				printf("%-10s %-3d  ", line+1, atoi(cf+seqoff));
				col += 16;
				first = 1;
			}
			continue;
		default: /* some format specifer and file name? */
			if (line[0] < 'a' || line[0] > 'z')
				continue;
			if (j == 0 || strcmp(file, line+1) != 0)
				(void) strcpy(file, line+1);
			j++;
			continue;
		case 'N':
			show(line+1, file, j);
			file[0] = '\0';
			j = 0;
		}
	}
	fclose(cfp);
	if (!lflag) {
		blankfill(SIZCOL);
		printf(MSGSTR(DISPLAYQ_17, "%ld bytes\n"), totsize);
		totsize = 0;
	}
}

inlist(name, file)
	char *name, *file;
{
	register int *r, n;
	register char **u, *cp;

	if (users == 0 && requests == 0)
		return(1);
	/*
	 * Check to see if it's in the user list
	 */
	for (u = user; u < &user[users]; u++)
		if (!strcmp(*u, name))
			return(1);
	/*
	 * Check the request list
	 */
	cp = file+3;
	if (!isdigit(*cp))
	    cp++;
	for (n = 0; isdigit(*cp); )
		n = n * 10 + (*cp++ - '0');
	for (r = requ; r < &requ[requests]; r++)
		if (*r == n && !strcmp(cp, from))
			return(1);
	return(0);
}

show(nfile, file, copies)
	register char *nfile, *file;
{
	if (strcmp(nfile, " ") == 0)
		nfile = MSGSTR(DISPLAYQ_18, "(standard input)");
	if (lflag)
		ldump(nfile, file, copies);
	else
		dump(nfile, file, copies);
}

/*
 * Fill the line with blanks to the specified column
 */
blankfill(n)
	register int n;
{
	while (col++ < n)
		putchar(' ');
}

/*
 * Give the abbreviated dump of the file names
 */
dump(nfile, file, copies)
	char *nfile, *file;
{
	register short n, fill;
	struct stat lbuf;

	/*
	 * Print as many files as will fit
	 *  (leaving room for the total size)
	 */
	 fill = first ? 0 : 2;	/* fill space for ``, '' */
	 if (((n = strlen(nfile)) + col + fill) >= SIZCOL-4) {
		if (col < SIZCOL) {
			printf(" ..."), col += 4;
			blankfill(SIZCOL);
		}
	} else {
		if (first)
			first = 0;
		else
			printf(", ");
		printf("%s", nfile);
		col += n+fill;
	}
	if (*file && !stat(file, &lbuf))
		totsize += copies * lbuf.st_size;
}

/*
 * Print the long info about the file
 */
ldump(nfile, file, copies)
	char *nfile, *file;
{
	struct stat lbuf;

	putchar('\t');
	if (copies > 1)
		printf(MSGSTR(DISPLAYQ_19, "%-2d copies of %-19s"), copies, nfile);
	else
		printf("%-32s", nfile);
	if (*file && !stat(file, &lbuf))
		printf(MSGSTR(DISPLAYQ_20, " %ld bytes"), lbuf.st_size);
	else
		printf(MSGSTR(DISPLAYQ_21, " ??? bytes"));
	putchar('\n');
}

/*
 * Print the job's rank in the queue,
 *   update col for screen management
 */
prank(n)
{
	char line[100];
	static char *r[] = {
		"th", "st", "nd", "rd", "th", "th", "th", "th", "th", "th"
	};

	if (n == 0) {
		printf(MSGSTR(DISPLAYQ_22, "active"));
		col += 6;
		return;
	}
	if ((n/10) == 1)
		(void) sprintf(line, "%dth", n);
	else
		(void) sprintf(line, "%d%s", n, r[n%10]);
	col += strlen(line);
	printf("%s", line);
}

