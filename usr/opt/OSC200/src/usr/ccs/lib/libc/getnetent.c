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
static char     *sccsid = "@(#)$RCSfile: getnetent.c,v $ $Revision: 4.2.9.5 $ (DEC) $Date: 1993/10/05 21:20:50 $";
#endif
/*
 */

/****************************************************************
 *								*
 *  Licensed to Digital Equipment Corporation, Maynard, MA	*
 *		Copyright 1985 Sun Microsystems, Inc.		*
 *			All rights reserved.			*
 *								*
 ****************************************************************/

/*
 * Modification History:
 *
 * 13-Nov-89	sue
 *	Changed SVC_GETNETFLAG initial value to -2 and now perform a
 *	check in getnetent to see if the setnetent has been called yet.
 *
 * 24-Jul-89	logcher
 *	Removed generic setnetent and endnetent calls from generic
 *	getnetbyname and getnetbyaddr.  Added the specific set and end
 *	calls in the specific get routines.
 *
 * 25-May-89	logcher
 *	Changed name of any() to getcommon_any().
 *
 * 16-May-89	logcher
 *      Modularized the code to have separate local, yp, bind/hesiod
 *      routines.  Added front end to check the /etc/svc.conf file
 *      for the service ordering.
 *
 * 19-Jan-88	logcher
 *	Changed getnetbyaddr() to be called a second time for
 *	network numbers ending in zero (meaning current subnet),
 *	and simplified nettoa() to strip off all trailing ".0"s.
 *
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak endnetent_r = __endnetent_r
#pragma weak getnetbyaddr_r = __getnetbyaddr_r
#pragma weak getnetbyname_r = __getnetbyname_r
#pragma weak getnetent_r = __getnetent_r
#pragma weak setnetent_r = __setnetent_r
#endif
#if !defined(_THREAD_SAFE)
#pragma weak endnetent = __endnetent
#pragma weak getnetbyaddr = __getnetbyaddr
#pragma weak getnetbyname = __getnetbyname
#pragma weak getnetent = __getnetent
#pragma weak setnetent = __setnetent
#pragma weak svc_getnetflag = __svc_getnetflag
#endif
#endif
#ifdef _NAME_SPACE_WEAK_STRONG
#ifdef endnetent_local
#undef endnetent_local
#endif
#ifdef getnetbyaddr_local
#undef getnetbyaddr_local
#endif
#ifdef getnetbyname_local
#undef getnetbyname_local
#endif
#ifdef getnetent_local
#undef getnetent_local
#endif
#ifdef setnetent_local
#undef setnetent_local
#endif
#endif

#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <netdb.h>
#include <errno.h>
#include <rpcsvc/ypclnt.h>
#include <sys/stat.h>
#include "ts_supp.h"

#ifdef	_THREAD_SAFE
#include "rec_mutex.h"
#endif	/* _THREAD_SAFE */

#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <sys/svcinfo.h>

#if defined(lint) && !defined(DEBUG)
#define DEBUG
#endif

#if defined(lint) && !defined(DEBUG_RESOLVER)
#define DEBUG_RESOLVER
#endif

/*
 * Internet version.
 */

#define	MAXALIASES	35
#define MAXADDRS	35
#define	MAXADDRSIZE	14

#ifdef _THREAD_SAFE

