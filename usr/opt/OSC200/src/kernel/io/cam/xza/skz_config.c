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
static char *rcsid = "@(#)$RCSfile: skz_config.c,v $ $Revision: 1.1.2.14 $ (DEC) $Date: 1992/11/05 17:31:15 $";
#endif

/************************************************************************
 *									*
 * File:	skz_config.c						*
 * Date:	December 30, 1991					*
 * Author:	Ron Hegli                                               *
 *									*
 * Description:								*
 *	This file contains the probe and attach routines, as well as    *
 * 	some supporting routines					*
 *									*
 ************************************************************************/
/*
** Include Files
*/
#include <sys/types.h>
#include <sys/lock.h>
#include <sys/time.h>

#include <vm/vm_kern.h>
#include <mach/vm_param.h>
#include <mach/kern_return.h>
#include <machine/rpb.h>
#include <machine/machparam.h>
#include <kern/thread.h>
#include <kern/task.h>
#include <kern/lock.h>

#include <io/common/devdriver.h>
#include <io/dec/xmi/xmireg.h>
#include <io/dec/mbox/mbox.h>

#include <io/common/iotypes.h>
#include <io/cam/cam.h>
#include <io/cam/dec_cam.h>
#include <io/cam/scsi_all.h>
#include <io/cam/cam_debug.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/dme.h>
#include <io/cam/sim_common.h>
#include <io/cam/sim.h>

#include <io/cam/pdrv.h>

#include <io/cam/xza/skz_params.h>
#include <io/cam/xza/skz_error.h>
#include <io/cam/xza/skz_xza.h>
#include <io/cam/xza/skz_nport.h>
#include <io/cam/xza/skz_xza_qb.h>
#include <io/cam/xza/skz.h>

/*
** Set up the XZA's SIM entry points
*/
extern int	sim_init();
extern int	sim_action();

/* Interrupt Routines */
long	skz_misc_intr();
long	skz_resp_intr();

/* Threads */
long	skz_misc_th();
long	skz_resp_th();
long	skz_error_th();

int	skz_reinit();
void	skz_thread_init ( XZA_SOFTC* );

extern task_t	first_task;
extern thread_t first_thread;

extern struct bus 		bus_list[];
extern struct controller 	controller_list[];
extern XZA_SOFTC* 		xza_softc_directory[];

struct controller* xza_get_ctlr ( char*, struct bus*, u_int );
struct bus* xza_get_bus ( char*, struct bus*, u_int );

CAM_SIM_ENTRY xza_entry = 
    {
	sim_init,
	sim_action
    };

extern unsigned int skz_hba_init();
extern unsigned int skz_go();
extern unsigned int skz_reset_bus();


