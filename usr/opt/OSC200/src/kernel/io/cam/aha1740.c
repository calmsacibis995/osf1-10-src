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
static char *rcsid = "@(#)$RCSfile: aha1740.c,v $ $Revision: 1.1.6.6 $ (DEC) $Date: 1993/12/09 20:25:13 $";
#endif

/* ---------------------------------------------------------------------- */

/* aha1740.c         Version 1.00                     April 13, 1993      */

/*	This file contains the CAM SIM HBA driver code for the Adaptec	
	AHA-1740A/1742A enhanced mode SCSI adapter.

	Modification History

	1.00	     04/13/93		Theresa Chin
	Created this file.

		     04/22/93		Theresa Chin
	Added more routines.

		     04/23/93		Theresa Chin
	Added mb() call after each call to 'write_io_port()' since
	'write_io_port()' doesn't do mb() for us.

		     04/26/93		Theresa Chin
	Modified to accommodate the fact that the queue pointer fields
	(next and prev) in the AHA_JOB structure are now at the top
	of the structure.

		     05/18/93		Theresa Chin
	Modified to merge to JENSEN BL1 sandbox.  Included 'eisa.h'.
	Modified interface to 'read_io_port' and 'write_io_port' since
	the interface changed.

		     05/21/93		Theresa Chin
	Modified to merge to JENSEN BL2.  
*/

/* ---------------------------------------------------------------------- */


/* Local defines.
 */
#define CAMERRLOG

#define ADAPTER_RESET_NEEDED		1
#define ADAPTER_RESET_NOT_NEEDED 	0

#define EDGE_TRIGGER 	0		/* Edge-sensitive interrupt flag */
#define LEVEL_TRIGGER 	1		/* Level-sensitive interrupt flag */

#define WAIT_20USEC	20		/* 20 microseconds delay value */
#define WAIT_10USEC	10		/* 10 microseconds delay value */
#define WAIT_100USEC	100		/* 100 microseconds delay value */

#define HRESET_RETRY	200000		/* Hard reset retry limit */
#define SEND_RETRY	10		/* Send request retry limit */

#define FLUSH_ALL 0xffffffff		/* Flush all queues flag */

/*
 * debug flag
 */
/*
#define AHADEBUG
*/

/* ---------------------------------------------------------------------- */
/* Include files
 */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <kern/lock.h>
#include <sys/lock.h>
#include <dec/binlog/errlog.h>          /* UERF errlog defines  */
#include <arch/alpha/machparam.h>
#include <vm/vm_kern.h>
#include <kern/thread.h>
#include <kern/sched.h>
#include <kern/parallel.h>
#include <kern/sched_prim.h>

#include <io/common/iotypes.h>
#include <io/common/devdriver.h>
#include <io/cam/dec_cam.h>
#include <io/cam/cam.h>
#include <io/cam/scsi_status.h>
#include <io/cam/scsi_all.h>
#include <io/cam/scsi_phases.h>
#include <io/cam/cam_logger.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/dme.h>
#include <io/cam/cam_debug.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim.h>
#include <io/cam/sim_common.h>
#include <io/cam/cam_errlog.h>

#include <io/cam/aha1740.h>		/* Driver specific defines 	*/

#include <io/dec/eisa/eisa.h>		/* EISA bus definitions		*/



/* ---------------------------------------------------------------------- */ 
/* External declarations
 */
extern void	scsiisr();
extern void	panic();		/* general purpose panic routine */
extern SIM_SOFTC *softc_directory[];
extern CAM_SIM_ENTRY dec_sim_entry;  
extern int shutting_down;
extern int bcopy();                     /* System routine to copy buffer  */
extern void bzero();			/* System routine to clear buffer */
extern char *sc_alloc();                /* SIM Common memory allocator */
extern void sc_free();                  /* SIM Common memory deallocator */
extern void sm_bus_free();		/* SIM I/O cleanup routine   */
extern vm_offset_t pmap_extract();      /* VM code to get a phy addr */
extern kern_return_t pmap_svatophys();	/* VM code to translate kernel  *
					 * address to phys. address     */

extern I32 cam_at_boottime();		/* Checks if system boot time   */
extern void ss_reset_detected();	/* Notify bus reset detected    */
extern void sc_setup_ws();



/* ---------------------------------------------------------------------- */
/* Forward references
 */
U32	aha_probe();
U32	aha_attach();
U32	aha_chip_reset();
U32	aha_unload();
U32	aha_cam_init();
U32	aha_init();
U32	aha_check_firmware();
U32	aha_alloc_pool();

U32	aha_go();
U32	aha_check_activeq();
U32	aha_setup_job();
AHA_JOB	*aha_alloc_job();
void	aha_dealloc_job();
U32	aha_send_cb();
U32	aha_dma_setup();
U32	aha_map_data();
void 	ahaintr();
void	aha_process_intr();
U32	aha_qsearch();
U32	aha_process_error();
AHA_JOB *aha_chk_bdr();
void	aha_iodone();

U32	aha_termio_abort_bdr();
U32	aha_ss_termio();
U32	aha_ss_abort();
U32	aha_ss_device_reset();
U32	aha_bus_reset();
void	aha_flush();
void	aha_flush_lun_queue();
void	aha_logger();

U32	aha_path_inq();

/* Debug routines */
void	aha_printcb();			
void	aha_printsb();
void	aha_print_inq();


/* ---------------------------------------------------------------------- */
/* Local declarations
 */

static void (*local_errlog)() = aha_logger;




/* ---------------------------------------------------------------------- */
/*		DRIVER CONFIGURATION AND INITIALIZATION ROUTINES       	  */
/* ---------------------------------------------------------------------- */


/* ---------------------------------------------------------------------- *
 *                                                                        *
 * ROUTINE NAME:  aha_probe()					 	  *
 *									  *
 * FUNCTIONAL DESCRIPTION: 						  *
 *	This routine checks the presence of the controller, allocates the *
 *	controller's data structures, and calls the CAM config. routine   *
 *	to attach this controller to the CAM subsystem.		  	  *
 *									  *
 * CALLED BY:  machine autoconfig routines at boot time                   *
 *									  *
 * FORMAL PARAMETERS:							  *
 *		addr -- Base address of the AHA registers	          *
 *		ctlr -- Pointer to the controller structure		  *
 *									  *
 * IMPLICIT INPUTS:							  *
 *		ctlr->slot -- EISA slot number of this controller         *
 *		ctlr->conn_priv[0] -- Pointer to 'eisainfo' structure     *
 *		irq_p.channel -- EISA interrupt level for this controller *
 *                               from get_config()                        *
 *		irq_p.trigger -- EISA interrupt high/low                  *
 *				    flag, 0 = edge (high), 1 = level (low)*
 *									  *
 *		ctlr->addr -- KSEG address of controller's base register  *
 *		ctlr->physaddr -- Controller's base register physical     *
 *				  address				  *
 * IMPLICIT OUTPUTS:							  *
 *		ctlr->physaddr2 -- AHA_SOFTC pointer to be used by the    *
 *				   interrupt handling routine             *
 *		ctlr->private[0] -- CAM controller string pointer         *
 *									  *
 * RETURN VALUE:							  *
 *		CAM_TRUE  (1)	if controller is successfully probed	  *
 *		CAM_FALSE (0)	if controller is not present or error	  *
 *									  *
 * SIDE EFFECTS:							  *
 *									  *
 * ---------------------------------------------------------------------- */

U32
aha_probe( addr, ctlr )
caddr_t	addr;			
struct controller *ctlr;
{
   register AHA_SOFTC	*aha_softc;		/* ptr. to HBA softc struct */
   register SIM_SOFTC	*sim_softc;		/* ptr. to SIM softc struct */  
   register U32 i;
   register io_handle_t iobaseaddr;		/* base reg address */
   U32 hw_id;					/* hardware ID of controller */
   register I32 retval;


   extern char cam_ctlr_string[];
   
   SIM_MODULE(aha_probe);

   SIM_PRINTD (  NOBTL,
                 NOBTL,
                 NOBTL,
                 CAMD_INOUT,
                 ("\nEntering %s", module) );

   SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_CONFIG),
             ("EISA slot %d, pathid = %d\n",ctlr->slot,ctlr->ctlr_num));



   /* Validate that this controller has not been previously probed.
    * If the CSR's match, just continue with the rest of the
    * initialization. If the CSR's do not match, return failure to the
    * system configuation code.
    */

   if( softc_directory[ ctlr->ctlr_num ] != NULL )
   {   
       CAM_ERROR(module,
                "cntlr already probed",
                SIM_LOG_SIM_SOFTC|SIM_LOG_PRISEVERE,
                softc_directory[ ctlr->ctlr_num ], NULL, NULL);
       if( ctlr->addr != (softc_directory[ ctlr->ctlr_num ])->csr_probe )
       {
           CAM_ERROR(module,
                     "cntlr already probed diff CSR",
                     SIM_LOG_SIM_SOFTC|SIM_LOG_PRISEVERE,
                     softc_directory[ ctlr->ctlr_num ], NULL, NULL);
           return(CAM_FALSE);           /* Signal that it is not there */
       }
       return (CAM_TRUE);
   }

   /* 
    * Get the controller's base address.  For EISA
    * controllers, the physical address of the adapter base register
    * can be calculated by shifting EISA slot # by 12 bits.
    */

   iobaseaddr = (io_handle_t)ctlr->physaddr;   

   /*
    * Read the controller's ID register to insure it is actually an
    * Adaptec 1740A or 1742A.
    */

   hw_id = (U32) READ_BUS_D32( (AHA_HID | iobaseaddr));

   if ( ((hw_id & AHA_ID_MASK) != AHA_1740A_HID) && 
	((hw_id & AHA_ID_MASK) != AHA_1742A_HID) )
   { 
	printf("aha_probe: no AHA1740A/1742A present in EISA slot %d\n",
		ctlr->slot);
	return (CAM_FALSE);
   }


   /* Allocate and setup the SIM_SOFTC structure */

   sim_softc = (SIM_SOFTC *) sc_alloc( sizeof(SIM_SOFTC) );
   if (sim_softc == (SIM_SOFTC *) NULL)
   {
	CAM_ERROR(module,
		  "SIM_SOFTC alloc failed",
	          SIM_LOG_PRISEVERE,
		  NULL, NULL, NULL);

	return (CAM_FALSE);
   }


   softc_directory[ ctlr->ctlr_num ] = sim_softc;
   sim_softc->csr_probe = ctlr->addr;      
   sim_softc->um_probe = (void *)ctlr;
   sim_softc->reg = (void *)ctlr->physaddr;  /* base io address */
   sim_softc->cntlr = ctlr->ctlr_num;	     /* path id */
   sim_softc->dme = 0;                       /* no DME for this driver */

   /* ???? Note that currently there is no way of getting host SCSI ID 
    *      from the console and passing it down to this routine. 
    */

   sim_softc->scsiid = DEFAULT_SCSIID;       /* set initiator value  */


   /* Allocate the AHA_SOFTC structure */

   aha_softc = (AHA_SOFTC *) sc_alloc( sizeof(AHA_SOFTC) );
   if (aha_softc == (AHA_SOFTC *) NULL)
   {
	CAM_ERROR(module,
		  "AHA_SOFTC alloc failed",
		  SIM_LOG_PRISEVERE,
		  NULL, NULL, NULL);

	return (CAM_FALSE);
   }

   sim_softc->hba_sc = (void *) aha_softc;  
   aha_softc->aha_sim_softc = sim_softc;

   ctlr->physaddr2 = (caddr_t)aha_softc;   /* Save softc ptr in ctlr */

   AHA_LOCK_INIT(aha_softc->aha_lock);	   /* init. the softc lock */
   aha_softc->aha_ctlr = ctlr;		   /* save controller struct. ptr. */

   /* Call the Config Drv to complete the init path.  If the attachment 
    * failed, signal back to the system that the controller is not ready. 
    */

   if(ccfg_simattach(&dec_sim_entry, ctlr->ctlr_num) == CAM_FAILURE)
   {
	SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_ERRORS),
		   ("aha_probe - ccfg_simattach == CAM_FAILURE\n"));
	
	return (CAM_FALSE);
   }

   /* Point to the CAM string in private area of controller structure so
    * that it can be identified as a CAM/SCSI controller later by peripheral
    * drivers.
    */
    ctlr->private[0] = (caddr_t) cam_ctlr_string;

    return (CAM_TRUE);

}	/* End of aha_probe() */


/* ---------------------------------------------------------------------- *
 *                                                                        *
 * ROUTINE NAME:  aha_attach()						  *
 *                                                                        *
 * FUNCTIONAL DESCRIPTION:                                                *
 *	This routine sets up the HBA function vectors in the SIM_SOFTC	  *
 *	structure and calls a subroutine to allocate and initialize the   *
 *	I/O job blocks to be used by this adapter.			  *
 *                                                                        *
 * CALLED BY:  SIM config routine, name_lookup_hba_attach()		  *
 *                                                                        *
 * FORMAL PARAMETERS:                                                     *
 *		sim_softc -- Pointer to the SIM_SOFTC structure		  *
 *                                                                        *
 * IMPLICIT INPUTS:                                                       *
 *                                                                        *
 * IMPLICIT OUTPUTS:                                                      *
 *		HBA function vectors are set up in SIM_SOFTC		  *
 *                                                                        *
 * RETURN VALUE:                                                          *
 *		CAM_REQ_CMP	if successful attach			  *
 *		CAM_REQ_CMP_ERR if attach failed            		  *
 *                                                                        *
 * SIDE EFFECTS:							  *
 *                                                                        *
 * ---------------------------------------------------------------------- */

U32
aha_attach( sim_softc )
register SIM_SOFTC *sim_softc;
{
   register AHA_SOFTC *aha_softc;
   register U32 retval;


   SIM_MODULE(aha_attach);

   aha_softc = (AHA_SOFTC *)sim_softc->hba_sc;	/* get our softc structure */   

   /* First allocate all resource necessary for this driver */
   retval = aha_alloc_pool(aha_softc);

   if (retval != CAM_REQ_CMP)
	return (CAM_REQ_CMP_ERR);

   sim_softc->hba_init = aha_cam_init;
   sim_softc->hba_go = aha_go;
   sim_softc->hba_bus_reset = aha_bus_reset;
   sim_softc->hba_sel_msgout = aha_termio_abort_bdr;


   return (CAM_REQ_CMP);

}	/* End of aha_attach() */


U32
aha_chip_reset( sim_softc )
SIM_SOFTC *sim_softc;
{

   SIM_MODULE(aha_chip_reset);

   SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_ERRORS),("aha_chip_reset called\n"));
   return (CAM_REQ_CMP);

}	/* End of aha_chip_reset() */

U32
aha_unload() 
{

   SIM_MODULE(aha_unload);

   SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_ERRORS),("aha_unload called\n"));
   return (CAM_REQ_CMP);

}	/* End of aha_unload() */

/* ---------------------------------------------------------------------- *
 *                                                                        *
 * ROUTINE NAME:  aha_cam_init()					  *
 *                                                                        *
 * FUNCTIONAL DESCRIPTION:                                                *
 * 	This routine is called by the sim_init() via SC_HBA_INIT macro.   *
 *	It calls the aha_init() with the 'scsi_bus_reset_at_boot' global  *
 *	flag.								  *
 *                                                                        *
 * CALLED BY:  sim_init()						  *
 *                                                                        *
 * FORMAL PARAMETERS:                                                     *
 *		sim_softc -- Pointer to the SIM_SOFTC structure           *
 *                                                                        *
 * IMPLICIT INPUTS:                                                       *
 *              scsi_bus_reset_at_boot -- System global flag indicating   *
 *                               whether SCSI bus reset should be done    *
 *				 at boot time, declared in cam_data.c	  *
 * IMPLICIT OUTPUTS:                                                      *
 *									  *
 * RETURN VALUE:                                                          *
 *		CAM_REQ_CMP	if successful initialization		  *
 *		CAM_REQ_CMP_ERR	if initialization fails			  *
 *                                                                        *
 * SIDE EFFECTS:                                                          *
 *                                                                        *
 * ---------------------------------------------------------------------- */

U32
aha_cam_init( sim_softc )
register SIM_SOFTC *sim_softc;
{
   register AHA_SOFTC *aha_softc;
   register U32 retval;
   register int oldspl;                 /* IPL to be saved */

   extern I32 scsi_bus_reset_at_boot;

   SIM_MODULE(aha_cam_init);

   SIM_PRINTD (  NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nEntering %s", module) );
   
   aha_softc = (AHA_SOFTC *)sim_softc->hba_sc;

   retval = aha_init(aha_softc, scsi_bus_reset_at_boot);

   return (retval);

}	/* End of aha_cam_init() */

/* ---------------------------------------------------------------------- *
 *                                                                        *
 * ROUTINE NAME:  aha_init()						  *
 *                                                                        *
 * FUNCTIONAL DESCRIPTION:                                                *
 *	This routine performs the hard reset of the controller if it is   *
 *      necessary.  If the SCSI bus reset request flag is passed,         *
 *	it will be performed as part of the adapter (re)initialization.   *
 *	This is done by setting the RSTPWR bit in the SCSI Definition     *
 *	register prior to performing the hard reset.  If the bus reset is *
 *	not required and this is the boot time init request, this routine *
 *	will first check if the adapter is already functional.  If that   *
 *	is the case, adapter reset is skipped and only registers will be  *
 *	set up.  Otherwise, hard reset is performed.  Also if this is     *
 *	the first time this routine is being called, this routine will    *
 *	issue a special request to the controller to check its firmware   *
 *	revision number and to store the information.  If the revision    *
 *	number is less than the one we can support, the routine will      *
 *	return error status to the caller.  				  *
 *									  *
 * CALLED BY: aha_cam_init(), aha_process_intr(), and aha_bus_reset()     *
 *                                                                        *
 * FORMAL PARAMETERS:                                                     *
 *		aha_softc -- Pointer to the controller's SOFTC structure  *
 *		resetflag -- SCSI bus reset flag			  *
 *                                                                        *
 * IMPLICIT INPUTS:                                                       *
 *									  *
 * IMPLICIT OUTPUTS:							  *
 *									  *
 * RETURN VALUE:                                                          *
 *		CAM_REQ_CMP	if successful initialization		  *
 *		CAM_REQ_CMP_ERR if initialization fails			  *
 *									  *
 * SIDE EFFECTS:                                                          *
 *	The adapter hard reset might occur.  The SCSI bus could be reset. *
 *                                                                        *
 * ---------------------------------------------------------------------- */

U32
aha_init( aha_softc, resetflag )
register AHA_SOFTC *aha_softc;
register U32	resetflag;
{
   SIM_SOFTC *sim_softc = aha_softc->aha_sim_softc;
   struct controller *ctlr = aha_softc->aha_ctlr;

   I32	aha_reset_required = FALSE;	/* if set, adapter reset is required */
   register int oldpri, oldspl;		/* IPL to be saved */
   register io_handle_t iobaseaddr;     /* base reg address */
   U32 portaddr;			/* PORTADDR register content */
   U32 ebctrl;				/* EBCTRL register content */
   U32 g2stat;				/* G2STAT register content */
   U32 scsidef;				/* SCSIDEF register content */
   U32 reset_status;			/* MBOXIN0 register content */
   register U32 retry;			/* retry count for checking reset status */
   U32 intdef;				/* INTDEF register value to be written */
   struct irq irq_p;			/* interrupt info from get_config */
   U32 retval = CAM_REQ_CMP;		/* return value from this routine */





   SIM_MODULE(aha_init);

   SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_INOUT),
	      ("\nEntering %s", module));

