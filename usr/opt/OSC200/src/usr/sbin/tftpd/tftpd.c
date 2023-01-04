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
static char	*sccsid = "@(#)$RCSfile: tftpd.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1994/01/10 17:23:36 $";
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
 * tftpd.c
 *
 *	Revision History:
 *
 * 20-Jun-91    Mary Walker
 *      added OSF 1.0.1 bug fixes
 *      added /etc/tftptab for diskless support
 *
 * 01-Apr-91	Fred Canter
 *	MIPS C 2.20+, changes for -std.
 *
 * 12-Mar-91	Fred Canter
 *	MIPS C 2.20+
 *
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*

*/
/* 
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
/*
#ifndef lint
char copyright[] =
"Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "tftpd.c	5.12 (Berkeley) 6/1/90";
#endif not lint
*/

#include <nl_types.h>
#include <locale.h>
#include "tftpd_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TFTPD,n,s) 


/*
 * Trivial file transfer protocol server.
 *
 * This version includes many modifications by Jim Guyton <guyton@rand-unix>
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/stat.h>


#include <netinet/in.h>

#include <arpa/tftp.h>

#include <netdb.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <sys/syslog.h>
#include <string.h>

#define	TIMEOUT		5

extern	int errno;
struct	sockaddr_in sin = { AF_INET };
int	peer;
int	rexmtval = TIMEOUT;
int	maxtimeout = 5*TIMEOUT;


#define	PKTSIZE	SEGSIZE+4
char	buf[PKTSIZE];
char	ackbuf[PKTSIZE];
struct	sockaddr_in from;
int	fromlen;

#define MAXARG  64
#define MAX_FILES 256
char    *rpath = (char *)0;
char    *dirs[MAXARG+1];
char    *files[MAX_FILES];
char    *tftptab = "/etc/tftptab";
void    readtab();
int	debug=0;

main(ac,av)
int ac;
char **av;
{
	register struct tftphdr *tp;
	register int n = 0, dircnt = 0;
	int on = 1;

	setlocale(LC_ALL, "");
	catd = NLcatopen(MF_TFTPD,NL_CAT_LOCALE);

	openlog("tftpd", LOG_PID, LOG_DAEMON);

	err_load();
	av++;

	while (ac-- > 1 && n < MAXARG) {
	 switch(av[n][0]) {
	  case '-': switch(av[n][1]) { 
		     case 'd': debug++;
			       break;
		     case 'r': rpath = av[n+1];
			       n++;
			       ac--;
			       break;
	             default: syslog(LOG_INFO, MSGSTR(BADOPT, "invalid option -%c (ignored)\n"), av[n][1]);
			      break;
		    }
		    n++;
		    break;
	  case '/': dirs[dircnt++] = av[n];
		    n++;
		    break;
	  default:  n++;
		    break;
	  }
	}
	readtab();
	if (ioctl(0, FIONBIO, &on) < 0) {
		syslog(LOG_ERR, MSGSTR( TFTPD_IOCTL,  "ioctl(FIONBIO): %m\n"));
		exit(1);
	}

	fromlen = sizeof (from);
	n = recvfrom(0, buf, sizeof (buf), 0,
	    (caddr_t)&from, &fromlen);
	if (n < 0) {
		syslog(LOG_ERR, MSGSTR( TFTPD_RECVFROM, "recvfrom: %m\n"));
		exit(1);
	}
	/*
	 * Now that we have read the message out of the UDP
	 * socket, we fork and exit.  Thus, inetd will go back
	 * to listening to the tftp port, and the next request
	 * to come in will start up a new instance of tftpd.
	 *
	 * We do this so that inetd can run tftpd in "wait" mode.
	 * The problem with tftpd running in "nowait" mode is that
	 * inetd may get one or more successful "selects" on the
	 * tftp port before we do our receive, so more than one
	 * instance of tftpd may be started up.  Worse, if tftpd
	 * break before doing the above "recvfrom", inetd would
	 * spawn endless instances, clogging the system.
	 */
	{
		pid_t pid;
		int i, j;

		for (i = 1; i < 20; i++) {
		    pid = fork();
		    if (pid < 0) {
				sleep(i);
				/*
				 * flush out to most recently sent request.
				 *
				 * This may drop some request, but those
				 * will be resent by the clients when
				 * they timeout.  The positive effect of
				 * this flush is to (try to) prevent more
				 * than one tftpd being started up to service
				 * a single request from a single client.
				 */
				j = sizeof from;
				i = recvfrom(0, buf, sizeof (buf), 0,
				    (caddr_t)&from, &j);
				if (i > 0) {
					n = i;
					fromlen = j;
				}
		    } else {
				break;
		    }
		}
		if (pid < 0) {
			syslog(LOG_ERR, MSGSTR( TFTPD_FORK, "fork: %m\n"));
			exit(1);
		} else if (pid != 0) {
			exit(0);
		}
	}
	from.sin_family = AF_INET;
	alarm(0);
	close(0);
	close(1);
	peer = socket(AF_INET, SOCK_DGRAM, 0);
	if (peer < 0) {
		syslog(LOG_ERR, MSGSTR( TFTPD_SOCKET, "socket: %m\n"));
		exit(1);
	}
	if (bind(peer, (caddr_t)&sin, sizeof (sin)) < 0) {
		syslog(LOG_ERR, MSGSTR( TFTPD_BIND, "bind: %m\n"));
		exit(1);
	}
	if (connect(peer, (caddr_t)&from, sizeof(from)) < 0) {
		syslog(LOG_ERR, MSGSTR( TFTPD_CONNECT, "connect: %m\n"));
		exit(1);
	}
        tp = (struct tftphdr *)buf;
	tp->th_opcode = ntohs(tp->th_opcode);
	if (tp->th_opcode == RRQ || tp->th_opcode == WRQ)
		tftp(tp, n);
	exit(1);
}