/************************************************************************
 *
 *  ROUTINE NAME:  kxmsaprobe()
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
unsigned int
kzmsaprobe ( 	vm_offset_t	nxv,
		vm_offset_t	nxp,
		unsigned int	xmi_bus_num,
		unsigned int	xza_xmi_node,
		struct xmidata	*xmidata,
		struct bus	*xmibus )

{
    u_int 	skz_status;
    u_int 	i;
    u_int 	s;

    u_int	delay;

    XDEV_REG	xdev;
    XPUD_REG	xpud;
    XBE_REG	xbe;
    XPD1_REG	xpd1;
    XPD2_REG	xpd2;

    ASR_REG	asr; 

    struct bus 		*xza_bus;
    struct controller	*ctlr;

    XZA_SOFTC*	xza_softc;
    SIM_SOFTC*	sim_softc;

    int		lamb_xmi_node;

    struct xmisw *pxmisw = xmidata->xmierr[xza_xmi_node].pxmisw;

    extern char cam_ctlr_string[];

    SIM_MODULE(xzaprobe);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nThis is %s", module) );

    /*
    ** Find the "bus" structure for this adapter
    */
    xza_bus = xza_get_bus ( "xza", xmibus, xza_xmi_node );
    if ( xza_bus == 0 )
    {
	printf ( "xza at xmi%d node %d not configured!\n",
			xmi_bus_num, xza_xmi_node );
	return ( 0 );
    }

    /*
    ** Allocate xza_softc structure
    */
    xza_softc = (XZA_SOFTC *) kmem_alloc ( kernel_map, sizeof ( XZA_SOFTC ) );
    if ( xza_softc == NULL )
	return ( 0 );
	
    bzero ( xza_softc, sizeof ( XZA_SOFTC ) );

    /*
    ** Initialize register addresses
    */
    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_FLOW,
		 ("\n%s: Initializing Register Addresses", module) );

    xza_softc -> xza_base = nxp;

    xza_softc -> xza_reg_addrs.xdev	= xza_softc -> xza_base + XDEV_OFFSET;
    xza_softc -> xza_reg_addrs.xbe 	= xza_softc -> xza_base + XBE_OFFSET;
    xza_softc -> xza_reg_addrs.xfadr 	= xza_softc -> xza_base + XFADR_OFFSET;
    xza_softc -> xza_reg_addrs.xcomm 	= xza_softc -> xza_base + XCOMM_OFFSET;
    xza_softc -> xza_reg_addrs.xfaer 	= xza_softc -> xza_base + XFAER_OFFSET;
    xza_softc -> xza_reg_addrs.xpd1 	= xza_softc -> xza_base + XPD1_OFFSET;
    xza_softc -> xza_reg_addrs.xpd2 	= xza_softc -> xza_base + XPD2_OFFSET;
    xza_softc -> xza_reg_addrs.afar1 	= xza_softc -> xza_base + AFAR1_OFFSET;
    xza_softc -> xza_reg_addrs.nre 	= xza_softc -> xza_base + NRE_OFFSET;
    xza_softc -> xza_reg_addrs.afar0 	= xza_softc -> xza_base + AFAR0_OFFSET;
    xza_softc -> xza_reg_addrs.xpud 	= xza_softc -> xza_base + XPUD_OFFSET;
    xza_softc -> xza_reg_addrs.asr 	= xza_softc -> xza_base + ASR_OFFSET;
    xza_softc -> xza_reg_addrs.gcq0ir 	= xza_softc -> xza_base + GCQ0IR_OFFSET;
    xza_softc -> xza_reg_addrs.gcq1ir 	= xza_softc -> xza_base + GCQ1IR_OFFSET;
    xza_softc -> xza_reg_addrs.qir 	= xza_softc -> xza_base + QIR_OFFSET;

    xza_softc -> xza_bus = xza_bus;
    xza_softc -> xza_xmi_node = xza_xmi_node;

    SKZ_LOCK_INIT ( xza_softc -> lock );

    /*
    ** Initialize the lamb xmi node number
    */
    for ( lamb_xmi_node = 0; lamb_xmi_node < MAX_XMI_NODE; lamb_xmi_node++ )
	if ( xmidata->xmiintr_dst & (1<<lamb_xmi_node) )
	    break;
    xza_softc -> lamb_xmi_node = lamb_xmi_node;

    /*
    ** Setup the SETNEX mask
    */
    xza_softc -> setnex_mask.qfroz = 1;
    xza_softc -> setnex_mask.tgt = 1;

    /*
    ** set probe flag, init code behaves a bit differently if this is set.
    */
    xza_softc -> flags |= SKZ_PROBE;

    /*
    ** Initialize the "error subsystem" ( I wish it was a joke! ) 
    */
    skz_status = skz_error_init ( xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	return ( 0 );

    /*
    ** Initialize the 'DME'
    */
    skz_status = skz_dme_init ( xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	return ( 0 );


    /*
    ** Initialize the xza's bus structure
    */
    xza_bus -> connect_num = xmi_bus_num;
    xza_bus -> slot = xza_xmi_node;
    xza_bus -> alive = ALV_ALIVE;

    /* Add the xza_bus to the bus hierarchy */
    conn_bus ( xmibus, xza_bus );

    /*
    ** Initialize this bus's mailbox
    */
    MBOX_GET(xmibus,xza_bus);

    /*
    ** Enter the xza_softc into it's directory
    */
    xza_softc_directory[xza_bus->bus_num] = xza_softc;


    /*************************************************************************
    ** 
    ** OK, start checking up on things ( I know it seems a little late )
    **
    **************************************************************************/


    /*
    ** Read the XDEV register
    */
    skz_status = skz_read_reg ( XDEV, &xdev, xza_softc );
    if ( skz_status != CAM_REQ_CMP ) 
    {
	skz_error (	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );

	skz_error (	module,
			SKZ_ERROR,
			SKZ_PROBE_FAILURE,
			NULL,
			xza_softc );

	return ( 0 );
    }

    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_FLOW,
		 ("\n%s: XDEV Register Value is %x\n", module, xdev) );

    /*
    ** Save xdev for future reference ( XZA features vs. firmware rev checks )
    */
    xza_softc -> xdev = xdev;

    /*
    ** Check the device type to be sure we're talking to an XZA
    */
    if ( xdev.device_type != XZA_DEVICE_TYPE )
    {
	skz_error ( 	module,
			SKZ_ERROR,
			SKZ_WRONG_DEV,
			NULL,
			xza_softc );

	return ( 0 );
    }

    /*
    ** Print device type and revision information
    */
    printf ( "xza%d at xmi%d node %d (%s %02X%02X)\n", 
	     xza_bus -> bus_num, xmi_bus_num, xza_xmi_node,
	     pxmisw -> xmi_name, xdev.hard_rev, xdev.firm_rev );



    /*
    ** Poll the XPUD for the uninitialized bit to be set.
    ** We must poll because:
    **  1) The XMI init code only waits until the STF bit is cleared in the 
    **     XBE before calling us
    **  2) The XZA performs several operations between clearing STF and
    **     setting XPUD<unin>, thus, just because it may not have been set
    **     yet when we first enter here doesn't mean the the adapter is
    **     not alive.
    */
 
    /*
    ** Get the initial value
    */
    skz_status = skz_read_reg ( XPUD, &xpud, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error ( module,
	   	SKZ_ERROR,
		skz_status,
		NULL,
		xza_softc );

	return ( 0 );
    }

    delay = 0;

    /*
    ** Loop until the unin bit is set or we're tired of waiting
    */
    while ( !(xpud.unin) && ( (delay/10) < XZA_ADAPTER_RESET_TIME ) )
    {
	/*
	** Wait a bit
	*/
	DELAY ( SKZ_ONE_SECOND_DELAY / 10 );	/* Delay 1/10 of a second */
	delay++;

	/*
	** Re-read the XPUD register
	*/
	skz_status = skz_read_reg ( XPUD, &xpud, xza_softc );
	if ( skz_status != CAM_REQ_CMP )
	{
	    skz_error ( module,
	   		SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );

	    return ( 0 );
	}
    }

    /*
    ** If the unin bit is not set we've got a problem...
    */

    if ( xpud.unin )
    {
	/*
	** Everything's OK, initialize..
	*/

    	SIM_PRINTD ( NOBTL,
		     NOBTL,
	 	     NOBTL,
	 	     CAMD_FLOW,
	 	     ("\n%s: Adapter OK\n", module) );

	/*
	** Set up both xza channels
	**
	** Get the bus structs for each of the 
	** two SCSI busses ( channels )
	*/
	for ( i = 0; i < XZA_CHANNELS; i++ )
	{
	    ctlr = (struct controller *) xza_get_ctlr ( "skz", xza_bus, i );
	    if ( ctlr )
	    {
	        ctlr -> slot = i;	/* slot == channel number for xza */
	        ctlr -> alive |= ALV_ALIVE;
	        ctlr -> addr = (char *)nxv;
	        ctlr -> physaddr = (char *)nxp;

		ctlr -> private[0] = (caddr_t) cam_ctlr_string;

	        /*
	        ** connect this controller to the XZA
	        */
	        conn_ctlr ( xza_bus, ctlr );

	        /*
	        ** Allocate a mailbox for this controller..
	        ** Even though the XZA's registers are accessed via
	        ** the XZA's mailbox, each controller and bus should have
	        ** have a mailbox
	        */
	        MBOX_GET(xza_bus, ctlr);

	        /*
	        ** Create the sim_softc for each controller
	        */
	        sim_softc = (SIM_SOFTC *) 
			kmem_alloc (kernel_map, sizeof (SIM_SOFTC));
	        if ( sim_softc == NULL )
	        {
	            skz_error ( module,
			    SKZ_ERROR,
			    SKZ_PROBE_FAILURE,
			    NULL,
			    xza_softc );
		    return ( 0 );
	        }

	        softc_directory[ctlr -> ctlr_num] = sim_softc;

	        /* 
	        ** Update the probe storage fields in the 
	        ** softc structure. 
	        */
    	        sim_softc->csr_probe = (caddr_t) nxv;
    	        sim_softc->um_probe = ctlr;
    	        /*
     	        ** Using the uba_ctlr structure and csr 
	        ** update the "normal" fields in the
     	        ** softc.  These are "expected" in the rest 
	        ** of the initialization path.
     	        */
    	        sim_softc->reg = (void *) nxv;        /* base addr */
    	        sim_softc->cntlr = ctlr -> ctlr_num;  /* path id */

                sim_softc->hba_sc = &(xza_softc -> channel[i]);

	        /*
	        ** Initialize the xza_chan structure
	        */ 
	        xza_softc -> channel[i].sim_softc = sim_softc;
	        xza_softc -> channel[i].xza_softc = (vm_offset_t) xza_softc;
	        xza_softc -> channel[i].chan_num = i;

	        xza_softc -> channel[i].state = UNINITIALIZED;
	        SKZ_LOCK_INIT ( xza_softc -> channel[i].lock );

		/*
		** The controller is fully initialized
		*/
	        xza_softc -> channel[i].flags |= SKZ_ALIVE;

	    }
	    else
	    {
	        /*
	        ** Initialize the xza_chan structure
	        */ 
	        xza_softc -> channel[i].sim_softc = 0;
	        xza_softc -> channel[i].xza_softc = (vm_offset_t) xza_softc;
	        xza_softc -> channel[i].chan_num = i;

	        xza_softc -> channel[i].state = UNINITIALIZED;

	        SKZ_LOCK_INIT ( xza_softc -> channel[i].lock );

	    } /* if ( ctlr ) */

	} /* for */

	/*
	** If no controllers configured for this XZA, log error
	** Else, perform the ccfg_simattach for each controller configured
	*/
	ctlr = xza_bus -> ctlr_list;
	if ( ctlr == NULL )
	{
	    skz_error ( module,
			SKZ_ERROR,
			SKZ_NO_CONTROLLERS,
			NULL,
			xza_softc );
	    return ( 0 );
	}
	else
	{
	    /*
	    ** Do all of the necessay "attaching"
	    */
	    /*
	    ** Call the xza_attach routine - this routine allocates N_Port
	    ** data structures and the necessary interrupt vectors.
	    */
	    skz_status = xza_attach ( xza_softc );
	    if ( skz_status != CAM_REQ_CMP )
		return ( 0 );

	    /*
	    ** Attach each controller to the CAM subsystem
	    */
	    while ( ctlr )
	    {
		/*
		** Call the CDrv to complete the init path.  
		** If the attachment failed
		** signal back to the system that the controller 
		** is not ready.  The attachment process allows a 
		** SIM to be included in the XPT
		** dispatch table such that peripheral driver 
		** may issue xpt calls to this SIM.
		*/
		if ( ccfg_simattach( &xza_entry, ctlr -> ctlr_num ) == 
								CAM_FAILURE )
		    skz_error (	module,
				SKZ_ERROR,
				SKZ_PROBE_FAILURE,
				NULL,
				xza_softc );
		else
		{
		    skz_status = skz_config_ctlr ( ctlr );
		    if ( skz_status != CAM_REQ_CMP )
	    		skz_error ( module,
			    	    SKZ_ERROR,
			    	    skz_status,
			    	    NULL,
				    xza_softc );
    		}

		ctlr = ctlr -> nxt_ctlr;

	    } /* while ( ctlr ) */

	    /*
	    ** Turn off I/O until we've had a chance to reinitialize
	    */
	    xza_softc -> flags |= SKZ_ADAPT_RESET_IN_PROGRESS;

	    /*
	    ** Create SKZ Kernel Threads and Re-initialize to enable interrupts
	    */
	    timeout ( skz_reinit, xza_softc, hz );


	} /* else */


    } /* if XPUD<unin> */
    else
    {
	/* 
	** The unin bit is not set, meaning that the XZA is
	** not in the UNINITIALIZED state.  This indicates
	** that the adapter failed self test and is not
	** useable.  
	*/
	/*
	** Log asr error to system console..
	*/
	skz_error (	module,
			SKZ_ERROR,
			SKZ_XZA_ADAPT_INIT_ERR,
			NULL,
			xza_softc );

	skz_error (	module,
			SKZ_ERROR,
			SKZ_PROBE_FAILURE,
			NULL,
			xza_softc );

	return ( 0 );

    } /* if xpud.unin */
	    

    return ( 1 );

}

