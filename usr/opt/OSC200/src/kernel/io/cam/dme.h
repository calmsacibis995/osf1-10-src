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
/*
 * @(#)$RCSfile: dme.h,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/11/19 17:15:35 $
 */
#ifndef _DME_INCL_
#define _DME_INCL_

/* ---------------------------------------------------------------------- */

/* dme.h		Version 1.09			Nov. 13, 1991 */

/*  This file contains the definitions and data structures needed by
    the CAM SIM Data Mover Engine (DME) related files.

Modification History

	Version	  Date		Who	Reason

	1.00    12/31/90        rln     Created this file.
	1.01     1/29/90        rln     Add scatter gather structures.
	1.02    03/31/91        jag     changed unix_bp to req_map, Rev 2.3
	1.03    05/17/91	rln	Remove DME_SEND_CMD macro.
	1.04	06/12/91	janet	Added "hba_cmd" to SEGMENT_ELEMENT
	1.05	06/21/91	rps	Added "dme_interrupt" to 
					HBA_DME_CONTROL. Added 
					DME_STATE_UNDEFINED to state (should
					be) enum.
	1.06	07/02/91	rps	Added SEG_ODD_DISCONNECT and
					pending_byte for sii code.
	1.07	07/20/91	rps	DME function vector changed to pointer.
					3MAX specific code moved to new .h file.
	1.08	10/22/91	janet	Added DME_DONE flag.
	1.09	11/13/91	janet	Added "VERS" defines.
	1.10	12/02/91	rln	Added DME_OBB define

/* ---------------------------------------------------------------------- */

/*
 * The following macros are used by the HBA to invoke the DME functions.
 */
#define DME_INIT(softc,dme_desc)((softc)->dme->vector.dme_init((softc)))

#define DME_SETUP(softc,sim_ws,len,buf,dir,dme_desc) \
    ( (softc)->dme->vector.dme_setup((sim_ws),(len),(buf),(dir),(dme_desc)))

#define DME_ATTACH(softc) ( dme_attach(softc))

#define DME_START(softc,dme_desc)((softc)->dme->vector.dme_start((dme_desc)))

#define DME_END(softc,dme_desc)((softc)->dme->vector.dme_end((dme_desc)))

#define DME_PAUSE(softc,dme_desc)((softc)->dme->vector.dme_pause((dme_desc)))

#define DME_RESUME(softc,dme_desc)((softc)->dme->vector.dme_resume((dme_desc)))

#define DME_SAVE(softc,dme_desc)((softc)->dme->vector.dme_save((dme_desc)))

#define DME_RESTORE(softc,dme_desc)((softc)->dme->vector.dme_restore\
				    ((dme_desc)))

#define DME_MODIFY(softc,dme_desc)((softc)->dme->vector.dme_modify((dme_desc)))

#define DME_COPYIN(softc,dme_desc)((softc)->dme->vector.dme_copyin((dme_desc)))

#define DME_COPYOUT(softc,dme_desc)((softc)->dme->vector.dme_copyout\
				    ((dme_desc)))

#define DME_CLEAR(softc,dme_desc)((softc)->dme->vector.dme_bclear((dme_desc)))

/*
 * The HBA_DME_CONTROL structure is the sim's and hba's interface to
 * the data mover engine utilized by a particular SCSI controller.
 */
typedef struct hba_dme_control {
#define HBA_DME_CONTROL_VERS 1
	U32 (*dme_init)();	/* initialize the data mover engine	*/
	U32 (*dme_setup)();	/* setup for a transfer			*/
	U32 (*dme_start)();	/* start the data mover engine		*/
	U32 (*dme_end)();	/* end a data transfer			*/
	U32 (*dme_pause)();	/* pause the data mover engine		*/
	U32 (*dme_resume)();	/* continue a transfer			*/
	U32 (*dme_save)();	/* save the pointers			*/
	U32 (*dme_restore)();/* pause the data mover engine		*/
	U32 (*dme_modify)();	/* modify the active data pointers	*/
	U32 (*dme_copyin)(); /* data in xfer with buffer		*/
	U32 (*dme_copyout)();/* data out xfer with buffer		*/
	U32 (*dme_clear)();	/* clear the buffer			*/
	U32 (*dme_interrupt)();/* dme interuupt routine              */
} HBA_DME_CONTROL;


/*
 * The SCATTER_ELEMENT struct contains the information about a scatter 
 * gather element. In a double buffer environment, multiple scatter 
 * gather elements may be active at one time.
 */

typedef struct scatter_element
{
#define SCATTER_ELEMENT_VERS 1

  SG_ELEM *element_ptr;  /* Pointer to next scatter gather element      */
  U32 xfer_cnt;	/* Count of bytes transfered within sg element     */
  U32 element_cnt;    /* Count (index) of current scat gather element*/

} SCATTER_ELEMENT;



