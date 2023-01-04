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
static char *rcsid = "@(#)$RCSfile: dme_pmax_sii_ram.c,v $ $Revision: 1.1.3.5 $ (DEC) $Date: 1992/06/25 17:48:58 $";
#endif

/**
 * FACILITY:
 * 
 *	ULTRIX SCSI CAM SUBSYSTEM
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
 * In the existing DEC HBA's, typically the SCSI chip has a
 * limited address space and therefore only has R/W access to a 128KB DMA
 * buffer. All data that is written or read from/to the SCSI bus must
 * pass through this buffer. Although DEC's current HBA's support DMA,
 * frequently there are restrictions on data alignment or limitations on
 * the address space of the HBA.
 *
 * The result of having this intermediate DMA buffer is that
 * the CPU must move data to this buffer on writes and move data out of
 * this buffer on reads. In the ULTRIX CAM architecture, the management
 * and use of this buffer and other DMA resources is delegated to the
 * SIMH which relies on the SIM DME to provide this support. The address
 * indexing, byte ordering or page alignment restrictions that apply to
 * the intermediate buffer vary from implementation to implementation.
 * The DME hides the details of the underlying HBA's data mover
 * mechanisms by abstracting the actual DMA into a series of distinct steps.
 * 
 * Pseudo DMA (PDMA) which is the basis for the SIM DME divides up the 
 * intermediate DMA buffer into a series of segments or pipes.
 * These segments are divided into 2 buffers. Each of these buffers are
 * used in concert to implement a double buffer scheme which allows the
 * DME to handle arbitrarily large transfers without throttling SCSI bus
 * throughput.
 *
 * For each active device there is one segment of the intermediate DMA buffer
 * allocated, typically a segment is 16KB in size. When a write operation is 
 * to occur, the driver (PDMA code) copies the first 8KB of the transfer to 
 * the first buffer of the segment and then starts the DMA operation. While 
 * the first buffer is being DMAed, the processor is filling the second 
 * segment of the buffer. When the DMA of buffer 0 completes the DMA of 
 * buffer 1 is started. 
 * This ping- ponging continues until the entire request has
 * been transferred.  SIM DME works very much like the existing PDMA code
 * does, except for the fact that in the DME segments and buffer are
 * allocated dynamically on a request by request basis and that there are
 * several pools from which segments are allocated from. In addition, DME
 * has extended support for save and restore pointers messages, that
 * allows the SIMH to modify DME pointers through a defined interface.
 *
 * The DME model has broken down data transfer operations into
 * 8 distinct steps or primitives: 
 * 
 * dme_init - Initialize DME subsystem, resource queues and control 
 * structures.
 * dme_setup - Allocate required DMA resources and preload buffer on write.
 * dme_resume - Continue (start/restart) DMA after terminal count or PAUSE 
 *		and fill or clear next buffer.  
 * dme_pause - Instructs the DME to pause the DMA to be resumed later.  
 * dme_save - Instructs the DME to save the current pointers
 * dme_restore - Instructs the DME to restore the current pointers
 * dme_end - Complete the DMA transaction, release any buffers, move residual
 *	     data to user space, and return actual bytes sent or received.
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
 *	Richard L. Napolitano	July 1, 1990
 *
 * MODIFIED BY:
 * 
 *    03/12/92	dallas	Added ram guard area for sii problem where it
 *			corrupts next segments first short...
 *    01/10/92  rln     Remove double allocation of DME struct during init.
 *    12/12/91  rln     For odd last bytes copy last to user buffer rather than
 *			the RAM buffer.
 *    12/03/91  rln     Handle odd byte as last byte of data in DME_END,PAUSE.
 *    11/20/91  rln     Fix merge/build problem of 4.2 version and allow dme
 *			resume to check for NULL segment and return an error.
 *    11/19/91  rln     Merge in use of 16KB buffer for large transfers.
 *    11/01/91  janet   Moved Mipsmate functions to dme_mipsmate_sii_ram.c
 *    10/29/91  rln     Fix merge problem
 *    10/28/91  rln     Change DME to use only 1KB and 8KB segments.
 *    10/25/91  rln     Make allocate_segment DME specific.
 *    10/24/91  janet   Added Mipsmate attach and deatach functions.
 *    10/22/91  janet	Added include of scsi_all.h and sim_target.
 *    10/12/91	rln	PreEFT changes: 1) Change get_segment => ramsii_get_seg
 *    10/11/91	rln	First pass sii support.
 *    07/20/91  rps     Changed references to HBA_DME_CONTROL to indirect
 *			  through pointer.
 *    07/02/91  rps     Changed routine names to help dbx.
 *
 **/


/* ---------------------------------------------------------------------- 
 *
 * Include files.
 *
 */
#include <io/common/iotypes.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/conf.h>
#include <sys/user.h>
#include <sys/file.h>
#include <sys/map.h>
#include <sys/vm.h>
#include <sys/dk.h>
#include <sys/proc.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include <sys/cmap.h>
#include <sys/uio.h>
#include <io/common/devio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <io/cam/dec_cam.h>
#include <mach/vm_param.h>
#include <io/cam/cam.h>
#include <io/cam/scsi_all.h>
#include <dec/binlog/errlog.h>	/* UERF errlog defines */
#include <io/cam/cam_logger.h>
#include <io/cam/scsi_status.h>
#include <kern/lock.h>
#include <machine/machparam.h>
#include <io/cam/cam_debug.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/dme.h>			/* DME specific structs and consts */
#include <io/cam/sim.h>			/* SIM specific structs and consts */
#include <io/cam/sim_sii.h>		/* DEC SII specific definitions */
#include <io/cam/sim_common.h>
#include <io/cam/dme_pmax_sii_ram.h>
#include <io/cam/cam_errlog.h>
/* ---------------------------------------------------------------------- 
 *
 * Local defines:
 * 
 */

/*
 * If we are building a debuggable kernel, then don't use registers.
 */
/*#define CAMDEBUG 0x10000000*/
#ifndef CAMDEBUG 
#define REGISTER  register 
#else
#define REGISTER 
#endif

/*
 * For the SII allocate only 1KB and 4Kb segments. 
 *
 * Later the distribution of these parameters will be made soft. We will base
 * the buffer size distribution on the page size and default file system record
 * size.
 * These need to be soft to allow them to be selectable for the different 
 * DME's.
 */
#define NUM_1KB 4		/* Number of 1KB segments */
#define NUM_4KB 0		/* Number of 4KB segments */
#define NUM_8KB 15		/* Number of 8KB segments */
#define NUM_16KB 0		/* Number of 16KB segments */

#define RAM_GUARD 16		/* Work around for sii ram buffer
				 * Problem where it clobbers the next
				 * location in the buffer when it should
				 * not... Now reserve 6 bytes at end of
				 * each segment..
				 */

#define B1KB 1024		/* How many bytes in one kilobyte etc... */
#define B2KB 2048
#define B4KB 4096
#define B8KB 8192
#define B16KB 16384
#define B128KB 131072

/* ---------------------------------------------------------------------- 
 *
 * External declarations:
 *
 */
extern struct timeval sxtime;    	/* System time data cell          */

/*
 * For debug of scatter gather, create a local scatter gather list.
 */
SG_ELEM sglist[10];

/* ---------------------------------------------------------------------- 
 *
 * Function Prototypes:
 */

U32 ramsii_get_segment();
U32 ramsii_allocate_segments();
U32 ramsii_dme_copyin();
U32 ramsii_dme_copyout();
U32 ramsii_dme_init();
U32 ramsii_dme_setup();
U32 ramsii_dme_end();
U32 ramsii_dme_pause();
U32 ramsii_dme_resume();
U32 ramsii_dme_save();
U32 ramsii_dme_restore();
U32 ramsii_dme_modify();
U32 ramsii_dme_clear();
U32 ramsii_dme_map_copy_write();
U32 ramsii_dme_map_buf();
U32 ramsii_dme_init_segment();
U32 ramsii_dme_reinit_segment();
U32 ramsii_dme_bump_sglist();
U32 ramsii_dme_toggle_buffer();
U32 ramsii_dme_buffer_map_move();    
U32 cam_wmbcopy();
U32 cam_rmbcopy();
U32 cam_wmbzero();
static char* allocate_list();
static U32 free_segment();		/* Release a segment of DMA buffer */
static U32 get_seg_sub();

/* ----------------------------------------------------------------------
 * 
 * Initialized and uninitialized data: 
 * 
 */


/*---------------------------------------------------------------------- 
 *
 * Function Prototypes: 
 *
 */ 
U32 sx_device_reset();		/* Call SIMX reset device function */


/*---------------------------------------------------------------------- 
 *
 * Local Type Definitions and Constants
 *
 */ 
typedef struct
{
    U32 ram_buf_addr;
} DMAAR;

#define ASC_AR_ADDR	((volatile DMAAR *)(sc->sc_slotvaddr + 0x40000))

#define DMA_AR		volatile DMAAR

/*
 * The following structures are allocated to help with debug to be used
 * as templates for DBGMON.
 */
char 		*gl_char_ptr;
SEGMENT_ELEMENT *gl_seg_ptr;
SEGMENT_BUFFER  *gl_segbuf_ptr;
DME_DESCRIPTOR  *gl_dme_desc;
CCB_SCSIIO	*gl_ccb;
SIM_WS		*gl_sim_ws;
SIM_SOFTC	*gl_sim_softc;
NEXUS		*gl_nexus;



/*
 * Transform the passed address from a monotonically increasing address space
 * to whatever implementation specific addressing scheme is used for this 
 * DMA RAM buffer. This macro will need to be implementation specific.
 *
 * For PMAX subtract off the base address of the RAM buffer shift left (*2)
 * one position and then add back in base address.
 */
#define DME_MOD_ADDRESS(addr,softc) (u_short*) (((U32) (addr) - \
		       (U32)SIMSII_GET_RAM_BUF(sim_softc)\
     		       <<1) + (U32)SIMSII_GET_RAM_BUF(sim_softc))


#define SIM_LOG_SII_ALL SIM_LOG_ALL_SIM_WS
                         

/* --------------------------------------------------------------------
 *
 * Start of SIM_DME.C routines.
 *
 */


/**
 * ramsii_dme_init -
 *
 * FUNCTIONAL DESCRIPTION:
 * 
 * Initialize the DME subsystem. Allocate and initialize the segment 
 * structures and perform any DME specific initialization.
 *
 * FORMAL PARAMETERS:
 *			SIM_SOFTC* sim_softc - Address of sim controller 
 *					       structure.
 *
 * IMPLICIT INPUTS:
 *       NONE
 *
 * IMPLICIT OUTPUTS:
 *	NONE
 *
 * RETURN VALUE:
 *       CAM_REQ_CMP     	- All is well
 *       CAM_FAILURE            - DME failed to initialize
 *
 * SIDE EFFECTS:
 *       NONE
 *
 * ADDITIONAL INFORMATION:
 *       NONE
 *
 *
 **/
U32 ramsii_dme_init (sim_softc)

    SIM_SOFTC* sim_softc;
{
    I32 num_ptes;		/* Number of PTE's need to 128KB             */
    void* sva_dblmap_buf;	/* The addr of the SVA space used to dbl map */
    I32 cnt;		  	/* Loop counter                              */
    DME_DESCRIPTOR dme_desc;  	/* Temp descriptor used to build segment     */
                                /* free queues.				     */
    U32 retval;		/* Function return value		     */
    DME_PMAX_STRUCT *ldme;

    SIM_MODULE(ramsii_dme_init);
    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	   ("[b/t/l] (ramsii_dme_init): Initialize the dme subsystem\n"));

    if ( sim_softc->simd_init )
      {
	CAM_ERROR(module,
		  "(ramsii_dme_pmax_init): duplicate init call\n",
		   (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR),
		   sim_softc,NULL,NULL);

	return CAM_REQ_CMP_ERR;
      }
    
    sim_softc->simd_init = 1;

    /*
     * First do a sanity check to be sure that we are can configure the 
     * DME the way that it has been defined. Someday, we might what these
     * constants to be soft!
     */
    if ( ((NUM_1KB*B1KB) + (NUM_4KB*B4KB) +  (NUM_8KB*B8KB) + 
	  (NUM_16KB*B16KB))  + (( NUM_1KB * RAM_GUARD) + (NUM_4KB * RAM_GUARD) 
	  + (NUM_8KB * RAM_GUARD) + (NUM_16KB * RAM_GUARD)) > B128KB)
    {	
	panic("CAM: The DME RAM buffer has been misconfigured");
    };		 /* The DME RAM buffer has been misconfigured */
    
    
    sim_softc->dme = (DME_STRUCT *)sc_alloc(sizeof(DME_STRUCT));
    if ( !sim_softc->dme )
      {
	CAM_ERROR(module,
		  "ramsii_dme_init: Can't allocate memory for DME_STRUCT.\n",
		   (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR),
		   sim_softc,NULL,NULL);

	return CAM_FAILURE;
      }

    sim_softc->dme->extension = (void *)sc_alloc(sizeof(DME_PMAX_STRUCT));
    if ( !sim_softc->dme->extension )
      {
	  CAM_ERROR(module,
		    "ramsii_dme_init: Can't allocate memory for DME_PMAX_STRUCT",
		   (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR),
		   sim_softc,NULL,NULL);

	return CAM_FAILURE;
      }

    ldme = (DME_PMAX_STRUCT *) sim_softc->dme->extension;

    /*
     * Initialize the various segment free queues to the empty
     * state. A segment free list is empty when the tail of the
     * the list points at the address of the head. In other words
     * if (ldme->tail_1kb == &ldme->head_1kb) 
     * list is empty!
     */
    
    ldme->head_1kb = ldme->tail_1kb =
	(SEGMENT_ELEMENT*)&ldme->head_1kb;
    ldme->head_4kb = ldme->tail_4kb = 
	(SEGMENT_ELEMENT*)&ldme->head_4kb;
    ldme->head_8kb = ldme->tail_8kb = 
	(SEGMENT_ELEMENT*)&ldme->head_8kb;
    ldme->head_16kb = ldme->tail_16kb =
	(SEGMENT_ELEMENT*)&ldme->head_16kb;
    
    /*
     * Get System Virtual Address of DMA buffer for later use.
     */
    ldme->SVA = SIMSII_GET_RAM_BUF(sim_softc);
    
    /*
     * Allocate the segment structures used to track the progress
     * of DMA activity. There is one allocated to each DMA request
     * "active" in the SIM. The RAM buffer is partitioned into 
     * a collection of these segments.
     */
    retval = ramsii_allocate_segments(sim_softc);

    return(retval);
};			/* ramsii_dme_init */


