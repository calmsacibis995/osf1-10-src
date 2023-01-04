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
 *	@(#)$RCSfile: pit.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:19:37 $
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/* 
 * OSF/1 Release 1.0
 */

/*
 *  Copyright 1988, 1989 by Intel Corporation
 *
 *         INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *     This software is supplied under the terms of a license 
 *    agreement or nondisclosure agreement with Intel Corpo-
 *    ration and may not be copied or disclosed except in
 *    accordance with the terms of that agreement.
 */

#include <cputypes.h>
#if	defined(MB1) || defined(MB2) || EXL > 0
/* Definitions for 8254 Programmable Interrupt Timer ports on 386/20 */
#define PITCTR0_PORT	0xD0		/* counter 0 port */	
#define PITCTR1_PORT	0xD2		/* counter 1 port */	
#define PITCTR2_PORT	0xD4		/* counter 2 port */	
#define PITCTL_PORT	0xD6		/* PIT control port */
#else	AT386
/* Definitions for 8254 Programmable Interrupt Timer ports on AT 386 */
#define PITCTR0_PORT	0x40		/* counter 0 port */	
#define PITCTR1_PORT	0x41		/* counter 1 port */	
#define PITCTR2_PORT	0x42		/* counter 2 port */	
#define PITCTL_PORT	0x43		/* PIT control port */
#define PITAUX_PORT	0x61		/* PIT auxiliary port */
#define CLKNUM	1193167			/* clock speed for timer */
/* bits used in auxiliary control port for timer 2 */
#define PITAUX_GATE2	0x01		/* aux port, PIT gate 2 input */
#define PITAUX_OUT2	0x02		/* aux port, PIT clock out 2 enable */
#endif	/* AT386 */

/* Definitions for 8254 commands */

/* Following are used for Timer 0 */
#define PIT_C0          0x00            /* select counter 0 */
#define PIT_LOADMODE	0x30		/* load least significant byte followed
					 * by most significant byte */
#define PIT_NDIVMODE	0x04		/*divide by N counter */
#define PIT_SQUAREMODE	0x06		/* square-wave mode */

/* Used for Timer 1. Used for delay calculations in countdown mode */
#define PIT_C1          0x40            /* select counter 1 */
#define PIT_READMODE	0x30		/* read or load least significant byte
					 * followed by most significant byte */
#define PIT_RATEMODE	0x06		/* square-wave mode for USART */

#if	defined(MB1)
#define CLKNUM 12300			/* clock speed for the timer in hz 
					 * divided by the constant HZ
					 * ( defined in param.h )
					 */
#endif
#if	defined(MB2) || EXL > 0
#define CLKNUM 12500			/* clock speed for the timer in hz 
					 * divided by the constant HZ
					 * ( defined in param.h )
					 */
#endif

#if	EXL
/* added micro-timer support.   --- csy */
typedef struct time_latch {
		time_t	ticks;          /* time in HZ since boot */
		time_t	uticks;         /* time in 1.25 MHZ */
/* don't need these two for now.   --- csy */
/*		time_t  secs;           /* seconds since boot */
/*		time_t  epochsecs;      /* seconds since epoch */
	} time_latch;
/* a couple in-line assembly codes for efficiency. */
asm  int   intr_disable()
{
     pushfl
     cli
}

asm  int   intr_restore()
{
     popfl
}

#endif	EXL
