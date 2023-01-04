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
static char	*sccsid = "@(#)$RCSfile: spellout.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:25:12 $";
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
 * ORIGINS: 10,13,26,27
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
/*  spellout.c	1.2  com/bsd.d/spell.d,3.1,8950 11/18/89 17:41:32";  */

/*
 * spellout - display words [not] in spelling list 
 *
 * spellout [-d] hlist < wordlist > (mis)matchlist 
 */

#include "spell_msg.h"
#define MSGSTR(n,s) NLgetamsg(MF_SPELL,MS_SPELL,n,s)


#include <stdio.h>
#include <locale.h>

#define	WORDLENGTH	30	/* NW in BSD spellout (spell.h) */

main (argc, argv)
    int             argc;
    char          **argv;
{
    char            word[WORDLENGTH];
    int             dflag = 0;

    setlocale (LC_ALL, "");
    if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'd')
    {
	dflag = (argc == argc);
	argc--;
	argv++;
    }
    if (argc <= 1)
    {
	fprintf (stderr, MSGSTR (ARGCNT, "spellout: arg count\n"));	/* MSG */
	exit (1);
    }
    if (!prime (argc, argv))
    {
	fprintf (stderr, MSGSTR (CANTINIT, "spellout: cannot initialize hash table\n"));	/* MSG */
	exit (1);
    }
    /*
     * Display Truth table -d flag | 0  1 --+----- hashlook 0 | 1  0 1 | 0  1 
     */
    while (fgets (word, sizeof (word), stdin))
    {
	register int    x;
	if (word[x = strlen (word) - 1] == '\n')
	    word[x] = NULL;
	if (dflag == hashlook (word))
	    puts (word);
    }
}
