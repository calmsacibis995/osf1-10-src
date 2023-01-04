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
static  char    *sccsid = "@(#)$RCSfile: gethostent.c,v $ $Revision: 4.2.11.6 $ (DEC) $Date: 1993/10/18 20:41:32 $";
#endif lint
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
 * 15-Nov-90    lebel
 *	Fixed strfind.
 *
 * 07-Feb-90	sue
 *	Remove calls to remove_local_domain().  This does not conform
 *	to standard BIND semantics.  Change the name of the routine and
 *	function to local_hostname_length which returns the length of 
 *	the short hostname if the string contains the local domain 
 *	name.
 *
 * 13-Nov-89	sue
 *	Changed svc_gethostflag initial value to -2 and now perform a
 *	check in gethostent to see if the sethostent has been called yet.
 *
 * 18-Aug-89	sue
 *	Added support for gethostent_bind().  It depends on the BIND
 *	host database being built by /var/dss/namedb/bin/make_hosts.
 *	Removed call to remove_local_domain in interpret().  It does
 *	not make sense to modify the answer without modifying the
 *	request.  The user should get what he asks for.
 *
 * 24-Jul-89	logcher
 *	Removed generic sethostent and endhostent calls from generic
 *	gethostbyname and gethostbyaddr.  Added the specific set and end
 *	calls in the specific get routines.  Moved remove_local_domain
 *	call in interpret to after nulls have been placed.
 *
 * 25-May-89	logcher
 *	Changed name of any() to getcommon_any().
 *
 * 16-May-89	logcher
 *	Remove all of the "svc" specific code and put in separate
 *	file, svcent.c,  so other get*ent.c routines can use them.
 *	Removed static declarations on *_[local|yp|bind] calls so 
 *	that these routines can be called outside of this file.
 *
 * 09-May-89	logcher
 *	Added remove_local_domain to strip off local domain from name,
 *	if it exists.  Changed FSG debug lines to DEBUG.
 *
 * 15-Jul-88	logcher
 *	In init_svcorder, do an fclose on the file descriptor for
 *	the /etc/svcorder.
 *
 * 07-Mar-88	rglaser
 *	Merge Fred's changes into the latest version from Berkeley.
 *	Change debug printouts to go to stderr for better daemon debug.
 *
 * 24-Feb-88    fglover
 *	Support entries in /etc/svcorder file as case insensitive,
 *	but map to upper case for internal comparisons.  Define
 *	macro L_TO_U to do the mapping.
 *
 * 26-Jan-88	logcher
 *	Added a few more BIND 4.7.3 changes as well as changing
 *	some ifdefed debug messages to print on the console to be 
 *	useful if compiled in daemons (rshd).
 *
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak endhostent_r = __endhostent_r
#pragma weak gethostbyaddr_r = __gethostbyaddr_r
#pragma weak gethostbyname_r = __gethostbyname_r
#pragma weak gethostdomain_r = __gethostdomain_r
#pragma weak gethostent_r = __gethostent_r
#pragma weak sethostent_r = __sethostent_r
#else
#pragma weak endhostent = __endhostent
#pragma weak gethostbyaddr = __gethostbyaddr
#pragma weak gethostbyname = __gethostbyname
#pragma weak gethostdomain = __gethostdomain
#pragma weak gethostent = __gethostent
#pragma weak sethostent = __sethostent
#pragma weak svc_gethostflag = __svc_gethostflag
#pragma weak local_hostname_length = __local_hostname_length
#pragma weak strfind = __strfind
#endif
#undef endhostent_local
#undef gethostbyaddr_local
#undef gethostbyname_local
#undef gethostent_local
#undef sethostent_local
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

#ifdef _THREAD_SAFE
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

#define	GETANSWER(a,b,c,d,e)	getanswer_r(a,b,c,d,e)
#define	GETHOSTDOMAIN(a,b,c,d)	gethostdomain_r(a,b,c,d)
#define	SETHOSTENT(a,b)		sethostent_r(a,b)
#define	SETHOSTENT_LOCAL(a,b)	sethostent_local_r(a,b)
#define	SETHOSTENT_YP(a,b)	sethostent_yp_r(a,b)
#define	SETHOSTENT_BIND(a,b)	setent_bind_r(a,b)
#define	SETHOSTENTS(info,index,a,b)	(*(sethostents_r[info->svcpath[SVC_HOSTS][index]]))(a,b)
#define	ENDHOSTENT(a)		endhostent_r(a)
#define	ENDHOSTENT_LOCAL(a)	endhostent_local_r(a)
#define	ENDHOSTENT_YP(a)	endhostent_yp_r(a)
#define	ENDHOSTENT_BIND(a)	endent_bind_r(a)
#define	ENDHOSTENTS(info,index,a)	(*(endhostents_r[info->svcpath[SVC_HOSTS][index]]))(a)
#define	GETHOSTENT(a,b)		gethostent_r(a,b)
#define	GETHOSTENT_LOCAL(a,b)	gethostent_local_r(a,b)
#define	GETHOSTENT_YP(a,b)	gethostent_yp_r(a,b)
#define	GETHOSTENT_BIND(a,b)	gethostent_bind_r(a,b)
#define	GETHOSTENTS(info,index,a,b)	(*(gethostents_r[info->svcpath[SVC_HOSTS][index]]))(a,b)
#define	GETHOSTBYNAME(a,b,c)	gethostbyname_r(a,b,c)
#define	GETHOSTBYNAME_LOCAL(a,b,c)	gethostbyname_local_r(a,b,c)
#define	GETHOSTBYNAME_YP(a,b,c)		gethostbyname_yp_r(a,b,c)
#define	GETHOSTBYNAME_BIND(a,b,c)	gethostbyname_bind_r(a,b,c)
#define	GETHOSTBYNAMES(info,index,a,b,c)	(*(gethostnames_r[info->svcpath[SVC_HOSTS][index]]))(a,b,c)
#define	GETHOSTBYADDR(a,b,c,d,e)	gethostbyaddr_r(a,b,c,d,e)
#define	GETHOSTBYADDR_LOCAL(a,b,c,d,e)	gethostbyaddr_local_r(a,b,c,d,e)
#define	GETHOSTBYADDR_YP(a,b,c,d,e)	gethostbyaddr_yp_r(a,b,c,d,e)
#define	GETHOSTBYADDR_BIND(a,b,c,d,e)	gethostbyaddr_bind_r(a,b,c,d,e)
#define	GETHOSTBYADDRS(info,index,a,b,c,d,e)	(*(gethostaddrs_r[info->svcpath[SVC_HOSTS][index]]))(a,b,c,d,e)

