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
static char	*sccsid = "@(#)$RCSfile: whois.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/10/11 19:54:35 $";
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
 * COMPONENT_NAME: TCPIP whois.c
 * 
 * FUNCTIONS: MSGSTR, Mwhois, usage 
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
 * Copyright (c) 1980 Regents of the University of California.
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
/*
#ifndef lint
char copyright[] =
" Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
#endif 

#ifndef lint
static char sccsid[] = "whois.c	5.6 (Berkeley) 10/11/88";
#endif  not lint */

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <stdio.h>
#include <netdb.h>

#ifdef MSG
#include "whois_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) NLcatgets(catd,MS_WHOIS,n,s) 
#else
#define MSGSTR(n,s) s
#endif

static usage();

#define	NICHOST	"rs.internic.net"

main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;
	extern int optind;
	register FILE *sfi, *sfo;
	register int c;
	struct sockaddr_in sin;
	struct hostent *hp;
	struct servent *sp;
	int ch, s;
	char *host;

#ifdef MSG
	catd = NLcatopen (MF_WHOIS, NL_CAT_LOCALE);
#endif
	host = NICHOST;
	while ((ch = getopt(argc, argv, "h:")) != EOF)
		switch((char)ch) {
		case 'h':
			host = optarg;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (argc != 1)
		usage();

	hp = gethostbyname(host);
	if (hp == NULL) {
		fprintf(stderr, MSGSTR(HOST, "whois: %s: "), host);
		herror((char *)NULL);
		exit(1);
	}
	host = hp->h_name;
	s = socket(hp->h_addrtype, SOCK_STREAM, 0, 0);
	if (s < 0) {
		perror(MSGSTR(SOCKET, "whois: socket"));
		exit(2);
	}
	bzero((caddr_t)&sin, sizeof (sin));
	sin.sin_family = hp->h_addrtype;
	if (bind(s, &sin, sizeof (sin), 0) < 0) {
		perror(MSGSTR(BIND, "whois: bind"));
		exit(3);
	}
	bcopy(hp->h_addr, &sin.sin_addr, hp->h_length);
	sp = getservbyname("whois", "tcp");
	if (sp == NULL) {
		fprintf(stderr, MSGSTR(UNKSERVICE, "whois: whois/tcp: unknown service\n"));
		exit(4);
	}
	sin.sin_port = sp->s_port;
	if (connect(s, &sin, sizeof (sin), 0) < 0) {
		perror(MSGSTR(CONNECT, "whois: connect"));
		exit(5);
	}
	sfi = fdopen(s, "r");
	sfo = fdopen(s, "w");
	if (sfi == NULL || sfo == NULL) {
		perror(MSGSTR(FDOPEN, "fdopen"));
		close(s);
		exit(1);
	}
	fprintf(sfo,  "%s\r\n", *argv);
	(void)fflush(sfo);
	while ((c = getc(sfi)) != EOF)
		putchar(c);
	
	exit(0);
}

static
usage()
{
	fprintf(stderr, MSGSTR(USAGE, "usage: whois [-h host] name\n"));
	exit(1);
}
