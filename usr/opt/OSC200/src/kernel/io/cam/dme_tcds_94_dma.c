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
static char *rcsid = "@(#)$RCSfile: dme_tcds_94_dma.c,v $ $Revision: 1.1.8.7 $ (DEC) $Date: 1993/10/18 17:44:36 $";
#endif

/* ---------------------------------------------------------------------- */

/* dme_tcds_94_dma.h	Version 1.02			Mar. 26, 1993 */

/*  This file contains the support code and DME interface routines for the
TurboChannel Dual SCSI, TCDS, scsi adaptor. */

/*
Modification History

	Version	  Date		Who	Reason

	1.00	10/15/92	jag	Created this file from the
					two DME files for flamingo + 3min.
	1.01	03/20/93	fek	Remove CPU specific tc clock rates
					from here - call H/W specific routine
					to get that information.
	1.02	03/26/93	jag	General comment updates, change to
					copy the bp into the working set.

*/

/* ---------------------------------------------------------------------- */
/*
 
SCSI CAM Subsystem DME module for the TCDS DMA engines.

The Data Mover Engine (DME) component is an evolution of the PDMA
method of data transfer used in the previous ULTRIX SCSI support.  The
DME's intent is to hide the details of data movement in different
HBA's, behind a well defined sequence of higher level functions.

This file contains the code to implement the DME functionaliy for the
TurboChannel Dual SCSI ASIC, (TCDS ASIC), DMA engine and NCR53c94 SCSI
chip.  The ASIC is a bus interface chip connecting the TURBOChannel,
TC, to two SCSI '94 chips.  There are two DMA engines contained in the
TCDS with seperated control registers and common TCDS control
registers.  This DME file only deals with the TCDS DMA engines and SCSI
'94 combination.

The ASIC is able to perform Direct Memmory Access, DMA, operations to
Main memory, via the TC, to service the DATA requests of the '94 SCSI
chip.  The two chips work together to perform data transfers.  The '94
chip keeps track of the data count, how many bytes to transfer for the
I/O, and the ASIC keeps track of where in memory the transfer has to
take place, the physical address.

There are limitations in the DMA functionality that the ASIC/TC
provides.  The ASIC does DMA operations within a system "page" boundary
using physical addresses, currently 8k in Alpha systems, using a single
address pointer register.  Another limitation, due to TC non-existant byte
addressing the ASIC can only do 4 bytes, a 32 bit word, across the TC
at a time.  Inorder to implement the 4 byte xfers the ASIC contains an
four byte FIFO that is used to hold the data until another four byte
DMA TC cycle is needed.  With these two limitations the I/O DME code
has to deal with two boundary conditions, 4 byte address and 8k page
sizes.

Any I/O transfer that contains starting address or ending addresses
that do not end on an 4 byte boundary, has to have special DMA
pre-processing to allow DMA to occur.  The DME code uses "safe zones"
that are contained in the DME working sets.  These safe zones are 4
bytes large and start on an 4 byte boundary.  All data bytes that are
not on a proper boundary in main memory are transfered via these zones,
with the ASIC using the physical address and the CPU using Kernel
virtual addresses.

For each I/O the DME code builds a Data Address Table, DAT, to describe
the buffer in main memory.  The table consists of elements, and each
element contains a two address pointers, a byte count, and flags.  Two
pointers are used, one the physical address of this part of the buffer
and the other is the kernel virtual address for the same location.  The
count, is how many contigious bytes are located at the pointers.  The
flags field is used for signaling and control information, ie the DAT
element is for one of the zones.  The code tries to build a complete
DAT to describe the entire I/O buffer, however as demand increases it
is possible that there are not enough available DAT elements.  The code
is capabable of using the allocated DAT elements as a ring, reusing
what is nolonger needed.  In the DAT calculation code all address go
through two manipulations, the "working" address is converted to it's
physical address and the physical address is converted to a kernel
virtual address.  The kernel virtual address is there to allow the CPU
to manupulate the user buffer when necessary.

There are a couple of Notes that are important to describe here, they are
not immediately obvious in the code.

The ASIC and the '94 chip are connected using a 16 bit wide data
bus.  Like the TC <=> ASIC 4 byte interface, the ASIC <=> '94 only
have data cycles of 2 bytes.  However when the '94's transfer counters
contain an odd count, ie 3 (actually the worst), it is not setup to
hold the last byte.  What will happen is that the '94 will do a
complete 2 byte data cycle with the high order byte "trash".  The
ASIC, with this last 2 bytes filling it's internal FIFO, will also do
a complete 4 byte DMA transfer into main memory with the "trash" byte
ending up in main memory at the 4th address.  This is why a safe zone
must also be used to handle the "end" conditions so that any real user
data, in the 4th byte, is not lost.

There are a number of routines that access the shared control byte in
the SDIC register.  The ASIC pointer routines make use of the address
bits and this enable routine makes use of the direction and read
prefetetch bits.  It is necessary to only use the &= and |= operators
on this register.  The Read Prefetch bit is current disabled pending
work on adding in the mapping register support.

Futures:
    There are two functional pieces that are left on the TODO list, mapping
    register support and enabling the ASIC Read Prefetch.   The mapping
    support needs to be implemented to allow many I/O's that may or may
    not have mapping resources allocated to them.  The functionality
    should also support "un-mapping" an I/O to handle SCSI disconnect
    bountry conditions.

    The Read prefetch functionality can be enabled once a check for the 
    "physical end of memory" has been done.  This still needs to be
    done for systems with mapping support, in the event that an I/O
    is valid but not mapped due to limited resources.

This DME file can be used by all the TCDS - '94 systems.  The TCDS is
available on the mother boards and as a TC option card.

The DME model has broken down data transfer operations into
8 distinct steps or DMA primitives: 

dme_init -	Initialize DME subsystem, resource queues and allocate control 
		structures.
dme_setup -	Allocate required DMA resources and build the DAT structure
dme_start -	(Currently not used, the functionality is contained in
		the DME primitve dme_resume).
dme_resume -	Continue (restart) DMA after terminal count or PAUSE and 
		fill or clear next buffer.  Transfer setup information from
		DME DAT elements to the ASIC and '94 and start DMA activity. 
dme_pause -	Instructs the DME to pause the DMA to be resumed later.  Any
		user data that is contained in a safe zone is moved.
dme_save -	Instructs the DME to save the current data pointer
dme_restore -	Instructs the DME to restore the current pointers
dme_modify -	Instructs the DME to modify the current data pointers.
dme_end -	Complete the DMA transaction, free up any allocated DME
		resources, and return actual bytes sent or received.

There are several other utility functions supported by the DME
interface but not implemented here.

dme_copyin -	Moves bytes out of the intermediate buffer to their
		destination.
dme_copyout -	Moves bytes into the intermediate buffer.
dme_clear -	Utility routine used to zero buffers for security reasons.

*/

/* ---------------------------------------------------------------------- */
/* Include files.  */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/buf.h>
#include <sys/proc.h>

#include <kern/lock.h>
#include <mach/vm_param.h>
#include <machine/machparam.h>
#include <machine/pmap.h>

#include <io/dec/tc/tc.h>

#include <io/common/iotypes.h>
#include <io/common/devdriver.h>
#include <io/cam/cam.h>
#include <io/cam/dec_cam.h>
#include <io/cam/scsi_all.h>
#include <io/cam/cam_debug.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim_common.h>		/* SIM common definitions. */
#include <io/cam/dme.h>			/* DME specific structs and consts */
#include <io/cam/sim.h>			/* SIM specific structs and consts */
#include <io/cam/sim_94.h>		/* N53C94 specific */
#include <io/cam/dme_common.h>		/* common defines for DAT based DMEs */
#include <io/cam/dme_tcds_94_dma.h>	/* specific defines for this DME */
#include <io/cam/tcds_adapt.h>		/* specific defines for the TCDS */

/* ---------------------------------------------------------------------- */
/* Defines and includes for Error logging.  */

#define CAMERRLOG		/* Turn on the error logging code */

#include <dec/binlog/errlog.h>		/* UERF errlog defines */
#include <io/cam/cam_errlog.h>		/* CAM error logging macro */
#include <io/cam/cam_logger.h>		/* CAM error logging definess */

/* ---------------------------------------------------------------------- */
/* Function Prototypes: */

U32 dme_tcds_init();
U32 dme_tcds_setup();
U32 dme_tcds_resume();
U32 dme_tcds_pause();
U32 dme_tcds_save();
U32 dme_tcds_restore();
U32 dme_tcds_end();
U32 dme_tcds_interrupt();
U32 dme_flam_interrupt();		
U32 tcds_dme_attach();
U32 tcds_dme_unload();
U32 dme_tcds_modify();
U32 asic_pointer_load();
void * asic_pointer_fetch();
void tcds_dma_enable();
void tcds_dma_disable();
U32 get_asic_count();
void set_asic_control();
void * tcds_flush_asic();
void dme_tcds_errlog();

/* ---------------------------------------------------------------------- */
/* External declarations: */

extern SIM_SOFTC *softc_directory[];	/* for accessing via "controller" */

extern int bcopy();			/* System routine to copy buffer  */
extern int bzero();			/* System routine to clear buffer */

extern U32 dcmn_dat_alloc();
extern U32 dcmn_dat_build();
extern U32 dcmn_dat_calc();
extern U32 dcmn_dat_adj();
extern U32 dcmn_flush_safe_zone();
extern U32 dcmn_rtn_resources();
extern DMA_CTRL * dcmn_alloc_ctrl();
extern void dcmn_free_ctrl();
extern U32 dcmn_alloc_wsets();
extern void dcmn_free_wsets();
extern U32 dcmn_alloc_dats();
extern U32 tcds_get_clock();
extern U32 tcds_get_period();
extern DME_STRUCT * dcmn_alloc_dme();

extern int  sim_poll_mode;		/* Signal for the '94 cmd MACRO */

/* ---------------------------------------------------------------------- */
/* Local Type Definitions and Constants */ 


/* This is the minimum number of DAT elements that a single DME I/O can 
work with.  With the only having 3 DAT elements, on a DAT_INCOMPLETE
element DME_RESUME() can start calculating for the next data and still
keep the one in front available for a disconnect. */

#define MIN_DATS	3		/* can work w/3 */

/* ---------------------------------------------------------------------- */
/* Initialized and uninitialized data: */

static void (*local_errlog)() = dme_tcds_errlog;	/* logging handler */

