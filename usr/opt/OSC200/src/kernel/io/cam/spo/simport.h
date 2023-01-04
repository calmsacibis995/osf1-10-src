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
 * @(#)$RCSfile: simport.h,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1993/09/21 21:53:51 $
 */
/* ----------------------------------------------------------
 * simport.h
 */
#ifndef _SIMPORT_INCL_
#define _SIMPORT_INCL_

#include <io/common/devdriver.h>

#define MAXCHAN	16
#define SIMPORT_SOFTC_VERS      1

/* 
 *  SIMport Data Structure Declaration
 */
typedef union {
        caddr_t ptr;
        U32 size[2];
} PTR;

/*
 *  SIMport Queue Carrier Declaration
 *  The Queue Carrier is the SIMport defined data structures used to
 *  form the interface queues between the host and the adapter.
 *  The Queue Carrier is used to pass queue buffers between the host and
 *  the adapter. It contains a link field which is used to form a
 *  one-way linked list. It contains a pointer to a queue buffer which
 *  must be formatted as a CAM or SIMport CCB command.  It also contains
 *  queue buffer and queue carrier token fields which are host virtual
 *  addresses. 
 */
typedef struct _spqcar {        /* SIMport carrier structure */
    PTR 	next;		/* Pointer to next queue carrier, physical */
    PTR 	bufp;		/* Queue buffer pointer, physical */
    PTR 	buftkn;		/* Queue buffer token, virtual */
    PTR 	cartkn;		/* Queue carrier token, virtual */ 
    U8	spo_func_code;		/* CAM or SIMport function code */
    U8	spo_status;		/* Stuatus of CAM or SIMport command */
    U16	spo_flags;		/* Flags for queue insert pri and status */
    U8	spo_rsvd;		/* CAM defined connect id, reserved */
    U8	spo_path_id;		/* CAM pathid, part of connection id */
    U8 	spo_target_id;		/* CAM target id, part of connection id */
    U8	spo_target_lun;		/* CAM lun id, part of connection id */
} SPQCAR;

#define spo_next_ptr	next.ptr
#define spo_buf_ptr 	bufp.ptr
#define spo_buf_token	buftkn.ptr
#define spo_car_token	cartkn.ptr

#define SPQ_STOPPER_ZERO	0x0
#define SPQ_STOPPER_ONE		0x1

#define SPQ_TAIL_INSERT		0x0	
#define SPQ_HEAD_INSERT		0x1	

typedef union spqptr {
    SPQCAR* ptr;	
    U32	size[2];
} SPQPTR;

/*
 *  SIMport Queue Header Declaration
 */
typedef struct _spqhdr    {               /* N_PORT Queues */
    SPQPTR         hptr;                  /* Queue header pointer */
    SPQPTR         tptr;                  /* Queue tail pointer */
} SPQHDR;

#define spo_head_ptr	hptr.ptr
#define spo_tail_ptr	tptr.ptr


/* 
 *  SIMport Adapter Block 
 *
 *  The Adapter Block contains initialization parameters and the links for
 *  the SIMport interface queues between the adapter and the host.
 *  spo_qb_size is the number of bytes in a SIMport queue buffer.
 *  spo_ccb_ptr_sz is the number of bytes in a SIMport host pointer field.
 *  The SPQHDR structures contain the head and tail queue carrier
 *  addresses for the SIMport queues.
 */ 
typedef struct _spab {
    U16	spo_qb_size;     /* Number of bytes in the queue buffer */
    U8	spo_ccb_ptr_sz;  /* Number of bytes allocated for a CCB pointer */ 
    U8	spo_pad[5];      /* Should be zero */
    U8	spo_pad2[8];     /* Should be zero */
    SPQHDR spo_dacqhd;	 /* Driver-Adapter command queue header */
    SPQHDR spo_adrqhd;	 /* Adapter-Driver response queue header */	
    SPQHDR spo_dafqhd;	 /* Driver-Adapter free queue header */
    SPQHDR spo_adfqhd;	 /* Adpater-Driver free queue header */
} SPAB;

/*
 * SIMport Buffer Segment Descriptor
 *
 * The BSD is a data structure used to contain the physical address
 * and size of a contiguous segment of host memory. The spo_type flag
 * indicates whether the format of the memory segment is that of a
 * host memory segment or of a SIMport Buffer Segment Map (BSM) . A
 * BSM is a list of BSDs and is used to address a host buffer that
 * consists of multiple host memory segments. The spo_bcount is the
 * number of bytes in the segment or in the BSM. The remaining fields
 * contain the host address bits of the memory segment or of the BSM.
 */
typedef struct spobsd
{
    U32	spo_type:2;		/* Buffer type */
    U32	spo_bufptr_lo:30;	/* Buffer pointer, lower 30 bits */
    U32	spo_bufptr_hi:16;	/* Buffer pointer, upper 16 bits */
    U32 spo_bcount:16;		/* Size in bytes of the buffer */
} SPOBSD;

#define SPO_BSD_TYPE	0	/* The BSD contains pointer to the host buf. */
#define SPO_BSM_TYPE	1	/* The BSD contains pointer to the BSMs */

/*
 *  SIMport Vendor Unique Commands and Messages
 */

/*
 * SIMport vendor unique function code definitions 
 */
#define	SPO_VU_SET_ADAP_STATE	0x80	/* Set Adapter State Command */
#define	SPO_VU_ADAP_STATE_SET	0x80	/* Adapter State Set Response Message */
#define	SPO_VU_SET_PARAM	0x81	/* Set Parameters Command */
#define	SPO_VU_PARAM_SET	0x81	/* Parameters Set Response Message */
#define	SPO_VU_SET_CHAN_STATE	0x82	/* Set Channel State Command */
#define	SPO_VU_CHAN_STATE_SET	0x82	/* Channel State Set Response Message */
#define SPO_VU_SET_DEV_STATE	0x83	/* Set Device State Command */
#define SPO_VU_DEV_STATE_SET	0x83	/* Device State Set Response Message */ 
#define SPO_VU_VERI_ADAP_SANITY	0x84	/* Verify Adapter Sanity Command */ 
#define SPO_VU_ADAP_SANITY_VERI	0x84	/* Adapter Sanity Verify Response */ 
#define SPO_VU_READ_CNTRS 	0x85	/* Read Counters Command */
#define SPO_VU_CNTRS_READ 	0x85	/* Counters Read Response Message */
#define SPO_VU_BSD_RSPN_CMD	0x86	/* BSD Response Command */
#define SPO_VU_BSD_REQUEST	0x86	/* BSD Requeset Message */
#define SPO_VU_BUS_RESET_CMD	0x87	/* Bus Reset Response Command */ 
#define SPO_VU_BUS_RESET_REQ	0x87	/* Bus Reset Request Message */ 
#define SPO_VU_UNSOL_RESEL	0x88	/* Unsolicited Reselection Message */
#define SPO_VU_CHAN_DISABLED	0x89	/* Channel Disabled Message */
#define SPO_VU_DEV_DISABLED	0x8a	/* Device Disabled Message */ 

/* 
 * Set Adapter State Command 
 */
typedef struct ccb_spvu_set_adpt
{
    CCB_HEADER  spo_cam_ch;     /* Header information fields */
    U8	spo_state;	/* The request state */
    U8	spo_pad[7];	/* Should be zero */
} CCB_SPVU_SET_ADPT;

/*
 * Adapter State Set Response Message 
 */
typedef struct ccb_spvu_state_set
{
    CCB_HEADER  spo_cam_ch;     /* Header information fields */
    U8	spo_state;	/* Adapter state */
    U8	spo_n_chan;	/* The number of SCSI channels */
    U16	spo_flags;	/* Supported features */
    U16	spo_n_adapt_qc;	/* Maximum number of queued comands */
    U8	spo_ka_time;	/* Minimum time in seconds for KEEPALIVE */
    U8	spo_n_freeq;	/* Minimum number of free queue entries */
    U8	spo_n_host_sg;	/* The number of host 4K segments memory */
    U8	spo_xfer_align;	/* Adapter alignment information */
    U8	spo_n_aen;	/* Recommended number of AEN buffers */
    U16	spo_pad[5];	/* Shuold be Zero */
    U8 	spo_nodes_on_chan[MAXCHAN];	/* Maximum number of nodes each channel */
} CCB_SPVU_STATE_SET;

/* word count */
#define	SPO_XFER_ALIGN_BYTE	0 
#define	SPO_XFER_ALIGN_WORD	1 
#define	SPO_XFER_ALIGN_LONGWORD	2 
#define	SPO_XFER_ALIGN_QUADWORD	3 
#define	SPO_XFER_ALIGN_OCTAWORD	4 

/*
 * Adapter State Set Response Message flags field definition.
 */
#define SPO_INTR_HOLDOFF	0x01  /* Support the intr. holdoff timer */
#define SPO_SOFT_SETTING	0x02  /* Supp. the soft setting of SCSI node */
#define SPO_LINKED_BSM		0x04  /* Support the linked BSMs */
#define SPO_BSD_REQUEST		0x08  /* Support BSD Request unsolicited */

#define SPO_WORD_MASK		0x01
#define SPO_LWORD_MASK		0x03
#define SPO_QWORD_MASK		0x07
#define SPO_OWORD_MASK		0x0f

/* 
 * Set Parameters Command 
 */
typedef struct ccb_spvu_set_param
{
    CCB_HEADER  spo_cam_ch;     /* Header information fields */
    U8  spo_pad;         	/* Should be zero */
    U8	spo_n_host_sg;		/* Host 4K segments allocated for adapter */
    U16 spo_flags;          	/* Flags controlling adapter operation */ 
    U32	spo_system_time;	/* system time */
    U32	spo_int_holdoff;	/* Completion interrupt holdoff timer control */
    U32	spo_rp_timer;		/* The reset pending timer */
    SPOBSD  spo_host_sg_bsd;	/* Buffer segment descriptor allocated */
} CCB_SPVU_SET_PARAM;

/*
 * Paramters Set Response Message 
 */
typedef struct ccb_spvu_param_set
{
    CCB_HEADER  spo_cam_ch;         /* Header information fields */
} CCB_SPVU_PARAM_SET;

/* 
 * Set Channel State Command 
 */
typedef struct ccb_spvu_set_chnl
{
    CCB_HEADER  spo_cam_ch; /* Header information fields */
    U8      spo_state;      /* The request state */
    U8      spo_pad[3];     /* Should be zero */
    U8	    spo_chan_id;    /* Channel id */
    U8	    spo_node_id;    /* SCSI bus node id */
    U8      spo_pad2[2];    /* Should be zero */
} CCB_SPVU_SET_CHNL;

/*
 * Channel State Set Response Message
 */
typedef struct ccb_spvu_chnl_set
{
    CCB_HEADER  spo_cam_ch;     /* Header information fields */
    U8  spo_state;      /* The current state */
    U8  spo_pad[3];     /* Should be zero */
    U8	spo_chan_id;	/* Channel id */
    U8	spo_node_id;	/* Assigned SCSI bus node id */
    U8	spo_pad2[2];	/* Should be zero */
} CCB_SPVU_CHNL_SET;

/*
 * Set Device State Command 
 */
typedef struct ccb_spvu_set_device
{
    CCB_HEADER  spo_cam_ch;        /* Header information fields */
    U8      spo_state;         /* The request state */
    U8      spo_pad[7];        /* Should be zero */
} CCB_SPVU_SET_DEVICE;

/*
 * Device State Set Response Message 
 */
typedef struct ccb_spvu_device_set
{
    CCB_HEADER  spo_cam_ch;         /* Header information fields */
    U8	spo_state;          /* The current state */
    U8	spo_pad[7];	    /* Should be zero */
} CCB_SPVU_DEVICE_SET;

/*
 * Verify Adapter Sanity Command
 */
typedef struct ccb_spvu_ver_adap_sanity
{   CCB_HEADER  spo_cam_ch;     /* Header information fields */
} CCB_SPVU_VER_ADAP_SANITY;

/*
 * Verify Adapter Sanity Command
 */
typedef struct ccb_spvu_adap_sanity_rspn
{   CCB_HEADER  spo_cam_ch;     /* Header information fields */
} CCB_SPVU_ADAP_SANITY_RSPN;

/*
 * Read Conuters Command 
 */
typedef struct ccb_spvu_read_cntr
{   
    CCB_HEADER  spo_cam_ch;         /* Header information fields */
    U8      spo_pad[2];         /* Should be zero */
    U8      spo_flags;          /* Flags controlling adapter operation */
    U8      spo_pad2[5];        /* Should be zero */
} CCB_SPVU_READ_CNTR;

/*
 * Counters Read Response Message
 */
typedef struct ccb_spvu_cntr_read_rspn 
{
    CCB_HEADER  spo_cam_ch;         /* Header information fields */
    U8      	spo_pad[4];	    /* Should be zero */
    U32		spo_cntrs[27];	    /* SIMport counters */ 
} CCB_SPVU_CNTR_RSPN;

/* 
 * BSD Request Message
 */
typedef struct ccb_spvu_bsd_req
{
    CCB_HEADER  spo_cam_ch;     /* Header information fields */
    U8	spo_buf_id;	/* The buffer within the private data of ccb */
    U8	spo_pad[3];	/* Should be zero */
    U32	spo_offset;	/* The byte offset from the start of the buffer */
    PTR	CCB_VA;		/* Host virtual address of the CCB */ 
} CCB_SPVU_BSD_REQ;

#define spo_ccb_va	CCB_VA.ptr

#define SPO_BUFID_DATA		1
#define SPO_BUFID_CDB		2
#define SPO_BUFID_SENSE		3
#define SPO_BUFID_MESSAGE	4

/*
 * BSD Response Command 
 */

#define SPO_BSD_RSPN_ENTRY	12


typedef struct ccb_spvu_bsd_rspn
{   
    CCB_HEADER  spo_cam_ch;     /* Header information fields */
    U8      spo_buf_id;         /* The buffer field with the command */ 
    U8      spo_pad[3];        	/* Should be zero */
    U32	    spo_offset;		/* Offset from the start of the data buf */
    PTR	    CCB_VA;		/* Host virtual address of the I/O CCB */
    SPOBSD  spo_bsd[SPO_BSD_RSPN_ENTRY];  /* Buffer segment Descriptor */
} CCB_SPVU_BSD_RSPN;

/*
 * Bus Reset Request Message 
 */
typedef struct ccb_spvu_bus_reset_req
{
    CCB_HEADER  spo_cam_ch;     /* Header information fields */
    U8	spo_reason;		/* Reason code for reset request */
    U8	spo_tgtid;		/* Target ID */
    U8  spo_pad[6];		/* Should be zero */
    PTR	CCB_VA;			/* The virtual address of the CCB active */
} CCB_SPVU_BUS_RESET_REQ;

/*
 * Bus Reset Response Command
 */
typedef struct ccb_spvu_bus_reset
{
    CCB_HEADER  spo_cam_ch;         /* Header information fields */
} CCB_SPVU_BUS_RESET;

/*
 * Device Disable Message
 */
typedef struct ccb_dev_disable
{
    CCB_HEADER  spo_cam_ch;         /* Header information fields */
} CCB_DEV_DISABLE;

/* 
 * SIMport Returned Status Codes, see SIMport spec. Table A-3
 */
#define SPO_SUCCESS	     1
#define SPO_INV_COMMAND	     0
#define SPO_NOT_ENABLED	    -1	/* The adapter/channel/function is not enable */
#define SPO_NOT_DISABLED    -2  /* The adapter/channel/func is not disable */
#define SPO_HOST_MEM	    -3	/* Required n_host_sg number of 4K page */
#define SPO_HOST_FQE	    -4	/* No free queue element */
#define SPO_INV_INT_HOLDOFF -5  /* Interrupt Holdoff timer invalid/not supp. */
#define SPO_HOST_SG	    -6  /* Insuff. host memory allocated */
#define SPO_INV_CHANID	    -7	/* Invalid channel ID or non-exist channel */
#define SPO_INV_NODEID	    -8	/* Invalid node ID or soft setting not supp. */
#define SPO_CANT_DISABLE    -9	/* A device channel cannot be disabled */
#define SPO_INV_OFFSET	    -10	/* Invalid offset value/offset out of range */ 
#define SPO_INV_CCB	    -11	/* Invalid CCB address */
#define SPO_INV_BUF_ID	    -12	/* Invalid buf_id field value */
#define SPO_NO_RESET	    -13	/* Adapter should not reset the SCSI bus */

#define SPO_BUS_RESET	     2
#define SPO_RESET_REJECT     3 /* Host denied the Adap's reset request */
#define SPO_BUS_DEV_RST	     4 /* bus Device Reset */
#define SPO_CHANN_HW_ERR     5 /* A hardware error has been detected */ 

#define SPOZONESIZE	4
#define NBSDSEGPBSM	17	/* Number of BSDs in the BSD segments */

/*
 * SIMport Buffer Segment Map
 *
 * The BSM is used to define a physically discontiguous host memory
 * buffer. The next_bsm_bsd field is a link to the next BSM, if
 * required. The num_entries field is a count of the number of valid
 * BSDs in the bsdseg. The Total_bytes field contains the total byte
 * count that this BSM maps. The buffer_offset field contains the
 * buffer offset from the start of the buffer that this BSM maps.  The
 * bsdseg field contains a BSD data structure for each physically
 * contiguous host memory segment.
 * The BSM is required to be hex aligned.  The system allocates a
 * page size of memory at the system initialization time  and divides 
 * the memory into BSM blocks.
 */
typedef struct spobsm
{
   SPOBSD next_bsm_bsd;
   U16    num_entries;
   U8 	  pad[6];
   U32	  total_bytes;	    /* Total number of bytes mapped by this BSM */
   U32	  buffer_offset;    /* Byte offset from the start of the host buffer */
   SPOBSD bsdseg[NBSDSEGPBSM]; /* BSD entries */
} SPOBSM;


typedef struct spobsm_header
{
    struct spobsm_header *spo_bsm_next;
    SPOBSM spo_bsm;
} SPOBSM_HEADER;

#define spo_bsm_bsd 	spo_bsm.next_bsm_bsd 
#define spo_bsm_entry 	spo_bsm.num_entries
#define spo_bsm_bytes	spo_bsm.total_bytes
#define spo_bsm_offset	spo_bsm.buffer_offset
#define spo_bsm_bsdseg	spo_bsm.bsdseg

/*
 *  SIMport Private Data Area of Execute SCSI I/O CCB 
 */
    
typedef struct ccb_spvu_scsiio_priv 
{
    U8	spo_scsi_state;
    U8 	spo_sen_res;
    U8	pad[2];
    U32		spo_data_res;
    SPOBSD	spo_data_bsd[2];
    SPOBSD	spo_cdb_bsd;
    SPOBSD	spo_sen_bsd;
    SPOBSD	spo_msg_bsd;
    PTR		spo_ccb_next_ptr;  /* physical address of next CCB if link */
} CCB_SPVU_SCSIIO_PRIV;


/*
 *  SIMport Buffer Segment Descriptor Manupilation Routine
 *  The design is to accomadate both 32 and 64 bits architecture.
 */
#define SPO_SETUP_BSD(bsdp, addr, len, align, t)		\
{								\
    (bsdp)->spo_type = (t);					\
    (bsdp)->spo_bufptr_lo = ((addr) >> (align)) & 0x3fffffff;	\
    (bsdp)->spo_bufptr_hi = ((addr) >> ((align)+30)) & 0xffff;	\
    (bsdp)->spo_bcount = (len);					\
}

#define SPO_ALLOC_BSM(spsc, bsmp)				\
{								\
    if (((spsc)->spo_freebsm) == NULL) {			\
	spo_bsm_alloc((spsc)); 					\
    }								\
    (bsmp) = (spsc)->spo_freebsm;				\
    if (bsmp) {							\
        (spsc)->spo_freebsm = 					\
		(SPOBSM_HEADER *)((bsmp)->spo_bsm_next);	\
	bsmp->spo_bsm_next = NULL;				\
    }								\
}

#define SPO_DEALLOC_BSM(spsc, bsmp)				\
{								\
    if ((bsmp)) {						\
        (bsmp)->spo_bsm_next = (spsc)->spo_freebsm;		\
        (spsc)->spo_freebsm = (bsmp);				\
    }								\
}

/*
 * Alpha dense address definition
 */
#ifdef __alpha
#ifndef DENSE
#define DENSE(x)       ((x) - 0x10000000)
#endif
#endif

/*
 *  SIMport state set value 
 */
#define SPO_STATE_ENABLED	1
#define SPO_STATE_DISABLED	2

/*
 *  SIMport Adapter Registers
 */
/*
 *      SIMport Registers Definitions
 */
/*
 * This definition intends to make the compiler that does not support
 * long long happy.
 */

typedef struct regbits {
#ifdef __alpha
    U64 paddr;
#else
    U32 paddr;
    U32 pad;
#endif
} REGBITS;


/*  
 *  SIMport Adapter Status Register Bit Fields Definitions
 *  The ASR register is a 64 bit register.  Currently SIMport
 *  does not define bits 32-63.
 *  Bits 10-30 are Should be Zero fields.
 */
#define SPO_ASR_0	0x00000001	/* Should be Zero */
#define SPO_ASR_1	0x00000002	/* Should be Zero */
#define SPO_ASR_ADSE	0x00000004  	/* Adapter Data Struture Error */
#define SPO_ASR_AMSE	0x00000008	/* Adapter Memory System Error */
#define SPO_ASR_AAC	0x00000010  	/* Adapter Abnormal Condition */ 
#define SPO_ASR_5	0x00000020	/* Should be Zero */
#define SPO_ASR_6	0x00000040	/* Should be Zero */
#define SPO_ASR_7	0x00000080	/* Should be Zero */
#define SPO_ASR_ASIC	0x00000100	/* Adapter Completion Interrupt Request */
#define SPO_ASR_UNIN	0x00000200	/* Unitialized State */
#define SPO_ASR_AME	0x00008000  	/* Adapter Maintenance Error */

#define SPO_4KSEG	4096
#define SPO_BSD_MAXLEN	(65536 - DME_PAGE_SIZE)   /* The maximum length for a BSD */

/*
 *  The following macros will redefine the host specific routines
 *  in the fields of sim_softc structure.  The operation overload
 *  has to be examined closely to guarantee there is no one use
 *  there fields except simport modules.
 */
#define spo_hba_set_abbr	hba_send_msg
#define spo_hba_read_asr	hba_sel_msgout
#define spo_hba_wrt_dacqir 	hba_msgout_pend
#define spo_hba_wrt_dafqir 	hba_msgout_clear 
#define spo_hba_ioccb_setup 	hba_xfer_info

/*
 *  The following macros overload the XFER_INFO field in the DMA_WSET
 *  for the unaligned data byte count.  This will save the computation
 *  when I/O returned with unaligned data.  The index and bxfer fields 
 *  in XFER_INFO are used for the byte count of safezone.  The saved.index 
 *  map to Zone A and saved.bxfer will map to Zone B.  The user unaligned
 *  buffer virtual addresses are also strore in the XFER vaddr field to 
 *  be used later. 
 */
#define spo_zone_a_count	saved.index	/* Number of byte in Zone A */
#define spo_zone_b_count	saved.bxfer	/* Number of byte in Zone B */
#define spo_zone_a_dest		saved.vaddr	/* Destination for Zone A */
#define spo_zone_b_dest		ahead.vaddr	/* Destination for Zone B */

/* 
 * SIMport Regsters
 */                                           
#define SPO_AMCSR	0
#define SPO_ABBR	1
#define SPO_DACQIR	2
#define SPO_DAFQIR	3
#define SPO_ASR		4
#define SPO_AFAR	5
#define SPO_AFPR	6


/* 
 *  Adapter Maintenance Control and Status Register - AMCSR
 */
#define	AMCSR_MIN	0x01	/* Maintenance Initialize */
#define	AMCSR_IE	0x08	/* Interrupt Enable */
/*
 *  SIMport PATHINQ Record Data Structure.  This declaration should be
 *  exactly identical to the CCB_PATHINQ except without the CCB_HEADER.
 */
typedef struct _spo_pathinq_rec {
    U8 cam_version_num;             /* Version number for the SIM/HBA */
    U8 cam_hba_inquiry;             /* Mimic of INQ byte 7 for the HBA */
    U8 cam_target_sprt;             /* Flags for target mode support */
    U8 cam_hba_misc;                /* Misc HBA feature flags */
    U16 cam_hba_eng_cnt;            /* HBA engine count */
    U8 cam_vuhba_flags[ VUHBA ];    /* Vendor unique capabilities */
    U32 cam_sim_priv;               /* Size of SIM private data area */
    U32 cam_async_flags;            /* Event cap. for Async Callback */
    U8 cam_hpath_id;                /* Highest path ID in the subsystem */
    U8 cam_initiator_id;            /* ID of the HBA on the SCSI bus */
    U8 cam_prsvd0;                  /* Reserved field, for alignment */
    U8 cam_prsvd1;                  /* Reserved field, for alignment */
    char cam_sim_vid[ SIM_ID ];     /* Vendor ID of the SIM */
    char cam_hba_vid[ HBA_ID ];     /* Vendor ID of the HBA */
    U8 *cam_osd_usage;              /* Ptr for the OSD specific area */
} SPO_PATHINQ_REC;

/* 
 *  SIMport specific softc structure
 */
typedef struct _simport_softc {
    SPAB*    		spo_abp;	/* Pointer to SPAB */
    U32			spo_state;	/* Adapter state */
    U32			spo_flags;	/* Supported features */
    U16			spo_n_adapt_qc;	/* Maximum number of queued comands */
    U8		spo_n_chan;	/* Number of SCSI channel */
    U8		spo_ka_time;	/* Minimum time in seconds for KEEPALIVE */
    U8		spo_n_freeq;	/* Minimum number of free queue entries */
    U8		spo_n_host_sg;	/* The number of host 4K segments memory */
    U8		spo_xfer_align;	/* Adapter alignment information */
    U8  spo_nodes_on_chan[MAXCHAN];	/* Maximum number of nodes each channel */
    U32			spo_int_holdoff;/* Interrupt holdoff timer */
    U32			spo_free_count;	/* Number of free queue carrier */
    SPQHDR		spo_freeq;	/* Free queue carrier list */
    SPOBSM_HEADER	*spo_freebsm;	/* Pointer to free SPOBSM list */
    U32			spo_bsm_count;	/* Free bsm count */
    U32			spo_bsmhdr_count;/* Free bsm header count */
    U32		(*spo_hba_pathinq)();	/* HW dependent pathinq routine */
    caddr_t	(*spo_get_reg_addr)();	/* Get the SIMport register addr */ 
    SPO_PATHINQ_REC	spo_pathinq_rec;/* Store the returned pathinq info */
    simple_lock_t	spo_dacqlk;	/* Driver-Adapter command queue lock */	
    simple_lock_t	spo_dafqlk;	/* Driver-Adapter free queue lock */
    simple_lock_t	spo_adrqlk;	/* Adapter-Driver response queue lock */
    simple_lock_t	spo_adfqlk;	/* Adapter-Driver free queue lock */
    simple_lock_t	spo_freeqlk;    /* Free queue carrier lock */
    simple_lock_t	spo_freebsmlk;  /* Free spobsm lock */
    simple_lock_t	spo_genericlk;  /* Generic lock, for other fields */ 
} SIMPORT_SOFTC;

/*
 * SIMport Adater State 
 */
#define SPO_UNIN_STATE		0x00000000	/* UNIN state */
#define SPO_INIT_STATE		0x00000001	/* INIT state */
#define SPO_ENABLED_STATE	0x00000002	/* ENABLED state */
#define SPO_RESET_PENDING	0x00000004	/* RST Pending state */
#define SPO_MISC_ERROR		0x00000008	/* Misc error state */
#define SPO_THREADS		0x00000010	/* SIMport thread started */
#define SPO_POLL_MODE		0x00000020	/* Polling mode */
#define SPO_VERI_SANITY		0x00000040	/* Verify adap sanity state */
#define SPO_PATHINQ_VALID	0x00000080	/* Pathinq information valid */ 

/*
 * SMP Lock; can be conditional compile in 
 */
#define SPO_SIMPLE_LOCK_INIT(l)	simple_lock_init((l))
#define SPO_SIMPLE_LOCK(l)	simple_lock((l))			
#define SPO_SIMPLE_UNLOCK(l)	simple_unlock((l))
#define SPO_LOCK_TRY(l)		simple_lock_try((l))	

#define SPO_IPL_SIMPLE_LOCK(s, l) {             \
        (s) = splbio();                         \
        simple_lock((l));                       \
}