/************************************************************************
 *
 *  ROUTINE NAME:  xza_attach()
 *
 *  FUNCTIONAL DESCRIPTION: This function allocates memory for the 
 *	N_Port adapter block, and the Q_Buffer and Carrier pools.
 *	It allocates interrupt vectors and initializes them.
 *	This routine also schedules a routine to run after boot
 *	time which creates the kernel threads used by the driver.
 *	The driver runs non-threaded until after the system boots,
 *	and then uses threads to process interrupts and errors after
 *	the skz_reinit has executed.
 *
 *  FORMAL PARAMETERS:
 *	xza_softc - a pointer to the xza_softc structure, the overall 
 *		    controlling structure for this SIM module.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	0 - failure
 *	
 *
 *  SIDE EFFECTS:
 *	1) Calling ccfg_simattach enables the adapter, interrupts, and makes
 *	   the system ready to do regular I/O
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

xza_attach ( XZA_SOFTC* xza_softc )

{
    unsigned int	skz_status;
    unsigned int 	i;
    struct controller*	ctlr;

    vm_offset_t		vecaddr;
    thread_t		thread;

    SIM_MODULE(xza_attach);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nThis is %s", module) );

    /*
    ** Allocate data structure pools
    */
    xza_softc -> ab = (AB *) kmem_alloc ( kernel_map, sizeof ( AB ) );
    if ( xza_softc -> ab == NULL )
    {
	skz_error (	module,
			SKZ_ERROR,
			SKZ_MEM_ALLOC_ERR,
			NULL,
			xza_softc );
	return ( CAM_REQ_CMP_ERR );
    }

    xza_softc -> np_qb_pool = 
		(QB *) kmem_alloc ( kernel_map, 
				    SKZ_QB_POOL_ENTRIES * sizeof ( QB ) );
    if ( xza_softc -> np_qb_pool == NULL )
    {
	skz_error (	module,
			SKZ_ERROR,
			SKZ_MEM_ALLOC_ERR,
			NULL,
			xza_softc );
	return ( CAM_REQ_CMP_ERR );
    }

    xza_softc -> np_carrier_pool = 
	(CARRIER *) kmem_alloc ( kernel_map, 
				 SKZ_CAR_POOL_ENTRIES * sizeof (CARRIER));
    if ( xza_softc -> np_carrier_pool == NULL )
    {
	skz_error (	module,
			SKZ_ERROR,
			SKZ_MEM_ALLOC_ERR,
			NULL,
			xza_softc );
	return ( CAM_REQ_CMP_ERR );
    }



    /*
    ** Set up interrupt vectors
    */
    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_FLOW,
		 ("\n%s: Setting up Interrupt Vectors", module) );

    /* 
    ** Response Interrupts 
    */
    if ( allocvec(1, &vecaddr) != KERN_SUCCESS )
    {
	skz_error (	module,
			SKZ_ERROR,
			SKZ_VEC_ALLOC_ERR,
			NULL,
			xza_softc );
	return ( CAM_REQ_CMP_ERR );
    }

    xza_softc -> resp_intr_vector = vecoffset(vecaddr);
    intrsetvec ( xza_softc -> resp_intr_vector, 
		 skz_resp_intr, 
		 xza_softc );
  
    /* 
    ** Channel 0 Miscellaneous Interrupts 
    */
    if ( allocvec(1, &vecaddr) != KERN_SUCCESS )
    {
	skz_error (	module,
			SKZ_ERROR,
			SKZ_VEC_ALLOC_ERR,
			NULL,
			xza_softc );
	return ( CAM_REQ_CMP_ERR );
    }

    xza_softc -> c0_misc_intr_vector = vecoffset(vecaddr);
    intrsetvec ( xza_softc -> c0_misc_intr_vector, 
		 skz_misc_intr, 
		 &(xza_softc -> channel[0]) );

    /* 
    ** Channel 1 Miscellaneous Interrupts 
    */
    if ( allocvec(1, &vecaddr) != KERN_SUCCESS )
    {
	skz_error (	module,
			SKZ_ERROR,
			SKZ_VEC_ALLOC_ERR,
			NULL,
			xza_softc );
	return ( CAM_REQ_CMP_ERR );
    }

    xza_softc -> c1_misc_intr_vector = vecoffset(vecaddr);
    intrsetvec ( xza_softc -> c1_misc_intr_vector, 
		 skz_misc_intr, 
		 &(xza_softc -> channel[1]) );

    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nLeaving %s", module) );

    return ( CAM_REQ_CMP );

}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_config_ctlr()
 *
 *  FUNCTIONAL DESCRIPTION:  This function was stolen from
 *	io/dec/xmi/xmininit.c and simplified for the XZA.  It
 *	calls the peripheral driver attach and slave routines
 *	as they are defined in the driver structure in cam_config.c
 *
 *  FORMAL PARAMETERS:
 *	ctlr - a pointer to an XZA channel's controller structure
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