#ifdef AHADEBUG				/* Debug code */

   (aha_softc->aha_init_called)++;
   printf ("aha_init called %d times\n", aha_softc->aha_init_called);

#endif /* AHADEBUG */				/* Debug code */


   iobaseaddr = (io_handle_t)ctlr->physaddr;    /* get base register address */

   AHA_IPL_LOCK (aha_softc->aha_lock, oldpri);	/* Lock softc structure */
   aha_softc->aha_state = AHA_RESET_IN_PROGRESS;

   /*
    * According to the spec, if CDEN bit is
    * not set in EBCTRL register, you can't access any enhanced mode
    * registers.
    */
   ebctrl = (U32)READ_BUS_D8( (AHA_EBCTRL | iobaseaddr));

   if ( !(ebctrl & EBCTL_CDEN) ) 
   {
	WRITE_BUS_D8( (AHA_EBCTRL | iobaseaddr),EBCTL_CDEN);

	WBFLUSH();			/* Do memory barrier */

	printf ("aha_init: CDEN bit was not set!\n");
	printf ("aha_init: CDEN bit is set now.\n");
   }

   if ( (!resetflag) && (cam_at_boottime()) )
   {
	/* Check if the adapter is already functional */

	portaddr = (U32)READ_BUS_D8( (AHA_PORTADDR | iobaseaddr));


	if ( !(portaddr & PORTADDR_EI) )	/* not in enhanced mode? */

 	   /* If not functional yet, set flag for adapter reset */
	   aha_reset_required = TRUE;
   }

   if ( (resetflag) || (aha_reset_required) )
   {
	if (resetflag)		/* bus reset is requested */
	{
	   aha_reset_required = TRUE;

	   SIM_SOFTC_LOCK(oldspl, sim_softc);

	   sim_softc->error_recovery |= ERR_BUS_RESET;
	   aha_softc->aha_state |= AHA_BUS_RESET_IN_PROGRESS;

	   SIM_SOFTC_UNLOCK(oldspl, sim_softc);

	   /* Read the SCSIDEF register first to read the SCSI ID value */
	   scsidef = (U32)READ_BUS_D8( (AHA_SCSIDEF | iobaseaddr));

	   /* Set 'bus reset on hard reset' bit in SCSIDEF register */
	   WRITE_BUS_D8( (AHA_SCSIDEF | iobaseaddr),SCSIDEF_RSTPWR | scsidef );

	   WBFLUSH();		/* Do memory barrier */
	}
	 
	/* Now try hard resetting the adapter.  After hard reset,
	 * the configuration data is lost.  If Initialize SCSI
	 * Subsystem command is not sent, the host adapter will
	 * use default setting.  This should be O.K. since default
	 * setting is acceptable for us.  
	 *
	 * The default setting are 10 Mbytes/sec, Parity Check enable,
	 * Synchronous Negotiation enable, and Disconnection enable.
	 */

#ifdef AHADEBUG                         /* Debug code */

	(aha_softc->aha_reset_num)++;		/* increment hard reset counter */
	printf ("aha_init: hard reset done %d times\n", aha_softc->aha_reset_num);

#endif /* AHADEBUG */                         /* Debug code */

	WRITE_BUS_D8( (AHA_G2CNTRL | iobaseaddr),G2CNTRL_HRESET );

	WBFLUSH();			/* Do memory barrier */
	
	/* The spec. says to hold this reset bit for at least 10 microseconds.
	 * We will hold it for 20 since DELAY is system clock dependent
	 * and may not be exact.
	 */
	DELAY( WAIT_20USEC );			

	/* Clear interrupts and clear HRESET bit to release reset line */
	WRITE_BUS_D8( (AHA_G2CNTRL | iobaseaddr),G2CNTRL_CLRINT );

	WBFLUSH();			/* Do memory barrier */

	/* 
	 * Wait for BUSY to go low in G2STAT register 
	 */
	retry = 0;		
	g2stat = (U32)READ_BUS_D8( (AHA_G2STAT | iobaseaddr));

	while (g2stat & G2STAT_BUSY)
	{
	   retry++;
	   if (retry > HRESET_RETRY)
	   {
		/* 
		 * Busy has not gone low.  Assume the card is gone.
		 * Log the error and fail the request.
		 */

		CAM_ERROR(module, 
			  "Adapter hard reset failed, can't use the card",
			  SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE,
			  sim_softc, NULL, NULL);

		retval = CAM_REQ_CMP_ERR;
		break;
	   }
	   else
	   {
		DELAY( WAIT_10USEC );		  
		g2stat = (U32)READ_BUS_D8( (AHA_G2STAT | iobaseaddr));
	   }
		
	}        

	/*
	 * Now read adapter reset status code from MBOXIN0 register 
	 */
	if (retval == CAM_REQ_CMP)
	{
	   reset_status = (U32)READ_BUS_D8( (AHA_MBOXIN0 | iobaseaddr));
	   
	   /* If error reset status, set return value to error status */

	   if ( (reset_status & 0xff) != MBOXIN0_NE )
		retval = CAM_REQ_CMP_ERR;
	}

	   
   }	/* if ( (resetflag) || (aha_reset_required) ) */

   /* 
    * Set up values in the adapter registers 
    */
   if (retval == CAM_REQ_CMP)
   {
	/* Enable the card */
	WRITE_BUS_D8( (AHA_EBCTRL | iobaseaddr),EBCTL_CDEN );

	WBFLUSH();			/* Do memory barrier */

	/* Make sure we are running enhanced mode firmware */
	WRITE_BUS_D8( (AHA_PORTADDR | iobaseaddr),PORTADDR_EI );

	WBFLUSH();			/* Do memory barrier */

	/* define the host's SCSI ID */
	WRITE_BUS_D8( (AHA_SCSIDEF | iobaseaddr),sim_softc->scsiid );

	WBFLUSH();			/* Do memory barrier */

	/* 
	 * Enable interrupts and set INTHIGH bit if necessary.
	 * We need to subtract 9 from passed interrupt level since
	 * the adapter expects 0 if interrupt level is 9, 1 if level
	 * is 10, and so on.
	 */
        get_config(ctlr,EISA_IRQ,"",&irq_p, 0);
	intdef = (U32)(irq_p.channel) & 0xff;
	intdef = intdef - EISA9_INTR;
        
				  /* INTHIGH should be set flag check */
        if (irq_p.trigger == EDGE_TRIGGER)
	   intdef = intdef | INTDEF_INTHIGH;

        WRITE_BUS_D8( (AHA_INTDEF | iobaseaddr),(INTDEF_INTEN | intdef) );

	WBFLUSH();			/* Do memory barrier */
   
   }

   if ( (!(aha_softc->aha_checked_fw)) && (retval == CAM_REQ_CMP) ) 
   {
        aha_softc->aha_checked_fw = TRUE;

	/* 
	 * Need to check the adapter rev. by issuing a control block to it
	 * The subroutine will return CAM_REQ_CMP_ERR if invalid firmware
	 * revision was found in the adapter.
	 */
	 
	retval = aha_check_firmware(aha_softc);
        /* DEBUG */
	if (retval == CAM_REQ_CMP)
	{

   	   /*
    	    * Enable the EISA interrupt for this controller.
    	    */

	   enable_option(ctlr);

	}

   }

   /*
    * If adapter reset was done, we need to flush any outstanding job blocks
    * that got lost due to the adapter/bus reset.  We will also notify
    * the CAM upper layer that bus reset was detected.
    */
   if (aha_reset_required)
   {
	aha_flush( aha_softc, FLUSH_ALL);

	if (resetflag) 
	{

	   CAM_ERROR(module,
	   	     "Bus reset detected",
		     SIM_LOG_HBA_SOFTC | SIM_LOG_PRISEVERE,
		     sim_softc, NULL, NULL);

	}
	else
	{
	   CAM_ERROR(module,
		    "Adapter reset detected",
		    SIM_LOG_HBA_SOFTC | SIM_LOG_PRISEVERE,
		    sim_softc, NULL, NULL);
	}

	/*
	 * Note that we can't call the next routine if this is the boot time since
	 * nexus flinks and blinks are not initialized yet.
         * Therefore, if this is the boot time and we did the reset, we need
         * to clear the bus reset flag in the sim_softc.
	 */

        if (cam_at_boottime())
	   sim_softc->error_recovery &= ~ERR_BUS_RESET;

	else
	   ss_reset_detected(sim_softc);

   }		/* if (resetflag) */

   if (retval == CAM_REQ_CMP)
   {
   	aha_softc->aha_state = AHA_ALIVE;	/* set adapter state */
	ctlr->alive = ALV_ALIVE;		/* set state */
   }

   else
   {
	aha_softc->aha_state = 0;	/* Adapter is not alive */
	ctlr->alive = 0;
   }

   AHA_IPL_UNLOCK (aha_softc->aha_lock, oldpri);

   return (retval);

}	/* End of aha_init() */


/*
 * aha_check_firmware()
 *
 * 	Subroutine to issue the "Read Host Adapter Inquiry Data" request
 *	to the adapter in order to get the firmware revision #.  This is
 *	checked to make sure that the adapter is running with the minimum
 *	acceptable firmware rev.  The inquiry data is also saved in
 *	the data structure to be accessed later, if upper layer requests
 *      it.
 *	If the firmware rev. is not acceptable or if we couldn't get the
 *	information, we will return error status from this routine.
 *
 *	Upon entry to this routine, the IPL is set to splbio() and
 *	AHA_SOFTC is locked.
 */

U32
aha_check_firmware( aha_softc )
register AHA_SOFTC *aha_softc;
{
   SIM_SOFTC *sim_softc = aha_softc->aha_sim_softc;
   struct controller *ctlr = aha_softc->aha_ctlr;

   register io_handle_t iobaseaddr;     /* base reg address */
   register U32 status;                 /* return status from routine calls */
   register AHA_JOB *aha_job;		/* job control block to be sent */
   register AHA_JOB *aha_job2;		/* job control block received */
   vm_offset_t kseg_addr;               /* KSEG virtual address */
   register AHA_CB *inq_cb;		/* control block for inquiry request */
   register AHA_SB *inq_sb;		/* status block for inquiry request */
   U64 inq_pa;				/* physical address of inq_data block */
   register U32 retry;			/* retry count for interrupt waiting */
   U32 g2stat;      		        /* G2STAT register content */
   U32 g2intst; 			/* G2INTST register content */
   U32 cb_pa;				/* MBOXIN register content, in this 
					 * case, it is control block address */
   register U32 istat;			/* Interrupt status bits 7 - 4 */
   register U32 targetid;		/* Target ID returned in G2INTST */

   register AHA_INQ_DATA *inq_data = aha_softc->aha_inquiry_data;

   
   SIM_MODULE(aha_check_firmware);

   SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_INOUT),
	      ("\nEntering %s", module));
   
   /* Get a control block from the free job queue */
   aha_job = aha_alloc_job( aha_softc );

   /* 
    * Set up the fields in the control block for Inquiry Data request.
    */
   inq_cb = &(aha_job->aha_cb);		/* Get ptr. to control block */
   inq_sb = &(aha_job->aha_sb);  

   inq_cb->command = AHA_OP_RHAID;	/* Set command operation code */
   aha_job->aha_cbcmdtype = AHA_OP_RHAID;	/* save command code */

   inq_cb->flag1 = AHA_F1_SES;		/* Suppress Error on Underrun */

   /* calculate PA of the inquiry data block */

   pmap_svatophys(inq_data, &inq_pa);

   inq_cb->dlptr = (U32)inq_pa;		/* Set PA of data pointer */
   inq_cb->dllen = sizeof(AHA_INQ_DATA);
   inq_cb->stat_ptr = aha_job->aha_sb_pa;

   /* Send the control block to the adapter */

   status = aha_send_cb( aha_job, sim_softc->scsiid );

   if (status != CAM_REQ_CMP)
   {
	(aha_softc->aha_total_acb)--;	/* decrement # of outstanding job */
   	aha_dealloc_job( aha_softc, aha_job );

	return (CAM_REQ_CMP_ERR);
   }

   /* 
    * Wait for the adapter to complete the I/O and service the interrupt  
    * directly to get back the data.  
    */
   retry = SEND_RETRY;			
   iobaseaddr = (io_handle_t)ctlr->physaddr;    /* get base register address */

   while (retry)
   {
	g2stat = (U32)READ_BUS_D8( (AHA_G2STAT | iobaseaddr));
	
	if (g2stat & G2STAT_IP)		/* If interrupt is pending */

	   retry = 0;			/* stop looping */

	else
	{       
	   DELAY( WAIT_100USEC );	       	/* wait 100 usec */
	   retry--;
	}
   }

   if ( !(g2stat & G2STAT_IP) )		/* never got interrupt */
	return (CAM_REQ_CMP_ERR);

   else					/* Process interrupt */
   {
	/* Read interrupt status register */

	g2intst = (U32)READ_BUS_D8( (AHA_G2INTST | iobaseaddr));

	istat = (g2intst & G2INTST_IMSK);       /* interrupt status     */
	targetid = (g2intst & G2INTST_TMSK); 	/* targid id returned */

	if ( (istat != G2INTST_CMP) && (istat != G2INTST_CAR) &&
	     (istat != G2INTST_CWE) )
	{
	   /* Clear EISA interrupt for this adapter */
	   WRITE_BUS_D8( (AHA_G2CNTRL | iobaseaddr),G2CNTRL_CLRINT );

	   WBFLUSH();		/* Do memory barrier */

	   return (CAM_REQ_CMP_ERR);
	}

	else 
	{
	   /* Read the address of the control block from the MBOXIN registers */

	   cb_pa = (U32)READ_BUS_D32( (AHA_MBOXIN | iobaseaddr));

	   /* Clear EISA interrupt for this adapter */
	   WRITE_BUS_D8( (AHA_G2CNTRL | iobaseaddr),G2CNTRL_CLRINT );

	   WBFLUSH();		/* Do memory barrier */
 

	   if (istat == G2INTST_CWE)	/* command completed w/ error */
	   {
		return (CAM_REQ_CMP_ERR);
	   }

	   else                         /* Process response block */
	   {
	      	/* Translate control block PA to virtual address */
	   	kseg_addr = PHYS_TO_KSEG( cb_pa );

		/* Calculate KSEG job block address from control block address */
		kseg_addr = AHA_JOB_PTR(kseg_addr);

		/* Now get the System VA of the job block */
		aha_job2 = (AHA_JOB *)((AHA_JOB *)kseg_addr)->aha_job_va;

		(aha_softc->aha_total_acb)--;   /* decrement # of outstanding job */

		if (aha_job2 != aha_job)	/* Not a correct job pointer */
		   return (CAM_REQ_CMP_ERR);
		else
		{
		   AHA_LOCK(aha_softc->aha_activeq[targetid][0].ahaq_lock);

		   /* Remove the job from activeq[][] */
		   AHA_REMOVE(aha_job);		

		   AHA_UNLOCK(aha_softc->aha_activeq[targetid][0].ahaq_lock);
		   
		   /* Check the firmware rev. number and return error
		    * status if less than minimally acceptable version
		    */
	 	   if (inq_data->firmware_rev < AHA_MIN_FIRMREV)
		   {
			printf ("aha_check_firmware: unsupported firmware rev.\n");
			printf ("Firmware rev. found is: %c%c%c%c\n", 
				(U8)(inq_data->firmware_rev),
				(U8)((inq_data->firmware_rev) >> 8),
				(U8)((inq_data->firmware_rev) >> 16),
				(U8)((inq_data->firmware_rev) >> 24));

			printf ("Minimum rev. required is rev. %c%c%c%c\n",
				(U8)(AHA_MIN_FIRMREV),
				(U8)((AHA_MIN_FIRMREV) >> 8),
				(U8)((AHA_MIN_FIRMREV) >> 16),
				(U8)((AHA_MIN_FIRMREV) >> 24));

			panic("(aha_check_firmware) Unsupported firmware rev. on SCSI card!");
			
       		   }
		   /*
		    * Store actual length of inquiry data.
		    */
		   else
		   {
			aha_softc->aha_inq_data_length =
					(U32)sizeof(AHA_INQ_DATA) - (inq_sb->rescnt);					
#ifdef AHADEBUG                         /* Debug code */

			printf ("aha_check_firmware: rev. found is: %c%c%c%c\n",
				(U8)(inq_data->firmware_rev), 
				(U8)((inq_data->firmware_rev) >> 8),
				(U8)((inq_data->firmware_rev) >> 16), 
				(U8)((inq_data->firmware_rev) >> 24));

#endif /* AHADEBUG */                         /* Debug code */

			return (CAM_REQ_CMP);
 		   }

		}
	   }		/* else process response block */
	   
	}


   }		/* else Process interrupt */


}	/* End of aha_check_firmware() */


/* ---------------------------------------------------------------------- */
/*		RESOURCE ALLOCATION ROUTINE				  */
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- *
 *                                                                        *
 * ROUTINE NAME:  aha_alloc_pool()					  *
 *                                                                        *
 * FUNCTIONAL DESCRIPTION:                                                *
 *	This is a routine which is called once during the controller 	  *
 *	attach time to allocate all the I/O request control blocks,       *
 *	status blocks, and scatter/gather lists to be used by this        *
 *	driver.  The AHA 1740A/1742A controller has the max. command	  *
 *	queue depths of 64.  Each I/O request consists of 48-byte control *
 *	block, 32-byte status block, and it can have up to a 1024-byte    *
 *	length scatter/gather list.  For the driver's convenience, a	  *
 *	256-byte block is also included in a job block for the sense 	  *
 *	buffer.  In addition to what the adapter requires, we have few    *
 *	more bytes at the end of the job block to keep track of the       *
 *	associated pointers such as the SIM_WS pointer and the physical	  *
 *	address of the control block structure.  			  *
 *									  *
 *	This routine will allocate 80 blocks of job blocks and set up	  *
 *	the free list queue within the AHA_SOFTC structure, also 	  *
 *	initializing the associated locks for this resource pool.  	  *
 *									  *
 *	The static values such as the physical address of the control 	  *
 *	block will be set up in this routine to minimize the process time *
 *	of each I/O at the run time.					  *
 *									  *
 *	A special job block is allocated separately for bus device        *
 *	resets.  This is the job block that will be pointed to by the	  *
 *	AHA_SOFTC structure and is used only for completing a bus	  *
 *	device reset request when the adapter interrupts the host with	  *
 *	the bus device reset completion interrupt.			  *
 *	This is necessary in order to let the I/O post-processing routine *
 *	handle the callback to the SIM layer, notifying that a bus device *
 *	reset has been performed.  Only one of this special job block is  *
 *	needed since there is only one special SIM working set assigned   *
 *	to the bus device reset operation in the SIM_SOFTC structure.     *
 *                         						  *
 *	A physically contiguous 256-byte block is allocated in order to   *
 *	get the inquiry data from the adapter.  This is kept around for   *
 *	later use and is pointed to by the AHA_SOFTC structure.           *
 *									  *
 * CALLED BY:	aha_attach()						  *
 *                                                                        *
 * FORMAL PARAMETERS:                                                     *
 *	aha_softc -- Pointer to the controller's softc struct.		  *
 *                                                                        *
 * IMPLICIT INPUTS:                                                       *
 *									  *
 * IMPLICIT OUTPUTS:                                                      *
 *									  *
 * RETURN VALUE:                                                          *
 *	CAM_REQ_CMP	if successful resource allocation		  *
 *	CAM_REQ_CMP_ERR	if failed to allocate resource			  *
 *                                                                        *
 * SIDE EFFECTS:                                                          *
 *                                                                        *
 * ---------------------------------------------------------------------- */

