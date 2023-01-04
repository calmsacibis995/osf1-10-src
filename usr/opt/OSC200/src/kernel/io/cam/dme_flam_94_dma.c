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
static char *rcsid = "@(#)$RCSfile: dme_flam_94_dma.c,v $ $Revision: 1.1.4.8 $ (DEC) $Date: 1992/09/30 15:53:35 $";
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
#include <sys/types.h>
#include <io/common/iotypes.h>
#include <io/common/devdriver.h>
#include <mach/vm_param.h>
#include <hal/cpuconf.h>

#include <sys/proc.h>
#include <sys/buf.h>
#include <kern/lock.h>
#include <io/cam/dec_cam.h>
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
#include <io/cam/dme_flam_94_dma.h>
#include <io/dec/tc/tc.h>

/* ---------------------------------------------------------------------- 
 *
 * Local defines:
 */
#define SIM_FLAM_CLK_SPEED	25	/* 25 MHz */

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

U32 dme_copyin();
U32 dme_copyout();
U32 dme_flam_init();
U32 dme_flam_setup();
U32 dme_flam_end();
U32 dme_flam_pause();
U32 dme_flam_resume();
U32 dme_flam_save();
U32 dme_flam_restore();
U32 dme_flam_modify();
U32 dme_flam_clear();
U32 dme_flam_bump_sglist();
U32 dme_flam_interrupt();

static int (*build_frag_table)();

void
flam_dumptbl( DME_FLAM_TABLE *tent );
static void ssr_dma_on();
static void ssr_dma_off();
static int tcds_rahead = 0;

extern int nCAMBUS;

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

DME_FLAM_TABLE *
new_dme_flam_table( void )
    {
    DME_FLAM_TABLE *ichi, *ni;
    int ika;

    ichi = (DME_FLAM_TABLE *)sc_alloc(MAX_TABLE_SIZE);
    if ( !ichi )
      {
        printf("new_dme_flam_table: Error allocating memory for table.\n" );
        return (DME_FLAM_TABLE *) CAM_REQ_CMP_ERR;
      }

    for( ika=0; ika<MAX_TABLE_ENTRIES; ika++ )
        {
        ni = (DME_FLAM_TABLE *) &ichi[ika];
        ni->length = 0;
        ni->addr = 0;
        ni->uadr = 0;
        ni->iadr = 0xdeadbeef;
	ni->base = 0;
        ni->completed = DME_FLAM_TABLE_INCOMPLETE;
        ni->dir = IOASIC_UNDEF;
        }
    return ichi;
    }

void
init_dme_flam_struct( DME_FLAM_STRUCT *d )
    {
    d->frag_buffer = (FRAGBUF *) NULL;
    d->frag_table = (DME_FLAM_TABLE *) NULL;
    d->sim_ws = NULL;
    d->xfer_size = 0;
    d->xfer_current_count = 0;
    d->state = DME_STATE_UNDEFINED;
    }

/**
 * dme_flam_init -
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
dme_flam_init( SIM_SOFTC *sc )
{
    int *slotp;                 /* IOASIC SCSI DMA slot register pointer */
    DME_FLAM_STRUCT *ldme;      /* private DME struct pointer */
    int i;                      /* private counter */
    long has_iomaps = tc_map_alloc(NBPG, 0);
    
    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
           ("[b/t/l] (dme_flam_init) Entry - sc=0x%x\n", sc ) );

    if ( sc->simd_init )
    {
	PRINTD(NOBTL, NOBTL, NOBTL, CAMD_DMA_FLOW,
	       ("[b/t/l] (dme_flam_init): Duplicate init call\n" ) );
	return CAM_REQ_CMP_ERR;
    }
    
    ldme = (DME_FLAM_STRUCT *)sc_alloc(MAX_UNIT_COUNT*sizeof(DME_FLAM_STRUCT));
    if ( !ldme )
    {
	printf("dme_flam_init: Couldn't allocate memory for DME_FLAM_STRUCT\n" );
        return CAM_REQ_CMP_ERR;
    }
    
    fragbase = (FRAGBUF *)sc_alloc(max(4096,sizeof(FRAGBUF)*MAX_UNIT_COUNT));

    if ( !fragbase )
    {
	printf("dme_flam_init: Error allocating memory for fragment buffers.\n" );
	return CAM_REQ_CMP_ERR;
    } 

#define TCIO_SGMAP 0x1

    if (has_iomaps != -1) {
	tc_map_free(has_iomaps, NBPG, 0);
	build_frag_table = build_flam_map_frag_table;
        kn15aa_set_ioslot(tc_slot[(int)((struct controller *)sc->um_probe)->tcindx].slot, TCIO_SGMAP);
	tcds_rahead = 0x40;
    } else {
	build_frag_table = build_flam_frag_table;
	tcds_rahead = 0;
    }

    sc->dme->extension = (void *) ldme;   /* fill in the DME specific field */
    
    for( i=0; i<MAX_UNIT_COUNT; i++ )
        init_dme_flam_struct( &ldme[i] );
    
    sc->dmaeng = sc->dme->hardware;   /* prep alternate pointer to hardware */

    /* tscsiconf hides the channel number in flags; we hide it
     * in rambuf instead of taking another indirection through
     * the um_probe field of the softc
     */
    sc->rambuf = (void *)((struct controller *)sc->um_probe)->flags;
    
    sc->simd_init = 1;
    
    return CAM_REQ_CMP;
}

