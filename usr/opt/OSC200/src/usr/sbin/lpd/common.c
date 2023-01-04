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
static char	*sccsid = "@(#)$RCSfile: common.c,v $ $Revision: 4.2.3.5 $ (DEC) $Date: 1992/10/14 14:26:23 $";
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
 * common.c	5.4 (Berkeley) 6/30/88
 */

/*
 * Routines and data common to all the line printer functions.
 */
#include <locale.h>
#include "printer_msg.h"
#include <NLchar.h>
#include <NLctype.h>
#include "lp.h"

nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_PRINTER,n,s)


int	DU;		/* daeomon user-id */
int	MX;		/* maximum number of blocks to copy */
int	MC;		/* maximum number of copies allowed */
char	*LP;		/* line printer device name */
char	*RM;		/* remote machine name */
char	*RP;		/* remote printer name */
char	*LO;		/* lock file name */
char	*ST;		/* status file name */
char	*SD;		/* spool directory */
char	*AF;		/* accounting file */
char	*LF;		/* log file for error messages */
char	*OF;		/* name of output filter (created once) */
char	*IF;		/* name of input filter (created per job) */
char	*RF;		/* name of fortran text filter (per job) */
char	*TF;		/* name of troff filter (per job) */
char	*NF;		/* name of ditroff filter (per job) */
char	*DF;		/* name of tex filter (per job) */
char	*GF;		/* name of graph(1G) filter (per job) */
char	*VF;		/* name of vplot filter (per job) */
char	*CF;		/* name of cifplot filter (per job) */
char	*PF;		/* name of vrast filter (per job) */
char	*XF;		/* name of non-filter filter (per job) */
char	*FF;		/* form feed string */
char	*TR;		/* trailer string to be output when Q empties */
char	*HN;		/* host name that is printer (-n filter arg) */
char	*FA;		/* additional output filter argument (-A filter arg) */
short	SC;		/* suppress multiple copies */
short	SF;		/* suppress FF on each print job */
short	SH;		/* suppress header page */
short	SB;		/* short banner instead of normal header */
short	HL;		/* print header last */
short	RW;		/* open LP for reading and writing */
short	PW;		/* page width */
short	PL;		/* page length */
short	PX;		/* page width in pixels */
short	PY;		/* page length in pixels */
short	BR;		/* baud rate if lp is a tty */
int	FC;		/* flags to clear if lp is a tty */
int	FS;		/* flags to set if lp is a tty */
int	XC;		/* flags to clear for local mode */
int	XS;		/* flags to set for local mode */
short	RS;		/* restricted to those with local accounts */
#if SEC_MAC
short	PS;		/* printer expects PostScript */
#endif
char     *TS;		/* LAT terminal server name */
char     *OP;		/* LAT terminal server port ID */
char     *OS;		/* LAT service (could be > 1 server) */
char	 *CT;		/* connection type */

char	line[BUFSIZ];
char	pbuf[BUFSIZ/2];	/* buffer for printcap strings */
char	*bp = pbuf;	/* pointer into pbuf for pgetent() */
char	*name;		/* program name */
char	*printer;	/* printer name */
char	host[MAXHOSTNAMELEN];	/* host machine name */
char	*from = host;	/* client's machine name */

/*
 * Create a connection to the remote printer server.
 * Most of this code comes from rcmd.c.
 */
getport(rhost)
	char *rhost;
{
	struct hostent *hp;
	struct servent *sp;
	struct sockaddr_in sin;
	int s, timo = 1, lport = IPPORT_RESERVED - 1;
	int err;

	/*
	 * Get the host address and port number to connect to.
	 */
	if (rhost == NULL)
		fatal(MSGSTR(COMMON_1, "no remote host to connect to"));
	hp = gethostbyname(rhost);
	if (hp == NULL)
		fatal(MSGSTR(COMMON_2, "unknown host %s"), rhost);
	sp = getservbyname("printer", "tcp");
	if (sp == NULL)
		fatal(MSGSTR(COMMON_3, "printer/tcp: unknown service"));
	bzero((char *)&sin, sizeof(sin));
	bcopy(hp->h_addr, (caddr_t)&sin.sin_addr, hp->h_length);
	sin.sin_family = hp->h_addrtype;
	sin.sin_port = sp->s_port;

	/*
	 * Try connecting to the server.
	 */
retry:
#if	SO_USEPRIV
	s = socket(AF_INET,SOCK_STREAM,0);
	if (s >= 0) {
		int one;
		one = 1;
		if (setsockopt(s,SOL_SOCKET,SO_USEPRIV,&one,sizeof(one)) < 0) {
			syslog(LOG_ERR,MSGSTR(COMMON_4, "getport: couldn't set SO_USEPRIV: %m"));
			close(s);
			/* use old mechanism for compatibility */
			s = rresvport(&lport);
		}
	}
#else	/* SO_USEPRIV */
	s = rresvport(&lport);
#endif	/* SO_USEPRIV */

	if (s < 0)
		return(-1);
	if (connect(s, (caddr_t)&sin, sizeof(sin), 0) < 0) {
		err = errno;
		(void) close(s);
		errno = err;
		if (errno == EADDRINUSE) {
			lport--;
			goto retry;
		}
		if (errno == ECONNREFUSED && timo <= 16) {
			sleep(timo);
			timo *= 2;
			goto retry;
		}
		return(-1);
	}
	return(s);
}

/*
 * Getline reads a line from the control file cfp, removes tabs, converts
 *  new-line to null and leaves it in line.
 * Returns 0 at EOF or the number of characters read.
 */
