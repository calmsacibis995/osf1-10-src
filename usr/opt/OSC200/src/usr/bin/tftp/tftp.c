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
static char	*sccsid = "@(#)$RCSfile: tftp.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/03/06 10:28:52 $";
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
 * tftp.c
 *
 *	Modification History:
 *
 * 01-Apr-91	Fred Canter
 *	MIPS C 2.20+, changes for -std
 *
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/* static char sccsid[] = "tftp.c	1.3  com/sockcmd/tftp,3.1,8943 10/8/89 17:37:59"; */
/* 
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
/*
#ifndef lint
static char sccsid[] = "tftp.c	5.9 (Berkeley) 6/1/90";
#endif not lint
*/
/* Many bug fixes are from Jim Guyton <guyton@rand-unix> */

/*
 * TFTP User Program -- Protocol Machines
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <netinet/in.h>

#include <arpa/tftp.h>

#include <signal.h>
#include <stdio.h>
#include <termio.h>
#include <errno.h>
#include <setjmp.h>

#include <nl_types.h>
#include <locale.h>
#include "tftp_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TFTP,n,s) 

extern	int errno;

extern  struct sockaddr_in sin;         /* filled in by main */
extern  int     f;                      /* the opened socket */
extern  int     trace;
extern  int     verbose;
extern  int     hash;
extern  int     rate;
extern  int     rexmtval;
extern  int     maxtimeout;

#define PKTSIZE    SEGSIZE+4
char    ackbuf[PKTSIZE];
int	timeout;
jmp_buf	toplevel;
jmp_buf	timeoutbuf;

void
timer()
{
	timeout += rexmtval;
	if (timeout >= maxtimeout) {
		printf(MSGSTR(TRANS_TIMED_OUT, "Transfer timed out.\n")); /*MSG*/
		longjmp(toplevel, -1);
	}
	longjmp(timeoutbuf, 1);
}

/*
 * Send the requested file.
 */
sendfile(fd, name, mode)
	int fd;
	char *name;
	char *mode;
{
	void timer();
	register struct tftphdr *ap;       /* data and ack packets */
	struct tftphdr *r_init(), *dp;
	register int block = 0, size, n;
	register unsigned int amount = 0;
	struct sockaddr_in from;
	int fromlen;
	int convert;                    /* true if converting lf -> crlf */
	FILE *file;

	startclock();           /* start stat's clock */
	dp = r_init();          /* reset fillbuf/read-ahead code */
	ap = (struct tftphdr *)ackbuf;
	file = fdopen(fd, "r");
	convert = !strcmp(mode, "netascii");

	signal(SIGALRM, timer);
	do {
		if (block == 0)
			size = makerequest(WRQ, name, dp, mode) - 4;
		else {
		/*      size = read(fd, dp->th_data, SEGSIZE);   */
			size = readit(file, &dp, convert);
			if (size < 0) {
				nak(errno + 100);
				break;
			}
			dp->th_opcode = htons((u_short)DATA);
			dp->th_block = htons((u_short)block);
		}
		timeout = 0;
		(void) setjmp(timeoutbuf);
send_data:
		if (trace)
			tpacket(MSGSTR(SENT, "sent"), dp, size + 4); /*MSG*/
		n = sendto(f, dp, size + 4, 0, (caddr_t)&sin, sizeof (sin));
		if (n != size + 4) {
			perror(MSGSTR(TFTPSENTO, "tftp: sendto")); /*MSG*/
			goto abort;
		}
		read_ahead(file, convert);
 		for ( ; ; ) {
			alarm(rexmtval);
			do {
				fromlen = sizeof (from);
				n = recvfrom(f, ackbuf, sizeof (ackbuf), 0,
				    (caddr_t)&from, &fromlen);
			} while (n <= 0);
			alarm(0);
			if (n < 0) {
				perror(MSGSTR(TFTPREC, "tftp: recvfrom")); /*MSG*/
				goto abort;
			}
			sin.sin_port = from.sin_port;   /* added */
			if (trace)
				tpacket(MSGSTR(RECEIVED, "received"), ap, n); /*MSG*/
			/* should verify packet came from server */
			ap->th_opcode = ntohs(ap->th_opcode);
			ap->th_block = ntohs(ap->th_block);
			if (ap->th_opcode == ERROR) {
				printf(MSGSTR(ERROR_CODE, "ERROR CODE %d: %s\n"), ap->th_code, /*MSG*/
					ap->th_msg);
				goto abort;
			}
			if (ap->th_opcode == ACK) {
				int j;

				if (ap->th_block == block) {
					break;
				}
				/* On an error, try to synchronize
				 * both sides.
				 */
				j = synchnet(f);
				if (j && trace) {
					printf(MSGSTR(DISCARDED, "discarded %d packets\n"), /*MSG*/
							j);
				}
				if (ap->th_block == (block-1)) {
					goto send_data;
				}
			}
		}

		if (block > 0)
			amount += size;
		if (block > 0 && hash) {
                        fflush(stdout);
                        putc('#',stderr);
                        fflush(stderr);
                }
		block++;
	} while (size == SEGSIZE || block == 1);
abort:

	fclose(file);
	stopclock();
	if (hash)
                putc('\n',stdout);
        if (amount > 0 && rate)
		printstats(MSGSTR(CSENT, "Sent"), amount); /*MSG*/

	return; 
}
char *asdf;

