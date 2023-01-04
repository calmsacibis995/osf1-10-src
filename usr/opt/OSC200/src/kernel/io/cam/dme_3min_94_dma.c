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
static char *rcsid = "@(#)$RCSfile: dme_3min_94_dma.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/10/14 12:43:06 $";
#endif

/**
 * FACILITY:
 * 
 *      ULTRIX SCSI CAM SUBSYSTEM
 *
 * ABSTRACT:
 *
 * The Data Mover Engine (DME) component is an evolution of the
 * PDMA method of data transfer used in the existing ULTRIX SCSI support.
 * PDMA's intent was to hide the details of data movement in different
 * HBA's, behind a well defined sequence of higher level functions. The
 * extensions to PDMA which will be discussed in this document, address
 * the CAM's requirements for the support of LUN's, queued commands and
 * the use of mapping in next generation SCSI HBA's (such as the XZA).
 *
 * The DME model has broken down data transfer operations into
 * 8 distinct steps or primitives: 
 * 
 * dme_init - Initialize DME subsystem, resource queues and control 
 * structures.
 * dme_setup - Allocate required DMA resources and preload buffer on write.
 * dme_start - Transfer setup information from DME segment structures to the 
 *             HBA and start DMA activity. Points to dme_resume.
 * dme_pause - Instructs the DME to pause the DMA to be resumed later.  
 * dme_save - Instructs the DME to save the current pointers
 * dme_restore - Instructs the DME to restore the current pointers
 * dme_resume - Continue (restart) DMA after terminal count or PAUSE and fill 
 *            or clear next buffer.  
 * dme_end - Complete the DMA transaction, release any buffers, move residual
 *           data to user space, and return actual bytes sent or received.
 * 
 * There are several other utility functions supported by DME.
 * 
 * dme_copyin -Moves bytes out of the intermediate buffer to their
 * destination.
 * dme_copyout - Moves bytes into the intermediate buffer.
 * dme_clear - Utility routine used to zero buffers for security reasons.
 * dme_modify - Instructs the DME to modify the current data pointers.
 *
 *
 * AUTHOR:
 *
 *      Robert P. Scott         6-Mar-1991
 *              Based upon original 3MAX implementation by Rich Napolitano
 *
 * MODIFIED BY:
 * 
 *	07/20/91	rps	Changed K?SEG handling, fragment buffer usage,
 *				IOASIC register loading.
 *
 *	09/09/91	rps	Temporary replacement of memory allocation code.
 *
 *	09/18/91	rps	Bad handling of swap requests; fixed uvtok0.
 *
 *      09/20/91	rps	Missing '94 command on writing of fragments.
 *
 *      10/02/91 	rps	DMA was turned off too soon for some tapes.
 *				Remove offending DMA control.
 *
 *	10/22/91	janet	Added include of scsi_all.h and sim_target.h
 *
 *	11/04/91	rps	Stop looking directly at '94 for TC bit.  Look
 *				at saved data structure instead.
 *
 *	11/06/91	rps	Always check passed paramaters
 *				(and their contents,...) for valid pointers!
 *
 *	11/06/91	jag	Removed the IOASIC Magic number and used
 *				an offset from the 94 base.
 *
 *	11/13/91	janet	setup clock in sim94_softc
 *
 *	11/22/91	rps	Change uvtok0 to avtok0; handle all KUSEG addrs.
 *
 **/

/*LINTLIBRARY*/

/* ---------------------------------------------------------------------- 
 *
 * Include files.
 *
 */
#include <io/common/iotypes.h>
#include <sys/types.h>
#include <hal/cpuconf.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <kern/lock.h>
#include <io/cam/dec_cam.h>
#include <mach/vm_param.h>
#include <io/cam/cam.h>
#include <io/cam/scsi_all.h>
#include <machine/pmap.h>
#include <io/cam/cam_debug.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/dme.h>
#include <io/cam/sim.h>
#include <io/cam/sim_94.h>
#include <io/cam/sim_common.h>
#include <io/cam/dme_3min_94_dma.h>
#include <io/dec/tc/tc.h>

/* ---------------------------------------------------------------------- 
 *
 * Local defines:
 */
#define SIM_KN02BA_CLK_SPEED	24	/* 24 MHz */

/* ---------------------------------------------------------------------- 
 *
 * External declarations:
 *
 */
extern SIM_SOFTC *softc_directory[];

extern struct proc *proc;		/* Process structure                */

extern int printstate;
extern struct timeval sxtime;           /* System time data cell          */
extern int sim_poll_mode;

/* Change this later should be int not u_long */
extern U32 bcopy();                  /* System routine to copy buffer  */
extern U32 bzero();                  /* System routine to clear buffer */

/* ---------------------------------------------------------------------- 
 *
 * Function Prototypes:
 */

char *avtok0();

U32 dme_copyin();
U32 dme_copyout();
U32 dme_3min_init();
U32 dme_3min_setup();
U32 dme_3min_end();
U32 dme_3min_pause();
U32 dme_3min_resume();
U32 dme_3min_save();
U32 dme_3min_restore();
U32 dme_3min_modify();
U32 dme_3min_clear();
U32 dme_3min_bump_sglist();
U32 dme_3min_interrupt();

/* ----------------------------------------------------------------------
 * 
 * Initialized and uninitialized data: 
 * 
 */

FRAGBUF *fragbase;

/*---------------------------------------------------------------------- 
 *
 * Local Type Definitions and Constants
 *
 */ 

DME_3MIN_TABLE *
new_dme_3min_table( void )
{
    DME_3MIN_TABLE *ichi, *ni;
    int ika;

    ichi = (DME_3MIN_TABLE *)sc_alloc(MAX_TABLE_SIZE);
    if ( !ichi ) {
        printf("new_dme_3min_table: Error allocating memory for table.\n" );
    }

    else {
	for( ika=0; ika<MAX_TABLE_ENTRIES; ika++ )
        {
	    ni = (DME_3MIN_TABLE *) &ichi[ika];
	    ni->length = 0;
	    ni->addr = NULL;
	    ni->uadr = NULL;
	    ni->iadr = NULL;
	    ni->completed = DME_3MIN_TABLE_INCOMPLETE;
	    ni->dir = IOASIC_UNDEF;
        }
    }
    return (DME_3MIN_TABLE *) ichi;
}

void
init_dme_3min_struct( DME_3MIN_STRUCT *d )
    {
    d->frag_buffer = (FRAGBUF *) NULL;
    d->frag_table = (DME_3MIN_TABLE *) NULL;
    d->sim_ws = NULL;
    d->xfer_size = 0;
    d->xfer_current_count = 0;
    d->state = DME_STATE_UNDEFINED;
    }

/**
 * dme_3min_init -
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Initialize the DME subsystem. Perform DME specific initialization.
 *
 * FORMAL PARAMETERS:
 *                      SIM_SOFTC* sc - Address of sim controller
 *                                             structure.
 *
 * IMPLICIT INPUTS:
 *       NONE
 *
 * IMPLICIT OUTPUTS:
 *      NONE
 *
 * RETURN VALUE:
 *       CAM_REQ_CMP            - All is well
 *       CAM_REQ_CMP_ERR            - DME failed to initialize
 *
 * SIDE EFFECTS:
 *       NONE
 *
 * ADDITIONAL INFORMATION:
 *       NONE
 *
 *
 **/
