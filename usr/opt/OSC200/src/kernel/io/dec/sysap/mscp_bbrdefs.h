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
 * derived from mscp_bbrdefs.h	2.3	(ULTRIX)	10/12/89
 */
/*
 *
 *   Facility:	Systems Communication Architecture
 *		MSCP-speaking Class Drivers
 *
 *   Abstract:	This module contains the MSCP Bad Block Replacement
 *		definitions.
 *
 *   Author:	David E. Eiche	Creation Date:	November 15, 1987
 *
 *   History:
 *
 *	27-Jul-1988	Pete Keilty
 *		Added logerr flag to the flags word & MSLG structure
 *		to struct _bbrb both used for errlogging.
 *
 */
/**/
#ifndef _MSCP_BBRDEFS_H_
#define _MSCP_BBRDEFS_H_

#include <io/dec/sysap/mscp_defs.h>
#include <io/dec/sysap/mscp_msg.h>

/* BBR state stack manipulation macros.
 */
#define Push_bbr_state( new_state ) { \
    if( bbrp->stack_depth >= 6 ) \
	panic( "mscp_bbr_xxx: stack overflow\n" ); \
    bbrp->stack[ bbrp->stack_depth++ ] = rp->state; \
    rp->state = new_state; }

#define Pop_bbr_state() { \
    if( bbrp->stack_depth == 0 ) \
	panic( "mscp_bbr_xxx: stack underflow\n" ); \
    rp->state = bbrp->stack[ --bbrp->stack_depth ]; }

/* Replacement control table sector 0 format
 */
typedef struct {
    u_int		volser[2];	/* Volume serial number		     */
    u_short		flags;		/* BBR status flags		     */
    u_short			   :16;	/* Reserved			     */
    u_int		lbn;		/* LBN of replaced block	     */
    u_int		rbn;		/* Replacement block number	     */
    u_int		badrbn;		/* RBN of replaced (bad) RBN	     */
    u_char		s0_rsvd[488];	/* Reserved			     */
} RCT_SECTOR_0;

typedef struct {
    u_char		data[512];	/* Temporary data storage	     */
} RCT_SECTOR_1;

/* RBN descriptor format
 */
typedef struct {
    u_int		lbn	   :28;	/* Revectored LBN		     */
    u_int		code	   :4;	/* Descriptor code		     */
} RCT_DESC;

/* Replacement control table sector k format (where 2 <= k < rct size)
 */
typedef struct {
    RCT_DESC		desc[128];	/* RCT descriptor vector	     */
} RCT_SECTOR_K;

/* RCT sector 0 flags word definitions
 */
#define RCT_S0_FE		0x0080	/* Forced error			     */
#define RCT_S0_BR		0x2000	/* Bad replacement block	     */
#define RCT_S0_P2		0x4000	/* BBR phase 2 in progress	     */
#define RCT_S0_P1		0x8000	/* BBR phase 1 in progress	     */

/* RCT descriptor code definitions
 */
#define RCT_DS_UNALLOC		0x00	/* Unallocated RB		     */ 
#define RCT_DS_PRIMARY		0x02	/* Primary RB			     */
#define RCT_DS_NONPRIM		0x03	/* Nonprimary RB		     */
#define RCT_DS_UNUSABL		0x04	/* Unusable RB			     */
#define RCT_DS_UNUSABLALT	0x05	/* Unusable RB alt. code	     */
#define RCT_DS_NULL		0x08	/* NULL sentinel entry		     */

/* BBR status flags definitions
 */
#define BBR_FL_FE		0x0001	/* Write LBN with forced error	     */
#define BBR_FL_ERROR		0x0002	/* Error during stress testing	     */
#define BBR_FL_MATCH		0x0004	/* (S) Matching LBN seen	     */
#define BBR_FL_NONPRIM		0x0008	/* (S) Descriptor is nonprimary	     */
#define BBR_FL_EOT		0x0010	/* (S) Table end encountered	     */
#define BBR_FL_DONE		0x0020	/* (S) Reserved			     */
#define BBR_FL_P2RECOV		0x0040	/* Recovering from phase 2 error     */
#define BBR_FL_BUSY		0x0080	/* BBR active on unit		     */
#define BBR_FL_TRANS		0x0100	/* Block was not replaced	     */
#define BBR_FL_FULL		0x0200	/* RCT full			     */
#define BBR_FL_RCTCOR		0x0400	/* RCT corrupt			     */
#define BBR_FL_RECURS		0x0800	/* RCT recursion failure	     */
#define BBR_FL_RPLFL		0x1000	/* REPLACE failed		     */
#define BBR_FL_FORCE		0x2000	/* Force replacement (via radisk)    */
#define BBR_FL_MWFAIL		0x4000	/* Multi-write failed		     */
#define BBR_FL_MRFAIL		0x8000	/* Multi-read failed		     */
#define BBR_FL_RPLATT		0x10000	/* RCT was modified		     */

