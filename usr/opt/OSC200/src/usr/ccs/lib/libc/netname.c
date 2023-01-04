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
static char     *sccsid = "@(#)$RCSfile: netname.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/09/03 18:31:29 $";
#endif
/*
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak getnetname = __getnetname
#pragma weak host2netname = __host2netname
#pragma weak netname2host = __netname2host
#pragma weak netname2user = __netname2user
#pragma weak user2netname = __user2netname
#endif
#ifdef stat
#undef stat
#endif
#endif

/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.9 88/02/08 
 */

/*
 * netname utility routines
 * convert from unix names to network names and vice-versa
 * This module is operating system dependent!
 * What we define here will work with any unix system that has adopted
 * the Sun NIS domain architecture.
 */
#include <sys/param.h>
#include <rpc/rpc.h>
#include <ctype.h>
#include <stdio.h>

extern char *strncpy();

static char *OPSYS = "unix";
static char *NETID = "netid.byname";

/*
 * Convert network-name into unix credential
 */
netname2user(netname, uidp, gidp, gidlenp, gidlist)
	char netname[MAXNETNAMELEN+1];
	int *uidp;
	int *gidp;
	int *gidlenp;
	int *gidlist;
{
	int stat;
	char *val;
	char *p;
	int vallen;
	char *domain;
	int gidlen;

	stat = yp_get_default_domain(&domain);
	if (stat != 0) {
		return (0);
	}
	stat = yp_match(domain, NETID, netname, strlen(netname), &val, &vallen);
	if (stat != 0) {
		return (0);
	}
	val[vallen] = 0;
	p = val;
	*uidp = atois(&p);
	if (p == NULL || *p++ != ':') {
		free(val);
		return (0);
	}
	*gidp = atois(&p);
	if (p == NULL) {
		free(val);
		return (0);
	}
	gidlen = 0;
	for (gidlen = 0; gidlen < NGROUPS; gidlen++) {	
		if (*p++ != ',') {
			break;
		}
		gidlist[gidlen] = atois(&p);
		if (p == NULL) {
			free(val);
			return (0);
		}
	}
	*gidlenp = gidlen;
	free(val);
	return (1);
}

/*
 * Convert network-name to hostname
 */
netname2host(netname, hostname, hostlen)
	char netname[MAXNETNAMELEN+1];
	char *hostname;
	int hostlen;
{
	int stat;
	char *val;
	int vallen;
	char *domain;

	stat = yp_get_default_domain(&domain);
	if (stat != 0) {
		return (0);
	}
	stat = yp_match(domain, NETID, netname, strlen(netname), &val, &vallen);
	if (stat != 0) {
		return (0);
	}
	val[vallen] = 0;
	if (*val != '0') {
		free(val);
		return (0);
	}	
	if (val[1] != ':') {
		free(val);
		return (0);
	}
	(void) strncpy(hostname, val + 2, hostlen);
	free(val);
	return (1);
}


/*
 * Figure out my fully qualified network name
 */
getnetname(name)
	char name[MAXNETNAMELEN+1];
{
	int uid;

	uid = geteuid(); 
	if (uid == 0) {
		return (host2netname(name, (char *) NULL, (char *) NULL));
	} else {
		return (user2netname(name, uid, (char *) NULL));
	}
}


/*
 * Convert unix cred to network-name
 */
user2netname(netname, uid, domain)
	char netname[MAXNETNAMELEN + 1];
	int uid;
	char *domain;
{
	char *dfltdom;

#define MAXIPRINT	(11)	/* max length of printed integer */

	if (domain == NULL) {
		if (yp_get_default_domain(&dfltdom) != 0) {
			return (0);
		}
		domain = dfltdom;
	}
	if (strlen(domain) + 1 + MAXIPRINT > MAXNETNAMELEN) {
		return (0);
	}
	(void) sprintf(netname, "%s.%d@%s", OPSYS, uid, domain);	
	return (1);
}


/*
 * Convert host to network-name
 */
host2netname(netname, host, domain)
	char netname[MAXNETNAMELEN + 1];
	char *host;
	char *domain;
{
	char *dfltdom;
	char hostname[MAXHOSTNAMELEN+1]; 

	if (domain == NULL) {
		if (yp_get_default_domain(&dfltdom) != 0) {
			return (0);
		}
		domain = dfltdom;
	}
	if (host == NULL) {
		(void) gethostname(hostname, sizeof(hostname));
		host = hostname;
	}
	if (strlen(domain) + 1 + strlen(host) > MAXNETNAMELEN) {
		return (0);
	} 
	(void) sprintf(netname, "%s.%s@%s", OPSYS, host, domain);
	return (1);
}


static
atois(str)
	char **str;
{
	char *p;
	int n;
	int sign;

	if (**str == '-') {
		sign = -1;
		(*str)++;
	} else {
		sign = 1;
	}
	n = 0;
	for (p = *str; isdigit(*p); p++) {
		n = (10 * n) + (*p - '0');
	}
	if (p == *str) {
		*str = NULL;
		return (0);
	}
	*str = p;	
	return (n * sign);
}
