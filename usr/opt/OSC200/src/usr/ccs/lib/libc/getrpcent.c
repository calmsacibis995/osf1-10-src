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
static char     *sccsid = "@(#)$RCSfile: getrpcent.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/10/05 21:01:48 $";
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
 *	Changed svc_getrpcflag initial value to -2 and now perform a
 *	check in getrpcent to see if the setrpcent has been called yet.
 *
 * 24-Jul-89	logcher
 *	Removed generic setrpcent and endrpcent calls from generic
 *	getrpcbyname and getrpcbynumber.  Added the specific set and end
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
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak endrpcent = __endrpcent
#pragma weak endrpcent_local = __endrpcent_local
#pragma weak endrpcent_yp = __endrpcent_yp
#pragma weak getrpcbyname = __getrpcbyname
#pragma weak getrpcbyname_bind = __getrpcbyname_bind
#pragma weak getrpcbyname_local = __getrpcbyname_local
#pragma weak getrpcbyname_yp = __getrpcbyname_yp
#pragma weak getrpcbynumber = __getrpcbynumber
#pragma weak getrpcbynumber_bind = __getrpcbynumber_bind
#pragma weak getrpcbynumber_local = __getrpcbynumber_local
#pragma weak getrpcbynumber_yp = __getrpcbynumber_yp
#pragma weak getrpcent = __getrpcent
#pragma weak getrpcent_bind = __getrpcent_bind
#pragma weak getrpcent_local = __getrpcent_local
#pragma weak getrpcent_yp = __getrpcent_yp
#pragma weak setrpcent = __setrpcent
#pragma weak setrpcent_local = __setrpcent_local
#pragma weak setrpcent_yp = __setrpcent_yp
#pragma weak svc_getrpcflag = __svc_getrpcflag
#endif
#endif

#include <stdio.h>
#include <netdb.h>
#include <rpc/netdb.h>
#include <sys/svcinfo.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <rpcsvc/ypclnt.h>
#include <malloc.h>
#include <assert.h>
#include <limits.h>

#if defined(lint) && !defined(DEBUG)
#define DEBUG
#endif

/*
 * Internet version.
 */
#define	MAXALIASES	35
#define	MAXADDRSIZE	14

static char RPCDB[] = "/etc/rpc";
static char *domain;
static FILE *rpcf = NULL;
static char *current = NULL;	/* current entry, analogous to hostf */
static int currentlen;
static int stayopen;
static struct svcinfo *svcinfo;

static struct rpcent *interpret();
char *getcommon_any();
char *yellowup();
int svc_getrpcflag = -2;
int svc_getrpcbind;

/*
 * Declare all service routines
 */
int setrpcent_local (); 
int setent_bind (); 
int setrpcent_yp ();
int endrpcent_local (); 
int endent_bind (); 
int endrpcent_yp ();
struct rpcent* getrpcent_local ();
struct rpcent* getrpcent_bind (); 
struct rpcent* getrpcent_yp ();
struct rpcent* getrpcbynumber_local (); 
struct rpcent* getrpcbynumber_bind (); 
struct rpcent* getrpcbynumber_yp (); 
struct rpcent* getrpcbyname_local ();
struct rpcent* getrpcbyname_bind ();	
struct rpcent* getrpcbyname_yp ();

static int	(*setrpcents []) ()={
		setrpcent_local,
		setrpcent_yp,
		setent_bind
};
static int 	(*endrpcents []) ()={
		endrpcent_local,
		endrpcent_yp,
		endent_bind
};
static struct rpcent * (*getrpcents []) ()={
		getrpcent_local,
		getrpcent_yp,
		getrpcent_bind
};
static struct rpcent * (*getrpcbynumbers []) ()={
		getrpcbynumber_local,
		getrpcbynumber_yp,
		getrpcbynumber_bind
};
static struct rpcent * (*getrpcbynames []) ()={
		getrpcbyname_local,
		getrpcbyname_yp,
		getrpcbyname_bind
};


