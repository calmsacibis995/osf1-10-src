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
 *	@(#)$RCSfile: resolv.h,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/12/15 22:14:00 $
 */ 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * Copyright (c) 1983, 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(##)resolv.h	5.10 (Berkeley) 6/1/90
 */

#ifndef _RESOLV_H_
#define _RESOLV_H_

/*
 * Resolver configuration file.
 * Normally not present, but may contain the address of the
 * inital name server(s) to query and the domain search list.
 */

#ifndef _PATH_RESCONF
#define _PATH_RESCONF        "/etc/resolv.conf"
#endif

/*
 * Global defines and variables for resolver stub.
 */
#define	MAXNS		3		/* max # name servers we'll track */
#define MAXDFLSRCH      3               /* # default domain levels to try */
#define MAXDNSRCH       6               /* max # domains in search path */
#define	LOCALDOMAINPARTS 2		/* min levels in name that is "local" */

#define	RES_TIMEOUT	5		/* seconds between retries */

struct state {
	int	retrans;	 	/* retransmition time interval */
	int	retry;			/* number of times to retransmit */
	long	options;		/* option flags - see below. */
	int	nscount;		/* number of name servers */
	struct	sockaddr_in nsaddr_list[MAXNS];	/* address of name server */
#define	nsaddr	nsaddr_list[0]		/* for backward compatibility */
	u_short	id;			/* current packet id */
	char	defdname[MAXDNAME];	/* default domain */
	char	*dnsrch[MAXDNSRCH+1];	/* components of domain to search */
};

/*
 * Resolver options
 */
#define RES_INIT	0x0001		/* address initialized */
#define RES_DEBUG	0x0002		/* print debug messages */
#define RES_AAONLY	0x0004		/* authoritative answers only */
#define RES_USEVC	0x0008		/* use virtual circuit */
#define RES_PRIMARY	0x0010		/* query primary server only */
#define RES_IGNTC	0x0020		/* ignore trucation errors */
#define RES_RECURSE	0x0040		/* recursion desired */
#define RES_DEFNAMES	0x0080		/* use default domain name */
#define RES_STAYOPEN	0x0100		/* Keep TCP socket open */
#define RES_DNSRCH	0x0200		/* search up local domain tree */

#define RES_DEFAULT	(RES_RECURSE | RES_DEFNAMES | RES_DNSRCH)

extern struct state _res;

#ifdef _NO_PROTO
extern char *p_cdname(), *p_rr(), *p_type(), *p_class(), *p_time();

#else /* _NO_PROTO */
#include <stdio.h>
#include <arpa/nameser.h>
_BEGIN_CPLUSPLUS
extern char *p_cdname(char *, char *, FILE *);
extern char *p_rr(char *, char *, FILE *);
extern char *p_type(int);
extern char *p_class(int);
extern int res_mkquery(int, char *, int, int, char *, int, struct rrec *, char *, int);
extern int res_query(char *, int, int, u_char *, int);
extern int res_search(char *, int, int, u_char *, int);
extern int res_querydomain(char *, char *, int, int, u_char *, int);
extern int res_send(char *,int, char *, int);
extern int res_init(void);
extern int dn_comp(unsigned char *, unsigned char *, int, unsigned char **, unsigned char **);
extern int dn_expand(unsigned char *, unsigned char *, unsigned char *, unsigned char *, int);
_END_CPLUSPLUS
#endif /* _NO_PROTO */

_BEGIN_CPLUSPLUS
#if defined(_REENTRANT) || defined(_THREAD_SAFE)
extern int	hostalias_r(char *, char *, int);
#else
extern char	*hostalias(char *);
#endif	/* _REENTRANT || _THREAD_SAFE */
_END_CPLUSPLUS
 
#endif /* _RESOLV_H_ */
