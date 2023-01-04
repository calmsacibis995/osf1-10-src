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
static char *rcsid = "@(#)$RCSfile: xpt.c,v $ $Revision: 1.1.12.3 $ (DEC) $Date: 1993/09/22 12:58:12 $";
#endif

/* ---------------------------------------------------------------------- */

/* xpt.c		Version 1.14			Dec. 10, 1991 

This file contains the routines and functional descriptions that makeup the
XPT layer. 

This file is divided into 3 parts, routines accessable from the
outside by the PDrvs, internal only routines, and possible descriptions for
routines needed by the XPT like ones for the SIM.

Modification History

	Version	  Date		Who	Reason

	1.00	09/26/90	jag	Creation date. Created from the XPT
					notes and functional spec.
	1.01	11/12/90	jag	Changes from the func. spec. review
					and general finishing up.
	1.02	12/03/90	jag	Corrected the free Q removal code for 
					a single packet left.  Added updating
					the busy counter in XPT Qhead.  Updated
					the command entry table to Rev. 2.1 of
					the CAM spec.  All VU commands will go
					to the specified SIM.
	1.03	12/10/90	jag	Added the changes from the code review.
	1.04	02/04/91	janet	Modified xpt_pool_free to use a temp.
					pointer during the freeing of the
					packets.
	1.05	03/21/91	janet	include dec_cam.h		       
	1.06	04/05/91	jag	Changes for the ISV/IHV kit.  Added
					better locking.
	1.07	04/07/91	jag	Initialized the Q head lock.
	1.08	05/08/91	jag	Added the code to support Path Inquiry.
	1.09	07/03/91	jag	Put in the support for Async callback
					and SIM bus_{de,}register.
	1.10	07/23/91	jag	Fixed bugs in the Async Callback code.
					Checks on the incomming arguments and
					passing the correct ones to the PDrvs.
	1.11	08/06/91	jag	Corrected a PRINTD() output.
	1.12	11/13/91	jag	Added Error Logging support.
	1.13	12/06/91	jag	Fixed the bug in xpt_sim_func().  The
					sim_path_id is now correctly checked
					aginst the end of the cam_conftbl[].
	1.14	12/10/91	janet	Removed bzero of SIM_WS in ccb_alloc()
*/

/* ---------------------------------------------------------------------- */
/* Include files. */

#include <io/common/iotypes.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <io/cam/dec_cam.h>
#include <mach/vm_param.h>
#include <io/cam/cam.h>
#include <io/cam/scsi_all.h>
#include <io/cam/pdrv.h>
#include <kern/thread.h>
#include <kern/sched.h>
#include <kern/sched_prim.h>
#include <kern/parallel.h>
#include <vm/vm_kern.h>
#include <io/cam/cam_debug.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/dme.h>
#include <io/cam/sim.h>
#include <io/cam/uagt.h>
#include <io/cam/xpt.h>
#include <io/cam/ccfg.h>

/* ---------------------------------------------------------------------- */
/* Defines and includes for Error logging. */

#define CAMERRLOG			/* Turn on the error logging code */

#include <dec/binlog/errlog.h>	/* UERF errlog defines */
#include <io/cam/cam_errlog.h>
#include <io/cam/cam_logger.h>		/* CAM error logging definess */

/* ---------------------------------------------------------------------- */
/* Local routines */
void xpt_pool_alloc_thread();


/* ---------------------------------------------------------------------- */
/* Local defines. */

#define XPT_ERR_CNT	5		/* error entry list count */

/* ---------------------------------------------------------------------- */
/* External declarations. */

extern int hz;
extern U32 cam_subsystem_ready;	/* global flag to signal all is ready */

extern U32 cam_ccb_pool_size;	/* initial size for the pool */
extern U32 cam_ccb_low_water;	/* lower limit for FREE CCBs */
extern U32 cam_ccb_increment;	/* number to add when low is reached */
extern int shutting_down;	/* System going down???		*/

extern CAM_SIM_ENTRY *cam_conftbl[];	/* array containing the SIM entries */
extern U32 N_cam_conftbl;		/* indicating the number of entries */

extern EDT *edt_dir[];			/* ptrs for EDT grid per HBA */

extern U32 ccfg_initialize();	/* initialize the CAM subsystem */
extern U32 ccfg_action();		/* do something with a CCB */
extern U32 ccfg_simattach();		/* init and attach a specific SIM */
extern void panic();			/* general purpose "Aieeee" routine */
extern void bzero();			/* gereral purpose 0 fill routine */
extern void cam_logger();		/* the error log handler for CAM */

extern void xpt_errlog();		/* local error logging routine */

#if defined(CAMDEBUG)
extern caddr_t cdbg_CamFunction();	/* sprt routine to dump func code */
#endif /* defined(CAMDEBUG) */

/* ---------------------------------------------------------------------- */
/* Initialized and uninitialized data. */

XPT_QHEAD xpt_qhead;		/* header struct for the packet pool */

CAM_SIM_ENTRY xpt_sim_entry =	/* fake place holder for cam_conftbl */
{
    NULL,			/* no init routine */
    NULL			/* no action routine */
};

XPT_CTRL cam_conf_ctrl;		/* global control struct for cam_conftbl[] */

XPT_COMPLETE_QUE xpt_cb_queue;	/* The callback queue			*/
int xpt_callback_thread_init = 0;
int xpt_pool_alloc_thread_init = 0;

static void (*local_errlog)() = xpt_errlog; /* static ptr for the local func */
static void xpt_alloc_dcp();

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/*
Routine Name : xpt_init

Functional Description :
    This routine is called from each of the PDrv's during/prior to
    their own initialization process.  If the CAM subsystem has not
    yet been initialized, this routine will call the CDrv to get everyone
    ready.  Any subsequent calls once the subsystem is all set, this
    routine will simply return.  

Formal Parameters : None

Implicit Inputs :
    BOOL cam_subsystem_ready 		Global boolean flag owned by the CDrv

Implicit Outputs :
    All global and internal CAM variables will be set to an initialized state.

Return Value :
    A long value, indicating CAM_SUCCESS or CAM_FAILURE.

Side Effects :
    The CAM subsystem will be initialized via the call to the configuration
    driver.

Additional Information :
    It is expected that this routine must be called from all the PDrvs
    including any externally supplied drivers.
*/

I32 
xpt_init()
{
    /* If the subsystem is not yet initialized then call the CDrv, else 
    return all's well. */

    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	("[b/t/l] xpt_init: Routine Enter\n") );

    if( cam_subsystem_ready != CAM_TRUE )
    {
	PRINTD( NOBTL, NOBTL, NOBTL, CAMD_FLOW,
	    ("[b/t/l] xpt_init: subsystem not ready, calling CDrv\n") );

	return( ccfg_initialize() );
    }
    else
    {
	return( CAM_SUCCESS );
    }
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : xpt_ccb_alloc

Functional Description :
    Formal interface to the PDrv for requesting a CCB to use.  A CAM
    packet will be removed from the front of the "free" side of the
    pool, marked as BUSY, and placed on the front of the "busy" side
    of the pool.  The free CCB counter will be decremented and the busy
    CCB counter incremented.  The counters will be checked for High/Low
    water marks and more packets will be allocated or deallocated if
    necessary.  The requests will be baised on the high/low/additional
    CCB information passed via the cam_data.c file.  The packet will have
    the SIM, PDrv, and CCB_UNION structures all cleared using bzero.
    All the pointers in the packet will be updated, the working sets to
    the CCB and the CCB to the working sets, even though it is not known
    what the CCB requested will be used for.

Formal Parameters : None

Implicit Inputs :
    XPT_WS xpt_qhead			The XPT CCB pool queue header
    I32 cam_low_water			The CAM CCB pool low water mark
    I32 cam_high_water			The CAM CCB pool high water mark
    I32 cam_ccb_increment		the CAM CCB pool increment value

Implicit Outputs : 
    The XPT CCB pools busy and free counter will be updated.

Return Value :
    A pointer to a "free" CCB with the working set pointers correctly updated.

Side Effects :
    The XPT working set and CCB will be moved from the free side of the pool to
    the busy side.  The free and busy counters will be updated.  If the free
    count reaches the "low water" mark the CCB pool will have more 
    DEC_CAM_PKTs allocated and added to the free side.  Due to KMALLOC()/FREE()
    calling constraints in OSF/1.  The High Water Mark will also be checked.
    If there are too many on the free side they will be returned to the
    system.

Additional Information :
    This routine has places that must be SMP locked. 

    Misc. issues for SMP locking w/in this routine.

	1) Remove a DEC_CAM_PKT/XPT_WS from the free side.
	    This will require locks.
	
	2) Setup the XPT_WS structure.
	    CCB->WS attachments
	    set status to BUSY

	3) Place the DEC_CAM_PKT/XPT_WS onto the busy side.
	    This will require locks.

	4) Return the CCB_UNION address to the caller

  *** NOTE for future engineers:

    There are a number of casts that are used in this routine.  These
    casts are used to allow single pointers to deal with specific
    aspects of the DEC_CAM_PKT with out having to deal with the packet
    definition.  It is necessary that the XPT working set remain at the
    "top" of the packet.  One of the pointer casts uses this
    arrangement.
*/

static char *xpt_ccb_alloc_func = "xpt_ccb_alloc()";	/* Func name */

