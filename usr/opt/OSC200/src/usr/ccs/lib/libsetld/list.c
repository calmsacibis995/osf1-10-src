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
static  char *sccsid = "@(#)$RCSfile: list.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/10/13 13:46:32 $";
#endif
/*
 */
#if !defined( lint) && !defined(_NOIDENT)

#endif
/*	list.c -
 *		singly linked list manipulation routines
 *
 *	mods:
 *	000	8904??	ccb	New
 *	001	890608	ccb	Add ListFree, free all nodes in a list
*/

#include	<setld/list.h>

/*LINTLIBRARY*/

/*	ListT	*ListAppend()
 *		concatenate lists
 *
 *	given:	ListT *p - list to append to
 *		ListT *q - list to append
 *	does:	append list q to list p
 *	return:	a pointer to the head of the merged list
*/

ListT *ListAppend( p, q )
ListT *p;
ListT *q;
{
	register ListT	*t;	/* temporary pointer */

	if( q == (ListT *) 0 )		/* nothing to append */
		return( p );

	if( p == (ListT *) 0 )		/* append to nothing */
		return( q );

	/* find the last element in p */
	for( t = p; t->l_next != (ListT *) 0; t = t->l_next )
		;

	/* cause the last element of p to point to q */
	t->l_next = q;

	/* done */
	return( p );
}


/*	void	ListFree() -
 *		free a list
 *
 *	given:	ListT *l - a pointer to a list
 *	does:	free all of the elements of the list
 *	return:	nothing
*/

void ListFree( l )
ListT *l;
{
	if( l == (ListT *) 0 )
		return;

	ListFree( l->l_next );
	free( (char *) l );
	return;
}


/*	int	ListLen() -
 *		return number of elements in a list
 *
 *	given:	ListT *l - beginning of a list
 *	does:	count list elements
 *	return:	0 for NULL list, or 1 greater than the length of the
 *		the same list without its first element.
 *
 *	NOTE:	will not return if given a circular list
*/

int ListLen( l )
ListT *l;
{
	if( l == (ListT *) 0 )
		return( 0 );

	return( ListLen( l->l_next ) + 1 );
}

