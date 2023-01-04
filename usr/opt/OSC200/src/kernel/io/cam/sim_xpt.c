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
static char *rcsid = "@(#)$RCSfile: sim_xpt.c,v $ $Revision: 1.1.16.9 $ (DEC) $Date: 1994/01/13 16:09:21 $";
#endif

/**
 * FACILITY:
 *
 *	ULTRIX SCSI CAM SUBSYSTEM
 *
 * ABSTRACT:
 *
 * The SIM XPT is the component of the SIM which the XPT calls to 
 * initiate I/O on behalf  of peripheral device drivers (PDrv).
 * The peripheral device driver allocates a CCB from the XPT
 * and fills CCB fields with SCSI command bytes and other information.
 * The PDrv then hands this CCB to the XPT which calls the selected SIM's,
 * sim_action routine to begin processing of the I/O request. The SIM XPT
 * (SIMX) portion of the SIM queues this request and notifies the SIM
 * SCHEDULER (SIMS) of the pending request. The SIMS then selects
 * the next I/O to be processed by the SIM HBA (SIMH). The SIMH
 * initiates the SCSI bus transaction on the HBA. When SIMH and
 * HBA complete the transaction, the SIMH returns to the SIMS which
 * calls the SIMX.  When the SIMX receives a completed I/O request
 * it will callback to  the peripheral device driver and release
 * any SIM resources allocated for the request.
 *
 * AUTHOR:
 *
 *	Richard L. Napolitano	13-Dec-1990
 *
 * MODIFIED BY:
 * 
 *	21-Feb-1990 	Richard L. Napolitano      
 *      Include comments from code review.
 *
 *	26-Feb-1991	Janet L. Schank
 *	Moved sx_process_timeouts and sx_initialize_timer to the
 *	SIM Scheduler code.  Removed sx_abort(), sx_terminate(),
 *	sx_bus_reset(),
 *
 *	26-Mar-1991	Janet L. Schank
 *	Return values no longer "or" in the queue frozen bit.  Now
 *	including dec_cam.h.
 *
 *	29-Mar-1991	John A. Gallant
 *	Modified the Initialization path.  Removed the csr argument from 
 *	sim_init, and the csr is nolonger stored in the softc here.
 *
 *	06-May-1991	John A. Gallant
 *	Added the support to implement the Path Inquiry CCB.
 *
 *	29-May-1991	Janet L. Schank
 *	sx_setup_ws() will now setup the "it_nexus" field the SIM_WS.
 *
 *	04-Jun-1991	Janet L. Schank
 *	Ignore return value from DME_END().
 *
 *	17-Jun-1991	Janet L. Schank
 *	In sx_command_complete, make sure auto sense is not in progress
 *	before coping the cam_status.
 *
 *	17-Jun-1991	Maria Vella
 *	In sx_setup_ws, save contoller number in sim_ws.
 *  
 *      28-Jun-1991     Robert P. Scott
 *      Added new configuration code.
 *
 *	08-Jul-1991	Janet L. Schank
 *	In sx_clear_nexusq() added a call to ss_nexus_delete() to remove
 *	the NEXUS from the waiting list.  Added sx_async().
 *
 *	15-Jul-1991	Maria Vella
 *	Added pathid argument to sx_async() call in sx_reset_detected().
 *	Return CAM_BUSY when reset in progress in sim_action.
 *
 *	20-Jul-1991	Robert P. Scott
 *	Added [b/t/l]'s to PRINTD's.
 *
 *	13-Aug-1991	Maria Vella
 *	Clear error_recovery field before calling sx_async().
 *
 *	05-Sep-1991	Janet L. Schank
 *	Modified to use the Nexus active tag array.
 *
 *	19-Sep-1991	Janet L. Schank
 *	Modified sx_get_tag() and sx_free_tag() to set and clear
 *	the SZ_TAG_ELEMENT_INUSE flag.
 *
 *	22-Oct-1991	Janet L. Schank
 *	o Added SIM_MODULE() to every function.
 *	o Replaced all PRINTD's with SIM_PRINTD's.
 *	o Added the function sx_alloc() to allocate SIM XPT structures.
 *	o Added a call to sim_enable_interrupts() to the end of sim_init().
 *
 *	11-Nov-1991	Janet L. Schank
 *	Removed all SC_ADD_FUNC() calls.
 *
 *	20-Nov-1991	Janet L. Schank
 *	Changed a bunch of variables to be registers.
 *
 **/



/* ---------------------------------------------------------------------- 
 *
 * Include files.
 *
 */
#include <io/common/iotypes.h>
#include <sys/types.h>
#include <sys/time.h>
#include <io/cam/dec_cam.h>
#include <mach/vm_param.h>
#include <io/cam/cam.h>
#include <io/cam/scsi_all.h>
#include <vm/vm_kern.h>
#include <io/cam/cam_debug.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/dme.h>
#include <io/cam/sim.h>
#include <io/cam/sim_common.h>
#include <io/cam/scsi_status.h>
#include <io/cam/sim_xpt.h>
#include <io/cam/cam_errlog.h>

/* ---------------------------------------------------------------------- 
 *
 * Local defines:
 * 
 */

#define SIM_NOTAG -1		/* Move this to sim.h                        */
#define MAX_CAM_HBA 4		/* This needs to be defined in a config file */
#define SX_TIMEOUT_PERIOD 4	/* Default timeout period for SIM timer      */
#define TIME_FACTOR 1000	/* Constant used to delay on order seconds   */

#define TBD_LOCK_HANDLE 0	/* For SMP, use the addr of a structure      */


/* ---------------------------------------------------------------------- 
/*
 * Local declarations
 */

void sx_bdr_notify_retry();
void sx_done_device_reset();
void sx_queue_async();
void sx_queue_done_async();
U32  sx_enable_lun();


/* ---------------------------------------------------------------------- 
 *
 * External declarations:
 *
 */
extern U32 ss_init();
extern U32 ss_go();	 	/* SIMS function to start a I/O request */
extern U32 ss_device_reset();
extern U32 ss_abort();
extern U32 ss_terminate_io();
extern void sc_init_wsq();
extern void xpt_callback();
extern void sim_enable_interrupts();
extern wakeup();
extern int hz;

/*
 * There is one SOFTC_DIRECTORY for the entire system. For each HBA present 
 * there is one entry in the SOFTC_DIRECTORY. The SOFTC_DIRECTORY is indexed 
 * by the pathid and contains the address of the softc for a particular HBA.
 * This structure is statically allocated and filled in when the kernel is
 * built.
 */
extern SIM_SOFTC* softc_directory[];	/* Address of HBA's sim_softc */



/*----------------------------------------------------------------------
 * 
 * Initialized and uninitialized data: 
 * 
 */
static u_short sx_sequence_number =0;	/* Count CAM_SCSIIO requests */

/*
 * This structure is used by the XPT to find the address of the SIM's
 * entry points. The cam_conftbl[] contains pointers to thest type of 
 structures.
 */
extern I32 sim_init();
extern I32 sim_action();

CAM_SIM_ENTRY dec_sim_entry =		/* One per SIM, used by the XPT */
{
    sim_init,				/* to initialize the SIM */
    sim_action				/* to "do" a CCB */
};

/*
 * These arrays contain the Vendor ID strings used in the Path Inquiry CCB.
 * Their length must not be any greater than, SIM_ID and HBA_ID, which
 * are 16 and 16 respectivly, from the cam.h include file. 
 */

/*  string position         "0000000001111111" */
/*  counter                 "1234567890123456" */
static char *simx_sim_vid = "DEC OSG BSD/V1  ";
static char *simx_hba_vid = "DEC WSE BSD/V1  ";

/*---------------------------------------------------------------------- 
 *
 * Function Prototypes: 
 *
 */ 
static SIM_WS* sx_setup_ws();
static U32 sx_clear_pending_ccb();
static U32 sx_clear_nexusq();
static U32 sx_free_tag();
U32 sx_log_error();
U32 sx_get_tag();
static U32 handle_immediate();
static void sx_dme_cleanup();
U32 sx_device_reset();		/* Call SIMX reset device function */
void sx_async();
static void sx_alloc();

/*---------------------------------------------------------------------- 
 *
 * Macros: 
 *
 * See sim_xpt.h include file.
 */	

/******************************************************************************
 *
 * Start of SIM_XPT.C routines.
 *
 *****************************************************************************/



