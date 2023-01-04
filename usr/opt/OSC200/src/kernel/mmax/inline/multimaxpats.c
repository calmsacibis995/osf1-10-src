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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: multimaxpats.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:39:11 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
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
 *	WARNING: The multimax assembler does not understand local labels.
 *		As a result inline has been hacked up to support them.
 *		The labels 0$ thru 15$ are the only labels that can be
 *		used in inline macros; inline will generate unique labels
 *		and do the replacements.  If you need more labels,
 *		you are probably trying to inline something that's too
 *		complex.  If you still need them, you'll have to modify the
 *		inline program itself.
 */

/*
 * TODO:	If usage ever warrants
 *	userio: {f,s}{i,}{byte,word}
 */

#include <inline/inline.h>
#include <mmax_xpc.h>
#include <mmax_apc.h>
#include <mmax_dpc.h>
#include <mach_ldebug.h>
#include <lock_stats.h>
#include <mmax_idebug.h>


struct pats ns32000_ptab[] = {

#if	MMAX_XPC || MMAX_APC
	{1, "_mtpr_MxPR_IVAR0\n",
"	lmr	ivar0, r0\n"},
	{1, "_mtpr_MxPR_IVAR1\n",
"	lmr	ivar1, r0\n"},
	{1, "_mtpr_MxPR_PTB0\n",
"	lmr	ptb0, r0\n"},
	{1, "_mtpr_MxPR_PTB1\n",
"	lmr	ptb1, r0\n"},

	{0, "_mfpr_MxPR_IVAR0\n",
"	smr	ivar0, r0\n"},
	{0, "_mfpr_MxPR_IVAR1\n",
"	smr	ivar1, r0\n"},
	{0, "_mfpr_MxPR_PTB0\n",
"	smr	ptb0, r0\n"},
	{0, "_mfpr_MxPR_PTB1\n",
"	smr	ptb1, r0\n"},
#endif

#if	MMAX_DPC
	{1, "_mtpr_MxPR_EIA\n",
"	lmr	eia, r0\n"},
	{1, "_mtpr_MxPR_PTB0\n",
"	lmr	ptb0, r0\n"},
	{1, "_mtpr_MxPR_PTB1\n",
"	lmr	ptb1, r0\n"},

	{0, "_mfpr_MxPR_EIA\n",
"	smr	eia, r0\n"},
	{0, "_mfpr_MxPR_PTB0\n",
"	smr	ptb0, r0\n"},
	{0, "_mfpr_MxPR_PTB1\n",
"	smr	ptb1, r0\n"},
#endif

	{1, "_mtpr_MxPR_FSR\n",
"	lfsr	r0\n"},
	{1, "_mtpr_MxPR_F0\n",
"	movd	r0,tos\n\
	movf	tos,f0\n"},
	{1, "_mtpr_MxPR_F1\n",
"	movd	r0,tos\n\
	movf	tos,f1\n"},
	{1, "_mtpr_MxPR_F2\n",
"	movd	r0,tos\n\
	movf	tos,f2\n"},
	{1, "_mtpr_MxPR_F3\n",
"	movd	r0,tos\n\
	movf	tos,f3\n"},
	{1, "_mtpr_MxPR_F4\n",
"	movd	r0,tos\n\
	movf	tos,f4\n"},
	{1, "_mtpr_MxPR_F5\n",
"	movd	r0,tos\n\
	movf	tos,f5\n"},
	{1, "_mtpr_MxPR_F6\n",
"	movd	r0,tos\n\
	movf	tos,f6\n"},
	{1, "_mtpr_MxPR_F7\n",
"	movd	r0,tos\n\
	movf	tos,f7\n"},

