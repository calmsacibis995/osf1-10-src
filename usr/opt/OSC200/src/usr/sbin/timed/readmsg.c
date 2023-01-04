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
static char	*sccsid = "@(#)$RCSfile: readmsg.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/12/09 16:02:42 $";
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
 * COMPONENT_NAME: TCPIP readmsg.c
 * 
 * FUNCTIONS: LOOKAT, MORETIME, ignoreack, masterack, print, readmsg, 
 *            slaveack 
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
 */
/* readmsg.c	1.2  com/sockcmd/timed,3.1,9021 10/8/89 17:47:04 */
/*
#ifndef lint
static char sccsid[] = "readmsg.c	2.11 (Berkeley) 6/18/88";
#endif  not lint */

#include "globals.h"
#include <protocols/timed.h>

extern char *tsptype[];

/*
 * LOOKAT checks if the message is of the requested type and comes from
 * the right machine, returning 1 in case of affirmative answer 
 */

#define LOOKAT(msg, mtype, mfrom, netp, froms) \
	(((((mtype) == TSP_ANY) || ((mtype) == (msg).tsp_type)) && \
	(((mfrom) == NULL) || (strcmp((mfrom), (msg).tsp_name) == 0)) && \
	(((netp) == NULL) || \
	(((netp)->mask & (froms).sin_addr.s_addr) == (netp)->net))) \
	? 1 : 0)

#define MORETIME(rtime, rtout) \
	(((rtime).tv_sec > (rtout).tv_sec || \
	    ((rtime).tv_sec == (rtout).tv_sec && \
		(rtime).tv_usec >= (rtout).tv_usec)) \
	? 0 : 1)

static struct timeval rtime, rwait, rtout;
struct tsp msgin;
static struct tsplist {
	struct tsp info;
	struct sockaddr_in addr;
	struct tsplist *p;
} msgslist;
struct sockaddr_in from;
struct netinfo *fromnet;

/*
 * `readmsg' returns message `type' sent by `machfrom' if it finds it 
 * either in the receive queue, or in a linked list of previously received 
 * messages that it maintains.
 * Otherwise it waits to see if the appropriate message arrives within
 * `intvl' seconds. If not, it returns NULL.
 */

struct tsp *
readmsg(type, machfrom, intvl, netfrom)

int type;
char *machfrom;
struct timeval *intvl;
struct netinfo *netfrom;
{
	int length;
	fd_set ready;
	static struct tsplist *head = &msgslist;
	static struct tsplist *tail = &msgslist;
	struct tsplist *prev;
	register struct netinfo *ntp;
	register struct tsplist *ptr;

	if (trace) {
		fprintf(fd, MSGSTR(LOOKING, "looking for %s from %s\n"),
			tsptype[type], machfrom == NULL ? "ANY" : machfrom);
		ptr = head->p;
		fprintf(fd, MSGSTR(MSGQUEUE, "msgqueue:\n"));
		while (ptr != NULL) {
			fprintf(fd, "\t");
			print(&ptr->info, &ptr->addr);
			ptr = ptr->p;
		}
	}

	ptr = head->p;
	prev = head;

	/*
	 * Look for the requested message scanning through the 
	 * linked list. If found, return it and free the space 
	 */

	while (ptr != NULL) {
		if (LOOKAT(ptr->info, type, machfrom, netfrom, ptr->addr)) {
			msgin = ptr->info;
			from = ptr->addr;
			prev->p = ptr->p;
			if (ptr == tail) 
				tail = prev;
			free((char *)ptr);
			fromnet = NULL;
			if (netfrom == NULL)
			    for (ntp = nettab; ntp != NULL; ntp = ntp->next) {
				    if ((ntp->mask & from.sin_addr.s_addr) ==
					ntp->net) {
					    fromnet = ntp;
					    break;
				    }
			    }
			else
			    fromnet = netfrom;
			if (trace) {
				fprintf(fd, MSGSTR(READMSG, "readmsg: "));
				print(&msgin, &from);
			}
			return(&msgin);
		} else {
			prev = ptr;
			ptr = ptr->p;
		}
	}

	/*
	 * If the message was not in the linked list, it may still be
	 * coming from the network. Set the timer and wait 
	 * on a select to read the next incoming message: if it is the
	 * right one, return it, otherwise insert it in the linked list.
	 */

	(void)gettimeofday(&rtime, (struct timezone *)0);
	rtout.tv_sec = rtime.tv_sec + intvl->tv_sec;
	rtout.tv_usec = rtime.tv_usec + intvl->tv_usec;
	if (rtout.tv_usec > 1000000) {
		rtout.tv_usec -= 1000000;
		rtout.tv_sec++;
	}