#define	SETNETENT(a,b)		setnetent_r(a,b)
#define	SETNETENT_LOCAL(a,b)	setnetent_local_r(a,b)
#define	SETNETENT_YP(a,b)	setnetent_yp_r(a,b)
#define	SETNETENT_BIND(a,b)	setent_bind_r(a,b)
#define	SETNETENTS(info,index,a,b)	(*(setnetents_r[info->svcpath[SVC_NETWORKS][index]]))(a,b)
#define	ENDNETENT(a)		endnetent_r(a)
#define	ENDNETENT_LOCAL(a)	endnetent_local_r(a)
#define	ENDNETENT_YP(a)	endnetent_yp_r(a)
#define	ENDNETENT_BIND(a)	endent_bind_r(a)
#define	ENDNETENTS(info,index,a)	(*(endnetents_r[info->svcpath[SVC_NETWORKS][index]]))(a)
#define	GETNETENT(a,b)		getnetent_r(a,b)
#define	GETNETENT_LOCAL(a,b)	getnetent_local_r(a,b)
#define	GETNETENT_YP(a,b)	getnetent_yp_r(a,b)
#define	GETNETENT_BIND(a,b)	getnetent_bind_r(a,b)
#define	GETNETENTS(info,index,a,b)	(*(getnetents_r[info->svcpath[SVC_NETWORKS][index]]))(a,b)
#define	GETNETBYNAME(a,b,c)	getnetbyname_r(a,b,c)
#define	GETNETBYNAME_LOCAL(a,b,c)	getnetbyname_local_r(a,b,c)
#define	GETNETBYNAME_YP(a,b,c)		getnetbyname_yp_r(a,b,c)
#define	GETNETBYNAME_BIND(a,b,c)	getnetbyname_bind_r(a,b,c)
#define	GETNETBYNAMES(info,index,a,b,c)	(*(getnetnames_r[info->svcpath[SVC_NETWORKS][index]]))(a,b,c)
#define	GETNETBYADDR(a,b,c,d)	getnetbyaddr_r(a,b,c,d)
#define	GETNETBYADDR_LOCAL(a,b,c,d)	getnetbyaddr_local_r(a,b,c,d)
#define	GETNETBYADDR_YP(a,b,c,d)	getnetbyaddr_yp_r(a,b,c,d)
#define	GETNETBYADDR_BIND(a,b,c,d)	getnetbyaddr_bind_r(a,b,c,d)
#define	GETNETBYADDRS(info,index,a,b,c,d)	(*(getnetaddrs_r[info->svcpath[SVC_NETWORKS][index]]))(a,b,c,d)

#define	GETSVC(i)		getsvc_r(i)
#define	INTERPRET(a,b,c,d,e)	interpret_r(a,b,c,d,e)
#define	NETF			nt_data->net_fp
#define	NETBUF			nt_data->line
#define	NET_ALIASES		nt_data->net_aliases
#define	STAYOPEN		nt_data->_net_stayopen

#define	SVC_GETNETFLAG		(nt_data->svc_getnetflag-2)
#define	SVC_GETNETBIND		nt_data->svc_getnetbind

extern struct rec_mutex _nis_rmutex;

#else

#define	SETNETENT(a,b)		setnetent(a)
#define	SETNETENT_LOCAL(a,b)	setnetent_local(a)
#define	SETNETENT_YP(a,b)	setnetent_yp(a)
#define	SETNETENT_BIND(a,b)	setent_bind(a)
#define	SETNETENTS(info,index,a,b)	(*(setnetents[info->svcpath[SVC_NETWORKS][index]]))(a)
#define	ENDNETENT(a)		endnetent()
#define	ENDNETENT_LOCAL(a)	endnetent_local()
#define	ENDNETENT_YP(a)	endnetent_yp()
#define	ENDNETENT_BIND(a)	endent_bind()
#define	ENDNETENTS(info,index,a)	(*(endnetents[info->svcpath[SVC_NETWORKS][index]]))()
#define	GETNETENT(a,b)		getnetent()
#define	GETNETENT_LOCAL(a,b)	getnetent_local(a)
#define	GETNETENT_YP(a,b)	getnetent_yp(a)
#define	GETNETENT_BIND(a,b)	getnetent_bind(a)
#define	GETNETENTS(info,index,a,b)	(*(getnetents[info->svcpath[SVC_NETWORKS][index]]))(a)
#define	GETNETBYNAME(a,b,c)	getnetbyname(a)
#define	GETNETBYNAME_LOCAL(a,b,c)	getnetbyname_local(a,b)
#define	GETNETBYNAME_YP(a,b,c)		getnetbyname_yp(a,b)
#define	GETNETBYNAME_BIND(a,b,c)	getnetbyname_bind(a,b)
#define	GETNETBYNAMES(info,index,a,b,c)	(*(getnetnames[info->svcpath[SVC_NETWORKS][index]]))(a,b)
#define	GETNETBYADDR(a,b,c,d)	getnetbyaddr(a,b,c)
#define	GETNETBYADDR_LOCAL(a,b,c,d)	getnetbyaddr_local(a,b,c)
#define	GETNETBYADDR_YP(a,b,c,d)	getnetbyaddr_yp(a,b,c)
#define	GETNETBYADDR_BIND(a,b,c,d)	getnetbyaddr_bind(a,b,c)
#define	GETNETBYADDRS(info,index,a,b,c,d)	(*(getnetaddrs[info->svcpath[SVC_NETWORKS][index]]))(a,b,c)

