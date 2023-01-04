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
/*	
 *	@(#)$RCSfile: globals.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/04/13 17:53:05 $
 */ 
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
 * 
 * COMPONENT_NAME: TCPIP globals.h
 * 
 * FUNCTIONS: MSGSTR 
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
 *
 *	globals.h	2.6 (Berkeley) 6/18/88
 * 	globals.h	1.2  com/sockcmd/timed,3.1,9021 10/8/89 17:45:48 
 */

#include <sys/param.h>
#include <stdio.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/syslog.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "timed_msg.h"
#define MSGSTR(n,s)   NLgetamsg(MF_TIMED, MS_TIMED, n, s)

extern int errno;
extern int sock;

#define SAMPLEINTVL	240		/* synch() freq for master, sec */
#define	MAXADJ		20		/* max correction (sec) for adjtime */
/*
 * Parameters for network time measurement
 * of each host using ICMP timestamp requests.
 */
#define RANGE		20		/* best expected round-trip time, ms */
#define MSGS 		5		/* # of timestamp replies to average */
#define TRIALS		10		/* max # of timestamp echos sent */

#define MINTOUT		360
#define MAXTOUT		900

#define GOOD		1
#define UNREACHABLE	2
#define NONSTDTIME	3
#define HOSTDOWN 	0x7fffffff

#define OFF	0
#define ON	1

/*
 * Global and per-network states.
 */
#define NOMASTER 	0		/* no master on any network */
#define SLAVE 		1
#define MASTER		2
#define IGNORE		4
#define ALL		(SLAVE|MASTER|IGNORE)
#define SUBMASTER	(SLAVE|MASTER)

#define NHOSTS		100	/* max number of hosts controlled by timed */

struct host {
	char *name;
	struct sockaddr_in addr;
	int delta;
	u_short seq;
};

struct netinfo {
	struct netinfo *next;
	u_int net;
	u_int mask;
	struct in_addr my_addr;
	struct sockaddr_in dest_addr;	/* broadcast addr or point-point */
	int status;
};

extern struct netinfo *nettab;
extern int status;
extern int trace;
extern int sock;
extern struct sockaddr_in from;
extern struct netinfo *fromnet, *slavenet;
extern FILE *fd;
extern char hostname[];
extern char tracefile[];
extern struct host hp[];
extern int backoff;
extern int delay1, delay2;
extern int slvcount;
extern int nslavenets;		/* Number of nets were I could be a slave */
extern int nmasternets;		/* Number of nets were I could be a master */
extern int nignorednets;	/* Number of ignored nets */
extern int nnets;		/* Number of nets I am connected to */

char *strcpy(), *malloc();
