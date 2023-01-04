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
static char	*sccsid = "@(#)$RCSfile: res_init.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/07 22:19:46 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*-
 * Copyright (c) 1985, 1989 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * res_init.c	6.14 (Berkeley) 6/27/90
 */


/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak res_init = __res_init
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/nameser.h>
#include <resolv.h>

#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"

extern struct rec_mutex	_resolv_rmutex;
#endif	/* _THREAD_SAFE */

/*
 * Resolver state default settings
 */
struct state _res = {
	RES_TIMEOUT,               	/* retransmition time interval */
	4,                         	/* number of times to retransmit */
	RES_DEFAULT,			/* options flags */
	1,                         	/* number of name servers */
};

/*
 * Set up default settings.  If the configuration file exist, the values
 * there will have precedence.  Otherwise, the server address is set to
 * INADDR_ANY and the default domain name comes from the gethostname().
 *
 * The configuration file should only be used if you want to redefine your
 * domain or run without a server on your machine.
 *
 * Return 0 if completes successfully, -1 on error
 */
int
res_init(void)
{
	register FILE	*fp;
	register char	*cp, **pp;
	register int	n;
	char		buf[BUFSIZ];
	int		nserv = 0;    /* nameserver records read from file */
	int		haveenv = 0;
	int		havesearch = 0;
	extern u_int	inet_addr();
	extern char	*index();
	extern char	*strcpy(), *strncpy();
	extern char	*getenv();

	TS_LOCK(&_resolv_rmutex);

	if(_res.options & RES_INIT) {
		TS_UNLOCK(&_resolv_rmutex);
		return(0);
	}

	_res.nsaddr.sin_addr.s_addr = INADDR_ANY;
	_res.nsaddr.sin_family = AF_INET;
	_res.nsaddr.sin_port = htons(NAMESERVER_PORT);
	_res.nscount = 0;

	/* Allow user to override the local domain definition */
	if ((cp = getenv("LOCALDOMAIN")) != NULL) {
		(void)strncpy(_res.defdname, cp, sizeof(_res.defdname));
		haveenv++;
	}

	if ((fp = fopen(_PATH_RESCONF, "r")) != NULL) {
	    /* read the config file */
	    while (fgets(buf, sizeof(buf), fp) != NULL) {
		/* read default domain name */
		if (!strncmp(buf, "domain", sizeof("domain") - 1)) {
		    if (haveenv)	/* skip if have from environ */
			    continue;
		    cp = buf + sizeof("domain") - 1;
		    while (*cp == ' ' || *cp == '\t')
			    cp++;
		    if ((*cp == '\0') || (*cp == '\n'))
			    continue;
		    (void)strncpy(_res.defdname, cp, sizeof(_res.defdname) - 1);
		    if ((cp = index(_res.defdname, '\n')) != NULL)
			    *cp = '\0';
		    havesearch = 0;
		    continue;
		}
		/* set search list */
		if (!strncmp(buf, "search", sizeof("search") - 1)) {
		    if (haveenv)	/* skip if have from environ */
			    continue;
		    cp = buf + sizeof("search") - 1;
		    while (*cp == ' ' || *cp == '\t')
			    cp++;
		    if ((*cp == '\0') || (*cp == '\n'))
			    continue;
		    (void)strncpy(_res.defdname, cp, sizeof(_res.defdname) - 1);
		    if ((cp = index(_res.defdname, '\n')) != NULL)
			    *cp = '\0';
		    /*
		     * Set search list to be blank-separated strings
		     * on rest of line.
		     */
		    cp = _res.defdname;
		    pp = _res.dnsrch;
		    *pp++ = cp;
		    for (n = 0; *cp && pp < _res.dnsrch + MAXDNSRCH; cp++) {
			    if (*cp == ' ' || *cp == '\t') {
				    *cp = 0;
				    n = 1;
			    } else if (n) {
				    *pp++ = cp;
				    n = 0;
			    }
		    }
		    /* null terminate last domain if there are excess */
		    while (*cp != '\0' && *cp != ' ' && *cp != '\t')
			    cp++;
		    *cp = '\0';
		    *pp++ = 0;
		    havesearch = 1;
		    continue;
		}
		/* read nameservers to query */
		if (!strncmp(buf, "nameserver", sizeof("nameserver") - 1) &&
		   nserv < MAXNS) {
		    cp = buf + sizeof("nameserver") - 1;
		    while (*cp == ' ' || *cp == '\t')
			    cp++;
		    if ((*cp == '\0') || (*cp == '\n'))
			    continue;
		    if ((_res.nsaddr_list[nserv].sin_addr.s_addr =
			inet_addr(cp)) == (unsigned)-1) {
			    _res.nsaddr_list[nserv].sin_addr.s_addr
				= INADDR_ANY;
			    continue;
		    }
		    _res.nsaddr_list[nserv].sin_family = AF_INET;
		    _res.nsaddr_list[nserv].sin_port = htons(NAMESERVER_PORT);
		    nserv++;
		    continue;
		}
	    }
	    if (nserv > 0) 
		_res.nscount = nserv;
	    (void) fclose(fp);
	}
	if (_res.defdname[0] == 0) {
		if (gethostname(buf, sizeof(_res.defdname)) == 0 &&
		   (cp = index(buf, '.')))
			(void)strcpy(_res.defdname, cp + 1);
		else
			/* do not return 0 if either there is no nameserver
				to ask questions of or there is no default
				domain name */
			return(-1);
			/* if this return is not here, a core dump will
				result in the code below */
	}

	/* find components of local domain that might be searched */
	if (havesearch == 0) {
		pp = _res.dnsrch;
		*pp++ = _res.defdname;
		for (cp = _res.defdname, n = 0; *cp; cp++)
			if (*cp == '.')
				n++;
		cp = _res.defdname;
		for (; n >= LOCALDOMAINPARTS && pp < _res.dnsrch + MAXDFLSRCH;
		    n--) {
			cp = index(cp, '.');
			*pp++ = ++cp;
		}
		*pp++ = 0;
	}

	/* do not return 0 if either there is no nameserver
		to ask questions of or there is no default
		domain name */
	if(_res.nscount == 0)
		/* If this return is not here, res_send will send a request
			to a non-existent nameserver. */
		return(-1);

	_res.options |= RES_INIT;

	TS_UNLOCK(&_resolv_rmutex);
	return (0);
}
