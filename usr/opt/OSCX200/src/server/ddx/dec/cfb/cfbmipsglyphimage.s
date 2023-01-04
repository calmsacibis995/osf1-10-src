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
/****************************************************************************
**                                                                          *
**                  COPYRIGHT (c) 1988, 1989, 1990 BY                       *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
****************************************************************************/

/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/server/ddx/dec/cfb/cfbmipsglyphimage.s,v 1.1.2.2 92/01/07 12:49:56 Jim_Ludwig Exp $ */

/* This file can be compiled for several different scenarios.  In all cases we
   are dealing with fixed-width and height glyphs, minimum width 4, maximum
   width 30.  We have the following posibilities:

	define   NGLYPHS   WIDTHGLYPH	possible glyph widths
	-----------------------------------------------------
	GLYPH6	    5	  	1		4..6
	GLYPH8	    4		1		7..8
	GLYPH10     3		2		9..10
	GLYPH16     2		2		11..16
	GLYPH32	    1         3..4     		17..30
*/

#if defined(GLYPH6) || defined(GLYPH8) || defined(GLYPH10) || defined(GLYPH16)
#else
#define GLYPH32
#endif

	.globl	CFBIMAGEGLYPHS

 # void CFBIMAGEGLYPHS(itfgbgmap, pdstBase, widthDst, nglyph, ppci)

	.ent	CFBIMAGEGLYPHS 2
CFBIMAGEGLYPHS:
#ifdef GLYPH6
	framesize = 36
	.mask	0x40ff0000, -4
#define NGLYPHS 5
#endif

#ifdef GLYPH8
	framesize = 32
	.mask	0x00ff0000, -4
#define NGLYPHS 4
#endif

#ifdef GLYPH10
	framesize = 28
	.mask	0x007f0000, -4
#define NGLYPHS 3
#endif

#ifdef GLYPH16
	framesize = 24
	.mask	0x003f0000, -4
#define NGLYPHS 2
#endif

#ifdef GLYPH32
	framesize = 24
	.mask	0x003f0000, -4
#define	NGLYPHS 1
#endif
	.frame	$sp, framesize, $31

/*
 * Register allocation.  Temporary registers t8-t9 ($24-$25) used as much as
 * possible as actual temporaries and limited-span temporaries in order to keep
 * temporaries t0-t7 ($8-$15) for variables.
 */

/* Scratch temporaries */
#define glyphbits	$2
#define pdst		$3

/* Parameters. */
#define itfgbgmap	$4
#define pdstBase	$5
#define widthDst	$6
#define nglyph		$7

/* Scratch temporaries */
#define pglyph0		$8
#define pci		$9
#define leftbits	$10
#define rightbits	$11
#define p		$12
#define ppci		$13
#define ww		$14
#define pdstEnd		$15

/* Must save if used registers */
#define width		$16
#define shiftprev	$17
#define pglyphprev	$18
#define tmp		$19

#ifdef GLYPH32
#define widthGlyph	$20
#define glyphMask	$21
#define widthn		width
#else
#define widthn		$20
#define pglyph1		$21
#endif

#if defined(GLYPH6) || defined(GLYPH8) || defined(GLYPH10)
#define pglyph2		$22
#endif
#if defined(GLYPH6) || defined(GLYPH8)
#define pglyph3		$23
#endif
#if defined(GLYPH6)
#define pglyph4		$30
#endif


/* Record offset definitions. */
#define leftSideBearing	 0
#define rightSideBearing 2
#define characterWidth	 4
#define ascent		 6
#define descent		 8
#define glyphBitmap	12

#define andbits		 0		/* Not used */
#define xorbits		 4

/* Load up arguments currently in memory */
	lw	ppci, 16($sp)

 #  while(nglyph != 0) {
	beq	nglyph, 0, $9999

/* Save callee's registers */
	.noalias	$sp, ppci
	.noalias	$sp, pci
	subu	$sp, framesize
	sw	$16, 0($sp)
	sw	$17, 4($sp)
	sw	$18, 8($sp)
	sw	$19, 12($sp)
	sw	$20, 16($sp)
	sw	$21, 20($sp)

#if defined(GLYPH6) || defined(GLYPH8) || defined(GLYPH10)
	sw	$22, 24($sp)
#endif
#if defined(GLYPH6) || defined(GLYPH8)
	sw	$23, 28($sp)
#endif
#if defined(GLYPH6)
	sw	$30, 32($sp)
#endif