U32
aha_alloc_pool( aha_softc )
register AHA_SOFTC *aha_softc;
{
   register AHA_JOB *aha_job;
   register U32 i, target, lun;
   register AHA_Q *freeq, *activeq;
   U64 cb_pa;				/* the holder of phys. address */
   register int oldspl;			/* IPL to be saved and restored */


   SIM_MODULE(aha_alloc_pool);

   SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_INOUT),
	      ("\nEntering %s", module));


   /*
    * Initialize the job queues' locks and head/tail pointers   
    */
   freeq = &aha_softc->aha_freeq;

   AHA_LOCK_INIT ( freeq->ahaq_lock );

   freeq->ahaq_head = (AHA_JOB *)freeq;
   freeq->ahaq_tail = (AHA_JOB *)freeq;

   for (target = 0; target < AHA_NDPS; target++)
   {
	for (lun = 0; lun < AHA_NLPT; lun++)
	{
	   activeq = &aha_softc->aha_activeq[target][lun];
	   AHA_LOCK_INIT ( activeq->ahaq_lock );

	   activeq->ahaq_head = (AHA_JOB *)activeq;
	   activeq->ahaq_tail = (AHA_JOB *)activeq;
	}
   }

   /*
    * Allocate job blocks from the CAM resource pool and queue them
    * to the free queue.
    */
   for (i = 0; i < AHA_NUM_JOBS; i++)
   {
	aha_job = (AHA_JOB *) sc_alloc( sizeof(AHA_JOB) );

	/* If allocation failed, don't try any more */
	if (aha_job == (AHA_JOB *)NULL)
	   break;
	
	/* Save self pointer for translating PA to VA later on */
	aha_job->aha_job_va = (vm_offset_t)aha_job;

	/* Calculate the PA of the I/O control block and store */

	pmap_svatophys(&(aha_job->aha_cb), &cb_pa);
	aha_job->aha_cb_pa = (U32)cb_pa;

	/* Calculate other PAs based on control block PA */

	aha_job->aha_sb_pa = (U32)( cb_pa + sizeof(AHA_CB) );
	aha_job->aha_sglist_pa = (U32)((aha_job->aha_sb_pa) + sizeof(AHA_SB));
	aha_job->aha_snsbuf_pa = (U32)((aha_job->aha_sglist_pa) + 
					AHA_DMA_MAX_SGLENGTH);

	/* Save static softc pointer in the job block */
	aha_job->aha_softc = aha_softc; 

	/* Insert this job block to the free queue */
	AHA_IPL_LOCK ( freeq->ahaq_lock, oldspl );

	AHA_INSERT (aha_job, freeq);

	AHA_IPL_UNLOCK ( freeq->ahaq_lock, oldspl );

   }
   if (i == 0)			/* Nothing was allocated */
	return (CAM_REQ_CMP_ERR);

   /*
    * Allocate one more job block to be used for bus device resets
    */
   aha_job = (AHA_JOB *) sc_alloc( sizeof(AHA_JOB) );

   if (aha_job != (AHA_JOB *)NULL) 
   {
	/* Save self pointer for translating PA to VA later on */
	aha_job->aha_job_va = (vm_offset_t)aha_job;

	/* Calculate the PA of the I/O control block and store */
	pmap_svatophys(&(aha_job->aha_cb), &cb_pa);

	aha_job->aha_cb_pa = (U32)cb_pa;

	/* Calculate other PAs based on control block PA */

	aha_job->aha_sb_pa = (U32)( cb_pa + sizeof(AHA_CB) );
	aha_job->aha_sglist_pa = (U32)((aha_job->aha_sb_pa) + sizeof(AHA_SB));
	aha_job->aha_snsbuf_pa = (U32)((aha_job->aha_sglist_pa) + 
					AHA_DMA_MAX_SGLENGTH);

	/* Save static softc pointer in the job block */
	aha_job->aha_softc = aha_softc;

	aha_job->aha_jobtype = AHA_SPECIAL;	/* device reset job block */
	
	aha_softc->aha_bdr_job = aha_job;	/* save in softc struct. */
   }
   else		/* Allocation failed for BDR job block */
   {
	if (i > AHA_NDPS) 		/* we have several job blocks */
	{
	   AHA_IPL_LOCK ( freeq->ahaq_lock, oldspl );

	   aha_job = (AHA_JOB *) AHA_REMOVE_HEAD(freeq); 

	   AHA_IPL_UNLOCK ( freeq->ahaq_lock, oldspl );

           aha_job->aha_jobtype = AHA_SPECIAL;  /* device reset job block */

	   aha_softc->aha_bdr_job = aha_job;	/* save in softc struct. */
	}
	else			/* clean up resource and return */
	{
	   AHA_IPL_LOCK ( freeq->ahaq_lock, oldspl );

	   while (i)     
	   {
		aha_job = (AHA_JOB *) AHA_REMOVE_HEAD(freeq);
		sc_free(aha_job, sizeof(AHA_JOB));
		i--;
	   }
	   AHA_IPL_UNLOCK ( freeq->ahaq_lock, oldspl );

	   return (CAM_REQ_CMP_ERR);
	}
   }

   /*
    * Allocate a 256-byte physically contiguous block to get
    * the adapter inquiry data.
    */
   aha_softc->aha_inquiry_data = (AHA_INQ_DATA *) sc_alloc( sizeof(AHA_INQ_DATA) );

   if ( (aha_softc->aha_inquiry_data) == (AHA_INQ_DATA *)NULL )
   {
	/* Take one from the free queue */
	AHA_IPL_LOCK ( freeq->ahaq_lock, oldspl );

	aha_softc->aha_inquiry_data = (AHA_INQ_DATA *) AHA_REMOVE_HEAD(freeq);

	AHA_IPL_UNLOCK ( freeq->ahaq_lock, oldspl );

	/* Clear buffer since it's not all clear in this case */
	bzero(aha_softc->aha_inquiry_data, sizeof(AHA_INQ_DATA));
   }

   return (CAM_REQ_CMP);

}	/* End of aha_alloc_pool() */


/* ---------------------------------------------------------------------- */
/* 		NORMAL I/O HANDLING ROUTINES		                  */
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- *
 *                                                                        *
 * ROUTINE NAME:  aha_go()						  *
 *                                                                        *
 * FUNCTIONAL DESCRIPTION:                                                *
 *	This is the HBA_GO routine for this driver.  The routine gets an  *
 *	adapter request control block from the free queue, sets up the    *
 *	control block according to the request type, and sets up the      *
 *	scatter/gather list for the current request if the request        *
 *	involves the data transfer and the data buffer is not physically  *
 *	contiguous.  Once the request is set up, the driver then sends it *
 *	to the adapter for processing.                                    *
 *	                                                                  *
 *	The AHA adapter does not accept any SCSI command length greater   *
 *	than 12 bytes.  Hence if any CDB length greater than 12 bytes is  *
 *	passed to this routine, the routine will return CAM_PROVIDE_FAIL  *
 *	error status.  Also the maximum data transfer size is 16 MB.      *
 *	If the data count passed is greater than 16 MB, an error status   *
 *	is returned.                                                      *
 *                                                                        *
 * CALLED BY:  ss_start() in sim_sched.c                                  *
 *                                                                        *
 * FORMAL PARAMETERS:                                                     *
 *		sws -- Pointer to the SIM working set for the request     *
 *			to be sent					  *
 *                                                                        *
 * IMPLICIT INPUTS:                                                       *
 *                                                                        *
 * IMPLICIT OUTPUTS:                                                      *
 *                                                                        *
 * RETURN VALUE:                                                          *
 *		CAM_REQ_CMP -- CCB request sent without error             *
 *		CAM_BUSY -- CAM HBA subsystem is busy		          *
 *		CAM_PROVIDE_FAIL -- Unable to provide requested capability*
 *		CAM_NO_HBA -- The adapter is off-line, i.e. is dead       *
 *                                                                        *
 * SIDE EFFECTS:                                                          *
 *                                                                        *
 * ---------------------------------------------------------------------- */

U32
aha_go( sws )
register SIM_WS *sws;
{
   register SIM_SOFTC *sim_softc = sws->sim_sc;
   register AHA_SOFTC *aha_softc = (AHA_SOFTC *)(sim_softc->hba_sc);
   register CCB_SCSIIO *ccb_ptr = sws->ccb;

   register AHA_JOB *aha_job;		/* job control block         */
   register io_handle_t iobaseaddr;     /* base reg address */

   register U32 retval = CAM_REQ_CMP;   /* return value from this routine */
   int oldpri; 			        /* IPL to be saved */
   U32 targetid, lun;			/* target ID and LUN number */


   SIM_MODULE(aha_go);

   SIM_PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	       ("\nEntering %s", module) );

   SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	      ("Entry - sws=0x%x\n", sws));
	       

   AHA_IPL_LOCK(aha_softc->aha_lock, oldpri);      /* Lock softc structure */

   /* Check if the adapter is operational for normal I/O */

   if ( (aha_softc->aha_state & AHA_RESET_IN_PROGRESS) || 
	(aha_softc->aha_state & AHA_BUS_RESET_IN_PROGRESS) )
   {
	SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_ERRORS),
		   ("aha_go - CAM_BUSY, return\n"));

	retval = CAM_BUSY;
   }

   else if ( !(aha_softc->aha_state & AHA_ALIVE) )
   {
	SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_ERRORS),
	          ("aha_go - CAM_NO_HBA, return\n"));

	retval = CAM_NO_HBA;
   }

   AHA_IPL_UNLOCK(aha_softc->aha_lock, oldpri);


   if (retval != CAM_REQ_CMP)		/* return if adapter not operational */
	return (retval);

   if (ccb_ptr->cam_cdb_len > AHA_MAX_CDB_LENGTH)
   {
	SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_ERRORS),
		  ("aha_go - can't support more than 12 bytes CDB\n"));

	return (CAM_PROVIDE_FAIL);
   }

   /*
    * The driver does not support linked commands or tagged commands yet.
    */
   if ( (sws->cam_flags & CAM_CDB_LINKED) ||
	(sws->flags & SZ_TAGGED) )
   {
	SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_ERRORS),
		   ("aha_go - no support for linked or tagged commands\n"));

	return (CAM_PROVIDE_FAIL);
   }

   /* Get the current request's target ID and LUN */

   targetid = sws->targid; 
   lun = sws->lun;

   /* 
    * If there is an outstanding I/O job already for this I-T-L nexus
    * and it is a non-tagged I/O, we need to return busy status.
    */
   if ( aha_check_activeq(targetid, lun, aha_softc) == CAM_TRUE )
	return (CAM_BUSY);

   aha_job = aha_alloc_job( aha_softc );	/* get a job block */

   if (aha_job == (AHA_JOB *)NULL)
	return (CAM_BUSY);

   /* 
    * Set up the job block with information passed in SIM working set & CCB 
    */

   retval = aha_setup_job(aha_job, aha_softc, sws);

   if (retval == CAM_REQ_CMP)
   
		/* Send the control block to the adapter */
	retval = aha_send_cb( aha_job, targetid );

   /* 
    * Either data buffer address mapping failed, or we couldn't send
    * the control block to the adapter.
    */
   if (retval != CAM_REQ_CMP) 
   {
	AHA_IPL_LOCK (aha_softc->aha_lock, oldpri);      /* Lock softc structure */

	(aha_softc->aha_total_acb)--;   /* decrement # of outstanding job */

	AHA_IPL_UNLOCK (aha_softc->aha_lock, oldpri);

	aha_dealloc_job( aha_softc, aha_job );

	return (retval);

   }


   return (retval);

}	/* End of aha_go() */


/*
 * aha_check_activeq(()
 *
 *  	Subroutine to the aha_go().  It checks the activeq[target][lun]
 *	to see if there is already an outstanding non-tagged I/O for
 * 	this I-T-L nexus.  If so, it returns CAM_TRUE.  If not, it
 *	returns CAM_FALSE.
 */

U32
aha_check_activeq( targetid, lun, aha_softc )
U32 targetid;
U32 lun;
AHA_SOFTC *aha_softc;
{
   register AHA_JOB *jobptr;            /* ptr. to job block    */
   register AHA_Q *activeq;		/* ptr. to active job queue */
   register int oldpri;                 /* IPL to be saved and restored */


   SIM_MODULE(aha_check_activeq);

   SIM_PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	       ("\nEntering %s", module) );

   /* Get activeq for this target ID & lun */

   activeq = &(aha_softc->aha_activeq[targetid][lun]);

   AHA_IPL_LOCK (activeq->ahaq_lock, oldpri);      /* Lock the queue */

   jobptr = activeq->ahaq_head;		/* Look at first thing in the queue */

   AHA_IPL_UNLOCK (activeq->ahaq_lock, oldpri);

   if (jobptr == (AHA_JOB *)activeq)	/* Queue is empty */
	return (CAM_FALSE);

   else if (jobptr->aha_jobtype == AHA_NON_TAGGED)
	return (CAM_TRUE);

   else
	return (CAM_FALSE);


}	/* End of aha_check_activeq() */


/*
 * aha_setup_job()
 *
 * 	Subroutine to the aha_go().  It sets up the job block and
 *	the associated control block with the information passed
 *	in SIM working set and CCB.
 *
 */

U32
aha_setup_job( aha_job, aha_softc, sws )
AHA_JOB *aha_job;
AHA_SOFTC *aha_softc;
SIM_WS *sws;
{
   AHA_CB *cbptr;             		/* I/O control block pointer */
   CCB_SCSIIO *ccb_ptr = sws->ccb;
   U32	 cmd_size, i; 
   U8 *cam_cdb;
   U32 retval = CAM_REQ_CMP;		/* return status from this routine */


   SIM_MODULE(aha_setup_job);

   SIM_PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	       ("\nEntering %s", module) );

   aha_job->aha_sws = sws;		/* Save SIM working set pointer */
   aha_job->aha_ccb = ccb_ptr;		/* Save pointer to CCB */

   if (sws->flags & SZ_TAGGED)		/* tagged I/O request */
   	aha_job->aha_jobtype = AHA_TAGGED;

   else
	aha_job->aha_jobtype = AHA_NON_TAGGED;

   cbptr = &(aha_job->aha_cb);		/* get control block pointer */ 

   /* Copy the physical address of the status block to control block field */

   cbptr->stat_ptr = aha_job->aha_sb_pa;

   /* Copy the SCSI command bytes to the control block */
 
   cmd_size = ccb_ptr->cam_cdb_len;
   cbptr->cdblen = cmd_size;

   if (ccb_ptr->cam_ch.cam_flags & CAM_CDB_POINTER)
   
	cam_cdb = ccb_ptr->cam_cdb_io.cam_cdb_ptr;

   else
	cam_cdb = ccb_ptr->cam_cdb_io.cam_cdb_bytes;
  
   for (i = 0; i < cmd_size; i++)
  	cbptr->cdb[i] = *cam_cdb++;


   /*
    * Set up the fields in the I/O control block for normal command
    */
   cbptr->command = AHA_OP_ISC;	  	/* Initiator SCSI Command */
   aha_job->aha_cbcmdtype = AHA_OP_ISC; 	/* Save command type */

   /* Set up scatter/gather list or data pointer if data buffer is present */
	 
   if ( (ccb_ptr->cam_dxfer_len) > 0 )
	retval = aha_dma_setup( aha_job );
	
   if (retval != CAM_REQ_CMP)
	return (retval);		/* return error status */

   /*
    * Set the data_xfer values to allow sim_xpt.c's
    * sx_command_complete to calculate the residual
    * number of bytes ( not transferred )
    */
   sws->data_xfer.data_count = ccb_ptr->cam_dxfer_len;

   /* 
    * Check the autosense flag and set up flag word and sense buffer
    * fields in the control block if autosense is allowed
    */
   if (!( sws->cam_flags & CAM_DIS_AUTOSENSE ))
   {
	cbptr->flag1 |= AHA_F1_ARS; 	
	cbptr->snsptr = aha_job->aha_snsbuf_pa;
	cbptr->snslen = ccb_ptr->cam_sense_len;
	aha_job->aha_senseflag = TRUE;
   }

   /* 
    * Set up the flag2 word in the control block with LUN info
    * and disconnect information
    */
   cbptr->flag2 = (U16)sws->lun;   /* copy LUN number */

   if (sws -> cam_flags & CAM_DIS_DISCONNECT)
	cbptr->flag2 |= AHA_F2_ND;   /* Set no disconnect bit */

   /* 
    * Set 'Data Transfer - Check Direction' and 'Direction of Transfer' bits
    * in the flag2 word if we have data transfer request
    */
   if ( (ccb_ptr->cam_dxfer_len) > 0 )
   {
	cbptr->flag2 |= AHA_F2_CD;	/* Check direction bit */

	if (sws->cam_flags & CAM_DIR_IN)
	   cbptr->flag2 |= AHA_F2_DIR;		/* Read operation */
   }

   cbptr->flag2 |= AHA_F2_NRB;		/* No retry on BUSY status bit */

   /* 
    * The following special set up for REQUEST SENSE command might not be
    * necessary after all.  But for now, we will do the same as other
    * O.S. groups for processing REQUEST SENSE command.
    */

   if (cbptr->cdb[0] == ALL_REQ_SENSE6_OP)
   {
	cbptr->command = AHA_OP_RSI;       /* Read Sense Information       */
	aha_job->aha_cbcmdtype = AHA_OP_RSI; 	/* save command type */

	cbptr->snsptr = aha_job->aha_snsbuf_pa;
	cbptr->snslen = ccb_ptr->cam_dxfer_len;
	aha_job->aha_senseflag = TRUE;

	cbptr->dlptr = 0;
	cbptr->dllen = 0;
	cbptr->cdblen = 0;		/* reset the CDB length for this */

	for (i = 0; i < AHA_MAX_CDB_LENGTH; i++)
	   cbptr->cdb[i] = 0;

        cbptr->flag1 = 0;
	cbptr->flag2 = (U16)sws->lun;	/* copy LUN number */

	if (sws -> cam_flags & CAM_DIS_DISCONNECT)
	   cbptr->flag2 |= AHA_F2_ND;	/* Set no disconnect bit */

   }


   return (retval);

}	/* End of aha_setup_job() */


/* ---------------------------------------------------------------------- *
 *                                                                        *
 * ROUTINE NAME:  aha_alloc_job()					  *
 *                                                                        *
 * FUNCTIONAL DESCRIPTION:                                                *
 *	This routine gets a job block from the free queue.  It checks the *
 *	total number of outstanding I/O control blocks that are sent to   *
 *	the adapter.  If the total number is 64 which is the limit of the *
 *	adapter, the routine returns NULL value to the caller.            *
 *                                                                        *
 *	If the total number of outstanding I/Os is less than 64, the      *
 *	routine will take off a job block from the free queue and return  *
 *	the pointer to this job block.  In this case, the total number of *
 *	outstanding control block, 'aha_total_acb', is incremented by one *
 *	with proper lock held.  This is done here even though the control *
 *	block hasn't been sent to the adapter yet because there could be  *
 *	a case where the block is allocated but hasn't been sent to the	  *
 *	adapter yet, and another user request thread comes in and tries   *
 *	to get a control block.  If the counter is not incremented here,  *
 *	the second thread will also see the same value as the first       *
 *	thread and will try to allocate another job block.                *
 *									  *
 *	If the free queue was empty, NULL value is returned.  		  *
 *									  *
 * CALLED BY:  aha_go() and aha_check_firmware()			  *
 *									  *
 * FORMAL PARAMETERS:                                                     *
 *		aha_softc -- Pointer to the AHA_SOFTC structure		  *
 *                                                                        *
 * IMPLICIT INPUTS:                                                       *
 *									  *
 * IMPLICIT OUTPUTS:							  *
 * 									  *
 * RETURN VALUE:                                                          *
 *		Pointer to the AHA_JOB block allocated or NULL		  *
 *                                                                        *
 * SIDE EFFECTS:                                                          *
 *                                                                        *
 * ---------------------------------------------------------------------- */
 