CCB_HEADER *
xpt_ccb_alloc()
{
    DEC_CAM_PKT *dcp;		/* for the pointer attachments CCB/WSs */
    XPT_WS *xws;		/* for the XPT working set */
    XPT_WS *xqh;		/* for the XPT Q head */
    CCB_SCSIIO *cio;		/* for the CCB union part of the packet */
    CAM_SIM_PRIV *csp;		/* for the private data struct overlay */
    int s;			/* IPL storage for locking */

    extern I32 xpt_pool_alloc();	/* to update the pool with packets */
    extern I32 xpt_pool_free();		/* to remove excess packets */

    /* SMP lock on the queue head, remove the front free packet and update the 
    number of free CCBs.  In the event that there are no more packets to give
    out it was agreed that this is the code point to panic. */

    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	("[b/t/l] xpt_ccb_alloc: Routine entered.\n") );

    xqh = &xpt_qhead.xws;			/* set the Q head pointer */
    XQHEAD_IPLSMP_LOCK( s, &xpt_qhead );	/* lock on the Q struct */

    /* NOTE: OSF special, if this routine is called while in a ISR
    context, If the pool is low (low water mark they are the only ones
    allowed to get a ccb. */

    /* Check to see if the pool is low.  It the free count has reached
    the low water mark the allocator needes to be called.  The Pool
    ceiling is part of this check to make sure that there are not too
    many.  Other wise as things free up those waiting will be called.
    If the allocator has already been scheculed or is actually running
    then don't bother to issue it's wakeup.  The global "DIVE-DIVE" 
    flag will also let us know that there is no need to wakeup the 
    allocator. */

    if((xqh->xpt_nfree <= cam_ccb_low_water) && 
	(xpt_qhead.xpt_ccbs_total < xpt_qhead.xpt_ccb_limit))
    {
	if((( xqh->xpt_flags & (XPT_ALLOC_SCHED | XPT_ALLOC_ACT)) == NULL) &&
	    !shutting_down)
	{
	    /* The pool allocator is not running or scheduled to run */
	    thread_wakeup_one((vm_offset_t)xpt_pool_alloc_thread);
	}
    }

    /* If the pool is already in a "wait" contition, then the sequence
    order of the incomming CCB requests *MUST* be maintained.  The
    I/O's are put onto the wait queue and allowed to funnel thru the
    wait->run queues until the "last" one comes thru.  Once the two
    queues are empty of XPT stalled I/O the flag can be cleared and
    the normal flow of I/O continued. */

    if((( xqh->xpt_flags & XPT_RESOURCE_WAIT ) != NULL) && !cam_inisr())
    {
	/* Must wait till my turn to get resources go to the sleep queue */

	XPT_WAIT( &xpt_qhead );		/* wait++, assert wait, block, wait-- */
    }

    /* This code path will catch the occurence of the Low water 
    contition.  The "wait" flag is set to bounce any more I/Os, not 
    in ISR, onto the wait queue.  The pool allocator and/or the
    xpt_ccb_free() code will be placing more CCBs into the pool
    that are being "waited" for. */

    if((xqh->xpt_nfree <= cam_ccb_low_water) && !cam_inisr())
    {
	xqh->xpt_flags |= XPT_RESOURCE_WAIT;	/* Signal the wait condition */
	xpt_qhead.xpt_times_wait++;		/* how many times here */
	XPT_WAIT( &xpt_qhead );		/* wait++, assert wait, block, wait-- */

    }

    /* If in ISR we did not go to sleep so this check is not made.  If
    this I/O-thread is the last one that has funneled thru the wait-run
    queues then the wait contition can now be removed.  This will allow
    the I/Os to run normally. */

    if( ((xqh->xpt_flags & XPT_RESOURCE_WAIT) != NULL) && 
	(xpt_qhead.xpt_wait_cnt == NULL) && !cam_inisr() )
    {
	xqh->xpt_flags &= ~XPT_RESOURCE_WAIT;		/* go back to normal */
    }

    /* One last check make sure there is something */

    if( xqh->x_flink == (XPT_WS *)&xpt_qhead){

	XQHEAD_IPLSMP_UNLOCK( s, &xpt_qhead );      /* unlock the Q struct */
        return( (CCB_HEADER *)NULL );	
    }

    /* Pull the front CCB out of the pool.  With this removal the pool
    levels need to be checked for the low water contdition. */

    xws = xqh->x_flink;
    XPT_WS_REMOVE(xws);  		/* grab the front packet */

    xqh->xpt_nfree--;			/* decr # of free */

    /* If there are sufficient CCBs in the pool and there are
    I/O-threads still waiting, issue another wakeup for the next I/O on
    the wait queue.  If there are not sufficient CCBs, then call the
    pool allocator thread. The allocator thread will put more CCBs into
    the pool and also wakeup the front thread that is waiting. */

    if(xqh->xpt_nfree >= cam_ccb_low_water) 		/* is there some */
    {
	/* Make sure that there are I/O's waiting. */
	if( xpt_qhead.xpt_wait_cnt != NULL ) 
	{
	    thread_wakeup_one( (vm_offset_t)&xpt_qhead.xpt_wait_cnt );
	}
    }
    else 	/* !(xqh->xpt_nfree >= cam_ccb_low_water) */
    {
	if(( xqh->xpt_flags & (XPT_ALLOC_SCHED | XPT_ALLOC_ACT)) == NULL)
	{
	    /* The pool allocator is not running or scheduled to run */
	    thread_wakeup_one( (vm_offset_t)xpt_pool_alloc_thread );
	}
    }

    XQHEAD_IPLSMP_UNLOCK( s, &xpt_qhead );	/* unlock the Q struct */

    /* Continue the I/O processing.  Now that there are CCB resources
    in the pool this CCB has to be setup and returned to the caller.
    With in the XPT working set update flink/blink and BUSY -> flags. */

    xws->x_blink = NULL;
    xws->xpt_flags = XPT_BUSY;
    xws->xpt_nbusy++;

    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_CCBALLOC,
	("[b/t/l] xpt_ccb_alloc:  pkt addr 0x%x, times used %d\n",
	xws, xws->xpt_nbusy) );
    
    /* Cast the XPT wrk set, to the packet pointer.  Initialize the packet
    for use. */

    dcp = (DEC_CAM_PKT *)xws;		/* the XPT working set is on top */

    /* Clear out the SIM, PDrv, and CCB regions of the packet.  This
    will allow the different modules to work with "initialized" data
    space.  The XPT working set will not be cleared, there are some
    minor statistics kept in there. */

    bzero( &(dcp->pws), sizeof(PDRV_WS) );
    bzero( &(dcp->ccb_un), sizeof(CCB_SIZE_UNION) );

    /* Assign the pointers in the XPT, PDrv, and SIM working sets to the
    CCB structure in the packet. */

    dcp->xws.xpt_ccb = (CCB_HEADER *)&(dcp->ccb_un);
    dcp->pws.pws_ccb = (CCB_SCSIIO *)&(dcp->ccb_un);
    dcp->sws.ccb = (CCB_SCSIIO *)&(dcp->ccb_un);

    /* Assign the ccb_io pointer into the packet and update the CCB -> WS
    pointers. */ 

    cio = &(dcp->ccb_un.csio);
    cio->cam_ch.my_addr = (CCB_HEADER *)cio;	/* the header's own address */

    cio->cam_pdrv_ptr = (u_char *)&(dcp->pws);	/* for the PDrv */

    csp = (CAM_SIM_PRIV *)(cio->cam_sim_priv);	/* for the SIM */
    csp->sim_ws = &(dcp->sws);
    csp->valid = DEC_VALID;

    XQHEAD_IPLSMP_LOCK( s, &xpt_qhead );/* lock on the Q struct */
    xqh->xpt_nbusy++;			/* increment the number of busy pkts */
    XQHEAD_IPLSMP_UNLOCK( s, &xpt_qhead );	/* unlock the Q struct */

    /* Return the pointer */
    return( (CCB_HEADER *)cio );	/* send the free CCB to the caller */
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : xpt_ccb_free

Functional Description :
    Formal interface to the PDrv for freeing up a used CCB.  The
    pointer for the CCB will be used to calculate the pointer to
    the working set.  Then the CCB/WS will be removed from the "busy"
    side of the pool.  Marked as FREE, and placed on the front of
    the "free" side of the pool.  The free CCB counter will be
    incremented and the busy CCB counter decremented. 

Formal Parameters : 
    CCB_HEADER *ch			pointer to the CCB to free up

Implicit Inputs :
    XPT_WS xpt_qhead			The XPT CCB pool queue header
    I32 cam_high_water			The CAM CCB pool high water mark
    I32 cam_ccb_increment		the CAM CCB pool increment value

Implicit Outputs :
    The XPT CCB pools busy and free counter will be updated.

Return Value :
    A I32 value, indicating CAM_SUCCESS or CAM_FAILURE.

Side Effects :
    The XPT working set and CCB will be moved from the busy side of the pool to
    the free side.  The free and busy counters will be updated.  

Additional Information :
    This routine has places that must be SMP locked.  There are a number
    of casts that are used in this routine.  These casts are used to
    allow single pointers to deal with specific aspects of the
    DEC_CAM_PKT with out having to deal with the packet definition.  It
    is necessary that the XPT working set remain at the "top" of the
    packet.  One of the pointer casts uses this arrangement.

    Misc. issues for SMP locking w/in this routine.

	1) Remove a DEC_CAM_PKT/XPT_WS from the busy side.
	    This will require locks.
	
	2) Setup the XPT_WS structure.
	    set status to FREE

	3) Place the DEC_CAM_PKT/XPT_WS onto the free side.
	    This will require locks.
*/

static char *xpt_ccb_free_func = "xpt_ccb_free()";	/* Func name */

I32
xpt_ccb_free( ch )
    CCB_HEADER *ch;		/* pointer to the CCB to free up */
{
    XPT_WS *xws;		/* pointer for the XPT working set */
    XPT_WS *xqh;		/* for the XPT Q head */
    int s;			/* IPL storage for locking */

    /* Have to check to make sure that the CCB is one of ours.  Verification
    is by comparing the "magic number" in the XPT_WS. */

    PRINTD( NOBTL, NOBTL, NOBTL, (CAMD_INOUT|CAMD_CCBALLOC),
	("[b/t/l] xpt_ccb_free: Entered with CCB addr 0x%x\n", ch) );

    xws = (XPT_WS *)(((U_WORD)ch) -
		     (((U_WORD)sizeof(XPT_WS)) + ((U_WORD)sizeof(PDRV_WS)) +
		      ((U_WORD)sizeof(SIM_WS))));

    if (xws->xpt_ccb != ch)
    {
	CAM_ERROR( xpt_ccb_free_func, "Invalid packet to FREE.",
	    (CAM_ERR_LOW), (void *)NULL, xws, (void *)NULL ); 

	return( XPT_CCB_INVALID );
    }

    /* Continue with the normal transition of a BUSY ccb to a FREE ccb.
    SMP lock on the queue head */

    xqh = &xpt_qhead.xws;			/* set the Q head pointer */
    XQHEAD_IPLSMP_LOCK( s, &xpt_qhead );	/* lock on the Q struct */
    xqh->xpt_nbusy--;		/* decrement the number of busy pkts */

    /* With in the XPT working set FREE flag. */

    xws->xpt_flags = XPT_FREE;
    xws->xpt_nfree++;

    /* Attach the now updated CCB to the front of the free side.  Update
    the number of free CCBs. */

    XPT_WS_INSERT(xws, xqh->x_blink);		/* attach to back of freelist */

    xqh->xpt_nfree++;		/* incr # of free */

    /* check to see if any one is in a resource wait and not below 
    low water mark if so them wake them up. */
    if(((xpt_qhead.xws.xpt_flags & XPT_RESOURCE_WAIT) != NULL) &&
		(xqh->xpt_nfree > cam_ccb_low_water)){
        thread_wakeup_one((vm_offset_t)&xpt_qhead.xpt_wait_cnt);
    }
    XQHEAD_IPLSMP_UNLOCK( s, &xpt_qhead );	/* unlock the Q struct */

    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_CCBALLOC,
	("[b/t/l] xpt_ccb_free:  pkt addr 0x%x, times freed %d\n",
	xws, xws->xpt_nfree) );
    
    /* Return to the caller. */
    return( CAM_SUCCESS );		/* all went well */
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : xpt_action

Functional Description :
    One of the formal entry points for the XPT from the PDrv level.  The
    argument is a CCB pointer.  The opcode in the header is checked for
    being w/in the XPT function lookup table bounds and then using the
    opcode the table is indexed and the lucky function called.  The
    return value from the jump table routine is passed back to the 
    caller from the PDrv level.

Formal Parameters :
    CCB_HEADER *ch;	Pointer to the CCB to act upon

Implicit Inputs :
    CCB_CMD_ENTRY ccb_tbl[]	The XPT command/function jump table.

Implicit Outputs : 
    The CAM status field in the CAM header will also contain the return value.

