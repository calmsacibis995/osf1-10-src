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
static char *rcsid = "@(#)$RCSfile: screenit.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/08/14 12:25:23 $";
#endif

/*
 * screenit.c
 * Decide if a datagram shall pass.
 *
 * 21 December 1988	Jeffrey Mogul/DECWRL
 *	Created.
 *	Copyright 1989, 1990 Digital Equipment Corporation
 */

#include <sys/types.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <stdio.h>
#include "screentab.h"

extern int debug;
extern int LastMatchRule;
int disable_cache = 0;
int LastCacheHit = 0;		/* for logging cache hits */

/*
 * A small lookaside cache for complete matches
 */

#define	CACHESIZE	16

struct cache_entry {
	struct unpacked_hdrs	hdrs;
	int			action;
	int			age;
	int			rule;
};

struct cache_entry Cache[CACHESIZE];
		/* ASSUMPTION: initialized to all zeros */

u_int cache_time = 0;	/* pseudo-age for LRU */

ScreenIt(ahp)
register struct annotated_hdrs *ahp;
{
	register int i;
	register struct cache_entry *cep;
	register struct cache_entry *eldest;
	register int action;
	
	if (debug)
	    printf("checking cache:\n");
	cep = Cache;
	eldest = Cache;
	for (i = 0; i < CACHESIZE; i++, cep++) {
	    /* keep track of oldest entry */
	    if (cep->age < eldest->age)
		eldest = cep;

	    if (debug) {
		printf("\tage %d ", cep->age);
		PrintUnpackedHdrs(&(cep->hdrs));
		printf("\n");
	    }
	    /* check for match on all components */
	    if (cep->hdrs.src.addr.s_addr != ahp->hdrs.src.addr.s_addr)
		continue;
	    if (cep->hdrs.dst.addr.s_addr != ahp->hdrs.dst.addr.s_addr)
		continue;
	    if (cep->hdrs.proto != ahp->hdrs.proto)
		continue;
	    /* next two may not always be valid but they always must match */
	    if (cep->hdrs.src.port != ahp->hdrs.src.port)
		continue;
	    if (cep->hdrs.dst.port != ahp->hdrs.dst.port)
		continue;

	    /* got here = match */
	    if (debug) {
		printf("match\n");
	    }
	    LastCacheHit = 1;
	    cep->age = cache_time++;	/* update LRU timer */
	    LastMatchRule = cep->rule;	/* restore rule info */
	    return(cep->action);
	}
	
	if (debug) {
	    printf("no match\n");
	}
	LastCacheHit = 0;

	/* got here = not in cache */
	action = MatchAction(ahp);
	
	if (disable_cache == 0) {
	    /* replace eldest cache entry */
	    eldest->hdrs = ahp->hdrs;
	    eldest->action = action;
	    eldest->age = cache_time++;
	    eldest->rule = LastMatchRule;
	}
	
	return(action);
}
