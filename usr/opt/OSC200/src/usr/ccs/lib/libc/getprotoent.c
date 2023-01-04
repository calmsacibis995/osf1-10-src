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
static char     *sccsid = "@(#)$RCSfile: getprotoent.c,v $ $Revision: 4.2.7.7 $ (DEC) $Date: 1993/10/18 21:04:19 $";
#endif
/*
 */

/************************************************************************
 *									*
 *			Copyright (c) 1984-1989 by			*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
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
 *	Changed svc_getprotoflag initial value to -2 and now perform a
 *	check in getprotoent to see if the setprotoent has been called
 *	yet.
 *
 * 24-Jul-89	logcher
 *	Removed generic setprotoent and endprotoent calls from generic
 *	getprotobyname and getprotobynumber.  Added the specific set
 *	and end calls in the specific get routines.
 *
 * 25-May-89	logcher
 *	Changed name of any() to getcommon_any().
 *
 * 16-May-89	logcher
 *      Modularized the code to have separate local, yp, bind/hesiod
 *      routines.  Added front end to check the /etc/svc.conf file
 *      for the service ordering.
 *
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak endprotoent_r = __endprotoent_r
#pragma weak getprotobyname_r = __getprotobyname_r
#pragma weak getprotobynumber_r = __getprotobynumber_r
#pragma weak getprotoent_r = __getprotoent_r
#pragma weak setprotoent_r = __setprotoent_r
#endif
#if !defined(_THREAD_SAFE)
#pragma weak endprotoent = __endprotoent
#pragma weak getprotobyname = __getprotobyname
#pragma weak getprotobynumber = __getprotobynumber
#pragma weak getprotoent = __getprotoent
#pragma weak setprotoent = __setprotoent
#pragma weak svc_getprotoflag = __svc_getprotoflag
#endif
#endif
#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <rpcsvc/ypclnt.h>
#include <sys/svcinfo.h>
#include <netdb.h>
#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
#endif	/* _THREAD_SAFE */

#if defined(lint) && !defined(DEBUG)
#define DEBUG
#endif

#if defined(lint) && !defined(DEBUG_RESOLVER)
#define DEBUG_RESOLVER
#endif

/*
 * Interproto version.
 */

#define	MAXALIASES	35
#define	MAXADDRSIZE	14

#ifdef _THREAD_SAFE

#define	SETPROTOENT(a,b)		setprotoent_r(a,b)
#define	SETPROTOENT_LOCAL(a,b)	setprotoent_local_r(a,b)
#define	SETPROTOENT_YP(a,b)	setprotoent_yp_r(a,b)
#define	SETPROTOENT_BIND(a,b)	setent_bind_r(a,b)
#define	SETPROTOENTS(info,index,a,b)	(*(setprotoents_r[info->svcpath[SVC_PROTOCOLS][index]]))(a,b)
#define	ENDPROTOENT(a)		endprotoent_r(a)
#define	ENDPROTOENT_LOCAL(a)	endprotoent_local_r(a)
#define	ENDPROTOENT_YP(a)	endprotoent_yp_r(a)
#define	ENDPROTOENT_BIND(a)	endent_bind_r(a)
#define	ENDPROTOENTS(info,index,a)	(*(endprotoents_r[info->svcpath[SVC_PROTOCOLS][index]]))(a)
#define	GETPROTOENT(a,b)		getprotoent_r(a,b)
#define	GETPROTOENT_LOCAL(a,b)	getprotoent_local_r(a,b)
#define	GETPROTOENT_YP(a,b)	getprotoent_yp_r(a,b)
#define	GETPROTOENT_BIND(a,b)	getprotoent_bind_r(a,b)
#define	GETPROTOENTS(info,index,a,b)	(*(getprotoents_r[info->svcpath[SVC_PROTOCOLS][index]]))(a,b)
#define	GETPROTOBYNAME(a,b,c)	getprotobyname_r(a,b,c)
#define	GETPROTOBYNAME_LOCAL(a,b,c)	getprotobyname_local_r(a,b,c)
#define	GETPROTOBYNAME_YP(a,b,c)		getprotobyname_yp_r(a,b,c)
#define	GETPROTOBYNAME_BIND(a,b,c)	getprotobyname_bind_r(a,b,c)
#define	GETPROTOBYNAMES(info,index,a,b,c)	(*(getprotonames_r[info->svcpath[SVC_PROTOCOLS][index]]))(a,b,c)
#define	GETPROTOBYNUMBER(a,b,c)		getprotobynumber_r(a,b,c)
#define	GETPROTOBYNUMBER_LOCAL(a,b,c)	getprotobynumber_local_r(a,b,c)
#define	GETPROTOBYNUMBER_YP(a,b,c)	getprotobynumber_yp_r(a,b,c)
#define	GETPROTOBYNUMBER_BIND(a,b,c)	getprotobynumber_bind_r(a,b,c)
#define	GETPROTOBYNUMBERS(info,index,a,b,c)	(*(getprotoaddrs_r[info->svcpath[SVC_PROTOCOLS][index]]))(a,b,c)