/* Compute constant height and width for all characters */
	lw	pci, 0(ppci)			#  pci = *ppci;
	lh	$24, ascent(pci)
	lh	$25, descent(pci)
/* Actually, rather than compute height, we always keep pdstEnd defined so
   that loops terminate when pdst == pdstEnd */
	addu	pdstEnd, $24, $25
	multu	pdstEnd, widthDst
	lh	width, characterWidth(pci)

#ifdef GLYPH32
	li	glyphMask, 1			# glyphMask = 1's for data
	sll	glyphMask, width
	subu	glyphMask, 1
	addu	widthGlyph, width, 7 		# widthGlyph = (width + 7) >> 3
	sra	widthGlyph, 3
#endif

/****************************************************************
Phase 1: Paint just enough of the first glyph to satisfy:
   (1) pdstBase & 3 == 0	(pdstBase is word-aligned)
   (2) 0..3 bits left to paint in each row of glyph
*****************************************************************/
 
	lw	pglyph0, glyphBitmap(pci)	# pglyph0 = pci->picture;
	move	pdst, pdstBase			# pdst = pdstBase;

/* Compute leftbits in [1..4], rightbits in [0..3], ww in [0..28] mod 4 */
	and	leftbits, pdstBase, 3		# leftbits = 4 - (pdstBase & 3)
	negu	leftbits, leftbits
	addu	leftbits, 4
 	addu	pdstBase, width			# pdstBase += width
 
	and	rightbits, pdstBase, 3		# rightbits = pdstBase & 3
	subu	pdstBase, rightbits		# word-align pdstBase

	subu	ww, width, leftbits		# ww = # bytes of full words
	subu	ww, rightbits

	mflo	pdstEnd				# Get result of multiply
	addu	pdstEnd, pdst			# pdstEnd = stop pointer

/* Loop over scanlines of glyph */

#if defined(GLYPH6) || defined(GLYPH8)
/* If ww == 0, which it can be for glyphs of width <= 7, we only need to
   paint very leftmost 1-4 pixels.  Do this test outside the loop rather than
   inside. */
	bne	ww, 0, $200
$120:
 #	    do {
/* Get glyph bits for this scanline */
	lbu	glyphbits, 0(pglyph0)		# glyphbits = *pglyph0;
 	addu	pglyph0, 1			# pglyph0 += widthGlyph;

/* Write left-most bits to get destination aligned */
	sll	glyphbits, 3
	and	$24, glyphbits, (15 << 3)
	addu	$24, itfgbgmap
	lw	$24, xorbits($24)
	swr	$24, 0(pdst)
 
/* End while h != 0 */
	addu	pdst, widthDst			# pdst += widthDst;
	bne	pdst, pdstEnd, $120
	b	$900
#endif GLYPH6 || GLYPH8

$200:
 #	    do {
/* Get glyph bits for this scanline */
#if defined(GLYPH6) || defined(GLYPH8)
	lbu	glyphbits, 0(pglyph0)		# glyphbits = *pglyph0;
 	addu	pglyph0, 1			# pglyph0 += widthGlyph;
#endif
#if defined(GLYPH10) || defined(GLYPH16)
	lhu	glyphbits, 0(pglyph0)		# glyphbits = *pglyph0;
 	addu	pglyph0, 2			# pglyph0 += widthGlyph;
#endif
#ifdef GLYPH32
	ulw	glyphbits, 0(pglyph0)		# glyphbits = *((int *) pglyph0)
	addu	pglyph0, widthGlyph		# pglyph0 += widthGlyph;
#endif

/* Offset glyphbits left 3 bits to make indexing itfgbg faster */
	sll	glyphbits, 3

/* Write left-most bits to get destination aligned */
	and	$24, glyphbits, (15 << 3)
	addu	$24, itfgbgmap
	lw	$24, xorbits($24)
	swr	$24, 0(pdst)
 
/* Paint remaining glyphbits in whole words.  */
/* ||| Could do branch to store words, like we do in ``loops'' below.
   Only really want to do branch for GLYPH10 or GLYPH16, as small guys
   don't loop. */
	srl	glyphbits, leftbits		# glyphbits >>= leftbits
	addu	p, pdst, leftbits		# p = pdst + leftbits;
#if defined(GLYPH6) || defined (GLYPH8)
/* Width is at most 6 or 8, so we execute the ``loop'' at most once. */
#else
	move	tmp, ww				# w = ww;
#endif

$300:
/* Write 4 bits worth of data */
	and	$24, glyphbits, (15 << 3)
	addu	$24, itfgbgmap
	lw	$24, xorbits($24)
	sw	$24, 0(p)
