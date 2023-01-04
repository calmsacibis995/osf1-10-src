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
static char     *sccsid = "@(#)$RCSfile: getnetent_l.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/10/05 21:01:40 $";
#endif
/*
 */

/* This is the local only version of network name lookups, for use
 * by STANDALONE utilities.  It only looks at /etc/networks
 *
 * WARNING:
 * This code is cloned from getnetent.c, any changes made here
 * should made over there too.
 *
 *	getnetbyaddr_local()
 *	getnetbyname_local()
 *
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak endnetent_local = __endnetent_local
#pragma weak getnetbyaddr_local = __getnetbyaddr_local
#pragma weak getnetbyname_local = __getnetbyname_local
#pragma weak getnetent_local = __getnetent_local
#pragma weak setnetent_local = __setnetent_local
#endif
#undef getcommon_any
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <malloc.h>
#include <assert.h>
#include <limits.h>

#define	MAXALIASES	35

static char NETDB[] = "/etc/networks";
static int stayopen;
static FILE *netf = NULL;
static struct netent *interpret_local();
struct netent *getnetent_local();
static char *getcommon_any();

struct netent *
getnetbyaddr_local(net, type)	/* needs to be global */
{
	register struct netent *p;

	setnetent_local(0);
	while (p = getnetent_local()) {
		if (p->n_addrtype == type && p->n_net == net)
			break;
	}
	endnetent_local();
	return (p);
}

struct netent *
getnetbyname_local(name)	/* needs to be global */
	register char *name;
{
	register struct netent *p;
	register char **cp;

	setnetent_local(0);
	while (p = getnetent_local()) {
		if (strcmp(p->n_name, name) == 0)
			break;
		for (cp = p->n_aliases; *cp != 0; cp++)
			if (strcmp(*cp, name) == 0)
				goto found;
	}
found:
	endnetent_local();
	return (p);
}

setnetent_local(f)
	int f;
{
	if (netf == NULL)
		netf = fopen(NETDB, "r");
	else
		rewind(netf);
	stayopen |= f;
}

endnetent_local()
{
	if (netf && !stayopen) {
		fclose(netf);
		netf = NULL;
	}
}

struct netent *
getnetent_local()
{
	static char *line1 = NULL;

	if (netf == NULL && (netf = fopen(NETDB, "r")) == NULL)
		return (NULL);
	if(line1 == NULL) {
                line1 = (char *)malloc(LINE_MAX);
                assert(line1);
        }
        if (fgets(line1, LINE_MAX, netf) == NULL)
		return (NULL);
	return interpret_local(line1, strlen(line1));
}

/* cloned from interpret() in getnetent.c, but with SVC_LOCAL hard-coded */
static struct netent *
interpret_local(val, len)
	char *val;
	int len;
{
	static char *net_aliases[MAXALIASES];
	static struct netent net;
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
		return (getnetent_local());
	cp = getcommon_any(p, "#\n");
	if (cp == NULL)
		return (getnetent_local());
	*cp = '\0';
	net.n_name = p;
	cp = getcommon_any(p, " \t");
	if (cp == NULL)
		return (getnetent_local());
	*cp++ = '\0';
	while (*cp == ' ' || *cp == '\t')
		cp++;
	p = getcommon_any(cp, " \t");
	if (p != NULL)
		*p++ = '\0';
	net.n_net = inet_network(cp);
	net.n_addrtype = AF_INET;
	q = net.n_aliases = net_aliases;
	if (p != NULL) 
		cp = p;
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &net_aliases[MAXALIASES - 1])
			*q++ = cp;
		cp = getcommon_any(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	return (&net);
}

/* source cloned from getcommon_any() in getcommon.c */
static char *
getcommon_any(cp, match)
	register char *cp;
	char *match;
{
	register char *mp, c;

	while (c = *cp) {
		for (mp = match; *mp; mp++)
			if (*mp == c)
				return (cp);
		cp++;
	}
	return ((char *)0);
}
