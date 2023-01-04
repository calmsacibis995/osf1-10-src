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
 *
 * Log History before file was renamed to skz_main.c
 *
 * Revision 1.1.2.12  92/06/26  13:59:35  Ronald_Hegli
 * 	Changed to reflect CAM header files moving from sys to io/cam.
 * 	[92/06/26  13:51:47  Ronald_Hegli]
 * 
 * Revision 1.1.2.11  92/06/26  11:51:33  Ronald_Hegli
 * 	Attempt #2 to submit 1.1.2.10
 * 	[92/06/26  11:49:49  Ronald_Hegli]
 * 
 * Revision 1.1.2.10  92/06/19  01:45:13  Ronald_Hegli
 * 	Improved debug #ifdefs, added arguments to debug routines.
 * 	[92/06/18  16:08:56  Ronald_Hegli]
 * 
 * Revision 1.1.2.9  92/06/18  15:47:48  Ronald_Hegli
 * 	Changes to work correctly with OSF Pal ipls.
 * 	Changed to allow booting and bus scanning at high ipl.
 * 	[92/06/18  15:37:03  Ronald_Hegli]
 * 
 * Revision 1.1.2.8  92/06/07  10:41:01  Ronald_Hegli
 * 	Incorporated code review comments.
 * 	Made improvements to bus reset code.
 * 	Numerous small improvements.
 * 	[92/06/07  10:29:13  Ronald_Hegli]
 * 
 * Revision 1.1.2.7  92/05/21  17:21:48  Joseph_Amato
 * 	drop the command field from the mailbox interface
 * 	[92/05/21  17:09:57  Joseph_Amato]
 * 
 * Revision 1.1.2.6  92/04/29  14:58:53  Ronald_Hegli
 * 	Error logging changes, some cleanup
 * 	[92/04/29  14:54:51  Ronald_Hegli]
 * 
 * Revision 1.1.2.5  92/04/06  16:12:35  Ronald_Heglie
 * 	Fixed problem in XZA bug workaround for luns > 0.
 * 	Improved bus reset code on bus wedged errors under load.
 * 	[92/04/06  16:09:32  Ronald_Heglie]
 * 
 * Revision 1.1.2.4  92/04/03  16:04:17  Ronald_Heglie
 * 	#ifdef'ed out structure dumps on errors.
 * 	[92/04/03  16:00:51  Ronald_Heglie]
 * 
 * Revision 1.1.2.3  92/04/01  20:13:42  Ronald_Heglie
 * 	Added workaround for XZA firmware bug when lun <> 0
 * 	[92/04/01  20:05:07  Ronald_Heglie]
 * 
 * Revision 1.1.2.2  92/04/01  00:02:47  Ronald_Heglie
 * 	Initial submission for LASER/Ruby XZA/SCSI support.
 * 	[92/03/31  23:40:07  Ronald_Heglie]
 */
#ifndef lint
static char *rcsid = "@(#)$RCSfile: skz_main.c,v $ $Revision: 1.1.15.2 $ (DEC) $Date: 1993/11/17 22:41:21 $";
#endif


/************************************************************************
 *									*
 * File:	skz_main.c						*
 * Date:	November 10, 1991					*
 * Author:	Ron Hegli                                               *
 *									*
 * Description:								*
 *	This file contains all of the SKZ module routines, comprising   *
 *	the heart of the XZA/SCSI "skz" driver				*
 *									*
 ************************************************************************/

/* #define SKZ_DUMP */
/* #define SKZ_DUMP_ON_ERROR */

/*
** Include Files
*/
#include <sys/types.h>
#include <sys/lock.h>
#include <sys/time.h>
#include <sys/proc.h>
#include <sys/buf.h>

#include <vm/vm_kern.h>
#include <mach/vm_param.h>
#include <mach/kern_return.h>
#include <kern/sched_prim.h>
#include <machine/rpb.h>
#include <machine/machparam.h>

#include <kern/thread.h>
#include <kern/lock.h>

#include <io/dec/mbox/mbox.h>
#include <io/common/devdriver.h>
#include <io/dec/xmi/xmireg.h>

#include <io/common/iotypes.h>

#include <io/cam/cam.h>
#include <io/cam/dec_cam.h>
#include <io/cam/scsi_status.h>
#include <io/cam/scsi_all.h>
#include <io/cam/cam_debug.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/dme.h>
#include <io/cam/sim.h>
#include <io/cam/sim_common.h>

#include <io/cam/xza/skz_params.h>
#include <io/cam/xza/skz_error.h>
#include <io/cam/xza/skz_xza.h>
#include <io/cam/xza/skz_nport.h>
#include <io/cam/xza/skz_xza_qb.h>
#include <io/cam/xza/skz.h>

extern I32	sim_default_timeout;

void skz_process_resp();
void skz_process_misc();
void skz_bus_reset_complete();
void skz_clear_command_queues();
void skz_release_asr();

extern u_int skz_ss_start();
extern u_int ss_sched();
extern u_int skz_ss_abort();
extern u_int skz_ss_termio();
extern u_int skz_ss_device_reset();

extern int shutting_down;

/************************************************************************
 *
 *  ROUTINE NAME:  skz_hba_init()
 *
 *  FUNCTIONAL DESCRIPTION: This routine is CAM's entry point to initialize
 * 	the XZA.  It calls the internal skz_init() function to do the
 *	actual initialization.  This is necessary because some XZA errors
 *	require that the adapter be completely reinitialized, which is
 *	performed by error recovery code which has no sim_softc to reference.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

skz_hba_init ( SIM_SOFTC* sim_softc, vm_offset_t csr )

{
    u_int		skz_status;
    u_int		s;

    XZA_SOFTC*		xza_softc;
    XZA_CHANNEL*	xza_chan;

    SIM_MODULE(skz_hba_init);

    /*
    ** Initialize the XZA-specific scheduler functions
    */
    sim_softc -> sched_start = skz_ss_start;
    sim_softc -> sched_run_sm = ss_sched;
    sim_softc -> sched_abort = skz_ss_abort;
    sim_softc -> sched_termio = skz_ss_termio;
    sim_softc -> sched_bdr = skz_ss_device_reset;

    /*
    ** Get the xza_softc structure and check to see if we've been here
    ** already ( that is, due to simattach'ing one channel, we've 
    ** already completely brought the adapter up to enabled, both channels,
    ** and so do not need to go through this again
    */
    xza_chan = (XZA_CHANNEL *) sim_softc -> hba_sc;
    xza_softc = (XZA_SOFTC *) xza_chan -> xza_softc;

    SKZ_LOCK ( xza_softc -> lock, s );

    if ( ( ( xza_softc -> channel[0].state == ENABLED ) ||
           !( xza_softc -> channel[0].flags & SKZ_ALIVE ) ) &&
	 ( ( xza_softc -> channel[1].state == ENABLED ) ||
	   !( xza_softc -> channel[1].flags & SKZ_ALIVE ) ) &&
	 ( xza_softc -> flags & SKZ_PROBE ) )
    {
	SKZ_UNLOCK ( xza_softc -> lock, s );
	return ( CAM_REQ_CMP );
    }

    SKZ_UNLOCK ( xza_softc -> lock, s );

    /*
    ** Do the real initialization work
    */
    skz_status = skz_init ( xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error (	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
	return ( CAM_REQ_CMP_ERR );
    }

    return ( CAM_REQ_CMP );

}

/************************************************************************
 *
 *  ROUTINE NAME:  skz_init()
 *
 *  FUNCTIONAL DESCRIPTION: 
 *	This routine initializes the adapter.  It brings both channels to
 *	the uninitialized state.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	CAM_REQ_CMP
 *	SKZ_FAILURE
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

skz_init ( XZA_SOFTC* xza_softc )

{
    u_int		skz_status;
    u_int		s;

    XPD1_REG		xpd1;
    XPD2_REG		xpd2;

    SIM_SOFTC*		sim_softc;

    u_int		chan_mask;

    SIM_MODULE(skz_init);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nEntering %s", module) );

    /*
    ** don't recurse
    */
    if ( ( xza_softc -> flags & SKZ_ADAPT_RESET_IN_PROGRESS ) &&
	  !(xza_softc -> flags & SKZ_PROBE) )
	return ( CAM_REQ_CMP );
    else
	xza_softc -> flags |= SKZ_ADAPT_RESET_IN_PROGRESS;

    /*
    ** Bring the adapter to the UNINITIALIZED state
    */
    skz_status = skz_halt_adapter ( xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error ( 	module,
			SKZ_ERROR, 
			skz_status,
			NULL,
			xza_softc );
	return ( SKZ_INIT_ERROR );
    }

    /*
    ** Zero out all of the various pools and adapter structures
    ** These were allocated during 'attach' time
    */
    SKZ_LOCK ( xza_softc -> lock, s );

    bzero ( xza_softc -> np_qb_pool, 
	    SKZ_QB_POOL_ENTRIES * sizeof ( QB ) ); /* queue buffer pool */
    bzero ( xza_softc -> np_carrier_pool, 
	    SKZ_CAR_POOL_ENTRIES * sizeof ( CARRIER ) ); /* carrier pool */
    bzero ( xza_softc -> ab, sizeof ( AB ) );	/* adapter block */

    SKZ_UNLOCK ( xza_softc -> lock, s );

    /*
    ** Initialize the Adapter Block
    */

    /*
    ** Initialize N_Port Queues
    */
    skz_status = skz_init_queues ( xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error ( 	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
	return ( SKZ_INIT_ERROR );
    }


    SKZ_LOCK ( xza_softc -> lock, s );

    /*
    ** Q Buffer length
    */
    xza_softc -> ab -> qe_len = SKZ_XZA_QE_LEN;

    /*
    ** Initialize interrupt vector fields in the AB
    */
    if ( !(cam_at_boottime()) && !shutting_down )
    {
        xza_softc -> ab -> adrq_intr_data.level = DEV_LEVEL_0;
        xza_softc -> ab -> adrq_intr_data.slot = xza_softc -> lamb_xmi_node;
        xza_softc -> ab -> adrq_intr_data.vector = 
				xza_softc -> resp_intr_vector >> 4;

	xza_softc -> ab -> c0_misc_intr_data.level = DEV_LEVEL_1;
	xza_softc -> ab -> c0_misc_intr_data.slot = xza_softc -> lamb_xmi_node;
	xza_softc -> ab -> c0_misc_intr_data.vector = 
				xza_softc -> c0_misc_intr_vector >> 4;

	xza_softc -> ab -> c1_misc_intr_data.level = DEV_LEVEL_1;
	xza_softc -> ab -> c1_misc_intr_data.slot = xza_softc -> lamb_xmi_node;
	xza_softc -> ab -> c1_misc_intr_data.vector = 
				xza_softc -> c1_misc_intr_vector >> 4;
    }

    chan_mask = 0; 

    if ( xza_softc -> channel[0].flags & SKZ_ALIVE )
	chan_mask |= XZA_CHAN_0;

    if ( xza_softc -> channel[1].flags & SKZ_ALIVE )
	chan_mask |= XZA_CHAN_1;

    SKZ_UNLOCK ( xza_softc -> lock, s );

    /*
    ** Bring adapter to the DISABLED state
    */
    skz_status = skz_disable_adapter ( xza_softc, chan_mask );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error ( 	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
        return ( SKZ_INIT_ERROR );
    }

    /*
    ** Initialize the AMPB
    */
    skz_status = skz_init_ampb ( xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error ( 	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
	return ( SKZ_INIT_ERROR );
    }


    /*
    ** Enable DQE miscellaneous interrupt by writing to the AMCSR
    */
    SIM_PRINTD (	NOBTL,
			NOBTL,
			NOBTL,
			CAMD_FLOW,
			("\n%s: Enabling DQE Interrupts", module) );

    bzero ( &xpd1, sizeof ( XPD1_REG ) );
    bzero ( &xpd2, sizeof ( XPD2_REG ) );

    xpd1.rcode = AMCSR_EMULATION;
    xpd1.c0 = 1;
    xpd1.c1 = 1;

    xpd2.xpd2_un.amcsr.dqe = 1;

    skz_status = skz_write_reg ( AMCSR, xpd1, xpd2, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error ( 	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
        return ( SKZ_INIT_ERROR );
    }

    /*
    ** Other xza_softc initializations
    **
    ** Get SCSI ID's
    */
    xza_softc -> channel[0].scsi_id.targid = 
		(unsigned char) xza_softc -> ab -> cpb[0].xp_addr;
    xza_softc -> channel[0].scsi_id.lun = 0;

    xza_softc -> channel[1].scsi_id.targid = 
		(unsigned char) xza_softc -> ab -> cpb[1].xp_addr;
    xza_softc -> channel[1].scsi_id.lun = 0;

    /*
    ** Copy scsi id's to sim_softc
    */
    if ( xza_softc -> channel[0].flags & SKZ_ALIVE )
    {
	sim_softc = xza_softc -> channel[0].sim_softc;
	sim_softc -> scsiid = xza_softc -> channel[0].scsi_id.targid;
    }

    if ( xza_softc -> channel[1].flags & SKZ_ALIVE )
    {
	sim_softc = xza_softc -> channel[1].sim_softc;
	sim_softc -> scsiid = xza_softc -> channel[1].scsi_id.targid;
    }

    /*
    ** Bring adapter to the ENABLED state
    */
    skz_status = skz_enable_channel ( xza_softc, chan_mask );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error ( 	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
        return ( SKZ_INIT_ERROR );
    }

    /*
    ** Clear reset in prog flag
    */
    xza_softc -> flags &= ~SKZ_ADAPT_RESET_IN_PROGRESS;
    xza_softc -> flags &= ~SKZ_RESET_NEEDED;


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nLeaving %s", module) );

    return ( CAM_REQ_CMP );

}



/************************************************************************
 *
 *  ROUTINE NAME:  skz_init_ampb()
 *
 *  FUNCTIONAL DESCRIPTION:  The XZA requests Adapter Memory Pages when
 *	the adapter is brought up to the DISABLED state.  This routine
 *	allocates them and sets up the Adapter Memory Pointers for them.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION: This routine assumes that the adapter's number
 *			    of requested pages will not change while the
 *			    system is booted ( pretty safe! )
 *
 ************************************************************************/

skz_init_ampb ( XZA_SOFTC* xza_softc )

{
    u_int		skz_status = CAM_REQ_CMP;
    u_int		s;
    u_int		i;

    kern_return_t	kern_status;
    vm_offset_t		phys_addr = 0;

    AMP_PTR*		ampb_ptr;
    vm_offset_t		amp_ptr;

    SIM_MODULE(skz_init_ampb);


    SIM_PRINTD (	NOBTL,
			NOBTL,
			NOBTL,
			CAMD_INOUT,
			("\nEntering %s", module) );

    /*
    ** Allocate/Init the Adapter Memory Block
    */

    /*
    ** If we're here for the second or later time, just
    ** initialize the memory, don't free/realloc it
    */
    if ( (xza_softc -> ampb_ptr) && (xza_softc -> cqib) )
    {
	bzero ( xza_softc -> ampb_ptr, 
		xza_softc -> ampb_size * sizeof ( AMP_PTR ) );

	bzero ( xza_softc -> cqib,
		xza_softc -> ampb_size * PAGE_SIZE );

	/* 
	** Initialize the length field in the AB, since we only get here
	** after the AB has been bzero'ed
	*/
	xza_softc -> ab -> ampb_len = xza_softc -> ampb_size * PAGE_SIZE;
    }
    else
    {
	/*
	** How many pages do we need?
	** Get the length from the APB
	*/
 	/*
	** Save the size in xza_softc in case we get re-initialized
	*/
	xza_softc -> ampb_size = xza_softc -> ab -> apb.rampb;

	/*
	** Set length field in AB
	*/
        xza_softc -> ab -> ampb_len = xza_softc -> ampb_size * PAGE_SIZE;

	/*
	** Allocate and reference the AMPB and the Requested AMP's
	*/

	/* AMPB */
	xza_softc -> ampb_ptr = (vm_offset_t) kmem_alloc ( kernel_map, 
				xza_softc -> ampb_size * sizeof ( AMP_PTR ) );
        if ( xza_softc -> ampb_ptr == NULL )
	    return ( SKZ_MEM_ALLOC_ERR );

        bzero ( xza_softc -> ampb_ptr, 
		xza_softc -> ampb_size * sizeof ( AMP_PTR ) );

	/* AMP's */
        xza_softc -> cqib = (CQIB *) kmem_alloc ( kernel_map,
			 		xza_softc -> ampb_size * PAGE_SIZE );
	if ( xza_softc -> cqib == NULL )
	    return ( SKZ_MEM_ALLOC_ERR );

	bzero ( xza_softc -> cqib, 
		xza_softc -> ampb_size * PAGE_SIZE );
    }


    /*
    ** Find physical address of AMPB
    */
    kern_status = kvtop ( xza_softc -> ampb_ptr, &phys_addr );
    if ( kern_status != KERN_SUCCESS )
	return ( SKZ_PHYS_CONV_ERR );

    /*
    ** Initialize the ampb_base field in the adapter block
    */
    xza_softc -> ab -> ampb_base.pa = phys_addr >> 5;
    xza_softc -> ab -> ampb_base.pax = phys_addr >> 32;
    xza_softc -> ab -> ampb_base.v = 1;

    /*
    ** Initialize the adapter memory page(s) requested by the adapter
    */
    amp_ptr = (vm_offset_t) xza_softc -> cqib;
    ampb_ptr = (AMP_PTR *) xza_softc -> ampb_ptr; 

    for ( i = 0; i < xza_softc -> ab -> apb.rampb; i++ )
    {
	kern_status = kvtop ( amp_ptr, &phys_addr );
	if ( kern_status != KERN_SUCCESS )
	    return ( SKZ_PHYS_CONV_ERR );

	ampb_ptr -> pa = phys_addr >> 13;
	ampb_ptr -> pax = phys_addr >> 32;
	ampb_ptr -> v = 1;

        ampb_ptr++;
	amp_ptr += PAGE_SIZE;

    }

    /*
    ** Initialize the queue stopper addresses in the CQIB
    */
    SKZ_LOCK ( xza_softc -> lock, s );

    for ( i = 0; i < XZA_CHANNELS; i++ )
    {
        /*
        ** Initialize stopper for channel command queue 2
        */
	xza_softc -> cqib -> qs[i].ccq2ir = 
		(vm_offset_t) xza_softc -> ab -> dccq[i].dccq2.head_ptr >> 5;

        /*
        ** Initialize stopper for channel command queue 1
        */
	xza_softc -> cqib -> qs[i].ccq1ir = 
		(vm_offset_t) xza_softc -> ab -> dccq[i].dccq1.head_ptr >> 5;
    }

    /*
    ** Initialize the stopper address for the adapter free queue 
    */
    xza_softc -> cqib -> afqir = 
		(vm_offset_t) xza_softc -> ab -> dafq.head_ptr >> 5;

    SKZ_UNLOCK ( xza_softc -> lock, s );

    SIM_PRINTD (	NOBTL,
			NOBTL,
			NOBTL,
			CAMD_INOUT,
			("\nEntering %s", module) );

    return ( skz_status );
}




/************************************************************************
 *
 *  ROUTINE NAME:  skz_init_queues()
 *
 *  FUNCTIONAL DESCRIPTION:  This routine initializes all of the 
 *	N_Port queues used by the driver and the adapter and inserts
 *	the memory allocated for Q Buffers and Carriers onto the free
 *	queues.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

skz_init_queues ( XZA_SOFTC* xza_softc )

{
    u_int		i;
    u_int		skz_status = CAM_REQ_CMP;

    QB*			qb_ptr;
    CARRIER*		car_ptr;

    SIM_MODULE(skz_init_queues);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_FLOW,
		 ("\nEntering %s", module) );

    /*
    ** Initialize all N_Port queues
    */
    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_FLOW,
		 ("\n%s: Initializing N_Port Queues", module) );

    car_ptr = (CARRIER *) xza_softc -> np_carrier_pool;
    qb_ptr = (QB *) xza_softc -> np_qb_pool;

    for ( i = 0; i < XZA_CHANNELS; i++ )
    {
        np_init_queue ( 	DRIVER_ADAPTER,
				&(xza_softc -> ab -> dccq[i].dccq2),
				car_ptr++ );

        np_init_queue ( 	DRIVER_ADAPTER,
				&(xza_softc -> ab -> dccq[i].dccq1),
				car_ptr++ );

        np_init_queue ( 	DRIVER_ADAPTER,
				&(xza_softc -> ab -> dccq[i].dccq0),
				car_ptr++ );

        np_init_queue ( 	DRIVER_ADAPTER,
				&(xza_softc -> ab -> dccq[i].cccq3),
				car_ptr++ );

        np_init_queue ( 	DRIVER_ADAPTER,
				&(xza_softc -> ab -> dccq[i].cccq2),
				car_ptr++ );

        np_init_queue ( 	DRIVER_ADAPTER,
				&(xza_softc -> ab -> dccq[i].cccq1),
				car_ptr++ );

        np_init_queue ( 	DRIVER_ADAPTER,
				&(xza_softc -> ab -> dccq[i].cccq0),
				car_ptr++ );
    }


    np_init_queue ( 	ADAPTER_DRIVER,
			&(xza_softc -> ab -> adrq),
			car_ptr++ );

    np_init_queue ( 	DRIVER_ADAPTER,
			&(xza_softc -> ab -> dafq),
			car_ptr++ );

    np_init_queue ( 	ADAPTER_DRIVER,
			&(xza_softc -> ab -> adfq),
			car_ptr++ );

    np_init_queue ( 	ADAPTER_ADAPTER,
			&(xza_softc -> ab -> aafq),
			car_ptr++ );

    np_init_queue ( 	DRIVER_DRIVER,
			&(xza_softc -> ddfq),
			car_ptr++ );

    /*
    ** Put queue entries onto the free queues
    */
    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_FLOW,
		 ("\n%s: Inserting Free Queue Entries", module) );

    /* driver-adapter free queue */
    for ( i = 0; i < SKZ_ADFQ_ENTRIES; i++ )
    {
	np_insert_da_q ( &(xza_softc -> ab -> dafq.tail_ptr),
			 car_ptr++,
			 qb_ptr++ );
    }

    /* driver-driver free queue */
    for ( i = SKZ_ADFQ_ENTRIES; i < SKZ_QB_POOL_ENTRIES; i++ )
    {
	np_insert_dd_q ( &(xza_softc -> ddfq.tail_ptr),
			 car_ptr++,
			 qb_ptr++ );
    }


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_FLOW,
		 ("\nLeaving %s", module) );

    return ( CAM_REQ_CMP );
}