U32
dme_3min_init( SIM_SOFTC *sc )
{
    int *slotp;                 /* IOASIC SCSI DMA slot register pointer */
    DME_3MIN_STRUCT *ldme;      /* private DME struct pointer */
    int i;                      /* private counter */
    
    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
           ("[b/t/l] (dme_3min_init) Entry - sc=0x%x\n", sc ) );

    if ( sc->simd_init )
    {
	PRINTD(NOBTL, NOBTL, NOBTL, CAMD_DMA_FLOW,
	       ("[b/t/l] (dme_3min_init): Duplicate init call\n" ) );
	return CAM_REQ_CMP_ERR;
    }
    
    ldme = (DME_3MIN_STRUCT *)sc_alloc(MAX_UNIT_COUNT*sizeof(DME_3MIN_STRUCT));
    if ( !ldme )
    {
	printf("dme_3min_init: Couldn't allocate memory for DME_3MIN_STRUCT\n" );
        return CAM_REQ_CMP_ERR;
    }
    
    fragbase = (FRAGBUF *)sc_alloc(max(4096,sizeof(FRAGBUF)*MAX_UNIT_COUNT));
    if ( !fragbase )
    {
	printf("dme_3min_init: Error allocating memory for fragment buffers.\n" );
	return CAM_REQ_CMP_ERR;
    } 

    sc->dme->extension = (void *) ldme;   /* fill in the DME specific field */
    
    for( i=0; i<MAX_UNIT_COUNT; i++ )
        init_dme_3min_struct( &ldme[i] );
    
    sc->dmaeng = sc->dme->hardware;   /* prep alternate pointer to hardware */
    
    /* The IOASIC SCSI DMA slot register MUST be initialized before any
       communication with the '94 is attempted.  */
    slotp = (int *) ( (unsigned)sc->dmaeng + SCSI_DMASLOT_O);
    
    *slotp = SCSI_SLOT_DATA; /* magic cookie as found in v1.6 of spec */
    
    *(int *)(PHYS_TO_K1( (unsigned)sc->dmaeng + IOA_S_DMABP_O ) ) = 0;
    sc->simd_init = 1;
    
    return CAM_REQ_CMP;
}

/**
 * dme_3min_setup -
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Allocate required DMA resources whether they be mapping registers or
 * segments of the intermediate DMA buffer and initialize the allocated
 * resource. In systems with an intermediate DMA buffer, this routine will
 * use the  Active Segment Array to allocated a segment suitable for this
 * data transfer. In the case of write operations, where the segment is
 * split into multiple buffers, the first buffer will be prefilled with data
 * to be sent to a target.
 *
 * FORMAL PARAMETERS:
 *
 *    sim_wset            - SIM Working Set of Active I/O request.
 *    count             - Number of bytes to transfer
 *    buf               - Address of source/dest buffer 
 *    dir               - Direction of data transfer,RD/WR? 
 *    dme_desc          - Address of descriptor that describes this DMA request
 *
 * IMPLICIT INPUTS:
 *      NONE
 *
 * IMPLICIT OUTPUTS:    
 *      NONE
 *
 * RETURN VALUE:
 *       CAM_REQ_CMP      - All is well
 *       CAM_FAIL         - a fatal(?) problem has  occurred
 *
 * RETURN VALUE:                TBD
 *
 * SIDE EFFECTS:                NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
dme_3min_setup( SIM_WS *sim_wset, U32 count, void *buf, U32 dir,
    DME_DESCRIPTOR *dme_desc )
    {
    SIM_SOFTC *sc;
    DME_3MIN_STRUCT *ldme, *dme;
    int controller = sim_wset->cntlr;
    int target = sim_wset->targid;
    int lun = sim_wset->lun;
    int ent = controller*MAX_CONTROLLER+target*MAX_TARGET+lun;

    PRINTD( controller, target, lun, CAMD_DMA_FLOW,
        ("[%d/%d/%d] (dme_3min_setup) Entry - sws=0x%x, count=%d,\n",
	 controller, target, lun, sim_wset, count ) );
    PRINTD( controller, target, lun, CAMD_DMA_FLOW,
        ("    buf=0x%x, dir=%d, dme_desc=0x%x\n", buf, dir, dme_desc ) );

    if ( !sim_wset || !dme_desc )
	return CAM_REQ_CMP_ERR;

    /*
     * Assign SIM_WS pointer to the passed DME_DESCRIPTOR.
     * Setup the data count, data ptr, and direction in the
     * DME_DESCRIPTOR.
     */
    dme_desc->sim_ws = (void *)sim_wset;  /* Get a copy for later use        */
    dme_desc->data_count = count;       /* How big is the entire request?  */
    dme_desc->data_ptr = buf;           /* User buffer address S0 or P0    */
    dme_desc->flags |= dir;             /* Determine whether to write/read */

    sc = GET_SOFTC_DESC( dme_desc );
    dme = (DME_3MIN_STRUCT *) sc->dme->extension;
    ldme = (DME_3MIN_STRUCT *) &dme[ent];

    ldme->state = DME_STATE_SETUP;

    if ( ldme->frag_table == NULL )            /* Has it been used yet? */
      {
        /* Allocate a transfer table for this target */
        ldme->frag_table = new_dme_3min_table();
        if ( ldme->frag_table == (DME_3MIN_TABLE *) NULL )
	  return CAM_REQ_CMP_ERR;

	ldme->frag_buffer = ( FRAGBUF * ) &fragbase[ent];
      }

    ldme->sim_ws = sim_wset;             /* the working set for this xfer */
    ldme->dme_desc = dme_desc;           /* for cross check               */


    if (sim_wset->cam_flags & CAM_SCATTER_VALID)
    {
	ldme->sg.xfer_cnt = 0; 
	    
	/*
	 * Setup counter of which scatter gather elemnt we are processing
	 * at this time.
	 */
	ldme->sg.element_cnt = 1; 
	
	/*
	 * Get pointer to the first SG_ELEM, in the case of a scatter 
	 * gather request the data_ptr points at the address of the 
	 * scatter gather list.
	 */
	ldme->sg.element_ptr = ((SG_ELEM*)dme_desc->data_ptr);
	
	/* 
	 * Get pointer to the buffer described by the first scatter gather
	 * element.
	 */
	ldme->user_buffer = ldme->sg.element_ptr->cam_sg_address;
	
	/*
	 * Start off with the transfer counter
	 */
	ldme->xfer_size = ldme->sg.element_ptr->cam_sg_count;

	ldme->xfer_current_count = 0;    /* running count of bytes xfer'd */
	ldme->direction = dir;           /* and direction of transfer     */
	
    }
    else
    {
	ldme->xfer_size = count;         /* requested transfer size       */
	ldme->xfer_current_count = 0;    /* running count of bytes xfer'd */
	ldme->user_buffer = buf;         /* the user's buffer             */
	ldme->direction = dir;           /* and direction of transfer     */
    }

    if ( build_frag_table( sim_wset, ldme ) == CAM_REQ_CMP_ERR )
        {
	PRINTD( controller, target, lun, CAMD_DMA_FLOW,
            ("[%d/%d/%d] (dme_3min_setup) Failed building xfer table.\n",
	     controller, target, lun ) );
        printf("dme_3min_setup: (%d/%d/%d) Failed building transfer table.\n",
		controller, target, lun );
	return (U32) CAM_REQ_CMP_ERR;
       }

    dme_desc->segment = (void *) ldme;    /* save our context here         */
    /*
     * Do a save of the data pointers at this point to insure, that we can
     * do an automatic restore in the case where the target disconnects and
     * reconnects as an error recovery mechanism. In this case, the target
     * is expecting that initiator will be doing an implicite restore pointers.
     * In order to do an implicite restore pointers, we must do an explicite
     * save pointers here!
     */
    dme_3min_save(dme_desc);

    return CAM_REQ_CMP;
    }                   /* dme_setup */

/**
 * dme_3min_resume -
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Start DMA activity between the target and initiator. This routine uses
 * the state of the DMA activity previously initialized by the SIM.
 *
 * NOTE: This function is not used by the HBA during normal operation and
 * is only used during debug of the DME.
 *
 * FORMAL PARAMETERS:
 *    dme_desc          - Address of descriptor that describes this DMA request
 *
 * IMPLICIT INPUTS:             NONE
 *
 * IMPLICIT OUTPUTS:            NONE
 *
 * RETURN VALUE:                CAM_REQ_CMP - All is well.
 *
 * SIDE EFFECTS:                NONE
 *
 * ADDITIONAL INFORMATION:      This routine was previously dme_3min_start.
 *
 **/