AHA_JOB *
aha_alloc_job( aha_softc )
register AHA_SOFTC *aha_softc;
{
   register AHA_JOB *jobptr;		/* ptr. to job block 	*/
   register AHA_Q *freeq;		/* ptr. to the job free queue */
   register int oldpri;			/* IPL to be saved and restored */


   SIM_MODULE(aha_alloc_job);

   SIM_PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	       ("\nEntering %s", module) );

   AHA_IPL_LOCK (aha_softc->aha_lock, oldpri);      /* Lock softc structure */

   if( (aha_softc->aha_total_acb) >= AHA_MAXACB )
   {
#ifdef AHADEBUG                         /* Debug code */
	printf("aha_alloc_job: # of outstanding jobs = %d\n", 
	       (aha_softc->aha_total_acb) );

#endif /* AHADEBUG */                         /* Debug code */

	AHA_IPL_UNLOCK (aha_softc->aha_lock, oldpri);

	return ( (AHA_JOB *)NULL );
   }

   freeq = &aha_softc->aha_freeq;	  

   AHA_LOCK ( freeq->ahaq_lock );

   jobptr = (AHA_JOB *) AHA_REMOVE_HEAD(freeq);

   AHA_UNLOCK ( freeq->ahaq_lock );

   if ( jobptr == (AHA_JOB *)NULL )
   {
	AHA_IPL_UNLOCK (aha_softc->aha_lock, oldpri);

	return ( (AHA_JOB *)NULL );
   }

   else
   {
	/* Increment the number of active job blocks here */

	aha_softc->aha_total_acb++;

	AHA_IPL_UNLOCK (aha_softc->aha_lock, oldpri);

	return ( jobptr );
   }

} 	/* End of aha_alloc_job() */


/* ---------------------------------------------------------------------- *
 *                                                                        *
 * ROUTINE NAME:  aha_dealloc_job()					  *
 *                                                                        *
 * FUNCTIONAL DESCRIPTION:                                                *
 *	This routine requeues the passed job block back into the free     *
 *	queue.  Before the job block is queued, the control block, the    *
 *      status block, the scatter/gather list area, and the sense buffer  *
 *	area will be reset to zero by this routine.                       *
 *                                                                        *
 * CALLED BY:  aha_check_firmware(), aha_go(), aha_iodone(),   		  *
 *	       aha_flush_lun_queue()					  *
 *                                                                        *
 * FORMAL PARAMETERS:                                                     *
 *		aha_softc -- Pointer to the AHA_SOFTC structure           *
 *		aha_job -- Pointer to the job block being returned 	  *
 *                                                                        *
 * IMPLICIT INPUTS:                                                       *
 *                                                                        *
 * IMPLICIT OUTPUTS:                                                      *
 *                                                                        *
 * RETURN VALUE:                                                          *
 *                                                                        *
 * SIDE EFFECTS:                                                          *
 *	The job block is queued back to the free queue.                   *
 *                                                                        *
 * ---------------------------------------------------------------------- */

void
aha_dealloc_job( aha_softc, aha_job )
register AHA_SOFTC *aha_softc;
register AHA_JOB *aha_job;
{
   register AHA_Q *freeq;               /* ptr. to the job free queue */
   register int oldspl;                 /* IPL to be saved and restored */
   register U32 length;			/* # of bytes to clear		*/


   SIM_MODULE(aha_dealloc_job);

   SIM_PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	       ("\nEntering %s", module) );


   /*
    * Zero out the areas in the job block being returned,
    * leaving only static areas: the self pointer to the block,
    * various physical addresses, and the pointer to the 
    * AHA_SOFTC structure at the end
    * of the AHA_JOB structure.  This is done by subtracting
    * the length of these static fields (1 quadword, 4 longwords and
    * the size of a structure pointer) from the total
    * length of the AHA_JOB structure.
    */

   length = sizeof(AHA_JOB) - ((sizeof(vm_offset_t)) + (sizeof(U32)*4) 
			+ sizeof(AHA_SOFTC *));

   bzero( aha_job, length ); 

   freeq = &aha_softc->aha_freeq;

   AHA_IPL_LOCK ( freeq->ahaq_lock, oldspl );

   AHA_INSERT_TAIL ( freeq, aha_job );

   AHA_IPL_UNLOCK ( freeq->ahaq_lock, oldspl );

   return;

}	/* End of aha_dealloc_job() */


/* ---------------------------------------------------------------------- *
 *                                                                        *
 * ROUTINE NAME:  aha_send_cb()						  *
 *                                                                        *
 * FUNCTIONAL DESCRIPTION:                                                *
 *	This routine sends an I/O control block to the adapter.  If       *
 *	successfully sent, it also queues the job block to the outstanding*
 *	job queue, sorted by the target ID and LUN number.  If failed to  *
 *	send the control block due to the adapter's mailbox out register  *
 *	not being free, the error status is returned to the caller.       *
 *                                                                        *
 * CALLED BY:  aha_go() and aha_check_firmware()			  *
 *                                                                        *
 * FORMAL PARAMETERS:                                                     *
 *		aha_job -- Pointer to the job block which contains the    *
 *			   control block to be sent			  *
 *		targetid -- Target SCSI ID to be written to the ATTN      *
 *			    register					  *
 *                                                                        *
 * IMPLICIT INPUTS:                                                       *
 *		LUN number in the control block's flag2 field             *
 *                                                                        *
 * IMPLICIT OUTPUTS:                                                      *
 *                                                                        *
 * RETURN VALUE:                                                          *
 *		CAM_REQ_CMP -- if successful				  *
 *		CAM_BUSY -- if failed to send the control block	          *
 *                                                                        *
 * SIDE EFFECTS:                                                          *
 *                                                                        *
 * ---------------------------------------------------------------------- */

U32
aha_send_cb( aha_job, targetid )
register AHA_JOB *aha_job;
U32	targetid;
{
   register AHA_SOFTC *aha_softc = aha_job->aha_softc;
   register struct controller *ctlr = aha_softc->aha_ctlr;

   register AHA_CB *cbptr = &(aha_job->aha_cb);   /* Control block pointer */

   register AHA_Q *activeq;		/* Pointer to the active queue */
   register int oldpri;			/* IPL to be saved and restored */
   U32 g2stat;  	                /* G2STAT register content */
   register U32 retry;                  /* retry count for sending I/O  */
   register io_handle_t iobaseaddr;     /* base reg address */
   register U32 lun;			/* LUN number for this request */
   U32	jobsent = FALSE;		/* flag indicating job was sent */


   SIM_MODULE(aha_send_cb);

   SIM_PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	       ("\nEntering %s", module) );



   iobaseaddr = (io_handle_t)ctlr->physaddr;    /* get base register address */
   lun = (cbptr->flag2 & AHA_F2_LUNMSK);	/* get LUN number */ 

   retry = SEND_RETRY;				/* try up to 10 times */

   AHA_IPL_LOCK (aha_softc->aha_lock, oldpri);      /* Lock softc structure */

   while (retry)
   {
	g2stat = (U32)READ_BUS_D8( (AHA_G2STAT | iobaseaddr));
	
	/* Check MBOXOUT empty bit and BUSY bit */

	if ( (g2stat & G2STAT_MOE) && (!(g2stat & G2STAT_BUSY)) )	
	{
	   /* Write physical address of control block to MBOXOUT registers */

	   WRITE_BUS_D32( (AHA_MBOXOUT | iobaseaddr),(aha_job->aha_cb_pa) );

	   WBFLUSH();		/* do memory barrier */

	   /* Write op code and SCSI ID of the target to ATTN register */
	   
	   WRITE_BUS_D8( (AHA_ATTN | iobaseaddr),(ATTN_OP_START | targetid) );

	   WBFLUSH();

	   retry = 0;
	   jobsent = TRUE;

	   /* Save current I/O control block pointers in AHA_SOFTC */

	   aha_softc->aha_cb_pa = aha_job->aha_cb_pa;
	   aha_softc->aha_cb_va = cbptr; 

	   /* Insert the job block to active queue */

	   activeq = &aha_softc->aha_activeq[targetid][lun];

	   AHA_LOCK(activeq->ahaq_lock);

	   AHA_INSERT_TAIL(activeq, aha_job); 

	   AHA_UNLOCK(activeq->ahaq_lock);

	}
	else
	{
	   retry--;
	   DELAY( WAIT_10USEC );		/* Wait 10 usec. before retry */
	}
   }

   AHA_IPL_UNLOCK (aha_softc->aha_lock, oldpri);

   if (jobsent)
	return (CAM_REQ_CMP);

   else
	return (CAM_BUSY);

}	/* End of aha_send_cb() */

/* ---------------------------------------------------------------------- *
 *                                                                        *
 * ROUTINE NAME:  aha_dma_setup()					  *
 *                                                                        *
 * FUNCTIONAL DESCRIPTION:                                                *
 *	This routine is called to set up the mapping of the user data     *
 *	buffer into the scatter/gather list that the adapter can          *
 *	understand and operate on.  If the CCB already has a CAM scatter/ *
 *	gather elements, this routine tries to map each CAM s/g segment   *
 *	to potentially multiple adapter s/g segments depending on if the  *
 *	passed CAM s/g element is physically contiguous or not.  Since a  *
 *	CAM s/g element is guaranteed only to be virtually contiguous,    *
 *	one CAM s/g element could map into several adapter s/g segments.  *
 *	If, after trying to map the data buffer, it turns out that the    *
 *	entire buffer is physically contiguous, this routine will set up  *
 *	the adapter control block fields to use one data pointer instead  *
 *	of the scatter/gather list.                                       *
 *                                                                        *
 * CALLED BY:  aha_setup_job()                                            *
 *                                                                        *
 * FORMAL PARAMETERS:                                                     *
 *		aha_job -- Pointer to the job block for the current       *
 *			   I/O request					  *
 *                                                                        *
 * IMPLICIT INPUTS:                                                       *
 *		aha_job->aha_ccb -- Pointer to the CCB			  *
 *                                                                        *
 * IMPLICIT OUTPUTS:                                                      *
 *		aha_job->aha_cb.dlptr -- set to data buffer physical      *
 *				        address or s/g list address       *
 *		aha_job->aha_cb.dllen -- set to data length or s/g list   *
 *					length                            *
 *                                                                        *
 * RETURN VALUE:                                                          *
 *		CAM_REQ_CMP -- Successful data buffer mapping		  *
 *		CAM_REQ_INVALID -- A virtual address passed couldn't get  *
 *			           translated to the physical address     *
 *		CAM_PROVIDE_FAIL -- The data buffer couldn't fit into the *
 *				    scatter/gather list                   *
 *                                                                        *
 * SIDE EFFECTS:                                                          *
 *	Scatter/gather list is built or a data pointer is set up.         *
 *                                                                        *
 * ---------------------------------------------------------------------- */

U32
aha_dma_setup( aha_job )
register AHA_JOB *aha_job;
{
   register AHA_CB *cbptr = &(aha_job->aha_cb);   /* Control block pointer */
   register CCB_SCSIIO *ccb_ptr = aha_job->aha_ccb;   /* CCB pointer */
   register SG_SEGMENT *sg_seg;			/* s/g segment pointer */
   register U32 num_seg = 0;			/* Number of s/g segments used */

   register U32 cam_data_length;                /* Current CAM s/g data size  * 
						 * or total CAM data size     */

   register U32 cam_num_seg;			/* Number of CAM s/g segments */
   register SG_ELEM *cam_sg;			/* Pointer to CAM s/g list    */
   register vm_offset_t data_addr;		/* Address to be mapped       */
   register U32 count;				/* # of bytes mapped          */

   register U32 total_count = 0;		/* total # of bytes mapped so far */


   SIM_MODULE(aha_dma_setup);

   SIM_PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	       ("\nEntering %s", module) );

   if (ccb_ptr->cam_dxfer_len > AHA_DMA_MAX_SIZE)
	return (CAM_PROVIDE_FAIL);

   sg_seg = &(aha_job->aha_sglist[0]);	/* Get first s/g seg. pointer */
   
   /* 
    * Take care of the case when CAM s/g elements are present 
    */
   if (ccb_ptr->cam_ch.cam_flags & CAM_SCATTER_VALID)
   {
	cam_num_seg = ccb_ptr->cam_sglist_cnt;
	cam_sg = (SG_ELEM *)ccb_ptr->cam_data_ptr;

	/* Get first s/g data address */
	data_addr = (vm_offset_t)(cam_sg->cam_sg_address);	

	cam_data_length = cam_sg->cam_sg_count; /* Get first s/g data count */

 	while ( (num_seg < AHA_DMA_MAX_SEGMENTS) &&
		(cam_num_seg > 0) )
	{
	   count = aha_map_data( data_addr, cam_data_length,
				 sg_seg, ccb_ptr->cam_req_map );

	   if (!count)
		return (CAM_REQ_INVALID);

	   else
	   {
		/* Calculate remaining # of bytes in current CAM s/g segment */
		cam_data_length -= count;	      

		total_count += count;		/* bytes mapped so far */
		
		num_seg++;			/* number of segments used */
		sg_seg++;			/* point to next s/g segment to use */

		if (cam_data_length == 0)	/* no more bytes in current CAM s/g */
		{
		   cam_sg++;			/* point to next CAM s/g segment */
		   cam_num_seg--;

		   if (cam_num_seg)		/* We still have more CAM s/g segs */
		   {
		      	data_addr = (vm_offset_t)(cam_sg->cam_sg_address); 
			cam_data_length = cam_sg->cam_sg_count;
		   } 

		}
		else
		   data_addr += count;		/* update data ptr within CAM seg. */

	   }

	}	/* while */

	if (cam_num_seg > 0)		/* still have bytes to map */
	   return (CAM_PROVIDE_FAIL);

   }

   /* 
    * Take care of the case when one data pointer is passed in CCB 
    */
   else
   {
	/* get number data bytes to map */
	cam_data_length = ccb_ptr->cam_dxfer_len;

	data_addr = (vm_offset_t)(ccb_ptr->cam_data_ptr);

	while ( (num_seg < AHA_DMA_MAX_SEGMENTS) && cam_data_length )
	{
	   count = aha_map_data( data_addr, cam_data_length,
				 sg_seg, ccb_ptr->cam_req_map );

	   if (!count)
		return (CAM_REQ_INVALID);

	   else
	   {
		cam_data_length -= count;
		data_addr += count;		/* update data address */

		total_count += count;           /* bytes mapped so far */

		num_seg++;                      /* number of segments used */
		sg_seg++;                       /* point to next s/g segment to use */
	   }
		
	}	/* while */

	if (cam_data_length > 0)	/* still have bytes to map */
	   return (CAM_PROVIDE_FAIL);
   }


   /* If only one data pointer was needed, write this to the control block */

   if (num_seg == 1)
   {
	cbptr->dlptr = aha_job->aha_sglist[0].dataptr;
	cbptr->dllen = aha_job->aha_sglist[0].count;
   }

   /* Else write s/g list pointer and the s/g list length */

   else
   {
	cbptr->flag1 |= AHA_F1_SG;	/* set scatter/gather bit flag */

	/* Write physical address of scatter/gather list */
	cbptr->dlptr = aha_job->aha_sglist_pa;

	cbptr->dllen = num_seg * (sizeof(SG_SEGMENT));

   	aha_job->aha_sgcount = num_seg;		/* save number of s/g segs used */
   }

   return (CAM_REQ_CMP);

}	/* End of aha_dma_setup() */


/* ---------------------------------------------------------------------- *
 *                                                                        *
 * ROUTINE NAME:  aha_map_data()					  *
 *                                                                        *
 * FUNCTIONAL DESCRIPTION:                                                *
 *	This is a subroutine to the aha_dma_setup().  It tries to map as  *
 *	many bytes as possible using one scatter/gather list segment.     *
 *	The routine checks if the source address passed is a user space   *
 *	virtual address or a kernel virtual address.  If it is a user     *
 *	virtual address, the mapping information passed is used to        *
 *	translate the address to the physical address.  If it is a kernel *
 *	virtual address, it uses a kernel translation routine to          *
 *	translate.  If for some reason, the translation fails, the        *
 *	routine returns 0 to notify this event.  Once the address is      *
 *	translated, the routine looks at one virtual page at a time to    *
 *	see if it is physically contiguous from the previous page.  If    *
 *	so, the routine will continue incrementing the bytes mapped until *
 *	it is done with the length requested or if it reaches the         *
 *	maximum length supported by one scatter/gather segment, which is  *
 *	4 MB.								  *
 *									  *
 *	Note that current implementation will check the translated        *
 *	physical address and if it is larger than 32-bit, it will return  *
 *	0.  This is because this adapter can only handle up to 32-bit     *
 *	address and currently we don't have mapping registers in the      *
 *	systems that use this adapter.  When and if such system is        *
 *	implemented in the future, this routine can be modified to take   *
 *	advantage of the system mapping registers.			  *
 *                                                                        *
 * CALLED BY:  aha_dma_setup()                                            *
 *                                                                        *
 * FORMAL PARAMETERS:                                                     *
 *		addr -- Address of the first byte to be mapped		  *
 *		length -- Size of the current data buffer segment to be   *
 *			  mapped					  *
 *		segptr -- Pointer to the adapter's scatter/gather list    *
 *			  current segment entry				  *
 *		map_info_buf -- CAM provided mapping information buffer   *
 *                                                                        *
 * IMPLICIT INPUTS:                                                       *
 *                                                                        *
 * IMPLICIT OUTPUTS:                                                      *
 *                                                                        *
 * RETURN VALUE:                                                          *
 *		Number of bytes mapped or 0 if address translation failed *
 *                                                                        *
 * SIDE EFFECTS:                                                          *
 *                                                                        *
 * ---------------------------------------------------------------------- */

U32
aha_map_data( addr, length, segptr, map_info_buf )
register vm_offset_t addr;
register U32 length;
register SG_SEGMENT *segptr;
struct buf *map_info_buf;
{
   register U64 size;
   U64 lastaddr, newaddr;
   

   SIM_MODULE(aha_map_data);

   SIM_PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	       ("\nEntering %s", module) );
	       
   if (CAM_IS_KUSEG(addr))		/* user space VA */
   {
	/* Translate to physical address */
	lastaddr = pmap_extract(map_info_buf->b_proc->task->map->vm_pmap, addr);

	if (lastaddr == NULL)		/* physical address is NULL */
	   return (0);
   }

   /* kernel virtual address case */

   else if (pmap_svatophys(addr, &lastaddr) == KERN_INVALID_ADDRESS)
	   return (0);

   /* If translated physical address is larger than 32-bit, return error */
   if (lastaddr > AHA_MAX_ADDRESS)
	return (0);

   segptr->dataptr = (U32)lastaddr;	/* Write the PA to segment entry */

   /* Make sure that no more than the maximum number of bytes per
    * entry allowed by the adapter is being mapped (4 MBytes).
    */

   if (length >= AHA_DMA_MAX_COUNT)
	length = AHA_DMA_MAX_COUNT;
   
   while (length)
   {
	/* look at only one page at a time through the loop */
	size = (NBPG)-(lastaddr & 0x1fff);
	if(size > length)
	   size = length;

	segptr->count += size;		/* update segment data count */
	addr += size;			/* update address to translate next */
	length -= size;

	/* if all bytes have been processed */
	if(!length)
	   break;

	if (CAM_IS_KUSEG(addr))		/* user space VA */
	{
	   newaddr = pmap_extract(map_info_buf->b_proc->task->map->vm_pmap, addr);

	   if (newaddr == NULL) 	/* physical address is NULL */
		return (0);
	}
	else if (pmap_svatophys(addr, &newaddr) == KERN_INVALID_ADDRESS)
	   return (0);

	/* If translated physical address is larger than 32-bit, return error */
	if (newaddr > AHA_MAX_ADDRESS)
	   return (0);

	/*
	 * If the next page is not physically contiguous with the last
	 * page then stop here.
	 */
	if ((newaddr - lastaddr) > NBPG)
	   break;
	lastaddr = newaddr;

   }

   return (segptr->count);	/* return number of bytes mapped */


}	/* End of aha_map_data() */