Return Value :
    A long value, containing a valid CAM status.

Side Effects :
    There can be many.  If the CCB is going to a SIM the CCB could be placed
    on internal queues or immediately acted up.  If the CCB is going to the
    CDrv the internal device tables may be accessed or updated.

Additional Information :
    This routine does a "blind" jump through the command lookup table.  The
    function code is checked aginst the upper bound to make sure that 
    a valid routine is called.  If the function code is in the Vendor Unique
    region, (0x80 - 0xFF), the selected SIM is called.

    Q: What other checking on the CCB could be done at this level ?
*/

I32
xpt_action( ch )
    CCB_HEADER *ch;	/* pointer to the CCB to act upon */
{
    extern CCB_CMD_ENTRY ccb_tbl[];
    extern U32 N_ccb_tbl;

    /* Verify that the function opcode in the CCB is within the bounds of the
    jump table.  If it is out of bounds call the Vendor Unique entry. */

    PRINTD( NOBTL, NOBTL, NOBTL, (CAMD_INOUT|CAMD_FLOW),
	("[b/t/l] xpt_action: CCB addr 0x%x, opcode 0x%x (%s)\n",
	ch, ch->cam_func_code,
	(char *)cdbg_CamFunction(ch->cam_func_code, CDBG_BRIEF) ));
    
    if( ch->cam_func_code >= N_ccb_tbl )
    {
	/* When the function code is within the VU region go through the
	jump table at the special XPT_VUNIQUE entry. */

	return( (*ccb_tbl[ XPT_VUNIQUE ])( ch ) );
    }

    /* Go through the jump table and return what the jump routine returns. */

    return( (*ccb_tbl[ ch->cam_func_code ])( ch ) );

}

/* ---------------------------------------------------------------------- */
/*
Routine Name : xpt_initialize

Functional Description :
    This routine is completly different from xpt_init().  This
    routine is responsible for doing all the work necessary to
    initialize the XPT layer software.  This routine will be called
    by the CDrv when it is necessary to get the XPT up and
    running.

Formal Parameters : None

Implicit Inputs : 
    Global CAM setup variables, those setup for the system via the
    cam_data.c file.

Implicit Outputs :
    All global and internal XPT variables will be set to an initialized state.

Return Value :
    A long value, indicating CAM_SUCCESS or CAM_FAILURE.

Side Effects :
    The XPT CCB pool will be filled with "free" KMALLOCed packets.  The 
    initial number of packets will be taken from the global value supplied
    via the cam_data.c file.

Additional Information :
    It will probably be accessed via the XPT "SIM entry" structure, or it
    could be a fixed subroutine name as part of the CAM subsystem.
*/

static char *xpt_initialize_func = "xpt_initialize()";	/* Func name */

I32
xpt_initialize()
{
    XPT_WS *xqh;		/* pointer for the queue head structure */

    extern I32 xpt_pool_alloc();

    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	("[b/t/l] xpt_initialize: ENTER\n") );

    /* Initialize all local/internal XPT variables. */

    xqh = &xpt_qhead.xws;	/* assign the pointer */ 

    xqh->xpt_flags = XPT_PHEAD;	/* signal the pool HEAD structure */
    xqh->x_flink = (XPT_WS *)&xqh->x_flink;/* init the "free" side pointer */
    xqh->x_blink = (XPT_WS *)&xqh->x_flink;/* init the "busy" side pointer */
    xqh->xpt_ccb = NULL;	/* no CCB's, also signal the HEAD struct */
    xqh->xpt_nfree = 0;		/* no free CCBs yet */
    xqh->xpt_nbusy = 0;		/* no busy CCBs yet */
    xpt_qhead.xpt_wait_cnt = 0;	/* No one waiting */
    xpt_qhead.xpt_times_wait = 0;	/* No one waiting */
    xpt_qhead.xpt_ccb_limit = 0x100000; /* Give it max until limit hit */	
    xpt_qhead.xpt_ccbs_total = 0;	/* No ccbs yet... */

    XQHEAD_INIT_LOCK( &xpt_qhead );	/* Init the Q head SMP lock */

    XCTRL_INIT_LOCK( &cam_conf_ctrl );	/* Init the control SMP lock */

    /*
     * If call back queue not init'ed do it
     */
    if( xpt_cb_queue.initialized == NULL ){
    	/*
    	 * Init the queue
    	 */
    	XPT_CB_LOCK_INIT(&xpt_cb_queue);
    	xpt_cb_queue.flink = (XPT_WS *)&xpt_cb_queue;
    	xpt_cb_queue.blink = (XPT_WS *)&xpt_cb_queue;
    	xpt_cb_queue.count = NULL;
    	xpt_cb_queue.flags = NULL;
	xpt_cb_queue.initialized = 1;
    }

    /* Call the header alloc routine with the initial size to setup the CCB
    free pool queue */

    PRINTD( NOBTL, NOBTL, NOBTL, (CAMD_FLOW|CAMD_CCBALLOC),
	("[b/t/l] xpt_initialize: Initial alloc of CCB packet pool.\n") );

    if( xpt_pool_alloc( cam_ccb_pool_size ) <= 0 ) /* fill to initial size */
    {
	CAM_ERROR( xpt_initialize_func, "Unable to Fill Packet Pool.",
	    (CAM_ERR_LOW), xqh, (void *)NULL, (void *)NULL ); 

	return( CAM_FAILURE );	/* there must be something in the pool */
    }

    timeout( xpt_pool_free, ( caddr_t )cam_ccb_increment, 30 * hz );

    return( CAM_SUCCESS );
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : xpt_bus_register

Functional Description :
    This routine is used by the SIM to inform the XPT that it has just
    been loaded by the OS.  This must be the first XPT call by the
    SIM.  The XPT will scan the cam_conftbl[] to find an empty slot.
    When a slot is available the XPT place holder SIM entry is put there
    and the ccfg_simattach() routine is called to complete the init/scanning
    work.

Formal Parameters :
    There is only one argument to this routine.  The argument is the
    pointer for the data structure defining the entry points for the SIM.  

Implicit Inputs : None

Implicit Outputs :
    The configuration table is updated.

Return Value :
    A long value, indicating CAM_SUCCESS or CAM_FAILURE.

Side Effects :
    The SIM is initialized and the EDT table is updated from the bus
    scan done by the CDrv.  If any PDrvs have registered for SIM
    loading they will be called via the Asysnc callback mechanism.  The
    SIM modules must be prepared to have the sim_init() routine called
    prior to the return of the call to xpt_sim_load().

Additional Information :
    Locking around the cam_conftbl[] needs to be done in:
    xpt_action->xpt_sim_func
    xpt_bus_deregister
    ccfg_simattach
    JAG: LOCKING ?
*/

I32
xpt_bus_register( cse )
    CAM_SIM_ENTRY *cse;			/* the entry struct for reference */
{
    I32 new_id;			/* path ID for the new SIM */
    int s;				/* IPL storage for locking */

    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	("[b/t/l] xpt_bus_register: Routine entered\n") );

    /* Scan the cam_conftbl[] to find the first NULL entry.  The NULL
    entry is updated with the XPT "dummy" CAM_SIM_ENTRY pointer.  If
    the cam_conftbl[] is filled, log the error and return failure to
    the SIM. */

    XCTRL_IPLSMP_LOCK( s, &cam_conf_ctrl );	/* Lock the confbtl */

    for( new_id = 0; new_id < N_cam_conftbl; new_id++ )/* scan cam_conftbl[] */
    {
	if( cam_conftbl[ new_id ] == NULL )	/* check the entry */
	{
	    cam_conftbl[ new_id ] = &xpt_sim_entry;	/* reserve the entry */

	    XCTRL_IPLSMP_UNLOCK( s, &cam_conf_ctrl );	/* unlock after write */

	    /* Contact the CDrv to initialize the newly loaded SIM, the
	    SIM path ID is passed to the CDrv for the SIM_init() call. 
	    The CDrv will be responsible for updating the device table. */

	    if( ccfg_simattach( cse, new_id ) != CAM_FAILURE )
	    {
		/* Now via the Async callbacks inform the PDrvs that a new
		SIM has been registered. */

		(void)xpt_async( AC_SIM_REGISTER, new_id, ASYNC_WILDCARD,
		    ASYNC_WILDCARD, (char *)NULL, 0 );
		
		return( CAM_SUCCESS );
	    }
	    else
	    {
		/* The SIM attach failed.  Remove the xpt_sim_entry from the
		cam_conftbl[] and let the failure terminate the loop.  There
		is an UNLOCK call at the end of the loop to unlock this 
		lock. */

		XCTRL_IPLSMP_LOCK( s, &cam_conf_ctrl );	/* Lock the confbtl */

		cam_conftbl[ new_id ] = NULL;		/* release the entry */
		break;			/* halt the loop */
	    }
	}
    }

    /* Control has fallen out of the loop, therefor no empty slots were found,
    or the attach failed.  Inform the SIM module of the failure. */

    XCTRL_IPLSMP_UNLOCK( s, &cam_conf_ctrl );
    return( CAM_FAILURE );
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : xpt_bus_deregister

Functional Description :
    This routine is used by the SIM to inform the XPT that it will be
    unloaded by the OS.  The SIM must be ready to "fail" any sim_action()
    calls until this routine returns.

Formal Parameters :
    There is only one argument to this routine.  The argument is the
    current path ID for the SIM.

Implicit Inputs : None

Implicit Outputs :
    The configuration table is updated.

Return Value :
    A long value, indicating CAM_SUCCESS or CAM_FAILURE.

Side Effects :
    All the PDrvs that have registered for AC_SIM_DEREGISTER will be called.
    However the SIM entry pointes will have already been removed prior
    to the calls.

Additional Information :
    Locking around the cam_conftbl[] needs to be done in:
    xpt_action->xpt_sim_func
    xpt_bus_register
    ccfg_simattach
    JAG: LOCKING ?
*/

I32
xpt_bus_deregister( path_id )
    I32 path_id;			/* path ID for the SIM */
{
    int s;				/* IPL storage for locking */

    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	("[b/t/l] xpt_bus_deregister: Routine entered\n") );

    /* Make sure that the path_id is ok to use. */

    if( path_id >= N_cam_conftbl )		/* is it with range */
    {
	return( CAM_FAILURE );			/* signal nothing done */
    }

    /* NULL out the path_id that the SIM is deregistering from. */

    XCTRL_IPLSMP_LOCK( s, &cam_conf_ctrl );	/* lock the cam_conftbl[] */

    cam_conftbl[ path_id ] = NULL;	/* remove the entry */

    XCTRL_IPLSMP_UNLOCK( s, &cam_conf_ctrl );	/* unlock the cam_conftbl[] */

    /* Now via the Async callbacks inform the PDrvs that a 
    SIM has been deregistered. */

    (void)xpt_async( AC_SIM_DEREGISTER, path_id, ASYNC_WILDCARD,
	ASYNC_WILDCARD, (char *)NULL, 0 );

    /* Inform the SIM module of the success of the deregister. */

    return( CAM_SUCCESS );
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : xpt_async

