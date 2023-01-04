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
 *	@(#)$RCSfile: lv_qmacros.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/29 14:20:33 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#ifndef Q_APPEND_PROTO
	/*
	 * Q_APPEND(q-pointer,element-pointer):
	 * hang element off of tail
	 * initialize end-of-list link
	 * set tail pointer address of link in new tail element
	 * Does not return a particularly useful value.
	 *
	 * This macro is equivalent to the following code:
	 *	struct queue *q; struct item *new;
	 *	if ((*q->tail = new) != NULL) {
	 *		new->next = NULL;	
	 *		q->count++;
	 *		q->tail = &(new->next);
	 *	}
	 */
#define Q_APPEND_PROTO(Q,NEW,HEAD,TAIL,LINK,COUNT) ((*((Q)->TAIL)=(NEW))\
				&&(((NEW)->LINK=NULL),\
				((Q)->COUNT++),(Q)->TAIL=(&((NEW)->LINK))))

#endif /* Q_APPEND_PROTO */

#ifndef Q_PREPEND_PROTO
	/*
	 * Q_PREPEND(q-pointer,element-pointer):
	 * hang element off of tail
	 * initialize end-of-list link
	 * set tail pointer address of link in new tail element
	 * Does not return a particularly useful value.
	 *
	 * This macro is equivalent to the following code:
	 *	struct queue *q; struct item *new;
	 *	if (q->head == NULL)
	 *		q->tail = &(new->next);
	 *	new->next = q->head;
	 *	q->head = new;
	 *	q->count++;
	 */
#define Q_PREPEND_PROTO(Q,NEW,HEAD,TAIL,LINK,COUNT) 			      \
			((((Q)->HEAD == NULL)&&((Q)->TAIL=(&((NEW)->LINK)))), \
			 ((NEW)->LINK=(Q)->HEAD),((Q)->HEAD=(NEW)),	      \
			 ((Q)->COUNT++))

#endif /* Q_PREPEND_PROTO */

#ifndef Q_FETCH_PROTO
	/*
	 * Q_FETCH(q-pointer,element-pointer):
	 * fetch first on queue
	 * if any, set head to link
	 * if empty, set tail to &head
	 * Does not return a particularly useful value.
	 *
	 * This macro is equivalent to the following code:
	 *	struct queue *q; struct item *e;
	 * 	if ((e = q->head) != NULL) {
	 *		q->count--;
	 *		if( (q->head = e->next) == NULL) {
	 *			q->tail = &(q->head);
	 *		}
	 *		e->next = NULL;
	 *	}
	 */
#define Q_FETCH_PROTO(Q,E,HEAD,TAIL,LINK,COUNT)  (((E)=(Q)->HEAD)\
			&&(((Q)->COUNT--),(((Q)->HEAD=(E)->LINK)\
				||(((Q)->TAIL)=(&((Q)->HEAD)))),\
				 ((E)->LINK=NULL)))

#endif /* Q_FETCH_PROTO */

#ifndef Q_INIT_PROTO
#define Q_INIT_PROTO(Q,HEAD,TAIL,COUNT)	\
			(((Q)->COUNT=0),\
			((Q)->HEAD=NULL),\
			((Q)->TAIL=(&((Q)->HEAD))))
#endif /* Q_INIT_PROTO */
