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
 *	@(#)$RCSfile: icu.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:40:53 $
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

#include <mmax_xpc.h>
#include <mmax_apc.h>

/*
 * This file contains all the offsets and the masks to be used in
 * conjunction with the NS16202 Interrupt Control Unit.
 */

/*
 *		ICU Race Conditions And Interrupt Handling
 *
 * 1.  Background.
 * The ICU has three registers critical to interrupt handling:  the
 * in-service (ISRV) register, the pending (IPND) register and the
 * mask (IMSK) register.  The mask register is set at CPU initialization
 * time and defines which interrupts the ICU will propagate to the CPU.
 * The pending register contains a set bit for each interrupt source that
 * has notified the ICU of an interrupt condition but for which the ICU
 * has yet to notify its CPU.  The in-service register contains a set bit
 * for each interrupt the ICU has passed on to its CPU that the CPU has not
 * yet completed processing.
 *
 * The ICU is initialized for fixed-priority interrupts and with a number of
 * unused interrupts disabled in the mask register.
 *
 * When an incoming interrupt is not masked, and a higher-priority interrupt
 * is not pending or in service, the ICU asserts INT/ to the CPU and when the
 * CPU has interrupt service enabled (PSR_I bit set) the CPU will acknowledge
 * the ICU's interrupt, disable interrupt service, and vector to the
 * appropriate service routine.
 *
 * 2.  Disabling interrupts from the CPU.
 * The APC and XPC boards are designed so that all interrupt sources, including
 * the system-wide non-maskable interrupt (SYSNMI), are routed through the ICU.
 * Because the NMI line isn't hooked directly to the CPU, there is no way to
 * interrupt the CPU when it has disabled interrupts by clearing the I bit in
 * its PSR.  In normal operation this behavior would cause no problem but
 * for kernel debugging and profiling this behavior would prevent examination
 * of relatively large and critical sections of kernel code.
 *
 * To allow debugging and profiling, then, the CPU disables interrupts (DISINT,
 * splany) by setting a bit in the ICU's ISRV.  The common interrupt sources
 * (time-slice end and vectorbus) have lower-priority than the chosen ``block''
 * bit, the ICU_NORMAL_BIT.  When this bit is set, the only interrupt sources
 * that are acknowledged are soft and hard board errors, which result only in
 * error logging or system crash.
 *
 * 3.  ICU confusion.
 * The inherent asynchronous relationship between CPU, ICU, and interrupt
 * sources can lead to a few problems.  While the CPU is ``disabling
 * interrupts'' by setting the ICU_NORMAL_BIT in the ISRV, the ICU can be
 * notifying the CPU about an interrupt, e.g., the vectorbus.  In fact,
 * the CPU can wind up in an interrupt service routine with *two* bits set
 * in the ISRV:  the ICU_NORMAL_BIT (which was supposed to block interrupts)
 * and the bit for the taken interrupt (e.g., ICU_VECBUS_BIT).  Care must be
 * taken so that the interrupt handler will exit leaving the ICU_NORMAL_BIT
 * set and the taken interrupt's bit (ICU_VECBUS_BIT) clear in the ISRV so
 * that interrupts will be blocked for the section of code that was attempting
 * to block them (ICU_NORMAL_BIT set) but the given interrupt source will
 * still be enabled (ICU_VECBUS_BIT clear).
 *
 * 4.  ICU confusion, redux.
 * Setting an ICU ISRV bit while another interrupt is coming in can cause a
 * second kind of confusion.  The ICU might assert the CPU's INT/ line but by
 * the time the CPU does interrupt acknowledge cycles the ICU may have decided
 * that no interrupt condition exists because the newly-set ISRV bit is of
 * higher priority than the incoming interrupt.  The ICU will respond to the
 * CPU's interrupt acknowledge cycles with the infamous BOGUS interrupt, 0xf.
 *
 * 5.  Model for CPU handling of ICU isrv.
 * A.  ``Normal'' interrupts are disabled by setting the ICU_NORMAL_BIT.
 *	These interrupts include TSE, VECBUS and BOGUS.
 * B.  Incoming interrupts are translated into setting the NORMAL bit.
 *	CPU interrupts (PSR_I bit) are immediately re-enabled to allow
 *	debugging and profiling.
 * C.  When an interrupt service routine exits, it preserves the NORMAL
 *	bit when a race as described in section #3 has occured.  This is
 *	done by setting a higher-priority bit (ICU_NONFATAL_BIT) in the
 *	ISRV, to give the interrupt routine's RETI something to clear.
 * D.  BOGUS interrupts, resulting from the race described in section #4,
 *	are ignored.
 * E.  The remaining interrupts, all of higher priority than NORMAL,
 *	result in error logging, debugging actions, or a system panic.
 *
 * Cases:
 *	1.  Incoming TSE or VECBUS.  Also, ICU-generated BOGUS.
 *		Enter:
 *			Save status of old NORMAL bit.
 *			Set NORMAL bit, clear TSE/VECBUS bit
 *				(no bit to clear for BOGUS case).
 *			Enable processor interrupts.
 *		Exit:
 *			If old NORMAL bit status set:
 *				Set NONFATAL bit.
 *			RETI.
 *		Note:
 *			Subroutines will be called with "interrupts disabled".
 *	2.  Setting NORMAL bit to block interrupts.
 *		Action:  set NORMAL bit.
 *		Note:
 *			Possible races with case #1.
 *	3.  Soft error or slave ICU interrupts between NORMAL and NONFATAL.
 *		Enter:
 *			Enable processor interrupts.
 *		Exit:
 *			RETI.
 *		Notes:
 *			1.  Can race with setting NONFATAL.  At exit time,
 *			this RETI will clear the NONFATAL bit and leave
 *			the soft error or slave ICU bit(s) set.  However,
 *			as the purpose of the NONFATAL bit is to allow a
 *			RETI to be done without clearing the NORMAL bit,
 *			the interrupted routine's RETI will still work as
 *			desired, in this case clearing the soft error or
 *			slave ICU bit(s) while leaving the NORMAL bit set.
 *			2.  Subroutines invoked with "interrupts disabled".
 *	4.  Setting NONFATAL bit.
 *		Action:  set ICU_NONFATAL_BIT.
 *		Notes:
 *			1.  Possible races with case #3.
 *			2.  Races with case #1 do not happen as the
 *			ICU_NORMAL_BIT has always been set first.
 *	5.  Hard errors between NONFATAL and POWERFAIL.
 *		Enter:
 *			Enable processor interrupts.
 *		Exit:
 *			RETI (not really).
 *		Note:
 *			Can race with setting POWERFAIL, which is not
 *			a problem because the hard error will cause the
 *			operating system to panic.  No RETI will be done.
 *	6.  Incoming SYSNMI.
 *		Enter:
 *			DISNMI (sets POWERFAIL).
 *			Clear ICU_SYSNMI_BIT.
 *			Enable processor interrupts.
 *		Exit:
 *			RETT (br rett_ast).
 *		Note:
 *			Can race with setting DISNMI in dbmon.  Too bad.
 *	7.  Incoming/setting POWERFAIL (DISNMI).
 *		Enter:
 *			Enable processor interrupts.
 *		Exit:
 *			RETI (not really).
 *		Notes:
 *			1.  Incoming POWERFAIL causes o.s. to panic.
 *			2.  Setting the ICU_POWERFAIL_BIT (DISNMI) is
 *			only done in dbmon.  Ho, ho.
 */
#if	MMAX_XPC || MMAX_APC

#define T16N8   0x01            /* 16-bit mode          */
#define NTAR    0x02            /* auto-rotate disable  */
#define FRZ     0x08            /* freeze IPND          */
#define FRZ_BIT    3
#define CLKM    0x10            /* pulsed wave (8 bit)  */
#define COUTM   0x20
#define COUTD   0x40
#define CFRZ    0x80            /* freeze counter       */
/*
 * ICU counter control definitions
 */
#define CDCRL   0x01
#define CDCRH   0x02
#define CRUNL   0x04
#define CRUNH   0x08
#define COUT0   0x10
#define COUT1   0x20
#define CFNPS   0x40
#define CCON    0x80
/*
 * counter interrupt control
 */
#define WENL    0x01
#define CIEL    0x02
#define CIRL    0x04
#define CERL    0x08
#define WENH    0x10
#define CIEH    0x20
#define CIRH    0x40
#define CERH    0x80
/*
 * Set pending interrupt.
 */
#define ISET    0x80

#define TIMERSTARTH     0x08		/* 0x28 in umax4.2 (abl) */
#define TIMERSTARTL     0x04		/* 0x24 in umax4.2 (abl) */
#define TIMERSTOPH      0xf7
#define TIMERSTOPL      0xfb

#if	MMAX_XPC
/*
 *	ICU definitions for the XPC.  There is no slave ICU (good riddance).
 *
 * Interrupt Number	MASTER	    EDGE/LEVEL
 * ----------------	------
 *	       15	spare/		    E
 *	       14	VECTOR_BUS	    L
 *	       13	ICU_TIMER_H/ (TSE)  L
 *	       12	BLOCK_NORMAL/	    E
 *
 *	       11	ICU_TIMER_L/	    L	(not used)
 *	       10	VB_ERR		    L
 *		9	SOFT_NBI_ERR/	    E
 *		8	BTAG_TAG_PAR_ERR/   E
 *
 *		7	CACHE_PAR_ERR/	    E
 *		6	RCV_ADDR_PAR_ERR/   E
 *		5	BTAG_CACHE_ERR/	    E
 *		4	BTAG_ERR/	    E
 *
 *		3	DESTSEL_PAR_ERR	    E
 *		2	HARD_NBI_ERR/	    E
 *		1	SYSNMI/		    E
 *		0	POWERFAIL/	    E
 *
 *
 * ELTG	Edge/Level Triggering:		0 = Edge    1 = Level
 *
 * TPL		Trigger Polarity	0 = Low	    1 = High
 *
 * IOCONFIG	Interrupt/Port select	0 = Port    1 = Interrupt
 *
 * CVECT	Counter Interrupt Pin	High nibble = High counter interrupt#.
 *					Low nibble  = Low counter interrupt #.
 */

#define	ELTG_M		0x6c00
#define	TPL_M		0x4408
#define	CVECT_M		0xdb	/* High Counter on 13...Low Counter on 11.  */
				/* Since both counters are used for TSE	    */
				/* setting the low counter interrupt	    */
				/* position just has the effect of ignoring */
				/* external hardware interrupts at int 11.  */
#define	IOCONFIG_M	0xff

#define	PORTDIR_M	0xff	/* All inputs for now */

#define	CASCADE_M	0x0000	/* Cascaded ICUs -- none */

#define	ICU_POWERFAIL_BIT	0
#define	ICU_SYSNMI_BIT		1
#define	ICU_HARD_NBI_BIT	2
#define ICU_NONFATAL_BIT	2	/* must be at a fatal int level */
#define	ICU_DESTSEL_PAR_BIT	3
#define	ICU_BTAG_BIT		4
#define	ICU_BTAG_CACHE_BIT	5
#define	ICU_RCV_ADDR_PAR_BIT	6
#define ICU_CACHE_PAR_BIT	7
#define	ICU_BTAG_TAG_PAR_BIT	8
#define	ICU_SOFT_NBI_BIT	9
#define ICU_VB_ERR_BIT		10
#define	ICU_TIMER_L_BIT		11	/* Unused, bogus interrupt */
#define	ICU_NORMAL_BIT		12	/* Unused, above normal intrs	    */
#define	ICU_TIMER_H_BIT		13
#define	ICU_VECBUS_BIT		14
#define ICU_BOGUS_BIT		15

#define	ICU_POWERFAIL		0x00000001
#define	ICU_SYSNMI		0x00000002
#define	ICU_HARD_NBI		0x00000004
#define	ICU_DESTSEL_PAR		0x00000008
#define	ICU_BTAG		0x00000010
#define	ICU_BTAG_CACHE		0x00000020
#define	ICU_RCV_ADDR_PAR	0x00000040
#define ICU_CACHE_PAR		0x00000080
#define	ICU_BTAG_TAG_PAR	0x00000100
#define	ICU_SOFT_NBI		0x00000200
#define ICU_VB_ERR		0x00000400
#define	ICU_TIMER_L		0x00000800	/* Unused, bogus if occurs */
#define	ICU_NORMAL		0x00001000	/* Unused, above normal */
#define	ICU_TIMER_H		0x00002000
#define	ICU_VECBUS		0x00004000

/* Interupt mask:  excludes unused inputs. */
#define	ICU_ENABLE_ALL		0x000067ff
/*#define	ICU_ENABLE_ALL		0x00006fff*/
#define	ICU_ERRORS		0x000007fc
#define ICU_ENABLE_ERRORS	ICU_ERRORS

/* Masks for checking the Error Status Register */
#define ICU_ERR_REG_MASK    \
    (ICU_HARD_NBI | ICU_BTAG | ICU_BTAG_CACHE | ICU_CACHE_PAR | ICU_SOFT_NBI)
#define ELTG_M_ERR_REG	(ELTG_M | ICU_ERR_REG_MASK)
#endif	MMAX_XPC


#if	MMAX_APC
/*
 *
 *  Interrupt Number    MASTER      EDGE/LEVEL      SLAVE       EDGE/LEVEL
 *  ----------------    ---         ----------      -----       ----------
 *             15       spare/              E   *   spare/              E
 *             14       spare/              E   *   ICU_TIMER_S/ (PROF) L
 *             13       spare/              E   *   DUART/              L
 *             12       VECTOR_BUS          L   *   CACHE_PARITY/       E
 *
 *             11       ICU_TIMER_M/ (TSE)  L   *   VB_OUT_OF_SYNC/     E
 *             10       spare/              E   *   VB_PAR/             E
 *              9       SOFT_NBI0/          E   *   VB_NAK/             E
 *              8       SLAVE_ICU/          L   *   BTAG_PAR1/          E
 *
 *              7       block_nonfatal/     E   *   BTAG_PAR0/          E
 *              6       DESTSEL_PAR         E   *   RCV_ADDR_PAR3/      E
 *              5       HARD_NBI3/          E   *   RCV_ADDR_PAR2/      E
 *              4       HARD_NBI2/          E   *   RCV_ADDR_PAR1/      E
 *
 *              3       HARD_NBI1/          E   *   RCV_ADDR_PAR0/      E
 *              2       HARD_NBI0/          E   *   RCV_BYTE_PAR/       E
 *              1       SYSNMI/             E   *   SOFT_NBI2/          E
 *              0       POWERFAIL/          E   *   SOFT_NBI1/          E
 *
 *
 *  ELTG        Edge/Level Triggering:  0 = Edge    1 = Level
 *
 *  TPL         Trigger Polarity        0 = Low     1 = High
 *
 *  IOCONFIG    Interrupt/Port select   0 = Port    1 = Interrupt
 *
 *  CVECT       Counter Interrupt Pin   High nibble = High counter interrupt#.
 *                                      Low nibble  = Low counter interrupt #.
 */

#define ELTG_M          0x1900
#define ELTG_S          0x6000
#define	ELTG_S_VBNAK	0x6200	/* ELTG with VB NAK set to level	    */
#define TPL_M           0x1060
#define TPL_S           0x0000
#define CVECT_M         0xbb
#define CVECT_S         0xee
#define IOCONFIG_M      0xff
#define IOCONFIG_S      0xff
/*
 * Make them all inputs for now.
 */
#define PORTDIR_M       0xff
#define PORTDIR_S       0xff
/*
 * Define the cascaded lines.
 */
#define CASCADE_M       0x0100
#define CASCADE_S       0x0000

#define ICU_POWERFAIL_BIT       0
#define ICU_SYSNMI_BIT          1
#define ICU_HARD_NBI0_BIT       2
#define ICU_HARD_NBI1_BIT       3
#define ICU_HARD_NBI2_BIT       4
#define ICU_HARD_NBI3_BIT       5
#define ICU_DESTSEL_PAR_BIT     6
#define ICU_NONFATAL_BIT        7       /* unused, above non-fatal */
#define ICU_SLAVE_BIT           8
#define ICU_SOFT_NBI0_BIT       9
#define ICU_NORMAL_BIT          10
#define ICU_TIMER_M_BIT         11
#define ICU_VECBUS_BIT          12
#define ICU_BOGUS_M_BIT         15
#define ICU_SOFT_NBI1_BIT       16
#define ICU_SOFT_NBI2_BIT       17
#define ICU_RCV_ADDR_PAR4_BIT   18
#define ICU_RCV_ADDR_PAR0_BIT   19
#define ICU_RCV_ADDR_PAR1_BIT   20
#define ICU_RCV_ADDR_PAR2_BIT   21
#define ICU_RCV_ADDR_PAR3_BIT   22
#define ICU_BTAG_PAR0_BIT       23
#define ICU_BTAG_PAR1_BIT       24
#define ICU_VB_NAK_BIT          25
#define ICU_VB_PAR_BIT          26
#define ICU_VB_OUT_OF_SYNC_BIT  27
#define ICU_CACHE_PAR_BIT       28
#define ICU_DUART_BIT           29
#define ICU_TIMER_S_BIT         30
#define ICU_PROFILE_BIT         ICU_TIMER_S_BIT

#define ICU_POWERFAIL           0x00000001
#define ICU_SYSNMI              0x00000002
#define ICU_HARD_NBI0           0x00000004
#define ICU_HARD_NBI1           0x00000008
#define ICU_HARD_NBI2           0x00000010
#define ICU_HARD_NBI3           0x00000020
#define ICU_DESTSEL_PAR         0x00000040
#define ICU_NONFATAL            0x00000080      /* unused, above non-fatal */
#define ICU_SLAVE               0x00000100
#define ICU_SOFT_NBI0           0x00000200
#define ICU_NORMAL              0x00000400
#define ICU_TIMER_M             0x00000800
#define ICU_VECBUS              0x00001000
#define ICU_BOGUS_M             0x00008000
#define ICU_SOFT_NBI1           0x00010000
#define ICU_SOFT_NBI2           0x00020000
#define ICU_RCV_ADDR_PAR4       0x00040000
#define ICU_RCV_ADDR_PAR0       0x00080000
#define ICU_RCV_ADDR_PAR1       0x00100000
#define ICU_RCV_ADDR_PAR2       0x00200000
#define ICU_RCV_ADDR_PAR3       0x00400000
#define ICU_BTAG_PAR0           0x00800000
#define ICU_BTAG_PAR1           0x01000000
#define ICU_VB_NAK              0x02000000
#define ICU_VB_PAR              0x04000000
#define ICU_VB_OUT_OF_SYNC      0x08000000
#define ICU_CACHE_PAR           0x10000000
#define ICU_DUART               0x20000000
#define ICU_TIMER_S             0x40000000
#define ICU_PROFILE             ICU_TIMER_S

#define ICU_ENABLE_ALL          0x5fff1b7f
#endif	MMAX_APC

/*
 * Vector bias.  (ICU interrupt n vectors to MICUBIAS + n.)
 */
#define MICUBIAS_M      0x10
#if	MMAX_APC
#define MICUBIAS_S      0x20
#endif	MMAX_APC
/*
 * No clock output desired.
 */
#define CWAVE           0

/*
 * Initialize port data.
 */
#define PDATAI          0
#define PRIOR           0

/*
 * ICU register offsets.
 */
#define IHVCT           0x00    /* Hardware vector, byte read by CPU    */
#define ISVCT           0x04    /* Software vector, R/W by software     */
#define IELTG           0x08    /* Edge/level trigger.  Level = 1       */
#define ITPL            0x10    /* Trigger polarity 1: Rising edge/high */
#define IPND            0x18    /* interrupts pending, SW intr on write */
#define ISRV            0x20    /* interrupt in service                 */
#define IMASK           0x28    /* interrupt masked if 1                */
#define ICSRC           0x30    /* cascaded source if 1                 */
#define IFPRT           0x38    /* first priority                       */
#define IMCTL           0x40    /*  Mode control register               */
#define IOCASN          0x44    /*  Output clock assignment             */
#define ICIPTR          0x48    /*  Counter interrupt pointer           */
#define IPDAT           0x4c    /*  Port data                           */
#define IPS             0x50    /*  Interrupt/port select               */
#define IPDIR           0x54    /*  Port direction                      */
#define ICCTL           0x58    /*  Counter control                     */
#define ICICTL          0x5c    /*  Counter interrupt control           */
#define ILCSV           0x60    /*  L-counter starting value            */
#define IHCSV           0x68    /*  H-counter starting value            */
#define ILCCV           0x70    /*  L-counter current value             */
#define IHCCV           0x78    /*  H-counter current value             */

/*  Master and Slave ICU base addresses */
#define M_ICU_BASE      0xfffffe00
#if	MMAX_APC
#define S_ICU_BASE      0xfffffe80
#endif	MMAX_APC

#define RBIAS           0x04    /*  Offset for 2nd half of 16 bit reg.  */
#define CINIT           0x80    /*  Initial count                       */

#define CRUHLSEL        (CCON + CRUNH)    /*  Start concatenated counters running */
#define CBEGINHH        0x00    /*  Start values for 4 bytes clock reg  */
#define CBEGINHL        0x01
#define CBEGINLH        0x00
#define CBEGINLL        0x00
#define C_HUGE_HL       0x7f    /*  HUGE value if reload unused (9 sec) */

#endif	MMAX_XPC || MMAX_APC
