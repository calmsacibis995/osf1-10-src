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
 * @(#)$RCSfile: sim.h,v $ $Revision: 1.1.11.6 $ (DEC) $Date: 1993/11/23 21:54:51 $
 */
#ifndef _SIM_
#define _SIM_

/* ---------------------------------------------------------------------- */

/* sim.h		Version 1.30			Jan. 10, 1992 */

/*  This file contains the definitions and data structures needed by
    the CAM SIM related files.

Modification History

	1.30	01/10/92	janet
	Added the defines SZ_RESELECTED and SZ_ERRLOG_CMDTMO.
	Changed the state machines data structure to use a SIM_SOFTC
	pointer instead of a SIM_WS.

	1.29	12/16/91	janet
	Replaced the "u_char pad0[2]" field of SIM_WS with
	"u_short errors_logged".  "Errors_logged" will be used
	to prevent multiple error logs due to the same error.
	Added the bit defines SZ_ERRLOG_DATAIN and SZ_ERRLOG_MSGIN
	to be used with this field.

	1.28	11/13/91	janet
	Added error log defines.  Removed functional queues.

	1.27	10/24/91	janet
	Added an abort count.

	1.26	10/22/91	janet
	o Added SIM_MAX_FUNCQ define for the functional circular queues.
	o Added SZ_VALID_REQSNS define for target mode.
	o Added flink and blink in the IT_NEXUS struct.
	o Added ALL_REQ_SNS_DATA to the IT_NEXUS struct for a storage
	  area for request sense data for target mode.
	o Added ALL_INQ_DATA to the IT_NEXUS struct for a storage
	  area for inquiry data for target mode.
	o Added a SIM_TARG_WS to the SIM_WS struct.
	o Added a functional queue to the SIM_WS struct.
	o Added functional pointers in the SIM_SOFTC struct for the following
	  target mode functions: (*hba_targ_cmd_cmplt)(),
	  (*hba_targ_recv_cmd)(), (*hba_targ_send_msg)(),
	  (*hba_targ_disconnect)(), (*hba_targ_recv_msg)().

	1.25	09/19/91	janet
	Added a flags field to the TAG_ELEMENT struct. Also created the
	SZ_TAG_ELEMENT_INUSE define.

        1.24	09/05/91	janet
	Added active tag array support.  Added SZ_ABORT_TAG_INPROG flag.
	Reorganized the file history.

	1.23	09/09/91	rps
	Added reference to chip reset function.

	1.22	07/26/91	janet
	Added ERR_ABORT_TAG_REJECTED define.

	1.21	07/08/91	janet
	Added pointers and counters to allow the SIM_SOFTC to keep a linked
	list of NEXUS structures which contain waiting I/O's.

        1.20    06/28/91        rps
	Added struct name and SM element.

	1.19	06/21/91	rps
	Added pointer to dma engine in sim_softc structure.

	1.18	06/18/91	janet
	Added state machine structures and defines.

	1.17	06/04/91	janet
	Added ERR_UNKNOWN, ERR_DATA_RUN define.

	1.16	05/29/91	janet
	Added coment for NEXUS flags field regarding the use of the SYNC
	related bits being used per target, not	per LUN.

	1.15	03/27/91	jag
	Added the storage for uba_ctlr and CSR values received at probe.
	(Ultrix/BSD configuration issue)

	1.14	03/26/91	janet
	Made changes after code review

	1.13	02/21/91	rln
	Add MAX_LUN/TARGETS

	1.12	12/11/90	janet
	Remove SZ_RESET_SEEN and SZ_ABORT_SEEN

	1.11	12/10/90	rln
	Merge new constants and sim_ws fields

	1.10	12/10/90	jag
	Made the valid flag in SIM_PRIV u_long and added the magic # for it.

	1.09	12/03/90	janet
	Added SZ_UNTAGGED define.

	1.08	11/27/90	janet
	Added buffers for the SIM_WS queues

	1.07	11/21/90	rln
	Continue to add numerous fields required by the SIM XPT to the nexus
	and sim_ws structures. Also add John's CAM_SIM_PRIV structure.

	1.06	11/21/90	janet
	Changed the SZ_CAN_DISCON to SZ_NO_DISCON.

	1.05	11/20/90	rln
	Add numerous fields required by the SIM	XPT to the nexus and
	sim_ws structures.

	1.04	11/19/90	janet
	Moved the state machine and SCSI related defines from this file
	to sim_comm.h

	1.03	11/09/90	janet
	Added state machine and SCSI related defines.

	1.02    10/21/90        janet
	Updated most structures and added defines.

	1.01    10/09/90        rln
	Update HBA_DME_CONTROL, rename HBA_XFER_INFO

	1.00    10/05/90        janet
	Created this file.

*/
/* ---------------------------------------------------------------------- */

/*
 * Define to turn on SIM functional queues.
 */
/*
#define SIM_FUNCQ_ON
*/

/*
 * Queue sizes
 */
#define SIM_MAX_MSGIQ	16
#define SIM_MAX_PHASEQ	32
#define SIM_MAX_MSGOQ	26
#define SIM_MAX_FUNCQ   256
#define SM_QUEUE_SZ	16

/*
 * Maximum number of times we will try to abort an I/O before we
 * will perform a SCSI bus reset.
 */
#define SIM_MAX_ABORT_CNT	0x3

/*
 * HBA specific data area within the SIM_WS.
 */
#define SIM_HBA_DATA	4	/* in bytes */

/*
 * Maximum number of tags per ATA (active tag array).
 */
#define SIM_MAX_TAG	256

/*
 * Number of entries in the SIM common error log header.
 */
#define SIM_LOG_SIZE	((U32)25)

/*
 * SIM error log defines.
 */
#define SIM_LOG_SIM_SOFTC		((U32)0x00000001)
#define SIM_LOG_IT_NEXUS		((U32)0x00000002)
#define SIM_LOG_NEXUS			((U32)0x00000004)
#define SIM_LOG_SIM_WS			((U32)0x00000008)
#define SIM_LOG_TAG_ELEMENT		((U32)0x00000010)
#define SIM_LOG_DME_STRUCT		((U32)0x00000020)
#define SIM_LOG_SIM_SM_DATA		((U32)0x00000040)
#define SIM_LOG_SIM_SM			((U32)0x00000080)
#define SIM_LOG_HBA_DME			((U32)0x00000100)
#define SIM_LOG_HBA_CSR			((U32)0x00000200)
#define SIM_LOG_HBA_SOFTC		((U32)0x00000400)
#define SIM_LOG_HBA_INTR		((U32)0x00000800)
#define SIM_LOG_FOLLOW_LINKS		((U32)0x00001000)
#define SIM_LOG_UNUSED			((U32)0x00002000)
#define SIM_LOG_NOLOG			((U32)0x00004000)
#define SIM_LOG_PRISEVERE		((U32)0x00008000)
#define SIM_LOG_PRIHIGH			((U32)0x00010000)
#define SIM_LOG_PRILOW			((U32)0x00020000)
#define SIM_LOG_ALL_SIM_SOFTC						\
    (SIM_LOG_SIM_SOFTC | SIM_LOG_TAG_ELEMENT | SIM_LOG_DME_STRUCT |	\
     SIM_LOG_FOLLOW_LINKS)
#define SIM_LOG_ALL_NEXUS						\
    (SIM_LOG_NEXUS | SIM_LOG_TAG_ELEMENT | SIM_LOG_FOLLOW_LINKS )
#define SIM_LOG_ALL_SIM_WS						\
    (SIM_LOG_SIM_WS | SIM_LOG_NEXUS | SIM_LOG_IT_NEXUS | SIM_LOG_SIM_SOFTC | \
     SIM_LOG_FOLLOW_LINKS | SIM_LOG_TAG_ELEMENT | SIM_LOG_DME_STRUCT)

/*
 * Bit defines for the "error_recovery" field of SIM_SOFTC and SIM_WS.
 */
#define ERR_BUS_RESET	 ((U32)0x00000001)/* bus reset recovery		*/
#define ERR_PARITY	 ((U32)0x00000002)/* parity error recovery	*/
#define ERR_MSGIN_PE	 ((U32)0x00000004)/* A parity error occurred on a *
					   * message in byte.		*/
#define ERR_DATAIN_PE	 ((U32)0x00000008)/* A parity error occurred on a *
					   * data in byte.		*/
#define ERR_STATUS_PE	 ((U32)0x00000010)/* A parity error occurred on a *
					   * status byte.		*/
#define ERR_PHASE	 ((U32)0x00000020)/* The target performed an	*
					   * invalid phase sequence	*/
#define ERR_MSG_REJ	 ((U32)0x00000040)/* The target rejected an	*
					   * essential message.		*/
#define ERR_TIMEOUT	 ((U32)0x00000080)/* The SIM_WS is in the process *
					   * of being timed out.	*/
#define ERR_ABORT_REJECTED \
    			 ((U32)0x00000100)/* Set if ABORT message was	*
					   * rejected by the target.	*/
#define ERR_UNKNOWN	 ((U32)0x00000200)/* An error of unknown cause	*
					   * has occured.		*/
#define ERR_DATA_RUN	 ((U32)0x00000400)/* A data over/under run has	*
					   * occured.			*/
#define ERR_ABORT_TAG_REJECTED \
			 ((U32)0x00000800)/* Set if ABORT TAG message was*
					   * rejected by the target.	*/

/*
 * Flags used for the TAG_ELEMENT structure.
 */
#define SZ_TAG_ELEMENT_INUSE	0x0001	/* This element is in use */

/*
 * "Flags" field for the SIM_SOFTC.
 */
#define SZ_TRYING_SELECT ((U32)0x00000001)/* Currently attempting to 	*
					   * select a target.		*/
#define SZ_RESELECTED	 ((U32)0x00000002)/* Controller has been reselected*/
#define SZ_WIDE_XFER	 ((U32)0x00000004)/* Set by the HBA init function*
					   * to notifiy the other modules*
					   * that WIDE xfers are allowed */
#define SZ_POLL_MODE	 ((U32)0x00000008)/* Poll for interrupts	 */
#define SZ_TARG_CAPABLE  ((U32)0x00000040)/* Set in probe if the HBA can*/
                                          /* support target mode operations*/
#define SZ_TARGET_MODE   ((U32)0x00010000)/* The SIM is operating in    *
                                           * target mode.               */

/*
 * "Flags" bits for the IT_NEXUS.
 */
#define SZ_SYNC_CLEAR	 ((U32)0x00000001)/* Renegotiate sync with a zero*
					   * offset and period		*/
#define SZ_SYNC_NEEDED	 ((U32)0x00000002)/* Negotiate for sync transfers */
#define SZ_SYNC		 ((U32)0x00000004)/* The device is set-up for 	*
					   * sync xfers			*/
#define SZ_SYNC_NEG	 ((U32)0x00000008)/* The device is negotiating a*
					   * sync set-up 		*/
#define SZ_VALID_REQSNS	 ((U32)0x00000010)/* The reqest sense data is val */

/*
 * "Flags" bits for the NEXUS.
 */
#define SZ_NO_DISCON	 ((U32)0x00000001)/* The device can't disconnect */
#define SZ_NOTUSED_BIT1	 ((U32)0x00000002)/* bit 1 is not used - it was  *
					   * the old FROZEN bit		*/
#define SZ_CONT_LINK	 ((U32)0x00000004)/* Continue the active linked	*
					   * command.			*/
#define SZ_UNTAGGED	 ((U32)0x00000008)/* Set if the current active	*
					   * request is untagged.	*/
#define SZ_TARG_DEF	 ((U32)0x00000010)/* Set if the NEXUS is set	*
					   * with target mode defaults	*/
#define SZ_PROCESSOR	 ((U32)0x00000020)/* Set if the NEXUS is setup	*
					   * for processor mode		*/
#define SZ_TARG_DISABLE_LUN ((U32)0x00000100)  
					  /*Disable of targ lun pending*/ 

#define DISABLE_LUN_RETRY	5	  /* Retry count for disable LUN */

/*
 * "Flags" bits for the SIM_WS.
 */
#define SZ_ABORT_NEEDED  ((U32)0x00000001)/* Perform an abort on this 	*
					   * request. 			*/
#define SZ_ABORT_INPROG  ((U32)0x00000002)/* An abort has been initiated*
					   * on this request.		*/
#define SZ_TERMIO_NEEDED ((U32)0x00000004)/* Perform a terminate I/O on *
					   * this request. 		*/
#define SZ_TERMIO_INPROG ((U32)0x00000008)/* A termio has been initiated*
					   * on this request.		*/
#define SZ_TAGGED	 ((U32)0x00000040)/* This request is tagged. 	*/
#define SZ_TAG_PENDING	 ((U32)0x00000080)/* This request is tagged, but*
					   * a tag was not assigned.  Try*
					   * to assign a tag.		*/
#define SZ_RESCHED	 ((U32)0x00000100)/* Reschedule this request.	*/
#define SZ_NO_DME	 ((U32)0x00000200)/* Don't use the DME machine	*
					   * to move data during data in*
					   * and out phases.		*/
#define SZ_CMD_CMPLT	 ((U32)0x00000400)/* The command has completed.	*/
#define SZ_DEVRS_INPROG	 ((U32)0x00000800)/* A device reset is in progress*/
#define SZ_DME_ACTIVE	 ((U32)0x00001000)/* The DME is currently active*/
#define SZ_INIT_RECOVERY ((U32)0x00002000)/* A init recovery message was*
					   * received.			*/
#define SZ_AS_INPROG	 ((U32)0x00004000)/* Autosense is in progress on  *
					   * this CCB.			*/
#define SZ_RDP_NEEDED	 ((U32)0x00008000)/* Before transferring any more*
					   * data, perform a DME_RESTORE*
					   * to restore the data pointer.*/
#define SZ_TARGET_MODE	 ((U32)0x00010000)/* The SIM is operating in 	*
					   * target mode.		*/
#define SZ_EXP_BUS_FREE	 ((U32)0x00020000)/* The SIM should expect a bus*
					   * free phase to occure.	*/
#define SZ_ABORT_TAG_INPROG ((U32)0x00040000)/* An abort tag has been	*
					      * initiated on this request.*/
#define SZ_CMD_TIMEOUT ((U32)0x00080000)  /* A timeout has occurred */ 

/*
 * "errors_logged" bits for the SIM_WS.
 */
#define SZ_ERRLOG_DATAIN	0x0001	/* Logged an error on datain phase */
#define SZ_ERRLOG_MSGIN		0x0002	/* Logged an error on msgin phase */
#define SZ_ERRLOG_CMDTMO	0x0004	/* Logged an error on cmd timeout */

/*
 * Now call it directly.  
 */
#define SIM_SCHED_ISR() scsiisr()

/*
 * Forward structure declarations.  This will allow the use of the
 * structure as a pointer before the structure is defined.
 */
struct sim_softc;
struct nexus;
struct sim_ws;
struct sim_sm;
struct sim_sm_data;

/*
 * Initiator/Target nexus information.
 *
 * If this structure is changed, up the IT_NEXUS_VERS number.
 */
typedef struct it_nexus {
#define IT_NEXUS_VERS 1
    struct sim_ws *flink;
    struct sim_ws *blink;
    short sync_offset;		/* sync offset value for this device	*/
    short sync_period;		/* sync period value for this device	*/
    U32 flags;		/* sync state 				*/

    /*
     * Request sense info, for target mode.
     */
    ALL_REQ_SNS_DATA *reqsns_data;

    /*
     * Inquiry data, for target mode.
     */
    ALL_INQ_DATA *inq_data;

} IT_NEXUS;

/*
 * TAG_ELEMENT -
 *
 * There is one TAG_ELEMENT for each tag in the CAM subsystem.
 * The Active Tag Array which is allocated during initialization is
 * is an array of TAG_ELEMENTS. The Active Tag Array (ATA) is used
 * during the allocation and deallocation of SIM Tags as well as allows
 * the SIM to map from a tag value to a sim_ws address.
 *
 * If this structure is changed, up the TAG_ELEMENT_VERS number.
 */
typedef struct tag_element {
#define TAG_ELEMENT_VERS 1
  struct tag_element *flink;
  struct sim_ws *sim_ws;/* Address of sim_ws or blink */
  I32 tag;		/* Tag value for this tag (usually equal to index) */
  I32 flags;		/* Is this element in use?, etc. */
} TAG_ELEMENT;

/*
 * The SIM_WS ("SIM Working Set") structure is the per I/O request
 * structure, used by the SIM. The SIM_WS contains the per I/O variables
 * used by the SIM during the execution of a CAM CCB.  This structure
 * has been defined within the nexus structure to allow use with
 * DBX.
 *
 * If this structure is changed, up the SIM_WS_VERS number.
 */
typedef struct sim_ws {
#define SIM_WS_VERS 2
    struct sim_ws *flink;	/* forward link				*/
    struct sim_ws *blink;	/* backward link			*/
    U32 cntlr;		/* logical controller number of this HBA*/
    U32 targid;		/* target identifier			*/
    U32 lun;			/* logical unit identifier		*/
    U32 cam_status;		/* set to request aborted, active, 	*
			 	 * inactive, etc.			*/
    U32 tag;			/* set to tag if command queuing is	*
			 	 * used, set to -1 if tag pending	*/
    U32 seq_num;		/* Sequence number assigned by SIMX in  *
 				 * sim_action for CAM_SCSIIO requests   */
    U32 time_stamp;		/* Time stamp of when SIM_WS is put     *
				 * on the NEXUS list                    */
    struct nexus *nexus;	/* Points to the I_T_L NEXUS structure	*/
    IT_NEXUS *it_nexus;		/* Points to the I_T Nexus struct	*/
    struct sim_softc *sim_sc;	/* Points to the sim_softc structure	*
				 * associated with the controller that	*
				 * this target/lun resides on.		*/
    CCB_SCSIIO *ccb;		/* Points to the ccb which this request	*
				 * represents.				*/
    U32 phase_sum;		/* Bit description of phases which have	*
				 * occurred				*/
    I32 flags;			/* miscellaneous flags, includes tag	*
				 * flags, termio flag.			*/
    I32 cam_flags;		/* copied from the CCB for rapid access	*/
    U32 error_recovery;	/* Use bits defined above to determine	*
				 * error type.				*/
    U32 recovery_status;	/* What has been done to recover from	*
				 * an error.				*/
    void (*as_callback)();	/* Temporary holder for callback routine*
				 * used during autosense.		*/
    CCB_SCSIIO *as_ccb;		/* Temporary holder for the CCB during	*
				 * autosense.				*/
    void (*tmo_fn)();		/* function pointer to a routine which	*
				 * will be called when a time-out	*
				 * occurs.				*/
    void *tmo_arg;		/* Argument passed to the timeout	*
				 * function when it is called.  	*/
    SIM_TARG_WS targ_ws;	/* Working set used during target mode	*/
    struct sim_ws *linked_ws;
				/* Used for linked commands.	 	*/
    struct timeval time;	/* Time structure used by the SIM timer */
    CIR_Q msgoutq;		/* Circular queue used for message out	*/
    CIR_Q msginq;		/* Circular queue used for message in	*/
    CIR_Q phaseq;		/* Circular queue used for phases	*/

    u_char msgoutq_buf[SIM_MAX_MSGOQ];
				/* Buffer for msgout queue		*/
    u_char msginq_buf[SIM_MAX_MSGIQ];
				/* Buffer for msgin queue		*/
    u_char phaseq_buf[SIM_MAX_PHASEQ];
				/* Buffer for phase queue.		*/
    u_char msgout_sent;		/* Number of message out bytes sent	*
				 * from current sequence.		*/
    u_char scsi_status;		/* set to SZ_STAT_ code defines		*/
	
    u_short errors_logged;	/* Used to allow an error which repeats	*
				 * to only be logged once.		*/

    /*
     * SIM_HBA specific data.
     */
    U_WORD hba_data[SIM_HBA_DATA];
	
    /*
     * Error counts.
     */
    U32 abort_cnt;		/* number of times we have tried to	*
				 * abort this I/O.			*/
    U32 lostarb;		/* count kept for number of lost	*
				 * arbitrations				*/
    /*
     * Data transfer information. Separate structures are kept for the
     * different bus phases. This allows for modified data pointers and
     * message rejects without any loss of information.
     */
    DME_DESCRIPTOR msgin_xfer;
    DME_DESCRIPTOR msgout_xfer;
    DME_DESCRIPTOR command_xfer;
    DME_DESCRIPTOR data_xfer;
    DME_DESCRIPTOR status_xfer;

    /*
     * If non-zero, used to sort on.
     */
    U32 cam_sort;

    /*
     * Priority level used to insert this SIM_WS into the NEXUS queue.
     */
    u_short cam_priority;

#ifdef SIM_FUNCQ_ON
    CIR_Q funcq;                /* Circular queue used for func flow    */
    char *funcq_buf[SIM_MAX_FUNCQ];
#endif

} SIM_WS;

/*
 * Initiator/Target/Lun NEXUS information.
 *
 * If this structure is modified, up the NEXUS_VERS number.
 */
typedef struct nexus {
#define NEXUS_VERS 3
    SIM_WS *flink;		/* Pointer to the first WS in NEXUS Q	*/
    SIM_WS *blink;		/* Pointer to the last WS in NEXUS Q	*/

    struct nexus *nexus_flink;	/* Pointer to next active nexus.	*/
    struct nexus *nexus_blink;	/* Pointer to previous active nexus.	*/

    /*
     * Per targ/lun information.
     */
    short termio_pend_cnt;	/* number of requests related to this	*
				 * structure which must be terminated.	*/
    short abort_pend_cnt;   	/* number of requests related to this	*
			     	 * structure which must be aborted.	*/
    U32 flags;		/* misc. flags. Used to determine if	*
				 * the device can disconnect		*
				 * if frozen, if device needs to	*
				 * be aborted, if target's tagged queue	*
				 * is full, if active non-tagged	*/
    TAG_ELEMENT *ata;           /* Pointer to the active tag array      */
    TAG_ELEMENT *ata_list;      /* Pointer to the head of the ATA linked*
                                 * list.                                */
    U32 sws_count;		/* Number of SIM_WS's associated with	*
				 * this NEXUS.				*/

    /*
     * Error logging information.
     */
    U32 parity_cnt;		/* number of parity errors which have	*
				 * occurred.				*/
    /*
     * Sorting fpointers, and counters.
     */
    SIM_WS *curr_list;
    U32 curr_cnt;
    U32 curr_time;		/* Time stamp of oldest SIM_WS on list */
    SIM_WS *next_list;
    U32 next_cnt;
    U32 next_time;		/* Time stamp of oldest SIM_WS on list */
    SIM_WS *as_simws_ptr;	/* autosense sim working set pointer	*/
    int freeze_count;		/* Freeze counter for tags		*/

} NEXUS;

/*
 * One SIM_SOFTC structure is used by each controller.
 *
 * If this structure is modified, up the SIM_SOFTC_VERS number.
 */
typedef struct sim_softc {
#define SIM_SOFTC_VERS 2
	void *reg;		/* address of the hba's register set	*/
	void *rambuf;		/* address of hba's rambuffer (may not	*
				 * be used)				*/
	void *dmaeng;		/* address of DMA engine register set   */
	void *ifchip1;		/* future expansion			*/
	U16 scsiid;		/* SCSI id for this HBA			*/
	U8 path_inq_flags;	/* Flags to pass back in path inq	*/
	U8 simsc_unused_flags;	/* future path inq flags (for alignment) */
	U32 cntlr;		/* logical controller number of this HBA*/
	SIM_WS *active_io;	/* set to NULL if no active I/O,	*
				 * otherwise points to active SIM_WS	*/
	U32 sync_offset;	/* max. sync offset for this HBA	*/
	U32 sync_period;	/* max. sync period for this HBA	*/
	U32 max_tag;		/* maximum number of tags for this	*
				 * controller				*/
	U32 flags;		/* misc. flags, i.e. SZ_TRYING_SELECT	*/
        TAG_ELEMENT *ata;	/* Pointer to the Active Tag Array.     *
				 * This pointer is here for debug only  */
	TAG_ELEMENT *ata_list;  /* Pointer to head of ATA list.         * 
				 * This pointer is here for debug only  */
	DME_STRUCT *dme;	/* The structure is used to support DME.*
				 * This is a ptr to a structure to allow*
				 * different DME engines to be handled	*
				 * consistently.			*/
	U32 simx_init;	/* Flag used to track state of SIM XPT 	*/
	U32 sims_init;	/* Flag used to track state of SIM SCHED*/
	U32 simh_init;	/* Flag used to track state of SIM HBA 	*/
	U32 simd_init;	/* Flag used to track state of SIM DME 	*/

	/*
	 * HBA specific pointers
	 */
	void *hba_sc;		/* pointer to HBA specific softc	*
				 * structure				*/
	U32 (*hba_init)();	/* initialize the HBA			*/
	U32 (*hba_go)();	/* function to start a command off	*
				 * within the hba.			*/
	U32 (*hba_sm)();	/* HBA specific state machine		*/
	U32 (*hba_bus_reset)();
				/* function to generate a SCSI bus	*
				 * reset				*/
	U32 (*hba_send_msg)();
				/* send a message byte			*/
	U32 (*hba_xfer_info)();
				/* transfer info without the use of the	*
				 * DME					*/
	U32 (*hba_sel_msgout)();
				/* arbitrate, select, and go to message	*
				 * out					*/
	U32 (*hba_msgout_pend)();
				/* request a message out phase		*/
	U32 (*hba_msgout_clear)();
				/* release the ATN line			*/
	U32 (*hba_msg_accept)();
				/* accept the message byte last read	*/
	U32 (*hba_setup_sync)();
				/* setup the HBA for specified sync	*
				 * offset and period.			*/
	U32 (*hba_discard_data)();
				/* throw away one byte coming in from	*
				 * the SCSI bus.			*/
	U32 (*hba_chip_reset)();
	                        /* used to reset hba hardware */

	/*
	 * Functions which are dependent on the Scheduler being used.
	 */
	U32 (*sched_start)();	/* setup the given SIM_WS with messages *
				 * and anything else needed to begin    *
				 * the I/O.			        */
	U32 (*sched_run_sm)();	/* Calls or schedules the state machine */
	U32 (*sched_abort)();	/* perform a SCSI abort operation	*/
	U32 (*sched_termio)();	/* perform a SCSI terminate I/O         */
	U32 (*sched_bdr)();	/* perform a bus device reset		*/

	/*
	 * Target mode routines.
	 */
	U32 (*hba_targ_cmd_cmplt)();
	U32 (*hba_targ_recv_cmd)();
	U32 (*hba_targ_send_msg)();
	U32 (*hba_targ_disconnect)();
	U32 (*hba_targ_recv_msg)();

	U32 error_recovery;	/* Use bits defined above to determine	*
				 * error type.				*/

	/*
	 * Device reset, abort, and terminate I/O flags.  Device reset
	 * over-rides aborts and terminate I/O requests.  Abort over-rides
	 * terminate I/O.
	 */
	U32 device_reset_needed;
				/* each bit in device_reset_needed	*
				 * corresponds to a target.  A target's	*
				 * bit set means that a device reset	*
				 * needs to be performed on that target.*/
	U32 device_reset_inprog;
				/* A device reset is currently in	*
				 * progress for the associated target	*
				 * with its bit set.			*/
	short abort_pend_cnt;	/* number of requests related to this	*
				 * structure which must be aborted.	*/
	short termio_pend_cnt;	/* number of terminate I/O requests	*
				 * pending for this controller.		*/
	/*
	 * Control structures which define the sim and hba interface.
	 * This is the I_T_L Nexus information.
	 */
	NEXUS nexus[NDPS][NLPT];
				/* The nexus structure is the head of	*
				 * the linked list used to store the	*
				 * active and pending requests for a T/L*/

	/*
	 * Control information for the I_T Nexus.  Currently only
	 * sync info is kept here.
	 */
	IT_NEXUS it_nexus[NDPS];

	/*
	 * Temporary SIM_WS.  Used during the reselection process and
	 * during error procedures.
	 */
	SIM_WS tmp_ws;

	/*
	 * NEXUS list information used by the scheduler.
	 */
	NEXUS nexus_list;
	NEXUS *next_nexus;
	U32 nexus_cnt;

	/*
	 * Error logging information.
	 */
	U32 err_recov_cnt;   /* used by error recovery routines	*/
	U32 bus_reset_count;	/* Count of bus resets for this contrl	*/

	/*
	 * Location to store the uba_ctlr * and csr from the probe call.
	 * Version 1.15
	 */

	void * um_probe;
	caddr_t	csr_probe;

	/*
	 * SIM_WS used for bus device resets.
	 */
	SIM_WS bdr_sws;

	/*
	 * Two lock structures will be used.  reg_lk will be used when
	 * any HBA registers (reg) are accessed. softc_lk will be used
	 * for all other fields within SIM_SOFTC.  If both locks are
	 * needed, softc_lk must be obtained before reg_lk.
	 */
	lock_data_t softc_lk;
	lock_data_t reg_lk;

	/*
	 *  This contains the active interrupt context.
         */
	struct sim_sm_data *active_interrupt_context;

#ifdef SIM_FUNCQ_ON
	/*
	 * SIM_SOFTC functional queue.  This must stay at the
	 * bottom of the structure.
	 */
	CIR_Q funcq;                /* Circular queue used for func flow    */
	char *funcq_buf[SIM_MAX_FUNCQ];
#endif

} SIM_SOFTC;

/*
 * Structures used by the SIM State machine.  The SIM_SM_DATA structure
 * contains a pointer to the HBA specific interrupt data and a pointer to
 * the corresponding SIM_WS.  This information will be placed on the
 * State Machine's queue by the HBA's interrupt handler.  It will then
 * be removed by the State Machine's soft interrupt handler, scsiisr(),
 * and passed to the HBA specific state machine handler.  The structure
 * SIM_SM is used by the state machine for communication with other layers
 * and to maintain its own private data.
 *
 * If this structure is modified, increment the SIM_SM_DATA_VERS number.
 */
typedef struct sim_sm_data {
#define SIM_SM_DATA_VERS 1
    u_char *hba_intr;
    SIM_SOFTC *sim_sc;
} SIM_SM_DATA;

/*
 * If this structure is modified, increment the SIM_SM_VERS number.
 */
typedef struct sim_sm {
#define SIM_SM_VERS 1
    U32 sm_active;
    U32 sm_queue_sz;
    /*
     * If a controller detects a bus reset, the controller should set
     * its bit in "bus_reset."
     */
    U32 bus_reset;

    /*
     * If a controller has an IO waiting to be started, it should
     * set its bit in "waiting_io."
     */
    U32 waiting_io;
    CIR_Q sm_queue;
    SIM_SM_DATA *sm_data;
    lock_data_t sm_lk;
} SIM_SM;

/*
 * SMP lock and unlock the SIM_SOFTC structure (all but the "reg" field).
 */
#define SIM_SOFTC_LOCK_INIT(sc); {					\
    lock_init( &((sc)->softc_lk), TRUE );				\
}
#define SIM_SOFTC_LOCK(s, sc); {					\
    s = splbio();							\
    CAM_LOCK_IT( &((sc)->softc_lk), LK_RETRY );				\
}
#define SIM_SOFTC_UNLOCK(s, sc); {					\
    CAM_UNLOCK_IT( &((sc)->softc_lk) );					\
    splx(s);								\
}