/**
 * allocate_segments -
 *
 * FUNCTIONAL DESCRIPTION:
 * 
 * This function is called during ramsii_dme_init() to divide up the RAM 
 * buffer and to allocate the segments elements which are used in managing 
 * the RAM buffer.
 *
 * FORMAL PARAMETERS:
 *			SIM_SOFTC* sim_softc - Address of sim controller 
 *					       structure.
 *
 * IMPLICIT INPUTS:
 *       NONE
 *
 * IMPLICIT OUTPUTS:
 *	NONE
 *
 * RETURN VALUE:
 *       CAM_REQ_CMP     	- All is well
 *       CAM_FAILURE            - DME failed to initialize
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
ramsii_allocate_segments(sim_softc)

    SIM_SOFTC* sim_softc;
{
    U32 retval;
    char* ram_buf_ptr;		  	/* Addr of free DMA RAM buffer */
    DME_PMAX_STRUCT *ldme = (DME_PMAX_STRUCT *) sim_softc->dme->extension;
        
    /*
     * Start with the first byte of the ram buffer as the free lists
     * are created bump to the pointer to the next free chunck of the
     * RAM buffer.
     */
    ram_buf_ptr = (char*) ldme->SVA;
    
    
    /*
     * Now, allocate each segment of the RAM buffer a segment at a time
     * and insert it onto the appropriate free list.
     * This is done in this way for two reasons 1) The intialization sequence
     * is not time critical 2) The insertion code can be tested during the
     * init sequence.
     *
     * First we will allocate the 1KB and then the 4,8 and 16KB segment lists
     * one at a time.
     */
    
    
    /*
     * Allocate the free list for segments of 1KB in size.
     */
    ram_buf_ptr = allocate_list(sim_softc, &ldme->head_1kb,&ldme->head_1kb,
				NUM_1KB, SEG_1KB, B1KB, ram_buf_ptr);
    /*
     * Check the returned pointer to insure that the list of segments of this
     * size have been allocated. If a NULL pointer is returned, then the list
     * was not correctly allocated, this should never happen... The returned
     * pointer is the next available byte in the RAM buffer.
     */
    if( ram_buf_ptr == (char *)NULL)
	return(CAM_REQ_CMP_ERR);


    /*
     * Allocate the free list for segments of 4KB in size.
     */
    ram_buf_ptr = allocate_list(sim_softc, &ldme->head_4kb,&ldme->head_4kb,
			   NUM_4KB, SEG_4KB, B4KB, ram_buf_ptr);
    /*
     * Check the returned pointer to insure that the list of segments of this
     * size have been allocated. If a NULL pointer is returned, then the list
     * was not correctly allocated, this should never happen... The returned
     * pointer is the next available byte in the RAM buffer.
     */
    if( ram_buf_ptr == (char *)NULL)
	return(CAM_REQ_CMP_ERR);
    
    /*
     * Allocate the free list for segments of 8KB in size.
     */
    ram_buf_ptr = allocate_list(sim_softc, &ldme->head_8kb,&ldme->head_8kb,
			   NUM_8KB, SEG_8KB, B8KB, ram_buf_ptr);
    /*
     * Check the returned pointer to insure that the list of segments of this
     * size have been allocated. If a NULL pointer is returned, then the list
     * was not correctly allocated, this should never happen... The returned
     * pointer is the next available byte in the RAM buffer.
     */
    if( ram_buf_ptr == (char *)NULL)
	return(CAM_REQ_CMP_ERR);


    
    /*
     * Allocate the free list for segments of 16KB in size.
     */
    ram_buf_ptr = allocate_list(sim_softc, &ldme->head_16kb,&ldme->head_16kb,
			   NUM_16KB, SEG_16KB, B16KB, ram_buf_ptr);
    /*
     * Check the returned pointer to insure that the list of segments of this
     * size have been allocated. If a NULL pointer is returned, then the list
     * was not correctly allocated, this should never happen... The returned
     * pointer is the next available byte in the RAM buffer.
     */
    if( ram_buf_ptr == (char *)NULL)return(CAM_REQ_CMP_ERR);
    return(CAM_REQ_CMP);

};			/* allocate_segments */


/**
 * allocate_list -
 *
 * FUNCTIONAL DESCRIPTION:
 * 
 * This function will allocate and initialize the number of segment elements
 * passed in the count parameter. These segment elements will then be inserted
 * on the appropriate free queue.
 *
 *
 * FORMAL PARAMETERS:
 *
 *   sim_softc		- Ptr to SIM control structure 
 *   head 		- Ptr to head of free list 
 *   tail		- Ptr to tail of free list 
 *   count		- Number of segments to allocate 
 *   segment_type	- Segment type to be assigned  (4KB,8KB..)
 *   segment_size	- Size in bytes of RAM to alloc 
 *   ram_buf_ptr	- Addr of unalligined RAM buffer space.
 *
 * IMPLICIT INPUTS:
 *       NONE
 *
 * IMPLICIT OUTPUTS:
 *	NONE
 *
 * RETURN VALUE:
 *       CAM_REQ_CMP     	- All is well
 *       CAM_FAILURE            - DME failed to initialize
 *
 * SIDE EFFECTS:
 *       NONE
 *
 * ADDITIONAL INFORMATION:
 *       NONE
 *
 *
 **/
static char*
allocate_list(sim_softc,head,tail,count,
	      segment_type,segment_size,ram_buf_ptr)
    
    SIM_SOFTC* sim_softc;		/* Ptr to SIM control structure */
    SEGMENT_ELEMENT* head;		/* Ptr to head of free list */
    SEGMENT_ELEMENT* tail;		/* Ptr to tail of free list */
    I32 count;				/* Number of segments to allocate */
    I32 segment_type;			/* Set tygpe to be assigned */
    I32 segment_size;			/* Size in bytes of RAM to alloc */
    char* ram_buf_ptr;			/* Addr of free DMA RAM space */
{
    I32 cnt;				/* Loop counter */
    U32 retval=CAM_REQ_CMP;		/* Routine return value */
    DME_DESCRIPTOR dme_desc;		/* Temporary descriptor */


    /*
     * There must be segments to allocate to execute this code. If no
     * segments are to be allocate, simply return success. This will allow
     * the caller (ramsii_dme_init) some flexibility, during the allocation of
     * the segment free lists.
     */
    if (count <= 0)
    {			       
	return(ram_buf_ptr);
    };

    /*
     * Allocate and assign the appropriate number of segments to
     * the free list.
     */
    for (cnt = 0; cnt < count; cnt++)
    {
        dme_desc.segment = (SEGMENT_ELEMENT *)sc_alloc(sizeof(SEGMENT_ELEMENT));
	
	/*
	 * Check for allocation failure.
	 */
	if (dme_desc.segment == NULL) 
	{
	    panic("CAM: Segment allocation failed");
	    return(NULL);
	};
	
	/* Set size/type and free it */
	dme_desc.segment->flags = segment_type;
	dme_desc.segment->segment_size = segment_size;
	
	/*
	 * Fill in the starting virtual address of this segment of the RAM
	 * buffer. Then bump the pointer to the next available segment.
	 */
	dme_desc.segment->dma_buffer = (void*) ram_buf_ptr;
	
	/*
	 * Setup the RAM buffer addresses for each of the two buffers
	 * within the segemnt. Remember, that for double buffering we divide
	 * the segment into two buffers of equal size.
	 */
	dme_desc.segment->buf0.dma_buffer = (void*) ram_buf_ptr;
	dme_desc.segment->buf1.dma_buffer = (void*) (ram_buf_ptr +
						     segment_size/2);
	/*
	 * Bump the pointer to the next available segment of the ram buffer.
	 * Please notice RAM_GUARD addition..
	 */
	ram_buf_ptr = ram_buf_ptr + segment_size + RAM_GUARD;
	
	/* 
	 * First calculate the number of PTE's needed for this segment.
	 * Then allocate that number of PTE's need to map a segment of this
	 * size plus two additional PTE's to map nonpage alligned requests.
	 * 
	 * NOTE: that this DME has been optimized around the no double buffer
	 * case. Thus those requests (most of them) that will fit into the 
	 * allocated segment will be transfered in one DMA transaction.
	 * To achieve this we must have enough PTE's to map the entire
	 * segment.
	 */
	dme_desc.segment->buf0.num_pte =  btoc(segment_size) + 2;
	CAM_ALLOC_SVA((int)dme_desc.segment->buf0.num_pte,
		          &dme_desc.segment->buf0.svapte,
		           dme_desc.segment->buf0.sva);
	
	/*
         * Due to the fact that we double buffer within the same segment
         * we really need two sets of virtual addresses for each segment. One
         * set of virtual addresses that can be used to map the entire segment
	 * an one set that can be used to map the second (buf1) half of the
	 * segment for double buffering. 
	 *
	 * There are one or two other ways to solve this problem, however
	 * at this point we will just allocated another set of PTE's.
	 * We should also consider optimizing our allocation of PTE's 
	 * based more procisely on the size of the segment.
	 */ 
	dme_desc.segment->buf1.num_pte =  btoc((segment_size/2)) + 2;
	CAM_ALLOC_SVA((int)dme_desc.segment->buf1.num_pte,
		          &dme_desc.segment->buf1.svapte,
		           dme_desc.segment->buf1.sva); 

	/*
	 * Insert the allocated segment on the appropriate segment free list.
	 */
	free_segment(sim_softc,&dme_desc);
	
	/* Remember to assign the PTE's that will be needed to double map */
    };			/*  for loop */

    return(ram_buf_ptr);
};		/* allocate_list */

/**
 * free_segment -
 *
 * FUNCTIONAL DESCRIPTION:
 * 
 * Releases a previously allocated segment to the appropriate segment free 
 * queue.
 *
 * FORMAL PARAMETERS:
 *
 *			SIM_SOFTC* sim_softc     - Addr of sim's cntrl struct
 *      		DME_DECSRIPTOR* dme_desc - Address of structure the 
 *					           contains the segement info
 *
 * IMPLICIT INPUTS:
 *       NONE
 *
 * IMPLICIT OUTPUTS:
 *			SEGMENT* dme_desc->segment is cleared.
 *
 * RETURN VALUE:
 *       CAM_REQ_CMP     	- All is well
 *       CAM_XXXX               - DME failed to initialize
 *       CAM_YYYY               - Allocation failure
 *
 * SIDE EFFECTS:
 *       NONE
 *
 * ADDITIONAL INFORMATION:
 *       NONE
 *
 *
 **/
static U32
free_segment(sim_softc,dme_desc)

    SIM_SOFTC*	sim_softc;
    DME_DESCRIPTOR* dme_desc;
    
{
    U32 retval=CAM_REQ_CMP;		/* Return value */
    DME_PMAX_STRUCT *ldme = (DME_PMAX_STRUCT *) sim_softc->dme->extension;
    
    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	   ("[b/t/l] (free_segment): Deallocate a segment of the DMA buffer\n"));

    /*
     * Make sure that the segment flags have been cleaned up.
     */
    dme_desc->segment->flags &=
	(SEG_1KB | SEG_2KB |SEG_4KB | SEG_8KB | SEG_16KB);


    /* 
     * Determine the segment type and then free it to the appropriate
     * free list.
     */
    
    switch ( dme_desc->segment->flags & (SEG_1KB|SEG_4KB|SEG_8KB|SEG_16KB))
    {
	
    case SEG_1KB:

	PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	       ("[b/t/l] (free_segment): free a 1KB segment.\n"));
	/* 
	 * Since the initial conditions of the list are as described above
	 * there is only one case when it comes to inserting segments onto 
	 * this list. Simply point the current last elements head at the new
	 * segment and then move the the last pointer to point at the new 
	 * segment. These two operations are sufficient to insert a new 
	 * segment structure onto a free list organized in this way.
	 *
	 * *** NOTE ***: There is an implicit assumption here that the
	 * header (head/tail) for the free lists in the sim_softc
	 * consists of a head and a tail pair and that the head is
	 * before the tail in the header. If this assumption changes 
	 * then the list empty conditions must be checked for and handle
	 * with a slightly different algorithm.
	 */
	
	/*
	 * Point forward link of new element at old last element.
	 */
	dme_desc->segment->sim_ws = ldme->tail_1kb->sim_ws;
	
	/*
	 * Now update forward link of current last segment to point at
	 * new last segment.
	 */
	ldme->tail_1kb->sim_ws = (void*) (dme_desc->segment);
	
	/*
	 * Then point the last pointer at the new segment.
	 */
	ldme->tail_1kb = dme_desc->segment;
	break;
	
    case SEG_4KB:

	PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	       ("[b/t/l] (free_segment): free a 4KB segment.\n"));

	/*
	 * Point forward link of new element at old last element.
	 */
	dme_desc->segment->sim_ws = ldme->tail_4kb->sim_ws;
	
	/*
	 * Now update forward link of current last segment to point at
	 * new last segment.
	 */
	ldme->tail_4kb->sim_ws = (void*) (dme_desc->segment);
	
	/*
	 * Then point the last pointer at the new segment.
	 */
	ldme->tail_4kb = dme_desc->segment;
	break;
	
    case SEG_8KB:

	PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	       ("[b/t/l] (free_segment): free a 8KB segment.\n"));
	/*
	 * Point forward link of new element at old last element.
	 */
	dme_desc->segment->sim_ws = ldme->tail_8kb->sim_ws;
	
	/*
	 * Now update forward link of current last segment to point at
	 * new last segment.
	 */
	ldme->tail_8kb->sim_ws = (void*)(dme_desc->segment);
	
	/*
	 * Then point the last pointer at the new segment.
	 */
	ldme->tail_8kb = dme_desc->segment;
	break;
	
    case SEG_16KB:

	PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	       ("[b/t/l] (free_segment): free a 16KB segment.\n"));
	/*
	 * Point forward link of new element at old last element.
	 */
	dme_desc->segment->sim_ws = ldme->tail_16kb->sim_ws;
	
	/*
	 * Now update forward link of current last segment to point at
	 * new last segment.
	 */
	ldme->tail_16kb->sim_ws = (void*)(dme_desc->segment);
	
	/*
	 * Then point the last pointer at the new segment.
	 */
	ldme->tail_16kb = dme_desc->segment;
	break;
	
    default:

	PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	       ("[b/t/l] (free_segment): Illegal packet type to free\n"));
	panic("CAM: Illegal packet type to free"); 
	break;
    };			/* End Switch   */

    /*
     * Clear segment pointer to prevent double deallocates or use of
     * stale pointers.
     */
    dme_desc->segment = NULL;
    return(retval);

};			/* free_segment */


