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
static char	*sccsid = "@(#)$RCSfile: cma_deb_event.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/12/10 18:13:06 $";
#endif 
/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	This module implements debugging events
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
 *		Initial creation of module
 *	002	Webb Scales	6 December 1990
 *		Made proto for signal_event conditional: on VMS only
 *	003	Paul Curtin	22 September 1992
 *		Turn on event reporting for OpenVMS Alpha AXP.
 */


/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_defs.h>
#include <cma_deb_event.h>
#include <cma_deb_core.h>

/*
 * GLOBAL DATA
 */

/*
 * LOCAL DATA
 */

/*
 * LOCAL MACROS
 */

/*
 * LOCAL FUNCTIONS
 */
#if _CMA_OS_ == _CMA__VMS
extern void
cma$$dbg_signal_event ();
#endif



/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Called to declare the a debugging event of a particular kind
 * 	has occurred.  
 * 
 *	If the event enable bits in the current thread indicate that the
 *	event has been enabled for the thread, or if the all-threads enable
 *	bit is set, then the event is triggered.  Triggering the event 
 *	causes program execution to stop, the event is announced, and the 
 *	debugger prompt appears.  Debugger in this context refers to
 *	the real debugger (if it has been trained to understand 
 *	events) or the CMA callable debugger.
 *
 *  FORMAL PARAMETERS:
 *
 *	event 	- The code defining which event is being declared
 *
 *  IMPLICIT INPUTS:
 *
 *	Event enable bits for all threads and for the current thread.
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	Changes made while in the debugger
 */

extern void
cma__debevt_report
#ifdef _CMA_PROTO_ 
	(
	cma__t_debevt 	event
	)
#else	/* no prototypes */
	(
	event
	)
	cma__t_debevt	event;
#endif	/* prototype */
    {
    cma__t_int_tcb	*tcb;		/* Current thread's TCB  */
    cma__t_int_tcb	*old_tcb;	/* Preempted thread's TCB  */
    cma_t_address	p1;		/* Parameters to event routine */

    tcb = cma__get_self_tcb ();		/* Get running thread */

    /*
     * Gather some additional info for certain events
     */
    if (event == cma__c_debevt_activating)
	p1 = (cma_t_address)tcb->debug.start_pc;
    else if (event == cma__c_debevt_preempting) {
	old_tcb = (cma__t_int_tcb *)tcb->debug.preempted_tcb;
	p1 = (cma_t_address)old_tcb->header.sequence;
	}

#if _CMA_OS_ == _CMA__VMS
    /* 
     * Pass it on to the BLISS code that knows how to inform the debugger
     */
    cma$$dbg_signal_event (
	event, 
	tcb,
	p1,
	0);	
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__debevt_report_2 is the same as cma__debevt_report except
 *	that two additional parameters are passed in.
 *
 *  FORMAL PARAMETERS:
 *
 *	event 	- The code defining which event is being declared
 *	p1	- Parameter 1 (event specific meaning)
 *	p2	- Parameter 2 (event specific meaning)
 *
 *  IMPLICIT INPUTS:
 *
 *	Event enable bits for all threads and for the current thread.
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	Changes made while in the debugger
 */

extern void
cma__debevt_report_2
#ifdef _CMA_PROTO_ 
	(
	cma__t_debevt 	event,
	cma_t_address	p1,
	cma_t_address	p2
	)
#else	/* no prototypes */
	(
	event,
	p1,
	p2
	)
	cma__t_debevt 	event;
	cma_t_address	p1;
	cma_t_address	p2;
#endif	/* prototype */
    {
    cma__t_int_tcb	*tcb;		/* Current thread's TCB  */

    tcb = cma__get_self_tcb ();		/* Get running thread */

#if _CMA_OS_ == _CMA__VMS
    /* 
     * Pass it on to the BLISS code that knows how to inform the debugger
     */
    cma$$dbg_signal_event (
	event, 
	tcb,
	p1,
	p2);	
#endif
    }


extern void
cma__debevt_notify
#ifdef _CMA_PROTO_ 
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma__t_int_tcb	*tcb;		/* Current thread's TCB  */

    tcb = cma__get_self_tcb ();		/* Get running thread */

#if _CMA_PLATFORM_ == _CMA__VAX_VMS
    /* 
     * Pass it on to the BLISS code that knows how to inform the debugger
     */
    cma$$dbg_notify (tcb);
#endif
    }


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Called to enable a debugging event of a particular kind.
 * 
 *	The event enable events in the thread or the global bits are
 *	set.
 *  
 *	NOTE: This is called from debugger context, so no locks may
 *	be taken out.
 *
 *  FORMAL PARAMETERS:
 *
 *	TCB	- The TCB for the thread whose events are to be set
 *		  If null, then the event is set globally, for all threads.
 *	Event 	- The code defining which event is being enabled
 *
 *  IMPLICIT INPUTS:
 *
 *	Event enable bits for all threads and for the current thread.
 *
 *  IMPLICIT OUTPUTS:
 *
 *	Event enable bits for all threads and for the current thread.
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	Changes made while in the debugger
 */