	{0, "_mfpr_MxPR_FSR\n",
"	sfsr	r0\n"},
	{0, "_mfpr_MxPR_F0\n",
"	movf	f0,tos\n\
	movd	tos,r0\n"},
	{0, "_mfpr_MxPR_F1\n",
"	movf	f1,tos\n\
	movd	tos,r0\n"},
	{0, "_mfpr_MxPR_F2\n",
"	movf	f2,tos\n\
	movd	tos,r0\n"},
	{0, "_mfpr_MxPR_F3\n",
"	movf	f3,tos\n\
	movd	tos,r0\n"},
	{0, "_mfpr_MxPR_F4\n",
"	movf	f4,tos\n\
	movd	tos,r0\n"},
	{0, "_mfpr_MxPR_F5\n",
"	movf	f5,tos\n\
	movd	tos,r0\n"},
	{0, "_mfpr_MxPR_F6\n",
"	movf	f6,tos\n\
	movd	tos,r0\n"},
	{0, "_mfpr_MxPR_F7\n",
"	movf	f7,tos\n\
	movd	tos,r0\n"},
#ifdef fix_for_gcc
	{3, "_bcmp\n",
"	movd	r0,r2\n\
	movd	tos,r0\n\
	cmpsb\n"},
#endif
	{1, "_ffs\n",
"	movqd	$0,r1	# start at beginning \n\
	ffsd	r0,r1\n\
	bfs	0$	# return 0 if not found \n\
	addqd	$1,r1	# correct bit position \n\
0$:	movd	r1,r0\n"},

#if	MMAX_APC || MMAX_XPC
	{2, "_pmap_pte\n",
"	movd	r1,r2		# save copy of address\n\
	movd	4(r0),r0	# address of pte1 entries\n\
	lshd	$-22,r1		# extract level 1 index\n\
	addr	0(r0)[r1:d],r0	# compute pte address\n\
	tbitb	$0,0(r0)	# pte valid ?\n\
	bfs	0$		# branch if yes\n\
	movqd	$0,r0		# return 0\n\
	br	1$\n\
0$:\n\
	movd	4096(r0),r0	# base of second level pte table\n\
	andd	$0x3ff000,r2	# mask second level index\n\
	lshd	$-10,r2		# convert to offset\n\
	addd	r2,r0		# and add to base to get address\n\
1$:\n"},
#else
	- + = NEED DPC VERSION - + =
#endif

	{ 0, "", ""}
};


/*
 *	spl macros.
 *
 *	WARNING: splx assumes the value it is passed came from splany
 *		(i.e. 0 or 0x800 on DPC, or ICU value on APC/XPC).  Use of
 *              any other values may cause bizarre failures (e.g., funky
 *              bits may get or'd into psw or set into ICU).
 *
 */

#if	MMAX_XPC
#define	splany	"	movb	@0xfffffe24,r0 \n\
        sbitb	$4,@0xfffffe24 \n"
#define splx    "	movb	r0,@0xfffffe24 \n"
#define spl0    "	cbitb	$4,@0xfffffe24 \n\
	bispsrw	$0x800 \n"
#endif	MMAX_XPC

#if	MMAX_APC
#define	splany	"	movb	@0xfffffe24,r0 \n\
        sbitb	$2,@0xfffffe24 \n"
#define splx    "	movb	r0,@0xfffffe24 \n"
#define spl0    "	cbitb	$2,@0xfffffe24 \n\
	bispsrw	$0x800 \n"
#endif	MMAX_APC

#if	MMAX_DPC
#define	splany	"	sprd	psr,r0\n\
	andw	$0x800,r0	# clear all but the I bit \n\
	bicpsrw	r0		# disable interrupts if they were on \n"
#define splx	"	bispsrw	r0\n"
#define spl0	"	bispsrw $0x800\n"
#endif	MMAX_DPC

struct pats mmax_ptab[] = {

#if	MMAX_IDEBUG
	/*
	 * When interrupt debugging is enabled, the interrupt macros
	 * become subroutine calls to functions in ../mmax/lo*.s
	 */
#else	/* MMAX_IDEBUG */
	{0, "_spl0\n", spl0},
	{0, "_spl1\n", splany},
	{0, "_spl2\n", splany},
	{0, "_spl3\n", splany},
	{0, "_spl4\n", splany},
	{0, "_spl5\n", splany},
	{0, "_spl6\n", splany},
	{0, "_spl7\n", splany},
	{0, "_splnet\n", splany},
	{0, "_spltty\n", splany},
	{0, "_splimp\n", splany},
	{0, "_splbio\n", splany},
	{0, "_splvm\n", splany},
	{0, "_splhi\n", splany},
	{0, "_splclock\n", splany},
	{0, "_splsoftclock\n", splany},
	{0, "_splhigh\n", splany},
	{0, "_splsched\n", splany},

