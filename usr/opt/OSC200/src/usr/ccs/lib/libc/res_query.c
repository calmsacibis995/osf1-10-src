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
static char *rcsid = "@(#)$RCSfile: res_query.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 22:20:30 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * res_query.c	5.7 (Berkeley) 6/1/90
 */


/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak res_query = __res_query
#pragma weak res_querydomain = __res_querydomain
#pragma weak res_search = __res_search
#if defined(_THREAD_SAFE)
#pragma weak hostalias_r = __hostalias_r
#endif
#if !defined(_THREAD_SAFE)
#pragma weak hostalias = __hostalias
#endif
#endif
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>

#include "ts_supp.h"

#if PACKETSZ > 1024
#define MAXPACKET	PACKETSZ
#else
#define MAXPACKET	1024
#endif

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex	_resolv_rmutex;

#define	RETURN(val)	return (TS_UNLOCK(&_resolv_rmutex), val)
#else
#define	RETURN(val)	return (val)
#endif	/* _THREAD_SAFE */

#ifdef DEBUG
#define	PRINT_DBG(info) \
	if (_res.options & RES_DEBUG) \
		printf info
#else
#define PRINT_DBG(info)
#endif  /* DEBUG */

extern int	_Set_h_errno(int), _Get_h_errno();

/*
 * Formulate a normal query, send, and await answer.
 * Returned answer is placed in supplied buffer "answer".
 * Perform preliminary check of answer, returning success only
 * if no error is indicated and the answer count is nonzero.
 * Return the size of the response on success, -1 on error.
 * Error number is left in h_errno.
 * Caller must parse answer and determine whether it answers the question.
 */
int
res_query(
	char *name,		/* domain name */
	int class, int type,	/* class and type of query */
	u_char *answer,		/* buffer to put answer */
	int anslen)		/* size of answer buffer */
{
	char	buf[MAXPACKET];
	HEADER	*hp;
	int	n;

	TS_LOCK(&_resolv_rmutex);

	if ((_res.options & RES_INIT) == 0 && res_init() == -1)
		RETURN (-1);

	PRINT_DBG(("res_query(%s, %d, %d)\n", name, class, type));

	n = res_mkquery(QUERY, name, class, type, (char *)NULL, 0, NULL,
	    buf, sizeof(buf));

	if (n <= 0) {
		PRINT_DBG(("res_query: mkquery failed\n"));
		_Set_h_errno(NO_RECOVERY);
		RETURN (n);
	}
	n = res_send(buf, n, (char *)answer, anslen);
	if (n < 0) {
		PRINT_DBG(("res_query: send error\n"));
		_Set_h_errno(TRY_AGAIN);
		RETURN(n);
	}

	hp = (HEADER *) answer;
	if (hp->rcode != NOERROR || ntohs(hp->ancount) == 0) {
		PRINT_DBG(("rcode = %d, ancount=%d\n", hp->rcode, \
			   ntohs(hp->ancount)));
		switch (hp->rcode) {
			case NXDOMAIN:
				_Set_h_errno(HOST_NOT_FOUND);
				break;
			case SERVFAIL:
				_Set_h_errno(TRY_AGAIN);
				break;
			case NOERROR:
				_Set_h_errno(NO_DATA);
				break;
			case FORMERR:
			case NOTIMP:
			case REFUSED:
			default:
				_Set_h_errno(NO_RECOVERY);
				break;
		}
		RETURN (-1);
	}
	RETURN (n);
}

/*
 * Formulate a normal query, send, and retrieve answer in supplied buffer.
 * Return the size of the response on success, -1 on error.
 * If enabled, implement search rules until answer or unrecoverable failure
 * is detected.  Error number is left in h_errno.
 * Only useful for queries in the same name hierarchy as the local host
 * (not, for example, for host address-to-name lookups in domain in-addr.arpa).
 */