#define	GETSVC(i)		getsvc_r(i)
#define	INTERPRET(a,b,c,d,e)	interpret_r(a,b,c,d,e)

#define HOST_ADDR		(ht_data->host_addr)
#define H_ADDR_PTRS		(ht_data->h_addr_ptrs)
#define	HOSTADDR		(ht_data->hostaddr)
#define	HOSTBUF			(ht_data->hostbuf)
#define	HOST_ALIASES		(ht_data->host_aliases)
#define	HOST_ADDRS		(ht_data->host_addrs)
#define	HOSTF			(ht_data->hostf)

#define	SVC_GETHOSTFLAG		(ht_data->svc_gethostflag-2)
#define	SVC_GETHOSTBIND		(ht_data->svc_gethostbind)

extern struct rec_mutex _getsvc_rmutex;
extern struct rec_mutex _nis_rmutex;

#else

#define	SETHOSTENT(a,b)		sethostent(a)
#define	SETHOSTENT_LOCAL(a,b)	sethostent_local(a)
#define	SETHOSTENT_YP(a,b)	sethostent_yp(a)
#define	SETHOSTENT_BIND(a,b)	setent_bind(a)
#define	SETHOSTENTS(info,index,a,b)	(*(sethostents[info->svcpath[SVC_HOSTS][index]]))(a)
#define	ENDHOSTENT(a)		endhostent()
#define	ENDHOSTENT_LOCAL(a)	endhostent_local()
#define	ENDHOSTENT_YP(a)	endhostent_yp()
#define	ENDHOSTENT_BIND(a)	endent_bind()
#define	ENDHOSTENTS(info,index,a)	(*(endhostents[info->svcpath[SVC_HOSTS][index]]))()
#define	GETHOSTENT(a,b)		gethostent()
#define	GETHOSTENT_LOCAL(a,b)	gethostent_local(a)
#define	GETHOSTENT_YP(a,b)	gethostent_yp(a)
#define	GETHOSTENT_BIND(a,b)	gethostent_bind(a)
#define	GETHOSTENTS(info,index,a,b)	(*(gethostents[info->svcpath[SVC_HOSTS][index]]))(a)
#define	GETHOSTBYNAME(a,b,c)	gethostbyname(a)
#define	GETHOSTBYNAME_LOCAL(a,b,c)	gethostbyname_local(a,b)
#define	GETHOSTBYNAME_YP(a,b,c)		gethostbyname_yp(a,b)
#define	GETHOSTBYNAME_BIND(a,b,c)	gethostbyname_bind(a,b)
#define	GETHOSTBYNAMES(info,index,a,b,c)	(*(gethostnames[info->svcpath[SVC_HOSTS][index]]))(a,b)
#define	GETHOSTBYADDR(a,b,c,d,e)	gethostbyaddr(a,b,c)
#define	GETHOSTBYADDR_LOCAL(a,b,c,d,e)	gethostbyaddr_local(a,b,c,d)
#define	GETHOSTBYADDR_YP(a,b,c,d,e)	gethostbyaddr_yp(a,b,c,d)
#define	GETHOSTBYADDR_BIND(a,b,c,d,e)	gethostbyaddr_bind(a,b,c,d)
#define	GETHOSTBYADDRS(info,index,a,b,c,d,e)	(*(gethostaddrs[info->svcpath[SVC_HOSTS][index]]))(a,b,c,d)

#define	GETSVC(i)		(!!(i=getsvc()))
#define	INTERPRET(a,b,c,d,e)	interpret(a,b,c,d)

#define	GETANSWER(a,b,c,d,e)	getanswer(a,b,c,d)
#define	GETHOSTDOMAIN(a,b,c,d)	gethostdomain(a,b,c)

#define	HOST_ADDR		host_addr
#define	H_ADDR_PTRS		h_addr_ptrs
#define	HOSTADDR		hostaddr
#define	HOSTBUF			hostbuf
#define	HOST_ALIASES		host_aliases
#define	HOST_ADDRS		host_addrs
#define	HOSTF			hostf

#define	SVC_GETHOSTFLAG		svc_gethostflag
#define	SVC_GETHOSTBIND		svc_gethostbind

static struct hostent	ht_host;
static struct in_addr	host_addr;
static char		*h_addr_ptrs[_MAXADDRS + 1];
static char		hostaddr[_MAXADDRS];
static char		*hostbuf;
static char		*host_aliases[_MAXALIASES];
static char		*host_addrs[2];
static FILE		*hostf = NULL;
static struct hostent _host;
static struct hostent *host = &_host;

#endif	/* _THREAD_SAFE */

#ifdef	_THREAD_SAFE
extern int svc_gethostflag;
#else
int svc_gethostflag = -2;
#endif	/* _THREAD_SAFE */
int svc_gethostbind;

/* The following variables need to be made thread-safe */
static char HOSTDB[] = "/etc/hosts";
static int stayopen = 0;
static char *current = NULL;
static int currentlen;

