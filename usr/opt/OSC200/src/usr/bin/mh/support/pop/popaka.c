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
/* popaka.c - generate POP entries for MMDF-II alias file */
#ifndef	lint
static char ident[] = "@(#)$RCSfile: popaka.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:40:04 $ devrcs Exp Locker: devbld $";
#endif	lint

#include <stdio.h>
#include "../zotnet/bboards.h"

static process();
/*  */

/* ARGSUSED */

main (argc, argv)
int     argc;
char  **argv;
{
    struct bboard  *bb;

    if (!setbbinfo (POPUID, POPDB, 1)) {
	fprintf (stderr, "setbbinfo(%s, %s, 1) failed -- %s\n",
		POPUID, POPDB, getbberr ());
	exit (1);
    }

    (void) setbbent (SB_STAY);
    while (bb = getbbent ())
	process (bb);
    (void) endbbent ();

    exit (0);
}

/*  */

static  process (bb)
struct bboard  *bb;
{
    printf ("%s: %s@pop\n", bb -> bb_name, bb -> bb_name);
}