U32
dme_3min_resume( DME_DESCRIPTOR* dme_desc )
    {
    int s;                              /* Used by DME lock                 */
    SIM_SOFTC* sc;                      /* Pointer to SIM S/W control struct*/
    U32 retval = CAM_REQ_CMP;        /* Return value                     */
    SIM94_REG *reg94;                   /* Address of 94 CSR's              */
    SIM_WS *sim_wset;                   /* SIM working set for this request */
    DME_3MIN_TABLE *xt, *te;            /* Our transfer descriptor table    */
    DME_3MIN_STRUCT *ldme;              /* DME specific data struct ptr.    */
    int controller;                     /* Controller in this request       */
    int target;                         /* Target in this request           */
    int lun;                            /* LUN in this request              */

    PRINTD( controller, target, lun, CAMD_DMA_FLOW,
           ("[%d/%d/%d] (dme_3min_resume) Entry - dme_desc=0x%x\n", 
	    controller, target, lun, dme_desc ) );
    
    if ( !dme_desc || !dme_desc->segment || !dme_desc->sim_ws )
	return CAM_REQ_CMP_ERR;

    /*
     * Setup required local variables.
     */
    sim_wset = dme_desc->sim_ws;
    controller = sim_wset->cntlr;
    target = sim_wset->targid;
    lun = sim_wset->lun;
    sc = GET_SOFTC_DESC( dme_desc );
    reg94 = SIM94_GET_CSR( sc );

    ldme = (DME_3MIN_STRUCT *) dme_desc->segment;
    if ( ldme == (DME_3MIN_STRUCT *) CAM_REQ_CMP_ERR )
      {
	printf( "(dme_3min_resume) Invalid ldme pointer.\n" );
        return (U32) CAM_REQ_CMP_ERR;
      }

    ldme->state = DME_STATE_RESUME;         /* Last DME state                */

    PRINTD( controller, target, lun, CAMD_DMA_FLOW,
           ("[%d/%d/%d] (dme_3min_resume) - dme=0x%x\n", 
	    controller, target, lun, ldme ) );

    if ( ldme->frag_index == (MAX_TABLE_ENTRIES - 1))
    {
	if ( build_frag_table( sim_wset, ldme ) == CAM_REQ_CMP_ERR )
        {
	    PRINTD( controller, target, lun, CAMD_DMA_FLOW,
		   ("[%d/%d/%d] (dme_3min_setup) Failed building xfer table.\n",
		    controller, target, lun ) );
	    printf("dme_3min_setup: (%d/%d/%d) Failed building transfer table.\n",
		   controller, target, lun );
	    return (U32) CAM_REQ_CMP_ERR;
	}
    }

    xt = (DME_3MIN_TABLE *) ldme->frag_table;
    te = (DME_3MIN_TABLE *) &xt[ldme->frag_index];

    /*
     * Setup HBA to start DMA activity.
     */
    LOCK_DME(s,sc);              /* synchronize access to hba */
    sim_wset->flags |= SZ_DME_ACTIVE;

    if ( te->length == 0 )              /* special case for padding or ? */
    {
        UNLOCK_DME(s,sc);           /* release lock on hba           */
	return CAM_DATA_RUN_ERR;
    }
    else
    {
        if ( te->uadr && te->dir == IOASIC_WRITE )   /* If a transfer of less*/
	{                                /* than 8 and a write...          */
	    SIM94_FLUSH_FIFO(reg94);
            sim94_load_fifo( reg94, te->length, te->uadr );
            SIM94_SEND_CMD( sim_wset, reg94,
			    SIM94_CMD_XFERINFO, sim_poll_mode); /* do it */
	}
        else
	{                                /* otherwise, use DMA             */
            set_ioasic_control( sc, 0 );
            retval = dma_pointer_load( sc, te->iadr );
            if ( retval == CAM_REQ_CMP_ERR )
	    {                            /* problem loading pointer!       */
		PRINTD( controller, target, lun, CAMD_DMA_FLOW,
		       ( "[%d/%d/%d] (dme_3min_resume) - Fail return from dma_pointer_load\n",
			controller, target, lun ) );
		dumpent( te );
                UNLOCK_DME(s,sc);           /* release lock on hba           */
		return (U32) retval;
	    }
            else
	    {
                ssr_dma_on( sc, te->dir );  /* Enable DMA and direction      */
		reg94->sim94_tcmsb = ( ( te->length ) & 0xff00 ) >> 8;
		reg94->sim94_tclsb = ( te->length ) & 0x00ff;
		wbflush();
                SIM94_SEND_CMD( sim_wset, reg94,
				SIM94_CMD_DMAXFER, sim_poll_mode ); /* do it */
                wbflush();                  /* conserve water                */
              }
          }
      }
    
    UNLOCK_DME(s,sc);                       /* release lock on hba           */

    return (U32) retval;
  }                     /* dme_resume */

