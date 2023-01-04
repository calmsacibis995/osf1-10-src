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
static char	*sccsid = "@(#)$RCSfile: domain.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/09/15 16:00:40 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#include "sendmail.h"

#if !defined(lint) && !defined(_NOIDENT)
#ifdef NAMED_BIND

#else

#endif
#endif
/*
 * Copyright (c) 1986 Eric P. Allman
 * Copyright (c) 1988 Regents of the University of California.
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
 */

/*
#ifndef lint
#ifdef NAMED_BIND

#else

#endif
#endif
*/
#ifdef NAMED_BIND

#include <sys/param.h>
#include <errno.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <netdb.h>

typedef union {
	HEADER qb1;
	char qb2[PACKETSZ];
} querybuf;

static char hostbuf[MAXMXHOSTS*PACKETSZ];

getmxrr(host, mxhosts, localhost, rcode)
	char *host, **mxhosts, *localhost;
	int *rcode;
{
	extern int h_errno;
	register u_char *eom, *cp;
	register int i, j, n, nmx;
	register u_char *bp;
	HEADER *hp;
	querybuf answer;
	int ancount, qdcount, buflen, seenlocal;
	u_short pref, localpref, type;
	u_int prefer[MAXMXHOSTS];

	errno = 0;
	n = res_search(host, C_IN, T_MX, (char *)&answer, sizeof(answer));
	if (n < 0)
	{
		if (tTd(8, 1))
			printf("getmxrr: res_search failed (errno=%d, h_errno=%d)\n",
			    errno, h_errno);
		switch (h_errno)
		{
		  case NO_DATA:
		  case NO_RECOVERY:
			/* no MX data on this host */
			goto punt;

		  case HOST_NOT_FOUND:
			/* the host just doesn't exist */
			*rcode = EX_NOHOST;
			break;

		  case TRY_AGAIN:
			/* couldn't connect to the name server */
			if (!UseNameServer && errno == ECONNREFUSED)
				goto punt;

			/* it might come up later; better queue it up */
			*rcode = EX_TEMPFAIL;
			break;

		  default:
			goto punt;
		}

		/* irreconcilable differences */
		return (-1);
	}

	/* find first satisfactory answer */
	hp = (HEADER *)&answer;
	cp = (u_char *)&answer + sizeof(HEADER);
	eom = (u_char *)&answer + n;
	for (qdcount = ntohs(hp->qdcount); qdcount--; cp += n + QFIXEDSZ)
		if ((n = dn_skipname(cp, eom)) < 0)
			goto punt;
	nmx = 0;
	seenlocal = 0;
	buflen = sizeof(hostbuf);
	bp = (unsigned char *)hostbuf;
	ancount = ntohs(hp->ancount);
	while (--ancount >= 0 && cp < eom && nmx < MAXMXHOSTS) {
		if ((n = dn_expand((unsigned char *)&answer, eom, cp, bp, buflen)) < 0)
			break;
		cp += n;
		GETSHORT(type, cp);
 		cp += sizeof(u_short) + sizeof(u_int);
		GETSHORT(n, cp);
		if (type != T_MX)  {
			if (tTd(8, 1) || _res.options & RES_DEBUG)
				printf("getmxrr: unexpected answer type %d, size %d\n",
				    type, n);
			cp += n;
			continue;
		}
		GETSHORT(pref, cp);
		if ((n = dn_expand((unsigned char *)&answer, eom, cp, bp, buflen)) < 0)
			break;
		cp += n;
		if (!strcasecmp(bp, localhost)) {
			if (seenlocal == 0 || pref < localpref)
				localpref = pref;
			seenlocal = 1;
			continue;
		}
		/* next line assumes knowledge of data sizes */
		prefer[nmx] = (pref << 16) | (random() & 0xffff);
		mxhosts[nmx++] = (char *)bp;
		n = strlen(bp) + 1;
		bp += n;
		buflen -= n;
	}
	if (nmx == 0) {
punt:		mxhosts[0] = strcpy(hostbuf, host);
		return(1);
	}

	/* sort the records */
	for (i = 0; i < nmx; i++) {
		for (j = i + 1; j < nmx; j++) {
			if (prefer[i] > prefer[j]) {
				register u_int temp;
				register char *temp1;

				temp = prefer[i];
				prefer[i] = prefer[j];
				prefer[j] = temp;
				temp1 = mxhosts[i];
				mxhosts[i] = mxhosts[j];
				mxhosts[j] = temp1;
			}
		}
		if (seenlocal && (prefer[i] >> 16) >= localpref) {
			/*
			 * truncate higher pref part of list; if we're
			 * the best choice left, we should have realized
			 * awhile ago that this was a local delivery.
			 */
			if (i == 0)
				goto punt;
			nmx = i;
			break;
		}
	}
	return(nmx);
}