skz_config_ctlr( struct controller* 	ctlr )

{
    register struct device *dev;
    register struct driver *drp;
    extern struct device device_list[];

    SIM_MODULE(skz_config_ctlr);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nEntering %s", module) );

	drp = ctlr->driver;
	drp->ctlr_list[ctlr->ctlr_num] = ctlr;

	config_fillin(ctlr);
	printf("\n");
	
	if(drp->cattach)
		(*drp->cattach)(ctlr);
	
	for(dev = device_list; dev->dev_name; dev++) {
		int savectlr;
		char *savectname;
		
		if(((dev->ctlr_num != ctlr->ctlr_num)&&(dev->ctlr_num !=-1)) ||
		   ((strcmp(dev->ctlr_name, ctlr->ctlr_name)) &&
		    (strcmp(dev->ctlr_name, "*"))) ||
		   (dev->alive & ALV_ALIVE) ) {
			continue;
		}
		
		savectlr = dev->ctlr_num;
		savectname = dev->ctlr_name;
		dev->ctlr_num = ctlr->ctlr_num;
		dev->ctlr_name = ctlr->ctlr_name;
		
		if((drp->slave) && (*drp->slave)(dev, ctlr -> addr)) {
			dev->alive |= ALV_ALIVE;
			dev->ctlr_num = ctlr->ctlr_num;
			conn_device(ctlr, dev);
			drp->dev_list[dev->logunit] = dev;
			perf_init(dev);
			printf("%s%d at %s%d target %d lun %d unit %d", 
				dev->dev_name, dev->logunit,
				ctlr->ctlr_name, ctlr->ctlr_num,
		(((UBA_UNIT_TO_DEV_UNIT(dev))>>TARGET_SHIFT)&TARGET_MASK),
		(((UBA_UNIT_TO_DEV_UNIT(dev))>>LUN_SHIFT)&LUN_MASK),
		dev -> unit );
			if(drp->dattach)
				(*drp->dattach)(dev);
			printf("\n");
		} else {
			dev->ctlr_num = savectlr;
			dev->ctlr_name = savectname;
		}
	}

    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nLeaving %s", module) );

    return(CAM_REQ_CMP);
} 

