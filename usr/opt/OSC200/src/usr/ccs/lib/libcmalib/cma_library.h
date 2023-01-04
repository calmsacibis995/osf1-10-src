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
 * @(#)$RCSfile: cma_library.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/03/16 21:57:55 $
 */
/*
 *  Copyright (c) 1990, 1993 by
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
 *	DECthreads Library services
 *
 *  ABSTRACT:
 *
 *	External definitions for DECthreads Library services
 *
 *  AUTHORS:
 *
 *	Webb Scales
 *
 *  CREATION DATE:
 * 
 *	7 August 1990
 *
 *  MODIFIED BY:
 *
 *	Webb Scales
 *	Dave Butenhof
 * 
 */

#ifndef CMA_LIB_INCLUDE
#define CMA_LIB_INCLUDE

#include <cma.h>

#ifdef __cplusplus
    extern "C" {
#endif

/*
 * The following are the portable CMA High-Level Library definitions
 */

/*
 * Operations on attributes objects
 */

typedef cma_t_handle	cma_lib_t_attr;

/*
 * An attributes object is created to specify the attributes of other CMA
 * objects that will be created.
 */
extern void
cma_lib_attr_create _CMA_PROTOTYPE_ ((
	cma_lib_t_attr	*new_attr,
	cma_lib_t_attr	*attr));

/*
 * An attributes object can be deleted when it is no longer needed.
 */
extern void
cma_lib_attr_delete _CMA_PROTOTYPE_ ((
	cma_lib_t_attr	*attr));

/*
 * Operations on queues
 */

typedef cma_t_handle	cma_lib_t_queue;

extern void
cma_lib_queue_create _CMA_PROTOTYPE_ ((
	cma_lib_t_queue	*new_queue,
	cma_lib_t_attr  *att));

extern void
cma_lib_queue_delete _CMA_PROTOTYPE_ ((
	cma_lib_t_queue	*queue));

extern void
cma_lib_queue_dequeue _CMA_PROTOTYPE_ ((
	cma_lib_t_queue	*queue,
	cma_t_address   *element));

extern void
cma_lib_queue_enqueue _CMA_PROTOTYPE_ ((
	cma_lib_t_queue	*queue,
	cma_t_address   element));

extern cma_t_boolean
cma_lib_queue_try_enqueue_int _CMA_PROTOTYPE_ ((
	cma_lib_t_queue	*queue,
	cma_t_address   element));

extern void
cma_lib_queue_requeue _CMA_PROTOTYPE_ ((
	cma_lib_t_queue	*queue,
	cma_t_address   element));

extern cma_t_boolean
cma_lib_queue_try_dequeue _CMA_PROTOTYPE_ ((
	cma_lib_t_queue	*queue,
	cma_t_address   *element));

extern cma_t_boolean
cma_lib_queue_try_enqueue _CMA_PROTOTYPE_ ((
	cma_lib_t_queue	*queue,
	cma_t_address   element));

extern cma_t_boolean
cma_lib_queue_try_requeue _CMA_PROTOTYPE_ ((
	cma_lib_t_queue	*queue,
	cma_t_address   element));

/*
 * Set or obtain the default queue size
 */
extern void
cma_lib_attr_get_queuesize _CMA_PROTOTYPE_ ((
	cma_lib_t_attr	*attr,
	cma_t_natural	*queuesize));

extern void
cma_lib_attr_set_queuesize _CMA_PROTOTYPE_ ((
	cma_lib_t_attr	*attr,
	cma_t_natural	queuesize));

#ifdef __cplusplus
    }
#endif

#endif
