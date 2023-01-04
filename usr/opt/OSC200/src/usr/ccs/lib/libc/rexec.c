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
static char	*sccsid = "@(#)$RCSfile: rexec.c,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/06/07 22:27:52 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#if !defined(lint) && !defined(_NOIDENT)
#endif
/*
 * Copyright International Business Machines Corp. 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * rexec.c	1.2  com/lib/c/net,3.1,8943 7/21/89 08:08:08
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak rexec_r = __rexec_r
#endif
#if !defined(_THREAD_SAFE)
#pragma weak rexec = __rexec
#endif
#endif
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <nl_types.h>
#include "libc_msg.h"

static nl_catd catd;

int
#ifdef _THREAD_SAFE
rexec_r(char **ahost, int rport, char *name, char *pass, char *cmd, int *fd2p,
	struct hostent_data *host_data)
#else
rexec(char **ahost, int rport, char *name, char *pass, char *cmd, int *fd2p)
#endif	/* _THREAD_SAFE */
{
	int s, timo = 1, s3;
	struct sockaddr_in sin, sin2, from;
	char c;
	short port=0;
	struct hostent *hp;

#ifdef	_THREAD_SAFE
	struct hostent host;
	hp = &host;

	if (gethostbyname_r(*ahost, hp, host_data) < 0)
#else
	hp = gethostbyname(*ahost);
	if (hp == 0)
#endif
	{
                catd = catopen(MF_LIBC,0);
                fprintf(stderr, catgets(catd,LIBCNET,NET22,
                                        "%s: unknown host\n"), *ahost);
		catclose(catd);
		return (-1);
	}
	*ahost = hp->h_name;
	ruserpass(hp->h_name, &name, &pass);
retry:
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		perror("rexec: socket");
		return (-1);
	}
	sin.sin_family = hp->h_addrtype;
	sin.sin_port = rport;
	bcopy(hp->h_addr, (caddr_t)&sin.sin_addr, hp->h_length);
	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		if (_Geterrno() == ECONNREFUSED && timo <= 16) {
			(void) close(s);
			sleep(timo);
			timo *= 2;
			goto retry;
		}
		perror(hp->h_name);
		return (-1);
	}
	if (fd2p == 0) {
		(void) write(s, "", 1);
		port = 0;
	} else {
		char num[8];
		int s2, sin2len;
		
		s2 = socket(AF_INET, SOCK_STREAM, 0);
		if (s2 < 0) {
			(void) close(s);
			return (-1);
		}
		listen(s2, 1);
		sin2len = sizeof (sin2);
		if (getsockname(s2, (struct sockaddr *)&sin2, &sin2len) < 0 ||
		  sin2len != sizeof (sin2)) {
			perror("getsockname");
			(void) close(s2);
			goto bad;
		}
		port = ntohs((u_short)sin2.sin_port);
		(void) sprintf(num, "%d", port);
		(void) write(s, num, strlen(num)+1);
		{ int len = sizeof (from);
		  s3 = accept(s2, (struct sockaddr *)&from, &len);
		  close(s2);
		  if (s3 < 0) {
			perror("accept");
			port = 0;
			goto bad;
		  }
		}
		*fd2p = s3;
	}
	(void) write(s, name, strlen(name) + 1);
	/* should public key encypt the password here */
	(void) write(s, pass, strlen(pass) + 1);
	(void) write(s, cmd, strlen(cmd) + 1);
	if (read(s, &c, 1) != 1) {
		perror(*ahost);
		goto bad;
	}
	if (c != 0) {
		while (read(s, &c, 1) == 1) {
			(void) write(2, &c, 1);
			if (c == '\n')
				break;
		}
		goto bad;
	}
	return (s);
bad:
	if (port)
		(void) close(*fd2p);
	(void) close(s);
	return (-1);
}