int	validate_access();
int	sendfile(), recvfile();

struct formats {
	char	*f_mode;
	int	(*f_validate)();
	int	(*f_send)();
	int	(*f_recv)();
	int	f_convert;
} formats[] = {
	{ "netascii",	validate_access,	sendfile,	recvfile, 1 },
	{ "octet",	validate_access,	sendfile,	recvfile, 0 },
#ifdef notdef
	{ "mail",	validate_user,		sendmail,	recvmail, 1 },
#endif
	{ 0 }
};

/*
 * Handle initial connection protocol.
 */
tftp(tp, size)
	struct tftphdr *tp;
	int size;
{
	register char *cp;
	int first = 1, ecode;
	register struct formats *pf;
	char *filename, *mode;

	filename = cp = tp->th_stuff;

again:
	while (cp < buf + size) {
		if (*cp == '\0')
			break;
		cp++;
	}
	if (*cp != '\0') {
		nak(EBADOP);
		exit(1);
	}
	if (first) {
		mode = ++cp;
		first = 0;
		goto again;
	}
	for (cp = mode; *cp; cp++)
		if (isupper(*cp))
			*cp = tolower(*cp);
	for (pf = formats; pf->f_mode; pf++)
		if (strcmp(pf->f_mode, mode) == 0)
			break;
	if (pf->f_mode == 0) {
		nak(EBADOP);
		exit(1);
	}
	ecode = (*pf->f_validate)(filename, tp->th_opcode);
	if (ecode) {
		nak(ecode);
		exit(1);
	}
	if (tp->th_opcode == WRQ)
		(*pf->f_recv)(pf);
	else
		(*pf->f_send)(pf);
	exit(0);
}


FILE *file;

/*
 * Validate file access.  Since we
 * have no uid or gid, for now require
 * file to exist and be publicly
 * readable/writable.
 * If we were invoked with arguments
 * from inetd then the file must also be
 * in one of the given directory prefixes.
 * Note also, full path name must be
 * given as we have no login directory.
 */
validate_access(filename, mode)
	char *filename;
	int mode;
{
	struct stat stbuf;
	int     fd;
	char fbuf[MAXPATHLEN];
        char *cp, **dirp, **filep;

	if (*filename != '/'){
	   if (rpath){
	   /* add the relative pathname to the file */
		strcpy(fbuf,rpath);
		fbuf[strlen(rpath)] = '\0';
		strcat(fbuf,"/");
		fbuf[strlen(rpath) + 1] = '\0';
		strcat(fbuf,filename);
		fbuf[strlen(rpath) + strlen(filename) + 1] = '\0';
		filename = fbuf;
	   }
	   else
		return (EACCESS);
	}
	/*
         * prevent tricksters from getting around the directory restrictions
         */
        for (cp = filename + 1; *cp; cp++)
                if(*cp == '.' && strncmp(cp-1, "/../", 4) == 0)
                        return(EACCESS);
        for (dirp = dirs; *dirp; dirp++)
                if (strncmp(filename, *dirp, strlen(*dirp)) == 0)
                        break;
        if (*dirp==0) { 
	        /*
		 * check for match with files listed in /etc/tftptab
		 */
	        for (filep = files; *filep; filep++) {
		        if (strcmp(filename, *filep) == 0)
			      break;
		}
		if (*filep==0 && (filep!= files || dirp!=dirs))
		        return (EACCESS);
	}
	if (stat(filename, &stbuf) < 0)
                return (errno == ENOENT ? ENOTFOUND : EACCESS);
		
        if (mode == RRQ) {
                if ((stbuf.st_mode&(S_IREAD >> 6)) == 0)
                        return (EACCESS);
	} else {
                if ((stbuf.st_mode&(S_IWRITE >> 6)) == 0)
                        return (EACCESS);
        }
	if ((stbuf.st_mode & S_IFMT) != S_IFREG)
		return (EACCESS);

	fd = open(filename, mode == RRQ ? 0 : 1);
	if (fd < 0){
		return (errno + 100);
	}
	file = fdopen(fd, (mode == RRQ) ? "r" : "w");
	if (file == NULL) {
		return errno+100;
	}
	return (0);
}

