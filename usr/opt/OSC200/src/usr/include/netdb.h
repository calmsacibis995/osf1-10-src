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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/* @(#)$RCSfile: netdb.h,v $ $Revision: 4.2.7.3 $ (OSF) $Date: 1993/06/14 19:02:41 $ */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *      netdb.h     5.10 (Berkeley) 6/27/88
 */


#ifndef _NETDB_H_
#define _NETDB_H_

#include <standards.h>
#include <rpc/netdb.h>

#define _PATH_HEQUIV    "/etc/hosts.equiv"
#define _PATH_HOSTS     "/etc/hosts"
#define _PATH_NETWORKS  "/etc/networks"
#define _PATH_PROTOCOLS "/etc/protocols"
#define _PATH_SERVICES  "/etc/services"

/* Internal constants
 */
#define _MAXALIASES	35
#define _MAXLINELEN	1024
#define _MAXADDRS	35
#define	_HOSTBUFSIZE	(BUFSIZ + 1)

/*
 * Structures returned by network data base library.  All addresses are
 * supplied in host order, and returned in network order (suitable for
 * use in system calls).
 */
struct  hostent {
	char	*h_name;	/* official name of host */
	char	**h_aliases;	/* alias list */
	int	h_addrtype;	/* host address type */
	int	h_length;	/* length of address */
	char	**h_addr_list;	/* list of addresses from name server */
#define	h_addr	h_addr_list[0]	/* address, for backward compatiblity */
};

#if defined(_REENTRANT) || defined(_THREAD_SAFE)
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>

/*
 * After a successful call to gethostbyname_r()/gethostbyaddr_r(), the
 * structure hostent_data will contain the data to which pointers in
 * the hostent structure will reference.
 */
struct  hostent_data {
	struct in_addr	host_addr;
	char		*h_addr_ptrs[_MAXADDRS + 1];
	char		hostaddr[_MAXADDRS];
	char		hostbuf[_HOSTBUFSIZE];
	char		*host_aliases[_MAXALIASES];
	char		*host_addrs[2];
	FILE		*hostf;
	int		svc_gethostflag;
	int		svc_gethostbind;
};
#endif	/* _REENTRANT || _THREAD_SAFE */

/*
 * Assumption here is that a network number
 * fits in 32 bits -- probably a poor one.
 */
struct	netent {
	char		*n_name;	/* official name of net */
	char		**n_aliases;	/* alias list */
	int		n_addrtype;	/* net address type */
	unsigned int	n_net;		/* network # */
};

#if defined(_REENTRANT) || defined(_THREAD_SAFE)
struct  netent_data {		/* should be considered opaque */
	FILE	*net_fp;
	char	line[_MAXLINELEN];
	char	*net_aliases[_MAXALIASES];
	int	_net_stayopen;
	int	svc_getnetflag;
};
#endif	/* _REENTRANT || _THREAD_SAFE */

struct	servent {
	char	*s_name;	/* official service name */
	char	**s_aliases;	/* alias list */
	int	s_port;		/* port # */
	char	*s_proto;	/* protocol to use */
};

#if defined(_REENTRANT) || defined(_THREAD_SAFE)
struct  servent_data {		/* should be considered opaque */
	FILE	*serv_fp;
	char	line[_MAXLINELEN];
	char	*serv_aliases[_MAXALIASES];
	int	_serv_stayopen;
	int	svc_getservflag;
};
#endif	/* _REENTRANT || _THREAD_SAFE */

struct	protoent {
	char	*p_name;	/* official protocol name */
	char	**p_aliases;	/* alias list */
	int	p_proto;	/* protocol # */
};

#if defined(_REENTRANT) || defined(_THREAD_SAFE)
struct  protoent_data {		/* should be considered opaque */
	FILE	*proto_fp;
	char	line[1024];
	char	*proto_aliases[_MAXALIASES];
	int	_proto_stayopen;
	int	svc_getprotoflag;
};
#endif	/* _REENTRANT || _THREAD_SAFE */

extern struct hostent *gethostbyname __((const char *));
extern struct hostent *gethostbyaddr __((const char *, int, int));
extern struct hostent *gethostent __((void));
extern int sethostent __((int));
extern void endhostent __((void));

extern struct netent *getnetbyname __((const char *));
extern struct netent *getnetbyaddr __((int, int));
extern struct netent *getnetent __((void));
extern int setnetent __((int));
extern void endnetent __((void));

extern struct servent *getservbyname __((const char *, const char *));
extern struct servent *getservbyport __((int, const char *));
extern struct servent *getservent __((void));
extern int setservent __((int));
extern void endservent __((void));

extern struct protoent *getprotobyname __((const char *));
extern struct protoent *getprotobynumber __((int));
extern struct protoent *getprotoent __((void));
extern int setprotoent __((int));
extern void endprotoent __((void));

#if defined(_REENTRANT) || defined(_THREAD_SAFE)

#include <arpa/nameser.h>
#include <resolv.h>

extern int gethostbyname_r __((const char *, struct hostent *, 
					struct hostent_data *));
extern int gethostbyaddr_r __((const char *, int, int, struct hostent *, \
					struct hostent_data *));
extern int gethostent_r __((struct hostent *, struct hostent_data *));
extern int sethostent_r __((int, struct hostent_data *));
extern void endhostent_r __((struct hostent_data *));

extern int getnetbyaddr_r __((long, int, struct netent *, \
					struct netent_data *));
extern int getnetbyname_r __((const char *, struct netent *, \
					struct netent_data *));
extern int getnetent_r __((struct netent *, struct netent_data *));
extern int setnetent_r __((int, struct netent_data *));
extern void endnetent_r __((struct netent_data *));

extern int getservbyname_r __((const char *, const char *, struct servent *, \
					struct servent_data *));
extern int getservbyport_r __((int, const char *, struct servent *, \
					struct servent_data *));
extern int getservent_r __((struct servent *, struct servent_data *));
extern int setservent_r __((int, struct servent_data *));
extern void endservent_r __((struct servent_data *));

extern int getprotobyname_r __((const char *, struct protoent *, \
					struct protoent_data *));
extern int getprotobynumber_r __((int, struct protoent *, \
					struct protoent_data *));
extern int getprotoent_r __((struct protoent *, struct protoent_data *));
extern int setprotoent_r __((int, struct protoent_data *));
extern void endprotoent_r __((struct protoent_data *));

/*
 * Per thread h_errno.
 */
#define h_errno   (*_h_errno())

#endif	/* _REENTRANT || _THREAD_SAFE */

/*
 * Error return codes from gethostbyname() and gethostbyaddr()
 * (left in extern int h_errno).
 */
extern	int h_errno;

#define	HOST_NOT_FOUND	1 /* Authoritative Answer Host not found */
#define	TRY_AGAIN	2 /* Non-Authoritive Host not found, or SERVERFAIL */
#define	NO_RECOVERY	3 /* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define	NO_DATA		4 /* Valid name, no data record of requested type */
#define	NO_ADDRESS	NO_DATA		/* no address, look for MX record */

#endif /* _NETDB_H_ */