/*
 * generic getrpc service routines
 */

setrpcent (f)
	int f;
{
	register i;

	svc_getrpcflag = -1;
	svc_getrpcbind = 0;
	if ((svcinfo = getsvc()) != NULL)
		for (i=0; (svc_lastlookup = svcinfo->svcpath[SVC_RPC][i]) != SVC_LAST; i++)
			(*(setrpcents [svcinfo->svcpath[SVC_RPC][i]])) (f);
}

endrpcent ()
{
	register i;

	svc_getrpcflag = -1;
	svc_getrpcbind = 0;
	if ((svcinfo = getsvc()) != NULL)
		for (i=0; (svc_lastlookup = svcinfo->svcpath[SVC_RPC][i]) != SVC_LAST; i++)
			(*(endrpcents [svcinfo->svcpath[SVC_RPC][i]])) ();
}

struct rpcent *
getrpcent()
{
	register struct rpcent *p=NULL;
	register i;

	/*
	 * Check if setrpcent was not made yet
	 */
	if (svc_getrpcflag == -2)
		setrpcent(0);
	/*
	 * Check if this is the first time through getrpcent
	 */
	if (svc_getrpcflag == -1) {
		/*
		 * If it is, init the svcinfo structure
		 */
		if ((svcinfo = getsvc()) == NULL)
			return((struct rpcent *)NULL);
		i = 0;
	}
	else {
		/*
		 * If it is not, set the index to the last used one
		 */
		i = svc_getrpcflag;
	}
	for (; (svc_lastlookup = svcinfo->svcpath[SVC_RPC][i]) != SVC_LAST; i++)
		if (p = ((*(getrpcents [svcinfo->svcpath[SVC_RPC][i]])) () )) {
			svc_getrpcflag = i;
			break;
		}
	return (p);
}

struct rpcent *
getrpcbyname (name)
	register char *name;
{
	register struct rpcent *p=NULL;
	register i;

	/* avoid null pointer de-reference on mips */
	if (name == 0)
		return(0);
	if ((svcinfo = getsvc()) != NULL)
		for (i=0; (svc_lastlookup = svcinfo->svcpath[SVC_RPC][i]) != SVC_LAST; i++)
			if (p = ((*(getrpcbynames [svcinfo->svcpath[SVC_RPC][i]])) (name) ))
				break;
	return (p);
}

struct rpcent *
getrpcbynumber (number)
	register int number;
{
	register struct rpcent *p=NULL;
	register i;

	if ((svcinfo = getsvc()) != NULL)
		for (i=0; (svc_lastlookup = svcinfo->svcpath[SVC_RPC][i]) != SVC_LAST; i++)
			if (p = ((*(getrpcbynumbers [svcinfo->svcpath[SVC_RPC][i]])) (number) ))
				break;
	return (p);
}

/*
 * specific getrpc service routines
 */

struct rpcent *
getrpcbynumber_local(number)
	register int number;
{
	register struct rpcent *p;

#ifdef DEBUG
	fprintf (stderr,"getrpcbynumber_local(%d)\n", number);
#endif DEBUG
	setrpcent_local(0);
	while (p = getrpcent_local()) {
		if (p->r_number == number)
			break;
	}
	endrpcent_local();
	return (p);
}

struct rpcent *
getrpcbynumber_bind(number)
	register int number;
{
#ifdef BINDOK
	char **pp, numbuf[12];

#ifdef DEBUG
	fprintf (stderr,"getrpcbynumber_bind(%d)\n", number);
#endif DEBUG
	setent_bind(0);
	sprintf(numbuf, "%d", number);
	pp = hes_resolve(numbuf, "rpc");
	endent_bind();
	return rpccommon(pp);
#endif BINDOK
}

