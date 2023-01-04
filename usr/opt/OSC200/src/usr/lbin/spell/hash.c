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
static char	*sccsid = "@(#)$RCSfile: hash.c,v $ $Revision: 4.2.2.3 $ (DEC) $Date: 1992/05/26 09:28:00 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: (CMDTEXT) Text Formatting Services
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3,10,27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*   hash.c	1.2  com/bsd.d/spell.d,3.1,8950 11/18/89 17:49:43";  */

#include "hash.h"

#define LOCHWIDTH 3
#define HICHWIDTH 3
#define CHARWIDTH (LOCHWIDTH+HICHWIDTH)
#define LOCHMASK ((1<<LOCHWIDTH)-1)

/*
 * if HASHWIDTH + CHARWIDTH < bitsizeof(int) one could make LOCHWIDTH=6 and
 * HICHWIDTH=0 and simplify accordingly; the hanky-panky is to avoid overflow
 * in long multiplication 
 */
#define NC 30

int            hashsize = HASHSIZE;
int            pow2[NC * 2];

static char     hashtab[] = {
			     -1, -1, -1, -1, -1, -1, 0, 31,	/* &' */
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     2, 25, 20, 35, 54, 61, 40, 39,	/* 0-7 */
			     42, 33, -1, -1, -1, -1, -1, -1,
			     -1, 60, 43, 30, 5, 16, 47, 18,	/* A-G */
			     41, 36, 51, 6, 13, 56, 55, 58,
			     49, 12, 59, 46, 21, 32, 63, 34,
			     57, 52, 3, -1, -1, -1, -1, -1,
			     -1, 22, 29, 8, 7, 10, 1, 28,	/* a-g */
			     11, 62, 37, 48, 15, 50, 9, 4,
			     19, 38, 45, 24, 23, 26, 17, 44,
			     27, 14, 53, -1, -1, -1, -1, -1
};


int 
hash (s)
    char            *s;
{
    register int     c;
    register int   *lp;
    int          h = 0;
    for (lp = pow2; c = *s++;)
    {
	/* be sure to bail out before failing off the end of pow2 */
	if (lp >= &pow2[NC*2 - 1]) break; 
	c = hashtab[c - ' '];
	h += (c & LOCHMASK) * *lp++;
	h += (c >> LOCHWIDTH) * *lp++;
	h %= hashsize;
    }
    return (h);
}

hashinit ()
{
    register int    i;
    if (1 << (HASHWIDTH + LOCHWIDTH) == 0
	|| 1 << (HASHWIDTH + HICHWIDTH) == 0)
	abort ();		/* overflow is imminent */
    pow2[0] = 1 << (HASHWIDTH - CHARWIDTH - 2);
    for (i = 0; i < 2 * NC - 3; i += 2)
    {
	pow2[i + 1] = (pow2[i] << LOCHWIDTH) % hashsize;
	pow2[i + 2] = (pow2[i + 1] << HICHWIDTH) % hashsize;
    }
}