	FD_ZERO(&ready);
	for (; MORETIME(rtime, rtout);
	    (void)gettimeofday(&rtime, (struct timezone *)0)) {
		rwait.tv_sec = rtout.tv_sec - rtime.tv_sec;
		rwait.tv_usec = rtout.tv_usec - rtime.tv_usec;
		if (rwait.tv_usec < 0) {
			rwait.tv_usec += 1000000;
			rwait.tv_sec--;
		}
		if (rwait.tv_sec < 0) 
			rwait.tv_sec = rwait.tv_usec = 0;

		if (trace) {
			fprintf(fd, MSGSTR(WAIT, "readmsg: wait: (%d %d)\n"), 
						rwait.tv_sec, rwait.tv_usec);
		}
		FD_SET(sock, &ready);
		if (select(FD_SETSIZE, &ready, (fd_set *)0, (fd_set *)0,
		    &rwait)) {
			length = sizeof(struct sockaddr_in);
			if (recvfrom(sock, (char *)&msgin, sizeof(struct tsp), 
						0, &from, &length) < 0) {
				syslog(LOG_ERR, MSGSTR(REVDATE, "receiving datagram packet: %m"));
				exit(1);
			}

			bytehostorder(&msgin);

			if (msgin.tsp_vers > TSPVERSION) {
				if (trace) {
				    fprintf(fd, MSGSTR(VERSMISMATCH, "readmsg: version mismatch\n"));
				    /* should do a dump of the packet, but... */
				}
				continue;
			}

			fromnet = NULL;
			for (ntp = nettab; ntp != NULL; ntp = ntp->next)
				if ((ntp->mask & from.sin_addr.s_addr) ==
				    ntp->net) {
					fromnet = ntp;
					break;
				}

			/*
			 * drop packets from nets we are ignoring permanently
			 */
			if (fromnet == NULL) {
				/* 
				 * The following messages may originate on
				 * this host with an ignored network address
				 */
				if (msgin.tsp_type != TSP_TRACEON &&
				    msgin.tsp_type != TSP_SETDATE &&
				    msgin.tsp_type != TSP_MSITE &&
#ifdef	TESTING
				    msgin.tsp_type != TSP_TEST &&
#endif
				    msgin.tsp_type != TSP_TRACEOFF) {
					if (trace) {
					    fprintf(fd, MSGSTR(DISCARD, "readmsg: discarded: "));
					    print(&msgin, &from);
					}
					continue;
				}
			}

			/*
			 * Throw away messages coming from this machine, unless
			 * they are of some particular type.
			 * This gets rid of broadcast messages and reduces
			 * master processing time.
			 */
			if ( !(strcmp(msgin.tsp_name, hostname) != 0 ||
					msgin.tsp_type == TSP_SETDATE ||
#ifdef TESTING
					msgin.tsp_type == TSP_TEST ||
#endif
					msgin.tsp_type == TSP_MSITE ||
					(msgin.tsp_type == TSP_LOOP &&
					msgin.tsp_hopcnt != 10) ||
					msgin.tsp_type == TSP_TRACEON ||
					msgin.tsp_type == TSP_TRACEOFF)) {
				if (trace) {
					fprintf(fd, MSGSTR(DISCARD, "readmsg: discarded: "));
					print(&msgin, &from);
				}
				continue;
			}

			/*
			 * Send acknowledgements here; this is faster and avoids
			 * deadlocks that would occur if acks were sent from a 
			 * higher level routine.  Different acknowledgements are
			 * necessary, depending on status.
			 */
			if (fromnet->status == MASTER)
				masterack();
			else if (fromnet->status == SLAVE)
				slaveack();
			else
				ignoreack();
				
			if (LOOKAT(msgin, type, machfrom, netfrom, from)) {
				if (trace) {
				    fprintf(fd, MSGSTR(READMSG, "readmsg: "));
				    print(&msgin, &from);
				}
				return(&msgin);
			} else {
				tail->p = (struct tsplist *)
						malloc(sizeof(struct tsplist)); 
				tail = tail->p;
				tail->p = NULL;
				tail->info = msgin;
				tail->addr = from;
			}
		} else {
			break;
		}
	}
	return((struct tsp *)NULL);
}

/*
 * `slaveack' sends the necessary acknowledgements: 
 * only the type ACK is to be sent by a slave 
 */

slaveack()
{
	int length;
	struct tsp resp;

	length = sizeof(struct sockaddr_in);
	switch(msgin.tsp_type) {

	case TSP_ADJTIME:
	case TSP_SETTIME:
	case TSP_ACCEPT:
	case TSP_REFUSE:
	case TSP_TRACEON:
	case TSP_TRACEOFF:
	case TSP_QUIT:
		resp = msgin;
		resp.tsp_type = TSP_ACK;
		resp.tsp_vers = TSPVERSION;
		(void)strcpy(resp.tsp_name, hostname);
		if (trace) {
			fprintf(fd, MSGSTR(SACK, "Slaveack: "));
			print(&resp, &from);
		}
		bytenetorder(&resp);     /* this is not really necessary here */
		if (sendto(sock, (char *)&resp, sizeof(struct tsp), 0, 
						&from, length) < 0) {
			syslog(LOG_ERR, MSGSTR(SENDTO2,"sendto: %m"));
			exit(1);
		}
		break;
	default:
		break;
	}
}

