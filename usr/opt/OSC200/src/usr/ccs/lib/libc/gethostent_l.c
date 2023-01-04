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
static char     *sccsid = "@(#)$RCSfile: gethostent_l.c,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/10/05 21:01:37 $";
#endif lint
/*
*/
/*
 * This is the local only version of hostname lookups, for use
 * by STANDALONE utilities.  It only looks at /etc/hosts.
 *
 * WARNING:
 * This code is cloned from gethostent.c, any changes made here
 * should made over there too.
 *
 *	gethostbyaddr_local()
 * 	gethostbyname_local()
 *	gethostent_local()
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak endhostent_local = __endhostent_local
#pragma weak gethostbyaddr_local = __gethostbyaddr_local
#pragma weak gethostbyname_local = __gethostbyname_local
#pragma weak gethostent_local = __gethostent_local
#pragma weak sethostent_local = __sethostent_local
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

#define MAXADDRS	35
#define	MAXALIASES	35

static char *getcommon_any();
static char HOSTDB[] = "/etc/hosts";
static struct hostent host;
static FILE *hostf = NULL;
static int stayopen;
static struct hostent *interpret_local();
static char hostaddr[MAXADDRS];
static char *host_aliases[MAXALIASES];
static char *host_addrs[2];
static char *line = NULL;

sethostent_local(f)
	int f;
{
	if (hostf == NULL)
		hostf = fopen(HOSTDB, "r");
	else
		rewind(hostf);
	stayopen |= f;
}

endhostent_local()
{
	if (hostf && !stayopen) {
		fclose(hostf);
		hostf = NULL;
	}
}

struct hostent *
gethostent_local()
{
	static char *line1 = NULL;

	if (hostf == NULL && (hostf = fopen(HOSTDB, "r")) == NULL)
		return (NULL);
        if(line1 == NULL) {
                line1 = (char *)malloc(LINE_MAX);
                assert(line1);
        }
        if (fgets(line1, LINE_MAX, hostf) == NULL)
		return (NULL);
	return interpret_local(line1, strlen(line1));
}

struct hostent *
gethostbyaddr_local(addr, len, type)	/* needs to be global */
	char *addr;
	register int len, type;
{
	register struct hostent *p=NULL;

	sethostent_local(0);
	while (p = gethostent_local()) {
		if (p->h_addrtype != type || p->h_length != len)
			continue;
		if (bcmp(p->h_addr, addr, len) == 0)
			break;
	}
	endhostent_local();
	return (p);
}

struct hostent *
gethostbyname_local(name)	/* needs to be global */
	register char *name;
{
	register struct hostent *p=NULL;
	register char **cp;

	sethostent_local(0);
	while (p = gethostent_local()) {
		if (strcmp(p->h_name, name) == 0)
			break;
		for (cp = p->h_aliases; *cp != 0; cp++)
			if (strcmp(*cp, name) == 0)
				goto found;
	}
found:
	endhostent_local();
	return (p);
}

/* this is cloned from interpret(), gethostent.c */
static struct hostent *
interpret_local(val, len)
	char *val;
	int len;
{
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
		return (gethostent_local());
	cp = getcommon_any(p, "#\n");
	if (cp == NULL)
		return (gethostent_local());
	*cp = '\0';
	cp = getcommon_any(p, " \t");
	if (cp == NULL)
		return (gethostent_local());
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

	host.h_addr_list    = host_addrs;	/* point to the list */
	host.h_addr_list[0] = hostaddr;		/* point to the host addrs */
	host.h_addr_list[1] = (char *)0;	/* and null terminate */
	*((u_int *)host.h_addr) = inet_addr(p);
	
	host.h_length = sizeof (u_int);
	host.h_addrtype = AF_INET;
	while (*cp == ' ' || *cp == '\t')
		cp++;
	host.h_name = cp;
	q = host.h_aliases = host_aliases;
	cp = getcommon_any(cp, " \t");
	if (cp != NULL) 
		*cp++ = '\0';
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &host_aliases[MAXALIASES - 1])
			*q++ = cp;
		cp = getcommon_any(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	return (&host);
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
