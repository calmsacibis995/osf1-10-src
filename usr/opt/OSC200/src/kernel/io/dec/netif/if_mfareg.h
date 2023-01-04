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
#ifndef _IF_MFAREG_H_
#define _IF_MFAREG_H_

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

#ifdef __vax__
#define	MFAFLUSH()	/* nothing */
#endif
#ifdef __alpha
#define	MFAFLUSH()	mb()
#endif
#ifdef __mips__
#define	MFAFLUSH()	wbflush()
#endif

struct  mfadevice {
        caddr_t mfabase;        /* base address of xmi registers */
        /*  addresses of xmi registers */
        caddr_t dtype_xmi;
        caddr_t xber_xmi;
        caddr_t xfadr_xmi;
        caddr_t xcomm_xmi;
        caddr_t xfaer_xmi;
	caddr_t	xpd1;		/* Start of mfa specific registers */
	caddr_t xpd2;
	caddr_t xpst;
	caddr_t xpud;
	caddr_t xpci;
	caddr_t xpcs;
	caddr_t xmit_fl;
	caddr_t rcv_fl;
	caddr_t cmd_fl;
	caddr_t unsol_fl;
	caddr_t xmit_hib_lo;
	caddr_t xmit_hib_hi;
	caddr_t rcv_hib_lo;
	caddr_t rcv_hib_hi;
	caddr_t eeprom_upd;
};

/*
 * Definition for device type register (wrong)  XXX ? What's wrong? 
 */
#define MFA_HWREV(x)	(((x) >> 24) + 'A')
#define MFA_FWREV(x)	((((x) >> 16) & 0xFF) + '0')
/*
 * Values for device type register
 */
#ifndef XMI_MFA				
#define	XMI_MFA		0x0823
#endif
#define	XMIDEMFA	XMI_MFA

#define XMI_DTYPE       0x00
#define XMI_XBER        0x04
#define XMI_XFADR       0x08
#define XMI_XCOMM       0x10
#define XMI_XFAER       0x2c

#define MFA_XPD1	0x100
#define MFA_XPD2	0x104
#define MFA_XPST	0x108
#define MFA_XPUD	0x10c
#define MFA_XPCI	0x110
#define MFA_XPCS	0x114
#define MFA_XMIT_FL	0x118
#define MFA_RCV_FL	0x11c
#define MFA_CMD_FL	0x120
#define MFA_UNSOL_FL	0x124
#define MFA_XMIT_HIB_LO	0x128
#define MFA_XMIT_HIB_HI	0x12c
#define MFA_RCV_HIB_LO	0x130
#define MFA_RCV_HIB_HI	0x134
#define MFA_EEPROM_UPD	0x138

/*
 * Port Control register commands. Writing a 1 to a port control
 * reg. issues the corresponding command.
 */
#define	XPCP_POLL	1
#define	XPCS_SHUT	1
#define	XPCI_INIT	1

/*
 * Port status register defines.
 */
#define XPST_MASK		0x07
#define	XPST_RESET		0x00
#define XPST_UNINIT		0x01
#define XPST_INIT		0x02
#define XPST_RUNNING		0x03
#define XPST_MAINTENANCE	0x04
#define XPST_HALT		0x05

/*
 * Format of a DEMFA buffer address.
 */
typedef struct {
    u_int xaddr_lo;		/* bits 31:00			*/
    u_int xaddr_hi:8,		/* bits 39:32			*/
          xaddr_entry_sz:8,	/* entry size (may be 0)	*/
          xlen:16;		/* ring size			*/
} mfaaddr_t;

#define MFASETPHYADDR(addrptr, svaddr, entry_sz, length) { \
        if(svatophys(svaddr,&phy) == KERN_INVALID_ADDRESS) \
                panic("mfasetphyaddr: Invalid physical address! \n"); \
	bzero(addrptr, sizeof(mfaaddr_t)); \
	(addrptr)->xaddr_lo = phy & 0xffffffff; \
	(addrptr)->xaddr_hi = (phy >> 32) & 0xffffffff; \
	(addrptr)->xaddr_entry_sz = (entry_sz); \
	(addrptr)->xlen = (length); }
	    
/*
 * Format of a DEMFA line counter entry. Each entry is a quad-word.
 */
typedef struct {
    u_int lo;
    u_int hi;
} mfactr_t;

/*
 * Format of the DEMFA interrupt longword
 */
typedef struct {
    u_int level:4,		/* MBZ */
          vector:12,
          nid_mask:16;
} mfavec_t;