#define	GETSVC(i)		(!!(i=getsvc()))
#define	INTERPRET(a,b,c,d,e)	interpret(a,b,c,d)

#define	NETF			netf
#define	NETBUF			netbuf
#define	NET_ALIASES		net_aliases
#define	STAYOPEN		stayopen
#define	SVC_GETNETFLAG		svc_getnetflag
#define	SVC_GETNETBIND		svc_getnetbind

static FILE *netf = NULL;
static char *netbuf = NULL;
static int stayopen = 0;
static char *net_aliases[_MAXALIASES];

#endif	/* _THREAD_SAFE */

#ifdef	_THREAD_SAFE
extern int svc_getnetflag;
#else
int svc_getnetflag = -2;
#endif	/* _THREAD_SAFE */
int svc_getnetbind;

/* The following variables need to be made thread-safe */
static char NETDB[] = "/etc/networks";
static char *current = NULL;
static int currentlen;

#ifndef	_THREAD_SAFE
static struct netent _net;
static struct netent *net = &_net;
#endif	/* _THREAD_SAFE */

extern char *inet_ntoa();
static char *strfind();
extern char *getcommon_any();
extern char *yellowup();
extern void endent_bind();
extern int setent_bind();

#if PACKETSZ > 1024
#define MAXPACKET       PACKETSZ
#else
#define MAXPACKET       1024
#endif

typedef union {
    HEADER qb1;
    char qb2[MAXPACKET];
} querybuf_t;

typedef union {
    long al;
    char ac;
} align_t;

/*
 * Internal functions
 */

#ifdef	_THREAD_SAFE
static void inet_ntoa_r(struct in_addr in, char *buf)
{
	register char *p;

	p = (char *)&in;
#define UC(b)   (((int)b)&0xff)
	sprintf(buf, "%d.%d.%d.%d", UC(p[0]), UC(p[1]), UC(p[2]), UC(p[3]));
}
#endif	/* _THREAD_SAFE */

/*
 * Strip off any trailing ".0" after inet_makeaddr
 */

static void nettoa(int net, char *buf)
{
	char *p, *rindex();
	struct in_addr in;

	in = inet_makeaddr(net, INADDR_ANY);
#ifdef	_THREAD_SAFE
	inet_ntoa_r(in, buf);
#else
	strcpy(buf, inet_ntoa(in));
#endif	/* _THREAD_SAFE */
	while (p = rindex(buf, '.'))
		if (!strcmp(p+1, "0"))
			*p = '\0';
		else
			break;
}

#ifdef BINDOK
static void netcommon(char **pp, struct netent *net)
{
#ifndef	_THREAD_SAFE
	static char *net_aliases[_MAXALIASES];
#endif	/* _THREAD_SAFE */
        register char *p, **q;

        if (pp == NULL)
                return(NULL);
        strcpy(buf, pp[0]);
        while(*pp)
		free(*pp++); /* necessary to avoid leaks */
        p = buf;
        net->n_name = p;
        p = netskip(p);
        net->n_net = inet_network(p);
        net->n_addrtype = AF_INET;
	q = net->n_aliases = NET_ALIASES;
	while (*p && (p = netskip(p)) != 0) {
		if (q < &NET_ALIASES[_MAXALIASES - 1])
			*q++ = p;
	}
        while (*p && *p != '\n')
                p++;
        *p = '\0';
}

static char *netskip(char *p)
{
        while (*p && *p != ':' && *p != '\n')
                ++p;
        if (*p)
                *p++ = 0;
        return(p);
}
#endif