/**
 * get_seg_sub -
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * This function returns the address of an allocated segment in the dme_desc
 * segment field. The segment is removed from the specified free list and
 * and the head and tail pointers are updated appropriately.
 *
 * FORMAL PARAMETERS:  		
 *			SEGMENT_ELEMENT** head - Addr of ptr to head 
 * 			SEGMENT_ELEMENT** tail - Addr of ptr to tail 
 * 			DME_DESCRIPTOR* dme_desc - DME request descriptor 
 *
 *
 * IMPLICIT INPUTS:     NONE
 *
 * IMPLICIT OUTPUTS:    dme_desc->segment - zeroed no segments or assigned
 *					    the address of allocated segment.
 *
 * RETURN VALUE:        CAM_REQ_CMP - All is well.
 *			CAM_REQ_CMP_ERR - Allocation of segment failed.
 *
 * SIDE EFFECTS:        The address of the allocated segment is returned in
 *			dme_desc->segment. If the allocation fails dme_desc->
 *			segment is cleared.
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
static U32 
get_seg_sub(head,tail,dme_desc)
    
    SEGMENT_ELEMENT **head;			/* Addr of ptr to head */
    SEGMENT_ELEMENT **tail;			/* Addr of ptr to tail */
    DME_DESCRIPTOR* dme_desc;			/* DME request descriptor */
    
{
    U32 retval=CAM_REQ_CMP;
    
    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	   ("[b/t/l] (get_seg_sub): get a segment for a %d sized request.\n",
	   dme_desc->data_count));
    
    /*
     * If the free list is not empty, then allocate
     * the first segment on the list. Else return with 
     * an error.
     */
    if (*tail != (SEGMENT_ELEMENT*) head) 
    {
	/* 
	 * Allocate the segment at the head of the list.
	 * Copy the adddress of the allocated segment to the
	 * passed dme descriptor.
	 */
	dme_desc->segment = (SEGMENT_ELEMENT*) *head;
	
	/*
	 * Now update the head and tail as necessary. If the we just
	 * removed the last segment, then reinitialize the list.
	 */
	if (*head != *tail)
	{
	    /*
	     * This was not the last segment, update the head to point
	     * at the next segment in the list and leave the tail where
	     * it was.
	     */
	    *head = (SEGMENT_ELEMENT*) dme_desc->segment->sim_ws;
	}
	else	/* The last segment was removed, init list */
	{
	    /*
	     * The list is now empty, make the head and tail
	     * reflect that.
	     */
	    *head = *tail = (SEGMENT_ELEMENT*)head;
	};
	/*
	 * Before returning clear the sim_ws pointer which previously
	 * pointed at the next segment in the list but now will be used
	 * as a SIM Working Set pointer.
	 */
	dme_desc->segment->sim_ws = NULL;
	
	
    }
    else		/* The free list is empty */
    {
	PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	       ("[b/t/l] (get_seg_sub): Allocation of %d segment failed\n",
		dme_desc->data_count));

	dme_desc->segment = NULL;
	return(CAM_REQ_CMP_ERR);
    };
    return(retval);
};		/* get_seg_sub */

/**
 * ramsii_get_segment -
 *
 * FUNCTIONAL DESCRIPTION:
 * 
 * Initialize the DME subsystem. Allocate and initialize the segment 
 * structures and perform any DME specific initialization.
 *
 * If an allocation failure occurs, this routine will first attempt to find a 
 * smaller sized segment to substitute, then it will try to find a larger 
 * segment than the request size (TBD). If we still cannot allocate a segment,
 * we simply return an error to the caller. The caller must retry at a later
 * time.
 *
 * We still need to implement the retry for a larger segment. This can be added
 * to the 1KB segment code. If that fails and it's the first time through the
 * (first time loop variable needed) then make the request size the largest 
 * possible segment size and call get_segment.
 *
 * FORMAL PARAMETERS:
 *			SIM_SOFTC* sim_softc     - Addr of sim's cntrl struct
 *      		DME_DECSRIPTOR* dme_desc - Address of structure the 
 *					           contains the segement info
 *
 * IMPLICIT INPUTS:
 *       NONE
 *
 * IMPLICIT OUTPUTS:
 *			dme_desc->segment	- A segment is allocated then
 *						  this field contains a ptr.
 *
 * RETURN VALUE:
 *       CAM_REQ_CMP     	- All is well
 *       CAM_REQ_CMP_ERR        - Allocation failure
 *
 * SIDE EFFECTS:
 *      1) Allocation sizes maybe upgrade depending upon buffer availibility.
 *	2) Zero byte allocation requests will return with smallest sized 
 *	segment available.
 *
 * ADDITIONAL INFORMATION:
 *       NONE
 *
 *
 **/
U32 ramsii_get_segment (sim_softc,dme_desc)

    SIM_SOFTC*	sim_softc;
    DME_DESCRIPTOR* dme_desc;    
{
    U32 retval=CAM_REQ_CMP;		/* Return value */
    DME_DESCRIPTOR temp_desc;		/*  Working dme descriptor */
    DME_PMAX_STRUCT *ldme = (DME_PMAX_STRUCT *) sim_softc->dme->extension;

    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	   ("[b/t/l] (get_segment): Allocate a segment of the dma buffer 0x%x\n",
	   dme_desc->data_count));
    
    /* 
     * Since the maximum transfer size for the SII is 8KB. The following
     * approach is taken. For small transfer we allocate a 1KB segment,
     * for tranfers from > 1KB <= 8KB will use a 8KB segement, for
     * transfer => 8KB we use a 16KB segment made up of two 8KB buffer.
     * The difference here is that for the 16KB segment, we always double
     * buffer since we can't address more than 8KB in a single DMA cycle.
     */
    if (dme_desc->data_count > B8KB)
    {		/* Use 16KB Segment */

	PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	       ("[B/T/L] (GET_SEGMENT): GET A 16KB SEGMENT.\N"));
	
	/*
	 * get a segment from a free list of 8kb segments.
	 */
	retval = get_seg_sub(&ldme->head_16kb,
			     &ldme->tail_16kb,
			     dme_desc);
	/*
	 * retry if an allocation failure occurs.
	 */
	if (retval != CAM_REQ_CMP)
	{
	   PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
		  ("get_segment: failed to allocated 16kb segment\n"));

           temp_desc = *dme_desc;	/* make copy of original request */
           temp_desc.data_count = (B8KB);/* request smaller xfer size */
           
	   /*
	    * make recursive call to allocate next smallest request size.
	    */
           retval = ramsii_get_segment(sim_softc,&temp_desc);

	   /*
            * if a segment was allocated then use it other return to unwind
	    * the stack.
            */
	   if (retval == CAM_REQ_CMP)
	   {
		/* return the allocated segment */
		dme_desc->segment = temp_desc.segment;
           }
	   else dme_desc->segment = 0;

	}	/* allocation attempt failed */

    }
    else if (dme_desc->data_count <= B1KB) 

    {		/* use 1kb segment */
	PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	       ("[b/t/l] (get_segment): get a 1KB segment.\n"));
	
	/*
	 * Get a segment from a free list of 1KB segments.
	 */
	retval = get_seg_sub(&ldme->head_1kb,
			     &ldme->tail_1kb,
			     dme_desc);
	/*
	 * If this fails, simply return to prevent infinite recursion.
	 */
	
    }		/* Use 1KB segment */
    
    else
    {		/* Use 8KB segment */
	PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	       ("[b/t/l] (get_segment): get a 8KB segment.\n"));
	
	/*
	 * Get a segment from a free list of 8KB segments.
	 */
	retval = get_seg_sub(&ldme->head_8kb,
			     &ldme->tail_8kb,
			     dme_desc);
	/*
	 * Retry if an allocation failure occurs.
	 */
	if (retval != CAM_REQ_CMP)
	{
	   PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
		  ("get_segment: failed to allocated 8kb segment\n"));

           temp_desc = *dme_desc;	/* Make copy of original request */
           temp_desc.data_count = (B1KB);/* Request smaller xfer size */
           
	   /*
	    * Make recursive call to allocate next smallest request size.
	    */
           retval = ramsii_get_segment(sim_softc,&temp_desc);

	   /*
            * If a segment was allocated then use it other return to unwind
	    * the stack.
            */
	   if (retval == CAM_REQ_CMP)
	   {
		/* Return the allocated segment */
		dme_desc->segment = temp_desc.segment;
           }
	   else
	   {
	       /*
		* We failed to allocate a segment.
		*/
	       dme_desc->segment = 0;

	   };

	}	/* Allocation attempt failed */
    }			
    return(retval);
};			/* get_segment */

/**
 * dme_setup -
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
 *    sim_ws		- SIM Working Set of Active I/O request.
 *    count		- Number of bytes to transfer       
 *    buf		- Address of source/dest buffer 
 *    dir		- Direction of data transfer,RD/WR? 
 *    dme_desc		- Address of descriptor that describes this DMA request
 *
 * IMPLICIT INPUTS:
 * 	NONE
 *
 * IMPLICIT OUTPUTS:	
 *	NONE
 *
 * RETURN VALUE:
 *       CAM_REQ_CMP      - All is well
 *       CAM_FAIL         - a fatal(?) problem has  occurred
 *
 * RETURN VALUE:        	TBD
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
ramsii_dme_setup( sim_ws, count, buf, dir, dme_desc )

    REGISTER SIM_WS* sim_ws;
    U32 count;			/* Number of bytes to transfer       */
    void* buf;				/* Address of source/dest buffer     */
    U32 dir;			        /* Direction of data transfer,RD/WR? */
    REGISTER DME_DESCRIPTOR* dme_desc;
    
{
    U32 retval;			/* Return value */
    REGISTER SIM_SOFTC *sim_softc;	/* Address of SIM S/W control blk */
    REGISTER SEGMENT_ELEMENT *seg_ptr;	/* Address of segment element */
    REGISTER SEGMENT_BUFFER  *segbuf_ptr;/* Address of segment buffer ptrs*/
    
    
    SIM_MODULE(ramsii_dme_setup);
    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	   ("[b/t/l] (RAMSII_DME_SETUP) Setup a dma transfer\n"));
    
    /*
     * Assign SIM_WS pointer to the passed DME_DESCRIPTOR.
     * Setup the data count, data ptr, and direction in the
     * DME_DESCRIPTOR.
     */
    dme_desc->sim_ws = (void *)sim_ws;	/* Get a copy for later use        */
    dme_desc->data_count = count;	/* How big is the entire request?  */
    dme_desc->data_ptr = buf;		/* User buffer address S0 or P0    */
    dme_desc->flags |= dir;		/* Determine whether to write/read */

    /*
     * If a DMA has previously been setup for this descriptor then simply
     * return to the caller. This will allow the HBA to simply reschedule
     * pending I/O requests without regard for the state of the dme.
     * Also, just return success for ZERO length requests.
     */
    if ((dme_desc->segment != NULL) || (count == 0))
	return(CAM_REQ_CMP);

    /*
     * Get address of softc to be used by the DME.
     */
    sim_softc = GET_SOFTC_DESC(dme_desc);
    
    /*
     * Allocate a segment of the RAM buffer for this request.
     */
    retval = ramsii_get_segment(sim_softc,dme_desc);
    
    /*
     * If this allocation request completes, then return the status to
     * the caller.
     */
    if (retval != CAM_REQ_CMP)
	return(retval);

    /*
     * Initialize various fields in the segment element to setup
     * the starting conditions for a transfer.
     */
    seg_ptr = dme_desc->segment;
    
    if (CAM_REQ_CMP != ramsii_dme_init_segment(dme_desc,seg_ptr))
    {
	CAM_ERROR(module,
		  "(ramsii_dme_setup): Failed to setup can't init segment\n",
		  (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR),
		  sim_softc,sim_softc->active_io,
		  sim_softc->active_interrupt_context->hba_intr);
	panic("ramsii_dme_setup: Can't initialize segment\n");
    };
    
    /*
     * Setup pointer to active buffer within the segment.
     */
    segbuf_ptr = seg_ptr->abuf;
    
    /*
     * Do a save of the data pointers at this point to insure, that we can
     * do an automatic restore in the case where the target disconnects and
     * reconnects as an error recovery mechanism. In this case, the target
     * is expecting that initiator will be doing an implicite restore pointers.
     * In order to do an implicite restore pointers, we must do an explicite
     * save pointers here!
     */
    ramsii_dme_save(dme_desc);
    
    seg_ptr->state = DME_STATE_SETUP;	/* Last DME state */
    
    return(retval);
};			/* ramsii_dme_setup */

/*
 * Description of odd byte handling in the SII.
 *
 * The SII handles odd byte alligned transfer with the aid of one interrupt
 * bit (OBB) and one register (DMABYTE). Since the SII can only perform word
 * alligned accesses, the driver must do the appropriate things when starting
 * an odd alligned transfer or when a transfer ends on an odd byte boundry.
 *
 * The SII DME handles odd byte alligned transfer by having dme_pause detect
 * the condition of a transfer ending on an odd byte boundry and by having
 * dme_resume setup the SII appropriately when a transfer resumes at an odd
 * byte boundry.
 *
 *
 * More details TBD.
 */