/************************************************************************
 *
 *  ROUTINE NAME:  skz_reinit()
 *
 *  FUNCTIONAL DESCRIPTION:  This function creates threads for:
 *	1) Response interrupt handling
 *	2) Miscellaneous interrupt handling for each channel
 *	3) Error handling
 *
 *  FORMAL PARAMETERS:
 *	xza_softc - a pointer to the driver's controlling data structure.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:  After this runs, the driver begins to run 'threaded'
 *	and almost no work is done in interrupt context.
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

skz_reinit ( XZA_SOFTC* xza_softc )

{
    u_int	skz_status;

    SIM_MODULE(skz_reinit);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nEntering %s", module) );

    /*
    ** Re-initalize the adapter, so we can run with interrupts 
    ** enabled, and begin using threads, if we're threaded.
    **
    ** skz_init clears the SKZ_ADAPT_RESET_IN_PROGRESS flag
    ** to allow I/O to begin
    */
    skz_status = skz_init ( xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	skz_error ( module,
		    SKZ_ERROR,
		    skz_status,
		    NULL,
		    xza_softc );

    /*
    ** Clear the probe flag, because we're completely done with
    ** probe/config, finally
    */
    xza_softc -> flags &= ~SKZ_PROBE;

    /*
    ** Create XZA kernel threads
    */
#if SKZ_THREADED == SKZ_TH_TIMEOUT
    skz_thread_init ( xza_softc ); 
#endif

    /*
    ** Turn I/O back on 
    */
    if ( xza_softc -> channel[0].flags & SKZ_ALIVE )
	SC_SCHED_RUN_SM ( xza_softc->channel[0].sim_softc );

    if ( xza_softc -> channel[1].flags & SKZ_ALIVE )
	SC_SCHED_RUN_SM ( xza_softc->channel[1].sim_softc );


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nLeaving %s", module) );

    return ( CAM_REQ_CMP );

}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_thread_init()
 *
 *  FUNCTIONAL DESCRIPTION:  This function creates threads for:
 *	1) Response interrupt handling
 *	2) Miscellaneous interrupt handling for each channel
 *	3) Error handling
 *
 *  FORMAL PARAMETERS:
 *	xza_softc - a pointer to the driver's controlling data structure.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:  After this runs, the driver begins to run 'threaded'
 *	and almost no work is done in interrupt context.
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

