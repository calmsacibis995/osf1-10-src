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
static char	*sccsid = "@(#)$RCSfile: slave.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/04/13 17:54:08 $";
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
 * COMPONENT_NAME: TCPIP slave.c
 * 
 * FUNCTIONS: answerdelay, slave 
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
 * Copyright (c) 1985 Regents of the University of California.
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
/* slave.c	1.2  com/sockcmd/timed,3.1,9021 10/8/89 17:47:21 */
/*
#ifndef lint
static char sccsid[] = "slave.c	2.19 (Berkeley) 9/20/88";
#endif  not lint */

#include "globals.h"
#include <protocols/timed.h>
#include <setjmp.h>

extern jmp_buf jmpenv;

extern u_short sequence;

slave()
{
	int length;
	int senddateack;
	int electiontime, refusetime, looktime;
	u_short seq;
	char candidate[MAXHOSTNAMELEN];
	struct tsp *msg, to, *readmsg();
	struct sockaddr_in saveaddr, msaveaddr;
	struct timeval time, wait;
	struct tsp *answer, *acksend();
	int timeout();
	char *date();
	int casual();
	int bytenetorder();
	char olddate[32];
	struct sockaddr_in server;
	register struct netinfo *ntp;
	int ind;
	struct tsp resp;
	extern int Mflag;
	extern int justquit;
#ifdef MEASURE
	extern FILE *fp;
#endif
	if (slavenet) {
		resp.tsp_type = TSP_SLAVEUP;
		resp.tsp_vers = TSPVERSION;
		(void)strcpy(resp.tsp_name, hostname);
		bytenetorder(&resp);
		if (sendto(sock, (char *)&resp, sizeof(struct tsp), 0,
		    &slavenet->dest_addr, sizeof(struct sockaddr_in)) < 0) {
			syslog(LOG_ERR, MSGSTR(SENDTO2, "sendto: %m"));
			exit(1);
		}
	}

	if (status & MASTER) {
#ifdef MEASURE
		if (fp == NULL) {
			fp = fopen("/usr/adm/timed.masterlog", "w");
			setlinebuf(fp);
		}
#endif
		syslog(LOG_INFO, MSGSTR(SUBMASTER1, "THIS MACHINE IS A SUBMASTER"));
		if (trace) {
			fprintf(fd, MSGSTR(SUBMASTER1, "THIS MACHINE IS A SUBMASTER\n"));
		}
		for (ntp = nettab; ntp != NULL; ntp = ntp->next)
			if (ntp->status == MASTER)
				masterup(ntp);

	} else {
		syslog(LOG_INFO, MSGSTR(SLAVE1, "THIS MACHINE IS A SLAVE"));
		if (trace) {
			fprintf(fd, MSGSTR(SLAVE1, "THIS MACHINE IS A SLAVE\n"));
		}
	}

	seq = 0;
	senddateack = OFF;
	refusetime = 0;

	(void)gettimeofday(&time, (struct timezone *)0);
	electiontime = time.tv_sec + delay2;
	if (Mflag)
		if (justquit)
			looktime = time.tv_sec + delay2;
		else 
			looktime = 1;
	else
		looktime = 0;

loop:
	length = sizeof(struct sockaddr_in);
	(void)gettimeofday(&time, (struct timezone *)0);
	if (time.tv_sec > electiontime) {
		if (trace) 
			fprintf(fd, MSGSTR(TIMEEXPIR, "election timer expired\n"));
		longjmp(jmpenv, 1);
	}
	if (looktime && time.tv_sec > looktime) {
		if (trace) 
			fprintf(fd, MSGSTR(NETS, "Looking for nets to master and loops\n"));
		
		if (nignorednets > 0) {
			for (ntp = nettab; ntp != NULL; ntp = ntp->next) {
				if (ntp->status == IGNORE) {
					lookformaster(ntp);
					if (ntp->status == MASTER)
						masterup(ntp);
					else
						ntp->status = IGNORE;
				}
			}
			setstatus();
#ifdef MEASURE
			/*
			 * Check to see if we just became master
			 * (file not open)
			 */
			if (fp == NULL) {
				fp = fopen("/usr/adm/timed.masterlog", "w");
				setlinebuf(fp);
			}
#endif
		}

		for (ntp = nettab; ntp != NULL; ntp = ntp->next) {
		    if (ntp->status == MASTER) {
			to.tsp_type = TSP_LOOP;
			to.tsp_vers = TSPVERSION;
			to.tsp_seq = sequence++;
			to.tsp_hopcnt = 10;
			(void)strcpy(to.tsp_name, hostname);
			bytenetorder(&to);
			if (sendto(sock, (char *)&to, sizeof(struct tsp), 0,
			    &ntp->dest_addr, sizeof(struct sockaddr_in)) < 0) {
				syslog(LOG_ERR, MSGSTR(SENDTO2,"sendto: %m"));
				exit(1);
			}
		    }
		}
		(void)gettimeofday(&time, (struct timezone *)0);
		looktime = time.tv_sec + delay2;
	}
	wait.tv_sec = electiontime - time.tv_sec + 10;
	wait.tv_usec = 0;
	msg = readmsg(TSP_ANY, (char *)ANYADDR, &wait, (struct netinfo *)NULL);
	if (msg != NULL) {
		switch (msg->tsp_type) {
		case TSP_SETDATE:
#ifdef TESTING
		case TSP_TEST:
#endif
		case TSP_MSITE:
		case TSP_TRACEOFF:
		case TSP_TRACEON:
			break;
		case TSP_MASTERUP:
			if (fromnet == NULL) {
				if (trace) {
					fprintf(fd, MSGSTR(SIGNORE, "slave ignored: "));
					print(msg, &from);
				}
				goto loop;
			}
			break;
		default:
			if (fromnet == NULL || fromnet->status == IGNORE) {
				if (trace) {
					fprintf(fd, MSGSTR(SIGNORE, "slave ignored: "));
					print(msg, &from);
				}
				goto loop;
			}
			break;
		}

		switch (msg->tsp_type) {

		case TSP_ADJTIME:
			if (fromnet->status != SLAVE)
				break;
			(void)gettimeofday(&time, (struct timezone *)0);
			electiontime = time.tv_sec + delay2;
			if (seq != msg->tsp_seq) {
				seq = msg->tsp_seq;
				if ((status & SUBMASTER) == SUBMASTER) {
					synch((msg->tsp_time.tv_sec * 1000) + 
					    (msg->tsp_time.tv_usec / 1000));
				} else {
					adjclock(&(msg->tsp_time));
				}
			}
			break;
		case TSP_SETTIME:
			if (fromnet->status != SLAVE)
				break;
			if (seq == msg->tsp_seq)
				break;

			seq = msg->tsp_seq;

			(void)strcpy(olddate, date());
			logwtmp("|", "date", "");
			(void)settimeofday(&msg->tsp_time,
				(struct timezone *)0);
			logwtmp("{", "date", "");
			syslog(LOG_NOTICE, MSGSTR(DATECHANGE2, "date changed by %s from: %s"),
				msg->tsp_name, olddate);
			if ((status & SUBMASTER) == SUBMASTER)
				spreadtime();
			(void)gettimeofday(&time, (struct timezone *)0);
			electiontime = time.tv_sec + delay2;

			if (senddateack == ON) {
				senddateack = OFF;
				msg->tsp_type = TSP_DATEACK;
				(void)strcpy(msg->tsp_name, hostname);
				bytenetorder(msg);
				length = sizeof(struct sockaddr_in);
				if (sendto(sock, (char *)msg, 
						sizeof(struct tsp), 0,
						&saveaddr, length) < 0) {
					syslog(LOG_ERR, MSGSTR(SENDTO2,"sendto: %m"));
					exit(1);
				}
			}
			break;
		case TSP_MASTERUP:
			if (slavenet && fromnet != slavenet)
				break;
			makeslave(fromnet);
			setstatus();
			msg->tsp_type = TSP_SLAVEUP;
			msg->tsp_vers = TSPVERSION;
			(void)strcpy(msg->tsp_name, hostname);
			bytenetorder(msg);
			answerdelay();
			length = sizeof(struct sockaddr_in);
			if (sendto(sock, (char *)msg, sizeof(struct tsp), 0, 
						&from, length) < 0) {
				syslog(LOG_ERR, MSGSTR(SENDTO2,"sendto: %m"));
				exit(1);
			}
			backoff = 1;
			delay2 = casual(MINTOUT, MAXTOUT);
			(void)gettimeofday(&time, (struct timezone *)0);
			electiontime = time.tv_sec + delay2;
			refusetime = 0;
			break;
		case TSP_MASTERREQ:
			if (fromnet->status != SLAVE)
				break;
			(void)gettimeofday(&time, (struct timezone *)0);
			electiontime = time.tv_sec + delay2;
			break;
		case TSP_SETDATE:
			saveaddr = from;
			msg->tsp_type = TSP_SETDATEREQ;
			msg->tsp_vers = TSPVERSION;
			(void)strcpy(msg->tsp_name, hostname);
			for (ntp = nettab; ntp != NULL; ntp = ntp->next) {
				if (ntp->status == SLAVE)
					break;
			}
			if (ntp == NULL)
				break;
			answer = acksend(msg, &ntp->dest_addr, (char *)ANYADDR,
			    TSP_DATEACK, ntp);
			if (answer != NULL) {
				msg->tsp_type = TSP_ACK;
				bytenetorder(msg);
				length = sizeof(struct sockaddr_in);
				if (sendto(sock, (char *)msg,
				    sizeof(struct tsp), 0, &saveaddr,
				    length) < 0) {
					syslog(LOG_ERR, MSGSTR(SENDTO2, "sendto: %m"));
					exit(1);
				}
				senddateack = ON;
			}
			break;
		case TSP_SETDATEREQ:
			saveaddr = from;
			if (status != SUBMASTER || fromnet->status != MASTER)
				break;
			for (ntp = nettab; ntp != NULL; ntp = ntp->next) {
				if (ntp->status == SLAVE)
					break;
			}
			ind = findhost(msg->tsp_name);
			if (ind < 0) {
			   syslog(LOG_WARNING,
			   MSGSTR(DATEREQ,"DATEREQ from uncontrolled machine"));
			   break;
			}
			syslog(LOG_DEBUG,
			MSGSTR(FORWARD,"forwarding date change request for %s"),
			    msg->tsp_name);
			(void)strcpy(msg->tsp_name, hostname);
			answer = acksend(msg, &ntp->dest_addr, (char *)ANYADDR,
			    TSP_DATEACK, ntp);
			if (answer != NULL) {
				msg->tsp_type = TSP_DATEACK;
				bytenetorder(msg);
				length = sizeof(struct sockaddr_in);
				if (sendto(sock, (char *)msg,
				    sizeof(struct tsp), 0, &saveaddr,
				    length) < 0) {
					syslog(LOG_ERR, MSGSTR(SENDTO2, "sendto: %m"));
					exit(1);
				}
			}
			break;
		case TSP_TRACEON:
			if (!(trace)) {
				fd = fopen(tracefile, "w");
				setlinebuf(fd);
				fprintf(fd, MSGSTR(TSTART, "Tracing started on: %s\n\n"), date());
			}
			trace = ON;
			break;
		case TSP_TRACEOFF:
			if (trace) {
				fprintf(fd, MSGSTR(TEND, "Tracing ended on: %s\n"), date());
				(void)fclose(fd);
			}
#ifdef GPROF
			moncontrol(0);
			_mcleanup();
			moncontrol(1);
#endif
			trace = OFF;
			break;
		case TSP_SLAVEUP:
			if ((status & MASTER) && fromnet->status == MASTER) {
				ind = addmach(msg->tsp_name, &from);
				newslave(ind, msg->tsp_seq);
			}
			break;
		case TSP_ELECTION:
			if (fromnet->status == SLAVE) {
				(void)gettimeofday(&time, (struct timezone *)0);
				electiontime = time.tv_sec + delay2;
				seq = 0;            /* reset sequence number */
				if (time.tv_sec < refusetime)
					msg->tsp_type = TSP_REFUSE;
				else {
					msg->tsp_type = TSP_ACCEPT;
					refusetime = time.tv_sec + 30;
				}
				(void)strcpy(candidate, msg->tsp_name);
				(void)strcpy(msg->tsp_name, hostname);
				answerdelay();
				server = from;
				answer = acksend(msg, &server, candidate, TSP_ACK,
				    (struct netinfo *)NULL);
				if (answer == NULL)
					syslog(LOG_WARNING,
					MSGSTR(NOANS, "no answer from master candidate\n"));
			} else {	/* fromnet->status == MASTER */
				to.tsp_type = TSP_QUIT;
				(void)strcpy(to.tsp_name, hostname);
				server = from;
				answer = acksend(&to, &server, msg->tsp_name,
				    TSP_ACK, (struct netinfo *)NULL);
				if (answer == NULL) {
					syslog(LOG_WARNING,
					    MSGSTR(ERRNORPY, "election error: no reply to QUIT"));
				} else {
					(void) addmach(msg->tsp_name, &from);
				}
			}
			break;
                case TSP_CONFLICT:
			if (fromnet->status != MASTER)
				break;
                        /*
                         * After a network partition, there can be
                         * more than one master: the first slave to
                         * come up will notify here the situation.
                         */
                        (void)strcpy(to.tsp_name, hostname);

                        if (fromnet == NULL)
                                break;
                        for(;;) {
                                to.tsp_type = TSP_RESOLVE;
                                answer = acksend(&to, &fromnet->dest_addr,
                                    (char *)ANYADDR, TSP_MASTERACK, fromnet);
                                if (answer == NULL)
                                        break;
                                to.tsp_type = TSP_QUIT;
                                server = from;
                                msg = acksend(&to, &server, answer->tsp_name,
                                    TSP_ACK, (struct netinfo *)NULL);
                                if (msg == NULL) {
                                        syslog(LOG_WARNING,
					MSGSTR(CONFLICT, "conflict error: no reply to QUIT"));
				} else {
                                        (void) addmach(answer->tsp_name, &from);
				}
                        }
                        masterup(fromnet);
                        break;
		case TSP_MSITE:
			if (!slavenet)
				break;
			msaveaddr = from;
			msg->tsp_type = TSP_MSITEREQ;
			msg->tsp_vers = TSPVERSION;
			(void)strcpy(msg->tsp_name, hostname);
			answer = acksend(msg, &slavenet->dest_addr,
					 (char *)ANYADDR, TSP_ACK, slavenet);
			if (answer != NULL) {
				msg->tsp_type = TSP_ACK;
				length = sizeof(struct sockaddr_in);
				bytenetorder(msg);
				if (sendto(sock, (char *)msg, 
						sizeof(struct tsp), 0,
						&msaveaddr, length) < 0) {
					syslog(LOG_ERR, MSGSTR(SENDTO2,"sendto: %m"));
					exit(1);
				}
			}
			break;
		case TSP_ACCEPT:
		case TSP_REFUSE:
			break;
		case TSP_RESOLVE:
			break;
		case TSP_QUIT:
			/* become slave */
#ifdef MEASURE
			if (fp != NULL) {
				(void)fclose(fp);
				fp = NULL;
			}
#endif
			longjmp(jmpenv, 2);
			break;
#ifdef TESTING
		case TSP_TEST:
			electiontime = 0;
			break;
#endif
		case TSP_MSITEREQ:
			if (status & MASTER)
				break;
			if (trace) {
				fprintf(fd, MSGSTR(GARBAGE, "garbage: "));
				print(msg, &from);
			}
			break;

		case TSP_LOOP:
			/* looking for loops of masters */
			if ( !(status & MASTER))
				break;
			if (fromnet->status == SLAVE) {
			    if ( !strcmp(msg->tsp_name, hostname)) {
				  for(;;) {
				    to.tsp_type = TSP_RESOLVE;
				    answer = acksend(&to, &fromnet->dest_addr,
					(char *)ANYADDR, TSP_MASTERACK,
					fromnet);
				    if (answer == NULL)
					    break;
				    to.tsp_type = TSP_QUIT;
				    (void)strcpy(to.tsp_name, hostname);
				    server = from;
				    answer = acksend(&to, &server,
					answer->tsp_name, TSP_ACK,
					(struct netinfo *)NULL);
				    if (answer == NULL) {
					syslog(LOG_ERR, MSGSTR(KILLERR, "loop kill error"));
				    } else {
					electiontime = 0;
				    }
				  }
			    } else {
				if (msg->tsp_hopcnt-- <= 0)
				    break;
				bytenetorder(msg);
				ntp = nettab;
				for (; ntp != NULL; ntp = ntp->next)
				    if (ntp->status == MASTER)
					if (sendto(sock, (char *)msg, 
					    sizeof(struct tsp), 0,
					    &ntp->dest_addr, length) < 0) {
						syslog(LOG_ERR, MSGSTR(SENDTO2, "sendto: %m"));
						exit(1);
					}
			    }
			} else {
			    /*
			     * We should not have received this from a net
			     * we are master on.  There must be two masters
			     * in this case.
			     */
			    if (fromnet->my_addr.s_addr == from.sin_addr.s_addr)
				break;
			    for (;;) {
				to.tsp_type = TSP_RESOLVE;
				answer = acksend(&to, &fromnet->dest_addr,
				    (char *)ANYADDR, TSP_MASTERACK,
				    fromnet);
				if (answer == NULL)
					break;
				to.tsp_type = TSP_QUIT;
				(void)strcpy(to.tsp_name, hostname);
				server = from;
				answer = acksend(&to, &server, answer->tsp_name,
				    TSP_ACK, (struct netinfo *)NULL);
				if (answer == NULL) {
					syslog(LOG_ERR, MSGSTR(KILLERR2, "loop kill error2"));
				} else {
					(void)addmach(msg->tsp_name, &from);
				}
			    }
			}
			break;
		default:
			if (trace) {
				fprintf(fd, MSGSTR(GARBAGE, "garbage: "));
				print(msg, &from);
			}
			break;
		}
	}
	goto loop;
}

/*
 * Used before answering a broadcast message to avoid network
 * contention and likely collisions.
 */
answerdelay()
{
	struct timeval timeout;

	timeout.tv_sec = 0;
	timeout.tv_usec = delay1;

	(void)select(0, (fd_set *)NULL, (fd_set *)NULL, (fd_set *)NULL,
	    &timeout);
	return;
}