#ifdef	_THREAD_SAFE
static int interpret_r(char *val, int len, int svc, struct netent *net, struct netent_data *nt_data)
#else
static int interpret(char *val, int len, int svc, struct netent *net)
#endif	/* _THREAD_SAFE */
{
	char *p;
	register char *cp, **q;

#ifndef	_THREAD_SAFE
	if(!NETBUF)
		if(!(NETBUF=(char *)malloc(_MAXLINELEN)))
			return TS_FAILURE;
#endif	/* !_THREAD_SAFE */
	if(val != NETBUF)
		strncpy(NETBUF, val, len);
	p = NETBUF;
	p[len] = '\n';
	if (*p == '#')
		switch (svc) {
			case SVC_LOCAL:
				return (GETNETENT_LOCAL(net, nt_data));
			case SVC_YP:
				return (GETNETENT_YP(net, nt_data));
		}
	cp = getcommon_any(p, "#\n");
	if (cp == NULL)
		switch (svc) {
			case SVC_LOCAL:
				return (GETNETENT_LOCAL(net, nt_data));
			case SVC_YP:
				return (GETNETENT_YP(net, nt_data));
		}
	*cp = '\0';
	net->n_name = p;
	cp = getcommon_any(p, " \t");
	if (cp == NULL)
		switch (svc) {
			case SVC_LOCAL:
				return (GETNETENT_LOCAL(net, nt_data));
			case SVC_YP:
				return (GETNETENT_YP(net, nt_data));
		}
	*cp++ = '\0';
	while (*cp == ' ' || *cp == '\t')
		cp++;
	p = getcommon_any(cp, " \t");
	if (p != NULL)
		*p++ = '\0';
	net->n_net = inet_network(cp);
	net->n_addrtype = AF_INET;
	q = net->n_aliases = NET_ALIASES;
	if (p != NULL) 
		cp = p;
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &NET_ALIASES[_MAXALIASES - 1])
			*q++ = cp;
		cp = getcommon_any(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	return TS_SUCCESS;
}

/*
 * specific getnet service routines
 */

#ifdef	_THREAD_SAFE
static int setnetent_local_r(int f, struct netent_data *nt_data)
#else
static int setnetent_local(int f)
#endif	/* _THREAD_SAFE */
{
	if (NETF == NULL)
		NETF = fopen(NETDB, "r");
	else
		rewind(NETF);
	STAYOPEN |= f;
	return TS_SUCCESS;
}

/*
 * setent_bind(stayopen) in getcommon.c
 */

#ifdef	_THREAD_SAFE
static int setnetent_yp_r(int f, struct netent_data *nt_data)
#else
static int setnetent_yp(int f)
#endif	/* _THREAD_SAFE */
{
	char *domain;
	int status;

	TS_LOCK(&_nis_rmutex);
	if ((domain = yellowup(1)) == NULL) {
		status = SETNETENT_LOCAL(f, nt_data);
		TS_UNLOCK(&_nis_rmutex);
		return status;
	}
	if (current)
		free(current);
	current = NULL;
	STAYOPEN |= f;
	TS_UNLOCK(&_nis_rmutex);
	return TS_SUCCESS;
}

#ifdef  _THREAD_SAFE
static int setent_bind_r(int f, struct netent_data *nt_data)
{
	return setent_bind(f);
}
#endif	/* _THREAD_SAFE */

#ifdef	_THREAD_SAFE
static void endnetent_local_r(struct netent_data *nt_data)
#else
static void endnetent_local()
#endif	/* _THREAD_SAFE */
{
	if (NETF && !STAYOPEN) {
		fclose(NETF);
		NETF = NULL;
	}
}

/*
 * endent_bind(stayopen) in getcommon.c
 */

#ifdef	_THREAD_SAFE
static void endnetent_yp_r(struct netent_data *nt_data)
#else
static void endnetent_yp()
#endif	/* _THREAD_SAFE */
{
	char *domain;

	TS_LOCK(&_nis_rmutex);
	if ((domain = yellowup(0)) == NULL) {
		ENDNETENT_LOCAL(nt_data);
		TS_UNLOCK(&_nis_rmutex);
		return;
	}
	if (current && !STAYOPEN) {
		free(current);
		current = NULL;
	}
	TS_UNLOCK(&_nis_rmutex);
}

#ifdef  _THREAD_SAFE
static void endent_bind_r(struct netent_data *nt_data)
{
	endent_bind();
}
#endif	/* _THREAD_SAFE */

#ifdef	_THREAD_SAFE
static int getnetbyaddr_local_r(int addr, int type,
	 struct netent *net, struct netent_data *nt_data)
#else
static int getnetbyaddr_local(int addr, int type,
	 struct netent *net)
#endif	/* _THREAD_SAFE */
{
	int status;

#ifdef DEBUG
	fprintf(stderr, "getnetbyaddr_bind(%d, %d)\n", addr, type);
#endif DEBUG
	SETNETENT_LOCAL(0, nt_data);
	while((status=GETNETENT_LOCAL(net, nt_data)) != TS_FAILURE) {
		if (net->n_addrtype == type && net->n_net == addr)
			break;
	}
	ENDNETENT_LOCAL(nt_data);
	return status;
}