/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/*
Routine Name: dme_tcds_init

Functional Description:

    Initialize the DME subsystem. Perform DME specific initialization.
    The DME control structure is allocated and attached to the
    extension field in the DME structure.  All the working sets and
    DAT groups are allocated and attached to the control structure.

Formal Parameters:
    SIM_SOFTC* sc - Address of sim controller structure.

Implicit Inputs: NONE

Implicit Outputs: NONE

Return Value:
    CAM_REQ_CMP            - All is well
    CAM_REQ_CMP_ERR        - DME failed to initialize

Side Effects:
    A large number of resources will be allocated out of the system.

Additional Information:
    At this time multiple init calls will result in a failure return.

    JAG: What to do with an allocation failure along the line ?
    Is it ok to continue w/out DMA_WSETs and DAT groups ?

*/

U32
dme_tcds_init( sc )
    SIM_SOFTC *sc;
{
    U32 *slotp;                 /* ASIC SCSI DMA slot register pointer */
    DMA_CTRL *dc;		/* for the control structure */
    DAT_ATTCH *da;		/* for the DAT group */
    DMA_WSET *dw;		/* for the working sets */

    int i;                      /* private counter */
    
    PRINTD( sc->cntlr, NOBTL, NOBTL, (CAMD_INOUT),
       ("[%d/t/l] (dme_tcds_init) Entry - sc=0x%x\n", sc->cntlr, sc ) );

    /* Check to make sure that this is not a repeat initialization call.
    If if is return failure, for future this path may force a complete 
    re-initialization. */

    if ( sc->simd_init )		/* already done ? */
    {
	PRINTD( sc->cntlr, NOBTL, NOBTL, (CAMD_ERRORS | CAMD_DMA_FLOW),
	   ("[%d/t/l] (dme_tcds_init): Duplicate init call\n", sc->cntlr ));
	return( CAM_REQ_CMP_ERR );	/* signal failure */
    }

    /* Allocate the DMA control structure and add it to the SIM_SOFTC. */

    dc = dcmn_alloc_ctrl();
    if( dc == (DMA_CTRL *)NULL )
    {
	PRINTD( sc->cntlr, NOBTL, NOBTL, (CAMD_ERRORS | CAMD_DMA_FLOW),
	    ("[%d/t/l] (dme_tcds_init): Failure to alloc DME control.\n", 
	    sc->cntlr) );
	return( CAM_REQ_CMP_ERR );	/* signal failure */
    }

    /* Initialize the control structure in preperation for the addition of
    the working sets and DATs.  Only a few of the working set fields will
    have any real meaning when used in the control structure. */

    dc->dma_ctrl_flags = DME_DMA_CTRL;
    dc->flink = (DMA_WSET *)NULL;		/* nothing on the free list */
    dc->blink = (DMA_WSET *)NULL;		/* nothing on the busy list */
    dc->nfree = 0;				/* the free list is empty */
    dc->nbusy = 0;				/* the busy list is empty */
    dc->dat_group = (DAT_ATTCH *)NULL;		/* nothing on the DAT list */
    dc->dat_per_ring = 0;			/* no Rings alloced yet */
    dc->dme_zone_size = TCDS_ZONE_SIZE;		/* the zone reqs for the DME */

    D3CTRL_INIT_LOCK( dc );			/* Init the control SMP lock */
    
    /* Alloc an initial bunch of DMA_WSETs.  The parameters for the working
    sets are contained in the dcmn_alloc_wsets() code.  The DMA_WSETs
    will be put onto the control struct.  If there were no DMA_WSETs
    allocated then failure is returned. */

    i = dcmn_alloc_wsets( dc );
    if( i == 0 )
    {
	PRINTD( sc->cntlr, NOBTL, NOBTL, (CAMD_ERRORS | CAMD_DMA_FLOW),
	    ("[%d/t/l] (dme_tcds_init): Failure to alloc DMA_WSETs.\n",
	    sc->cntlr ) );

	/* Dealloc the control structure */
	dcmn_free_ctrl( dc );	/* it is not needed anymore */

	return( CAM_REQ_CMP_ERR );	/* signal failure */
    }

    /* Alloc an initial DAT group.  The parameters for the DAT groups
    are contained in the dcmn_alloc_dats() code.  The DAT group
    will be put onto the control structure.  If the allocation of the
    DAT group failed the DME initialization will also fail. */

    i = dcmn_alloc_dats( dc );
    if( i == 0 )
    {
	PRINTD( sc->cntlr, NOBTL, NOBTL, (CAMD_ERRORS | CAMD_DMA_FLOW),
	    ("[%d/t/l] (dme_tcds_init): Failure to alloc DAT group.\n", 
	    sc->cntlr) );

	/* Dealloc the DMA_WSETs. */
	dcmn_free_wsets( dc );	/* all of them are freed */

	/* Dealloc the control structure */
	dcmn_free_ctrl( dc );	/* it is not needed anymore */

	return( CAM_REQ_CMP_ERR );	/* signal failure */
    }

    /* Now that the control structure and all the assorted DME resources
    have been setup put the control pointer in the SIM_SOFTC's DME struct. */

    sc->dme->extension = (void *)dc;   /* fill in the DME specific field */
    
    /* tscsiconf hides the channel number in slot; we hide it in
    rambuf instead of taking another indirection through the um_probe
    field of the softc */

    sc->rambuf = (void *)((struct controller *)sc->um_probe)->slot;
    
    /* The ASIC hardware has to be setup to allow it to be used.  */

    sc->dmaeng = sc->dme->hardware;	/* prep alternate pointer to hardware */
    
    sc->simd_init = 1;			/* all done with the initialization */
    
    return( CAM_REQ_CMP );
}

/* ---------------------------------------------------------------------- */
/*
Routine Name: dme_tcds_setup

Functional Description:

    Allocate required DMA resources needed by the TCDS DME for this
    I/O.  A DMA_WSET is taken from the free list, and the necessary
    number of DATs are taken from a DAT group.  If either if these
    alloc's fail the setup will also fail and hopefully it will be
    rescheduled above.

Formal Parameters:

    sim_wset          - SIM Working Set of Active I/O request.
    count             - Number of bytes to transfer
    buf               - Address of source/dest buffer 
    dir               - Direction of data transfer,RD/WR? 
    dme_desc          - Address of descriptor that describes this DMA request

Implicit Inputs:
    The DME working sets and DAT groups attached to this controller via
    the SIM_SOFTC.

Implicit Outputs:    NONE

Return Value:
    CAM_REQ_CMP			- All is well setup is finished
    CAM_PROVIDE_FAIL		- Failure alloc w/current resources
    CAM_REQ_CMP_ERR		- a fatal problem has occurred

Side Effects:

The local pools are subtracted from.

Additional Information:

    For the first implementation pass there are no attempts to "grow"
    the working sets or the DAT groups when they run out.  This
    enhancement will be for the second pass.
*/

static char *dme_tcds_setup_func = "dme_tcds_setup()";	/* func name */