int
res_search(
	char *name,		/* domain name */
	int class, int type,	/* class and type of query */
	u_char *answer,		/* buffer to put answer */
	int anslen)		/* size of answer */
{
	register char	*cp, **domain;
	int		n, ret, got_nodata = 0;
#ifdef _THREAD_SAFE
	char		buf[MAXDNAME];
#endif	/* _THREAD_SAFE */

	TS_LOCK(&_resolv_rmutex);

	if ((_res.options & RES_INIT) == 0 && res_init() == -1)
		RETURN (-1);

	_Seterrno(0);
	_Set_h_errno(HOST_NOT_FOUND);		/* default, if we never query */
	for (cp = name, n = 0; *cp; cp++)
		if (*cp == '.')
			n++;
#ifdef _THREAD_SAFE
	if (n == 0 && (cp = (hostalias_r(name, buf, sizeof(buf)) ? 0 : buf)))
#else
	if (n == 0 && (cp = hostalias(name)))
#endif	/* _THREAD_SAFE */
		RETURN (res_query(cp, class, type, answer, anslen));

	/*
	 * We do at least one level of search if
	 *	- there is no dot and RES_DEFNAME is set, or
	 *	- there is at least one dot, there is no trailing dot,
	 *	  and RES_DNSRCH is set.
	 */
	if ((n == 0 && _res.options & RES_DEFNAMES) ||
	   (n != 0 && *--cp != '.' && _res.options & RES_DNSRCH))
	     for (domain = _res.dnsrch; *domain; domain++) {
		ret = res_querydomain(name, *domain, class, type,
		    answer, anslen);
		if (ret > 0)
			RETURN (ret);
		/*
		 * If no server present, give up.
		 * If name isn't found in this domain,
		 * keep trying higher domains in the search list
		 * (if that's enabled).
		 * On a NO_DATA error, keep trying, otherwise
		 * a wildcard entry of another type could keep us
		 * from finding this entry higher in the domain.
		 * If we get some other error (negative answer or
		 * server failure), then stop searching up,
		 * but try the input name below in case it's fully-qualified.
		 */
		if (_Geterrno() == ECONNREFUSED) {
			_Set_h_errno(TRY_AGAIN);
			RETURN (-1);
		}
		if (_Get_h_errno() == NO_DATA)
			got_nodata++;
		if ((_Get_h_errno() != HOST_NOT_FOUND && 
		     _Get_h_errno() != NO_DATA) || 
		     (_res.options & RES_DNSRCH) == 0)
			break;
	}
	/*
	 * If the search/default failed, try the name as fully-qualified,
	 * but only if it contained at least one dot (even trailing).
	 * This is purely a heuristic; we assume that any reasonable query
	 * about a top-level domain (for servers, SOA, etc) will not use
	 * res_search.
	 */
	if (n && (ret = res_querydomain(name, (char *)NULL, class, type,
	    answer, anslen)) > 0)
		RETURN (ret);
	if (got_nodata)
		_Set_h_errno(NO_DATA);
	RETURN (-1);
}

/*
 * Perform a call on res_query on the concatenation of name and domain,
 * removing a trailing dot from name if domain is NULL.
 */
int
res_querydomain(
	char *name,
	char *domain,
	int class, int type,	/* class and type of query */
	u_char *answer,		/* buffer to put answer */
	int anslen)		/* size of answer */
{
	char	nbuf[2*MAXDNAME+2];
	char	*longname = nbuf;
	int	n;

	PRINT_DBG(("res_querydomain(%s, %s, %d, %d)\n", \
		   name, domain, class, type));

	if (domain == NULL) {
		/*
		 * Check for trailing '.';
		 * copy without '.' if present.
		 */
		n = strlen(name) - 1;
		if (name[n] == '.' && n < sizeof(nbuf) - 1) {
			bcopy(name, nbuf, n);
			nbuf[n] = '\0';
		} else
			longname = name;
	} else
		(void)sprintf(nbuf, "%.*s.%.*s",
		    MAXDNAME, name, MAXDNAME, domain);

	return (res_query(longname, class, type, answer, anslen));
}

#ifdef _THREAD_SAFE
#define	LEN	len
int
hostalias_r(register char *name, char *abuf, int len)
#else
#define	LEN	(sizeof(abuf))
char *
hostalias(register char *name)
#endif	/* _THREAD_SAFE */
{
	register char	*C1, *C2;
	FILE		*fp;
	char		*file, *getenv(), *strcpy(), *strncpy();
	char		buf[BUFSIZ];
#ifndef _THREAD_SAFE
	static char	abuf[MAXDNAME];
#endif	/* _THREAD_SAFE */

	TS_EINVAL((abuf == 0 || len < MAXDNAME));
	file = getenv("HOSTALIASES");
	if (file == NULL || (fp = fopen(file, "r")) == NULL)
		return (TS_FAILURE);
	buf[sizeof(buf) - 1] = '\0';
	while (fgets(buf, sizeof(buf), fp)) {
		for (C1 = buf; *C1 && !isspace(*C1); ++C1);
		if (!*C1)
			break;
		*C1 = '\0';
		if (!strcasecmp(buf, name)) {
			while (isspace(*++C1));
			if (!*C1)
				break;
			for (C2 = C1 + 1; *C2 && !isspace(*C2); ++C2);
			abuf[LEN - 1] = *C2 = '\0';
			(void)strncpy(abuf, C1, LEN - 1);
			fclose(fp);
			return (TS_FOUND(abuf));
		}
	}
	fclose(fp);
	return (TS_NOTFOUND);
}