#ifdef	_THREAD_SAFE
static int getnetbyaddr_bind_r(int addr, int type,
	struct netent *net, struct netent_data *nt_data)
#else
static int getnetbyaddr_bind(int addr, int type,
	struct netent *net)
#endif	/* _THREAD_SAFE */
{
#ifdef	BINDOK
	register char **pp;
	char adrstr[18];
	int status = TS_FAILURE;

	if (type != AF_INET)
		return TS_FAILURE;
	SETNETENT_BIND(0, nt_data);
	nettoa(addr, adrstr);
#ifdef DEBUG
	fprintf(stderr, "getnetbyaddr_bind(%s, %d)\n", adrstr, type);
#endif	/* DEBUG */
	TS_LOCK(&_hesiod_rmutex);
	pp = hes_resolve(adrstr, "networks");
	endent_bind();
	if(pp)
		if((status=netcommon(pp, net)) != TS_FAILURE)
			memcpy(net, np, sizeof (struct netent));
	TS_UNLOCK(&_hesiod_rmutex);
	return status;
#else
	return TS_FAILURE;
#endif	/* BINDOK */
}

#ifdef	_THREAD_SAFE
static int getnetbyaddr_yp_r(int addr, int type,
	struct netent *net, struct netent_data *nt_data)
#else
static int getnetbyaddr_yp(int addr, int type,
	struct netent *net)
#endif	/* _THREAD_SAFE */
{
	int reason, status = TS_FAILURE;
	char *val = NULL;
	int vallen = 0;
	int found = 0;
	int first = 0;
	char *domain;
	char adrstr[18];

	TS_LOCK(&_nis_rmutex);
	SETNETENT_YP(0, nt_data);
	if ((domain = yellowup(0)) == NULL) {
		status = GETNETBYADDR_LOCAL(addr, type, net, nt_data);
		TS_UNLOCK(&_nis_rmutex);
		return status;
	}
	nettoa(addr, adrstr);
#ifdef DEBUG
	fprintf(stderr, "getnetbyaddr_yp(%s, %d)\n", adrstr, type);
#endif DEBUG

	while(!found) {
		if (reason = yp_match(domain, "networks.byaddr", adrstr, strlen(adrstr), &val, &vallen)) {
#ifdef DEBUG
			fprintf(stderr, "reason yp_first failed is %d\n", reason);
#endif	/* DEBUG */
			status = TS_FAILURE;
			if (!first) {
				strcat(adrstr, ".0\0");
				first = 1;
			} else
				found = 1;
		} else {
			status = INTERPRET(val, vallen, SVC_YP, net, nt_data);
			free(val);
			found = 1;
		}
	}
	ENDNETENT_YP(nt_data);
	TS_UNLOCK(&_nis_rmutex);
	return status;
}

#ifdef	_THREAD_SAFE
static int getnetbyname_local_r(char *name,
	struct netent *net, struct netent_data *nt_data)
#else
static int getnetbyname_local(char *name,
	struct netent *net)
#endif	/* _THREAD_SAFE */
{
	register char **cp;
	int status;

#ifdef DEBUG
	fprintf(stderr,  "getnetbyname_local(%s)\n", name);
#endif DEBUG
	TS_LOCK(&_nis_rmutex);
	SETNETENT_LOCAL(0, nt_data);
	while((status=GETNETENT_LOCAL(net, nt_data)) != TS_FAILURE) {
		if (strcmp(net->n_name, name) == 0)
			break;
		for (cp = net->n_aliases; *cp != 0; cp++)
			if (strcmp(*cp, name) == 0)
				goto found;
	}
found:
	ENDNETENT_LOCAL(nt_data);
	TS_UNLOCK(&_nis_rmutex);
	return status;
}

#ifdef	_THREAD_SAFE
static int getnetbyname_bind_r(char *name,
	struct netent *net, struct netent_data *nt_data)
#else
static int getnetbyname_bind(char *name,
	struct netent *net)