/**
 * sim_action -
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * This routine initiates an I/O (sim_ws) request from a peripheral driver.
 * If the action specified in the function code field of the CCB is an 
 * "immediate" function the SIMX will perform the action or call down to a SIMH
 * to execute the function. If the function is a queued function, the SIMX
 * will insert this CCB onto a nexus specific queue using the sim_ws and then 
 * notify the SIMS of this request. When the request has completed the SIMX
 * will callback directly to the peripheral driver via the CCB callback 
 * address.
 *
 * This routine is used by the XPT for immediate as well as queued
 * operations. The SIMX decides whether an operation is immediate
 * or queued based on the function code field in the CCB.  All 
 * queued operations such as "Execute SCSI I/O" (reads or writes)
 * are queued by the SIMX to a nexus specific queue and return with
 * a CAM status of CAM_INPROG. Some immediate operations, as
 * described in the CAM specification, may not actually be executed
 * immediately. However, all immediate CCB's will return to the 
 * XPT immediately. For example, the  CAM  Abort command function
 * will not always complete synchronous with it's call, however the
 * Abort CCB  will be returned to the XPT immediately. Reset SCSI
 * bus CCB's will both return immediately to the XPT and execute
 * synchronously with the call.
 *
 *
 * FORMAL PARAMETERS:    CCB_HEADER CCB_HDR  - ADDRESS OF HEAD OF CCB.
 *
 * IMPLICIT INPUTS:      The sim_ws is found by referencing the TBD in DCP
 *
 * IMPLICIT OUTPUTS:	 sim_ws
 *
 * RETURN VALUE:         This routine will return valid CAM STATUS bytes ONLY
 *                       as the return argument to this routine as well as in 
 *			 the CCB CAM status field. See Table 9-2 in the CAM
 *			 specification.
 *
 *                       CAM_REQ_INPROG  - For queued commands
 *                       CAM_REQ_CMP     - For immediate commands
 *
 * SIDE EFFECTS:         Too numerous to enumerate at this time.
 *
 * ADDITIONAL INFORMATION:         Callers -  CAM XPT.
 *
 **/
I32
sim_action (ccb_hdr)
    
    register CCB_HEADER* ccb_hdr;
    
{
    register SIM_SOFTC *sim_softc = SC_GET_SOFTC(ccb_hdr->cam_path_id);
    U32 retval;
    register SIM_WS* sim_ws;		/* Ptr to current I/O's working set */
    SIM_MODULE(sim_action);

    /*
     * Do not allow any requests if currently recovering from a
     * SCSI bus reset.
     */
    if (sim_softc->error_recovery & ERR_BUS_RESET) {
	ccb_hdr->cam_status = CAM_BUSY;
	return(CAM_BUSY);
    }
    /*
     * Do not allow any requests if currently recovering from a
     * SCSI bus device reset.
     */
    if(sim_softc->device_reset_inprog & (1 << ccb_hdr->cam_target_id)){
	ccb_hdr->cam_status = CAM_BUSY;
	return(CAM_BUSY);
    }
	
    /*
     * Set the cam_status of the CCB to CAM_REQ_INPROG.
     */
    ccb_hdr->cam_status = CAM_REQ_INPROG;

    /*
     * The XPT has called the SIM with a CCB/DCP from a peripheral driver.
     * The SIM XPT will look at the CCB to determine whether this is a queued
     * or immediate function.
     */
    if (ccb_hdr->cam_func_code == XPT_SCSI_IO )
    {
	/*
	 * Setup this I/O's SIM working set.
	 */
	sim_ws = sx_setup_ws(ccb_hdr);

	/*
	 * Is this a tagged request?
	 */
	if (ccb_hdr->cam_flags & CAM_QUEUE_ENABLE)
	{
	    /*
	     * If so, allocate a tag (if we can) before calling ss_go().
	     */
	    sx_get_tag(sim_ws);
	}

	/*
	 * Wakeup the schedular and initiate an I/O.  Ss_go()
	 * will place the SIM_WS on the NEXUS queue.  It will
	 * also determing if the NEXUS queue is frozen and
	 * return an appropriate status.
	 */
	retval = ss_go((SIM_SOFTC*)sim_ws->sim_sc,sim_ws);
    }			/* If queued function       */
    else		/* Else, immediate function */
    {
	/*
	 * Execute all CCB's other than queued and return cam status
	 */
	retval = ccb_hdr->cam_status = handle_immediate(ccb_hdr);
    };			/* Else, immediate function */
    
    return(retval);
};			/* sim_action */

/**
 * sim_init -
 *
 * FUNCTIONAL DESCRIPTION:  This routine starts the SIM initialization sequence
 * The SIMX calls the TBD SIMH entry point to cause the SIMH to initialize the
 * HBA. The SIMX will clear all it's queues and release all allocated resources
 * in response to this call.   The Active Tag Array(s) (ATA) is allocated and
 * filled in. The CAM_SIM_ENTRY structure must have been assigned prior to 
 * calling this routine. This function may be called at anytime and it is up to
 * the SIMX and SIMH to insure that data integrity is maintained.
 *
 * FORMAL PARAMETERS:    U32 pathid
 *
 * IMPLICIT INPUTS:      none.
 *
 * IMPLICIT OUTPUTS:     none.
 *
 * RETURN VALUE:         CAM_REQ_CMP - All went well.
 *
 * SIDE EFFECTS:         none.
 *
 * ADDITIONAL INFORMATION: Callers - CAM XPT
 *
 **/
I32 
sim_init ( pathid )
    U32 pathid;
{
    I32 target_cnt;		   	/* Loop counter */
    I32 lun_cnt;			/* Loop counter */
    U32 retval=CAM_FAILURE;		/* Routine return values */
    U32 retval1;			/* Routine return values */
    SIM_SOFTC* sim_softc;		/* Pointer to sim softc structure */
    caddr_t csr;			/* CSR addess from softc struct */
    caddr_t CSR =0;			/* CSR of HBA to initialize */
    U32 tag;				/* Used when setting up the ATA */
    SIM_MODULE(sim_init);

    SIM_PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	       ("entry - pathid=%d \n", pathid ) );

    sim_softc = SC_GET_SOFTC(pathid);		   /* Get softc pointer */

    SIM_PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	       ("sc=0x%x \n", sim_softc ) );

    csr = sim_softc->csr_probe;		/* get what was stored by probe */

    /* The SIM softc strucuture was "allocated" and placed into the 
     * directory array.  It has not yet been completely initialized.
     * The misc flags, locks, and structures that are global are set here.
     */

    /* JAG/RLN : What other fields ? */
    SIM_SOFTC_LOCK_INIT(sim_softc);	/* Initialize the softc lock */
    SIM_REG_LOCK_INIT(sim_softc);	/* Initialize the reg/HBA lock */

    if ( hba_dme_attach( pathid, sim_softc ) != CAM_REQ_CMP )
      {
	/* 
	 * If the attach fails don't do any further initialization.
	 */
	SIM_PRINTD( pathid, NOBTL, NOBTL, CAMD_INOUT,
		   ("hba_dme attach failed.\n") );
	retval = CAM_REQ_CMP_ERR;
      }
    else
        {
	/*
	 * Initialize the SIM SCHED.
	 */
	retval1 = ss_init(sim_softc);	/* Initialize the SIM SCHED */
	/* 
	 * Initialize the SIM HBA   
	 */
	retval = SC_HBA_INIT(sim_softc, csr);
        }
    /*
     * If we can intialize the SIM HBA and the SIM SCHEDULAR then initialize
     * the SIM XPT otherwise return to the XPT with an error status.
     */
    if (
	(sim_softc->simx_init == 0) &&
	(retval  == CAM_REQ_CMP ) &&	/* If HBA initialized   */
	(retval1 == CAM_REQ_CMP ) 	/* If SIM SCHED Initialized */
	)
    {
	/* 
	 * If the Active Tags Array has already been allocated
	 * then don't allocate it again.
	 */
	if (sim_softc->ata == (TAG_ELEMENT *)NULL)
	{
	    sim_softc->max_tag = SIM_MAX_TAG;

	    /* 
	     * Allocate SIM XPT structures 
	     */
	    sx_alloc(sim_softc);

	    /*
	     * Setup the ATA and ATA linked list.
	     */
	    sim_softc->ata_list->flink = sim_softc->ata_list;
	    sim_softc->ata_list->sim_ws = (SIM_WS *)sim_softc->ata_list;
	    for (tag=0; tag < sim_softc->max_tag; tag++)
            {
	         sim_softc->ata[tag].tag = tag;
		 insque(&sim_softc->ata[tag], sim_softc->ata_list->sim_ws);
            }

	    sim_softc->simx_init = CAM_TRUE; /* The SIM XPT has initialized  */

	}	    		         /* The ATA was NOT allocated before */

	/*
	 * There has been a design decision made to reduce the amount of
	 * memory required to manage tags in the sim. The SIM will
	 * allow a maximum of 255 active tags in a given SIM. The SCSI
	 * specification allows 255 tags per I_T_L nexus, however at this
	 * time that seems extremely excessive, however this code has been 
	 * designed to allow that possiblity in the future, by maintaining 
	 * the pointer to the ATA in the nexus structure rather than the 
	 * sim_softc. In order to support 255 tags per lun this routine 
	 * should be modified to allocate one ATA per nexus. The tag 
	 * allocation and deallocation routine will reference the ATA 
	 * through the nexus structure, therefore reducing the work 
	 * involved in supporting 255 tags per I_T_L nexus.
	 */
	for ( target_cnt = MAX_TARGETS; target_cnt >= 0; target_cnt--)
	{
	    for ( lun_cnt = MAX_LUNS; lun_cnt >= 0; lun_cnt--)
	    {
		/* 
		 * Copy the one system wide ATA address into the
		 * per nexus structure. This will allow all nexus
		 * to share a common pool of tags.
		 */
		sim_softc->nexus[target_cnt][lun_cnt].ata = sim_softc->ata;
		sim_softc->nexus[target_cnt][lun_cnt].ata_list =
			sim_softc->ata_list;

		/*
		 * Initialize the nexus queues forward and backward SIM_WS
		 * pointers. The nexus queues are used by the SIMX and SIMS
		 * to schedule I/O. The SIMX inserts requests onto the nexus
		 * queues to have them scheduled by the scheduler. The SIMX
		 * removes requests from these queues as these requests 
		 * complete.
		 *
		 * Set the flink and blink to point at the flink to start 
		 * the list off.
		 */
		sim_softc->nexus[target_cnt][lun_cnt].flink = 
		    sim_softc->nexus[target_cnt][lun_cnt].blink = 
			(SIM_WS *)(&sim_softc->nexus[target_cnt][lun_cnt]);


	    };		/* For lun_cnt */
	};		/* Loop through all nexus structures */
	retval = CAM_REQ_CMP;
    };			/* If the SIM XPT has been initialized */

    /*
     * We can now handle an interrupt.
     */
    sim_enable_interrupts(sim_softc);

    return(retval);
};				/* sim_init */

