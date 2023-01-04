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
 *	@(#)$RCSfile: cma_list.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:57:30 $
 */
/*
 *  Copyright (c) 1990 by
 *  Digital Equipment Corporation, Maynard Massachusetts.
 *  All rights reserved.
 *
 *  This software is furnished under a license and may be used and  copied
 *  only  in  accordance  with  the  terms  of  such  license and with the
 *  inclusion of the above copyright notice.  This software or  any  other
 *  copies  thereof may not be provided or otherwise made available to any
 *  other person.  No title to and ownership of  the  software  is  hereby
 *  transferred.
 *
 *  The information in this software is subject to change  without  notice
 *  and  should  not  be  construed  as  a commitment by DIGITAL Equipment
 *  Corporation.
 *
 *  DIGITAL assumes no responsibility for the use or  reliability  of  its
 *  software on equipment which is not supplied by DIGITAL.
 */

/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	Header file for generic list functions operating on singly linked
 *	null-terminated lists.  Items may not be REMOVED from the list!  The
 *	intent is that the list can be traversed (for read-only operations)
 *	without locking, since insertion is "safe" (though not truely
 *	atomic).  THIS ASSUMES THAT THE HARDWARE MAKES WRITES VISIBLE TO READS
 *	IN THE ORDER IN WHICH THEY OCCURRED!  WITHOUT SUCH READ/WRITE
 *	ORDERING, IT MAY BE NECESSARY TO INSERT "BARRIERS" TO PRODUCE THE
 *	REQUIRED VISIBILITY!
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	26 January 1990
 *
 *  MODIFICATION HISTORY:
 *
 */

#ifndef CMA_LIST
#define CMA_LIST

/*
 *  INCLUDE FILES
 */

#include <cma.h>

/*
 * CONSTANTS AND MACROS
 */

#define cma__c_null_list	((cma__t_list *)cma_c_null_ptr)

/*
 * Test whether a list is empty.  Return cma_c_true if so, else
 * cma_c_false.
 */
#define cma__list_empty(head)	((head)->link == cma__c_null_list)

/*
 * Initialize a queue header to empty.
 */
#define cma__list_init(head)	(void)((head)->link = cma__c_null_list)

/*
 * Insert an element in a list following the specified item (or at the
 * beginning of the list if "list" is the list head).  NOTE: insertion
 * operations should be interlocked by the caller!
 */
#define cma__list_insert(element,list)    (void)(	\
    (element)->link		= (list)->link,		\
    (list)->link		= (element))

/*
 * Return the next item in a list (or the first, if the address is of the
 * list header)
 */
#define cma__list_next(element)    ((element)->link)

/*
 * TYPEDEFS
 */

typedef struct CMA__T_LIST {
    struct CMA__T_LIST	*link;		/* Forward link */
    } cma__t_list;

/*
 *  GLOBAL DATA
 */

/*
 * INTERNAL INTERFACES
 */

#endif
/*  DEC/CMS REPLACEMENT HISTORY, Element CMA_LIST.H */
/*  *4    10-JUN-1991 19:54:14 SCALES "Convert to stream format for ULTRIX build" */
/*  *3    10-JUN-1991 19:21:06 BUTENHOF "Fix the sccs headers" */
/*  *2    10-JUN-1991 18:22:20 SCALES "Add sccs headers for Ultrix" */
/*  *1    12-DEC-1990 21:47:12 BUTENHOF "Atomic lists" */
/*  DEC/CMS REPLACEMENT HISTORY, Element CMA_LIST.H */