/*
 * Receive a file.
 */
recvfile(fd, name, mode)
	int fd;
	char *name;
	char *mode;
{
	void timer();
	register struct tftphdr *ap;
	struct tftphdr *dp, *w_init();
	register int block = 1, n, size;
	unsigned int amount = 0;
	struct sockaddr_in from;
	int fromlen, firsttrip = 1;
	FILE *file;
	int convert;                    /* true if converting crlf -> lf */

	startclock();
	dp = w_init();
	ap = (struct tftphdr *)ackbuf;
	file = fdopen(fd, "w");
	convert = !strcmp(mode, "netascii");

	signal(SIGALRM, timer);
	do {
		if (firsttrip) {
			size = makerequest(RRQ, name, ap, mode);
			firsttrip = 0;
		} else {
			ap->th_opcode = htons((u_short)ACK);
			ap->th_block = htons((u_short)(block));
			size = 4;
			block++;
		}
		timeout = 0;
		(void) setjmp(timeoutbuf);
send_ack:
		if (trace)
			tpacket(MSGSTR(SENT, "sent"), ap, size); /*MSG*/
		if (sendto(f, ackbuf, size, 0, (caddr_t)&sin,
		    sizeof (sin)) != size) {
			alarm(0);
			perror(MSGSTR(TFTPSENTO, "tftp: sendto")); /*MSG*/
			goto abort;
		}
		write_behind(file, convert);
		for ( ; ; ) {
			alarm(rexmtval);
			do  {
				fromlen = sizeof (from);
				n = recvfrom(f, dp, PKTSIZE, 0,
				    (caddr_t)&from, &fromlen);
			} while (n <= 0);
			alarm(0);
			if (n < 0) {
				perror(MSGSTR(TFTPREC, "tftp: recvfrom")); /*MSG*/
				goto abort;
			}
			sin.sin_port = from.sin_port;   /* added */
			if (trace)
				tpacket(MSGSTR(RECEIVED, "received"), dp, n); /*MSG*/
			/* should verify client address */
			dp->th_opcode = ntohs(dp->th_opcode);
			dp->th_block = ntohs(dp->th_block);
			if (dp->th_opcode == ERROR) {
				printf(MSGSTR(ERROR_CODE, "Error code %d: %s\n"), dp->th_code, /*MSG*/
					dp->th_msg);
				goto abort;
			}
			if (dp->th_opcode == DATA) {
				int j;

				if (dp->th_block == block) {
					break;          /* have next packet */
				}
				/* On an error, try to synchronize
				 * both sides.
				 */
				j = synchnet(f);
				if (j && trace) {
					printf(MSGSTR(DISCARDED, "discarded %d packets\n"), j); /*MSG*/
				}
				if (dp->th_block == (block-1)) {
					goto send_ack;  /* resend ack */
				}
			}
		}

	/*      size = write(fd, dp->th_data, n - 4); */
		size = writeit(file, &dp, n - 4, convert);
		if (size < 0) {
			nak(errno + 100);
			break;
		}
		amount += size;
		if (hash) {
                        fflush(stdout);
                        putc('#',stderr);
                        fflush(stderr);
                }
	} while (size == SEGSIZE);
abort:                             /* ok to ack, since user has seen err msg */
	ap->th_opcode = htons((u_short)ACK);
	ap->th_block = htons((u_short)block);
	(void) sendto(f, ackbuf, 4, 0, &sin, sizeof (sin));
	write_behind(file, convert);            /* flush last buffer */
	fclose(file);
	stopclock();
	if (hash)
                putc('\n',stdout);
        if (amount > 0 && rate)
	if (amount > 0)
		printstats(MSGSTR(CRECEIVED, "Received"), amount); /*MSG*/

}