#define	GETSVC(i)		getsvc_r(i)
#define	INTERPRET(a,b,c,d,e)	interpret_r(a,b,c,d,e)
#define	PROTOF			pt_data->proto_fp
#define	PROTOBUF			pt_data->line
#define	PROTO_ALIASES		pt_data->proto_aliases
#define	STAYOPEN		pt_data->_proto_stayopen

#define	SVC_GETPROTOFLAG		(pt_data->svc_getprotoflag-2)
#define	SVC_GETPROTOBIND		pt_data->svc_getprotobind

extern struct rec_mutex _nis_rmutex;

#else

#define	SETPROTOENT(a,b)		setprotoent(a)
#define	SETPROTOENT_LOCAL(a,b)	setprotoent_local(a)
#define	SETPROTOENT_YP(a,b)	setprotoent_yp(a)
#define	SETPROTOENT_BIND(a,b)	setent_bind(a)
#define	SETPROTOENTS(info,index,a,b)	(*(setprotoents[info->svcpath[SVC_PROTOCOLS][index]]))(a)
#define	ENDPROTOENT(a)		endprotoent()
#define	ENDPROTOENT_LOCAL(a)	endprotoent_local()
#define	ENDPROTOENT_YP(a)	endprotoent_yp()
#define	ENDPROTOENT_BIND(a)	endent_bind()
#define	ENDPROTOENTS(info,index,a)	(*(endprotoents[info->svcpath[SVC_PROTOCOLS][index]]))()
#define	GETPROTOENT(a,b)		getprotoent()
#define	GETPROTOENT_LOCAL(a,b)	getprotoent_local(a)
#define	GETPROTOENT_YP(a,b)	getprotoent_yp(a)
#define	GETPROTOENT_BIND(a,b)	getprotoent_bind(a)
#define	GETPROTOENTS(info,index,a,b)	(*(getprotoents[info->svcpath[SVC_PROTOCOLS][index]]))(a)
#define	GETPROTOBYNAME(a,b,c)	getprotobyname(a)
#define	GETPROTOBYNAME_LOCAL(a,b,c)	getprotobyname_local(a,b)
#define	GETPROTOBYNAME_YP(a,b,c)		getprotobyname_yp(a,b)
#define	GETPROTOBYNAME_BIND(a,b,c)	getprotobyname_bind(a,b)
#define	GETPROTOBYNAMES(info,index,a,b,c)	(*(getprotonames[info->svcpath[SVC_PROTOCOLS][index]]))(a,b)
#define	GETPROTOBYNUMBER(a,b,c)	getprotobynumber(a,b)
#define	GETPROTOBYNUMBER_LOCAL(a,b,c)	getprotobynumber_local(a,b)
#define	GETPROTOBYNUMBER_YP(a,b,c)	getprotobynumber_yp(a,b)
#define	GETPROTOBYNUMBER_BIND(a,b,c)	getprotobynumber_bind(a,b)
#define	GETPROTOBYNUMBERS(info,index,a,b,c)	(*(getprotoaddrs[info->svcpath[SVC_PROTOCOLS][index]]))(a,b)

#define	GETSVC(i)		(!!(i=getsvc()))
#define	INTERPRET(a,b,c,d,e)	interpret(a,b,c,d)

#define	PROTOF			protof
#define	PROTOBUF			protobuf
#define	PROTO_ALIASES		proto_aliases
#define	STAYOPEN		stayopen
#define	SVC_GETPROTOFLAG		svc_getprotoflag
#define	SVC_GETPROTOBIND		svc_getprotobind