/*
**	Getcanonname() below is broken in the sense that it won't return
**	unqualified local host names with their full domain extension,
**	unless the argument is an alias.
**
**	Since gethostbyname() calls the name server with bind 4.8,
**	I don't see why this function would be needed at all.  I've
**	therefore restored the old code in maphostname() of daemon.c
**	that uses gethostbyname().  If there's something I've missed,
**	feel free to change maphostname() to again call getcanonname(),
**	but also make sure that the latter will qualify the host with
**	its full domain AND return a status code indicating if the host
**	was found.
**
**	Lennart Lovstrand, Rank Xerox EuroPARC, 24-Aug-88
**
**	Unfortunately, just using gethostbyname won't do it.  It queries for
**	and returns only A Resource Records, so it will miss a host with only
**	an MX record listed.  If you're trying to deliver everything you can
**	via the Internet, and deliver to only those hosts who are not in the
**	domain name system at all by a "smart UUCP" mailer like smail, you
**	will end up delivering more than you want to via that smart mailer.
**	In that case, gethostbyname isn't enough.
**
**	Chet Ramey, Case Western Reserve University, 15-Sep-88
**
**	Changed to make it return FALSE on an error, TRUE if an answer was
**	found (any answer is enough).
**
**	Chet Ramey, Case Western Reserve University, 16-Sep-88
**
**	In the case of a host with a MX record pointing at localhost,
**	another routing method must be used.  Examine any MX RRs returned.
**	If the best one points to localhost, return FALSE.
**
**	Paul Pomes, University of Illinois, 10-Oct-88
*/
getcanonname(host, hbsize)
	char *host;
	int hbsize;
{
	register u_char *eom, *cp;
	register int n; 
	HEADER *hp;
	querybuf answer;
	u_short type;
	int first, ancount, qdcount, loopcnt;
	u_char nbuf[PACKETSZ];
	u_short MailPreference = 65535;
	char MailAgent[MAXNAME];
	char MyName[MAXNAME];
	char **MyAliases;

	extern char **myhostname();

	MailAgent[0] = '\0';

	loopcnt = 0;
loop:
	/*
	 * Use query type of ANY if possible (NO_WILDCARD_MX), which will
	 * find types CNAME, A, and MX, and will cause all existing records
	 * to be cached by our local server.  If there is (might be) a
	 * wildcard MX record in the local domain or its parents that are
	 * searched, we can't use ANY; it would cause fully-qualified names
	 * to match as names in a local domain.
	 */
# ifndef NO_WILDCARD_MX
	_res.options &= ( ~RES_DEFNAMES & 0xffff );
# endif /* NO_WILDCARD_MX */
	n = res_search(host, C_IN, T_ANY, (char *)&answer, sizeof(answer));
# ifndef NO_WILDCARD_MX
	_res.options |= RES_DEFNAMES;
# endif /* NO_WILDCARD_MX */

	if (n < 0) {
		if (tTd(8, 1))
			printf("getcanonname:  res_search failed (errno=%d, h_errno=%d)\n",
			    errno, h_errno);
		return FALSE;
	}

	/* find first satisfactory answer */
	hp = (HEADER *)&answer;
	ancount = ntohs(hp->ancount);

	/* we don't care about errors here, only if we got an answer */
	if (ancount == 0) {
		if (tTd(8, 1))
			printf("rcode = %d, ancount=%d\n", hp->rcode, ancount);
		return FALSE;
	}
	cp = (u_char *)&answer + sizeof(HEADER);
	eom = (u_char *)&answer + n;
	for (qdcount = ntohs(hp->qdcount); qdcount--; cp += n + QFIXEDSZ)
		if ((n = dn_skipname(cp, eom)) < 0)
			return FALSE;

	/*
	 * just in case someone puts a CNAME record after another record,
	 * check all records for CNAME; otherwise, just take the first
	 * name found.
	 */
	for (first = 1; --ancount >= 0 && cp < eom; cp += n) {
		if ((n = dn_expand((unsigned char *)&answer, eom, cp, nbuf,
		    sizeof(nbuf))) < 0)
			break;
		if (first) {			/* XXX */
			(void)strncpy(host, nbuf, hbsize);
			host[hbsize - 1] = '\0';
			first = 0;
		}
		cp += n;
		GETSHORT(type, cp);
 		cp += sizeof(u_short) + sizeof(u_int);
		GETSHORT(n, cp);
		if (type == T_CNAME)  {
			/*
			 * assume that only one cname will be found.  More
			 * than one is undefined.  Copy so that if dn_expand
			 * fails, `host' is still okay.
			 */
			if ((n = dn_expand((unsigned char *)&answer, eom, cp, nbuf,
			    sizeof(nbuf))) < 0)
				break;
			(void)strncpy(host, nbuf, hbsize); /* XXX */
			host[hbsize - 1] = '\0';
			if (++loopcnt > 8)	/* never be more than 1 */
				return FALSE;
			goto loop;
		}
		else if (type == T_MX) {
			/*
			 * Be sure that the best MX record doesn't point
			 * to the local machine.  If it does, some other
			 * delivery method is assumed.
			 */

			u_short preference;

			GETSHORT(preference, cp);
			if ((n = dn_expand((unsigned char *)&answer, eom, cp, nbuf,
			    sizeof(nbuf))) < 0)
				break;
			cp + n;
			if (tTd(8, 1))
				printf("getcannonname: MX host %s, preference %d\n",
				       nbuf, preference);
			 if (preference < MailPreference) {
				 MailPreference = preference;
				 (void) strcpy(MailAgent, nbuf);
			 }
		}
		else
			cp += n;
	}
	/* test MailAgent against $j */
	if (MailAgent[0] != '\0' && MyHostName != NULL &&
	    strcasecmp(MailAgent, MyHostName) == 0)
		return (FALSE);

	/* test MailAgent against our DNS name and aliases */
	if (MailAgent[0] != '\0' &&
	    (MyAliases = myhostname(MyName, MAXNAME)) != NULL) {
		if (strcasecmp(MailAgent, MyName) == 0)
			return FALSE;
		for (; *MyAliases != NULL; MyAliases++)
			if (strcasecmp(MailAgent, *MyAliases) == 0)
				return FALSE;
		return TRUE;
	}
	else
		return TRUE;
}

# else /* NAMED_BIND */

#include <netdb.h>

getcanonname(host, hbsize)
	char *host;
	int hbsize;
{
	struct hostent *hp;

	hp = gethostbyname(host);
	if (hp == NULL)
		return;

	if (strlen(hp->h_name) >= hbsize)
		return;

	(void) strcpy(host, hp->h_name);
}

# endif /* NAMED_BIND */
