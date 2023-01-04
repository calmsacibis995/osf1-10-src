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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: hash.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/10 19:47:37 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * hash.c - hash table functions
 *
 * DESCRIPTION
 *
 *	These routines are provided to manipulate the hashed list
 *	of filenames in an achive we are updating.
 *
 * AUTHOR
 *
 *     Tom Jordahl - The Open Software Foundation
 *
 *
 */

/* Headers */

#include "pax.h"


static	Hashentry	*hash_table[HTABLESIZE]; 

/* hash_name - enter a name into the table
 *
 * DESCRIPTION
 *
 *	hash_name places an entry into the hash table.  The name
 *	is hashed in to a 16K table of pointers by adding all the
 *	byte values in the pathname modulo 16K.  This gives us a
 *	moderatly size table without too much memory usage.
 *	
 *	Will update the mtime of an entry which already exists.
 *
 * PARAMETERS
 *
 *	char 	*name 	- Name to be hashed
 *	Stat	*sb	- stat buffer to get the mtime out of
 *
 */


void hash_name(char *name, Stat *sb)

{
    Hashentry	*entry;
    Hashentry	*hptr;
    char  	*p;
    uint	 total=0;


    p = name;
    while (*p != '\0') {
	total += *p; 
	p++;
    }

    total = total % HTABLESIZE;

    if ((entry = (Hashentry *)mem_get(sizeof(Hashentry))) == NULL) {
	fatal(MSGSTR(NOMEM, "Out of memory"));
    }

    if ((hptr = hash_table[total]) != NULL) {
	while (hptr->next != NULL) {
	    if (!strcmp(name, hptr->name)) {
		hptr->mtime =  sb->sb_mtime;
		free(entry);
		return;
	    }
	    hptr = hptr->next;
	}
	hptr->next = entry;
    } else {
	hash_table[total] = entry;
    }

    entry->name = mem_str(name);
    entry->mtime = sb->sb_mtime;
    entry->next = NULL;

}


/* hash_lookup - lookup the modification time of a file in hash table
 *
 * DESCRIPTION
 *
 *	Check the hash table for the given filename and returns the
 *	modification time stored in the table, -1 otherwise.
 *
 * PARAMETERS
 *
 *	char *name 	- name of file to lookup
 *
 * RETURNS
 *
 *	modification time found in the hash table.
 *	-1 if name isn't found.
 *
 */


time_t hash_lookup(char *name)

{
    char	*p;
    uint	 total=0;
    Hashentry	*hptr;

    p = name;
    while (*p != '\0') {
	total += *p; 
	p++;
    }

    total = total % HTABLESIZE;
   
    if ((hptr = hash_table[total]) == NULL) 
	return((time_t) -1);

    while (hptr != NULL) {
	if (!strcmp(name, hptr->name)) 
	    return(hptr->mtime);	/* found it */
	hptr = hptr->next;
    }

    return((time_t) -1);		/* not found */
}