#if defined(GLYPH6) || defined (GLYPH8)
/* Width is at most 6 or 8, so we execute the ``loop'' at most once. */
#else
	srl	glyphbits, 4
	addu	p, 4
	subu	tmp, 4
	bne	tmp, 0, $300
#endif

$400:
/* End while h != 0 */
	addu	pdst, widthDst			# pdst += widthDst;
	bne	pdst, pdstEnd, $200

$900:
/* Move pdstEnd in sync with pdstBase */
	addu	pdstEnd, width
	subu	pdstEnd, rightbits

/****************************************************************
Phase 2: Paint groups of NGLYPHS at a time, plus 0..3 bits from
   the previous glyph.  Again, leave loop with conditions:
   (1) pdstBase & 3 == 0	(pdstBase is word-aligned)
   (2) 0..3 bits left to paint in each row of last glyph painted
*****************************************************************/
 
	subu	nglyph, (NGLYPHS+1)		# count glyph done above
	blt	nglyph, 0, $2000

#ifdef GLYPH6
	sll	widthn, width, 2		# widthn = 5 * width
	addu	widthn, width
#endif
#ifdef GLYPH8
	sll	widthn, width, 2		# widthn = 4 * width
#endif
#ifdef GLYPH10
	sll	widthn, width, 1		# widthn = 3 * width
	addu	widthn, width
#endif
#ifdef GLYPH16
	sll	widthn, width, 1		# widthn = 2 * width
#endif	

#ifdef GLYPH8
/* In the case of painting 4 glyphs, all of the same width, we get some nice
   invariants through the loop.  In particular, widthn mod 4 = 0, and so 
   leftbits, rightbits, ww, mywidthdst, and the computed jump address $25
   are invariant.  We just compute these outside the loop. */

	move	leftbits, rightbits		# leftbits = 4 - (pdstBase & 3)
	and	rightbits, leftbits, 3		# rightbits = pdstBase & 3
	move	ww, widthn
	subu	shiftprev, width, leftbits

/* Set up computed jump address $25 to sequential full word stores,
   and computed jump address tmp to sequential full word bg stores. */
	la	$25, $1000
	addu	$25, ww
	lw	$25, 0($25)
	la	tmp, $1100
	subu	tmp, ww
#endif GLYPH8

$1032:
/* main loop on glyphs NGLYPHS at a time */
	lw	pglyphprev, glyphBitmap(pci)	# pglyphprev = pci->picture;

	lw	pci, 4(ppci)			# pci = ppci[1];
	lw	pglyph0, glyphBitmap(pci)	# pglyph0 = pci->picture;
 
#if defined(GLYPH6) || defined(GLYPH8) || defined(GLYPH10) || defined(GLYPH16)
	lw	pci, 8(ppci)			# pci = ppci[2];
	lw	pglyph1, glyphBitmap(pci)	# pglyph1 = pci->picture;
#endif
#if defined(GLYPH6) || defined(GLYPH8) || defined(GLYPH10)
	lw	pci, 12(ppci)			# pci = ppci[3];
	lw	pglyph2, glyphBitmap(pci)	# pglyph2 = pci->picture;
#endif
#if defined(GLYPH6) || defined(GLYPH8)
	lw	pci, 16(ppci)			# pci = ppci[4];
	lw	pglyph3, glyphBitmap(pci)	# pglyph3 = pci->picture;
#endif
#if defined(GLYPH6)
	lw	pci, 20(ppci)			# pci = ppci[5];
	lw	pglyph4, glyphBitmap(pci)	# pglyph4 = pci->picture;
#endif

#ifndef GLYPH8
/* New leftbits is old rightbits, compute new rightbits */
	move	leftbits, rightbits		# leftbits = rightbits;
	addu	ww, widthn, leftbits		# ww = # bytes of full words
	and	rightbits, ww, 3		# new # bits left over
	subu	ww, rightbits
	subu	shiftprev, width, leftbits

/* Set up computed jump address $25 to sequential full word stores,
   and computed jump address tmp to sequential full word bg stores. */
	la	$25, $1000
	addu	$25, ww
	lw	$25, 0($25)
	la	tmp, $1100
	subu	tmp, ww
#endif

	move	pdst, pdstBase			# pdst = pdstBase;

/* Loop over scanlines of glyph */
$1033:
 #	    do {