#define SPO_IPL_SIMPLE_UNLOCK(s, l) {           \
        simple_unlock((l));                     \
        splx((s));                              \
}
	
#ifdef __mips__
#define CLEAN_DCACHE_ADDR(addr, size, mask) {			\
    vm_offset_t	pfn;						\
    svatophys((addr), &pfn);					\
    clean_dcache(((u_long)PHYS_TO_K0( pfn ) & ~(mask)), (size));\
}								
#else
#define CLEAN_DCACHE_ADDR(addr, size, mask)
#endif

/*
 * SPO_SCHED_ISR
 * SIMport interrupt service kernel thread.
 */
#define SPO_SCHED_ISR(sc)  				\
{							\
    if((sc)->simh_init == CAM_TRUE) 			\
	thread_wakeup_one((sc));			\
}
	
/*
 * SPO_GET_FREE_QCAR
 * 	Get a free queue carrier from the SIMport free queue 
 * Arguments:
 *      spsc:    Pointer to the SIMport softc structure 
 *      qcarp:   Pointer to a queue carrier to be returned  
 */
#define SPO_GET_FREE_QCAR_ENTRY(spsc, qcarp)             		\
{									\
    SPQHDR* freeq = &((spsc)->spo_freeq);				\
    if(freeq->spo_head_ptr) {						\
	qcarp = freeq->spo_head_ptr;					\
	freeq->spo_head_ptr = (SPQCAR *)((qcarp)->spo_next_ptr);	\
	if (freeq->spo_tail_ptr == (qcarp)) {				\
	    freeq->spo_tail_ptr = NULL;					\
	}								\
        spsc->spo_free_count--;						\
    }									\
    else {								\
        (qcarp) = NULL;							\
    }									\
}