U32
dme_tcds_setup( sim_wset, count, buf, dir, dme_desc )
    SIM_WS *sim_wset;			/* pointer to the SIM working set */
    U32 count;				/* number of bytes to xfer */
    void *buf;				/* users address for the data */
    U32 dir;				/* direction of the data flow */
    register DME_DESCRIPTOR *dme_desc;	/* pointer for the I/O desc. */
{
    SIM_SOFTC *sc;			/* for accessing the SIM_SOFTC */
    register DMA_WSET *dws;		/* for the specific working set */
    DMA_CTRL *dctrl;			/* ptr for the control structure */
    XFER_INFO *xc;			/* ptr for the current xfer struct */

    int s;				/* for locking */
    int ndat_elem;			/* how many DAT elements to req */

    PRINTD( sim_wset->cntlr, sim_wset->targid, sim_wset->lun,
	(CAMD_INOUT | CAMD_DMA_FLOW),
        ("[%d/%d/%d] (dme_tcds_setup) Entry - sws=0x%x, count=%d,\n",
	 sim_wset->cntlr, sim_wset->targid, sim_wset->lun, sim_wset, count ) );
    PRINTD( sim_wset->cntlr, sim_wset->targid, sim_wset->lun, 
	(CAMD_INOUT | CAMD_DMA_FLOW),
        ("\tbuf=0x%x, dir=0x%x, dme_desc=0x%x\n", buf, dir, dme_desc ) );

    /* Make sure that the dir parameter only contains the CAM direction
    flags. */

    dir &= CAM_DIR_NONE;		/* keep only the DIR bits 07:06 */

    /* Assign SIM_WS pointer to the passed DME_DESCRIPTOR.  Setup the
     data count, data ptr, and direction in the DME_DESCRIPTOR.  */

    dme_desc->sim_ws = (void *)sim_wset;	/* Get a copy for later use */
    dme_desc->data_count = count;       /* How big is the entire request? */
    dme_desc->data_ptr = buf;           /* User buffer address S0 or P0 */
    dme_desc->flags |= dir;             /* Save the CAM dir for this I/O */

    /* Try to alloc the resources before attaching the working sets and
    descriptors to the DME descriptor. */

    /* SMP lock on the control struct, remove the front free DMA_WSET and
    update the number of free DMA_WSETs.  In the event that there are no
    more DMA_WSETs to give out return with a status indicating allocation
    failure. */

    sc = GET_SOFTC_DESC( dme_desc );			/* get the softc */
    dctrl = (DMA_CTRL *)sc->dme->extension;		/* get the control */

    D3CTRL_IPLSMP_LOCK( s, dctrl );		/* lock on the control */
    dws = dctrl->flink;			/* grab the front one */

    if( dws == NULL )			/* No more left */
    {
	D3CTRL_IPLSMP_UNLOCK( s, dctrl );	/* unlock control */
	return( CAM_PROVIDE_FAIL );	/* signal a simple resource failure */
    }

    dctrl->flink = dws->flink;	/* bump dwn the free side */

    if( dws->flink != NULL )		/* if more than one left on the Q */
    {
	(dws->flink)->blink = NULL;	/* NULL the front back ptr */
    }

    dctrl->nfree--;			/* decr # of free */

    /* Update the desc and control pointers in the working set. */

    dws->dme_desc = dme_desc;           /* for back access to the desc */
    dws->dma_ctrl = dctrl;		/* for back access to the control */
    
    /* Request a Number of DATs from the DAT groups.  The requested
    number is based on the number of system "PAGES" in the users buffer
    plus 3, one for each of the TCDS Safe zones, and one for the first
    DAT calculation incase an available DAT is needed.  It is possible
    for the DAT alloc code to be unable to provide all that was
    requested.  It will return what it was able to alloc.  If that is
    not enough to do the job they will be returned and the DME setup
    failed. */

    ndat_elem = ((count + (DME_PAGE_SIZE - 1)) / DME_PAGE_SIZE) + 3;

    /* Check to see how many have been acutally allocated.  The alloc DAT
    code attempts to do a best fit with what is available. */

    if( dcmn_dat_alloc( dws, ndat_elem ) < MIN_DATS )	/* can work with min */
    {
	/* Return the allocated DATs back to the groups, for the next
	working set setup call.  They are nolonger needed. */

	dcmn_rtn_resources( dws );		/* put them back */

	D3CTRL_IPLSMP_UNLOCK( s, dctrl );	/* unlock control */
	return( CAM_PROVIDE_FAIL );	/* signal a simple resource failure */
    }

    /* Update the busy counter to keep overall track of how many are in
    service. */

    dctrl->nbusy++;			/* how many are attched to SIM_WSETs */

    /* After all the resources are allocated release the control struct. */

    D3CTRL_IPLSMP_UNLOCK( s, dctrl );	/* unlock the control struct */

    /* Update the working set with all the necessary information to get 
    ready for the I/O. */

    /* Set the DME direction in the working set.  The CAM DATA IN/OUT
    flags from the "dir" parameter need to be converted into
    ASIC_RECEIVE/TRANSMIT flags. */

    if( dir == CAM_DIR_IN )
    {
	dws->dir = ASIC_RECEIVE;	/* Data going into memory */
    }
    else
    {
	dws->dir = ASIC_TRANSMIT;	/* Data comming in and default */
    }

    /* Carry around a local copy of the "bp" pointer from the CAM CCB,
    in the working set.  The code that builds that DAT table will use
    the working set copy. */

    dws->dme_bp = (void *)GET_BP_SIMWS( dme_desc->sim_ws );

    /* Load the XFER_INFO current structure with the DATA information.  If
    the data info in the CCB is a Scatter/gather list we will only process
    one S/G element at a time. */

    xc = &(dws->current);		/* Attach to the current struct */

    if(( sim_wset->cam_flags & CAM_SCATTER_VALID) != 0 )
    {
	/* Update the S/G struct in the working set to point to the
	passed scatter/gather list.  Setup counter of which scatter
	gather elemnt we are processing at this time.  */

	dws->sg_current.xfer_cnt = 0; 		/* no xfer yet */
	dws->sg_current.element_cnt = 1;	/* start with the first */

	/* Get pointer to the first SG_ELEM, in the case of a scatter
	gather request the data_ptr points at the address of the
	scatter gather list. */

	dws->sg_current.element_ptr = ((SG_ELEM*)dme_desc->data_ptr);

	/* Load the information from the first SG_ELEM into the current
	XFER_INFO structure. */

	xc->count = dws->sg_current.element_ptr->cam_sg_count;
	xc->vaddr = dws->sg_current.element_ptr->cam_sg_address;

	/* Signal in the DME desc that this is a scatter/gather I/O. */

	dme_desc->flags |= DME_SG_LIST;
    }
    else
    {
	/* The transfer information is a simple linear buffer.  Load the 
	current XFER_INFO structure with the data. */

	xc->count = count;			/* the down counter */
	xc->vaddr = buf;
    }

    /* Proir to doing any thing with the buffer address and count, (the
    DAT calc code may have to move some data), prepare the system cache. */

    DO_CACHE_BEFORE_DMA( buf, count );

    /* Call the DAT build code to do the DAT calculations. */

    if( dcmn_dat_build( dws ) != CAM_REQ_CMP)
    {
	PRINTD( sim_wset->cntlr, sim_wset->targid, sim_wset->lun,
	(CAMD_ERRORS | CAMD_DMA_FLOW),
	    ("[%d/%d/%d] (dme_tcds_setup) Failed building xfer table.\n",
	     sim_wset->cntlr, sim_wset->targid, sim_wset->lun ) );
	CAM_ERROR( dme_tcds_setup_func, "Failed building DAT transfer table.",
	    (CAM_ERR_LOW), 0, dme_desc, (MEMERR_LOG *)NULL );

	/* Return the WSET and DATs they are nolonger needed. */

	D3CTRL_IPLSMP_LOCK( s, dctrl );		/* lock on control */
	(void)dcmn_rtn_resources( dws );
	D3CTRL_IPLSMP_UNLOCK( s, dctrl );	/* unlock control */

	return( CAM_REQ_CMP_ERR );
    }

    PRINTD( sim_wset->cntlr, sim_wset->targid, sim_wset->lun, (CAMD_DMA_FLOW),
	("[%d/%d/%d] (dme_tcds_setup) DAT xfer table complete.\n",
	 sim_wset->cntlr, sim_wset->targid, sim_wset->lun ) );
    PRINTD( sim_wset->cntlr, sim_wset->targid, sim_wset->lun, (CAMD_DMA_FLOW),
	("", dcmn_dumptbl( dws )) );

    /* Do a save of the data pointers at this point to insure, that we
    can do an automatic restore in the case where the target
    disconnects and reconnects as an error recovery mechanism. In this
    case, the target is expecting that initiator will be doing an
    implicite restore pointers.  In order to do an implicite restore
    pointers, we must do an explicite save pointers here! */

    dme_desc->segment = (void *)dws;	/* save our context here         */

    /*
     * Determine what NCR53C94 command to use when transferring
     * data.  If in target mode, determine the direction of the
     * transfer.
     */
    if (sim_wset->flags & SZ_TARGET_MODE) 
    {
        if (dir & CAM_DIR_IN)
            dws->hba_cmd = (SIM94_CMD_RECDATA | SIM94_CMD_DMA);
        else
            dws->hba_cmd = (SIM94_CMD_SNDDATA | SIM94_CMD_DMA);
    }
    else
        dws->hba_cmd = SIM94_CMD_DMAXFER;

    (void)dme_tcds_save( dme_desc );

    /* Set the state to SETUP, now that all has been done. */

    dws->wset_state = DME_STATE_SETUP;

    /* Set the overall I/O transfer counters to 0 for setup. */

    xc->bxfer = 0;		/* the up counter in the current XFER_INFO */
    dme_desc->xfer_count = 0;	/* the up counter in the DME descriptor */

    return( CAM_REQ_CMP );
}			 /* dme_setup */

/* ---------------------------------------------------------------------- */
/*
Routine Name: dme_tcds_resume

Functional Description:

    Start DMA activity between the target and initiator. This routine
    uses the state of the DMA activity previously initialized by the
    SIM.

Formal Parameters:
    dme_desc          - Address of descriptor that describes this DMA request

Implicit Inputs:             NONE

Implicit Outputs:            NONE

Return Value:                CAM_REQ_CMP - All is well.

Side Effects:                NONE

Additional Information:      This routine was previously dme_tcds_start.

*/

static char *dme_tcds_resume_func = "dme_tcds_resume()";	/* func name */

U32
dme_tcds_resume( dme_desc )
    DME_DESCRIPTOR *dme_desc;
{
    int s;				/* Used by DME lock                 */
    SIM_SOFTC* sc;			/* Pointer to SIM S/W control struct*/
    SIM94_REG *reg94;			/* Address of 94 CSR's              */
    SIM_WS *sim_wset;			/* SIM working set for this request */
    register DMA_WSET *dws;		/* DME specific data struct ptr.    */
    register DAT_ELEM *cdat;		/* pointer for the current DAT */

    /* Setup required local variables.  */

    sim_wset = dme_desc->sim_ws;
    sc = GET_SOFTC_DESC( dme_desc );
    reg94 = SIM94_GET_CSR( sc );

    PRINTD( sim_wset->cntlr, sim_wset->targid, sim_wset->lun, 
	(CAMD_INOUT),
	("[%d/%d/%d] (dme_tcds_resume) Entry - dme_desc=0x%x\n", 
	sim_wset->cntlr, sim_wset->targid, sim_wset->lun, dme_desc ) );
    
    dws = (DMA_WSET *)dme_desc->segment;
    if( dws == (DMA_WSET *)NULL )
    {
	CAM_ERROR( dme_tcds_resume_func, "Invalid DME working set pointer.",
	    (CAM_ERR_HIGH), 0, dme_desc, (MEMERR_LOG *)NULL );
        return( CAM_REQ_CMP_ERR );
    }

    dws->wset_state = DME_STATE_RESUME;         /* current DME state */

    PRINTD( sim_wset->cntlr, sim_wset->targid, sim_wset->lun, (CAMD_DMA_FLOW),
	("[%d/%d/%d] (dme_tcds_resume) Current DAT xfer table.\n",
	 sim_wset->cntlr, sim_wset->targid, sim_wset->lun ) );
    PRINTD( sim_wset->cntlr, sim_wset->targid, sim_wset->lun, (CAMD_DMA_FLOW),
	("", dcmn_dumptbl( dws )) );

    /* Get the address of the current DAT to be worked on.  Check to make
    sure that it is all set to go, contains valid I/O information. */

    cdat = GET_DAT_PTR( dws, dws->current.index );

    PRINTD( sim_wset->cntlr, sim_wset->targid, sim_wset->lun, CAMD_DMA_FLOW,
	("[%d/%d/%d] (dme_tcds_resume) - dws=0x%x cdat=0x%x\n", 
	sim_wset->cntlr, sim_wset->targid, sim_wset->lun, dws, cdat ) );

    if( (cdat->dat_flags & DAT_VALID) == 0 )
    {
	CAM_ERROR( dme_tcds_resume_func, "Invalid DME DAT element.",
	    (CAM_ERR_HIGH), DME_ERR_LOGDAT, dme_desc, (MEMERR_LOG *)NULL );
	return( CAM_DATA_RUN_ERR );
    }

    /* Check this DAT entry for a data count of 0.  It is possible for 
    this to happen due to the disconnects around the funny ASIC boundaries.
    If a 0 count is found, call dme_tcds_resume() again with setting up
    to go to the next DAT entry. */

    if( cdat->dat_count == 0 )			/* NULL entry */
    {
	/* Bump to the next current DAT. */

	cdat->dat_flags &= ~(DAT_VALID);	/* this one is nolonger valid */
	dws->current.index = NEXT_DAT( dws, dws->current.index );
	return( dme_tcds_resume( dme_desc ) );	/* try again */
    }

    /* Setup the ASIC and 94 chip to start DMA activity.  */

    LOCK_DME( s, sc );              /* synchronize access to hba */
    sim_wset->flags |= SZ_DME_ACTIVE;

    /* Setup the DMA engine to transfer the data associated with this DAT.
    Make sure that the ASIC is disabled for DMA. */

    tcds_dma_disable( sc );			/* JIC, turn DMA OFF */

    (void)asic_pointer_load( sc, cdat->dat_paddr );

    /* Load the counts from the DAT into the '94s counter registers. */

    LOAD_94_COUNTER( reg94, cdat->dat_count );
    WBFLUSH();

    /* Enable DMA in the ASIC and kick off the '94 chip. */

    tcds_dma_enable( sc, dws ); 		/* Enable DMA and dir */
    SIM94_SEND_CMD(((SIM_WS*)(dme_desc)->sim_ws),reg94,dws->hba_cmd,
		sim_poll_mode);
    WBFLUSH();					/* conserve water */

    cdat->dat_flags |= DAT_INPROGRESS;		/* Signal men at work */
    
    UNLOCK_DME( s, sc );			/* release lock on hba */

    /* Now that DMA engine and SCSI chip are off doing xfer things look to
    see if any more DATs need to be calculated. */

    if( (cdat->dat_flags & DAT_INCOMPLETE) != 0 )	/* needs to more calc */
    {
	PRINTD( sim_wset->cntlr, sim_wset->targid, sim_wset->lun, CAMD_DMA_FLOW,
	("[%d/%d/%d] (dme_tcds_resume) - DAT_INCOMPLETE dws=0x%x cdat=0x%x\n", 
	    sim_wset->cntlr, sim_wset->targid, sim_wset->lun, dws, cdat ) );

	cdat->dat_flags &= ~(DAT_INCOMPLETE);	/* clear out the flag */

	/* Call the DAT calc code to continue and fill in more DATs
	toward completing this I/O.  The calc code can use all but the
	last 2 DAT element.  These two are the current one and the
	previous one, saved for possible disconnects. */

	if( dcmn_dat_calc( dws, dws->de_count - 2 ) != CAM_REQ_CMP )
	{
	    return( CAM_REQ_CMP_ERR );
	}
    }

    return( CAM_REQ_CMP );
}                     /* dme_resume */

