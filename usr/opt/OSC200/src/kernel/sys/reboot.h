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
 *	@(#)$RCSfile: reboot.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:59:36 $
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

/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

#ifndef	_SYS_REBOOT_H_
#define _SYS_REBOOT_H_

#ifdef	KERNEL
#include <mach_kdb.h>
#endif	/* KERNEL */

/*
 * Arguments to reboot system call.
 * These are passed to boot program in r11,
 * and on to init.
 */

#define RB_AUTOBOOT	0	/* flags for system auto-booting itself */

#define RB_ASKNAME	0x01	/* ask for file name to reboot from */
#define RB_SINGLE	0x02	/* reboot to single user only */
#define RB_NOSYNC	0x04	/* dont sync before reboot */
#define RB_KDB		0x04	/* load kernel debugger */
#define RB_HALT		0x08	/* don't reboot, just halt */
#define RB_INITNAME	0x10	/* name given for /etc/init */
#define RB_DFLTROOT	0x20	/* use compiled-in rootdev */
#define RB_ALTBOOT	0x40	/* use /boot.old vs /boot */
#define RB_UNIPROC	0x80	/* don't start slaves */
#define RB_PANIC	0	/* reboot due to panic */
#define RB_BOOT		1	/* reboot due to boot() */

/*
 * Constants for converting boot-style device number to type,
 * adaptor (uba, mba, etc), unit number and partition number.
 * Type (== major device number) is in the low byte
 * for backward compatibility.  Except for that of the "magic
 * number", each mask applies to the shifted value.
 */
#define B_ADAPTORSHIFT	24
#define B_ADAPTORMASK	0x0f
#define B_UNITSHIFT	16
#define B_UNITMASK	0xff
#define B_PARTITIONSHIFT 8
#define B_PARTITIONMASK	0xff
#define B_TYPESHIFT	0
#define B_TYPEMASK	0xff
#define B_MAGICMASK	0xf0000000
#define B_DEVMAGIC	0xa0000000

#ifdef	ibmrt
#define RB_SUSPEND	0x40	/* (6152) suspend unix */
#endif	/* ibmrt */
#if	defined(sun3) || defined(sun4)
#define RB_NOBOOTRC	0x20	/* don't run '/etc/rc.boot' */
#endif	/* defined(sun3) || defined(sun4) */
#if	multimax
/* Additional boot flags on multimax, plus bit defs for standard flags
	Note that multimax uses a different bit for debugger. */

#define RB_B_ASKNAME	0	/* Ask for file name to reboot from */
#define RB_B_SINGLE	1	/* Reboot to single user only */
#define RB_B_NOSYNC	2	/* Don't sync before reboot */
#define RB_B_HALT	3	/* Don't reboot, just halt */
#define RB_B_INITNAME	4	/* Name given for /etc/init */

#define RB_B_PROFILED	28	/* OS Profiling */
#define RB_PROFILED	(1 << RB_B_PROFILED)
#define RB_B_MULTICPU	29	/* Multiprocessor boot */
#define RB_MULTICPU	(1 << RB_B_MULTICPU)
#define RB_B_INTERACT	30	/* Interactive boot */
#define RB_INTERACT	(1 << RB_B_INTERACT)
#define RB_B_DEBUG	31	/* Debug mode boot */
#define RB_DEBUG	(1 << RB_B_DEBUG)
#endif	/* multimax */

#if	balance
/*
 * Sequent specific "reboot" flags.
 */
#define RB_NO_CTRL	0x20	/* for FIRMWARE, don't start controller */
#define RB_NO_INIT	0x40	/* for FIRMWARE, don't init system */
#define RB_AUXBOOT	0x80	/* Boot auxiliary boot name */
#define RB_DUMP		RB_AUXBOOT
#define RB_CONFIG	0x100	/* for FIRMWARE, only build cfg table */
#endif	/* balance */

#if	defined(__hp_osf)
#define RB_TTY		0x100	/* use tty port instead of display */
#define RB_LOADONLY	0x200	/* load the image, do not execute */
#endif

#endif	/* _SYS_REBOOT_H_ */