/*
 * SPO_ADD_FREE_QCAR
 *	Added free queue carrier to the tail of the free queue.
 * Arguments:
 *	freeq:  A pointer to the free queue carrier
 *	qcarp:  A pointe to a queue carrier to be added
 */
#define	SPO_ADD_FREE_QCAR(spsc, qcarp)  				\
{									\
	SPQHDR* freeq = &((spsc)->spo_freeq);				\
	(qcarp)->spo_next_ptr = NULL;					\
	if(freeq->spo_tail_ptr) {					\
	    (freeq->spo_tail_ptr)->spo_next_ptr = (caddr_t)(qcarp);	\
	} else {							\
	    freeq->spo_head_ptr = (qcarp);				\
        }								\
	freeq->spo_tail_ptr = (qcarp);					\
	(spsc)->spo_free_count++;					\
}


#define writecsr

/*
 * SPO_ADD_QUEUE_ENTRY
 * Insert SIMport queue carrier to the tail of the queue.
 * Arguments:
 *	qhdr  Pointer to the queue header SPQHDR
 *	qcarp Pointer to the queue carrier to be inserted
 *	qbuf  Queue buffer pointer
 */
#define SPO_ADD_QUEUE_ENTRY(qhdr, qcarp, qbufp)  		\
{								\
	SPQCAR* qtail = (qhdr)->spo_tail_ptr;			\
    	(qcarp)->spo_next_ptr = SPQ_STOPPER_ZERO; 		\
	(qcarp)->spo_car_token = (caddr_t)(qcarp);		\
        svatophys((qbufp), qtail->spo_buf_ptr);			\
	qtail->spo_buf_token = (caddr_t)(qbufp);		\
	WBFLUSH();						\
	svatophys((qcarp), qtail->spo_next_ptr);		\
	qtail->spo_next_ptr += 1;  				\
	WBFLUSH();						\
	writecsr;	/* signal adapter */			\
	(qhdr)->spo_tail_ptr = (qcarp);				\
}