/**
 * ramsii_dme_pause -
 *
 * FUNCTIONAL DESCRIPTION:
 * 
 * This function is called by the HBA each time the HBA receives an interrupt 
 * from the DME subsystem when the DME is active. ramsii_dme_pause() will 
 * read the state of device registers to determine the progress of a dma 
 * transaction.The state of the dma request will be updated to reflect the
 * number of bytes successfully transfered.
 *
 * This function must be called before a ramsii_dme_save, ramsii_dme_end or 
 * ramsii_dme_resume call may be executed by the HBA.
 *
 * FORMAL PARAMETERS:  	       
 *    dme_desc		- Address of descriptor that describes this DMA request
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	CAM_REQ_CMP - All is well.
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
ramsii_dme_pause(dme_desc)
    
    REGISTER DME_DESCRIPTOR* dme_desc;
    
{ 			/* ramsii_dme_pause */

    REGISTER SII_INTR *sii_intr;	/* Address of interrupt state struct */
    REGISTER SIM_SOFTC* sim_softc;	/* Pointer to SIM S/W control struct */
    REGISTER SEGMENT_ELEMENT *seg_ptr;	/* Ptr to segment element            */
    REGISTER SEGMENT_BUFFER  *segbuf_ptr;/* Ptr to active segment buffer     */
    REGISTER SIMSII_REG *siireg;	/* Address of SII CSR's              */
    U32 retval;			/* Return value                      */
    U32 xfer_cnt;			/* Number of bytes really transfered */
    int s;				/* Save the current IPL              */
    SIM_MODULE(ramsii_dme_pause);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	   ("Pause the DME and save snapshot state.\n"));
    
    /*
     * Setup required local variables.
     */
    sim_softc = GET_SOFTC_DESC(dme_desc);  /* Addr of SIM S/W control struct */
    retval = CAM_REQ_CMP;		   /* Return value             */
    sii_intr = (SII_INTR*) 
	sim_softc->active_interrupt_context->hba_intr;
    siireg = SIMSII_GET_CSR(sim_softc);	   /* Address of SII CSR's      */
    seg_ptr = dme_desc->segment;	   /* Get ptr to segment        */
    segbuf_ptr = seg_ptr->abuf;	   	   /* Address of active buffer  */
    
    /*
     * Clear the DME active bit, so that that SIM HBA doesn't think that
     * any future interrupts are from the DME.
     */
    ((SIM_WS *)dme_desc->sim_ws)->flags &= ~SZ_DME_ACTIVE;
    
    LOCK_DME(s,sim_softc);
	
    /*
     * Calculate actual number of bytes sent. act_pend_cnt is the number
     * bytes for this segment of the transfer. The dmlotc has the count
     * of the number of bytes left.
     */
    xfer_cnt = segbuf_ptr->act_pend_cnt - (U32) siireg->dmlotc;
    segbuf_ptr->act_buf_ptr = (void *) ((U32)(segbuf_ptr->act_buf_ptr) +
					xfer_cnt);
	
    /*
     * Now check for "Odd Byte" disconnects. If such a disconnect occurs
     * then we must save away the "odd" byte and indicate, that this 
     * segment has an odd byte pending. The ramsii_dme_resume() function 
     * will look at this flag and setup the next data transfer to handle
     * this condition.
     */
    if(dme_desc->flags & DME_OBB)
    {		/* OBB */
	
	PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	       ("Disconnect on odd byte detected\n"));
	
	/*
	 * Clear the ODD BYTE flag since we are now about to detected that we
	 * indeed ended on an odd byte.
	 */
	dme_desc->flags &= ~DME_OBB;
	    
	/*
	 * We will treat reads and write the same here.
	 * 
	 * Flag the fact that we have a byte pending and
	 * save the byte in the seg_ptr. DME_RESUME and DME_END
	 * will use this flag to indicate that we are processing
	 * an odd byte transfer at this time. 
	 */
	seg_ptr->flags |= SEG_ODD_DISCONNECT;

	/*
	 * Get last byte if odd boundry.
	 */
	seg_ptr->pending_byte = siireg->dmabyte;

    };		/* OBB */
    
    
    UNLOCK_DME(s,sim_softc);			/* Get device lock */
    
    /*
     * Although the following checks should not be necessary, we will
     * include it in the first release since it is a useful check for
     * devices with have something pathologically wrong with them.
     * Basically, this should never happen, but a target which stays in 
     * data phase might(has) cause(d) this to happen.
     */
    if (segbuf_ptr->act_pend_cnt < xfer_cnt)
    {
	CAM_ERROR(module,
		  "DME_PAUSE called at wrong time, act_pend_cnt < xfer",
		  (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR),
		  sim_softc,sim_softc->active_io,sii_intr);
	xfer_cnt = 0; 	/* Force a zero transfer size and continue */
    }

    /*
     * Calculate the total number of bytes transfered. act_pend_cnt has the
     * count that was loaded into the SII when the transfer was started.
     * Now subtract off what wasn't sent and that results in the count
     * of the bytes actaully transfered.
     */
    segbuf_ptr->act_pend_cnt -= xfer_cnt;/* Number of bytes left to send  */
    
    /*
     * Setup to transfer the rest of this buffer by bumping the user
     * and ram buffer pointers.
     */
    segbuf_ptr->act_ram_ptr = (void *) ((U32)(segbuf_ptr->act_ram_ptr) +
					xfer_cnt);
    segbuf_ptr->act_buf_ptr = (void *) ((U32)(segbuf_ptr->act_buf_ptr) +
					xfer_cnt);
    
    seg_ptr->act_xfer_cnt += xfer_cnt;		/* Accumulated xfer count */
    seg_ptr->state = DME_STATE_PAUSE;		/* Last DME state */
    
    return(retval);
};			/* ramsii_dme_pause */

/**
 * ramsii_dme_save - Maybe this should be a macro!
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * This function is called by the HBA when it receives a SAVE POINTERS message
 * from the target. The routine will save the current state of the DME of this 
 * descriptor for later use.
 *
 * FORMAL PARAMETERS:  	       
 *    dme_desc		- Address of descriptor that describes this DMA request
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	CAM_REQ_CMP - All is well.
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
ramsii_dme_save(dme_desc)
    
    DME_DESCRIPTOR* dme_desc;
    
{
    U32 retval=CAM_REQ_CMP;			/* Return value            */
    SEGMENT_ELEMENT *seg_ptr;			/* Ptr to  segment element */
    
    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	   ("[b/t/l] (RAMSII_DME_save) Save the state of this request.\n"));
        
    /*
     * Setup required local variables.
     */
    if (dme_desc->segment != NULL)
	seg_ptr = dme_desc->segment;	   /* Get segment */
    else return(CAM_REQ_CMP_ERR);	   /* DME called with invalid desc   */
    
    /*
     * Move the active data pointers to the save data pointers for
     * the passed dme_descriptor. It's only necessary to save the active 
     * and not the pending buffers pointers.
     */
    seg_ptr->save_abuf = *seg_ptr->abuf;
    seg_ptr->save_sg   = seg_ptr->sg;

    return(retval);
};			/* ramsii_dme_save */

/**
 * ramsii_dme_restore - Maybe this should be a macro!
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * This routine is called by the HBA in response to a RESTORE POINTER message.
 * This routine will return the state of the DME for this request to the last
 * saved DME state.
 *
 * FORMAL PARAMETERS:  	       
 *    dme_desc		- Address of descriptor that describes this DMA request
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	CAM_REQ_CMP - All is well.
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
ramsii_dme_restore(dme_desc)
    
    DME_DESCRIPTOR* dme_desc;
    
{
    U32 retval=CAM_REQ_CMP;			/* Return value           */
    SEGMENT_ELEMENT *seg_ptr;			/* Ptr to segment element */
    
    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	   ("[b/t/l] (RAMSII_DME_restore) Restore saved data pointer to the active.\n"));
        
    /*
     * Setup required local variables.
     */
    if (dme_desc->segment != NULL)
	seg_ptr = dme_desc->segment;	   /* Get segment */
    else return(CAM_REQ_CMP_ERR);	   /* DME called with invalid desc   */
    
    /*
     * Move the saved data pointers to the active data pointers for
     * the passed dme_descriptor.
     */
    
    /*
     * Move the active data pointers to the save data pointers for
     * the passed dme_descriptor. It's only necessary to save the active 
     * and not the pending buffers pointers.
     */
    *seg_ptr->abuf = seg_ptr->save_abuf;
    seg_ptr->sg   = seg_ptr->save_sg;

    return(retval);
};			/* ramsii_dme_restore */

/**
 * ramsii_dme_resume -
 *
 * FUNCTIONAL DESCRIPTION:
 * 
 * This function is called by the HBA in order to start/resume a transfer over
 * the SCSI bus. This routine will setup the DME hardware to transfer data and
 * then start the dma engine. If double buffering or scatter gather is active
 * on this request, this routine will traverse the scatter gather lists as
 * well as fill/empty buffers in the case of double buffered requests.
 *
 * This routine calls ramsii_dme_reinit segment which handles scatter gather,
 * double buffer toggles as well as mapping and copying data into and out of
 * the RAM buffer (if required).
 *
 * FORMAL PARAMETERS:  	       
 *    dme_desc		- Address of descriptor that describes this DMA request
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	CAM_REQ_CMP - All is well.
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      It is assumed that DME_PAUSE has been called
 *				before this function is executed. 
 *
 **/
U32
ramsii_dme_resume(dme_desc)
    
    REGISTER   DME_DESCRIPTOR* dme_desc;
    
{

    REGISTER SIM_SOFTC* sim_softc;	/* Pointer to SIM S/W control struct */
    REGISTER SEGMENT_ELEMENT *seg_ptr;	/* Ptr to segment element 	     */
    REGISTER SEGMENT_BUFFER  *segbuf_ptr;/* Active segment buffer pointer    */
    REGISTER SII_INTR *sii_intr;	/* Address of interrupt state struct */
    REGISTER SIMSII_REG *siireg;       	/* Address of SII CSR's		     */
    U32 retval;			/* Return value 		     */
    int s;
    U32 ram_buf_addr;		/* Relative Ram buffer address       */

    u_short sii_cmd;			/* SII command to be issued to chip  */

    SIM_MODULE(ramsii_dme_resume);

    /*
     * Setup  local variables.
     */
    sim_softc = GET_SOFTC_DESC(dme_desc);  /* Addr of SIM S/W control struct */
    retval = CAM_REQ_CMP;		   /* Return value                   */
    siireg = SIMSII_GET_CSR(sim_softc);	   /* Address of SII CSR's           */
    seg_ptr = dme_desc->segment;	   /* Get addr of segment element    */
    sii_intr = (SII_INTR*) 
      sim_softc->active_interrupt_context->hba_intr;
    
    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	   ("[b/t/l] (ramsii_dme_resume) entered the resume DMA function.\n"));
    
    /* 
     * Check for NULL segment, if NULL then return. A NULL could be passed if
     * the HBA is calling the DME although there was NO data phase expected
     * for this I/O or an erroneous byte count of zero was passed by the
     * peripheral driver in the CCB.
     */
    if (seg_ptr == NULL)
    {
	
	PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	   ("[b/t/l] (ramsii_dme_resume:) Entered with ZERO segment element"));
	return(CAM_REQ_CMP_ERR); /* DME called with invalid desc   */
    }
    
    segbuf_ptr = seg_ptr->abuf;	  	   /* Get address of active buffer   */
    
    /*
     * Before resuming a transfer check to see whether there is anything left
     * to transfer. If there are still bytes left in act_pend_cnt, this means
     * that the last dma operation still had some bytes to send. 
     *
     * The "normal" case is to just resume the DMA using the state in segment
     * element's active values. However, if we need to switch buffers (double
     * buffering) or switch elements (scatter gather) then the reinit_segment 
     * will be called to adjust the segment elements active pointers.
     */
    if( (dme_desc->data_count > seg_ptr->act_xfer_cnt) &&
       !(segbuf_ptr->act_pend_cnt > 0))
    {				       

	/*
	 * Since the pending count is zero we may just be at the end of a SG 
	 * element or a buffer within a segment has completed.
	 */
	if ( CAM_REQ_CMP != ramsii_dme_reinit_segment(dme_desc,seg_ptr))
	{
	    CAM_ERROR(module,
		      "Attempted to resume DME with no bytes left\n",
		      (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR),
		      sim_softc,sim_softc->active_io,sii_intr);
	    return(CAM_DATA_RUN_ERR);
	};

	/*
	 * Since we toggle buffers, now point segbuf_ptr at the new active
	 * buffer.
	 */
	segbuf_ptr = seg_ptr->abuf;	   /* Get address of active buffer   */

    };				/* If not done with this segment */

    /*
     * Now do a quick check to determine whether there really is anything
     * else left to transfer. If not then we should not have been called 
     * and return an error.
     */
    if (segbuf_ptr->act_pend_cnt <= 0)
    {
	    CAM_ERROR(module,
		      "DME_RESUME entered with zero act_pend_cnt",
		      (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR),
		      sim_softc,sim_softc->active_io,sii_intr);
	return(CAM_DATA_RUN_ERR);
    }

    	

    /*
     * Setup HBA to resume DMA activity.
     */
    LOCK_DME(s,sim_softc);		/* synchronize access to hba */
    
    /* 
     * For DATAIN requests handle any pending bytes from odd byte disconnects.
     * Then setup the dma transfer count, load the starting address in the dma
     * buffer into the dma address registers. The starting virtual address is 
     * kept in the segment buffer for this request. Once this setup of the 
     * SII has been completed, issue a "transfer info" command to the SII
     * indicating that this is a DMA transfer.
     *
     * DETAILS OF ODD BYTE HANDLING:
     *
     * If the this transfer is starting on an odd byte boundry, we must
     * load the SII DMABYTE register with the first (odd byte) of the
     * transfer.
     *
     * If the last transfer (datain) ended on an odd byte boundry the last byte
     * was left in the SII. Ramsii_Dme_pause() removed this byte from
     * the the SII and saved it in the seg_ptr. This is done for  odd byte 
     * disconnect where the SII doesn't transfer the last byte into
     * memory. The DME must handle this case by reloading this byte into the
     * SII before resuming the transfer. Since the count of pending bytes 
     * INCLUDES this pending byte, when SII starts up again, it will transfer
     * this pending byte and the balance of the pending byte count.
     */

    
    /*
     * Check for an odd alligned starting transfer address.
     * If the starting address is odd we must first load the SII dmabyte reg.
     */
    if ((U32)segbuf_ptr->act_ram_ptr & 0x1)
    {		/* If OBB */


	/*
	 * Treat READ and WRITE the same, the old code did..
	 * (That should be warning enough in itself..)
	 *
	 * Before starting the next DMA setup the dmabyte register with
	 * the odd byte which was not transfered to/from memory.
	 */
	seg_ptr->flags &= ~SEG_ODD_DISCONNECT; 
	siireg->dmabyte = (u_short) *((u_char*)segbuf_ptr->act_ram_ptr);

    }		/* If OBB */

    /*
     * Insure that we start/resume a DMA that the ODD BYTE flag is 
     * cleared. The OBB bit will be set if an interrupt is seen by the
     * ISR that has the OBB set in the dstat register.
     */
    dme_desc->flags &= ~DME_OBB;

    /*
     * Setup the count of the number of bytes transfer.
     */
    siireg->dmlotc = segbuf_ptr->act_pend_cnt;

    /*
     * Calculate the RAM buffer address, the SII only can address 18 bits.
     * The ram buffer address is the offset within the ram buffer to start
     * the DMA from. This address is an 18 bit address which is relative to the
     * start of the RAM buffer, thus we subtract off the base address of the
     * buffer to get the address to be loaded into the SII.
     */
    ram_buf_addr = (U32) ((0x0003ffff &
			      (U32)segbuf_ptr->act_ram_ptr -
			      (U32)SIMSII_GET_RAM_BUF(sim_softc)));

    /*
     * Setup the starting address of this DMA transfer.
     */
    siireg->dmaddrl = (ram_buf_addr & 0xffff);
    siireg->dmaddrh = (ram_buf_addr >> 16);



    /*
     * Insure that all the previously setup registers are written to the SII.
     */
    WBFLUSH(); 

    ((SIM_WS *)dme_desc->sim_ws)->flags |= SZ_DME_ACTIVE;

    /*
     * Issue a transfer info command with dma and the phase set to
     * the current phase to start a DMA transfer. The SII requires that
     * bits 0-2 of DSTAT and 4-6 of CSTAT be the same for the COMM register.
     */
    sii_cmd = ((sii_intr->cstat &  SII_CSTAT_STATE) |
	       (sii_intr->dstat & SII_DSTAT_PHASE)|
	       (SII_COMM_INFO_XFER | SII_COMM_DMA));
    CMD_PENDING(sim_softc->hba_sc, sii_cmd,siireg);
		
    
    WBFLUSH();			/* Flush write buffer */
    UNLOCK_DME(s,sim_softc);	/* release lock on hba */
    
    
    /*
     * If we are double buffering and there is a buffer to empty, then fill 
     * the next buffer or empty the previous.
     */
    if ( (seg_ptr->flags & SEG_DOUBLE) && 
	(seg_ptr->pbuf->flags & SEG_FLUSH_BUFFER))
    {
	seg_ptr->pbuf->flags &= ~SEG_FLUSH_BUFFER; /* Clear the flush flag */
	
	/*
	 * Now call function to load or clear next buffer. Rememebr that for
	 * double buffering we divide the segment into two buffers.
	 *
	 * BUG -- copys for writes are being done at dataout time, 
	 *        don't do it here too!
	 */
	if (!(dme_desc->flags & DME_WRITE))
	    ramsii_dme_buffer_map_move(dme_desc,seg_ptr,seg_ptr->pbuf,1);
    };
    
    seg_ptr->state = DME_STATE_RESUME;/* Update last DME state */
    return(retval);
};			/* ramsii_dme_resume */

