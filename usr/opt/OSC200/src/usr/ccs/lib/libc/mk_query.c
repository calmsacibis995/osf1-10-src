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
static char	*sccsid = "@(#)$RCSfile: mk_query.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/07 23:29:18 $";
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
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 * 
 * mk_query.c	6.7 (Berkeley) 3/7/88
 * res_mkquery.c	6.7 (Berkeley) 3/7/88
 */


/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak mk_query = __mk_query
#endif
#endif
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#if	!CMU
#include <resolv.h>
#endif

/*
 * Form all types of queries.
 * Returns the size of the result or -1.
 */
#if	CMU
mk_query(op, dname, class, type, data, datalen, newrr, buf, buflen)
#else
res_mkquery(op, dname, class, type, data, datalen, newrr, buf, buflen)
#endif
	int op;			/* opcode of query */
	char *dname;		/* domain name */
	int class, type;	/* class and type of query */
	char *data;		/* resource record data */
	int datalen;		/* length of data */
	struct rrec *newrr;	/* new rr for modify or append */
	char *buf;		/* buffer to put query */
	int buflen;		/* size of buffer */
{
	register HEADER *hp;
	register char *cp;
	register int n;
	char dnbuf[MAXDNAME];
	char *dnptrs[10], **dpp, **lastdnptr;
	extern char *index();

#if	!CMU
#ifdef DEBUG_RESOLVER
	if (_res.options & RES_DEBUG)
		printf("res_mkquery(%d, %s, %d, %d)\n", op, dname, class, type);
#endif DEBUG_RESOLVER
#endif
	/*
	 * Initialize header fields.
	 */
	hp = (HEADER *) buf;
#if	CMU
	hp->id = 0;
#else
	hp->id = htons(++_res.id);
#endif
	hp->opcode = op;
	hp->qr = hp->aa = hp->tc = hp->ra = 0;
#if	CMU
	hp->pr = 0;
	hp->rd = 0;
#else
	hp->pr = (_res.options & RES_PRIMARY) != 0;
	hp->rd = (_res.options & RES_RECURSE) != 0;
#endif
	hp->rcode = NOERROR;
	hp->qdcount = 0;
	hp->ancount = 0;
	hp->nscount = 0;
	hp->arcount = 0;
	cp = buf + sizeof(HEADER);
	buflen -= sizeof(HEADER);
	dpp = dnptrs;
	*dpp++ = buf;
	*dpp++ = NULL;
	lastdnptr = dnptrs + sizeof(dnptrs)/sizeof(dnptrs[0]);
	/*
	 * If the domain name contains no dots (single label), then
	 * append the default domain name to the one given.
	 */
#if	!CMU
	if ((_res.options & RES_DEFNAMES) && dname != 0 && dname[0] != '\0' &&
	    index(dname, '.') == NULL) {
		if (!(_res.options & RES_INIT))
			if (res_init() == -1)
				return(-1);
		if (_res.defdname[0] != '\0') {
			(void)sprintf(dnbuf, "%s.%s", dname, _res.defdname);
			dname = dnbuf;
		}
	}
#endif
	/*
	 * perform opcode specific processing
	 */
	switch (op) {
	case QUERY:
		buflen -= QFIXEDSZ;
		if ((n = dn_comp(dname, cp, buflen, dnptrs, lastdnptr)) < 0)
			return (-1);
		cp += n;
		buflen -= n;
		putshort(type, cp);
		cp += sizeof(u_short);
		putshort(class, cp);
		cp += sizeof(u_short);
		hp->qdcount = htons(1);
		if (op == QUERY || data == NULL)
			break;
		/*
		 * Make an additional record for completion domain.
		 */
		buflen -= RRFIXEDSZ;
		if ((n = dn_comp(data, cp, buflen, dnptrs, lastdnptr)) < 0)
			return (-1);
		cp += n;
		buflen -= n;
		putshort(T_NULL, cp);
		cp += sizeof(u_short);
		putshort(class, cp);
		cp += sizeof(u_short);
		putlong(0, cp);
		cp += sizeof(u_int);
		putshort(0, cp);
		cp += sizeof(u_short);
		hp->arcount = htons(1);
		break;

	case IQUERY:
		/*
		 * Initialize answer section
		 */
		if (buflen < 1 + RRFIXEDSZ + datalen)
			return (-1);
		*cp++ = '\0';	/* no domain name */
		putshort(type, cp);
		cp += sizeof(u_short);
		putshort(class, cp);
		cp += sizeof(u_short);
		putlong(0, cp);
		cp += sizeof(u_int);
		putshort(datalen, cp);
		cp += sizeof(u_short);
		if (datalen) {
			bcopy(data, cp, datalen);
			cp += datalen;
		}
		hp->ancount = htons(1);
		break;

#ifdef ALLOW_UPDATES
	/*
	 * For UPDATEM/UPDATEMA, do UPDATED/UPDATEDA followed by UPDATEA
	 * (Record to be modified is followed by its replacement in msg.)
	 */
	case UPDATEM:
	case UPDATEMA:

	case UPDATED:
		/*
		 * The res code for UPDATED and UPDATEDA is the same; user
		 * calls them differently: specifies data for UPDATED; server
		 * ignores data if specified for UPDATEDA.
		 */
	case UPDATEDA:
		buflen -= RRFIXEDSZ + datalen;
		if ((n = dn_comp(dname, cp, buflen, dnptrs, lastdnptr)) < 0)
			return (-1);
		cp += n;
		putshort(type, cp);
                cp += sizeof(u_short);
                putshort(class, cp);
                cp += sizeof(u_short);
		putlong(0, cp);
		cp += sizeof(u_int);
		putshort(datalen, cp);
                cp += sizeof(u_short);
		if (datalen) {
			bcopy(data, cp, datalen);
			cp += datalen;
		}
		if ( (op == UPDATED) || (op == UPDATEDA) ) {
			hp->ancount = htons(0);
			break;
		}
		/* Else UPDATEM/UPDATEMA, so drop into code for UPDATEA */

	case UPDATEA:	/* Add new resource record */
		buflen -= RRFIXEDSZ + datalen;
		if ((n = dn_comp(dname, cp, buflen, dnptrs, lastdnptr)) < 0)
			return (-1);
		cp += n;
		putshort(newrr->r_type, cp);
                cp += sizeof(u_short);
                putshort(newrr->r_class, cp);
                cp += sizeof(u_short);
		putlong(0, cp);
		cp += sizeof(u_int);
		putshort(newrr->r_size, cp);
                cp += sizeof(u_short);
		if (newrr->r_size) {
			bcopy(newrr->r_data, cp, newrr->r_size);
			cp += newrr->r_size;
		}
		hp->ancount = htons(0);
		break;

#endif ALLOW_UPDATES
	}
	return (cp - buf);
}
