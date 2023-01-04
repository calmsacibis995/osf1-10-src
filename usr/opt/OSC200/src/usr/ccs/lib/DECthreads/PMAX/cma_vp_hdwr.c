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
static char *rcsid = "@(#)$RCSfile: cma_vp_hdwr.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/09/29 14:34:39 $";
#endif
/*
 *  Copyright (c) 1992 by
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
 *  and  should  not  be  construed  as  a commitment by Digital Equipment
 *  Corporation.
 *
 *  Digital assumes no responsibility for the use or  reliability  of  its
 *  software on equipment which is not supplied by Digital.
 */

/*
 *  FACILITY:
 *
 *	DECthreads services
 *
 *  ABSTRACT:
 *
 *	Implement hardware-specific "virtual processor" functions to help out
 *	VP layer. For MIPS R2000/R3000 architecture.
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	28 July 1992
 *
 *  MODIFICATION HISTORY:
 *
 */

/*
 *  INCLUDE FILES
 */

#if _CMA_KTHREADS_ == _CMA__MACH	/* Module is NULL if not Mach */
#include <cma.h>
#include <cma_defs.h>
#include <cma_thread.h>
#include <cma_vp.h>
#include <cma_vm.h>
#include <cma_assem.h>
# include <mach.h>
# include <mach_error.h>
# include <mach/machine/syscall_sw.h>

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

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	interrupt a VP
 *
 *  FORMAL PARAMETERS:
 *
 *	vpid		Which VP to interrupt
 *	handler		Address of routine to call in target VP
 *	arg		Argument to pass to interrupt handler
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
 *	status of operation
 *
 *  SIDE EFFECTS:
 *
 *	none
 * 
 */
cma__t_vp_status
cma__vp_interrupt
#ifdef _CMA_PROTO_
	(cma__t_vpid		vpid,
	cma__t_vp_handler	handler,
	cma_t_address		arg)
#else	/* no prototypes */
	(vpid, handler, arg)
	cma__t_vpid		vpid;
	cma__t_vp_handler	handler;
	cma_t_address		arg;
#endif	/* prototype */
    {
    struct mips_thread_state	state_array;
    unsigned int		state_count = MIPS_THREAD_STATE_COUNT;
    long int			*stack_array;
    kern_return_t		status;


    cma__trace ((
	    cma__c_trc_vp,
	    "(vp_interrupt) interrupting vp %ld to %08lx(%08lx)",
	    vpid->vp,
	    handler,
	    arg));

    /*
     * First, suspend the target thread
     */
    if ((status = thread_suspend (vpid->vp)) != KERN_SUCCESS) {
	cma__trace ((
		cma__c_trc_vp | cma__c_trc_bug,
		"(vp_interrupt) error \"%s\" (%ld) suspending vp %ld",
		mach_error_string (status),
		status,
		vpid->vp));
	cma__bugcheck ("vp_interrupt: thread_suspend");
	}

    /*
     * Now, abort any kernel wait in the target so it won't be restarted
     * when the thread wakes up.
     */
    if ((status = thread_abort (vpid->vp)) != KERN_SUCCESS) {
	cma__trace ((
		cma__c_trc_vp | cma__c_trc_bug,
		"(vp_interrupt) error \"%s\" (%ld) aborting operation on vp %ld",
		mach_error_string (status),
		status,
		vpid->vp));
	cma__bugcheck ("vp_interrupt: thread_abort");
	}

    /*
     * Set up for the async call
     */

    if ((status = thread_get_state (
	    vpid->vp,
	    MIPS_THREAD_STATE,
	    (thread_state_t)&state_array,
	    &state_count)) != KERN_SUCCESS) {
	cma__trace ((
		cma__c_trc_vp | cma__c_trc_bug,
		"(vp_interrupt) error \"%s\" (%ld) getting state on vp %ld",
		mach_error_string (status),
		status,
		vpid->vp));
	cma__bugcheck ("vp_interrupt: thread_get_state");
	}

    stack_array = (long int *)(state_array.r29 - (4 * sizeof (cma_t_integer)));
    stack_array[0] = state_array.pc;	/* Save pc */
    state_array.pc = (cma_t_integer)cma__do_interrupt;
    stack_array[1] = state_array.r16;	/* Save s0 */
    state_array.r16 = (cma_t_integer)handler;
    stack_array[2] = state_array.r4;	/* Save a0 */
    state_array.r4 = (cma_t_integer)arg;
    state_array.r29 = (cma_t_integer)stack_array;

    if ((status = thread_set_state (
	    vpid->vp,
	    MIPS_THREAD_STATE,
	    (thread_state_t)&state_array,
	    MIPS_THREAD_STATE_COUNT)) != KERN_SUCCESS) {
	cma__trace ((
		cma__c_trc_vp | cma__c_trc_bug,
		"(vp_interrupt) error \"%s\" (%ld) setting state on vp %ld",
		mach_error_string (status),
		status,
		vpid->vp));
	cma__bugcheck ("vp_interrupt: thread_set_state");
	}

    /*
     * Now let it run the new code
     */
    if ((status = thread_resume (vpid->vp)) != KERN_SUCCESS) {
	cma__trace ((
		cma__c_trc_vp | cma__c_trc_bug,
		"(vp_interrupt) error \"%s\" (%ld) resuming vp %ld",
		mach_error_string (status),
		status,
		vpid->vp));
	cma__bugcheck ("vp_interrupt: thread_resume");
	}

    return status;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Set up initial VP stack and start function
 *
 *  FORMAL PARAMETERS:
 *
 *	vpid		ID of kernel thread
 *	state		state array
 *
 *  IMPLICIT INPUTS:
 *
 *	the current kernel thread
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	Mach status
 *
 *  SIDE EFFECTS:
 *
 *	none
 * 
 */
cma__t_vp_status
cma__vp_set_start
#ifdef _CMA_PROTO_
	(cma__t_vpid	vpid,
	cma__t_vp_state	*state)
#else	/* no prototypes */
	(vpid, state)
	cma__t_vpid	vpid;
	cma__t_vp_state	*state;
#endif	/* prototype */
    {
    kern_return_t		status;
    struct mips_thread_state	thd_state;
    cma_t_integer		*stack_array;
    int				state_count = MIPS_THREAD_STATE_COUNT;


    stack_array = (cma_t_integer *)(state->stack - (2 * sizeof (cma_t_integer)));
    stack_array[0] = state->tcb;	/* a0 */
    stack_array[1] = 0;			/* Clear out a null a1 */

    /*
     * Set up mips call frame and registers.  First, zero the area.
     */
    bzero((char *)&thd_state, sizeof(struct mips_thread_state));
    thd_state.r28 = (cma_t_integer)(cma__fetch_gp ());	/* Get our gp */
    thd_state.r29 = (cma_t_integer)stack_array;
    thd_state.pc = (cma_t_integer)cma__execute_thread;
    status = thread_set_state (
	    vpid->vp,
	    MIPS_THREAD_STATE,
	    (thread_state_t)&thd_state,
	    MIPS_THREAD_STATE_COUNT);

    /*
     * Make sure the mach call works
     */

    if (status != KERN_SUCCESS)
	cma__bugcheck ("cma__vp_set_start: 1");

    return status;
    }
#endif