/*
 * The SEGMENT_BUFFER struct contains the information about a segment
 * the buffer used for double buffering within a segment. When a segment is 
 * double buffered it is divided into two buffers of equal size. Each of these
 * buffers is within the same segment however, the DME acts on these buffers
 * as if they were independent. The SEGMENT_ELEMENT maintains a pointer to the
 * buffer which is active.
 */


typedef struct segment_buffer
{
#define SEGMENT_BUFFER_VERS 1

  /* Active data pointers and counters */
  void *act_ram_ptr;	/* Address is RAM buffer of data 		   */
  void *act_buf_ptr;	/* Mapped user/kernel buffer address               */
  U32 act_pend_cnt;  /* Count of bytes remaining to be sent or received */
  u_short flags;   	    /* Flags                                       */
  U32 buffer_size;	    /* Size in bytes of buffer (segment_size/2)    */
  void *dma_buffer;	    /* RAM address for this segment buffer         */
  
  /* 
   * The following three fields maintain the starting conditions for this 
   * segment buffer across dme_pause and dme_resume calls. These fields are
   * needed since the act_ram_ptr,act_buf_ptr and act_pend_cnt may be modified
   * during a DMA sequence.
   */
  void *init_ram_ptr;	/* Initial address is RAM buffer of data  	     */
  void *init_buf_ptr;	/* Initial Mapped user/kernel buffer address         */
  U32 init_pend_cnt; /* Initial count of bytes remaining to be transfered */


  /*
   * PTE information used during double map of user and RAM buffer
   * Each buffer within the segment can be mapped indendantly of the other
   * buffer. The first (buf0) of a segment is capable of mapping the entire
   * segment, this is done to allow us to do a single dma transaction which is
   * equal to the entire segment length.
   */
  struct pte *svapte;	/* System Virt Address of first PTE used to double */
  			/* map raw I/O to this buffer.			   */
  U32 num_pte;	/* The number of PTE's allocated for this segment  */
  void   *sva;		/* Kernel (S0) Virtual Address of dbl mapped buffer*/


  /*
   * The following fields are used to track the state of scatter gathered
   * requests
   */
  SCATTER_ELEMENT sg;

} SEGMENT_BUFFER;


/*
 * The SEGMENT_ELEMENT struct contains the information about a segment
 * of the DMA buffer. These structure are used by the SIM DME to manage
 * the DMA buffer in the CAM subsystem. Segment structures are allocated
 * and inserted on the free queues by the SIM DME INIT code. There is one 
 * SEGMENT_ELMENET for each segment of the DMA buffer. The size of each 
 * segment is determined by looking at the flag bits in the segment 
 * structure. This structure is DME specific and may be modified independent
 * of the SIM HBA.
 */

/*
 * Definition of flags for segment.flags.
 */
#define SEG_BUSY 0x0001		/* Segment busy/free bit 1=free */
#define SEG_1KB  0x0002		/* Segment size indicator */
#define SEG_2KB  0x0004		/* Segment size indicator */
#define SEG_4KB  0x0008		/* Segment size indicator */
#define SEG_8KB  0x0010		/* Segment size indicator */
#define SEG_16KB 0x0020		/* Segment size indicator */
#define SEG_SGLIST 0x0100	/* Scatter gather request */
#define SEG_DOUBLE 0x0200	/* Double buffered request*/
#define SEG_FLUSH_BUFFER 0x0400	/* Segment buffer flush pending */
#define SEG_ODD_DISCONNECT 0x0800 /* Disconnect on odd byte     */


/*
 * Definition of dme states used for tracking state transitions in the DME.
 */
#define DME_STATE_UNDEFINED 0x0000   /* Undefined state */
#define DME_STATE_SETUP 0x0001	/* Execute DME_SETUP  */
#define DME_STATE_START 0x0002	/* Execute DME_START  */
#define DME_STATE_PAUSE 0x0003	/* Execute DME_PAUSE  */
#define DME_STATE_RESUME 0x0005	/* Execute DME_RESUME */
#define DME_STATE_END   0x0006	/* Execute DME_END    */
#define DME_STATE_SAVE		0x0007	/* Execute DME_SAVE */
#define DME_STATE_RESTORE	0x0008	/* Execute DME_RESTORE */