int	timeout;
jmp_buf	timeoutbuf;

void
timer()
{

	timeout += rexmtval;
	if (timeout >= maxtimeout) {
		syslog(LOG_ERR, MSGSTR(MAXTIME, "timer: maxtimeout reached.\n")); /*MSG*/
		exit(1);
	}
	longjmp(timeoutbuf, 1);
}

/*
 * Send the requested file.
 */
sendfile(pf)
	struct formats *pf;
{
	void timer();
	struct tftphdr *fp;
	struct tftphdr *dp, *r_init();
	register struct tftphdr *ap;    /* ack packet */
	register int block = 1, size, n;


	signal(SIGALRM, timer);
	dp = r_init();
	ap = (struct tftphdr *)ackbuf;

	/*
	 * Get name of file from initial buffer
	 * and print message to syslogd indicating
	 * file to be sent to client.
	 */
	if (debug) {
		fp = (struct tftphdr *)buf;
		syslog(LOG_INFO, MSGSTR(SENDFILE, "sending file %s\n"), fp->th_stuff);
	}

	do {
		size = readit(file, &dp, pf->f_convert);
		if (size < 0) {
			nak(errno + 100);
			goto abort;
		}
		dp->th_opcode = htons((u_short)DATA);
		dp->th_block = htons((u_short)block);
		timeout = 0;
		(void) setjmp(timeoutbuf);

send_data:
		if (send(peer, dp, size + 4, 0) != size + 4) {
			syslog(LOG_ERR, MSGSTR(SENDERR1, "tftpd: write: %m\n")); /*MSG*/
			goto abort;
		}
		read_ahead(file, pf->f_convert);
		for ( ; ; ) {
			alarm(rexmtval);        /* read the ack */
			n = recv(peer, ackbuf, sizeof (ackbuf), 0);
			alarm(0);
			if (n < 0) {
				syslog(LOG_ERR, MSGSTR(RECERR1, "tftpd: read: %m\n")); /*MSG*/
				goto abort;
			}
			ap->th_opcode = ntohs((u_short)ap->th_opcode);
			ap->th_block = ntohs((u_short)ap->th_block);

			if (ap->th_opcode == ERROR) {
				goto abort;
			}
			
			if (ap->th_opcode == ACK) {
				if (ap->th_block == block) {
					break;
				}
				/* Re-synchronize with the other side */
				(void) synchnet(peer);
				if (ap->th_block == (block -1)) {
					goto send_data;
				}
			}

		}
		block++;
	} while (size == SEGSIZE);

	if (debug)
		syslog(LOG_INFO, MSGSTR(FINSEND, "finished sending %s\n"), fp->th_stuff); 

abort:
	(void) fclose(file);
}

void
justquit()
{
	exit(0);
}


/*
 * Receive a file.
 */
recvfile(pf)
	struct formats *pf;
{
	void timer();
	void justquit();
	struct tftphdr *dp, *w_init();
	register struct tftphdr *ap;    /* ack buffer */
	register int block = 0, n, size;