char *hostalias();
extern char *inet_ntoa();
char *strfind();
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
static int interpret_r(char *val, int len, int svc, struct hostent *host, struct hostent_data *ht_data)
#else
static int interpret(char *val, int len, int svc, struct hostent *host)
#endif	/* _THREAD_SAFE */
{
	char *p;
	register char *cp, **q;

#ifndef	_THREAD_SAFE
	if(!HOSTBUF && !(HOSTBUF=(char *)malloc(_HOSTBUFSIZE)))
		return TS_FAILURE;
#endif	/* _THREAD_SAFE */
	if(val != HOSTBUF)
		strncpy(HOSTBUF, val, len);
	p = HOSTBUF;
	p[len] = '\n';
	if (*p == '#')
		switch (svc) {
			case SVC_LOCAL:
				return GETHOSTENT_LOCAL(host, ht_data);
			case SVC_YP:
				return GETHOSTENT_YP(host, ht_data);
		}
	cp = getcommon_any(p, "#\n");
	if (cp == NULL)
		switch (svc) {
			case SVC_LOCAL:
				return GETHOSTENT_LOCAL(host, ht_data);
			case SVC_YP:
				return GETHOSTENT_YP(host, ht_data);
		}
	*cp = '\0';
	cp = getcommon_any(p, " \t");
	if (cp == NULL)
		switch (svc) {
			case SVC_LOCAL:
				return GETHOSTENT_LOCAL(host, ht_data);
			case SVC_YP:
				return GETHOSTENT_YP(host, ht_data);
		}
	*cp++ = '\0';
	/* THIS STUFF IS INTERNET SPECIFIC */

	/* 
	 *	The hostent structure was modified in 4.3BSD to return
	 *	a list of host addresses, with the last address in the list
	 *	NULL.  Some 4.3 utilities expect the list, so set it up, 
	 *	but provide backward compatibility for local /etc/hosts
	 *	and YP by defining host.h_addr as the first address in 
	 *	the list.
	 */

	host->h_addr_list    = HOST_ADDRS;	/* point to the list */
	host->h_addr_list[0] = HOSTADDR;		/* point to the host addrs */
	host->h_addr_list[1] = (char *)0;	/* and null terminate */
	*((u_int *)host->h_addr) = inet_addr(p);
	
	host->h_length = sizeof (u_int);
	host->h_addrtype = AF_INET;
	while (*cp == ' ' || *cp == '\t')
		cp++;
	host->h_name = cp;
	q = host->h_aliases = HOST_ALIASES;
	cp = getcommon_any(cp, " \t");
	if (cp != NULL) 
		*cp++ = '\0';
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &HOST_ALIASES[MAXALIASES - 1])
			*q++ = cp;
		cp = getcommon_any(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	return TS_SUCCESS;
}

/*
 * specific gethost service routines
 */

#ifdef	_THREAD_SAFE
static int sethostent_local_r(int f, struct hostent_data *ht_data)
#else
static int sethostent_local(int f)
#endif	/* _THREAD_SAFE */
{
	if (HOSTF == NULL)
		HOSTF = fopen(HOSTDB, "r");
	else
		rewind(HOSTF);
	stayopen |= f;
	return TS_SUCCESS;
}

/*
 * setent_bind(stayopen) in getcommon.c
 */

#ifdef	_THREAD_SAFE
static int sethostent_yp_r(int f, struct hostent_data *ht_data)
#else
static int sethostent_yp(int f)
#endif	/* _THREAD_SAFE */
{
	char *domain;
	int status;

	TS_LOCK(&_nis_rmutex);
	if ((domain = yellowup(1)) == NULL) {
		status = SETHOSTENT_LOCAL(f, ht_data);
		TS_UNLOCK(&_nis_rmutex);
		return status;
	}
	if (current)
		free(current);
	current = NULL;
	stayopen |= f;
	TS_UNLOCK(&_nis_rmutex);
	return TS_SUCCESS;
}

#ifdef  _THREAD_SAFE
static int setent_bind_r(int f, struct hostent_data *ht_data)
{
	return setent_bind(f);
}
#endif	/* _THREAD_SAFE */

#ifdef	_THREAD_SAFE
static void endhostent_local_r(struct hostent_data *ht_data)
#else
static void endhostent_local()
#endif	/* _THREAD_SAFE */
{
	if (HOSTF && !stayopen) {
		fclose(HOSTF);
		HOSTF = NULL;
	}
}

/*
 * endent_bind(stayopen) in getcommon.c
 */

#ifdef	_THREAD_SAFE
static void endhostent_yp_r(struct hostent_data *ht_data)
#else
static void endhostent_yp()
#endif	/* _THREAD_SAFE */
{
	char *domain;

	TS_LOCK(&_nis_rmutex);
	if ((domain = yellowup(0)) == NULL) {
		ENDHOSTENT_LOCAL(ht_data);
		TS_UNLOCK(&_nis_rmutex);
		return;
	}
	if (current && !stayopen) {
		free(current);
		current = NULL;
	}
	TS_UNLOCK(&_nis_rmutex);
}

#ifdef  _THREAD_SAFE
static void endent_bind_r(struct hostent_data *ht_data)
{
	endent_bind();
}
#endif	/* _THREAD_SAFE */

#ifdef	_THREAD_SAFE
static int gethostbyaddr_local_r(char *addr, int len, int type,
	 struct hostent *host, struct hostent_data *ht_data)
#else
static int gethostbyaddr_local(char *addr, int len, int type,
	 struct hostent *host)
#endif	/* _THREAD_SAFE */
{
	int status;

#ifdef DEBUG
	fprintf(stderr,  "gethostbyaddr_local(%d.%d.%d.%d, %d, %d)\n", addr[0], addr[1], addr[2], addr[3], len, type);
#endif DEBUG
	SETHOSTENT_LOCAL(0, ht_data);
	while((status=GETHOSTENT_LOCAL(host, ht_data)) != TS_FAILURE) {
		if (host->h_addrtype != type || host->h_length != len)
			continue;
		if (memcmp(host->h_addr, addr, len) == 0)
			break;
	}
	ENDHOSTENT_LOCAL(ht_data);
	if (status == TS_FAILURE)                  /* Host is not found */
		h_errno = HOST_NOT_FOUND;
	return status;
}