/************************************************************************
 *
 *  ROUTINE NAME:  skz_go()
 *
 *  FUNCTIONAL DESCRIPTION:
 * 	This function allocates a Q_Buffer and initializes it for 
 * 	a SCSI data transfer.
 *
 *	This is the HBA_GO routine
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

unsigned int
skz_go ( SIM_WS* sws )

{
    u_int		skz_status;
    u_int		s;

    u_int		delay;

    vm_offset_t		phys_addr;
    vm_offset_t		virt_addr;
    kern_return_t	kern_status;

    QB*			qb_ptr;
    SCSI_QB_HEADER*	qb_hdr_ptr;
    SNDSCSI_CMD*	qb_body_ptr;
    QB_DRIVER*		qb_driver_ptr;

    CARRIER* 		car_ptr;

    XZA_CHANNEL*	xza_chan;
    XZA_SOFTC*		xza_softc;
    SKZ_DME_DSC*        dme_dsc_ptr;

    SIM_MODULE(skz_go);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nEntering %s", module) );

    /*
    ** setup
    */
    xza_chan = (XZA_CHANNEL *) sws -> sim_sc -> hba_sc;
    xza_softc = (XZA_SOFTC *) xza_chan -> xza_softc;

    /*
    ** Is this a normal I/O or is it directed at the
    ** adapter itself ( i.e. a diagnostic function ) ?
    */
    if ( sws -> targid == xza_chan -> scsi_id.targid )
    {
	skz_status = skz_diag_command ( sws );
	if ( skz_status == CAM_REQ_CMP )
	    return ( CAM_REQ_CMP );
    }

    /*
    ** It's a normal I/O...
    */

    /*
    ** Lock the xza_softc structure ????
    */
    SKZ_LOCK ( xza_softc -> lock, s );

    /*
    ** All Clear?
    */
    if ( 
	 /*
	 ** If not enabled, we can't process I/O
	 */
	 ( xza_chan -> state != ENABLED ) ||
	 /*
	 ** If we're waiting for the error thread to do
	 ** some re-initialization, we shouldn't go any further
	 */
	 ( xza_chan -> flags & SKZ_RESET_NEEDED ) ||
	 ( xza_softc -> flags & SKZ_RESET_NEEDED ) ||
	 /*
	 ** If we're in the middle of re-initializing the adapter
	 ** of this particular bus, stop here
	 */
	 ( xza_chan -> flags & SKZ_BUS_RESET_IN_PROGRESS ) ||
	 ( xza_softc -> flags & SKZ_ADAPT_RESET_IN_PROGRESS ) )
    {
	/* 
	** Unlock and return CAM_BUSY
	*/
        SKZ_UNLOCK ( xza_softc -> lock, s );

	return ( CAM_BUSY );
    }

    SKZ_UNLOCK ( xza_softc -> lock, s );

    /*
    ** No support for linked commands
    */
    if ( sws -> cam_flags & CAM_CDB_LINKED )
	return ( CAM_PROVIDE_FAIL );

    /*
    ** Get a Q_Buffer from the free queue
    */
    skz_status = skz_get_queue ( DDFQ, &qb_ptr, &car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error (	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
	return ( CAM_PROVIDE_FAIL );
    }

    qb_hdr_ptr = (SCSI_QB_HEADER *) &( qb_ptr -> qb_header );
    qb_body_ptr = (SNDSCSI_CMD *) ( qb_ptr -> qb_body );
    qb_driver_ptr = (QB_DRIVER *) &( qb_ptr -> qb_driver );

    dme_dsc_ptr = &(qb_driver_ptr -> dme_dsc);

    /*
    ** Initialize Q_Buffer fields
    */
    qb_hdr_ptr -> opc = SNDPM;
    qb_hdr_ptr -> chnl_idx = xza_chan -> chan_num;
    qb_hdr_ptr -> flags.r = 1;			/* always get response */
    qb_hdr_ptr -> scsi_opc = SNDSCSI;
    qb_hdr_ptr -> dst_xport.targid = sws -> targid;
    qb_hdr_ptr -> dst_xport.lun = sws -> lun;
    qb_hdr_ptr -> src_xport.targid = xza_chan -> scsi_id.targid;

    /*
    ** If it's boot time and we can't take interrupts, 
    ** set the I bit in the flags field to suppress the
    ** response interrupt for this Q Buffer
    */
/*
** Turns out that this isn't implemented in firmware yet
**
    if ( cold )
	qb_hdr_ptr -> flags.i = 1;
*/

 
    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\n%s: Initialized qb_hdr_ptr fields", module) );

    qb_body_ptr -> xct_id = sws -> seq_num;

    /* 
    ** Check for autosense and synchronous transfer negotiation
    */
    if ( sws -> cam_flags & CAM_DIS_AUTOSENSE )
    {
	qb_hdr_ptr -> scsi_flags.as = 0;
    }
    else
    {
	/* set autosense bit */
	qb_hdr_ptr -> scsi_flags.as = 1;

	/*
	** setup autosense buffer and the pointer and length fields
	** We're assuming that the actual buffer is allocated
	** at the pdrv level..
	*/
        /* must be hexaword aligned */
	if ( sws -> cam_flags & CAM_SNS_BUF_PHYS )
	    bcopy ( &(sws -> ccb -> cam_sense_ptr),
		    qb_body_ptr -> autosense_ptr, 
		    sizeof ( qb_body_ptr -> autosense_ptr ) );
	else
 	{
	    /*
	    ** The autosense buffer is potentially a user space va.
	    ** In case it is, check for it and use pmap_extract to
	    ** extract the physical address if it is
	    */
	    if ( IS_SYS_VA ( sws -> ccb -> cam_sense_ptr ) )
	    {
		/*
		** It's just a kernel virtual address
		*/
	        kern_status = kvtop ( sws -> ccb -> cam_sense_ptr, &phys_addr ); 
	        if ( kern_status != KERN_SUCCESS )
	        {
	            skz_error (	module,
			        SKZ_ERROR,
			        SKZ_PHYS_CONV_ERR,
				NULL,
				xza_softc );
		    /* 
		    ** attempt to put the Q Buffer back on the free queue, and
		    ** ignore the return status for now
		    */
		    skz_status = 
			skz_put_queue ( DDFQ, qb_ptr, car_ptr, xza_softc );

	            return ( CAM_PROVIDE_FAIL );
	        }
	    }
	    else
	    {
		/* 
		** The address is in user space
		*/
                phys_addr = pmap_extract (
                  ((struct buf *) sws -> ccb -> cam_req_map ) -> 
				b_proc -> task -> map -> vm_pmap,
                  	sws -> ccb -> cam_sense_ptr );
                if ( phys_addr == NULL )
		{
	            skz_error (	module,
			        SKZ_ERROR,
			        SKZ_PHYS_CONV_ERR,
				NULL,
				xza_softc );
		    /* 
		    ** attempt to put the Q Buffer back on the free queue, and
		    ** ignore the return status for now
		    */
		    skz_status = 
			skz_put_queue ( DDFQ, qb_ptr, car_ptr, xza_softc );

                    return ( CAM_PROVIDE_FAIL );
		}
	    }

	    bcopy ( &phys_addr, qb_body_ptr -> autosense_ptr, 
			sizeof ( qb_body_ptr -> autosense_ptr ) );
	}

	qb_body_ptr -> as_len = sws -> ccb -> cam_sense_len;
    }

    /* read or write */
    if ( sws -> cam_flags & CAM_DIR_IN )
	qb_hdr_ptr -> scsi_flags.rd = 1;
    else 
	qb_hdr_ptr -> scsi_flags.rd = 0;

    /* queuing */
    if ( sws -> cam_flags & CAM_QUEUE_ENABLE )
    {
	qb_hdr_ptr -> scsi_flags.tq = 1;

	/*
        ** Set type of tag
        */
	qb_body_ptr -> tag_msg = sws -> ccb -> cam_tag_action;
    }
    else
	qb_hdr_ptr -> scsi_flags.tq = 0;

    /* synchronous negotiation */
    if ( sws -> cam_flags & CAM_INITIATE_SYNC )
	qb_hdr_ptr -> scsi_flags.sdtr = 1;
    else
        qb_hdr_ptr -> scsi_flags.sdtr = 0;

    /* disconnects */
    if ( sws -> cam_flags & CAM_DIS_DISCONNECT )
	qb_hdr_ptr -> scsi_flags.disc = 0;
    else
	qb_hdr_ptr -> scsi_flags.disc = 1;

    /*
    ** Set timeout value
    */
    if ( sws -> ccb -> cam_timeout == NULL )
	qb_body_ptr -> cmd_tmo = sim_default_timeout;
    else
	qb_body_ptr -> cmd_tmo = sws -> ccb -> cam_timeout;

    /*
    ** Copy the CDB into the qb
    */
    if ( sws -> cam_flags & CAM_CDB_POINTER )
    {
	if ( sws -> cam_flags & CAM_CDB_PHYS )
	    virt_addr = PHYS_TO_KSEG (sws->ccb->cam_cdb_io.cam_cdb_ptr);
	else
	    virt_addr = ( vm_offset_t ) sws->ccb->cam_cdb_io.cam_cdb_ptr;
    }
    else
    {
	if ( sws -> cam_flags & CAM_CDB_PHYS )
	    virt_addr = PHYS_TO_KSEG (&(sws->ccb->cam_cdb_io.cam_cdb_bytes[0]));
	else
	    virt_addr = (vm_offset_t) &(sws->ccb->cam_cdb_io.cam_cdb_bytes[0]);
    }

    /*
    ** Do the actual bcopy, and set the cdb length
    */
    bcopy ( virt_addr, 
	    qb_body_ptr -> cdb, 
	    sws->ccb->cam_cdb_len );

    qb_body_ptr -> cdb_len = sws->ccb->cam_cdb_len;


    /*
    ** Keep the sws and channel pointers
    */
    qb_driver_ptr -> sws = (vm_offset_t) sws;
    qb_driver_ptr -> xza_chan_ptr = (vm_offset_t) xza_chan;

    /*
    ** Map the data into an XZA compatible format
    */
    if ( (sws -> cam_flags & CAM_DIR_NONE) != CAM_DIR_NONE )
    {
	/*
	** Map the data for transfer
	*/
        skz_status = skz_dme_setup ( sws, dme_dsc_ptr, xza_softc );
        if ( skz_status != CAM_REQ_CMP )
        {
	    skz_error (	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
	    /* 
	    ** attempt to put the Q Buffer back on the free queue, and
	    ** ignore the return status for now
	    */
	    skz_status = skz_put_queue ( DDFQ, qb_ptr, car_ptr, xza_softc );

	    return ( CAM_PROVIDE_FAIL );
        }

	/*
	** Fill in the qb_body information
	*/
	qb_body_ptr -> buf_root_ptr_0 = dme_dsc_ptr -> buf_root_ptr_0;
	qb_body_ptr -> buf_root_ptr_1 = dme_dsc_ptr -> buf_root_ptr_1;
	qb_body_ptr -> xfer_offset = dme_dsc_ptr -> xfer_offset;

	qb_body_ptr -> xfer_len = dme_dsc_ptr -> xfer_len;
	qb_body_ptr -> ret_len = dme_dsc_ptr -> xfer_len;

    }

    /*
    ** Turn off certain scsi options if the XZA firmware rev isn't high
    ** enough.
    */
    if ( xza_softc -> xdev.firm_rev < XZA_MIN_DISCONN_REV )
	qb_hdr_ptr -> scsi_flags.disc = 0;

    if ( xza_softc -> xdev.firm_rev < XZA_MIN_SDTR_REV )
	qb_hdr_ptr -> scsi_flags.sdtr = 0;

#ifdef SKZ_CAM_AUTOSENSE

    qb_hdr_ptr -> scsi_flags.as = 0;

#else

    if ( xza_softc -> xdev.firm_rev < XZA_MIN_AS_REV )
	qb_hdr_ptr -> scsi_flags.as = 0;

#endif


#ifdef SKZ_DUMP
    printf ( "\n\nBefore issuing command\n" );
    skz_dump_scsi_qb_hdr ( qb_hdr_ptr );
    skz_dump_scsi_qb_body ( qb_body_ptr, qb_ptr );
#endif

    /*
    ** Issue command by inserting onto the adapter's command queue
    */
    skz_status = skz_put_queue ( DCCQ0, qb_ptr, car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error (	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
	/* 
	** attempt to put the Q Buffer back on the free queue, and
	** ignore the return status for now
	*/
	skz_status = skz_put_queue ( DDFQ, qb_ptr, car_ptr, xza_softc );

	return ( CAM_PROVIDE_FAIL );
    }

    /*
    ** If it's not boot time or a shutdown, just return and the 
    ** response interrupt will handle the completion of the command
    ** If it *is* boot time or we're shutting down, we need to poll until 
    ** we get the response, since we won't get interrupts
    */
    if ( cam_at_boottime() || shutting_down )
    {
	delay = 0;

	/*
	** Ensure that threads are not enabled, important
	** during shutdowns
	*/
	xza_softc -> flags &= ~( SKZ_ERROR_TH | SKZ_RESP_TH );
	xza_chan -> flags &= ~SKZ_THREADS_ACTIVE;

	/*
	** Wait for the response to appear on the response queue
	*/	
	while ( (delay < XZA_RESPONSE_TIME) &&
		(np_drv_q_entry_present ( &(xza_softc -> ab -> adrq) )
			!= NP_SUCCESS) )
	{
	    /*
	    ** If an event occurred, process it
	    ** and return, because there'll be no "response"
	    ** if any of these events occurrs.
	    */
	    skz_process_misc ( xza_chan );

	    DELAY ( SKZ_ONE_SECOND_DELAY / 1000 );
	    delay++;
	}

	/*
	** Just attempt to process anything that might be there now,
	** and let skz_process_resp handle status, etc.
	*/
	if (np_drv_q_entry_present (&(xza_softc -> ab -> adrq)) == NP_SUCCESS)
	{
	    skz_process_resp ( xza_softc );
	}
	else
	{
	    /* set status only */
	    sws -> cam_status = CAM_PROVIDE_FAIL;
	}

    } /* if cam_at_bootime() */
#if SKZ_THREADED == SKZ_TH_POST_BOOT
    else if ( !( xza_softc -> flags & SKZ_THREADS_ACTIVE ) )
	skz_thread_init ( xza_softc );
#endif

    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nLeaving %s", module) );

    return ( CAM_REQ_CMP );
}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_command_complete()
 *
 *  FUNCTIONAL DESCRIPTION:
 * 	This routine performs completion work on Q_Buffers returned on
 * 	the response queue and calls upward into the SIMS and SIMX
 * 	layers.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *	Queue Buffer is returned to the free queue
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

void
skz_command_complete ( QB* qb_ptr, CARRIER* car_ptr )

{
    u_int		skz_status;
    u_int		s;
    int			i;

    XZA_SOFTC*		xza_softc;
    XZA_CHANNEL*	xza_chan;
    XZA_SCSI_TARGET*	xza_target;

    SIM_SOFTC*		sim_softc;
    SIM_WS*		sws;

    QB*			freed_qb_ptr;
    CARRIER*		freed_car_ptr;
    RETQE_CMD*		qb_rq_body_ptr;
    READPARAM_CMD*	qb_rp_body_ptr;
    WRITEPARAM_CMD*	qb_wp_body_ptr;

    NP_QB_HEADER*	np_qb_hdr;
    SCSI_QB_HEADER*	scsi_qb_hdr;
    SETDIAG_QB_HEADER*	diag_qb_hdr;

    SNDSCSI_CMD*	scsi_qb_body_ptr;


    SIM_MODULE(skz_command_complete);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nEntering %s", module) );

    xza_chan = (XZA_CHANNEL *) qb_ptr -> qb_driver.xza_chan_ptr;
    xza_softc = (XZA_SOFTC *) xza_chan -> xza_softc;
    sim_softc = xza_chan -> sim_softc;

    /* get sws */
    sws = (SIM_WS *) qb_ptr -> qb_driver.sws;

    /*
    ** Free up DME resources associated with this command, if any.
    */
    skz_status = skz_dme_end ( qb_ptr, xza_softc );

    /*
    ** Dissect the Q Buffer...
    **
    ** At this point we have a pointer to a Q Buffer that was returned on
    ** the adapter response queue.  The Q Buffer has two opcodes, an
    ** N_Port opcode and potentially a scsi or diagnostic opcode.
    ** The action taken on the completed Q Buffer depends upon the
    ** the opcodes.
    **
    ** First case off the N_Port opcode.. if the opcode is PMSNT or
    ** PMREC, check all of the scsi opcodes and take appropriate action.
    ** For the common case, where the scsi opcode is SNDSCSI, simply
    ** check status and complete the command via ss_finish and 
    ** sx_command_complete.
    */

    switch ( ((SCSI_QB_HEADER *) qb_ptr) -> opc ) {

	case PMSNT: /* Physical Media Sent, normal data transfer */
	case PMREC:

	    /*
	    ** Get pointers into the scsi Q Buffer
	    */
	    scsi_qb_hdr = &(qb_ptr -> qb_header.scsi_qb_hdr);
	    scsi_qb_body_ptr = (SNDSCSI_CMD *) (qb_ptr -> qb_body);

#ifdef SKZ_DUMP
	    printf ( "\nCommand completion...\n" );
	    skz_dump_scsi_qb_hdr ( scsi_qb_hdr );
	    skz_dump_scsi_qb_body ( scsi_qb_body_ptr, qb_ptr );
#endif

	    /*
	    ** Now, check all of the possible scsi opcodes
	    */
	    switch ( scsi_qb_hdr -> scsi_opc ) {

		case SCSISNT:	/* Normal data transfer */

		    /*
		    ** Check failure status
		    */
		    if ( scsi_qb_hdr -> status.fail )
		    {
			skz_error (	module,
					SKZ_QB_ERROR, 
					scsi_qb_hdr -> status.type, 
					&(sws -> cam_status),
					qb_ptr );
#ifdef SKZ_DUMP_ON_ERROR
			if ( !(xza_softc -> flags & SKZ_PROBE) )
			{
			    printf ( "\nsws = %x", sws );
    			    skz_dump_xza_softc ( xza_softc );
    			    skz_dump_scsi_qb_hdr ( scsi_qb_hdr );
    			    skz_dump_scsi_qb_body ( scsi_qb_body_ptr, qb_ptr );
			}
#endif
		    }	
		    else /* Fail bit NOT set */
		    {
		        /*
		        ** Get SCSI status, if any
		        */
		        if ( scsi_qb_hdr -> status.type == XZA_SCSI_STATUS_RET )
			    sws->scsi_status = scsi_qb_hdr->status.scsi_status;

			if   ( ( scsi_qb_hdr -> status.scsi_status ==
				SCSI_STAT_CHECK_CONDITION ) ||
			       ( scsi_qb_hdr -> status.scsi_status ==
				SCSI_STAT_COMMAND_TERMINATED ) ) 
			{	
			    sws -> cam_status = CAM_REQ_CMP_ERR;
#ifdef SKZ_CAM_AUTOSENSE
			    as_start ( sws ); 
#else
			    if ( xza_softc -> xdev.firm_rev < XZA_MIN_AS_REV )
				as_start ( sws );
			    else
			    {
				sws -> cam_status |= CAM_AUTOSNS_VALID;
				sws -> ccb -> cam_sense_resid = 0;
			    }
#endif
			}
			else if ( scsi_qb_hdr -> status.scsi_status !=
			 		SCSI_STAT_GOOD ) 
			    sws -> cam_status = CAM_REQ_CMP_ERR;
			else
			    sws -> cam_status = CAM_REQ_CMP;

			/*
			** Set the data_xfer values to allow sim_xpt.c's
			** sx_command_complete to calculate the residual
			** number of bytes ( not transferred )
 			*/
			sws -> data_xfer.data_count = 
				sws -> ccb -> cam_dxfer_len;
			sws -> data_xfer.xfer_count = 
				scsi_qb_body_ptr -> ret_len;


#ifdef SKZ_DUMP
			    skz_dump_data ( sws, qb_ptr );
#endif

		    }

#ifdef SKZ_DUMP
	    printf ( "\nCommand completion dump complete\n\n" );
#endif

		    SIM_PRINTD ( 	NOBTL,
					NOBTL,
					NOBTL,
					CAMD_FLOW,
					("\n%s: sws->cam_status = %x",
					module,
					sws -> cam_status) );

		    /*
		    ** Command Completion
		    */
		    if ( ( sws -> flags & SZ_ABORT_INPROG ) &&
	 		!(xza_chan -> flags & SKZ_BUS_RESET_IN_PROGRESS) )
			ss_abort_done ( sim_softc, sws );
		    else
		    {
		        ss_finish ( sws );
		        sx_command_complete ( sws );
		    }

		    break;

		case RSTBD: /* Reset Bus Device */
		    /*
		    ** Check status, if bus_wedged, then reset the bust
		    */
		    if ( scsi_qb_hdr -> status.fail )
			skz_error (	module,
					SKZ_QB_ERROR, 
					scsi_qb_hdr -> status.type, 
					&(sws -> cam_status),
					qb_ptr );
		    else
			sws -> cam_status = CAM_REQ_CMP;


		    break;

		case CMDABRT: /* Abort Command */
		    /*
		    ** This *could* have failed, but what to do about it?
	  	    */
/*
		    if ( scsi_qb_hdr -> status.fail )
			skz_error (	module,
					SKZ_QB_ERROR, 
					scsi_qb_hdr -> status.type, 
					&sws -> cam_status,
					qb_ptr );
*/
		    break;

		case ABRTMRK: /* Mark Abort */
		    /*
		    ** Do nothing at all
		    */
		    break;

		case VNDRSNT: /* vendor specific command sent */

		    break;

	    }

	    /*
	    ** Free up the Queue_Buffer
	    */
	    skz_status = skz_put_queue ( DDFQ, qb_ptr, car_ptr, xza_softc );
	    if ( skz_status != CAM_REQ_CMP )
		skz_error ( module,
			    SKZ_ERROR,
			    skz_status,
			    NULL,
			    xza_softc );

	    break;

	case QERET: /* Queue Entries Returned, in response to RETQE */
	    
	    /*
	    ** We can get the actual number of queue entries that were
	    ** returned by the adapter
            */
	    np_qb_hdr = &(qb_ptr -> qb_header.np_qb_hdr);

	    if ( np_qb_hdr -> status.fail )
		skz_error (	module,
				SKZ_QB_ERROR, 
				np_qb_hdr -> status.type, 
				&(sws -> cam_status),
				qb_ptr );
	    else
	    {
		/* 
	        ** Take returned entries and put them on the DDFQ
	        */
		qb_rq_body_ptr = (RETQE_CMD *) qb_ptr -> qb_body;

	        for ( i = 0; i < qb_rq_body_ptr -> qe_ret; i++ )
	        {
	   	    /*
		    ** Remove entry from ADFQ
		    */
		    skz_status = skz_get_queue (ADFQ, 
						&freed_qb_ptr, 
						&freed_car_ptr, 	
						xza_softc );
		    if ( skz_status == CAM_REQ_CMP )
		        /*
		        ** Insert entry onto DDFQ
		        */
		        skz_status = skz_put_queue ( DDFQ,
						     freed_qb_ptr,	
						     freed_car_ptr,
						     xza_softc );

	        }
	    }

	    /*
	    ** Free up the Queue_Buffer
	    */
	    skz_status = skz_put_queue ( DDFQ, qb_ptr, car_ptr, xza_softc );

	    break;

	case QPURG:

	    /*
	    ** Decrement outstanding PURGQ's
	    */
	    np_qb_hdr = &(qb_ptr -> qb_header.np_qb_hdr);
	    xza_target = &(xza_chan -> target[np_qb_hdr -> dst_xport.targid]);
	    xza_target -> purgqs--;

	    /*
            ** Are we done with this reset? We are if both PURGQ's came back
            */
            if ( xza_target -> purgqs == 0 )
            {
                skz_start_device ( xza_chan, sws );
                ss_device_reset_done ( sim_softc, sws );
            }
                                                                                
	    /*
	    ** Free up the Queue_Buffer
	    */
	    skz_status = skz_put_queue ( DDFQ, qb_ptr, car_ptr, xza_softc );

	    break;

	case CNTRD:

	    /*
	    ** Print ( or write to a file? ) the counter data
	    */
	    np_qb_hdr = &(qb_ptr -> qb_header.np_qb_hdr);

	    if ( np_qb_hdr -> status.fail )
	    {
		skz_error (	module,
				SKZ_QB_ERROR, 
				np_qb_hdr -> status.type, 
				&(sws -> cam_status),
				qb_ptr );
	    }
            else
	    {
		/*
		** Copy QB data to user buffer
		*/
		skz_copyout ( qb_ptr );

	   	sws -> cam_status = CAM_REQ_CMP;

	    }

	    ss_finish ( sws );
	    sx_command_complete ( sws );

	    /*
	    ** Free up the Queue_Buffer
	    */
	    skz_status = skz_put_queue ( DDFQ, qb_ptr, car_ptr, xza_softc );

	    break;

	case DIAGSET:

	    diag_qb_hdr = &(qb_ptr -> qb_header.setdiag_qb_hdr);

	    /*
	    ** Both channels have transitioned to the DIAGNOSTIC state
	    */
	    SKZ_LOCK ( xza_softc -> lock, s );
	    xza_softc -> channel[0].state = DIAGNOSTIC;
	    xza_softc -> channel[1].state = DIAGNOSTIC;
	    SKZ_UNLOCK ( xza_softc -> lock, s );

	    switch ( diag_qb_hdr -> diag_opc ) {

		case EE$WriteImage: 

	    	    if ( diag_qb_hdr -> status.fail )
			sws -> cam_status = CAM_REQ_CMP_ERR;
		    else
		    {
			/*
			** Set the data_xfer values to allow sim_xpt.c's
			** sx_command_complete to calculate the residual
			** number of bytes ( not transferred )
 			*/
			sws -> data_xfer.data_count = 
				sws -> ccb -> cam_dxfer_len;
			sws -> data_xfer.xfer_count = 
				sws -> ccb -> cam_dxfer_len;

			sws -> cam_status = CAM_REQ_CMP;
		    }

		    ss_finish ( sws );
		    sx_command_complete ( sws );

		    /*
		    ** Free up the Queue_Buffer
		    */
		    skz_status = skz_put_queue ( DDFQ, 
						 qb_ptr, 
					 	 car_ptr, 
						 xza_softc );

		    break;

		case EE$ReadImage:

	    	    if ( diag_qb_hdr -> status.fail )
			sws -> cam_status = CAM_REQ_CMP_ERR;
		    else
		    {
			/*
			** Set the data_xfer values to allow sim_xpt.c's
			** sx_command_complete to calculate the residual
			** number of bytes ( not transferred )
 			*/
			sws -> data_xfer.data_count = 
				sws -> ccb -> cam_dxfer_len;
			sws -> data_xfer.xfer_count = 
				sws -> ccb -> cam_dxfer_len;

			sws -> cam_status = CAM_REQ_CMP;
		    }

		    ss_finish ( sws );
		    sx_command_complete ( sws );

		    /*
		    ** Free up the Queue_Buffer
		    */
		    skz_status = skz_put_queue ( DDFQ, 
						qb_ptr, 
						car_ptr, 
						xza_softc );

		    break;

		case EE$WriteEEPROM:

	    	    if ( diag_qb_hdr -> status.fail )
			sws -> cam_status = CAM_REQ_CMP_ERR;
		    else
			sws -> cam_status = CAM_REQ_CMP;			

		    skz_write_eeprom_complete ( xza_softc );

		    ss_finish ( sws );
		    sx_command_complete ( sws );

		    /*
		    ** No need to free the qb here because the adapter
		    ** get re-initialized in write_eeprom_complete
		    */

	    	    break;

		case EE$ReadEEPROM:

	    	    if ( diag_qb_hdr -> status.fail )
			sws -> cam_status = CAM_REQ_CMP_ERR;
		    else
		    {
			/*
			** Set the data_xfer values to allow sim_xpt.c's
			** sx_command_complete to calculate the residual
			** number of bytes ( not transferred )
 			*/
			sws -> data_xfer.data_count = 
				sws -> ccb -> cam_dxfer_len;
			sws -> data_xfer.xfer_count = 
				sws -> ccb -> cam_dxfer_len;

			sws -> cam_status = CAM_REQ_CMP;
		    }

		    ss_finish ( sws );
		    sx_command_complete ( sws );

		    /*
		    ** Free up the Queue_Buffer
		    */
		    skz_status = skz_put_queue ( DDFQ, 
						qb_ptr, 
						car_ptr, 
						xza_softc );

		    break;

		case EE$ReadParam:

	    	    if ( diag_qb_hdr -> status.fail )
		    {
			sws -> cam_status = CAM_REQ_CMP_ERR;
		    }
		    else
		    {
			/*
			** Copy QB data to user buffer
			*/
			skz_copyout ( qb_ptr );

			qb_rp_body_ptr = (READPARAM_CMD *) qb_ptr -> qb_body;

			xza_softc -> flags |= SKZ_EEPROM_DATA_UPDATED;
			xza_softc -> eeprm_fcn = qb_rp_body_ptr -> eeprm_fcn;

			/* Should the scsi id's be updated here as well? */

	   		sws -> cam_status = CAM_REQ_CMP;

		    }

		    ss_finish ( sws );
		    sx_command_complete ( sws );

		    /*
		    ** Re-init the adapter, both channels back to ENABLED
		    */
		    skz_error (	module,
				SKZ_ERROR, 
				SKZ_DIAG_COMPL, 
				NULL,
				xza_softc );

		    break;

		case EE$WriteParam:

	    	    if ( diag_qb_hdr -> status.fail )
			sws -> cam_status = CAM_REQ_CMP_ERR;
		    else
		    {
			qb_wp_body_ptr = (WRITEPARAM_CMD *) qb_ptr -> qb_body;

			xza_softc -> flags |= SKZ_EEPROM_DATA_UPDATED;
			xza_softc -> eeprm_fcn = qb_wp_body_ptr -> eeprm_fcn;

			/* Should the scsi id's be updated here as well? */

	   		sws -> cam_status = CAM_REQ_CMP;

		    }

		    ss_finish ( sws );
		    sx_command_complete ( sws );

		    /*
		    ** Re-init the adapter, both channels back to ENABLED
		    */
		    skz_error (	module,
				SKZ_ERROR, 
				SKZ_DIAG_COMPL, 
				NULL,
				xza_softc );

		    break;

		case EE$ReadErrHrd:
		case EE$ReadErrSoft:
		
		    /*
		    ** Check failure status
		    */
		    if ( diag_qb_hdr -> status.fail )
		    {
			sws -> cam_status = CAM_REQ_CMP_ERR;
			skz_error (	module,
					SKZ_QB_ERROR, 
					diag_qb_hdr -> status.type, 
					&(sws -> cam_status),
					qb_ptr );
					
		    }
		    else
		    {
			/*
			** Set the data_xfer values to allow sim_xpt.c's
			** sx_command_complete to calculate the residual
			** number of bytes ( not transferred )
 			*/
			sws -> data_xfer.data_count = 
				sws -> ccb -> cam_dxfer_len;
			sws -> data_xfer.xfer_count = 
				sws -> ccb -> cam_dxfer_len;

	   	        sws -> cam_status = CAM_REQ_CMP;
		    }

		    /*
		    ** Complete the command
		    */
		    ss_finish ( sws );
		    sx_command_complete ( sws );

		    /*
		    ** Re-init the adapter, both channels back to ENABLED
		    */
		    skz_error (	module,
				SKZ_ERROR, 
				SKZ_DIAG_COMPL, 
				NULL,
				xza_softc );

		    break;

		case NCR$ReadReg:

		    /*
		    ** Check failure status
		    */
		    if ( diag_qb_hdr -> status.fail )
		    {
			sws -> cam_status = CAM_REQ_CMP_ERR;
		    }
		    else
		    {
			/*
			** Copy QB data to user buffer
			*/
			skz_copyout ( qb_ptr );
		
			sws -> cam_status = CAM_REQ_CMP;
		    }

		    ss_finish ( sws );
		    sx_command_complete ( sws );

		    /*
		    ** Re-init the adapter, both channels back to ENABLED
		    */
		    skz_error (	module,
				SKZ_ERROR, 
				SKZ_DIAG_COMPL, 
				NULL,
				xza_softc );

		    break;

		case NCR$WriteReg:

		    /*
		    ** Free up the Queue_Buffer
		    */
		    skz_status = skz_put_queue ( DDFQ, 
						qb_ptr, 
						car_ptr, 
						xza_softc );

		    break;

		default:
		    /*
		    ** Free up the Queue_Buffer
		    */
		    skz_status = skz_put_queue ( DDFQ, 
						qb_ptr, 
						car_ptr, 
						xza_softc );

		    break;

	    }

	case NEXSET:
	    /*
	    ** Will we ever want to look at the previous settings??
	    */
	    /*
	    ** Free up the Queue_Buffer
	    */
	    skz_status = skz_put_queue ( DDFQ, qb_ptr, car_ptr, xza_softc );

	    break;

	default:
	    /*
	    ** Free up the Queue_Buffer
	    */
	    skz_status = skz_put_queue ( DDFQ, qb_ptr, car_ptr, xza_softc );

	    break;

    }

    /*
    ** Kick off any waiting commands
    */
    if ( !(xza_chan -> flags & SKZ_RESET_NEEDED) &&
	 !(xza_chan -> flags & SKZ_BUS_RESET_IN_PROGRESS) &&
	 !(xza_softc -> flags & SKZ_PROBE) &&
	 (sim_softc) )
        SC_SCHED_RUN_SM ( sim_softc );

	

    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nLeaving %s", module) );

    return; 
}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_abort_command
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine performs the necessary N_Port/XZA operations to
 * 	abort a previously issued SCSI command.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

unsigned int	
skz_abort_command ( SIM_SOFTC* sim_softc, SIM_WS* sws, u_int tiop_flag )

{
    unsigned int	skz_status;

    QB*			qb_ptr;
    CARRIER*		car_ptr;

    SCSI_QB_HEADER*	qb_hdr_ptr;
    ABRTCMD_CMD*	qb_ab_body_ptr;
    MRKABRT_CMD*	qb_ma_body_ptr;
    QB_DRIVER*  	qb_driver_ptr;

    XZA_SOFTC*		xza_softc;
    XZA_CHANNEL*	xza_chan;

    SIM_MODULE(skz_abort_command);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nEntering %s", module) );


    xza_chan = (XZA_CHANNEL *) sim_softc -> hba_sc;
    xza_softc = (XZA_SOFTC *) xza_chan -> xza_softc;

    /*
    ** Check for minimum firmware revision
    */
    if ( (xza_softc -> xdev.firm_rev < XZA_MIN_ABORT_REV) ||
	 cam_at_boottime() || shutting_down )
	return ( CAM_PROVIDE_FAIL );

    /*
    ** Get a Q_Buffer from the free queue
    */
    skz_status = skz_get_queue ( DDFQ, &qb_ptr, &car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error (	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
	return ( CAM_PROVIDE_FAIL );
    }

    qb_hdr_ptr = (SCSI_QB_HEADER *) &( qb_ptr -> qb_header );
    qb_ab_body_ptr = (ABRTCMD_CMD *) ( qb_ptr -> qb_body );
    qb_driver_ptr = (QB_DRIVER *) &( qb_ptr -> qb_driver );

    /*
    ** Initialize the ABRTCMD command
    */
    qb_hdr_ptr -> opc = SNDPM;
    qb_hdr_ptr -> chnl_idx = xza_chan -> chan_num;
    qb_hdr_ptr -> flags.r = 1;			/* always get response */
    qb_hdr_ptr -> scsi_opc = ABRTCMD;
    qb_hdr_ptr -> dst_xport.targid = sws -> targid;
    qb_hdr_ptr -> dst_xport.lun = sws -> lun;
    qb_hdr_ptr -> src_xport.targid = xza_chan -> scsi_id.targid;

    /*
    ** Set the 'terminate I/O process' bit
    */
    qb_hdr_ptr -> scsi_flags.tiop = tiop_flag;

    qb_ab_body_ptr -> xct_id = sws -> seq_num;

    qb_driver_ptr -> sws = (vm_offset_t) sws;
    qb_driver_ptr -> xza_chan_ptr = (vm_offset_t) xza_chan;

    /*
    ** Insert abort command onto high priority command queue
    */
    skz_status = skz_put_queue ( DCCQ2, qb_ptr, car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error (	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
	return ( CAM_PROVIDE_FAIL );
    }

    /*
    ** Create and issue MRKABRT commands for the other 
    ** two command queues on the channel
    */

    /*
    ** Get a Q_Buffer from the free queue for DCCQ1
    */
    skz_status = skz_get_queue ( DDFQ, &qb_ptr, &car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error (	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
	return ( CAM_PROVIDE_FAIL );
    }

    qb_hdr_ptr = (SCSI_QB_HEADER *) &( qb_ptr -> qb_header );
    qb_ma_body_ptr = (MRKABRT_CMD *) ( qb_ptr -> qb_body );
    qb_driver_ptr = (QB_DRIVER *) &( qb_ptr -> qb_driver );

    /*
    ** Initialize the MRKABRT command
    */
    qb_hdr_ptr -> opc = SNDPM;
    qb_hdr_ptr -> chnl_idx = xza_chan -> chan_num;
    qb_hdr_ptr -> flags.r = 1;			/* always get response */
    qb_hdr_ptr -> scsi_opc = MRKABRT;
    qb_hdr_ptr -> dst_xport.targid = sws -> targid;
    qb_hdr_ptr -> dst_xport.lun = sws -> lun;
    qb_hdr_ptr -> src_xport.targid = xza_chan -> scsi_id.targid;

    qb_driver_ptr -> sws = (vm_offset_t) sws;
    qb_driver_ptr -> xza_chan_ptr = (vm_offset_t) xza_chan;

    /*
    ** Insert abort command onto medium priority command queue
    */
    skz_status = skz_put_queue ( DCCQ1, qb_ptr, car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error (	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
	return ( CAM_PROVIDE_FAIL );
    }


    /*
    ** Get a Q_Buffer from the free queue for DCCQ0
    */
    skz_status = skz_get_queue ( DDFQ, &qb_ptr, &car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error (	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
	return ( CAM_PROVIDE_FAIL );
    }

    qb_hdr_ptr = (SCSI_QB_HEADER *) &( qb_ptr -> qb_header );
    qb_ma_body_ptr = (MRKABRT_CMD *) ( qb_ptr -> qb_body );
    qb_driver_ptr = (QB_DRIVER *) &( qb_ptr -> qb_driver );

    /*
    ** Initialize the MRKABRT command
    */
    qb_hdr_ptr -> opc = SNDPM;
    qb_hdr_ptr -> chnl_idx = xza_chan -> chan_num;
    qb_hdr_ptr -> flags.r = 1;			/* always get response */
    qb_hdr_ptr -> scsi_opc = MRKABRT;
    qb_hdr_ptr -> dst_xport.targid = sws -> targid;
    qb_hdr_ptr -> dst_xport.lun = sws -> lun;
    qb_hdr_ptr -> src_xport.targid = xza_chan -> scsi_id.targid;

    qb_driver_ptr -> sws = (vm_offset_t) sws;
    qb_driver_ptr -> xza_chan_ptr = (vm_offset_t) xza_chan;

    /*
    ** Insert MRKABRT command onto low priority command queue
    */
    skz_status = skz_put_queue ( DCCQ0, qb_ptr, car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error (	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
	return ( CAM_PROVIDE_FAIL );
    }


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nLeaving %s", module) );

    return ( CAM_REQ_CMP );
}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_reset_bus()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine causes a specified SCSI bus ( XZA channel ) to 
 * 	reset.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 * 
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:  This routine assumes that threads are active!
 *
 ************************************************************************/

skz_reset_bus ( SIM_SOFTC* sim_softc )

{
    u_int		skz_status;
    u_int		s;

    u_int		delay;

    u_int		chan_mask;

    XPD1_REG		xpd1;
    ASR_REG		asr;

    XZA_SOFTC*		xza_softc;
    XZA_CHANNEL*	xza_chan;

    SIM_MODULE(skz_reset_bus);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nEntering %s", module) );

    xza_chan = (XZA_CHANNEL *) sim_softc -> hba_sc;
    xza_softc = (XZA_SOFTC *) xza_chan -> xza_softc;

    /*
    ** Set the bus reset flag in the sim_softc
    ** If already set, just return, the reset is under way
    */
    if ( sim_softc -> error_recovery & ERR_BUS_RESET )
	return ( CAM_REQ_CMP );
    else
	sim_softc -> error_recovery |= ERR_BUS_RESET;

    /*
    ** Check that the specified channel is ENABLED, otherwise
    ** the CBRCR emulation has no effect
    */
    if ( xza_chan -> state != ENABLED )
	return ( CAM_REQ_CMP );

    /*
    ** setup XPD1 register 
    */
    bzero ( &xpd1, sizeof ( XPD1_REG ) );

    xpd1.rcode = CBRCR_EMULATION;
    if ( xza_chan -> chan_num == 0 )
	xpd1.c0 = 1;
    else
    	xpd1.c1 = 1;

    /*
    ** Set the reset_in_progress flag 
    */
    SKZ_LOCK ( xza_softc -> lock, s );
    xza_chan -> flags |= SKZ_BUS_RESET_IN_PROGRESS; /* Bus reset in progress */
    SKZ_UNLOCK ( xza_softc -> lock, s );

    /*
    ** Write to the CBRCR register.  This causes the bus to be reset.
    */
    SIM_PRINTD (	NOBTL,
			NOBTL,
			NOBTL,
			CAMD_FLOW,
			("\n%s: Writing to CBRCR", module) );

    skz_status = skz_write_reg ( CBRCR, xpd1, NULL, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error ( module,
		    SKZ_ERROR,
		    skz_status,
		    NULL,
		    xza_softc );

        SKZ_LOCK ( xza_softc -> lock, s );
        xza_chan -> flags &= ~SKZ_BUS_RESET_IN_PROGRESS; /* reset in progress */
        SKZ_UNLOCK ( xza_softc -> lock, s );

	/*
	** Couldn't even write to CBRCR successfully
	*/
	return ( CAM_REQ_CMP_ERR );
    }

    if ( cam_at_boottime() || shutting_down )
    {
	/*
	** We successfully wrote the CBRCR and the bus is being reset...
	** now wait for the reset to complete before returning
	*/

	/*
	** Wait for state transition
	*/
	skz_status = skz_state_wait ( xza_chan, DISABLED, XZA_BUS_RESET_TIME );
	if ( skz_status != CAM_REQ_CMP )
	    skz_status = CAM_REQ_CMP_ERR;
    }

    return ( skz_status );

}

/************************************************************************
 *
 *  ROUTINE NAME:  skz_reset_device()
 *
 *  FUNCTIONAL DESCRIPTION:
 * 	This routine performs the N_Port/XZA operations required to
 * 	initiate a bus device reset.
 *
 *  FORMAL PARAMETERS:
 *	sws - pointer the the SIM Working Set structure
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/
unsigned int
skz_reset_device ( SIM_WS* sws )

{
    u_int		skz_status;
    u_int		s;
    u_int 		delay;

    QB*			qb_ptr;
    CARRIER*		car_ptr;

    SCSI_QB_HEADER*	qb_hdr_ptr;
    BDRST_CMD*		qb_bdr_body_ptr;
    PURGQ_CMD*		qb_pq_body_ptr;
    QB_DRIVER*  	qb_driver_ptr;

    XZA_SOFTC*		xza_softc;
    XZA_CHANNEL*	xza_chan;
    XZA_SCSI_TARGET*	xza_target;

    SIM_MODULE(skz_reset_device);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nEntering %s", module) );

    xza_chan = (XZA_CHANNEL *) sws -> sim_sc -> hba_sc;
    xza_target = &(xza_chan -> target[sws->targid]);
    xza_softc = (XZA_SOFTC *) xza_chan -> xza_softc;

    /*
    ** Check to see whether this XZA can handle BDRs at all
    */
    if ( xza_softc -> xdev.firm_rev < XZA_MIN_BDR_REV )
    {
	ss_device_reset_done ( sws -> sim_sc, sws );
	return ( CAM_REQ_CMP );
    }

    /*
    ** Get a Q_Buffer from the free queue
    */
    skz_status = skz_get_queue ( DDFQ, &qb_ptr, &car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error ( 	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
	return ( CAM_PROVIDE_FAIL );
    }

    qb_hdr_ptr = (SCSI_QB_HEADER *) &( qb_ptr -> qb_header );
    qb_bdr_body_ptr = (BDRST_CMD *) ( qb_ptr -> qb_body );
    qb_driver_ptr = (QB_DRIVER *) &( qb_ptr -> qb_driver );

    /*
    ** Initialize the BDRST command
    */
    qb_hdr_ptr -> opc = SNDPM;
    qb_hdr_ptr -> chnl_idx = xza_chan -> chan_num;
    qb_hdr_ptr -> flags.r = 1;			/* always get response */
    qb_hdr_ptr -> scsi_opc = BDRST;
    qb_hdr_ptr -> dst_xport.targid = sws -> targid;
    qb_hdr_ptr -> dst_xport.lun = sws -> lun;
    qb_hdr_ptr -> src_xport.targid = xza_chan -> scsi_id.targid;
    qb_hdr_ptr -> src_xport.lun = 0;

    qb_bdr_body_ptr -> xct_id = sws -> seq_num;

    qb_driver_ptr -> sws = (vm_offset_t) sws;
    qb_driver_ptr -> xza_chan_ptr = (vm_offset_t) xza_chan;

    /*
    ** Insert bdrst command onto high priority command queue
    */
    skz_status = skz_put_queue ( DCCQ2, qb_ptr, car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error ( 	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
	return ( CAM_PROVIDE_FAIL );
    }

    /*
    ** Create PURGQ commands for dccq1 and dccq0
    */
    /*
    ** Get a Q_Buffer from the free queue
    */
    skz_status = skz_get_queue ( DDFQ, &qb_ptr, &car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error ( 	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
	return ( CAM_PROVIDE_FAIL );
    }

    qb_hdr_ptr = (SCSI_QB_HEADER *) &( qb_ptr -> qb_header );
    qb_pq_body_ptr = (PURGQ_CMD *) ( qb_ptr -> qb_body );
    qb_driver_ptr = (QB_DRIVER *) &( qb_ptr -> qb_driver );

    /*
    ** Initialize the PURGQ command
    */
    qb_hdr_ptr -> opc = PURGQ;
    qb_hdr_ptr -> chnl_idx = xza_chan -> chan_num;
    qb_hdr_ptr -> flags.r = 1;			/* always get response */
    qb_hdr_ptr -> dst_xport.targid = sws -> targid;
    qb_hdr_ptr -> dst_xport.lun = sws -> lun;
    qb_hdr_ptr -> src_xport.targid = xza_chan -> scsi_id.targid;
    qb_hdr_ptr -> src_xport.lun = 0;

    /* qb_body_ptr -> lunq_mask = 1 << sws -> lun; */
    qb_pq_body_ptr -> lunq_mask = SKZ_ALL_LUNS;

    qb_driver_ptr -> sws = (vm_offset_t) sws;
    qb_driver_ptr -> xza_chan_ptr = (vm_offset_t) xza_chan;

    /*
    ** Insert purgq command onto medium priority command queue
    */
    skz_status = skz_put_queue ( DCCQ1, qb_ptr, car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error ( 	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
	return ( CAM_PROVIDE_FAIL );
    }

    /*
    ** Increment the number of purgq's outstanding
    */
    SKZ_LOCK ( xza_softc -> lock, s );

    xza_target -> purgqs++;

    SKZ_UNLOCK ( xza_softc -> lock, s );

    /*
    ** Get a Q_Buffer from the free queue
    */
    skz_status = skz_get_queue ( DDFQ, &qb_ptr, &car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error ( 	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
	return ( CAM_PROVIDE_FAIL );
    }

    qb_hdr_ptr = (SCSI_QB_HEADER *) &( qb_ptr -> qb_header );
    qb_pq_body_ptr = (PURGQ_CMD *) ( qb_ptr -> qb_body );
    qb_driver_ptr = (QB_DRIVER *) &( qb_ptr -> qb_driver );


    /*
    ** Initialize the PURGQ command
    */
    qb_hdr_ptr -> opc = PURGQ;
    qb_hdr_ptr -> chnl_idx = xza_chan -> chan_num;
    qb_hdr_ptr -> flags.r = 1;			/* always get response */
    qb_hdr_ptr -> dst_xport.targid = sws -> targid;
    qb_hdr_ptr -> dst_xport.lun = sws -> lun;
    qb_hdr_ptr -> src_xport.targid = xza_chan -> scsi_id.targid;
    qb_hdr_ptr -> src_xport.lun = 0;

    /* qb_body_ptr -> lunq_mask = 1 << sws -> lun; */
    qb_pq_body_ptr -> lunq_mask = SKZ_ALL_LUNS;

    qb_driver_ptr -> sws = (vm_offset_t) sws;
    qb_driver_ptr -> xza_chan_ptr = (vm_offset_t) xza_chan;

    /*
    ** Insert purgq command onto low priority command queue
    */
    skz_status = skz_put_queue ( DCCQ0, qb_ptr, car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error (	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
	return ( CAM_PROVIDE_FAIL );
    }

    /*
    ** Increment the number of purgq's outstanding
    */
    SKZ_LOCK ( xza_softc -> lock, s );

    xza_target -> purgqs++;

    SKZ_UNLOCK ( xza_softc -> lock, s );

    /*
    ** Wait for the QPURG responses to come back, indicating
    ** that the target is ready to be re-enabled
    */
    if ( cam_at_boottime() || shutting_down )
    {
	delay = 0;
	
	while ( (delay < XZA_BUS_DEV_RESET_TIME) &&
		(xza_target -> purgqs > 0) )
	{
	    /*
	    ** If an event occurred, process it
	    ** and return, because there'll be no "response"
	    ** if any of these events occurrs.
	    */
	    skz_process_misc ( xza_chan );

	    /*
	    ** Just attempt to process anything that might be there now,
	    ** and let skz_process_resp handle status, etc.
	    */
	    skz_process_resp ( xza_softc );


	    DELAY ( SKZ_ONE_SECOND_DELAY );
	    delay++;
	}

	if ( xza_target -> purgqs != 0 )
	{
	    /*
	    ** The device reset never completed
	    */
	    skz_error (	module,
			SKZ_ERROR,
			SKZ_BDRST_TIMEOUT,
			NULL,
			xza_softc );

	    skz_status = CAM_REQ_CMP_ERR;
	}
	else
	    skz_status = CAM_REQ_CMP;


    } /* if cam_at_bootime() */
    

    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nLeaving %s", module) );

    return ( skz_status );
}

/************************************************************************
 *
 *  ROUTINE NAME:  skz_halt_adapter()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine brings the XZA to the uninitialized state, using
 *	the nhalt bit in the XBE.  Adapter history and counter data is
 *	preserved.
 * 
 *	This routine only halts the adapter *if needed!*.  If both channels
 *	are already in the UNINITIALIZED state, there's no need to do the
 *	halt.  This help save on scsi bus resets.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

skz_halt_adapter ( XZA_SOFTC* xza_softc )

{
    u_int	skz_status = CAM_REQ_CMP;
    u_int	s;
    u_int	delay;
    vm_offset_t	wait_event = 0;

    XBE_REG	xbe;
    XPUD_REG	xpud;

    unsigned int	channel_mask;

    SIM_MODULE(skz_halt_adapter);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nEntering %s", module) );
    /*
    ** Only do the halt if the adapter channels are NOT already
    ** in the UNINITIALIZED state or DIAGNOSTIC state
    */
/*
** If we're enabling both channels even though they both aren't
** technically "alive", then this isn't right...
*/

    if ( ( !( xza_softc -> channel[0].flags & SKZ_ALIVE ) ||
	   ( xza_softc -> channel[0].state == UNINITIALIZED ) ) &&
         ( !( xza_softc -> channel[1].flags & SKZ_ALIVE ) ||
	   ( xza_softc -> channel[1].state == UNINITIALIZED ) ) )
	return ( CAM_REQ_CMP );	/* already halted */
/*
**
** Because of a bug in the XZA firmware, we have to initialize the
** adapter to get out of the DIAGNOSTIC state.  When this gets fixed,
** we won't need to halt, we can go right to the DISABLED state using
** CICR.  This will save 3 bus resets per bus for *each* DIAG command
**
    if ( ( xza_softc -> channel[0].state == DIAGNOSTIC ) &&
	 ( xza_softc -> channel[1].state == DIAGNOSTIC ) )
	return ( CAM_REQ_CMP );	
*/

    /*
    ** If either channel is ENABLED, then first
    ** do a bus reset.  This allows us to clean up commands already
    ** queued to the adapter before actually resetting
    */
    if ( (xza_softc -> channel[0].state == ENABLED) &&
	!(xza_softc -> flags & SKZ_PROBE) )
    {
	skz_status = skz_reset_bus ( xza_softc -> channel[0].sim_softc );
/*
** If the reset_bus fails for some reason, let's just go ahead and halt
** the adapter, rather than trying to recover from the reset failure
	if ( skz_status != CAM_REQ_CMP )
	    return ( skz_status );
*/
    }

    if ( (xza_softc -> channel[1].state == ENABLED) &&
	!(xza_softc -> flags & SKZ_PROBE) )
    {
	skz_status = skz_reset_bus ( xza_softc -> channel[1].sim_softc );
/*
	if ( skz_status != CAM_REQ_CMP )
	    return ( skz_status );
*/
    }

    bzero ( &xbe, sizeof ( XBE_REG ) );
    bzero ( &xpud, sizeof ( XPUD_REG ) );

    /*
    ** First Read the XBE contents, then only change the nhalt bit
    */
    skz_status = skz_read_reg ( XBE, &xbe, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	return ( skz_status );

    /*
    ** Write to the XBE register, setting the nhalt bit, and then clearing it
    */
    xbe.nhalt = 1;
    xbe.emp = 1;
    xbe.dxto = 1;

    skz_status = skz_write_reg ( XBE, xbe, NULL, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	return ( skz_status );

    xbe.nhalt = 0;

    skz_status = skz_write_reg ( XBE, xbe, NULL, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	return ( skz_status );

    /*
    ** Check the XPUD<unin> bit for transition to UNINITIALIZED state 
    */
    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_FLOW,
		 ("\n%s: Waiting for Adapter to Enter UNINITIALIZED state", 
		   module) );

    /*
    ** Hang on a bit, so that the adapter can clear the xpud<unin>
    ** bit initially.  Otherwise we grab an invalid value from xpud 
    ** too fast and don't really wait for the halt to complete
    */
    DELAY ( 100000 );

    /*
    ** First read of XPUD ( ASR )
    */
    skz_status = skz_read_reg ( XPUD, &xpud, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	return ( skz_status );

    delay = 0;

    /*
    ** Poll for xpud.unin 
    */
    while ( !(xpud.unin) && (delay < XZA_ADAPTER_HALT_TIME) )
    {
	DELAY ( SKZ_ONE_SECOND_DELAY );
	delay++;

	/*
	** RE-read the register
	*/
	skz_status = skz_read_reg ( XPUD, &xpud, xza_softc );
        if ( skz_status != CAM_REQ_CMP )
	    return ( skz_status );

    }


    SIM_PRINTD (NOBTL,
		NOBTL,
		NOBTL,
		CAMD_FLOW,
		("\n%s: XPUD = %x", module, xpud) );

    /*
    ** Verify timeout or state change
    */
    if ( !(xpud.unin) )
        skz_status = SKZ_ADAPTER_HALT_TIMEOUT;
    else
	skz_status = CAM_REQ_CMP;

    /*
    ** Set the adapter state to UNINITIALIZED
    */
    SKZ_LOCK ( xza_softc -> lock, s );
    xza_softc -> channel[0].state = UNINITIALIZED;
    xza_softc -> channel[1].state = UNINITIALIZED;
    SKZ_UNLOCK ( xza_softc -> lock, s );

    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nLeaving %s", module) );


    return ( skz_status );
 
}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_reset_adapter()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine brings the XZA to the uninitialized state, using
 *	the nrst bit in the XBE.  Adapter history and counter data is
 *	preserved.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

skz_reset_adapter ( XZA_SOFTC* xza_softc )

{
    u_int		skz_status = CAM_REQ_CMP;
    u_int		s;
    u_int		delay;
    vm_offset_t		wait_event = 0;

    XBE_REG		xbe;
    XPUD_REG		xpud;

    unsigned int	channel_mask;

    SIM_MODULE(skz_reset_adapter);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nEntering %s", module) );

    bzero ( &xbe, sizeof ( XBE_REG ) );
    bzero ( &xpud, sizeof ( XPUD_REG ) );
    /*
    ** If either channel is ENABLED, then first
    ** do a bus reset.  This allows us to clean up commands already
    ** queued to the adapter before actually resetting
    */
    if ( xza_softc -> channel[0].state == ENABLED )
    {
	skz_status = skz_reset_bus ( xza_softc -> channel[0].sim_softc );
/*
	if ( skz_status != CAM_REQ_CMP )
	    return ( skz_status );
*/
    }

    if ( xza_softc -> channel[1].state == ENABLED )
    {
	skz_status = skz_reset_bus ( xza_softc -> channel[1].sim_softc );
/*
	if ( skz_status != CAM_REQ_CMP )
	    return ( skz_status );
*/
    }

    /*
    ** First Read the XBE contents, then only change the nhalt bit
    */
    skz_status = skz_read_reg ( XBE, &xbe, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	return ( skz_status );

    /*
    ** Write to the XBE register, setting the nhalt bit, and then clearing it
    */
    xbe.nrst = 1;

    skz_status = skz_write_reg ( XBE, xbe, NULL, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	return ( skz_status );

    xbe.nrst = 0;

    skz_status = skz_write_reg ( XBE, xbe, NULL, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	return ( skz_status );

    DELAY ( 100000 ); /* Wait for XZA to clear unin bit */

    /*
    ** Check the XPUD<unin> bit for transition to UNINITIALIZED state 
    */
    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_FLOW,
		 ("\n%s: Waiting for Adapter to Enter UNINITIALIZED state", 
		   module) );

    /*
    ** First read of XPUD ( ASR )
    */
    skz_status = skz_read_reg ( XPUD, &xpud, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	return ( skz_status );

    delay = 0;

    /*
    ** Poll for xpud.unin 
    */
    while ( !(xpud.unin) && (delay < XZA_ADAPTER_RESET_TIME) )
    {
	DELAY ( SKZ_ONE_SECOND_DELAY );
	delay++;

	/*
	** RE-read the register
	*/
	skz_status = skz_read_reg ( XPUD, &xpud, xza_softc );
        if ( skz_status != CAM_REQ_CMP )
	    return ( skz_status );
    }


    SIM_PRINTD (NOBTL,
		NOBTL,
		NOBTL,
		CAMD_FLOW,
		("\n%s: XPUD = %x", module, xpud) );
    /*
    ** Verify timeout or state change
    */
    if ( !(xpud.unin) )
        skz_status = SKZ_ADAPTER_RESET_TIMEOUT;
    else
	skz_status = CAM_REQ_CMP;

    /*
    ** Set the adapter state to UNINITIALIZED
    */
    SKZ_LOCK ( xza_softc -> lock, s );
    xza_softc -> channel[0].state = UNINITIALIZED;
    xza_softc -> channel[1].state = UNINITIALIZED;
    SKZ_UNLOCK ( xza_softc -> lock, s );

    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nLeaving %s", module) );

    return ( skz_status );
 
}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_disable_adapter()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine puts the specified channel(s) into the disabled
 * 	state, regardless of whether the channel(s) was in the enabled
 * 	or uninitialized state when this routine is called.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

skz_disable_adapter( XZA_SOFTC* xza_softc, u_int channel_mask )

{
    unsigned int	skz_status = CAM_REQ_CMP;
    vm_offset_t		phys_addr;
    kern_return_t	kern_status;

    ASR_REG		asr;
    XPD1_REG		xpd1;
    XPD2_REG		xpd2;

    XZA_CHANNEL*	xza_chan0;
    XZA_CHANNEL*	xza_chan1;


    SIM_MODULE(skz_disable_adapter);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nEntering %s", module) );

    xza_chan0 = &(xza_softc -> channel[0]);
    xza_chan1 = &(xza_softc -> channel[1]);

    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_FLOW,
 		 ("\n%s: Channel 0 State = %d", module, xza_chan0 -> state));

    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_FLOW,
 		 ("\n%s: Channel 1 State = %d", module, xza_chan1 -> state));

    /*
    ** Should we DISABLE channel 0?
    */
    if ( channel_mask & XZA_CHAN_0 )
	if ( ( xza_chan0 -> state == UNINITIALIZED ) ||
	     ( xza_chan0 -> state == DIAGNOSTIC ) )
	{

	    bzero ( &xpd1, sizeof ( XPD1_REG ) );
	    bzero ( &xpd2, sizeof ( XPD2_REG ) );

	    /*
            ** Setup XPDx registers for CICR emulation
            */
	    if ( ( xza_chan1 -> state == UNINITIALIZED ) ||
		 ( xza_chan1 -> state == DIAGNOSTIC ) )
	    {
		kern_status = kvtop ( xza_softc -> ab, &phys_addr );
		if ( kern_status != KERN_SUCCESS )
	    	    return ( SKZ_PHYS_CONV_ERR );

		xpd1.c0 = 1;
		xpd1.rcode = CICR_EMULATION;
		xpd1.pax = phys_addr >> 37;
		xpd2.xpd2_un.pa = phys_addr >> 5;

	        if ( !(cam_at_boottime()) && !shutting_down )
	        {
	    	    xpd1.slot = xza_softc -> lamb_xmi_node;
                    xpd1.vector = xza_softc -> c0_misc_intr_vector >> 4;
	            xpd1.ipl = DEV_LEVEL_1;
	        }

	    }
	    else
	    {
	        xpd1.rcode = CICR_EMULATION;
	        xpd1.c0 = 1;
	    }

	    /*
	    ** Write to the register
	    */
	    skz_status = skz_write_reg ( CICR, xpd1, xpd2, xza_softc );
	    if ( skz_status != CAM_REQ_CMP )
	        return ( skz_status );

	    /*
	    ** wait for state transition
	    */
	    skz_status = skz_state_wait ( xza_chan0, 
					  DISABLED,
					  XZA_STATE_CHANGE_TIME );
	    if ( skz_status != CAM_REQ_CMP )
		return ( SKZ_STCHGTMO_DISABLE );

	}
	else
	    return ( SKZ_WRONGSTATE );

    /*
    ** DISABLE Channel 1 ??
    */
    if ( channel_mask & XZA_CHAN_1 )
	if ( ( xza_chan1 -> state == UNINITIALIZED ) ||
	     ( xza_chan1 -> state == DIAGNOSTIC ) )
	{

	    bzero ( &xpd1, sizeof ( XPD1_REG ) );
	    bzero ( &xpd2, sizeof ( XPD2_REG ) );
	    phys_addr = 0;

	    /*
            ** Setup XPDx registers for CICR emulation
            */
	    /*
	    ** If the other channel is already DISABLED or ENABLED,
	    ** we can do the simple version of CICR write
	    */
	    if ( ( xza_chan0 -> state == UNINITIALIZED ) ||
		 ( xza_chan0 -> state == DIAGNOSTIC ) )
	    {
		kern_status = kvtop ( xza_softc -> ab, &phys_addr );
		if ( kern_status != KERN_SUCCESS )
	    	    return ( SKZ_PHYS_CONV_ERR );

		xpd1.c1 = 1;
		xpd1.rcode = CICR_EMULATION;
		xpd1.pax = phys_addr >> 37;
		xpd2.xpd2_un.pa = phys_addr >> 5;

	        if ( !(cam_at_boottime()) && !shutting_down )
	        {
	    	    xpd1.slot = xza_softc -> lamb_xmi_node;
                    xpd1.vector = xza_softc -> c1_misc_intr_vector >> 4;
	            xpd1.ipl = DEV_LEVEL_1;
	        }

	    }
	    else
	    {
	        xpd1.rcode = CICR_EMULATION;
	        xpd1.c1 = 1;
	    }

	    /*
	    ** Write to the register
	    */
	    skz_status = skz_write_reg ( CICR, xpd1, xpd2, xza_softc );
	    if ( skz_status != CAM_REQ_CMP )
	        return ( skz_status );

	    /*
	    ** wait for state transition
	    */
	    skz_status = skz_state_wait ( xza_chan1, 
					  DISABLED,
					  XZA_STATE_CHANGE_TIME );
	    if ( skz_status != CAM_REQ_CMP )
		return ( SKZ_STCHGTMO_DISABLE );

	}
	else
	    return ( SKZ_WRONGSTATE );

    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nLeaving %s", module) );

    return ( skz_status );
}

/************************************************************************
 *
 *  ROUTINE NAME:  skz_enable_channel()
 *
 *  FUNCTIONAL DESCRIPTION: 
 * 	This routine puts the specified channel(s) into the enabled state.
 * 	The channel(s) must be in the disabled state when this routine
 * 	is called.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

skz_enable_channel ( XZA_SOFTC* xza_softc, u_int channel_mask )

{
    u_int	skz_status = CAM_REQ_CMP;
    u_int	delay;

    XPD1_REG xpd1;
    ASR_REG  asr;

    XZA_CHANNEL*	xza_chan0;
    XZA_CHANNEL*	xza_chan1;

    thread_t		thread;


    SIM_MODULE(skz_enable_channel);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nEntering %s", module) );

    xza_chan0 = &(xza_softc -> channel[0]);
    xza_chan1 = &(xza_softc -> channel[1]);

    /*
    ** Check for correct states, and enable each channel requested
    */
    if ( channel_mask & XZA_CHAN_0 ) 
	if ( xza_chan0->state != DISABLED ) 
	    return ( SKZ_WRONGSTATE );
	else
	{
	    /*
	    ** Set up the XPD1 register
	    */
	    bzero ( &xpd1, sizeof ( XPD1_REG ) );
	    xpd1.rcode = CECR_EMULATION;
	    xpd1.c0 = 1;

    	    /*
	    ** Write to the CECR, this causes the channel to go to ENABLED
	    ** Do this for each channel requested
	    */
	    skz_status = skz_write_reg ( CECR, xpd1, NULL, xza_softc );
	    if ( skz_status != CAM_REQ_CMP )
		return ( skz_status );

	    /*
	    ** Wait for the state transition to complete
	    */
	    skz_status = skz_state_wait ( xza_chan0, 
					  ENABLED,
					  XZA_STATE_CHANGE_TIME );
	    if ( skz_status != CAM_REQ_CMP )
		return ( SKZ_STCHGTMO_ENABLE );

	}

    if ( channel_mask & XZA_CHAN_1 ) 
	if ( xza_chan1->state != DISABLED ) 
	    return ( SKZ_WRONGSTATE ); 
	else
	{
	    /*
	    ** Set up the XPD1 register
	    */
	    bzero ( &xpd1, sizeof ( XPD1_REG ) );
	    xpd1.rcode = CECR_EMULATION;
	    xpd1.c1 = 1;

    	    /*
	    ** Write to the CECR, this causes the channel to go to ENABLED
	    ** Do this for each channel requested
	    */
	    skz_status = skz_write_reg ( CECR, xpd1, NULL, xza_softc );
	    if ( skz_status != CAM_REQ_CMP )
		return ( skz_status );

	    /*
	    ** Wait for the state transition to complete
	    */
	    skz_status = skz_state_wait ( xza_chan1, 
					  ENABLED, 
					  XZA_STATE_CHANGE_TIME );
	    if ( skz_status != CAM_REQ_CMP )
		return ( SKZ_STCHGTMO_ENABLE );
	}


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nLeaving %s", module) );

    return ( skz_status );
}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_state_wait()
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

skz_state_wait ( XZA_CHANNEL* xza_chan, u_int state, u_int timeout )

{
    u_int	skz_status;
    u_int	s;
    u_int	delay;
    vm_offset_t	wait_event = 0;

    int		done;

    thread_t	thread;

    ASR_REG	asr;
    ASR_REG	asr_mask;

    SIM_MODULE(skz_state_wait);


    /*
    ** If change already happened, return success
    */
    if ( xza_chan -> state == state )
	return ( CAM_REQ_CMP );


    if ( cam_at_boottime() || shutting_down )
    {
	/*
	** Poll for completion, since we can't take interrupts
	*/
	/*
	** Set up the asr_mask.  This describes the state change that
	** we're going to wait for.  Once we've set this up, all we need to
	** do is 'and' this with the asr values we read from the adapter
	** to determine whether the state change we are looking for has
	** occurred.
	*/
	bzero ( &asr_mask, sizeof ( ASR_REG ) );

	if ( state == DISABLED )
	    asr_mask.cic = 1;
	else 
	    asr_mask.cec = 1;

	if ( xza_chan -> chan_num == 0 )
	    asr_mask.c0 = 1;
	else
	    asr_mask.c1 = 1;

	/*
	** First read the ASR
	*/
	skz_status = skz_read_reg ( ASR, &asr, xza_chan -> xza_softc );
	if ( skz_status != CAM_REQ_CMP )
	    return ( skz_status );

	delay = 0;

	/*
	** I know that there's a much better way to do this, I just
	** can't think of it right now..
	*/
	done = (asr.cic == asr_mask.cic) &&
	       (asr.cec == asr_mask.cec) &&
	       (asr.c0 == asr_mask.c0) &&
	       (asr.c1 == asr_mask.c1);

	/*
	** Poll the ASR for the state transition
	*/
	while ( !(done) && (delay < timeout) )
	{
	    DELAY ( SKZ_ONE_SECOND_DELAY );
	    delay++;

	    skz_status = skz_read_reg ( ASR, &asr, xza_chan -> xza_softc );
	    if ( skz_status != CAM_REQ_CMP )
	        return ( skz_status );

	    done = (asr.cic == asr_mask.cic) &&
	           (asr.cec == asr_mask.cec) &&
	           (asr.c0 == asr_mask.c0) &&
	           (asr.c1 == asr_mask.c1);
	}

	if ( !(done) )
	{
	    /*
	    ** if anything else has happened, process it
	    */
	    if ( asr.ac || asr.ame || asr.dse )
	        skz_process_misc ( xza_chan );    

	    skz_status = SKZ_STATE_CHANGE_TIMEOUT;
	}
        else
	{
	    /*
	    ** process the state change
	    */
	    skz_process_misc ( xza_chan );    

	    if ( xza_chan -> state == state )
	        skz_status = CAM_REQ_CMP;
	    else
		skz_status = SKZ_STATE_CHANGE_TIMEOUT;
	}
    }
    else if ( ( xza_chan -> flags & SKZ_THREADS_ACTIVE ) && !SKZ_IDLE_THREAD )
    {
        /*
        ** Block if threads are activated ( post-boot ) and we haven't already
        ** called assert_wait, otherwise assert_wait panics.  This can happen
        ** if we're re-initializing the adapter from the response thread,
        ** as can happen following diagnostic command responses.
        */
	if ( SKZ_ALREADY_WAITING )
	    SKZ_SAVE_WAIT;
        /*
        ** Block until the interrupt awakens the the misc_th,
        ** which sets the state and resumes this thread
        */
	assert_wait((vm_offset_t)&(xza_chan -> state), TRUE );

        /*
        ** Set a timeout for the state change
        */
        thread_set_timeout (timeout * hz );
        thread_block();

	thread = current_thread();

	if ( xza_chan -> state == state )
	    skz_status = CAM_REQ_CMP;
	else if ( thread -> wait_result == THREAD_TIMED_OUT )
	    skz_status = SKZ_STATE_CHANGE_TIMEOUT;
	else if ( thread -> wait_result == THREAD_AWAKENED )
	    skz_status = CAM_REQ_CMP;

	SKZ_RESTORE_WAIT;
    }
    else
    {
	delay = 0;

	/*
	** Wait for the event to happen
	*/
	while ( (xza_chan -> state != state) && 
		(delay < timeout ) )
        {
	    DELAY ( SKZ_ONE_SECOND_DELAY ); 
	    delay++;
	}

	if ( xza_chan -> state != state )
	{
	    /*
	    ** If we're trying to get from UNINIT to DISABLED
	    ** then call process_misc to process any errors that may
	    ** have occurred which prevent us from getting to 
	    ** DISABLED and from enabling interrupts.
	    */

	    if ( xza_chan -> state == UNINITIALIZED )
		skz_process_misc ( xza_chan );

	    skz_status = SKZ_STATE_CHANGE_TIMEOUT;

	}
        else
	    skz_status = CAM_REQ_CMP;
    }

    return ( skz_status );

}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_put_queue()
 *
 *  FUNCTIONAL DESCRIPTION:
 * 	An N_Port Queue Buffer structure is inserted into the specified
 * 	queue.  Depending upon which queue is being added to, this
 * 	routine also writes to the appropriate adapter register to 
 * 	notify the adapter firmware that the queue has been changed.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

skz_put_queue ( QUEUE_NAMES 	queue, 
		QB* 		qb_ptr, 
		CARRIER* 	car_ptr,
		XZA_SOFTC* 	xza_softc )

{
    u_int		skz_status = CAM_REQ_CMP;
    u_int		s;

    vm_offset_t		phys_addr;
    kern_return_t	kern_status;

    NP_STATUS		np_status;

    unsigned char	channel;		/* CHECK!! */

    SIM_MODULE(skz_put_queue);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nEntering %s", module) );


    SKZ_LOCK ( xza_softc -> lock, s );

    channel = qb_ptr -> qb_header.np_qb_hdr.chnl_idx;

    switch ( queue ) {

	case DCCQ2:
		/*
		** Insert the qb onto the queue
		*/
		np_status = np_insert_da_q (
				&(xza_softc->ab->dccq[channel].dccq2.tail_ptr),
				car_ptr,
				qb_ptr );
		if ( np_status != NP_SUCCESS )
		{
		    skz_status = SKZ_QUEUE_INSERT_ERROR;
		    break;
		}

		/*
		** Update the CQIB
		*/
		phys_addr = 0;
		kern_status = kvtop ( car_ptr, &phys_addr );
		if ( kern_status != KERN_SUCCESS )
		    return ( SKZ_PHYS_CONV_ERR );

		xza_softc -> cqib -> qs[channel].ccq2ir = phys_addr >> 5;

		/*
		** Write to the QIR Register
		*/
		skz_status = skz_write_reg ( QIR, NULL, NULL, xza_softc );
		if ( skz_status != CAM_REQ_CMP )
		    break;

		xza_softc -> channel[channel].commands_sent++;

		break;

	case DCCQ1:
		/*
		** Insert the qb onto the queue
		*/
		np_status = np_insert_da_q (
				&(xza_softc->ab->dccq[channel].dccq1.tail_ptr),
				car_ptr,
				qb_ptr );
		if ( np_status != NP_SUCCESS )
		{
		    skz_status = SKZ_QUEUE_INSERT_ERROR;
		    break;
		}

		/*
		** Update the CQIB
		*/
		kern_status = kvtop ( car_ptr, &phys_addr );
		if ( kern_status != KERN_SUCCESS )
		    return ( SKZ_PHYS_CONV_ERR );

		xza_softc -> cqib -> qs[channel].ccq1ir = phys_addr >> 5;

		/*
		** Write to the QIR Register
		*/
		skz_status = skz_write_reg ( QIR, NULL, NULL, xza_softc );
		if ( skz_status != CAM_REQ_CMP )
		    break;

		xza_softc -> channel[channel].commands_sent++;

		break;

	case DCCQ0:
		/*
		** Insert the qb onto the queue
		*/
		np_status = np_insert_da_q (
				&(xza_softc->ab->dccq[channel].dccq0.tail_ptr),
				car_ptr,
				qb_ptr );
		if ( np_status != NP_SUCCESS )
		{
		    skz_status = SKZ_QUEUE_INSERT_ERROR;
		    break;
		}

		/*
		** Write to the GCQxIR Register
		*/
		if ( channel == 0 )
		    skz_status = skz_write_reg 
			( GCQ0IR, NULL, NULL, xza_softc );
		else if ( channel == 1 )
		    skz_status = skz_write_reg 
			( GCQ1IR, NULL, NULL, xza_softc );

		if ( skz_status != CAM_REQ_CMP )
		    break;
		    

		xza_softc -> channel[channel].commands_sent++;

		break;

	case DAFQ:
		/*
		** Insert the qb onto the queue
		*/
		bzero ( qb_ptr, sizeof ( QB ) );
		bzero ( car_ptr, sizeof ( CARRIER ) );
		np_status = np_insert_da_q (
				&(xza_softc->ab->dafq.tail_ptr),
				car_ptr,
				qb_ptr );
		if ( np_status != NP_SUCCESS )
		{
		    skz_status = SKZ_QUEUE_INSERT_ERROR;
		    break;
		}

		/*
		** Update the CQIB
		*/
		kern_status = kvtop ( car_ptr, &phys_addr );
		if ( kern_status != KERN_SUCCESS )
		    return ( SKZ_PHYS_CONV_ERR );

		xza_softc -> cqib -> afqir = phys_addr >> 5;

		/*
		** Write to the QIR Register
		*/
		skz_status = skz_write_reg ( QIR, NULL, NULL, xza_softc );

		break;

	case DDFQ:
		/*
		** Insert the qb onto the queue
		*/
		bzero ( qb_ptr, sizeof ( QB ) );
		bzero ( car_ptr, sizeof ( CARRIER ) );
		np_status = np_insert_dd_q (
				&(xza_softc->ddfq.tail_ptr),
				car_ptr,
				qb_ptr );
		if ( np_status != NP_SUCCESS )
		    skz_status = SKZ_QUEUE_INSERT_ERROR;

		break;

	case DDEQ:
		/*
		** Insert the qb onto the queue
		*/
		np_status = np_insert_dd_q (
				&(xza_softc->ddeq.tail_ptr),
				car_ptr,
				qb_ptr );
		if ( np_status != NP_SUCCESS )
		    skz_status = SKZ_QUEUE_INSERT_ERROR;

		break;

	case DDEFQ:
		/*
		** Insert the qb onto the queue
		*/
		bzero ( qb_ptr, sizeof ( SKZ_ERR_BUF ) );
		bzero ( car_ptr, sizeof ( CARRIER ) );
		np_status = np_insert_dd_q (
				&(xza_softc->ddefq.tail_ptr),
				car_ptr,
				qb_ptr );
		if ( np_status != NP_SUCCESS )
		    skz_status = SKZ_QUEUE_INSERT_ERROR;

		break;

	case DDSGQ:
		bzero ( car_ptr, sizeof ( CARRIER ) );
		np_status = np_insert_dd_q (
				&(xza_softc->ddsgq.tail_ptr),
				car_ptr,
				qb_ptr );
		if ( np_status != NP_SUCCESS )
		    skz_status = SKZ_QUEUE_INSERT_ERROR;

		break;

	default:
		skz_status = SKZ_INVALID_QUEUE_TYPE;

    } /* switch */

    /*
    ** Unlock the xza_softc structure
    */
    SKZ_UNLOCK ( xza_softc -> lock, s );


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nLeaving %s", module) );

    return ( skz_status );
}

/************************************************************************
 *
 *  ROUTINE NAME:  skz_get_queue()
 *
 *  FUNCTIONAL DESCRIPTION:
 * 	This routine takes a queue id and removes an entry from the head
 * 	of the queue.  It returns a pointer to the removed Q_Buffer.
 *
 *  FORMAL PARAMETERS:
 *	queue_id - name of queue to be taken from
 *	qb_ptr - a pointer to a qb_ptr pointer to hold the address of the 
 *		 remove qb
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION: The DME seg queue stuff should get added here 
 * 			    and removed from skz_dme.c
 *
 ************************************************************************/

skz_get_queue ( QUEUE_NAMES	queue,
		QB** 		qb_ptr,
		CARRIER**	car_ptr,
		XZA_SOFTC*	xza_softc )

{
    u_int		skz_status = CAM_REQ_CMP;
    u_int		s;
    NP_STATUS		np_status;


    SIM_MODULE(skz_get_queue);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nEntering %s", module) );

    SKZ_LOCK ( xza_softc -> lock, s );

    switch ( queue ) {

	case ADRQ: 
  	    /*
	    ** Remove qb from adapter-driver response queue
	    */
	    np_status = np_rem_drv_q_entry (
				&(xza_softc->ab->adrq.head_ptr),
				car_ptr,
				qb_ptr );
	    if ( np_status != NP_SUCCESS )
		skz_status = SKZ_QUEUE_REMOVE_ERROR;

	    break;
	
	case DDFQ:
	    /*
	    ** Remove qb from driver-driver free queue
	    */
	    np_status = np_rem_drv_q_entry (
				&(xza_softc->ddfq.head_ptr),
				car_ptr,
				qb_ptr );
	    if ( np_status != NP_SUCCESS )
		skz_status = SKZ_QUEUE_REMOVE_ERROR;

	    break;
	
	case DDEQ:
	    /*
	    ** Remove qb from driver-driver error queue
	    */
	    np_status = np_rem_drv_q_entry (
				&(xza_softc->ddeq.head_ptr),
				car_ptr,
				qb_ptr );
	    if ( np_status != NP_SUCCESS )
		skz_status = SKZ_QUEUE_REMOVE_ERROR;

	    break;
	
	case DDEFQ:
	    /*
	    ** Remove qb from driver-driver error free queue
	    */
	    np_status = np_rem_drv_q_entry (
				&(xza_softc->ddefq.head_ptr),
				car_ptr,
				qb_ptr );
	    if ( np_status != NP_SUCCESS )
		skz_status = SKZ_QUEUE_REMOVE_ERROR;

	    break;

	case DDSGQ:

	    /*
	    ** Remove qb from driver-driver scatter-gather queue
	    */
	    np_status = np_rem_drv_q_entry (
				&(xza_softc->ddsgq.head_ptr),
				car_ptr,
				qb_ptr );
	    if ( np_status != NP_SUCCESS )
		skz_status = SKZ_QUEUE_REMOVE_ERROR;

	    break;

	case ADFQ: 
  	    /*
	    ** Remove qb from adapter-driver response queue
	    */
	    np_status = np_rem_drv_q_entry (
				&(xza_softc->ab->adfq.head_ptr),
				car_ptr,
				qb_ptr );
	    if ( np_status != NP_SUCCESS )
		skz_status = SKZ_QUEUE_REMOVE_ERROR;

	    break;

	default:
	    skz_status = SKZ_INVALID_QUEUE_TYPE;

    }

    SKZ_UNLOCK ( xza_softc -> lock, s );


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nLeaving %s", module) );

    return ( skz_status );
}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_resp_intr()
 *
 *  FUNCTIONAL DESCRIPTION:
 * 	Interrupts generated when the adapter places Q_Buffers onto the
 * 	response queue are handled by this routine.  
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

unsigned int
skz_resp_intr ( XZA_SOFTC* xza_softc )

{
    u_int	s;

    SIM_MODULE(skz_resp_intr);

    SIM_PRINTD ( 	NOBTL,
			NOBTL,
			NOBTL,
			CAMD_INOUT,
			("\nEntering %s", module) );

    /*
    ** Either wakeup the response thread, or just process the 
    ** interrupt, depending on whether we have threads yet or not
    */
#if SKZ_THREADED > 0
    if ( xza_softc -> flags & SKZ_RESP_TH )
	skz_clear_wait ( xza_softc -> resp_th, THREAD_AWAKENED );
    else
#endif
	skz_process_resp ( xza_softc );

    SIM_PRINTD ( 	NOBTL,
			NOBTL,
			NOBTL,
			CAMD_INOUT,
			("\nLeaving %s", module) );
}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_resp_th()
 *
 *  FUNCTIONAL DESCRIPTION:  If threads are active, this routine is awakened
 *	via thread_wakeup and processes the completed command in thread
 *	context.
 * 
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

unsigned int
skz_resp_th ()

{
    u_int	skz_status;
    u_int	s;

    XZA_SOFTC*	xza_softc;
    thread_t	thread;

    QB*		qb_ptr;
    CARRIER*	car_ptr;

    SIM_MODULE(skz_resp_th);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nEntering %s", module) );

    thread = current_thread();
    thread_swappable(thread, FALSE);

    /* Collect argument left by kernel_thread_w_arg() */
    xza_softc = (XZA_SOFTC *) thread -> reply_port;
    thread -> reply_port = PORT_NULL;

    /*
    ** When this thread gets created it also gets started..
    ** Thus, the first time we just need to get the xza_softc pointer
    ** and then block until we actually get a response interrupt from 
    ** the XZA
    */

    /*
    ** Check thread initialization, if the error thread and 
    ** this thread have initialized, set the THREADS_ACTIVE flag
    */
    SKZ_LOCK ( xza_softc -> lock, s );

    xza_softc -> flags |= SKZ_RESP_TH;	/* needed if we get here first */

    SKZ_UNLOCK ( xza_softc -> lock, s );

    /*
    ** Loop, processing responses as we get awakened by response interrupts
    */
    for ( ;; )
    {
	/*
	** we'll wait again after we've processed all of the 
	** responses on the response queue
	*/
	SKZ_LOCK ( xza_softc -> lock, s );

        if ( (np_drv_q_entry_present ( &(xza_softc -> ab -> adrq) )
                        != NP_SUCCESS) )
	    assert_wait((vm_offset_t)xza_softc, TRUE );

	SKZ_UNLOCK ( xza_softc -> lock, s );

	/*
	** Wait until the next response interrupt comes in
 	*/
	thread_block();

	if ( thread -> wait_result == THREAD_INTERRUPTED )
	    continue;

	SIM_PRINTD ( NOBTL,
		     NOBTL,
		     NOBTL,
		     CAMD_FLOW,
		     ("\n%s: Back from thread_block()", module) );

	/*
	** Process any responses that have arrived
	*/
	skz_process_resp ( xza_softc );

    }
}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_process_resp()
 *
 *  FUNCTIONAL DESCRIPTION:  This routine analyzes the Q Buffer and
 *	determines what the CAM status is, causes errors to be logged,
 *	calls the higher SIM layers to do the command completion, etc.,
 *	as appropriate based on the opcode field in the qb header.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

void 
skz_process_resp ( XZA_SOFTC* xza_softc )

{
    u_int	skz_status;

    QB*		qb_ptr;
    CARRIER*	car_ptr;

    SIM_MODULE(skz_process_resp);


    SIM_PRINTD (	NOBTL,
			NOBTL,
			NOBTL,
			CAMD_INOUT,
			("\nEntering %s", module) );

    /*
    ** Loop while there are responses on the response queue
    */
    while ( skz_get_queue ( ADRQ, &qb_ptr, &car_ptr, xza_softc ) 
				== CAM_REQ_CMP )
    {
	SIM_PRINTD ( NOBTL,
	 	 NOBTL,
	 	 NOBTL,
	 	 CAMD_FLOW,
	 	 ("\n%s: from get_queue, qb_ptr = %x, car_ptr = %x", 
		 module,
		 qb_ptr,
		 car_ptr) );

  	/*
	** Use skz_command_complete for each response
   	*/
	skz_command_complete ( qb_ptr, car_ptr );

    } /* while */

    SIM_PRINTD (	NOBTL,
			NOBTL,
			NOBTL,
			CAMD_INOUT,
			("\nLeaving %s", module) );

}

/************************************************************************
 *
 *  ROUTINE NAME:  skz_misc_intr()
 *
 *  FUNCTIONAL DESCRIPTION:
 * 	This routine is responsible for handling adapter miscellaneous
 * 	interrupts which occur as the result of state changes, bus or
 * 	device resets, and a variety of error conditions.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

unsigned int
skz_misc_intr ( XZA_CHANNEL*	xza_chan )

{
    u_int	skz_status;
    u_int	s;

    SIM_MODULE(skz_misc_intr);


    SIM_PRINTD ( 	NOBTL,
			NOBTL,
			NOBTL,
			CAMD_INOUT,
			("\nEntering %s", module) );

    /*
    ** If threads are active, just wakeup the misc_th, otherwise
    ** call skz_process_misc directly
    */
#if SKZ_THREADED > 0
    if ( xza_chan -> flags & SKZ_THREADS_ACTIVE )
	skz_clear_wait ( xza_chan -> misc_th, THREAD_AWAKENED );
    else
#endif
	skz_process_misc ( xza_chan );


    SIM_PRINTD ( 	NOBTL,
			NOBTL,
			NOBTL,
			CAMD_INOUT,
			("\nLeaving %s", module) );
}



/************************************************************************
 *
 *  ROUTINE NAME:  skz_misc_th()
 *
 *  FUNCTIONAL DESCRIPTION:  If threads are active, this routine is awakened
 *	via thread_wakeup from skz_misc_intr to process the miscellaneous
 *	interrupt.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

unsigned int
skz_misc_th ()

{
    u_int		skz_status;
    u_int		s;

    XZA_CHANNEL*	xza_chan;

    thread_t		thread;

    SIM_MODULE(skz_misc_th);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nEntering %s", module) );

    thread = current_thread();
    thread_swappable(thread, FALSE);

    /* Collect argument left by kernel_thread_w_arg() */
    xza_chan = (XZA_CHANNEL *) thread -> reply_port;

    thread -> reply_port = PORT_NULL;

    /*
    ** Set thread active flag
    */
    SKZ_LOCK ( xza_chan -> lock, s );
    xza_chan -> flags |= SKZ_THREADS_ACTIVE;
    SKZ_UNLOCK ( xza_chan -> lock, s );

    /*
    ** Loop, handling misc interrupts when awakened by a
    ** miscellaneous interrupt
    */
    /*
    ** When this thread gets created it also gets started..
    ** Thus the first time it runs it just needs to get the
    ** xza_chan pointer and then block until a real interrupt
    ** occurs and restarts it.
    */

    for ( ;; )
    {
	/*
	** Wait for the next interrupt
	*/
        SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_FLOW,
		 ("\n%s: Calling thread_block", module) );

	assert_wait((vm_offset_t)xza_chan, TRUE );

	thread_block();

	if ( thread -> wait_result == THREAD_INTERRUPTED )
	    continue;

	SIM_PRINTD (	NOBTL,
			NOBTL,
			NOBTL,
			CAMD_FLOW,
			("\n%s: back from thread_block") );

	/*
	** Process the interrupt
	*/
	skz_process_misc ( xza_chan );

    } /* for ( ;; ) */

}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_process_misc()
 *
 *  FUNCTIONAL DESCRIPTION:
 * 	This routine is responsible for handling adapter miscellaneous
 * 	interrupts which occur as the result of state changes, bus or
 * 	device resets, and a variety of error conditions.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

void 
skz_process_misc ( XZA_CHANNEL* xza_chan )

{
    u_int		skz_status;
    u_int		s;
    u_int		i;

    ASR_REG		asr;
    XPD1_REG		xpd1;

    QB*			qb_ptr;
    CARRIER*		car_ptr;

    XZA_SOFTC*		xza_softc;

    SIM_MODULE(skz_process_misc);

 
    SIM_PRINTD (	NOBTL,
			NOBTL,
			NOBTL,
			CAMD_INOUT,
			("\nEntering %s", module) );

    xza_softc = (XZA_SOFTC *) xza_chan -> xza_softc;

    /*
    ** Setup up XPD1 for ASRCR emulation
    */
    bzero ( &xpd1, sizeof ( XPD1_REG ) );
    xpd1.rcode = ASRCR_EMULATION;

    /*
    ** Read the contents of the adapter status register
    */
    SIM_PRINTD (NOBTL,
		    NOBTL,
		    NOBTL,
		    CAMD_FLOW,
		    ("\n%s: Reading the ASR", module) );

    skz_status = skz_read_reg ( ASR, &asr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error (	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );	
	return; 
    }

    SIM_PRINTD (NOBTL,
		    NOBTL,
		    NOBTL,
		    CAMD_FLOW,
		    ("\n%s: ASR = %x", module, asr) );

    /*
    ** If the asr<unin> bit is set, the ASR is really the XPUD, and this
    ** routine doesn't handle XPUD parsing.  This check is needed since
    ** we call skz_process_misc() in poll mode, when the XZA may have gotten
    ** reset..
    */
    if ( asr.unin )
    {
	/*
	** Set new state
	*/
	SKZ_LOCK ( xza_softc -> lock, s );
	xza_softc -> channel[0].state = UNINITIALIZED;
	xza_softc -> channel[1].state = UNINITIALIZED;
	SKZ_UNLOCK ( xza_softc -> lock, s );

	return;
    }

    /*
    ** check for adapter free queue error
    */
    if ( asr.afqe )
    {
	/*
	** The adapter needs free queue entries
	** Take some from the driver free queue and put them on the 
   	** adapter free queue
	*/
	for ( i = 0; i < SKZ_ADFQ_ENTRIES; i++ )
	{
	    skz_status = skz_get_queue ( DDFQ, &qb_ptr, &car_ptr, xza_softc );
	    if ( skz_status != CAM_REQ_CMP )
	        skz_error (	module,
				SKZ_ERROR,
				skz_status,
				NULL,
				xza_softc );

	    skz_status = skz_put_queue ( DAFQ, qb_ptr, car_ptr, xza_softc );
	    if ( skz_status != CAM_REQ_CMP )
	        skz_error (	module,
				SKZ_ERROR,
				skz_status,
				NULL,
				xza_softc );
	}
    }

    /*
    ** Check for adapter maintenance errors
    */
    else if ( asr.ame )
    {
	/*
	** Set new state
	*/
	SKZ_LOCK ( xza_softc -> lock, s );
	xza_softc -> channel[0].state = UNINITIALIZED;
	xza_softc -> channel[1].state = UNINITIALIZED;
	SKZ_UNLOCK ( xza_softc -> lock, s );

	/*
	** log asr.amec error 
	*/
	skz_error ( module, 
		    SKZ_AME_ERROR, 
		    asr.accamec, 
		    NULL,
		    xza_chan );
    }

    /*
    ** Check for abnormal condition
    */
    else if ( asr.ac )
    {
	/*
	** Set new state
	*/
	SKZ_LOCK ( xza_softc -> lock, s );
	xza_chan -> state = DISABLED;
	SKZ_UNLOCK ( xza_softc -> lock, s );

	/*
	** Act upon the abnormal condition code
	*/
	skz_error ( module, 
		    SKZ_ACC_ERROR, 
		    asr.accamec, 
		    NULL, 
		    xza_chan );
    }

    /*
    ** Check for data structure errors
    */
    else if ( asr.dse )
    {
	/*
	** Set new state
	*/
	SKZ_LOCK ( xza_softc -> lock, s );
	xza_chan -> state = DISABLED;
	SKZ_UNLOCK ( xza_softc -> lock, s );

	/*
	** Log error
	*/
	skz_error ( module,
	    	    SKZ_DSE_ERROR, 
		    asr.adsec, 
		    NULL,
		    xza_chan );
    }

    /*
    ** Check for state transitions
    */
    else
    {
	/*
	** Check for channel initialization complete
	*/
	if ( asr.cic )
	{
            SIM_PRINTD (NOBTL,
		 	NOBTL,
		 	NOBTL,
		 	CAMD_FLOW,
		 	("\n%s: Got a CIC Interrupt", module) );

	    /*
	    ** Set the new channel state
	    */
	    SKZ_LOCK ( xza_softc -> lock, s );
	    xza_chan -> state = DISABLED;
	    SKZ_UNLOCK ( xza_softc -> lock, s );

	    /*
	    ** If we've reset the bus, call reset_complete now
	    */
	    if ( ( xza_chan -> flags & SKZ_BUS_RESET_IN_PROGRESS ) ||
	         !( xza_softc -> flags & SKZ_ADAPT_RESET_IN_PROGRESS ) )
	    {	
		/*
		** Set reset in progress, in case it's not set..
		*/
		SKZ_LOCK ( xza_softc -> lock, s );
		xza_chan -> flags |= SKZ_BUS_RESET_IN_PROGRESS;
		SKZ_UNLOCK ( xza_softc -> lock, s );
	        /*
	        ** Log successfull completion of the reset.. this will also 
	        ** cause the channel to be re-enabled
	        */
	        skz_error (	module,
			    	SKZ_ACC_ERROR,
				XZA_ACC_BUS_RESET_RECEIVED,
				NULL,
				xza_chan );
	    }

    	    /*
	    ** Wakeup up the user thread that's waiting for
	    ** the state change to complete, if there is one.
	    */
	    else if ( xza_chan -> flags & SKZ_THREADS_ACTIVE )
	        skz_thread_wakeup((vm_offset_t)&(xza_chan->state),
							THREAD_AWAKENED);

	}

	/*
	** Check for channel enable complete
	*/
	if ( asr.cec )
	{
            SIM_PRINTD (NOBTL,
		 	NOBTL,
		 	NOBTL,
		 	CAMD_FLOW,
		 	("\n%s: Got CEC interrupt", module) );

	    /*
	    ** Set the new channel state
	    */
	    SKZ_LOCK ( xza_softc -> lock, s );
	    xza_chan -> state = ENABLED;
	    SKZ_UNLOCK ( xza_softc -> lock, s );

    	    /*
	    ** Wakeup up the user thread that's waiting for
	    ** the state change to complete
	    */
	    if ( xza_chan -> flags & SKZ_THREADS_ACTIVE )
	        skz_thread_wakeup((vm_offset_t)&(xza_chan -> state), 
				    THREAD_AWAKENED);

	}
    }

    /*
    ** Write to the ASRCR register to release the interrupt 
    ** ( moved to the end of the loop so that the registers
    **   will be latched until after we call skz_error )
    */

    /*
    ** Emulate write to ASRCR
    */
    skz_status = skz_write_reg ( ASRCR, xpd1, NULL, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	skz_error (	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );

    return;

}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_emul_complete()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function waits for an N_Port register emulation to complete.
 *	Many of the register writes are emulations of N_Port registers,
 *	using XPD1 and XPD2.  XZA firmware requires that the port driver
 *	wait until XPD1 and XPD2 are cleared, indicating that register
 *	emulation is complete, before performing another register
 *	emulation ( writing XPD1 or XPD2 again ).  Registers which are
 *	emulated include CBRCR ( bus resets ), AICR ( adapter initialization
 *	control reg ), AECR ( adapter enable control reg ) , etc.
 *
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

skz_emul_complete ( XZA_SOFTC* xza_softc )

{
    u_int 		skz_status;
    u_int		delay;
    vm_offset_t		wait_event = 0;

    u_int		xpd1;
    u_int		xpd2;

    SIM_MODULE(skz_emul_complete);


    /*
    ** Wait for emulation to complete, indicated by firmware clearing XPD1
    ** and XPD2
    */
    delay = 0;

    skz_status = skz_read_reg ( XPD1, &xpd1, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error (	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
	return ( SKZ_EMUL_ERR );
    }
/*
** 
** We can get away with only checking XPD1
**
    skz_status = skz_read_reg ( XPD2, &xpd2, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error (	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
	return ( SKZ_EMUL_ERR );
    }
*/

    while ( ( xpd1 ) && ( delay < XZA_EMUL_TIME ) )
    {
	DELAY ( SKZ_ONE_SECOND_DELAY / 10 );
	delay++;

	skz_status = skz_read_reg ( XPD1, &xpd1, xza_softc );
	if ( skz_status != CAM_REQ_CMP )
	{
	    skz_error (	module,
		      	SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
	    return ( SKZ_EMUL_ERR );
	}
/*
            skz_status = skz_read_reg ( XPD2, &xpd2, xza_softc );
            if ( skz_status != CAM_REQ_CMP )
	    {
	    	skz_error (	module,
				SKZ_ERROR,
				skz_status,
				NULL,
				xza_softc );
	    	return ( SKZ_EMUL_ERR );
	    }
*/
    }

    /*
    ** Either the time is up or the registers got cleared
    */
    if ( xpd1 )
    {
	skz_error ( module,
		    SKZ_ERROR,
		    SKZ_EMUL_TIMEOUT_ERR,
		    NULL,
		    xza_softc );
	return ( SKZ_EMUL_ERR );
    }
    else
	return ( CAM_REQ_CMP );
}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_read_reg()
 *
 *  FUNCTIONAL DESCRIPTION:
 * 	This routine takes a register id and returns the
 * 	value contained in the register.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

skz_read_reg (     unsigned int reg,
		   unsigned int* value,
		   XZA_SOFTC*	xza_softc )

{
    u_int	skz_status = CAM_REQ_CMP;
    u_int	s;

    SIM_MODULE(skz_read_reg);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nEntering %s", module) );

    /*
    ** We need to be sure that all mailbox accesses are done at the
    ** same ipl.  Just to be safe and very clear about it, set the ipl
    ** to SPLBIO here.
    */
    s = splbio();

    /*
    ** Use the mailbox interface to read the requested register
    **
    ** The bus structure for the xza has the pointer to the mailbox.
    ** The xza_reg_addrs structure contains all of the register addresses.
    */
    switch ( reg ) {

	case AFAR1:
	    /* 
	    ** Read AFAR1 register
	    */
	    *value = (int) RDCSR(	LONG_32, 
					xza_softc -> xza_bus, 
					xza_softc -> xza_reg_addrs.afar1 );
	    break;

	case AFAR0:
	    /* 
	    ** Read AFAR0 register
	    */
	    *value = (int) RDCSR(	LONG_32, 
					xza_softc -> xza_bus, 
					xza_softc -> xza_reg_addrs.afar0 );
	    break;

	case ASR:
	    /* 
	    ** Read ASR register
	    */
	    *value = (int) RDCSR(	LONG_32, 
					xza_softc -> xza_bus, 
					xza_softc -> xza_reg_addrs.asr );
	    break;

	case XBE:
	    /* 
	    ** Read XBE register
	    */
	    *value = (int) RDCSR(	LONG_32, 
					xza_softc -> xza_bus, 
					xza_softc -> xza_reg_addrs.xbe );
	    break;

	case XDEV:
	    /* 
	    ** Read XDEV register
	    */
	    *value = (int) RDCSR(	LONG_32, 
					xza_softc -> xza_bus, 
					xza_softc -> xza_reg_addrs.xdev );
	    break;

	case XFADR:

	    *value = (int) RDCSR(	LONG_32, 
					xza_softc -> xza_bus, 
					xza_softc -> xza_reg_addrs.xfadr );
	    break;

	case XFAER:

	    *value = (int) RDCSR(	LONG_32, 
					xza_softc -> xza_bus, 
					xza_softc -> xza_reg_addrs.xfaer );
	    break;

	case XPD1:
	    /* 
	    ** Read xpd1 register
	    */
	    *value = (int) RDCSR(	LONG_32, 
					xza_softc -> xza_bus, 
					xza_softc -> xza_reg_addrs.xpd1 );
	    break;

	case XPD2:
	    /* 
	    ** Read xpd2 register
	    */
	    *value = (int) RDCSR(	LONG_32, 
					xza_softc -> xza_bus, 
					xza_softc -> xza_reg_addrs.xpd2 );
	    break;

	case XPUD:

	    /* 
	    ** Read xpud register
	    */
	    *value = (int) RDCSR(	LONG_32, 
					xza_softc -> xza_bus, 
					xza_softc -> xza_reg_addrs.xpud );
	    break;

	/*
	** The are "write-only" registers
	*/ 
	case AECR:
	case AICR:
	case ASRCR:
	case CBRCR:
	case CECR:
	case CICR:
	case AMCSR:
	case GCQ0IR:
	case GCQ1IR:
	case NRE:
	case QIR:
	case XCOMM:
	default:
	    skz_status = SKZ_INVALID_REGISTER;	
	    break;

    }	 /* switch */

    /*
    ** Set the ipl back to what it was before we got in here
    */
    splx(s);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nLeaving %s", module) );


    return ( skz_status );
}

/************************************************************************
 *
 *  ROUTINE NAME:  skz_write_reg()
 *
 *  FUNCTIONAL DESCRIPTION:
 * 	This routine takes a register id and a value and writes the value
 * 	to the register.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/


skz_write_reg (	    unsigned int reg,
		    unsigned int value1,
		    unsigned int value2,
		    XZA_SOFTC* xza_softc )

{
    u_int	skz_status = CAM_REQ_CMP;
    u_int	s;

    SIM_MODULE(skz_write_reg);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nEntering %s", module) );

    /*
    ** We need to make sure that all register accesses are performed
    ** at the same ipl.  To make sure of this, set the ipl here to 
    ** SPLBIO
    */
    s = splbio();

    /*
    ** Write to the requested register
    */
    switch ( reg ) {

	case AECR:
	case CECR:
	    /*
	    ** Check for register emulation complete
	    */
	    skz_status = skz_emul_complete ( xza_softc );
	    if ( skz_status != CAM_REQ_CMP )
		skz_status = SKZ_REG_WRITE_ERR;

	    /*
	    ** Write value1 to XPD1
	    */
	    WRTCSR(	LONG_32, 
			xza_softc -> xza_bus, /* CHECK!!!!! */
			xza_softc -> xza_reg_addrs.xpd1, 
			value1 );
	    /*
	    ** Write a zero to NRE
	    */
   	    mb();
	    WRTCSR(	LONG_32, 
			xza_softc -> xza_bus, 
			xza_softc -> xza_reg_addrs.nre, 
			0 );
	    mb();

	    break;

	case AICR:
	case CICR:
	    /*
	    ** Check for register emulation complete
	    */
	    skz_status = skz_emul_complete ( xza_softc );
	    if ( skz_status != CAM_REQ_CMP )
		skz_status = SKZ_REG_WRITE_ERR;

	    /*
	    ** Write value1 to XPD1 
	    */
	    WRTCSR(	LONG_32, 
			xza_softc -> xza_bus, 
			xza_softc -> xza_reg_addrs.xpd1, 
			value1 );
	    mb();
	    /* 
	    ** Write value2 to XPD2
	    */
	    WRTCSR(	LONG_32, 
			xza_softc -> xza_bus, 
			xza_softc -> xza_reg_addrs.xpd2, 
			value2 );
	    mb();

	    break;

	case ASRCR:
	    /*
	    ** Write value1 to XPD1 
	    */
	    WRTCSR(	LONG_32, 
			xza_softc -> xza_bus, 
			xza_softc -> xza_reg_addrs.xpd1, 
			value1 );

	    /* 
	    ** Write to NRE
	    */
	    mb();
	    WRTCSR(	LONG_32, 
			xza_softc -> xza_bus, 
			xza_softc -> xza_reg_addrs.nre, 
			0 );

	    mb();

	    break;

	case CBRCR:
	    /*
	    ** Check for register emulation complete
	    */
	    skz_status = skz_emul_complete ( xza_softc );
	    if ( skz_status != CAM_REQ_CMP )
		skz_status = SKZ_REG_WRITE_ERR;

	    /*
	    ** Write value1 to XPD1 
	    */
	    WRTCSR(	LONG_32, 
			xza_softc -> xza_bus, 
			xza_softc -> xza_reg_addrs.xpd1, 
			value1 );

	    /* 
	    ** Write to NRE
	    */
	    mb();
	    WRTCSR(	LONG_32, 
			xza_softc -> xza_bus, 
			xza_softc -> xza_reg_addrs.nre, 
			0 );

	    mb();

	    break;

	case AMCSR:
	    /*
	    ** Check for register emulation complete
	    */
	    skz_status = skz_emul_complete ( xza_softc );
	    if ( skz_status != CAM_REQ_CMP )
		skz_status = SKZ_REG_WRITE_ERR;

	    /*
	    ** Write value1 to XPD1 
	    */
	    WRTCSR(	LONG_32, 
			xza_softc -> xza_bus, 
			xza_softc -> xza_reg_addrs.xpd1, 
			value1 );

	    /* 
	    ** Write value2 to XPD2
	    */
	    mb();
	    WRTCSR(	LONG_32, 
			xza_softc -> xza_bus, 
			xza_softc -> xza_reg_addrs.xpd2, 
			value2 );

	    mb();

	    break;

	case GCQ0IR:
	    /* 
	    ** Write value1 to GCQ0IR
	    */
	    WRTCSR(	LONG_32, 
			xza_softc -> xza_bus, 
			xza_softc -> xza_reg_addrs.gcq0ir, 
			value1 );
	    mb();

	    break;

	case GCQ1IR:
	    /* 
	    ** Write value1 to GCQ0IR
	    */
	    WRTCSR(	LONG_32, 
			xza_softc -> xza_bus, 
			xza_softc -> xza_reg_addrs.gcq1ir, 
			value1 );
	    mb();

	    break;

	case QIR:
	    /* 
	    ** Write value1 to GCQ1IR
	    */
	    WRTCSR(	LONG_32, 
			xza_softc -> xza_bus, 
			xza_softc -> xza_reg_addrs.qir, 
			value1 );

	    mb();

	    break;

	case NRE:
	    /* 
	    ** Write value1 to NRE
	    */
	    WRTCSR(	LONG_32, 
			xza_softc -> xza_bus, 
			xza_softc -> xza_reg_addrs.nre, 
			value1 );

	    mb();

	    break;

	case XBE:
	    /* 
	    ** Write value1 to XBE
	    */
	    WRTCSR(	LONG_32, 
			xza_softc -> xza_bus, 
			xza_softc -> xza_reg_addrs.xbe, 
			value1 );

	    mb();

	    break;

	case XPD1:
	    /* 
	    ** Write value1 to XPD1
	    */
	    WRTCSR(	LONG_32, 
			xza_softc -> xza_bus, 
			xza_softc -> xza_reg_addrs.xpd1, 
			value1 );

	    mb();

	    break;

	case XPD2:
	    /* 
	    ** Write value1 to XPD2
	    */
	    WRTCSR(	LONG_32, 
			xza_softc -> xza_bus, 
			xza_softc -> xza_reg_addrs.xpd2, 
			value1 );

	    mb();

	    break;

	/*
	** These are "read-only" registers
	*/
	case AFAR0:
	case AFAR1:
	case ASR:
	case XCOMM:
	case XDEV:
	case XFADR:
	case XFAER:
	case XPUD:
	default:
	    skz_status = SKZ_INVALID_REGISTER;
	    break;

    }	 /* switch */

    /*
    ** Set the ipl back to what it was when we entered here
    */
    splx(s);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nLeaving %s", module) );


    return ( skz_status );
}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_bus_reset_complete()
 *
 *  FUNCTIONAL DESCRIPTION:
 * 	This routine calls the SIMS function ss_reset_detected when 
 *	a bus reset has completed. It is called from the 
 * 	skz_process_misc routine.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

void
skz_bus_reset_complete ( XZA_CHANNEL* xza_chan )

{
    u_int		skz_status = CAM_REQ_CMP;
    u_int		s;
    u_int		chan_mask = 0;


    SIM_MODULE(skz_bus_reset_complete);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nEntering %s", module) );

    /*
    ** Clear out the driver-channel command queues of any pending
    ** commands not yet de-queued by the adapter
    */
    skz_clear_command_queues ( xza_chan );

    /*
    ** Re-enable the channel, it transitioned to DISABLED when
    ** we wrote the CBRCR to reset the bus
    */
    skz_status = skz_enable_channel ( (XZA_SOFTC *) xza_chan -> xza_softc, 
					  1 << xza_chan -> chan_num  );
    if ( skz_status != CAM_REQ_CMP )
    {
        skz_error (  module,
                     SKZ_ERROR,
                     skz_status,
                     NULL,
                     xza_chan -> xza_softc );
	return;
    }

    /*
    ** Clear the reset flags
    */
    SKZ_LOCK ( xza_softc -> lock, s );
    xza_chan -> flags &= ~SKZ_BUS_RESET_IN_PROGRESS;
    xza_chan -> flags &= ~SKZ_RESET_NEEDED;
    SKZ_UNLOCK ( xza_softc -> lock, s );

    /*
    ** Call back to sim_sched to clear nexus queues and call
    ** back to registered peripheral drivers
    */
    if ( ( xza_chan -> flags & SKZ_ALIVE ) &&
	 ( xza_chan -> sim_softc -> sims_init == CAM_TRUE ) &&
	 ( xza_chan -> sim_softc -> simx_init == CAM_TRUE ) )
	ss_reset_detected ( xza_chan -> sim_softc );

    /*
    ** Make sure anything that's waiting gets scheduled
    */
    SC_SCHED_RUN_SM ( xza_chan -> sim_softc );


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nLeaving %s", module) );

    return;
}

/************************************************************************
 *
 *  ROUTINE NAME:  skz_clear_command_queues()
 *
 *  FUNCTIONAL DESCRIPTION:  This routine clears unprocessed Q Buffers
 *	from driver-channel command queues when a bus or the adapter
 *	gets reset.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

void
skz_clear_command_queues ( XZA_CHANNEL* xza_chan )

{
    u_int	skz_status;
    NP_STATUS	np_status;
    u_int	s;

    QB*		qb_ptr;
    CARRIER*	car_ptr;
    NP_Q*	q_head_ptr;

    XZA_SOFTC*	xza_softc;
    struct dcc_queues	command_queues;

    SIM_MODULE(skz_clear_command_queues);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nEntering %s", module) );

    xza_softc = (XZA_SOFTC *) xza_chan -> xza_softc;

    /*
    ** Clear out any commands which have been placed on the channel's
    ** command queues but which have not been de-queued by the adapter.
    */
    command_queues = xza_softc -> ab -> dccq[xza_chan -> chan_num];

    /* 
    ** Clear DCCQ0
    */
    q_head_ptr = &(command_queues.dccq0);

    np_status = np_rem_adapt_q_entry (	&q_head_ptr,
					&car_ptr,
					&qb_ptr );
    while ( np_status == NP_SUCCESS )
    {
	/*
	** do the completion on the ccb
	*/
	qb_ptr -> qb_header.np_qb_hdr.status.fail = 1;
	qb_ptr -> qb_header.np_qb_hdr.status.type = XZA_BUS_WAS_RESET;
	(void) skz_command_complete ( qb_ptr, car_ptr );

        np_status = np_rem_adapt_q_entry (	&q_head_ptr,
						&car_ptr,
						&qb_ptr );
    }

    /* 
    ** Clear DCCQ1
    */
    q_head_ptr = &(command_queues.dccq1);

    np_status = np_rem_adapt_q_entry (	&q_head_ptr,
					&car_ptr,
					&qb_ptr );
    while ( np_status == NP_SUCCESS )
    {
	/*
	** do the completion on the ccb
	*/
	qb_ptr -> qb_header.np_qb_hdr.status.fail = 1;
	qb_ptr -> qb_header.np_qb_hdr.status.type = XZA_BUS_WAS_RESET;
	(void) skz_command_complete ( qb_ptr, car_ptr );

        np_status = np_rem_adapt_q_entry (	&q_head_ptr,
						&car_ptr,
						&qb_ptr );
    }
    
    /* 
    ** Clear DCCQ2
    */
    q_head_ptr = &(command_queues.dccq2);

    np_status = np_rem_adapt_q_entry (	&q_head_ptr,
					&car_ptr,
					&qb_ptr );
    while ( np_status == NP_SUCCESS )
    {
	/*
	** do the completion on the ccb
	*/
	qb_ptr -> qb_header.np_qb_hdr.status.fail = 1;
	qb_ptr -> qb_header.np_qb_hdr.status.type = XZA_BUS_WAS_RESET;
	(void) skz_command_complete ( qb_ptr, car_ptr );

        np_status = np_rem_adapt_q_entry (	&q_head_ptr,
						&car_ptr,
						&qb_ptr );
    }


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nLeaving %s", module) );

    return;
}




/************************************************************************
 *
 *  ROUTINE NAME:  skz_start_device()
 *
 *  FUNCTIONAL DESCRIPTION:
 * 	This routine restarts a device after a device reset by issuing
 * 	a SETNEX XZA command.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

skz_start_device ( XZA_CHANNEL* xza_chan, SIM_WS* sws )

{
    u_int		skz_status;
    u_int		delay;

    QB*			qb_ptr;
    CARRIER*		car_ptr;

    NP_QB_HEADER*	qb_hdr_ptr;
    SETNEX_CMD*		qb_body_ptr;
    QB_DRIVER*  	qb_driver_ptr;

    XZA_SOFTC*		xza_softc;
    XZA_SCSI_TARGET*	xza_target;

    SIM_MODULE(skz_start_device);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nEntering %s", module) );

    xza_softc = (XZA_SOFTC *) xza_chan -> xza_softc;

    /*
    ** Get a Q_Buffer from the free queue
    */
    skz_status = skz_get_queue ( DDFQ, &qb_ptr, &car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error (	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
	return ( CAM_REQ_CMP_ERR );
    }

    qb_hdr_ptr = (NP_QB_HEADER *) &( qb_ptr -> qb_header );
    qb_body_ptr = (SETNEX_CMD *) ( qb_ptr -> qb_body );
    qb_driver_ptr = (QB_DRIVER *) &( qb_ptr -> qb_driver );

    /*
    ** Initialize for a SETNEX command
    */
    qb_hdr_ptr -> opc = SETNEX;
    qb_hdr_ptr -> chnl_idx = xza_chan -> chan_num;
    qb_hdr_ptr -> flags.r = 1;			/* always get response */
    qb_hdr_ptr -> dst_xport.targid = sws -> targid;
    qb_hdr_ptr -> dst_xport.lun = sws -> lun;
    qb_hdr_ptr -> src_xport.targid = xza_chan -> scsi_id.targid;
    qb_hdr_ptr -> src_xport.lun = 0;

    /*
    ** Initialize for nexus entry within the XZA.. just go ahead and
    ** and clear qfroz, bdr, and tmo for all luns via the tgt bit
    */
    qb_body_ptr -> mask.qfroz = 1;
    qb_body_ptr -> mask.bdr = 1;
    qb_body_ptr -> mask.tmo = 1;
    qb_body_ptr -> mask.tgt = 1;

    qb_body_ptr -> m_value.qfroz = 0;
    qb_body_ptr -> m_value.bdr = 0;
    qb_body_ptr -> m_value.tmo = 0;
    qb_body_ptr -> m_value.tgt = 1;


    qb_driver_ptr -> sws = (vm_offset_t) sws;
    qb_driver_ptr -> xza_chan_ptr = (vm_offset_t) xza_chan;

    /*
    ** Insert Q_Buffer onto the highest priority command queue
    */
    skz_status = skz_put_queue ( DCCQ2, qb_ptr, car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error (	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
	return ( skz_status );
    }

    /*
    ** If we don't have interrupts, poll for the SETNEX 
    ** completion
    */
    if ( cam_at_boottime() || shutting_down )
    {
	delay = 0;

	/*
	** Wait for the response to appear on the response queue
	*/	
	while ( (delay < XZA_RESPONSE_TIME) &&
		(np_drv_q_entry_present ( &(xza_softc -> ab -> adrq) )
			!= NP_SUCCESS) )
	{
	    /*
	    ** If an event occurred, process it
	    ** and return, because there'll be no "response"
	    ** if any of these events occurrs.
	    */
	    skz_process_misc ( xza_chan );

	    DELAY ( SKZ_ONE_SECOND_DELAY / 1000 );
	    delay++;
	}

	/*
	** Just attempt to process anything that might be there now,
	** and let skz_process_resp handle status, etc.
	*/
	if (np_drv_q_entry_present (&(xza_softc -> ab -> adrq)) == NP_SUCCESS)
	{
	    skz_process_resp ( xza_softc );
	}
	else
	{
	    /* set status only */
	    sws -> cam_status = CAM_PROVIDE_FAIL;
	}

    } /* if cam_at_bootime() */


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nLeaving %s", module) );


    return ( skz_status );

}
