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
static char	*sccsid = "@(#)$RCSfile: acksend.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:32:44 $";
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
 * COMPONENT_NAME: TCPIP acksend.c
 * 
 * FUNCTIONS: acksend 
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
/* acksend.c	1.2  com/sockcmd/timed,3.1,9021 10/8/89 17:42:44 */
/*
#ifndef lint
static char sccsid[] = "acksend.c	2.6 (Berkeley) 6/18/88";
#endif 
*/

#include "globals.h"
#include <protocols/timed.h>

#define RECEIVED	0
#define LOST	 	1
#define SECFORACK	1	/* seconds */
#define USECFORACK	0	/* microseconds */
#define MAXCOUNT	5

struct tsp *answer;

/*
 * Acksend implements reliable datagram transmission by using sequence 
 * numbers and retransmission when necessary.
 * `name' is the name of the destination
 * `addr' is the address to send to
 * If `name' is ANYADDR, this routine implements reliable broadcast.
 */

struct tsp *acksend(message, addr, name, ack, net)
struct tsp *message;
struct sockaddr_in *addr;
char *name;
int ack;
struct netinfo *net;
{
	int count;
	int flag;
	extern u_short sequence;
	struct timeval tout;
	struct tsp *readmsg();

	count = 0;

	message->tsp_vers = TSPVERSION;
	message->tsp_seq = sequence;
	if (trace) {
		fprintf(fd, MSGSTR(ACKDEND,"acksend: "));
		if (name == ANYADDR)
			fprintf(fd, MSGSTR(BROADCAST,"broadcast: "));
		else
			fprintf(fd, "%s: ", name);
		print(message, addr);
	}
	bytenetorder(message);
	do {
		if (sendto(sock, (char *)message, sizeof(struct tsp), 0, addr,
		    sizeof(struct sockaddr_in)) < 0) {
			syslog(LOG_ERR, MSGSTR(SENDTO, "acksend: sendto: %m"));
			exit(1);
		}
		tout.tv_sec = SECFORACK;
		tout.tv_usec = USECFORACK;
		answer  = readmsg(ack, name, &tout, net);
		if (answer != NULL) {
			if (answer->tsp_seq != sequence) {
				if (trace)
					fprintf(fd, MSGSTR(SEQ,"acksend: seq # %d != %d\n"),
					    answer->tsp_seq, sequence);
				continue;
			}
			flag = RECEIVED;
		} else {
			flag = LOST;
			if (++count == MAXCOUNT) {
				break;
			}
		}
	} while (flag != RECEIVED);
	sequence++;
	return(answer);
}
