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
 *	@(#)$RCSfile: pcb.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:19:21 $
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
 *    Copyright 1988  Intel Corporation.
 */

#ifndef	_PCB_
#define _PCB_

#include <sys/types.h>
#include <i386/fpreg.h>

/* Flags Register */

typedef struct tss386_flags {
	u_int	fl_cf	:  1,		/* carry/borrow */
			:  1,		/* reserved */
		fl_pf	:  1,		/* parity */
			:  1,		/* reserved */
		fl_af	:  1,		/* carry/borrow */
			:  1,		/* reserved */
		fl_zf	:  1,		/* zero */
		fl_sf	:  1,		/* sign */
		fl_tf	:  1,		/* trace */
		fl_if	:  1,		/* interrupt enable */
		fl_df	:  1,		/* direction */
		fl_of	:  1,		/* overflow */
		fl_iopl :  2,		/* I/O privilege level */
		fl_nt	:  1,		/* nested task */
			:  1,		/* reserved */
		fl_rf	:  1,		/* reset */
		fl_vm	:  1,		/* virtual 86 mode */
		fl_res	: 14;		/* reserved */
} tss386_flags_t;

#define PS_C		0x0001		/* carry bit			*/
#define PS_P		0x0004		/* parity bit			*/
#define PS_AC		0x0010		/* auxiliary carry bit		*/
#define PS_Z		0x0040		/* zero bit			*/
#define PS_N		0x0080		/* negative bit			*/
#define PS_T		0x0100		/* trace enable bit		*/
#define PS_IE		0x0200		/* interrupt enable bit		*/
#define PS_D		0x0400		/* direction bit		*/
#define PS_V		0x0800		/* overflow bit			*/
#define PS_IOPL		0x3000		/* I/O privilege level		*/
#define PS_NT		0x4000		/* nested task flag		*/
#define PS_RF		0x10000		/* Reset flag			*/
#define PS_VM		0x20000		/* Virtual 86 mode flag		*/

/*
 * Maximum I/O address that will be in TSS bitmap
 */
#define MAXTSSIOADDR	0x3ff

struct tss386 {
	u_long t_link;
	u_long t_esp0;
	u_long t_ss0;
	u_long t_esp1;
	u_long t_ss1;
	u_long t_esp2;
	u_long t_ss2;
	u_long t_cr3;
	u_long t_eip;
	u_long t_eflags;
	u_long t_eax;
	u_long t_ecx;
	u_long t_edx;
	u_long t_ebx;
	u_long t_esp;
	u_long t_ebp;
	u_long t_esi;
	u_long t_edi;
	u_long t_es;
	u_long t_cs;
	u_long t_ss;
	u_long t_ds;
	u_long t_fs;
	u_long t_gs;
	u_long t_ldt;
	u_short t_t;
	u_short t_bitmapbase;
};

/*
 * 386 TSS definition
 */

struct pcb {

	struct tss386 pcb_tss;

	/* 
	 * Software extensions.  Note that the fpvalid and fps fields 
	 * are assumed to be adjacent (see ldt_init()).
	 * XXX - It would[ be better if fpstate.state were a struct 
	 *       instead of a byte array, but I don't know anything 
	 *       about the '287 layout. -mdk
	 */
	struct pt_entry *pcb_cr3;	/* page directory pointer - CR3 */
	int	pcb_context[7];		/* save context for kernel */
	int	pcb_fpvalid;		/* saved fp state is valid */
	struct	fpstate {
		u_char	state[FP_STATE_BYTES];	
					/* 287/387 saved state; see fpreg.h */
		int	status;		/* status word at exception */
	} pcb_fps;
	int	pcb_flags;
	/* 
	 * Start of COPROC_SPANPAGE_BUG fields.  These aren't ifdef'd, 
	 * because user programs have no way of knowing whether the 
	 * kernel was config'd for that fix.
	 */
	int	pcb_last_uip;		/* prev. user IP at interrupt */
	int	pcb_ip_same;		/* number times IP was the same */
	/* End of COPROC_SPANPAGE_BUG fields. */
};


#define		PS_USER		3
#define		PS_KERNEL	0


/* 
 * Flags.
 * 
 * PSF_SINGLESTEP can be set if there is a debug trap in system mode.
 * The flag tells syscall() to set the single-step flag in the user's
 * flags register, so that the user will be put back into single-step
 * mode after returning from the syscall.
 */

#define PSF_SINGLESTEP	0x1


/* 
 * Public functions exported by pcb.c.
 */

extern void pcb_synch();

#if	defined(__STDC__) || defined(__GNUC__)
extern void pcb_init(/*struct thread *thread, vm_offset_t ksp*/);
extern void ldt_init(/*struct seg_desc *ldt, char *fpstart, int fpsize*/);
#endif	/* __STDC__ || __GNUC__ */

#endif	/* _PCB_ */