U32
sim_unload( SIM_SOFTC *sim_softc )
    {
    SIM_MODULE(sim_unload);
    return CAM_REQ_CMP;
    }

/**
 * sx_setup_ws -
 *
 * FUNCTIONAL DESCRIPTION:
 * 
 * This routine is called by sim_action to return a pointer to a ccb's sim_ws
 * as well as doing the required sim_ws setup.
 *
 * FORMAL PARAMETERS:   CCB_HEADER CCB_HDR  - ADDRESS OF HEAD OF CCB.
 *
 * IMPLICIT INPUTS:	None.
 *
 * IMPLICIT OUTPUTS:	None.
 *
 * RETURN VALUE:        This routine return a pointer to the sim_ws.
 *
 * SIDE EFFECTS:        None.
 *
 * ADDITIONAL INFORMATION:         Callers -  sim_action.
 *
 **/
static SIM_WS* 
sx_setup_ws(ccb_hdr)

    register CCB_HEADER* ccb_hdr;
   
{
    register SIM_WS* sim_ws;
    SIM_MODULE(sx_setup_ws);

    sim_ws = SC_GET_WS(ccb_hdr);	/* Get the address of the sim_ws */

    /*
     * Zero out the contents of the SIM_WS.
     */
    bzero(sim_ws, sizeof(SIM_WS));
    
    /*
     * Setup various fields in the sim_ws that are required by the SIM to 
     * process an I/O request.
     *
     */
    sim_ws->sim_sc 	= (void*) SC_GET_SOFTC(ccb_hdr->cam_path_id);
    sim_ws->cam_flags	= ccb_hdr->cam_flags;
    sim_ws->cam_sort    = ((CCB_SCSIIO *)ccb_hdr)->cam_sort;
    sim_ws->cam_priority=
	((CCB_SCSIIO *)ccb_hdr)->cam_vu_flags & DEC_CAM_PRIORITY_MASK;
    sim_ws->cntlr	= ccb_hdr->cam_path_id;
    sim_ws->targid	= ccb_hdr->cam_target_id;
    sim_ws->lun		= ccb_hdr->cam_target_lun;
    sim_ws->it_nexus	= SC_GET_IT_NEXUS(sim_ws->sim_sc,
					  ccb_hdr->cam_target_id);
    sim_ws->nexus = SC_GET_NEXUS(sim_ws->sim_sc,ccb_hdr->cam_target_id,
			      ccb_hdr->cam_target_lun);
    /*
     * Setup pointer to CCB in working set so that we can get directly to
     * the CCB when given the SIM_WS.
     */
    sim_ws->ccb		= (CCB_SCSIIO*) ccb_hdr;
    
    /* 
     * Assign a relatively unique sequence number to each request in the
     * SIM. There is NO guarantee that this sequence number is unique!
     * However, it's a low cost way of tracking each individual I/O in
     * the SIM. It should only be used for debug and error logging!!!
     */
    sim_ws->seq_num = sx_sequence_number++;
    
    /*
     * Here we are using a valid CAM status as a flag to the SIM HBA 
     * rather than to the peripheral driver, which is the "normal" case.
     * The status of CAM_CDB_RECVD used by the SIM HBA as a flag to
     * indicate that a sim_ws on the nexus queue  is ready be executed.
     */
    sim_ws->cam_status = CAM_CDB_RECVD;

    /*
     * Setup the queues.
     */
    sc_init_wsq(sim_ws);
    
    return(sim_ws);
};			/* sx_setup_ws */

/**
 * handle_immediate -
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * This routine is called by sim_action to execute all XPT CCB's other than
 * SCSI I/O requests.
 *
 * FORMAL PARAMETERS:   CCB_HEADER CCB_HDR  - ADDRESS OF HEAD OF CCB.
 *
 * IMPLICIT INPUTS:     The sim_ws is found by referencing the TBD in DCP
 *
 * IMPLICIT OUTPUTS:	None.
 *
 * RETURN VALUE:        This routine will return valid CAM STATUS bytes ONLY
 *                      as the return argument to this routine as well as in
 *			the CCB CAM status field. See Table 9-2 in the CAM
 *			specification.
 *
 * SIDE EFFECTS:        None.
 *
 * ADDITIONAL INFORMATION:         Callers -  sim_action.
 *
 **/
static U32
handle_immediate (ccb_hdr)

register CCB_HEADER* ccb_hdr;	/* Pointer to CCB header */
    