/*
 * SPO_REMOVE_QUEUE_ENTRY
 * Remove a queue carrier from the SIMport queue.  
 * Arguments:
 *	qhdr   Pointer to the queue header SPQHDR 
 *	qcarp  Pointer to the queue carrier
 *	qbufp  Pointer to the queue buffer
 *	lock   SMP lock for the queue
 * Returns:
 *	queue carrier is returned by qcarp
 *	queue buffer is returned by qbufp
 * Notes:
 *	SPQ_STOPPER_ZERO: Valid carriers have next pointer<0>
 */	
#define SPO_REMOVE_QUEUE_ENTRY(qhdr, qcarp, qbufp) 			    \
{									    \
	qcarp = (qhdr)->spo_head_ptr;					    \
	CLEAN_DCACHE_ADDR(qcarp, sizeof (SPQCAR), 0x1ff);		    \
	if (!(((unsigned long)((qcarp)->spo_next_ptr)) & SPQ_STOPPER_ZERO)) \
	{								    \
	    qbufp = (CCB_HEADER *)((qcarp)->spo_buf_ptr);		    \
	    CLEAN_DCACHE_ADDR(qbufp, sizeof(CCB_HEADER), 0x1ff); 	    \
	    (qhdr)->spo_head_ptr = (SPQCAR *)((qcarp)->spo_next_ptr);	    \
	    WBFLUSH();							    \
	}								    \
	else {								    \
	    (qcarp) = NULL;						    \
	}								    \
}