/* ---------------------------------------------------------------------- */
/*
Routine Name: dme_tcds_pause

Functional Description:

    This function is called by the HBA each time the HBA receives an
    interrupt from the DME subsystem when the DME is active.
    dme_pause() will read the state of device registers to determine
    the progress of a dma transaction.  The state of the dma request
    will be updated to reflect the number of bytes successfully
    transfered.

    This function must be called before a dme_save, dme_end or
    dme_resume call be executed by the HBA.

Formal Parameters:
    dme_desc          - Address of descriptor that describes this DMA request

Implicit Inputs:             NONE

Implicit Outputs:            NONE

Return Value:                CAM_REQ_CMP - All is well.

Side Effects:                NONE

Additional Information:      NONE

*/

static char *dme_tcds_pause_func = "dme_tcds_pause()";	/* func name */

U32
dme_tcds_pause( dme_desc )
    DME_DESCRIPTOR *dme_desc;
{
    SIM_SOFTC* sc;                      /* Pointer to SIM S/W control struct */
    SIM94_REG *reg94;                   /* Address of 94 CSR's               */
    SIM_WS *sim_wset;                   /* Working set pointer for this req. */
    SIM94_SOFTC *ssc;                   /* HBA specific pointer              */
    register DMA_WSET *dws;		/* DME specific data struct ptr.    */
    register DAT_ELEM *cdat;		/* pointer for the current DAT */
    U32 fifo_residual;			/* Number of bytes left in FIFO      */
    U32 residual;			/* Number of bytes left Transfer CNT */
    U32 asic_residual;			/* Number of bytes left in ASIC    */
    U32 xfer_cnt;			/* Number of bytes really transfered */
    U32 zone;				/* for storage of the ZONE A/B flags */

    int s;                              /* Save the current IPL              */
    U32 sr;                          	/* '94 Status Register               */

    /*
     * Setup required local variables.
     */
    sim_wset = dme_desc->sim_ws;
    ssc = (SIM94_SOFTC *)sim_wset->sim_sc->hba_sc;
    sr = SIM94_GET_SR((SIM94_INTR *)ssc->active_intr);
    sc = GET_SOFTC_DESC( dme_desc );
    reg94 = SIM94_GET_CSR( sc );

    PRINTD( sim_wset->cntlr, sim_wset->targid, sim_wset->lun, 
	(CAMD_INOUT),
	("[%d/%d/%d] (dme_tcds_pause) - Entry - dme_desc=0x%x\n", 
	sim_wset->cntlr, sim_wset->targid, sim_wset->lun, dme_desc ) );

    dws = (DMA_WSET *)dme_desc->segment;
    if ( dws == (DMA_WSET *)NULL )
    {
	CAM_ERROR( dme_tcds_pause_func, "Invalid DME working set pointer.",
	    (CAM_ERR_HIGH), 0, dme_desc, (MEMERR_LOG *)NULL );
	return( (U32)CAM_REQ_CMP_ERR );
    }

    dws->wset_state = DME_STATE_PAUSE;		/* Last DME state */

    cdat = GET_DAT_PTR( dws, dws->current.index );

    if( (cdat->dat_count == 0) || !( sim_wset->flags & SZ_DME_ACTIVE ) )
    {                             /* Why do we get called when there */
                                  /* is nothing to do?               */  
	return( (U32)CAM_REQ_CMP );       /* Who knows!                      */
    }

    /* Clear the DME active bit, so that that SIM HBA doesn't think that
    any futher interrupts are for the DME.  */

    sim_wset->flags &= ~SZ_DME_ACTIVE;

    LOCK_DME( s, sc );

    tcds_dma_disable( sc );			/* Turn off the ASIC engine */

    /* Get the count of the number of bytes that were left in the
    fifo.  Sometimes the DMA engine will not transfer all the bytes
    and the residual is left in the fifo.  */

    fifo_residual = reg94->sim94_ffss & SIM94_FIFO_MSK;

    PRINTD(sim_wset->cntlr, sim_wset->targid, sim_wset->lun, CAMD_DMA_FLOW,
	("[%d/%d/%d] (dme_tcds_pause) - The FIFO resid byte count was %d.\n",
	sim_wset->cntlr, sim_wset->targid, sim_wset->lun, fifo_residual ) );

    residual = GET_94_COUNTER( reg94 );

    PRINTD(sim_wset->cntlr, sim_wset->targid, sim_wset->lun, CAMD_DMA_FLOW,
	("[%d/%d/%d] (dme_tcds_pause) - The '94 resid byte count was %d.\n",
	sim_wset->cntlr, sim_wset->targid, sim_wset->lun, residual));

    if( sr & SIM94_STAT_TC )  /* timing problem? */
    {   /* JAG - This test/code should come out if the Log never occurs */
	if( residual )
	{
	    CAM_ERROR( dme_tcds_pause_func, "TC bit is set, resid is non 0.", 
		(CAM_ERR_LOW), DME_ERR_LOGDAT, dme_desc, (MEMERR_LOG *)NULL );
	    residual = 0;
	}
    }

    /* Save the zone bit/indicator if it is set. It will be used later. */

    zone = GET_DAT_ZONE( cdat );		/* get the flags for later */

    /* Get what may be left in the ASIC.  For DATA IN there may be valid
    data bytes still in the ASICs FIFO. */

    asic_residual = get_asic_count( sc, dws->dir );

    PRINTD(sim_wset->cntlr, sim_wset->targid, sim_wset->lun, CAMD_DMA_FLOW,
	("[%d/%d/%d] (dme_tcds_pause) - The asic resid byte count was %d.\n",
	sim_wset->cntlr, sim_wset->targid, sim_wset->lun, asic_residual));

    /* Based on the direction of the data flow cleanup for this current DAT
    entry. */

    if( dws->dir == ASIC_RECEIVE )	/* DATA goesinta */
    {
	/* Based on what is left in the 94 counters figure what is the 
	actual count for the xfered data. */

	xfer_cnt = cdat->dat_count - residual;

	/* The ASIC keeps its count in 16 bit values.  If the
	transfer count for this DAT is odd then just subtract one off
	of the ASIC residual count. */

	if( (asic_residual != 0) && (xfer_cnt & 1 ))
	{
	    asic_residual--;
	}

	/* If there is any data left in the ASIC, move it out. */

        if( asic_residual != 0 )		/* is there data being held */
	{
            (void)tcds_flush_asic( sc, dws );	/* data to memory */
	}

	UNLOCK_DME(s,sc);                  /* Release the Lock. */

	/* Was this DAT pointing to one of the safe zones.  The data will have
	to be moved into the users buffer. */

        if( zone != 0 )
	{
            dcmn_flush_safe_zone( dws, cdat, xfer_cnt );
	}
        else
	{ 
	    /* Get the system virtual address and call the system code to 
	    clean out any of the data cache that may have contained the
	    old data.  The virtual K0 segment address is already stored
	    in the DAT, it is used. */

	    DO_CACHE_AFTER_DMA( (char *)cdat->dat_vaddr, xfer_cnt );
	}
    }
    else                                      /* A write operation        */
    {
	/* Calculate how many bytes have gone out.  There is no need to 
	move or save bytes on a DATA out.  The 94 fifo is flushed to
	remove any "stale" data bytes. */

	xfer_cnt = cdat->dat_count - (residual + fifo_residual);

        SIM94_FLUSH_FIFO( reg94 ); 		/*do it */
	WBFLUSH();

	UNLOCK_DME(s,sc);                  /* Release the Lock. */
    }

    /* If there was a zone used now allow it to be re-used in the working
    set. */

    if( zone != 0 )				/* was it a zone */
    {
	dws->dme_flags |= zone;		/* allow the zone to be used */
    }

    /* Using the xfer count update the various counters and pointers in the
    working set current xfer info struct and the DME descrptor. */ 

    PRINTD(sim_wset->cntlr, sim_wset->targid, sim_wset->lun, CAMD_DMA_FLOW,
	("[%d/%d/%d] (dme_tcds_pause) - The xfer byte count was %d.\n",
	sim_wset->cntlr, sim_wset->targid, sim_wset->lun, xfer_cnt));

    dws->current.count -= xfer_cnt;		/* New current down count */
    dws->current.vaddr = (void *)((vm_offset_t)dws->current.vaddr + xfer_cnt);
    dws->current.bxfer += xfer_cnt;		/* New current xfer amount */

    dme_desc->xfer_count += xfer_cnt;    	/* adjust transferred count */

    /*
      Now that we know the total number of bytes transfered for this data
      phase, determine whether all of the data has been transfered. If all
      of the data requested by the state machine in dme_setup has been
      transfered, then set the DME_DONE bit in the DME_DESCRITOR. This bit
      is used by the state machine when the state machine is executing target
      functions. DME_DONE signals the state machine that all of the data has
      been sent and that the state machine should change phase.
    */
    if ( dme_desc->xfer_count == dme_desc->data_count)  {
        /* Flag that all of the data has been sent */
        dme_desc->flags |= DME_DONE;
    }

    /* Update the count in the DAT with what was transfered.  If there
    is some more data still pointed to by this DAT then the
    address/counts have to be adjusted.  If the disconnect caused an
    ASIC mis-alighment then the previous DAT entry can be used to
    deal with the safe zone. */

    cdat->dat_flags &= ~(DAT_INPROGRESS);	/* no longer being worked on */
    cdat->dat_count -= xfer_cnt;		/* adjust the DAT count */
    if ( cdat->dat_count != 0 )			/* not end of this segment? */
    {
	/* Call the DAT adjust code to update the pointers and counters in
	this DAT, and possibly use the previous one. */

	cdat->dat_vaddr = (void *)((vm_offset_t)cdat->dat_vaddr + xfer_cnt);
	cdat->dat_paddr = (void *)((vm_offset_t)cdat->dat_paddr + xfer_cnt);

	(void)dcmn_dat_adj( dws, dws->current.index );

	PRINTD( sim_wset->cntlr, sim_wset->targid, sim_wset->lun,
	    (CAMD_DMA_FLOW),
	    ("[%d/%d/%d] (dme_tcds_pause) Adjusted DAT xfer elements.\n",
	     sim_wset->cntlr, sim_wset->targid, sim_wset->lun ) );
	PRINTD( sim_wset->cntlr, sim_wset->targid, sim_wset->lun,
	    (CAMD_DMA_FLOW), ("", dcmn_dumptbl( dws )) );

	return( CAM_REQ_CMP );		/* there are still DATs left */
    }
    else
    {
	/* All done with this DAT entry.  Setup the next one for the following
	call to DME_RESUME().  Update the flags for administriva. */

	cdat->dat_flags &= ~(DAT_VALID);
	cdat->dat_flags |= DAT_USED;

	/* If this DAT is not the last one, DAT_FINAL, then increment the
	current DAT index to prepare for the next call to DME_RESUME(). */

	if( (cdat->dat_flags & DAT_FINAL) == 0 )	/* not the last one ? */
	{
	    /* Increment to the next DAT element. */

	    dws->current.index = NEXT_DAT( dws, dws->current.index );

	    return( CAM_REQ_CMP );	/* there are still DATs left */
	}
    }

    /* If the code has made it this far then this is the last, DAT_FINAL,
    DAT entry, check to see if the I/O contained a scatter/gather list. 
    There is a game that is being played with S/G lists and the DAT calcs.
    The code in DME_SETUP() and DME_PAUSE() are dealing with moving
    the DATs along the S/G list.  The next S/G element has to be setup
    in the current/ahead info and the DAT calculations done all over
    again.  */

    if( (dme_desc->flags & DME_SG_LIST) != 0 )		/* a S/G list ? */
    {
	/* Update the sg_current transfer counter, this trick for dealing
	with S/G lists in DME_PAUSE() is like a do-while() loop. */

	dws->sg_current.xfer_cnt += dws->sg_current.element_ptr->cam_sg_count;

	/* If there is not another S/G element than just leave. */

	if( dws->sg_current.element_cnt == sim_wset->ccb->cam_sglist_cnt )
	{
	    /* This is the end of the S/G list. */

	    return( CAM_REQ_CMP );		/* All done w/the I/O */
	}

	cdat->dat_flags &= ~(DAT_FINAL);	/* clear the flag */

	/* Increment to the next element in the S/G list in onder to
	setup the current XFER_INFO fields. */

	dws->sg_current.element_cnt++;		/* next index */
	dws->sg_current.element_ptr++;		/* next element */

	/* Load the information from the new SG_ELEM into the current
	XFER_INFO structure. */

	dws->current.count = dws->sg_current.element_ptr->cam_sg_count;
	dws->current.vaddr = dws->sg_current.element_ptr->cam_sg_address;

	/* Call the DAT build code to Redo the DAT calculations. */

	if( dcmn_dat_build( dws ) != CAM_REQ_CMP )
	{
	    return( CAM_REQ_CMP_ERR );
	}
    }

    /* All the adjusting has been finished, return to the state machine. */

    return( CAM_REQ_CMP );
}                                           /* END of dme_pause         */