	{1, "_splx\n", splx},
#endif	/* MMAX_IDEBUG */

	{ 0, "", ""}
};

struct pats unix_ptab[] = {

	{1, "_htonl\n",
"	rotw	$8, r0		# 3!2!0!1\n\
	rotd	$16, r0		# 0!1!3!2\n\
	rotw	$8, r0		# 0!1!2!3\n"},

	{1, "_htons\n",
"	movzwd	r0, r0		# zero!zero!1!0\n\
	rotw	$8, r0		# zero!zero!0!1\n"},

	{1, "_ntohl\n",
"	rotw	$8, r0		# 3!2!0!1\n\
	rotd	$16, r0		# 0!1!3!2\n\
	rotw	$8, r0		# 0!1!2!3\n"},

	{1, "_ntohs\n",
"	movzwd	r0, r0		# zero!zero!1!0\n\
	rotw	$8, r0		# zero!zero!0!1\n"},

	{ 0, "", ""}
};

struct pats mach_ptab[] = {

#if	MACH_LDEBUG || LOCK_STATS
	/*
	 * When compiled for debugging or statistics, read/write
	 * locks are handled exclusively by the functions in ../kern/lock.c.
	 */
#else
	/*
	 * Inlined read/write lock functions for speed.
	 */
#if	MMAX_XPC
	{1, "_lock_write\n",
"0$:	sbitib	$0, 0(r0)		# lock the interlock \n\
	bfc	2$			\n\
1$:	cmpqb	$0, 0(r0)		# spin in cache until free \n\
	bne	1$			\n\
	br	0$			\n\
2$:	movd	4(r0), r1		# if (no readers, no upgrade, no writer)\n\
	bicd	$0xfffc0000, r1		\n\
	cmpqd	$0, r1			\n\
	bne	3$			\n\
	sbitd	$17, 4(r0)		# put (one writer)\n\
        movzbd	@0xffffff1c,r1		# get our cpu id\n\
	andd	$0x3d, r1		# mask off right bits \n\
        movd	@_active_threads[r1:d],16(r0) # store current thread\n\
        jsr	_getpcr1		# get pc into r1 \n\
        movd	r1,20(r0)		# save pc of last locker\n\
	movqb	$0, 0(r0)		# drop interlock and continue \n\
	br	4$			\n\
3$:	movqb	$0, 0(r0)		# clear lock and\n\
	jsr	@_lock_write		# give up\n\
4$:\n"	},
#endif	MMAX_XPC
#if	MMAX_APC
	{1, "_lock_write\n",
"0$:	sbitib	$0, 0(r0)		# lock the interlock \n\
	bfc	2$			\n\
1$:	cmpqb	$0, 0(r0)		# spin in cache until free \n\
	bne	1$			\n\
	br	0$			\n\
2$:	movd	4(r0), r1		# if (no readers, no upgrade, no writer)\n\
	bicd	$0xfffc0000, r1		\n\
	cmpqd	$0, r1			\n\
	bne	3$			\n\
	sbitd	$17, 4(r0)		# put (one writer)\n\
        movzbd	@0xffffff54,r1		# get our cpu id\n\
        movd	@_active_threads[r1:d],16(r0) # store current thread\n\
        jsr	_getpcr1		# get pc into r1 \n\
        movd	r1,20(r0)		# save pc of last locker\n\
	movqb	$0, 0(r0)		# drop interlock and continue \n\
	br	4$			\n\
3$:	movqb	$0, 0(r0)		# clear lock and\n\
	jsr	@_lock_write		# give up\n\
4$:\n"	},
#endif	MMAX_APC

