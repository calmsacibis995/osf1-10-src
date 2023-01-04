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
 *	@(#)$RCSfile: ntp_control.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:41:08 $
 */
/*
 */

/*
 * ntp_control.h - definitions related to NTP mode 6 control messages
 */
struct ntp_control {
	u_char li_vn_mode;		/* leap, version, mode */
	u_char r_m_e_op;		/* response, more, error, opcode */
	u_short sequence;		/* sequence number of request */
	u_short status;			/* status word for association */
	u_short associd;		/* association ID */
	u_short offset;			/* offset of this batch of data */
	u_short count;			/* count of data in this packet */
	u_char data[(480 + 12)];	/* data + auth */
};

/*
 * Length of the control header, in octets
 */
#define	CTL_HEADER_LEN		12
#define	CTL_MAX_DATA_LEN	468


/*
 * Limits and things
 */
#define	CTL_MAXTRAPS	3		/* maximum number of traps we allow */
#define	CTL_TRAPTIME	(60*60)		/* time out traps in 1 hour */
#define	CTL_MAXAUTHSIZE	64		/* maximum size of an authen'ed req */

/*
 * Decoding for the r_m_e_op field
 */
#define	CTL_RESPONSE	0x80
#define	CTL_ERROR	0x40
#define	CTL_MORE	0x20
#define	CTL_OP_MASK	0x1f

#define	CTL_ISRESPONSE(r_m_e_op)	(((r_m_e_op) & 0x80) != 0)
#define	CTL_ISMORE(r_m_e_op)	(((r_m_e_op) & 0x20) != 0)
#define	CTL_ISERROR(r_m_e_op)	(((r_m_e_op) & 0x40) != 0)
#define	CTL_OP(r_m_e_op)	((r_m_e_op) & CTL_OP_MASK)

/*
 * Opcodes
 */
#define	CTL_OP_UNSPEC		0
#define	CTL_OP_READSTAT		1
#define	CTL_OP_READVAR		2
#define	CTL_OP_WRITEVAR		3
#define	CTL_OP_READCLOCK	4
#define	CTL_OP_WRITECLOCK	5
#define	CTL_OP_SETTRAP		6
#define	CTL_OP_ASYNCMSG		7
#define	CTL_OP_UNSETTRAP	31

/*
 * {En,De}coding of the system status word
 */
#define	CTL_SST_TS_UNSPEC	0	/* time source unspecified */
#define	CTL_SST_TS_LF		1	/* time source LF radio */
#define	CTL_SST_TS_UHF		2	/* time source UHF radio */
#define	CTL_SST_TS_HF		3	/* time source HF radio */
#define	CTL_SST_TS_LOCAL	4	/* time source LOCAL */
#define	CTL_SST_TS_NTP		5	/* time source NTP */
#define	CTL_SST_TS_UDPTIME	6	/* time source UDP/TIME */
#define	CTL_SST_TS_WRSTWTCH	7	/* time source is wristwatch*/

#define	CTL_SYS_MAXEVENTS	15

#define	CTL_SYS_STATUS(li, source, nevnt, evnt) \
		((((li)<< 14)&0xc000) | \
		(((source)<<8)&0x3f00) | \
		(((nevnt)<<4)&0x00f0) | \
		((evnt)&0x000f))

#define	CTL_SYS_LI(status)	(((status)>>14) & 0x3)
#define	CTL_SYS_SOURCE(status)	(((status)>>8) & 0x3f)
#define	CTL_SYS_NEVNT(status)	(((status)>>4) & 0xf)
#define	CTL_SYS_EVENT(status)	((status) & 0xf)

/*
 * {En,De}coding of the peer status word
 */
#define	CTL_PST_CONFIG		0x80
#define	CTL_PST_AUTHENABLE	0x40
#define	CTL_PST_AUTHENTIC	0x20
#define	CTL_PST_REACH		0x10
#define	CTL_PST_SANE		0x08
#define	CTL_PST_DISP		0x04

#define	CTL_PST_SEL_REJECT	0
#define	CTL_PST_SEL_SELCAND	1
#define	CTL_PST_SEL_SYNCCAND	2
#define	CTL_PST_SEL_SYSPEER	3

#define	CTL_PEER_MAXEVENTS	15

#define	CTL_PEER_STATUS(status, nevnt, evnt) \
		((((status)<<8) & 0xff00) | \
		(((nevnt)<<4) & 0x00f0) | \
		((evnt) & 0x000f))

#define	CTL_PEER_STATVAL(status)(((status)>>8) & 0xff)
#define	CTL_PEER_NEVNT(status)	(((status)>>4) & 0xf)
#define	CTL_PEER_EVENT(status)	((status) & 0xf)

/*
 * {En,De}coding of the clock status word
 */
#define	CTL_CLK_OKAY		0
#define	CTL_CLK_NOREPLY		1
#define	CTL_CLK_BADFORMAT	2
#define	CTL_CLK_FAULT		3
#define	CTL_CLK_PROPAGATION	4
#define	CTL_CLK_BADDATE		5
#define	CTL_CLK_BADTIME		6

#define	CTL_CLK_STATUS(status, event) \
		((((status)<<8) & 0xff00) | \
		((event) & 0x00ff))

/*
 * Error code responses returned when the E bit is set.
 */
#define	CERR_UNSPEC	0
#define	CERR_PERMISSION	1
#define	CERR_BADFMT	2
#define	CERR_BADOP	3
#define	CERR_BADASSOC	4
#define	CERR_UNKNOWNVAR	5
#define	CERR_BADVALUE	6

#define	CERR_NORESOURCE	CERR_PERMISSION	/* wish there was a different code */


/*
 * System variables we understand
 */
#define	CS_LEAP		1
#define	CS_STRATUM	2
#define	CS_PRECISION	3
#define	CS_DISTANCE	4
#define	CS_DISPERSION	5
#define	CS_REFID	6
#define	CS_REFTIME	7
#define	CS_HOLD		8
#define	CS_PEERID	9
#define	CS_OFFSET	10
#define	CS_DRIFT	11
#define	CS_COMPLIANCE	12
#define	CS_CLOCK	13
#define	CS_LEAPIND	14
#define	CS_LEAPWARNING	15
#define	CS_PROCESSOR	16
#define	CS_SYSTEM	17
#define	CS_KEYID	18
#define	CS_MAXSKEW	19

#define	CS_MAXCODE	CS_MAXSKEW

/*
 * Peer variables we understand
 */
#define	CP_CONFIG	1
#define	CP_AUTHENABLE	2
#define	CP_AUTHENTIC	3
#define	CP_SRCADR	4
#define	CP_SRCPORT	5
#define	CP_DSTADR	6
#define	CP_DSTPORT	7
#define	CP_LEAP		8
#define	CP_HMODE	9
#define	CP_STRATUM	10
#define	CP_PPOLL	11
#define	CP_HPOLL	12
#define	CP_PRECISION	13
#define	CP_DISTANCE	14
#define	CP_DISPERSION	15
#define	CP_REFID	16
#define	CP_REFTIME	17
#define	CP_ORG		18
#define	CP_REC		19
#define	CP_XMT		20
#define	CP_REACH	21
#define	CP_VALID	22
#define	CP_TIMER	23
#define	CP_ESTDELAY	24
#define	CP_ESTOFFSET	25
#define	CP_ESTDISP	26
#define	CP_KEYID	27
#define	CP_DELAY	28
#define	CP_OFFSET	29
#define	CP_PMODE	30
#define	CP_RECEIVED	31
#define	CP_SENT		32

#define	CP_MAXCODE	CP_SENT

/*
 * Clock variables we understand
 */
#define	CC_TYPE		1
#define	CC_TIMECODE	2
#define	CC_POLL		3
#define	CC_NOREPLY	4
#define	CC_BADFORMAT	5
#define	CC_BADDATA	6
#define	CC_FUDGETIME1	7
#define	CC_FUDGETIME2	8
#define	CC_FUDGEVAL1	9
#define	CC_FUDGEVAL2	10
#define	CC_FLAGS	11
#define	CC_DEVICE	12

#define	CC_MAXCODE	CC_DEVICE

/*
 * Definition of the structure used internally to hold trap information.
 * ntp_request.c wants to see this.
 */
struct ctl_trap {
	struct sockaddr_in tr_addr;	/* address of trap recipient */
	struct interface *tr_localaddr;	/* interface to send this through */
	u_int tr_settime;		/* time trap was set */
	u_int tr_count;		/* async messages sent to this guy */
	u_int tr_origtime;		/* time trap was originally set */
	u_int tr_resets;		/* count of resets for this trap */
	u_short tr_sequence;		/* trap sequence id */
	u_char tr_flags;		/* trap flags */
	u_char tr_version;		/* version number of trapper */
};

/*
 * Flag bits
 */
#define	TRAP_INUSE	0x1		/* this trap is active */
#define	TRAP_NONPRIO	0x2		/* this trap is non-priority */
#define	TRAP_CONFIGURED	0x4		/* this trap was configured */