/**
 * dme_3min_pause -
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * This function is called by the HBA each time the HBA receives an interrupt
 * from the DME subsystem when the DME is active. dme_pause() will read the
 * state of device registers to determine the progress of a dma transaction.
 * The state of the dma request will be updated to reflect the number of bytes
 * successfully transfered.
 *
 * This function must be called before a dme_save, dme_end or dme_resume call
 * may be executed by the HBA.
 *
 * FORMAL PARAMETERS:
 *    dme_desc          - Address of descriptor that describes this DMA request
 *
 * IMPLICIT INPUTS:             NONE
 *
 * IMPLICIT OUTPUTS:            NONE
 *
 * RETURN VALUE:                CAM_REQ_CMP - All is well.
 *
 * SIDE EFFECTS:                NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
dme_3min_pause( DME_DESCRIPTOR *dme_desc )
  {
    SIM_SOFTC* sc;                      /* Pointer to SIM S/W control struct */
    U32 retval = CAM_REQ_CMP;        /* Return value                      */
    SIM94_REG *reg94;                   /* Address of 94 CSR's               */
    SIM_WS *sim_wset;                   /* Working set pointer for this req. */
    SIM94_SOFTC *ssc;                   /* HBA specific pointer              */
    DME_3MIN_STRUCT *ldme;              /* DME specific pointer              */
    DME_3MIN_TABLE *xt, *te;            /* Our transfer descriptor table     */
    U32 fifo_residual;               /* Number of bytes left in FIFO      */
    U32 residual;                    /* Number of bytes left Transfer CNT */
    int ioasic_residual;             /* Number of bytes left in IOASIC    */
    U32 xfer_cnt;                    /* Number of bytes really transfered */
    int s;                              /* Save the current IPL              */
    int controller;                     /* Controller in request             */
    int target;                         /* Target in request                 */
    int lun;                            /* LUN in request                    */
    struct buf *bp;			/* Pointer to the users BP           */
    struct proc *process;		/* Process structure                 */
    char *sva;                          /* Temp. system virtual address      */
    u_char sr;                          /* '94 Status Register               */

    PRINTD( controller, target, lun, CAMD_DMA_FLOW,
           ("[%d/%d/%d] (dme_3min_pause) - Entry - dme_desc=0x%x\n", 
	    controller, target, lun, dme_desc ) );

    /* Passed in bad pointer?  Return an error */
    if ( !dme_desc || !dme_desc->segment || !dme_desc->sim_ws )
	return CAM_REQ_CMP_ERR;

    /*
     * Setup required local variables.
     */
    sim_wset = dme_desc->sim_ws;
    ssc = (SIM94_SOFTC *)sim_wset->sim_sc->hba_sc;
    sr = SIM94_GET_SR((SIM94_INTR *)ssc->active_intr);
    controller = sim_wset->cntlr;
    target = sim_wset->targid;
    lun = sim_wset->lun;
    sc = GET_SOFTC_DESC( dme_desc );
    reg94 = SIM94_GET_CSR( sc );

    ldme = (DME_3MIN_STRUCT *) dme_desc->segment;
    if ( ldme == (DME_3MIN_STRUCT *) CAM_REQ_CMP_ERR )
      {
	printf("dme_3min_pause: Invalid ldme pointer.\n" );
	return (U32) CAM_REQ_CMP_ERR;
      }

    ldme->state = DME_STATE_PAUSE;              /* Last DME state           */

    xt = (DME_3MIN_TABLE *) ldme->frag_table;  
    te = (DME_3MIN_TABLE *) &xt[ldme->frag_index];

    if ( te->length < 1 || !( sim_wset->flags & SZ_DME_ACTIVE ) )
    {                             /* Why do we get called when there */
                                  /* is nothing to do?               */  
	return (U32) CAM_REQ_CMP;       /* Who knows!                      */
    }

    /*
     * Clear the DME active bit, so that that SIM HBA doesn't think that
     * any future interrupts are from the DME.
     */
    sim_wset->flags &= ~SZ_DME_ACTIVE;

    LOCK_DME( s, sc );

    ssr_dma_off( sc );

    /*
     * Get the count of the number of bytes that were left in the fifo.
     * Sometimes the DMA engine will not transfer all the bytes and the
     * residual is left in the fifo.
     */
    fifo_residual = reg94->sim94_ffss & SIM94_FIFO_MSK;

    PRINTD(controller, target, lun, CAMD_DMA_FLOW,
       ("[%d/%d/%d] (dme_3min_pause) - The fifo residual byte count was %d.\n",
	controller, target, lun, fifo_residual ) );

    residual = GET_94_COUNTER( reg94 );
    PRINTD(controller, target, lun, CAMD_DMA_FLOW,
       ("[%d/%d/%d] (dme_3min_pause) - The '94 residual byte count was %d.\n",
	controller, target, lun, residual));

    if ( sr & SIM94_STAT_TC )  /* timing problem? */
    {   /* RPS - This test/code should come out if the cprint never occurs */
	if ( residual )
	    printf("TC bit is set, residual is 0x%x, fifo_residual = 0x%x\n", 
		    residual, fifo_residual );
	residual = 0;
    }

    if ( te->length > 8 && residual > te->length ) 
    {   /* RPS - This test/code should come out if the cprint never occurs */
	printf("[%d/%d/%d] resid %d > te->length %d\n", 
		controller, target, lun, residual, te->length );
	residual = 0;
    }

    ioasic_residual = get_ioasic_count( sc, te->dir );
    PRINTD(controller, target, lun, CAMD_DMA_FLOW,
     ("[%d/%d/%d] (dme_3min_pause) - The ioasic residual byte count was %d.\n",
      controller, target, lun, ioasic_residual));

    if ( ioasic_residual && ( te->length & 1 )) /* if odd, then count is off */
    {                                           /* by 1. RPS 03/28/91        */
	ioasic_residual--;
    }

    if ( te->dir == IOASIC_READ )
    {
	xfer_cnt = te->length - residual;

	ldme->xfer_current_count += xfer_cnt;    /* adjust transferred count */

        if ( ioasic_residual )
            flush_ioasic( sc, ioasic_residual );
        if ( te->uadr )
	{
            flush_fragment_buffer( sim_wset, ldme, xfer_cnt );
	}
        else
	{ 
	    if ( CAM_IS_KUSEG( te->addr ) )
	    {
		bp = GET_BP_SIMWS(sim_wset); /* Get the Buffer Struct Addr  */
        	sva = (void *)PHYS_TO_K0(pmap_extract(bp->b_proc->task->map->vm_pmap, te->addr ));

		clean_dcache( sva, te->length );
	    }
	    else
	    {
		U32 pfn;
		svatophys( te->addr, &pfn );
		clean_dcache( PHYS_TO_K0( pfn ), te->length );

	    }
            te->length -= xfer_cnt;           /* adjust segment length    */
            if ( te->length )                 /* not end of this segment? */
	    {                               /* then rebuild xfer table  */
		if ( build_frag_table( sim_wset, ldme ) == CAM_REQ_CMP_ERR )
		{
		    PRINTD( controller, target, lun, CAMD_DMA_FLOW,
			   ("[%d/%d/%d] (dme_3min_pause) Failed building xfer table.\n",
			    controller, target, lun ) );
		    printf("dme_3min_pause: (%d/%d/%d) Failed building transfer table.\n",
			    controller, target, lun );
                    UNLOCK_DME(s,sc);      /* Get device lock          */
		    return (U32) CAM_REQ_CMP_ERR;
		}
	    }
	    else
	    {
                te->completed = DME_3MIN_TABLE_COMPLETE;
                ldme->frag_index++;            /* increment segment index  */
	    }
	}
    }
    else                                      /* A write operation        */
    {
	if ( te->uadr )          
            xfer_cnt = te->length - fifo_residual;
        else
	    xfer_cnt = te->length - residual - fifo_residual;

        SIM94_FLUSH_FIFO(reg94);

	ldme->xfer_current_count += xfer_cnt; /* adjust transferred count */

        te->length -= xfer_cnt;               /* adjust segment length    */
        if ( te->length )                    /* is this segment done?    */
	{
	    if ( build_frag_table( sim_wset, ldme ) == CAM_REQ_CMP_ERR )
	    {
		PRINTD( controller, target, lun, CAMD_DMA_FLOW,
		       ("[%d/%d/%d] (dme_3min_pause) Failed building xfer table.\n",
			controller, target, lun ) );
		printf("dme_3min_pause: (%d/%d/%d) Failed building transfer table.\n",
			controller, target, lun );
                UNLOCK_DME(s,sc);      /* Get device lock          */
		return (U32) CAM_REQ_CMP_ERR;
	    }
	}
	else
	{
            te->completed = DME_3MIN_TABLE_COMPLETE; /*mark this segment done*/
            ldme->frag_index++;                /* point to next segment    */
	}
    }

    dme_desc->xfer_count += xfer_cnt;/* Total bytes transfered */

    te = (DME_3MIN_TABLE *) &xt[ldme->frag_index];

    if (sim_wset->cam_flags & CAM_SCATTER_VALID && te->length == 0 )
    {
        ldme->sg.xfer_cnt = ldme->xfer_current_count;

	if ( dme_3min_bump_sglist( dme_desc, ldme ) == CAM_REQ_CMP_ERR )
	{
	    PRINTD( controller, target, lun, CAMD_DMA_FLOW,
		   ("[%d/%d/%d] (dme_3min_pause) Reached end of SG list.\n",
		    controller, target, lun ) );
	    
	}
	else
	{
	    if ( build_frag_table( sim_wset, ldme ) == CAM_REQ_CMP_ERR )
	    {
		PRINTD( controller, target, lun, CAMD_DMA_FLOW,
		       ("[%d/%d/%d] (dme_3min_pause) Failed building xfer table after SG bump.\n",
			controller, target, lun ) );
		printf("dme_3min_pause: (%d/%d/%d) Failed building transfer table after SG bump.\n",
			controller, target, lun );
                UNLOCK_DME(s,sc);      /* Get device lock          */
		return (U32) CAM_REQ_CMP_ERR;
	    }
	}
    }

    UNLOCK_DME(s,sc);                  /* Get device lock          */

    return (U32) retval;
}                                           /* END of dme_pause         */

/**
 * dme_3min_save - Maybe this should be a macro!
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * This function is called by the HBA when it receives a SAVE POINTERS message
 * from the target. The routine will save the current state of the DME of this
 * descriptor for later use.
 *
 * FORMAL PARAMETERS:
 *    dme_desc          - Address of descriptor that describes this DMA request
 *
 * IMPLICIT INPUTS:             NONE
 *
 * IMPLICIT OUTPUTS:            NONE
 *
 * RETURN VALUE:                CAM_REQ_CMP - All is well.
 *
 * SIDE EFFECTS:                NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
dme_3min_save( DME_DESCRIPTOR *dme_desc )
  {
    U32 retval=CAM_REQ_CMP;         /* Return value                       */
    DME_3MIN_STRUCT *ldme;             /* DME specific data struct           */
    DME_3MIN_TABLE *xt, *te;           /* Our transfer descriptor table      */
    SIM_SOFTC* sc;                     /* Pointer to SIM S/W control struct  */
    SIM_WS *sim_wset;                  /* SIM working set for this request   */
    int controller;                    /* Controller of current request      */
    int target;                        /* Target of current request          */
    int lun;                           /* LUN of current request             */

    PRINTD(controller, target, lun, CAMD_DMA_FLOW,
           ("[%d/%d/%d] (dme_3min_save) Entry - dme_desc=0x%x\n", 
	    controller, target, lun, dme_desc ) );

    /* If a bad pointer is passed in, return an error. */
    if ( !dme_desc || !dme_desc->segment || !dme_desc->sim_ws )
	return CAM_REQ_CMP_ERR;

    /* initialize local variables */
    sim_wset = dme_desc->sim_ws;
    controller = sim_wset->cntlr;
    target = sim_wset->targid;
    lun = sim_wset->lun;
    sc = GET_SOFTC_DESC( dme_desc );

    ldme = (DME_3MIN_STRUCT *) dme_desc->segment;
    if ( ldme == (DME_3MIN_STRUCT *) CAM_REQ_CMP_ERR )
      {
	return (U32) CAM_REQ_CMP_ERR;
      }

    xt = (DME_3MIN_TABLE *) ldme->frag_table;
    te = (DME_3MIN_TABLE *) &xt[ldme->frag_index];

    return retval;
}                     /* dme_save */