typedef struct segment_element
{
#define SEGMENT_ELEMENT_VERS 1

  void *sim_ws;		/* Address of sim_ws or pointer to next free seg */
  U32 flags;		/* Segment flags, free or busy 2,4,8 or 16KB     */
  U32 segment_size;	/* Segment size 2,4,8,16KB in bytes              */
  void *dma_buffer;	/* Address of intermediate DMA ram buffer        */
  U32 state;		/* State variable which contains last dme_state  */

  /*
   * HBA specific intstruction to start the data transfer.
   */
  U32 hba_cmd;

  /*
   * The active pointers and counters for this segment.
   */
  U32 act_xfer_cnt;	/* Running count of bytes sent or received           */
  SEGMENT_BUFFER buf0;	/* Contains state for first buffer or entire segment */
  SEGMENT_BUFFER buf1;	/* Contains state of 2nd buffer for double buffering */

  SEGMENT_BUFFER *abuf;	/* Address of active buffer  */
  SEGMENT_BUFFER *pbuf;	/* Address of pending buffer */
  U32 pending_byte;  /* Save bytes resulting from odd byte disconnects */


  /*
   * The following fields are used to track the state of scatter gathered
   * requests
   */
  SCATTER_ELEMENT sg;

  /*
   * The saved pointers and counters for this segment.
   */
  SEGMENT_BUFFER save_abuf; /* The saved pointers for active buffer */
  SCATTER_ELEMENT save_sg;  /* The saved scatter gather pointers    */

} SEGMENT_ELEMENT;


/*
 * The DME_DESCRIPTOR struct contains all information needed to setup
 * a SCSI transfer.  This transfer may be message, command, status,
 * or data bytes. This structure is visable to the HBA. Different DME's
 * should maintain a common DME_DESCRIPTOR format. The SEGMENT_ELEMENT
 * structure may vary on a DME by DME basis.
 */


/* 
 * Definition of the flags field in the DME Descriptor
 */
#define DME_READ    CAM_DIR_IN	/* Data is moving in from the target	  */
#define DME_WRITE   CAM_DIR_OUT	/* Data is moving out to the target 	  */
#define DME_SG_LIST	0x0004	/* Data_ptr points at a Scatt Gather List */
#define DME_DOUBLE_BUF	0x0008	/* Set if the allocated segment is to be  */
				/* to double buffer the data transfer.    */
#define DME_DONE	0x0010  /* The DME transfer has completed    	  */
#define DME_MDP_FUBAR	0x0100  /* The device has sent an MDP message     */
#define DME_OBB		0x8000	/* Did the DMA end on an odd byte?   	  */
typedef struct dme_descriptor{
#define DME_DESCRIPTOR_VERS 1
    
    void *sim_ws;	/* Ptr to SIM Working Set associated w/this DMA */
    u_short flags;	/* Direction,Scatter Gather, virtual	*/
    U32 data_count;	/* Total number of bytes to transfer	*/
    U32 xfer_count; 	/* Actual count of bytes sent		*/
    void *data_ptr;	/* Address of data area or pointer to	*/
			/* scatter gather list as defined in	*/
			/* table 8-4				*/
    SEGMENT_ELEMENT *segment;/* Filled in by dme_setup		*/

} DME_DESCRIPTOR;


/*
 * Definition for the DME/HBA specific structure 
 * used to maintain the state of the DME subsystem.
 *
 * It is likely that this structure will be unique for each DME.
 * The goal is to keep the DME_DESCRIPTOR the same for all DME's
 * while allowing the SEGMENT_ELEMENT to vary to meet the different
 * requirements of the dma engines that exist in our systems.
 */

typedef struct dme_struct {
#define DME_STRUCT_VERS 1
    /*
     * Control structures which define the SIM's interface to the 
     * data mover engine.				
     */
    HBA_DME_CONTROL vector;	/* The routine vectors for DME functions */

    /*
     * Hardware may be associated with the DME.  A pointer is required
     * to point to this hardware base address or possibly a structure
     * describing all registers, etc.
     */
    void *hardware;

    /*
     * DME implementation specific data will be held in a structure
     * defined by the DME designer.  However, a pointer to this
     * structure is included here.
     */

    void *extension;

} DME_STRUCT;


/******************************************************************************
 *
 * Macros defined locally for use in the DME.
 *
 *
 *****************************************************************************/

#define GET_SOFTC_DESC(desc) ((SIM_SOFTC*) ((SIM_WS*)(desc)->sim_ws)->sim_sc)

#ifdef OSF
#define CAM_ALLOC_SVA(num_ptes, dest, ret); 		\
    	(ret) = (void *)vm_alloc_kva((num_ptes) * NBPG);

#else /* OSF */
#define CAM_ALLOC_SVA(num_ptes, dest, ret); 		\
    	(ret) = (void *)(get_sys_ptes((num_ptes),(dest)));

#endif /* OSF */

#define LOCK_DME(s,lock_handle); {	\
		s = splbio();		\
}
#define UNLOCK_DME(s,lock_handle); {	\
		splx(s);		\
}

#define GET_BP_SIMWS(sim_ws)(\
			   (struct buf *)(((SIM_WS*)sim_ws)->ccb->cam_req_map)\
			     )

#define MINIM(value1,value2) ( (value1) >= (value2) ? (value2) : (value1));



#endif /* _DME_INCL_ */