/*
 * Format of a 802.1 Address for the DEMFA
 */
typedef struct {
    u_char addr[6];
    u_short rsvd;
} mfahwaddr_t;

/*
 * Up to 16 multicast addresses per user definition
 */
#define	NMULTI	16

/*
 * DEMFA port data block. The port data block provides host/port
 * communications. Block is kmem_alloc'ed in driver and must be aligned to
 * 512-byte boundary.
 */
typedef struct {
    mfaaddr_t	cmd;			/* Command ring address		*/
    mfaaddr_t	xmt;			/* Transmit ring address	*/
    mfaaddr_t	rcv;			/* Receive ring address		*/
    u_int	max_buf_size:16,	/* Maximum size of supplied buffer */
    		max_rcv_frame:16;	/* Maximum size of rcvd frame	*/
    mfaaddr_t	unsol;			/* Unsoliticited ring address	*/
    mfavec_t	intvec;			/* Interrupt vector		*/
    u_int	p_sbua:1,		/* potenial system buffer unavail */
    		rsvd:31;
    mfaaddr_t	ubua;			/* UBUA counters		*/
/*  mfactr_t	ctrs[15];		   saved counters		*/
    mfactr_t	pdb_sbua;		/* SBUA counter			*/
    mfactr_t	pdb_octets_rcvd;	/* Octets received counter	*/
    mfactr_t	pdb_octets_sent;	/* Octets sent counter		*/
    mfactr_t	pdb_frames_rcvd;	/* Frames received counter	*/
    mfactr_t	pdb_frames_sent;	/* Frames sent counter          */
    mfactr_t	pdb_mc_octets_rcvd;	/* Multicast octets rcv counter */
    mfactr_t	pdb_mc_octets_sent;	/* Multicast octets sent counter*/
    mfactr_t	pdb_mc_frames_rcvd;	/* Multicast frames received	*/
    mfactr_t	pdb_mc_frames_sent;	/* Multicast frames sent 	*/
    mfactr_t	pdb_block_check_errors;	/* Block check errors 		*/
    mfactr_t	pdb_frame_status_errors;/* Frame status errors		*/
    mfactr_t	pdb_frame_alignment_errors; /*Frame alignment error	*/
    mfactr_t	pdb_frame_toolong_errors;/* Frame too long errors	*/
    mfactr_t	pdb_unrecog_indiv_frame_dst; /* Unrecognized frame dest */
    mfactr_t	pdb_unrecog_mc_frame_dst;/* Unrecognized Multi frame dst*/
    u_char	port_err[128];		/* error log area		*/
    u_char	port_resvd[212];	/* reserved to port		*/
} mfapdb_t;

typedef struct {
    caddr_t ring;		/* pointer to ring */
    caddr_t pa_ring;		/* phys addr of ring */
    struct mbuf **mbufs;	/* array of ring mbufs */
    int goal;			/* number of entries to preallocate */
    int max;			/* number of entries in ring */
    int size;			/* size of ring in octets */
    int active;			/* rings owned by adapter */
    caddr_t limit;		/* ptr to last ring entry + 1 */
    caddr_t nextin;		/*  first entry port "owns" */
    caddr_t nextout;		/* first entry host "owns" */
    caddr_t lastin;		/* last entry ack'ed by port */
} mfaring_t;

#define RING_NEXTOUT(ringptr, type_t) 		((type_t *) (ringptr)->nextout)
#define RING_NEXTIN(ringptr, type_t)		((type_t *) (ringptr)->nextin)
#define	RING_SETNEXTIN(ringptr, rp, type_t)	( *(type_t **) (&(ringptr)->nextin) = (rp))
#define RING_LASTIN(ringptr, type_t)		((type_t *) (ringptr)->lastin)
#define	RING_SETLASTIN(ringptr, rp, type_t)	( *(type_t **) (&(ringptr)->lastin) = (rp))
#define RING_ADV_NEXTOUT(ringptr, type_t) \
	((ringptr)->active++, \
	((ringptr)->nextout + sizeof(type_t) >= ((ringptr)->limit) \
	  ?  ((ringptr)->nextout = (ringptr)->ring) \
	  :  ((ringptr)->nextout += sizeof(type_t))))

#define RING_ADV_NEXTIN(ringptr, type_t) \
	((ringptr)->active--, \
	((ringptr)->nextin + sizeof(type_t) >= ((ringptr)->limit) \
	  ?  ((ringptr)->nextin = (ringptr)->ring) \
	  :  ((ringptr)->nextin += sizeof(type_t))))

