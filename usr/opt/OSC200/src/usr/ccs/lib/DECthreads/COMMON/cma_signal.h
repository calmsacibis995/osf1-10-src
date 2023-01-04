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
 *	@(#)$RCSfile: cma_signal.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/08/06 17:39:39 $
 */

/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	Header file for signal operations
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	23 May 1991
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Paul Curtin	3 June 1991
 *		Added a prototype for new reinit routine.
 *	002	Webb Scales	7 April 1991
 *		Set up for building on VMS.
 */

#ifndef CMA_SIGNAL
#define CMA_SIGNAL

/*
 *  INCLUDE FILES
 */

#include <cma_tcb_defs.h>

/*
 * CONSTANTS AND MACROS
 */

/*
 *  TYPE DEFINITIONS
 */

/*
 *  GLOBAL DATA
 */

/*
 * EXTERNAL INTERFACES
 */

#if _CMA_OS_ != _CMA__VMS
extern void
cma__abort_process _CMA_PROTOTYPE_ ((
	cma_t_integer		abort_signal));
#endif

extern void
cma__init_signal _CMA_PROTOTYPE_ ((void));

extern void
cma__reinit_sigal _CMA_PROTOTYPE_ ((
	cma_t_integer		flag));

extern cma_t_boolean
cma__sig_deliver _CMA_PROTOTYPE_ ((
	int			sig,
	int			code,
	struct	sigcontext	*scp));

#if _CMA_OSIMPL_ == _CMA__OS_OSF
extern void
cma__sig_thread_init _CMA_PROTOTYPE_ ((void));
#endif

#endif
/*  DEC/CMS REPLACEMENT HISTORY, Element CMA_SIGNAL.H */
/*  *6    16-APR-1992 18:11:05 SCALES "Set up for building on VMS" */
/*  *5    10-JUN-1991 19:56:14 SCALES "Convert to stream format for ULTRIX build" */
/*  *4    10-JUN-1991 19:21:37 BUTENHOF "Fix the sccs headers" */
/*  *3    10-JUN-1991 18:23:38 SCALES "Add sccs headers for Ultrix" */
/*  *2     5-JUN-1991 16:14:28 CURTIN "fork work" */
/*  *1    29-MAY-1991 17:51:27 BUTENHOF "signals" */
/*  DEC/CMS REPLACEMENT HISTORY, Element CMA_SIGNAL.H */