#endif	/* _THREAD_SAFE */
{
#ifdef	BINDOK
	register char *pp;
	int status = TS_FAILURE;

#ifdef DEBUG
	fprintf(stderr,  "getnetbyname_bind(%s)\n", name);
#endif /* DEBUG */
	SETNETENT_BIND(0, nt_data);
	TS_LOCK(&_hesiod_rmutex);
	pp = hes_resolve(name, "networks");
	ENDNETENT_BIND(nt_data);
	if(pp)
		if((status=netcommon(pp, net)) != TS_FAILURE)
			memcpy(net, np, sizeof (struct netent));
	TS_UNLOCK(&_hesiod_rmutex);
	return status;
#else
	return TS_FAILURE;
#endif	/* BINDOK */
}

#ifdef	_THREAD_SAFE
static int getnetbyname_yp_r(char *name,
	struct netent *net, struct netent_data *nt_data)
#else
static int getnetbyname_yp(char *name,
	struct netent *net)
#endif	/* _THREAD_SAFE */
{
	int reason, status;
	char *domain, *val = NULL;
	int vallen = 0;

#ifdef DEBUG
	fprintf(stderr,  "getnetbyname_yp(%s)\n", name);
#endif DEBUG
	TS_LOCK(&_nis_rmutex);
	SETNETENT_YP(0, nt_data);
	if ((domain = yellowup(0)) == NULL) {
		status = (GETNETBYNAME_LOCAL(name, net, nt_data));
		TS_UNLOCK(&_nis_rmutex);
		return status;
	}
	if (reason = yp_match(domain, "networks.byname", name, strlen(name), &val, &vallen)) {
#ifdef DEBUG
		fprintf(stderr,  "reason yp_first failed is %d\n", reason);
#endif	/* DEBUG */
		status = TS_FAILURE;
	} else {
		status = INTERPRET(val, vallen, SVC_YP, net, nt_data);
		free(val);
	}
	ENDNETENT_YP(nt_data);
	TS_UNLOCK(&_nis_rmutex);
	return status;
}

#ifdef	_THREAD_SAFE
static int getnetent_local_r(struct netent *net, struct netent_data *nt_data)
#else
static int getnetent_local(struct netent *net)
#endif	/* _THREAD_SAFE */
{
#ifdef DEBUG
	fprintf(stderr, "getnetent_local\n");
#endif DEBUG
	if (NETF == NULL && (NETF = fopen(NETDB, "r")) == NULL)
		return TS_FAILURE;
#ifndef	_THREAD_SAFE
	if(!NETBUF)
		if(!(NETBUF=(char *)malloc(_MAXLINELEN)))
			return TS_FAILURE;
#endif	/* !_THREAD_SAFE */
	if (fgets(NETBUF, _MAXLINELEN, NETF) == NULL)
		return TS_FAILURE;
	return INTERPRET(NETBUF, strlen(NETBUF), SVC_LOCAL, net, nt_data);
}

#ifdef	_THREAD_SAFE
static int getnetent_bind_r(struct netent *net, struct netent_data *nt_data)
#else
static int getnetent_bind(struct netent *net)
#endif	/* _THREAD_SAFE */
{
#ifdef	BINDOK
	char bindbuf[64];

	sprintf(bindbuf, "networks-%d", SVC_GETNETBIND);
#ifdef DEBUG
	fprintf(stderr, "getnetent_bind(%s)\n", bindbuf);
#endif /* DEBUG */
	if(GETNETBYNAME_BIND(bindbuf, net, nt_data) == TS_FAILURE)
		return TS_FAILURE;
#ifdef	_THREAD_SAFE
	nt_data->svc_getnetbind++;
	svc_getnetbind = nt_data->svc_getnetbind;
#else
	svc_getnetbind++;
#endif	/* _THREAD_SAFE */
	return TS_SUCCESS;
#else
	return TS_FAILURE;
#endif	/* BINDOK */
}