/**
 * ramsii_dme_end -
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * This function is called by the HBA at the end of each request in order to
 * deallocate any DME resources as well as flush any pending buffer segments 
 * before completeing the request. This function MUST be called by the HBA
 * if ramsii_dme_setup() has been called.
 *
 *
 * FORMAL PARAMETERS:  	       
 *    dme_desc		- Address of descriptor that describes this DMA request
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	CAM_REQ_CMP - All is well.
 *
 *
 * SIDE EFFECTS:        	The HBA must call DME_PAUSE prior to executing
 *				this function. DME_PAUSE prepares the SEGMENT
 *				ELEMENT for DME_END.
 *		
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
ramsii_dme_end(dme_desc)

    REGISTER DME_DESCRIPTOR* dme_desc;
    
{
    REGISTER SIM_SOFTC* sim_softc;	/* Pointer to SIM S/W control struct */
    REGISTER SEGMENT_ELEMENT *seg_ptr;	/* Ptr to segmen element             */
    REGISTER SEGMENT_BUFFER  *segbuf_ptr;/* Active segment buffer address    */
    REGISTER SIMSII_REG *siireg;	/* Address of SII CSR's              */
    U32 retval;			/* Return value                      */
    U32 xfer_cnt;		        /* Number of bytes really transfered */
    u_char * last_byte;			/* Address of last byte in RAM buffer*/
    
    /*
     * Setup required local variables.
     */
    if (dme_desc->sim_ws != NULL)
	sim_softc = GET_SOFTC_DESC(dme_desc);/* Addr of SIM SW control struct*/
    else return(CAM_REQ_CMP_ERR);	   /* DME called with invalid desc   */
       
    retval = CAM_REQ_CMP;		   /* Return value                   */
    seg_ptr = dme_desc->segment;	   /* Get ptr to segment             */
    
    /*
     * Check the segment pointer first to determine whether or not there is
     * a valid segment. If not, then simply return to the caller (HBA).
     */
    if (seg_ptr == NULL)
    {
	PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	       ("[b/t/l] (ramsii_dme_end:) Entered with ZERO segment element, return."));
	return(retval);
    }
    
    segbuf_ptr = seg_ptr->abuf; 	   /* Get pointer to active segment */
   
    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	   ("[b/t/l] (RAMSII_DME_end) Complete a DME transaction deallocate resourses.\n"));

    /*
     * Clear the DME active bit, so that that SIM HBA doesn't think that
     * any future interrupts are from the DME.
     */
    ((SIM_WS *)dme_desc->sim_ws)->flags &= ~SZ_DME_ACTIVE;
    
    /*
     * For read operations data must be copied from the  RAM buffer, into
     * the user buffer. 
     */
    if (dme_desc->flags & DME_READ)
    {

	/* 
	 * Call "copyout" function to move data from the ram buffer into
	 * mapped user space. This code assumes that on reads that the
	 * user data area is mapped into system space. The mapping will
	 * be handled in the setup/resume functions..
	 */
	(*sim_softc->dme->vector.dme_copyout)((char*)segbuf_ptr->init_ram_ptr,
					      (char*)segbuf_ptr->init_buf_ptr,
					      segbuf_ptr->init_pend_cnt -
					      segbuf_ptr->act_pend_cnt,
					      sim_softc
					      );

	/*
	 * If there is a pending odd byte disconnect, then dme_pause saw an
	 * odd byte disconnect. However dme_resume was never called again, 
	 * therefore there is one byte that has been received that hasn't
	 * been moved to the user buffer.
	 */
	if (seg_ptr->flags & SEG_ODD_DISCONNECT)
	{
	    /*
	     * Clear the flag and then move the byte from the segbuf structure
	     * to the last location in the users buffer since this byte has not
	     * been transfered yet.
	     */
 	    seg_ptr->flags &= ~SEG_ODD_DISCONNECT;
	    last_byte = (u_char*) ( (U32) segbuf_ptr->init_buf_ptr + 
				   ((U32) segbuf_ptr->init_pend_cnt -
				    (U32) segbuf_ptr->act_pend_cnt));

	    last_byte--;	/* Get address of last byte */
	    (u_char) *(last_byte) = (u_char) seg_ptr->pending_byte;

	}


    };
    

    /*
     * Return the actaul number of bytes transfered to the caller.
     */
    dme_desc->xfer_count = seg_ptr->act_xfer_cnt;/* Total bytes transfered */


    /*
     * Free the segment of DMA RAM buffer allocated by this descriptor.
     */
    retval = free_segment(sim_softc,dme_desc);
    
    /*
     * If the deallocation fails, return to the caller
     * with failing status and expect the caller to retry.
     */
    if (retval != CAM_REQ_CMP) return(retval);
    
    seg_ptr->state = DME_STATE_END;	/* Update last DME state */
    return(retval);
};			/* dme_end */

/**
 * ramsii_dme_attach -
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * This function is called during the initialization sequence in the SIM
 * to configure the DME by loading the appropriate routines into the the
 * HBA_DME_CONTROL structure in the sim_softc. This function is provided 
 * to allow the Data Mover Engine to be configured based on the capabilities
 * of the underlying HBA. 
 *
 * Currently, this routine supports the 3MAX DME.
 *
 * FORMAL PARAMETERS:  		SIM_SOFTC* sim_softc - Addr of SIM cntrl strc
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		sim_softc->dme initialized.
 *
 * RETURN VALUE:        	CAM_REQ_CMP - All is well.
 *
 * SIDE EFFECTS:        	This function calls ramsii_dme_init, to init the DME
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
ds3100_dme_attach(sim_softc)
    
    SIM_SOFTC* sim_softc;
    
{
    U32 retval=CAM_REQ_CMP;			/* Return value */
    
    /*  
     * Implement some for of scan for the appropriate vectors to load
     * into the dme_control structure.
     */
    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	   ("[b/t/l] (ds3100_dme_attach) XXXXXXX\n"));

    if ( CAM_REQ_CMP == ramsii_dme_init(sim_softc))
    {
	/* initialize the data mover engine	*/
	sim_softc->dme->vector.dme_init = ramsii_dme_init;
	
	/* setup for a transfer			*/
	sim_softc->dme->vector.dme_setup = ramsii_dme_setup;
	
	/* end a data transfer			*/
	sim_softc->dme->vector.dme_end  = ramsii_dme_end;
	
	/* pause the data mover engine		*/
	sim_softc->dme->vector.dme_pause= ramsii_dme_pause;
	
	 /* continue a transfer			*/
	sim_softc->dme->vector.dme_resume = ramsii_dme_resume;
	
	/* save the pointers			*/
	sim_softc->dme->vector.dme_save = ramsii_dme_save;
	
	/* pause the data mover engine		*/
	sim_softc->dme->vector.dme_restore = ramsii_dme_restore;
	
	/* modify the active data pointers	*/
	sim_softc->dme->vector.dme_modify  = ramsii_dme_modify;
	
	/* data in xfer with buffer */
	sim_softc->dme->vector.dme_copyin  = cam_wmbcopy;

	/* data out xfer with buffer */
	sim_softc->dme->vector.dme_copyout = cam_rmbcopy;
	
	/* clear the buffer */
	sim_softc->dme->vector.dme_clear   = cam_wmbzero; 
	
	return(retval);
    }
    else
	return(CAM_FAILURE);
    
};			/* ramsii_dme_attach */

/**
 * ds3100_dme_unload -
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * FORMAL PARAMETERS:  		NONE
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	TBD
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
ds3100_dme_unload()
{
  return(1);
};

/**
 * ramsii_dme_modify -
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * FORMAL PARAMETERS:  		NONE
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	TBD
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
ramsii_dme_modify (dme_desc,offset)
    DME_DESCRIPTOR* dme_desc;
    I32 offset;				/* Byte count offset */
    
{
    U32 retval=CAM_REQ_ABORTED;		/* Return value */
    SEGMENT_ELEMENT *seg_ptr;
    SEGMENT_BUFFER  *segbuf_ptr;
    
    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	   ("[b/t/l] (RAMSII_DME_MODIFY) Not fully supported\n"));
    
    seg_ptr = dme_desc->segment;
    segbuf_ptr = seg_ptr->abuf;
    
    /* Backup pending count 
       fix this later
       
       (I32) (segbuf_ptr->act_pend_cnt)+= (I32) offset;
       (I32) (segbuf_ptr->act_ram_ptr) -= (I32) offset;
       (I32) (segbuf_ptr->act_buf_ptr) -= (I32) offset;
       */
    return(retval);

};			/* ramsii_dme_modify */

/**
 * ramsii_dme_map_buf -
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * This function is called by the DME when a buffer to be processed is not 
 * mapped into to kernel. This routine will "map" this user buffer (P0)
 * into kernel (S0) space, by copying the user pte's that map the user's 
 * buffer, into a set of pte's allocated by the DME to double map. These DME
 * pte's are used to create a "new" address space which maps to the same
 * user buffer. This new address is accessable from all points in kernel
 * context, assuming that the user buffer was locked down during physio.
 * 
 * This double mapping is only required for DME's that have an intermediate 
 * buffer. The use of double mapping here reduces by one the number of times a
 * a buffer must be copied if it's destination is a user buffer. If it were
 * not for this double mapping, we would have to first copy the data in the
 * RAM buffer into a system buffer and then copy the data into the user.
 *
 * The good news is that since most I/O in unix goes through the buffer cache
 * most I/O will never execute this code!
 * 
 *
 *
 * FORMAL PARAMETERS:  		
 *	
 *	dme_desc	-  DME request descriptor
 *      seg_ptr		-  Local pointer segment element 
 *      segbuf_ptr	-  Local pointer segment buffer     
 *
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	TBD
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/

U32
ramsii_dme_map_buf(dme_desc,seg_ptr,segbuf_ptr)
    
    DME_DESCRIPTOR *dme_desc;
    SEGMENT_ELEMENT *seg_ptr;		/* Local pointer segment element    */
    SEGMENT_BUFFER *segbuf_ptr;		/* Local pointer segment buffer     */
    
{
    U32 retval=CAM_REQ_CMP;          /* Return value */
    SIM_SOFTC* sim_softc;               /* Pointer to SIM S/W control struct*/
    struct buf *bp;                     /* Pointer to the users BP */
    void *sva;                          /* S0 VA created to map buffer */
    pmap_t pmap;
    vm_offset_t v;
    int o;
    int cnt;

    /*
     * Setup required local variables.
     */
    sim_softc = GET_SOFTC_DESC(dme_desc);  /* Addr of SIM S/W control struct */
    bp      = GET_BP_SIMWS(dme_desc->sim_ws); /* Get the Buffer Struct Addr  */

    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	   ("[b/t/l] (ramsii_dme_map_buf): Double map a buffer.\n"));
    

    cnt = segbuf_ptr->init_pend_cnt;
    pmap = bp->b_proc->task->map->vm_pmap;
    v = (vm_offset_t)segbuf_ptr->init_buf_ptr;
    o = v & PGOFSET;

    sva = (void *)((U32)segbuf_ptr->sva + (U32) o);

    pmap_dup((pmap_t)pmap, (vm_offset_t)v, (vm_size_t)cnt,
             (vm_offset_t)sva, (vm_prot_t)VM_PROT_WRITE,
             (vm_tbop_t)TB_SYNC_LOCAL);
    segbuf_ptr->init_buf_ptr = sva;
 
    return(retval);
}; 

/**
 * ramsii_dme_map_copy_write -
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * This routine is an internal DME routine, it is executed for write requests.
 * If the user buffer specified is not an S0 buffer then the users buffer will
 * be double mapped into S0 and P0 space. When the buffer has been mapped it
 * will be copied into the RAM buffer.
 *
 * FORMAL PARAMETERS:  		
 * 
 *	dme_desc	-  DME request descriptor
 *      seg_ptr		-  Local pointer segment element 
 *      segbuf_ptr	-  Local pointer segment buffer     
 *	
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	CAM_REQ_CMP - Success
 *				CAM_REQ_CMP_ERR - Buffer map failed.
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
ramsii_dme_map_copy_write(dme_desc,segbuf_ptr)
    
    DME_DESCRIPTOR *dme_desc;
    SEGMENT_BUFFER *segbuf_ptr;		/* Local pointer segment buffer */
    