	{1, "_lock_read\n",
"0$:	sbitib	$0, 0(r0)		# lock the interlock \n\
	bfc	2$			\n\
1$:	cmpqb	$0, 0(r0)		# spin in cache until free \n\
	bne	1$			\n\
	br	0$			\n\
2$:	movd	4(r0), r1		# if (no upgrade, no writer)\n\
	bicd	$0xfffcffff, r1		\n\
	cmpqd	$0, r1			\n\
	bne	3$			\n\
	addqw	$1, 4(r0)		# add a reader\n\
	movqb	$0, 0(r0)		# drop interlock and continue\n\
	br	4$			\n\
3$:	movqb	$0, 0(r0)		# clear lock and\n\
	jsr	@_lock_read		# give up\n\
4$:\n"},

	{1, "_lock_done\n", 
"0$:	sbitib	$0, 0(r0)		# lock the interlock \n\
	bfc	2$			\n\
1$:	cmpqb	$0, 0(r0)		# spin in cache until free \n\
	bne	1$			\n\
	br	0$			\n\
2$:	ord	$0x80000000,20(r0)	# Note not now locked \n\
        jsr	_getpcr1		# get pc \n\
        movd	r1,24(r0)		# save pc of last unlocker \n\
	cmpqw	$0, 4(r0)		# readers ?\n\
	beq	3$			\n\
	addqw	$-1, 4(r0)		# one less reader\n\
	br	5$			# waiters ?\n\
3$:	cmpqd	$0, 12(r0)		# recursive\n\
	beq	4$			\n\
	addqd	$-1, 12(r0)		# decr recursion\n\
	br	6$			# out\n\
4$:	cbitd	$16, 4(r0)		# clear upgrade\n\
	bfs	5$			# take branch if set\n\
	cbitd	$17, 4(r0)		# clear writer\n\
5$:	cbitd	$18, 4(r0)		# anyone waiting?\n\
	bfc	6$			\n\
	movd	r0, tos			# save and arg in r0\n\
	jsr	@_thread_wakeup		\n\
	movd	tos, r0			# restore\n\
6$:	movqb	$0, 0(r0)\n"},
#endif	/* MACH_LDEBUG || LOCK_STATS */

#if	MACH_LDEBUG
	{1, "_simple_lock_init\n",
"	movqb	$0, 0(r0)\n\
	movd	$-1, 8(r0)\n\
	movd	$-1, 12(r0)\n"},
#else
	{1, "_simple_lock_init\n",
"	movqb	$0, 0(r0)\n"},
#endif

#if	MACH_LDEBUG
	{1, "_simple_lock\n",
"0$:	sbitib	$0, 0(r0)\n\
	bfc	2$\n\
1$:	cmpqb	$0, 0(r0)\n\
	bne	1$\n\
	br	0$\n\
2$:	addr	.+0,r1\n\
	jsr	_slhack\n"},
#else
	{1, "_simple_lock\n",
"0$:	sbitib	$0, 0(r0)\n\
	bfc	2$\n\
1$:	cmpqb	$0, 0(r0)\n\
	bne	1$\n\
	br	0$\n\
2$:\n"},
#endif
	
#if	MACH_LDEBUG
	{1, "_simple_lock_try\n",
"	movd	r0, r1		# save lock address\n\
	sbitib	$0, 0(r0)	# try for lock \n\
	sfcd	r0		# F set -> didn't get it \n\
	bfs	1$\n\
	movd	r0,tos		# Save condition code on stack \n\
	movd	r1,r0		# Move lock addr back to r0 \n\
	addr	.+0,r1		# Current PC to r1  \n\
	jsr	_slhack		# Record info \n\
	movd	tos,r0		# Restore condition code \n\
1$:\n"},
#else
	{1, "_simple_lock_try\n",
"	sbitib	$0, 0(r0)	# try for lock \n\
	sfcd	r0		# F set -> didn't get it \n"},
#endif

#if	MACH_LDEBUG
	{1, "_simple_unlock\n",
"	addr	.+0, r1\n\
	jsr	_sunhack\n\
	movqb	$0, 0(r0)\n"},
#else
	{1, "_simple_unlock\n",
"	movqb	$0, 0(r0)\n"	},
#endif