struct rpcent *
getrpcbynumber_yp(number)
	register int number;
{
	register struct rpcent *p;
	int reason;
	char adrstr[10], *val = NULL;
	int vallen = 0;

#ifdef DEBUG
	fprintf (stderr,"getrpcbynumber_yp(%d)\n", number);
#endif DEBUG
	setrpcent_yp(0);
	sprintf(adrstr, "%d", number);
	if (reason = yp_match(domain, "rpc.bynumber", adrstr, strlen(adrstr), &val, &vallen)) {
#ifdef DEBUG
		fprintf(stderr, "reason yp_first failed is %d\n", reason);
#endif
		p = NULL;
	}
	else {
		p = interpret(val, vallen, SVC_YP);
		free(val);
	}
	endrpcent_yp();
	return (p);
}

struct rpcent *
getrpcbyname_local(name)
	char *name;
{
	register struct rpcent *rpc=NULL;
	register char **rp;

#ifdef DEBUG
	fprintf (stderr,"getrpcbyname_local(%s)\n", name);
#endif DEBUG
	setrpcent_local(0);
	while (rpc = getrpcent_local()) {
		if (strcmp(rpc->r_name, name) == 0)
			break;
		for (rp = rpc->r_aliases; *rp != 0; rp++)
			if (strcmp(*rp, name) == 0)
				goto found;
	}
found:
	endrpcent_local();
	return (rpc);
}

struct rpcent *
getrpcbyname_bind(name)
	char *name;
{
#ifdef BINDOK
	char **pp;

#ifdef DEBUG
	fprintf (stderr,"getrpcbyname_bind(%s)\n", name);
#endif DEBUG
	setent_bind(0);
	pp = hes_resolve(name, "rpc");
	endent_bind();
	return rpccommon(pp);
#endif BINDOK
}

struct rpcent *
getrpcbyname_yp(name)
	char *name;
{
	register struct rpcent *rpc=NULL;
	register char **rp;

#ifdef DEBUG
	fprintf (stderr,"getrpcbyname_yp(%s)\n", name);
#endif DEBUG
	setrpcent_yp(0);
	while (rpc = getrpcent_yp()) {
		if (strcmp(rpc->r_name, name) == 0)
			break;
		for (rp = rpc->r_aliases; *rp != 0; rp++)
			if (strcmp(*rp, name) == 0)
				goto found2;
	}
found2:
	endrpcent_yp();
	return (rpc);
}

setrpcent_local(f)
	int f;
{
	if (rpcf == NULL)
		rpcf = fopen(RPCDB, "r");
	else
		rewind(rpcf);
	stayopen |= f;
}

/*
 * setent_bind(f) is in getcommon.c
 */

setrpcent_yp(f)
{
	if ((domain = yellowup(1)) == NULL)
		return(setrpcent_local(f));
	if (current)
		free(current);
	current = NULL;
}

endrpcent_local()
{
	if (rpcf && !stayopen) {
		fclose(rpcf);
		rpcf = NULL;
	}
}

/*
 * endent_bind() is in getcommon.c
 */

endrpcent_yp()
{
	if (current && !stayopen) {
		free(current);
		current = NULL;
	}
}

struct rpcent *
getrpcent_local()
{
	static char *line1 = NULL;
#ifdef _DEBUG
	printf ("l ");
#endif
	if (rpcf == NULL && (rpcf = fopen(RPCDB, "r")) == NULL)
		return (NULL);
        if(line1 == NULL) {
                line1 = (char *)malloc(LINE_MAX);
                assert(line1);
        }
        if (fgets(line1, LINE_MAX, rpcf) == NULL)
		return (NULL);
	return interpret(line1, strlen(line1), SVC_LOCAL);
}

struct rpcent *
getrpcent_bind()
{
#ifdef BINDOK
	char bindbuf[64];
	struct rpcent *hp = NULL;

	sprintf(bindbuf, "rpc-%d", svc_getrpcbind);
#ifdef DEBUG
	fprintf(stderr, "getrpcent_bind(%s)\n", bindbuf);
#endif DEBUG
	if ((hp = getrpcbyname_bind(bindbuf)) == NULL)
		return(NULL);
	svc_getrpcbind++;
	return(hp);
#endif BINDOK
}