#define RING_MBUF(ringptr, rp, type_t) \
	((ringptr)->mbufs[(rp) - ((type_t *)((ringptr)->ring))])
#define RING_SETMBUF(ringptr, rp, type_t, m) \
	((ringptr)->mbufs[(rp) - ((type_t *)((ringptr)->ring))] = (m))

#define RING_EMPTY(ringptr)			((ringptr)->nextin == (ringptr)->nextout)
#define RING_ACTIVE(ringptr) 			((ringptr)->active)
#define RING_FREESPACE(ringptr)			((ringptr)->max - 1 - RING_ACTIVE(ringptr))

#define RING_MOVEENTRY(ringptr, from, to, type_t) do { \
	    if ((ringptr)->mbufs) { \
		RING_SETMBUF(ringptr, to, type_t, RING_MBUF(ringptr, from, type_t)); \
		RING_SETMBUF(ringptr, from, type_t, NULL); \
	    } \
	    *(to) = *(from); \
	} while (0)

#define RING_RESET(ringptr, type_t, expr, mexpr) do { \
	    register int ringidx; \
	    struct mbuf *ringm; \
	    register type_t *next = (type_t *)((ringptr)->ring); \
	    for (ringidx = 0; ringidx < (ringptr)->max; next++, ringidx++) { \
		bzero(next, sizeof(type_t)); \
		(expr); \
		if ((ringptr)->mbufs) { \
		    if ((ringm = (ringptr)->mbufs[ringidx]) && (mexpr)) \
			(ringptr)->mbufs[ringidx] = NULL; \
		} \
	    } \
	    (ringptr)->nextout = (ringptr)->ring; \
	    (ringptr)->nextin = (ringptr)->ring; \
	    (ringptr)->lastin = (ringptr)->limit - sizeof(type_t); \
	    (ringptr)->active = 0; \
	} while(0)
#define RING_LASTIN_PA(ringptr, type_t) \
	((ringptr)->pa_ring + ((ringptr)->lastin - (ringptr)->ring))

/*
 * DEMFA command ring entry. Command ring
 * entries are of fixed length determined by MFA_CMD_NBUFS. Only a single
 * buffer segment is allowed for commands. The "_buf_un" address is used
 * to keep the head of the mbuf chain to be freed for transmits, and the
 * address of the KM_ALLOC'd buffer for commands. (For commands, this
 * address is the address used to wakeup the issuer of the command.)
 */
typedef struct {
    u_int pa_lo:9,		/* address (08:00)	*/
          length:13,		/* length of entry	*/
          uindex:5,		/* user id		*/
          erc:3,		/* error code		*/
          ers:1,		/* error summary	*/
          owner:1;		/* owner bit		*/
    u_int pa_hi:32;		/* address (39:09)	*/
} mfacmd_ent_t;

typedef struct {
    u_int pa_lo:9,
          rsvd:6,
          ers:1,
          length:13,
          eop:1,
          sop:1,
          owner:1;
    u_int pa_hi:31,
          rsvd2:1;
} mfaxmt_ent_t;

/*
 * DEMFA receive ring entry.
 *
 * Each receive ring entry will store one cluster mbuf.  This assumes that
 * a receive ring entry will be able to completely reference a mbuf (currently
 * true for both mips and vax).
 */

typedef struct {
    u_int pa_lo:9,
          uindex:5,
          unknown:1,
          musers:1,
          length1:13,
          eop:1,
          sop:1,
          owner:1;
    u_int pa_hi1:31,
          lst1:1;
    u_int rmc_rcc:9,
          rmc_fsb:5,
          fsc:2,
          length2:13,
          fsc2:1,
          rsvd:1,
          ers:1;
    u_int pa_hi2:31,
          lst2:1;
} mfarcv_ent_t;

/*
 * DEMNA unsolicited ring entry
 */
typedef struct {
    u_short opcode;
    u_short rsvd:15,
            owner:1;
    u_int data[7];
} mfaunsol_ent_t;

/*
 * Command error codes
 */
#define MFACMD_NO_ERROR			0x00
#define MFACMD_BUFLEN_ERROR		0x01
#define MFACMD_WRONG_STATE		0x02
#define MFACMD_BUFXFER_FAILED		0x03
#define MFACMD_INVALID_COMMAND		0x04
#define MFACMD_INVALID_OPCODE		0x05
#define MFACMD_INVALID_LOOPBACK_CMD	0x06

