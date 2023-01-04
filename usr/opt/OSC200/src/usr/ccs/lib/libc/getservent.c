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
static char     *sccsid = "@(#)$RCSfile: getservent.c,v $ $Revision: 4.2.10.8 $ (DEC) $Date: 1993/11/09 21:52:09 $";
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
 * 30-Oct-90    terry
 *      Added code to allow getservbyname_yp() and getservbyport_yp() to use
 *      getservent_yp() instead of yp_match if the server is running a 
 *      release prior to ULTRIX 4.2.
 *
 * 30-Aug-90    terry
 *    Changed getservbyname_yp() and getservbyport_yp() to use yp_match
 *    instead of yp_first/yp_next.
 *
 * 13-Nov-89	sue
 *	Changed svc_getservflag initial value to -2 and now perform a
 *	check in getservent to see if the setservent has been called yet.
 *
 * 24-Jul-89	logcher
 *	Removed generic setservent and endservent calls from generic
 *	getservbyname and getservbyport.  Added the specific set and end
 *	calls in the specific get routines.
 *
 * 25-May-89	logcher
 *	Changed name of any() to getcommon_any().
 *
 * 16-May-89	logcher
 *	Modularized the code to have separate local, yp, bind/hesiod
 *	routines.  Added front end to check the /etc/svc.conf file
 *	for the service ordering.
 *
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak endservent_r = __endservent_r
#pragma weak getservbyname_r = __getservbyname_r
#pragma weak getservbyport_r = __getservbyport_r
#pragma weak getservent_r = __getservent_r
#pragma weak setservent_r = __setservent_r
#endif
#if !defined(_THREAD_SAFE)
#pragma weak endservent = __endservent
#pragma weak getservbyname = __getservbyname
#pragma weak getservbyport = __getservbyport
#pragma weak getservent = __getservent
#pragma weak setservent = __setservent
#pragma weak svc_getservflag = __svc_getservflag
#endif
#endif
#ifdef _NAME_SPACE_WEAK_STRONG
#ifdef endservent_local
#undef endservent_local
#endif
#ifdef getservbyname_local
#undef getservbyname_local
#endif
#ifdef getservbyport_local
#undef getservbyport_local
#endif
#ifdef getservent_local
#undef getservent_local
#endif
#ifdef serv
#undef serv
#endif
#ifdef setservent_local
#undef setservent_local
#endif
#endif
/* 
 * unlike gethost, getpw, etc, this doesn't implement getservbyxxx
 * directly
 */

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
#define	MAXADDRSIZE	14

#ifdef _THREAD_SAFE

#define	SETSERVENT(a,b)		setservent_r(a,b)
#define	SETSERVENT_LOCAL(a,b)	setservent_local_r(a,b)
#define	SETSERVENT_YP(a,b)	setservent_yp_r(a,b)
#define	SETSERVENT_BIND(a,b)	setent_bind_r(a,b)
#define	SETSERVENTS(info,index,a,b)	(*(setservents_r[info->svcpath[SVC_SERVICES][index]]))(a,b)
#define	ENDSERVENT(a)		endservent_r(a)
#define	ENDSERVENT_LOCAL(a)	endservent_local_r(a)
#define	ENDSERVENT_YP(a)	endservent_yp_r(a)
#define	ENDSERVENT_BIND(a)	endent_bind_r(a)
#define	ENDSERVENTS(info,index,a)	(*(endservents_r[info->svcpath[SVC_SERVICES][index]]))(a)
#define	GETSERVENT(a,b)		getservent_r(a,b)
#define	GETSERVENT_LOCAL(a,b)	getservent_local_r(a,b)
#define	GETSERVENT_YP(a,b)	getservent_yp_r(a,b)
#define	GETSERVENT_BIND(a,b)	getservent_bind_r(a,b)
#define	GETSERVENTS(info,index,a,b)	(*(getservents_r[info->svcpath[SVC_SERVICES][index]]))(a,b)
#define	GETSERVBYNAME(a,b,c,d)	getservbyname_r(a,b,c,d)
#define	GETSERVBYNAME_LOCAL(a,b,c,d)	getservbyname_local_r(a,b,c,d)
#define	GETSERVBYNAME_YP(a,b,c,d)		getservbyname_yp_r(a,b,c,d)
#define	GETSERVBYNAME_BIND(a,b,c,d)	getservbyname_bind_r(a,b,c,d)
#define	GETSERVBYNAMES(info,index,a,b,c,d)	(*(getservnames_r[info->svcpath[SVC_SERVICES][index]]))(a,b,c,d)
#define	GETSERVBYPORT(a,b,c,d)	getservbyport_r(a,b,c,d)
#define	GETSERVBYPORT_LOCAL(a,b,c,d)	getservbyport_local_r(a,b,c,d)
#define	GETSERVBYPORT_YP(a,b,c,d)	getservbyport_yp_r(a,b,c,d)
#define	GETSERVBYPORT_BIND(a,b,c,d)	getservbyport_bind_r(a,b,c,d)
#define	GETSERVBYPORTS(info,index,a,b,c,d)	(*(getservports_r[info->svcpath[SVC_SERVICES][index]]))(a,b,c,d)