struct rpcent *
getrpcent_yp()
{
	struct rpcent *hp;
	int reason;
	char *key = NULL;
	char *val = NULL;
	int keylen = 0;
	int vallen = 0;

#ifdef _DEBUG
	printf("yp ");
#endif
	if ((domain = yellowup(0)) == NULL)
		return(getrpcent_local());
	if (current == NULL) {
		if (reason =  yp_first(domain, "rpc.bynumber", &key, &keylen, &val, &vallen)) {
#ifdef DEBUG
			fprintf(stderr, "reason yp_first failed is %d\n", reason);
#endif
			return NULL;
		}
	}
	else {
		if (reason = yp_next(domain, "rpc.bynumber", current, currentlen, &key, &keylen, &val, &vallen)) {
#ifdef DEBUG
			fprintf(stderr, "reason yp_next failed is %d\n", reason);
#endif
			return NULL;
		}
	}
	if (current)
		free(current);
	current = key;
	currentlen = keylen;
	hp = interpret(val, vallen, SVC_YP);
	free(val);
	return (hp);
}

static struct rpcent *
interpret(val, len, svc)
	char *val;
	int len, svc;
{
	static char *rpc_aliases[MAXALIASES];
	static struct rpcent rpc;
	static char *line = NULL;
	char *p;
	register char *cp, **q;

	if(line == NULL) {
                line = (char *)malloc(LINE_MAX);
                assert(line);
        }
	strncpy(line, val, len);
	p = line;
	line[len] = '\n';
	if (*p == '#')
		switch (svc) {
			case SVC_LOCAL:
				return (getrpcent_local());
			case SVC_YP:
				return (getrpcent_yp());
		}
	cp = getcommon_any(p, "#\n");
	if (cp == NULL)
		switch (svc) {
			case SVC_LOCAL:
				return (getrpcent_local());
			case SVC_YP:
				return (getrpcent_yp());
		}
	*cp = '\0';
	cp = getcommon_any(p, " \t");
	if (cp == NULL)
		switch (svc) {
			case SVC_LOCAL:
				return (getrpcent_local());
			case SVC_YP:
				return (getrpcent_yp());
		}
	*cp++ = '\0';
	/* THIS STUFF IS INTERNET SPECIFIC */
	rpc.r_name = line;
	while (*cp == ' ' || *cp == '\t')
		cp++;
	rpc.r_number = atoi(cp);
	q = rpc.r_aliases = rpc_aliases;
	cp = getcommon_any(cp, " \t");
	if (cp != NULL) 
		*cp++ = '\0';
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &rpc_aliases[MAXALIASES - 1])
			*q++ = cp;
		cp = getcommon_any(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	return (&rpc);
}

#ifdef BINDOK
static
struct rpcent *
rpccommon(pp)
char **pp;
{
	static char *rpc_aliases[MAXALIASES];
        register char *p, **q;

        if (pp == NULL)
                return(NULL);
        /* choose only the first response (only 1 expected) */
        strcpy(buf, pp[0]);
        while(*pp) free(*pp++); /* necessary to avoid leaks */
        p = buf;
        rpc.r_name = p;
        p = rpcskip(p);
        rpc.r_number = atoi(p);
	q = rpc.r_aliases = rpc_aliases;
	while (*p && (p = rpcskip(p)) != 0) {
		if (q < &rpc_aliases[MAXALIASES - 1])
			*q++ = p;
	}
        while (*p && *p != '\n')
                p++;
        *p = '\0';
        return(&rpc);
}

static
char *
rpcskip(p)
register char *p;
{
        while (*p && *p != ':' && *p != '\n')
                ++p;
        if (*p)
                *p++ = 0;
        return(p);
}
#endif