/**
 * dme_flam_setup -
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
dme_flam_setup( SIM_WS *sim_wset, U32 count, void *buf, U32 dir,
    DME_DESCRIPTOR *dme_desc )
    {
    SIM_SOFTC *sc;
    DME_FLAM_STRUCT *ldme, *dme;
    int controller = sim_wset->cntlr;
    int target = sim_wset->targid;
    int lun = sim_wset->lun;
    int ent = controller*MAX_CONTROLLER+target*MAX_TARGET+lun;
    SIM_WS *savews = dme_desc->sim_ws;

    PRINTD( controller, target, lun, CAMD_DMA_FLOW,
        ("[%d/%d/%d] (dme_flam_setup) Entry - sws=0x%x, count=%d,\n",
	 controller, target, lun, sim_wset, count ) );
    PRINTD( controller, target, lun, CAMD_DMA_FLOW,
        ("    buf=0x%x, dir=%d, dme_desc=0x%x\n", buf, dir, dme_desc ) );

    if ( !sim_wset || !dme_desc )
	return CAM_REQ_CMP_ERR;

    dme_desc->sim_ws = (void *)sim_wset;  /* Get a copy for later use        */
    sc = GET_SOFTC_DESC( dme_desc );
    dme = (DME_FLAM_STRUCT *) sc->dme->extension;
    ldme = (DME_FLAM_STRUCT *) &dme[ent];

    if (ldme->state != DME_STATE_END && ldme->state != DME_STATE_UNDEFINED) {
        dme_desc->sim_ws = savews;
        return CAM_REQ_CMP_ERR;
    }

    /*
     * If a DMA has previously been setup for this descriptor then simply
     * return to the caller. This will allow the HBA to simply reschedule
     * pending I/O requests without regard for the state of the dme.
     * Also, just return success for ZERO length requests.
     */
    if ((dme_desc->segment != NULL) || (count == 0))
        return(CAM_REQ_CMP);

    /*
     * Assign SIM_WS pointer to the passed DME_DESCRIPTOR.
     * Setup the data count, data ptr, and direction in the
     * DME_DESCRIPTOR.
     */
    dme_desc->data_count = count;       /* How big is the entire request?  */
    dme_desc->data_ptr = buf;           /* User buffer address S0 or P0    */
    dme_desc->flags |= dir;             /* Determine whether to write/read */

    ldme->state = DME_STATE_SETUP;

    if ( ldme->frag_table == NULL )            /* Has it been used yet? */
      {
        /* Allocate a transfer table for this target */
        ldme->frag_table = new_dme_flam_table();
        if ( ldme->frag_table == (DME_FLAM_TABLE *) CAM_REQ_CMP_ERR )
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

    if ( (*build_frag_table)( sim_wset, ldme ) == CAM_REQ_CMP_ERR )
        {
	PRINTD( controller, target, lun, CAMD_DMA_FLOW,
            ("[%d/%d/%d] (dme_flam_setup) Failed building xfer table.\n",
	     controller, target, lun ) );
        printf("dme_flam_setup: (%d/%d/%d) Failed building transfer table.\n",
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
    dme_flam_save(dme_desc);

    return CAM_REQ_CMP;
    }                   /* dme_setup */

/**
 * dme_flam_resume -
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
 * ADDITIONAL INFORMATION:      This routine was previously dme_flam_start.
 *
 **/
U32
dme_flam_resume( DME_DESCRIPTOR* dme_desc )
    {
    int s;                              /* Used by DME lock                 */
    SIM_SOFTC* sc;                      /* Pointer to SIM S/W control struct*/
    U32 retval = CAM_REQ_CMP;        /* Return value                     */
    SIM94_REG *reg94;                   /* Address of 94 CSR's              */
    SIM_WS *sim_wset;                   /* SIM working set for this request */
    DME_FLAM_TABLE *xt, *te;            /* Our transfer descriptor table    */
    DME_FLAM_STRUCT *ldme;              /* DME specific data struct ptr.    */
    int controller = 0;                     /* Controller in this request       */
    int target;                         /* Target in this request           */
    int lun;                            /* LUN in this request              */

    PRINTD( controller, target, lun, CAMD_DMA_FLOW,
           ("[%d/%d/%d] (dme_flam_resume) Entry - dme_desc=0x%x\n", 
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

    ldme = (DME_FLAM_STRUCT *) dme_desc->segment;
    if ( ldme == (DME_FLAM_STRUCT *) CAM_REQ_CMP_ERR )
      {
	printf( "(dme_flam_resume) Invalid ldme pointer.\n" );
        return (U32) CAM_REQ_CMP_ERR;
      }

    ldme->state = DME_STATE_RESUME;         /* Last DME state                */

    PRINTD( controller, target, lun, CAMD_DMA_FLOW,
           ("[%d/%d/%d] (dme_flam_resume) - dme=0x%x\n", 
	    controller, target, lun, ldme ) );

    if ( ldme->frag_index == (MAX_TABLE_ENTRIES - 1) )
    {
	if ((*build_frag_table)( sim_wset, ldme ) == CAM_REQ_CMP_ERR )
        {
	    PRINTD( controller, target, lun, CAMD_DMA_FLOW,
		   ("[%d/%d/%d] (dme_flam_setup) Failed building xfer table.\n",
		    controller, target, lun ) );
	    printf("dme_flam_setup: (%d/%d/%d) Failed building transfer table.\n",
		   controller, target, lun );
	    return (U32) CAM_REQ_CMP_ERR;
	}
    }

    xt = (DME_FLAM_TABLE *) ldme->frag_table;
    te = (DME_FLAM_TABLE *) &xt[ldme->frag_index];

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
            SIM94_SEND_CMD( sim_wset, reg94,
			    SIM94_CMD_FFIFO, sim_poll_mode); /* do it */
		WBFLUSH();
            sim94_load_fifo( reg94, te->length, te->uadr );
            SIM94_SEND_CMD( sim_wset, reg94,
			    SIM94_CMD_XFERINFO, sim_poll_mode); /* do it */
		WBFLUSH();
	}
        else
	{                                /* otherwise, use DMA             */
            ssr_dma_off(sc);
            retval = dma_pointer_load( sc, te->iadr );
            if ( retval == CAM_REQ_CMP_ERR )
	    {                            /* problem loading pointer!       */
		PRINTD( controller, target, lun, CAMD_DMA_FLOW,
		       ( "[%d/%d/%d] (dme_flam_resume) - Fail return from dma_pointer_load\n",
			controller, target, lun ) );
		dumpent( te );
                UNLOCK_DME(s,sc);           /* release lock on hba           */
		return (U32) retval;
	    }
            else
	    {
                ssr_dma_on( sc, te->dir );  /* Enable DMA and direction      */
		reg94->sim94_tcmsb = ( ( te->length ) & 0xff00 ) >> 8;
		WBFLUSH();
		reg94->sim94_tclsb = ( te->length ) & 0x00ff;
		WBFLUSH();
                SIM94_SEND_CMD( sim_wset, reg94,
				SIM94_CMD_DMAXFER, sim_poll_mode ); /* do it */
                WBFLUSH();                  /* conserve water                */
              }
          }
      }
    
    UNLOCK_DME(s,sc);                       /* release lock on hba           */

    return (U32) retval;
  }                     /* dme_resume */

/**
 * dme_flam_pause -
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
dme_flam_pause( DME_DESCRIPTOR *dme_desc )
  {
    SIM_SOFTC* sc;                      /* Pointer to SIM S/W control struct */
    U32 retval = CAM_REQ_CMP;        /* Return value                      */
    SIM94_REG *reg94;                   /* Address of 94 CSR's               */
    SIM_WS *sim_wset;                   /* Working set pointer for this req. */
    SIM94_SOFTC *ssc;                   /* HBA specific pointer              */
    DME_FLAM_STRUCT *ldme;              /* DME specific pointer              */
    DME_FLAM_TABLE *xt, *te;            /* Our transfer descriptor table     */
    U32 fifo_residual;               /* Number of bytes left in FIFO      */
    U32 residual;                    /* Number of bytes left Transfer CNT */
    int ioasic_residual;             /* Number of bytes left in IOASIC    */
    U32 offset;
    U32 xfer_cnt;                    /* Number of bytes really transfered */
    int s;                              /* Save the current IPL              */
    int controller = 0;                     /* Controller in request             */
    int target;                         /* Target in request                 */
    int lun;                            /* LUN in request                    */
    struct buf *bp;			/* Pointer to the users BP           */
    struct proc *process;		/* Process structure                 */
    char *sva;                          /* Temp. system virtual address      */
    u_char sr;                          /* '94 Status Register               */

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

    PRINTD( controller, target, lun, CAMD_DMA_FLOW,
           ("[%d/%d/%d] (dme_flam_pause) - Entry - dme_desc=0x%x\n", 
	    controller, target, lun, dme_desc ) );

    ldme = (DME_FLAM_STRUCT *) dme_desc->segment;
    if ( ldme == (DME_FLAM_STRUCT *) CAM_REQ_CMP_ERR )
      {
	printf("dme_flam_pause: Invalid ldme pointer.\n" );
	return (U32) CAM_REQ_CMP_ERR;
      }

    ldme->state = DME_STATE_PAUSE;              /* Last DME state           */

    xt = (DME_FLAM_TABLE *) ldme->frag_table;  
    te = (DME_FLAM_TABLE *) &xt[ldme->frag_index];

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
       ("[%d/%d/%d] (dme_flam_pause) - The fifo residual byte count was %d.\n",
	controller, target, lun, fifo_residual ) );

    residual = GET_94_COUNTER( reg94 );
    PRINTD(controller, target, lun, CAMD_DMA_FLOW,
       ("[%d/%d/%d] (dme_flam_pause) - The '94 residual byte count was %d.\n",
	controller, target, lun, residual));

    if ( sr & SIM94_STAT_TC )  /* timing problem? */
    {   /* RPS - This test/code should come out if the cprint never occurs */
	if ( residual )
	    printf("TC bit is set, residual is 0x%x, fifo_residual = 0x%x\n", 
		    residual, fifo_residual );
	residual = 0;
    }

    if ( te->length > 4 && residual > te->length ) 
    {   /* RPS - This test/code should come out if the cprint never occurs */
	printf("[%d/%d/%d] resid %d > te->length %d\n", 
		controller, target, lun, residual, te->length );
	residual = 0;
    }

    ioasic_residual = get_ioasic_count( sc, te->dir );
    PRINTD(controller, target, lun, CAMD_DMA_FLOW,
     ("[%d/%d/%d] (dme_flam_pause) - The ioasic residual byte count was %d.\n",
      controller, target, lun, ioasic_residual));

/* if one byte was transferred, then the ASIC reports two, but one
 * is subtracted from the count to correct this mistake.
 * if two bytes are transferred, no correction is necessary
 * if three bytes are transferred, the DMA engine moves four bytes
 * into the frag buffer, but xfer_count will have three bytes
 * we need to look at the chip residual too since the target *can*
 * disconnect after an odd number of bytes have been transferred
 */
    if ( ioasic_residual && ( (te->length & 1) || (residual & 1) )) /* if odd, then count is off */
    {                                           /* by 1. RPS 03/28/91        */
	ioasic_residual--;
    }

    if (te->base) {
	tc_map_free(te->base, te->length, 8);
	te->base = 0;
    }
    if ( te->dir == IOASIC_READ )
    {
	xfer_cnt = te->length - residual;

	ldme->xfer_current_count += xfer_cnt;    /* adjust transferred count */

	/* if the target disconnects on something other than a four-byte
	 * boundary, and the original request was for more than three bytes,
	 * then the offset handed into flush_ioasic must reflect the number
	 * of bytes moved by the ASC, minus the number currently stuck in the
	 * ASIC
	 */
	offset = (te->length > 3) ? xfer_cnt - ioasic_residual : 0;
        if (ioasic_residual )
            flush_ioasic( sc, te->addr, ioasic_residual, offset, sim_wset);
        if ( te->uadr )
	{
            flush_fragment_buffer( sim_wset, ldme, xfer_cnt );
	}
        else
	{ 
	    WBFLUSH();
            te->length -= xfer_cnt;           /* adjust segment length    */
            if ( te->length )                 /* not end of this segment? */
	    {                               /* then rebuild xfer table  */
    /* free map space for remaining fragments because they'll get allocated again in a moment */

                DME_FLAM_TABLE *mte;            /* Our transfer descriptor table     */
                int i;

                for (i = ldme->frag_index + 1, mte = &xt[i]; mte->base && i <= 255;
                    i++, mte = &xt[i]) {
                    tc_map_free(mte->base, mte->length, 8);
                    mte->base = 0;
                }

		if ((*build_frag_table)( sim_wset, ldme ) == CAM_REQ_CMP_ERR )
		{
		    PRINTD( controller, target, lun, CAMD_DMA_FLOW,
			   ("[%d/%d/%d] (dme_flam_pause) Failed building xfer table.\n",
			    controller, target, lun ) );
		    printf("dme_flam_pause: (%d/%d/%d) Failed building transfer table.\n",
			    controller, target, lun );
                    UNLOCK_DME(s,sc);      /* Get device lock          */
		    return (U32) CAM_REQ_CMP_ERR;
		}
	    }
	    else
	    {
                te->completed = DME_FLAM_TABLE_COMPLETE;
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

        SIM94_SEND_CMD( sim_wset, reg94,
			SIM94_CMD_FFIFO, sim_poll_mode ); /* do it */
	WBFLUSH();
        SIM94_SEND_CMD( sim_wset, reg94,
			SIM94_CMD_NOP, sim_poll_mode ); /* do it */
	WBFLUSH();

	ldme->xfer_current_count += xfer_cnt; /* adjust transferred count */

        te->length -= xfer_cnt;               /* adjust segment length    */
        if ( te->length )                    /* is this segment done?    */
	{
    /* free map space for remaining fragments because they'll get allocated again in a moment */

            DME_FLAM_TABLE *mte;            /* Our transfer descriptor table     */
            int i;

            for (i = ldme->frag_index + 1, mte = &xt[i]; mte->base && i <= 255;
                i++, mte = &xt[i]) {
                tc_map_free(mte->base, mte->length, 8);
                mte->base = 0;
            }

	    if ((*build_frag_table)( sim_wset, ldme ) == CAM_REQ_CMP_ERR )
	    {
		PRINTD( controller, target, lun, CAMD_DMA_FLOW,
		       ("[%d/%d/%d] (dme_flam_pause) Failed building xfer table.\n",
			controller, target, lun ) );
		printf("dme_flam_pause: (%d/%d/%d) Failed building transfer table.\n",
			controller, target, lun );
                UNLOCK_DME(s,sc);      /* Get device lock          */
		return (U32) CAM_REQ_CMP_ERR;
	    }
	}
	else
	{
            te->completed = DME_FLAM_TABLE_COMPLETE; /*mark this segment done*/
            ldme->frag_index++;                /* point to next segment    */
	}
    }

    dme_desc->xfer_count += xfer_cnt;/* Total bytes transfered */

    te = (DME_FLAM_TABLE *) &xt[ldme->frag_index];

    if (sim_wset->cam_flags & CAM_SCATTER_VALID && te->length == 0 )
    {
        ldme->sg.xfer_cnt = ldme->xfer_current_count;

	if ( dme_flam_bump_sglist( dme_desc, ldme ) == CAM_REQ_CMP_ERR )
	{
	    PRINTD( controller, target, lun, CAMD_DMA_FLOW,
		   ("[%d/%d/%d] (dme_flam_pause) Reached end of SG list.\n",
		    controller, target, lun ) );
	    
	}
	else
	{
	    if ((*build_frag_table)( sim_wset, ldme ) == CAM_REQ_CMP_ERR )
	    {
		PRINTD( controller, target, lun, CAMD_DMA_FLOW,
		       ("[%d/%d/%d] (dme_flam_pause) Failed building xfer table after SG bump.\n",
			controller, target, lun ) );
		printf("dme_flam_pause: (%d/%d/%d) Failed building transfer table after SG bump.\n",
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
 * dme_flam_save - Maybe this should be a macro!
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
dme_flam_save( DME_DESCRIPTOR *dme_desc )
  {
    U32 retval=CAM_REQ_CMP;         /* Return value                       */
    DME_FLAM_STRUCT *ldme;             /* DME specific data struct           */
    DME_FLAM_TABLE *xt, *te;           /* Our transfer descriptor table      */
    SIM_SOFTC* sc;                     /* Pointer to SIM S/W control struct  */
    SIM_WS *sim_wset;                  /* SIM working set for this request   */
    int controller;                    /* Controller of current request      */
    int target;                        /* Target of current request          */
    int lun;                           /* LUN of current request             */

    PRINTD(controller, target, lun, CAMD_DMA_FLOW,
           ("[%d/%d/%d] (dme_flam_save) Entry - dme_desc=0x%x\n", 
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

    ldme = (DME_FLAM_STRUCT *) dme_desc->segment;
    if ( ldme == (DME_FLAM_STRUCT *) CAM_REQ_CMP_ERR )
      {
	return (U32) CAM_REQ_CMP_ERR;
      }

    xt = (DME_FLAM_TABLE *) ldme->frag_table;
    te = (DME_FLAM_TABLE *) &xt[ldme->frag_index];

    return retval;
}                     /* dme_save */

/**
 * dme_flam_restore - Maybe this should be a macro!
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
dme_flam_restore( DME_DESCRIPTOR *dme_desc )
  {
    U32 retval=CAM_REQ_CMP;         /* Return value                       */
    DME_FLAM_STRUCT *ldme;             /* DME specific data struct           */
    DME_FLAM_TABLE *xt, *te;           /* Our transfer descriptor table      */
    SIM_SOFTC* sc;                     /* Pointer to Soft-c  control struct  */
    SIM_WS *sim_wset;                  /* SIM working set for this request   */
    int controller;                    /* Controller of current request      */
    int target;                        /* Target of current request          */
    int lun;                           /* LUN of current request             */

    PRINTD(controller, target, lun, CAMD_DMA_FLOW,
           ("[%d/%d/%d] (dme_flam_restore) Entry - dme_desc=0x%x\n", 
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

    ldme = (DME_FLAM_STRUCT *) dme_desc->segment;
    if ( ldme == (DME_FLAM_STRUCT *) CAM_REQ_CMP_ERR )
	return (U32) CAM_REQ_CMP_ERR;

    xt = (DME_FLAM_TABLE *) ldme->frag_table;
    te = (DME_FLAM_TABLE *) &xt[ldme->frag_index];

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
dme_flam_end( DME_DESCRIPTOR *dme_desc )
  {
    U32 retval=CAM_REQ_CMP;          /* Return value                      */
    DME_FLAM_STRUCT *ldme;              /* DME specific data struct          */
    SIM_WS *sim_wset;                   /* SIM working set for this request  */
    int controller;                     /* Controller of current request     */
    int target;                         /* Target of current request         */
    int lun;                            /* LUN of current request            */
    DME_FLAM_TABLE *xt, *te;
    int i;

    PRINTD(controller, target, lun, CAMD_DMA_FLOW,
           ("[b/t/l] (dme_flam_end) Entry - dme_desc=0x%x.\n", dme_desc ) );

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

    ldme = (DME_FLAM_STRUCT *) dme_desc->segment;
    if ( ldme == (DME_FLAM_STRUCT *) CAM_REQ_CMP_ERR )
	{
	printf( "(dme_flam_end) Invalid ldme pointer.\n" );
	return (U32) CAM_REQ_CMP_ERR;
	}

    if (build_frag_table == build_flam_map_frag_table) {
    xt = (DME_FLAM_TABLE *) ldme->frag_table;
    te = (DME_FLAM_TABLE *) &xt[ldme->frag_index];
        for (i = ldme->frag_index; te->base && i <= 255; ++i, te = &xt[i]) {
           tc_map_free(te->base, te->length, 8);
           te->base = 0;
        }
     }

    dme_desc->segment = 0;

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
dme_flam_interrupt( int controller )
    {
    SIM_SOFTC *sc;                   /* The control struct for subsystem     */
    volatile U32 *sir;              /* System Interrupt Register ptr.       */
    U32 sirp, resetval;    /* What was/will-be in SIR              */
    int retval;                      /* All important return value           */
    char *pa;                      /* physical address                     */
    unsigned map_offset, offset;
    tcmap_t *mapaddr;
    struct tc_memerr_status status;  /* memory error status struct           */
    volatile TCDS_DMA_COMMON *tcds;

    PRINTD( controller, NOBTL, NOBTL, CAMD_INOUT,
	       ("[%d/t/l] (dme_flam_interrupt) Entry\n", controller) );

    sc = (SIM_SOFTC *)SC_GET_SOFTC( controller );
    sir = (U32 *)sc->dmaeng;	/* engine address */

    resetval = 0xffff0000;
    sirp = *sir;
    resetval |= sirp;	/* preserve control information */
    retval = 1;

    if (sirp & SCSI_ERRMASK) {
            if (sirp & (SCSI0_EDMA | SCSI1_EDMA)) {  /* memory read error?    */
            tcds = &((TCDS_DMA *)sir)->dmaregs[(int)sc->rambuf];
            ssr_dma_off( sc );
            pa = (char *)backcvt(tcds->dma_addr.reg);
            if (build_frag_table == build_flam_map_frag_table) {
    	        map_offset = (unsigned long)pa >> PGSHIFT - 2;
    	        offset = (unsigned long)pa & PGOFSET;
    	        mapaddr = (tcmap_t *)(TC_MAP_PHYSADDR + map_offset);
    	        pa = (char *)((*mapaddr | offset) & 0x1fffff);
    	    }
            printf("scsi%d: dma memory error \n", controller );
            status.pa = pa;
            status.va = 0;
            status.log = TC_LOG_MEMERR;
            status.blocksize = 4;
            tc_isolate_memerr(&status);
        }
	if (sirp & (SCSI0_PAR | SCSI1_PAR)) {
	    printf("tcds%d: SCSI data bus parity error, channel %d\n", controller, (int)sc->rambuf);
	}
	if (sirp & (SCSI0_DMAPAR | SCSI1_DMAPAR)) {
	    printf("tcds%d: SCSI DMA buffer parity error, channel %d\n", controller, (int)sc->rambuf);
	}
	if (sirp & (SCSI0_TCPAR | SCSI1_TCPAR)) {
	    printf("tcds%d: Turbochannel DMA read data parity error, channel %d\n", controller, (int)sc->rambuf);
	}
	if (sirp & SCSITCWDPAR) {
	    printf("tcds%d: Turbochannel IO write data parity error, channel %d\n", controller, (int)sc->rambuf);
	}
	if (sirp & SCSITCADPAR) {
	    printf("tcds%d: Turbochannel IO address parity error, channel %d\n", controller, (int)sc->rambuf);
	}
        resetval &= ~SCSI_ERRMASK;
    }
    if (sirp & (SCSI0_PREF | SCSI1_PREF)) {
        printf("tcds%d: Unexpected prefetch interrupt, channel %d\n", controller, (int)sc->rambuf);
	resetval &= ~(SCSI0_PREF | SCSI1_PREF);
    }

    if (!(sc->rambuf) && (sirp & SCSI0_C94 ))
        {
        retval = 0;		       /* go through '94 code                */
        resetval &= ~SCSI0_C94;
        }
    if ((sc->rambuf) && (sirp & SCSI1_C94))
        {
        retval = 0;		       /* go through '94 code                */
        resetval &= ~SCSI1_C94;
        }

    *sir = resetval;
    WBFLUSH();
    return retval;
    }

/**
 * dme_flam_attach -
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * This function is called during the initialization sequence in the SIM
 * to configure the DME by loading the appropriate routines into the the
 * HBA_DME_CONTROL structure in the sim_softc. This function is provided 
 * to allow the Data Mover Engine to be configured based on the capabilities
 * of the underlying HBA. 
 *
 * Currently, this routine supports the FLAM DME.
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
dma94_flam_dme_attach( SIM_SOFTC *sc )
  {
    register volatile SIM94_REG *reg = (SIM94_REG *)sc->reg;
    register SIM94_SOFTC *ssc = (SIM94_SOFTC *)sc->hba_sc;
    DME_STRUCT *dme;

    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_DMA_FLOW,
           ("[b/t/l] (dme_flam_attach) entry - sc=0x%x\n", sc ));

    /*
     * Store the clock speed.  To be used when setting up the
     * synchronous period.  Store the clock * 10 to allow for
     * fractions.
     */
    ssc->clock = SIM_FLAM_CLK_SPEED * 10;

    /*
     * Set the clock conversion register for FLAM's speed.
     */
    reg->sim94_ccf = SIM94_CONVERT_CLOCK(SIM_FLAM_CLK_SPEED);
    WBFLUSH();

    /*
     * Calculate the minimum period.  The value which is used to
     * negotiate synchronous is the actual period divided by four.
     * Convert the clock speed from MHz to nanoseconds by dividing
     * 1000 by the clock speed.
     */
    sc->sync_period = ((1000 / SIM_FLAM_CLK_SPEED) * SIM94_PERIOD_MIN) / 4;

    dme = (DME_STRUCT *)sc_alloc(sizeof( DME_STRUCT ));

    if ( !dme )
    {
	printf("[b/t/l] dme_flam_attach:  Unable to allocate memory for dme structure.\n" );
	return CAM_REQ_CMP_ERR;
    }

    sc->dme = dme;

    /*
     * Calculate the base address for the IOASIC.  To get the base address
     * the BASE_IOASIC_OFFSET has to be *SUBTRACTED* from the base address
     * of the 94 chip.  The 94 chip is stored in the SIM_SOFTC stucture.
     */

    dme->hardware = (void *)&((struct tcds_asic *)sc->csr_probe)->cir;

    dme->vector.dme_init = dme_flam_init;
    dme->vector.dme_setup = dme_flam_setup;
    dme->vector.dme_start = dme_flam_resume;
    dme->vector.dme_end = dme_flam_end;
    dme->vector.dme_pause = dme_flam_pause;
    dme->vector.dme_resume = dme_flam_resume;
    dme->vector.dme_save = dme_flam_save;
    dme->vector.dme_restore = dme_flam_restore;
    dme->vector.dme_modify = dme_flam_modify;
    dme->vector.dme_copyin = bcopy;
    dme->vector.dme_copyout = bcopy;
    dme->vector.dme_clear = bzero;
    dme->vector.dme_interrupt = dme_flam_interrupt;

    if ( dme_flam_init( sc ) != CAM_REQ_CMP )
        return CAM_REQ_CMP_ERR;

    return CAM_REQ_CMP;
}                  /* dme_attach */

U32
dma94_flam_dme_unload()
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
dme_flam_modify ( DME_DESCRIPTOR *dme_desc, I32 offset )
  {
    U32 retval=CAM_REQ_CMP;         /* Return value                       */
    DME_FLAM_STRUCT *ldme;             /* DME specific data struct           */
    DME_FLAM_TABLE *xt, *te;           /* Our transfer descriptor table      */
    SIM_SOFTC* sc;                     /* Pointer to SIM S/W control struct  */
    SIM_WS *sim_wset;                  /* SIM working set for this request   */
    int controller;                    /* Controller of current request      */
    int target;                        /* Target of current request          */
    int lun;                           /* LUN of current request             */

    PRINTD(controller, target, lun, CAMD_DMA_FLOW,
           ("[%d/%d/%d] (dme_flam_modify) Not fully supported\n",
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

    ldme = (DME_FLAM_STRUCT *) dme_desc->segment;
    if ( ldme == (DME_FLAM_STRUCT *) CAM_REQ_CMP_ERR )
      {
	return (U32) CAM_REQ_CMP_ERR;
      }

    xt = (DME_FLAM_TABLE *) ldme->frag_table;
    te = (DME_FLAM_TABLE *) &xt[ldme->frag_index];

    /* Backup pending count 
       fix this later
       
       (I32) (segbuf_ptr->act_pend_cnt)+= (I32) offset;
       (I32) (segbuf_ptr->act_ram_ptr) -= (I32) offset;
       (I32) (segbuf_ptr->act_buf_ptr) -= (I32) offset;
       */
    return retval;
}                       /* dme_modify */

flam_dumpent( DME_FLAM_TABLE *tent )
    {
    printf( "  len=%d addr=0x%lx uadr=0x%lx iadr=0x%x, ",
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
    return (tent->dir == IOASIC_UNDEF);
    }

void
flam_dumptbl( DME_FLAM_TABLE *tent )
    {
    int k;
    int so = 0;

    printf( "Table dump\n" );
    for( k = 0; k < MAX_TABLE_ENTRIES; k++ )
        {
        printf( " index:%d  ", k );

        if ( flam_dumpent( &(tent[k]) ) )
            k = MAX_TABLE_ENTRIES;
        else
            so = 1;
        }
    }


static int
build_flam_table_entry( SIM_WS *sws, DME_FLAM_TABLE *tent, U32 ecount, 
		  char *addr, char *uadr, int dir )
    {
    struct buf *bp;
    struct proc *p;
    unsigned long phys;

    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_DMA_FLOW,
	   ("[b/t/l] (build_flam_table_entry) Entry - tent=0x%x, ecount=%d, addr=0x%x, uadr=0x%x, dir=%d\n", 
	    tent, ecount, addr, uadr, dir ) );

    if ( !tent )
      {
	printf( "build_flam_table_entry: Null tent pointer.\n" );
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
{
        tent->iadr = ioa_addrcvt( sws, tent->addr );
}
    else
	{
        tent->iadr = 0xdeadbeef;
	}

    return CAM_REQ_CMP;
    }

static int
build_flam_map_table_entry( SIM_WS *sws, DME_FLAM_TABLE *tent, U32 ecount, 
		  char *addr, char *uadr, int dir )
    {
    struct buf *bp;
    struct proc *p;
    unsigned long phys;

    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_DMA_FLOW,
	   ("[b/t/l] (build_flam_table_entry) Entry - tent=0x%x, ecount=%d, addr=0x%x, uadr=0x%x, dir=%d\n", 
	    tent, ecount, addr, uadr, dir ) );

    if ( !tent )
      {
	printf( "build_flam_table_entry: Null tent pointer.\n" );
	return CAM_REQ_CMP_ERR;
      }

    tent->length = ecount;
    tent->addr = addr;
    tent->uadr = uadr;
    tent->completed = 0;

    bp = GET_BP_SIMWS(sws);
    if (bp)
	p = bp->b_proc;
    else
	p = &proc[0];

    if ( dir & DME_READ )
        tent->dir = IOASIC_READ;
    else if ( dir & DME_WRITE )
        tent->dir = IOASIC_WRITE;
    else 
        tent->dir = IOASIC_UNDEF;

    if ( addr ) {
if (tent->base)
    printf("already have map 0x%lx\n", tent->base);
	if ((phys = tc_loadmap(p, &tent->base, addr, ecount, 8)) == -1) {
		printf("tc_loadmap failed: base == 0x%lx, addr == 0x%lx, count == %d\n",
			tent->base, addr, ecount);
		return (CAM_REQ_CMP_ERR);
	} else
		tent->iadr = phys >> 2;
    }
    else
	{
        tent->iadr = 0xdeadbeef;
	}

    return CAM_REQ_CMP;
    }

/*  build_flam_frag_table - Builds a table of DMA buffers suitable for use with
		the flam.  Users buffer address is broken up both at
		page boundaries and, when buffer areas are less than 8
		bytes in length, fixed buffer areas allocated by the
		driver are used instead.

    Inputs:
                SIM_WS *sws            - our context
                DME_FLAM_STRUCT *ldme  - and our transfer table

    Return:	CAM_REQ_CMP_ERR on failure
*/

int
build_flam_frag_table( SIM_WS *sws, DME_FLAM_STRUCT *ldme )
    {
    DME_FLAM_TABLE *table = ldme->frag_table;
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
        ("[b/t/l] (build_flam_frag_table) Entry - ldme=0x%x\n", ldme ) );
    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_DMA_FLOW,
        ("          table=0x%x, frag=0x%x, dir=0x%x, count=%d\n",
	 table, frag, dir, count ) );
    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_DMA_FLOW,
        ("          xfer_size=%d, xfer_current_count=%d\n", 
	 ldme->xfer_size, ldme->xfer_current_count ) );

    baddr = (char *) ldme->user_buffer;
    save_addr = addr = (char *) &baddr[ldme->xfer_current_count];

    bp = GET_BP_SIMWS(sws); /* Get the Buffer Struct Addr  */

    while (count && index<(MAX_TABLE_ENTRIES - 1))
        {
        if (CAM_IS_KUSEG(save_addr) && count)
            {
    	    if (!bp)
    	        {
    	        printf("build_flam_frag_table: Mrs. Fletcher has fallen!\n" );
    	        return CAM_REQ_CMP_ERR;
    	        }
            addr = (char *)CAM_PHYS_TO_KVA(pmap_extract(bp->b_proc->task->map->vm_pmap, save_addr ));

            }

        if (!(ecount = flam_calc_table_entry(addr, count)))
            {
            printf( "build_flam_frag_table: Illegal return value (0) from caldatent().\n" );
            return CAM_REQ_CMP_ERR;
            }
        if (ecount < 4)
            {
            eaddr = frag;
            if (CAM_IS_KUSEG(addr))
		{
        	uadr = (char *)CAM_PHYS_TO_KVA(pmap_extract(bp->b_proc->task->map->vm_pmap, addr ));

                }
	    else
                uadr = addr;
	    frag = (char *)((unsigned long)frag + sizeof (FRAGBUF) / 2);
            }
	else
            {
            eaddr = addr;
            uadr = 0;
            }
	if (build_flam_table_entry( sws, &table[index++], ecount, eaddr,
			       uadr, dir ) == CAM_REQ_CMP_ERR ) 
	    {
            printstate |= PANICPRINT;
	    printf("eaddr=0x%x ecount=%d\n ", eaddr, ecount);
	    flam_dumptbl( table );
            panic("blddatbl:  Invalid IOASIC physical address .\n");
	    }
        count -= ecount;
        save_addr += ecount;
        addr += ecount;
        }
    if ( index == (MAX_TABLE_ENTRIES - 1) )              /* xfer too big for table? */
    {
	if ( build_flam_table_entry( sws, &table[index++], count, addr, NULL, dir )
	    == CAM_REQ_CMP_ERR )
	    return CAM_REQ_CMP_ERR;
    }
    else if ( build_flam_table_entry( sws, &table[index++], 0, NULL, NULL, IOASIC_UNDEF )
	== CAM_REQ_CMP_ERR )
	return CAM_REQ_CMP_ERR;
    else
	;

    ldme->frag_index = 0;

    return CAM_REQ_CMP;
    }

int
build_flam_map_frag_table( SIM_WS *sws, DME_FLAM_STRUCT *ldme )
    {
    DME_FLAM_TABLE *table = ldme->frag_table;
    char *frag = (char *) ldme->frag_buffer;
    int index = 0;
    unsigned ecount = 0;
    int dir = ldme->direction;
    char *eaddr, *uadr;
    U32 count = ldme->xfer_size - ldme->xfer_current_count;
    char *baddr, *save_addr, *addr;
    struct buf *bp;			/* Pointer to the users BP           */
    struct proc *process;		/* Process structure                */

    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_DMA_FLOW,
        ("[b/t/l] (build_flam_map_frag_table) Entry - ldme=0x%x\n", ldme ) );
    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_DMA_FLOW,
        ("          table=0x%x, frag=0x%x, dir=0x%x, count=%d\n",
	 table, frag, dir, count ) );
    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_DMA_FLOW,
        ("          xfer_size=%d, xfer_current_count=%d\n", 
	 ldme->xfer_size, ldme->xfer_current_count ) );

    baddr = (char *) ldme->user_buffer;
    save_addr = addr = (char *) &baddr[ldme->xfer_current_count];

    bp = GET_BP_SIMWS(sws); /* Get the Buffer Struct Addr  */

    while (count && index<255)
        {
        addr = save_addr;

        if (!(ecount = flam_calc_map_table_entry(addr, count)))
            {
            printf( "build_flam_map_frag_table: Illegal return value (0) from caldatent().\n" );
            return CAM_REQ_CMP_ERR;
            }
        if (ecount < 4)
            {
            eaddr = frag;
            if (CAM_IS_KUSEG(addr))
		{
        	uadr = (char *)CAM_PHYS_TO_KVA(pmap_extract(bp->b_proc->task->map->vm_pmap, addr ));

                }
	    else
                uadr = addr;
	    frag = (char *)((unsigned long)frag + sizeof (FRAGBUF) / 2);
            }
	else
            {
            eaddr = addr;
            uadr = 0;
            }
	if (build_flam_map_table_entry( sws, &table[index++], ecount, eaddr, 
			       uadr, dir ) == CAM_REQ_CMP_ERR ) 
	    {
            printstate |= PANICPRINT;
	    printf("eaddr=0x%x ecount=%d\n ", eaddr, ecount);
	    flam_dumptbl( table );
            panic("blddatbl:  Invalid IOASIC physical address .\n");
	    }
        count -= ecount;
        save_addr += ecount;
        addr += ecount;
        }
    if ( index == 255 )              /* xfer too big for table? */
    {
	if (build_flam_map_table_entry( sws, &table[index++], count, addr, NULL, dir )
	    == CAM_REQ_CMP_ERR )
	    return CAM_REQ_CMP_ERR;
    }
    else if (build_flam_map_table_entry( sws, &table[index++], 0, NULL, NULL, IOASIC_UNDEF )
	== CAM_REQ_CMP_ERR )
	return CAM_REQ_CMP_ERR;
    else
	;

    ldme->frag_index = 0;

    return CAM_REQ_CMP;
    }

/* rps - original 4K page params - should use other kernel defines */
#define STRPW  0xfffffffffffffffcL
#define PAGS   0x00002000
#define SEGMSK 0x00001fff
#define FRAGSZ 0x00000004
#define FRAGM  0x00000003

static int 
flam_calc_table_entry( char *addr, U32 count )
    {
    unsigned long rem, rem2;
    unsigned long ecount;

    rem = ((unsigned long)addr) & FRAGM;
    rem2 = ((unsigned long)addr) & SEGMSK;
    if ( count < FRAGSZ )
        ecount = count;
    else if ( rem ) /* not octabyte aligned? */
        ecount = FRAGSZ - rem;
    else if ( rem2 )	/* not page aligned? */
        {
        ecount = PAGS - rem2;
        if ( count < ecount )	
            ecount = ( ( ((unsigned long)addr) + count ) & STRPW ) 
                - ((unsigned long)addr);
        }
    else				/* a 4k page */
        {
        ecount = PAGS;
        if ( count < ecount )
            ecount = ( ( ((unsigned long)addr) + count ) & STRPW ) 
                - ((unsigned long)addr);
        }
    return ecount;
    }

static int
flam_calc_map_table_entry( char *addr, U32 count )
    {
    unsigned long rem;
    unsigned long ecount;

    rem = ((unsigned long)addr) & FRAGM;
    if ( count < FRAGSZ )
        ecount = count;
    else if ( rem ) /* not octabyte aligned? */
        ecount = FRAGSZ - rem;
    else
        {
        ecount = 0x10000;       /* C94 can handle up to 64K at a time */
        if ( count < ecount )
            ecount = ( ( ((unsigned long)addr) + count ) & STRPW )
                - ((unsigned long)addr);
        }
    return ecount;
    }

static
flush_fragment_buffer( SIM_WS *sws, DME_FLAM_STRUCT *ldme, 
		      U32 lcount )
    {
    DME_FLAM_TABLE *datp, *datep; /* pointer to table and individual entry */
    struct buf *bp;			/* Pointer to the users BP          */
    struct proc *process;		/* Process structure                */
    char *sva;				/* S0 VA created to map buffer      */
    
    /*
     * Setup required local variables.
     */

    if ( lcount > 4 )
      {
        printf("flush_fragbuf: ldme=0x%x, lcount=%d\n", ldme, lcount );
	return;
      }

  /* First pull out the table entry corresponding to this xfer */
    datp = (DME_FLAM_TABLE *)ldme->frag_table;
    datep = (DME_FLAM_TABLE *)&(datp[ldme->frag_index]);

    if( lcount > 0 )	/* don't bother if no data */
      {
        if ( CAM_IS_KUSEG( datep->uadr ) )
	  {
	    bp = GET_BP_SIMWS(sws); /* Get the Buffer Struct Addr  */
            sva = (void *)CAM_PHYS_TO_KVA(pmap_extract(bp->b_proc->task->map->vm_pmap, datep->uadr ));
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
            datep->completed = DME_FLAM_TABLE_COMPLETE;
            ldme->frag_index++;
	  }
	WBFLUSH();
        }
else printf("nothing to flush?\n");
    }

static int
dma_pointer_load( SIM_SOFTC *sc, unsigned int addr )
    {
    volatile unsigned *dmap;         /* pointer to IOASIC DMA Ptr. reg. */

    PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	       ("[b/t/l] (dmapload) - Entry\n"));

    dmap = &((struct tcds_dma *)sc->dmaeng)->dmaregs[(int)sc->rambuf].dma_addr.reg;

    /* unaligned bytes ? */
    *dmap = addr;
    WBFLUSH();

    return CAM_REQ_CMP;
    }

static void
ssr_dma_on( SIM_SOFTC *sc, int dir )
    {
    volatile unsigned *ssrp;          /* pointer to IOASIC SSR */
    volatile U32 *xcir;

    PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	       ("[b/t/l] (ssr_dma_on) -  Entry\n"));

    xcir = (volatile unsigned *)sc->dmaeng; /* engine address */

    ssrp = (volatile unsigned *)&((struct tcds_dma *)sc->dmaeng)->dmaregs[(int)sc->rambuf].dma_dmic.reg;

    if ( dir == IOASIC_READ )		/* read */
	*ssrp = (SSR_DMADIR | tcds_rahead);
    else if ( dir == IOASIC_WRITE )
        {
        *ssrp = tcds_rahead;
        }
    WBFLUSH();
    if (sc->rambuf)
	*xcir |= SSR_DMA1ENB;
    else
        *xcir |= SSR_DMA0ENB;
    WBFLUSH();
    }

static void
ssr_dma_off( SIM_SOFTC *sc )
    {
    volatile U32 *xcir;

    PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	       ("[b/t/l] (ssr_dma_off) -  Entry\n"));

    xcir = sc->dmaeng;

    if (sc->rambuf)
        *xcir &= ~SSR_DMA1ENB;
    else
        *xcir &= ~SSR_DMA0ENB;
    WBFLUSH();
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

static bits_on[] = {0, 1, 1, 2, 1, 2, 2, 3};

static int 
get_ioasic_count( SIM_SOFTC *sc, int dir )
    {
    volatile TCDS_DMA_COMMON *tcds = &((TCDS_DMA *)sc->dmaeng)->dmaregs[(int)sc->rambuf];
    U32 left = tcds->dma_unaln0.reg;

    if (left & (7 << 1))
            printf("unaligned 0 == 0x%x\n", left);
    left = tcds->dma_unaln1.reg;
    return bits_on[left >> 24];
    }

static int 
flush_ioasic( SIM_SOFTC *sc, vm_offset_t addr, int count, int nxfer, SIM_WS *sim_ws )
    {
    volatile TCDS_DMA_COMMON *tcds = &((TCDS_DMA *)sc->dmaeng)->dmaregs[(int)sc->rambuf];
    U32 left = tcds->dma_unaln1.reg;
    volatile char *to;
    struct buf *bp;

/* if were running with map registers, then te->addr may be either a user or kernel
 * address.  If not, then te->addr is always a kernel address
 */
    if (build_frag_table == build_flam_map_frag_table) {
	if (CAM_IS_KUSEG(addr)) {
	    bp = GET_BP_SIMWS(sim_ws);
	    to = (volatile char *)CAM_PHYS_TO_KVA(pmap_extract( bp->b_proc->task->map->vm_pmap, addr ));
	} else
            to = (volatile char *)addr;
        to += nxfer;
    } else
        to = (volatile char *)backcvt(tcds->dma_addr.reg);

    for (; count; --count, ++to, left >>= 8)
	*to = left & 0xff;

    return 0;
    }

/* ---------------------------------------------------------------------- */
/*
static unsigned *
ioa_addrcvt( addr )

Inputs:
	char *addr;		 K2SEG address for the data 

Function:
 	Convert input virtual address to physical which, in turn, must
        be modified to meet IOASIC format requirements.
Return:
	Physical memory address in IOASIC format.
*/

static unsigned
ioa_addrcvt ( SIM_WS *sws, char *addr )
    {
    unsigned long a;
    unsigned long p;
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
    return ((unsigned int)((p >> 2) & 0xffffffff));
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

/* FIXME
 */
static void *backcvt( U32 addr )
    {
    U32 a = (U32) addr;
    U32 p;
    p = a << 2;
    return (void *)( CAM_PHYS_TO_KVA( p ));
    }

/**
 * dme_flam_bump_sglist -
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
dme_flam_bump_sglist( DME_DESCRIPTOR *dme_desc, DME_FLAM_STRUCT *ldme )
  {
    U32 retval=CAM_REQ_CMP;          /* Return value */
    SIM_WS *sim_wset;                     /* Local sim_ws pointer */
    
    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_DMA_FLOW,
           ("[b/t/l] (dme_flam_bump_sglist): entered dme_bump_sglist\n"));       
    
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
               ("[b/t/l] (dme_flam_bump_sglist) no more sglist elements tp process\n"));
        return CAM_DATA_RUN_ERR;
    }
    
    return retval;
}                              /* dme_flam_bump_sglist */

/* wake up the TCDS ASIC
 */
tcds_enable(SIM_SOFTC *sc)
{
    volatile unsigned int *pimr = &((TCDS_DMA *)sc->dmaeng)->imer.reg;
    volatile unsigned int *xcir = sc->dmaeng;
    extern int cpu;

    *xcir = 0xc00;
    WBFLUSH();
    *pimr = 0xc000c;
    WBFLUSH();
}

void
tcdsintr(int controller)
{
    register SIM_SOFTC *sc = (SIM_SOFTC *)SC_GET_SOFTC(controller);
    volatile unsigned int *xcir = sc->dmaeng;

    ascintr(controller + ((*xcir & SCSI1_C94) >> 19));
    return ;
}

#define Cprintf

int
tscsiconf(slotp,busp,ctlrp)
struct tc_slot *slotp;
struct controller *ctlrp;
struct bus *busp;
{
    int i;
    int tcindex = slotp - tc_slot;
    register struct controller *ctlr;
    register volatile SIM94_REG *reg;
    TCDS_ASIC *tcds;

    Cprintf ("tscsiconf: slot = %d, module = %s, device = %s\n",
		slotp->slot, slotp->modulename, slotp->devname);
    Cprintf ("tscsiconf: bus = %s%d\n",
		busp->bus_name, busp->bus_num);

    tcds = (TCDS_ASIC *)CAM_PHYS_TO_KVA(slotp->physaddr);

    for (i = 0; i < 2; i++) {		/* two controllers on a TSCSI */
	/*
	 * Check for the existence of an actual chip
	 * on the other side of the TSCSI
	 *
	 * This is done by writing two different values
	 * to the 94 chip.   If the values are still present
	 * when they are read, then the chip must be there.
	 */
	reg = (SIM94_REG *)(i ? &tcds->c941_reg : &tcds->c940_reg);

	reg->sim94_cnf2 = 0x8a;
	reg->sim94_cnf3 = 0x05;
	if (((reg->sim94_cnf2 & 0xff) != 0x8a) || ((reg->sim94_cnf3 & 0xff) != 0x05))
	    continue;

	Cprintf("tscsiconf: Calling get_ctlr %d\n", i);
	Cprintf("tscsiconf: name = '%s', slot = %d\n",
		slotp->devname, slotp->slot);
	/* Get the controller structure.  Check in descending levels of
	 * of specificity (nice word eh).
	 */
	if( (ctlr = get_ctlr(slotp->devname, 
			     slotp->slot,
			     busp->bus_name,
			     busp->bus_num)) ||
	    (ctlr = get_ctlr(slotp->devname,
			     slotp->slot,
			     busp->bus_name,
			     -1)) ||
	    (ctlr = get_ctlr(slotp->devname,
			     -1,
			     busp->bus_name,
			     busp->bus_num)) ||
	    (ctlr = get_ctlr(slotp->devname,
			     -1,
			     busp->bus_name,
			     -1)) ||
	    (ctlr = get_ctlr(slotp->devname,
			     -1,
			     "*",
			     -99)) ) {
			
		Cprintf("tscsiconf: Found controller: %lx\n", ctlr);
		Cprintf("tscsiconf: Calling config_cont\n");

		ctlr->bus_num = busp->bus_num;
		ctlr->bus_name = busp->bus_name;
		ctlr->tcindx = (caddr_t)tcindex;
		ctlr->flags = i;
		if (!(tc_config_cont(i ? &tcds->c941_reg : &tcds->c940_reg,
				     tcds,
				     slotp->slot,
				     slotp->devname, 
				     ctlr))) {
		    printf("%s%d not probed\n",
			   slotp->devname, slotp->unit + i);
		    (*tc_sw.disable_option)(tcindex);
		    return;	/* If one fails, just bail out */
		} else {
		    Cprintf("tscsiconf: Calling conn_ctlr\n");
		    conn_ctlr(busp, ctlr);
		    slotp->flags |= TC_ACTIVE;
	        }
	}
    }

    /* The slot probed OK, turn it on if needed */
    if (slotp->intr_aft_attach) {
	(*tc_sw.enable_option)(tcindex);
    } else {
	(*tc_sw.disable_option)(tcindex);
    }
}


