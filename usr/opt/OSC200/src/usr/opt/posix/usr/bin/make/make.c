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
static char *rcsid = "@(#)$RCSfile: make.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/29 22:05:23 $";
#endif

/*
 * Copyright (c) 1988, 1989, 1990 The Regents of the University of California.
 * Copyright (c) 1988, 1989 by Adam de Boor
 * Copyright (c) 1989 by Berkeley Softworks
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
 * make.c --
 *	The functions which perform the examination of targets and
 *	their suitability for creation
 *
 * Interface:
 *	Make_Run 	    	Initialize things for the module and recreate
 *	    	  	    	whatever needs recreating. Returns TRUE if
 *	    	    	    	work was (or would have been) done and FALSE
 *	    	  	    	otherwise.
 *
 *	Make_TimeStamp	    	Function to set the parent's cmtime field
 *	    	  	    	based on a child's modification time.
 *
 *	Make_DoAllVar	    	Set up the various local variables for a
 *	    	  	    	target, including the .ALLSRC variable, making
 *	    	  	    	sure that any variable that needs to exist
 *	    	  	    	at the very least has the empty value.
 *
 *	Make_OODate 	    	Determine if a target is out-of-date.
 *
 *	Make_HandleTransform	See if a child is a transform node for a parent
 *				and perform the transform actions if so.
 */

#include    <stdio.h>
#include    "make.h"
#include    "pmake_msg.h"

extern nl_catd	catd;


/*
 * MAKETRACE macro used only during development for tracing MAKE module.
 * Use the compiler option -DMAKETRACE or -DALLTRACE to turn on.
 */

#if defined(MAKETRACE) || defined(ALLTRACE)
#undef MAKETRACE
#define MAKETRACE(message) { \
   printf("MAKETRACE: %s.\n",message);\
   }
#else
#define MAKETRACE(message)
#endif

/*-
 *-----------------------------------------------------------------------
 * Make_TimeStamp --
 *	Set the cmtime field of a parent node based on the mtime stamp in its
 *	child. Called from MakeOODate via Lst_ForEach. 
 *
 * Results:
 *	Always returns 0. 
 *
 * Side Effects:
 *	The cmtime of the parent node will be changed if the mtime
 *	field of the child is greater than it.
 *-----------------------------------------------------------------------
 */
int
Make_TimeStamp (GNode *pgn, GNode *cgn)
{
    /* GNode *pgn;	The current parent */
    /* GNode *cgn;	The child we've just examined */

    if (cgn->mtime > pgn->cmtime) {
	pgn->cmtime = cgn->mtime;
    }
    return (0);
}

/*-
 *-----------------------------------------------------------------------
 * Make_OODate --
 *	See if a given node is out of date with respect to its sources.
 *	Used by Make_Run when deciding which nodes to place on the
 *	toBeMade queue initially and by Make_Update to screen out
 *	EXEC nodes. In the latter case, however, any other sort of node
 *	must be considered out-of-date since at least one of its children
 *	will have been recreated.
 *
 * Results:
 *	TRUE if the node is out of date. FALSE otherwise. 
 *
 * Side Effects:
 *	The mtime field of the node and the cmtime field of its parents
 *	will/may be changed.
 *-----------------------------------------------------------------------
 */