/*
 * SPO_CCB_ALLOC
 * Allocates a CCB.  This is the SIMport interface to xpt_ccb_alloc
 * routine.
 *
 * Arguments:
 *	Pointer to the CCB_HEADER structure.	
 *
 * Returns:
 *	CAM_BUSY if there is no more CCB available on the system.	
 *
 */
#define	SPO_CCB_ALLOC(ccbp) 					\
{								\
    if (((ccbp) = (CCB_HEADER *)xpt_ccb_alloc()) == NULL) {	\
    	return CAM_BUSY;					\
    }								\
}

/*
 * SPO_CCB_DEALLOC
 * Deallocates a CCB.  This is the SIMport interface to xpt_ccb_free
 * routine.
 *
 * Arguments:
 *	Pointer to the CCB_HEADER structure.	
 *
 */
#define SPO_CCB_DEALLOC(ccbp) 	(xpt_ccb_free(ccbp))

/*
 * Macro Name: SPO_GET_PATHID
 *     	Return the pathid based on the argument. 
 *
 * Arguments:
 *     	sc 	Pointer to the SIM_SOFTC structure. 
 */
#define SPO_GET_PATHID(sc)	(sc)->cntlr		
    
/*
 * Macro Name: SPO_CCB_INIT 
 *	Set the CCB pathid, targetid and lun to the current adapter.
 * Arguments:
 *      sc:   Pointer to the SIM_SOFTC structre
 *	ccbp: Pointer to the CCB header that needs to be initialized  
 */
