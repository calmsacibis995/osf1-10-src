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
/*
 * @(#)$RCSfile: lst.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/29 22:02:18 $
 */

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
 *
 *	@(#)lst.h	5.3 (Berkeley) 6/1/90
 */

/*-
 * lst.h --
 *	Header for using the list library
 */
#ifndef _LST_H_
#define _LST_H_

#include	"alphaosf.h"
#include	  <sys/types.h>


typedef struct LstNode {
	struct LstNode	*prevPtr;   /* previous element in list */
	struct LstNode	*nextPtr;   /* next in list */
	int	    	useCount:8, /* Count of functions using the node.
				     * node may not be deleted until count
				     * goes to 0 */
 	    	    	flags:8;    /* Node status flags */
	ClientData	datum;	    /* datum associated with this element */
} *LstNode;
/*
 * Flags required for synchronization
 */
#define LN_DELETED  	0x0001      /* List node should be removed when done */

#define NilLstNode	((LstNode)-1)

typedef enum {
    Head, Middle, Tail, Unknown
} Where;

typedef struct	Lst {
	LstNode  	firstPtr; /* first node in list */
	LstNode  	lastPtr;  /* last node in list */
	Boolean	  	isCirc;	  /* true if the Lst should be considered
				   * circular */
/*
 * fields for sequential access
 */
	Where	  	atEnd;	  /* Where in the list the last access was */
	Boolean	  	isOpen;	  /* true if list has been Lst_Open'ed */
	LstNode  	curPtr;	  /* current node, if open. NilLstNode if
				   * *just* opened */
	LstNode  	prevPtr;  /* Previous node, if open. Used by
				   * Lst_Remove */
} *Lst;

#define NilLst	  	((Lst)-1)
#define	NILLST		((Lst) NIL)
#define	NILLNODE	((LstNode) NIL)


/*
 * NOFREE can be used as the freeProc to Lst_Destroy when the elements are
 *	not to be freed.
 * 
 */

#define NOFREE		((void (*)(void*)) 0)

#define LST_CONCNEW	0   /* create new LstNode's when using Lst_Concat */
#define LST_CONCLINK	1   /* relink LstNode's when using Lst_Concat */

/*
 * Creation/destruction functions
 */
Lst		  Lst_Init(Boolean circ);                 /* Create a new list */
void		  Lst_Destroy(Lst l, void (*)(void*));	  /* Destroy an old one */

/* Find the length of a list */

Boolean		  Lst_IsEmpty(Lst l);	/* True if list is empty */

/*
 * Functions to modify a list
 */

ReturnStatus	  Lst_Insert(Lst l, LstNode ln, ClientData d);	    	/* Insert an element before another */
ReturnStatus	  Lst_Append(Lst l, LstNode ln, ClientData d);	    	/* Insert an element after another */
ReturnStatus	  Lst_AtFront(Lst l, ClientData d);    	/* Place an element at the front of
							 * a lst. */
ReturnStatus	  Lst_AtEnd(Lst l, ClientData d);	    	/* Place an element at the end of a
								 * lst. */
ReturnStatus	  Lst_Remove(Lst l, LstNode ln);	    	/* Remove an element */
ReturnStatus	  Lst_Replace(LstNode ln, ClientData d);	/* Replace a node with a new value */
ReturnStatus	  Lst_Concat(Lst l1, Lst l2, int flags);	/* Concatenate two lists */

/*
 * Node-specific functions
 */

LstNode		  Lst_First(Lst l);                /* Return first element in list */
LstNode		  Lst_Last(Lst l);                 /* Return last element in list */
LstNode		  Lst_Succ(LstNode ln);	           /* Return successor to given element */
ClientData	  Lst_Datum(LstNode ln);           /* Get datum from LstNode */

/*
 * Functions for entire lists
 */

LstNode		  Lst_Find(Lst l, ClientData d, int (*)(void*,void*));	 /* Find an element in a list */
LstNode		  Lst_FindFrom(Lst l, LstNode ln, ClientData, int (*)(void*,void*));	
                                                                        /* Find an element starting from
									 * somewhere */
LstNode	    	  Lst_Member(Lst l, ClientData d);	    	        /* See if the given datum is on the
									 * list. Returns the LstNode containing
									 * the datum */
void		  Lst_ForEach(Lst l, int (*)(void*,void*), ClientData d); /* Apply a function to all elements of
									  * a lst */

void	    	  Lst_ForEachFrom(Lst l, LstNode ln, int (*)(void*,void*), ClientData d);
  	
                                                                        /* Apply a function to all elements of
									 * a lst starting from a certain point.
									 * If the list is circular, the
									 * application will wrap around to the
									 * beginning of the list again. */
/*
 * these functions are for dealing with a list as a table, of sorts.
 * An idea of the "current element" is kept and used by all the functions
 * between Lst_Open() and Lst_Close().
 */

ReturnStatus	  Lst_Open(Lst l);	    	        /* Open the list */
LstNode		  Lst_Next(Lst l);	    	        /* Next element please */
void		  Lst_Close(Lst l);	    	        /* Finish table access */

/*
 * for using the list as a queue
 */

/* ReturnStatus	  Lst_EnQueue(Lst l, ClientData d);	Place an element at tail of queue */
ClientData	  Lst_DeQueue(Lst l);	                /* Remove an element from head of
							 * queue */

#endif /* _LST_H_ */
