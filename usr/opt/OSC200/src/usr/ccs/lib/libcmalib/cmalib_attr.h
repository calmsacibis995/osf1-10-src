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
 * @(#)$RCSfile: cmalib_attr.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/03/16 21:58:48 $
 */
/*
 *  Copyright (c) 1990, 1992 by
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
 *	Concert Multithread Architecture (CMA) Library services
 *
 *  ABSTRACT:
 *
 *	Header file for library objects attributes objects
 *
 *  AUTHORS:
 *
 *	Webb Scales
 *
 *  CREATION DATE:
 *
 *	7 August 1990
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Webb Scales	28 August 1990
 *		Remove unused extern declaration.
 *	002	Dave Butenhof	01 June 1992
 *		Modify for new build environment
 */


#ifndef CMALIB_ATTR
#define CMALIB_ATTR

/*
 *  INCLUDE FILES
 */
#include <cmalib_defs.h>

/*
 * CONSTANTS AND MACROS
 */

/*
 * TYPEDEFS
 */
typedef struct CMA_LIB__T_CACHE {
    cma_t_natural		revision;	/* Revisions */
    cma_t_natural		count;
    cma_lib__t_queue		queue;		/* Cache headers */
    } cma_lib__t_cache;

typedef struct CMA_LIB__T_INT_ATTR {
    cma_lib__t_object		header;		/* Common header */
    struct CMA_LIB__T_INT_ATTR	*attributes;	/* Point to controlling attr */
    cma_t_mutex			mutex;		/* Serialize access to object */
    cma_t_natural		queue_size;	/* Number of nodes allocated */
    cma_lib__t_cache		cache[cma_lib__c_obj_num];
    cma_t_boolean		delete_pending;	/* attr. obj. is deleted */
    cma_t_natural		refcnt;	/* Number of objects using attr. obj */
    } cma_lib__t_int_attr;

/*
 *  GLOBAL DATA
 */

extern cma_lib__t_int_attr	cma_lib__g_def_attr;

/*
 * INTERNAL INTERFACES
 */

extern void
cma_lib__destroy_attributes _CMA_PROTOTYPE_ ((
	cma_lib__t_int_attr	*old_attr));	/* Deallocate (don't cache) */

extern void
cma_lib__free_attributes _CMA_PROTOTYPE_ ((
	cma_lib__t_int_attr	*old_attr));	/* Attributes object to free */

extern cma_lib__t_int_attr *
cma_lib__get_attributes _CMA_PROTOTYPE_ ((
	cma_lib__t_int_attr	*attrib));	/* Attributes object to use */

extern void
cma_lib__init_attr _CMA_PROTOTYPE_ ((void));

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_ATTR.H */
/*  *3     1-JUN-1992 14:39:41 BUTENHOF "Modify for new build environment" */
/*  *2    28-AUG-1990 16:34:26 SCALES "Remove unused extern" */
/*  *1    27-AUG-1990 02:15:33 SCALES "" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_ATTR.H */