/* ---------------------------------------------------------------------- */
/* 		INTERRUPT PROCESSING ROUTINES				  */
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- *
 *                                                                        *
 * ROUTINE NAME:  ahaintr()						  *
 *                                                                        *
 * FUNCTIONAL DESCRIPTION:                                                *
 *	This is the interrupt routine called by the system interrupt      *
 *	service.  This interrupt routine will get the corresponding       *
 *	aha_softc pointer by first getting the SIM_SOFTC pointer for this *
 *	adapter from the softc_directory[] and then call the              *
 *	aha_process_intr() to actually process the interrupt only if      *
 *	there is a corresponding aha_softc pointer in the SIM_SOFTC       *
 *	structure.  If the controller softc pointer field in the SIM_SOFTC*
 *	structure is 0, the interrupt is dismissed as a spurious interrupt*
 *	occuring before the driver is fully initialized.  It is assumed   *
 *	that the EOI (end of interrupt) notification for the EISA bus     *
 *	will be done by the system interrupt service when we return from  *
 *	this routine.                                                     *
 *                                                                        *
 * CALLED BY:  system interrupt service routine                           *
 *                                                                        *
 * FORMAL PARAMETERS:                                                     *
 *              ctlr_num -- Controller number found in 'controller'       *
 *                          structure's 'ctlr_num' field                  *
 *                                                                        *
 * IMPLICIT INPUTS:                                                       *
 *		softc_directory[] -- contains arrays of SIM_SOFTC pointers*
 *                                                                        *
 * IMPLICIT OUTPUTS:                                                      *
 *                                                                        *
 * RETURN VALUE:                                                          *
 *		none							  *
 *                                                                        *
 * SIDE EFFECTS:                                                          *
 *                                                                        *
 * ---------------------------------------------------------------------- */

void
ahaintr( ctlr_num )
int ctlr_num;
{
   SIM_SOFTC *sim_softc = (SIM_SOFTC *)SC_GET_SOFTC(ctlr_num);
   AHA_SOFTC *aha_softc;
   struct controller *ctlr;		/* pointer to the controller structure */


   SIM_MODULE(ahaintr);

   SIM_PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	       ("\nEntering %s", module) );


   /*
    * If SIM_SOFTC pointer is NULL, then this is a spurious interrupt at boot time.
    */
   if ( sim_softc == (SIM_SOFTC *)NULL )
   {
	printf( "sim_spurious_interrupt -- Unexpected interrupt on controller %d\n",
		ctlr_num );

	return;
   }
    
   else
   {              
	/* Get our softc structure and call our interrupt routine */

	aha_softc = (AHA_SOFTC *)sim_softc->hba_sc;

	aha_process_intr( aha_softc );
   }



}	/* End of ahaintr() */


/* ---------------------------------------------------------------------- *
 *                                                                        *
 * ROUTINE NAME:  aha_process_intr()					  *
 *                                                                        *
 * FUNCTIONAL DESCRIPTION:                                                *
 *	This is the main interrupt processing routine.  The routine will  *
 *	check the interrupt status and act accordingly.                   *
 *                                                                        *
 *	There are basically 4 types of interrupts.  The most common one   *
 *	is for the I/O completion interrupt which has the corresponding   *
 *	I/O control block to work with.  Another type of interrupt is the *
 *	response interrupt for the 'immediate' command such as bus device *
 *	reset and adapter/bus reset.  There is also an Asynchronous Event *
 *	Notification interrupt if the adapter detected a bus reset or if  *
 *	the adapter ws selected by another initiator.  The last type of   *
 *      interrupt is the adapter hardware failure interrupt.              *
 *                                 					  *
 * CALLED BY:  ahaintr()                                                  *
 *                                                                        *
 * FORMAL PARAMETERS:                                                     *
 *		aha_softc -- Pointer to the controller's softc structure  *
 *                                                                        *
 * IMPLICIT INPUTS:                                                       *
 *		At the entry to this routine, the CPU processor level is  *
 *		already at device interrupt ipl.  We don't need to raise  *
 *		the IPL again in this routine.				  *
 *                                                                        *
 * IMPLICIT OUTPUTS:                                                      *
 *                                                                        *
 * RETURN VALUE:                                                          *
 *		none							  *
 *                                                                        *
 * SIDE EFFECTS:                                                          *
 *                                                                        *
 * ---------------------------------------------------------------------- */

void
aha_process_intr( aha_softc )
register AHA_SOFTC *aha_softc;
{
   register struct controller *ctlr = aha_softc->aha_ctlr;
   register io_handle_t iobaseaddr;     /* base register address */
   U32 g2intst;		                /* G2INTST register content */
   register U32 istat;                  /* Interrupt status bits 7 - 4 */
   register U32 targetid;               /* Target ID returned in G2INTST */
	    U32 scsiid;			/* host SCSI ID       */
   register U32	lun;			/* LUN number found in control block */
   register U32 cb_pa;                  /* MBOXIN register content, in this
					 * case, it is control block address */
   U32 mboxin;				/* MBOXIN register content */
   U32 g2stat;    	                /* G2STAT register content */
   register AHA_CB *cbptr;		/* control block of returned I/O */
   register AHA_SB *sbptr;		/* status block of returned I/O */
   register AHA_JOB *aha_job = NULL;    /* job control block of returned I/O */
   vm_offset_t kseg_addr;		/* KSEG virtual address	*/
   register U32 status;                 /* return status from routine calls */
   register U32 hard_reset_needed = FALSE;
					/* flag indicating if adapter reset *
					 * is needed due to error interrupt */
   register U32 sb_status;		/* Status word in status block */
   register U32	sb_tstatus;		/* Target status byte */
   register SIM_WS *sws;		/* SIM working set */


   SIM_MODULE(aha_process_intr);

   SIM_PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	       ("\nEntering %s", module) );

   /*
    * Set up base I/O address for register access
    */
   iobaseaddr = (io_handle_t)ctlr->physaddr;

   /* 
    * Lock the aha_softc.  We don't need to raise IPL since it is already
    * raised by the interrupt service.
    */
   AHA_LOCK (aha_softc->aha_lock);

   /* 
    * The following code is probably not necessary since the interrupt
    * pending bit should be set if we entered this routine.  However
    * for completeness' sake, we will check and if not set, we will print
    * error message and dismiss the interrupt for now.
    */

   g2stat = (U32)READ_BUS_D8( (AHA_G2STAT | iobaseaddr));

   if ( !(g2stat & G2STAT_IP) )         /* If interrupt is not pending */
   {
	printf("aha_process_intr: spurious interrupt on ctlr %x\n",
	       ctlr->ctlr_num);
	AHA_UNLOCK (aha_softc->aha_lock);


   /*
    * Acknowledge the interrupt by clearing the EISA interrupt bit
    */
        WRITE_BUS_D8( (AHA_G2CNTRL | iobaseaddr),G2CNTRL_CLRINT );

        WBFLUSH();                /* Do memory barrier */

	return;
   }

#ifdef AHADEBUG                         /* Debug code */

   if ((I32)aha_softc->aha_total_acb < 0)
    
	printf("aha_process_intr: total outstanding job count is = %d\n",
		aha_softc->aha_total_acb);

#endif /* AHADEBUG */                         /* Debug code */

   /*
    * Read interrupt status register 
    */
   g2intst = (U32)READ_BUS_D8( (AHA_G2INTST | iobaseaddr));

   istat = (g2intst & G2INTST_IMSK);       /* interrupt status     */
   targetid = (g2intst & G2INTST_TMSK);    /* targid id returned */

   scsiid = aha_softc->aha_sim_softc->scsiid;     /* get host SCSI ID */

   /*
    * Get physical address of the control block if any from MBOXIN registers. 
    * It could also contain error information for Immediate command interrupt
    * and Asynchronous Event Notification interrupt.
    */
   mboxin = (U32)READ_BUS_D32( (AHA_MBOXIN | iobaseaddr));

   cb_pa = mboxin;		/* copy to another variable for PA */
   
   /*
    * Acknowledge the interrupt by clearing the EISA interrupt bit
    */
   WRITE_BUS_D8( (AHA_G2CNTRL | iobaseaddr),G2CNTRL_CLRINT );

   WBFLUSH();                /* Do memory barrier */

   
   switch ( istat )
   {
   /*
    * Command Control Block (CCB) completed with or without error
    */
   case G2INTST_CMP:
   case G2INTST_CAR:			/* completed after retry */
   case G2INTST_CWE:			/* completed with error  */

	/* 
	 * If this PA matches the latest one sent to adapter, we
	 * don't need to calculate VA since we already have it.
	 */
	if (cb_pa == aha_softc->aha_cb_pa)
	{
	   cbptr = aha_softc->aha_cb_va;
	   aha_job = (AHA_JOB *)AHA_JOB_PTR(cbptr);   /* get corresponding job block */
	}

	else
        {
	   /* Translate control block PA to virtual address */
	   kseg_addr = (vm_offset_t)PHYS_TO_KSEG( cb_pa );

	   /* Calculate KSEG job block address from control block address */
	   kseg_addr = AHA_JOB_PTR(kseg_addr);

	   /* Now get the System VA of the job block */
	   aha_job = (AHA_JOB *)((AHA_JOB *)kseg_addr)->aha_job_va;

	   cbptr = &(aha_job->aha_cb);

	}

	if (aha_job == (AHA_JOB *)NULL)
	{
	   CAM_ERROR(module,
		     "bad address returned by adapter\n",
		     SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE,
		     aha_softc->aha_sim_softc, NULL,NULL);
	}
	else     
	{
	   sbptr = &(aha_job->aha_sb);   /* get status block pointer */
	   lun = (cbptr->flag2) & AHA_F2_LUNMSK;   /* get LUN number */

	   /* 
	    * Check to make sure that we were waiting for this I/O response,
	    * and if so, take it off the active queue 
	    */
	   status = aha_qsearch( &(aha_softc->aha_activeq[targetid][lun]),
				 aha_job );

	   if (status != CAM_TRUE)	/* no corresponding request found */
	   {
		CAM_ERROR(module,
			  "no corresponding request found\n",
			  SIM_LOG_HBA_SOFTC|SIM_LOG_PRILOW,
			  aha_softc->aha_sim_softc, NULL,NULL);

#ifdef AHADEBUG                         /* Debug code */
		printf("aha_process_intr: no corresponding request found\n");

		printf("aha_process_intr: aha_job address is: 0x%x\n", aha_job);

#endif /* AHADEBUG */                         /* Debug code */

		aha_job = (AHA_JOB *)NULL;

           }

	   else			/* corresponding request found */
	   {
		/* decrement # of outstanding control block */
		(aha_softc->aha_total_acb--);

		sb_status = sbptr->stat & 0xffff;     /* get status word */
		sb_tstatus = sbptr->tstat & 0xff;     /* get target status */

		sws = aha_job->aha_sws;		/* get SIM working set pointer */

		/* Save target SCSI status returned in SIM working set */
		
		sws->scsi_status = (U8)sb_tstatus;


		/*
		 * Command Done - No Error bit set  
		 */
		if (sb_status & AHA_SW_DON)
		{
		   sws->cam_status = CAM_REQ_CMP;

		   /*
		    * If data underrun bit is set, calculate actual number
		    * of bytes transferred and save in SIM working set 
		    */
		   if (sb_status & AHA_SW_DU)		
			sws->data_xfer.xfer_count =
				aha_job->aha_ccb->cam_dxfer_len - (sbptr->rescnt);
	        }	
		/*
		 * Command completed with error
		 */
		else
		{
		   if (sb_tstatus == SCSI_STAT_BUSY)  
		        sws->cam_status = CAM_REQ_CMP_ERR;
		   
		   else
		   {   
		        sws->cam_status = CAM_PROVIDE_FAIL;

		        status = aha_process_error(aha_job, sws, sbptr);

		        if (status == ADAPTER_RESET_NEEDED)
			   hard_reset_needed = TRUE;
		   }
		   
		}
	   } 		/* corresponding request found */
	}
   
	break;

   /*
    * Immediate command completed with or without error
    */
   case G2INTST_ICMP:
   case G2INTST_ICE:

#ifdef AHADEBUG                         /* Debug code */
	printf("aha_process_intr: immediate command interrupt, status = 0x%x, tid = 0x%x\n",
		istat, targetid);
#endif /* AHADEBUG */                         /* Debug code */

	mboxin = mboxin & 0xff;		/* One byte of error code */

	if (istat == G2INTST_ICE)		/* error return */
	{

           /* 
	    * If 'firmware not downloaded'  or 'host adapter hardware failure'
	    * error byte is set, we need to reset the adapter.
	    */
	   if ( (mboxin == HS_FW_NDL) || (mboxin == HS_HA_HE) ) 
	   {
		hard_reset_needed = TRUE;

		CAM_ERROR(module,"hardware failure occurred\n",
			  SIM_LOG_HBA_SOFTC|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE,
			  aha_softc->aha_sim_softc, NULL,g2intst);
	   }

	}

	/* 
	 * If there was an outstanding bus device reset job block in
	 * the activeq, this immediate command return is for the
	 * bus device reset.  We need to take that special job block off
	 * the activeq and flush all outstanding jobs for this target.
	 * 
	 * Note that according to the adapter spec, the target ID is
	 * returned in the interrupt status register to distinguish if
	 * this immediate command was a adapter/bus reset or a device
	 * reset request.  
	 */


	if (targetid != scsiid)     			/* take care of BDR */
	{
	   aha_job = aha_chk_bdr(aha_softc, targetid);

	   CAM_ERROR(module,"bus device reset attempted\n",
		     SIM_LOG_HBA_SOFTC|SIM_LOG_HBA_CSR,
		     aha_softc->aha_sim_softc, NULL,g2intst);

	   aha_flush(aha_softc, targetid);
	}

	/*
	 * If not bus device reset, this must be bus reset immediate
	 * command response.  
	 */
	else
	{
	   CAM_ERROR(module,"SCSI bus reset occurred\n",
		     SIM_LOG_HBA_SOFTC|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE,
		     aha_softc->aha_sim_softc, NULL,g2intst);

	   /* Flush all outstanding job blocks for the entire bus */
	   aha_flush(aha_softc, FLUSH_ALL);

	   /* Clear flag for bus reset */
	   aha_softc->aha_state &= ~AHA_BUS_RESET_IN_PROGRESS;

	   /* 
	    * Notify upper layer of CAM to clean up queued I/Os for this bus.
            */

	   ss_reset_detected(aha_softc->aha_sim_softc);

	}

	break;

   /*
    * Asynchronous event notification interrupt
    */
   case G2INTST_AEN:

	/*
	 * This interrupt can mean either 
	 *	1) A SCSI bus reset has occurred
	 *	2) The host adapter was selected by another initiator
	 */

	mboxin = mboxin & 0xff;         /* One byte of error code */

	if (targetid == scsiid)		/* SCSI bus reset has occurred */
	{
	   if (mboxin == HS_SB_RSTH)	/* bus reset by host adapter */
	   {
	   	CAM_ERROR(module,"SCSI bus reset by host adapter\n",
		          SIM_LOG_HBA_SOFTC|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE,
		          aha_softc->aha_sim_softc, NULL,g2intst);
	   }
	   else
	   {
		CAM_ERROR(module,"SCSI bus reset by other device\n",
			  SIM_LOG_HBA_SOFTC|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE,
			  aha_softc->aha_sim_softc, NULL,g2intst);
	   }

	   /* Flush all outstanding job blocks for the entire bus */

	   aha_flush(aha_softc, FLUSH_ALL);

	   /* Notify upper layer of CAM to clean up queued I/Os for this bus */

	   ss_reset_detected(aha_softc->aha_sim_softc);
	
	}

	else
	   CAM_ERROR(module,"Host selected by another initiator!\n",
		     SIM_LOG_HBA_SOFTC|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE,
		     aha_softc->aha_sim_softc, NULL,g2intst);

	break;

   /*
    * Adapter hardware failure interrupt
    */
   case G2INTST_AHF:
	
	hard_reset_needed = TRUE;

	CAM_ERROR(module,"hardware failure occurred\n",
		  SIM_LOG_HBA_SOFTC|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE,
		  aha_softc->aha_sim_softc, NULL,g2intst);

	break;

   default:

	CAM_ERROR(module,"unknown interrupt status\n",
		  SIM_LOG_HBA_SOFTC|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE,
		  aha_softc->aha_sim_softc, NULL,g2intst);
		  
	break;

   }		/* switch (istat) */

   /*
    * If there was a job block (including the bus device reset job block),
    * finish up processing the job and do callback to upper SIM layer.
    */
   if (aha_job != (AHA_JOB *)NULL)

	aha_iodone(aha_softc, aha_job);
   
   /*
    * If adapter reset is needed due to severe error, perform
    * the reset.
    */
   if (hard_reset_needed)
   {
#ifdef AHADEBUG                         /* Debug code */
	printf("aha_process_intr: adapter failure, reset needed\n");
#endif /* AHADEBUG */                         /* Debug code */

	aha_softc->aha_state &= ~AHA_ALIVE;
	
	/* Do the adapter reset with bus reset flag on */
	status = aha_init (aha_softc, TRUE);
   }

   /* 
    * See if we can start off more I/O request.  SC_SCHED_RUN_SM
    * macro will end up calling SIM_SCHED_ISR() if needed.
    */
   if (aha_softc->aha_state == AHA_ALIVE)
	SC_SCHED_RUN_SM (aha_softc->aha_sim_softc);


   AHA_UNLOCK (aha_softc->aha_lock);

  return;

}	/* End of aha_process_intr() */

/*
 * aha_qsearch()
 *
 * 	This is a subroutine to the aha_process_intr() to check the
 * 	specified queue (active queue for a particular target/LUN)
 *	to see if the passed job block is in that queue and if so,
 *	to take it off the queue and to return the CAM_TRUE status.  
 *	If not, CAM_FALSE is returned.
 *
 *	It is assumed that the proper IPL is already held upon
 *	entry to this routine.
 */

U32
aha_qsearch( queptr, aha_job )
register AHA_Q *queptr;
register AHA_JOB *aha_job;
{
   register AHA_JOB *curjobptr;
   register U32 found = FALSE;

   SIM_MODULE(aha_qsearch);

   SIM_PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	       ("\nEntering %s", module) );

   AHA_LOCK (queptr->ahaq_lock);  	/* lock the queue */

   curjobptr = queptr->ahaq_head;	/* get first element in queue */

   while ( (curjobptr != (AHA_JOB *)queptr) && (!found) )  
   {
	if (curjobptr == aha_job)		/* found the matching element */
        {
	    found = TRUE;
	    AHA_REMOVE (curjobptr);
        }
      	else 
	   curjobptr = curjobptr->aha_next;	/* get next element in queue */
   }

   AHA_UNLOCK (queptr->ahaq_lock);	/* unlock the queue */

   if (found)
   	return (CAM_TRUE);

   else
	return (CAM_FALSE);


}	/* End of aha_qsearch() */

