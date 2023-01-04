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
static char *rcsid = "@(#)$RCSfile: buildtab.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/05/05 17:09:57 $";
#endif

/*
 * buildtab.c
 * Build screen tables from individual entries
 *
 * 19 December 1988	Jeffrey Mogul/DECWRL
 *	Created.
 *	Copyright 1989, 1990 Digital Equipment Corporation
 */

#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include "screentab.h"

int	debug_build = 0;
extern int semantic_errors;

InitTables()
{
	InitNetmaskTable();
	InitActionTable();
}

StoreNetmask(nmp)
register struct NetmaskData *nmp;
{
	if (debug_build)
	    PrintNetmaskData(nmp);
	NetMaskInsert(nmp);
}

StoreAction(ap)
register struct ActionSpec *ap;
{
	if (debug_build)
	    PrintActionSpec(ap);
	ActionInsert(ap);
}

/*
 * Netmask stuff is stored in a hash table where the "hash" function
 * is trivial, because we don't expect to have many entries.
 */

#define	CHEAT(ia)	(ia).addr_un.S_un_b
#define	HASHSIZE (1<<8)		/* 8 bits of result */
#define	HASHFUNC(ia) \
	(CHEAT(ia).s_b1 ^ CHEAT(ia).s_b2 ^ CHEAT(ia).s_b3 ^ CHEAT(ia).s_b4)

struct nm_hashentry {
	struct NetmaskData nmdata;
	struct nm_hashentry *next;	/* chain of entries */
};

struct nm_hashentry *nm_hashtable[HASHSIZE];

InitNetmaskTable()
{
	bzero(nm_hashtable, sizeof(nm_hashtable));
}

NetMaskInsert(nmp)
register struct NetmaskData *nmp;
{
	register int i;
	struct in_addr_byte j;
	register struct nm_hashentry *hp;
	register struct nm_hashentry *hep;
	
	j.addr_un.s_addr = nmp->network.s_addr;
	i = HASHFUNC(j); 
	hp = (struct nm_hashentry *)malloc(sizeof(*hp));
	if (hp == NULL) {
	    perror("NetMaskInsert/malloc");
	    exit(1);
	}
	hp->nmdata = *nmp;
	hp->next = NULL;

	if ((hep = nm_hashtable[i]) == NULL) {	/* easy */
	    nm_hashtable[i] = hp;
	    return;
	}
	
	/* collision, must search for end of chain */
	do {
	    if (hep->nmdata.network.s_addr == nmp->network.s_addr) {
		yyerror("duplicate netmask information for network");
		semantic_errors++;
		return;
	    }
	    if (hep->next == NULL)
		break;
	    hep = hep->next;
	} while (1);
	
	hep->next = hp;
}

/*
 * Returns value by reference; returns true iff success
 */
NetMaskLookup(netp, maskp)
register struct in_addr *netp;
register struct in_addr *maskp;
{
	register int i;
	struct in_addr_byte j;
	register struct nm_hashentry *hp;

	j.addr_un.s_addr = netp->s_addr;
	i = HASHFUNC(j); 
	hp = nm_hashtable[i];
	
	while (hp) {
	    if (hp->nmdata.network.s_addr == netp->s_addr) {
		*maskp = hp->nmdata.mask;
		return(1);
	    }
	    hp = hp->next;
	}
	return(0);	/* not found */
}

DumpNetMaskTable()
{
	register int i;
	register struct nm_hashentry *hp;

	printf("Netmask hash table:\n");
	for (i = 0; i < HASHSIZE; i++) {
	    if ((hp = nm_hashtable[i]) == NULL)
		continue;
	    printf("[hash %d] ", i);
	    while (hp) {
		printf(" (%s, ", inet_ntoa(hp->nmdata.network));
		printf("%s)", inet_ntoa(hp->nmdata.mask));
		hp = hp->next;
	    }
	    printf("\n");
	}
}
