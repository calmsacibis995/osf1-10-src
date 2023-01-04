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
 *	@(#)$RCSfile: if_slvar.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/06/29 17:31:53 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * Definitions for SLIP interface data structures
 * 
 * (this exists so programs like slstats can get at the definition
 *  of sl_softc.)
 *
 *	Base:	if_slvar.h	7.4 (Berkeley) 1/20/90
 *		from if_slvar.h,v 1.3 89/05/31 02:25:18 van Exp
 */

#ifndef _IF_SLVAR_H_
#define _IF_SLVAR_H_

struct sl_softc {
	struct	ifnet sc_if;		/* network-visible interface */
	struct	ifqueue sc_fastq;	/* interactive output queue */
	struct	tty *sc_ttyp;		/* pointer to tty structure */
	u_char	*sc_mp;			/* pointer to next available buf char */
	u_char	*sc_ep;			/* pointer to last available buf char */
	u_char	*sc_buf;		/* input buffer */
	caddr_t	sc_cluster;		/* cluster page holding above */
	u_int	sc_flags;		/* see below */
	u_int	sc_escape;	/* =1 if last char input was FRAME_ESCAPE */
	u_int	sc_bytessent;
	u_int	sc_bytesrcvd;
	long	sc_lasttime;		/* last time a char arrived */
	long	sc_starttime;		/* last time a char arrived */
	long	sc_abortcount;		/* number of abort escape chars */
	struct	slcompress sc_comp;	/* tcp compression data */
};

/* visible flags */
#define	SC_COMPRESS	0x0002		/* compress TCP traffic */
#define	SC_NOICMP	0x0004		/* supress ICMP traffic */
#define	SC_AUTOCOMP	0x0008		/* auto-enable TCP compression */
/* internal flags (should be separate) */
#define	SC_ABORT	0x10000		/* have been sent an abort request */

#endif