void 
skz_thread_init ( XZA_SOFTC* xza_softc )

{

    SIM_MODULE(skz_thread_init);

    /*
    **
    ** Create Threads
    **
    */
    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_FLOW,
		 ("\n%s: Creating Kernel Interrupt Threads", module) );

    if ( SKZ_IDLE_THREAD )
	return;


#if SKZ_THREADED > 0

    /*
    ** Interrupt Threads
    */

    xza_softc -> resp_th = kernel_thread_w_arg 
		( first_task, skz_resp_th, xza_softc );

#endif

    if ( xza_softc -> channel[0].flags & SKZ_ALIVE )
	xza_softc -> channel[0].misc_th = kernel_thread_w_arg 
		( first_task, skz_misc_th, &(xza_softc -> channel[0]) );

    if ( xza_softc -> channel[1].flags & SKZ_ALIVE )
	xza_softc -> channel[1].misc_th = kernel_thread_w_arg 
		( first_task, skz_misc_th, &(xza_softc -> channel[1]) );

    /*
    ** Error Thread
    */
    xza_softc -> error_th = kernel_thread_w_arg 
		( first_task, skz_error_th, xza_softc );


    xza_softc -> flags |= SKZ_THREADS_ACTIVE;


    return;

}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_probe()
 *
 *  FUNCTIONAL DESCRIPTION: This is a stub, may not be needed any longer
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

