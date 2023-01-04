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
static char *rcsid = "@(#)$RCSfile: cmalib_queue.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/03/16 21:59:13 $";
#endif
/*
 *  Copyright (c) 1991, 1992 by
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
 *	CMA Library services
 *
 *  ABSTRACT:
 *
 *	CMA library queuing primitives
 *
 *  AUTHORS:
 *
 *	Paul Curtin
 *
 *  CREATION DATE:
 *
 *	20 February 1991
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	01 June 1992
 *		Modify for new build environment
 */

/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_library.h>
#include <cmalib_defs.h>
#include <cmalib_queue.h>

/*
 * LOCAL DATA
 */

/*
 * LOCAL FUNCTIONS
 */

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Remove the first element from a queue.  Return a pointer to the element.
 *
 *  FORMAL PARAMETERS:
 *
 *	head  -  a pointer to the head of the queue
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	Returns a pointer to the object which was removed from the queue.
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern cma_lib__t_queue *
cma_lib__queue_dequeue
#ifdef _CMA_PROTO_
	(
	cma_lib__t_queue	*head)
#else	/* no prototypes */
	(head)
	cma_lib__t_queue	*head;
#endif	/* prototype */
    {
    cma_lib__t_queue	*element 	= head->flink;


    cma_lib__assert_fail (
	    head != (cma_lib__t_queue *) cma_c_null_ptr, 
	    "cma_lib__queue_dequeue: attempting dequeue using a null queue head");

    cma_lib__assert_fail (
	    ((head->flink != (cma_lib__t_queue *)cma_c_null_ptr)
	    && (head->blink != (cma_lib__t_queue *)cma_c_null_ptr)),
	    "cma_lib__queue_dequeue: dequeue on uninitialized queue head");

    /*
     * If the queue is empty, return the null pointer; otherwise remove and
     * return the first element.
     */
    if (head == element)
	return (cma_lib__t_queue *)cma_c_null_ptr;
    else
	return cma_lib__queue_remove (element);

    }