/**
 * dme_3min_restore - Maybe this should be a macro!
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * This routine is called by the HBA in response to a RESTORE POINTER message.
 * This routine will return the state of the DME for this request to the last
 * saved DME state.
 *
 * FORMAL PARAMETERS:
 *    dme_desc          - Address of descriptor that describes this DMA request
 *
 * IMPLICIT INPUTS:             NONE
 *
 * IMPLICIT OUTPUTS:            NONE
 *
 * RETURN VALUE:                CAM_REQ_CMP - All is well.
 *
 * SIDE EFFECTS:                NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
dme_3min_restore( DME_DESCRIPTOR *dme_desc )
  {
    U32 retval=CAM_REQ_CMP;         /* Return value                       */
    DME_3MIN_STRUCT *ldme;             /* DME specific data struct           */
    DME_3MIN_TABLE *xt, *te;           /* Our transfer descriptor table      */
    SIM_SOFTC* sc;                     /* Pointer to Soft-c  control struct  */
    SIM_WS *sim_wset;                  /* SIM working set for this request   */
    int controller;                    /* Controller of current request      */
    int target;                        /* Target of current request          */
    int lun;                           /* LUN of current request             */

    PRINTD(controller, target, lun, CAMD_DMA_FLOW,
           ("[%d/%d/%d] (dme_3min_restore) Entry - dme_desc=0x%x\n", 
	    controller, target, lun, dme_desc ) );

    /* If a bad pointer is passed in, return an error. */
    if ( !dme_desc || !dme_desc->segment || !dme_desc->sim_ws )
	return CAM_REQ_CMP_ERR;

    /* initialize local variables */
    sim_wset = dme_desc->sim_ws;
    controller = sim_wset->cntlr;
    target = sim_wset->targid;
    lun = sim_wset->lun;
    sc = GET_SOFTC_DESC( dme_desc );

    ldme = (DME_3MIN_STRUCT *) dme_desc->segment;
    if ( ldme == (DME_3MIN_STRUCT *) CAM_REQ_CMP_ERR )
	return (U32) CAM_REQ_CMP_ERR;

    xt = (DME_3MIN_TABLE *) ldme->frag_table;
    te = (DME_3MIN_TABLE *) &xt[ldme->frag_index];

    return retval;
  }                     /* dme_restore */

/**
 * dme_end -
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * This function is called by the HBA at the end of each request in order to
 * deallocate any DME resources as well as flush any pending buffer segments
 * before completeing the request. This function MUST be called by the HBA
 * if dme_setup() has been called.
 *
 *
 * FORMAL PARAMETERS:
 *    dme_desc          - Address of descriptor that describes this DMA request
 *
 * IMPLICIT INPUTS:             NONE
 *
 * IMPLICIT OUTPUTS:            NONE
 *
 * RETURN VALUE:                CAM_REQ_CMP - All is well.
 *
 *
 * SIDE EFFECTS:                The HBA must call DME_PAUSE prior to executing
 *                              this function. DME_PAUSE prepares the SEGMENT
 2*                              ELEMENT for DME_END.
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
dme_3min_end( DME_DESCRIPTOR *dme_desc )
  {
    U32 retval=CAM_REQ_CMP;          /* Return value                      */
    DME_3MIN_STRUCT *ldme;              /* DME specific data struct          */
    SIM_WS *sim_wset;                   /* SIM working set for this request  */
    int controller;                     /* Controller of current request     */
    int target;                         /* Target of current request         */
    int lun;                            /* LUN of current request            */
    
    PRINTD(controller, target, lun, CAMD_DMA_FLOW,
           ("[b/t/l] (dme_3min_end) Entry - dme_desc=0x%x.\n", dme_desc ) );

    /* If bad pointers are passed in, then return an error. */
    if ( !dme_desc || !dme_desc->segment || !dme_desc->sim_ws )
        return (U32) CAM_REQ_CMP_ERR;

    /*
     * Setup required local variables.
     */
    sim_wset = dme_desc->sim_ws;
    controller = sim_wset->cntlr;
    target = sim_wset->targid;
    lun = sim_wset->lun;

    ldme = (DME_3MIN_STRUCT *) dme_desc->segment;
    if ( ldme == (DME_3MIN_STRUCT *) CAM_REQ_CMP_ERR )
	{
	printf( "(dme_3min_end) Invalid ldme pointer.\n" );
	return (U32) CAM_REQ_CMP_ERR;
	}

    ldme->state = DME_STATE_END;     /* Update last DME state */

    /*
     * Deallocate the fragment table at boot, it will be reallocated 
     * when the device is accessed for the first time.
     */
    if ( cam_at_boottime() )            {
    	sc_free(ldme->frag_table, MAX_TABLE_SIZE);
    	ldme->frag_table = NULL;
    }

    /*
     * Clear the DME active bit, so that that SIM HBA doesn't think that
     * any future interrupts are from the DME.
     */
    ((SIM_WS *)dme_desc->sim_ws)->flags &= ~SZ_DME_ACTIVE;

    return retval;
  }                       /* dme_end */

U32
dme_3min_interrupt( int controller )
    {
    SIM_SOFTC *sc;                   /* The control struct for subsystem     */
    unsigned *dmap;                  /* pointer to IOASIC DMA Ptr. reg.      */
    U32 *sir;              /* System Interrupt Register ptr.       */
    U32 sirp, resetval;    /* What was/will-be in SIR              */
    int retval;                      /* All important return value           */
    U32 *ioasicp;	     /* pointer the DMA engine register base */
    caddr_t pa;                      /* physical address                     */
    struct tc_memerr_status status;  /* memory error status struct           */

    PRINTD( controller, NOBTL, NOBTL, CAMD_INOUT,
	       ("[%d/t/l] (dme_3min_interrupt) Entry\n", controller) );

    sc = (SIM_SOFTC *)SC_GET_SOFTC( controller );
    ioasicp = (U32 *)sc->dmaeng;	/* engine address */
    sir = (U32 *) ( (unsigned) ioasicp + SIR_O );

    resetval = 0xffffffff;
    sirp = *sir;
    retval = 1;

    if ( sirp & SCSI_DBPL )          /* has the DMA buffer pointer           */
        {                            /* been loaded?                         */
        resetval &= ~SCSI_DBPL;
        }

    if ( sirp & SCSI_OERR )          /* has an overrun error occurred?       */
        {
        resetval &= ~SCSI_OERR;
        }

    if ( sirp & SCSI_MERR )          /* memory read error?                   */
        {
	ssr_dma_off( sc );
        dmap = (unsigned *) ( ( (unsigned) ioasicp ) + IOA_S_DMAP_O );
        pa = (char *) backcvt( (void *)*dmap ); /* Calc. the user buf address*/
        printf( "scsi%d: dma memory read error \n", controller );
        status.pa = pa;
        status.va = 0;
        status.log = TC_LOG_MEMERR;
        status.blocksize = 4;
        tc_isolate_memerr(&status);
        resetval &= ~SCSI_MERR;
        }

#ifdef NOTNEEDED
    if ( sirp & SCSI_DRDY )
        {
        retval = 0;
        printf("ioasicint: Unexpected 53C94 data ready interrupt.\n" );
        resetval &= ~SCSI_DRDY;
        }
#endif

    if ( sirp & SCSI_C94 )
        {
        retval = 0;		       /* go through '94 code                */
        resetval &= ~SCSI_C94;
        }

    *sir = resetval;
    return retval;
    }