/*
 * aha_process_error()
 *
 * 	This is a subroutine to the aha_process_intr().  This routine
 *	is called when the 'command completed with error' interrupt
 *	status was returned to find out what type of error occurred.
 *	If the error was severe and requires an adapter reset, the
 *	routine returns the status, ADAPTER_RESET_NEEDED.  If not,
 *	the return status is ADAPTER_RESET_NOT_NEEDED.
 *
 */

U32
aha_process_error( aha_job, sws, sbptr )
register AHA_JOB *aha_job;
register SIM_WS *sws;
register AHA_SB *sbptr;
{
   register U32 retval = ADAPTER_RESET_NOT_NEEDED;
   register U32 sb_status;		/* Status word in status block */
   register U32 sb_hstatus;             /* Host adapter status byte */


   SIM_MODULE(aha_process_error);

   SIM_PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	       ("\nEntering %s", module) );

   sb_status = sbptr->stat & 0xffff;     /* get status word */
   sb_hstatus = sbptr->hstat & 0xff;     /* get host adapter status */

#ifdef AHADEBUG                         /* Debug code */

/*
   printf("aha_process_error: status = 0x%x, host adapter status = 0x%x\n",
	  sb_status, sb_hstatus);

   printf("aha_process_error: SCSI status = 0x%x\n", (U8)sbptr->tstat);

   printf("aha_process_error: target id = 0x%x\n", sws->targid);

*/
#endif /* AHADEBUG */                         /* Debug code */


   /*
    * Major Error or Exception Occurred bit set in status word
    */
   if (sb_status & AHA_SW_ME)
   {

	switch ( sb_hstatus )		/* Check host adapter status byte */
	{
	case HS_CMD_AB_HT:		/* Command Aborted by Host */
	case HS_CMD_AB_HA:		/* Command Aborted by Host Adapter */
	   
	   sws->cam_status = CAM_REQ_ABORTED;

#ifdef AHADEBUG                 /* Debug code */
	   printf("aha_process_error: aborted job is 0x%x\n", aha_job);
#endif /* AHADEBUG */           /* Debug code */

	   break;

	case HS_SEL_TO:			/* Selection Timeout */

	   sws->cam_status = CAM_SEL_TIMEOUT;
	   break;

	case HS_DA_OR:			/* Data Overrun or Underrun */

	   /*
	    * If data underrun bit is set, calculate actual number
	    * of bytes transferred and save in SIM working set
	    */
	   if (sb_status & AHA_SW_DU)		/* Data underrun */
	   {
		sws->data_xfer.xfer_count =
				aha_job->aha_ccb->cam_dxfer_len - (sbptr->rescnt);

		sws->cam_status = CAM_REQ_CMP;
	   }
	   else					/* Data overrun */
	   {
		/* Number of bytes transferred is number of bytes requested */

		sws->data_xfer.xfer_count = aha_job->aha_ccb->cam_dxfer_len;

		sws->cam_status = CAM_DATA_RUN_ERR;

	   }
	   break;

	case HS_UE_BF:			/* Unexpected Bus Free */
	   
	   sws->cam_status = CAM_UNEXP_BUSFREE;
	   break;

	case HS_IV_BPD:			/* Invalid Bus Phase Detected */

	   sws->cam_status = CAM_SEQUENCE_FAIL;
	   break;

	case HS_IV_SLO:			/* Invalid SCSI Linking Operation */

	   sws->cam_status = CAM_REQ_INVALID;
	   break;

	case HS_RSC_F:			/* Request Sense Command Failed */

	   sws->cam_status = CAM_AUTOSENSE_FAIL;
	   break;

	case HS_TQ_MR:			/* Tag Queuing Message Rejected by Target */

	   sws->cam_status = CAM_MSG_REJECT_REC;
	   break;

	case HS_HA_HE:			/* Host Adapter Hardware Error */

	   retval = ADAPTER_RESET_NEEDED;

	   CAM_ERROR(module,"Host adapter hardware error, adapter reinit required\n",
		     SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE,
		     aha_job->aha_softc->aha_sim_softc, NULL, NULL);

	   break;

	case HS_TA_NRA:			/* Target did not Respond to ATTN (SCSI Reset) */
	case HS_SB_RSTH:		/* SCSI Bus Reset by Host Adapter */
	case HS_SB_RSTD:		/* SCSI Bus Reset by Other Device */

	   sws->cam_status = CAM_SCSI_BUS_RESET;
	   break;
	   
	case HS_PG_CF:			/* Program Checksum Failure */

	   retval = ADAPTER_RESET_NEEDED;

	   CAM_ERROR(module,"Checksum failure, adapter reinit required\n",
		     SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE,
		     aha_job->aha_softc->aha_sim_softc, NULL, NULL);
	   break;

	case HS_FW_NDL:			/* Firmware Not Downloaded */

	   retval = ADAPTER_RESET_NEEDED;
	
	   CAM_ERROR(module,"Firmware not downloaded, adapter reinit required\n",
		     SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE,
		     aha_job->aha_softc->aha_sim_softc, NULL, NULL);

	   break;

	default:

	   break;

	}

   }

   /*
    * Specification Check bit set in status word
    */
   else if (sb_status & AHA_SW_SC)
   {
	CAM_ERROR(module,"Invalid request from host\n",
		  SIM_LOG_HBA_SOFTC|SIM_LOG_HBA_DME|SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
		  aha_job->aha_softc->aha_sim_softc, aha_job, NULL);

	/* 
	 * If 'Target Not Assigned to SCSI Subsystem' error code set in 
	 * host adapter status byte, set appropriate error status
	 */
	if (sb_hstatus == HS_TA_NA)
	   sws->cam_status = CAM_TID_INVALID;

	/*
	 * Else panic since this means the driver has bug in
	 * setting up the I/O request.
	 */
	else 
	   panic("(aha_process_error) invalid I/O control block!");

   }

   /*
    * Host Adapter Queue Full bit set in status word 
    */
   else if (sb_status & AHA_SW_QF)
   {
	CAM_ERROR(module,"host adapter queue full\n",
		  SIM_LOG_HBA_SOFTC|SIM_LOG_HBA_DME|SIM_LOG_PRISEVERE,
		  aha_job->aha_softc->aha_sim_softc, aha_job, NULL);

	sws->cam_status = CAM_SCSI_BUSY;
   }

   /*
    * Chaining Halted bit set in status word
    */
   else if (sb_status & AHA_SW_CH) 
   {
	CAM_ERROR(module,"Chaining halted, illegal interrupt\n",
		  SIM_LOG_HBA_SOFTC|SIM_LOG_HBA_DME|SIM_LOG_PRISEVERE,
		  aha_job->aha_softc->aha_sim_softc, aha_job, NULL);
   }

   /*
    * Initialization Required bit set in status word
    */
   else if (sb_status & AHA_SW_INI)
   {
	retval = ADAPTER_RESET_NEEDED;

	CAM_ERROR(module,"Adapter initialization is required\n",
		  SIM_LOG_HBA_SOFTC|SIM_LOG_HBA_DME|SIM_LOG_PRISEVERE,
		  aha_job->aha_softc->aha_sim_softc, aha_job, NULL);
   }

   /*
    * Extended Contingent Allegiance bit set in status word
    */
   else if (sb_status & AHA_SW_ECA)
   {
	CAM_ERROR(module,"Extended Contingent Allegiance state detected\n",
		 SIM_LOG_HBA_SOFTC|SIM_LOG_HBA_DME|SIM_LOG_PRISEVERE,
		 aha_job->aha_softc->aha_sim_softc, aha_job, NULL);

	/* 
	 * If this happens, something is very wrong, we will ask
	 * for adapter reset to be done.  We might want to issue
	 * RESUME immediate command instead in the future if that
	 * is a better way to handle this case.
	 */
	retval = ADAPTER_RESET_NEEDED;
   }


   return (retval);


}	/* End of aha_process_error() */  

/*
 * aha_chk_bdr()
 *
 *	This subroutine is called by the aha_process_intr().  It checks
 *	the specified target's activeq[targetid][0] queue to see if
 *	the corresponding bus device reset job block is queued there.
 *	If that is the case, the routine dequeues it and returns the
 *	job block pointer.  If not, the NULL pointer is returned.
 *
 *	On entry to this routine, the AHA_SOFTC structure's lock and
 *	the bio IPL is already held by the caller.
 */

AHA_JOB *
aha_chk_bdr( aha_softc, targetid )
register AHA_SOFTC *aha_softc;
register U32 targetid;
{
   register AHA_JOB *jobptr = NULL;     /* ptr. to job block    */
   register AHA_Q *activeq;             /* ptr. to active job queue */
   register found = FALSE;		/* BDR job found flag */


   SIM_MODULE(aha_chk_bdr);

   SIM_PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	       ("\nEntering %s", module) );

   /* Get activeq for this target ID and LUN 0 */

   activeq = &(aha_softc->aha_activeq[targetid][0]); 

   AHA_LOCK (activeq->ahaq_lock);      /* Lock the queue */

   /* loop until we find the job block or end of the queue */

   jobptr = activeq->ahaq_head;         /* Look at first thing in the queue */

   while ( (!found) && (jobptr != (AHA_JOB *)activeq) )
   {
	if (jobptr->aha_jobtype == AHA_SPECIAL)
	{
	   found = TRUE;
	   AHA_REMOVE (jobptr);
	}
	else
	   jobptr = jobptr->aha_next;     /* get next element in queue */

   }

   AHA_UNLOCK (activeq->ahaq_lock);      /* Unlock the queue */

   if (found)
   	return (jobptr);

   else
	return ((AHA_JOB *)NULL);


}	/* End of aha_chk_bdr() */


/* ---------------------------------------------------------------------- *
 *                                                                        *
 * ROUTINE NAME:  aha_iodone()						  *
 *                                                                        *
 * FUNCTIONAL DESCRIPTION:                                                *
 *	This routine is called directly from the interrupt service        *
 *	routine.  The purpose of the routine is to do the callback to the *
 *	upper layer of CAM for post-I/O processing of the current I/O job.* 
 *	Since this adapter does not return the actual data transfer       *
 *      count, if the SCSI status returned is good, the original          *
 *	requested transfer count is copied to the actual data transfer    *
 *	count field in the SIM working set.  However if the command was a *
 *	REQUEST SENSE and we used the special I/O control block for this  *
 *	command, we do get the transfer count, so in this case, we return *
 *	the actual transfer size.  Also if the data underrun happened,    *
 *	the residual byte count is available from the adapter and this    *
 *	was used to calculate the actual data transfer size in the        *
 *	aha_process_intr() routine.  If there was any other error in this *
 *	I/O response, the actual data transfer count field is not         *
 *	calculated in this routine.					  *
 *                                                                        *
 *	An exception to the above is when the error status is that        *
 *	the device returned CHECK CONDITION status and we did an auto     *
 *	sense of the sense data.  In this case, since the adapter doesn't *
 *	tell us the actual transfer count, we also have to just copy the  *
 *	request transfer count and let the peripheral driver take care of *
 *	handling appropriately depending on the sense data returned.      *
 *                                                                        *
 * CALLED BY:  aha_process_intr()                                         *
 *                                                                        *
 * FORMAL PARAMETERS:                                                     *
 *		aha_softc -- Pointer to the controller's softc structure  *
 *		aha_job -- Pointer to the job block to be processed       *
 *                                                                        *
 * IMPLICIT INPUTS:                                                       *
 *		IPL is at device spl and the AHA_SOFTC lock is held.      *
 *                                                                        *
 * IMPLICIT OUTPUTS:                                                      *
 *                                                                        *
 * RETURN VALUE:                                                          *
 *                                                                        *
 * SIDE EFFECTS:                                                          *
 *	A callback is made to the upper CAM layer for the I/O.            *
 *                                                                        *
 * ---------------------------------------------------------------------- */

void
aha_iodone( aha_softc, aha_job )
register AHA_SOFTC *aha_softc;
register AHA_JOB *aha_job;
{
   register SIM_WS *sws = aha_job->aha_sws;	/* SIM working set */
   register U32 sb_status;              /* Status word in status block */
   register U32 sb_tstatus;             /* Target status byte */

   vm_offset_t bufaddr;			/* buffer address to copy data into */
   U64 physaddr;			/* physical address of the user buffer */
   struct buf *map_info_buf;	


   SIM_MODULE(aha_iodone);

   SIM_PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	       ("\nEntering %s", module) );

   /*
    * If current job block is a special job block used for bus device
    * reset, we need to return this job block to the AHA_SOFTC structure
    * so that other bus device reset requests can be satisfied and then
    * call the upper layer to notify the completion of the request.
    */
   if (aha_job->aha_jobtype == AHA_SPECIAL)
   {
 	aha_softc->aha_bdr_job = aha_job;

	sws->flags |= SZ_EXP_BUS_FREE;

	sws->cam_status = CAM_REQ_CMP;

	sm_bus_free(sws);

	return;

   }

   sws->flags |= SZ_CMD_CMPLT;


   sb_status = aha_job->aha_sb.stat & 0xffff;     /* get status word */
   sb_tstatus = aha_job->aha_sb.tstat & 0xff;     /* get target status */

   /* 
    * If command completed with no error bit is set and there was no
    * data underrun, we copy the requested transfer length to actual
    * transfer length field.
    */
   if ( (sb_status & AHA_SW_DON) && (!(sb_status & AHA_SW_DU)) )
   {
	sws->data_xfer.xfer_count = aha_job->aha_ccb->cam_dxfer_len;

	if (sb_tstatus != SCSI_STAT_GOOD)
	   sws->cam_status = CAM_REQ_CMP_ERR;
   }

   /*
    * If 'Sense Information Stored' bit is set, we have the sense data.
    * In this case, we need to copy the sense data from our buffer
    * to the user buffer if our buffer was used.  Also we need to
    * get the sense length.
    */
   if ( (aha_job->aha_senseflag) && (sb_status & AHA_SW_SNS) )
   {
	if (aha_job->aha_cbcmdtype == AHA_OP_RSI)
	{
	   /*
	    * This was a REQUEST SENSE command.  We copy the sense
	    * data to user data buffer.
	    *
	    * If a user space VA, we need to get PA, then translate it to KSEG address
	    * before calling the bcopy() routine. 
	    */

	   bufaddr = (vm_offset_t)(sws->ccb->cam_data_ptr);

	   if (CAM_IS_KUSEG(bufaddr))
	   {
	       map_info_buf = (struct buf *)(sws->ccb->cam_req_map);
	   
	       physaddr = pmap_extract(map_info_buf->b_proc->task->map->vm_pmap, bufaddr);
	       bufaddr = PHYS_TO_KSEG( physaddr );
	   }
	   
	   bcopy( &(aha_job->aha_sensebuf[0]), bufaddr,
		      aha_job->aha_sb.snslen );
		   
	   sws->data_xfer.xfer_count = aha_job->aha_sb.snslen;
	}
	else
	{
	   /* 
	    * We got an autosense data.  Copy data to user sense buffer.
	    * 
	    * If a user space VA, we need to get PA, then translate it to KSEG address
	    * before calling the bcopy() routine.
	    */

	   bufaddr = (vm_offset_t)(sws->ccb->cam_sense_ptr);
	
	   if (CAM_IS_KUSEG(bufaddr))
	   {
	       map_info_buf = (struct buf *)(sws->ccb->cam_req_map);
	   
	       physaddr = pmap_extract(map_info_buf->b_proc->task->map->vm_pmap, bufaddr);
	       bufaddr = PHYS_TO_KSEG( physaddr );
           }
	   
	   bcopy( &(aha_job->aha_sensebuf[0]), bufaddr,
	          aha_job->aha_sb.snslen );

	   /* Calculate residual sense count */

	   if (sws->ccb->cam_sense_len == aha_job->aha_sb.snslen)
		sws->ccb->cam_sense_resid = 0;

	   else
		sws->ccb->cam_sense_resid = sws->ccb->cam_sense_len - 
					    aha_job->aha_sb.snslen;

	}
   }

   if (sb_tstatus == SCSI_STAT_CHECK_CONDITION)
   {
        /*
         * As long as it wasn't data underrun, we return the same requested
	 * transfer length.  In the case of data underrun, we already
         * calculated the correct transfer count.  
         */
	if (!(sb_status & AHA_SW_DU))
	   sws->data_xfer.xfer_count = aha_job->aha_ccb->cam_dxfer_len;

	sws->cam_status = CAM_REQ_CMP_ERR;

	if (sb_status & AHA_SW_SNS)
	   sws->cam_status |= CAM_AUTOSNS_VALID;
   }

   sws->flags |= SZ_EXP_BUS_FREE;

   /* return the job block to the free queue */

   aha_dealloc_job(aha_softc, aha_job);

   sm_bus_free(sws);


}	/* End of aha_iodone() */


/* ---------------------------------------------------------------------- */
/*		ERROR HANDLING ROUTINES					  */
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- *
 *                                                                        *
 * ROUTINE NAME:  aha_termio_abort_bdr()				  *
 *                                                                        *
 * FUNCTIONAL DESCRIPTION:                                                *
 *	This routine is called by the ss_perform_termio() or the          *
 *	ss_perform_abort() or the ss_perform_device_reset() via a call    *
 *	to the SC_HBA_SEL_MSGOUT macro which is translated to call this   *
 *	routine.  This routine will check the 'flags' field in the SIM    *
 *	working set passed to determine if it is called to do a terminate *
 *	I/O or an abort operation or a bus device reset.  If a terminate  *
 *	I/O is being requested, the aha_ss_termio() is called.  If an     *
 *	abort is being requested, the aha_ss_abort() is called.           *
 *	Otherwise, the aha_ss_device_reset() is called to handle the      *
 *	device reset.                                                     *
 *									  *
 * CALLED BY:  ss_perform_termio(), ss_perform_abort(),                   *
 *	       ss_perform_device_reset()			          *
 *                                                                        *
 * FORMAL PARAMETERS:                                                     *
 *		sws -- Pointer to the SIM working set			  *
 *                                                                        *
 * IMPLICIT INPUTS:                                                       *
 *		Prior to entering this routine, the SIM scheduler already *
 *		set the IPL to splbio and locked the SIM_SOFTC lock.	  *
 *                                                                        *
 * IMPLICIT OUTPUTS:                                                      *
 *                                                                        *
 * RETURN VALUE:                                                          *
 *		CAM_REQ_CMP    						  *
 *		CAM_PROVIDE_FAIL -- If we could not send the device       *
 *				    reset request to the adapter          *
 *                                                                        *
 * SIDE EFFECTS:                                                          *
 *                                                                        *
 * ---------------------------------------------------------------------- */

U32
aha_termio_abort_bdr( sws )
register SIM_WS *sws;
{

   SIM_MODULE(aha_termio_abort_bdr);

   SIM_PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	       ("\nEntering %s", module) );


   if (sws->flags & (SZ_ABORT_TAG_INPROG|SZ_ABORT_INPROG|SZ_TERMIO_INPROG))
   {

#ifdef AHADEBUG                         /* Debug code */
   	if (sws->flags & SZ_TERMIO_INPROG)
        {
   	   printf("aha_termio_abort_bdr: the request is terminate I/O\n");
	   printf("            for target 0x%x\n", sws->targid);
	}

	else
        {
	   printf("aha_termio_abort_bdr: the request is abort I/O\n");
	   printf("            for target 0x%x\n", sws->targid);
        }

#endif /* AHADEBUG */                         /* Debug code */

   	if (sws->flags & SZ_TERMIO_INPROG)
	   return ( aha_ss_termio(sws) );

	else
	   return ( aha_ss_abort(sws) );

   }		/* if terminate I/O or abort I/O */

   /* 
    * If not terminate I/O or abort I/O request, this must be device reset
    * request.
    */
   else
   {
#ifdef AHADEBUG                         /* Debug code */

   	printf("aha_termio_abort_bdr: the request is device reset\n");
        printf("            for target 0x%x\n", sws->targid);

#endif /* AHADEBUG */                         /* Debug code */

	return ( aha_ss_device_reset(sws) );

   }

}	/* End of aha_termio_abort_bdr() */