#define SPO_CCB_INIT(sc, ccbp) 			\
{						\
    ccbp->cam_path_id = SPO_GET_PATHID((sc));	\
    ccbp->cam_target_id = (sc)->scsiid;		\
    ccbp->cam_target_lun = 0;			\
}

/*
 * Macro Name: SPO_SET_ADAP_STATE_BLD
 * 	Initialize the field in the adapter state structure
 * Arguments:
 *	spop:	Pointer to the CCB_SPVU_SET_ADAP structure.
 *	state:	Enable or diable.
 */
#define	SPO_SET_ADAP_STATE_BLD(spop, state) 		\
{							\
    (spop)->spo_state = state;				\
    bzero(&((spop)->spo_pad[0]), 7);			\
} 

/*
 * Macro Name: SPO_SET_PARAM_BLD
 *      Initialize the field in the adapter state structure
 * Arguments:
 *      spop:   A pointer to the CCB_SPVU_SET_ADAP structure.
 *      state:  Enable or diable.
 */
#define SPO_SET_PARAM_BLD(spop, flags, sys_time, 		\
		int_holdoff, rp_timer, n_host_sg, host_sg_bsd) 	\
{                                              		        \
    (spop)->spo_n_host_sg = n_host_sg;				\
    (spop)->spo_pad = 0;					\
    (spop)->spo_flags = flags;					\
    (spop)->spo_system_time = sys_time;				\
    (spop)->spo_int_holdoff = int_holdoff;			\
    (spop)->spo_rp_timer = rp_timer;				\
    (spop)->spo_host_sg_bsd = host_sg_bsd;			\
}