#define	GETSVC(i)		getsvc_r(i)
#define	INTERPRET(a,b,c,d,e)	interpret_r(a,b,c,d,e)
#define	SERVF			st_data->serv_fp
#define	SERVBUF			st_data->line
#define	SERV_ALIASES		st_data->serv_aliases
#define	STAYOPEN		st_data->_serv_stayopen

#define	SVC_GETSERVFLAG		(st_data->svc_getservflag-2)
#define	SVC_GETSERVBIND		st_data->svc_getservbind

extern struct rec_mutex _nis_rmutex;

#else

#define	SETSERVENT(a,b)		setservent(a)
#define	SETSERVENT_LOCAL(a,b)	setservent_local(a)
#define	SETSERVENT_YP(a,b)	setservent_yp(a)
#define	SETSERVENT_BIND(a,b)	setent_bind(a)
#define	SETSERVENTS(info,index,a,b)	(*(setservents[info->svcpath[SVC_SERVICES][index]]))(a)
#define	ENDSERVENT(a)		endservent()
#define	ENDSERVENT_LOCAL(a)	endservent_local()
#define	ENDSERVENT_YP(a)	endservent_yp()
#define	ENDSERVENT_BIND(a)	endent_bind()
#define	ENDSERVENTS(info,index,a)	(*(endservents[info->svcpath[SVC_SERVICES][index]]))()
#define	GETSERVENT(a,b)		getservent()
#define	GETSERVENT_LOCAL(a,b)	getservent_local(a)
#define	GETSERVENT_YP(a,b)	getservent_yp(a)
#define	GETSERVENT_BIND(a,b)	getservent_bind(a)
#define	GETSERVENTS(info,index,a,b)	(*(getservents[info->svcpath[SVC_SERVICES][index]]))(a)
#define	GETSERVBYNAME(a,b,c,d)	getservbyname(a)
#define	GETSERVBYNAME_LOCAL(a,b,c,d)	getservbyname_local(a,b,c)
#define	GETSERVBYNAME_YP(a,b,c,d)		getservbyname_yp(a,b,c)
#define	GETSERVBYNAME_BIND(a,b,c,d)	getservbyname_bind(a,b,c)
#define	GETSERVBYNAMES(info,index,a,b,c,d)	(*(getservnames[info->svcpath[SVC_SERVICES][index]]))(a,b,c)
#define	GETSERVBYPORT(a,b,c,d)	getservbyport(a,b,c)
#define	GETSERVBYPORT_LOCAL(a,b,c,d)	getservbyport_local(a,b,c)
#define	GETSERVBYPORT_YP(a,b,c,d)	getservbyport_yp(a,b,c)
#define	GETSERVBYPORT_BIND(a,b,c,d)	getservbyport_bind(a,b,c)
#define	GETSERVBYPORTS(info,index,a,b,c,d)	(*(getservports[info->svcpath[SVC_SERVICES][index]]))(a,b,c)

#define	GETSVC(i)		(!!(i=getsvc()))
#define	INTERPRET(a,b,c,d,e)	interpret(a,b,c,d)

