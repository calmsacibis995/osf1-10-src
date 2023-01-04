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
static char *rcsid = "@(#)$RCSfile: lstConcat.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/29 22:03:19 $";
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
 * listConcat.c --
 *	Function to concatentate two lists.
 */

#include    "lstInt.h"
#include    <stdlib.h>

/*-
 *-----------------------------------------------------------------------
 * Lst_Concat --
 *	Concatenate two lists. New elements are created to hold the data
 *	elements, if specified, but the elements themselves are not copied.
 *	If the elements should be duplicated to avoid confusion with another
 *	list, the Lst_Duplicate function should be called first.
 *	If LST_CONCLINK is specified, the second list is destroyed since
 *	its pointers have been corrupted and the list is no longer useable.
 *
 * Results:
 *	SUCCESS if all went well. FAILURE otherwise.
 *
 * Side Effects:
 *	New elements are created and appended the the first list.
 *-----------------------------------------------------------------------
 */
ReturnStatus
Lst_Concat (Lst l1, Lst l2, int flags)
{
    /* Lst    	  	l1; 	The list to which l2 is to be appended */
    /* Lst    	  	l2; 	The list to append to l1 */
    /* int	   	flags;  LST_CONCNEW if LstNode's should be duplicated
     *                          LST_CONCLINK if should just be relinked */

    register LstNode  	ln;     /* original LstNode */
    register LstNode  	nln;    /* new LstNode */
    register LstNode  	last;   /* the last element in the list. Keeps
				 * bookkeeping until the end */
    register Lst 	list1 = (Lst)l1;
    register Lst 	list2 = (Lst)l2;

    if (!LstValid (l1) || !LstValid (l2)) {
	return (FAILURE);
    }

    if (flags == LST_CONCLINK) {
	if (list2->firstPtr != NilLstNode) {
	    /*
	     * We set the nextPtr of the
	     * last element of list two to be NIL to make the loop easier and
	     * so we don't need an extra case should the first list turn
	     * out to be non-circular -- the final element will already point
	     * to NIL space and the first element will be untouched if it
	     * existed before and will also point to NIL space if it didn't.
	     */
	    list2->lastPtr->nextPtr = NilLstNode;
	    /*
	     * So long as the second list isn't empty, we just link the
	     * first element of the second list to the last element of the
	     * first list. If the first list isn't empty, we then link the
	     * last element of the list to the first element of the second list
	     * The last element of the second list, if it exists, then becomes
	     * the last element of the first list.
	     */
	    list2->firstPtr->prevPtr = list1->lastPtr;
	    if (list1->lastPtr != NilLstNode) {
 		list1->lastPtr->nextPtr = list2->firstPtr;
	    }
	    list1->lastPtr = list2->lastPtr;
	}
	if (list1->isCirc && list1->firstPtr != NilLstNode) {
	    /*
	     * If the first list is supposed to be circular and it is (now)
	     * non-empty, we must make sure it's circular by linking the
	     * first element to the last and vice versa
	     */
	    list1->firstPtr->prevPtr = list1->lastPtr;
	    list1->lastPtr->nextPtr = list1->firstPtr;
	}
	free ((Address)l2);
    } else if (list2->firstPtr != NilLstNode) {
	/*
	 * We set the nextPtr of the last element of list 2 to be nil to make
	 * the loop less difficult. The loop simply goes through the entire
	 * second list creating new LstNodes and filling in the nextPtr, and
	 * prevPtr to fit into l1 and its datum field from the
	 * datum field of the corresponding element in l2. The 'last' node
	 * follows the last of the new nodes along until the entire l2 has
	 * been appended. Only then does the bookkeeping catch up with the
	 * changes. During the first iteration of the loop, if 'last' is nil,
	 * the first list must have been empty so the newly-created node is
	 * made the first node of the list.
	 */
	list2->lastPtr->nextPtr = NilLstNode;
	for (last = list1->lastPtr, ln = list2->firstPtr;
	     ln != NilLstNode;
	     ln = ln->nextPtr)
	{
	    PAlloc (nln, LstNode);
	    nln->datum = ln->datum;
	    if (last != NilLstNode) {
		last->nextPtr = nln;
	    } else {
		list1->firstPtr = nln;
	    }
	    nln->prevPtr = last;
	    nln->flags = nln->useCount = 0;
	    last = nln;
	}

	/*
	 * Finish bookkeeping. The last new element becomes the last element
	 * of list one. 
	 */
	list1->lastPtr = last;

	/*
	 * The circularity of both list one and list two must be corrected
	 * for -- list one because of the new nodes added to it; list two
	 * because of the alteration of list2->lastPtr's nextPtr to ease the
	 * above for loop.
	 */
	if (list1->isCirc) {
	    list1->lastPtr->nextPtr = list1->firstPtr;
	    list1->firstPtr->prevPtr = list1->lastPtr;
	} else {
	    last->nextPtr = NilLstNode;
	}

	if (list2->isCirc) {
	    list2->lastPtr->nextPtr = list2->firstPtr;
	}
    }

    return (SUCCESS);
}