/* ---------------------------------------------------------------------- */
/*
Routine Name: dme_tcds_save

Functional Description:

    This function is called by the HBA when it receives a SAVE DATA
    POINTER message from the target. The routine will save the current
    xfer state of the DME of this descriptor for later use.

Formal Parameters:
   dme_desc          - Address of descriptor that describes this DMA request

Implicit Inputs:             NONE

Implicit Outputs:            NONE

Return Value:                CAM_REQ_CMP - All is well.

Side Effects:                NONE

Additional Information:      NONE

*/

static char *dme_tcds_save_func = "dme_tcds_save()";	/* func name */

U32
dme_tcds_save( dme_desc )
    DME_DESCRIPTOR *dme_desc;
{
    register DMA_WSET *dws;		/* working set pointer */
    SIM_WS *sim_wset;			/* SIM working set for this request */

    /* initialize local variables */
    sim_wset = dme_desc->sim_ws;

    PRINTD(sim_wset->cntlr, sim_wset->targid, sim_wset->lun, 
	(CAMD_INOUT | CAMD_DMA_FLOW),
	("[%d/%d/%d] (dme_tcds_save) Entry - dme_desc=0x%x\n", 
	sim_wset->cntlr, sim_wset->targid, sim_wset->lun, dme_desc ) );

    dws = (DMA_WSET *)dme_desc->segment;	/* get the working set */
    if( dws == (DMA_WSET *)NULL )
    {
	CAM_ERROR( dme_tcds_save_func, "Invalid DME working set pointer.",
	    (CAM_ERR_HIGH), 0, dme_desc, (MEMERR_LOG *)NULL );
        return( CAM_REQ_CMP_ERR );
    }

    /* Move the current xfer info stuff to the saved xfer info struct. */

    dws->saved.count = dws->current.count;	/* save the down count */
    dws->saved.vaddr = dws->current.vaddr;	/* save the vaddr */
    dws->saved.index = dws->current.index;	/* save the DAT index */

    dws->saved.bxfer = dws->current.bxfer;	/* save the total count */

    /* If the I/O contains a S/G list also move the current S/G info into
    the save S/G structure. */

    if( (dme_desc->flags & DME_SG_LIST) != 0 )		/* S/G list */
    {
	/* More work has to be done to "save" the counts and addresses
	with a scatter/gather I/O.  The XFER_INFO structures only keep
	track of the counts for a single S/G element.  The SCATTER_ELEMENT
	structure, saved also has to be updated. */

	dws->sg_saved.element_ptr = dws->sg_current.element_ptr;
	dws->sg_saved.xfer_cnt    = dws->sg_current.xfer_cnt;
	dws->sg_saved.element_cnt = dws->sg_current.element_cnt;
    }

    /* Set the current state to be "saved". */

    dws->wset_state = DME_STATE_SAVE;		/* signal save data pointer */

    return( CAM_REQ_CMP );
}                     /* dme_save */

/* ---------------------------------------------------------------------- */
/*
Routine Name: dme_tcds_restore

Functional Description:

    This routine is called by the HBA in response to a RESTORE POINTERS
    message. This routine will return the state of the DME for this
    request to the last saved DME state.  All of the DAT calculations
    will be redone.  With a restore pointers message any of the
    "previous" xfer state must be discarded.

Formal Parameters:
    dme_desc          - Address of descriptor that describes this DMA request

Implicit Inputs:

    The xfer info structs in the working sets.

Implicit Outputs:

    The DATs are redone as if from setup.

Return Value:                CAM_REQ_CMP - All is well.

Side Effects:                NONE

Additional Information:      NONE

*/

static char *dme_tcds_restore_func = "dme_tcds_restore()";	/* func name */

U32
dme_tcds_restore( dme_desc )
    DME_DESCRIPTOR *dme_desc;
{
    register DMA_WSET *dws;		/* DME working set for the I/O */
    SIM_WS *sim_wset;			/* SIM working set for this request */
    U32 prev_state;			/* what the prev DME state was */
    U32 i;				/* loop counter */

    /* initialize local variables */
    sim_wset = dme_desc->sim_ws;

    PRINTD(sim_wset->cntlr, sim_wset->targid, sim_wset->lun, 
	(CAMD_INOUT | CAMD_DMA_FLOW),
	("[%d/%d/%d] (dme_tcds_restore) Entry - dme_desc=0x%x\n", 
	sim_wset->cntlr, sim_wset->targid, sim_wset->lun, dme_desc ) );

    dws = (DMA_WSET *)dme_desc->segment;
    if( dws == (DMA_WSET *)NULL )
    {
	CAM_ERROR( dme_tcds_restore_func, "Invalid DME working set pointer.",
	    (CAM_ERR_HIGH), 0, dme_desc, (MEMERR_LOG *)NULL );
        return( CAM_REQ_CMP_ERR );
    }

    /* Save the current state value, and then update it to be RESTORE. */

    prev_state = dws->wset_state;		/* save the previous one */
    dws->wset_state = DME_STATE_RESTORE;

    /* Check to see what the previous DME state was.  If the last state
    was either SETUP or SAVE then the DAT contents are still valid.  There
    has been no DME/DATA activity to make the "ahead" calculations 
    useless. */

    if( (prev_state == DME_STATE_SETUP) || (prev_state == DME_STATE_SAVE) )
    {
	/* The calcs are ok. */

	return( CAM_REQ_CMP );		/* all done */
    }

    PRINTD(sim_wset->cntlr, sim_wset->targid, sim_wset->lun, 
	(CAMD_DMA_FLOW),
	("[%d/%d/%d] (dme_tcds_restore) Restoring current, prev_state 0x%x\n",
	sim_wset->cntlr, sim_wset->targid, sim_wset->lun, prev_state ) );

    /* Update the current xfer info struct with the saved info. */

    dws->current.count = dws->saved.count;	/* restore the count */
    dws->current.vaddr = dws->saved.vaddr;	/* restore the vaddr */
    dws->current.index = dws->saved.index;	/* restore the index */
    dws->current.bxfer = dws->saved.bxfer;	/* restore the bxfer */

    /* Re-update the DME_DESCRIPTOR current count to be based on the
    "old" - (now current) byte transfer counter. */

    dme_desc->xfer_count = dws->current.bxfer;	/* ok for linear or S/G */

    /* Check to see if this transfer is a normal linear user buffer or
    a scatter/gather list. */

    if( (dme_desc->flags & DME_SG_LIST) != 0 )		/* S/G list */
    {
	/* More work has to be done to "restore" the counts and addresses
	with a scatter/gather I/O.  The XFER_INFO structures only keep
	track of the counts for a single S/G element.  The SCATTER_ELEMENT
	structures, current and saved also have to be updated. */

	dws->sg_current.element_ptr = dws->sg_saved.element_ptr;
	dws->sg_current.xfer_cnt    = dws->sg_saved.xfer_cnt;
	dws->sg_current.element_cnt = dws->sg_saved.element_cnt;
    }

    /* Call the DAT build code to Redo the DAT calculations. */

    if( dcmn_dat_build( dws ) != CAM_REQ_CMP )
    {
	return( CAM_REQ_CMP_ERR );
    }

    return( CAM_REQ_CMP );		/* all done */
}                     /* dme_restore */

