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
static char *rcsid = "@(#)$RCSfile: acttab.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/05/11 21:37:09 $";
#endif

/*
 * acttab.c
 * Manage and search the action table
 *
 * 21 December 1988	Jeffrey Mogul/DECWRL
 *	Created.
 *	Copyright 1989, 1990 Digital Equipment Corporation
 */

#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include "screentab.h"

extern int debug;
extern int semantic_errors;
extern int defaction;

int LastMatchRule;

/*
 * Actions are stored in an array, in order of appearance in the
 * configuration file.
 *
 */

struct ActionSpec *acttab = 0;
int acttabsize = 0;
int acttabcount = 0;

InitActionTable()
{
	acttabsize = 1;		/* XXX change this after debugging XXX */
	acttab = (struct ActionSpec *)malloc(acttabsize * sizeof(*acttab));
	if (acttab == NULL) {
	    perror("InitActionTable/malloc");
	    exit(1);
	};
}

ExpandActionTable()
{
	register struct ActionSpec *new;
	register int newsize;
	
	newsize = acttabsize * 2;
	new = (struct ActionSpec *)malloc(newsize * sizeof(*new));
	if (new == NULL) {
	    perror("ExpandActionTable/malloc");
	    exit(1);
	}
	bcopy(acttab, new, acttabsize * sizeof(*new));
	free(acttab);
	acttab = new;
	acttabsize = newsize;
}

ActionInsert(ap)
register struct ActionSpec *ap;
{
	/* Make sure that "from" and "to" have the same proto field */
	if (ap->from.pspec.proto != ap->to.pspec.proto) {
	    if (ap->from.pspec.proto && (ap->to.pspec.proto == 0)) {
		ap->to.pspec.proto = ap->from.pspec.proto;
		if (ap->from.pspec.proto == IPPROTO_ICMP) { /* see below */
		    ap->to.pspec.pval.code = ap->from.pspec.pval.code;
		}
	    }
	    else if ((ap->from.pspec.proto == 0) && ap->to.pspec.proto) {
		ap->from.pspec.proto = ap->to.pspec.proto;
		if (ap->to.pspec.proto == IPPROTO_ICMP) { /* see below */
		    ap->from.pspec.pval.code = ap->to.pspec.pval.code;
		}
	    }
	    else {
		yyerror("from and to must have same protocol");
		semantic_errors++;
		return;
	    }
	}

	/* A little extra hackery for ICMP (blecch!) */
	if (ap->from.pspec.proto == IPPROTO_ICMP) {
	    /* make sure that at most one ICMP code is specified */
	    if ((ap->from.pspec.pval.code != ICMPV_ANY)
			 && (ap->to.pspec.pval.code != ICMPV_ANY) &&
		((ap->from.pspec.pval.code) != (ap->to.pspec.pval.code))) {
		yyerror("cannot specify two different ICMP types");
		semantic_errors++;
		return;
	    }
	}

	if (acttabcount >= acttabsize)
	    ExpandActionTable();
	acttab[acttabcount] = *ap;
	acttabcount++;
}

DumpActionTable()
{
	register int i;
		
	printf("Action table: (%d/%d slots full)\n",
		acttabcount, acttabsize);
	for (i = 0; i < acttabcount; i++) {
	    printf("[%d] ", i);
	    PrintActionSpec(&(acttab[i]));
	}
}

MatchAction(ahp)
register struct annotated_hdrs *ahp;
{
	register int i;
	register struct ActionSpec *asp;
	register int match;
		
	asp = acttab;
	for (i = 0; i < acttabcount; i++, asp++) {
	    if (debug) {
		printf("checking [%d] ", i);
		PrintActionSpec(asp);
	    }
	    
	    /* Check "from" ObjectSpec */
	    match = MatchObject(&(ahp->hdrs.src), ahp->hdrs.proto,
			&(ahp->srcnote), &(asp->from));
	    if (!match) {
		if (debug)
		    printf("src wrong\n");
		continue;
	    }

	    /* Check "to" ObjectSpec */
	    match = MatchObject(&(ahp->hdrs.dst), ahp->hdrs.proto,
			&(ahp->dstnote), &(asp->to));
	    if (!match) {
		if (debug)
		    printf("dst wrong\n");
		continue;
	    }

	    /* it matches! */
	    if (debug) {
		printf("match\n");
	    }
	    LastMatchRule = i;
	    return(asp->action);
	}

	/* not found */
	LastMatchRule = -1;
	return(defaction);
}

/*
 * Returns true if the header matches the object;
 * may fill in the annotation fields if necessary (lazy evaluation)
 */