extern void
cma__debevt_set
#ifdef _CMA_PROTO_ 
	(
	cma__t_int_tcb	*tcb,
	cma__t_debevt 	event
	)
#else	/* no prototypes */
	(
	tcb, 
	event
	)
	cma__t_int_tcb	*tcb;
	cma__t_debevt	event;
#endif	/* prototype */
    {

    /*
     * When any event is set, turn on the event reporting code
     * and leave it on.
     */
    cma__g_debug_state.events_enabled = cma_c_true;
         
    /*
     * Select either the global flags or the TCB flags, and set
     */
    if (tcb == (cma__t_int_tcb *)cma_c_null_ptr)
	cma__g_debug_state.flags[(cma_t_integer)event]	= cma_c_true;
    else
	tcb->debug.flags[(cma_t_integer)event]		= cma_c_true;

    }


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Called to disable a debugging event of a particular kind.
 * 
 *	The event enable events in the thread or the global bits are
 *	cleared.
 *  
 *	NOTE: This is called from debugger context, so no locks may
 *	be taken out.
 *
 *  FORMAL PARAMETERS:
 *
 *	TCB	- The TCB for the thread whose event is to be cleared
 *		  If null, then the event is cleared globally, for all threads.
 *	Event 	- The code defining which event is being disabled.
 *
 *  IMPLICIT INPUTS:
 *
 *	Event enable bits for all threads and for the current thread.
 *
 *  IMPLICIT OUTPUTS:
 *
 *	Event enable bits for all threads and for the current thread.
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	Changes made while in the debugger
 */

extern void
cma__debevt_clear
#ifdef _CMA_PROTO_ 
	(
	cma__t_int_tcb	*tcb,
	cma__t_debevt 	event
	)
#else	/* no prototypes */
	(
	tcb, 
	event
	)
	cma__t_int_tcb	*tcb;
	cma__t_debevt	event;
#endif	/* prototype */
    {
         
    /*
     * Select either the global flags or the TCB flags, and set
     */
    if (tcb == (cma__t_int_tcb *)cma_c_null_ptr)
	cma__g_debug_state.flags[(cma_t_integer)event]	= cma_c_false;
    else
	tcb->debug.flags[(cma_t_integer)event]		= cma_c_false;

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Called to test if a debugging event of a particular kind is
 *	enabled.
 * 
 *	The event enable events in the thread or the global bits are
 *	tested.
 *  
 *	NOTE: This is called from debugger context, so no locks may
 *	be taken out.
 *
 *  FORMAL PARAMETERS:
 *
 *	TCB	- The TCB for the thread whose event is to be tested
 *		  If null, then the global flags are tested.
 *	Event 	- The code defining which event is being tested.
 *
 *  IMPLICIT INPUTS:
 *
 *	Event enable bits for all threads and for the current thread.
 *
 *  IMPLICIT OUTPUTS:
 *
 *	Event enable bits for all threads and for the current thread.
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	None
 */

extern cma_t_boolean
cma__debevt_test
#ifdef _CMA_PROTO_ 
	(
	cma__t_int_tcb	*tcb,
	cma__t_debevt 	event
	)
#else	/* no prototypes */
	(
	tcb, 
	event
	)
	cma__t_int_tcb	*tcb;
	cma__t_debevt	event;
#endif	/* prototype */
    {
         
    /*
     * Select either the global flags or the TCB flags, and set
     */
    if (tcb == (cma__t_int_tcb *)cma_c_null_ptr)
	return (
	    (cma_t_boolean)(cma__g_debug_state.flags[(cma_t_integer)event]));
    else
	return (
	    (cma_t_boolean)(tcb->debug.flags[(cma_t_integer)event]));

    }
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_DEB_EVENT.C */
/*  *3    23-SEP-1992 11:24:25 CURTIN "Turn on EVMS DEBUG events" */
/*  *2    10-JUN-1991 18:19:53 SCALES "Add sccs headers for Ultrix" */
/*  *1    12-DEC-1990 21:43:58 BUTENHOF "Debug support" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_DEB_EVENT.C */