Functional Description :
    The SIM calls this routine to inform the XPT that an async event
    has occured and that there may be peripheral device drivers that
    need to be informed.  The EDT structure can support multiple XPT
    "readers", and the CDrv module, the "writer" will wait for the readers
    to finish.  There is a counter within the EDT strucutre that the
    readers use to communicate between themselves.  Each reader that
    starts will increment the counter and when finished it will decrement
    the counter.  If the counter decrements to 0, then the writer is 
    issued a wakeup() call incase it may have been waiting.

Formal Parameters :
    Four of the six arguments are long, 32 bits, values: opcode,
    path_id, target_id, lun, and data_cnt.  The path_id, target_id, and
    lun define a nexus for the async callback.  The opcode contains the
    value for what has happened.  The buffer_ptr and data_cnt are used
    to inform the XPT where and how much data is associated with the
    opcode.

Implicit Inputs :
    The internal structure containing the async callback registration info.

Implicit Outputs : None

Return Value :
    A long value, indicating CAM_SUCCESS or CAM_FAILURE.

Side Effects :
    Zero or many PDrvs may be called from the XPT.  Any PDrvs that have
    registered for the passed opcode will be called via the callback
    mechanism.  The PDrvs may at this time do any type of CAM actions.

Additional Information :
    When control has returned from this call all the PDrvs that were
    registered have been contacted.  It is planned that other PDrvs will
    be called independent of SUCCESS/FAILURE of any previous calls.  There
    will be no attempt to catch multiple calls to a PDrv.  The target and 
    lun can contain a -1 value to indicate a "wildcard", any number of
    matches can be used with them.

    EDT NOTE:  The check uses N_cam_conftbl, the size of the
    cam_conftbl[] lookup structure when checking path_id for indexing
    into the EDT directory.  It is EXPECTED/ASSUMED that the edt_dir[]
    and cam_conftbl[] have the same number of elements.

    SMP NOTE:  The locking will work with XPT/CCFG or XPT/XPT interactions.
    There could be a bug with other SIMs calling to the same PDrv and its
    buffers.  It looks like the PDrvs will have to keep multiple buffers (?).
*/