/*
 * Certain packets may arrive from this machine on ignored networks.
 * These packets should be acknowledged.
 */

ignoreack()
{
	int length;
	struct tsp resp;

	length = sizeof(struct sockaddr_in);
	switch(msgin.tsp_type) {

	case TSP_TRACEON:
	case TSP_TRACEOFF:
		resp = msgin;
		resp.tsp_type = TSP_ACK;
		resp.tsp_vers = TSPVERSION;
		(void)strcpy(resp.tsp_name, hostname);
		if (trace) {
			fprintf(fd, MSGSTR(IGNACK, "Ignoreack: "));
			print(&resp, &from);
		}
		bytenetorder(&resp);     /* this is not really necessary here */
		if (sendto(sock, (char *)&resp, sizeof(struct tsp), 0, 
						&from, length) < 0) {
			syslog(LOG_ERR, MSGSTR(SENDTO2, "sendto: %m"));
			exit(1);
		}
		break;
	default:
		break;
	}
}

/*
 * `masterack' sends the necessary acknowledgments 
 * to the messages received by a master 
 */

masterack()
{
	int length;
	struct tsp resp;

	length = sizeof(struct sockaddr_in);

	resp = msgin;
	resp.tsp_vers = TSPVERSION;
	(void)strcpy(resp.tsp_name, hostname);

	switch(msgin.tsp_type) {

	case TSP_QUIT:
	case TSP_TRACEON:
	case TSP_TRACEOFF:
	case TSP_MSITE:
	case TSP_MSITEREQ:
		resp.tsp_type = TSP_ACK;
		bytenetorder(&resp);
		if (trace) {
			fprintf(fd, MSGSTR(MACK, "Masterack: "));
			print(&resp, &from);
		}
		if (sendto(sock, (char *)&resp, sizeof(struct tsp), 0, 
						&from, length) < 0) {
			syslog(LOG_ERR, MSGSTR(SENDTO2,"sendto: %m"));
			exit(1);
		}
		break;
	case TSP_RESOLVE:
	case TSP_MASTERREQ:
		resp.tsp_type = TSP_MASTERACK;
		bytenetorder(&resp);
		if (trace) {
			fprintf(fd, MSGSTR(MACK, "Masterack: "));
			print(&resp, &from);
		}
		if (sendto(sock, (char *)&resp, sizeof(struct tsp), 0, 
						&from, length) < 0) {
			syslog(LOG_ERR, MSGSTR(SENDTO2,"sendto: %m"));
			exit(1);
		}
		break;
	case TSP_SETDATEREQ:
		resp.tsp_type = TSP_DATEACK;
		bytenetorder(&resp);
		if (trace) {
			fprintf(fd, MSGSTR(MACK, "Masterack: "));
			print(&resp, &from);
		}
		if (sendto(sock, (char *)&resp, sizeof(struct tsp), 0, 
						&from, length) < 0) {
			syslog(LOG_ERR, MSGSTR(SENDTO2,"sendto: %m"));
			exit(1);
		}
		break;
	default:
		break;
	}
}

/*
 * Print a TSP message 
 */
print(msg, addr)
struct tsp *msg;
struct sockaddr_in *addr;
{
	switch (msg->tsp_type) {

	case TSP_LOOP:
		fprintf(fd, "%s %d %d (#%d) %s %s\n",
			tsptype[msg->tsp_type],
			msg->tsp_vers,
			msg->tsp_seq,
			msg->tsp_hopcnt,
			msg->tsp_name,
			inet_ntoa(addr->sin_addr));
		break;

	case TSP_SETTIME:
	case TSP_ADJTIME:
	case TSP_SETDATE:
	case TSP_SETDATEREQ:
		fprintf(fd, "%s %d %d (%d, %d) %s %s\n",
			tsptype[msg->tsp_type],
			msg->tsp_vers,
			msg->tsp_seq,
			msg->tsp_time.tv_sec, 
			msg->tsp_time.tv_usec, 
			msg->tsp_name,
			inet_ntoa(addr->sin_addr));
		break;

	default:
		fprintf(fd, "%s %d %d %s %s\n",
			tsptype[msg->tsp_type],
			msg->tsp_vers,
			msg->tsp_seq,
			msg->tsp_name,
			inet_ntoa(addr->sin_addr));
		break;
	}
}