#ifdef	_THREAD_SAFE
static int gethostbyaddr_bind_r(char *addr, int len, int type,
	struct hostent *host, struct hostent_data *ht_data)
#else
static int gethostbyaddr_bind(char *addr, int len, int type,
	struct hostent *host)
#endif	/* _THREAD_SAFE */
{
	int n, status;
	querybuf_t buf;
	char qbuf[MAXDNAME];

#ifdef DEBUG
	fprintf(stderr, "gethostbyaddr_bind(%d.%d.%d.%d, %d, %d)\n", addr[0], addr[1], addr[2], addr[3], len, type);
#endif DEBUG
	if (type != AF_INET)
		return TS_FAILURE;
	SETHOSTENT_BIND(0, ht_data);
	(void)sprintf(qbuf, "%d.%d.%d.%d.in-addr.arpa",
		((unsigned)addr[3] & 0xff),
		((unsigned)addr[2] & 0xff),
		((unsigned)addr[1] & 0xff),
		((unsigned)addr[0] & 0xff));
	n = res_mkquery(QUERY, qbuf, C_IN, T_PTR, (char *)NULL, 0, NULL,
		(char *)&buf, sizeof(buf));
	if (n < 0) {
#ifdef DEBUG_RESOLVER
		if (_res.options & RES_DEBUG)
			printf("res_mkquery failed\n");
#endif
		ENDHOSTENT_BIND(ht_data);
		return TS_FAILURE;
	}
	status = GETANSWER((char *) &buf, n, 1, host, ht_data);
	if (status == TS_FAILURE) {
		ENDHOSTENT_BIND(ht_data);
		return TS_FAILURE;
	}
	host->h_addrtype = type;
	host->h_length = len;
	H_ADDR_PTRS[0] = (char *)&HOST_ADDR;
	H_ADDR_PTRS[1] = (char *)0;
	HOST_ADDR = *(struct in_addr *)addr;
	ENDHOSTENT_BIND(ht_data);
	return TS_SUCCESS;
}

#ifdef	_THREAD_SAFE
static int gethostbyaddr_yp_r(char *addr, int len, int type,
	struct hostent *host, struct hostent_data *ht_data)
#else
static int gethostbyaddr_yp(char *addr, int len, int type,
	struct hostent *host)
#endif	/* _THREAD_SAFE */
{
	int reason, status;
	char *domain, *val = NULL;
	int vallen = 0;
#ifdef	_THREAD_SAFE
	char adrstr[18];
#else
	char *adrstr;
#endif	/* _THREAD_SAFE */

#ifdef DEBUG
	fprintf(stderr,  "gethostbyaddr_yp(%d.%d.%d.%d, %d, %d)\n", addr[0], addr[1], addr[2], addr[3], len, type);
#endif DEBUG
	TS_LOCK(&_nis_rmutex);
	SETHOSTENT_YP(0, ht_data);
	if ((domain = yellowup(0)) == NULL) {
		status = GETHOSTBYADDR_LOCAL(addr, len, type, host, ht_data);
		TS_UNLOCK(&_nis_rmutex);
		return status;
	}

#ifdef	_THREAD_SAFE
#define UC(b)   (((int)b)&0xff)
	sprintf(adrstr, "%d.%d.%d.%d", UC(addr[0]), UC(addr[1]), UC(addr[2]), UC(addr[3]));
#else
	adrstr = inet_ntoa(*(struct in_addr *) addr);
#endif	/* _THREAD_SAFE */

	if (reason = yp_match(domain, "hosts.byaddr", adrstr, strlen(adrstr), &val, &vallen)) {
#ifdef DEBUG
		fprintf(stderr, "reason yp_first failed is %d\n", reason);
#endif
		status = TS_FAILURE;
	}
	else {
		status = INTERPRET(val, vallen, SVC_YP, host, ht_data);
		free(val);
	}
	ENDHOSTENT_YP(ht_data);
	TS_UNLOCK(&_nis_rmutex);
 	if (status == TS_FAILURE) {		/* Host is not found */
		h_errno = HOST_NOT_FOUND;
		return TS_FAILURE;
	} else
		return TS_SUCCESS;
}

#ifdef	_THREAD_SAFE
static int gethostbyname_local_r(char *name,
	struct hostent *host, struct hostent_data *ht_data)
#else
static int gethostbyname_local(char *name,
	struct hostent *host)
#endif	/* _THREAD_SAFE */
{
	register char **cp;
	int status;

#ifdef DEBUG
	fprintf(stderr,  "gethostbyname_local(%s)\n", name);
#endif DEBUG
	SETHOSTENT_LOCAL(0, ht_data);
	while((status=GETHOSTENT_LOCAL(host, ht_data)) != TS_FAILURE) {
		if (strcmp(host->h_name, name) == 0)
			break;
		for (cp = host->h_aliases; *cp != 0; cp++)
			if (strcmp(*cp, name) == 0)
				goto found;
	}
found:
	ENDHOSTENT_LOCAL(ht_data);
	if (status == TS_FAILURE)		/* Host is not found */
		h_errno = HOST_NOT_FOUND;
	return status;
}

#ifdef	_THREAD_SAFE
static int gethostbyname_bind_r(char *name,
	struct hostent *host, struct hostent_data *ht_data)
#else
static int gethostbyname_bind(char *name,
	struct hostent *host)