static FILE *protof = NULL;
static char *protobuf = NULL;
static int stayopen = 0;
static char *proto_aliases[_MAXALIASES];

#endif	/* _THREAD_SAFE */

#ifdef	_THREAD_SAFE
extern int svc_getprotoflag;
#else
int svc_getprotoflag = -2;
#endif	/* _THREAD_SAFE */
int svc_getprotobind;

/* The following variables need to be made thread-safe */
static char PROTODB[] = "/etc/protocols";
static char *current = NULL;
static int currentlen;

#ifndef	_THREAD_SAFE
static struct protoent _proto;
static struct protoent *proto = &_proto;
#endif	/* _THREAD_SAFE */

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

#ifdef	BINDOK
typedef union {
    HEADER qb1;
    char qb2[MAXPACKET];
} querybuf_t;

typedef union {
    long al;
    char ac;
} align_t;
#endif	/* BINDOK */

/*
 * Internal functions
 */

#ifdef BINDOK
static void protocommon(char **pp, struct protoent *proto)
{
#ifndef	_THREAD_SAFE
	static char *proto_aliases[_MAXALIASES];
#endif	/* _THREAD_SAFE */
        register char *p, **q;

        if (pp == NULL)
                return(NULL);
        strcpy(buf, pp[0]);
        while(*pp)
		free(*pp++); /* necessary to avoid leaks */
        p = buf;
        proto->p_name = p;
        p = protoskip(p);
        proto->p_proto = atoi(p);
	q = proto->p_aliases = PROTO_ALIASES;
	while (*p && (p = protoskip(p)) != 0) {
		if (q < &PROTO_ALIASES[_MAXALIASES - 1])
			*q++ = p;
	}
        while (*p && *p != '\n')
                p++;
        *p = '\0';
}

static char *protoskip(char *p)
{
        while (*p && *p != ':' && *p != '\n')
                ++p;
        if (*p)
                *p++ = 0;
        return(p);
}
#endif

#ifdef	_THREAD_SAFE
static int interpret_r(char *val, int len, int svc, struct protoent *proto, struct protoent_data *pt_data)
#else
static int interpret(char *val, int len, int svc, struct protoent *proto)
#endif	/* _THREAD_SAFE */
{
#ifdef	_THREAD_SAFE
	static char *proto_aliases[_MAXALIASES];
#endif	/* _THREAD_SAFE */
	char *p;
	register char *cp, **q;

#ifndef	_THREAD_SAFE
	if(!PROTOBUF)
		if(!(PROTOBUF=(char *)malloc(_MAXLINELEN)))
			return TS_FAILURE;
#endif	/* !_THREAD_SAFE */
	if(val != PROTOBUF)
		strncpy(PROTOBUF, val, len);
	p = PROTOBUF;
	p[len] = '\n';
	if (*p == '#')
		switch (svc) {
			case SVC_LOCAL:
				return (GETPROTOENT_LOCAL(proto, pt_data));
			case SVC_YP:
				return (GETPROTOENT_YP(proto, pt_data));
		}
	cp = getcommon_any(p, "#\n");
	if (cp == NULL)
		switch (svc) {
			case SVC_LOCAL:
				return (GETPROTOENT_LOCAL(proto, pt_data));
			case SVC_YP:
				return (GETPROTOENT_YP(proto, pt_data));
		}
	*cp = '\0';
	proto->p_name = p;
	cp = getcommon_any(p, " \t");
	if (cp == NULL)
		switch (svc) {
			case SVC_LOCAL:
				return (GETPROTOENT_LOCAL(proto, pt_data));
			case SVC_YP:
				return (GETPROTOENT_YP(proto, pt_data));
		}
	*cp++ = '\0';
	while (*cp == ' ' || *cp == '\t')
		cp++;
	p = getcommon_any(cp, " \t");
	if (p != NULL)
		*p++ = '\0';
	proto->p_proto = atoi(cp);
	q = proto->p_aliases = PROTO_ALIASES;
	if (p != NULL)  {
		cp = p;
		while (cp && *cp) {
			if (*cp == ' ' || *cp == '\t') {
				cp++;
				continue;
			}
			if (q < &PROTO_ALIASES[_MAXALIASES - 1])
				*q++ = cp;
			cp = getcommon_any(cp, " \t");
			if (cp != NULL)
				*cp++ = '\0';
		}
	}
	*q = NULL;
	return TS_SUCCESS;
}

