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
 *	@(#)$RCSfile: inline_lock.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:40:59 $
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

#ifndef __MMAX_INLINE_LOCK
#define __MMAX_INLINE_LOCK

#include <mmax_idebug.h>
#include <mmax_xpc.h>
#include <mmax_apc.h>
#include <mmax_dpc.h>

/*
 * Define inline functions to be used with compilers that can benefit
 * from this. For the multimax we define simple lock and spl macros.
 */
/* Use GNU-C's asm capability to generate fast locking macros */
#if	__GNUC__

#if	MMAX_IDEBUG
	/*
	 * When interrupt debugging is enabled, the interrupt macros
	 * become subroutine calls to functions in ../mmax/lo*.s
	 */
#else	/* MMAX_IDEBUG */
#if	MMAX_XPC
#define _splany() \
        ({unsigned s; \
          __asm__ volatile ("movb	@0xfffffe24,%0 \n\
		             sbitb	$4,@0xfffffe24 \n\
                            " : "=r" (s) ); \
           s;})
#define splx(s) \
        ({__asm__ volatile ("movb %0,@0xfffffe24" : /* void */ : "r" (s));})
#define spl0() \
        ({__asm__ volatile ("cbitb	$4,@0xfffffe24 \n\
			     bispsrw	$0x800 \n\
                            ");0;})
#endif	/* MMAX_XPC */
#if	MMAX_APC
#define _splany() \
        ({unsigned s; \
          __asm__ volatile ("movb	@0xfffffe24,%0 \n\
		             sbitb	$2,@0xfffffe24 \n\
                            " : "=r" (s) ); \
           s;})
#define splx(s) \
        ({__asm__ volatile ("movb %0,@0xfffffe24" : /* void */ : "r" (s));})
#define spl0() \
        ({__asm__ volatile ("cbitb	$2,@0xfffffe24 \n\
			     bispsrw	$0x800 \n\
                            ");0;})
#endif	/* MMAX_APC */
#if	MMAX_DPC
#define _splany() \
        ({unsigned s; \
          __asm__ volatile ("sprd	psr,%0\n\
			     andw	$0x800,%0	# clear all but the I bit \n\
			     bicpsrw	%0		# disable interrupts if they were on \n\
                            " : "=r" (s) ); \
           s;})
#define splx(s) \
        ({__asm__ volatile ("bispsrw	%0" : /* void */ : "r" (s));})
#define spl0() \
        ({__asm__ volatile ("bispsrw $0x800");0;})
#endif	/* MMAX_DPC */

/* Common spl code */
#define spl1	_splany
#define spl2	_splany
#define spl3	_splany
#define spl4	_splany
#define spl5	_splany
#define spl6	_splany
#define spl7	_splany
#define spltty	_splany
#define splnet	_splany
#define splimp	_splany
#define splbio	_splany
#define splvm	_splany
#define splhi	_splany
#define splclock	_splany
#define splsoftclock	_splany
#define splhigh	_splany
#define splsched	_splany

#endif	/* MMAX_IDEBUG */

#define __SLOCKS 1

/* Gets the current location for slhack() */
#define current_pc \
        ({int pc; __asm__ volatile ("addr .+0,%0" : "=r" (pc) ); pc;})

/* Definitions for simple locks */
extern __inline__ _simple_lock(l)
        register char *l;
{
        __asm__ volatile ("\n\
                sbitib	$0, 0(%0)\n\
                bfc	.+9\n\
                cmpqb	$0, 0(%0)\n\
                bne	.-3\n\
                br	.-12\n\
        " : /* void */ : "r" (l));
}

extern __inline__ _simple_lock_try(l)
        register char *l;
{
        register int ret;
        __asm__ volatile ("\n\
		sbitib	$0, 0(%1)	# try for lock\n\
		sfcd	%0		# F set -> didn't get it\n\
	" : "=r" (ret) : "r" (l));
        return(ret);
}

extern __inline__ _simple_unlock(l)
        register char *l;
{
        __asm__ volatile ("movqb $0, 0(%0)" : /* void */ : "r" (l));
}

#define _simple_lock_init _simple_unlock

#define SIMPLE_LOCK_DEBUG(l) \
        slhack(l, current_pc)

#define SIMPLE_UNLOCK_DEBUG(l) \
        sunhack(l, current_pc);

#endif	/* __GNUC__  */

#endif	/* __MMAX_INLINE_LOCK */