/**
 * dme_3min_attach -
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * This function is called during the initialization sequence in the SIM
 * to configure the DME by loading the appropriate routines into the the
 * HBA_DME_CONTROL structure in the sim_softc. This function is provided 
 * to allow the Data Mover Engine to be configured based on the capabilities
 * of the underlying HBA. 
 *
 * Currently, this routine supports the 3MIN DME.
 *
 * FORMAL PARAMETERS:           SIM_SOFTC* sc - Addr of SIM cntlr strc
 *
 * IMPLICIT INPUTS:             NONE
 *
 * IMPLICIT OUTPUTS:            sc->dme initialized.
 *
 * RETURN VALUE:                CAM_REQ_CMP - All is well.
 *
 * SIDE EFFECTS:                This function calls dme_init, to init the DME
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
dma94_dme_attach( SIM_SOFTC *sc )
  {
    register volatile SIM94_REG *reg = (SIM94_REG *)sc->reg;
    register SIM94_SOFTC *ssc = (SIM94_SOFTC *)sc->hba_sc;
    DME_STRUCT *dme;

    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_DMA_FLOW,
           ("[b/t/l] (dme_3min_attach) entry - sc=0x%x\n", sc ));

    /*
     * Store the clock speed.  To be used when setting up the
     * synchronous period.  Store the clock * 10 to allow for
     * fractions.
     */
    ssc->clock = SIM_KN02BA_CLK_SPEED * 10;

    /*
     * Set the clock conversion register for 3MIN's speed.
     */
    reg->sim94_ccf = SIM94_CONVERT_CLOCK(SIM_KN02BA_CLK_SPEED);

    /*
     * Calculate the minimum period.  The value which is used to
     * negotiate synchronous is the actual period divided by four.
     * Convert the clock speed from MHz to nanoseconds by dividing
     * 1000 by the clock speed.
     */
    sc->sync_period = ((1000 / SIM_KN02BA_CLK_SPEED) * SIM94_PERIOD_MIN) / 4;

    dme = (DME_STRUCT *)sc_alloc(sizeof( DME_STRUCT ));

    if ( !dme )
    {
	printf("[b/t/l] dme_3min_attach:  Unable to allocate memory for dme structure.\n" );
	return CAM_REQ_CMP_ERR;
    }

    sc->dme = dme;

    /*
     * Calculate the base address for the IOASIC.  To get the base address
     * the BASE_IOASIC_OFFSET has to be *SUBTRACTED* from the base address
     * of the 94 chip.  The 94 chip is stored in the SIM_SOFTC stucture.
     */

    dme->hardware = (void *)((unsigned int)sc->reg - BASE_IOASIC_OFFSET);

    dme->vector.dme_init = dme_3min_init;
    dme->vector.dme_setup = dme_3min_setup;
    dme->vector.dme_start = dme_3min_resume;
    dme->vector.dme_end = dme_3min_end;
    dme->vector.dme_pause = dme_3min_pause;
    dme->vector.dme_resume = dme_3min_resume;
    dme->vector.dme_save = dme_3min_save;
    dme->vector.dme_restore = dme_3min_restore;
    dme->vector.dme_modify = dme_3min_modify;
    dme->vector.dme_copyin = bcopy;
    dme->vector.dme_copyout = bcopy;
    dme->vector.dme_clear = bzero;
    dme->vector.dme_interrupt = dme_3min_interrupt;

    if ( dme_3min_init( sc ) != CAM_REQ_CMP )
        return CAM_REQ_CMP_ERR;

    return CAM_REQ_CMP;
}                  /* dme_attach */

U32
dma94_dme_unload()
    {
    return CAM_REQ_CMP;
    }

/**
 * dme_modify -
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * FORMAL PARAMETERS:           NONE
 *
 * IMPLICIT INPUTS:             NONE
 *
 * IMPLICIT OUTPUTS:            NONE
 *
 * RETURN VALUE:                TBD
 *
 * SIDE EFFECTS:                NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
dme_3min_modify ( DME_DESCRIPTOR *dme_desc, I32 offset )
  {
    U32 retval=CAM_REQ_CMP;         /* Return value                       */
    DME_3MIN_STRUCT *ldme;             /* DME specific data struct           */
    DME_3MIN_TABLE *xt, *te;           /* Our transfer descriptor table      */
    SIM_SOFTC* sc;                     /* Pointer to SIM S/W control struct  */
    SIM_WS *sim_wset;                  /* SIM working set for this request   */
    int controller;                    /* Controller of current request      */
    int target;                        /* Target of current request          */
    int lun;                           /* LUN of current request             */

    PRINTD(controller, target, lun, CAMD_DMA_FLOW,
           ("[%d/%d/%d] (dme_3min_modify) Not fully supported\n",
	    controller, target, lun ));
    
    /* If a bad pointer is passed in, return an error. */
    if ( !dme_desc || !dme_desc->segment || !dme_desc->sim_ws )
	return CAM_REQ_CMP_ERR;

    /* initialize local variables */
    sim_wset = dme_desc->sim_ws;
    controller = sim_wset->cntlr;
    target = sim_wset->targid;
    lun = sim_wset->lun;
    sc = GET_SOFTC_DESC( dme_desc );

    ldme = (DME_3MIN_STRUCT *) dme_desc->segment;
    if ( ldme == (DME_3MIN_STRUCT *) CAM_REQ_CMP_ERR )
      {
	return (U32) CAM_REQ_CMP_ERR;
      }

    xt = (DME_3MIN_TABLE *) ldme->frag_table;
    te = (DME_3MIN_TABLE *) &xt[ldme->frag_index];

    /* Backup pending count 
       fix this later
       
       (I32) (segbuf_ptr->act_pend_cnt)+= (I32) offset;
       (I32) (segbuf_ptr->act_ram_ptr) -= (I32) offset;
       (I32) (segbuf_ptr->act_buf_ptr) -= (I32) offset;
       */
    return retval;
}                       /* dme_modify */

dumpent( DME_3MIN_TABLE *tent )
    {
    printf( "  len=%d addr=0x%x uadr=0x%x iadr=0x%x, ", 
    tent->length, tent->addr, tent->uadr, tent->iadr );
    if ( !tent->completed )
        printf( "not ");
    printf( "completed, " );
    if ( tent->dir == IOASIC_READ )
        printf( "read\n" );
    else if ( tent->dir == IOASIC_WRITE )
        printf( "write\n" );
    else if ( tent->dir == IOASIC_UNDEF )
        printf( "undefined\n" );
    else
        printf( "type 0x%x\n", tent->dir );
    return tent->length;
    }
 
void
dumptbl( DME_3MIN_TABLE *tent )
    {
    int k;
    int so = 0;

    printf( "Table dump\n" );
    for( k = 0; k < MAX_TABLE_ENTRIES; k++ )
        {
        printf( " index:%d  ", k );

        if ( !dumpent( &(tent[k]) ) && so )
            k = MAX_TABLE_ENTRIES;
        else
            so = 1;
        }
    }

int
build_table_entry( SIM_WS *sws, DME_3MIN_TABLE *tent, U32 ecount, 
		  char *addr, char *uadr, int dir )
    {
    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_DMA_FLOW,
	   ("[b/t/l] (build_table_entry) Entry - tent=0x%x, ecount=%d, addr=0x%x, uadr=0x%x, dir=%d\n", 
	    tent, ecount, addr, uadr, dir ) );

    if ( !tent )
      {
	printf( "build_table_entry: Null tent pointer.\n" );
	return CAM_REQ_CMP_ERR;
      }

    tent->length = ecount;
    tent->addr = addr;
    tent->uadr = uadr;
    tent->completed = 0;

    if ( dir & DME_READ )
        tent->dir = IOASIC_READ;
    else if ( dir & DME_WRITE )
        tent->dir = IOASIC_WRITE;
    else 
        tent->dir = IOASIC_UNDEF;

    if ( addr )
        tent->iadr = (unsigned *) ioa_addrcvt( sws, tent->addr );
    else
        tent->iadr = 0;

    return CAM_REQ_CMP;
    }

/*  build_frag_table - Builds a table of DMA buffers suitable for use with
		the 3min.  Users buffer address is broken up both at
		page boundaries and, when buffer areas are less than 8
		bytes in length, fixed buffer areas allocated by the
		driver are used instead.

    Inputs:
                SIM_WS *sws            - our context
                DME_3MIN_STRUCT *ldme  - and our transfer table

    Return:	CAM_REQ_CMP_ERR on failure
*/

