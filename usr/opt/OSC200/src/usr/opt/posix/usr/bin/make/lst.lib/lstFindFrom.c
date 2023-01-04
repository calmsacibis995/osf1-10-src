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
static char *rcsid = "@(#)$RCSfile: lstFindFrom.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/29 22:03:56 $";
#endif

/*
 * Copyright (c) 1988, 1989, 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Adam de Boor.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*-
 * LstFindFrom.c --
 *	Find a node on a list from a given starting point. Used by Lst_Find.
 */

#include	"lstInt.h"

/*-
 *-----------------------------------------------------------------------
 * Lst_FindFrom --
 *	Search for a node starting and ending with the given one on the
 *	given list using the passed datum and comparison function to
 *	determine when it has been found.
 *
 * Results:
 *	The found node or NILLNODE
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
LstNode
Lst_FindFrom (Lst l, LstNode ln, ClientData d, int (*cProc)(void*,void*))
{
    /* Lst	  l; */
    /* LstNode    ln; */
    /* ClientData d; */ 
    /* int	  (*cProc)(); */

    register LstNode	tln;
    Boolean		found = FALSE;
    
    if (!LstValid (l) || LstIsEmpty (l) || !LstNodeValid (ln, l)) {
	return (NILLNODE);
    }
    
    tln = (LstNode)ln;
    
    do {
	if ((*cProc) (tln->datum, d) == 0) {
	    found = TRUE;
	    break;
	} else {
	    tln = tln->nextPtr;
	}
    } while (tln != (LstNode)ln && tln != NilLstNode);
    
    if (found) {
	return ((LstNode)tln);
    } else {
	return (NILLNODE);
    }
}