/* Load up n glyphs for this scanline, bump pointers to next glyph row */
#if defined(GLYPH6)
	lbu	glyphbits, 0(pglyph4)
	addu	pglyph4, 1
	lbu	$24, 0(pglyph3)
	addu	pglyph3, 1
	sll	glyphbits, width
	or	glyphbits, $24
	lbu	$24, 0(pglyph2)
	addu	pglyph2, 1
	sll	glyphbits, width
	or	glyphbits, $24
	lbu	$24, 0(pglyph1)
	addu	pglyph1, 1
	sll	glyphbits, width
	or	glyphbits, $24
	lbu	$24, 0(pglyph0)
	addu	pglyph0, 1
	sll	glyphbits, width
	or	glyphbits, $24
	lbu	$24, 0(pglyphprev)
	addu	pglyphprev, 1
#endif
#if defined(GLYPH8)
	lbu	glyphbits, 0(pglyph3)
	addu	pglyph3, 1
	lbu	$24, 0(pglyph2)
	addu	pglyph2, 1
	sll	glyphbits, width
	or	glyphbits, $24
	lbu	$24, 0(pglyph1)
	addu	pglyph1, 1
	sll	glyphbits, width
	or	glyphbits, $24
	lbu	$24, 0(pglyph0)
	addu	pglyph0, 1
	sll	glyphbits, width
	or	glyphbits, $24
	lbu	$24, 0(pglyphprev)
	addu	pglyphprev, 1
#endif
#if defined(GLYPH10)
	lhu	glyphbits, 0(pglyph2)
	addu	pglyph2, 2
	lhu	$24, 0(pglyph1)
	addu	pglyph1, 2
	sll	glyphbits, width
	or	glyphbits, $24
	lhu	$24, 0(pglyph0)
	addu	pglyph0, 2
	sll	glyphbits, width
	or	glyphbits, $24
	lhu	$24, 0(pglyphprev)
	addu	pglyphprev, 2
#endif
#if defined(GLYPH16)
	lhu	glyphbits, 0(pglyph1)
	addu	pglyph1, 2
	lhu	$24, 0(pglyph0)
	addu	pglyph0, 2
	sll	glyphbits, width
	or	glyphbits, $24
	lhu	$24, 0(pglyphprev)
	addu	pglyphprev, 2
#endif
#if defined(GLYPH32)
	ulw	glyphbits, 0(pglyph0)
	addu	pglyph0, widthGlyph
	ulw	$24, 0(pglyphprev)
	and	$24, glyphMask			# Dump possibly junk high bits
	addu	pglyphprev, widthGlyph
#endif

/* Or in bits from previous glyph */
	srl	$24, shiftprev
	sll	glyphbits, leftbits
	or	glyphbits, $24

/* Is glyph row all 0's? */
	sll	$24, glyphbits, 3		# Assume not all 0's
	bne	glyphbits, 0, $1200

/* *** SET NOREORDER to get rid of all delay slots, and to make computation
   of tmp trivial. */
	.set	noreorder
	lw	$24, xorbits(itfgbgmap)		# Load 32 bits of background
	j	tmp
	addu	p, pdst, ww

$1108:	sw	$24, -32(p)
$1107:	sw	$24, -28(p)
$1106:	sw	$24, -24(p)
$1105:	sw	$24, -20(p)
$1104:	sw	$24, -16(p)
$1103:	sw	$24, -12(p)
$1102:	sw	$24, -8(p)
$1101:	addu	pdst, widthDst			# pdst += widthDst;
/* End while h != 0 */
$1100:	bne	pdst, pdstEnd, $1033
	sw	$24, -4(p)
	.set	reorder
	j 	$1900

$1200:
/* Write first full word */
 #	sll	$24, glyphbits, 3
	and	$24, (15 << 3)
	addu	$24, itfgbgmap
	lw	$24, xorbits($24)
	sw	$24, 0(pdst)
/* Leave glyphbits left offset 3 bits to make fgbgmap indexing faster.
   Couldn't do this until now, as might lose significant bits off top. */
	srl	glyphbits, 1

/* Offset p from pdst so we can negative index off it */
	addu	p, pdst, ww

/* Do case jump into straight-line code for rest of complete words. */
	j	$25
	
	.rdata
$1000:
	.word	OOPS	# Never used
	.word	OOPS	# Never used
	.word	OOPS	# Never used
	.word	OOPS	# Never used
	.word	$1003	# Write 3 more full words (total 4)
	.word	$1004	# Write 4 more full words (total 5)
	.word	$1005	# Write 5 more full words (total 6)
	.word	$1006	# Write 6 more full words (total 7)
	.word	$1007	# Write 7 more full words (total 8)
	.text