#define	SERVF			servf
#define	SERVBUF			servbuf
#define	SERV_ALIASES		serv_aliases
#define	STAYOPEN		stayopen
#define	SVC_GETSERVFLAG		svc_getservflag
#define	SVC_GETSERVBIND		svc_getservbind

static FILE *servf = NULL;
static char *servbuf = NULL;
static int stayopen = 0;
static char *serv_aliases[_MAXALIASES];

#endif	/* _THREAD_SAFE */

#ifdef	_THREAD_SAFE
extern int svc_getservflag;
#else
int svc_getservflag = -2;
#endif	/* _THREAD_SAFE */
int svc_getservbind;

/* The following variables need to be made thread-safe */
static char SERVDB[] = "/etc/services";
static char *current = NULL;
static int currentlen;

#ifndef	_THREAD_SAFE
static struct servent _serv;
static struct servent *serv = &_serv;
#endif	/* _THREAD_SAFE */

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


#ifdef	BINDOK
static void servcommon(char **pp, struct servent *serv)
{
#ifndef	_THREAD_SAFE
	static char *serv_aliases[_MAXALIASES];
#endif	/* _THREAD_SAFE */
        register char *p, **q;

        if (pp == NULL)
                return(NULL);
	/* choose only the first response (only 1 expected) */
        strcpy(buf, pp[0]);
        while(*pp)
		free(*pp++); /* necessary to avoid leaks */
        p = buf;
        serv->s_name = p;
        p = servskip(p);
        serv->s_port = htons((u_short)atoi(p));
	p = servskip(p);
	serv->s_proto = p;
	q = serv->s_aliases = SERV_ALIASES;
	while (*p && (p = servskip(p)) != 0) {
		if (q < &SERV_ALIASES[_MAXALIASES - 1])
			*q++ = p;
	}
        *p = '\0';
}

static char *servskip(char *p)
{
        while (*p && *p != ':' && *p != '\n')
                ++p;
        if (*p)
                *p++ = 0;
        return(p);
}
#endif

#ifdef	_THREAD_SAFE
static int interpret_r(char *val, int len, int svc, struct servent *serv, struct servent_data *st_data)
#else
static int interpret(char *val, int len, int svc, struct servent *serv)
#endif	/* _THREAD_SAFE */
{
#ifdef	_THREAD_SAFE
	static char *serv_aliases[_MAXALIASES];
#endif	/* _THREAD_SAFE */
	char *p;
	register char *cp, **q;

#ifndef	_THREAD_SAFE
	if(!SERVBUF)
		if(!(SERVBUF=(char *)malloc(_MAXLINELEN)))
			return TS_FAILURE;
#endif	/* !_THREAD_SAFE */
	if(val != SERVBUF)
		strncpy(SERVBUF, val, len);
	p = SERVBUF;
	p[len] = '\n';
	if (*p == '#')
		switch (svc) {
			case SVC_LOCAL:
				return (GETSERVENT_LOCAL(serv, st_data));
			case SVC_YP:
				return (GETSERVENT_YP(serv, st_data));
		}
	cp = getcommon_any(p, "#\n");
	if (cp == NULL)
		switch (svc) {
			case SVC_LOCAL:
				return (GETSERVENT_LOCAL(serv, st_data));
			case SVC_YP:
				return (GETSERVENT_YP(serv, st_data));
		}
	*cp = '\0';
	serv->s_name = p;
	p = getcommon_any(p, " \t");
	if (p == NULL)
		switch (svc) {
			case SVC_LOCAL:
				return (GETSERVENT_LOCAL(serv, st_data));
			case SVC_YP:
				return (GETSERVENT_YP(serv, st_data));
		}
	*p++ = '\0';
	while (*p == ' ' || *p == '\t')
		p++;
	cp = getcommon_any(p, ",/");
	if (cp == NULL)
		switch (svc) {
			case SVC_LOCAL:
				return (GETSERVENT_LOCAL(serv, st_data));
			case SVC_YP:
				return (GETSERVENT_YP(serv, st_data));
		}
	*cp++ = '\0';
	serv->s_port = htons((u_short)atoi(p));
	serv->s_proto = cp;
	q = serv->s_aliases = serv_aliases;
	cp = getcommon_any(cp, " \t");
	if (cp != NULL)
		*cp++ = '\0';
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &SERV_ALIASES[_MAXALIASES - 1])
			*q++ = cp;
		cp = getcommon_any(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	return TS_SUCCESS;
}

