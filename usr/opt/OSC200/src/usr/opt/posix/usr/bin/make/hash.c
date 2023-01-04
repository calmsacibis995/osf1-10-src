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
static char *rcsid = "@(#)$RCSfile: hash.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/29 22:02:03 $";
#endif

/*
 * Copyright (c) 1988, 1989, 1990 The Regents of the University of California.
 * Copyright (c) 1988, 1989 by Adam de Boor
 * Copyright (c) 1989 by Berkeley Softworks
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Adam de Boor.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* hash.c --
 *
 * 	This module contains routines to manipulate a hash table.
 * 	See hash.h for a definition of the structure of the hash
 * 	table.  Hash tables grow automatically as the amount of
 * 	information increases.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "alphaosf.h"
#include "hash.h"
#include "pmake_msg.h"

extern	nl_catd catd;


extern void *emalloc(size_t);

/*
 * Forward references to local procedures that are used before they're
 * defined:
 */

static void		RebuildTable(Hash_Table *t);

/* 
 * The following defines the ratio of # entries to # buckets
 * at which we rebuild the table to make it larger.
 */

#define rebuildLimit 8

/*
 * Trace macro used only during development for tracing hash module.
 * Use the compiler option -DHASHTRACE or -DALLTRACE to turn on.
 */

#if defined(HASHTRACE) || defined(ALLTRACE)
#undef HASHTRACE
#define HASHTRACE(message) { \
   printf("HASHTRACE: %s.\n",message);\
   }
#else
#define HASHTRACE(message)
#endif

/*
 *---------------------------------------------------------
 * 
 * Hash_InitTable --
 *
 *	This routine just sets up the hash table.
 *
 * Results:	
 *	None.
 *
 * Side Effects:
 *	Memory is allocated for the initial bucket area.
 *
 *---------------------------------------------------------
 */

void
Hash_InitTable(Hash_Table *t, int numBuckets)
{
	/* Hash_Table *t;	Structure to use to hold table. */
	/* int numBuckets;	How many buckets to create for starters. */
                                 
				/* This number is rounded up to a power of
				 * two.   If <= 0, a reasonable default is
				 * chosen. The table will grow in size later
				 * as needed. */

	int i;
	struct Hash_Entry **hp;

	HASHTRACE("Hash_InitTable: Set up the hash table");

	/*
	 * Round up the size to a power of two. 
	 */
	if (numBuckets <= 0)
		i = 16;
	else {
		for (i = 2; i < numBuckets; i <<= 1)
			 /* void */ ;
	}
	t->numEntries = 0;
	t->size = i;
	t->mask = i - 1;
	t->bucketPtr = hp = (struct Hash_Entry **)emalloc(sizeof(*hp) * i);
	while (--i >= 0)
		*hp++ = NULL;
}

/*
 *---------------------------------------------------------
 *
 * Hash_DeleteTable --
 *
 *	This routine removes everything from a hash table
 *	and frees up the memory space it occupied (except for
 *	the space in the Hash_Table structure).
 *
 * Results:	
 *	None.
 *
 * Side Effects:
 *	Lots of memory is freed up.
 *
 *---------------------------------------------------------
 */

void
Hash_DeleteTable(Hash_Table *t)
{
	/* Hash_Table *t; */

	struct Hash_Entry **hp, *h, *nexth;
	int i;

	HASHTRACE("Hash_DeleteTable: Remove everything from a hash table");


	for (hp = t->bucketPtr, i = t->size; --i >= 0;) {
		for (h = *hp++; h != NULL; h = nexth) {
			nexth = h->next;
			free((char *)h);
		}
	}
	free((char *)t->bucketPtr);

	/*
	 * Set up the hash table to cause memory faults on any future access
	 * attempts until re-initialization. 
	 */
	t->bucketPtr = NULL;
}

/*
 *---------------------------------------------------------
 *
 * Hash_FindEntry --
 *
 * 	Searches a hash table for an entry corresponding to key.
 *
 * Results:
 *	The return value is a pointer to the entry for key,
 *	if key was present in the table.  If key was not
 *	present, NULL is returned.
 *
 * Side Effects:
 *	None.
 *
 *---------------------------------------------------------
 */

Hash_Entry *
Hash_FindEntry(Hash_Table *t, char *key)
{
	/* Hash_Table *t;	Hash table to search. */
	/* char *key;		A hash key. */

	Hash_Entry *e;
	unsigned h;
	char *p;


	HASHTRACE("Hash_FindEntry: Searches a hash table for an entry corresponding to key");

	for (h = 0, p = key; *p;)
	  h = (h << 5) - h + *p++;

	p = key;

	for (e = t->bucketPtr[h & t->mask]; e != NULL; e = e->next)
		if (e->namehash == h && strcmp(e->name, p) == 0)
			return (e);
	return (NULL);
}