{
    int s;			/* Save the IPL */
    register SIM_SOFTC *sim_softc = SC_GET_SOFTC(ccb_hdr->cam_path_id);
    register NEXUS *nexus =
	&sim_softc->nexus[ccb_hdr->cam_target_id][ccb_hdr->cam_target_lun];
    U32 retval;
    SIM_MODULE(handle_immediate);
    
    /*
     * Parse the cam cam_func_code field to determine the type of CCB 
     */
    switch ( ccb_hdr->cam_func_code )
    {
	
    case XPT_NOOP:		/* Noop */
	SIM_PRINTD(ccb_hdr->cam_path_id,ccb_hdr->cam_target_id,
		   ccb_hdr->cam_target_lun,CAMD_FLOW,
		   ("The NOOP command is supported\n"));

	retval = CAM_REQ_CMP;	/* Request completed */
	break;
	
    case XPT_SCSI_IO:   		/* execute the requested scsi io */
	panic("Internal logic error");/* These CCB's should be handled above */
	retval = CAM_REQ_CMP;	/* Request completed */
	break;
	
    case XPT_GDEV_TYPE: 		/* get the device type information */
	SIM_PRINTD(ccb_hdr->cam_path_id,ccb_hdr->cam_target_id,
		   ccb_hdr->cam_target_lun,CAMD_FLOW,
		   ("this command should have been handled by the xpt\n"));

	retval = CAM_REQ_INVALID;	/* Bad request to the SIM_XPT */
	break;
	
    /*
     * The Path Inq CCB is used to relay information about the
     * XPT/SIM back to the PDrv.  This CCB has already been
     * "pre-processed" by the XPT and some of the fields may have
     * info loaded.  Because of this all the bit settings have
     * to be "|=", or-ed in, so not to unset any of the pre-set flags.
     * Note: All of the "feature" fields have been cleared by the XPT.
     */
    case XPT_PATH_INQ:  		/* path inquiry */
	{
	    CCB_PATHINQ *cpi;		/* pointer for the specific CCB */

	    SIM_PRINTD(ccb_hdr->cam_path_id, NOBTL, NOBTL, CAMD_FLOW,
		       ("XPT_PATH_INQ CCB received.\n"));

	    cpi = (CCB_PATHINQ *)ccb_hdr;	/* Cast to the Path Inq CCB */

	    /*
	     * Using the softc structure and the static ID arrays, set the
	     * rest of the Path Inq. CCB.
	     */

	    cpi->cam_hba_inquiry |=		/* Known features */
		(PI_SDTR_ABLE);

	    cpi->cam_hba_inquiry |=		/* HBA specific features */
		sim_softc->path_inq_flags;

	    cpi->cam_hba_eng_cnt = 0;		/* no HBA engines */

	    bzero( cpi->cam_vuhba_flags, VUHBA ); /* clear VU flags */

	    cpi->cam_async_flags |=		/* SCSI Bus impacts */
		(AC_SENT_BDR | AC_UNSOL_RESEL | AC_BUS_RESET);

	    cpi->cam_initiator_id = (u_char)sim_softc->scsiid; /* SCSI bus ID */

	    bcopy( simx_sim_vid, cpi->cam_sim_vid, SIM_ID );	/* SIM Vendor */
	    bcopy( simx_hba_vid, cpi->cam_hba_vid, HBA_ID );	/* HBA Vendor */

	    retval = CAM_REQ_CMP;	/* Request completed */
	}
	break;

    /*
     * Release the SIM queue that is frozen.
     */
    case XPT_REL_SIMQ: 
    {

	SIM_PRINTD(ccb_hdr->cam_path_id,ccb_hdr->cam_target_id,
		   ccb_hdr->cam_target_lun,CAMD_FLOW,
		   ("Unfreeze queue CCB received.\n"));

	/*
	 * Raise IPL and SMP lock.
	 */
	LOCK_NEXUS(s,TBD_LOCK_HANDLE);

	/*
	 * Reduce the freeze count in the NEXUS field.
	 */
	if ((--nexus->freeze_count) < 0 ) {
		nexus->freeze_count = 0;
	}

	/*
	 * Lower IPL and SMP unlock.
	 */
	UNLOCK_NEXUS(s,TBD_LOCK_HANDLE);

	/*
	 * Call ss_go() to start off any requests which might be on
	 * the previously frozen queue.
	 */
	ss_go(sim_softc, (SIM_WS *)NULL);
	retval = CAM_REQ_CMP;
	break;
    }
	
    case XPT_SASYNC_CB:	 	/* set async callback parameters */
	SIM_PRINTD(ccb_hdr->cam_path_id,ccb_hdr->cam_target_id,
		   ccb_hdr->cam_target_lun,CAMD_FLOW,
		   ("What's the right thing to do here, sasync_cb.\n"));
	retval = CAM_REQ_INVALID;
	break;
	
    case XPT_SDEV_TYPE: 		/* set the device type information */
	SIM_PRINTD(ccb_hdr->cam_path_id,ccb_hdr->cam_target_id,
		   ccb_hdr->cam_target_lun,CAMD_FLOW,
		   ("This command should have been handled by the XPT\n"));
	retval = CAM_REQ_INVALID;	/* Bad request to the SIM_XPT */
	break;
	
    case XPT_ABORT:     		/* Abort the selected CCB */
	SIM_PRINTD(ccb_hdr->cam_path_id,ccb_hdr->cam_target_id,
		   ccb_hdr->cam_target_lun,CAMD_FLOW,
		   ("Abort immediate CCB received.\n"));
	/* Call the SIMX abort function */	
	ss_abort(sim_softc, (U32)ccb_hdr->cam_target_id,
		 (U32)ccb_hdr->cam_target_lun,
		 (CCB_SCSIIO*)((CCB_ABORT*)ccb_hdr)->cam_abort_ch);
	retval = CAM_REQ_CMP;		/* Always return success        */
	break;
	
    case XPT_RESET_BUS: 		/* Reset the SCSI bus */
	SIM_PRINTD(ccb_hdr->cam_path_id,ccb_hdr->cam_target_id,
		   ccb_hdr->cam_target_lun,CAMD_FLOW,
		   ("RESET bus immediate CCB received.\n"));
	/*
	 * Call the HBA specific SCSI bus reset function.
	 */
	SC_HBA_BUS_RESET(sim_softc);
	retval = CAM_REQ_CMP;		/* Always return success        */
	break;

    case XPT_RESET_DEV: 		/* Reset the device bus */
	SIM_PRINTD(ccb_hdr->cam_path_id,ccb_hdr->cam_target_id,
		   ccb_hdr->cam_target_lun,CAMD_FLOW,
		   ("RESET device CCB received.\n"));
	/* Call SIMS reset device function */
	ss_device_reset(sim_softc, (U32)ccb_hdr->cam_target_id);
	retval = CAM_REQ_CMP;		/* Always return success        */
	break;

    case XPT_TERM_IO: 			/* Terminate I/O */
	SIM_PRINTD(ccb_hdr->cam_path_id,ccb_hdr->cam_target_id,
		   ccb_hdr->cam_target_lun,CAMD_FLOW,
		   ("Terminate I/O CCB received.\n"));
	/* Call SIMS term I/O function */
	ss_terminate_io(sim_softc, (U32)ccb_hdr->cam_target_id,
			(U32)ccb_hdr->cam_target_lun,
			(CCB_SCSIIO*)((CCB_TERMIO*)ccb_hdr)->cam_termio_ch);
	retval = CAM_REQ_CMP;		/* Always return success        */
	break;
	
  case XPT_EN_LUN:
        SIM_PRINTD(ccb_hdr->cam_path_id,ccb_hdr->cam_target_id,
                   ccb_hdr->cam_target_lun,CAMD_FLOW,
                   ("Enable LUN CCB received\n"));
        retval = sx_enable_lun(sim_softc, nexus, ccb_hdr);
        break;

    default:
	SIM_PRINTD(ccb_hdr->cam_path_id,ccb_hdr->cam_target_id,
		   ccb_hdr->cam_target_lun,CAMD_FLOW,
		   ("Bad CCB function code\n"));
	retval = CAM_REQ_INVALID;
	break;
    };
    
    return(retval);
};			/* handle_immediate */

/**
 * SX_GET_TAG -
 *
 * FUNCTIONAL DESCRIPTION: This routine writes the value of the
 * next available tag in the sim_ws tag field. Sx_get_tag
 * guarantees that a tag is unique for the duration of the request.
 * This routine insures that there are not more than max_tags per bus and not 
 * more than 255 tags active on a given nexus by maintaining a count of the 
 * number of tags assigned to each nexus. Increment counters of the number of
 * tags outstanding on the SIM and on this nexus. If there are no tags 
 * available, a tag value of SIM_NOTAG (-1) is returned in the sim_ws.
 *
 * FORMAL PARAMETERS:   SIM_WS*  sim_ws   - Address of sim_ws.
 *
 * IMPLICIT INPUTS:     sim_ws->nexus.
 *			
 * IMPLICIT OUTPUTS:	sim_ws->tag - filled in with unique tag value
 *			sim_ws->flags - Set SZ_TAGGED(if tagged) or 
 *					SZ_TAG_PENDING if no tag available.
 *
 * RETURN VALUE:        U32	CAM_REQ_CMP - All is well.
 *
 * SIDE EFFECTS:        none.
 *
 * ADDITIONAL INFORMATION: Callers - sim_action
 **/
U32 
sx_get_tag(sim_ws)
    register SIM_WS* sim_ws;
    
{
    U32 retval = CAM_REQ_CMP;
    register TAG_ELEMENT *tag_element;
    int s;
    SIM_MODULE(sx_get_tag);

    /*
     * SMP lock on the SIM_SOFTC.
     */
    SIM_SOFTC_LOCK(s, sim_ws->sim_sc);

    /*
     * Get the TAG_ELEMENT from the front on the NEXUS linked list.
     */
    tag_element = sim_ws->nexus->ata_list->flink;

    /*
     * Are there any tags available in the NEXUS's ATA?  If not,
     * use a tag of -1.
     */
    if (tag_element == sim_ws->nexus->ata_list)
    {
         sim_ws->tag = SIM_NOTAG;
         sim_ws->flags |= SZ_TAG_PENDING;
    }

    /*
     * If there were availabe tags, setup the TAG_ELEMENT.
     */
    else
    {
        remque(tag_element);
	tag_element->sim_ws = sim_ws;
	tag_element->flags |= SZ_TAG_ELEMENT_INUSE;
        sim_ws->tag = tag_element->tag;
	sim_ws->flags |= SZ_TAGGED;
        sim_ws->flags &= ~SZ_TAG_PENDING;
    }
    
    SIM_SOFTC_UNLOCK(s, sim_ws->sim_sc);

    return(retval);
};			/* sx_get_tag */



/**
 * sx_free_tag -
 * 
 * FUNCTIONAL DESCRIPTION: This routine will free the tag value
 * from this sim_ws and decrement the count of outstanding tags on this nexus.
 *
 * FORMAL PARAMETERS:    LONG   sim_ws   - Address of sim_ws.
 *
 * IMPLICIT INPUTS:	softc, tag_free_head,tag_free_tail, max_nexus_tags,
 * 			max_sim_tags
 * IMPLICIT OUTPUTS:     sim_ws tag field cleared, the tag_free_head pointer is
 *			 updated
 *
 * RETURN VALUE:         LONG CAM_REQ_CMP - All is well.
 *
 * SIDE EFFECTS:         none.
 *
 * ADDITIONAL INFORMATION: Callers - SIMX - sx_command_complete
 *
 */
static U32 
sx_free_tag(sim_ws)
    register SIM_WS* sim_ws;
    