skz_probe ( vm_offset_t nxv, struct controller* ctlr )

{
    return ( CAM_REQ_CMP );
}

skz_attach ( SIM_SOFTC* sim_softc )

{

    sim_softc -> hba_init = skz_hba_init;
    sim_softc -> hba_go = skz_go;
    sim_softc -> hba_bus_reset = skz_reset_bus;
                    
    return ( CAM_REQ_CMP );
}

skz_reset_attach ( SIM_SOFTC* sim_softc )
{
    return ( CAM_REQ_CMP );
}

skz_unload ( SIM_SOFTC* sim_softc )
{
    return ( CAM_REQ_CMP );
}


/************************************************************************
 *
 *  ROUTINE NAME:  xza_get_bus()
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

/* get_bus()
 *
 *	The get_bus() routine searches the static bus_list array for a 
 *	bus structure with values matching the passed parameters.
 *	A parameter value of -1 is used to indicate that a particular
 *	member need not match.
 */
struct bus *
xza_get_bus(bus_name, conn_bus, xza_xmi_node )
char		*bus_name;		/* device name	*/
struct bus 	*conn_bus; 		/* bus name connected to */
u_int		xza_xmi_node;		/* xmi node number	*/
{
    struct bus 	*bus;

    for (bus = bus_list; bus->bus_name != 0; bus++) 
    {
	if ( (!strcmp(bus->bus_name, bus_name)) &&
	     (!strcmp(bus->connect_bus, conn_bus->bus_name)) &&
	     ((bus->connect_num == conn_bus->bus_num) ||
	      (bus->connect_num == -1)) &&
	     ((bus->slot == xza_xmi_node) ||
	      (bus->slot == -1)) &&
	     (bus->alive == 0) )
		break;
    }

    if (bus->bus_name)
	return(bus);
    else
	return(0);
}