/*
 * Transmit error codes
 */
#define MFAXMT_NO_ERROR			0x00
#define MFAXMT_FORMAT_ERROR		0x20

/*
 * RMC Receive Completion Codes
 */
#define MFARCV_RCC			0x1ff		/* rmc rcc bit 21-13 */
#define MFARCV_ERROR			0x100		/* bit 21 set */
#define MFARCV_OVERRUN			0x1a9		/* frame too long */
#define MFARCV_INTERFACE_ERR		0x1aa		/* rmc/mac interface error */
#define MFARCV_RCC_C			0x80		/* block check error */
#define MFARCV_FSB_E			0x10		/* frame status error */
#define MFARCV_FSC_29			0x1		/* frame status count bit 29 */
#define MFARCV_FSC_28_27		0x3		/* frame status count bit 28 & 29 */
#define MFARCV_RCC_rrr			0x70		/* rmc rrr field bits 19 - 17 */
#define MFARCV_RCC_NORMAL		0x00		/* normal rrr */
#define MFARCV_RCC_SA_MATCHED		0x10		/* SA match */
#define MFARCV_RCC_DA_NOMATCH		0x20		/* no DA match */
#define MFARCV_RCC_RMC_ABORT		0x30		/* rmc abort */
#define MFARCV_RCC_RMC_INVALID_LEN	0x40		/* invalid number of symbols */
#define MFARCV_RCC_FRAGMENT		0x50		/* fragments */
#define MFARCV_RCC_FORMAT_ERR		0x60		/* format error */
#define MFARCV_MAC_RESET		0x70		/* MAC reset */

/*
 * Receive error codes
 */
#define MFARCV_NO_ERROR			0x00
#define MFARCV_FMT_ERR_NO_SOP		0x01
#define MFARCV_ONE_PAGE_NO_EOP		0x04
#define MFARCV_NO_END			0x08

/*
 * Unsolicited Opcodes
 */
#define MFAUNSOL_BASE			0xF005
#define MFAUNSOL_RING_INIT_INITIATED	0xF005
#define MFAUNSOL_RING_INIT_RCVD		0xF006
#define MFAUNSOL_RING_BEACON_INITIATED	0xF007
#define MFAUNSOL_DUPL_ADDR_TEST_FAILURE	0xF008
#define MFAUNSOL_DUPL_TOKEN_RCVD	0xF009
#define MFAUNSOL_RING_PURGER_ERROR	0xF00A
#define MFAUNSOL_BRIDGE_STOP_ERROR	0xF00B
#define MFAUNSOL_RING_OP_OSCILLATION	0xF00C
#define MFAUNSOL_DIRECTED_BEACON_RCVD	0xF00D
#define MFAUNSOL_TRACE_INITIATED	0xF00E
#define MFAUNSOL_TRACE_RCVD		0xF00F
#define MFAUNSOL_BASE2			0xF019
#define MFAUNSOL_XMIT_UNDERRUN		0xF019
#define MFAUNSOL_XMIT_FAILURE		0xF01A
#define MFAUNSOL_RECEIVE_OVERRUN	0xF01B
#define MFAUNSOL_BASE3			0xF022
#define MFAUNSOL_LEM_REJECT		0xF022
#define MFAUNSOL_EBUFF_ERROR		0xF023
#define MFAUNSOL_LCT_REJECT		0xF024
/*
 * DEMFA counters block (quad-word values). CMD_RDCNTR
 */
typedef struct {
    u_int	reltime[4];
    mfactr_t	octets_rcvd;
    mfactr_t	octets_sent;
    mfactr_t	frames_rcvd;
    mfactr_t	frames_sent;
    mfactr_t	mc_octets_rcvd;
    mfactr_t	mc_octets_sent;
    mfactr_t	mc_frames_rcvd;
    mfactr_t	mc_frames_sent;
    mfactr_t	xmt_underruns;
    mfactr_t	xmt_failures;
    mfactr_t	block_check_errors;
    mfactr_t	frame_status_errors;
    mfactr_t	frame_alignment_errors;
    mfactr_t	frame_toolong_errors;
    mfactr_t	unrecog_indiv_frame_dst;
    mfactr_t	unrecog_mc_frame_dest;
    mfactr_t	data_overruns;
    mfactr_t	link_buffer_unavailable;
    mfactr_t	user_buffer_unavailable;
    mfactr_t	frame_count;
    mfactr_t	error_count;
    mfactr_t	lost_count;
    mfactr_t	ring_init_initiated;
    mfactr_t	ring_init_other_station;
    mfactr_t	dup_addr_test_failed;
    mfactr_t	duplicate_token_detected;
    mfactr_t	ring_purge_errors;
    mfactr_t	bridge_strip_errors;
    mfactr_t	pc_traces_initiated;
    mfactr_t	link_self_test_errors;
    mfactr_t	lem_rejects;
    mfactr_t	lem_link_errors;
    mfactr_t	lct_rejects;
    mfactr_t	tne_exp_rejects;
    mfactr_t	elm_parity_errors;
    mfactr_t	connections_completed;
    mfactr_t	pc_traces_received;
    mfactr_t	reserved;
} mfactrblk_t;