MatchObject(uhp, proto, anp, osp)
register struct unpacked_hdr *uhp;
int proto;
register struct annotation *anp;
register struct ObjectSpec *osp;
{
	register int osp_port;
	register int matchval;
	
	switch (osp->aspec.addrtype) {
	case ASAT_ANY:
		matchval = 1;
		break;

	case ASAT_HOST:
		matchval = (osp->aspec.aval.host.s_addr == uhp->addr.s_addr);
		break;

	case ASAT_NET:
		if (anp->net.s_addr == INADDR_ANY) {
		    /* initialize this annotation */
		    netextract(&(uhp->addr), &(anp->net));
		}
		matchval = (osp->aspec.aval.network.s_addr == anp->net.s_addr);
		break;

	case ASAT_SUBNET:
		if (anp->net.s_addr == INADDR_ANY) {
		    /* must initialize this annotation before subnet */
		    netextract(&(uhp->addr), &(anp->net));
		}
		if (anp->subnet.s_addr == INADDR_ANY) {
		    /* now initialize this annotation */
		    subnetextract(&(uhp->addr), &(anp->net), &(anp->subnet));
		}
		matchval =
			(osp->aspec.aval.subnet.s_addr == anp->subnet.s_addr);
		break;

	default:
	    fprintf(stderr,
		"internal error: MatchObject/unknown addrtype %d\n",
			osp->aspec.addrtype);
	    exit(1);
	}
	
	/* So far, we know if the address part matches */
	if (osp->flags & OSF_NOTADDR) {	/* if looking for non-match addr */
	    if (matchval)	/* but it does match */
		return(0);	/* so return false */
	}
	else {				/* looking for matching addr */
	    if (matchval == 0)	/* but it doesn't match */
		return(0);	/* so return false */
	}
	
	if (osp->pspec.proto) {
	    /* proto is specified, check it */
	    matchval = (osp->pspec.proto == proto);
	}
	else
	    matchval = 1;	/* any protocol matches */

	/* Now we know if the protocol matches */
	if (osp->flags & OSF_NOTPROTO) { /* if looking for non-match proto */
	    if (matchval)	/* but it does match */
		return(0);	/* so return false */
	}
	else {				/* looking for matching proto */
	    if (matchval == 0)	/* but it doesn't match */
		return(0);	/* so return false */
	}

	/*
	 * Assumption here:
	 *	if this is ICMP then the code is either ICMPV_ANY or
	 *		we should match it
	 *	if this is none of (ICMP, UDP, TCP) then port.descrim is
	 *		PORTV_ANY for both source and dest
	 * Thus, we can safely use the following code no matter what
	 *	the protocol.
	 */
	if (proto == IPPROTO_ICMP) {
	    osp_port = osp->pspec.pval.code;
	    if (osp_port == ICMPV_ANY) {
		matchval = 1;		/* any ICMP matches */
	    }
	    else if (osp_port == ICMPV_ECHOREPLY) {
		/* special hack because of value collision */
		matchval = (uhp->port == ICMP_ECHOREPLY);
	    }
	    else if (osp_port == ICMPV_INFOTYPE) {
		/* is it an "infotype"? */
		matchval = ICMP_INFOTYPE(uhp->port);
	    }
	    else
		matchval = (uhp->port == osp_port);
	}
	else {
	    osp_port = osp->pspec.pval.port.value;
	    switch (osp->pspec.pval.port.discrim) {
	    case PORTV_EXACT:
		/* is it the specified port? */
		matchval = (uhp->port == osp_port);
		break;

	    case PORTV_RESERVED:
		/* Range comparisons must be done in host byte-order */
		osp_port = ntohs(uhp->port);
		/* is it "reserved"? */
		matchval = ((osp_port >= 0) && (osp_port < IPPORT_RESERVED));
		break;

	    case PORTV_XSERVER:
		/* Range comparisons must be done in host byte-order */
		osp_port = ntohs(uhp->port);
		/* is it in the range used by X-servers? */
		matchval = ((osp_port >= XSERVERPORT_MIN) &&
			     (osp_port < XSERVERPORT_MAX));
		break;

	    case PORTV_ANY:
	    default:
		matchval = 1;       /* any port in a storm */
		break;
	    }
	}

	/* Now we know if the port matches */
	if (osp->flags & OSF_NOTPORT) {	/* if looking for non-match port */
	    return(matchval == 0);	/* true if no match */
	}
	else {				/* looking for matching port */
	    return(matchval);		/* true if match */
	}
}

netextract(iap, netp)
register struct in_addr *iap;
register struct in_addr *netp;
{
	register u_int i = ntohl(iap->s_addr);
	
	if (IN_CLASSA(i))
	    netp->s_addr = htonl(i&IN_CLASSA_NET);
	else if (IN_CLASSB(i))
	    netp->s_addr = htonl(i&IN_CLASSB_NET);
	else if (IN_CLASSC(i))
	    netp->s_addr = htonl(i&IN_CLASSC_NET);
	else
	    netp->s_addr = i;	/* XXX wrong for Multicast? XXX */
}

subnetextract(iap, netp, subnetp)
register struct in_addr *iap;
register struct in_addr *netp;
register struct in_addr *subnetp;
{
	static struct in_addr mask;
	
	if (NetMaskLookup(netp, &mask)) {
	    subnetp->s_addr = iap->s_addr & mask.s_addr;
	}
	/* otherwise don't set the subnet value */
}