#endif	/* _THREAD_SAFE */
{
	register char *cp, **domain;
	int n;

#ifdef DEBUG
	fprintf(stderr,  "gethostbyname_bind(%s)\n", name);
#endif DEBUG
	if (!(_res.options & RES_INIT) && res_init() == -1)
		return TS_FAILURE;
	SETHOSTENT_BIND(0, ht_data);
	/*
	 * disallow names consisting only of digits/dots, unless
	 * they end in a dot.
	 */
	if (isdigit(name[0]))
		for (cp = name;; ++cp) {
			if (!*cp) {
				if (*--cp == '.')
					break;
				h_errno = HOST_NOT_FOUND;
				ENDHOSTENT_BIND(ht_data);
				return TS_FAILURE;
			}
			if (!isdigit(*cp) && *cp != '.') 
				break;
		}
	_Seterrno(0);
	for (cp = name, n = 0; *cp; cp++)
		if (*cp == '.')
			n++;

	if (n == 0 && (cp = hostalias(name))) {
		ENDHOSTENT_BIND(ht_data);
		if(GETHOSTDOMAIN(cp, (char *)NULL, host, ht_data) != TS_FAILURE)
			return TS_SUCCESS;
		else
			return TS_FAILURE;
	}
	if ((n == 0 || *--cp != '.') && (_res.options & RES_DEFNAMES))
	    for (domain = _res.dnsrch; *domain; domain++) {
		h_errno = 0;
		if(GETHOSTDOMAIN(name, *domain, host, ht_data) != TS_FAILURE) {
			ENDHOSTENT_BIND(ht_data);
			return TS_SUCCESS;
		}
		/*
		 * If no server present, use host table.
		 * If host isn't found in this domain,
		 * keep trying higher domains in the search list
		 * (if that's enabled).
		 * On a NO_ADDRESS error, keep trying, otherwise
		 * a wildcard MX entry could keep us from finding
		 * host entries higher in the domain.
		 * If we get some other error (non-authoritative negative
		 * answer or server failure), then stop searching up,
		 * but try the input name below in case it's fully-qualified.
		 */
		if (_Geterrno() == ECONNREFUSED) {
			ENDHOSTENT_BIND(ht_data);
			return TS_FAILURE;
		}
		if ((h_errno != HOST_NOT_FOUND && h_errno != NO_ADDRESS) ||
		    (_res.options & RES_DNSRCH) == 0)
			break;
	}
	/*
	 * If the search/default failed, try the name as fully-qualified,
	 * but only if it contained at least one dot (even trailing).
	 */
	if (n) {
		ENDHOSTENT_BIND(ht_data);
		if(GETHOSTDOMAIN(name, (char *)NULL, host, ht_data) == TS_FAILURE)
			return TS_FAILURE;
		else
			return TS_SUCCESS;
	}
	ENDHOSTENT_BIND(ht_data);
	return TS_FAILURE;
}

#ifdef	_THREAD_SAFE
static int gethostbyname_yp_r(char *name,
	struct hostent *host, struct hostent_data *ht_data)
#else
static int gethostbyname_yp(char *name,
	struct hostent *host)
#endif	/* _THREAD_SAFE */
{
	int reason, status;
	char *domain, *val = NULL;
	int vallen = 0;

#ifdef DEBUG
	fprintf(stderr,  "gethostbyname_yp(%s)\n", name);
#endif DEBUG
	TS_LOCK(&_nis_rmutex);
	SETHOSTENT_YP(0, ht_data);
	if ((domain = yellowup(0)) == NULL) {
		status = (GETHOSTBYNAME_LOCAL(name, host, ht_data));
		TS_UNLOCK(&_nis_rmutex);
		return status;
	}
	if (reason = yp_match(domain, "hosts.byname", name, strlen(name), &val, &vallen)) {
#ifdef DEBUG
		fprintf(stderr,  "reason yp_first failed is %d\n", reason);
#endif
		status = TS_FAILURE;
	}
	else {
		status = INTERPRET(val, vallen, SVC_YP, host, ht_data);
		free(val);
	}
	ENDHOSTENT_YP(ht_data);
	TS_UNLOCK(&_nis_rmutex);
 	if (status == TS_FAILURE)                  /* Host is not found */
		h_errno = HOST_NOT_FOUND;
	return status;
}

#ifdef	_THREAD_SAFE
static int gethostent_local_r(struct hostent *host, struct hostent_data *ht_data)
#else
static int gethostent_local(struct hostent *host)
#endif	/* _THREAD_SAFE */
{
#ifdef DEBUG
	fprintf(stderr, "gethostent_local\n");
#endif DEBUG
	if (HOSTF == NULL && (HOSTF = fopen(HOSTDB, "r")) == NULL)
		return TS_FAILURE;
#ifndef	_THREAD_SAFE
	if(!HOSTBUF && !(HOSTBUF=(char *)malloc(_HOSTBUFSIZE)))
		return TS_FAILURE;
#endif	/* _THREAD_SAFE */
	if (fgets(HOSTBUF, _HOSTBUFSIZE, HOSTF) == NULL)
		return TS_FAILURE;
	return INTERPRET(HOSTBUF, strlen(HOSTBUF), SVC_LOCAL, host, ht_data);
}

#ifdef	_THREAD_SAFE
static int gethostent_bind_r(struct hostent *host, struct hostent_data *ht_data)
#else
static int gethostent_bind(struct hostent *host)
#endif	/* _THREAD_SAFE */
{
	char bindbuf[64];

	sprintf(bindbuf, "hosts-%d", SVC_GETHOSTBIND);
#ifdef DEBUG
	fprintf(stderr, "gethostent_bind(%s)\n", bindbuf);
#endif DEBUG
	if(GETHOSTBYNAME_BIND(bindbuf, host, ht_data) == TS_FAILURE)
		return TS_FAILURE;
#ifdef	_THREAD_SAFE
	ht_data->svc_gethostbind++;
	svc_gethostbind = ht_data->svc_gethostbind;
#else
	svc_gethostbind++;
#endif	/* _THREAD_SAFE */
	return TS_SUCCESS;
}

