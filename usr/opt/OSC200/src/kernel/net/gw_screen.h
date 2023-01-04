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
 * @(#)$RCSfile: gw_screen.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/29 17:29:31 $
 */
/*
 *	Copyright (c) 1988 by Digital Equipment Corporation
 *	16 December 1988	Jeffrey Mogul/DECWRL
 *		Created.
 */

#ifndef _GW_SCREEN_H_
#define _GW_SCREEN_H_
/*
 * Definitions for Gateway Screening mechanism
 */

/*
 * Access to this facility should be by a new system call, but
 * to keep things simple, we use several new ioctls instead.
 */

#define	SIOCSCREENON	_IOWR('i', 51, int)
		/*
		 * Turns screening on/off (based on arg)
		 * and returns previous setting (value-result arg)
		 */
#define	SCREENMODE_OFF		0	/* THIS MUST BE ZERO */
#define	SCREENMODE_ON		1	/* THIS MUST BE NONZERO */
#define	SCREENMODE_NOCHANGE	2
		/* any other value == NOCHANGE as well */

#define	SIOCSCREEN	_IOWR('i', 52, struct screen_data)
		/*
		 * Transmits from user to kernel an screen_data
		 * struct, and then copies the next one from kernel
		 * to user into the same buffer (value-result arg).
		 * This allows us to do each transaction with
		 * only one system call.
		 *
		 * This ioctl blocks until the next transaction
		 * is available.
		 */

#define	SIOCSCREENSTATS	_IOR('i', 53, struct screen_stats)
		/*
		 * Provides current statistics block
		 */

/*
 * For each packet, a transaction occurs where the kernel
 * passes the packet header out to the user process, which
 * checks the values and then tells the kernel whether or
 * not to allow the packet.
 *
 * We stick this header struct before the packet itself.
 * Some fields of this struct are "output" fields (kernel write,
 * user read), and some are "input" (user write, kernel read).
 */

struct screen_data_hdr {
	short	sdh_count;	/* length of entire record */	/* OUT */
	short	sdh_dlen;	/* bytes of packet header */	/* OUT */
	u_int	sdh_xid;	/* transaction ID */		/* OUT */
	struct timeval sdh_arrival;				/* OUT */
				/* time at which this packet arrived */
	short	sdh_family;	/* address family */		/* OUT */

	int	sdh_action;	/* disposition for this pkt */	/* IN */
				/*	see defs below      */

	/* Other fields: incoming i/f name? */
};

/*
 * Possible dispositions of the packet
 */
#define	SCREEN_ACCEPT	0x0001	/* Accept this packet */
#define	SCREEN_DROP	0x0000	/* Don't accept this packet */
#define	SCREEN_NOTIFY	0x0002	/* Notify the sender of failure */
#define	SCREEN_NONOTIFY	0x0000	/* Don't notify the sender */

/*
 * Screening information + the actual packet
 */

#define	SCREEN_MAXLEN	120	/* length of struct screen_data */
#if	SCREEN_MAXLEN > IOCPARM_MASK
THIS IS TOO BIG TO BE USED WITH IOCTL!
#endif /* SCREEN_MAXLEN > IOCPARM_MASK	*/

struct screen_data {
	struct screen_data_hdr sd_hdr;				/* IN/OUT */

#define	SCREEN_DATALEN	(SCREEN_MAXLEN - sizeof(struct screen_data_hdr))
	char	sd_data[SCREEN_DATALEN];
				/* sd_dlen bytes of pkt hdr */	/* OUT */
};

#define	sd_count	sd_hdr.sdh_count
#define	sd_dlen		sd_hdr.sdh_dlen
#define	sd_xid		sd_hdr.sdh_xid
#define	sd_action	sd_hdr.sdh_action
#define	sd_arrival	sd_hdr.sdh_arrival
#define	sd_family	sd_hdr.sdh_family

struct screen_stats {
	u_long	ss_packets;	/* total packets screened */
	u_long	ss_nobuffer;	/* dropped because buffer was full */
	u_long	ss_accept;	/* total accepted */
	u_long	ss_reject;	/* total rejected */
	u_long	ss_badsync;	/* dropped because user was out of sync */
	u_long	ss_stale;	/* dropped because too old */
};

#endif