{
    U32 retval = CAM_REQ_CMP;
    register TAG_ELEMENT *tag_elem;
    int s;
    SIM_MODULE(sx_free_tag);

    /*
     * Is the tag valid?  If not, return a fail status.
     */
    if ((sim_ws->tag >= 0) && (sim_ws->tag < sim_ws->sim_sc->max_tag))
    {
         /*
          * Get the TAG_ELEMENT and add it to the end of the ata_list.
          */
         tag_elem = &sim_ws->nexus->ata[sim_ws->tag];
	 tag_elem->flags &= ~SZ_TAG_ELEMENT_INUSE;
         SIM_SOFTC_LOCK(s, sim_ws->sim_sc);
         insque(tag_elem, sim_ws->nexus->ata_list->sim_ws);
         SIM_SOFTC_UNLOCK(s, sim_ws->sim_sc);
    }
    else
         retval = CAM_FAILURE;

    return(retval);
};

/**
 * sx_command_complete -
 *
 * FUNCTIONAL DESCRIPTION: This routine is called after the SIMS
 * has completed work on a SIM_WS. The SIM SCHEDULER has returned the
 * SIM_WS to the SIMX and this routine will execute the end of I/O
 * processing. If the I/O failed with a check condition, then this
 * routine will freeze the nexus queue and initiate auto request
 * sense processing, otherwise this routine will cleanup and
 * callback the peripheral driver.
 *
 * FORMAL PARAMETERS:    LONG   sim_ws   - Address of sim_ws.
 *
 * IMPLICIT INPUTS:      none.
 *
 * IMPLICIT OUTPUTS:     none.
 *
 * RETURN VALUE:         CAM_REQ_CMP - All is well
 *
 * SIDE EFFECTS:         none.
 *
 * ADDITIONAL INFORMATION: Callers - SIMS, ss_finish
 *
 **/
U32 sx_command_complete(sim_ws)
    register SIM_WS* sim_ws;
    
{

    SIM_MODULE(sx_command_complete);

    /* 
     * Copy SCSI status which was received by the SIM HBA into CCB 
     * so the peripheral driver can get the SCSI status of this request.
     */
    sim_ws->ccb->cam_scsi_status = sim_ws->scsi_status;
    
    /*
     * Trace each queued command
     */
    SIM_PRINTD(sim_ws->ccb->cam_ch.cam_path_id,sim_ws->targid,sim_ws->lun,
	       CAMD_FLOW,
	       ("CCB - SEQNUM = %d, CMD= %x\n",
	    	sim_ws->seq_num,sim_ws->ccb->cam_cdb_io));
    
    /*
     * If this was a tagged queued request then release any tags
     * that were allocated to it.
     */
    if (sim_ws->flags & SZ_TAGGED)
    {
	(void)sx_free_tag(sim_ws);
    };

    /*
     * Calculate the residual byte count. The residual byte count is the
     * requested transfer size minus the number of bytes actaully transfered.
     */
    sim_ws->ccb->cam_resid = 
	sim_ws->data_xfer.data_count - sim_ws->data_xfer.xfer_count;

    /*
     * Now that this request has completed copy the cam_status of the 
     * sim_hba/sim_sched into the CCB.
     *
     * If autosense is inprogress on this CCB don't copy the cam_status.
     * the as_finish() function will take care of this.
     */
    if (!(sim_ws->flags & SZ_AS_INPROG))
	sim_ws->ccb->cam_ch.cam_status = sim_ws->cam_status;

    /*
     * Check the CCB to determine whether the callback address is
     * valid.
     */
    if (!(sim_ws->ccb->cam_ch.cam_flags & CAM_DIS_CALLBACK))
    {
	/* 
	 * Call xpt_callback for insertion onto the callback
	 * queue and thread wakeup if needed.
	 */
	xpt_callback( sim_ws->ccb );
    };

    return(CAM_REQ_CMP);

};			/* sx_command_complete */

/**
 * sx_reset_detected -
 *
 * FUNCTIONAL DESCRIPTION: 
 * 
 * This routine is called by the SIMH in response to a reset of the 
 * SCSI bus to perform SIMX cleanup and call sx_clear_pending_ccb 
 * to flushing pending requests to the peripheral drivers.
 * This routine will traverse the nexus queues returning each request 
 * to it's peripheral driver with a CAM status of CAM_SCSI_BUS_RESET. 
 * The SIM will return a CAM status of CAM_BUSY until all pending requests 
 * have been completed.
 *
 * FORMAL PARAMETERS:	U32 pathID      - HBA controller identification
 *
 * IMPLICIT INPUTS:     none.
 *
 * IMPLICIT OUTPUTS:    none.
 *
 * RETURN VALUE:        CAM_REQ_CMP - All is well
 *
 * SIDE EFFECTS:          All pending I/O's are timed out.
 *
 * ADDITIONAL INFORMATION:      	Callers -  SIMH, TBD
 *				This function will initiate I/O cleanup of
 *				pending I/O requests; call sx_clear_pending_ccb
 *
 **/
U32 
sx_reset_detected(pathid)

    U32 pathid;
{
    U32 retval;		/* Routine return values */
    SIM_SOFTC* sim_softc;	/* Pointer to softc      */
    int s;			/* Save copy of IPL      */
    SIM_MODULE(sx_reset_detected);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	       ("Entered: sx_reset_detected.\n"));
    /*
     * Clear all pending requests on sim nexus queues.
     */
    sim_softc = SC_GET_SOFTC(pathid);

    /*
     * Clear all pending CCB's off of the nexus queues
     *
     * For SMP we really need to lock the enter SIM, not just a single
     * nexus. In the case we need to lock all the nexus. We may want to set
     * cam busy, but given this design that is not necessary.
     */
    LOCK_NEXUS(s,TBD_LOCK_HANDLE);
    retval = sx_clear_pending_ccb(sim_softc);
    UNLOCK_NEXUS(s,TBD_LOCK_HANDLE);

    /*
     * The clear operation fail, then log an error.
     */
    if (retval != CAM_REQ_CMP)
    {
	retval = sx_log_error();
    };		       /* Log addition information about clr_pnd_ccb failure */
	

    sx_queue_async(AC_BUS_RESET,pathid,-1,-1, sx_reset_detected );
    return( retval );

};			/* sx_reset_detected */

/* 
 * sx_bdr_notify_retry
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * With the command completion callbacks now done in a thread 
 * we must guarantee the callbacks to the drives have been done
 * before we do the async callback notification.
 * This routine is called as a timeout routine when the ccb
 * pools are depleted. This is detected by sx_queue_async
 * and is only used for BDR's to decode the pathid passed.
 * Then calls sx_done_device_reset for the notification .
 *
 * FORMAL PARAMETERS: pathid - decoded to bus target lun
 *
 * IMPLICIT INPUTS:
 *
 * RETURN VALUE:         none
 *
 **/
void
sx_bdr_notify_retry( pathid )
U32	pathid;
{
    U32	bus;
    I32 targid, lun;

    lun = ( pathid & 0xff );
    targid = (( pathid >> 8) & 0xff );
    bus = (( pathid >> 16)  & 0xff );

    sx_done_device_reset( (U32)AC_SENT_BDR, bus, targid, lun);
}

/* 
 * sx_done_device_reset
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * With the command completion callbacks now done in a thread 
 * we must guarantee the callbacks to the drives have been done
 * before we do the async callback notification. This routine
 * is called to handle the the scheduling of the call to 
 * the xpt_layer.
 *
 * FORMAL PARAMETERS:	 opcode
 *			 pathid - bus
 *			 target id
 *			 lun id;
 *
 * IMPLICIT INPUTS:
 *
 * RETURN VALUE:         none
 *
 **/
void
sx_done_device_reset( opcode, pathid, targid, lun )
U32 opcode, pathid; 
I32 targid, lun;
{


    sx_queue_async(AC_SENT_BDR, pathid,targid,lun, sx_bdr_notify_retry );
    return;
}



/* 
 * sx_queue_async
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * With the command completion callbacks now done in a thread 
 * we must guarantee the callbacks to the drives have been done
 * before we do the async callback notification.
 * This routine allocates a ccb and queues it to the call back
 * queue to insure that when the sx_queue_async_done routine
 * gets called all ccbs have been delivered.
 * This is only used for Bus device resets and Bus resets
 *
 * FORMAL PARAMETERS:    opcode  --  type of event
 *                       pathid
 *                       target id
 *			 timo_routine -- Who handles no ccbs
 *			 
 *
 * IMPLICIT INPUTS:      CCB_SCSIIO
 *
 * RETURN VALUE:         none
 *
 **/