{
    U32 retval=CAM_REQ_CMP;		/* Return value	                    */
    SIM_SOFTC* sim_softc;		/* Pointer to SIM S/W control struct*/
    SEGMENT_ELEMENT* seg_ptr;		/* Ptr to segment element           */
    
    /*
     * Setup required local variables.
     */
    sim_softc = GET_SOFTC_DESC(dme_desc);  /* Addr of SIM S/W control struct */
    seg_ptr = dme_desc->segment;	   /* Get ptr to segment             */
    
    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	   ("[b/t/l] (ramsii_dme_map_copy_write): Double map a buffer and copy\n"));
    
    /*
     * Since this is an P0 buffer, we must first map the buffer before doing
     * the buffer copy into the RAM buffer.
     */
    if (ramsii_dme_map_buf(dme_desc,seg_ptr,segbuf_ptr) != CAM_REQ_CMP)
	return(CAM_REQ_CMP_ERR);
    

    /* 
     * Now that we have double mapped the users buffer into system space,
     * call "copyin" function to move data into the ram buffer from
     * mapped user space.
     */
    (sim_softc->dme->vector.dme_copyin)((void*)segbuf_ptr->init_buf_ptr,
					(void*)segbuf_ptr->init_ram_ptr,
					segbuf_ptr->init_pend_cnt,
					sim_softc
					);
    
    return(retval);
};			/* ramsii_dme_map_copy_write */

/**
 * ramsii_dme_map_copy_read -
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * This routine is an internal DME routine, it is executed for read requests.
 * If the user buffer specified is not an S0 buffer then the users buffer will
 * be double mapped into S0 and P0 space.
 *
 * FORMAL PARAMETERS:  		
 *
 *	dme_desc	-  DME request descriptor
 *      segbuf_ptr	-  Local pointer segment buffer     
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	TBD
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
ramsii_dme_map_copy_read(dme_desc,segbuf_ptr)
    
    DME_DESCRIPTOR *dme_desc;
    SEGMENT_BUFFER *segbuf_ptr;		/* Local pointer segment buffer */
    
{
    U32 retval=CAM_REQ_CMP;		/* Return value	                    */
    SIM_SOFTC* sim_softc;		/* Pointer to SIM S/W control struct*/
    SEGMENT_ELEMENT* seg_ptr;		/* Ptr to segment element           */
    
    
    /*
     * Setup required local variables.
     */
    sim_softc = GET_SOFTC_DESC(dme_desc);  /* Addr of SIM S/W control struct */
    seg_ptr = dme_desc->segment;	   /* Get ptr to segment             */
    
    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	   ("[b/t/l] (ramsii_dme_map_copy_read): Double map a buffer and copy\n"));
    
    /*
     * Since this is an P0 buffer, we must first map the buffer before doing
     * the buffer copy into the RAM buffer.
     */
    if (ramsii_dme_map_buf(dme_desc,seg_ptr,segbuf_ptr) != CAM_REQ_CMP)
	return(CAM_REQ_CMP_ERR);
    
    /* 
     * Now that we have double mapped the users buffer into system space,
     * call "copyout" function to move data into the ram buffer from
     * mapped user space.
     */
    (sim_softc->dme->vector.dme_copyout)((char*)segbuf_ptr->init_ram_ptr,
					 (char*)segbuf_ptr->init_buf_ptr,
					 segbuf_ptr->init_pend_cnt - 
					 segbuf_ptr->act_pend_cnt,
					 sim_softc
					 );
    
    return(retval);
};			/* ramsii_dme_map_copy_read */


/**
 * ramsii_dme_init_segment -
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Initialize the segment element based on the passed DME_DESC and SIM_WS. This
 * function is called by DME_SETUP to initialize this requests segment element
 * structure.
 *
 * The ramsii_dme_init_segment() functions main purpose is to correctly initialize
 * the init_buf_ptr, init_ram_ptr and init_pend_cnt fields in the segment 
 * buffer.
 * These fields express the details of a data movement to the routines in 
 * the dme that actaully "MOVE BYTES". How these fields are adjusted depends
 * upon a number of things including, whether the request uses scatter
 * gather, double buffering or not.
 *
 * FORMAL PARAMETERS:  		
 *
 *	dme_desc	-  DME request descriptor
 *      seg_ptr		-  Local pointer segment element 
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	TBD
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
ramsii_dme_init_segment(dme_desc,seg_ptr)
    
    DME_DESCRIPTOR  *dme_desc;
     SEGMENT_ELEMENT *seg_ptr;    
{
    U32 retval=CAM_REQ_CMP;		/* Return value                 */
    SIM_WS *sim_ws;			/* Local pointer to sim_ws      */
    SEGMENT_BUFFER *segbuf_ptr;		/* Local pointer segment buffer */
    
    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	   ("[b/t/l] (ramsii_dme_init_segment) setup the segment for this request\n"));
    
    sim_ws = ((SIM_WS *)dme_desc->sim_ws);	/* Get working set */
    
    /*
     * Common dme segment initialization.
     */
    seg_ptr->act_xfer_cnt = 0;		/* Start with a zeo count */
    seg_ptr->flags &= ~(SEG_DOUBLE|SEG_SGLIST); /* Clear these bits */
    
    /*
     * Setup segment flags. 
     *
     * We will double buffer all scatter gather request to avoid a stall
     * during a segment switch. Otherwise, only double buffer if the size of 
     * the allocated segment is less than the total request size.
     */
    if (sim_ws->cam_flags & CAM_SCATTER_VALID)
	seg_ptr->flags |= (SEG_DOUBLE|SEG_SGLIST); /* Flag dbl buffer and sg */
    else

	/*
	 * Note: we force all transfers using 16KB segments to be double
	 * buffered, since the SII can only <= 8KB DMA operations.
	 */
	if ( (dme_desc->data_count > seg_ptr->segment_size) || 
	    (seg_ptr->segment_size == B16KB) )
	    seg_ptr->flags |= SEG_DOUBLE;	  /* Flag dbl buffering      */

    /*
     * There are four cases below:
     *
     * 1) NO Double Buffer, NO Scatter Gather 	(Most Common)
     * 2) NO Double Buffer, Scatter Gather (Not used since all sg are dbl buf)
     * 3) Double Buffer, NO Scatter Gather
     * 4) Double Buffer, Scatter Gather		(Least Common)
     *
     * The following code is effectively a switch on the cases above.
     * The code has intentionaly been left straightline
     *
     */
    if ( !(seg_ptr->flags & SEG_DOUBLE) )
    {			       		/* NO Double Buffering (Normal Case) */
	
	/*
	 * Setup pointers to active segment buffer.
	 */
	seg_ptr->abuf = segbuf_ptr = &seg_ptr->buf0;
	
	/*
	 * Since this is not a scatter gather request, just setup
	 * the starting conditions for this segment.
	 *
	 * Setup pointer to start address of user buffer.
	 */
	segbuf_ptr->init_buf_ptr = dme_desc->data_ptr;
	
	/*
	 * Begin the transfer at the start of the dma buffer.
	 */
	segbuf_ptr->init_ram_ptr = seg_ptr->dma_buffer;
	
	/*
	 * Start off with the transfer counter for the entire request.
	 */
	segbuf_ptr->init_pend_cnt = dme_desc->data_count;
	
	/*
	 * Now that we have setup the active buffer, for write copy the first
	 * buffer within the segment into the RAM buffer. For reads insure 
	 * that the first buffer is mapped into system space.
	 */
	retval = ramsii_dme_buffer_map_move(dme_desc,seg_ptr,segbuf_ptr,0);
	
	
    }					/* NO Double Buffering */
    else
    {					/* Double Buffering */
	
	/* 
	 * Check to see whether this is a scatter gather request or
	 * not, if not then take "simpler" double buffered path.
	 */
	if (!(seg_ptr->flags & SEG_SGLIST))
	{
	    /* 
	     * Simple double buffering path, just setup pointers to
	     * active/pending segment buffers. For double buffering there
	     * is an active and pending buffer. The active buffer is used
	     * first and then we switch to the pending once we has started
	     * sending an receiving data.
	      */
	    seg_ptr->abuf = segbuf_ptr = &seg_ptr->buf0;
	    seg_ptr->pbuf = &seg_ptr->buf1;
	    
	    /*
	     * Since this is not a scatter gather request, just setup
	     * the starting conditions for this segment.
	     *
	     * Setup pointer to start address of user buffer.
	     */
	    segbuf_ptr->init_buf_ptr = dme_desc->data_ptr;
	    
	    /*
	     * Begin the transfer at the start of the dma buffer.
	     */
	    segbuf_ptr->init_ram_ptr = segbuf_ptr->dma_buffer;
	    
	    /*
	     * Based on how each segment is used the buffer size for each
	     * buffer within the segments must be initialized.  Here we 
	     * we are double buffering thus the size of each buffer is
	     * half the segment.
	     */
	    seg_ptr->buf0.buffer_size =  
		 seg_ptr->buf1.buffer_size = seg_ptr->segment_size/2;
	    
	    /*
	     * Start off with the transfer counter which is the lessor of the 
	     * request size  * and 1/2 the segment size. However, since we are
	     * just starting we know that the transfer size is equal to the 
	     * buffer size.
	     */
	    segbuf_ptr->init_pend_cnt = (segbuf_ptr->buffer_size);
	    
	    /*
	     * Now that we have setup the active buffer, for write copy the 
	     * first buffer within the segment into the RAM buffer. For reads
	     * insure  that the first the buffer is mapped into system space.
	     */
	    retval = ramsii_dme_buffer_map_move(dme_desc,seg_ptr,segbuf_ptr,0);
	    
	}
	else 	/* Else Scatter Gather */
	{
	    
	    /*
	     * Double Buffering with Scatter Gather
	     *
	     * Setup the Scatter Gather Control information.
	     */
	    
	    /*
	     * Setup pointers to active segment buffer.
	     */
	    seg_ptr->abuf = segbuf_ptr = &seg_ptr->buf0;
	    seg_ptr->pbuf = &seg_ptr->buf1;
	    
	    /*
	     * Setup counter which determines how far into the scatter gather
	     * list  element we are.
	     * For double buffer we need to know how far into the sglist entry
	     * we have transfered to know when we are done with the entry and
	     * move on to the next one. Start with zero bytes processed.
	     */
	    seg_ptr->sg.xfer_cnt = segbuf_ptr->sg.xfer_cnt = 0; 
	    seg_ptr->pbuf->sg.xfer_cnt = 0;
	    
	     /*
	      * Setup counter of which scatter gather elemnt we are processing
	      * at this time.
	      */
	     seg_ptr->sg.element_cnt = segbuf_ptr->sg.element_cnt = 1; 
	    
	    /*
	     * Get pointer to the first SG_ELEM, in the case of a scatter 
	     * gather request the data_ptr points at the address of the 
	     * scatter gather list.
	     */
	    seg_ptr->sg.element_ptr = segbuf_ptr->sg.element_ptr  =  
		((SG_ELEM*)dme_desc->data_ptr);
	    
	    /* 
	     * Setup address to start of RAM buffer for each segment buffer
	     */
	    segbuf_ptr->init_ram_ptr = segbuf_ptr->dma_buffer;
	    
	    /*
	     * Get pointer to the buffer described by the first scatter gather
	     * element.
	     */
	    segbuf_ptr->init_buf_ptr = (void*) 
		segbuf_ptr->sg.element_ptr->cam_sg_address;
	    
	    /*
	     * Based on how each segment is used the buffer size for each
	     * buffer within the segments must be initialized.  Here we 
	     * we are double buffering thus the size of each buffer is
	     * half the segment.
	     */
	    seg_ptr->buf0.buffer_size =  
		seg_ptr->buf1.buffer_size = seg_ptr->segment_size/2;
	    
	    /*
	     * Start off with the transfer counter which is the lessor of the 
	     * scatter gather element request size or 1/2 the segment size.
	     */
	    segbuf_ptr->init_pend_cnt = 
		MINIM(segbuf_ptr->sg.element_ptr->cam_sg_count,
		      segbuf_ptr->buffer_size);

	    
	    /*
	     * Now that we have setup the active buffer, for write copy the 
	     * first buffer within the segment into the RAM buffer. For reads
	     * insure  that the first the buffer is mapped into system space.
	     */
	    retval = ramsii_dme_buffer_map_move(dme_desc,seg_ptr,segbuf_ptr,0);
	    
	};		/* Else Scatter Gather */
	
    };			/* Double Buffering */
    return(retval);
    
};			/* ramsii_dme_init_segment */


/**
 * ramsii_dme_reinit_segment -
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Reinitialize  the segment element based on the passed DME_DESC and SIM_WS. 
 * This function is called by ramsii_dme_resume() to initialize this requests segment
 * buffer structure. This function will only be called in the cases of double
 * buffered I/O or scatter gather I/O. During normal character I/O this routine
 * will not be executed.
 *
 * Basically, this routine does a lot of bookeeping for the DME, it's rather
 * complex due to all the possible states involved with doing double buffer
 * and scatter gather on the same request. 
 *
 * FORMAL PARAMETERS:  		
 *
 *	dme_desc	-  DME request descriptor
 *      seg_ptr		-  Local pointer segment element 
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	TBD
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:     
 *
 *
 **/