#ifdef	_THREAD_SAFE
static int getnetent_yp_r(struct netent *net, struct netent_data *nt_data)
#else
static int getnetent_yp(struct netent *net)
#endif	/* _THREAD_SAFE */
{
	int reason, status;
	char *key = NULL;
	char *val = NULL;
	char *domain;
	int keylen = 0;
	int vallen = 0;

#ifdef DEBUG
	fprintf(stderr, "getnetent_yp\n");
#endif /* DEBUG */
	TS_LOCK(&_nis_rmutex);
	if ((domain = yellowup(0)) == NULL) {
		status = GETNETENT_LOCAL(net, nt_data);
		TS_UNLOCK(&_nis_rmutex);
		return status;
	}
	if (current == NULL) {
		if (reason =  yp_first(domain, "networks.byaddr", &key, &keylen, &val, &vallen)) {
#ifdef DEBUG
			fprintf(stderr,  "reason yp_first failed is %d\n", reason);
#endif	/* DEBUG */
			TS_UNLOCK(&_nis_rmutex);
			return TS_FAILURE;
		}
	} else {
		if (reason = yp_next(domain, "networks.byaddr", current, currentlen, &key, &keylen, &val, &vallen)) {
#ifdef DEBUG
			fprintf(stderr,  "reason yp_next failed is %d\n", reason);
#endif	/* DEBUG */
			TS_UNLOCK(&_nis_rmutex);
			return TS_FAILURE;
		}
	}
	if (current)
		free(current);
	current = key;
	currentlen = keylen;
	status = INTERPRET(val, vallen, SVC_YP, net, nt_data);
	free(val);
	TS_UNLOCK(&_nis_rmutex);
	return status;
}

/* 
 *	call service routines indirectly
 */

#ifdef	_THREAD_SAFE

static int	(*setnetents_r []) (int, struct netent_data *)={
		setnetent_local_r,
		setnetent_yp_r,
		setent_bind_r
};
static void	(*endnetents_r []) (struct netent_data *)={
		endnetent_local_r,
		endnetent_yp_r,
		endent_bind_r
};
static int	(*getnetents_r []) ()={
		getnetent_local_r,
		getnetent_yp_r,
		getnetent_bind_r
};
static int	(*getnetaddrs_r [])
	(int, int, struct netent *, struct netent_data *)={
		getnetbyaddr_local_r,
		getnetbyaddr_yp_r,
		getnetbyaddr_bind_r
};
static int	(*getnetnames_r [])
	(char *, struct netent *, struct netent_data *)={
		getnetbyname_local_r,
		getnetbyname_yp_r,
		getnetbyname_bind_r
};

#else	/* _THREAD_SAFE */

static int	(*setnetents []) (int)={
		setnetent_local,
		setnetent_yp,
		setent_bind
};
static void	(*endnetents []) (void)={
		endnetent_local,
		endnetent_yp,
		endent_bind
};
static int (*getnetents [])
	(struct netent *)={
		getnetent_local,
		getnetent_yp,
		getnetent_bind
};
static int (*getnetaddrs [])
	(int, int, struct netent *)={
		getnetbyaddr_local,
		getnetbyaddr_yp,
		getnetbyaddr_bind
};
static int (*getnetnames [])
	(char *, struct netent *)={
		getnetbyname_local,
		getnetbyname_yp,
		getnetbyname_bind
};

#endif	/* _THREAD_SAFE */

/*
 * generic getnet service routines
 */
#ifdef	_THREAD_SAFE
int setnetent_r(int f, struct netent_data *nt_data)
#else
int setnetent(int f)
#endif	/* _THREAD_SAFE */
{
	register i;
#ifdef	_THREAD_SAFE
	struct svcinfo si;
	struct svcinfo *svcinfo = &si;
#else
	struct svcinfo *svcinfo;
#endif	/* _THREAD_SAFE */

	svc_getnetflag = -1;
	svc_getnetbind = 0;
#ifdef	_THREAD_SAFE
	nt_data->svc_getnetflag = 1;
#ifdef	BINDOK
	nt_data->svc_getnetbind = 0;
#endif	/* BINDOK */
#endif	/* _THREAD_SAFE */
	if(GETSVC(svcinfo) == TS_SUCCESS)
		for (i=0; (svc_lastlookup = svcinfo->svcpath[SVC_NETWORKS][i]) != SVC_LAST; i++)
			SETNETENTS(svcinfo,i,f,nt_data);
	return TS_SUCCESS;
}