Boolean
Make_OODate (GNode *gn)
{
    /* GNode *gn;	      The node to check */

    Boolean         oodate;

    MAKETRACE("Make_OODate: See if a given node is out of date with respect to its sources");

    (void) Dir_MTime (gn);
    if (DEBUG(MAKE)) {
	if (gn->mtime != 0) {
	    printf (catgets(catd, MS_DEBUG, MAKE010,
		"Make_OODate: \"%s\" was modified %s .\n"), gn->name,
		Targ_FmtTime(gn->mtime));
	} else {
	    printf (catgets(catd, MS_DEBUG, MAKE011, 
		"Make_OODate: \"%s\" is considered \"non-existent\".\n"),gn->name);
	}
    }

    /*
     * A target is remade in one of the following circumstances:
     *	its modification time is smaller than that of its youngest child
     *	    and it would actually be run (has commands or type OP_NOP)
     *	it's the object of a force operator
     *	it has no children, was on the lhs of an operator and doesn't exist
     *	    already.
     *
     * Libraries are only considered out-of-date if the archive module says
     * they are.
     *
     * These weird rules are brought to you by Backward-Compatability and
     * the strange people who wrote 'Make'.
     */
    if ((gn->mtime < gn->cmtime) ||
	       ((gn->cmtime == 0) &&
		((gn->mtime==0) || (gn->type & OP_DOUBLEDEP))))
    {
	/*
	 * A node whose modification time is less than that of its
	 * youngest child or that has no children (cmtime == 0) and
	 * either doesn't exist (mtime == 0) or was the object of a
	 * :: operator is out-of-date. Why? Because that's the way Make does
	 * it.
	 */
	if (DEBUG(MAKE)) {
	    if (gn->mtime < gn->cmtime) {
		printf(catgets(catd, MS_DEBUG, MAKE012,
		  "Make_OODate: \t\"%s\" was modified before sources.\n"), gn->name);
	    } else if (gn->mtime == 0) {
		printf(catgets(catd, MS_DEBUG, MAKE013, 
		  "Make_OODate: \t\"%s\" is non-existent and has no sources.\n"), gn->name);
	    } else {
		printf(catgets(catd, MS_DEBUG, MAKE014,
		  "Make_OODate: \t \"::\" operator and no sources.\n"));
	    }
	}
	oodate = TRUE;
    } else {
	oodate = FALSE;
    }

    /*
     * If the target isn't out-of-date, the parents need to know its
     * modification time. Note that targets that appear to be out-of-date
     * but aren't, because they have no commands and aren't of type OP_NOP,
     * have their mtime stay below their children's mtime to keep parents from
     * thinking they're out-of-date.
     */

    if (!oodate) {
	Lst_ForEach (gn->parents, (int(*)(void*,void*))Make_TimeStamp, (ClientData)gn);
    }

    return (oodate);
}

/*-
 *-----------------------------------------------------------------------
 * Make_HandleTransform --
 *	Function called by Make_Run and SuffApplyTransform on the downward
 *	pass to handle transformation nodes. A callback function
 *	for Lst_ForEach, it implements the transformation
 *	functionality by copying the node's commands, type flags
 *	and children to the parent node. Should be called before the
 *	children are enqueued to be looked at by MakeAddChild.
 *
 * Results:
 *	returns 0.
 *
 * Side Effects:
 *	Children and commands may be added to the parent and the parent's
 *	type may be changed.
 *
 *-----------------------------------------------------------------------
 */
int
Make_HandleTransform (GNode *cgn, GNode *pgn)
{
    /* GNode	*cgn;	The transform node (node containing command) */
    /* GNode   	*pgn;	The target of the transform node */

    GNode	*gn;	/* A child of the transform node */
    LstNode	ln; 	/* An element in the children list */

    MAKETRACE("Make_HandleTransform: Handle transformation nodes");

    if (cgn->type & OP_TRANSFORM) {
	if (Lst_IsEmpty(pgn->commands)) {
	    /*
	     * transformation and target has no commands -- append
	     * the child's commands to the parent.
	     */
	    Lst_Concat (pgn->commands, cgn->commands, LST_CONCNEW);
	}
	
	if (Lst_Open (cgn->children) == SUCCESS) {
	    while ((ln = Lst_Next (cgn->children)) != NILLNODE) {
		gn = (GNode *)Lst_Datum (ln);

		if (Lst_Member ((Lst)pgn->children, (ClientData)gn) == NILLNODE) { 
		    Lst_AtEnd ((Lst)pgn->children, (ClientData)gn);
		    Lst_AtEnd ((Lst)gn->parents, (ClientData)pgn);
		    pgn->unmade += 1;
		}
	    }
	    Lst_Close (cgn->children);
	}
	
	pgn->type |= cgn->type & ~(OP_OPMASK|OP_TRANSFORM);
    }
    return (0);
}