	{1, "_byte_lock\n",
"0$:	sbitib	$0, 0(r0)\n\
	bfc	2$\n\
1$:	cmpqb	$0, 0(r0)\n\
	bne	1$\n\
	br	0$\n\
2$:\n"},
	
	{1, "_byte_unlock\n",
"	movqb	$0, 0(r0)\n"	},

	{1, "_crq_lock_init\n",
"	movqb	$0, 0(r0)\n"},

	{1, "_crq_lock\n",
"0$:	sbitib	$0, 0(r0)\n\
	bfc	2$\n\
1$:	cmpqb	$0, 0(r0)\n\
	bne	1$\n\
	br	0$\n\
2$:\n"},

	{1, "_crq_unlock\n",
"	movqb	$0, 0(r0)\n"	},

	{2, "_bit_test\n",
"	extd	r0, 0(r1), r0, $1\n"},

	{2, "_bit_set\n",
"	insb	r0, $1, 0(r1), $1\n"},

	{2, "_bit_clear\n",
"	insb	r0, $0, 0(r1), $1\n"},

	{2, "_enqueue_head\n",
"	movd	0(r0),r2	# r2 = que->next\n\
	movd	r2,0(r1)	# elt->next = que->next\n\
	movd	r0,4(r1)	# elt->prev = que\n\
	movd	r1,4(r2)	# {elt,que}->next->prev = elt\n\
	movd	r1,0(r0)	# que->next = elt\n"},

	{2, "_enqueue_tail\n",
"	movd	4(r0),r2	# r2 = que->prev\n\
	movd	r0,0(r1)	# elt->next = que\n\
	movd	r2,4(r1)	# elt->prev = que->prev\n\
	movd	r1,0(r2)	# {elt,que}->prev->next = elt\n\
	movd	r1,4(r0)	# que->prev = elt\n"},

	{1, "_dequeue_head\n",
"	cmpd	r0,0(r0)	# if (que->next == que)\n\
	bne	1$		# \n\
	movqd	$0,r0		# { return(0)} \n\
	br	2$		# \n\
1$:	movd	0(r0),r1	# elt = que->next\n\
	movd	0(r1),r2	# r2 = elt->next\n\
	movd	r0,4(r2)	# elt->next->prev = que\n\
	movd	r2,0(r0)	# que->next = elt->next\n\
	movd	r1,r0		# return(elt)\n\
2$:\n"},

	{1, "_dequeue_tail\n",
"	cmpd	r0,4(r0)	# if (que->prev == que)\n\
	bne	1$		# \n\
	movqd	$0,r0		# {return (0)} \n\
	br	2$		# \n\
1$:	movd	4(r0),r1	# elt = que->prev\n\
	movd	4(r1),r2	# r2 = elt->prev\n\
	movd	r0,0(r2)	# elt->prev->next = que\n\
	movd	r2,4(r0)	# que->prev = elt->prev\n\
	movd	r1,r0		# return(elt)\n\
2$:\n"},

	{2, "_remqueue\n",
"	movd 0(r1),r0		# r0 = elt->next\n\
	movd 4(r1),r2		# r2 = elt->prev\n\
	movd r2,4(r0)		# elt->next->prev = elt->prev\n\
	movd r0,0(r2)		# elt->prev->next = elt->next\n"},

	{2, "_insque\n",
"	movd 0(r1),r2		# r2 = pred->next\n\
	movd r2,0(r0)		# entry->next = pred->next\n\
	movd r1,4(r0)		# entry->prev = pred\n\
	movd r0,4(r2)		# pred->next->prev = entry\n\
	movd r0,0(r1)		# pred->next = entry\n"},

	{1, "_remque\n",
"	movd 0(r0),r1		# r1 = elt->next\n\
	movd 4(r0),r2		# r2 = elt->prev\n\
	movd r2,4(r1)		# elt->next->prev = elt->prev\n\
	movd r1,0(r2)		# elt->prev->next = elt->next\n\
				# return(elt) [r0 is unchanged]\n"},

	{0, "", ""}
};
