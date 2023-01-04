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
static char	*sccsid = "@(#)$RCSfile: spellinprg.c,v $ $Revision: 4.2.2.5 $ (DEC) $Date: 1993/01/21 14:21:55 $";
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
 * ORIGINS: 3,10,13,27
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
/*  spellinprg.c	1.2  com/bsd.d/spell.d,3.1,8950 11/18/89 17:39:20"; */
#include <stdio.h>
#include <stdlib.h>	/* 002 */
#include <locale.h>
#include "hash.h"

#define S (BYTE*sizeof(int))
#define B (BYTE*sizeof(unsigned))
unsigned        tablearea[ND];
unsigned       *table = tablearea;
int             spell_index[NI]; /* 001-kak */
unsigned        wp;		/* word pointer */
int             bp = B;		/* bit pointer */
int             ignore;
int             extra;

#include "spell_msg.h"
#define MSGSTR(num, str)    catgets(catd, MS_SPELL, num, str)

#define HASHCODEWIDTH ((HASHWIDTH + 2) / 3)   /* 002 - width calculation taken
							from hashmake.c */

/*
 * usage: hashin N where N is number of words in dictionary and standard
 * input contains sorted, unique hashed words in octal 
 */
main (argc, argv)
    char          **argv;
{
    int            h,
                   k,
                   d;
    register       i;
    int            count;
    int            w;
    int            x;
    int            t,
                   u;
    extern double  huff ();
    double         atof ();
    double         z;
    nl_catd        catd;
    double         num_hash_codes;	/* 002 */
    char          *endptr;		/* 002 */
    char           hcbuf[HASHCODEWIDTH + 2];  /* 002 buf for hc, \n, \0 */

    setlocale (LC_ALL, "");
    catd = catopen (MF_SPELL, 0);
    k = 0;
    u = 0;
    if (argc != 2)
    {
	fprintf (stderr, MSGSTR (SPIN_ARG, "spellin: arg count\n"));
	exit (1);
    }

    /* 002 validate hash code number arg.
     */
    num_hash_codes = strtod(argv[1], &endptr);
    if (*endptr != '\0' || num_hash_codes <= 0)
    {
	fprintf (stderr, MSGSTR (SPIN_NUM,
	    "spellin: number of hash codes = %s; positive number expected\n"),
	    argv[1]);
	exit (1);
    }

      z = huff ( (double)(1 << HASHWIDTH) / num_hash_codes);
    fprintf (stderr,
	     MSGSTR (SPIN_WID, "spellin: expected code widths = %f\n"), z);
    for (count = 0; fgets(hcbuf, sizeof(hcbuf), stdin); ++count)
    {
    	/* 002  Check for bad hash code
     	 */
    	if (hcbuf[HASHCODEWIDTH] != '\n' || sscanf(hcbuf, "%o", &h) != 1)
    	{
	    fprintf (stderr, MSGSTR(SPIN_BAD,
			"spellin: bad input; octal hash code expected\n"));
	    exit (1);
    	}

	if ((t = h >> (HASHWIDTH - INDEXWIDTH)) != u)
	{
	    if (bp != B)
		wp++;
	    bp = B;
	    while (u < t)
		spell_index[++u] = wp; /* 001-kak */
	    k = (int) t << (HASHWIDTH - INDEXWIDTH);
	}
	d = h - k;
	k = h;
	for (;;)
	{
	    for (x = d;; x /= 2)
	    {
		i = encode (x, &w);
		if (i > 0)
		    break;
	    }
	    if (i > B)
	    {
		if (!(
		      append ((unsigned) (w >> (i - B)), B) &&
		      append ((unsigned) (w << (B + B - i)), i - B)))
		    ignore++;
	    } else
	    if (!append ((unsigned) (w << (B - i)), i))
		ignore++;
	    d -= x;
	    if (d > 0)
		extra++;
	    else
		break;
	}
    }

    /* 002  Check for no hash codes on input.
     */
    if (count == 0)
    {
	fprintf (stderr, MSGSTR(SPIN_NON, "spellin: no hash codes on input\n"));
	exit (1);
    }

    if (bp != B)
	wp++;
    while (++u < NI)
	spell_index[u] = wp; /* 001-kak */
    whuff ();
    fwrite ((char *) spell_index, sizeof (*spell_index), NI, stdout);/* 001-kak */
    fwrite ((char *) table, sizeof (*table), wp, stdout);
    fprintf (stderr, MSGSTR (SPIN_ITMS,
	   "spellin: %ld items, %d ignored, %d extra, %u words occupied\n"),
	     count, ignore, extra, wp);
    count -= ignore;
	/* 001-kak */
    fprintf (stderr, MSGSTR (SPIN_BITS,
		      "spellin: %f table bits/item, %f table+spell_index bits\n"),
	     ((float) BYTE * wp) * sizeof (*table) / count,
	     BYTE * ((float) wp * sizeof (*table) + sizeof (spell_index)) / count);
    return (0);
}

append (w, i)
    register unsigned w;
    register        i;
{
    while (wp < ND - 1)
    {
	table[wp] |= w >> (B - bp);
	i -= bp;
	if (i < 0)
	{
	    bp = -i;
	    return (1);
	}
	w <<= bp;
	bp = B;
	wp++;
    }
    return (0);
}