int
build_frag_table( SIM_WS *sws, DME_3MIN_STRUCT *ldme )
    {
    DME_3MIN_TABLE *table = ldme->frag_table;
    char *frag = (char *) ldme->frag_buffer;
    int index = 0;
    unsigned ecount;
    int dir = ldme->direction;
    char *eaddr, *uadr;
    U32 count = ldme->xfer_size - ldme->xfer_current_count;
    char *baddr, *save_addr, *addr;
    struct buf *bp;			/* Pointer to the users BP           */
    struct proc *process;		/* Process structure                */

    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_DMA_FLOW,
        ("[b/t/l] (build_frag_table) Entry - ldme=0x%x\n", ldme ) );
    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_DMA_FLOW,
        ("          table=0x%x, frag=0x%x, dir=0x%x, count=%d\n",
	 table, frag, dir, count ) );
    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_DMA_FLOW,
        ("          xfer_size=%d, xfer_current_count=%d\n", 
	 ldme->xfer_size, ldme->xfer_current_count ) );

    baddr = (char *) ldme->user_buffer;
    save_addr = addr = (char *) &baddr[ldme->xfer_current_count];

    bp = GET_BP_SIMWS(sws); /* Get the Buffer Struct Addr  */

    while( count && index < (MAX_TABLE_ENTRIES - 1))
        {
        if ( CAM_IS_KUSEG( save_addr ) && count )
        {
    	    if ( !bp )
    	    {
    	        printf("build_frag_table: Mrs. Fletcher has fallen!\n" );
    	        return CAM_REQ_CMP_ERR;
    	    }
            addr = (void *)PHYS_TO_K0(pmap_extract(bp->b_proc->task->map->vm_pmap, save_addr ));

            }

        if ( !(ecount = calc_table_entry( addr, count )) )
            {
            printf( "build_frag_table: Illegal return value (0) from caldatent().\n" );
            return CAM_REQ_CMP_ERR;
            }
        if ( ecount < 8 )
            {
            eaddr = frag;
	    if ( CAM_IS_KUSEG( addr ) )
		{
        	uadr = (void *)PHYS_TO_K0(pmap_extract(bp->b_proc->task->map->vm_pmap, addr ));

                }
	    else
                uadr = addr;
            }
	else
            {
            eaddr = addr;
            uadr = 0;
            }
	if ( build_table_entry( sws, &(table[index++]), ecount, eaddr, 
			       uadr, dir ) == CAM_REQ_CMP_ERR ) 
	    {
            printstate |= PANICPRINT;
	    printf("eaddr=0x%x ecount=%d\n ", eaddr, ecount);
	    dumptbl( table );
            panic("blddatbl:  Invalid IOASIC physical address .\n");
	    }
        count -= ecount;
        save_addr += ecount;
        addr += ecount;
        }
    if ( index == (MAX_TABLE_ENTRIES - 1))     /* xfer too big for table? */
    {
	if ( build_table_entry( sws, &table[index++], count, addr, NULL, dir )
	    == CAM_REQ_CMP_ERR )
	    return CAM_REQ_CMP_ERR;
    }
    else
    if ( build_table_entry( sws, &table[index++], NULL, NULL, NULL, NULL )
        == CAM_REQ_CMP_ERR )
        return CAM_REQ_CMP_ERR;

    ldme->frag_index = 0;
    return CAM_REQ_CMP;
    }

/* rps - original 4K page params - should use other kernel defines */
#define STRPW  0xfffffff8
#define PAGS   0x00001000
#define SEGMSK 0x00000fff
#define	FRAGSZ 0x00000008
#define FRAGM  0x00000007

int 
calc_table_entry( char *addr, U32 count )
    {
    U32 rem, rem2;
    U32 ecount;

    rem = ((U32)addr) & FRAGM;
    rem2 = ((U32)addr) & SEGMSK;
    if ( count < FRAGSZ )
        ecount = count;
    else if ( rem ) /* not octabyte aligned? */
        ecount = FRAGSZ - rem;
    else if ( rem2 )	/* not page aligned? */
        {
        ecount = PAGS - rem2;
        if ( count < ecount )	
            ecount = ( ( ((U32)addr) + count ) & STRPW ) 
                - ((U32)addr);
        }
    else				/* a 4k page */
        {
        ecount = PAGS;
        if ( count < ecount )
            ecount = ( ( ((U32)addr) + count ) & STRPW ) 
                - ((U32)addr);
        }
    return ecount;
    }


flush_fragment_buffer( SIM_WS *sws, DME_3MIN_STRUCT *ldme, 
		      U32 lcount )
    {
    DME_3MIN_TABLE *datp, *datep; /* pointer to table and individual entry */
    struct buf *bp;			/* Pointer to the users BP          */
    struct proc *process;		/* Process structure                */
    char *sva;				/* S0 VA created to map buffer      */
    
    /*
     * Setup required local variables.
     */

    if ( lcount > 7 )
      {
        printf("flush_fragbuf: ldme=0x%x, lcount=%d\n", ldme, lcount );
	return;
      }

  /* First pull out the table entry corresponding to this xfer */
    datp = (DME_3MIN_TABLE *)ldme->frag_table;
    datep = (DME_3MIN_TABLE *)&(datp[ldme->frag_index]);

    if( lcount > 0 )	/* don't bother if no data */
      {
	if ( CAM_IS_KUSEG( datep->uadr ) )
	  {
	    bp = GET_BP_SIMWS(sws); /* Get the Buffer Struct Addr  */
            sva = (void *)PHYS_TO_K0(pmap_extract(bp->b_proc->task->map->vm_pmap, datep->uadr ));

	    bcopy( datep->addr, sva, lcount );
	}
	else
	{
	    sva = datep->uadr;
	    bcopy( datep->addr, sva, lcount );
	}

        datep->length -= lcount;	/* adjust DAT entry length */  
        datep->uadr += lcount;
        if ( datep->length == 0 )
	  {
            datep->completed = DME_3MIN_TABLE_COMPLETE;
            ldme->frag_index++;
	  }
	wbflush();
        }
    }

int
dma_pointer_load( SIM_SOFTC *sc, unsigned int *addr )
    {
    unsigned *dmap;         /* pointer to IOASIC DMA Ptr. reg. */
    U32 *ioasicp;	/* pointer for the SCSI DMA engine register */

    PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	       ("[b/t/l] (dmapload) - Entry\n"));

    ioasicp = (U32 *) sc->dmaeng;	/* engine address */

    dmap = (unsigned *) ( ( (unsigned) ioasicp ) + IOA_S_DMAP_O );

    *dmap = (unsigned) addr;
    wbflush();
    return CAM_REQ_CMP;
    }

void
ssr_dma_on( SIM_SOFTC *sc, int dir )
    {
    unsigned *ssrp;          /* pointer to IOASIC SSR */
    U32 *ioasicp;	/* pointer for the SCSI DMA engine register */

    PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	       ("[b/t/l] (ssr_dma_on) -  Entry\n"));

    ioasicp = (U32 *)sc->dmaeng;	/* engine address */

    ssrp = (unsigned *)((unsigned)ioasicp + SSR_O);

    if ( dir == IOASIC_READ )		/* read */
	*ssrp |= ( SSR_DMADIR | SSR_DMAENB );
    else if ( dir == IOASIC_WRITE )
        {
        *ssrp &= ( ~SSR_DMADIR );
        *ssrp |= SSR_DMAENB;
        }
    wbflush();
    }

void
ssr_dma_off( SIM_SOFTC *sc )
    {
    volatile unsigned *ssrp;          /* pointer to IOASIC SSR */
    U32 *ioasicp;	/* pointer for the SCSI DMA engine register */

    PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	       ("[b/t/l] (ssr_dma_off) -  Entry\n"));

    ioasicp = (U32 *)sc->dmaeng;	/* engine address */

    ssrp = (unsigned *)((unsigned)ioasicp + SSR_O);
    *ssrp &= ( ~SSR_DMAENB );
    wbflush();
    }

void
dumphex( char *ptr, unsigned int len )
    {
    int i,j,index;

    printf("\nDump of 0x%x, length %d\n\n", ptr, len );
    for( i=0; i<len; i+=16 )
        {
        printf("  %05x: ", i );
        for( j=0; j<16; j++ )
            {
            index = i+j;
            if (index >= len )
                break;
            printf( "%2x ", ptr[index] );
            }
        printf( "\n" );
        }
    }

/* ------------------------------------------------------------------------ */
/*	getdbuffer( sc, bufp )		Copies the IOASIC data buffers into */
/*					a user buffer.  User buffer must be */
/*					at least 8 bytes long!              */
/*	Inputs:                                                             */
/*	    sc				Pointer to soft_c structure         */
/*	    bufp			Pointer to user buffer.             */
/*                                                                          */
/*	Return value:			Number of valid data bytes or -1 on */
/*					error.                              */
/* ------------------------------------------------------------------------ */