getline(cfp)
	FILE *cfp;
{
	register int linel = 0;
	register char *lp = line;
	register c;

	while ((c = getc(cfp)) != '\n') {
		if (c == EOF)
			return(0);
		if (c == '\t') {
			do {
				*lp++ = ' ';
				linel++;
			} while ((linel & 07) != 0);
			continue;
		}
		*lp++ = c;
		linel++;
	}
	*lp++ = '\0';
	return(linel);
}

/*
 * Scan the current directory and make a list of daemon files sorted by
 * creation time.
 * Return the number of entries and a pointer to the list.
 */
getq(namelist)
	struct queue *(*namelist[]);
{
	register struct dirent *d;
	register struct queue *q, **queue;
	register int nitems;
	struct stat stbuf;
	int arraysz;
	int compar();
	DIR *dirp;

	if ((dirp = opendir(SD)) == NULL)
		return(-1);
	if (fstat(dirp->dd_fd, &stbuf) < 0)
		goto errdone;

	/*
	 * Estimate the array size by taking the size of the directory file
	 * and dividing it by a multiple of the minimum size entry. 
	 */
	arraysz = (stbuf.st_size / 24);
	queue = (struct queue **)malloc(arraysz * sizeof(struct queue *));
	if (queue == NULL)
		goto errdone;

	nitems = 0;
	while ((d = readdir(dirp)) != NULL) {
		if (d->d_name[0] != 'c' || d->d_name[1] != 'f')
			continue;	/* daemon control files only */
		if (stat(d->d_name, &stbuf) < 0)
			continue;	/* Doesn't exist */
		q = (struct queue *)malloc(sizeof(time_t)+strlen(d->d_name)+1);
		if (q == NULL)
			goto errdone;
		q->q_time = stbuf.st_mtime;
		strcpy(q->q_name, d->d_name);
		/*
		 * Check to make sure the array has space left and
		 * realloc the maximum size.
		 */
		if (++nitems > arraysz) {
			queue = (struct queue **)realloc((char *)queue,
				(stbuf.st_size/12) * sizeof(struct queue *));
			if (queue == NULL)
				goto errdone;
		}
		queue[nitems-1] = q;
	}
	closedir(dirp);
	if (nitems)
		qsort(queue, nitems, sizeof(struct queue *), compar);
	*namelist = queue;
	return(nitems);

errdone:
	closedir(dirp);
	return(-1);
}

/*
 * a modified version of getq() which indicates whether a lpd should
 * start up or not. We use lots of printers but only a few tend to have
 * print requests pending. This avoids having 40 or more processes around
 * at start-up...
 */
checkq(spooldir)
char *spooldir;
{
	register struct dirent *d;
	register int nitems;
	struct stat stbuf;
	DIR *dirp;

	if ((dirp = opendir(spooldir)) == NULL)
		return(-1);
	if (fstat(dirp->dd_fd, &stbuf) < 0)
		goto errdone;

	nitems = 0;
	while ((d = readdir(dirp)) != NULL) {
		if (d->d_name[0] != 'c' || d->d_name[1] != 'f')
			continue;	/* daemon control files only */
		if (stat(d->d_name, &stbuf) < 0)
			continue;	/* Doesn't exist */
		/* found a potential queue entry */
		nitems++;
		break;
	}
	closedir(dirp);
	return(nitems);

errdone:
	closedir(dirp);
	return(-1);
}

/*
 * Compare modification times.
 */
static
int compar(p1, p2)
	register struct queue **p1, **p2;
{
	int v1,v2;

	/* compar priority letter in queue name */
	v1 = (*p1)->q_name[3];
	if (!islower(v1)) v1 = NICELET;
	v2 = (*p2)->q_name[3];
	if (!islower(v2)) v2 = NICELET;
	if (v1 < v2)
		return(-1);
	if (v1 > v2)
		return(1);
	if ((*p1)->q_time < (*p2)->q_time)
		return(-1);
	if ((*p1)->q_time > (*p2)->q_time)
		return(1);
	return(0);
}

/*VARARGS1*/
fatal(msg, a1, a2, a3)
	char *msg;
        caddr_t a1, a2, a3;
{
	if (from != host)
		fprintf(stderr, "%s: ", host);
	fprintf(stderr, "%s: ", name);
/*	if (printer)
 *		fprintf(stderr, "%s: ", printer); 
 */
	fprintf(stderr, msg, a1, a2, a3);
	putc('\n', stderr);
	/* try to record this for debugging */
	syslog(LOG_DEBUG,msg,a1,a2,a3);
	exit(1);
}

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

	strcpy(unk,MSGSTR(COMMON_5, "Unknown error "));
	*itoa(&unk,cod) = '\0';

	return(unk);
}

/*
 * my version of gethostbyname
 * This version is neccessary to avoid the time taken by the resolving
 * process using the local context which is valid for user interfaces but
 * not for protocol modules.
 *
 * NOTE: there is a def in lp.h which maps gethostbyname to mygethostbyname
 */
struct hostent *mygethostbyname(nam)
char *nam;
{
    struct hostent *he;
    char dotnam[256];
    register int len;

    strncpy(dotnam,nam,sizeof(dotnam));
    dotnam[sizeof(dotnam)-2] = '\0';
    len = strlen(dotnam);
    if (dotnam[len-1] != '.') {
	dotnam[len++] = '.';
	dotnam[len] = '\0';
    }
#undef gethostbyname
    he = gethostbyname(dotnam);
    if (he == NULL)
	syslog(LOG_DEBUG,MSGSTR(COMMON_6, "gethostbyname '%s' %s %d rs %s"),
	       dotnam,errmsg(-1),h_errno);
    return(he);
}