/* ---------------------------------------------------------------------- *
 *                                                                        *
 * ROUTINE NAME:  aha_ss_termio()					  *
 *                                                                        *
 * FUNCTIONAL DESCRIPTION:                                                *
 *	This is the routine that gets called when a terminate I/O         *
 *	request is present.  Since this adapter does not give us means to *
 *	issue the terminate I/O message, this routine will upgrade the    *
 *	terminate I/O request to an abort I/O request.  The 'flags' field *
 *	in the SIM working set will be modified to upgrade to abort       *
 *	pending.  Then this routine calls the aha_ss_abort() to do the    *
 *	abort operation.					          *
 *									  *
 *	This routine will always return CAM_REQ_CMP since terminate I/O   *
 *	request is not guaranteed to succeed.                             *
 *                                                                        *
 * CALLED BY:  aha_termio_abort_bdr()                                     *
 *                                                                        *
 * FORMAL PARAMETERS:                                                     *
 *		sws -- Pointer to the SIM working set of the I/O to be    *
 *		       terminated          				  *
 *                                                                        *
 * IMPLICIT INPUTS:                                                       *
 *		Prior to entering this routine, the SIM scheduler already *
 *		set the IPL to splbio and locked the SIM_SOFTC lock.      *
 *                                                                        *
 * IMPLICIT OUTPUTS:                                                      *
 *                                                                        *
 * RETURN VALUE:                                                          *
 *		CAM_REQ_CMP						  * 
 *                                                                        *
 * SIDE EFFECTS:                                                          *
 *                                                                        *
 * ---------------------------------------------------------------------- */

U32
aha_ss_termio( sws )
register SIM_WS *sws;
{
   SIM_MODULE(aha_ss_termio);

   SIM_PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	       ("\nEntering %s", module) );

   sws->flags &= ~SZ_TERMIO_INPROG;	/* Clear terminate I/O flag */
   sws->flags |= SZ_ABORT_INPROG;	/* Set abort I/O flag */

   return ( aha_ss_abort(sws) );


}	/* End of aha_ss_termio() */


/* ---------------------------------------------------------------------- *
 *                                                                        *
 * ROUTINE NAME:  aha_ss_abort()					  *
 *                                                                        *
 * FUNCTIONAL DESCRIPTION:                                                *
 *	This routine will try to abort the requested I/O process.  If the *
 *	requested I/O process was not in the active (outstanding) I/O     *
 *	queue, that means the I/O is already performed.  In this case     *
 *	nothing further can be done, so CAM_REQ_CMP status is returned.   *
 *	If the original request is still in the active I/O queue, this    *
 *	routine will try several times to send the abort request to the   *
 *	adapter.  If the retries all fail, the abort attempt is           *
 *	terminated.  							  *
 *									  *
 *	There is no interrupt associated with the abort request command   *
 *	since if the requested I/O is really aborted due to this action,  *
 *	that I/O will cause the interrupt to the host with the 'command   *
 *      aborted' status.						  *
 *                                                                        *
 *      This routine will always return CAM_REQ_CMP since abort I/O       *
 *      request is not guaranteed to succeed.                             *
 *                                                                        *
 * CALLED BY:   aha_termio_abort_bdr(), aha_ss_termio()                   *
 *                                                                        *
 * FORMAL PARAMETERS:                                                     *
 *		sws -- Pointer to the SIM working set of the I/O to be    *
 *		       aborted                                            *
 *                                                                        *
 * IMPLICIT INPUTS:                                                       *
 *              Prior to entering this routine, the SIM scheduler already *
 *              set the IPL to splbio and locked the SIM_SOFTC lock.      *
 *                                                                        *
 * IMPLICIT OUTPUTS:                                                      *
 *                                                                        *
 * RETURN VALUE:                                                          *
 *		CAM_REQ_CMP						  *
 *                                                                        *
 * SIDE EFFECTS:                                                          *
 *	Bus reset will occur if the I/O to be aborted is currently on the *
 *      the bus.  This is due to the adapter firmware bug which resets    *
 *	the bus instead of issuing an abort message to the target.        *
 *                                                                        *
 * ---------------------------------------------------------------------- */

U32
aha_ss_abort( sws )
register SIM_WS *sws;
{
   register SIM_SOFTC *sim_softc = sws->sim_sc;
   register AHA_SOFTC *aha_softc = (AHA_SOFTC *)(sim_softc->hba_sc);
   register struct controller *ctlr = aha_softc->aha_ctlr;
   register io_handle_t iobaseaddr;     /* base reg address */
   U32 g2stat;  	                /* G2STAT register content */
   register U32 retry;                  /* retry count for sending command  */
   register AHA_JOB *aha_job;		/* Pointer to the job to be aborted */
   U32	abort_cmdsent = FALSE;		/* flag indicating command was sent */

   register AHA_Q *activeq;                     /* activeq pointer */
   U32 sws_found = FALSE;

   
   SIM_MODULE(aha_ss_abort);

   SIM_PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	       ("\nEntering %s", module) );
	       
#ifdef AHADEBUG                         /* Debug code */


   iobaseaddr = (io_handle_t)ctlr->physaddr;

   g2stat = (U32)READ_BUS_D8( (AHA_G2STAT | iobaseaddr));

   if (g2stat & G2STAT_IP)         
   {
	printf ("aha_ss_abort: interrupt is pending for ctlr %x\n",
		ctlr->ctlr_num); 

   }
   else
	printf ("aha_ss_abort: no interrupt is pending for ctlr %x\n",
		ctlr->ctlr_num); 

#endif /* AHADEBUG */                         /* Debug code */


   /*
    * We will first check the active queue to see if the I/O is
    * still outstanding to the adapter.  If the corresponding request is
    * not in the activeq[][], we don't need to abort since it is already
    * executed by the adapter.
    */
   activeq = &(aha_softc->aha_activeq[sws->targid][sws->lun]);

   AHA_LOCK (activeq->ahaq_lock);        /* Lock the queue */

   aha_job = activeq->ahaq_head;         /* Look at first thing in the queue */

   while ((aha_job != (AHA_JOB *)activeq) && (!sws_found))
   {
	if (aha_job->aha_sws == sws)         /* found matching SIM working set */
	   sws_found = TRUE;

	else
	   aha_job = aha_job->aha_next;    /* get next element in queue */
   }

   AHA_UNLOCK (activeq->ahaq_lock);

   if (!sws_found)         /* original request is already executed */
	return (CAM_REQ_CMP);

   /*
    * The I/O is still outstanding.  We will send abort command to the adapter.
    * Lock the driver's softc structure.  We don't need to raise IPL
    * since it is already done by the ss_sched().
    */
   AHA_LOCK (aha_softc->aha_lock);

   iobaseaddr = (io_handle_t)ctlr->physaddr;    /* get base register address */

   retry = SEND_RETRY;                  /* try up to 10 times */

   while (retry)
   {
	g2stat = (U32)READ_BUS_D8( (AHA_G2STAT | iobaseaddr));

	/* Check MBOXOUT empty bit and BUSY bit */

	if ( (g2stat & G2STAT_MOE) && (!(g2stat & G2STAT_BUSY)) )
	{
	   /* 
	    * Write the address of the control block to be aborted to 
	    * MBOXOUT registers
	    */
	   WRITE_BUS_D32( (AHA_MBOXOUT | iobaseaddr),aha_job->aha_cb_pa);

	   WBFLUSH();		/* do memory barrier */

	   /*
	    * Write op code for abort command and SCSI ID of the target
	    * to ATTN register
	    */
	    WRITE_BUS_D8( (AHA_ATTN | iobaseaddr),
			 (ATTN_OP_ABORT | (sws->targid)) );

	   WBFLUSH();

	   retry = 0;
	   abort_cmdsent = TRUE;

	}

	else
	{
	   retry--;
	   DELAY( WAIT_10USEC );           /* Wait 10 usec. before retry */
	}
   }

   AHA_UNLOCK (aha_softc->aha_lock);

#ifdef AHADEBUG                         /* Debug code */

   printf ("aha_ss_abort: abort command sent to adapter\n");

#endif /* AHADEBUG */                         /* Debug code */


   return (CAM_REQ_CMP);


}	/* End of aha_ss_abort() */


/* ---------------------------------------------------------------------- *
 *                                                                        *
 * ROUTINE NAME:  aha_ss_device_reset()					  *
 *                                                                        *
 * FUNCTIONAL DESCRIPTION:                                                *
 *	This routine tries to send down an immediate (non-control block)  *
 *	command to the adapter to issue the device reset message.  If     *
 *	the adapter's mailbox out register is empty and the adapter busy  *
 *	bit is clear, we send the immediate request to the adapter, and   *
 *	insert the special device reset job block to the outstanding job  *
 *	queue for that target so that we can inform the CAM later on      *
 *	when the adapter interrupts us with the immediate command         *
 *	completion.  If retries for sending the immediate command failed  *
 *	due to the mailbox out register being busy, the routine will      *
 *	return CAM_PROVIDE_FAIL status so that the device reset can be    *
 *	rescheduled.							  *
 *                                                                        *
 * CALLED BY:  aha_termio_abort_bdr(), aha_ss_abort()                     *
 *                                                                        *
 * FORMAL PARAMETERS:                                                     *
 *		sws -- Pointer to the device reset SIM working set        * 
 *                                                                        *
 * IMPLICIT INPUTS:                                                       *
 *              Prior to entering this routine, the SIM scheduler already *
 *              set the IPL to splbio and locked the SIM_SOFTC lock.      *
 *                                                                        *
 * IMPLICIT OUTPUTS:                                                      *
 *                                                                        *
 * RETURN VALUE:                                                          *
 *		CAM_REQ_CMP if we sent device reset immediate command to  *
 *			    the adapter					  *
 *		CAM_PROVIDE_FAIL if we could not send the device reset    *
 *			    request to the adapter                        *
 *                                                                        *
 * SIDE EFFECTS:                                                          *
 *                                                                        *
 * ---------------------------------------------------------------------- */

U32
aha_ss_device_reset( sws )
register SIM_WS *sws;
{
   register SIM_SOFTC *sim_softc = sws->sim_sc;
   register AHA_SOFTC *aha_softc = (AHA_SOFTC *)(sim_softc->hba_sc);
   register AHA_JOB *bdr_job;                   /* pointer to job block */

   register AHA_Q *activeq;		/* Pointer to the active queue */
   register struct controller *ctlr = aha_softc->aha_ctlr;
   register io_handle_t iobaseaddr;     /* base reg address */
   U32 g2stat;                          /* G2STAT register content */
   register U32 retry;                  /* retry count for sending command  */
   U32  bdr_sent = FALSE;                /* flag indicating command was sent */


   SIM_MODULE(aha_ss_device_reset);

   SIM_PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	       ("\nEntering %s", module) );

   /* 
    * Lock the driver's softc structure.  We don't need to raise IPL
    * since it is already done by the ss_sched().
    */
   AHA_LOCK (aha_softc->aha_lock);                  

   /* If someone else is using device reset job block, return */

   if (aha_softc->aha_bdr_job == (AHA_JOB *)NULL)
   {
	AHA_UNLOCK (aha_softc->aha_lock);

   	return (CAM_PROVIDE_FAIL);
   }

   iobaseaddr = (io_handle_t)ctlr->physaddr;    /* get base register address */

   retry = SEND_RETRY;                  /* try up to 10 times */

   while (retry)
   {
	g2stat = (U32)READ_BUS_D8( (AHA_G2STAT | iobaseaddr));

	/* Check MBOXOUT empty bit and BUSY bit */

	if ( (g2stat & G2STAT_MOE) && (!(g2stat & G2STAT_BUSY)) )
	{
	   /* Write device reset immediate command to MBOXOUT registers */

	   WRITE_BUS_D32( (AHA_MBOXOUT | iobaseaddr),ICMD_DEV_RESET);
	
   	   WBFLUSH();                /* do memory barrier */

	   /* 
	    * Write op code for immediate command and SCSI ID of the target 
	    * to ATTN register 
	    */
	   WRITE_BUS_D8( (AHA_ATTN | iobaseaddr),(ATTN_OP_ICMD | (sws->targid)) );

	   WBFLUSH();

	   retry = 0;
	   bdr_sent = TRUE;

	   /*
	    * Get the special BDR job block from the driver's softc
	    * structure and clear that field so that others will
	    * know there is a BDR outstanding.
	    */
	   bdr_job = aha_softc->aha_bdr_job;

	   aha_softc->aha_bdr_job = (AHA_JOB *)NULL;

	   /* Save the BDR SIM working set pointer in this job block */

	   bdr_job->aha_sws = sws;		

	   /* Insert the BDR special job block to active queue */
 
           activeq = &aha_softc->aha_activeq[sws->targid][0];

	   AHA_LOCK(activeq->ahaq_lock);
	    
	   AHA_INSERT_TAIL(activeq, bdr_job);

	   AHA_UNLOCK(activeq->ahaq_lock);

	}

        else
	{
	   retry--;
	   DELAY( WAIT_10USEC );           /* Wait 10 usec. before retry */
	}
   }

   AHA_UNLOCK (aha_softc->aha_lock);

   if (bdr_sent)
	return (CAM_REQ_CMP);

   else
	return (CAM_PROVIDE_FAIL);


}	/* End of aha_ss_device_reset() */


/* ---------------------------------------------------------------------- *
 *                                                                        *
 * ROUTINE NAME:  aha_bus_reset()					  *
 *                                                                        *
 * FUNCTIONAL DESCRIPTION:                                                *
 *	This routine is called from CAM when the peripheral driver issues *
 *	a SCSI bus reset request.  This routine tries to use the adapter  *
 *	reset immediate command to do the bus reset.   If we couldn't     *
 *	send the command to the adapter, we will do the adapter hard      *
 *	reset to force the bus reset as part of the adapter reset.        *
 *                                                                        *
 * CALLED BY:  handle_immediate() in sim_xpt.c                            *
 *                                                                        *
 * FORMAL PARAMETERS:                                                     *
 *		sim_softc -- Pointer to the SIM_SOFTC structure for the   *
 *			     bus to be reset                              *
 *                                                                        *
 * IMPLICIT INPUTS:                                                       *
 *                                                                        *
 * IMPLICIT OUTPUTS:                                                      *
 *                                                                        *
 * RETURN VALUE:                                                          *
 *		CAM_REQ_CMP						  *
 *                                                                        *
 * SIDE EFFECTS:                                                          *
 *	Bus reset can occur and adapter hard reset can occur too if we    *
 *	couldn't send the immediate command.                              *
 *                                                                        *
 * ---------------------------------------------------------------------- */

U32
aha_bus_reset( sim_softc )
register SIM_SOFTC *sim_softc;
{
   register AHA_SOFTC *aha_softc = (AHA_SOFTC *)(sim_softc->hba_sc);
   register struct controller *ctlr = aha_softc->aha_ctlr;
   register io_handle_t iobaseaddr;     /* base reg address */
   register I32 status;                 /* return status from routine calls */
   U32 g2stat;  	                /* G2STAT register content */
   register U32 retry;                  /* retry count for sending command  */
   U32 bus_reset_sent = FALSE;		/* flag indicating command was sent */
   register int oldspl;     		/* IPL to be saved */


   SIM_MODULE(aha_bus_reset);

   SIM_PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	       ("\nEntering %s", module) );

#ifdef AHADEBUG                         /* Debug code */

   printf("aha_bus_reset: bus reset is requested\n");

#endif /* AHADEBUG */                         /* Debug code */

   SIM_SOFTC_LOCK(oldspl, sim_softc);

   if (!(sim_softc->error_recovery & ERR_BUS_RESET))
	sim_softc->error_recovery |= ERR_BUS_RESET;

   SIM_SOFTC_UNLOCK(oldspl, sim_softc);

   AHA_IPL_LOCK(aha_softc->aha_lock, oldspl);	/* Lock softc structure */

   /* Set state flag that adapter is to do bus reset */

   aha_softc->aha_state |= AHA_BUS_RESET_IN_PROGRESS;


   CAM_ERROR(module, "Resetting the SCSI bus at request", 
	     SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE,
	     sim_softc, NULL, NULL);

   /*
    * Try issuing an adapter reset immediate command to do the bus reset
    */
   iobaseaddr = (io_handle_t)ctlr->physaddr;    /* get base register address */

   retry = SEND_RETRY;			/* try up to 10 times */

   while (retry)
   {
	g2stat = (U32)READ_BUS_D8( (AHA_G2STAT | iobaseaddr));

	/* Check MBOXOUT empty bit and BUSY bit */

	if ( (g2stat & G2STAT_MOE) && (!(g2stat & G2STAT_BUSY)) )
	{
	   /* Write adapter reset immediate command to MBOXOUT registers */

	   WRITE_BUS_D32( (AHA_MBOXOUT | iobaseaddr),ICMD_ADP_RESET);

	   WBFLUSH();                /* do memory barrier */

	   /*
	    * Write op code for immediate command and SCSI ID of the host   
	    * adapter to ATTN register
	    */
	   WRITE_BUS_D8( (AHA_ATTN | iobaseaddr),(ATTN_OP_ICMD | sim_softc->scsiid) );

	   WBFLUSH();

	   retry = 0;
	   bus_reset_sent = TRUE;

	}

	else
	{
	   retry--;
	   DELAY( WAIT_10USEC );           /* Wait 10 usec. before retry */
	}
   }

   /* 
    * If we couldn't send the bus reset immediate command, we will
    * try hard reset of the adapter with bus reset flag on.
    */
   if (!bus_reset_sent)
	status = aha_init(aha_softc, TRUE);

   AHA_IPL_UNLOCK(aha_softc->aha_lock, oldspl);

   return (CAM_REQ_CMP);

}	/* End of aha_bus_reset() */


/* ---------------------------------------------------------------------- *
 *                                                                        *
 * ROUTINE NAME:  aha_flush()						  *
 *                                                                        *
 * FUNCTIONAL DESCRIPTION:                                                *
 *	This subroutine checks the outstanding job queues in the          *
 *	AHA_SOFTC structure and if there are any (which means the command *
 *	was outstanding to the adapter but got flushed/aborted due to a   *
 *	device reset or a bus reset), it takes off each of these control  *
 *	blocks and returns the block to the job free queue.  If the SCSI  *
 *	ID passed is a valid target ID, then only that target's active    *
 *	job queues are flushed.  If the value passed is FLUSH_ALL,	  *
 *	the routine will go through all targets' active job queues to     *
 *	flush jobs.  The reason behind just returning the job blocks to   *
 *	the free queue is that when a device reset or a bus reset is      *
 *	issued by the host, the SIM scheduler and the SIM_XPT modules     *
 *	take care of cleaning up the relevant nexus queues with a proper  *
 *	cam_status (CAM_BDR_SENT or CAM_SCSI_BUS_RESET) to be returned to *
 *	the peripheral drivers once this HBA driver lets the CAM SIM know *
 *	that the reset is completed.  This routine also decrements the    *
 *	'total_acb' count in the AHA_SOFTC structure for each job block   *
 *	flushed from the queues.                                          *
 *                                                                        *
 *	One exception to the above process is the special job block for   *
 *	device reset.  If the BDR job block is found from an active queue,*
 *	it is returned to the AHA_SOFTC structure instead of to the free  *
 *	queue.  It also returns the associated SIM working set to CAM.    *
 *	Since it is not a normal I/O job block, the 'total_acb' count is  *
 *	also not decremented in this case.                                *
 *                                                                        *
 * CALLED BY:   aha_init(), aha_process_intr()				  *
 *                                                                        *
 * FORMAL PARAMETERS:                                                     *
 *		aha_softc -- Pointer to the controller's softc structure  *
 *		scsiid -- Integer value indicating which target queues    *
 *			  should be cleaned up, 0xffffffff will indicate  *
 *			  to flush all queues.                            *
 *                                                                        *
 * IMPLICIT INPUTS:                                                       *
 *              At the entry to this routine, the CPU processor level is  *
 *              already at device interrupt ipl.  We don't need to raise  *
 *              the IPL again in this routine.                            *
 *                                                                        *
 * IMPLICIT OUTPUTS:                                                      *
 *                                                                        *
 * RETURN VALUE:                                                          *
 *		none							  *
 *                                                                        *
 * SIDE EFFECTS:                                                          *
 *                                                                        *
 * ---------------------------------------------------------------------- */