OOPS:	j	$1002				# Better than crashing?
	

$1007:
	and	$24, glyphbits, (15 << 3)
	addu	$24, itfgbgmap
	lw	$24, xorbits($24)
	sw	$24, -28(p)
	srl	glyphbits, 4

$1006:
	and	$24, glyphbits, (15 << 3)
	addu	$24, itfgbgmap
	lw	$24, xorbits($24)
	sw	$24, -24(p)
	srl	glyphbits, 4

$1005:
	and	$24, glyphbits, (15 << 3)
	addu	$24, itfgbgmap
	lw	$24, xorbits($24)
	sw	$24, -20(p)
	srl	glyphbits, 4

$1004:
	and	$24, glyphbits, (15 << 3)
	addu	$24, itfgbgmap
	lw	$24, xorbits($24)
	sw	$24, -16(p)
	srl	glyphbits, 4

$1003:
	and	$24, glyphbits, (15 << 3)
	addu	$24, itfgbgmap
	lw	$24, xorbits($24)
	sw	$24, -12(p)
	srl	glyphbits, 4

$1002:
	and	$24, glyphbits, (15 << 3)
	addu	$24, itfgbgmap
	lw	$24, xorbits($24)
	sw	$24, -8(p)
	srl	glyphbits, 4

$1001:
	and	$24, glyphbits, (15 << 3)
	addu	$24, itfgbgmap
	lw	$24, xorbits($24)
	sw	$24, -4(p)

/* End while h != 0 */
	addu	pdst, widthDst			# pdst += widthDst;
	bne	pdst, pdstEnd, $1033

$1900:
/*  end while nglyph >= 0 */
	addu	pdstBase, ww			# pdstBase += ww;
	addu	pdstEnd, ww			# pdstEnd += ww;
	addu	ppci, NGLYPHS*4			# ppci += NGLYPHS
	subu	nglyph, NGLYPHS			# nglyph -= NGLYPHS;
	bge	nglyph, 0, $1032

$2000:
	addu	nglyph, NGLYPHS
	
#ifndef GLYPH32
/****************************************************************
Phase 3: Paint the last NGLYPHS-1 glyphs, plus 0..3 bits from the previous
   glyph.  Again, leave loop with conditions:
   (1) pdstBase & 3 == 0	(pdstBase is word-aligned)
   (2) 0..3 bits left to paint in each row of last glyph painted
*****************************************************************/
 
	beq	nglyph, 0, $4000

$3032:
/* main loop on glyphs one at a time */
	lw	pglyphprev, glyphBitmap(pci)	# pglyphprev = pci->picture;

	lw	pci, 4(ppci)			# pci = ppci[1];
	lw	pglyph0, glyphBitmap(pci)	# pglyph0 = pci->picture;
 
/* New leftbits is old rightbits, compute new rightbits */
	move	leftbits, rightbits		# leftbits = rightbits;
	addu	ww, width, leftbits		# ww = # bytes of full words
	and	rightbits, ww, 3		# new # bits left over
	subu	ww, rightbits

/* Set up computed jump address $25 to sequential full word stores used below */
	la	$25, $3000
	addu	$25, ww
	lw	$25, 0($25)

	move	pdst, pdstBase			# pdst = pdstBase;
	addu	pdstBase, ww			# pdstBase += ww;

/* Loop over scanlines of glyph */
$3033:
 #	    do {
/* Load up glyph for this scanline, bump pointers to next glyph row */
#if defined(GLYPH6) || defined(GLYPH8)
	lbu	glyphbits, 0(pglyph0)
	addu	pglyph0, 1
	lbu	$24, 0(pglyphprev)
	addu	pglyphprev, 1
#else defined(GLYPH10) || defined(GLYPH16)
	lhu	glyphbits, 0(pglyph0)
	addu	pglyph0, 2
	lhu	$24, 0(pglyphprev)
	addu	pglyphprev, 2
#endif

/* Or in bits from previous glyph */
	subu	tmp, width, leftbits
	srl	$24, tmp
	sll	glyphbits, leftbits
	or	glyphbits, $24

/* Offset glyphbits left 3 bits to make fgbgmap indexing faster. */
	sll	glyphbits, 3

