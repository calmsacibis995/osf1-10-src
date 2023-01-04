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
 *	@(#)$RCSfile: cma_once.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/08/18 14:49:01 $
 */
/*
 *  FACILITY:
 *
 *	Concert Multithread Architecture (CMA) services
 *
 *  ABSTRACT:
 *
 *	Header file for cma_once.c
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	19 July 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	12 October 1989
 *		Convert to use internal mutex operations.
 *	002	Dave Butenhof	27 August 1990
 *		Change flag enum to begin at 0 rather than 1, to make static
 *		initialization easier (for languages without type-based init
 *		or macros).
 *	003	Paul Curtin	21 May 1991
 *		Added cma__g_once_mutexes: new queue header
 *	004	Paul Curtin	24 May 1991
 *		Added a prototype for cma__reinit_once.
 *	005	Dave Butenhof	 4 June 1993
 *		Remove support for queue of once block mutexes (which was
 *		never used).
 */

#ifndef CMA_ONCE
#define CMA_ONCE

/*
 *  INCLUDE FILES
 */

#include <cma_mutex.h>

/*
 * CONSTANTS AND MACROS
 */

/*
 * TYPEDEFS
 */

typedef enum CMA__T_ONCE_FLAG {
    cma__c_once_uninit = 0,
    cma__c_once_initing = 1,
    cma__c_once_inited = 2
    } cma__t_once_flag;

typedef struct CMA__T_INT_ONCE {
    cma_t_integer	mbz;
    cma__t_int_mutex	*mutex;
    cma__t_once_flag	flag;
    } cma__t_int_once;

/*
 *  GLOBAL DATA
 */

/*
 * INTERNAL INTERFACES
 */

extern void
cma__init_once _CMA_PROTOTYPE_ ((void));

extern void
cma__reinit_once _CMA_PROTOTYPE_ ((
	cma_t_integer	flag));

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_ONCE.H */
/*  *6     4-JUN-1993 11:29:14 BUTENHOF "Delete once block queues" */
/*  *5    10-JUN-1991 19:54:43 SCALES "Convert to stream format for ULTRIX build" */
/*  *4    10-JUN-1991 19:21:13 BUTENHOF "Fix the sccs headers" */
/*  *3    10-JUN-1991 18:22:38 SCALES "Add sccs headers for Ultrix" */
/*  *2     5-JUN-1991 16:13:55 CURTIN "fork work" */
/*  *1    12-DEC-1990 21:47:51 BUTENHOF "Client one-time init" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_ONCE.H */
