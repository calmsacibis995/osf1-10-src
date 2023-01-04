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
 *	@(#)$RCSfile: mtio.h,v $ $Revision: 4.2.3.4 $ (DEC) $Date: 1992/06/03 13:45:14 $
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
 * derived from mtio.h	2.2	(ULTRIX/OSF)	1/15/91
 */
/*
 * mtio.h
 *
 * Modification History:
 *
 * 18-Jun-91	Tom Tierney
 *	Added norewind, first unit, high density default tape device
 *	(DEFTAPE_NH).
 *
 * 12-Jan-91	Fred Canter
 *
 *	Changed default tape device (DEFTAPE) to rmt0h.
 *
 */


/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */
/*
 * Structures and definitions for mag tape io control commands
 */

#ifndef	_SYS_MTIO_H_
#define _SYS_MTIO_H_

#include <sys/types.h>

/* Structure for MTIOCTOP ioctl - mag tape operation command */
struct	mtop	{
	short	mt_op;			/* Operations defined below	*/
	daddr_t mt_count;		/* How many of them		*/
};

/* Structure for MTIOCGET ioctl - mag tape get status command */
struct	mtget	{
	short	mt_type;		/* Type of device defined below */
	short	mt_dsreg;		/* ``drive status'' register	*/
	short	mt_erreg;		/* ``error'' register		*/
	short	mt_resid;		/* Residual count		*/
};

/* Basic definitions common to various tape drivers */
#define b_repcnt	b_bcount		/* Repeat count 	*/
/*  #define b_command	b_resid		 	/* Command value NOTE: in buf.h */
#define SSEEK		0x01			/* Seeking		*/
#define SIO		0x02			/* Doing sequential i/o */
#define SCOM		0x03			/* Sending control cmd. */
#define SREW		0x04			/* Sending drive rewnd. */
#define SERASE		0x05			/* Erase interrec. gap	*/
#define SERASED 	0x06			/* Erased interrec. gap */
#define MASKREG(r)	((r) & 0xffff)		/* Register mask	*/
#define INF		(daddr_t)1000000L	/* Invalid block number */
#define DISEOT		0x01			/* Disable EOT code	*/
#define DBSIZE		0x20			/* Dump blocksize (32)	*/
#define PHYS(a,b)	((b)((int)(a)&0x7fffffff)) /* Physical dump dev.*/

#define REWIND_DEV	0x00		/* Rewind device		*/
#define NO_REWIND	0x01		/* No rewind device		*/
#define DENS_MASK	0x06		/* Mask off the density bits	*/
#define LOW_DENS	0x00		/* Low density			*/
#define MED_DENS	0x04		/* Medium density		*/
#define HI_DENS		0x02		/* High density			*/
#define AUX_DENS	0x06		/* Auxiliary density		*/
#define MTLR		(LOW_DENS|REWIND_DEV)	/* Low density/Rewind        */
#define MTMR		(MED_DENS|REWIND_DEV)	/* Medium density/Rewind     */
#define MTHR		(HI_DENS|REWIND_DEV)	/* High density/Rewind       */
#define MTAR		(AUX_DENS|REWIND_DEV)	/* Auxiliary density/Rewind  */
#define MTLN		(LOW_DENS|NO_REWIND)	/* Low density/Norewind      */ 
#define MTMN		(MED_DENS|NO_REWIND)	/* Medium density/Norewind   */
#define MTHN		(HI_DENS|NO_REWIND)	/* High density/Norewind     */
#define MTAN		(AUX_DENS|NO_REWIND)	/* Auxiliary density/Rewind  */
#define MTX0		0x00			/* eXperimental 0 -historical*/
#define MTX1		0x01			/* eXperimental 1 -historical*/

/* Tape operation definitions for operation word (mt_op) */
#define MTWEOF		0x00		/* Write end-of-file record	*/
#define MTFSF		0x01		/* Forward space file		*/
#define MTBSF		0x02		/* Backward space file		*/
#define MTFSR		0x03		/* Forward space record 	*/
#define MTBSR		0x04		/* Backward space record	*/
#define MTREW		0x05		/* Rewind			*/
#define MTOFFL		0x06		/* Rewind and unload tape	*/
#define MTNOP		0x07		/* No operation 		*/
#define MTCACHE 	0x08		/* Enable tmscp caching 	*/
#define MTNOCACHE	0x09		/* Disable tmscp caching	*/
#define MTCSE		0x0a		/* Clear serious exception	*/
#define MTCLX		0x0b		/* Clear hard/soft-ware problem */
#define MTCLS		0x0c		/* Clear subsystem		*/
#define MTENAEOT	0x0d		/* Enable default eot code	*/
#define MTDISEOT	0x0e		/* Disable default eot code	*/
#define MTFLUSH		0x0f		/* Flush controller write cache */
#define MTGTON		0x10		/* Turn on gapless TBC tm32	*/
#define MTGTOFF		0x11		/* Torn off gapless mode	*/
#define MTRETEN		0x12		/* Retension command qic like	*/
#define MTSEOD		0x13		/* Space to end of recorded data */
#define MTERASE		0x14		/* Erase tape command.		*/
#define MTONLINE	0x15		/* Load a tape opposite of MTOFFL*/
#define MTLOAD		0x16		/* Issue a load tape		*/
#define MTUNLOAD	0x17		/* Issue a unoad tape 		*/

/* Get status definitions for device type word (mt_type) */
#define MT_ISTS 	0x01		/* ts11/ts05/tu80		*/
#define MT_ISHT 	0x02		/* tm03/te16/tu45/tu77		*/
#define MT_ISTM 	0x03		/* tm11/te10			*/
#define MT_ISMT 	0x04		/* tm78/tu78			*/
#define MT_ISUT 	0x05		/* tu45 			*/
#define MT_ISTMSCP	0x06		/* All tmscp tape drives	*/
#define MT_ISST		0x07		/* TZK50 on VS2000/MV2000	*/
#define MT_ISSCSI	0x08		/* SCSI tapes (TZK50 & TZ30)	*/

/* Default tape device definitions for programs */

#ifndef	KERNEL
#define DEFTAPE		"/dev/rmt0h"	/* 1st tape, rewind, high dens. */
#define DEFTAPE_NH	"/dev/nrmt0h"	/* 1st tape, norew., high dens. */
#define DEFTAPE_RM	"/dev/rmt0m"	/* 1st tape, rewind, med. dens. */
#define DEFTAPE_RH	"/dev/rmt0h"	/* 1st tape, rewind, high dens. */
#define DEFTAPE_NL	"/dev/nrmt0l"	/* 1st tape, norew., low dens.	*/
#define DEFTAPE_NM	"/dev/nrmt0m"	/* 1st tape, norew., med. dens. */
#endif /* KERNEL */
#endif /* _SYS_MTIO_H_ */