int 
getdbuffer( SIM_SOFTC *sc, void *bufp )
    {
    U32 *dbufp, *ubufp;
    volatile unsigned int *cregp, creg;
    U32 *ioasicp = (U32 *)sc->dmaeng; 
    /* pointer for DMA engine register */

    if ( !bufp )			/* check for invalid user address */
	return -1;

    ubufp = (U32 *)bufp;	/* address user buffer using 32's */

    dbufp = (U32 *)((unsigned)ioasicp + SCSI_DATA0_O );
    *ubufp++ = *dbufp;			/* Copy word 1 */

    dbufp = (U32 *)((unsigned)ioasicp + SCSI_DATA1_O );
    *ubufp = *dbufp;			/* Copy word 2 */

    cregp = (unsigned int *)((unsigned)ioasicp + SCSI_CTRL_O );
    creg = *cregp;			/* grab the scsi control register */

    if ( creg & CREG_DMA_M )		/* DMA operation in progress? */
	return 0;			/* nothing to read if a write */

    creg &= CREG_BUSG_M;		/* mask of to byte usage count */

    return creg<<1;			/* multiply hword's to get bytes */
    }

/* ------------------------------------------------------------------------ */
/*	getdbcount( sc, dir )		Returns the number of bytes filled  */
/*					in the IOASIC data buffers.         */
/*	Inputs:                                                             */
/*	    sc				Pointer to soft_c structure         */
/*	    dir				IO Direction (IOASIC_READ or _WRITE)*/
/*                                                                          */
/*	Return value:			Number of valid data bytes or -1 on */
/*					error.                              */
/* ------------------------------------------------------------------------ */

int 
get_ioasic_count( SIM_SOFTC *sc, int dir )
    {
    volatile unsigned int *cregp, creg;		/* Control pointer and content.    */
    U32 *ioasicp=(U32 *)sc->dmaeng;	/* SCSI DMA engine register        */
    int bcnt;				

    cregp = (unsigned int *)((unsigned)ioasicp + SCSI_CTRL_O );
    creg = *cregp;			/* Grab the scsi control register. */

    bcnt = creg & CREG_BUSG_M;		/* Mask off to byte usage count.   */

    if ( dir == IOASIC_WRITE )		/* During writes, the sense of the */
        {				/* DMA bit is inverted.            */
        if ( creg & CREG_DMA_M )	/* Buffer not empty?               */
	    return (4-bcnt)<<1;		/* IOASIC counts in half-words (16)*/
	return 0;			/* bcnt is invalid                 */
        }				/* We've handled all WRITE cases.  */

    if ( creg & CREG_DMA_M )		/* DMA in progress?                */
        return 0;

    return bcnt<<1;			/* Multiply hword's to get bytes.  */
    }

void
set_ioasic_control( SIM_SOFTC *sc, int val )
    {
    volatile unsigned int *cregp;		/* Control pointer and content.    */
    U32 *ioasicp= (U32 *)sc->dmaeng;	/* SCSI DMA engine register        */

    ssr_dma_off( sc );			/* JIC, turn DMA OFF */

    cregp = (unsigned int *)((unsigned)ioasicp + SCSI_CTRL_O );
    *cregp = val;
    wbflush();
    }

int 
flush_ioasic( SIM_SOFTC *sc, int cnt )
    {
    U32 *ioasicp=(U32 *)sc->dmaeng;	/* SCSI DMA engine register        */
    unsigned int *dmap;	                /* pointer to IOASIC DMA Ptr. reg. */
    char *padr;                         /* physical address of user buffer */
    char lbuf[32];			/* Local data buffer               */
    int bcnt;

    dmap = (unsigned int *) ( (unsigned) ioasicp + IOA_S_DMAP_O );

    padr = (char *) backcvt( (void *)*dmap ); /* Calc. the user buf address */

    bcnt = getdbuffer( sc, lbuf );      /* grab data bytes *//*rps 11/01/91*/

    bcopy( lbuf, padr, cnt );

    return cnt;
    }

/* ---------------------------------------------------------------------- */
/*
unsigned *
ioa_addrcvt( addr )

Inputs:
	char *addr;		 K2SEG address for the data 

Function:
 	Convert input virtual address to physical which, in turn, must
        be modified to meet IOASIC format requirements.
Return:
	Physical memory address in IOASIC format.
*/

unsigned *
ioa_addrcvt ( SIM_WS *sws, char *addr )
    {
    U32 a;
    U32 p;
    struct buf *bp;			/* Pointer to the users BP          */
    struct proc *process;		/* Process structure                */
    
    /*
     * Setup required local variables.
     */
    if ( CAM_IS_KUSEG( addr ) )
      {
	bp = GET_BP_SIMWS(sws); /* Get the Buffer Struct Addr  */
	if ( !bp )         /* this should never happen!!! */
	{
	    printf("ioa_addrcvt: Mrs. Fletcher:  I've fallen and I can't get up!!!\n" );
	}
        p = pmap_extract( bp->b_proc->task->map->vm_pmap, addr );

      }
    else
      {
	svatophys( addr, &p );
      }
    a = ( p & 0x1ffffffc ) << 3; 
    return (unsigned *) a;
  }

/* ---------------------------------------------------------------------- */
/*
void *backcvt( addr )

Inputs:
	void *addr;		 IOASIC format physical address 

Function:
        Convert an IOASIC format physical address into normal physical
        address format.

Return:
	Physical memory address in standard format.
*/

void *backcvt( void *addr )
    {
    U32 a = (U32) addr;
    U32 p;
    p = a >> 3;
    return (void *)( PHYS_TO_K0( p ));
    }

/**
 * dme_3min_bump_sglist -
 *
 * FUNCTIONAL DESCRIPTION:
 * This function is called by the DME to begin processing of the next scatter
 * gather entry in the scatter gather list.  This function updates the ldme
 * scatter gather information.
 *
 * FORMAL PARAMETERS:           NONE
 *
 *      dme_desc        -  DME request descriptor
 *      ldme          -  Local pointer transfer structure
 *
 * IMPLICIT INPUTS:             NONE
 *
 * IMPLICIT OUTPUTS:            ldme->init_user_buffer
 *                              ldme->init_xfer_size
 *
 * RETURN VALUE:                TBD
 *
 * SIDE EFFECTS:                NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
dme_3min_bump_sglist( DME_DESCRIPTOR *dme_desc, DME_3MIN_STRUCT *ldme )
  {
    U32 retval=CAM_REQ_CMP;          /* Return value */
    SIM_WS *sim_wset;                     /* Local sim_ws pointer */
    
    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_DMA_FLOW,
           ("[b/t/l] (dme_3min_bump_sglist): entered dme_bump_sglist\n"));       
    
    sim_wset = ((SIM_WS *)dme_desc->sim_ws);      /* Get working set */
    
    /*
     * First check to see whether there are any more scatter gather
     * elements to process.
     */
    if (ldme->sg.element_cnt < sim_wset->ccb->cam_sglist_cnt)
      {
        /* Incr count of SG ELEM'*/
        ldme->sg.element_cnt++;

        /* Get ptr to next SG_ELEM */
        ldme->sg.element_ptr++;
        
        /*
         * Now clear count of bytes within segment transfered.
         */
        ldme->sg.xfer_cnt = 0;

        /*
         * Get pointer to the buffer described by the first scatter gather
         * element.
         */
        ldme->user_buffer = (void *)ldme->sg.element_ptr->cam_sg_address;
        
        /*
         * Start this element off with the transfer counter for this 
         * sg element.
         */
        ldme->xfer_size = ldme->sg.element_ptr->cam_sg_count;
	    
	ldme->xfer_current_count = 0;    /* running count of bytes xfer'd */
    }
    else                /* No more scatter gather elements in list */
    {
        PRINTD( NOBTL, NOBTL, NOBTL, CAMD_DMA_FLOW,
               ("[b/t/l] (dme_3min_bump_sglist) no more sglist elements tp process\n"));
        return CAM_DATA_RUN_ERR;
    }
    
    return retval;
}                              /* dme_3min_bump_sglist */