#ifdef	_THREAD_SAFE
static int gethostent_yp_r(struct hostent *host, struct hostent_data *ht_data)
#else
static int gethostent_yp(struct hostent *host)
#endif	/* _THREAD_SAFE */
{
	int reason, status;
	char *key = NULL;
	char *val = NULL;
	char *domain;
	int keylen = 0;
	int vallen = 0;

#ifdef DEBUG
	fprintf(stderr, "gethostent_yp\n");
#endif DEBUG
	TS_LOCK(&_nis_rmutex);
	if ((domain = yellowup(0)) == NULL) {
		status = GETHOSTENT_LOCAL(host, ht_data);
		TS_UNLOCK(&_nis_rmutex);
		return status;
	}
	if (current == NULL) {
		if (reason =  yp_first(domain, "hosts.byaddr", &key, &keylen, &val, &vallen)) {
#ifdef DEBUG
			fprintf(stderr,  "reason yp_first failed is %d\n", reason);
#endif
			TS_UNLOCK(&_nis_rmutex);
			return TS_FAILURE;
		}
	}
	else {
		if (reason = yp_next(domain, "hosts.byaddr", current, currentlen, &key, &keylen, &val, &vallen)) {
#ifdef DEBUG
			fprintf(stderr,  "reason yp_next failed is %d\n", reason);
#endif
			TS_UNLOCK(&_nis_rmutex);
			return TS_FAILURE;
		}
	}
	if (current)
		free(current);
	current = key;
	currentlen = keylen;
	status = INTERPRET(val, vallen, SVC_YP, host, ht_data);
	free(val);
	TS_UNLOCK(&_nis_rmutex);
	return status;
}

/* 
 *	call service routines indirectly
 */

#ifdef	_THREAD_SAFE

static int	(*sethostents_r []) (int, struct hostent_data *)={
		sethostent_local_r,
		sethostent_yp_r,
		setent_bind_r
};
static void	(*endhostents_r []) (struct hostent_data *)={
		endhostent_local_r,
		endhostent_yp_r,
		endent_bind_r
};
static int	(*gethostents_r []) ()={
		gethostent_local_r,
		gethostent_yp_r,
		gethostent_bind_r
};
static int	(*gethostaddrs_r [])
	(char *, int, int, struct hostent *, struct hostent_data *)={
		gethostbyaddr_local_r,
		gethostbyaddr_yp_r,
		gethostbyaddr_bind_r
};
static int	(*gethostnames_r [])
	(char *, struct hostent *, struct hostent_data *)={
		gethostbyname_local_r,
		gethostbyname_yp_r,
		gethostbyname_bind_r
};

#else	/* _THREAD_SAFE */

static int	(*sethostents []) (int)={
		sethostent_local,
		sethostent_yp,
		setent_bind
};
static void	(*endhostents []) (void)={
		endhostent_local,
		endhostent_yp,
		endent_bind
};
static int (*gethostents [])
	(struct hostent *)={
		gethostent_local,
		gethostent_yp,
		gethostent_bind
};
static int (*gethostaddrs [])
	(char *, int, int, struct hostent *)={
		gethostbyaddr_local,
		gethostbyaddr_yp,
		gethostbyaddr_bind
};
static int (*gethostnames [])
	(char *, struct hostent *)={
		gethostbyname_local,
		gethostbyname_yp,
		gethostbyname_bind
};

#endif	/* _THREAD_SAFE */

/*
 * generic gethost service routines
 */
#ifdef	_THREAD_SAFE
int sethostent_r(int f, struct hostent_data *ht_data)
#else
int sethostent(int f)
#endif	/* _THREAD_SAFE */
{
	register i;
#ifdef	_THREAD_SAFE
	struct svcinfo si;
	struct svcinfo *svcinfo = &si;
#else
	struct svcinfo *svcinfo;
#endif	/* _THREAD_SAFE */

#ifdef	_THREAD_SAFE
	TS_EINVAL(!ht_data);
#endif	/* _THREAD_SAFE */
	svc_gethostflag = -1;
	svc_gethostbind = 0;
#ifdef	_THREAD_SAFE
	ht_data->svc_gethostflag = 1;
	ht_data->svc_gethostbind = 0;
#endif	/* _THREAD_SAFE */
	if(GETSVC(svcinfo) == TS_SUCCESS)
		for (i=0; (svc_lastlookup = svcinfo->svcpath[SVC_HOSTS][i]) != SVC_LAST; i++)
			SETHOSTENTS(svcinfo,i,f,ht_data);
	return TS_SUCCESS;
}

#ifdef	_THREAD_SAFE
void endhostent_r(struct hostent_data *ht_data)
#else
void endhostent()
#endif	/* _THREAD_SAFE */
{
	register i;
#ifdef	_THREAD_SAFE
	struct svcinfo si;
	struct svcinfo *svcinfo = &si;
#else
	struct svcinfo *svcinfo;
#endif	/* _THREAD_SAFE */

#ifdef	_THREAD_SAFE
	if(!ht_data) {
		_Seterrno(EINVAL);
		return;
	}
#endif	/* _THREAD_SAFE */
	svc_gethostflag = -1;
	svc_gethostbind = 0;
#ifdef	_THREAD_SAFE
	ht_data->svc_gethostflag = 1;
	ht_data->svc_gethostbind = 0;
#endif	/* _THREAD_SAFE */
	if(GETSVC(svcinfo) == TS_SUCCESS)
		for (i=0; (svc_lastlookup = svcinfo->svcpath[SVC_HOSTS][i]) != SVC_LAST; i++)
			ENDHOSTENTS(svcinfo,i,ht_data);
}

#ifdef	_THREAD_SAFE
int gethostent_r(struct hostent *host, struct hostent_data *ht_data)
#else
struct hostent *gethostent()
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

#ifdef	_THREAD_SAFE
	TS_EINVAL(!host || !ht_data);
#endif	/* _THREAD_SAFE */
	/*
	 * Check if sethostent was not made yet
	 */
	if (SVC_GETHOSTFLAG == -2)
		SETHOSTENT(0, ht_data);
	/*
	 * Check if this is the first time through gethostent
	 */
	if (SVC_GETHOSTFLAG == -1) {
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
		i = SVC_GETHOSTFLAG;
	}
	for (; (svc_lastlookup = svcinfo->svcpath[SVC_HOSTS][i]) != SVC_LAST; i++)
		if((status=GETHOSTENTS(svcinfo, i, host, ht_data)) != TS_FAILURE) {
			svc_gethostflag = i;
#ifdef	_THREAD_SAFE
			ht_data->svc_gethostflag = i+2;
#endif	/* _THREAD_SAFE */
			break;
		}