	signal(SIGALRM, timer);
	dp = w_init();
	ap = (struct tftphdr *)ackbuf;
	do {
		timeout = 0;
		ap->th_opcode = htons((u_short)ACK);
		ap->th_block = htons((u_short)block);
		block++;
		(void) setjmp(timeoutbuf);
send_ack:
		if (send(peer, ackbuf, 4, 0) != 4) {
			syslog(LOG_ERR, MSGSTR(SENDERR2, "tftpd: write: %m\n")); /*MSG*/
			goto abort;
		}
		write_behind(file, pf->f_convert);
		for ( ; ; ) {
			alarm(rexmtval);
			n = recv(peer, dp, PKTSIZE, 0);
			alarm(0);
			if (n < 0) {            /* really? */
				syslog(LOG_ERR, MSGSTR(RECERR2, "tftpd: read: %m\n")); /*MSG*/
				goto abort;
			}
			dp->th_opcode = ntohs((u_short)dp->th_opcode);
			dp->th_block = ntohs((u_short)dp->th_block);
			if (dp->th_opcode == ERROR) {
				goto abort;
			}
			if (dp->th_opcode == DATA) {
				if (dp->th_block == block) {
					break;   /* normal */
				}
				/* Re-synchronize with the other side */
				(void) synchnet(peer);
				if (dp->th_block == (block-1)) {
					goto send_ack;          /* rexmit */
				}
			}
		}

		/*  size = write(file, dp->th_data, n - 4); */
		size = writeit(file, &dp, n - 4, pf->f_convert);
		if (size != (n-4)) {                    /* ahem */
			if (size < 0) nak(errno + 100);
			else nak(ENOSPACE);
			goto abort;
		}
	} while (size == SEGSIZE);
	write_behind(file, pf->f_convert);
	(void) fclose(file);            /* close data file */

	ap->th_opcode = htons((u_short)ACK);    /* send the "final" ack */
	ap->th_block = htons((u_short)(block));
	(void) send(peer, ackbuf, 4, 0);

	signal(SIGALRM, justquit);      /* just quit on timeout */
	alarm(rexmtval);
	n = recv(peer, buf, sizeof (buf), 0); /* normally times out and quits */
	alarm(0);
	if (n >= 4 &&                   /* if read some data */
	    dp->th_opcode == DATA &&    /* and got a data block */
	    block == dp->th_block) {	/* then my last ack was lost */
		(void) send(peer, ackbuf, 4, 0);     /* resend final ack */
	}
abort:
	return;
}
struct errmsg {
        int     e_code;
        char    *e_msg;
} errmsgs[] = {
        { EUNDEF,       0 },
        { ENOTFOUND,    0 },
        { EACCESS,      0 },
        { ENOSPACE,     0 },
        { EBADOP,       0 },
        { EBADID,       0 },
        { EEXISTS,      0 },
        { ENOUSER,      0 },
        { -1,           0 }
};

err_load()
{
        extern char *malloc();

	errmsgs[0].e_msg = MSGSTR(UNDEFERR, "Undefined error code");
	errmsgs[1].e_msg = MSGSTR(BADFILE, "File not found");
	errmsgs[2].e_msg = MSGSTR(ILLACESS, "Access violation");
	errmsgs[3].e_msg = MSGSTR(DISKFULL, "Disk full or allocation exceeded");
	errmsgs[4].e_msg = MSGSTR(ILLOPE, "Illegal TFTP operation");
	errmsgs[5].e_msg = MSGSTR(BADID, "Unknown transfer ID");
	errmsgs[6].e_msg = MSGSTR(FILEXIST, "File already exists");
	errmsgs[7].e_msg = MSGSTR(BADUSER, "No such user");
}

/*
 * Send a nak packet (error message).
 * Error code passed in is one of the
 * standard TFTP codes, or a UNIX errno
 * offset by 100.
 */
nak(error)
	int error;
{
	register struct tftphdr *tp;
	int length;
	register struct errmsg *pe;
	extern char *sys_errlist[];


	tp = (struct tftphdr *)buf;
	tp->th_opcode = htons((u_short)ERROR);
	tp->th_code = htons((u_short)error);
	for (pe = errmsgs; pe->e_code >= 0; pe++)
		if (pe->e_code == error)
			break;
	if (pe->e_code < 0) {
		pe->e_msg = sys_errlist[error - 100];
		tp->th_code = EUNDEF;   /* set 'undef' errorcode */
	}
	strcpy(tp->th_msg, pe->e_msg);
	length = strlen(pe->e_msg);
	tp->th_msg[length] = '\0';
	length += 5;
	if (send(peer, buf, length, 0) != length) {
		syslog(LOG_ERR, MSGSTR( TFTPD_NAK, "nak: %m\n"));
	}

}

/*
 * Read the tftptab file consisting of file names we are allowed to
 * transfer
 */
void
readtab()
{
        FILE *fp;
        int i;
	int j = 0;
	char line[256];

	bzero(files, sizeof(files));
        if ((fp = fopen(tftptab, "r")) == NULL)
	       return;

	while (j < MAX_FILES) {
	       if (fgets(line, sizeof(line), fp) == NULL)
		       break;         /* done */
	       if (i = strlen(line))
		       line[i-1] = 0; /* remove trailing newline */
	       if (line[0] == '#' || line[0] == 0 || line[0] == ' ')
		       continue;     /* skip comment lines */
	       files[j] = malloc(i + 1);
	       strcpy(files[j++], line);
        }
}