void
sx_queue_async(opcode, pathid, targid, lun, timo_routine)
U32 opcode, pathid; 
I32 targid, lun;
void (*timo_routine)();
{
    CCB_SCSIIO *ccb;	/* what we are going to que */
    U32		ident = 0;



    if(( ccb = (CCB_SCSIIO *)xpt_ccb_alloc()) == (CCB_SCSIIO *)NULL){
	/*
	 * We didn't get a ccb pools must have been depleted
	 * Timeout this request to be handled later
	 * Must build our ident
	 */
	if( targid == -1){ /* Must be reset */
	    ident = (pathid & 0xff);
	}
	else { /* BDR */
	    ident = (((pathid & 0xff) << 16) | ( (targid & 0xff) << 8)  | 
			(lun & 0xff));
	}
	timeout( timo_routine, ident, hz/10 );
	return;
    }

    /*
     * Fill in the ccb 
     */
    ccb->cam_ch.cam_path_id = (u_char)pathid;
    ccb->cam_ch.cam_target_id = (u_char)targid;
    ccb->cam_ch.cam_target_lun = (u_char)lun;
    ccb->cam_ch.cam_flags = opcode; /* Since this never goes anywhere */

    ccb->cam_cbfcnp = sx_queue_done_async;

    /* 
     * Now call the xpt_callback routine to queue it...
     */

    xpt_callback(ccb);

    return;
}



/* 
 * sx_queue_done_async
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * With the command completion callbacks now done in a thread 
 * we must guarantee the callbacks to the drives have been done
 * before we do the async callback notification.
 * When the callback queue has drained the routine gets
 * called which then allows us to notify the upper
 * levels. Done by calling sx_async()
 *
 * FORMAL PARAMETERS:    CCB_SCSIIO *
 *
 * IMPLICIT INPUTS:      opcode  --  type of event
 *                       pathid
 *                       target id
 *			 lun id
 *			 softc * -- Constructed from pathid
 *			 
 *
 *
 * RETURN VALUE:         none
 *
 **/
void
sx_queue_done_async( ccb )
CCB_SCSIIO	*ccb;
{
    SIM_SOFTC* sim_softc;	/* Pointer to softc      */
    U32 pathid, opcode; 
    I32 targid, lun;




    pathid = (U32)ccb->cam_ch.cam_path_id;

    if(( ccb->cam_ch.cam_target_id & 0xff) == 0xff){
	targid = -1;
    }
    else {
        targid = (I32)ccb->cam_ch.cam_target_id;
    }
    if(( ccb->cam_ch.cam_target_lun & 0xff) == 0xff){
	lun = -1;
    }
    else {
        lun = (I32)ccb->cam_ch.cam_target_lun;
    }
    opcode = (U32)ccb->cam_ch.cam_flags;

    /* 
     * Done with the ccb free it
     */
    xpt_ccb_free( (CCB_HEADER *)ccb);

    /* 
     * get our softc 
     */
    sim_softc = SC_GET_SOFTC(pathid);

    if( sim_softc == (SIM_SOFTC *)NULL){
	panic("sx_queue_done_async: No softc");
    }

    /*
     * Clear the bus reset detected flag in SIM_SOFTC.
     */
    sim_softc->error_recovery = 0;

    if( opcode == (U32)AC_SENT_BDR){
	
        /*
         * Clear the target's bit in the device_reset_inprog field
         * of the SIM_SOFTC.
         */
        sim_softc->device_reset_inprog &= ~(1 << targid);
    }
    /*
     * A reset on the SCSI is one of the events that a peripheral driver may
     * register an asynch callback for. Here we call the XPT to notify the 
     * it of a SCSI bus reset. The XPT will callback any peripheral drivers 
     * that have registered callbacks.
     */
    sx_async(opcode,pathid,targid,lun ,NULL,0);


    return;
}
/**
 * sx_clear_pending_ccb -
 * 
 * FUNCTIONAL DESCRIPTION: 
 *
 * Clear ALL nexus queues of pending sim_ws or CCB's. The SIM will return
 * CAM busy status until all of the pending requests have been returned to 
 * peripheral drivers.
 * The SIM will force all pending requests to complete and return to the
 * peripheral driver with a CAM status of CAM_SCSI_BUS_RESET.
 *
 * FORMAL PARAMETERS:    SIM_SOFTC sim_softc
 *
 * IMPLICIT INPUTS:      nexus queues...
 *
 * IMPLICIT OUTPUTS:     none.
 *
 * RETURN VALUE:         CAM_REQ_CMP - All is well
 *
 * SIDE EFFECTS:         The SIM returns to PDrvr CAM_BUSY until all pending 
 *			requests have been completed.
 *
 * ADDITIONAL INFORMATION:         Callers -  SIMH, TBD
 *
 *
 **/
static U32 
sx_clear_pending_ccb (sim_softc)

    register SIM_SOFTC* sim_softc;
    				/* Addr of SOFTC which contains nexus queues */
    
{
    U32 retval;		/* Routine return values */
    register I32 target;	/* Loop counter for SCSI target id's */
    register I32 lun;		/* Loop counter for LUN's */
    SIM_MODULE(sx_clear_pending_ccb);
    
    SIM_PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	       ("clear all nexus of pending I/O requests.\n"));
    
    /*
     * Clear each target queue 
     */
    for (target = MAX_TARGETS; target >= 0; --target)
    {
	/*
	 * Clear each nexus queue 
	 */
	for (lun = MAX_LUNS; lun >= 0; --lun)
	{
	    /*
	     * Clear all pending none-active I/O's on this nexus 
	     */
	     retval = sx_clear_nexusq(SC_GET_NEXUS(sim_softc,target,lun),
				     (U32) CAM_SCSI_BUS_RESET);
	    
	    if (retval != CAM_REQ_CMP)
	    {
		break;		/* Stop if any nexus  can't be cleared */
	    };			/* clear nexus call failed */
	};			/* Loop through lun's */
    };				/* Loop through all targets */
    
    return(retval);
};			/* sx_clear_pending_ccb */

/**
 * sx_clear_nexusq -
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Clear the specified nexus queues of all pending  sim_ws or CCB's. The SIM
 * will complete all pending requests with the passed cam status and return 
 * them  to the peripheral driver. 
 * The SIM will force all pending requests to complete and return to the 
 * peripheral driver with a CAM status of CAM_SCSI_BUS_RESET.
 *
 * If this routine is called with a null nexus, the routine will simply return
 * CAM_REQ_CMP.
 *
 * FORMAL PARAMETERS:   
 *			NEXUS* nexus - Target/LUN queue of pending requests.
 *			U32 cam_status - CAM status to return for each 
 *					    pending I/O.
 *
 * IMPLICIT INPUTS:      None.
 *
 * IMPLICIT OUTPUTS:     None.
 *
 * RETURN VALUE:         CAM_REQ_CMP - All is well
 *
 * SIDE EFFECTS:         The SIM returns busy until all pending requests have 
 *			 been completed.
 *
 * ADDITIONAL INFORMATION:           Callers -  SIMS, ss_abort,ss_device _reset
 *
 **/
static U32
sx_clear_nexusq (nexus, cam_status)

    register NEXUS* nexus;	/* Address of the nexus I/O queue    */
    U32 cam_status;	/* CAM status to return fro each pending I/O */
    
{
    register SIM_WS* pend_ws;/* Temporary pointer to sim_ws on nexus queues */
    U32 retval;	/* Routiine return value                       */
    int s;		/* Save the IPL                                */
    SIM_MODULE(sx_clear_nexusq);
    
    /*
     * As a sanity check, be sure this isn't a NULL nexus.
     */
    if (nexus == NULL) 
	return(CAM_REQ_CMP);

    /*
     * Traverse the nexus queue and complete each pending I/O with a CAM status
     * of CAM_SCSI_BUS_RESET.
     */
    
    retval = CAM_REQ_CMP;
    LOCK_NEXUS(s,TBD_LOCK_HANDLE);/* Get nexus lock for synchronization */

    /*
     * Clear the Sorting curr_list and next_list counts and pointers.
     */
    nexus->curr_list = (SIM_WS *)NULL;
    nexus->curr_cnt = 0;
    nexus->next_list = (SIM_WS *)NULL;
    nexus->next_cnt = 0;

    pend_ws = nexus->flink;	  /* Start by pointing at first sim_ws */
    
    /*
     * Loop through all the sim_ws on this nexus. The list is empty if
     * flink of the nexus queue points at the nexus queue itself. The
     * nexus queue is organized as a doubley linked list. This allows
     * the SIMX to insert sim_ws on the front or rear of the queue.
     */
    while (
	   (pend_ws != (SIM_WS*)nexus) &&
	   (pend_ws->flink != NULL) 
	   )
    {
        /*
         * If this is a target mode nexus do not remove the ccbs.
         */
        if (nexus->flags & (SZ_PROCESSOR | SZ_TARG_DEF))
        {
		break;
	}
		
	/*
	 * Set status of CCB appropriately
	 */
	pend_ws->cam_status = cam_status;
	    
	/*
	 * Remove the SIM_WS from the NEXUS queue.
	 */
	SC_WS_REMOVE(pend_ws);

	/*
	 * Update the NEXUS waiting list.
         */
	if (--pend_ws->nexus->sws_count == 0) {
            ss_nexus_delete(pend_ws->sim_sc, pend_ws->nexus);
	}

	/*
	 * Release an DME resources which may have been allocated.
	 */
	sx_dme_cleanup(pend_ws);

	/*
	 * Complete this queued request, remove from nexus queue and
	 * call back peripheral driver if required.
	 */
	retval = sx_command_complete(pend_ws); 
	
	/*
	 * Start again and the head of the NEXUS list.
	 */
	pend_ws = nexus->flink;

    };			/* While loop through nexus sim_ws queue */
    UNLOCK_NEXUS(s,TBD_LOCK_HANDLE);	/* Free nexus lock */
    return(retval);
};			/* sx_clear_nexusq */