U32
ramsii_dme_reinit_segment(dme_desc,seg_ptr)
    
    REGISTER DME_DESCRIPTOR  *dme_desc;
    REGISTER SEGMENT_ELEMENT *seg_ptr;    
{
    U32 retval=CAM_REQ_CMP;		/* Return value              */
    SIM_WS *sim_ws;			/* Local pointer to sim_ws   */
    U32 ram_ptr;			/* New RAM buffer start addr */
    U32 user_ptr;			/* New users buf start addr  */
    U32 pend_cnt;			/* Number of bytes to DMA    */
    U32 buf_size;			/* Size of double buf buffer */
    U32 bytes_left;			/* Number of bytes remaining */
    SEGMENT_BUFFER *segbuf_ptr;/* Ptr to segment buffer     */
    
    SIM_MODULE(ramsii_dme_reinit_segment);

    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	   ("[b/t/l] (ramsii_dme_reinit_segment) setup the segment for this request\n"));
    
    sim_ws = ((SIM_WS *)dme_desc->sim_ws);	/* Get working set */
    
    /*
     * At this point we know that the target is still in data phase
     * and that we expected to transfer more bytes. Now we must decide
     * whether we are at the end of a scatter gather element or we need
     * to flip to another buffer of the segment or both!!!
     */
    
    segbuf_ptr = seg_ptr->abuf;	/* Get ptr to active buffer within segment */
    
    
    /*
     * First handle double buffer with no scatter gather.
     *
     * If it is a double buffered request, save the current state and setup 
     * (PING PONG) the next buffer and flag that the last buffer must be 
     * flushed.
     */
    
    if ((seg_ptr->flags & SEG_DOUBLE) &&
	!(seg_ptr->flags & SEG_SGLIST))
	
    {					/* Just Double Buffering */
	
	if (ramsii_dme_toggle_buffer(dme_desc,seg_ptr,&segbuf_ptr) == CAM_REQ_CMP)
	{
	    /*
	     * Now that we have setup the active buffer, for write copy the 
	     * first buffer within the segment into the RAM buffer. For reads
	     * insure  that the first buffer is mapped into system space.
	     */
	    retval = ramsii_dme_buffer_map_move(dme_desc,seg_ptr,segbuf_ptr,0);
	}
	else
	{
	    CAM_ERROR(module,
		      "DME Failed to toggle buffer\n",
		      (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR),
		      sim_softc,sim_softc->active_io,NULL);
	    retval = CAM_DATA_RUN_ERR;
	}
	
    }  					/* Just Double Buffering */
    
    
    /*
     * Then handle the case of double buffering with scatter gather.
     */
    else  if ((seg_ptr->flags & SEG_DOUBLE) &&
	      (seg_ptr->flags & SEG_SGLIST))
	
    {			/* Double Buffer and scatter gather */

	/*
	 * Increment count of bytes within this scatter gather element
	 * which have been transfered.
	 */
	seg_ptr->sg.xfer_cnt += segbuf_ptr->init_pend_cnt;

	/*
	 * First check to see whether we have exausted the current segment.
	 * If so flip to the next segment and then handle the double buffer-
	 * ing.
	 *
	 * If this is a scatter gather request and there are still bytes
	 * in the segment, just flip buffers.
	 */
	if ((seg_ptr->sg.xfer_cnt < seg_ptr->sg.element_ptr->cam_sg_count))
	    
	{			/* Just toggle buffer */
	    
	    /*
	     * Since there are still bytes within this segment to transfer,
	     * toggle to next buffer within the segment.
	     */
	    
	    if (ramsii_dme_toggle_buffer(dme_desc,seg_ptr,&segbuf_ptr) ==
		CAM_REQ_CMP)
	    {
		/*
		 * Find the starting location of the next byte in the user
		 * buffer by getting the starting address of the sglist entry
		 * and adding the number of bytes within the entry transfered.
		 */
		segbuf_ptr->init_buf_ptr = (void *)(seg_ptr->sg.xfer_cnt +
				      seg_ptr->sg.element_ptr->cam_sg_address);
		
		/*
		 * If the balance of the scatter gather element fits into the
		 * buffer then transfer that number of bytes otherwise transfer
		 * the number of bytes that equal the buffer size. But first we
		 * must calculate how many bytes are actaully left to send in
		 * this scatter gather element.
		 */
		bytes_left = (seg_ptr->sg.element_ptr->cam_sg_count -
			      seg_ptr->sg.xfer_cnt);   

		segbuf_ptr->init_pend_cnt = MINIM(bytes_left,
						  segbuf_ptr->buffer_size);

		/*
		 * Now that we have setup the active buffer, for writes copy 
		 * the first buffer within the segment into the RAM buffer.
		 * For reads insure  that the first buffer is mapped into
		 * system space.
		 */
		retval = ramsii_dme_buffer_map_move(dme_desc,seg_ptr,segbuf_ptr,0);
	    }
	    else
	    {
		CAM_ERROR(module,
			  "DME Failed to toggle buffer (Reinit)\n",
			  (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR),
			  sim_softc,sim_softc->active_io,NULL);
		retval = CAM_DATA_RUN_ERR;
	    };
	    
	}			/* Just toggle buffer */
	else
	{			/* Else Scatter Gather */

	    
	    /* 
	     * Setup for next scatter gather element. Bump to the next scatter
	     * gather element and toggle to the next buffer. The toggle buffer
	     * must be called first.
	     */

	    if (ramsii_dme_toggle_buffer(dme_desc,seg_ptr,&segbuf_ptr) == CAM_REQ_CMP)
	    {
		if (ramsii_dme_bump_sglist(dme_desc,seg_ptr,segbuf_ptr)==CAM_REQ_CMP)
		{
		    
		    /*
		     * The ramsii_dme_dump_sglist returned the address of a new uesr
		     * buffer and the size of that buffer. The
		     * ramsii_dme_toggle_buffer returned the address in the RAM 
		     * Now that we have setup the active buffer, for write 
		     * copy the first buffer within the segment into the RAM 
		     * buffer. For reads insure  that the first the buffer is
		     * mapped into system space.
		     */
		    retval =
			ramsii_dme_buffer_map_move(dme_desc,seg_ptr,segbuf_ptr,0);

		}	/* Bump scatter gather failed */
		else
		{

		    CAM_ERROR(module,
			      "DME bump sg failed\n",
			      (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR),
			      sim_softc,sim_softc->active_io,NULL);

		    retval = CAM_DATA_RUN_ERR;
		};
	    }
	    else	/* Toggle failed */ 
	    {

		CAM_ERROR(module,
			  "DME toggle of seg buf failed\n",
			  (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR),
			  sim_softc,sim_softc->active_io,NULL);

		retval = CAM_DATA_RUN_ERR;
	    };
	    
	}		/* Double buffer and scatter gather */
    }	
    else
    {	   		/* Neither scatter gather or double buffer */
	
	/*
	 * We should never execute this function if we are neither using 
	 * scatter gather or double buffering, for now this will be fatal 
	 * until we figure out what to do.
	 */


	CAM_ERROR(module,
		  "DME, This I/O must be either sglist or dbl buffered",
		  (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR),
		  sim_softc,sim_softc->active_io,NULL);

	/*
	 * This can't happen, but lets check anyway.
	 */
	panic("CAM: must be scatter gather or double buffered segment\n");
	
    };		       /* Neither scatter gather or double buffer */
    return(retval);
};			/* ramsii_dme_reinit_segment */

/**
 * ramsii_dme_bump_sglist -
 *
 * FUNCTIONAL DESCRIPTION:
 * This function is called by the DME to begin processing of the next scatter
 * gather entry in the scatter gather list.  This function updates the  seg_ptr
 * scatter gather information and initializes segment_buffer.
 *
 * FORMAL PARAMETERS:  		NONE
 *
 *	dme_desc	-  DME request descriptor
 *      seg_ptr		-  Local pointer segment element 
 *      segbuf_ptr	-  Local pointer segment buffer     
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		segbuf_ptr->init_buf_ptr
 *				segbuf_ptr->init_pend_cnt
 *
 * RETURN VALUE:        	TBD
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
ramsii_dme_bump_sglist(dme_desc, seg_ptr, segbuf_ptr)
    
    REGISTER DME_DESCRIPTOR  *dme_desc;
    REGISTER SEGMENT_ELEMENT *seg_ptr;	/* Address of segment element */
    REGISTER SEGMENT_BUFFER  *segbuf_ptr;/* Address of segment buffer  */
{

    U32 retval=CAM_REQ_CMP;		/* Return value */
    SIM_WS *sim_ws;			/* Local sim_ws pointer */
    SIM_MODULE(ramsii_dme_bump_sglist);

    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	   ("[b/t/l] (ramsii_dme_bump_sglist): entered ramsii_dme_bump_sglist\n"));	    
    
    sim_ws = ((SIM_WS *)dme_desc->sim_ws);	/* Get working set */
    
    /*
     * First check to see whether there are any more scatter gather
     * elements to process.
     */
    if (seg_ptr->sg.element_cnt < sim_ws->ccb->cam_sglist_cnt)
    {
	
	/* Incr count of SG ELEM'*/
	seg_ptr->sg.element_cnt++;	
	segbuf_ptr->sg.element_cnt = seg_ptr->sg.element_cnt;

	/* Get ptr to next SG_ELEM */
	seg_ptr->sg.element_ptr++;
	segbuf_ptr->sg.element_ptr = seg_ptr->sg.element_ptr;
	
	/*
	 * Now clear count of bytes within segment transfered.
	 */
	seg_ptr->sg.xfer_cnt = 0;

	/*
	 * Get pointer to the buffer described by the first scatter gather
	 * element.
	 */
	segbuf_ptr->init_buf_ptr = 
	    (void *)seg_ptr->sg.element_ptr->cam_sg_address;
	
	/*
	 * Start this element off with the transfer counter for this 
	 * sg element.
	 */
	segbuf_ptr->init_pend_cnt =MINIM(seg_ptr->sg.element_ptr->cam_sg_count,
					  segbuf_ptr->buffer_size);
	
    }
    else		/* No more scatter gather elements in list */
    {

	CAM_ERROR(module,
		  "No more sglist elements tO process\n",
		  (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR),
		  sim_softc,sim_softc->active_io,NULL);

	return(CAM_DATA_RUN_ERR);
    };

    return(retval);
 };				/* ramsii_dme_bump_sglist */
 
 /**
  * ramsii_dme_toggle_buffer -
  *
  * FUNCTIONAL DESCRIPTION:
  * This function is called by the DME to toggle between buffers within a 
  * segment of the DMA/RAM buffer during a double buffered DMA operation. 
  * This function will update seg_ptr double buffer state information and 
  * initialize the segment_element to the point a DMA operation can be 
  * executed by ramsii_dme_resume();
  *
  * FORMAL PARAMETERS: 
  *
  *	dme_desc	-  DME request descriptor
  *     seg_ptr		-  Local pointer segment element 
  *     segbuf_ptr	-  Address of pointer to segment buffer     
  *
  * IMPLICIT INPUTS:     	NONE
  *
  * IMPLICIT OUTPUTS:		NONE
  *
  * RETURN VALUE:        	TBD
  *
  * SIDE EFFECTS:        	NONE
  *
  * ADDITIONAL INFORMATION:      NONE
  *
  **/
U32
ramsii_dme_toggle_buffer (dme_desc, seg_ptr, segbuf_ptr)
    
    REGISTER DME_DESCRIPTOR  *dme_desc;
    REGISTER SEGMENT_ELEMENT *seg_ptr;	/* Address of segment element        */
    REGISTER SEGMENT_BUFFER  **segbuf_ptr;/* Active segment buffer pointer   */
    
    
{
    U32 retval=CAM_REQ_CMP;		/* Return value              */
    U32 buf_size;			/* Size of double buf buffer */
    U32 bytes_left;			/* Number of bytes remaining */

    
    SIM_MODULE(ramsii_dme_toggle_buffer);

    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	   ("[b/t/l] (ramsii_dme_toggle_buffer): enter toggle buffer\n"));	    
    
    /*
     * Toggle Segment Buffers -
     * 
     * The active buffer within the segment has completed, now make the pending
     * buffer the active buffer (toggle). 
     */
    seg_ptr->abuf = seg_ptr->pbuf;	/* Make the current pending buffer */
    seg_ptr->pbuf = *segbuf_ptr;	/* and make the active pending.    */
    *segbuf_ptr   = seg_ptr->abuf;	/* Now point at active             */
    
    /*
     * We could empty the last buffer here however, that would preclude us
     * from the opportunity to start the next DMA transaction in parallel
     * with the execution of the copy into/out of the RAM buffer. The 
     * actual copy will occur at the end of the ramsii_dme_resume() function 
     * after a DMA operation  is started.
     */
    seg_ptr->pbuf->flags |= SEG_FLUSH_BUFFER;

    /*
     * Begin the transfer at the first bytes of this buffer within
     * the RAM buffer segment.
     */
    seg_ptr->abuf->init_ram_ptr = seg_ptr->abuf->dma_buffer;
    
    /*
     * For non-scatter gather requests, setup the starting conditions for the 
     * new active buffer by initializing init_pend_cnt, init_ram_ptr and 
     * init_user_ptr.
     */
    if (!(seg_ptr->flags & SEG_SGLIST))
    {
	/*
	 * Since this is not a scatter gather request, just setup
	 * the starting conditions for this segment.
	 *
	 * Setup pointer to start address of user buffer.
	 */
	seg_ptr->abuf->init_buf_ptr = (void*) ((U32)dme_desc->data_ptr + 
					       (U32)seg_ptr->act_xfer_cnt);
	
	/*
	 * Determine the number of bytes to remaining for none scatter gather
	 * requests.
	 */
	bytes_left = (dme_desc->data_count - seg_ptr->act_xfer_cnt);   
	
	/*
	 * Start off with the transfer counter which is the lessor of the 
	 * remaining byte count or 1/2 the segment size.
	 */
	seg_ptr->abuf->init_pend_cnt = 
	    MINIM(bytes_left,(*segbuf_ptr)->buffer_size);
	
    };
    return(retval);
};			/* ramsii_dme_toggle_buffer */


