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
 *	@(#)$RCSfile: mtpr.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:41:58 $
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
 * mtpr.h
 *
 *	This file contains the defined register constants used by the
 *	functions mtpr and mfpr.  The defined registers are all of the 
 *	special processor registers, the MMU registers and the floating
 *	point registers.
 */

#ifndef __MMAX_MTPR_H
#define __MMAX_MTPR_H
/*
 *	NOTE:	mtpr and mfpr are now done by inline; the macros that
 *		make this work are at the bottom of the file.  The register
 *		definitions are preserved here for documentation only.
 */

#if	0

#include <mmax_xpc.h>
#include <mmax_apc.h>
#include <mmax_dpc.h>


/* Processor registers. */

#define MxPR_UPSR	   0x00
#define MxPR_USP	   0x07
#define MxPR_FP		   0x08
#define MxPR_SP		   0x09
#define MxPR_SB		   0x0a
#define MxPR_PSR	   0x0d
#define MxPR_INTBASE	   0x0e
#define MxPR_MOD	   0x0f

#if	MMAX_XPC
#define	MxPR_USP_FAST	   0x0b
#define	MxPR_DCR	   0x01
#define MxPR_BPC	   0x02
#define MxPR_DSR	   0x03
#define MxPR_CAR	   0x04
#define MxPR_CFG	   0x0c
#endif	MMAX_XPC

/* MMU registers. */

#if	MMAX_XPC
#define MxPR_MCR	0x19	    /* Memory Management Control Register */
#define MxPR_MSR	0x1a	    /* Memory Management Status Register */
#define MxPR_TEAR	0x1b	    /* Translation Exception Address Register */
#define MxPR_PTB0	0x1c	    /* Page Table Base 0 (Supervisor) */
#define MxPR_PTB1	0x1d	    /* Page Table Base 1 (User) */
#define MxPR_IVAR0	0x1e	    /* Invalidate Virtual Address -- Write only */
#define MxPR_IVAR1	0x1f	    /* Invalidate Virtual Address -- Write only */
#endif	MMAX_XPC

#if	MMAX_APC
#define MxPR_BAR	   0x10
#define MxPR_BMR	   0x12
#define MxPR_BDR	   0x13
#define MxPR_BEAR	   0x16
#define MxPR_FEW	   0x19
#define MxPR_ASR	   0x1a
#define MxPR_TEAR	   0x1b
#define MxPR_PTB0	   0x1c
#define MxPR_PTB1	   0x1d
#define MxPR_IVAR0	   0x1e
#define MxPR_IVAR1	   0x1f
#endif	MMAX_APC

#if	MMAX_DPC
#define MxPR_BPR0	   0x10
#define MxPR_BPR1	   0x11
#define MxPR_PF0	   0x14
#define MxPR_PF1	   0x15
#define MxPR_SC		   0x18
#define MxPR_MSR	   0x1a
#define MxPR_BCNT	   0x1b
#define MxPR_PTB0	   0x1c
#define MxPR_PTB1	   0x1d
#define MxPR_EIA	   0x1f
#endif	MMAX_DPC

#if	MMAX_XPC
/*
 *  Note:  the floating point registers on the ns32532
 *  are 64 bits wide, twice the size of the floating
 *  point registers on the ns32032 and ns32332.  These registers
 *  don't fit into the mfpr/mtpr model, which assumes that the
 *  data can be passed in r0.
 */
#endif	MMAX_XPC

/* Floating point status register */

#define MxPR_FSR	0x20
#define MxPR_F0		0x21
#define MxPR_F1		0x22
#define MxPR_F2		0x23
#define MxPR_F3		0x24
#define MxPR_F4		0x25
#define MxPR_F5		0x26
#define MxPR_F6		0x27
#define MxPR_F7		0x28

#endif	0

/*
 *	mtpr/mfpr macros.  The register name is added to mtpr or
 *	mfpr.  Inline recognizes the combined name and does the rest.
 */

#if	__GNUC__
#define mtpr(reg,value)		_mtpr_/**/reg(value)
#define mfpr(reg) 		_mfpr_/**/reg()

#if	MMAX_XPC||MMAX_APC
extern __inline__ _mtpr_MxPR_IVAR0(val)
        unsigned val;
{
        __asm__ volatile ("lmr	ivar0, %0\n" : /* void */ : "r" (val));
}
extern __inline__ _mtpr_MxPR_IVAR1(val)
        unsigned val;
{
        __asm__ volatile ("lmr	ivar1, %0\n" : /* void */ : "r" (val));
}
extern __inline__ _mtpr_MxPR_PTB0(val)
        unsigned val;
{
        __asm__ volatile ("lmr	ptb0, %0\n" : /* void */ : "r" (val));
}
extern __inline__ _mtpr_MxPR_PTB1(val)
        unsigned val;
{
        __asm__ volatile ("lmr	ptb1, %0\n" : /* void */ : "r" (val));
}

extern __inline__ _mfpr_MxPR_IVAR0()
{
        unsigned reg;
        __asm__ volatile ("smr	ivar0, %0\n" : "=r" (reg));
        return(reg);
}
extern __inline__ _mfpr_MxPR_IVAR1()
{
        unsigned reg;
        __asm__ volatile ("smr	ivar1, %0\n" : "=r" (reg));
        return(reg);
}
extern __inline__ _mfpr_MxPR_PTB0()
{
        unsigned reg;
        __asm__ volatile ("smr	ptb0, %0\n" : "=r" (reg));
        return(reg);
}
extern __inline__ _mfpr_MxPR_PTB1()
{
        unsigned reg;
        __asm__ volatile ("smr	ptb1, %0\n" : "=r" (reg));
        return(reg);
}
#endif

#if	MMAX_DPC
extern __inline__ _mtpr_MxPR_EIA(val)
        unsigned val;
{
        __asm__ volatile ("lmr	eia, %0\n" : /* void */ : "r" (val));
}
extern __inline__ _mtpr_MxPR_PTB0(val)
        unsigned val;
{
        __asm__ volatile ("lmr	ptb0, %0\n" : /* void */ : "r" (val));
}
extern __inline__ _mtpr_MxPR_PTB1(val)
        unsigned val;
{
        __asm__ volatile ("lmr	ptb1, %0\n" : /* void */ : "r" (val));
}

extern __inline__ _mfpr_MxPR_EIA()
{
        unsigned reg;
        __asm__ volatile ("smr	eia, %0\n" : "=r" (reg));
        return(reg);
}
extern __inline__ _mfpr_MxPR_PTB0()
{
        unsigned reg;
        __asm__ volatile ("smr	ptb0, %0\n" : "=r" (reg));
        return(reg);
}
extern __inline__ _mfpr_MxPR_PTB1()
{
        unsigned reg;
        __asm__ volatile ("smr	ptb1, %0\n" : "=r" (reg));
        return(reg);
}
#endif

extern __inline__ _mtpr_MxPR_FSR(val)
        unsigned val;
{
        __asm__ volatile ("lfsr	%0\n" : /* void */ : "r" (val));
}
extern __inline__ _mtpr_MxPR_F0(val)
        unsigned val;
{
        __asm__ volatile ("	movd	%0,tos\n\
				movf	tos,f0\n"
                          : /* void */ : "r" (val) : "f0");
}
extern __inline__ _mtpr_MxPR_F1(val)
        unsigned val;
{
        __asm__ volatile ("	movd	%0,tos\n\
				movf	tos,f1\n"
                          : /* void */ : "r" (val) : "f1");
}
extern __inline__ _mtpr_MxPR_F2(val)
        unsigned val;
{
        __asm__ volatile ("	movd	%0,tos\n\
				movf	tos,f2\n"
                          : /* void */ : "r" (val) : "f2");
}
extern __inline__ _mtpr_MxPR_F3(val)
        unsigned val;
{
        __asm__ volatile ("	movd	%0,tos\n\
				movf	tos,f3\n"
                          : /* void */ : "r" (val) : "f3");
}
extern __inline__ _mtpr_MxPR_F4(val)
        unsigned val;
{
        __asm__ volatile ("	movd	%0,tos\n\
				movf	tos,f4\n"
                          : /* void */ : "r" (val) : "f4");
}
extern __inline__ _mtpr_MxPR_F5(val)
        unsigned val;
{
        __asm__ volatile ("	movd	%0,tos\n\
				movf	tos,f5\n"
                          : /* void */ : "r" (val) : "f5");
}
extern __inline__ _mtpr_MxPR_F6(val)
        unsigned val;
{
        __asm__ volatile ("	movd	%0,tos\n\
				movf	tos,f6\n"
                          : /* void */ : "r" (val) : "f6");
}
extern __inline__ _mtpr_MxPR_F7(val)
        unsigned val;
{
        __asm__ volatile ("	movd	%0,tos\n\
				movf	tos,f7\n"
                          : /* void */ : "r" (val) : "f7");
}

extern __inline__ _mfpr_MxPR_FSR()
{
        unsigned reg;
        __asm__ volatile ("sfsr	%0\n" : "=r" (reg));
        return(reg);
}
extern __inline__ _mfpr_MxPR_F0()
{
        unsigned reg;
        __asm__ volatile ("	movf	f0,tos\n\
				movd	tos,%0\n"
                          : "=r" (reg));
        return(reg);
}
extern __inline__ _mfpr_MxPR_F1()
{
        unsigned reg;
        __asm__ volatile ("	movf	f1,tos\n\
				movd	tos,%0\n"
                          : "=r" (reg));
        return(reg);
}
extern __inline__ _mfpr_MxPR_F2()
{
        unsigned reg;
        __asm__ volatile ("	movf	f2,tos\n\
				movd	tos,%0\n"
                          : "=r" (reg));
        return(reg);
}
extern __inline__ _mfpr_MxPR_F3()
{
        unsigned reg;
        __asm__ volatile ("	movf	f3,tos\n\
				movd	tos,%0\n"
                          : "=r" (reg));
        return(reg);
}
extern __inline__ _mfpr_MxPR_F4()
{
        unsigned reg;
        __asm__ volatile ("	movf	f4,tos\n\
				movd	tos,%0\n"
                          : "=r" (reg));
        return(reg);
}
extern __inline__ _mfpr_MxPR_F5()
{
        unsigned reg;
        __asm__ volatile ("	movf	f5,tos\n\
				movd	tos,%0\n"
                          : "=r" (reg));
        return(reg);
}
extern __inline__ _mfpr_MxPR_F6()
{
        unsigned reg;
        __asm__ volatile ("	movf	f6,tos\n\
				movd	tos,%0\n"
                          : "=r" (reg));
        return(reg);
}
extern __inline__ _mfpr_MxPR_F7()
{
        unsigned reg;
        __asm__ volatile ("	movf	f7,tos\n\
				movd	tos,%0\n"
                          : "=r" (reg));
        return(reg);
}
#else

#define mtpr(reg,value)		mtpr_/**/reg(value)
#define mfpr(reg) 		mfpr_/**/reg()
#endif	/* __GNUC__ */

#endif	/* __MMAX_MTPR_H */
