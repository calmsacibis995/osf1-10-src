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
 *	@(#)$RCSfile: rtc.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:10:08 $
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
 * Copyright (c) 1988 Intel Corporation.
 * All rights reserved.
 *
 * INTEL CORPORATION PROPRIETARY INFORMATION
 *
 * This software is supplied under the terms of a license agreement 
 * or nondisclosure agreement with Intel Corporation and may not be 
 * copied or disclosed except in accordance with the terms of that 
 * agreement.
 *
 */

#define RTC_ADDR	0x70	/* I/O port address of for register select */
#define RTC_DATA	0x71	/* I/O port address for data read/write */

/*
 * Register A definitions
 */
#define RTC_A		0x0a	/* register A address */
#define RTC_UIP		0x80	/* Update in progress bit */
#define RTC_DIV0	0x00	/* Time base of 4.194304 MHz */
#define RTC_DIV1	0x10	/* Time base of 1.048576 MHz */
#define RTC_DIV2	0x20	/* Time base of 32.768 KHz */
#define RTC_RATE6	0x06	/* interrupt rate of 976.562 */

/*
 * Register B definitions
 */
#define RTC_B		0x0b	/* register B address */
#define RTC_SET		0x80	/* stop updates for time set */
#define RTC_PIE		0x40	/* Periodic interrupt enable */
#define RTC_AIE		0x20	/* Alarm interrupt enable */
#define RTC_UIE		0x10	/* Update ended interrupt enable */
#define RTC_SQWE	0x08	/* Square wave enable */
#define RTC_DM		0x04	/* Date mode, 1 = binary, 0 = BCD */
#define RTC_HM		0x02	/* hour mode, 1 = 24 hour, 0 = 12 hour */
#define RTC_DSE		0x01	/* Daylight savings enable */

/* 
 * Register C definitions
 */
#define RTC_C		0x0c	/* register C address */
#define RTC_IRQF	0x80	/* IRQ flag */
#define RTC_PF		0x40	/* PF flag bit */
#define RTC_AF		0x20	/* AF flag bit */
#define RTC_UF		0x10	/* UF flag bit */

/*
 * Register D definitions
 */
#define RTC_D		0x0d	/* register D address */
#define RTC_VRT		0x80	/* Valid RAM and time bit */

#define RTC_NREG	0x0e	/* number of RTC registers */
#define RTC_NREGP	0x0a	/* number of RTC registers to set time */

#define RTCRTIME	_IOR('c', 0x01, struct rtc_st) /* Read time from RTC */
#define RTCSTIME	_IOW('c', 0x02, struct rtc_st) /* Set time into RTC */

struct rtc_st {
	char	rtc_sec;
	char	rtc_asec;
	char	rtc_min;
	char	rtc_amin;
	char	rtc_hr;
	char	rtc_ahr;
	char	rtc_dow;
	char	rtc_dom;
	char	rtc_mon;
	char	rtc_yr;
	char	rtc_statusa;
	char	rtc_statusb;
	char	rtc_statusc;
	char	rtc_statusd;
};

/*
 * this macro reads contents of real time clock to specified buffer 
 */
#define load_rtc(regs) \
{\
	register int i; \
	\
	for (i = 0; i < RTC_NREG; i++) { \
		outb(RTC_ADDR, i); \
		regs[i] = inb(RTC_DATA); \
	} \
}

/*
 * this macro writes contents of specified buffer to real time clock 
 */ 
#define save_rtc(regs) \
{ \
	register int i; \
	for (i = 0; i < RTC_NREGP; i++) { \
		outb(RTC_ADDR, i); \
		outb(RTC_DATA, regs[i]);\
	} \
}	