/*-
 *-----------------------------------------------------------------------
 * MakeAddAllSrc --
 *	Add a child's name to the ALLSRC and OODATE variables of the given
 *	node. Called from Make_DoAllVar via Lst_ForEach. A child is added only
 *	if it has not been given the .EXEC or .INVISIBLE attributes.
 *	.EXEC children are very rarely going to be files, so...
 *	A child is added to the OODATE variable if its modification time is
 *	later than that of its parent, as defined by Make, except if the
 *	parent is a .JOIN node. In that case, it is only added to the OODATE
 *	variable if it was actually made (since .JOIN nodes don't have
 *	modification times, the comparison is rather unfair...)..
 *
 * Results:
 *	Always returns 0
 *
 * Side Effects:
 *	The ALLSRC variable for the given node is extended.
 *-----------------------------------------------------------------------
 */
static int
MakeAddAllSrc (GNode *cgn, GNode *pgn)
{
    /* GNode	*cgn;	The child to add */
    /* GNode	*pgn;	The parent to whose ALLSRC variable it should be added */


    MAKETRACE("Add a child's name to the ALLSRC and OODATE variables");

    if ((cgn->type & OP_INVISIBLE) == 0) {
	char *child;

	child = Var_Value(TARGET, cgn);
	Var_Append (ALLSRC, child, pgn);
	if ((pgn->mtime < cgn->mtime) ||
		   (cgn->mtime >= now && cgn->made == MADE))
	{
	    /*
	     * It goes in the OODATE variable if the parent is younger than the
	     * child or if the child has been modified more recently than
	     * the start of the make. This is to keep pmake from getting
	     * confused if something else updates the parent after the
	     * make starts (shouldn't happen, I know, but sometimes it
	     * does). In such a case, if we've updated the kid, the parent
	     * is likely to have a modification time later than that of
	     * the kid and anything that relies on the OODATE variable will
	     * be hosed.
	     *
	     * XXX: This will cause all made children to go in the OODATE
	     * variable, even if they're not touched, if RECHECK isn't defined,
	     * since cgn->mtime is set to now in Make_Update. According to
	     * some people, this is good...
	     */
	    Var_Append(OODATE, child, pgn);
	}
    }
    return (0);
}

/*-
 *-----------------------------------------------------------------------
 * Make_DoAllVar --
 *	Set up the ALLSRC and OODATE variables. Sad to say, it must be
 *	done separately, rather than while traversing the graph. This is
 *	because Make defined OODATE to contain all sources whose modification
 *	times were later than that of the target, *not* those sources that
 *	were out-of-date. Since in both compatibility and native modes,
 *	the modification time of the parent isn't found until the child
 *	has been dealt with, we have to wait until now to fill in the
 *	variable. As for ALLSRC, the ordering is important and not
 *	guaranteed when in native mode, so it must be set here, too.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	The ALLSRC and OODATE variables of the given node is filled in.
 *	If the node is a .JOIN node, its TARGET variable will be set to
 * 	match its ALLSRC variable.
 *-----------------------------------------------------------------------
 */
void
Make_DoAllVar (GNode *gn)
{
    /* GNode	*gn; */


    MAKETRACE("Make_DoAllVar: Set up the ALLSRC and OODATE variables");

    Lst_ForEach ((Lst)gn->children, (int(*)(void*,void*))MakeAddAllSrc, (ClientData)gn);

    if (!Var_Exists (OODATE, gn)) {
	Var_Set (OODATE, "", gn);
    }
    if (!Var_Exists (ALLSRC, gn)) {
	Var_Set (ALLSRC, "", gn);
    }
}
