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
 *	@(#)$RCSfile: if_emc.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:44:01 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *        Copyright 1986 Encore Computer Corporation
 *
 * ALL RIGHTS RESERVED. Licensed Material - Property of Encore Computer
 * Corporation. This software is made available solely pursuant to the
 * terms of a software license agreement which governs its use. 
 * Unauthorized duplication, distribution or sale are strictly prohibited.
 *
 * Include file description:
 *	EMC/Ethernet data structure definition.
 *
 * Original Author: F. Oliveira	    Created on: 06/21/86
 *
 */

/*
 * Structure of the returned statistics for a get_stats or get_clear_stats
 * primitive.
 */
struct eth_stats {
	short st_firm_ver;	/* Firmware version */
	short st_firm_rev;	/* Firmware revision */
	u_char st_ethr_addr[6];	/* Ethernet address */
	short st_dev_state;	/* Current device state */
	long st_xmit_frames;	/* Frames transmitted */
	long st_rcv_frames;	/* Frames received */
	long st_xmit_bytes;	/* Bytes transmitted */
	long st_rcv_bytes;	/* Bytes received */
	long st_xmit_nocoll;	/* Frames transmitted - no collisions */
	long st_xmit_onecoll;	/* Frames transmitted - one collision */
	long st_xmit_mulcoll;	/* Frames transmitted - multiple collisions */
	long st_xmit_exccoll;	/* Transmits failed - too many collisons */
	long st_xmit_undrnerr;	/* Transmits failed - DMA underrun */
	long st_rcv_crcerr;	/* Received frames with CRC errors */
	long st_rcv_alignerr;	/* Received frames with alignment errors */
	long st_rcv_rscerr;	/* Received frames lost - no ctlr resource */
	long st_rcv_ovrnerr;	/* Received frames lost - ctlr bus overrun */
	long st_rcv_hostbufs;	/* Received frames lost - no host buffers */
};

/* Interface state values */

#define EN_SINITING	  01	/* device is initializing */
#define EN_STERMING	  02	/* device is terminating */
#define EN_SINIT	  03	/* interface initialized */

#define EN_TIMEOUT	 (60)

/* Resource parameters held by driver */

#define ENM_BDLS	4	/* Max number of bdls to map a recv frame */
#define ENBUF_SIZE	1571	/* Size of a send buffer + 7 for alignment */

/* Specify the number of data structures */
#define	    NO_EN_LRCVS	     20
#define	    NO_EN_SRCVS	     50
#define	    NO_EN_LRATTN     2
#define	    NO_EN_SRATTN     2
#define	    NO_EN_SENDS	     20
#define	    NO_EN_SATTN	     1

/* Receive frame command (CRQOP_EN_RCV_FRAME).  In the standard EMC CRQ
 * message header, em_compltn_cnt will contain the receive byte count
 * in the command response.
 */

struct	crq_en_rcv_msg {
	emc_msg_t	en_rcv_hdr;	/* Standard message header */
	bd_t		*en_bdl;	/* Pointer to bdl. */

	/* Following is not interpreted by the controller code. */
	struct	mbuf   *en_mbuf;	/* Pointer to first mbuf */
	bd_t	bdl[ENM_BDLS+1];	/* bdls to map msg + 1 for alignment */
};

/*
 * Transmit frame command (CRQOP_EN_XMIT_FRAME). 
 *
 * Transmit frame with status command (CRQOP_EN_XMIT_FRAME_STS).  In the
 * standard EMC CRQ message header, em_compltn_cnt will contain the number
 * of collisions(if transmit succeeded) and em_xtnd_status will contain
 * the distance to a fault in the cable in meters (if transmit failed)
 * in the response command.
 */

struct	crq_en_xmit_msg {
	emc_msg_t	en_xmit_hdr;	/* Standard message header */
	bd_t		*en_bdl;	/* Pointer to bdl. */
	int		en_xmit_bytes;	/* Number of bytes to xmit */

	/* Following is not interpreted by the controller code. */
	struct	mbuf   *en_mbuf;	/* Pointer to first mbuf */
	bd_t	bdl[ENM_BDLS+1];	/* bdls to map msg + 1 for alignment */
};


/*
 * Read statistics command (CRQOP_EN_READ_STAT).
 *
 * Read and reset statistics command (CRQOP_EN_READ_RST_STAT).
 */

struct	crq_en_rdstats_msg {
	emc_msg_t	en_rdstats_hdr;		/* Standard message header */
	struct   	eth_stats *en_rdstats;	/* Ptr to stats buffer.*/
};

/*
 * Control commands (CRQOP_EN_CONTROL).
 */

struct	crq_en_cntl_msg {
	emc_msg_t	 en_cntl_hdr;		/* Standard message header */
	short		 en_cntl_subcode;	/* Command sub-code */
};

/*
 * Subcodes for the Ethernet control command (CRQOP_EN_CONTROL).
 */
#define ENSUB_SET_ONLINE	1
#define ENSUB_SET_OFFLINE	2
#define ENSUB_RESET		3
#define ENSUB_DSB_MC_FILTER	4
#define ENSUB_SET_INT_LPBACK	5
#define ENSUB_SET_EXT_LPBACK    6
#define ENSUB_ENB_RCV_ON_ERR	7
#define ENSUB_DSB_RCV_ON_ERR	8
#define ENSUB_ENB_MC_FILTER	9

/* Escape command (CRQOP_EN_ESCAPE_CMD).  In the standard EMC CRQ message
 * header in the response command, em_xtnd_status contains the 586 status
 * word.
 */

struct	crq_en_esc_msg {
	emc_msg_t	en_esc_hdr;	/* Standard message header */
	bd_t		*en_bdl;	/* Pointer to bdl. */
	short		en_esc_586cmd;	/* Intel 82586 command opcode */

	/* Following is not interpreted by the controller code. */
	struct	mbuf   *en_mbuf;	/* Pointer to first mbuf */
	bd_t	bdl[ENM_BDLS+1];	/* bdls to map msg + 1 for alignment */
};

/*
 * Definition of largest small receive CRQ message.  Used by EMC controller
 * code to decide when to put received frame on the small receive CRQ and
 * when to put it on the large receive CRQ.
 */
#define EN_SML_RCVSIZE	112

#define MAX_EN_ATTN_MSGSIZE	sizeof(CRQ_LOCKTMO_MSG)
/*
 * Ethernet device state definitions.
 */
#define EN_OFFLINE	0	/* Offline - receiving no packets */
#define EN_ONLINE	1	/* Online - receiving packets from network */
#define EN_INTLPBK	2	/* Internal loopback - all packets sent */
				/* looped back without going to cable. */
#define EN_EXTLPBK	3	/* External loopback - all packets sent */
				/* looped back after going to cable. */