/*
 * MFA parameter block (opcode = CMD_PARAM)
 */
typedef struct {
    u_int	version;
    mfahwaddr_t	mla;
    u_int	t_max;
    u_int	t_req;
    u_int	tvx;
    u_int	lem_threshold;
    u_char	bvc[8];
    u_int	max_user;
    u_int	max_adr;
    u_int	flags;
#define PARAM_LM		(0x03 << 0)	/* Loopback Mode */
#define PARAM_LM_NoLoopback	(0x00 << 0)	/*   No (Normal) Loopback */
#define PARAM_LM_ExtLoopback	(0x01 << 0)	/*   External Loopback */
#define PARAM_LM_IntLoopback	(0x02 << 0)	/*   Internal Loopback */
#define PARAM_BOO		(0x01 << 2)	/* Boot Enable */
#define PARAM_Ring_Purger	(0x01 << 3)	/* Disable Ring Purger */
    u_short	update_interval;
    u_short	rtoken_timeout;
    mfactrblk_t	cntrs;
} mfaparam_t;

/*
 * MFA SET SMT block (opcode = CMD_SETSMT)
 */
typedef struct {
    u_int	valid;
    u_int	dis_ring_purger;
    u_int	t_req;
    u_int	tvx;
    u_int	lem_threshold;
    u_int	rtoken_timeout;
} mfasetsmt_t;
 
/* 
 * SET SMT valid mask flags
 */
#define	SMT_RING_PURGE	0x1
#define SMT_TREQ	0x2
#define SMT_TVX		0x4
#define SMT_LEM		0x8
#define	SMT_RTOKEN	0x10

/*
 * DEMFA status block. CMD_STATUS
 */
typedef struct {
    u_int	reltime[4];	/* Binary Relative Time since reset */
    u_int	version;
    mfahwaddr_t	mla;			/* My Long Address */
    u_int	t_max;
    u_int	t_neg;
    u_int	tvx;
    u_int	lem_threshold;
    mfahwaddr_t	upstream_neighbor;	
    u_char	bvc[8];			/* Boot Verification Code */
    u_int	current_dests;
    u_int	current_users;
    u_int	current_mcas;
    u_int	max_users;
    u_int	max_addrs;
    u_int	phy_state;
    u_int	port_type;
    u_char	serial_number[12];
    u_int	flags;
    u_int	rtoken_timeout;		/* restricted token timeout value */
    u_int	smt_version_id:16,	/* SMT version ID          */
    		min_smt_version_id:16;  /* Minimum SMT version ID  */
    u_int	rsvd_4:16,		/* Reserved                */
    		max_smt_version_id:16;  /* Maximum SMT version ID  */
    mfahwaddr_t	old_upstream_addr;	/* old upstream neighbor address  */
    mfahwaddr_t	downstream_neighbor;	/* downstream neighbor address	  */
    mfahwaddr_t	old_downstream_addr;	/* old downstream neighbor address */
    u_int	rsvd_5:24,		/* Reserved        */
    		una_dupladdr:8;		/* Upstream neighbor DAT flag */
    u_int	ring_fault_reason;	/* Ring Error Reason */
    u_int	ring_latency;		/* Ring latency */
    u_int	phy_type;		/* phy type          */
    u_int	pmd_type;		/* pmd type          */
    u_int	neighbor_phy_type;	/* neighbor phy type */
    u_int	link_error_estimate;	/* link error estimate */
    u_int	rsvd_6:24,		/* Reserved        */
    		reject_reason:8;	/* reject reason   */
} mfastatus_t;
#define	STS_BOO				0x01		      /* Boot Enable */
							       /* FDDI State */
