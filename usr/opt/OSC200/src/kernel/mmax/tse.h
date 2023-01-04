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
 *	@(#)$RCSfile: tse.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:43:31 $
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
 *        Copyright 1985 Encore Computer Corporation
 *
 * ALL RIGHTS RESERVED. Licensed Material - Property of Encore Computer
 * Corporation. This software is made available solely pursuant to the
 * terms of a software license agreement which governs its use. 
 * Unauthorized duplication, distribution or sale are strictly prohibited.
 *
 * Include file description:
 *	Defines the structure of various hardware timers
 *
 * Original Author: Tony Anzelmo	Created on: 85/04/29
 */

/*
 * Intel 8253 timer definition.
 */

typedef	struct i8253 {				/* Intel 8253 device		*/
	char	clock[3];		/* Three clocks */
	char	n_mode;			/* One mode byte */
} I8253;

#define TIMER struct i8253

/* Definition of mode control bits */

/* Counter selection */

#define SC0	0x00			/* Select counter 0 */
#define SC1	0x40			/* Select counter 1 */
#define SC2	0x80			/* Select counter 2 */

/* Read/Load bits */

#define LATCH	0x00			/* Latch counter		*/
#define HIBYTE	0x20			/* Read/Load counter hi byte	*/
#define LOBYTE	0x10			/* Read/Load counter lo byte	*/
#define BYTES2	0x30			/* Read/Load both counter bytes	*/

/* Mode control bits */

#define TIM_MODE0	0x00		/* Interrupt on terminal count	*/
#define TIM_MODE1	0x02		/* Programmable one-shot	*/
#define TIM_MODE2	0x04		/* Rate generator		*/
#define TIM_MODE3	0x06		/* Square wave rate generator	*/
#define TIM_MODE4	0x08		/* Software triggered strobe	*/
#define TIM_MODE5	0x0a		/* Hardware triggered strobe	*/

/* BCD counter */

#define BCDCOUNT 0x01			/* Count value is BCD */

#ifndef	BYTE
#define BYTE	*(char *)&	/* Causes compiler to generate byte movs */
#endif

/*
 * MAXDRIFTRECIP
 */
#define	MAXDRIFTRECIP	(10000)	/* A good guess XXX */
