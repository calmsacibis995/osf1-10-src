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
static char	*sccsid = "@(#)$RCSfile: tsearch.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:41:05 $";
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
/*
 * tsearch.c
 *
 *	Revision History:
 *
 * 12-Mar-91	Fred Canter
 *	MIPS C 2.20+
 *
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: tsearch, tdelete, twalk, tfind
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *	tsearch.c	1.8  com/lib/c/gen,3.1,8943 10/11/89 16:59:44
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak tdelete = __tdelete
#pragma weak tfind = __tfind
#pragma weak tsearch = __tsearch
#pragma weak twalk = __twalk
#endif
#endif
#include <stdio.h>		/* for NULL			*/
#include <stdlib.h>		/* for NULL			*/
#include <search.h>		/* for preorder, postorder, etc	*/


/*
 * NAME:	tsearch
 *                                                                    
 * FUNCTION:	Tree search algorithm, generalized from Knuth
 *		(6.2.2) Algorithm T.
 *                                                                    
 * NOTES:	Tsearch performs a binary tree search and insert.
 *		'Key' is the value to search for, 'rootp' is the
 *		address of the root of this particular tree, and
 *		'compar' is the address of a function to use for
 *		comparision.  'Compar' is a the same type of function
 *		that bsearch() uses.
 *		
 *		The NODE * arguments are declared in the lint files
 *		as char *, because the definition of NODE isn't
 *		available to the user.
 *
 * RETURN VALUE DESCRIPTION:	NULL is returned if 'rootp' is NULL or
 *		if a malloc fails.  Otherwise a pointer is returned to
 *		the matching node (that we've possibly just inserted)
 *		within the tree.
 */

/*
 * These functions are now declared as specified by the XOPEN standard.
 */

typedef char *POINTER;		/* pointer type			*/
				/* tree node type */
typedef struct node { POINTER key; struct node *llink, *rlink; } NODE;

static void _twalk(NODE *root, void (*action)(void *,VISIT,int), int level);

#ifdef	_NONSTD_TYPES
char *
#else
void *
#endif
tsearch(                   	/* Find or insert key into search tree*/
const void *	key,			/* Key to be located */
void	**rootp,		/* Address of the root of the tree */
int	(*compar)(const void *, const void *))	/* Comparison function */

{
	NODE *q;	/* New node if key not found */

	if (rootp == NULL)
		return (NULL);

	while (*rootp != NULL) {			/* T1: */
		int r = (*compar)(key, (void*)((NODE *)*rootp)->key);	/* T2: */
		if (r == 0)
			return ((void *) *rootp);	/* Key found */
		rootp = (void **)((r < 0) ?
		    &((NODE*)*rootp)->llink :		/* T3: Take left branch */
		    (void *)&((NODE*)*rootp)->rlink);		/* T4: Take right branch */
	}

	q = (NODE *) malloc((size_t)sizeof(NODE));	/* T5: Not found */
	if (q != NULL) {			/* Allocate new node */
		*rootp = q;			/* Link new node to old */
		q->key = (char *) key;		/* Initialize new node */
		q->llink = q->rlink = NULL;
	}

	return (q);
}

/*
 * NAME:	tdelete
 *                                                                    
 * FUNCTION:	Delete a node within the tree.
 *                                                                    
 * NOTES:	Tdelete deletes a particular node within a binary
 *		tree.  The arguments are the same as tsearch.
 *
 * RETURN VALUE DESCRIPTION:	NULL if 'rootp' is NULL or if
 *		'key' could not be found.  Else a pointer to
 *		the parent of the deleted node.
 */  

