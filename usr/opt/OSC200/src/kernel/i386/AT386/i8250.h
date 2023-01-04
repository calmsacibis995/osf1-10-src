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
 *	@(#)$RCSfile: i8250.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:08:31 $
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
 *         INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *     This software is supplied under the terms of a license 
 *    agreement or nondisclosure agreement with Intel Corpo-
 *    ration and may not be copied or disclosed except in
 *    accordance with the terms of that agreement.
 */

/*
 * Header file for i8250 chip
 */

/* port offsets from the base i/o address */

#define RDAT		0
#define RIE		1
#define RID		2
#define RFC		2
#define RLC		3
#define RMC		4
#define RLS		5
#define RMS		6
#define RDLSB		0
#define RDMSB		1

/* interrupt control register */

#define IERD		0x01	/* read int */
#define IETX		0x02	/* xmit int */
#define IELS		0x04	/* line status int */
#define IEMS		0x08	/* modem int */

/* interrupt status register */

#define IDIP		0x01	/* not interrupt pending */
#define IDMS		0x00	/* modem int */
#define IDTX		0x02	/* xmit int */
#define IDRD		0x04	/* read int */
#define IDLS		0x06	/* line status int */
#define IDMASK		0x0f	/* interrupt ID mask */

/* line control register */

#define LC5		0x00	/* word length 5 */
#define LC6		0x01	/* word length 6 */
#define LC7		0x02	/* word length 7 */
#define LC8		0x03	/* word length 8 */
#define LCSTB		0x04	/* 2 stop */
#define LCPEN		0x08	/* parity enable */
#define LCEPS		0x10	/* even parity select */
#define LCSP		0x20	/* stick parity */
#define LCBRK		0x40	/* send break */
#define LCDLAB		0x80	/* divisor latch access bit */
#define LCPAR		0x38	/* parity mask */

/* line status register */

#define LSDR		0x01	/* data ready */
#define LSOR		0x02	/* overrun error */
#define LSPE		0x04	/* parity error */
#define LSFE		0x08	/* framing error */
#define LSBI		0x10	/* break interrupt */
#define LSTHRE		0x20	/* xmit holding reg empty */
#define LSTSRE		0x40	/* xmit shift reg empty */

/* modem control register */

#define MCDTR		0x01	/* DTR */
#define MCRTS		0x02	/* RTS */
#define MCOUT1		0x04	/* OUT1 */
#define MCOUT2		0x08	/* OUT2 */
#define MCLOOP		0x10	/* loopback */

/* modem status register */

#define MSDCTS		0x01	/* delta CTS */
#define MSDDSR		0x02	/* delta DSR */
#define MSTERI		0x04	/* delta RE */
#define MSDRLSD 	0x08	/* delta CD */
#define MSCTS		0x10	/* CTS */
#define MSDSR		0x20	/* DSR */
#define MSRI		0x40	/* RE */
#define MSRLSD		0x80	/* CD */

/* divisor latch register settings for various baud rates */

#define BCNT1200	0x60
#define BCNT2400	0x30
#define BCNT4800	0x18
#define BCNT9600	0x0c
