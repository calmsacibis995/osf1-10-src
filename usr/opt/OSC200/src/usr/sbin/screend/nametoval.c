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
static char *rcsid = "@(#)$RCSfile: nametoval.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1993/01/05 18:27:41 $";
#endif

/*
 * nametoval.c
 * Translations from an IP sort of name to a number
 *
 * 19 December 1988	Jeffrey Mogul/DECWRL
 *	Created.
 *	Copyright 1989, 1990 Digital Equipment Corporation
 */

#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <stdio.h>
#include "screentab.h"

NetNameToNumber(name)
char *name;
{
	struct netent *nep;
	
	nep = getnetbyname(name);
	if (nep == NULL) {
	    name_error(name, "net");
	    return(0);
	}
	return(nep->n_net);
}

MaskNameToNumber(name)
char *name;
{
	struct hostent *hep;
	struct in_addr ia;
	
	hep = gethostbyname(name);
	if (hep == NULL) {
	    name_error(name, "netmask");
	    return(0);
	}
	bcopy(hep->h_addr_list[0], &ia, sizeof(ia));
	return(ia.s_addr);
}

SubnetNameToNumber(name)
char *name;
{
	struct netent *nep;
	
	nep = getnetbyname(name);
	if (nep == NULL) {
	    name_error(name, "subnet");
	    return(0);
	}
	return(nep->n_net);
}

HostNameToNumber(name)
char *name;
{
	struct hostent *hep;
	struct in_addr ia;
	
	hep = gethostbyname(name);
	if (hep == NULL) {
	    name_error(name, "host");
	    return(0);
	}
	bcopy(hep->h_addr_list[0], &ia, sizeof(ia));
	if (hep->h_addr_list[1]) {
	    warn_multi(name, hep->h_addr_list[0]);
	    /* XXX fix this right! XXX */
	}
	return(ia.s_addr);
}

PortNameToNumber(name)
char *name;
{
	struct servent *sep;
	
	sep = getservbyname(name, NULL);	/* XXX Fishy XXX */
	if (sep == NULL) {
	    name_error(name, "port");
	    return(0);
	}
	return(sep->s_port);
}

char *
PortNumberToName(number)
int number;
{
	struct servent *sep;
	
	sep = getservbyport(number, NULL);	/* XXX Fishy XXX */
	if (sep == NULL) {
	    return(NULL);
	}
	return(sep->s_name);
}

ProtoNameToNumber(name)
char *name;
{
	struct protoent *pep;
	
	pep = getprotobyname(name);
	if (pep == NULL) {
	    name_error(name, "protocol");
	    return(0);
	}
	return(pep->p_proto);
}

char *
ProtoNumberToName(number)
char *number;
{
	struct protoent *pep;
	
	pep = getprotobynumber(number);
	if (pep == NULL) {
	    return(NULL);
	}
	return(pep->p_name);
}

struct ICMPName {
	char *name;
	char *abbrev;
	int type;
};
struct ICMPName ICMPNames[] = {
	/* XXX the long string names are pretty bad XXX */
	{"echoreply", "EchoRep", ICMP_ECHOREPLY},
	{"unreachable", "Unreach", ICMP_UNREACH},
	{"sourcequench", "Quench", ICMP_SOURCEQUENCH},
	{"redirect", "Redirect", ICMP_REDIRECT},
	{"echo", "EchoReq", ICMP_ECHO},
	{"timeexceeded", "TimeXcd", ICMP_TIMXCEED},
	{"parameterproblem", "Param", ICMP_PARAMPROB},
	{"timestamp", "TStampReq", ICMP_TSTAMP},
	{"timestampreply", "TStampRep", ICMP_TSTAMPREPLY},
	{"informationrequest", "InfoReq", ICMP_IREQ},
	{"informationrreply", "InfoRep", ICMP_IREQREPLY},
	{"addressmaskrequest", "MaskReq", ICMP_MASKREQ},
	{"addressmaskreply", "MaskRep", ICMP_MASKREPLY},
	{0,0}
};

ICMPNameToNumber(name)
register char *name;
{
	register struct ICMPName *icmpnp = ICMPNames;
	
	while (icmpnp->name) {
		if (strcmp(icmpnp->name, name) == 0) {
		    /* special hack to deal with value collision */
		    if (icmpnp->type == ICMP_ECHOREPLY)
			return(ICMPV_ECHOREPLY);
		    else
			return(icmpnp->type);
		}
		icmpnp++;
	}
	name_error(name, "ICMP type");
	return(0);
}

char *
ICMPNumberToName(number)
register int number;
{
	register struct ICMPName *icmpnp = ICMPNames;
	
	/* special hack to deal with value collision */
	if (number == ICMPV_ECHOREPLY)
	    number = ICMP_ECHOREPLY;

	while (icmpnp->name) {
		if (icmpnp->type == number)
			return(icmpnp->name);
		icmpnp++;
	}
	return(NULL);
}

char *
ICMPNumberToAbbrev(number)
register int number;
{
	register struct ICMPName *icmpnp = ICMPNames;
	
	while (icmpnp->name) {
		if (icmpnp->type == number)
			return(icmpnp->abbrev);
		icmpnp++;
	}
	return(NULL);
}

name_error(name, kind)
char *name;
char *kind;
{
	static char message[128];
	
	sprintf(message, "%s `%.64s' not found", kind, name);
	yyerror(message);
}

warn_multi(name, iap)
char *name;
struct in_addr *iap;
{
	static char message[128];
	
	sprintf(message,
	   "Warning: host %s has multiple addresses, using only [%s]", name,
					inet_ntoa(iap->s_addr));
	yywarn(message);
}