#ifdef	_THREAD_SAFE
void endnetent_r(struct netent_data *nt_data)
#else
void endnetent()
#endif	/* _THREAD_SAFE */
{
	register i;
#ifdef	_THREAD_SAFE
	struct svcinfo si;
	struct svcinfo *svcinfo = &si;
#else
	struct svcinfo *svcinfo;
#endif	/* _THREAD_SAFE */

	svc_getnetflag = -1;
	svc_getnetbind = 0;
#ifdef	_THREAD_SAFE
	nt_data->svc_getnetflag = 1;
#ifdef	BINDOK
	nt_data->svc_getnetbind = 0;
#endif	/* BINDOK */
#endif	/* _THREAD_SAFE */
	if(GETSVC(svcinfo) == TS_SUCCESS)
		for (i=0; (svc_lastlookup = svcinfo->svcpath[SVC_NETWORKS][i]) != SVC_LAST; i++)
			ENDNETENTS(svcinfo,i,nt_data);
}

#ifdef	_THREAD_SAFE
int getnetent_r(struct netent *net, struct netent_data *nt_data)
#else
struct netent *getnetent()
#endif	/* _THREAD_SAFE */
{
	register i;
	int status;
#ifdef	_THREAD_SAFE
	struct svcinfo si;
	struct svcinfo *svcinfo = &si;
#else
	struct svcinfo *svcinfo;
#endif	/* _THREAD_SAFE */

	/*
	 * Check if setnetent was not made yet
	 */
	if (SVC_GETNETFLAG == -2)
		SETNETENT(0, nt_data);
	/*
	 * Check if this is the first time through getnetent
	 */
	if (SVC_GETNETFLAG == -1) {
		/*
		 * If it is, init the svcinfo structure
		 */
		if(GETSVC(svcinfo) != TS_SUCCESS)
			return TS_FAILURE;
		i = 0;
	}
	else {
		/*
		 * If it is not, set the index to the last used one
		 */
		i = SVC_GETNETFLAG;
	}
	for (; (svc_lastlookup = svcinfo->svcpath[SVC_NETWORKS][i]) != SVC_LAST; i++)
		if((status=GETNETENTS(svcinfo, i, net, nt_data)) != TS_FAILURE) {
			svc_getnetflag = i;
#ifdef	_THREAD_SAFE
			nt_data->svc_getnetflag = i+2;
#endif	/* _THREAD_SAFE */
			break;
		}
#ifdef	_THREAD_SAFE
	return status;
#else
	if(status != TS_FAILURE)
		return net;
	else
		return (struct netent *) 0;
#endif	/* _THREAD_SAFE */
}

#ifdef	_THREAD_SAFE
int getnetbyname_r(char *name, struct netent *net, struct netent_data *nt_data)
#else
struct netent *getnetbyname(char *name)
#endif	/* _THREAD_SAFE */
{
	register i;
	int status;
#ifdef	_THREAD_SAFE
	struct svcinfo si;
	struct svcinfo *svcinfo = &si;
#else
	struct svcinfo *svcinfo;
#endif	/* _THREAD_SAFE */

	/* avoid null pointer de-reference on mips */
	if (name == 0)
		return TS_FAILURE;
	if(GETSVC(svcinfo) == TS_SUCCESS)
		for (i=0; (svc_lastlookup = svcinfo->svcpath[SVC_NETWORKS][i]) != SVC_LAST; i++)
			if((status=GETNETBYNAMES(svcinfo,i,name,net,nt_data)) != TS_FAILURE)
				break;
#ifdef	_THREAD_SAFE
	return status;
#else
	if(status != TS_FAILURE)
		return net;
	else
		return (struct netent *) 0;
#endif	/* _THREAD_SAFE */
}

#ifdef	_THREAD_SAFE
int getnetbyaddr_r(int addr, int type, struct netent *net, struct netent_data *nt_data)
#else
struct netent *getnetbyaddr (int addr, int type)
#endif	/* _THREAD_SAFE */
{
	register i;
	int status;
#ifdef	_THREAD_SAFE
	struct svcinfo si;
	struct svcinfo *svcinfo = &si;
#else
	struct svcinfo *svcinfo;
#endif	/* _THREAD_SAFE */

	if(GETSVC(svcinfo) == TS_SUCCESS)
		for (i=0; (svc_lastlookup = svcinfo->svcpath[SVC_NETWORKS][i]) != SVC_LAST; i++)
			if((status=GETNETBYADDRS(svcinfo,i,addr,type,net,nt_data)) != TS_FAILURE)
				break;
#ifdef	_THREAD_SAFE
	return status;
#else
	if(status != TS_FAILURE)
		return net;
	else
		return (struct netent *) 0;
#endif	/* _THREAD_SAFE */
}