/*
 * Macro Name: SPO_SET_CHANNEL_STATE_BLD
 *      Initialize the field in the set channel state structure
 * Arguments:
 *      spop:   A pointer to the CCB_SPVU_SET_ADAP structure.
 *      state:  Enable or diable.
 *	chan_id: The internal identifier of the channel id
 *	node_id: The SCSI bus node id for the channel
 */
#define SPO_SET_CHANNEL_STATE_BLD(spop, state, chan_id, node_id) \
{                                                                \
    (spop)->spo_state = state;					 \
    bzero(&((spop)->spo_pad[0]), 3);                             \
    (spop)->spo_chan_id = chan_id;	                         \
    (spop)->spo_node_id = node_id; 				 \
    bzero(&((spop)->spo_pad2[0]), 2);                            \
}

/*
 * Macro Name: SPO_SET_DEV_STATE_BLD
 *      Initialize the fields in the set channel state structure
 * Arguments:
 *      spop:   A pointer to the CCB_SPVU_SET_ADAP structure.
 *      state:  Enable or diable.
 *      chan_id: The internal identifier of the channel id
 *      node_id: The SCSI bus node id for the channel
 */
#define SPO_SET_DEV_STATE_BLD(spop, state)         		\
{                                                               \
    (spop)->spo_state = state;                                  \
    bzero(&((spop)->spo_pad[0]), 7);                            \
}

