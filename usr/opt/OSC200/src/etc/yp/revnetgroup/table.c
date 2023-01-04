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
static char     *sccsid = "@(#)$RCSfile: table.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 21:59:39 $";
#endif
/*
 */


#include <ctype.h>
#include "util.h"
#include "table.h"



/*
 * Hash table manager. Store/lookup strings, keyed by string
 */

/*
 * Generate the key into the table using the first two letters
 * of "str".  The table is alphabetized, with no distinction between
 * upper and lower case.  Non-letters are given least significance.
 */
int
tablekey(str)
	register char *str;
{
#	define TOLOWER(c) (islower(c) ? c : \
							(isupper(c) ? tolower(c) : ('a'+NUMLETTERS-1)))

	register int c1,c2;

	c1 = *str++;
	c2 = *str;
	if (c1 == EOS) {
		c2 = EOS;	/* just in case */
	}
	c1 = TOLOWER(c1) - 'a';
	c2 = TOLOWER(c2) - 'a';
	return (c1*NUMLETTERS + c2);
}


void
store(table,key,datum)
	stringtable table;
	char *key;
	char *datum;
{
	int index;
	tablelist cur,new;

	index = tablekey(key);
	cur = table[index];	

	new = MALLOC(tablenode);
	new->key = key;
	new->datum = datum;
	new->next = cur;
	table[index] = new;
}
	
	
char *
lookup(table,key)
	stringtable table;
	char *key;
{
	tablelist cur;

	cur = table[tablekey(key)];
	while (cur && strcmp(cur->key,key)) {
		cur = cur->next;
	} 
	if (cur) {
		return(cur->datum);
	} else {
		return(NULL);
	}
}
		
	