void
aha_flush( aha_softc, scsiid )
register AHA_SOFTC *aha_softc;
U32 scsiid;
{
   register AHA_Q *activeq;
   register U32 target, lun;

   
   SIM_MODULE(aha_flush);

   SIM_PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	       ("\nEntering %s", module) );

#ifdef AHADEBUG                         /* Debug code */

   printf("aha_flush: flushing queues for target = 0x%x\n", scsiid);

#endif /* AHADEBUG */                         /* Debug code */

   if (scsiid == FLUSH_ALL)
   {
	for (target = 0; target < AHA_NDPS; target++)
	{
	   for (lun = 0; lun < AHA_NLPT; lun++)
	   {
		activeq = &aha_softc->aha_activeq[target][lun];

		aha_flush_lun_queue(aha_softc, activeq);
	   }
	}
   }
   /*
    * Flush only the specified target's active queues.
    */
   else
   {
	target = scsiid;

	for (lun = 0; lun < AHA_NLPT; lun++)
	{
	   activeq = &aha_softc->aha_activeq[target][lun];

	   aha_flush_lun_queue(aha_softc, activeq);
	}
   }


}	/* End of aha_flush() */


/*
 * aha_flush_lun_queue()
 *
 * 	This is a subroutine of the aha_flush() to flush one
 *	active queue (activeq[target][lun]).  
 *
 */
void
aha_flush_lun_queue( aha_softc, activeq )
register AHA_SOFTC *aha_softc;
register AHA_Q *activeq;
{
   register AHA_JOB *aha_job;           /* Pointer to a job block */
   register SIM_WS *sws;		/* SIM working set pointer */


   SIM_MODULE(aha_flush_lun_queue);

   SIM_PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	       ("\nEntering %s", module) );

   /* Lock the queue lock */
    
   AHA_LOCK (activeq->ahaq_lock);

   /* Remove a job from the head of the queue */

   aha_job = (AHA_JOB *) AHA_REMOVE_HEAD(activeq);

   while (aha_job != NULL)
   {
	/*
	 * If this job block is the BDR job block, we need
	 * to return the pointer to the AHA_SOFTC and callback
	 * to CAM with SIM working set.
	 */
	if (aha_job->aha_jobtype == AHA_SPECIAL)
	{
#ifdef AHADEBUG                         /* Debug code */

	   printf("aha_flush_lun_queue: BDR job block found\n");

#endif /* AHADEBUG */                         /* Debug code */
	  
	   aha_softc->aha_bdr_job = aha_job;

	   sws = aha_job->aha_sws;

	   sws->flags |= SZ_EXP_BUS_FREE;

	   sws->cam_status = CAM_REQ_CMP;

	   sm_bus_free(sws);

	}

	else                    /* This is a normal job block */
	{
	   AHA_LOCK (aha_softc->aha_lock);

	   (aha_softc->aha_total_acb)--;   /* decrement # of outstanding job */

	   AHA_UNLOCK (aha_softc->aha_lock);

#ifdef AHADEBUG                         /* Debug code */

	   if ((I32)aha_softc->aha_total_acb < 0)
		printf("aha_flush_lun_queue: total active jobs less than 0!\n");

	   printf("aha_flush: addr of aha_job is: 0x%x\n", aha_job);

#endif /* AHADEBUG */                         /* Debug code */

	   aha_dealloc_job( aha_softc, aha_job );

	}

	/* Get next job from the queue */

   	aha_job = (AHA_JOB *) AHA_REMOVE_HEAD(activeq);
   }

   AHA_UNLOCK (activeq->ahaq_lock);


}	/* End of aha_flush_lun_queue() */


/* ---------------------------------------------------------------------- *
 *                                                                        *
 * ROUTINE NAME:  aha_logger()						  *
 *                                                                        *
 * FUNCTIONAL DESCRIPTION:                                                *
 *	This is the local error logging routine for this driver.  This    *
 *	routine is called through the CAM_ERROR macro within this driver. *
 *	In order to support error logging for this driver, cam_logger.h   *
 *	is also modified to include the definitions for this driver's     *
 *	log constants.                                                    *
 *                                                                        *
 * CALLED BY: CAM_ERROR macro						  *
 *                                                                        *
 * FORMAL PARAMETERS:                                                     *
 *		func -- Calling module name, may be NULL		  *
 *		msg -- Error description, may be NULL			  *
 *		flags -- SIM log flags described in sim.h		  *
 *		sim_softc -- SIM_SOFTC pointer, may be NULL		  *
 *		aha_job -- AHA_JOB pointer, may be NULL			  *
 *		intreg -- Interrupt status register content, may be NULL  *
 *                                                                        *
 * IMPLICIT INPUTS:                                                       *
 *                                                                        *
 * IMPLICIT OUTPUTS:                                                      *
 *                                                                        *
 * RETURN VALUE:                                                          *
 *		none							  *
 *                                                                        *
 * SIDE EFFECTS:                                                          *
 *                                                                        *
 * ---------------------------------------------------------------------- */

void
aha_logger( func, msg, flags, sim_softc, aha_job, intreg )
u_char *func;
u_char *msg;
U32 flags;
register SIM_SOFTC *sim_softc;
register AHA_JOB *aha_job;
U32 intreg;
{
   register CAM_ERR_HDR hdr;
   static CAM_ERR_ENTRY entrys[SIM_LOG_SIZE];
   register CAM_ERR_ENTRY *entry;
   register AHA_SOFTC *aha_softc;

   SIM_MODULE(aha_logger);

   SIM_PRINTD(0,0,0,CAMD_ERRORS,("aha_logger: '%s' '%s'\n",func,msg));

   hdr.hdr_type = CAM_ERR_PKT;
   hdr.hdr_class = CLASS_AHA;
   hdr.hdr_subsystem = SUBSYS_AHA;

   hdr.hdr_size = 0;
   hdr.hdr_entries = 0;
   hdr.hdr_list = entrys;
   if (flags & SIM_LOG_PRISEVERE)
	hdr.hdr_pri = EL_PRISEVERE;

   else if (flags & SIM_LOG_PRIHIGH)
	hdr.hdr_pri = EL_PRIHIGH;

   else
	hdr.hdr_pri = EL_PRILOW;

   /*
    * Log the module name
    */
   if (func != (U8 *)NULL)
   {
	entry = &hdr.hdr_list[hdr.hdr_entries++];
        entry->ent_type = ENT_STR_MODULE;
      	entry->ent_size = strlen(func) + 1;
      	entry->ent_vers = 0;
      	entry->ent_data = func;
      	entry->ent_pri = PRI_BRIEF_REPORT;
   } 

   /*
    *  Log the message.
    */
   if (msg != (U8 *)NULL)
   {
	entry = &hdr.hdr_list[hdr.hdr_entries++];
	entry->ent_type = ENT_STRING;
	entry->ent_size = strlen(msg) + 1;
        entry->ent_vers = 0;
      	entry->ent_data = msg;
      	entry->ent_pri = PRI_BRIEF_REPORT;
   }

   /*
    * Log the AHA_SOFTC structure
    */
   if (flags & SIM_LOG_HBA_SOFTC)
   {
	if (sim_softc != (SIM_SOFTC *)NULL)
	{
	   aha_softc = sim_softc->hba_sc;	/* get softc pointer */
	   
	   entry = &hdr.hdr_list[hdr.hdr_entries++];
	   entry->ent_type = ENT_AHA_SOFTC;

	   /* 
	    * Don't log the entire AHA_SOFTC structure since most of
	    * it is the activeq array which is 4K on Alpha and not
	    * needed by the error logger.  Hence the activeq array,
	    * the free queue, and the lock substructures are not
	    * logged.
	    */
	   entry->ent_size = sizeof(AHA_SOFTC) - 
	            ( (sizeof(AHA_Q) * AHA_NDPS * AHA_NLPT) + sizeof(AHA_Q) +
			sizeof(simple_lock_data_t) );

	   entry->ent_vers = AHA_SOFTC_VERSION;
	   entry->ent_data = (U8 *)aha_softc;
	   entry->ent_pri = PRI_FULL_REPORT;

	}
   }

   /*
    * Log the job block structure if any specified
    */
   if (flags & SIM_LOG_HBA_DME)
   {
	if (aha_job)
	{
  	   entry = &hdr.hdr_list[hdr.hdr_entries++];
	   entry->ent_type = ENT_AHA_JOB;
	   entry->ent_size = sizeof(AHA_JOB);
	   entry->ent_vers = AHA_JOB_VERSION;
	   entry->ent_data = (U8 *)aha_job;
	   entry->ent_pri = PRI_FULL_REPORT;
	}
   }

   /*
    * Log the interrupt status register content if passed
    */
   if (flags & SIM_LOG_HBA_CSR)
   {
	entry = &hdr.hdr_list[hdr.hdr_entries++];
	entry->ent_type = ENT_STRUCT_UNKNOWN;
	entry->ent_size = sizeof(U32);
	entry->ent_vers = 1;
	entry->ent_data = (U8 *)&intreg;
	entry->ent_pri = PRI_FULL_REPORT;
   }

   /*
    * Call sc_logger to log the common structures.
    */
   sc_logger(&hdr, SIM_LOG_SIZE, sim_softc, aha_job?aha_job->aha_sws:0, flags);


} 	/* End of aha_logger() */



/* ---------------------------------------------------------------------- */
/*		PLACEHOLDER ROUTINES FOR FUTURE IMPLEMENTATION		  */
/* ---------------------------------------------------------------------- */

U32
aha_path_inq( sws )
register SIM_WS *sws;
{
   SIM_MODULE(aha_path_inq);

   SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_ERRORS),("aha_path_inq called\n"));
   return (CAM_REQ_CMP);

}	/* End of aha_path_inq() */


/* ---------------------------------------------------------------------- */
/*		ALTERNATE ABORT ROUTINE                                   */
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- *
 *                                                                        *
 * ROUTINE NAME:  aha_ss_abort2()                                         *
 *                                                                        *
 * FUNCTIONAL DESCRIPTION:                                                *
 *      This routine upgrades the abort request to a device reset         *
 *      request for now since the adapter firmware currently will try to  *
 *      reset the entire bus instead of issuing an abort message to the   *
 *      target.  Since the device reset request uses the special SIM      *
 *      working set in the SIM_SOFTC structure, this routine will just    *
 *      clear the abort I/O pending flag in the 'flags' field of the      *
 *      passed SIM working set.  Then it needs to get the device reset    *
 *      request SIM working set to perform the device reset.  If the      *
 *      device reset SIM working set is being used already for another    *
 *      device reset, this routine will just return.  Otherwise, it grabs *
 *      the device reset SIM working set, sets up the necessary flags     *
 *      and calls aha_ss_device_reset() to perform the device reset.      *
 *                                                                        *
 *      This routine will always return CAM_REQ_CMP since abort I/O       *
 *      request is not guaranteed to succeed.                             *
 *                                                                        *
 * CALLED BY:   aha_termio_abort_bdr(), aha_ss_termio()                   *
 *                                                                        *
 * FORMAL PARAMETERS:                                                     *
 *              sws -- Pointer to the SIM working set of the I/O to be    *
 *                     aborted                                            *
 *                                                                        *
 * IMPLICIT INPUTS:                                                       *
 *              Prior to entering this routine, the SIM scheduler already *
 *              set the IPL to splbio and locked the SIM_SOFTC lock.      *
 *                                                                        *
 * IMPLICIT OUTPUTS:                                                      *
 *                                                                        *
 * RETURN VALUE:                                                          *
 *              CAM_REQ_CMP                                               *
 *                                                                        *
 * SIDE EFFECTS:                                                          *
 *      Device reset might occur or bus reset might occur.                *
 *                                                                        *
 * ---------------------------------------------------------------------- */

U32
aha_ss_abort2( sws )
register SIM_WS *sws;
{
   register SIM_SOFTC *sim_softc = sws->sim_sc;
   register SIM_WS *bdr_sws;                    /* Device reset SIM working set */
   register U32 targid = sws->targid;           /* Target ID for device reset */


   SIM_MODULE(aha_ss_abort);

   SIM_PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
               ("\nEntering %s", module) );

   sws->flags &= ~SZ_ABORT_INPROG;              /* Clear abort I/O flag */

   /*
    * Set the target's bit in the SIM_SOFTC's "device_reset_needed" field.
    */
    sim_softc->device_reset_needed |= (1 << targid);

   /*
    * Make sure that the bdr_sws is available.  If not, the device reset
    * request will be picked up later by the scheduler.
    */
   if (sim_softc->bdr_sws.flags & SZ_DEVRS_INPROG)
        return (CAM_REQ_CMP);

   SIM_PRINTD(sim_softc->cntlr, targid, NOBTL, CAMD_FLOW,
              ("target to be reset: %d\n", targid));

#ifdef AHADEBUG                         /* Debug code */

   printf("aha_ss_abort:  device reset to be performed for abort, target = %d\n",
          targid);

#endif /* AHADEBUG */                         /* Debug code */

   /*
    * The SIM_SOFTC's bdr_ws working set will be used
    */
   bdr_sws = &(sim_softc->bdr_sws);

   /*
    * Set up the "tmp_ws."
    */
   sc_setup_ws(sim_softc, bdr_sws, targid, 0L);
   SC_ADD_FUNC(bdr_sws, module);
   bdr_sws->flags = SZ_DEVRS_INPROG;
   bdr_sws->ccb = (CCB_SCSIIO *)NULL;

   /*
    * Set the device's bit in the device_reset_inprog field of the
    * SIM_SOFTC and clear it in the device_reset_needed field.
    */
   sim_softc->device_reset_inprog |= 1<<targid;
   sim_softc->device_reset_needed &= ~(1<<targid);

   /*
    * Call device reset routine to handle reset
    */
   if ( aha_ss_device_reset(bdr_sws) != CAM_REQ_CMP)
   {
        /*
         * If unsuccessful in sending device reset request to the
         * adapter, we need to clear some flags so that the device
         * reset request can be re-issued by the scheduler later.
         */
        sim_softc->device_reset_inprog &= ~(1<<targid);
        sim_softc->device_reset_needed |= 1<<targid;

        bdr_sws->flags &= ~SZ_DEVRS_INPROG;
   }

   return (CAM_REQ_CMP);


}       /* End of aha_ss_abort2() */



/* ---------------------------------------------------------------------- */
/*		DEBUG ROUTINES 						  */
/* ---------------------------------------------------------------------- */

/*
 * aha_printcb()
 *
 *	Debug routine to print a control block contents
 */

void 
aha_printcb( cbp )
register AHA_CB *cbp;		/* pointer to the control block */
{
#ifdef AHADEBUG                         /* Debug code */

   register U64 i;

   printf("control block pointer = 0x%x\n", cbp);
   printf("cbp->command    = 0x%x\n", cbp->command & 0xffff);
   printf("cbp->flag1      = 0x%x\n", cbp->flag1 & 0xffff);
   printf("cbp->flag2      = 0x%x\n", cbp->flag2 & 0xffff);
   printf("cbp->dlptr      = 0x%x\n", cbp->dlptr);
   printf("cbp->dllen      = 0x%x\n", cbp->dllen);
   printf("cbp->stat_ptr   = 0x%x\n", cbp->stat_ptr);
   printf("cbp->chnaddr    = 0x%x\n", cbp->chnaddr);
   printf("cbp->snsptr     = 0x%x\n", cbp->snsptr);
   printf("cbp->snslen     = 0x%x\n", cbp->snslen & 0xff);
   printf("cbp->cdblen     = 0x%x\n", cbp->cdblen & 0xff);
   printf("cbp->data_cksum = 0x%x\n", cbp->data_cksum & 0xffff);

   for (i = 0; i < 12; i++)
   {
   	printf("cbp->cdb[%d]  = 0x%x\t", i, cbp->cdb[i] & 0xff);
   	if ((i % 3) == 2)
	   printf("\n");
   }

#endif /* AHADEBUG */                         /* Debug code */

}	/* End of aha_printcb() */

/*
 * aha_printsb()
 *
 *	Debug routine to print a status block contents
 */

void
aha_printsb( sbp )
register AHA_SB *sbp;		/* pointer to the status block */
{
#ifdef AHADEBUG                         /* Debug code */

   register U64 i;

   printf("status block pointer = 0x%x\n", sbp);
   printf("sbp->stat       = 0x%x\n", sbp->stat & 0xffff);
   printf("sbp->hstat      = 0x%x\n", sbp->hstat & 0xff);
   printf("sbp->tstat      = 0x%x\n", sbp->tstat & 0xff);
   printf("sbp->rescnt     = 0x%x\n", sbp->rescnt);
   printf("sbp->resaddr    = 0x%x\n", sbp->resaddr);
   printf("sbp->addstlen   = 0x%x\n", sbp->addstlen & 0xffff);
   printf("sbp->snslen     = 0x%x\n", sbp->snslen & 0xff);

   for (i = 0; i < 6; i++)
   {
	printf("sbp->cdb[%d]  = 0x%x\t", i, sbp->cdb[i] & 0xff);
	if ((i % 3) == 2)
	   printf("\n");
   }

#endif AHADEBUG                         /* Debug code */

}	/* End of aha_printsb() */

/*
 * aha_print_inq()
 *
 *	Debug routine to print the inquiry data block
 */

void
aha_print_inq( inqp )
register AHA_INQ_DATA *inqp;	/* pointer to the inquiry data block */
{
#ifdef AHADEBUG                         /* Debug code */

   register U64 i;

   printf("Adapter inquiry data pointer = 0x%x\n", inqp);
   printf("inqp->dev_type	 = 0x%x\n", inqp->dev_type & 0xffff);
   printf("inqp->support_level   = 0x%x\n", inqp->support_level & 0xffff);
   printf("inqp->residual_length = 0x%x\n", inqp->residual_length & 0xff);
   printf("inqp->num_lun	 = 0x%x\n", inqp->num_lun & 0xff);
   printf("inqp->num_cb		 = 0x%x\n", inqp->num_cb & 0xff);
   printf("inqp->flags		 = 0x%x\n", inqp->flags & 0xff);
   printf("inqp->vendor_name     = %8c\n", inqp->vendor_name[0]);
   printf("inqp->product_id	 = %8c\n", inqp->product_id[0]);
   printf("inqp->firmware_type	 = %8c\n", inqp->firmware_type[0]);
   printf("inqp->firmware_rev	 = 0x%x\n", inqp->firmware_rev);
   printf("inqp->release_date	 = %8c\n", inqp->release_date[0]);
   printf("inqp->release_time	 = %8c\n", inqp->release_time[0]);
   printf("inqp->firmware_checksum = 0x%x\n", inqp->firmware_checksum & 0xffff);

#endif AHADEBUG                         /* Debug code */

}	/* End of aha_print_inq() */