/**
 * sx_clear_itl_nexus -
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Clear all I/O's pending on the nexus queue specified by the inputs to this
 * function. Pending requests will be completed and return to the 
 * peripheral driver with a CAM status of CAM_SCSI_BUS_RESET.
 *
 * If this routine is called with a null nexus, the routine will simply return
 * CAM_REQ_CMP.
 *
 * NOTE: If the LUN passed to this function is -1 then all the
 * LUN's associated with a particular target id will be flushed 
 * of pending requests.
 *
 * FORMAL PARAMETERS: 
 *
 *		SIM_SOFTC* sim_softc;
 *		I32       target_id;
 *		I32       lun_id;
 * 		U32 	cam_status -  CAM status to return fro each pending I/O 
 *
 *
 * IMPLICIT INPUTS:     None.
 *
 * IMPLICIT OUTPUTS:    None.
 *
 * RETURN VALUE:        CAM_REQ_CMP - All is well
 *
 * SIDE EFFECTS:	The SIM returns busy until all pending requests have 
 *			been completed.
 *
 * ADDITIONAL INFORMATION:           Callers -  SIMS, ss_abort,ss_device _reset
 *
 **/
U32
sx_clear_itl_nexus (sim_softc, target_id, lun_id, cam_status)

    register SIM_SOFTC* sim_softc;
    register I32       target_id;
    register I32       lun_id;
    U32 cam_status;	/* CAM status to return fro each pending I/O */
    
{
    U32 retval;
    I32   lun;
    SIM_MODULE(sx_clear_itl_nexus);

    /* 
     * If the caller passes a LUN which is minus one to this function 
     * all of the nexi associated with a target id will be cleared 
     * and returned to the peripheral driver with the passed cam
     * status.
     */
    if (lun_id == -1)
    {			/* If lun == -1 */
	for (lun = MAX_LUNS; lun >= 0; --lun)
	{
	    
	    /*
	     * Clear all pending none active I/O's on this nexus 
	     */
	    retval = sx_clear_nexusq(SC_GET_NEXUS(sim_softc,target_id,lun),
				     cam_status);
	}			/* For */
	
    }  			/* If lun == -1 */
    else
    {    		/* Else lun != -1 */
	/*
	 * Clear all pending none active I/O's on this nexus.
	 */
	retval = sx_clear_nexusq(SC_GET_NEXUS(sim_softc,target_id,lun_id),
				 cam_status);
	    
    }			/* Else lun != -1 */

    return(retval);
};			/* sx_clear_itl_nexus */



/**
 * sx_enable_lun
 * 
 * FUNCTIONAL DESCRIPTION: 
 *
 * Sx_enable_lun() is called as a result of the ENABLE LUN CCB.
 * If the list count is non zero, then the LUN is to be enabled and
 * If the list count is zero then the LUN is to be disabled.
 *
 *
 * FORMAL PARAMETERS:    sc	- softc ptr.
 *                       nexus	- nexus ptr.
 *                       en_lun	- ptr to ENABLE LUN CCB.
 *
 * RETURN VALUE:
 *		CAM_TID_INVALID - Invalid host id specified in ENABLE LUN CCB.
 *		CAM_REQ_INVALID - Unsupported command in cdb.
 *				- Invalid cdb command length for command.
 *				- Disable has come in but LUN is not enabled.
 *				- Enable Lun request on a LUN that is 
 *				  already enabled.
 *				- Enable Lun request is missing an inquiry 
 *				  and/or a request sense ccb.
 *		CAM_PROVIDE_FAIL - Can't setup necessary DME resources.
 *		CAM_REQ_CMP - Success.
 *
 *
 **/
U32
sx_enable_lun(sc, nexus, en_lun)
SIM_SOFTC *sc;
NEXUS *nexus;
CCB_EN_LUN *en_lun;
{
    CCB_HEADER *ccb_hdr = (CCB_HEADER *)en_lun;
    CCB_HEADER **ccb_list;
    SIM_WS *sws;
    u_char cmd;
    u_char inq_found=0, reqsns_found=0;
    int i, s, retry;

    /*
     * First determine whether this HBA can support target mode operations.
     */
    if( !(sc->flags & SZ_TARG_CAPABLE) )
    {
        return(CAM_PROVIDE_FAIL);
    }

    /*
     * The target ID specified in the enable LUN CCB must match
     * what is returned by the HBA inquiry.
     */
    if (ccb_hdr->cam_target_id != sc->scsiid) 
    {
	return(CAM_TID_INVALID);
    }


    /*
     * Has the specified LUN already been setup for processor  mode?  If so, 
     * check to see if we are being requested to disable it.
     */
    if (nexus->flags & SZ_PROCESSOR) 
    {

	/*
	 * Is the number of target CCB's zero?  If so, disable processor mode.
	 */
	if (en_lun->cam_ccb_listcnt == 0) 
	{
    	    /*
     	     * Set flag indicating that this LUN is being disabled.
     	     */
    	    nexus->flags |= SZ_TARG_DISABLE_LUN;

	    /*
	     * For each SIM_WS in the nexus, make sure they are available
	     * to the SIM to remove (ie sim_ws is not active).
	     */
	    SIM_SOFTC_LOCK(s, sc);
	    sws = nexus->flink;
	    while (sws != (SIM_WS *)nexus) 
	    {
		retry = 0;
		/*
		 * Wait for the ccb to become available before disabling it.
		 */
		while(!(sws->ccb->cam_ch.cam_flags & CAM_TGT_CCB_AVAIL))
		{
		    timeout(wakeup, sws->ccb, 4*hz);
		    sleep(sws->ccb, PRIBIO);
		    
		    /*
		     * If the ccb is now available to us then no more waiting.
		     */
		    if (sws->ccb->cam_ch.cam_flags & CAM_TGT_CCB_AVAIL)
			break;
		
		    if(retry++ > DISABLE_LUN_RETRY)
		    {
			/*
			 * If here, we are going to give up on the disable
			 * so let's put the ccbs back to where they were 
			 * before the disbale LUN attempt (ie make them 
			 * available again.)
			 */
			SIM_WS *sptr = nexus->flink;
	    		while (sptr != sws) 
			{
			    sptr->ccb->cam_ch.cam_flags |= CAM_TGT_CCB_AVAIL;
			    sptr = sptr->flink;
			}
	    		SIM_SOFTC_UNLOCK(s, sc);
			return(CAM_PROVIDE_FAIL);
		    }
		} 	/* CCB not available */

		/*
		 * Indicate this ccb is no longer available for incoming CDBs.
		 */
		sws->ccb->cam_ch.cam_flags &= ~CAM_TGT_CCB_AVAIL;

		/*
		 * Get the next one in the list.
		 */
	        sws = sws->flink;
	    }	/* while */

	    /*
	     * Now that we have marked all the CCBs as not available then we
	     * must remove them from the NEXUS queue and call DME_END to 
	     * release their resources.  
	     */
	    while (nexus->flink != (SIM_WS *)nexus) 
	    {
	    	sws = nexus->flink;
		/*
		 * Remove it from the NEXUS queue.
		 */
		ss_finish(sws);

		/*
		 * Release its DME resources.
		 */
		DME_END(sc, &sws->data_xfer);
	    }

	    /*
	     * Clear the Processor mode and disabling flags.
	     */
	    nexus->flags &= ~(SZ_PROCESSOR | SZ_TARG_DISABLE_LUN);

	    SIM_SOFTC_UNLOCK(s, sc);

	    return(CAM_REQ_CMP);
	}

	/*
	 * We are already setup for processor mode and the enable lun CCB is 
	 * making another request to setup the same LUN.  The CAM document 
	 * specifies that a cam_status of invalid request will be returned.
	 */
	return(CAM_REQ_INVALID);
    }

    /*
     * We haven't been setup for processor mode yet.  Make sure the the 
     * enable lun CCB specfies at least one CCB.
     */
    if (en_lun->cam_ccb_listcnt == 0)
    {
	return(CAM_REQ_INVALID);
    }
   

    /*
     * Check all of the given CDB's.  If any are not supported
     * do not enable the LUN.
     */
    ccb_list = (CCB_HEADER **)en_lun->cam_ccb_listptr;
    for (i=0; i < en_lun->cam_ccb_listcnt; i++) 
    {
	/*
	 * Make sure that the CCB has a function code of
	 * XPT_SCSI_IO.
	 */
	if (ccb_list[i]->cam_func_code != XPT_SCSI_IO) 
      	{
	    return(CAM_REQ_INVALID);
	}

	/*
	 * Make sure that the CDB and the CDB length are valid.
	 */
	if (ccb_list[i]->cam_flags & CAM_CDB_POINTER) 
	    cmd = ((CCB_SCSIIO *)ccb_list[i])->cam_cdb_io.cam_cdb_ptr[0];
	else
	    cmd = ((CCB_SCSIIO *)ccb_list[i])->cam_cdb_io.cam_cdb_bytes[0];

	if (targ_validate_cdb(cmd, ((CCB_SCSIIO *)ccb_list[i])->cam_cdb_len) 
	    != CAM_REQ_CMP) 
	{
	    	return(CAM_REQ_INVALID);
	}

	/*
	 * Check for a request sense and inquiry CDB.  Both must
	 * be present for the enable lun to succeed.
	 */
	if (cmd == ALL_INQ_OP)
	    inq_found = 1;
	else if (cmd == ALL_REQ_SENSE6_OP)
	    reqsns_found = 1;
    }

    /*
     * Make sure that both an inquiry CDB and request sense CDB are present.
     */
    if (!inq_found || !reqsns_found) 
    {
	return(CAM_REQ_INVALID);
    }

    /*
     * Raise IPL and lock on the SIM_SOFTC
     */
    SIM_SOFTC_LOCK(s, sc);

    /*
     * If this NEXUS has been setup with default target CCB's,
     * release them before proceeding.
     */
    if (nexus->flags & SZ_TARG_DEF) 
    {
	targ_release_default(sc, nexus);
    }

    /*
     * Set the SZ_PROCESSOR flag.
     */
    nexus->flags |= SZ_PROCESSOR;

    /*
     * Add the CCB's to the NEXUS list.
     */
    ccb_list = (CCB_HEADER **)en_lun->cam_ccb_listptr;
    for (i=0; i < en_lun->cam_ccb_listcnt; i++) 
    {

	CCB_SCSIIO *ccb;

	/*
	 * Ensure that the ccb has the same bus, target, and lun as the 
	 * enable LUN CCB.
	 */
	ccb_list[i]->cam_path_id = ccb_hdr->cam_path_id;
	ccb_list[i]->cam_target_id = ccb_hdr->cam_target_id;
	ccb_list[i]->cam_target_lun = ccb_hdr->cam_target_lun;
	
	ccb = (CCB_SCSIIO *)ccb_list[i];
	/*
	 * Make sure that the CAM_TGT_CCB_AVAIL flag is set.
	 */
	ccb_list[i]->cam_flags |= CAM_TGT_CCB_AVAIL;

	/*
	 * Call xpt_action() to add it to the NEXUS.
	 */
	if (xpt_action(ccb_list[i]) != CAM_REQ_INPROG) 
	{
	    /*
	     * Unlock the SIM_SOFTC.
	     */
	    SIM_SOFTC_UNLOCK(s, sc);
	    return(CAM_PROVIDE_FAIL);
	}

	/*
	 * Set-up the necessary DME resources by calling DME_SETUP().
	 * Check to be sure that a DME setup hasn't already been
	 * performed.  It's possible that this request was rescheduled.
	 */
	sws = SC_GET_WS(ccb_list[i]);
	if (DME_SETUP(sc, sws, (U32) sws->ccb->cam_dxfer_len,
			  (u_char *) sws->ccb->cam_data_ptr,
			  sws->cam_flags, &sws->data_xfer) != CAM_REQ_CMP) {
		
		/*
	 	 * Unlock the SIM_SOFTC.
	 	 */
		SIM_SOFTC_UNLOCK(s, sc);
		return(CAM_PROVIDE_FAIL);
	}

    }		/* For */

    /*
     * Unlock the SIM_SOFTC.
     */
    SIM_SOFTC_UNLOCK(s, sc);

    /*
     * The enable LUN has succeded.
     */
    return(CAM_REQ_CMP);
}