#define STS_ST				(0x07 << 1)
#define STS_ST_Off_Maint		(0x00 << 1)
#define STS_ST_Off_Ready		(0x01 << 1)
#define STS_ST_Off_Fault_Recovery	(0x02 << 1)
#define STS_ST_On_Ring_Init		(0x03 << 1)
#define STS_ST_On_Ring_Run		(0x04 << 1)
#define STS_ST_Broken			(0x05 << 1)
#define STS_DAT				(0x03 << 4)   /* Dual Addr Tst Rslts */
#define STS_DAT_Unknown			(0x00 << 4)
#define STS_DAT_Passed			(0x01 << 4)
#define STS_DAT_Failure			(0x02 << 4)
#define STS_DAT_Reserved		(0x03 << 4)
#define STS_RP				(0x03 << 6)	/* Ring Purger State */
#define	STS_RP_Purger_Off		(0x00 << 6)
#define	STS_RP_CandidatePurger		(0x01 << 6)
#define	STS_RP_NonPurger		(0x02 << 6)
#define	STS_RP_Purger			(0x03 << 6)
#define	STS_CTY				(0x01 << 8)   /* Claim Token Yield */
#define STS_UNTO			(0x01 << 9)  /* Upstrm Neighbor Tmo */
#define STS_RP_ENABLED			(0x01 << 10)  /* Ring Purger Enabled */

/*
 * DEMFA user start command.  (opcode = CMD_USTART, CMD_UCHANGE, CMD_USTOP)
 */
typedef struct {
    u_int	mode;
#define USTART_MODE		(0x03 << 0)	/* Filtering mode */
#define USTART_MODE_Normal	(0x00 << 0)	/* Normal mode */
#define USTART_MODE_Promisc	(0x01 << 0)	/* FC Filtering (Promisc) */
#define USTART_P_S		(0x01 << 2)	/* SNAP or SAP */
#define USTART_UNK		(0x01 << 3)	/* Unknown user */
#define USTART_AMC		(0x01 << 4)	/* All Multicast */
#define USTART_CL1		(0x01 << 5)	/* Class 1 service */
#define USTART_IFCS		(0x01 << 6)	/* Ignore FCS */
#define USTART_DE		(0x01 << 7)	/* Deliver Errors */
    u_int	fc;			/* frame control */
#define USTART_FC_SMT		0x08		/* SMT Frames */
#define USTART_FC_LLC		0x20		/* LLC Frames */
    mfahwaddr_t	user_phys_addr;
    u_short	addr_len;
    u_short	rsvd1;
    mfahwaddr_t mcalist[NMULTI];
    u_char	sap;
    u_char	gsap[7];
    u_char	protoid[5];
    u_char	rsvd[3];
} mfaustart_t;

/*
 * DEMFA command buffer formats. Command buffers are KM_ALLOC'd and cast
 * to the appropriate union member. Normal transmit buffers consist of the
 * data regions of mbufs passed in by mfaoutput(). The maintenence commands
 * are not implemented by the ULTRIX port driver.
 */
typedef struct {
    u_int opcode;		/* Command opcodes, defined above */
    union {
	mfaparam_t	_mfaparam;
	mfactrblk_t	_mfactrs;
	mfastatus_t	_mfastatus;
	mfaustart_t	_mfaustart;
	mfasetsmt_t	_mfasetsmt;
	/*
	 * SYSID data region
	 */
	u_char	_mfadata[483];	/* largest SYSID block == 464 bytes */
    } _cmd_un;
} mfacmd_buf_t;

/*
 * defines for union members
 */
#define	mfaparam	_cmd_un._mfaparam
#define	mfastatus	_cmd_un._mfastatus
#define	mfactrs		_cmd_un._mfactrs
#define	mfaustart	_cmd_un._mfaustart
#define	mfasysid	_cmd_un._mfadata
#define mfasetsmt	_cmd_un._mfasetsmt

/*
 * Command opcodes. Maintenence commands not supported.
 */
#define	CMD_NOP		0x00
#define CMD_SYSID	0x01
#define CMD_PARAM	0x02
#define CMD_STATUS	0x03
#define CMD_RDCNTR	0x04
#define CMD_SETSMT	0x05
#define CMD_USTART	0x07
#define CMD_UCHANGE	0x08
#define CMD_USTOP	0x09
#define CMD_MAINT	0x0a

#define	CMD_COMPLETE	0xfe		/* Command complete signal */
#define CMD_INVAL	0xff		/* Command failure signal */

#define MFA_UNKNOWNU	29		/* The Unknown user */

#endif