makerequest(request, name, tp, mode)
	int request;
	char *name, *mode;
	struct tftphdr *tp;
{
	register char *cp;

	tp->th_opcode = htons((u_short)request);
	cp = tp->th_stuff;
	strcpy(cp, name);
	cp += strlen(name);
	*cp++ = '\0';
	strcpy(cp, mode);
	cp += strlen(mode);
	*cp++ = '\0';
	return (cp - (char *)tp);
}
struct errmsg {
	int	e_code;
	char	*e_msg;
} errmsgs[] = {
	{ EUNDEF,	0 }, 
        { ENOTFOUND,	0 },
        { EACCESS,	0 },
        { ENOSPACE,   	0 },
        { EBADOP,	0 },
        { EBADID,	0 },
	{ EEXISTS,	0 },
        { ENOUSER,	0 },
        { -1,           0 }
};

err_load()
{
	extern char *malloc();

/** there has got to be a better way??? **/
	
	errmsgs[0].e_msg = malloc(strlen( MSGSTR( UNDEFERR, "Undefined error code") + 1));
        errmsgs[1].e_msg = malloc( strlen( MSGSTR( BADFILE, "File not found") + 1) );
        errmsgs[2].e_msg = malloc( strlen( MSGSTR( ILLACESS, "Access violation") + 1) );
        errmsgs[3].e_msg = malloc( strlen( MSGSTR( DISKFULL, "Disk full or allocation exceeded") + 1) );
        errmsgs[4].e_msg = malloc( strlen( MSGSTR( ILLOPE, "Illegal TFTP operation") + 1) );
        errmsgs[5].e_msg = malloc( strlen( MSGSTR( BADID, "Unknown transfer ID") + 1) );
	errmsgs[6].e_msg = malloc( strlen( MSGSTR( FILEXIST, "File already exists") + 1) );
        errmsgs[7].e_msg = malloc( strlen( MSGSTR( BADUSER, "No such user") + 1) );
}

#ifdef notdef
struct errmsg {
	int	e_code;
	char	*e_msg;
} errmsgs[9];

char err_msg[10][128];

