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
 *	@(#)$RCSfile: cma_deb_event.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:56:28 $
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
 *	Header file for debugging events
 *
 *  AUTHORS:
 *
 *	Bob Conti
 *
 *  CREATION DATE:
 *
 *	9 September 1990
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Bob Conti	9 September 1990
 *		Create module
 */

#ifndef CMA_DEB_EVENT
#define CMA_DEB_EVENT

/*
 *  INCLUDE FILES
 */
#include <cma.h>
#include <cma_tcb_defs.h>	/* Contains the debug event definitions */

/*
 * CONSTANTS AND MACROS
 */

/* Compile time flag to enable the event reporting code */
/* 
 * FIX-ME: This should come from the compile qualifier eventually so
 * that the dispatcher can be compiled two ways: with and without 
 * the event reporting code.
 */
#define _CMA_DEBEVT_	1

/*
 * TYPEDEFS
 */

/*
 *  GLOBAL DATA
 */

/*
 * INTERNAL INTERFACES
 */

/* 
 * Routines to report that a CMA debugging-event has occurred 
 */
extern void
cma__debevt_report _CMA_PROTOTYPE_ ((
	cma__t_debevt 	event
	));

extern void
cma__debevt_report_2 _CMA_PROTOTYPE_ ((
	cma__t_debevt 	event,
	cma_t_address 	p1,
	cma_t_address 	p2
	));

/* 
 * Routine to notify debugger that a thread it requested is about to run
 */
extern void
cma__debevt_notify _CMA_PROTOTYPE_ ((
	void
	));

/* 
 * Routine to enable a CMA debugging-event 
 */
extern void
cma__debevt_set _CMA_PROTOTYPE_ ((
	cma__t_int_tcb	*tcb,
	cma__t_debevt 	event
	));

/* 
 * Routine to disable a CMA debugging-event 
 */
extern void
cma__debevt_clear _CMA_PROTOTYPE_ ((
	cma__t_int_tcb	*tcb,
	cma__t_debevt 	event
	));

/* 
 * Routine to test a CMA debugging-event 
 */
extern cma_t_boolean
cma__debevt_test    _CMA_PROTOTYPE_ ((
	cma__t_int_tcb	*tcb,
	cma__t_debevt 	event
	));

#endif


/*  DEC/CMS REPLACEMENT HISTORY, Element CMA_DEB_EVENT.H */
/*  *4    10-JUN-1991 19:52:13 SCALES "Convert to stream format for ULTRIX build" */
/*  *3    10-JUN-1991 19:20:29 BUTENHOF "Fix the sccs headers" */
/*  *2    10-JUN-1991 18:20:10 SCALES "Add sccs headers for Ultrix" */
/*  *1    12-DEC-1990 21:44:00 BUTENHOF "Debug support" */
/*  DEC/CMS REPLACEMENT HISTORY, Element CMA_DEB_EVENT.H */
