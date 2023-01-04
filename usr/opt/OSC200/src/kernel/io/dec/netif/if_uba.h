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
/* ------------------------------------------------------------------------
 * Modification History: /sys/vaxif/if_uba.h
 *
 * 11-jul-85 -- jaw
 *	fix bua/bda map registers.
 *
 * 19-Jun-85 -- jaw
 *	VAX8200 name change.
 * 
 * 13 Mar 85 -- Jaw
 * 	add support for VAX8200 and bua.	
 *
 * -----------------------------------------------------------------------
 */
#ifndef _IF_UBA_H_
#define _IF_UBA_H_
/*
 * Structure and routine definitions
 * for UNIBUS network interfaces.
 */

#define	IF_MAXNUBAMR	10
/*
 * Each interface has one of these structures giving information
 * about UNIBUS resources held by the interface.
 *
 * We hold IF_NUBAMR map registers for datagram data, starting
 * at ifr_mr.  Map register ifr_mr[-1] maps the local network header
 * ending on the page boundary.  Bdp's are reserved for read and for
 * write, given by ifr_bdp.  The prototype of the map register for
 * read and for write is saved in ifr_proto.
 *
 * When write transfers are not full pages on page boundaries we just
 * copy the data into the pages mapped on the UNIBUS and start the
 * transfer.  If a write transfer is of a (1024 byte) page on a page
 * boundary, we swap in UNIBUS pte's to reference the pages, and then
 * remap the initial pages (from ifu_wmap) when the transfer completes.
 *
 * When read transfers give whole pages of data to be input, we
 * allocate page frames from a network page list and trade them
 * with the pages already containing the data, mapping the allocated
 * pages to replace the input pages for the next UNIBUS data input.
 */

struct	ifuba {
	short	ifu_uban;			/* uba number */
	short	ifu_hlen;			/* local net header length */
	struct	uba_regs *ifu_uba;		/* uba regs, in vm */
	struct ifrw {
		caddr_t	ifrw_addr;		/* virt addr of header */
		int	ifrw_bdp;		/* unibus bdp */
		int	ifrw_info;		/* value from ubaalloc */
		int	ifrw_proto;		/* map register prototype */
		struct	pt_entry *ifrw_mr;		/* base of map registers */
	} ifu_r, ifu_w;
	struct	pt_entry ifu_wmap[IF_MAXNUBAMR];	/* base pages for output */
	short	ifu_xswapd;			/* mask of clusters swapped */
	short	ifu_flags;			/* used during uballoc's */
	struct	mbuf *ifu_xtofree;		/* pages being dma'd out */
};

#ifdef 	KERNEL
struct	mbuf *if_rubaget();
#endif

#endif