/*
 * specific getserv service routines
 */

#ifdef	_THREAD_SAFE
static int setservent_local_r(int f, struct servent_data *st_data)
#else
static int setservent_local(int f)
#endif	/* _THREAD_SAFE */
{
	if (SERVF == NULL)
		SERVF = fopen(SERVDB, "r");
	else
		rewind(SERVF);
	STAYOPEN |= f;
	return TS_SUCCESS;
}

/*
 * setent_bind(stayopen) in getcommon.c
 */

#ifdef	_THREAD_SAFE
static int setservent_yp_r(int f, struct servent_data *st_data)
#else
static int setservent_yp(int f)
#endif	/* _THREAD_SAFE */
{
	char *domain;
	int status;

	TS_LOCK(&_nis_rmutex);
	if ((domain = yellowup(1)) == NULL) {
		status = SETSERVENT_LOCAL(f, st_data);
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
static int setent_bind_r(int f, struct servent_data *st_data)
{
	setent_bind(f);
}
#endif	/* _THREAD_SAFE */

#ifdef	_THREAD_SAFE
static void endservent_local_r(struct servent_data *st_data)
#else
static void endservent_local()
#endif	/* _THREAD_SAFE */
{
	if (SERVF && !STAYOPEN) {
		fclose(SERVF);
		SERVF = NULL;
	}
}

/*
 * endent_bind(stayopen) in getcommon.c
 */

#ifdef	_THREAD_SAFE
static void endservent_yp_r(struct servent_data *st_data)
#else
static void endservent_yp()
#endif	/* _THREAD_SAFE */
{
	char *domain;

	TS_LOCK(&_nis_rmutex);
	if ((domain = yellowup(0)) == NULL) {
		ENDSERVENT_LOCAL(st_data);
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
static void endent_bind_r(struct servent_data *st_data)
{
	endent_bind();
}
#endif	/* _THREAD_SAFE */

#ifdef	_THREAD_SAFE
static int getservbyport_local_r(int port, char *proto,
	 struct servent *serv, struct servent_data *st_data)
#else
static int getservbyport_local(int port, char *proto,
	 struct servent *serv)
#endif	/* _THREAD_SAFE */
{
	int status;

#ifdef DEBUG
	if(proto == 0)
		fprintf(stderr, "getservbyport_local(%d)\n", port);
	else
		fprintf(stderr, "getservbyport_local(%d/%s)\n", port, proto);
#endif DEBUG
	SETSERVENT_LOCAL(0, st_data);
	while((status=GETSERVENT_LOCAL(serv, st_data)) != TS_FAILURE) {
		if(serv->s_port != port)
			continue;
		if(proto == 0 || strcmp(serv->s_proto, proto) == 0)
			break;
	}
	ENDSERVENT_LOCAL(st_data);
	return status;
}

#ifdef	_THREAD_SAFE
static int getservbyport_bind_r(int port, char *proto,
	struct servent *serv, struct servent_data *st_data)
#else
static int getservbyport_bind(int port, char *proto,
	struct servent *serv)
#endif	/* _THREAD_SAFE */
{
#ifdef	BINDOK
	register char **pp;
	char adrstr[10];
	int status = TS_FAILURE;

	if (proto == 0)
		sprintf(portbuf, "%d", ntohs((u_short)port));
	else
		sprintf(portbuf, "%d/%s", ntohs((u_short)port), proto);
#ifdef	DEBUG
	fprintf(stderr, "getservbyport_bind(%d, %s)\n", port, portbuf);
#endif	/* DEBUG */
	SETSERVENT_BIND(0, st_data);
	TS_LOCK(&_hesiod_rmutex);
	pp = hes_resolve(adrstr, "services");
	endent_bind();
	if(pp)
		if((status=servcommon(pp, serv)) != TS_FAILURE)
			memcpy(serv, np, sizeof (struct servent));
	TS_UNLOCK(&_hesiod_rmutex);
	return status;
#else
	return TS_FAILURE;
#endif	/* BINDOK */
}

#ifdef	_THREAD_SAFE
static int getservbyport_yp_r(int port, char *proto,
	struct servent *serv, struct servent_data *st_data)
#else
static int getservbyport_yp(int port, char *proto,
	struct servent *serv)
#endif	/* _THREAD_SAFE */
{
	int reason, status = TS_FAILURE;
	char *val = NULL;
	int vallen = 0;
	char *domain, *portstr;
	int ok = 0;

#ifdef	DEBUG
	if (proto == 0)
		fprintf(stderr, "getservbyport_yp(%d)\n", port);
	else
		fprintf(stderr, "getservbyport_yp(%d/%s)\n", port, proto);
#endif	/* DEBUG */
	TS_LOCK(&_nis_rmutex);
	if ((domain = yellowup(0)) == NULL) {
		status = GETSERVBYPORT_LOCAL(port, proto, serv, st_data);
		TS_UNLOCK(&_nis_rmutex);
		return status;
	}
	SETSERVENT_YP(0, st_data);
	if(proto != 0) {
		portstr = (char *) malloc(sizeof(int) + strlen(proto) + 2);
		sprintf(portstr, "%d/%s", ntohs((u_short)port), proto);
		if (yp_match(domain, "services.byport", portstr,
				strlen(portstr),&val, &vallen) == 0) {
			status = INTERPRET(val, vallen, SVC_YP, serv, st_data);
			free(val);
			ok = 1;
		}
		free(portstr);	/* DAL001, one more memory leak fixed */
	}
	if(!ok) {
		/* YP database 'services.byport' is not served by the current YP
		 * server or proto = 0.  Use getservent() instead of yp_match.
		 */
		while((status=GETSERVENT_YP(serv, st_data)) != TS_FAILURE) {
			if(serv->s_port != port)
				continue;
			if (proto == 0 || strcmp(serv->s_proto, proto) == 0)
				break;
		}
	}
	ENDSERVENT_YP(st_data);
	TS_UNLOCK(&_nis_rmutex);
	return status;
}

#ifdef	_THREAD_SAFE
static int getservbyname_local_r(char *name, char *proto,
	struct servent *serv, struct servent_data *st_data)
#else
static int getservbyname_local(char *name, char *proto,
	struct servent *serv)
#endif	/* _THREAD_SAFE */
{
	register char **cp;
	int status;

#ifdef DEBUG
	fprintf(stderr,  "getservbyname_local(%s)\n", name);
#endif DEBUG
	SETSERVENT_LOCAL(0, st_data);
	while((status=GETSERVENT_LOCAL(serv, st_data)) != TS_FAILURE) {
		if (strcmp(serv->s_name, name) == 0)
			goto gotname;
		for (cp = serv->s_aliases; *cp != 0; cp++)
			if (strcmp(*cp, name) == 0)
				goto gotname;
		continue;
gotname:
		if(proto == 0 || strcmp(serv->s_proto, proto) == 0)
			break;
	}
	ENDSERVENT_LOCAL(st_data);
	return status;
}

#ifdef	_THREAD_SAFE
static int getservbyname_bind_r(char *name, char *proto,
	struct servent *serv, struct servent_data *st_data)
#else
static int getservbyname_bind(char *name, char *proto,
	struct servent *serv)
#endif	/* _THREAD_SAFE */
{
#ifdef	BINDOK
	register char *pp, namebuf[MAXHOSTNAMELEN];
	int status = TS_FAILURE;

	if(proto == 0)
		sprintf(namebuf, "%s", name);
	else
		sprintf(namebuf, "%s/%s", name, proto);
#ifdef DEBUG
	fprintf(stderr,  "getservbyname_bind(%s)\n", namebuf);
#endif /* DEBUG */
	SETSERVENT_BIND(0, st_data);
	TS_LOCK(&_hesiod_rmutex);
	pp = hes_resolve(namebuf, "services");
	ENDSERVENT_BIND(st_data);
	if(pp)
		if((status=servcommon(pp, serv)) != TS_FAILURE)
			memcpy(serv, np, sizeof (struct servent));
	TS_UNLOCK(&_hesiod_rmutex);
	return status;
#else
	return TS_FAILURE;
#endif	/* BINDOK */
}

#ifdef	_THREAD_SAFE
static int getservbyname_yp_r(char *name, char *proto,
	struct servent *serv, struct servent_data *st_data)
#else
static int getservbyname_yp(char *name, char *proto,
	struct servent *serv)
#endif	/* _THREAD_SAFE */
{
	char *namestr, **cp;
	int reason, status;
	char *domain, *val = NULL;
	int vallen = 0;

#ifdef DEBUG
	if (proto == 0)
		fprintf(stderr,  "getservbyname_yp(%s)\n", name);
	else
		fprintf(stderr,  "getservbyname_yp(%s/%s)\n", name, proto);
#endif	/* DEBUG */
	TS_LOCK(&_nis_rmutex);
	if ((domain = yellowup(0)) == NULL) {
		status = GETSERVBYNAME_LOCAL(name, proto, serv, st_data);
		TS_UNLOCK(&_nis_rmutex);
		return status;
	}
	SETSERVENT_YP(0, st_data);
	if (proto == 0) {
		namestr = (char *) malloc (strlen(name) +1);
		strcpy(namestr, name);
	}
	else {
		namestr = (char *) malloc(strlen(name) + strlen(proto) + 2);
		strcpy(namestr, name);
		strcat(namestr, "/");
		strcat(namestr, proto);
	}
	if (reason = yp_match(domain, "services.byname_proto", namestr, strlen(namestr), &val, &vallen)) {
		/* Server may not serve the database 'services.byname_proto'.
		 * Use getservent_yp to find the service.
		 */
		while((status=GETSERVENT_YP(serv, st_data)) != TS_FAILURE) {
			if (strcmp(name, serv->s_name) == 0)
				goto gotname2;
			for (cp = serv->s_aliases; *cp; cp++)
				if (strcmp(name, *cp) == 0)
					goto gotname2;
			continue;
gotname2:
			if (proto == 0 || strcmp(serv->s_proto, proto) == 0)
				break;
		}
	} else {
		status = INTERPRET(val, vallen, SVC_YP, serv, st_data);
		free(val);
	}
	ENDSERVENT_YP(st_data);
	free(namestr);
	TS_UNLOCK(&_nis_rmutex);
	return status;
}

#ifdef	_THREAD_SAFE
static int getservent_local_r(struct servent *serv, struct servent_data *st_data)
#else
static int getservent_local(struct servent *serv)
#endif	/* _THREAD_SAFE */
{
#ifdef DEBUG
	fprintf(stderr, "getservent_local\n");
#endif DEBUG
	if (SERVF == NULL && (SERVF = fopen(SERVDB, "r")) == NULL)
		return TS_FAILURE;
#ifndef	_THREAD_SAFE
	if(!SERVBUF)
		if(!(SERVBUF=(char *)malloc(_MAXLINELEN)))
			return TS_FAILURE;
#endif	/* !_THREAD_SAFE */
	if (fgets(SERVBUF, _MAXLINELEN, SERVF) == NULL)
		return TS_FAILURE;
	return INTERPRET(SERVBUF, strlen(SERVBUF), SVC_LOCAL, serv, st_data);
}

#ifdef	_THREAD_SAFE
static int getservent_bind_r(struct servent *serv, struct servent_data *st_data)
#else
static int getservent_bind(struct servent *serv)
#endif	/* _THREAD_SAFE */
{
#ifdef	BINDOK
	char bindbuf[64];

	sprintf(bindbuf, "services-%d", SVC_GETSERVBIND);
#ifdef DEBUG
	fprintf(stderr, "getservent_bind(%s)\n", bindbuf);
#endif /* DEBUG */
	if(GETSERVBYNAME_BIND(bindbuf, 0, serv, st_data) == TS_FAILURE)
		return TS_FAILURE;
#ifdef	_THREAD_SAFE
	st_data->svc_getservbind++;
	svc_getservbind = st_data->svc_getservbind;
#else
	svc_getservbind++;
#endif	/* _THREAD_SAFE */
	return TS_SUCCESS;
#else
	return TS_FAILURE;
#endif	/* BINDOK */
}

#ifdef	_THREAD_SAFE
static int getservent_yp_r(struct servent *serv, struct servent_data *st_data)
#else
static int getservent_yp(struct servent *serv)
#endif	/* _THREAD_SAFE */
{
	int reason, status;
	char *key = NULL;
	char *val = NULL;
	char *domain;
	int keylen = 0;
	int vallen = 0;

#ifdef DEBUG
	fprintf(stderr, "getservent_yp\n");
#endif /* DEBUG */
	TS_LOCK(&_nis_rmutex);
	if ((domain = yellowup(0)) == NULL) {
		status = GETSERVENT_LOCAL(serv, st_data);
		TS_UNLOCK(&_nis_rmutex);
		return status;
	}
	if (current == NULL) {
		if (reason =  yp_first(domain, "services.byname", &key, &keylen, &val, &vallen)) {
#ifdef DEBUG
			fprintf(stderr,  "reason yp_first failed is %d\n", reason);
#endif	/* DEBUG */
			TS_UNLOCK(&_nis_rmutex);
			return TS_FAILURE;
		}
	} else {
		if (reason = yp_next(domain, "services.byname", current, currentlen, &key, &keylen, &val, &vallen)) {
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
	status = INTERPRET(val, vallen, SVC_YP, serv, st_data);
	free(val);
	TS_UNLOCK(&_nis_rmutex);
	return status;
}

/* 
 *	call service routines indirectly
 */

#ifdef	_THREAD_SAFE

static int	(*setservents_r []) (int, struct servent_data *)={
		setservent_local_r,
		setservent_yp_r,
		setent_bind_r
};
static void	(*endservents_r []) (struct servent_data *)={
		endservent_local_r,
		endservent_yp_r,
		endent_bind_r
};
static int	(*getservents_r []) (struct servent *, struct servent_data *)={
		getservent_local_r,
		getservent_yp_r,
		getservent_bind_r
};
static int	(*getservports_r [])
	(int, char *, struct servent *, struct servent_data *)={
		getservbyport_local_r,
		getservbyport_yp_r,
		getservbyport_bind_r
};
static int	(*getservnames_r [])
	(char *, char *, struct servent *, struct servent_data *)={
		getservbyname_local_r,
		getservbyname_yp_r,
		getservbyname_bind_r
};

#else	/* _THREAD_SAFE */

static int	(*setservents []) (int)={
		setservent_local,
		setservent_yp,
		setent_bind
};
static void	(*endservents []) (void)={
		endservent_local,
		endservent_yp,
		endent_bind
};
static int (*getservents [])
	(struct servent *)={
		getservent_local,
		getservent_yp,
		getservent_bind
};
static int (*getservports [])
	(int, char *, struct servent *)={
		getservbyport_local,
		getservbyport_yp,
		getservbyport_bind
};
static int (*getservnames [])
	(char *, char *, struct servent *)={
		getservbyname_local,
		getservbyname_yp,
		getservbyname_bind
};

#endif	/* _THREAD_SAFE */

/*
 * generic getserv service routines
 */
#ifdef	_THREAD_SAFE
int setservent_r(int f, struct servent_data *st_data)
#else
int setservent(int f)
#endif	/* _THREAD_SAFE */
{
	register i;
#ifdef	_THREAD_SAFE
	struct svcinfo si;
	struct svcinfo *svcinfo = &si;
#else
	struct svcinfo *svcinfo;
#endif	/* _THREAD_SAFE */

	TS_EINVAL(!st_data);
	svc_getservflag = -1;
	svc_getservbind = 0;
#ifdef	_THREAD_SAFE
	st_data->svc_getservflag = 1;
#ifdef	BINDOK
	st_data->svc_getservbind = 0;
#endif	/* BINDOK */
#endif	/* _THREAD_SAFE */
	if(GETSVC(svcinfo) == TS_SUCCESS)
		for (i=0; (svc_lastlookup = svcinfo->svcpath[SVC_SERVICES][i]) != SVC_LAST; i++)
			SETSERVENTS(svcinfo,i,f,st_data);
	return TS_SUCCESS;
}

#ifdef	_THREAD_SAFE
void endservent_r(struct servent_data *st_data)
#else
void endservent()
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
	if(!st_data) {
		_Seterrno(EINVAL);
		return;
	}
#endif	/* _THREAD_SAFE */
	svc_getservflag = -1;
	svc_getservbind = 0;
#ifdef	_THREAD_SAFE
	st_data->svc_getservflag = 1;
#ifdef	BINDOK
	st_data->svc_getservbind = 0;
#endif	/* BINDOK */
#endif	/* _THREAD_SAFE */
	if(GETSVC(svcinfo) == TS_SUCCESS)
		for (i=0; (svc_lastlookup = svcinfo->svcpath[SVC_SERVICES][i]) != SVC_LAST; i++)
			ENDSERVENTS(svcinfo,i,st_data);
}

#ifdef	_THREAD_SAFE
int getservent_r(struct servent *serv, struct servent_data *st_data)
#else
struct servent *getservent()
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

	TS_EINVAL(!serv || !st_data);
	/*
	 * Check if setservent was not made yet
	 */
	if (SVC_GETSERVFLAG == -2)
		SETSERVENT(0, st_data);
#ifdef	_THREAD_SAFE
	/*
	 * always init the svcinfo structure
	 */
	if(GETSVC(svcinfo) != TS_SUCCESS)
		return TS_FAILURE;
#endif	/* _THREAD_SAFE */
	/*
	 * Check if this is the first time through getservent
	 */
	if (SVC_GETSERVFLAG == -1) {
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
		i = SVC_GETSERVFLAG;
	}
	for (; (svc_lastlookup = svcinfo->svcpath[SVC_SERVICES][i]) != SVC_LAST; i++)
		if((status=GETSERVENTS(svcinfo, i, serv, st_data)) != TS_FAILURE) {
			svc_getservflag = i;
#ifdef	_THREAD_SAFE
			st_data->svc_getservflag = i+2;
#endif	/* _THREAD_SAFE */
			break;
		}
#ifdef	_THREAD_SAFE
	if (status == TS_FAILURE)
		fix_err();
	return status;
#else
	if(status != TS_FAILURE)
		return serv;
	else
	{
		fix_err();
		return (struct servent *) 0;
	}
#endif	/* _THREAD_SAFE */
}

#ifdef	_THREAD_SAFE
int getservbyname_r(char *name, char *proto, struct servent *serv, struct servent_data *st_data)
#else
struct servent *getservbyname(char *name, char *proto)
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

	TS_EINVAL(!serv || !st_data);
	/* avoid null pointer de-reference on mips */
	if (name == 0)
		return TS_FAILURE;
	if(GETSVC(svcinfo) == TS_SUCCESS)
		for (i=0; (svc_lastlookup = svcinfo->svcpath[SVC_SERVICES][i]) != SVC_LAST; i++)
			if((status=GETSERVBYNAMES(svcinfo,i,name,proto,serv,st_data)) != TS_FAILURE)
				break;
#ifdef	_THREAD_SAFE
	if (status == TS_FAILURE)
		fix_err();
	return status;
#else
	if(status != TS_FAILURE)
		return serv;
	else
	{
		fix_err();
		return (struct servent *) 0;
	}
#endif	/* _THREAD_SAFE */
}

#ifdef	_THREAD_SAFE
int getservbyport_r(int port, char *proto, struct servent *serv, struct servent_data *st_data)
#else
struct servent *getservbyport (int port, char *proto)
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

	TS_EINVAL(!serv || !st_data);
	if(GETSVC(svcinfo) == TS_SUCCESS)
		for (i=0; (svc_lastlookup = svcinfo->svcpath[SVC_SERVICES][i]) != SVC_LAST; i++)
			if((status=GETSERVBYPORTS(svcinfo,i,port,proto,serv,st_data)) != TS_FAILURE)
				break;
#ifdef	_THREAD_SAFE
	if (status == TS_FAILURE)
		fix_err();
	return status;
#else
	if(status != TS_FAILURE)
		return serv;
	else
	{
		fix_err();
		return (struct servent *) 0;
	}
#endif	/* _THREAD_SAFE */
}

static void
fix_err()
{
	int	l_errno;

#ifdef _THREAD_SAFE
        l_errno = geterrno();
#else
        l_errno = errno;
#endif

	if (l_errno == 0 || l_errno == EWOULDBLOCK)
		_Seterrno(ESRCH);
}