/**
 * sx_async
 * 
 * FUNCTIONAL DESCRIPTION: 
 *
 * Sx_async() will notify the XPT of an asynchronous event.
 *
 * FORMAL PARAMETERS:    opcode  --  type of event
 *                       pathid
 *                       target id
 *                       lun
 *                       bufptr
 *                       cnt
 *
 * RETURN VALUE:         none
 *
 **/
void
sx_async(opcode, pathid, targid, lun, buf, cnt)
U32 opcode, pathid, targid, lun, cnt;
u_char *buf;
{
    SIM_MODULE(sx_async);
    XPT_ASYNCH_CALLBACK(opcode, pathid, targid, lun, buf, cnt);
}

/**
 * sx_alloc
 * 
 * FUNCTIONAL DESCRIPTION: 
 *
 * Sx_alloc() will allocate the ATA list structures.
 *
 * FORMAL PARAMETERS:    sim_softc - SIM_SOFTC pointer
 *
 * RETURN VALUE:         none
 *
 **/
static void
sx_alloc(sim_softc)
register SIM_SOFTC *sim_softc;
{
    SIM_MODULE(sx_alloc);

    sim_softc->ata = (TAG_ELEMENT *)
	cam_zalloc((U32)(sim_softc->max_tag * sizeof(TAG_ELEMENT)));

    sim_softc->ata_list = (TAG_ELEMENT *)cam_zalloc((U32)sizeof(TAG_ELEMENT));

}

/**
 * sx_error_log -
 *
 * FUNCTIONAL DESCRIPTION:  Log errors that cannot be returned in any other way.
 *
 * FORMAL PARAMETERS:     TBD
 *
 * IMPLICIT INPUTS:       none.
 *
 * IMPLICIT OUTPUTS:      none.
 *
 * RETURN VALUE:          CAM_REQ_CMP - All is well
 *
 *   SIDE EFFECTS:        none.
 *
 *  ADDITIONAL INFORMATION:         Callers - SIM - SIMX
 *
 **/
U32 sx_error_log(sim_ws)

    register SIM_WS *sim_ws;
{
    U32 retval;		/* Routine return values */
    SIM_MODULE(sx_error_log);
    
    SIM_PRINTD(sim_ws->ccb->cam_ch.cam_path_id,sim_ws->targid,sim_ws->lun,
	       CAMD_INOUT,
	       ("sim_ws = 0x%x.\n", sim_ws));

    retval = CAM_REQ_CMP;
    return(retval);
};			/* sx_log_error */


/**
 * sx_log_event -
 *
 * FUNCTIONAL DESCRIPTION: For debug, trace selected events.
 *
 * FORMAL PARAMETERS:     none.
 *
 * IMPLICIT INPUTS:       none.
 *
 * IMPLICIT OUTPUTS:      none.
 * 
 * RETURN VALUE:          CAM_REQ_CMP - All is well
 *
 * SIDE EFFECTS:          none.
 *
 *  ADDITIONAL INFORMATION:         Callers - SIM - SIMX
 *
 **/
U32 
sx_log_error()

{
    SIM_MODULE(sx_log_error);
    SIM_PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	       ("Log Error: \n"));
    return(CAM_REQ_CMP);
};			/* sx_log_error */

/*
 * Release any DME resources.  Check the "segment" for each DME
 * descriptor.  If NULL, no resources have been allocated for it.
 */
static void
sx_dme_cleanup(sws)
SIM_WS *sws;
{
    SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    SIM_MODULE(sx_dme_cleanup);

    if (sws->msgin_xfer.segment != (SEGMENT_ELEMENT *)NULL)
	(void)DME_END(sc, &sws->msgin_xfer);
    if (sws->msgout_xfer.segment != (SEGMENT_ELEMENT *)NULL)
	(void)DME_END(sc, &sws->msgout_xfer);
    if (sws->command_xfer.segment != (SEGMENT_ELEMENT *)NULL)
	(void)DME_END(sc, &sws->command_xfer);
    if (sws->data_xfer.segment != (SEGMENT_ELEMENT *)NULL)
	(void)DME_END(sc, &sws->data_xfer);
    if (sws->status_xfer.segment != (SEGMENT_ELEMENT *)NULL)
	(void)DME_END(sc, &sws->status_xfer);
}

void
sim94_reset(controller)
    short controller;
{
    register SIM_SOFTC *sc = (SIM_SOFTC *)SC_GET_SOFTC(controller);

    if (sc == (SIM_SOFTC *)NULL )
    {
        return;
    }

    (sc->hba_chip_reset)(sc);
    SC_HBA_BUS_RESET(sc);
}