err_load()
{
	int i;

	for(i=0; i<8; i++) 
		errmsgs[i].e_msg = &err_msg[i][0];

	errmsgs[0].e_code = EUNDEF;
	NCstrncpy(&err_msg[0][0], MSGSTR(UNDEFERR, "Undefined error code"), 40); /*MSG*/
	errmsgs[1].e_code = ENOTFOUND;
	NCstrncpy(&err_msg[1][0], MSGSTR(BADFILE, "File not found"), 40); /*MSG*/
	errmsgs[2].e_code = EACCESS;
	NCstrncpy(&err_msg[2][0], MSGSTR(ILLACESS, "Access violation"), 40); /*MSG*/
	errmsgs[3].e_code = ENOSPACE;
	NCstrncpy(&err_msg[3][0], MSGSTR(DISKFULL, "Disk full or allocation exceeded"), 40); /*MSG*/
	errmsgs[4].e_code = EBADOP;
	NCstrncpy(&err_msg[4][0], MSGSTR(ILLOPE, "Illegal TFTP operation"), 40); /*MSG*/
	errmsgs[5].e_code = EBADID;
	NCstrncpy(&err_msg[5][0], MSGSTR(BADID, "Unknown transfer ID"), 40); /*MSG*/
	errmsgs[6].e_code = EEXISTS;
	NCstrncpy(&err_msg[6][0], MSGSTR(FILEXIST, "File already exists"), 40);
	errmsgs[7].e_code = ENOUSER;
	NCstrncpy(&err_msg[7][0], MSGSTR(BADUSER, "No such user"), 40); /*MSG*/
	errmsgs[8].e_code = -1;
	errmsgs[8].e_msg = 0;
}
#endif

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


	tp = (struct tftphdr *)ackbuf;
	tp->th_opcode = htons((u_short)ERROR);
	tp->th_code = htons((u_short)error);
	for (pe = errmsgs; pe->e_code >= 0; pe++)
		if (pe->e_code == error)
			break;
	if (pe->e_code < 0) {
		pe->e_msg = sys_errlist[error - 100];
		tp->th_code = EUNDEF;
	}
	strcpy(tp->th_msg, pe->e_msg);
	length = strlen(pe->e_msg) + 4;
	if (trace)
		tpacket(MSGSTR(SENT, "sent"), tp, length); /*MSG*/
	if (sendto(f, ackbuf, length, 0, &sin, sizeof (sin)) != length)
		perror("nak");

}

tpacket(s, tp, n)
	char *s;
	struct tftphdr *tp;
	int n;
{
	static char *opcodes[] =
	   { "#0", "RRQ", "WRQ", "DATA", "ACK", "ERROR" };
	register char *cp, *file;
	u_short op = ntohs(tp->th_opcode);
	char *index();
	char tmp[80];


	strcpy(tmp, s);
	if (op < RRQ || op > ERROR)
		printf(MSGSTR(OPCODE, "%s opcode=%x "), tmp, op);/*MSG*/
	else
		printf(MSGSTR(OPCODES, "%s %s "), tmp, opcodes[op]); /*MSG*/
	switch (op) {

	case RRQ:
	case WRQ:
		n -= 2;
		file = cp = tp->th_stuff;
		cp = index(cp, '\0');
		printf(MSGSTR(FILE_MODE, "<file=%s, mode=%s>\n"), file, cp + 1); /*MSG*/
		break;

	case DATA:
		printf(MSGSTR(BLOCK_BYTES, "<block=%d, %d bytes>\n"), ntohs(tp->th_block), n - 4); /*MSG*/
		break;

	case ACK:
		printf(MSGSTR(BLOCK, "<block=%d>\n"), ntohs(tp->th_block)); /*MSG*/
		break;

	case ERROR:
		printf(MSGSTR(CODE_MSG, "<code=%d, msg=%s>\n"), ntohs(tp->th_code), tp->th_msg); /*MSG*/
		break;
	}

}

struct timeval tstart;
struct timeval tstop;
struct timezone zone;

startclock() {
	gettimeofday(&tstart, &zone);
}

stopclock() {
	gettimeofday(&tstop, &zone);
}

printstats(direction, amount)
char *direction;
unsigned int amount;
{
	double delta;
	char tmp[40];
			/* compute delta in 1/10's second units */
	delta = ((tstop.tv_sec*10.)+(tstop.tv_usec/100000)) -
		((tstart.tv_sec*10.)+(tstart.tv_usec/100000));
	delta = delta/10.;      /* back to seconds */
	strcpy(tmp, direction);
        printf(MSGSTR(STATS, "%s %d Bytes in %.1f Seconds"), tmp, amount, delta);
	if (verbose)
		printf(MSGSTR(BITS_PER_SEC, " [%.0f bits/sec]"), (amount*8.)/delta); /*MSG*/
	putchar('\n');
}