/* ---------------------------------------------------------------------- */
/*
Routine Name: dme_tcds_end

Functional Description:

    This function is called by the HBA at the end of each request in
    order to deallocate any DME resources as well as flush any pending
    buffer segments before completeing the request. This function MUST
    be called by the HBA if dme_setup() has been called.

Formal Parameters:
   dme_desc          - Address of descriptor that describes this DMA request

Implicit Inputs:             NONE

Implicit Outputs:            NONE

Return Value:                CAM_REQ_CMP - All is well.

Side Effects:
    The HBA must call DME_PAUSE prior to executing this function.
    DME_PAUSE prepares the SEGMENT ELEMENT for DME_END.

Additional Information:      NONE

*/
U32
dme_tcds_end( dme_desc )
    DME_DESCRIPTOR *dme_desc;
{
    SIM_SOFTC* sc;                      /* Pointer to SIM S/W control struct */
    DMA_CTRL *dctrl;			/* ptr for the control structure */
    register DMA_WSET *dws;		/* DME specific data struct */
    SIM_WS *sim_wset;			/* SIM working set for this request */
    int s;				/* for the control struct locking */
    
    /* Setup required local variables.  */
    sim_wset = dme_desc->sim_ws;

    dws = (DMA_WSET *)dme_desc->segment;
    if( dws == (DMA_WSET *)NULL )
    {
	/* Simply return to the State Machine.  The code is being asked to
	DME_END() on a Non-DME setup I/O.  Oh well ! */

        return( CAM_REQ_CMP );
    }

    sc = GET_SOFTC_DESC( dme_desc );			/* get the softc */
    dctrl = (DMA_CTRL *)sc->dme->extension;		/* get the control */

    PRINTD(sim_wset->cntlr, sim_wset->targid, sim_wset->lun, 
	(CAMD_INOUT),
	("[%d/%d/%d] (dme_tcds_end) Entry - dme_desc=0x%x.\n", 
	sim_wset->cntlr, sim_wset->targid, sim_wset->lun, dme_desc ) );

    dws->wset_state = DME_STATE_END;     /* Update last DME state */

    /* Clear the DME active bit, so that that SIM HBA doesn't think
    that any future interrupts are from the DME.  */

    ((SIM_WS *)dme_desc->sim_ws)->flags &= ~SZ_DME_ACTIVE;

    /* Clear the segment element from the DME structure. */

    dme_desc->segment = (void *)NULL;

    /* Release all of the DME resources, the DATs and working set. */

    D3CTRL_IPLSMP_LOCK( s, dctrl );		/* lock on control */
    (void)dcmn_rtn_resources( dws );
    D3CTRL_IPLSMP_UNLOCK( s, dctrl );		/* unlock control */

    /* There has to be a check to determine if the SCSI device has
    used Modify Data Pointer, MDP, to manipulate the transfer.  If the
    DME_MODIFY() code was called the MDP flag will be set in the DME
    descriptor flags.  With this flag set the DME has to "trust" the
    SCSI device that all the data has been transfered.  The original
    transfer count is moved into the ending transfer count. */

    if( (dme_desc->flags & DME_MDP_FUBAR) != 0 )
    {
	dme_desc->xfer_count = dme_desc->data_count;	/* trust the device */
    }

    /* With the DME_PAUSE() routine having already been called the xfer 
    count in the DME descriptor is up to date.  All that has to be done
    is to set the DME_DONE flag. */

    dme_desc->flags |= DME_DONE;	 /* signal that the I/O is done. */

    return( CAM_REQ_CMP );
}                       /* dme_end */

/* ---------------------------------------------------------------------- */
/*
Routine Name: tcds_dme_attach

Functional Description:

    This function is called during the initialization sequence in the SIM
    to configure the DME by loading the appropriate routines into the the
    HBA_DME_CONTROL structure in the sim_softc. This function is provided
    to allow the Data Mover Engine to be configured based on the
    capabilities of the underlying HBA.

    Currently, this routine supports the TCDS DME.

Formal Parameters:           SIM_SOFTC* sc - Addr of SIM cntlr strc

Implicit Inputs:             None

Implicit Outputs:            sc->dme initialized.

Return Value:                CAM_REQ_CMP - All is well.

Side Effects:                This function calls dme_init, to init the DME

Additional Information:      None

*/

static char *dme_tcds_attach_func = "dme_tcds_attach()";	/* func name */

U32
tcds_dme_attach( sc )
    SIM_SOFTC *sc;
{
    register volatile SIM94_REG *reg = (SIM94_REG *)sc->reg;
    register SIM94_SOFTC *ssc = (SIM94_SOFTC *)sc->hba_sc;
    DME_STRUCT *dme;

    PRINTD( sc->cntlr, NOBTL, NOBTL, (CAMD_INOUT),
	("[%d/t/l] (dme_tcds_attach) entry - sc=0x%x\n", sc->cntlr, sc ));

    /* Check to make sure that the DME attachment structure can be allocated
    and the calls be inserted. */

    if( (dme  = dcmn_alloc_dme()) == (DME_STRUCT *)NULL )
    {
	CAM_ERROR( dme_tcds_attach_func,
	    "Unable to alloc memory for DME Attach Struct.",
	    (CAM_ERR_HIGH), 0, (DME_DESCRIPTOR *)NULL, (MEMERR_LOG *)NULL );
	return( CAM_REQ_CMP_ERR );
    }

    /* Store the clock speed.  To be used when setting up the
    synchronous period.  Store the clock * 10 to allow for fractions. */

    ssc->clock = tcds_get_clock( sc );	/* get clock rate in MHz * 10 */

    /*
     * Set the clock conversion register for TCDS's speed (using the
     * real crystal speed).
     */
    reg->sim94_ccf = SIM94_CONVERT_CLOCK( ssc->clock/10 );

    /* Calculate the minimum period.  The value which is used to
    negotiate synchronous is the actual period divided by four.
    Convert the clock speed from MHz to nanoseconds by dividing 1000
    by the clock speed (10000/(clock*10)).  */

    sc->sync_period = ((10000 / (ssc->clock)) * tcds_get_period( sc )) / 4;

    /* Get the csr address for the ASIC.  This value was saved during
     * probe so we could get to it later.  It would be a big pain to
     * have to calculate it based on the register address of the C94
     * since there are two of them sharing the same ASIC.
     */

    dme->hardware = (void *)&((TCDS_ASIC *)sc->csr_probe)->dma_cir;

    dme->vector.dme_init	 = dme_tcds_init;
    dme->vector.dme_setup	 = dme_tcds_setup;
    dme->vector.dme_start	 = dme_tcds_resume;
    dme->vector.dme_end		 = dme_tcds_end;
    dme->vector.dme_pause	 = dme_tcds_pause;
    dme->vector.dme_resume	 = dme_tcds_resume;
    dme->vector.dme_save	 = dme_tcds_save;
    dme->vector.dme_restore	 = dme_tcds_restore;
    dme->vector.dme_modify	 = dme_tcds_modify;
    dme->vector.dme_copyin	 = (U32 (*)())bcopy;
    dme->vector.dme_copyout	 = (U32 (*)())bcopy;
    dme->vector.dme_clear	 = (U32 (*)())bzero;
    
    /* Attach the allocated struct to the softc now that it is filled in. */

    sc->dme = dme;

    /* Call the initialization code for this module. */

    if( dme_tcds_init( sc ) != CAM_REQ_CMP )
    {
        return( CAM_REQ_CMP_ERR );
    }

    return( CAM_REQ_CMP );
}                  /* dme_attach */

/* ---------------------------------------------------------------------- */
/*
Routine Name: tcds_dme_unload

Functional Description:

    A place holder for the DME unload functionality.

Formal Parameters:

Implicit Inputs:
      NONE

Implicit Outputs:
     NONE

Return Value:
      CAM_REQ_CMP		- All is well
      CAM_REQ_CMP_ERR	- DME failed to initialize

Side Effects:
      NONE

Additional Information:
      NONE
*/

U32
tcds_dme_unload()
{
    return CAM_REQ_CMP;
}

/* ---------------------------------------------------------------------- */
/*
Routine Name: dme_tcds_modify

Functional Description:  In a nut shell: "It don't work!"

Formal Parameters:           NONE

Implicit Inputs:             NONE

Implicit Outputs:            NONE

Return Value:                TBD

Side Effects:                NONE

Additional Information:      NONE

*/
U32
dme_tcds_modify ( dme_desc, offset )
    DME_DESCRIPTOR *dme_desc;
    I32 offset;
{
    SIM_SOFTC *sc;                     /* Pointer to SIM S/W control struct  */
    SIM_WS *sim_wset;                  /* SIM working set for this request   */

    /* If a bad pointer is passed in, return an error. */
    if ( !dme_desc || !dme_desc->segment || !dme_desc->sim_ws )
	return CAM_REQ_CMP_ERR;

    /* initialize local variables */
    sim_wset = dme_desc->sim_ws;
    sc = GET_SOFTC_DESC( dme_desc );

    PRINTD(sim_wset->cntlr, sim_wset->targid, sim_wset->lun, 
	(CAMD_INOUT | CAMD_DMA_FLOW | CAMD_ERRORS),
	("[%d/%d/%d] (dme_tcds_modify) Not fully supported\n",
	sim_wset->cntlr, sim_wset->targid, sim_wset->lun ));
    
    /* JAG - TODO ** Right now modify will cause error handling to kick in */

    return( CAM_REQ_CMP_ERR );
}                       /* dme_modify */