/*
 *---------------------------------------------------------
 *
 * Hash_CreateEntry --
 *
 *	Searches a hash table for an entry corresponding to
 *	key.  If no entry is found, then one is created.
 *
 * Results:
 *	The return value is a pointer to the entry.  If *newPtr
 *	isn't NULL, then *newPtr is filled in with TRUE if a
 *	new entry was created, and FALSE if an entry already existed
 *	with the given key.
 *
 * Side Effects:
 *	Memory may be allocated, and the hash buckets may be modified.
 *---------------------------------------------------------
 */

Hash_Entry *
Hash_CreateEntry(Hash_Table *t, char *key, Boolean *newPtr)
{
	/* Hash_Table *t;	     Hash table to search. */
	/* char *key;	             A hash key. */
	/* Boolean *newPtr;  Filled in with TRUE if new entry created, FALSE otherwise. */

	Hash_Entry *e;
	unsigned h;
	char *p;
	int keylen;
	struct Hash_Entry **hp;


	HASHTRACE("Hash_CreateEntry: Searches a hash table for an entry corresponding to key");

	/*
	 * Hash the key.  As a side effect, save the length (strlen) of the
	 * key in case we need to create the entry.
	 */

	for (h = 0, p = key; *p;) {
	  h = (h << 5) - h + *p++;
	}

	keylen = p - key;

	p = key;

	for (e = t->bucketPtr[h & t->mask]; e != NULL; e = e->next) {
		if (e->namehash == h && strcmp(e->name, p) == 0) {
			if (newPtr != NULL)
				*newPtr = FALSE;
			return (e);
		}
	}

	/*
	 * The desired entry isn't there.  Before allocating a new entry,
	 * expand the table if necessary (and this changes the resulting
	 * bucket chain). 
	 */

	HASHTRACE("Hash_CreateEntry: rebuildtable");

	if (t->numEntries >= rebuildLimit * t->size)
		RebuildTable(t);
	e = (Hash_Entry *) emalloc(sizeof(*e) + keylen);
	hp = &t->bucketPtr[h & t->mask];
	e->next = *hp;
	*hp = e;
	e->clientData = NULL;
	e->namehash = h;
	(void) strcpy(e->name, p);
	t->numEntries++;

	if (newPtr != NULL)
		*newPtr = TRUE;
	return (e);
}

/*
 *---------------------------------------------------------
 *
 * Hash_DeleteEntry --
 *
 * 	Delete the given hash table entry and free memory associated with
 *	it.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Hash chain that entry lives in is modified and memory is freed.
 *
 *---------------------------------------------------------
 */

void
Hash_DeleteEntry(Hash_Table *t, Hash_Entry *e)
{
	/* Hash_Table *t; */
	/* Hash_Entry *e; */

	Hash_Entry **hp, *p;
	char		     *errstr;


	HASHTRACE("Hash_DeleteEntry: Delete the given hash table entry");

	if (e == NULL)
		return;
	for (hp = &t->bucketPtr[e->namehash & t->mask];
	     (p = *hp) != NULL; hp = &p->next) {
		if (p == e) {
			*hp = p->next;
			free((char *)p);
			t->numEntries--;
			return;
		}
	}
	errstr = catgets(catd, MS_MAKE, BADCALL,
		 "bad call to Hash_DeleteEntry\n") ;
	(void) write(2, errstr, strlen(errstr)) ;
	abort();
}

/*
 *---------------------------------------------------------
 *
 * RebuildTable --
 *	This local routine makes a new hash table that
 *	is larger than the old one.
 *
 * Results:	
 * 	None.
 *
 * Side Effects:
 *	The entire hash table is moved, so any bucket numbers
 *	from the old table are invalid.
 *
 *---------------------------------------------------------
 */

static void
RebuildTable(Hash_Table *t)
{
	/* Hash_Table *t; */

	Hash_Entry *e, *next, **hp, **xp;
	int i, mask;
        Hash_Entry **oldhp;
	int oldsize;


	HASHTRACE("Hash RebuildTable: Make a new hash table larger than the old one");

	oldhp = t->bucketPtr;
	oldsize = i = t->size;
	i <<= 1;
	t->size = i;
	t->mask = mask = i - 1;
	t->bucketPtr = hp = (struct Hash_Entry **) emalloc(sizeof(*hp) * i);
	while (--i >= 0)
		*hp++ = NULL;
	for (hp = oldhp, i = oldsize; --i >= 0;) {
		for (e = *hp++; e != NULL; e = next) {
			next = e->next;
			xp = &t->bucketPtr[e->namehash & mask];
			e->next = *xp;
			*xp = e;
		}
	}
	free((char *)oldhp);
}