#ifdef	_THREAD_SAFE
	return status;
#else
	if(status != TS_FAILURE)
		return host;
	else
		return (struct hostent *) 0;
#endif	/* _THREAD_SAFE */
}

#ifdef	_THREAD_SAFE
int gethostbyname_r(char *name, struct hostent *host, struct hostent_data *ht_data)
#else
struct hostent *gethostbyname(char *name)
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
	if (name == 0) {
		_Seterrno(EINVAL);
		return TS_FAILURE;
	}
#ifdef	_THREAD_SAFE
	TS_EINVAL(!host || !ht_data);
#endif	/* _THREAD_SAFE */
	if(GETSVC(svcinfo) == TS_SUCCESS)
		for (i=0; (svc_lastlookup = svcinfo->svcpath[SVC_HOSTS][i]) != SVC_LAST; i++)
			if((status=GETHOSTBYNAMES(svcinfo,i,name,host,ht_data)) != TS_FAILURE)
				break;
#ifdef	_THREAD_SAFE
	return status;
#else
	if(status != TS_FAILURE)
		return host;
	else
		return (struct hostent *) 0;
#endif	/* _THREAD_SAFE */
}

#ifdef	_THREAD_SAFE
int gethostbyaddr_r(char *addr, int len, int type, struct hostent *host, struct hostent_data *ht_data)
#else
struct hostent *gethostbyaddr (char *addr, int len, int type)
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

#ifdef	_THREAD_SAFE
	TS_EINVAL(!addr || !host || !ht_data);
#endif	/* _THREAD_SAFE */
	if(GETSVC(svcinfo) == TS_SUCCESS)
		for (i=0; (svc_lastlookup = svcinfo->svcpath[SVC_HOSTS][i]) != SVC_LAST; i++)
			if((status=GETHOSTBYADDRS(svcinfo,i,addr,len,type,host,ht_data)) != TS_FAILURE)
				break;
#ifdef	_THREAD_SAFE
	return status;
#else
	if(status != TS_FAILURE)
		return host;
	else
		return (struct hostent *) 0;
#endif	/* _THREAD_SAFE */
}

/*  In res_query.c -- terry 5/9/91 */

/*
static char *hostalias(name)
	register char *name;
{
	register char *C1, *C2;
	FILE *fp;
	char *file, *getenv();
	char buf[BUFSIZ];
	static char abuf[MAXDNAME];

	file = getenv("HOSTALIASES");
	if (file == NULL || (fp = fopen(file, "r")) == NULL)
		return (NULL);
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
			abuf[sizeof(abuf) - 1] = *C2 = '\0';
			(void)strncpy(abuf, C1, sizeof(abuf) - 1);
			fclose(fp);
			return (abuf);
		}
	}
	fclose(fp);
	return (NULL);
}

*/

#ifdef	_THREAD_SAFE
int gethostdomain_r(char *name, char *domain, struct hostent *host,
	struct hostent_data *ht_data)
#else
int gethostdomain(char *name, char *domain,
	struct hostent *host)
#endif	/* _THREAD_SAFE */
{
	querybuf_t buf;
	char nbuf[2*MAXDNAME+2];
	char *longname = nbuf;
	int n;

	if (domain == NULL || strfind(name,domain)) {
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
	}
	else 
		(void)sprintf(nbuf, "%.*s.%.*s", MAXDNAME, name, MAXDNAME, domain);

	n = res_mkquery(QUERY, longname, C_IN, T_A, (char *)NULL, 0, NULL, (char *)&buf, sizeof(buf));
	if (n < 0) {
#ifdef DEBUG_RESOLVER
		if (_res.options & RES_DEBUG)
			printf("res_mkquery failed\n");
#endif
		return TS_FAILURE;
	}
	return GETANSWER((char *)&buf, n, 0, host, ht_data);
}

#ifdef	_THREAD_SAFE
static int getanswer_r(char *msg, int msglen, int iquery,
	struct hostent *host, struct hostent_data *ht_data)
#else
static int getanswer(char *msg, int msglen, int iquery,
	struct hostent *host)