/* ---------------------------------------------------------------------- */
/*
Routine Name: asic_pointer_load

Functional Description:
    This routine will load the physical address of the data from the
    DAT entry into the ASICs address register.  The address is shifted
    to deal with the format of the ASICs addresses.

Formal Parameters:
    The SIM_SOFTC pointer to get the ASIC base address.
    The DAT element that contians the physical address.

Implicit Inputs: None

Implicit Outputs:
    The ASIC SCSI DMA address register.

Return Value:
    CAM_REQ_CMP		All went well

Side Effects:
    The ASIC address register is loaded with a new value.  The address bits
    in the SDIC register are cleared.

Additional Information:
    The format of the ASIC SCSI DMA address register looks like:

    31                               0
    +---------------------------------+
    |          PADDR 33:02            |
    +---------------------------------+
    The physcal address from the DAT needs to have the low order 2 bits
    masked off, due to TurboChannel only addressing 32 bit words, and byte
    address bits are not used.  Then the address bits 33:02 need to be
    shifted down into the register bits 31:00, effectivly shifting PA bit 02
    into Reg bit 00.

    For the description of the SCSI DMA Inerrupt/Control register see
    the "Additional Information" in the tcds_dma_enable() routine
    header.

*/

U32
asic_pointer_load( sc, pa )
    SIM_SOFTC *sc;		/* contains the ASIC base address */
    register void *pa;		/* the address to load */
{
    U32 ioaddr;			/* holds the converted address */
    volatile U32 *sdic;		/* SCSI DMA Interrupt/Control Reg */

    PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	("[%d/t/l] (asic_pointer_load) - Entry, pa = 0x%x\n", sc->cntlr, pa));

    /* Set the pointer to the ASIC SCSI DMA I/C register */

    sdic = TCDS_DMA_SDIC_PTR(sc);	/* get the pointer */

    /* Get the normal format physical address from the DAT entry, convert
    it to the ASIC format and load the address into the ASIC register.
    See "Additional Information" above. */

    ioaddr = (((vm_offset_t)pa & IOA_ADDRMASK) >> 2);	/* mv bit 02 into 00 */

    *TCDS_DMA_ADDR_PTR(sc) = ioaddr;			/* load the Phys addr */
    WBFLUSH();

    *sdic &= ~(SDIC_ABITS_MASK);	/* clear the addr bits 01:00 */
    WBFLUSH();

    return( CAM_REQ_CMP );				/* all done */
}

/* ---------------------------------------------------------------------- */
/*
Routine Name: asic_pointer_fetch

Functional Description:
    This routine will return the physical address value contained in the
    ASIC address register.  The address from the register is shifted
    and masked to return a "normal" byte address from the format of
    the ASICs addresses.

Formal Parameters:
    The SIM_SOFTC pointer to get the ASIC base address.

Implicit Inputs:
    The ASIC physical address register in the common area.

Implicit Outputs: None

Return Value:
    A normal physical address.

Side Effects: None

Additional Information:
 
*/

void *
asic_pointer_fetch( sc )
    SIM_SOFTC *sc;
{
    register U32 paddr;			/* holder for the address */

    /* Get the address for the DMA ptr. */

    paddr = *TCDS_DMA_ADDR_PTR(sc);	/* copy the register contents */ 

    /* Shift the address bit 02 from 00, and then mask out the 63:34
    bits.  SEE the "Addtional Information" header section in
    asic_pointer_load(). */

    paddr <<= 2;                        /* move 02 up from 00 */
    paddr &= IOA_ADDRMASK;              /* remove the extra bits */

    return( (void *)paddr);		/* return the normalized address */
}

/* ---------------------------------------------------------------------- */
/*
Routine Name: tcds_dma_enable

Functional Description:
    This routine will set the ASIC control interrupt register to enable 
    SCSI DMA and set/unset the DMA direction bit.  The TCDS read prefetch
    is setup if enabled for the I/O.

Formal Parameters:
    The SIM_SOFTC pointer for the ASIC base address.
    A DME working set containing the direction for the I/O

Implicit Inputs: None

Implicit Outputs:
    The ASIC SCSI DMA Interrupt/Control register is modified.

Return Value: None

Side Effects:
    The ASIC is enable for DMA.

Additional Information:
    This routine MUST be called with the SIM register access locked.

    There are a number of routines that access the shared control byte in
    the SDIC register.  The ASIC pointer routines make use of the address
    bits and this enable routine makes use of the direction and read
    prefetetch bits.  It is necessary to only use the &= and |= operators
    on this register.


SCSI DMA Interrupt Control Register:

    3         2 2        1 1        0 0        0
    1         4 3        6 5        8 7        0
   +-----------+----------+----------+----------+
   | Intr byte | FIFO Flgs|  Status  | Control  |
   +-----------+----------+----------+----------+

The Control byte looks like:
        7    6              1    0
   +------------------------------+
   | Dir | RP | X X X X | 01 | 00 |
   +------------------------------+
   Dir: Direction of the DMA xfer
   RP : Read Prefetch enabled
   10 & 00 : low order address bits for the byes in Unaligned Reg X

*/

static void
tcds_dma_enable( sc, dws )
    SIM_SOFTC *sc;			/* contains the register address */
    DMA_WSET *dws;			/* contains the direction */
{
    register volatile U32 *sdic;	/* pointer to ASIC SDIC */

    PRINTD( sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
       ("[%d/t/l] (tcds_dma_enable) -  Entry\n", sc->cntlr));

    sdic = TCDS_DMA_SDIC_PTR(sc);	/* get the pointer */

    /* Based on the direction setting in the working set, set the direction
    bit in the SCSI DMA Interrupt/Control register. */

    if( dws->dir == ASIC_RECEIVE )		/* SCSI DATA IN Phase */
    {
	/* Set the direction bit for a read and set the enable bit. */
	*sdic |= (SDIC_DMADIR);			/* write TO memory */
    }
    else if( dws->dir == ASIC_TRANSMIT )	/* SCSI DATA OUT Phase */
    {
	/* Clear the direction bit for a write and set the enable bit. */
        *sdic &= ~(SDIC_DMADIR);		/* read FROM memory */

	/* Set the TCDS Read prefetch functionality based on the settings in
	the working set.  The read prefetch can only be used with the I/O
	mapping enabled. */

	TCDS_PREFETCH( sc, dws );		/* set the feature */
    }
    WBFLUSH();				/* make sure the data gets out */

    /* Set the SCSI DMA enable bit, now the ASIC will respond to 
    DREQ from the SCSI chip. */

    TCDS_CIR_DMA_ENABLE( sc );		/* set the bit in the CIR */
    WBFLUSH();
}

/* ---------------------------------------------------------------------- */
/*
Routine Name: tcds_dma_disable

Functional Description:
    This routine will clear the ASIC control/interrupt register to disable 
    SCSI DMA for the particular '94 chip.

Formal Parameters:
    The SIM_SOFTC pointer for the ASIC base address.

Implicit Inputs: None

Implicit Outputs:
    The ASIC control/interrupt register is modified.

Return Value: None

Side Effects:
    The ASIC is disable for DMA.

Additional Information:
    This routine MUST be called with the SIM register access locked.
    The TCDS ASIC control/interrupt register is one of the shared
    ASIC registers between the two SCSI controllers.  A macro is used
    to "hide" this sharing.

    It is highly likely that this routine could become only a MACRO.
    It is still being kept alive to have both an enable and disable
    matched pair of support routines.
*/

void
tcds_dma_disable( sc )
    SIM_SOFTC *sc;			/* hold the ASIC base addr */
{
    PRINTD( sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
       ("[%d/t/l] (tcds_dma_disable) -  Entry\n", sc->cntlr));

    /* Clear the SCSI DMA enable bit, now the ASIC will not respond to any
    DREQ from the SCSI chip. */

    TCDS_CIR_DMA_DISABLE( sc );		/* clear the bit in the CIR */
    WBFLUSH();				/* wait for the bits to go out */
}

/* ---------------------------------------------------------------------- */
/*
Routine Name: get_asic_count

Functional Description:
    This routine will return the number of bytes "stuck" in the ASICs
    SCSI DMA unaligned data register 1.  The Address bits in the SCSI
    DMA Control register are used for the count.  The represent the
    low order address bits from the physical address register for the
    "next" address, if the DMA xfer was to continue.  The DMA method used
    by this DME these bits can also be used for the "count" of the number
    of bytes that are in the ASIC.

Formal Parameters:
    The SIM_SOFTC pointer to get the ASIC base address.

Implicit Inputs:
    The ASICs SCSI DMA control register

Implicit Outputs: None

Return Value:
    The number of bytes stuck in there.

Side Effects: None

Additional Information:

    The method of data transfer in this DME file used will always have
    the data xfer *Start* on an aligned boundary.  With this method the
    first unaligned data register is not used.  The layout of the
    unaligned data register is described below. For the description of
    the SCSI DMA Inerrupt/Control register see the "Additional
    Information" in the tcds_dma_enable() routine header.

    This routine will ALWAYS return a even number of bytes.  Due to the
    16bit data path between the ASIC and the '94 chip, when a single
    byte is transfered from the '94 a full 16 bits are "clocked" in.

Unaligned Register 1:

    3         2 2        1 1        0 0        0
    1         4 3        6 5        8 7        0
   +-----------+----------+----------+----------+
   | Mask byte | byte[10] | byte[01] | byte[00] |
   +-----------+----------+----------+----------+

*/

U32 
get_asic_count( sc )
    SIM_SOFTC *sc;			/* contains the ASIC Reg addresses */
{
    U32 unalign0;			/* To hold the unaligned 0 contents */

    /* For Error checking check the contents of the Unaligned 0 register and
    report an error if there appears to be anything in there. */

    unalign0 = *(TCDS_DMA_UNALN0_PTR(sc));
    if( unalign0 & UNALIGN_VALID_MASK )
    {
	/* JAG - To Do - Make this into a real error message. */
	printf( "(get_asic_count) Unalign Reg 0 contains valid data: 0x%x\n" );
    }

    /* Simply return the address bits from the SDIC register as the count. */

    return( (*TCDS_DMA_SDIC_PTR(sc) & SDIC_ABITS_MASK) );
}