/*
 * Macro Name: SPO_READ_COUNTER_BLD
 *      Initialize the fileds in the read counters structure 
 * Arguments:
 *      spop:   A pointer to the CCB_SPVU_READ_CNTR structure.
 *      flags:	ZERO_COUNTER, the counters are zeroed after being 
 *		read into the response message. 
 */
#define SPO_READ_COUNTER_BLD(spop, flags)                       \
{                                                               \
    bzero(&((spop)->spo_pad[0]), 2);                            \
    (spop)->spo_flags = flags;					\
    bzero(&((spop)->spo_pad2[0]), 5);                           \
}


/*
 * Macro Name: SPO_RESET_BUS_BLD
 *      Initialize the fileds in the reset bus ccb 
 * Arguments:
 *      ccb:   A pointer to the CCB_HEADER structure.
 *      flags:	ZERO_COUNTER, the counters are zeroed after being 
 *		read into the response message. 
 */
#define SPO_RESET_BUS_BLD(ccb, pathid)                	\
{                                                       \
    SPO_CCB_ALLOC((ccb));					\
    (ccb)->cam_ccb_len = sizeof (CCB_RESETBUS);		\
    (ccb)->cam_func_code = XPT_RESET_BUS;			\
    (ccb)->cam_path_id = pathid;				\
    (ccb)->cam_flags = CAM_DIR_NONE;			\
}

#endif /* _SIMPORT_INCL_ */