/* Misc Defines
 */
#define BBR_BLOCKSIZE		512

typedef struct _bbrb {
    struct {				/* Queue of requests waiting for     */
	REQB		*flink;		/*  BBR service			     */
	REQB		*blink;		/*  ...				     */
    } bbr_wq;				/*  ...				     */
    REQB		*cur_reqb;	/* Current request being serviced    */
    u_int		lbn;		/* LBN on which error occurred	     */
    u_int		rbn;		/* RBN of replacement block	     */
    union {				/* Status flags			     */
	u_int		mask;		/*    accessed as a mask	     */
	struct {			/* Accessed as individual bits	     */
	    u_int	fe	   :1;	/* Write LBN with forced error	     */
	    u_int	error	   :1;	/* Error during stress testing	     */
	    u_int	match	   :1;	/* (S) Matching LBN seen	     */
	    u_int	nonprim	   :1;	/* (S) Desdcriptor is nonprimary     */
	    u_int	eot	   :1;	/* (S) Table end encountered	     */
	    u_int	done	   :1;	/* (S) Reserved			     */
	    u_int	p2recov	   :1;	/* Recovering from phase 2 error     */
	    u_int	busy	   :1;	/* BBR active on connection	     */
	    u_int	trans	   :1;	/* Block was transient error	     */
	    u_int	full	   :1;	/* RCT full condition		     */
	    u_int	corrupt	   :1;	/* RCT corrupt			     */
	    u_int	recurs	   :1;	/* RCT recursion error		     */
	    u_int	repfail	   :1;	/* REPLACE failed		     */
	    u_int	force	   :1;	/* Force replacement (via radisk)    */
	    u_int	mwfail	   :1;	/* Failure during multi-write	     */
	    u_int	mrfail	   :1;	/* Failure during multi-read	     */
	    u_int	rplatt	   :1;	/* Replacement attempted	     */
	    u_int	logerr	   :1;	/* logging error packet to errlog */
	    u_int		   :14;	/* Reserved			     */
	} bit;
    } flags;
    char		recursion_ct;	/* Replacement recursion counter     */
    char		loop_ct1;	/* Loop/retry counter		     */
    char		loop_ct2;	/* Loop/retry counter		     */
    char		copy_ct;	/* (M) Multi-read/write copy counter */
    char		bad_copies;	/* (M) Multi-write bad copy counter  */
    u_char		*multi_buf;	/* (M) Multi-read/write buffer ptr   */
    u_int		hash_rbn;	/* (S) Primary descriptor RBN	     */
    u_int		hash_block;	/* (S) Primary descriptor RCT block  */
    u_int		hash_offset;	/* (S) Primary descriptor RCT offset */
    u_int		cur_rbn;	/* (S) Current RBN being searched    */
    u_int		cur_block;	/* (S) Current RCT block 	     */
    u_int		match_rbn;	/* (S) Allocated RB that matches LBN */
    u_int		max_host_rbn;	/* (S) Maximum host RBN		     */
    RCT_DESC		prev_desc;	/* Previous RBN descriptor	     */
    u_int		stack_depth;	/* State stack depth		     */
    u_int		stack[6];	/* State stack			     */
    u_char		buf0[512];	/* Buffer 0 - Usually RCT page 0     */
    u_char		buf1[512];	/* Buffer 1 - Usually RCT page 1     */
    u_char		buf2[512];	/* Buffer 2 - utility buffer	     */
    u_char		buf3[512];	/* Buffer 3 - utility buffer	     */
    REQB		bbr_reqb;	/* Request block for BBR operations  */
    MSLG		bbr_mslg;	/* Last mslg datagram packet      */
    struct buf		bbr_buf;	/* Buf structure for BBR operations  */
    struct slock	bbrq_lk;	/* SMP lock */
} BBRB;

#endif
