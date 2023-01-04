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
static char	*sccsid = "@(#)$RCSfile: hashcheck.c,v $ $Revision: 4.2.2.4 $ (DEC) $Date: 1993/01/21 14:21:18 $";
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
/* hashcheck.c	1.2  com/bsd.d/spell.d,3.1,8950 11/18/89 17:14:33"; */
#include <stdio.h>
#include <locale.h>
#include "hash.h"
#include "spell_msg.h"	/* 003 */
#define MSGSTR(num, str)    catgets(catd, MS_SPELL, num, str) /* 003 */

int            fetch ();
int             spell_index[NI]; /* 001-kak */
unsigned        tablearea[ND];
unsigned       *table = tablearea;
unsigned        wp;
int             bp;
#define U (BYTE*sizeof(unsigned))
#define L (BYTE*sizeof(int))

main ()
{
    int             i;
    int            v;
    int            a;
    nl_catd        catd;	/* 003 */
    setlocale (LC_ALL, "");
    catd = catopen (MF_SPELL, 0);	/* 003 */

    /* 003 Read in huffcode header and verify that file is correct format.
     */
    if (!rhuff (stdin) || !checkhuff())
    {
	fprintf (stderr, MSGSTR (HCHK_BAD,
	  "hashcheck: bad input format; compressed spelling list expected.\n"));
	exit (1);
    }
	/* start block 001-kak */
    fread ((char *) spell_index, sizeof (*spell_index), NI, stdin);
    fread ((char *) table, sizeof (*table), spell_index[NI - 1], stdin);
	/* end block 001-kak */
    for (i = 0; i < NI - 1; i++)
    {
	bp = U;
	v = (int) i << (HASHWIDTH - INDEXWIDTH);
	for (wp = spell_index[i]; wp < spell_index[i + 1];) /* 001-kak */
	{
	    if (wp == spell_index[i] && bp == U) /* 001-kak */
		a = fetch ();
	    else
	    {
		a = fetch ();
		if (a == 0)
		    break;
	    }
		/* 001-kak */
	    if (wp > spell_index[i + 1] || 
		wp == spell_index[i + 1] && bp < U)
		break;
	    v += a;
	    printf ("%.9lo\n", v);
	}
    }
}

int 
fetch ()
{
    int             w;
    int             y = 0;
    int             empty = L;
    int             i = bp;
    int             tp = wp;
    while (empty >= i)
    {
	empty -= i;
	i = U;
	y |= (int) table[tp++] << empty;
    }
    if (empty > 0)
	y |= table[tp] >> i - empty;
    i = decode ((y >> 1) & (0x7fffffff), &w);
    bp -= i;
    while (bp <= 0)
    {
	bp += U;
	wp++;
    }
    return (w);
}