I32
xpt_async( opcode, path_id, target_id, lun, buffer_ptr, data_cnt )
    I32 opcode;			/* what has happened */
    I32 path_id;			/* bus number for the async event */
    I32 target_id;			/* the ID of the target */
    I32 lun;				/* the LUN under the target */
    char *buffer_ptr;			/* local SIM buffer containing data */
    I32 data_cnt;			/* # of valid bytes in the buffer */
{
    EDT *eg;				/* for accessing the EDT struct */
    CAM_EDT_ENTRY *ee;			/* for accessing EDT elements */
    ASYNC_INFO *p;			/* pointer for info list */
    U32 t_scan_count;		/* local copy of the counter */
    I32 t, l;				/* target/lun loop counters */
    int s;				/* for IPL/Locking */

    PRINTD( path_id, NOBTL, NOBTL, CAMD_INOUT,
	("[%d/t/l] xpt_async: called op 0x%x, t: %d l: %d\n", path_id,
	opcode, target_id, lun) );

    /* Make sure that the passed path_id is valid to index the EDT. 
    NOTE:  The check uses N_cam_conftbl, the size of the lookup table.
    These two arrays are exptected to be the same size. */

    if( path_id >= N_cam_conftbl )		/* is there room ? */
    {
	return( CAM_FAILURE );		/* Signal nothing done */
    }

    /* Index into the edt_dir[] to get the EDT pointer.  If the pointer
    is not valid, == NULL, then return with failure. */

    if( (eg = edt_dir[ path_id ]) == NULL )
    {
	return( CAM_FAILURE );		/* there is no EDT for the ID */
    }

    /* Lock the EDT, set the ASYNC_CB_INPROG flag and increment the readers
    counter. */

    EDT_IPLSMP_LOCK( s, eg );			/* lock on the EDT */

    eg->edt_flags |= ASYNC_CB_INPROG;		/* Signal a read */
    eg->edt_scan_count++;			/* update the counter */

    EDT_IPLSMP_UNLOCK( s, eg );			/* remove the lock */

    /* Scan along the Async register arrays to find matches with the opcode.
    The two for loops will scan along the target/LUNs.  If the target and lun
    match or are wild cards the opcode will be checked on the ASYNC_INFO
    list.  If the opcodes match then the PDrv will be called and the
    data will be moved if necessary. */

    for( t = 0; t < NDPS; t++ )				/* Scan targets */
    {
	if( (t == target_id) || (target_id == ASYNC_WILDCARD) )	/* match ? */
	{
	    for( l = 0; l < NLPT; l++ )			/* Scan LUNs */
	    {
		if( (l == lun) || (lun == ASYNC_WILDCARD) )	/* match ? */
		{
		    /* Walk along the linked list of ASYNC_INFO structures.
		    The first check is just to see if the PDrv wants a 
		    async callback on the passed opcode.  The switch
		    statement will handle the particular opcodes. */

		    p = eg->edt[t][l].cam_ainfo;	/* head of the list */

		    while( p != NULL )
		    {
			if( (opcode & p->cam_event_enable) != 0 ) /* match */
			{
			    /* Using the passed opcode call the PDrv's 
			    callback handler with the proper arguments. */

			    switch( opcode )
			    {
				/* Send the passed parameters to the callback
				handler. */

				case AC_SENT_BDR :
				case AC_FOUND_DEVICES :
				case AC_UNSOL_RESEL :
				case AC_BUS_RESET :
				    (*p->cam_async_func)( opcode, path_id,
					t, l, (char *)NULL, 0 );
				break;

				/* Move the path_id of the SIM into the PDrv's
				buffer.  Then call the handler with the 
				buffer address and count of bytes. */

				case AC_SIM_DEREGISTER :
				case AC_SIM_REGISTER :

				    *(p->cam_async_ptr) = (u_char)path_id;

				    (*p->cam_async_func)( opcode,
					XPT_PATH_INQ_ID, t, l,
					p->cam_async_ptr, 1 );
				break;

				/* Copy the AEN bytes from the SIM buffer to
				the PDrv buffer.  The minimum number will
				be transfered. */

				case AC_SCSI_AEN :
				    bcopy( buffer_ptr, p->cam_async_ptr,
					MIN( data_cnt, p->cam_async_blen ));

				    (*p->cam_async_func)( opcode, path_id,
					t, l, p->cam_async_ptr, 
					MIN( data_cnt, p->cam_async_blen ));
				break;

				/* Right now just send the same passed 
				parameters to the handler. */

				default:
				    (*p->cam_async_func)( opcode, path_id,
					t, l, (char *)NULL, 0 );
				break;
			    }
			}

			/* Move on to the next struct on the list, to check
			on it's event enables. */

			p = p->cam_async_next;		/* get next */
		    }
		}
	    }
	}
    }

    /* Now that the scanning has completed.  Unset the ASYNC_CB_INPROG flag and
    decrement the reader counter.  If the counter is 0, clear the flag the
    CDrv is sleeping waiting on it. */

    EDT_IPLSMP_LOCK( s, eg );			/* lock on the EDT */

    t_scan_count = --eg->edt_scan_count;	/* predecremeent and save */

    if( t_scan_count == 0 )
    {
	eg->edt_flags &= ~ASYNC_CB_INPROG;	/* clear the flag */
    }
    EDT_IPLSMP_UNLOCK( s, eg );			/* remove the lock */

    /* Should a wakeup() call occur ?  Only the last reader of the EDT should
    issue the wakeup to the ccfg_setasync() routine to allow it to modify
    the EDT. */

    if( t_scan_count == 0 )
    {
	wakeup( eg );				/* wakeup the CDrv */
    }
    return( CAM_SUCCESS );			/* all done */
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* Internaly defined functions: */
    
/*
Routine Name : xpt_pinq_func()

Functional Description :
This routine is responsible for implementing the Path Inquiry CCB.  There
are two directions associated with each CCB.   The Path Inquiry can be
for the XPT or for the selected SIM. The PDrv can ask for two different
pieces of information.  If the path_id is 0xFF, then the CCB is for the
XPT else it is for the selected SIM.  This routine will return the current
maximun SIM ID assigned, if it is the selected path id.  If the CCB is
destined to a SIM, this routine will set the bits that the XPT and CDrv
"know" about and then send the CCB to the SIM.

Formal Parameters : 
A pointer to the PATH INQUIRY CCB.

Implicit Inputs :
The CAM cam_conftbl[] data strcutrre.

Implicit Outputs :
None.

Return Value :
A valid CAM status, also contained in the CAM status field.

Side Effects :

Additional Information :
The Features supported fields are completely cleared prior to setting
any bits in them.  These fields have to be cleared due to their being
in the same offset as the pdrv_ptr field in the SCSI IO CCB.  The XPT
alloc routines will preload the pdrv_ptr field with an address.
Because these fields would contain "bizzare" bit settings due to the
"address" they have to be cleared. 
*/

static U32
xpt_pinq_func( cpi )
    CCB_PATHINQ *cpi;			/* pointer to the path inq CCB */
{
    extern U32 xpt_sim_func();	/* local SIM caller */

    I32 i;				/* loop var for the conftbl[] scan */

    PRINTD( cpi->cam_ch.cam_path_id, NOBTL, NOBTL, CAMD_INOUT,
	("[%d/t/l] (xpt_pinq_func) INQ started.\n", cpi->cam_ch.cam_path_id ));

    /* The CAM Feature fields are always cleared, just in case. */

    cpi->cam_version_num = 0x00;	/* clear out the address bits */
    cpi->cam_hba_inquiry = 0x00;	/* clear out the address bits */
    cpi->cam_target_sprt = 0x00;	/* clear out the address bits */
    cpi->cam_hba_misc    = 0x00;	/* clear out the address bits */

    /* The path id in the CCB header is checked to see who this CCB is
    destined for.  If the ID is XPT_PATH_INQ_ID, the CCB is for the XPT
    and the "Highest Path ID Assigned" is returned.  Else the CCB is
    destined for the selected SIM. */

    if( cpi->cam_ch.cam_path_id == XPT_PATH_INQ_ID )
    {
	/* Backwards scan the cam_conftbl[] until the highest non NULL
	SIM entry is found.  */

	for( i = (N_cam_conftbl - 1); i >= 0; i-- )	/* start at top */
	{
	    if( cam_conftbl[ i ] != (CAM_SIM_ENTRY *)NULL )
	    {
		/* Found the highest assigned SIM ID, copy the ID to
		the CCB and return it to the user.  This is the only
		valid field on return. */

		cpi->cam_hpath_id = i;

		cpi->cam_ch.cam_status = CAM_REQ_CMP;
		return( CAM_REQ_CMP );
	    }
	}

	/* The for loop has fallen out, there for there were no valid
	SIM entries found.  Return the failure to the PDrv. */

	cpi->cam_ch.cam_status = CAM_PATH_INVALID;
	return( CAM_PATH_INVALID );
    }
    else				/* it's for a SIM */
    {
	/* The PDrv wants to find out more information about the SIM
	it selected in the path_id field.  There are a number of bits
	that the XPT and CDrv know about.  These bits will be set by
	the XPT and then the SIM will be called to fill in the rest of
	the CCB.  Any of the fields that the XPT has loaded can be 
	over written by the calling SIM. */

	cpi->cam_version_num = CAM_VERSION;
	cpi->cam_hba_misc = 0;			/* scan 0 -> 7, incl removable,
						inquiry data stored */
	cpi->cam_async_flags |=
	    (AC_FOUND_DEVICES | AC_SIM_DEREGISTER | AC_SIM_REGISTER);

	cpi->cam_sim_priv = SIM_PRIV;		/* part of the CCB pool */

	cpi->cam_hpath_id = 0;			/* not valid */

	/* Call the SIM via the local xpt->sim calling routine. */

	return( xpt_sim_func( (CCB_HEADER *)cpi ) );
    }
}

/* ---------------------------------------------------------------------- */

/*
Routine Name : xpt_ccfg_func

Functional Description :
    One of the functions accessed via the function lookup table.  It
    will call the CDrv_action() routine with the CCB pointer.

Formal Parameters : 
    The pointer for the CCB Header, passed to the xpt_action() routine.

Implicit Inputs : None

Implicit Outputs : None

Return Value :
    It will return the value returned by the CDrv_action call.

Side Effects :
    Depending on the particular function opcode, the CDrv will probably
    have some changes for the internal data structures.

Additional Information :
    This routine may not be needed if a clean way can be worked out to have
    the CDrv directly called via the opcode jump table.
*/

static U32
xpt_ccfg_func( ch )
    CCB_HEADER *ch;		/* header for the CCB to use */
{
    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	("[b/t/l] xpt_ccfg_func: CCB addr 0x%x, calling ccfg_action()\n", ch) );

    /* Call the CDrv_action() routine with the CCB pointer. */

    return( ccfg_action( ch ) );
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : xpt_sim_func

Functional Description :
    This routine deals with CCBs to be sent to the SIM.
    It will call the sim_action() routine accessed via the
    cam_conftbl[] using the path ID field in the CCB.  The value
    returned from the sim_action() call will also be returned to the
    caller.  For error checking, if the sim_action() routine pointer,
    or the struct pointer is NULL, or the path ID is out of bounds
    for the cam_conftbl[] array, the CAM status "Invalid Path ID"
    will be placed in the CAM status field and used for the return
    value.

Formal Parameters :
    The pointer to the CCB header.

Implicit Inputs :
    The configuration lookup table contianing SIM entry points, cam_conftbl[].

Implicit Outputs :
    The CAM status field in the CCB header will be updated.

Return Value :
    A long value, containing a valid CAM status code.

Side Effects :
    The CCB is passed to the indexed SIM.  The CCB may be placed on an internal
    SIM queue or immediatly acted upon.  The CAM status field in the CCB
    header will be updated.

Additional Information :
*/

static U32
xpt_sim_func( ch )
    CCB_HEADER *ch;			/* pointer for the CCB header */
{
    CAM_SIM_ENTRY *cse;			/* pointer to the entry */
    int s;				/* IPL storage for locking */

    PRINTD( ch->cam_path_id, NOBTL, NOBTL, CAMD_INOUT,
	("[%d/t/l] xpt_sim_func: CCB addr 0x%x\n", ch->cam_path_id, ch) );

    /* Assign the pointer to the indexed cam_conftbl[] entry using the 
    CAM path ID field in the CCB header */

    if( ch->cam_path_id >= N_cam_conftbl )
    {
	/* The path ID indexes past the end of the table. */

	PRINTD( ch->cam_path_id, NOBTL, NOBTL, CAMD_ERRORS,
	    ("[%d/t/l] xpt_sim_func: id exceeds the cam_conftbl[]\n",
	    ch->cam_path_id) );

	ch->cam_status = CAM_PATH_INVALID;
	return( CAM_PATH_INVALID );
    }

    /* Lock on the cam_conftbl[] and copy the SIM entry into the local
    pointer. */

    XCTRL_IPLSMP_LOCK( s, &cam_conf_ctrl );	/* lock for the read */

    cse = cam_conftbl[ ch->cam_path_id ];	/* local copy */

    XCTRL_IPLSMP_UNLOCK( s, &cam_conf_ctrl );	/* read is done, unlock */

    if( cse == NULL )
    {
	/* There is no valid SIM entry for the path ID. */

	PRINTD( NOBTL, NOBTL, NOBTL, CAMD_ERRORS,
	    ("[%d/t/l] xpt_sim_func: No valid SIM entry at id.\n",
	    ch->cam_path_id) );

	ch->cam_status = CAM_PATH_INVALID;
	return( CAM_PATH_INVALID );
    }

    if( cse->sim_action == NULL )
    {
	/* For the indexed entry there is no valid action routine. */

	PRINTD( NOBTL, NOBTL, NOBTL, CAMD_ERRORS,
	    ("[%d/t/l] xpt_sim_func: No valid sim_action at id\n",
	    ch->cam_path_id) );

	ch->cam_status = CAM_PATH_INVALID;
	return( CAM_PATH_INVALID );
    }

    /* Call the SIM with the passed CCB and return it's status. */

    return( (*cse->sim_action)( ch ) );
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : xpt_reserved

Functional Description :
    Support function for the XPT function opcode lookup table.  If
    called will place the CAM error code : 0x06 - Invalid req.
    into the status field in the CCB.  The same status value will be
    returned via the jump table/xpt_action to the PDrv.

Formal Parameters :
    The pointer to the CCB header.

Implicit Inputs : None

Implicit Outputs : None

Return Value :
    A long value, set to the CAM status code: 0x06 - Invalid req.

Side Effects :
    The CAM status field in the CCB header will also be updated with the 
    same error code.

Additional Information :
*/

static char *xpt_reserved_func = "xpt_reserved()";	/* Func name */
static U32
xpt_reserved( ch )
    CCB_HEADER *ch;			/* pointer for the CCB header */
{
    /* Update the status field and return the error code. */

    PRINTD( NOBTL, NOBTL, NOBTL, (CAMD_INOUT|CAMD_ERRORS),
	("[b/t/l] xpt_reserved: opcode %d in CCB 0x%x\n",
	ch->cam_func_code, ch) );

    CAM_ERROR( xpt_reserved_func, "Invalid Request, Reserved XPT Opcode.",
	(CAM_ERR_LOW), (void *)NULL, (void *)NULL, ch ); 

    ch->cam_status = CAM_REQ_INVALID;
    return( CAM_REQ_INVALID );
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : xpt_pool_alloc

Functional Description :
    New DEC_CAM_PKTs are placed at the head of the free side of the CCB
    pool.  The queue head structure is the only part needed.  The
    passed argument is the number of DEC_CAM_PKTs to add to the pool.
    This routine will work independent of the pool being full or
    empty.  The CCB free counter will be incremented by the number
    of DEC_CAM_PKTs added to the pool. 

Formal Parameters :
    The number of packets to add to the CCB pool.

Implicit Inputs :
    XPT_WS xpt_qhead		The XPT CCB pool queue header

Implicit Outputs :
    The XPT free CCB counter will be updated.

Return Value :
    A long value, containing the number of packets added to the pool.

Side Effects :
    Each of the packets will have the wrk set to CCB pointers already attached.
    These pointers should never have to be reassigned.  The PDrvs and SIMs
    do not have to mung with these pointers.

Additional Information :
    This routine will call the system KMALLOC code or it's equilivent.  There
    will be one call for each packet.  This will allow the reverse,
    KMFREE, code to be used to allow the xpt_pool_free() routine to
    just pick packets from the free queue and release them.  A string of 
    the new packets to add will be built and then the entire string will
    be added to the pool.  This will limit the locking needed to be done 
    on the pool.  A flag is set in the Q header structure to signal that 
    a pool alloc is in progress.  If this routine is called with that bit
    set it will simply return.
*/

static char *xpt_pool_alloc_func = "xpt_pool_alloc()";	/* Func name */

static I32
xpt_pool_alloc( pcnt )
    U32 pcnt;			/* number of DEC_CAM_PKTs to add */
{
    extern I32 cam_inisr();		/* for determining interrupt context */

    U32 i;				/* loop counter */
    DEC_CAM_PKT *dcp;			/* for the packet */
    XPT_WS *xws;			/* XPT working set part of the packet */
    XPT_WS *xqh;			/* for the XPT Q head */
    int s;				/* IPL storage for locking */

    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	("[b/t/l] xpt_pool_alloc: Routine entered, packets to add %d\n", pcnt));

    /* Add the requested number of packets to the pool.
    NOTE: OSF special, if this routine is called while in a ISR context,
    just return with a warning, will have to wait for a normal context
    to make the KMALLOC calls. */

    if( cam_inisr() )
    {
	PRINTD( NOBTL, NOBTL, NOBTL, CAMD_FLOW,
	("[b/t/l] xpt_pool_alloc: Interrupt context!\n") );

	return( XPT_ISR_CONTEXT );
    }

    /* Do a quick check to see if a pool update is already in progress.  The
    flag is checked, and if TRUE no more work is done.  If the flag is not
    TRUE the process is continued. */

    xqh = &xpt_qhead.xws;			/* set the Q head pointer */
    XQHEAD_IPLSMP_LOCK( s, &xpt_qhead );	/* lock on the Q struct */
    if( (xqh->xpt_flags & XPT_UPDIP) != 0 )	/* is there an update inprog */
    {
	PRINTD( NOBTL, NOBTL, NOBTL, CAMD_FLOW,
	    ("[b/t/l] xpt_pool_alloc: CCB pool update inprogress\n") );

        XQHEAD_IPLSMP_UNLOCK( s, &xpt_qhead );	/* unlock the Q struct */
	return( CAM_SUCCESS );		/* everything will get done */
    }
    /* Set The pool update and allocate flags */

    xqh->xpt_flags |= (XPT_UPDIP | XPT_ALLOC_CALL); 
    XQHEAD_IPLSMP_UNLOCK( s, &xpt_qhead );	/* unlock the Q struct */

    /* The packets will be added to the back of the free side one at a time.
    NOTE: SMP locking, locks will be placed around the qhead access. */

    for( i = pcnt; i > 0; --i )
    {
	/* Make sure that a valid pointer is returned.  Chances are that it
	will be a moot point, the kernel will probably crash real soon
	anyway. */
	/* JAG TO DO: validate the parameters */

	dcp = (DEC_CAM_PKT *)cam_zalloc(sizeof( DEC_CAM_PKT));
	if( dcp  == NULL )
	{
	    /* We are max memory resources we can have....
	    Set the number so we can now have a reference
	    to it.. */

	    xpt_qhead.xpt_ccb_limit = xpt_qhead.xpt_ccbs_total;

	    break;			/* no more kernel resources */
	}

	/* Update the working set and add it to the free list. */
	xws = &(dcp->xws);

	xws->xpt_flags = XPT_FREE;	/* a free CCB */
	xws->x_blink = NULL;		/* no back pointer for now */
	xws->xpt_nfree = 1;		/* first time free */
	xws->xpt_nbusy = 0;		/* hasn't been busy yet */

	XQHEAD_IPLSMP_LOCK( s, &xpt_qhead );            /* lock on Q struct */

	XPT_WS_INSERT( xws, xpt_qhead.xws.x_blink);/* Insert on qhead */

	xpt_qhead.xws.xpt_nfree++;
	xpt_qhead.xpt_ccbs_total++;

	XQHEAD_IPLSMP_UNLOCK( s, &xpt_qhead );  /* unlock the Q struct */
    }

    XQHEAD_IPLSMP_LOCK( s, &xpt_qhead );            /* lock on Q struct */
    xqh->xpt_flags &= ~(XPT_UPDIP); /* release the update flag */
    XQHEAD_IPLSMP_UNLOCK( s, &xpt_qhead );  /* unlock the Q struct */

    /* Signal all done. */
    return( pcnt - i );			/* how many were really added */
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : xpt_pool_free

Functional Description :
    Excess free DEC_CAM_PKTs are removed from the free side
    of the CCB pool.  The passed argument will be the number to
    free back to the Kmalloc space.  The routine will free up the
    requested number or in the event of a oops only as many free
    DEC_CAM_PKTs that exist.  The free CCB counter will be decremented.

Formal Parameters :
    The number of packets to return to the system.

Implicit Inputs :
    The XPT pool queue header strucuture.

Implicit Outputs :
    The XPT free counter will be updated.

Return Value :
    A long value, contianing the actual number of packets freed.

Side Effects :
    A call to the systems KMFREE code or it's equilivent will be made for
    each of the packets to be freed.

Additional Information :
    This routine will only remove packets down to the low water mark.
    Once the packets are to be freed the system KMALLOC/FREE code or
    it's equilivent is called.  There will be one call for each
    packet.  A seperate string of the packets to remove will be taken
    from the free side.  Then the entire string will be walked and each
    packet passed back to the system using the KMFREE call.  This will
    limit the locking needed to be done on the pool.  A flag is set in
    the Q header structure to signal that a pool alloc is in progress.
    If this routine is called with that bit set it will simply return.

*/

static char *xpt_pool_free_func = "xpt_pool_free()";	/* Func name */

static I32
xpt_pool_free( pcnt )
    U32 pcnt;			/* number of DEC_CAM_PKTs to free */
{
    extern I32 cam_inisr();		/* for determining interrupt context */

    U32 i;
    U32 free_cnt;			/* for checking the free count */
    XPT_WS *xws;			/* XPT working set part of the packet */
    XPT_WS *xqh;			/* for the XPT Q head */
    XPT_WS *t_xws;			/* temp XPT_WS pointer assignment */
    int s;				/* IPL storage for locking */

    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	("[b/t/l] xpt_pool_free: Routine entered, packets to remove %d\n",
	pcnt) );

    /* Remove the requested number of packets from the pool.
    NOTE: OSF special, if this routine is called while in an ISR context,
    just return with a warning, the subsystem will have to wait for a normal
    context to make the KMFREE calls. */

    if( cam_inisr() )
    {
	PRINTD( NOBTL, NOBTL, NOBTL, CAMD_FLOW,
	("[b/t/l] xpt_pool_free: Interrupt context !\n") );

	timeout( xpt_pool_free, ( caddr_t )cam_ccb_increment, 30 * hz );
	return( XPT_ISR_CONTEXT );
    }

    /* Do a quick check to see if a pool update is already in progress.  The
    flag is checked, and if TRUE no more work is done.  If the flag is not
    TRUE the process is continued. Grab the free count from the header
    for first internal loop check. */

    xqh = &xpt_qhead.xws;			/* set the Q head pointer */
    XQHEAD_IPLSMP_LOCK( s, &xpt_qhead );	/* lock on the Q struct */

    /* is there an update inprog or has there been a pool alloc  */
    if( (xqh->xpt_flags & (XPT_UPDIP | XPT_ALLOC_CALL )) != 0 )
    {
	PRINTD( NOBTL, NOBTL, NOBTL, CAMD_FLOW,
	    ("xpt_pool_free() update inprogress\n") );

	/* Ckeck to see it just the ALLOC_CALL flag is set if so clear it. */
	if(((xqh->xpt_flags & XPT_ALLOC_CALL) != NULL) && (xqh->xpt_flags &
			XPT_UPDIP) == NULL)
	{
	    /* Clear the allocation flag..... */
	    xqh->xpt_flags &= ~XPT_ALLOC_CALL;
	}

        XQHEAD_IPLSMP_UNLOCK( s, &xpt_qhead );	/* unlock the Q struct */
	timeout( xpt_pool_free, ( caddr_t )cam_ccb_increment, 30 * hz );
	return( CAM_SUCCESS );		/* everything will get done */
    }


    xqh->xpt_flags |= XPT_UPDIP;		/* kind of a test and set seq */

    free_cnt = xqh->xpt_nfree;			/* local copy of # of free */


    /* Check the packet count aginst the number of free and the pool low
    water mark.  If necessary adjust "pcnt" to not go lower than the
    low water mark. */

    if( (I32)free_cnt > cam_ccb_pool_size )
    {
	pcnt = MIN( free_cnt - cam_ccb_pool_size, pcnt );

    }
    else
    {
	/* We don't want to trim past ccb pool size */
	xqh->xpt_flags &= ~(XPT_UPDIP);         /* release the update flag */
	XQHEAD_IPLSMP_UNLOCK( s, &xpt_qhead );    /* unlock on the Q struct */
	timeout( xpt_pool_free, ( caddr_t )cam_ccb_increment, 30 * hz );
	return( CAM_SUCCESS );
    }

    /* The packets will be removed from the front of the free side.  The pool
    is scanned along and when the correct number, adjusted pcnt, of packets
    have been counted off they are "transfered" to the temp Q.  Then packets
    are then returned to the system off of the temp Q one at a time.
    NOTE: SMP locking, locks will be placed around the queue head access. */

    /* SMP lock on the queue head, remove the packet and update the 
    number of free CCBs. */


    for( i = 0 ; i < pcnt; i++ )

    {
	if ( xpt_qhead.xws.x_flink == (XPT_WS *)&xpt_qhead.xws.x_flink ) 
	{
	    /* We are in trouble.... Our free count doesn't match
	    whats on the list... */
	    break;
	}
	/* Get the ccb to free from the front... */
        xws = xpt_qhead.xws.x_flink;			
	XPT_WS_REMOVE( xws ); 
	xpt_qhead.xws.xpt_nfree--;
	cam_zfree((char *)xws, sizeof(DEC_CAM_PKT));
	xpt_qhead.xpt_ccbs_total--;
    }
    if( xpt_qhead.xws.x_flink == (XPT_WS *)&xpt_qhead.xws.x_flink )
    {
	CAM_ERROR( xpt_pool_free, "CAM XPT free counts don't match list", 
	    (CAM_ERR_SEVERE), xqh, (void *)NULL, (void *)NULL ); 
    }

    xqh->xpt_flags &= ~(XPT_UPDIP);     /* release the update flag */

    XQHEAD_IPLSMP_UNLOCK( s, &xpt_qhead );
    timeout( xpt_pool_free, ( caddr_t )cam_ccb_increment, 30 * hz );

    /* Signal all done. */
    return( i );			/* return the number actually freed */
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : xpt_errlog

Functional Description :

Local error logging routine for the XPT.  Using the arguments from the 
caller a system error log packet is filled with the error information and 
then stored within the system.

Formal Parameters :
The six arguments via the macro:

    fs : The Function name string
    ms : The message string
    ef : Relivant error flags
    qh : The XPT Qhead pointer if not NULL
    ws : An XPT Working Set pointer if not NULL
    ch : An CCB Header structure pointer if not NULL

Implicit Inputs : None

Implicit Outputs : Error Log Structures.

Return Value : None

Side Effects : A Lot of Error log work.

Additional Information :
*/

/* ARGSUSED */
static void
xpt_errlog( fs, ms, ef, qh, ws, ch )
    char *fs;				/* Function Name string */
    char *ms;				/* The message string */
    U32 ef;				/* Relivant error flags */
    XPT_QHEAD *qh;			/* XPT Qhead pointer */
    XPT_WS *ws;				/* XPT Working Set pointer */
    CCB_HEADER *ch;			/* CCB Header structure pointer */
{
    CAM_ERR_HDR err_hdr;		/* Local storage for the header */
    CAM_ERR_HDR *eh;			/* pointer for accessing it */

    CAM_ERR_ENTRY err_ent[XPT_ERR_CNT];	/* the local error entry list */
    CAM_ERR_ENTRY *cee;			/* error entry pointer */

    U32 ent;				/* current entry number in the list */
    U32 i;				/* loop counter */

    /* Setup the local variables. */

    ent = 0;				/* first entry */

    /* Clear out the header structure and the Error structures. */

    bzero( &err_hdr, sizeof(CAM_ERR_HDR) );	/* Zero fill the header */

    for( i = 0; i < XPT_ERR_CNT; i++ )
    {
	bzero( &err_ent[i], sizeof(CAM_ERR_ENTRY) );	/* fill the entry */
    }

    /* Setup the Error log header. */

    eh = &err_hdr;			/* pointer for the header */

    eh->hdr_type = CAM_ERR_PKT;		/* A CAM error packet */
    eh->hdr_class = CLASS_XPT;		/* the XPT is in a class by itself */
    eh->hdr_subsystem = SUBSYS_XPT;	/* fill in the subsystem field */
    eh->hdr_entries = ent;		/* set the entries */
    eh->hdr_list = err_ent;		/* point to the entry list */
    eh->hdr_pri = ef;			/* the priority is passed in */

    /* Setup the First Error entry on the list.  This entry will contain the
    function string. */

    cee = &err_ent[ ent ];		/* set the pointer */

    if( fs != (char *)NULL )			/* make sure there is one */
    {
	cee->ent_type = ENT_STR_MODULE;		/* the entry is a string */
	cee->ent_size = strlen(fs) +1;		/* how long it is */
	cee->ent_vers = 1;			/* Version 1 for strings */
	cee->ent_data = (u_char *)fs;			/* point to it */
	cee->ent_pri = PRI_BRIEF_REPORT;	/* strings are brief */
	ent++;					/* on to the next entry */
    }

    /* Setup the Next Error entry on the list.  This entry will contain the
    message string. */

    cee = &err_ent[ ent ];		/* set the pointer */

    if( ms != (char *)NULL )			/* make sure there is one */
    {
	cee->ent_type = ENT_STRING;		/* the entry is a string */
	cee->ent_size = strlen(ms) +1;		/* how long it is */
	cee->ent_vers = 1;			/* Version 1 for strings */
	cee->ent_data = (u_char *)ms;		/* point to it */
	cee->ent_pri = PRI_BRIEF_REPORT;	/* strings are brief */
	ent++;					/* on to the next entry */
    }

    /* Setup the Next Error entry on the list.  This entry will contain the
    XPT Q-Head structure if present. */

    cee = &err_ent[ ent ];		/* set the pointer */

    if( qh != (XPT_QHEAD *)NULL )		/* make sure there is one */
    {
	cee->ent_type = ENT_XPT_QHEAD;		/* this entry is the Q-head */
	cee->ent_size = sizeof(XPT_QHEAD);	/* how big it is */
	cee->ent_vers = XPT_QHEAD_VERS;		/* current version */
	cee->ent_data = (u_char *)qh;		/* point to it */
	cee->ent_pri = PRI_FULL_REPORT;		/* structures are full */
	ent++;					/* on to the next entry */
    }

    /* Setup the Next Error entry on the list.  This entry will contain 
    a XPT working set structure if present. */

    cee = &err_ent[ ent ];		/* set the pointer */

    if( ws != (XPT_WS *)NULL )			/* make sure there is one */
    {
	cee->ent_type = ENT_XPT_WS;		/* this entry is the wrk set */
	cee->ent_size = sizeof(XPT_WS);		/* how long it is */
	cee->ent_vers = XPT_WS_VERS;		/* current version */
	cee->ent_data = (u_char *)ws;		/* point to it */
	cee->ent_pri = PRI_FULL_REPORT;		/* structures are full */
	ent++;					/* on to the next entry */
    }

    /* Setup the Next Error entry on the list.  This entry will contain 
    a CCB header structure if present. */

    cee = &err_ent[ ent ];		/* set the pointer */

    if( ch != (CCB_HEADER *)NULL )		/* make sure there is one */
    {
	cee->ent_type = (U32)ch->cam_func_code; /* use the func code */
	cee->ent_size = (U32)ch->cam_ccb_len;   /* use the passed len */
	cee->ent_vers = CAM_VERSION;		/* current version */
	cee->ent_data = (u_char *)ch;		/* point to it */
	cee->ent_pri = PRI_FULL_REPORT;		/* structures are full */
	ent++;					/* on to the next entry */
    }

    /* Call the CAM Error logger handler with the error structures. */

    eh->hdr_entries = ent;		/* signal how many valid entries */

    cam_logger( eh, (char)NO_ERRVAL, (char)NO_ERRVAL, (char)NO_ERRVAL ); 

    return;				/* all done */
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* This code will have to change for the ULTRIX/BSD to ULTRIX/OSF port !! */
/* ---------------------------------------------------------------------- */

/*
Routine Name : cam_inisr()

Functional Description :

    This routine will return a TRUE/FALSE value to signal if the code is
currently running in an interrupt context.

Formal Parameters : None

Implicit Inputs : 

    System CPU data structures.

Implicit Outputs : None

Return Value :

    A TRUE/FALSE value.

Side Effects : TBD

Additional Information :

    This routine will be one of the Ultrix/BSD -> Ultrix/OSF port problems.
*/

I32
cam_inisr()
{
    /* Use the CPU specific flag/MACRO signaling interrupt state. */

    return( AT_INTR_LVL() );
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : cam_at_boottime()

Functional Description :

    The cam_at_boottime routine is used to determine if the code is running
during the system probe.

Formal Parameters : None

Implicit Inputs : 

    System global variable : cold

Implicit Outputs : None

Return Value :

    A TRUE/FALSE value.

Side Effects : TBD

Additional Information :

    This routine will be one of the Ultrix/BSD -> Ultrix/OSF port problems.
*/

I32
cam_at_boottime()
{
    extern int cold;			/* CPU conf global */

    return( (cold == 1) );
}

/*
Routine Name : xpt_alloc_dcp()

Functional Description :

    The xpt_alloc_dcp routine is used to allocate a Dec CAM Packet.
This function was created to allow a clean interface given the OSF
and ULTRIX differences in allocating space.

Formal Parameters : dcp, pointer to a pointer of type DEC_CAM_PKT.

Implicit Inputs :  None

Implicit Outputs :  None

Return Value : None

Side Effects : TBD

Additional Information :

*/
static void
xpt_alloc_dcp(dcp)
DEC_CAM_PKT **dcp;
{
    *dcp = (DEC_CAM_PKT *)cam_zalloc(sizeof(DEC_CAM_PKT));
}


/* ---------------------------------------------------------------------- */
/*
 * Routine Name : xpt_callback_thread() 
 *
 * Functional Description : This routine calls back the drivers
 *	when the command is complete or an error has occurred.
 *	operates as a thread.
 *
 * Call Syntax : xpt_callback_thread()
 *
 * Arguments :	None
 *
 * Return Value :  None
 */
void
xpt_callback_thread()
{
	register XPT_WS *xpt_ws;
	int s;
        thread_t thread = current_thread();

        thread_swappable(thread, FALSE);
/*
 * RT_SCHED: Change hardcoded priority to a constant.  Always valid.
 * The constant is defined in kern/sched.h.
 * Stomping on sched_pri here won't hurt, because the thread is running and
 * therefore not on a run queue.
 */
        thread->priority = thread->sched_pri = BASEPRI_HIGHEST;

/*        unix_master();          /* XXX signals sent in timeouts */

	
        (void) splsoftclock();

        XPT_CB_LOCK(&xpt_cb_queue, s);

        for (;;) {
	    /*
	    s = splbio();
	    */
	    if (xpt_callback_thread_init && !shutting_down){
		assert_wait((vm_offset_t)xpt_callback_thread, FALSE);
	        XPT_CB_UNLOCK(&xpt_cb_queue, s);
	    }
	    else {
		XPT_CB_UNLOCK(&xpt_cb_queue, s);
	    }
            thread_block();

	    XPT_CB_LOCK(&xpt_cb_queue, s);
	    xpt_cb_queue.flags = XPT_CB_ACTIVE;
	    XPT_CB_UNLOCK(&xpt_cb_queue, s);

again:
	    do {
	        XPT_CB_LOCK(&xpt_cb_queue, s);
		if( xpt_cb_queue.flink != (XPT_WS *)&xpt_cb_queue){
		    xpt_ws = xpt_cb_queue.flink;
		    XPT_CB_WS_REMOVE( xpt_cb_queue.flink);
		    xpt_cb_queue.count--;

	    	    XPT_CB_UNLOCK(&xpt_cb_queue, s);

		    if( xpt_ws == (XPT_WS *)NULL){
		    	panic("callback thread xpt_ws == NULL");
		    }

		    /*
		     * Call back
		     */
		    ((CCB_SCSIIO *)xpt_ws->xpt_ccb)->cam_cbfcnp(xpt_ws->xpt_ccb);

		}
		else {
		    XPT_CB_UNLOCK(&xpt_cb_queue, s);
	        }

	    } while( xpt_cb_queue.flink != (XPT_WS *)&xpt_cb_queue)

	    XPT_CB_LOCK(&xpt_cb_queue, s);
	    /*
	     * MAKE sure nothing on queue 
	     */
	    if(  xpt_cb_queue.flink != (XPT_WS *)&xpt_cb_queue){ 
		XPT_CB_UNLOCK(&xpt_cb_queue, s);
		goto again;
	    }

	    xpt_cb_queue.flags &= ~XPT_CB_ACTIVE;

        }
        /* NOTREACHED */
}

/* ---------------------------------------------------------------------- */
/*
 * Routine Name : xpt_pool_alloc_thread() 
 *
 * Functional Description : This routine calls xpt_pool_alloc
 *	to get more memory from the system, operates as a thread.
 *
 * Call Syntax : xpt_pool_alloc_thread()
 *
 * Arguments :	None
 *
 * Return Value :  None
 */

void
xpt_pool_alloc_thread()
{
	int s;
        thread_t thread = current_thread();

        thread_swappable(thread, FALSE);
/*
 * RT_SCHED: Change hardcoded priority to a constant.  Always valid.
 * The constant is defined in kern/sched.h.
 * Stomping on sched_pri here won't hurt, because the thread is running and
 * therefore not on a run queue.
 */
        thread->priority = thread->sched_pri = BASEPRI_HIGHEST;

/*        unix_master();          /* XXX signals sent in timeouts */

	
        (void) splsoftclock();
	
        XQHEAD_IPLSMP_LOCK( s, &xpt_qhead );	/* lock on the Q struct */
	for(;;){
	    /*
	    s = splbio();
	    */
	    if (xpt_pool_alloc_thread_init && !shutting_down){
		assert_wait((vm_offset_t)xpt_pool_alloc_thread, FALSE);
	        XQHEAD_IPLSMP_UNLOCK(s, &xpt_qhead);
	    }
	    else {
	        XQHEAD_IPLSMP_UNLOCK(s, &xpt_qhead);
	    }
            thread_block();
again:

            XQHEAD_IPLSMP_LOCK( s, &xpt_qhead );/* lock on the Q struct */
	    /* Clear the scheduled bit and set active */
	    xpt_qhead.xws.xpt_flags &= ~XPT_ALLOC_SCHED;
	    xpt_qhead.xws.xpt_flags |= XPT_ALLOC_ACT;
	    XQHEAD_IPLSMP_UNLOCK(s, &xpt_qhead);

	    /* Call the memory allocator */
	    xpt_pool_alloc( cam_ccb_increment );

	    XQHEAD_IPLSMP_LOCK( s, &xpt_qhead );    /* lock on the Q struct */
	    /* 
	     * Check to see if we have to go again.... saves on thread 
	     * context switch
	     */
    	    if((xpt_qhead.xws.xpt_nfree <= cam_ccb_low_water) && 
			(xpt_qhead.xpt_ccbs_total < xpt_qhead.xpt_ccb_limit)) {
		XQHEAD_IPLSMP_UNLOCK(s, &xpt_qhead);
		goto again;			/* BD did this */
	    }
	    xpt_qhead.xws.xpt_flags &= ~XPT_ALLOC_ACT;

	    /* Following a allocation request, successful or other
	    wise, there *HAS TO BE* a wakeup issued for I/Os on the
	    wait queue.  In the event that the above GOTO->AGAIN: loop
	    drops out without allocating any CCBs, then the Low water
	    CCBs wil be used. */

	    thread_wakeup_one( (vm_offset_t)&xpt_qhead.xpt_wait_cnt );
	}
}

/* ---------------------------------------------------------------------- */
/*
 * Routine Name :  scsiisr_init
 *
 * Functional Description : Initialize All SCSI threads here.
 *
 * Call Syntax : scsiisr_init()
 *
 * Arguments:	None
 *
 * Return Value :  None
 */
void
scsiisr_init()
{
	int i;
        extern task_t first_task;

	if (xpt_callback_thread_init == NULL){
	    /*
	     * Init the thread and get it running
	     */
	    xpt_callback_thread_init++;
            (void) kernel_thread(first_task, xpt_callback_thread);
	}
	if(xpt_pool_alloc_thread_init == NULL) {
	    /*
	     * Init the thread and get it running
	     */
	    xpt_pool_alloc_thread_init++;
            (void) kernel_thread(first_task, xpt_pool_alloc_thread);
	}

	return;
}



/* ---------------------------------------------------------------------- */
/*
 * Routine Name :  xpt_callback
 *
 * Functional Description : This routine inserts to the tail of the
 *	xpt_callback_que. If the machine is shutting down or the
 * 	thread is not init'ed it calls the driver itself. If normal
 *	operation then a wakeup is done on the xpt_callback_thread.
 *
 * Call Syntax : xpt_callback( CCB_SCSIIO * )
 *
 * Arguments:	Pointer to the sim working set that need a callback.
 *
 * Return Value :  None
 */
void
xpt_callback( ccb )
    CCB_SCSIIO *ccb;
{
    XPT_WS *xpt_ws;
    int s1;

    /*
     * Get this ccb's XPT_WS
     */

    xpt_ws = XPT_GET_WS_ADDR( ccb );

    /*
     * Lock the call back queue
     */
    XPT_CB_LOCK( &xpt_cb_queue, s1 );

    /*
     * Insert on queue
     */
    XPT_CB_WS_INSERT( xpt_ws,xpt_cb_queue.blink );
    xpt_cb_queue.count++;
	
    if( xpt_callback_thread_init && !shutting_down ){
        /*
         * If the call back thread is not active or scheduled
         * wake it up
         */
        if(( xpt_cb_queue.flags == NULL)) {
	    xpt_cb_queue.flags |= XPT_CB_SCHED;
	    thread_wakeup_one((vm_offset_t)xpt_callback_thread);
	    XPT_CB_UNLOCK(&xpt_cb_queue, s1 );
        }

    }
    else {
        XPT_CB_UNLOCK(&xpt_cb_queue, s1 );
        while ( xpt_cb_queue.flink != (XPT_WS *)&xpt_cb_queue ){
	    XPT_CB_LOCK( &xpt_cb_queue, s1 );
	    xpt_ws = xpt_cb_queue.flink;
	    XPT_CB_WS_REMOVE( xpt_cb_queue.flink );
	    xpt_cb_queue.count--;
            XPT_CB_UNLOCK(&xpt_cb_queue, s1 );
	    if(xpt_ws == (XPT_WS *)NULL){
	        panic("sx_commnad_complete XPT_WS == NULL");
	    }

		
	    /*
	     * Call back the driver
	     */
    	    ((CCB_SCSIIO *)xpt_ws->xpt_ccb)->cam_cbfcnp(xpt_ws->xpt_ccb);
        }
    }
}

/* ---------------------------------------------------------------------- */
/* This is the Declaration of the CAM function code lookup jump table.
At this time all the routines loaded are internal to the XPT layer and they
inturn call the respective "action" routines.  This table must match the
current CAM spec's function opcode table.  This really belongs at the head
of this file in the data section, but it is better for just out down here. */

CCB_CMD_ENTRY ccb_tbl[] =
{
    xpt_sim_func,		/* 0x00 XPT_NOOP */
    xpt_sim_func,		/* 0x01 XPT_SCSI_IO */
    xpt_ccfg_func,		/* 0x02 XPT_GDEV_TYPE */
    xpt_pinq_func,		/* 0x03 XPT_PATH_INQ */
    xpt_sim_func,		/* 0x04 XPT_REL_SIMQ */
    xpt_ccfg_func,		/* 0x05 XPT_SASYNC_CB */
    xpt_ccfg_func,		/* 0x06 XPT_SDEV_TYPE */
     xpt_reserved,		/* 0x07 Reserved cmd */
     xpt_reserved,		/* 0x08 Reserved cmd */
     xpt_reserved,		/* 0x09 Reserved cmd */
     xpt_reserved,		/* 0x0A Reserved cmd */
     xpt_reserved,		/* 0x0B Reserved cmd */
     xpt_reserved,		/* 0x0C Reserved cmd */
     xpt_reserved,		/* 0x0D Reserved cmd */
     xpt_reserved,		/* 0x0E Reserved cmd */
     xpt_reserved,		/* 0x0F Reserved cmd */

    xpt_sim_func,		/* 0x10 XPT_ABORT */
    xpt_sim_func,		/* 0x11 XPT_RESET_BUS */
    xpt_sim_func,		/* 0x12 XPT_RESET_DEV */
    xpt_sim_func,		/* 0x13 XPT_TERM_IO */
     xpt_reserved,		/* 0x14 Reserved cmd */
     xpt_reserved,		/* 0x15 Reserved cmd */
     xpt_reserved,		/* 0x16 Reserved cmd */
     xpt_reserved,		/* 0x17 Reserved cmd */
     xpt_reserved,		/* 0x18 Reserved cmd */
     xpt_reserved,		/* 0x19 Reserved cmd */
     xpt_reserved,		/* 0x1A Reserved cmd */
     xpt_reserved,		/* 0x1B Reserved cmd */
     xpt_reserved,		/* 0x1C Reserved cmd */
     xpt_reserved,		/* 0x1D Reserved cmd */
     xpt_reserved,		/* 0x1E Reserved cmd */
     xpt_reserved,		/* 0x1F Reserved cmd */

     xpt_reserved,		/* 0x20 Reserved cmd */
     xpt_reserved,		/* 0x21 Reserved cmd */
     xpt_reserved,		/* 0x22 Reserved cmd */
     xpt_reserved,		/* 0x23 Reserved cmd */
     xpt_reserved,		/* 0x24 Reserved cmd */
     xpt_reserved,		/* 0x25 Reserved cmd */
     xpt_reserved,		/* 0x26 Reserved cmd */
     xpt_reserved,		/* 0x27 Reserved cmd */
     xpt_reserved,		/* 0x28 Reserved cmd */
     xpt_reserved,		/* 0x29 Reserved cmd */
     xpt_reserved,		/* 0x2A Reserved cmd */
     xpt_reserved,		/* 0x2B Reserved cmd */
     xpt_reserved,		/* 0x2C Reserved cmd */
     xpt_reserved,		/* 0x2D Reserved cmd */
     xpt_reserved,		/* 0x2E Reserved cmd */
     xpt_reserved,		/* 0x2F Reserved cmd */

     xpt_sim_func,		/* 0x30 XPT_EN_LUN */
     xpt_reserved,		/* 0x31 XPT_TARGET_IO */
     xpt_reserved,		/* 0x32 Reserved cmd */
     xpt_reserved,		/* 0x33 Reserved cmd */
     xpt_reserved,		/* 0x34 Reserved cmd */
     xpt_reserved,		/* 0x35 Reserved cmd */
     xpt_reserved,		/* 0x36 Reserved cmd */
     xpt_reserved,		/* 0x37 Reserved cmd */
     xpt_reserved,		/* 0x38 Reserved cmd */
     xpt_reserved,		/* 0x39 Reserved cmd */
     xpt_reserved,		/* 0x3A Reserved cmd */
     xpt_reserved,		/* 0x3B Reserved cmd */
     xpt_reserved,		/* 0x3C Reserved cmd */
     xpt_reserved,		/* 0x3D Reserved cmd */
     xpt_reserved,		/* 0x3E Reserved cmd */
     xpt_reserved,		/* 0x3F Reserved cmd */

     xpt_reserved,		/* 0x40 Reserved cmd */
     xpt_reserved,		/* 0x41 Reserved cmd */
     xpt_reserved,		/* 0x42 Reserved cmd */
     xpt_reserved,		/* 0x43 Reserved cmd */
     xpt_reserved,		/* 0x44 Reserved cmd */
     xpt_reserved,		/* 0x45 Reserved cmd */
     xpt_reserved,		/* 0x46 Reserved cmd */
     xpt_reserved,		/* 0x47 Reserved cmd */
     xpt_reserved,		/* 0x48 Reserved cmd */
     xpt_reserved,		/* 0x49 Reserved cmd */
     xpt_reserved,		/* 0x4A Reserved cmd */
     xpt_reserved,		/* 0x4B Reserved cmd */
     xpt_reserved,		/* 0x4C Reserved cmd */
     xpt_reserved,		/* 0x4D Reserved cmd */
     xpt_reserved,		/* 0x4E Reserved cmd */
     xpt_reserved,		/* 0x4F Reserved cmd */

     xpt_reserved,		/* 0x50 Reserved cmd */
     xpt_reserved,		/* 0x51 Reserved cmd */
     xpt_reserved,		/* 0x52 Reserved cmd */
     xpt_reserved,		/* 0x53 Reserved cmd */
     xpt_reserved,		/* 0x54 Reserved cmd */
     xpt_reserved,		/* 0x55 Reserved cmd */
     xpt_reserved,		/* 0x56 Reserved cmd */
     xpt_reserved,		/* 0x57 Reserved cmd */
     xpt_reserved,		/* 0x58 Reserved cmd */
     xpt_reserved,		/* 0x59 Reserved cmd */
     xpt_reserved,		/* 0x5A Reserved cmd */
     xpt_reserved,		/* 0x5B Reserved cmd */
     xpt_reserved,		/* 0x5C Reserved cmd */
     xpt_reserved,		/* 0x5D Reserved cmd */
     xpt_reserved,		/* 0x5E Reserved cmd */
     xpt_reserved,		/* 0x5F Reserved cmd */

     xpt_reserved,		/* 0x60 Reserved cmd */
     xpt_reserved,		/* 0x61 Reserved cmd */
     xpt_reserved,		/* 0x62 Reserved cmd */
     xpt_reserved,		/* 0x63 Reserved cmd */
     xpt_reserved,		/* 0x64 Reserved cmd */
     xpt_reserved,		/* 0x65 Reserved cmd */
     xpt_reserved,		/* 0x66 Reserved cmd */
     xpt_reserved,		/* 0x67 Reserved cmd */
     xpt_reserved,		/* 0x68 Reserved cmd */
     xpt_reserved,		/* 0x69 Reserved cmd */
     xpt_reserved,		/* 0x6A Reserved cmd */
     xpt_reserved,		/* 0x6B Reserved cmd */
     xpt_reserved,		/* 0x6C Reserved cmd */
     xpt_reserved,		/* 0x6D Reserved cmd */
     xpt_reserved,		/* 0x6E Reserved cmd */
     xpt_reserved,		/* 0x6F Reserved cmd */

     xpt_reserved,		/* 0x70 Reserved cmd */
     xpt_reserved,		/* 0x71 Reserved cmd */
     xpt_reserved,		/* 0x72 Reserved cmd */
     xpt_reserved,		/* 0x73 Reserved cmd */
     xpt_reserved,		/* 0x74 Reserved cmd */
     xpt_reserved,		/* 0x75 Reserved cmd */
     xpt_reserved,		/* 0x76 Reserved cmd */
     xpt_reserved,		/* 0x77 Reserved cmd */
     xpt_reserved,		/* 0x78 Reserved cmd */
     xpt_reserved,		/* 0x79 Reserved cmd */
     xpt_reserved,		/* 0x7A Reserved cmd */
     xpt_reserved,		/* 0x7B Reserved cmd */
     xpt_reserved,		/* 0x7C Reserved cmd */
     xpt_reserved,		/* 0x7D Reserved cmd */
     xpt_reserved,		/* 0x7E Reserved cmd */
     xpt_reserved,		/* 0x7F Reserved cmd */

     xpt_sim_func		/* 0x80 XPT_VUNIQUE Region */
};

U32 N_ccb_tbl = (sizeof( ccb_tbl )/sizeof( CCB_CMD_ENTRY ));

/* ---------------------------------------------------------------------- */
/*
Routine Name :

Functional Description :

Formal Parameters : None

Implicit Inputs : None

Implicit Outputs : None

Return Value :
    A long value, indicating CAM_SUCCESS or CAM_FAILURE.

Side Effects :

Additional Information :
*/

/* ---------------------------------------------------------------------- */
/* END OF FILE */
/* ---------------------------------------------------------------------- */


