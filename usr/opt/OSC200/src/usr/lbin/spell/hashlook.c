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
static char	*sccsid = "@(#)$RCSfile: hashlook.c,v $ $Revision: 4.2.2.3 $ (DEC) $Date: 1992/12/07 10:05:50 $";
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
/*  hashlook.c	1.2  com/bsd.d/spell.d,3.1,8950 11/18/89 17:16:07"; */
#include <stdio.h>
#include "hash.h"
#include "huff.h"

unsigned        tablearea[ND];
unsigned       *table = tablearea;
int             spell_index[NI]; /* 001-kak */

#define B (BYTE*sizeof(unsigned))
#define L (BYTE*sizeof(int)-1)
#define MASK (~(1<<L))

/*
 * Don't use stdio on 16 bit machines.  The fetch macro below is also set up
 * for 16/32 bits depending on whether DONT_USE_STDIO is defined. 
 */
#ifdef pdp11
#define	DONT_USE_STDIO
#endif
#ifdef M_I286
#define	DONT_USE_STDIO
#endif
#ifdef M_I86
#define	DONT_USE_STDIO
#endif
#ifdef	DONT_USE_STDIO
#define fetch(wp,bp)\
	(((((int)wp[0]<<B)|wp[1])<<(B-bp))|(wp[2]>>bp))
#else				/* sizeof(unsigned)==sizeof(int) */
#define rshift(a,bp)	(((bp) >= B) ? 0 : (a) >> (bp))
#define fetch(wp,bp) (((wp)[0] << (B-(bp))) | rshift((wp)[1],(bp)))
#define	USE_STDIO
#endif

hashlook (s)
    char           *s;
{
    int            h;
    int            t;
    register        bp;
    register unsigned *wp;
    int             i;
    int            sum;
    unsigned       *tp;

    h = hash (s);
    t = h >> (HASHWIDTH - INDEXWIDTH);
    wp = &table[spell_index[t]]; /* 001-kak */
    tp = &table[spell_index[t + 1]]; /* 001-kak */
    bp = B;
    sum = (int) t << (HASHWIDTH - INDEXWIDTH);
    for (;;)
    {
	{			/* this block is equivalent to bp -=
				 * decode((fetch(wp,bp)>>1)&MASK, &t); */
	    int            y;
	    int            v;

	    y = (fetch (wp, bp) >> 1) & MASK;
	    if (y < cs)
	    {
		t = y >> (L + 1 - w);
		bp -= w - 1;
	    } else
	    {
		for (bp -= w, v = v0; y >= qcs; y = (y << 1) & MASK, v += n)
		    bp -= 1;
		t = v + (y >> (L - w));
	    }
	}
	while (bp <= 0)
	{
	    bp += B;
	    wp++;
	}
	if (wp >= tp && (wp > tp || bp < B))
	    return (0);
	sum += t;
	if (sum < h)
	    continue;
	return (sum == h);
    }
}


prime (argc, argv)
    char          **argv;
{
    register FILE  *f;
    register        fd;


    if (argc <= 1)
	return (0);
#ifndef USE_STDIO		/* because of insufficient address space for
				 * buffers */
    fd = dup (0);
    close (0);
    if (open (argv[1], 0) != 0)
	return (0);
    f = stdin;
 	/* start 001-kak */
    if (rhuff (f) == 0
	|| read (fileno (f), (char *) spell_index, NI * sizeof (*spell_index)) != NI * sizeof (*spell_index)
	|| read (fileno (f), (char *) table, sizeof (*table) * spell_index[NI - 1])
	!= spell_index[NI - 1] * sizeof (*table))
	return (0);
 	/* end 001-kak */
    close (0);
    if (dup (fd) != 0)
	return (0);
    close (fd);
#else
    if ((f = fopen (argv[1], "ri")) == NULL)
	return (0);
 	/* start 001-kak */
    if (rhuff (f) == 0
	|| fread ((char *) spell_index, sizeof (*spell_index), NI, f) != NI
	|| fread ((char *) table, sizeof (*table), spell_index[NI - 1], f)
	!= spell_index[NI - 1])
	{
fprintf(stderr, "NI =%d  ; spell_index[NI-1] = %d \n", NI, spell_index[NI - 1]);
	return (0);
	}
	/* end 001-kak */
    fclose (f);
#endif
    hashinit ();
    return (1);
}