#endif	/* _THREAD_SAFE */
{
	register HEADER *hp;
	register u_char *cp;
	register int n;
	querybuf_t answer;
	u_char *eom, *bp, **ap;
	int type, class, buflen, ancount, qdcount;
	int haveanswer, getclass = C_ANY;
	char **hap;

	n = res_send(msg, msglen, (char *)&answer, sizeof(answer));
	if (n < 0) {
#ifdef DEBUG_RESOLVER
		int terrno;
		terrno = errno;
		if (_res.options & RES_DEBUG)
			printf("res_send failed\n");
		errno = terrno;
#endif
		h_errno = TRY_AGAIN;
		return TS_FAILURE;
	}
	eom = (u_char *)&answer + n;
	/*
	 * find first satisfactory answer
	 */
	hp = (HEADER *) &answer;
	ancount = ntohs(hp->ancount);
	qdcount = ntohs(hp->qdcount);
	if (hp->rcode != NOERROR || ancount == 0) {
#ifdef DEBUG_RESOLVER
		if (_res.options & RES_DEBUG)
			printf("rcode = %d, ancount=%d\n", hp->rcode, ancount);
#endif
		switch (hp->rcode) {
			case NXDOMAIN:
				/* Check if it's an authoritive answer */
				if (hp->aa)
					h_errno = HOST_NOT_FOUND;
				else
					h_errno = TRY_AGAIN;
				break;
			case SERVFAIL:
				h_errno = TRY_AGAIN;
				break;
			case NOERROR:
				if (hp->aa)
					h_errno = NO_ADDRESS;
				else
					h_errno = TRY_AGAIN;
				break;
			case FORMERR:
			case NOTIMP:
			case REFUSED:
				h_errno = NO_RECOVERY;
		}
		return TS_FAILURE;
	}
#ifdef	_THREAD_SAFE
	buflen = sizeof(HOSTBUF);
#else
	buflen = _HOSTBUFSIZE;
	if(!HOSTBUF && !(HOSTBUF=(char *)malloc(buflen)))
		return TS_FAILURE;
#endif	/* _THREAD_SAFE */
	bp = (u_char *)HOSTBUF;
	cp = (u_char *)&answer + sizeof(HEADER);
	if (qdcount) {
		if (iquery) {
			if ((n = dn_expand((u_char *)&answer, eom,
			     cp, bp, buflen)) < 0) {
				h_errno = NO_RECOVERY;
				return TS_FAILURE;
			}
			cp += n + QFIXEDSZ;
			host->h_name = (char *) bp;
			n = strlen((char *)bp) + 1;
			bp += n;
			buflen -= n;
		} else
			cp += dn_skipname(cp, eom) + QFIXEDSZ;
		while (--qdcount > 0)
			cp += dn_skipname(cp, eom) + QFIXEDSZ;
	} else if (iquery) {
		if (hp->aa)
			h_errno = HOST_NOT_FOUND;
		else
			h_errno = TRY_AGAIN;
		return TS_FAILURE;
	}
	ap = (u_char **)HOST_ALIASES;
	host->h_aliases = HOST_ALIASES;
	hap = H_ADDR_PTRS;
#ifdef h_addr
	host->h_addr_list = H_ADDR_PTRS;
#endif
	haveanswer = 0;
	while (--ancount >= 0 && cp < eom) {
		if ((n = dn_expand((u_char *)&answer, eom, cp, bp, buflen)) < 0)
			break;
		cp += n;
		type = _getshort(cp);
 		cp += sizeof(u_short);
		class = _getshort(cp);
 		cp += sizeof(u_short) + sizeof(u_int);
		n = _getshort(cp);
		cp += sizeof(u_short);
		if (type == T_CNAME) {
			cp += n;
			if (ap >= (u_char **)&HOST_ALIASES[MAXALIASES-1])
				continue;
			*ap++ = bp;
			n = strlen((char *)bp) + 1;
			bp += n;
			buflen -= n;
			continue;
		}
		if (type == T_PTR) {
			if ((n = dn_expand((u_char *)&answer, eom,
			    cp, bp, buflen)) < 0) {
				cp += n;
				continue;
			}
			cp += n;
			host->h_name = (char *) bp;
			return TS_SUCCESS;
		}
		if (type != T_A)  {
#ifdef DEBUG_RESOLVER
			if (_res.options & RES_DEBUG)
				printf("unexpected answer type %d, size %d\n",
					type, n);
#endif
			cp += n;
			continue;
		}
		if (haveanswer) {
			if (n != host->h_length) {
				cp += n;
				continue;
			}
			if (class != getclass) {
				cp += n;
				continue;
			}
		} else {
			host->h_length = n;
			getclass = class;
			host->h_addrtype = (class == C_IN) ? AF_INET : AF_UNSPEC;
			if (!iquery) {
				host->h_name = (char *)bp;
				bp += strlen((char *)bp) + 1;
			}
		}

		bp += sizeof (align_t) - ((u_long)bp % sizeof (align_t));

#ifdef	_THREAD_SAFE
		if ((char *)bp + n >= &HOSTBUF[sizeof(HOSTBUF)]) {
#else
		if ((char *)bp + n >= &HOSTBUF[_HOSTBUFSIZE]) {
#endif	/* _THREAD_SAFE */
#ifdef DEBUG_RESOLVER
			if (_res.options & RES_DEBUG)
				printf("size (%d) too big\n", n);
#endif
			break;
		}
		bcopy(cp, *hap++ = (char *)bp, n);
		bp +=n;
		cp += n;
		haveanswer++;
	}
	if (haveanswer) {
		*ap = NULL;
#ifdef h_addr
		*hap = NULL;
#else
		host->h_addr = H_ADDR_PTRS[0];
#endif
		return TS_SUCCESS;
	} else {
		h_errno = TRY_AGAIN;
		return TS_FAILURE;
	}
}

#ifndef	_THREAD_SAFE
/*
 * If
 *	1.  There is a dot in hname, and
 *	2.  the resolver is inited, and
 *	3.  the default domain name is not null, and
 *	4.  the end of hname contains the local domain name
 * return the length of the local host name of hname.
 */
local_hostname_length(hname)
	char *hname;
{
	char *cp;
	int i,j = 0;

	if (hname == NULL)
		return(NULL);
	i = strlen(hname);
	for (cp = hname; cp && *cp != NULL && *cp != '.'; cp++, j++)
		;
	if ((j < i) && (res_init() != -1) && (_res.defdname[0] != '\0') && ((strcasecmp(cp+1, _res.defdname)) == NULL))
		return(j);
	return(NULL);
}

/* Function:
 *
 *	strfind
 *
 * Function Description:
 *
 *	Searches for  substr  at the END of text. ie.
 *	See if one string is terminated by the occurance of another string.
 *	
 * Arguments:
 *
 *	char	*text		Text to search
 *	char	*substr		String to locate
 *
 * Return values:
 *
 *	Pointer to  substr  starting position in text if found.
 *	NULL if not found.
 *
 * Side Effects:
 *
 *	
 */
char *strfind(text,substr)
		char *text, *substr;
{
	int	substrlen;
	int	textlen;

textlen = strlen(text);
substrlen = strlen(substr);

if (textlen > substrlen) 
	text = (text + textlen) - substrlen;

else
	return(NULL);

/* Search end of text for match.
 */
if (strncasecmp(text, substr, substrlen) == NULL)
        return(text);
else
        return(NULL);
}/*E strfind() */
#endif	/* !_THREAD_SAFE */