/*
 * SMP lock and unlock the SIM_SOFTC "reg" field.
 */
#define SIM_REG_LOCK_INIT(sc); {					\
    lock_init( &((sc)->reg_lk), TRUE );					\
}
#define SIM_REG_LOCK(s, sc); {						\
    s = splbio();							\
    CAM_LOCK_IT( &((sc)->reg_lk), LK_RETRY );				\
}
#define SIM_REG_UNLOCK(s, sc); {					\
    CAM_UNLOCK_IT( &((sc)->reg_lk) );					\
    splx(s);								\
}

/*
 * SMP lock and unlock the SIM_SM
 */
#define SIM_SM_LOCK_INIT(sm); {						\
    CAM_SPIN_LOCK_INIT( &((sm)->sm_lk), &lock_device15_d );		\
}
#define SIM_SM_LOCK(s, sm); {						\
    s = splbio();							\
    CAM_LOCK_IT( &((sm)->sm_lk), LK_RETRY );				\
}
#define SIM_SM_UNLOCK(s, sm); {						\
    CAM_UNLOCK_IT( &((sm)->sm_lk) );					\
    splx(s);								\
}

/*
 * CAM_SIM_PRIV -
 *
 * This structure describes the overlay for the SIM private data space in
 * the SCSI I/O CCB.
 * 
 * Note: at this time the SIM private data is about 50 bytes long.  Make sure
 * that this structure does not go over that. 
 *
 * If this structure is modified, increment the CAM_SIM_PRIV_VERS number.
 */

#define DEC_VALID	0xDEC00DEC	/* Magic # for the valid field */

typedef struct cam_sim_priv {
#define CAM_SIM_PRIV_VERS 1
    SIM_WS *sim_ws;			/* pointer to this I/O's working set */
    U32 valid;			/* indicates that sim_ws is valid */
} CAM_SIM_PRIV;

#endif /* _SIM_ */

