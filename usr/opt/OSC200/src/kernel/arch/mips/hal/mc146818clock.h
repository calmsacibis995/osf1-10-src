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
 *	@(#)$RCSfile: mc146818clock.h,v $ $Revision: 1.2.4.2 $ (DEC) $Date: 1992/03/18 15:21:40 $
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
 * derived from mc146818clock.h	4.3	(ULTRIX)	9/1/90
 */


/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */

/* ------------------------------------------------------------------------
 * Modification History: mc146818clock.h
 *
 * 31-Aug-89 -- afd
 *	Remove hard-wired define for RT_CLOCK_ADDR.
 *	rt_clock_addr will be assigned in processor specific kn__init routines.
 *
 * ------------------------------------------------------------------------
 */

#define	FILL3(x)	char fill_/**/x[3]
/*
 * Definitions for use HD146818 real time clock
 */
struct rt_clock {
	u_char	rt_secs;		/* current seconds */
	FILL3(0);
	u_char	rt_seca;		/* alarm seconds */
	FILL3(1);
	u_char	rt_mins;		/* current minutes */
	FILL3(2);
	u_char	rt_mina;		/* alarm minutes */
	FILL3(3);
	u_char	rt_hours;		/* current hours */
	FILL3(4);
	u_char	rt_houra;		/* alarm hours */
	FILL3(5);
	u_char	rt_dayw;		/* day of the week */
	FILL3(6);
	u_char	rt_daym;		/* day of the month */
	FILL3(7);
	u_char	rt_month;		/* month */
	FILL3(8);
	u_char	rt_year;		/* year */
	FILL3(9);
	u_char	rt_rega;		/* register a */
	FILL3(10);
	u_char	rt_regb;		/* register b */
	FILL3(11);
	u_char	rt_regc;		/* register c */
	FILL3(12);
	u_char	rt_regd;		/* register d */
	FILL3(13);
	u_char	rt_mem[50*4];		/* general purpose battery-bkup ram */
};

#define	RT_MEMX(x)	((x)<<2)

/*
 * Register A bit definitions
 */
#define	RTA_UIP		0x80		/* update in progress */
#define	RTA_DV4M	(0<<4)		/* time base is 4.194304 MHz */
#define	RTA_DV1M	(1<<4)		/* time base is 1.048576 MHz */
#define	RTA_DV32K	(2<<4)		/* time base is 32.768 kHz */
#define	RTA_DVRESET	(7<<4)		/* reset divider chain */
#define	RTA_RSNONE	0		/* disable periodic intr and SQW */
#define RTA_RS0		0x01
#define RTA_RS1		0x02
#define RTA_RS2		0x04
#define RTA_RS3		0x08
/* Here are rates to drive the periodic interrupt - we can try other rates */
#define RTA_1ms		RTA_RS1|RTA_RS2			/* every 1 ms */
#define RTA_2ms		RTA_RS0|RTA_RS1|RTA_RS2		/* every 2 ms */
#define RTA_4ms		RTA_RS3				/*       4 ms */
#define RTA_8ms		RTA_RS0|RTA_RS3			/*       8 ms */
#define RTA_15ms	RTA_RS1|RTA_RS3			/*      15 ms */

/*
 * Register B bit definitions
 */
#define	RTB_SET		0x80		/* inhibit date update */
#define	RTB_PIE		0x40		/* enable periodic interrupt */
#define	RTB_AIE		0x20		/* enable alarm interrupt */
#define	RTB_UIE		0x10		/* enable update-ended interrupt */
#define	RTB_SQWE	0x08		/* square wave enable */
#define	RTB_DMBINARY	0x04		/* binary data (0 => bcd data) */
#define	RTB_24HR	0x02		/* 24 hour mode (0 => 12 hour) */
#define	RTB_DSE		0x01		/* daylight savings mode enable */

/*
 * Register C bit definitions
 */
#define	RTC_IRQF	0x80		/* interrupt request flag */
#define	RTC_PF		0x40		/* periodic interrupt flag */
#define	RTC_AF		0x20		/* alarm interrupt flag */
#define	RTC_UF		0x10		/* update-ended interrupt flag */

/*
 * Register D bit definitions
 */
#define	RTD_VRT		0x80		/* valid RAM and time flag */

/*
 * Definitions for 8254 programmable interval timer
 *
 * NOTE: counter2 is clocked at MASTER_FREQ (3.6864 MHz), the
 * output of counter2 is the clock for both counter0 and counter1.
 * Counter0 output is tied to Interrupt 2 to act as the scheduling
 * clock and the output of counter1 is tied to Interrupt 4 to act as
 * the profiling clock.
 */

struct pt_clock {
	u_char	pt_counter0;		/* counter 0 port */
	FILL3(0);
#ifndef ultrix
	u_char	pt_counter1;		/* counter 1 port */
	FILL3(1);
	u_char	pt_counter2;		/* counter 2 port */
	FILL3(2);
	u_char	pt_control;		/* control word */
#endif /* ultrix */
};

/*
 * control word definitions
 */
#define	PTCW_SC(x)	((x)<<6)	/* select counter x */
#define	PTCW_RBCMD	(3<<6)		/* read-back command */
#define	PTCW_CLCMD	(0<<4)		/* counter latch command */
#define	PTCW_LSB	(1<<4)		/* r/w least signif. byte only */
#define	PTCW_MSB	(2<<4)		/* r/w most signif. byte only */
#define	PTCW_16B	(3<<4)		/* r/w 16 bits, lsb then msb */
#define	PTCW_MODE(x)	((x)<<1)	/* set mode to x */
#define	PTCW_BCD	0x1		/* operate in BCD mode */

/*
 * Mode definitions
 */
#define	MODE_ITC	0		/* interrupt on terminal count */
#define	MODE_HROS	1		/* hw retriggerable one-shot */
#define	MODE_RG		2		/* rate generator */
#define	MODE_SQW	3		/* square wave generator */
#define	MODE_STS	4		/* software triggered strobe */
#define	MODE_HTS	5		/* hardware triggered strobe */

#define	CRYSTAL_HZ	3686400		/* input clock to master divider */

/*
 * MACRO to clear cpu board TIM0 acknowledge register. We use a macro 
 * rather than a function so that we can inline it. If we ever get a 
 * compiler that can do this for us this should go back to being defined
 * as a function.
 */
#define	MC146818ACKRTCLOCK()						     \
   {									     \
   extern char *rt_clock_addr;	/* addr of the mc146818clock chip */	     \
   register volatile struct rt_clock *rtc =(struct rt_clock *)rt_clock_addr; \
   register int c;							     \
   c = rtc->rt_regc;							     \
   }