/************************************************************************
 *
 *  ROUTINE NAME:  xza_get_ctlr()
 *
 *  FUNCTIONAL DESCRIPTION: This function finds a controller structure
 *	based upon controller name and number and the bus name and number
 *	that it is connected to.  The routine in /io/common/driver_support.c
 *	was not adequate for the needs of this design, this routine is 
 *	a modified version of the get_ctlr routine there.
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

struct controller *
xza_get_ctlr ( char* ctlr_name, struct bus* bus, u_int chan )

{
    struct controller *ctlr;

    for (ctlr = controller_list; ctlr->driver != 0; ctlr++) 
    {
	if (!(strcmp(ctlr->ctlr_name, ctlr_name)) &&
	    !(strcmp(ctlr->bus_name, bus -> bus_name)) &&
	    ( (ctlr->bus_num == bus -> bus_num) ||
	      (ctlr->bus_num == -1) ) &&
	    ( (ctlr->slot == chan) ||
	      (ctlr->slot == -1) ) &&
	    (ctlr->alive == 0) )
	{
	    break;
	}
    }

    if (ctlr->driver) 
	return (ctlr);
    else
	return (0);
}




/*
** Other odds and ends related to configuration
*/

int kfmsbprobe ()
{
	return ( 1 );
}

int xzaconfl1 ()
{
	return ( 1 );
}

int xzaconfl2()
{
	return ( 1 );
}