#ifdef	_NONSTD_TYPES
char *
#else
void *
#endif
tdelete(                   	/* Delete node with key key */
const void *	key,			/* Key to be deleted */
void	**rootp,		/* Address of the root of tree */
int	(*compar)(const void *, const void *))	/* Comparison function */
{
	NODE *p;		/* Parent of node to be deleted */
	NODE *q;		/* Successor node */
	NODE *r;		/* Right son node */
	int ans;		/* Result of comparison */

	if (rootp == NULL || (p = (NODE *) *rootp) == NULL)
		return (NULL);

	while ((ans = (*compar)(key, (void*)((NODE *)*rootp)->key)) != 0) {
		p = (NODE *) *rootp;
		rootp = (void **)((ans < 0) ?
		    &((NODE*)*rootp)->llink :		/* Take left branch */
		    (void *)&((NODE*)*rootp)->rlink);		/* Take right branch */
		if (*rootp == NULL)
			return (NULL);		/* Key not found */
	}

	r = ((NODE *)*rootp)->rlink;			/* D1: */
	if ((q = ((NODE *)*rootp)->llink) == NULL)	/* Llink NULL? */
		q = r;
	else if (r != NULL) {			/* Rlink NULL? */
		if (r->llink == NULL) {		/* D2: Find successor */
			r->llink = q;
			q = r;
		} else {			/* D3: Find NULL link */
			for (q = r->llink; q->llink != NULL; q = r->llink)
		 		r = q;
			r->llink = q->rlink;
			q->llink = ((NODE *)*rootp)->llink;
			q->rlink = ((NODE *)*rootp)->rlink;
		}
	}

	free((void*) *rootp);		/* D4: Free node */
	*rootp = q;			/* Link parent to replacement */

	return (p);
}

/*
 * NAME:	twalk
 *                                                                    
 * FUNCTION:	Twalk walks a binary tree previously created by tsearch().
 *                                                                    
 * NOTES:	'Root' is the root of the tree to be walked.  'Action' is
 *		a pointer to a function that will be invoked at each node.
 *		'Action' will be invoked with 3 arguments:
 *			1)	address of current node
 *			2)	one of the values in the VISIT enum type
 *			3)	the level of the node
 *
 */  

void
twalk(             		/* Walk the nodes of a tree */
const void	*root,			/* Root of the tree to be walked */
void	(*action)(const void *,VISIT,const int)) /* Function to be called at each node */
{

	if (root != NULL && action != NULL)
		_twalk((const NODE *)root, action, 0);
}

/*
 * NAME:	_twalk
 *                                                                    
 * FUNCTION:	_twalk is the recursive function that actually walks
 *		the tree.
 *                                                                    
 * NOTES:
 *
 */  

static void
_twalk(			/* Walk the nodes of a tree */
const NODE	*root,		/* Root of the tree to be walked */
void	(*action)(const void *,VISIT,int),	/* Function to be called at each node */
int	level)
{
	if (root->llink == NULL && root->rlink == NULL)
		(*action)(root, leaf, level);

	else {
		(*action)(root, preorder, level);

		if (root->llink != NULL)
			_twalk(root->llink, action, level + 1);

		(*action)(root, postorder, level);

		if (root->rlink != NULL)
			_twalk(root->rlink, action, level + 1);

		(*action)(root, endorder, level);
	}
}

/*
 * NAME:	tfind
 *                                                                    
 * FUNCTION:	Find a node but do not insert.
 *                                                                    
 * NOTES:	Tfind acts like tsearch except that the node is
 *		not inserted if not found.
 *
 * RETURN VALUE DESCRIPTION:	NULL is returned if the root is NULL
 *		or if 'key' could not be found.  Else a pointer to
 *		the matching node is returned.
 */  

#ifdef	_NONSTD_TYPES
char *
#else
void *
#endif
tfind(                   
const void *	key,			/* Key to be located */
void * const *rootp,		/* Address of the root of the tree */
int	(*compar)(const void *, const void *))		/* Comparison function */
{
	if (rootp == NULL)
		return (NULL);

	while (*rootp != NULL) {			/* T1: */
		int r = (*compar)(key, (void*)((NODE *)*rootp)->key);	/* T2: */
		if (r == 0)
			return (*rootp);	/* Key found */
		rootp = (void **)((r < 0) ?
		    &((NODE *)*rootp)->llink :		/* T3: Take left branch */
		    (void *)&((NODE *)*rootp)->rlink);		/* T4: Take right branch */
	}

	return (void *)(NULL);
}