/*
 * ramsii_dme_buffer_map_move-
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * For write operations load the pending buffer with the next segment buffer 
 * to be written to the target.
 *
 * For read operations, flush the pending buffer from the RAM buffer to users
 * space. 
 *
 * FORMAL PARAMETERS:  		
 *
 *     dme_desc		-  DME request descriptor
 *     seg_ptr		-  Local pointer segment element 
 *     segbuf_ptr	-  Address of pointer to segment buffer     
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	TBD
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
ramsii_dme_buffer_map_move(dme_desc,seg_ptr,segbuf_ptr,copy_on_read)
    
    REGISTER DME_DESCRIPTOR  *dme_desc;
    REGISTER SEGMENT_ELEMENT *seg_ptr;		/* Address of segment element*/
    REGISTER SEGMENT_BUFFER  *segbuf_ptr;	/* Address of segment buffer */
    U32 copy_on_read;			/* Read copy no copy flag    */
{
    REGISTER SIM_SOFTC *sim_softc;         /* Address of SIM S/W control blk */
    U32 retval=CAM_REQ_CMP;	           /* Return value */
    
    SIM_MODULE(ramsii_dme_buffer_map_move);

    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	   ("[b/t/l] (ramsii_dme_buffer_map_move) Move data to/from ram buffer.\n"));
    

    /*
     * If the act_buf_ptr in the segment element, indicate that this
     * transfer is to or from a none system (S0) buffer, then the 
     * buffer will need to be mapped into system space. In addition
     * if the request is a read request, then the first part of the
     * the buffer will be copied to the RAM/DMA buffer.
      */
    if (CAM_IS_KUSEG(segbuf_ptr->init_buf_ptr))
    {					/* NOT an S0 Address */
	/*
	 * Since this is not a S0 buffer, double map it
	 */
	if (dme_desc->flags & DME_WRITE)
	{				/* If a write operation */
	    /*
	     * Now that we know this is a write request, and that it
	     * is not a system buffer. Double map this user buffer into 
	     * system space using the previously allocated System PTE's. 
	     * This function will load the SPTE's and perform the actual
	     * bcopy. The user PTE's are reached by looking up the BP in 
	     * the SIM_WS and using the PROC structure to find the users 
	     * PMAP (User Page Tables)
	     */
	    retval = ramsii_dme_map_copy_write(dme_desc,segbuf_ptr);
	}
	else		/* Read operation */
	{

	    /*
	     * If this is a read request, and the buffer is not already
	     * mapped into system space. Then double map this buffer into
	     * System space using the PTE's allocated for each segment of
	     * the RAM buffer. Decide whether to copy data or not.
	     * 
	     * Sometimes we will want to copy out of the buffer later
	     * and just map it now.
	     */
	    if (copy_on_read)
	    {
		retval = ramsii_dme_map_copy_read(dme_desc,segbuf_ptr);
	    }
	    else  retval = ramsii_dme_map_buf(dme_desc,seg_ptr,segbuf_ptr);
	    
	}				/* Read operation    */
    }					/* NOT an SO Address */
    
    /*
     * The buffer is already mapped into system space, so just copy it
     * for writes or copy for reads if so required by caller
     */
    else if ((dme_desc->flags & DME_READ) && copy_on_read)
    {				/* If a write operation */

	/*
	 * Get address of softc to be used by the DME.
	 */
	sim_softc = GET_SOFTC_DESC(dme_desc);
	
	/* 
	 * Call "copyout" function to move data out the ram buffer from
	 * mapped user space.
	 */
	(*sim_softc->dme->vector.dme_copyout)((char*)segbuf_ptr->init_ram_ptr,
					      (char*)segbuf_ptr->init_buf_ptr,
					      segbuf_ptr->init_pend_cnt -
					      segbuf_ptr->act_pend_cnt,
					      sim_softc);
    }
    /*
     * The buffer is already mapped into system space, so just copy it
     * for writes and simply return on reads.
     */
    else if (dme_desc->flags & DME_WRITE)
    {				/* If a write operation */
	/*
	 * For write operations data must be copied to the ram buffer,
	 * before starting the DMA engine. Here we will prime the pump
	 * by moving the first buffer of a segment for this request.
	 * Remember, that the DME handles DMA request by allocating 
	 * segments of the RAM buffer which are broken down into 
	 * individual buffer which are used in a double buffering 
	 * scheme (TBD)..
	 */
	
	/*
	 * Get address of softc to be used by the DME.
	 */
	sim_softc = GET_SOFTC_DESC(dme_desc);
	
	/* 
	 * Call "copyin" function to move data into the ram buffer from
	 * mapped user space.
	 */
	(*sim_softc->dme->vector.dme_copyin)((void*)segbuf_ptr->init_buf_ptr,
					     (void*)segbuf_ptr->init_ram_ptr,
					     segbuf_ptr->init_pend_cnt,
					     sim_softc
					     );
    };				/* If a write operation */
    

    /*
     * Now copy the starting conditions for the DMA to locations
     * which may be changed and that are used by the ramsii_dme_resume() 
     * and ramsii_dme_pause() functions.
     */
    segbuf_ptr->act_ram_ptr = segbuf_ptr->init_ram_ptr;
    segbuf_ptr->act_buf_ptr = segbuf_ptr->init_buf_ptr;
    segbuf_ptr->act_pend_cnt = segbuf_ptr->init_pend_cnt;

    return(retval);
};			/* ramsii_dme_buffer_map_move */


/* ---------------------------------------------------------------------- */

/* Move data from user space to the RAM buffer.  In the xfer loops the RAM
buffer pointer is incremented twice.  The 16 bit words in the RAM buffer are
aligned on 32 bit boundries.  The data is moves and then short pointer is
incremented again to the next word in the RAM buffer. */

U32
cam_wmbcopy(src, dst, len, sim_softc) /* COPY IN */
register  u_char *src;	/* pointer to user data */
register  u_short *dst;	/* pointer to the RAM buffer */
register  u_int len;		/* count of bytes to xfer */
                  SIM_SOFTC *sim_softc; /* Softc control structure  */ 
{
    int oddmv;			/* flag for odd count in length */
    register u_short tword;	/* local copy for word xfers to RAM */
    register u_short *wsrc;	/* for word aligned xfers */



    /*
     * Depending upon the organization of the ram buffer and the host interface
     * to this buffer, the addressing scheme used to address this space my 
     * change. The DME treats the RAM buffer as a contigous logical address
     * space. Thus, the addresses passed to  routine assume that the address
     * space of the ram buffer monotonically increases. For example, the 
     * address of the first byte in the ram buffer is zero, the address of the
     * 101st byte is 100. 
     * Since the RAM buffer is not always addressed with monotonically 
     * increasing addresses, the buffer copy routines must adjusted the passed
     * address to account for these variances in RAM buffer addressing.
     */
    dst = DME_MOD_ADDRESS(dst,sim_softc);
    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	   ("DST = %x, SRC = %x, CNT = %d.\n", dst, src, len));	


  /* Check for an odd count in len.  The byte will be moved at the bottom of
    the routine. */

    oddmv = (len & 0x1);	/* set the odd count flag is true */

    len >>= 1;			/* shift to word count */

    if( len != 0 )		/* was there only one byte to move ? */
    {
      /* If the source address is odd the data is "unaligned".  The xfer
	will have to be done a word at a time.  The local register
	variable tword is used to load the bytes from user space, one
	at a time, and then write the word to the RAM buffer.  This
	allows a single access to the RAM buffer for the word write. */

	if((u_int)src & 0x01)
	{
	    while(len-- != 0)			/* compare aginst 0 */
	    {
		tword = *src++;			/* fill lsb */
		tword |= (*src++ << 8);		/* fill msb */
		*dst = tword; 			/* load word */

		dst += 2;
	    }
	}
	else 		/* user data is word aligned */
	{
	    wsrc = (u_short *)src;		/* copy to word pointer */

	  /* The data xfer loops are unrolled.  The len variable is shifted
	    left to the next byte boundry and data is moved if necessary. 
	    The final xfer loop is a 16 byte while loop. */

	    if( len & 0x1 )			/* word check */
	    {
		*dst = *wsrc; 			/* move word */

		wsrc += 1;
		dst  += 2;
	    }

	    len >>= 1;				/* move to long word bnd */
	    if( len != 0 )			/* any more data */
	    {
		if( len & 0x1 )			/* long word check */
		{
		    *dst        = *wsrc;	/* move long word */
		    *(dst + 2)  = *(wsrc + 1);

		    wsrc += 2;
		    dst  += 4;
		}

		len >>= 1;			/* move to quad word bnd */
		if( len != 0 )			/* any more data */
		{
		    if( len & 0x1 )		/* quad word check */
		    {
			*dst        = *wsrc;	/* move quad word */
			*(dst + 2)  = *(wsrc + 1);
			*(dst + 4)  = *(wsrc + 2);
			*(dst + 6)  = *(wsrc + 3);

			wsrc += 4;
			dst  += 8;
		    }

		    len >>= 1;			/* move to hex word bnd */
		    while( len-- != 0 )		/* loop till out of data */
		    {
			*dst        = *wsrc;	/* move hex word */
			*(dst + 2)  = *(wsrc + 1);
			*(dst + 4)  = *(wsrc + 2);
			*(dst + 6)  = *(wsrc + 3);
			*(dst + 8)  = *(wsrc + 4);
			*(dst + 10) = *(wsrc + 5);
			*(dst + 12) = *(wsrc + 6);
			*(dst + 14) = *(wsrc + 7);

			wsrc += 8;
			dst  += 16;
		    }
		}
	    }
	    src = (u_char *)wsrc;		/* update the src pointer */
	}
    }

  /* Check the odd byte flag, if it is set move the last byte from user space
    to the RAM buffer using the local word variable. */

    if( oddmv )
    {
	tword = *dst;		/* read in the full 16 bits */
	tword &= 0xFF00;	/* clear out LSB */

	tword |= *src;		/* move the last byte */
	*dst = tword;		/* into the RAM buffer word */
    }
}

/* ---------------------------------------------------------------------- */

/* Move data from the RAM buffer to user space.  In the xfer loops the RAM
buffer pointer is incremented twice.  The 16 bit words in the RAM buffer are
aligned on 32 bit boundries.  The data is moved and then short pointer is
incremented again to the next word in the RAM buffer. */

U32 
cam_rmbcopy(src, dst, len, sim_softc) /* Copyout */
register u_short *src;	/* pointer to the RAM buffer */
register u_char *dst;	/* pointer to user data      */
register u_int len;	/* count of bytes to xfer    */
SIM_SOFTC *sim_softc; /* Softc control structure  */ 
{
    int oddmv;			/* flag for odd count in length     */
    register u_short tword;	/* local copy for word xfers to RAM */
    register u_short *wdst;	/* for word aligned xfers           */


    /*
     * Depending upon the organization of the ram buffer and the host interface
     * to this buffer, the addressing scheme used to address this space my 
     * change. The DME treats the RAM buffer as a contigous logical address
     * space. Thus, the addresses passed to  routine assume that the address
     * space of the ram buffer monotonically increases. For example, the 
     * address of the first byte in the ram buffer is zero, the address of the
     * 101st byte is 100. 
     * Since the RAM buffer is not always addressed with monotonically 
     * increasing addresses, the buffer copy routines must adjusted the passed
     * address to account for these variances in RAM buffer addressing.
     */
    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	   ("DST = %x, SRC = %x, CNT = %d.\n", dst, src, len));	

    src = DME_MOD_ADDRESS(src,sim_softc);

  /* Check for an odd count in len.  The byte will be moved at the bottom of
    the routine. */
    oddmv = (len & 0x1);	/* set the odd count flag is true    */

    len >>= 1;			/* shift to word count               */

    if( len != 0 )		/* was there only one byte to move ? */
    {
      /* If the dest address is odd the data is "unaligned".  The xfer
	will have to be done a word at a time.  The local register
	variable tword is used to get the word from the RAM buffer and
	xfer the bytes into user space, one at a time.  This
	allows a single access to the RAM buffer for the word read. */

	if( (u_int)dst & 0x01 )
	{
	    while(len-- != 0)			/* compare aginst 0 */
	    {
		tword = *src;		/* read word, incr to next */

		*dst++ = (u_char)tword;		/* xfer lsb */
		*dst++ = (u_char)(tword >> 8);	/* xfer msb */
		src += 2;
	    }
	}
	else 		/* user data is word aligned */
	{
	    wdst = (u_short *)dst;		/* copy to word pointer */

	  /* The data xfer loops are unrolled.  The len variable is shifted
	    left to the next byte boundry and data is moved if necessary. 
	    The final xfer loop is a 16 byte while loop. */

	    if( len & 0x1 )			/* word check */
	    {
		*wdst = *src;			/* move word */
		wdst += 1;
		src  += 2;
	    }

	    len >>= 1;				/* move to long word bnd */
	    if( len != 0 )			/* any more data */
	    {
		if( len & 0x1 )			/* long word check */
		{
		    *(wdst)     = *(src);	/* move long word */
		    *(wdst + 1) = *(src + 2);

		    wdst += 2;
		    src  += 4;
		}

		len >>= 1;			/* move to quad word bnd */
		if( len != 0 )			/* any more data */
		{
		    if( len & 0x1 )		/* quad word check */
		    {
			*(wdst)     = *(src);	/* move quad word */
			*(wdst + 1) = *(src + 2);
			*(wdst + 2) = *(src + 4);
			*(wdst + 3) = *(src + 6);

			wdst += 4;
			src  += 8;
		    }

		    len >>= 1;			/* move to hex word bnd */
		    while( len-- != 0 )		/* loop till out of data */
		    {
			*(wdst)     = *(src);	/* move hex word */
			*(wdst + 1) = *(src + 2);
			*(wdst + 2) = *(src + 4);
			*(wdst + 3) = *(src + 6);
			*(wdst + 4) = *(src + 8);
			*(wdst + 5) = *(src + 10);
			*(wdst + 6) = *(src + 12);
			*(wdst + 7) = *(src + 14);

			wdst += 8;
			src  += 16;
		    }
		}
	    }
	    dst = (u_char *)wdst;		/* update the dst pointer */
	}
    }

  /* Check the odd byte flag, if it is set move the last byte from user space
    to the RAM buffer using the local word variable. */

    if( oddmv )
    {
	tword = *src;			/* move the last byte */
	*dst = (u_char)tword;		/* from the RAM buffer word */
    }
}

/* ---------------------------------------------------------------------- */
/* Zero out a region in the RAM buffer.  NOTE: this routine will not bother
to check for odd counts. */

U32 
cam_wmbzero(dst, len, sim_softc)
register volatile u_short *dst;
register u_int len;
    SIM_SOFTC *sim_softc; /* Softc control structure  */ 
{

    /*
     * Depending upon the organization of the ram buffer and the host interface
     * to this buffer, the addressing scheme used to address this space my 
     * change. The DME treats the RAM buffer as a contigous logical address
     * space. Thus, the addresses passed to  routine assume that the address
     * space of the ram buffer monotonically increases. For example, the 
     * address of the first byte in the ram buffer is zero, the address of the
     * 101st byte is 100. 
     * Since the RAM buffer is not always addressed with monotonically 
     * increasing addresses, the buffer copy routines must adjusted the passed
     * address to account for these variances in RAM buffer addressing.
     */
    dst = DME_MOD_ADDRESS(dst,sim_softc);

    len >>= 1;				/* shift to word counts */
    while(len-- != 0)
    {
	*dst++ = 0; dst++;
    }
}


/**
 * ramsii_dme_XXXXX -
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * FORMAL PARAMETERS:  		NONE
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	TBD
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
ramsii_dme_XXXXX (dme_desc)
    
    DME_DESCRIPTOR* dme_desc;
    
{
    U32 retval=CAM_REQ_CMP;			/* Return value */
    
    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,("[b/t/l] (RAMSII_DME_XXXXX) XXXXXXX\n"));
    return(retval);
};			/* ramsii_dme_XXXXX */