/* ---------------------------------------------------------------------- */
/*
Routine Name: tcds_flush_asic

Functional Description:
    This routine will remove any of the users data bytes from the ASIC
    SCSI data alignment registers and move them into the physical address
    contained in the ASIC SCSI address register.  The bytes are moved
    from the I/O ASIC alignment register into the users destination buffer.

Formal Parameters:
    The pointer to the SIM_SOFTC structure containing the ASIC base addr.

Implicit Inputs:
    The ASICs SCSI unaligned data register 1, and address register.

Implicit Outputs:
    The physical address in the ASIC will receive the data bytes.

Return Value:
    The physical address of the final resting place for the ASIC unaligned
    bytes.

Side Effects:
    All four bytes of the users data space is modified.  As described in
    the thesis at the top of the file it is OK that the extra bytes are
    "trashed".

Additional Information:
    This routine MUST be called with the SIM register structure locked.
    This code will have to be "expanded" when the mapping register support
    is implemented.  At that point the "physical address" in the ASIC isn't
    anymore.  There will have to be some address calculations done using
    the Kernel Virtual address and transfer count.  For this the DME
    working set is being passed in, but for now is not used.

*/

static void *
tcds_flush_asic( sc, dws )
    SIM_SOFTC *sc;			/* for the ASIC address */
    DMA_WSET *dws;			/* DME specific data struct */
{
    void *padr;                         /* physical address of user buffer */
    volatile U32 *sdic;			/* For accessing addr bits 01:00 */
    U32 count;				/* temp storage of the count */
    U32 unalign_1;			/* ending unaligned data bytes */
    volatile U32 *dest;			/* Where the left over bytes belong */

    /* Setup the pointers using the TCDS address information in the SOFTC. */

    unalign_1 = *(TCDS_DMA_UNALN1_PTR(sc));
    sdic = (volatile U32 *)TCDS_DMA_SDIC_PTR(sc); 

    /* Save the current count, from the control reg addr bits. JAG */ 

    count = *sdic & SDIC_ABITS_MASK;

    /* Get the physical address from ASIC's address pointer.  The two 
    address bits in the control register are cleared at this point.  Once
    the bytes have been flushed, these address bytes can not be left
    around any more.  If the bits were left in they would affect the address
    DMA cycles for the next DMA. */

    padr = (void *)asic_pointer_fetch( sc );	/* get physical address */
    *sdic &= ~(SDIC_ABITS_MASK);		/* clear the addr bits 01:00 */

    /* Convert the physical address into a kernel virtual address for the 
    CPU data xfer. */

    dest = (U32 *)CAM_PHYS_TO_KVA( padr );

    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_DMA_FLOW,
    ("[b/t/l/] (tcds_flush_asic) - ASIC unaligned bytes: 0x%x, dest: 0x%x.\n",
	unalign_1, dest ) );

    /* Simply move all of the bytes into the destination. */

    *dest = unalign_1;			/* Send out all four bytes always */

    /* Return to the caller where the bytes went. */
    return( padr );
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : dme_tcds_errlog

Functional Description :

    Local error logging routine for the CDrv.  Using the arguments from the 
    caller a system error log packet is filled with the error information and 
    then stored within the system.

Formal Parameters :
    The six arguments via the macro:

	fs : The Function name string
	ms : The message string
	ef : Relivant error flags
	dd : The I/O DME descriptor if not NULL
	ci : The interrupting system SCSI controller, if DME_MEM_ERR is valid.
	pa : The physical address on a MEM error, if DME_MEM_ERR is valid.

Implicit Inputs : None

Implicit Outputs : Error Log Structures.
 
Return Value : None

Side Effects : A Lot of Error log work.

Additional Information :

*/

/* ARGSUSED */
void
dme_tcds_errlog( fs, ms, ef, df, dd, ml )
    char *fs;				/* Function Name string */
    char *ms;				/* The message string */
    U32 ef;				/* Relivant Logger error flags */
    U32 df;				/* DME error flags */
    DME_DESCRIPTOR *dd;			/* The DME working set if not NULL */
    MEMERR_LOG *ml;			/* pointer to a memory error log */
{
    CAM_ERR_HDR err_hdr;		/* Local storage for the header */
    CAM_ERR_HDR *eh;			/* pointer for accessing it */

    CAM_ERR_ENTRY err_ent[DME_TCDS_ERR_CNT];/* the local error entry list */
    CAM_ERR_ENTRY *cee;			/* error entry pointer */

    DMA_WSET *dws;			/* for the I/O working set */

    U32 ent;				/* current entry number in the list */
    U32 i;				/* loop counter */

    int controller = NO_ERRVAL;		/* Controller of current log */
    int target = NO_ERRVAL;		/* Target of current log */
    int lun = NO_ERRVAL;		/* LUN of current log */

    /* Setup the local variables. */

    ent = 0;				/* first entry */

    /* Assign values to controller/target/lun if available.   The error flag
    DME_ERR_MEMERR has priority over the DME descriptor check. */

    if( (df & DME_ERR_MEMERR) != 0 )	/* contains a memory error log */
    {
	controller = ml->controller;
    }
    else if( dd != (DME_DESCRIPTOR *)NULL )	/* a dme_desc I/O error log */
    {
	if( (SIM_WS *)dd->sim_ws != (SIM_WS *)NULL )/* sanity check SIM_WS */
	{
	    controller = ((SIM_WS *)dd->sim_ws)->cntlr;
	    target = ((SIM_WS *)dd->sim_ws)->targid;
	    lun = ((SIM_WS *)dd->sim_ws)->lun;
	}
    }

    /* Clear out the header structure and the Error structures. */

    bzero( &err_hdr, sizeof(CAM_ERR_HDR) );	/* Zero fill the header */

    for( i = 0; i < DME_TCDS_ERR_CNT; i++ )
    {
	bzero( &err_ent[i], sizeof(CAM_ERR_ENTRY) );	/* fill the entry */
    }

    /* Setup the Error log header. */

    eh = &err_hdr;			/* pointer for the header */

    eh->hdr_type = CAM_ERR_PKT;		/* A CAM error packet */
    eh->hdr_class = CLASS_DME;		/* the DME class of errors */
    eh->hdr_subsystem = SUBSYS_TCDS_DME;/* fill in the subsystem field */
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
	cee->ent_data = (u_char *)fs;		/* point to it */
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

    /* Setup the Next Error entry on the list.  If this is a ASIC memory
    read error, DME_ERR_MEMERR, this entry contains the physical addresst
    that was taken from the ASIC's SCSI DMA data address register. */

    cee = &err_ent[ ent ];		/* set the pointer */

    if( (df & DME_ERR_MEMERR) != 0 ) 
    {
	cee->ent_type = ENT_DME_PHYADDR;	/* this entry is the addr */
	cee->ent_size = sizeof(void *);		/* how big is a pointer */
	cee->ent_vers = DME_MEMERR_VERS;	/* current version */
	cee->ent_data = (u_char *)ml->pa;	/* point to it */
	cee->ent_pri = PRI_FULL_REPORT;		/* structures are full */
	ent++;					/* on to the next entry */
    }

    /* Setup the Next Error entries on the list.  These entries will contain 
    the DME descriptor, DME working set, and DAT for the I/O. */

    cee = &err_ent[ ent ];		/* set the pointer */

    if( dd != (DME_DESCRIPTOR *)NULL )		/* make sure there is one */
    {
	cee->ent_type = ENT_DME_DESCRIPTOR;	/* this entry is the desc */
	cee->ent_size = sizeof(DME_DESCRIPTOR);	/* how long it is */
	cee->ent_vers = DME_DESCRIPTOR_VERS;	/* current version */
	cee->ent_data = (u_char *)dd;		/* point to it */
	cee->ent_pri = PRI_FULL_REPORT;		/* structures are full */
	ent++;					/* on to the next entry */
    }

    /* Setup the Next Error entry on the list.  This entry will contain 
    the DME working set. */

    cee = &err_ent[ ent ];		/* set the pointer */

    if( dd != (DME_DESCRIPTOR *)NULL )		/* make sure there is one */
    {
	dws = (DMA_WSET *)dd->segment;

	if( dws != (DMA_WSET *)NULL )		/* a valid working set */
	{
	    cee->ent_type = ENT_DMA_WSET;	/* this entry is the wrk set */
	    cee->ent_size = sizeof(DMA_WSET);	/* how long it is */
	    cee->ent_vers = DMA_WSET_VERS;	/* current version */
	    cee->ent_data = (u_char *)dws;	/* point to it */
	    cee->ent_pri = PRI_FULL_REPORT;	/* structures are full */
	    ent++;				/* on to the next entry */
	}
    }

    /* Setup the Next Error entry on the list.  This entry will contain 
    the DME working set DAT strucuture if the DME_ERR_LOGDAT flag is set. */

    cee = &err_ent[ ent ];		/* set the pointer */

    if( (dws != (DMA_WSET *)NULL) && ((df & DME_ERR_LOGDAT) != 0) )
    {
	cee->ent_type = ENT_DME_DAT;		/* this entry is the DAT ring */
	cee->ent_size = (sizeof(DAT_ELEM) *
			    dws->de_count);	/* how many there are */
	cee->ent_vers = DME_DAT_VERS;		/* current version */
	cee->ent_data = (u_char *)dws->de_base;	/* point to it */
	cee->ent_pri = PRI_FULL_REPORT;		/* structures are full */
	ent++;					/* on to the next entry */
    }

    /* Setup the Next Error entry on the list.  This entry will contain 
    the DME DAT attach strucuture from the DME working set if the
    DME_ERR_LOGDAT flag is set. */

    cee = &err_ent[ ent ];		/* set the pointer */

    if( (dws != (DMA_WSET *)NULL) && ((df & DME_ERR_LOGDAT) != 0) )
    {
	cee->ent_type = ENT_DME_ATTCH;		/* this is the DAT attch */
	cee->ent_size = sizeof(DAT_ATTCH);	/* how long it is */
	cee->ent_vers = DME_ATTCH_VERS;		/* current version */
	cee->ent_data = (u_char *)dws->dat_grp;	/* point to it */
	cee->ent_pri = PRI_FULL_REPORT;		/* structures are full */
	ent++;					/* on to the next entry */
    }

    /* Call the CAM Error logger handler with the error structures. */

    eh->hdr_entries = ent;		/* signal how many valid entries */

    cam_logger( eh, (char)controller, (char)target, (char)lun ); 

    return;				/* all done */
}

/* ---------------------------------------------------------------------- */

/* END OF FILE */