/*
 * specific getproto service routines
 */

#ifdef	_THREAD_SAFE
static int setprotoent_local_r(int f, struct protoent_data *pt_data)
#else
static int setprotoent_local(int f)
#endif	/* _THREAD_SAFE */
{
	if (PROTOF == NULL)
		PROTOF = fopen(PROTODB, "r");
	else
		rewind(PROTOF);
	STAYOPEN |= f;
	return TS_SUCCESS;
}

/*
 * setent_bind(stayopen) in getcommon.c
 */

#ifdef	_THREAD_SAFE
static int setprotoent_yp_r(int f, struct protoent_data *pt_data)
#else
static int setprotoent_yp(int f)
#endif	/* _THREAD_SAFE */
{
	char *domain;
	int status;

	TS_LOCK(&_nis_rmutex);
	if ((domain = yellowup(1)) == NULL) {
		status = SETPROTOENT_LOCAL(f, pt_data);
		TS_UNLOCK(&_nis_rmutex);
		return status;
	}
	if (current)
		free(current);
	current = NULL;
	TS_UNLOCK(&_nis_rmutex);
	return TS_SUCCESS;
}

#ifdef  _THREAD_SAFE
static int setent_bind_r(int f, struct protoent_data *pt_data)
{
	return setent_bind(f);
}
#endif	/* _THREAD_SAFE */

#ifdef	_THREAD_SAFE
static void endprotoent_local_r(struct protoent_data *pt_data)
#else
static void endprotoent_local()
#endif	/* _THREAD_SAFE */
{
	if (PROTOF && !STAYOPEN) {
		fclose(PROTOF);
		PROTOF = NULL;
	}
}

/*
 * endent_bind(stayopen) in getcommon.c
 */

