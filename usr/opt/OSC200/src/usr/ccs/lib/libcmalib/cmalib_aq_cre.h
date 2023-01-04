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
 * @(#)$RCSfile: cmalib_aq_cre.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/03/16 21:58:06 $
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
 *	CMA Library services
 *
 *  ABSTRACT:
 *
 *	Header file for external Library (Atomic) Queue services
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
 *	001	Paul Curtin	20 March 1991
 *		Added cma_lib__c_default_queue here from platform
 *		includes.
 *	002	Dave Butenhof	01 June 1992
 *		Modify for new build environment
 *	003	Dave Butenhof	03 June 1992
 *		Repair accidental open comment deletion.
 */


#ifndef CMALIB_AQ_CRE
#define CMALIB_AQ_CRE

/*
 *  INCLUDE FILES
 */
#include <cma.h>
#include <cma_library.h>
#include <cmalib_defs.h>
#include <cmalib_aq_op.h>

/*
 * CONSTANTS AND MACROS
 */

/*
 * Default attribute value for queue allocation.
 */
#define cma_lib__c_default_queue    128


/*
 * TYPEDEFS
 */
struct	CMA_LIB__T_INT_ATTR;

typedef struct CMA_LIB__T_QUEUE_OBJ {
	cma_lib__t_object		header;		/* Common header */
	struct	CMA_LIB__T_INT_ATTR	*attributes;	/* Back link */
	cma_lib__t_aq_header		queue_head;	/* Platform specific */
	cma_t_mutex			mutex;
	cma_t_cond			insert;
	cma_t_cond			remove;
	} cma_lib__t_queue_obj;

typedef struct CMA_LIB__T_QUEUE_ITEM {
	cma_lib__t_aq_link	node;		/* Must be first field */
	cma_t_address		item;
	} cma_lib__t_queue_item;


/*
 *  GLOBAL DATA
 */

/*
 * INTERNAL INTERFACES
 */
extern void
cma_lib__destroy_queue_obj _CMA_PROTOTYPE_ ((
	cma_lib__t_queue_obj	*queue_obj));

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_AQ_CREDEL.H */
/*  *6     3-JUN-1992 07:10:11 BUTENHOF "Fix typo" */
/*  *5     1-JUN-1992 14:39:28 BUTENHOF "Modify for new build environment" */
/*  *4    23-MAY-1991 13:46:45 BUTENHOF "Satisfy new MIPS C compiler" */
/*  *3    20-MAR-1991 16:45:52 CURTIN "adding c_default_queue from plaform .h's" */
/*  *2    29-AUG-1990 17:03:00 SCALES "Convert to stream format" */
/*  *1    27-AUG-1990 02:14:59 SCALES "" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_AQ_CREDEL.H */
