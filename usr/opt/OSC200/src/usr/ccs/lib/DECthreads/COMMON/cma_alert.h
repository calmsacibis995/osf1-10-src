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
 *	@(#)$RCSfile: cma_alert.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/12/10 18:10:34 $
 */
/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	Header file for ALERT functions
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	14 September 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	27 October 1989
 *		Make cma___attempt_delivery externally visible.
 *	002	Dave Butenhof	4 December 1989
 *		Include cma_tcb_defs.h instead of cma_tcb.h
 *	003	Dave Butenhof	15 August 1990
 *		Add new function cma__asynch_delivery, used by assembler code
 *		to call cma__attempt_delivery with appropriate TCB (since
 *		the assembler code doesn't have access to the macros
 *		specifying how to get the current TCB).
 *	004	Paul Curtin	 8 May 1991
 *		Added the cma__int_alert_test macro
 *	005	Dave Butenhof	01 September 1992
 *		Remove locking requirements for alert.
 */

#ifndef CMA_ALERT
#define CMA_ALERT

/*
 *  INCLUDE FILES
 */
#include <cma_tcb_defs.h>

/*
 * CONSTANTS AND MACROS
 */

/*
 * TYPEDEFS
 */

/*
 *  GLOBAL DATA
 */

/*
 * MACROS
 */

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *  cma__int_alert_test - Performs work for cma_alert_test
 *
 *  FORMAL PARAMETERS:
 *
 *	none
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
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */

#define cma__int_alert_test() { \
    cma__t_int_tcb      *_tcb_; \
    _tcb_ = cma__get_self_tcb (); \
    cma__attempt_delivery (_tcb_); \
    }


/*
 * INTERNAL INTERFACES
 */

extern void
cma__async_delivery _CMA_PROTOTYPE_ ((void));

extern void
cma__attempt_delivery _CMA_PROTOTYPE_ ((
	cma__t_int_tcb	*tcb));		/* TCB to check */

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_ALERT.H */
/*  *7     2-SEP-1992 16:23:28 BUTENHOF "Update" */
/*  *6    10-JUN-1991 19:50:09 SCALES "Convert to stream format for ULTRIX build" */
/*  *5    10-JUN-1991 19:19:54 BUTENHOF "Fix the sccs headers" */
/*  *4    10-JUN-1991 18:16:37 SCALES "Add sccs headers for Ultrix" */
/*  *3    10-MAY-1991 11:09:22 CURTIN "added cma__int_alert_test macro" */
/*  *2    12-FEB-1991 01:28:35 BUTENHOF "New alert control primitives" */
/*  *1    12-DEC-1990 21:40:34 BUTENHOF "alerts" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_ALERT.H */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_ALERT.H */
/*  *8     5-NOV-1992 14:23:59 BUTENHOF "Fix async alert" */
/*  *7     2-SEP-1992 16:23:28 BUTENHOF "Update" */
/*  *6    10-JUN-1991 19:50:09 SCALES "Convert to stream format for ULTRIX build" */
/*  *5    10-JUN-1991 19:19:54 BUTENHOF "Fix the sccs headers" */
/*  *4    10-JUN-1991 18:16:37 SCALES "Add sccs headers for Ultrix" */
/*  *3    10-MAY-1991 11:09:22 CURTIN "added cma__int_alert_test macro" */
/*  *2    12-FEB-1991 01:28:35 BUTENHOF "New alert control primitives" */
/*  *1    12-DEC-1990 21:40:34 BUTENHOF "alerts" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_ALERT.H */
