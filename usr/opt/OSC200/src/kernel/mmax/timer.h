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
 *	@(#)$RCSfile: timer.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:43:21 $
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */

#ifndef	_TIMER_MACHINE_
#define _TIMER_MACHINE_

/*
 *	Machine dependent timer definitions.
 */

/*
 *	TIMER_MAX is not used on the MULTIMAX because a 32-bit rollover
 *	timer does not need to be adjusted for maximum value.
 */

/*
 *	TIMER_RATE is the rate of the timer in ticks per second.
 *	It is used to calculate percent cpu usage.
 */

#define TIMER_RATE	1000000

/*
 *	TIMER_HIGH_UNIT is the unit for high_bits in terms of low_bits.
 *	Setting it to TIMER_RATE makes the high unit seconds.
 */

#define TIMER_HIGH_UNIT	TIMER_RATE

/*
 *	TIMER_ADJUST is used to adjust the value of a timer after
 *	it has been copied into a time_value_t.  No adjustment is needed
 *	on Multimax because high_bits is in seconds.
 */

/*
 *	Following definitions swiped from sccdefs.h
 */
#define SCCMEM_BASE		0xfffc0000
#define SCCMEM_BASE_24		0x00fc0000
#define SCCREG_BASE		(SCCMEM_BASE + 0x10000)
#define SCCREG_FRCNT	(long *)(SCCREG_BASE + 0x200)
#define FRcounter	*SCCREG_FRCNT

/*
 *	get_timestamp() macro returns a 32 bit timestamp from FR counter
 */
#define get_timestamp()	(FRcounter)

#endif	_TIMER_MACHINE_