#ifdef	_THREAD_SAFE
static void endprotoent_yp_r(struct protoent_data *pt_data)
#else
static void endprotoent_yp()
#endif	/* _THREAD_SAFE */
{
	char *domain;

	TS_LOCK(&_nis_rmutex);
	if ((domain = yellowup(0)) == NULL) {
		ENDPROTOENT_LOCAL(pt_data);
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
static void endent_bind_r(struct protoent_data *pt_data)
{
	endent_bind();
}
#endif	/* _THREAD_SAFE */

#ifdef	_THREAD_SAFE
static int getprotobynumber_local_r(int addr,
	 struct protoent *proto, struct protoent_data *pt_data)
#else
static int getprotobynumber_local(int addr,
	 struct protoent *proto)
#endif	/* _THREAD_SAFE */
{
	int status;

#ifdef DEBUG
	fprintf(stderr, "getprotobynumber_bind(%d)\n", addr);
#endif DEBUG
	SETPROTOENT_LOCAL(0, pt_data);
	while((status=GETPROTOENT_LOCAL(proto, pt_data)) != TS_FAILURE) {
		if(proto->p_proto == addr)
			break;
	}
	ENDPROTOENT_LOCAL(pt_data);
	return status;
}

#ifdef	_THREAD_SAFE
static int getprotobynumber_bind_r(int addr,
	struct protoent *proto, struct protoent_data *pt_data)
#else
static int getprotobynumber_bind(int addr,
	struct protoent *proto)
#endif	/* _THREAD_SAFE */
{
#ifdef	BINDOK
	register char **pp;
	char protobuf[12];
	int status = TS_FAILURE;

	if (type != AF_IPROTO)
		return TS_FAILURE;
	SETPROTOENT_BIND(0, pt_data);
	sprintf(addrbuf, "%d", proto);
#ifdef DEBUG
	fprintf(stderr, "getprotobynumber_bind(%d)\n", addr);
#endif	/* DEBUG */
	TS_LOCK(&_hesiod_rmutex);
	pp = hes_resolve(protobuf, "protocols");
	endent_bind();
	if(pp)
		if((status=protocommon(pp, proto)) != TS_FAILURE)
			memcpy(proto, np, sizeof (struct protoent));
	TS_UNLOCK(&_hesiod_rmutex);
	return status;
#else
	return TS_FAILURE;
#endif	/* BINDOK */
}

#ifdef	_THREAD_SAFE
static int getprotobynumber_yp_r(int addr,
	struct protoent *proto, struct protoent_data *pt_data)
#else
static int getprotobynumber_yp(int addr,
	struct protoent *proto)
#endif	/* _THREAD_SAFE */
{
	int reason, status = TS_FAILURE;
	char *val = NULL;
	int vallen = 0;
	char *domain;
	char protobuf[12];

	TS_LOCK(&_nis_rmutex);
	SETPROTOENT_YP(0, pt_data);
	if ((domain = yellowup(0)) == NULL) {
		status = GETPROTOBYNUMBER_LOCAL(addr, proto, pt_data);
		TS_UNLOCK(&_nis_rmutex);
		return status;
	}
	sprintf(protobuf, "%d", addr);
#ifdef DEBUG
	fprintf(stderr, "getprotobynumber_yp(%d)\n", addr);
#endif DEBUG

	if (reason = yp_match(domain, "protocols.bynumber", protobuf, strlen(protobuf), &val, &vallen)) {
#ifdef DEBUG
		fprintf(stderr, "reason yp_first failed is %d\n", reason);
#endif	/* DEBUG */
		status = TS_FAILURE;
	} else {
		status = INTERPRET(val, vallen, SVC_YP, proto, pt_data);
		free(val);
	}
	ENDPROTOENT_YP(pt_data);
	TS_UNLOCK(&_nis_rmutex);
	return status;
}

#ifdef	_THREAD_SAFE
static int getprotobyname_local_r(char *name,
	struct protoent *proto, struct protoent_data *pt_data)
#else
static int getprotobyname_local(char *name,
	struct protoent *proto)
#endif	/* _THREAD_SAFE */
{
	register char **cp;
	int status;

#ifdef DEBUG
	fprintf(stderr,  "getprotobyname_local(%s)\n", name);
#endif DEBUG
	SETPROTOENT_LOCAL(0, pt_data);
	while((status=GETPROTOENT_LOCAL(proto, pt_data)) != TS_FAILURE) {
		if (strcmp(proto->p_name, name) == 0)
			break;
		for (cp = proto->p_aliases; *cp != 0; cp++)
			if (strcmp(*cp, name) == 0)
				goto found;
	}
found:
	ENDPROTOENT_LOCAL(pt_data);
	return status;
}

#ifdef	_THREAD_SAFE
static int getprotobyname_bind_r(char *name,
	struct protoent *proto, struct protoent_data *pt_data)
#else
static int getprotobyname_bind(char *name,
	struct protoent *proto)
#endif	/* _THREAD_SAFE */
{
#ifdef	BINDOK
	register char *pp;
	int status = TS_FAILURE;

#ifdef DEBUG
	fprintf(stderr,  "getprotobyname_bind(%s)\n", name);
#endif /* DEBUG */
	SETPROTOENT_BIND(0, pt_data);
	TS_LOCK(&_hesiod_rmutex);
	pp = hes_resolve(name, "protocols");
	ENDPROTOENT_BIND(pt_data);
	if(pp)
		if((status=protocommon(pp, proto)) != TS_FAILURE)
			memcpy(proto, np, sizeof (struct protoent));
	TS_UNLOCK(&_hesiod_rmutex);
	return status;
#else
	return TS_FAILURE;
#endif	/* BINDOK */
}

#ifdef	_THREAD_SAFE
static int getprotobyname_yp_r(char *name,
	struct protoent *proto, struct protoent_data *pt_data)
#else
static int getprotobyname_yp(char *name,
	struct protoent *proto)
#endif	/* _THREAD_SAFE */
{
	int reason, status;
	char *domain, *val = NULL;
	int vallen = 0;

#ifdef DEBUG
	fprintf(stderr,  "getprotobyname_yp(%s)\n", name);
#endif DEBUG
	TS_LOCK(&_nis_rmutex);
	SETPROTOENT_YP(0, pt_data);
	if ((domain = yellowup(0)) == NULL) {
		status = (GETPROTOBYNAME_LOCAL(name, proto, pt_data));
		TS_UNLOCK(&_nis_rmutex);
		return status;
	}
	if (reason = yp_match(domain, "protocols.byname", name, strlen(name), &val, &vallen)) {
#ifdef DEBUG
		fprintf(stderr,  "reason yp_first failed is %d\n", reason);
#endif	/* DEBUG */
		status = TS_FAILURE;
	} else {
		status = INTERPRET(val, vallen, SVC_YP, proto, pt_data);
		free(val);
	}
	ENDPROTOENT_YP(pt_data);
	TS_UNLOCK(&_nis_rmutex);
	return status;
}

#ifdef	_THREAD_SAFE
static int getprotoent_local_r(struct protoent *proto, struct protoent_data *pt_data)
#else
static int getprotoent_local(struct protoent *proto)
#endif	/* _THREAD_SAFE */
{
#ifdef DEBUG
	fprintf(stderr, "getprotoent_local\n");
#endif DEBUG
	if (PROTOF == NULL && (PROTOF = fopen(PROTODB, "r")) == NULL)
		return TS_FAILURE;
#ifndef	_THREAD_SAFE
	if(!PROTOBUF)
		if(!(PROTOBUF=(char *)malloc(_MAXLINELEN)))
			return TS_FAILURE;
#endif	/* !_THREAD_SAFE */
	if (fgets(PROTOBUF, _MAXLINELEN, PROTOF) == NULL)
		return TS_FAILURE;
	return INTERPRET(PROTOBUF, strlen(PROTOBUF), SVC_LOCAL, proto, pt_data);
}

#ifdef	_THREAD_SAFE
static int getprotoent_bind_r(struct protoent *proto, struct protoent_data *pt_data)
#else
static int getprotoent_bind(struct protoent *proto)
#endif	/* _THREAD_SAFE */
{
#ifdef	BINDOK
	char bindbuf[64];

	sprintf(bindbuf, "protocols-%d", SVC_GETPROTOBIND);
#ifdef DEBUG
	fprintf(stderr, "getprotoent_bind(%s)\n", bindbuf);
#endif /* DEBUG */
	if(GETPROTOBYNAME_BIND(bindbuf, proto, pt_data) == TS_FAILURE)
		return TS_FAILURE;
#ifdef	_THREAD_SAFE
	pt_data->svc_getprotobind++;
	svc_getprotobind = pt_data->svc_getprotobind;
#else
	svc_getprotobind++;
#endif	/* _THREAD_SAFE */
	return TS_SUCCESS;
#else
	return TS_FAILURE;
#endif	/* BINDOK */
}

#ifdef	_THREAD_SAFE
static int getprotoent_yp_r(struct protoent *proto, struct protoent_data *pt_data)
#else
static int getprotoent_yp(struct protoent *proto)
#endif	/* _THREAD_SAFE */
{
	int reason, status;
	char *key = NULL;
	char *val = NULL;
	char *domain;
	int keylen = 0;
	int vallen = 0;

#ifdef DEBUG
	fprintf(stderr, "getprotoent_yp\n");
#endif /* DEBUG */
	TS_LOCK(&_nis_rmutex);
	if ((domain = yellowup(0)) == NULL) {
		status = GETPROTOENT_LOCAL(proto, pt_data);
		TS_UNLOCK(&_nis_rmutex);
		return status;
	}
	if (current == NULL) {
		if (reason =  yp_first(domain, "protocols.bynumber", &key, &keylen, &val, &vallen)) {
#ifdef DEBUG
			fprintf(stderr,  "reason yp_first failed is %d\n", reason);
#endif	/* DEBUG */
			TS_UNLOCK(&_nis_rmutex);
			return TS_FAILURE;
		}
	} else {
		if (reason = yp_next(domain, "protocols.bynumber", current, currentlen, &key, &keylen, &val, &vallen)) {
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
	status = INTERPRET(val, vallen, SVC_YP, proto, pt_data);
	free(val);
	TS_UNLOCK(&_nis_rmutex);
	return status;
}

/* 
 *	call service routines indirectly
 */

#ifdef	_THREAD_SAFE

static int	(*setprotoents_r []) (int, struct protoent_data *)={
		setprotoent_local_r,
		setprotoent_yp_r,
		setent_bind_r
};
static void	(*endprotoents_r []) (struct protoent_data *)={
		endprotoent_local_r,
		endprotoent_yp_r,
		endent_bind_r
};
static int	(*getprotoents_r []) ()={
		getprotoent_local_r,
		getprotoent_yp_r,
		getprotoent_bind_r
};
static int	(*getprotoaddrs_r [])
	(int, struct protoent *, struct protoent_data *)={
		getprotobynumber_local_r,
		getprotobynumber_yp_r,
		getprotobynumber_bind_r
};
static int	(*getprotonames_r [])
	(char *, struct protoent *, struct protoent_data *)={
		getprotobyname_local_r,
		getprotobyname_yp_r,
		getprotobyname_bind_r
};

#else	/* _THREAD_SAFE */

static int	(*setprotoents []) (int)={
		setprotoent_local,
		setprotoent_yp,
		setent_bind
};
static void	(*endprotoents []) (void)={
		endprotoent_local,
		endprotoent_yp,
		endent_bind
};
static int (*getprotoents [])
	(struct protoent *)={
		getprotoent_local,
		getprotoent_yp,
		getprotoent_bind
};
static int (*getprotoaddrs [])
	(int, struct protoent *)={
		getprotobynumber_local,
		getprotobynumber_yp,
		getprotobynumber_bind
};
static int (*getprotonames [])
	(char *, struct protoent *)={
		getprotobyname_local,
		getprotobyname_yp,
		getprotobyname_bind
};

#endif	/* _THREAD_SAFE */

/*
 * generic getproto service routines
 */
#ifdef	_THREAD_SAFE
int setprotoent_r(int f, struct protoent_data *pt_data)
#else
int setprotoent(int f)
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
	if(!pt_data) {
		_Seterrno(EINVAL);
		return TS_FAILURE;
	}
#endif	/* _THREAD_SAFE */
	svc_getprotoflag = -1;
	svc_getprotobind = 0;
#ifdef	_THREAD_SAFE
	pt_data->svc_getprotoflag = 1;
#ifdef	BINDOK
	pt_data->svc_getprotobind = 0;
#endif	/* BINDOK */
#endif	/* _THREAD_SAFE */
	if(GETSVC(svcinfo) == TS_SUCCESS)
		for (i=0; (svc_lastlookup = svcinfo->svcpath[SVC_PROTOCOLS][i]) != SVC_LAST; i++)
			SETPROTOENTS(svcinfo,i,f,pt_data);
	return TS_SUCCESS;
}

#ifdef	_THREAD_SAFE
void endprotoent_r(struct protoent_data *pt_data)
#else
void endprotoent()
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
	if(!pt_data) {
		_Seterrno(EINVAL);
		return;
	}
#endif	/* _THREAD_SAFE */
	svc_getprotoflag = -1;
	svc_getprotobind = 0;
#ifdef	_THREAD_SAFE
	pt_data->svc_getprotoflag = 1;
#ifdef	BINDOK
	pt_data->svc_getprotobind = 0;
#endif	/* BINDOK */
#endif	/* _THREAD_SAFE */
	if(GETSVC(svcinfo) == TS_SUCCESS)
		for (i=0; (svc_lastlookup = svcinfo->svcpath[SVC_PROTOCOLS][i]) != SVC_LAST; i++)
			ENDPROTOENTS(svcinfo,i,pt_data);
}

#ifdef	_THREAD_SAFE
int getprotoent_r(struct protoent *proto, struct protoent_data *pt_data)
#else
struct protoent *getprotoent()
#endif	/* _THREAD_SAFE */
{
	register i;
	int status;
	static void fix_err();
#ifdef	_THREAD_SAFE
	struct svcinfo si;
	struct svcinfo *svcinfo = &si;
#else
	static struct svcinfo *svcinfo;
#endif	/* _THREAD_SAFE */

#ifdef	_THREAD_SAFE
	if(!proto || !pt_data) {
		_Seterrno(EINVAL);
		return TS_FAILURE;
	}
#endif	/* _THREAD_SAFE */
	/*
	 * Check if setprotoent was not made yet
	 */
	if (SVC_GETPROTOFLAG == -2)
		SETPROTOENT(0, pt_data);
#ifdef	_THREAD_SAFE
	/*
	 * Always init the svcinfo structure
	 */
	if(GETSVC(svcinfo) != TS_SUCCESS)
		return TS_FAILURE;
#endif	/* _THREAD_SAFE */
	/*
	 * Check if this is the first time through getprotoent
	 */
	if (SVC_GETPROTOFLAG == -1) {
#ifndef	_THREAD_SAFE
		/*
		 * If it is, init the svcinfo structure
		 */
		if(GETSVC(svcinfo) != TS_SUCCESS)
				return TS_FAILURE;
#endif	/* _THREAD_SAFE */
		i = 0;
	}
	else {
		/*
		 * If it is not, set the index to the last used one
		 */
		i = SVC_GETPROTOFLAG;
	}
	for (; (svc_lastlookup = svcinfo->svcpath[SVC_PROTOCOLS][i]) != SVC_LAST; i++)
		if((status=GETPROTOENTS(svcinfo, i, proto, pt_data)) != TS_FAILURE) {
			svc_getprotoflag = i;
#ifdef	_THREAD_SAFE
			pt_data->svc_getprotoflag = i+2;
#endif	/* _THREAD_SAFE */
			break;
		}
#ifdef	_THREAD_SAFE
	if (status == TS_FAILURE)
		fix_err();
	return status;
#else
	if(status != TS_FAILURE)
		return proto;
	else
	{
		fix_err();
		return (struct protoent *) 0;
	}
#endif	/* _THREAD_SAFE */
}

#ifdef	_THREAD_SAFE
int getprotobyname_r(char *name, struct protoent *proto, struct protoent_data *pt_data)
#else
struct protoent *getprotobyname(char *name)
#endif	/* _THREAD_SAFE */
{
	register i;
	int status;
	static void fix_err();
#ifdef	_THREAD_SAFE
	struct svcinfo si;
	struct svcinfo *svcinfo = &si;
#else
	struct svcinfo *svcinfo;
#endif	/* _THREAD_SAFE */

	/* avoid null pointer de-reference on mips */
	if (name == 0)
		return TS_FAILURE;
#ifdef	_THREAD_SAFE
	if(!proto || !pt_data) {
		_Seterrno(EINVAL);
		return TS_FAILURE;
	}
#endif	/* _THREAD_SAFE */
	if(GETSVC(svcinfo) == TS_SUCCESS)
		for (i=0; (svc_lastlookup = svcinfo->svcpath[SVC_PROTOCOLS][i]) != SVC_LAST; i++)
			if((status=GETPROTOBYNAMES(svcinfo,i,name,proto,pt_data)) != TS_FAILURE)
				break;
#ifdef	_THREAD_SAFE
	if (status == TS_FAILURE)
		fix_err();
	return status;
#else
	if(status != TS_FAILURE)
		return proto;
	else
	{
		fix_err();
		return (struct protoent *) 0;
	}
#endif	/* _THREAD_SAFE */
}

#ifdef	_THREAD_SAFE
int getprotobynumber_r(int addr, struct protoent *proto, struct protoent_data *pt_data)
#else
struct protoent *getprotobynumber (int addr)
#endif	/* _THREAD_SAFE */
{
	register i;
	int status;
	static void fix_err();
#ifdef	_THREAD_SAFE
	struct svcinfo si;
	struct svcinfo *svcinfo = &si;
#else
	struct svcinfo *svcinfo;
#endif	/* _THREAD_SAFE */

#ifdef	_THREAD_SAFE
	if(!proto || !pt_data) {
		_Seterrno(EINVAL);
		return TS_FAILURE;
	}
#endif	/* _THREAD_SAFE */
	if(GETSVC(svcinfo) == TS_SUCCESS)
		for (i=0; (svc_lastlookup = svcinfo->svcpath[SVC_PROTOCOLS][i]) != SVC_LAST; i++)
			if((status=GETPROTOBYNUMBERS(svcinfo,i,addr,proto,pt_data)) != TS_FAILURE)
				break;
#ifdef	_THREAD_SAFE
	if (status == TS_FAILURE)
		fix_err();
	return status;
#else
	if(status != TS_FAILURE)
		return proto;
	else
	{
		fix_err();
		return (struct protoent *) 0;
	}
#endif	/* _THREAD_SAFE */
}

static void
fix_err()
{
	int l_errno;

#ifdef _THREAD_SAFE
        l_errno = geterrno();
#else
        l_errno = errno;
#endif

	if (l_errno == 0 || l_errno == EWOULDBLOCK)
		_Seterrno(ESRCH);
}
