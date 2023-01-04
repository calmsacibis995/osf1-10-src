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
 * derived from bvpscs.h	4.3	(ULTRIX)	10/11/90	
 */
#ifndef _BVPSCS_H_
#define _BVPSCS_H_

typedef struct _bvp_ssppccb {		/* BVP SSP specific fields of PCCB */

	struct	_gvpbq	dfreeq;		/* Datagram free queue head	*/
	struct	_gvpbq	mfreeq;		/* Message free queue head	*/
	struct	bvpregs	*port_regs;	/* Pointer to Port Registers	*/
	struct biic_regs *nxv;		/* VA of Adapter BIIC Regs	*/
	struct	_pb	*pb;		/* Pointer to Path Block	*/
	u_int		bvp_ctlr;	/* Controller number		*/
	int		binumber;	/* BI number			*/
	int		binode;		/* BI node number		*/
	struct	bidata	*bidata;	/* Pointer to bidata structure	*/
	u_int		cmd_pend;	/* Port command pending vector	*/
	u_int		ivec;		/* Interrupt vector		*/
	int		poll_rate;	/* Timer poll rate		*/
	u_int		port_state;	/* Current port state		*/
	int		rip;		/* Recovery in progress ind.	*/
	int		incarn;		/* Port incarnation number	*/
	} BVPSSPPCCB;



typedef struct _bvp_ssppqb {		/* BVP SSP specific fields of PQB */

	struct _gvpbq	*dfreeq_hdr;	/* Datagram free queue head pointer */
	struct _gvpbq	*mfreeq_hdr; 	/* Message free queue head pointer  */
	u_int		dqe_len	: 16;	/* Datagram queue entry length	*/
	u_int			: 16;	/* MBZ				*/
	u_int		mqe_len	: 16;	/* Message queue entry length	*/
	u_int			: 16;	/* MBZ				*/
	struct _gvppqb	*vpqb_base;	/* PQB system virtual address	*/
	struct _gvpbd	*bdt_base;	/* BDT system virtual address	*/
	u_short		bdt_len;	/* BDT octaword length		*/
	u_short			: 16;	/* MBZ				*/
	struct pte	*spt_base;	/* System page table physical address*/
	u_int		spt_len	: 22;	/* SPT longword length		*/
	u_int			: 10;	/* MBZ				*/
	struct pte	*gpt_base;	/* Global page table physical address*/
	u_int		gpt_len	: 22;	/* GPT longword length		*/
	u_int			: 10;	/* MBZ				*/
	u_int		keep_alive;	/* Keep-alive timer		*/
	u_int		function_mask;	/* Port charateristics mask	*/
	u_int		piv;		/* Port Interrupt Vector	*/
	u_int		bvp_level;	/* BVP funtionality level	*/
	u_char		reserved1[ 32 ];/* Reserved			*/
	u_int		pd_prtvrs  :8;	/* Port driver PPD version	*/
	u_int		reserved2  :24;	/* Reserved			*/
	u_int		pd_max_dg  :16;	/* Maximum datagram size - port	*/
	u_int		pd_max_msg :16;	/* Maximum message size - port	*/
	u_int		pd_sw_type;	/* Operating system "U-32" 	*/
	u_int		pd_sw_version;	/* Operating system version	*/
	u_int		pd_hw_type;	/* Port hardware type		*/
	u_dodec		pd_hw_version;	/* Port hardware version	*/
	u_quad		pd_cur_time;	/* Current time			*/
	u_char		reserved4[ 24 ];/* Reserved			*/
	u_int		ad_prtvrs  :8;	/* Adapter PPD protocol version	*/
	u_int		ad_type	   :8;	/* Adapter port type		*/
	u_int		reserved3  :16;	/* Reserved			*/
	u_int		reserved6;	/* Reserved			*/
	u_int		ad_max_dg  :16;	/* Maximum datagram size - adap	*/
	u_int		ad_max_msg :16;	/* Maximum message size - adap	*/
	u_int		ad_sw_type;	/* Adapter software type	*/
	u_int		ad_sw_version;	/* Adapter software version	*/
	u_int		ad_hw_type;	/* Adapter hardware type	*/
	u_dodec		ad_hw_version;	/* Adapter harware version	*/
	u_char		reserved5[ 24 ];/* Reserved			*/
	struct	_gvph	*qe_logout[ BVP_NOLOG ];/* Queue Entry logout area */
	} BVPSSPPQB;

#endif
