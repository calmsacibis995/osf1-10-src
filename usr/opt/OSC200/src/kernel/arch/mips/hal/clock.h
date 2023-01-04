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
 *	@(#)$RCSfile: clock.h,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:09:37 $
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
 * derived from clock.h	2.1	(ULTRIX)	6/7/90 
 */
/* ------------------------------------------------------------------------
 * Modification History:
 *
 * 12-Sep-1990	burns
 *	First hack at moving to OSF/1 (snap3). Added ifdef'd clkwrap
 *	macro.
 *
 * 19-Jun-85 -- jaw
 *	VAX8200 name change.
 *
 * 7 JAN 85 -- jaw
 *	Add support for VAX8200.
 *
 * ------------------------------------------------------------------------
 */


/*
 * VAX clock registers
 */

#define	ICCS_RUN	0x00000001
#define	ICCS_TRANS	0x00000010
#define	ICCS_SS		0x00000020
#define	ICCS_IE		0x00000040
#define	ICCS_INT	0x00000080
#define	ICCS_ERR	0x80000000
	
/*
 * General time definitions
 */
#define	SECMIN	((unsigned)60)			/* seconds per minute */
#define	SECHOUR	((unsigned)(60*SECMIN))		/* seconds per hour */
#define	SECDAY	((unsigned)(24*SECHOUR))	/* seconds per day */
#define	SECYR	((unsigned)(365*SECDAY))	/* sec per reg year */

#define	YRREF		1970
#define	LEAPYEAR(year)	((year)%4==0)	/* good till time becomes negative */
#define	BASEYEAR	72			/* MUST be a leap year */


/* - burns - taken from ka650.h
 * Bits in TCR0/TCR1: Programable Timer Control Registers (cvqssc->cvq4_tcrx)
 * (The rest of the bits are the same as in the standard VAX
 *  Interval timer and are defined in clock.h)
 */
#define TCR_STP 0x00000004		/* <2>  Stop after time-out */
