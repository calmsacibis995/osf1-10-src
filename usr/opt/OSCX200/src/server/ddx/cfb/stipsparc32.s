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
 * $XConsortium: stipsparc32.s,v 1.1 91/12/19 14:17:39 keith Exp $
 *
 * Copyright 1990 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * SPARC assembly code for optimized text rendering.
 *
 * Other stippling could be done in assembly, but the payoff is
 * not nearly as large.  Mostly because large areas are heavily
 * optimized already.
 */

/* not that I expect to ever see an LSB SPARC, but ... */
#ifdef LITTLE_ENDIAN
# define BitsR		sll
# define BitsL		srl
# define WO(o)		3-o
# define FourBits(dest,bits)	and	bits, 0xf, dest
#else
# define BitsR		srl
# define BitsL		sll
# define WO(o)		o
# define FourBits(dest,bits)	srl	bits, 28, dest
#endif

/*
 * cfb32StippleStack(addr, stipple, value, stride, Count, Shift)
 *               4       5       6      7     16(sp) 20(sp)
 *
 *  Apply successive 32-bit stipples starting at addr, addr+stride, ...
 *
 *  Used for text rendering, but only when no data could be lost
 *  when the stipple is shifted left by Shift bits
 */
/* arguments */
#define addr	%i0
#define stipple	%i1
#define value	%i2
#define stride	%i3
#define count	%i4
#define shift	%i5

/* local variables */
#define atemp	%l0
#define bits	%l1
#define lshift	%l2
#define sbase	%l3
#define stemp	%l4

#define CASE_SIZE	5	/* case blocks are 2^5 bytes each */
#define CASE_MASK	0x1e0	/* first case mask */

#define ForEachLine	LY1
#define NextLine	LY2
#define CaseBegin	LY3
#define ForEachBits	LY4
#define NextBits	LY5

#ifdef TETEXT
#define	_cfb32StippleStack	_cfb32StippleStackTE
#endif

	.seg	"text"
	.proc	16
	.globl	_cfb32StippleStack
_cfb32StippleStack:
	save	%sp,-64,%sp
	sethi	%hi(CaseBegin),sbase		/* load up switch table */
	or	sbase,%lo(CaseBegin),sbase

	mov	4,lshift			/* compute offset within */
	sub	lshift, shift, lshift		/*  stipple of remaining bits */
#ifdef LITTLE_ENDIAN
	inc	CASE_SIZE, shift		/* first shift for LSB */
#else
	inc	28-CASE_SIZE, shift		/* first shift for MSB */
#endif
	/* do ... while (--count > 0); */
ForEachLine:
	ld	[stipple],bits			/* get stipple bits */
	mov	addr,atemp			/* set up for this line */
#ifdef TETEXT
	/* Terminal emulator fonts are expanded and have many 0 rows */
	tst	bits
	bz	NextLine			/* skip out early on 0 */
#endif
	add	addr, stride, addr		/* step for the loop */
	BitsR	bits, shift, stemp		/* get first bits */
	and	stemp, CASE_MASK, stemp		/* compute first jump */
	BitsL	bits, lshift, bits		/* set remaining bits */
	jmp	sbase+stemp			/*  ... */
	tst	bits

ForEachBits:
	inc	16, atemp
ForEachBits1:
	FourBits(stemp, bits)			/* compute jump for */
	sll	stemp, CASE_SIZE, stemp		/*  these four bits */
	BitsL	bits, 4, bits			/* step for remaining bits */
	jmp	sbase+stemp			/* jump */
	tst	bits
CaseBegin:
	bnz,a	ForEachBits1			/* 0 */
	inc	16, atemp
NextLine:
	deccc	1, count
NextLine1:
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
	nop

	bnz	ForEachBits			/* 1 */
	st	value, [atemp+WO(12)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
	nop
					
	bnz	ForEachBits			/* 2 */
	st	value, [atemp+WO(8)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
	nop
					
	st	value, [atemp+WO(8)]		/* 3 */
	bnz	ForEachBits
	st	value, [atemp+WO(12)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
					
	bnz	ForEachBits			/* 4 */
	st	value, [atemp+WO(4)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
	nop
					
	st	value, [atemp+WO(4)]		/* 5 */
	bnz	ForEachBits
	st	value, [atemp+WO(12)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
					
	st	value, [atemp+WO(4)]		/* 6 */
	bnz	ForEachBits
	st	value, [atemp+WO(8)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
					
	st	value, [atemp+WO(4)]		/* 7 */
	st	value, [atemp+WO(8)]
	bnz	ForEachBits
	st	value, [atemp+WO(12)]
	b	NextLine1
	deccc	1, count
	nop
	nop
					
	bnz	ForEachBits			/* 8 */
	st	value, [atemp+WO(0)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
	nop
					
	st	value, [atemp+WO(0)]		/* 9 */
	bnz	ForEachBits
	st	value, [atemp+WO(12)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
					
	st	value, [atemp+WO(0)]		/* a */
	bnz	ForEachBits
	st	value, [atemp+WO(8)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
					
	st	value, [atemp+WO(0)]		/* b */
	st	value, [atemp+WO(8)]
	bnz	ForEachBits
	st	value, [atemp+WO(12)]
	b	NextLine1
	deccc	1, count
	nop
	nop
					
	st	value, [atemp+WO(0)]		/* c */
	bnz	ForEachBits
	st	value, [atemp+WO(4)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
					
	st	value, [atemp+WO(0)]		/* d */
	st	value, [atemp+WO(4)]
	bnz	ForEachBits
	st	value, [atemp+WO(12)]
	b	NextLine1
	deccc	1, count
	nop
	nop
					
	st	value, [atemp+WO(0)]		/* e */
	st	value, [atemp+WO(4)]
	bnz	ForEachBits
	st	value, [atemp+WO(8)]
	b	NextLine1
	deccc	1, count
	nop
	nop
					
	st	value, [atemp+WO(0)]		/* f */
	st	value, [atemp+WO(4)]
	st	value, [atemp+WO(8)]
	bnz	ForEachBits
	st	value, [atemp+WO(12)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
