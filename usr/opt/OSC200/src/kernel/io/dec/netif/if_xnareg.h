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
 * derived from:
 *	@(#)if_xnareg.h	5.1	(ULTRIX)	3/30/91
 */
#ifndef _IF_XNAREG_H_
#define _IF_XNAREG_H_
/*
 * DEBNI/DEMNA device registers.
 */
#ifdef __mips
struct	xnareg {
	u_int _reg;
};

struct	xnadevice {
	struct	xnabase {
		union {
			u_int	_xdev;
			struct	xmi_reg	_xnaxmi;
			struct	bi_nodespace	_xnabi;
		} _xnabase_un;
	} *xnabase;
	struct	xnareg *_xpd1;
	struct	xnareg *_xpd2;
	struct	xnareg *_xpst;
	struct	xnareg *_xpud;
	struct	xnareg *_xpci;
	struct	xnareg *_xpcp;
	struct	xnareg *_xpcs;
};
#endif

#ifdef __alpha
struct	xnadevice {
	caddr_t	xnabase;	/* base address of xmi registers */
	/*  addresses of xmi registers */
	caddr_t dtype_xmi;
	caddr_t xber_xmi;
	caddr_t xfadr_xmi;
	caddr_t xcomm_xmi;
	caddr_t xfaer_xmi;
	/*  addresses of DEMNA specific registers */
	caddr_t	xnapd1;		/* port data register 1 */
	caddr_t	xnapd2;		/* port data register 2 */
	caddr_t	xnapst;		/* port status register */
	caddr_t	xnapud;		/* power-up diagnostic register */
	caddr_t	xnapci;		/* port control initialize register */
	caddr_t	xnapcp;		/* port control poll register */
	caddr_t	xnapcs;		/* port control shutdown register */
};
#endif

#define XMI_DTYPE	0x00
#define XMI_XBER	0x04
#define XMI_XFADR	0x08
#define XMI_XCOMM	0x10
#define XMI_XFAER	0x2c

#define	XNAPD1_XMI	0x100
#define	XNAPD2_XMI	0x104
#define	XNAPST_XMI	0x108
#define	XNAPUD_XMI	0x10c
#define	XNAPCI_XMI	0x110
#define	XNAPCP_XMI	0x114
#define	XNAPCS_XMI	0x118

/*
 * Values for device type register
 */
#define	XNADEMNA	XMI_XNA
#define	XNADEBNI	BI_XNA

#ifdef BI_SUPPORTED
#define	XNAPST_BI	0x0f0
#define	XNAPD1_BI	0x0f4
#define	XNAPD2_BI	0x0f8
#define	XNAPUD_BI	0x0fc
#define	XNAPCI_BI	0x208
#define	XNAPCP_BI	0x20c
#define	XNAPCS_BI	0x210
/*
 * Definitions for base register access (BI). biic_ctrl used for BI
 * error logging; xfadr and xfaer registers not supported for BI.
 */
#define	xctrl_bi	xnabase->_xnabase_un._xnabi.biic.biic_ctrl
#define xber_bi		xnabase->_xnabase_un._xnabi.biic.biic_err
/* binode_pad is an array of int's */
#define	xcomm_bi	xnabase->_xnabase_un._xnabi.binode_pad[0x040]
#endif

#ifdef __mips
/*
 * Definitions for base register access (XMI).
 */
#define	xna_dtype	xnabase->_xnabase_un._xdev
#define xber_xmi	xnabase->_xnabase_un._xnaxmi.xmi_xbe
#define xfadr_xmi	xnabase->_xnabase_un._xnaxmi.xmi_fadr
/* xmi_pad is an array of char's */
#define	xcomm_xmi	xnabase->_xnabase_un._xnaxmi.xmi_pad[0x000];
#define	xfaer_xmi	xnabase->_xnabase_un._xnaxmi.xmi_pad[0x020]

/*
 * DEBNI/DEMNA port data/control registers
 */
#define xnapst		_xpst->_reg
#define xnapd1		_xpd1->_reg
#define xnapd2		_xpd2->_reg
#define xnapud		_xpud->_reg
#define xnapci		_xpci->_reg
#define xnapcp		_xpcp->_reg
#define xnapcs		_xpcs->_reg
#endif

/*
 * Port Control register commands. Writing ANY value to a port control
 * reg. issues the corresponding command.
 */
#define	XPCP_POLL	0x01
#define	XPCS_SHUT	XPCP_POLL
#define	XPCI_INIT	XPCP_POLL

/*
 * Port status register defines.
 */
#define XPST_MASK	0xff
#define	XPST_RESET	0x00
#define XPST_UNDEF	0x01
#define XPST_INIT	0x02

/*
 * Format of a DEBNI/DEMNA buffer address.
 */
#ifdef __mips
struct xnaaddr	{
	u_int	xaddr_lo;
	unsigned xaddr_hi:16;
	unsigned xlen:16;
#define	xmbz	xlen
};
#endif

#ifdef __alpha
struct xnaaddr	{
	u_int	xaddr_lo;
	unsigned xaddr_hi:8;
	unsigned mbz1:8;
	unsigned mbz2:16;
#define	xlen	mbz2
#define	xmbz	mbz2
};
#endif

/*
 * Format of a DEBNI/DEMNA line counter entry. Each entry is a quad-word.
 */
struct xnactr_ent {
	u_short lo[2];
	u_short hi[2];
};

/*
 * Up to 64 multicast addresses per user definition
 */
#define	NMULTI	64

/*
 * DEBNI/DEMNA port data block. The port data block provides host/port
 * communications. Block is KM_ALLOC'd in driver and must be aligned to
 * 512-byte boundary.
 */
struct xnapdb {
	unsigned 	addr_mode:8;		/* Addressing Mode */
	unsigned 	resvd1:24;
	struct		xnaaddr	cmd;		/* Command ring address */
	struct		xnaaddr	recv;		/* Receive ring address */
	u_int		spt_len;		/* System page table len. */
	struct		xnaaddr	spt;		/* System page table addr. */
	u_int		gpt;			/* Global page table addr. */
	/*
	 * Host interrupt and error interrupt data
	 */
	struct		xnavec {
			unsigned level:2;
			unsigned vector:14;
			unsigned nid_mask:16;
	} ivec, evec;
	struct		xnaaddr	ubua;		/* UBUA counter addr. */
	struct		xnactr_ent p_sbua;	/* Potental sysbuf unavail */
	struct		xnactr_ent a_sbua;	/* Actual sysbuf unavail */
	struct		xnactr_ent a_dor;	/* Actual Data Overrun */
	u_char		driver_resvd[52];	/* reserved to driver	*/
	u_char		port_err[128];		/* error log area	*/
	u_char		port_resvd[256];	/* reserved to port	*/
};

/*
 * DEBNI/DEMNA addressing modes. (Only use AM_VIRT30 for VAXen).
 */
#define AM_VIRT30	30
#define AM_VIRT30_I	31
#define AM_VIRT34	34
#define AM_VIRT34_I	35
#define AM_PHYS40	40
#define AM_PHYS40_I	41

/*
 * DEBNI/DEMNA command and transmit ring entry. Command and transmit ring
 * entries are of fixed length determined by XNA_XMIT_NBUFS. Only a single
 * buffer segment is allowed for commands. The "_buf_un" address is used
 * to keep the head of the mbuf chain to be freed for transmits, and the
 * address of the KM_ALLOC'd buffer for commands. (For commands, this
 * address is the address used to wakeup the issuer of the command.)
 */
struct xnacmd_ring {
	unsigned	usr_index:8;
	unsigned	nbufs:8;
	unsigned	error:8;
	unsigned	status:8;
	int		mbufindx;
/*
 * MUST be: (2 <= XNA_XMIT_NBUFS <= 15)
 */
#define	XNA_XMIT_NBUFS	12
	struct		xnaaddr bseg[XNA_XMIT_NBUFS];
};

/*
 * DEBNI/DEMNA receive ring entry. Only a single buffer address allowed.
 */
struct xnarecv_ring {
	unsigned	len:16;
	unsigned	usr_index:8;
	unsigned	error:6;
	unsigned	status:2;
	struct		xnaaddr	bseg;
	int		mbufindx;
};

/*
 * Transmit, command status bits
 */
#define ST_MAP		0x10
#define ST_CMD		0x20
#define ST_TERR		0x40
#define ST_TOWN		0x80

/*
 * Receive status bits
 */
#define ST_RERR		0x01
#define ST_ROWN		0x02

/*
 * Transmit error codes
 */
#define XMITERR_FRAME		0x01
#define XMITERR_ADDR_INVAL	0x02
#define XMITERR_ADDR_TRANS	0x03
#define XMITERR_TRANSFER	0x04
#define XMITERR_LOSS		0x05
#define XMITERR_RETRY		0x06
#define XMITERR_LATE		0x07
#define XMITERR_TIMEOUT		0x08
#define XMITERR_OTHER		0x09

/*
 * Receive error codes
 */
#define	RECVERR_FRAME		0x01
#define RECVERR_ADDR_INVAL	0x02
#define RECVERR_ADDR_TRANS	0x03
#define	RECVERR_TRANSFER	0x04
#define	RECVERR_LONG		0x05
#define RECVERR_CRC		0x06

/*
 * Command error codes
 */
#define	CMDERR_FRAME		XMITERR_FRAME
#define	CMDERR_ADDR_INVAL	XMITERR_ADDR_INVAL
#define	CMDERR_ADDR_TRANS	XMITERR_ADDR_TRANS
#define	CMDERR_TRANSFER		XMITERR_TRANSFER
#define	CMDERR_CMDINVAL		XMITERR_LOSS
#define	CMDERR_OPINVAL		XMITERR_RETRY

/*
 * DEBNI/DEMNA command buffer formats. Command buffers are KM_ALLOC'd and cast
 * to the appropriate union member. Normal transmit buffers consist of the
 * data regions of mbufs passed in by xnaoutput(). The maintenence commands
 * are not implemented by the ULTRIX port driver.
 */
struct xnacmd_buf {
	u_int		opcode;		/* Command opcodes, defined above */
	union	{
		/*
		 * XNA parameter block
		 * (opcode = CMD_PARAM)
		 */
		struct	_xnaparam {
			u_int		sysdate_lo;
			u_int		sysdate_hi;
			struct		xnaaddr dpa;
			struct		xnaaddr apa;
			u_char		bvc[8];
			u_short		cur_src;
			u_short		cur_dst;
			u_short		cur_mca;
			u_short		cur_user;
			u_short		max_adr;
			u_short		max_user;
			unsigned	loop_mode:2;
			unsigned	flags:6;
			unsigned	resvd:24;
			u_char		serial_num[12];
		} _xnaparam;

		/*
		 * XNA counters block (quad-word values). These counters
		 * cannot be cast as a pair of longwords safely, since the
		 * DEBNI does not maintain VAX-style word ordering.
		 * (opcode = CMD_RDCNTR, CMD_RCCNTR)
		 */
		struct	_xnactrs {
			struct		xnactr_ent	seconds;
			struct		xnactr_ent	bytercvd;
			struct		xnactr_ent	bytesent;
			struct		xnactr_ent	blokrcvd;
			struct		xnactr_ent	bloksent;
			struct		xnactr_ent	mbytercvd;
			struct		xnactr_ent	mblokrcvd;
			struct		xnactr_ent	mbytesent;
			struct		xnactr_ent	mbloksent;
			struct		xnactr_ent	deferred;
			struct		xnactr_ent	single;
			struct		xnactr_ent	multiple;
			/*
			 * Bit map values (expanded into counters).
			 */
			struct		xnactr_ent	sendfail_retry;
			struct		xnactr_ent	sendfail_carrier;
			struct		xnactr_ent	sendfail_short;
			struct		xnactr_ent	sendfail_open;
			struct		xnactr_ent	sendfail_long;
			struct		xnactr_ent	sendfail_defer;
			struct		xnactr_ent	recvfail_crc;
			struct		xnactr_ent	recvfail_frame;
			struct		xnactr_ent	recvfail_long;
			struct		xnactr_ent	unrecog;
			struct		xnactr_ent	overrun;
			struct		xnactr_ent	sysbuf;
			struct		xnactr_ent	userbuf;
			struct		xnactr_ent	collis;
		} _xnactrs;

		/*
		 * XNA user counters block (quad-word values).
		 * (opcode = CMD_RCUCNTR, CMD_RDUCNTR)
		 */
		struct	_xnauctrs {
			struct		xnactr_ent	seconds;
			struct		xnactr_ent	bytercvd;
			struct		xnactr_ent	bytesent;
			struct		xnactr_ent	blokrcvd;
			struct		xnactr_ent	bloksent;
			struct		xnactr_ent	recvfail_error;
			struct		xnactr_ent	recvfail_mca;
			struct		xnactr_ent	recvfail_long;
			struct		xnactr_ent	sendfail;
		} _xnauctrs;

		/*
		 * XNA user start command. NOTE: if defining an 802
		 * user, the "xnasnap" struct will come after the N'th
		 * multi_addr, where: 0 <= N <= NMULTI. This means that
		 * the format of this structure may be of variable length.
		 * (opcode = CMD_USTART, CMD_UCHANGE, CMD_USTOP)
		 */
		struct	_xnaustart {
			unsigned	sap_ptt:16;
			unsigned	resvd:8;
			unsigned	mode:8;
			struct		xnaaddr	user_phys;
			u_short		addr_len;
			u_short		addr_alloc;
			struct		multi_addr {
					u_char	addr[6];
			} multi_addr[NMULTI];
			struct		xnasnap {
					u_char	snap_sap_id[5];
					u_char	ngsaps;
					u_char	gsap[NMULTI];
			} xnasap;
		} _xnaustart;

		/*
		 * SYSID data region
		 */
		u_char	_xnadata[464];	/* largest SYSID block == 464 bytes */
	} _cmd_un;
};

/*
 * defines for union members
 */
#define	xnaparam	_cmd_un._xnaparam
#define	xnactrs		_cmd_un._xnactrs
#define	xnauctrs	_cmd_un._xnauctrs
#define	xnaustart	_cmd_un._xnaustart
#define	xnasysid	_cmd_un._xnadata

/*
 * Command opcodes. Maintenence commands not supported.
 */
#define	CMD_NOP		0x00
#define CMD_SYSID	0x01
#define CMD_PARAM	0x02
#define CMD_RCCNTR	0x03
#define CMD_RDCNTR	0x04
#define CMD_RCUCNTR	0x05
#define CMD_RDUCNTR	0x06
#define CMD_USTART	0x07
#define CMD_UCHANGE	0x08
#define CMD_USTOP	0x09
#define CMD_MAINT	0x0a

#define	CMD_COMPLETE	0xfe		/* Command complete signal */
#define CMD_INVAL	0xff		/* Command failure signal */

/*
 * Bits for PARAM and USTART (UCHANGE) command flags
 */
#define	PARAM_NOLOOP	0x00
#define	PARAM_ELOOP	0x01
#define PARAM_ILOOP	0x02

#define	PARAM_DCRC	0x01
#define	PARAM_BOO	0x02
#define	PARAM_BTM	0x04
#define	PARAM_MPA	0x08
#define	PARAM_LEN	0x10

#define	USTART_BAD	0x01
#define	USTART_PAD	0x02
#define	USTART_CL1	0x04
#define	USTART_PROM	0x08
#define	USTART_802	0x10
#define	USTART_BDC	0x20
#define	USTART_UNK	0x40
#define	USTART_AMC	0x80

/*
 * The ethernet user
 */
#define XNA_ETHERU	0x00

#endif