/* Do case jump into straight-line code for writing complete words.
   We know that we will paint at minimum and at maximum:

		  minimum			maximum
	GLYPH6	4 / 4	1 word		(3+6) / 4	2 words
	GLYPH8	7 / 4	1 word		(3+8) / 4	2 words
	GLYPH10	9 / 4	2 words		(3+10) / 4	3 words
	GLYPH16 11 / 4	2 words		(3+16) / 4	4 words
	GLYPH32 does not use this case, as phase 2 has already satisfied
	 	the conditions that phase 3 must satisfy.
*/

/*  First point pdst past everything, so we can count down to it. */

	addu	p, pdst, ww			# p = pdst + ww;
	j	$25
	
	.rdata
$3000:
	.word	$3001	# Never used
	.word	$3001	# Write 1 full word
	.word	$3002	# Write 2 full words
#if defined(GLYPH10) || defined(GLYPH16)
	.word	$3003	# Write 3 full words
#if defined(GLYPH16)
	.word	$3004	# Write 4 full words
#endif defined(GLYPH16)
#endif defined(GLYPH10) || defined(GLYPH16)
	.text

#if defined(GLYPH16)
$3004:
	and	$24, glyphbits, (15 << 3)
	addu	$24, itfgbgmap
	lw	$24, xorbits($24)
	sw	$24, -16(p)
	srl	glyphbits, 4
#endif

#if defined(GLYPH10) || defined(GLYPH16)
$3003:
	and	$24, glyphbits, (15 << 3)
	addu	$24, itfgbgmap
	lw	$24, xorbits($24)
	sw	$24, -12(p)
	srl	glyphbits, 4
#endif

$3002:
	and	$24, glyphbits, (15 << 3)
	addu	$24, itfgbgmap
	lw	$24, xorbits($24)
	sw	$24, -8(p)
	srl	glyphbits, 4

$3001:
	and	$24, glyphbits, (15 << 3)
	addu	$24, itfgbgmap
	lw	$24, xorbits($24)
	sw	$24, -4(p)

/* End while h != 0 */
	addu	pdst, widthDst			# pdst += widthDst;
	bne	pdst, pdstEnd, $3033

/*  end while nglyph > 0 */
	addu	pdstEnd, ww			# pdstEnd += ww;
	addu	ppci, 4				# ppci += 1
	subu	nglyph, 1			# nglyph -= 1;
	bne	nglyph, 0, $3032
#endif !defined(GLYPH32)

/****************************************************************
Phase 4: Paint the last 0..3 bits from the previous glyph.
*****************************************************************/
 
$4000:
	move	leftbits, rightbits
	beq	rightbits, 0, $9900

	lw	pglyphprev, glyphBitmap(pci)	# pglyphprev = pci->picture;

/* Compute shifts to use on glyphbits */
	addu	pdst, pdstBase, rightbits	# adjust for swl instruction
	addu	pdstEnd, rightbits
	subu	rightbits, width, leftbits	# right shift = width - leftbits
	negu	leftbits, leftbits		# left shift = 7 - leftbits
	addu	leftbits, 7

/* Loop over scanlines of glyph */
$4033:
#if defined(GLYPH6) || defined(GLYPH8)
	lbu	glyphbits, 0(pglyphprev)
	addu	pglyphprev, 1
#endif
#if defined(GLYPH10) || defined(GLYPH16)
	lhu	glyphbits, 0(pglyphprev)
	addu	pglyphprev, 2
#endif
#if defined(GLYPH32)
	ulw	glyphbits, 0(pglyphprev)
	addu	pglyphprev, widthGlyph
#endif

/* Shift bits to perfect position */
	srl	glyphbits, rightbits
	sll	glyphbits, leftbits

/* Write ragged right bits */
	and	$24, glyphbits, (15 << 3)
	addu	$24, itfgbgmap
	lw	$24, xorbits($24)
	swl	$24, -1(pdst)

/* End while h != 0 */
	addu	pdst, widthDst			# pdst += widthDst;
	bne	pdst, pdstEnd, $4033

$9900:

/* Restore callee's registers */
#ifdef GLYPH6
	lw	$30, 32($sp)
#endif
#if defined(GLYPH6) || defined(GLYPH8)
	lw	$23, 28($sp)
#endif
#if defined(GLYPH6) || defined(GLYPH8) || defined(GLYPH10)
	lw	$22, 24($sp)
#endif
	lw	$21, 20($sp)
	lw	$20, 16($sp)
	lw	$19, 12($sp)
	lw	$18, 8($sp)
	lw	$17, 4($sp)
	lw	$16, 0($sp)
	addu	$sp, framesize
 # } /* CFBIMAGEGLYPHS */

$9999:
	j	$31
	.end	CFBIMAGEGLYPHS

