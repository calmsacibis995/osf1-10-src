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
**                 COPYRIGHT (c) 1988, 1989, 1990, 1991 BY                  *
**              DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.               *
**			     ALL RIGHTS RESERVED                            *
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

/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/server/ddx/dec/sfb/sfbteglyph.s,v 1.1.3.3 92/02/21 11:44:58 David_Coleman Exp $ */

#include "sfbasm.h"

/* This file can be compiled for several different scenarios.  In all cases we
   are dealing with fixed-width and height glyphs, minimum width 1, maximum
   width 32:

SFBTESPLATGLYPHS	glyph widths handled
--------------------------------------------
sfbTESplatGlyphs8	     4-8
sfbTESplatGlyphs16	     9-16
sfbTESplatGlyphs32	    17-32

*/

	.sdata
fakeResidueBits:	.word 0
	.text
	

	.globl	SFBTESPLATGLYPHS

 # void SFBTESPLATGLYPHS(pdstBase, widthDst, nglyph, ppci, [sfb])

	.ent	SFBTESPLATGLYPHS 2
	framesize = 40
SFBTESPLATGLYPHS:
#if GLYPHWIDTH == 8
	.mask	0xc0ff0000, -4
#elif GLYPHWIDTH == 16
	.mask	0xc00f0000, -4
#elif GLYPHWIDTH == 32
	.mask	0xc00f0000, -4
#endif
	.frame	$sp, framesize, $31

/*
 * Register allocation.  Registers $24, $25 used as much as
 * possible for expression temporaries and limited-span temporaries
 */

/* ||| Need another register */
#define residue		$15

/* Scratch temporaries $2-$3 */
#define width		$2
#define nwidth		$3

/* Register parameters $4-$7 */
#define pdstBase	$4
#define widthDst	$5
#define nglyph		$6
#define ppci		$7

/* Scratch temporaries $8-$15 */
#define tnt		$8
#define rightShift	$9
#define pglyph0		$10
#define pglyph1		$11
#define pGlyphResidue	$12
#define newres		$13
#define n		$14
#ifdef SFBIMAGETEXT
#define sfb		$15
#endif

/* Must save if used $16-$23 */
#define threshhold_mask	$16
#define pdstEnd		$17
#if GLYPHWIDTH == 8
#define	pglyph2		$18
#define	pglyph3		$19
#define pglyph4		$20
#define	pglyph5		$21
#define pglyph6		$22
#define pglyph7		$23
#elif GLYPHWIDTH == 16
#define pglyph2		$18
#define pglyph3		$19
#elif GLYPHWIDTH == 32
#define glyphMask	$18
#define glyphWidth	$19
#endif

/* Scratch temporaries $24-$25 	*/
/* general temporary	$24 	*/
/* general temporary	$25	*/

/* Must save if used $30-$31 	*/
#define pdst		$30
/* jal register linkage	$31 	*/



/* Record offset definitions. */
#define leftSideBearing	 0
#define rightSideBearing 2
#define characterWidth	 4
#define ascent		 6
#define descent		 8
#define glyphBitmap	12

#define NDIV		 0
#define NWIDTH		 1

/* Load up additional arguments */
#ifdef SFBIMAGETEXT
	lw	sfb, 16($sp)
#endif

/* Save callee's registers */
	.noalias	$sp, ppci
	subu	$sp, framesize
	sw	$16, 0($sp)
	sw	$17, 4($sp)
	sw	$18, 8($sp)
	sw	$19, 12($sp)
#if GLYPHWIDTH == 8
	sw	$20, 16($sp)
	sw	$21, 20($sp)
	sw	$22, 24($sp)
	sw	$23, 28($sp)
#endif
	sw	$30, 32($sp)
	sw	$31, 36($sp)

/* Compute constant height and width for all characters */
	lw	tnt, 0(ppci)			#  pci = *ppci;
	lh	$24, ascent(tnt)
	lh	$25, descent(tnt)
	lh	width, characterWidth(tnt)
/* Actually, rather than compute height, we always keep pdstEnd defined so
   that loops terminate when pdst == pdstEnd */
	addu	pdstEnd, $24, $25
	multu	pdstEnd, widthDst

#if GLYPHWIDTH == 32
	li	glyphMask, -1			# glyphMask = 1's for data
	negu	$24, width
	srl	glyphMask, $24
	addu	glyphWidth, width, 7 		# glyphWidth = (width + 7) >> 3
	sra	glyphWidth, 3
#endif

/****************************************************************
Phase 1: Paint as many glyphs as we can, given that we also have to shift
         them left because alignment constraints.  Leave pdstBase aligned s.t.:
   (1) pdstBase & SFBALIGNMASK == 0 (pdstBase is SFB-aligned)
   (2) 0..width-1 bits left to paint in each row of glyph
*****************************************************************/
 
	andi	newres, pdstBase, SFBALIGNMASK	# align = pdstBase&SFBALIGNMASK
	subu	pdstBase, newres		# pdstBase -= align
	SFBBYTESTOPIXELS(newres)		# align /= SFBPIXELBYTES

#ifdef SFBIMAGETEXT
	li	$24, SFBSTIPPLEALL1		# mask = SFBLEFTMASK(align)
	SFBLEFTSTIPPLEMASK(threshhold_mask, newres, $24)
#endif

/* Compute tn, nwidth, residue.  (Note mul below is really just a shift.) */
	li	tnt, SFBBUSBITS/GLYPHWIDTH	# tn = SFBBUSBITS/GLYPHWIDTH
	mul	nwidth, width, SFBBUSBITS/GLYPHWIDTH # nwidth = tn*width
	addu	residue, nwidth, newres		# residue = nwidth + align...
	subu	residue, SFBBUSBITS 		# ... - SFBBUSBITS

	bge	residue, 0, $120		# while (residue < 0)
$110:	addu	tnt, 1				# tn++
	addu	nwidth, width			# nwidth += width
	addu	residue, width			# residue -= width
	blt	residue, 0, $110

$120:	mflo	$24				# pdstEnd = pdstBase + ...
	addu	pdstEnd, pdstBase, $24

	ble	tnt, nglyph, $130		# if (tn > nglyph)
#ifdef SFBIMAGETEXT
	multu	nglyph, width			# nglyph * width...
#endif
	move	residue, newres
	li	newres, 0
	la	pGlyphResidue, fakeResidueBits
	j	PAINTFIRSTANDLASTCHARS

/* Incredibly gross.  To avoid nops, we have to load up two pglyphs at a time.
   So we have two sets of load code: one for odd tn, one for even tn. */

	.set	noreorder
#if GLYPHWIDTH == 8
LPG76:	lw	$24, 28(ppci)
	lw	$25, 24(ppci)
	lw	pglyph7, glyphBitmap($24)
	lw	pglyph6, glyphBitmap($25)
LPG54:	lw	$24, 20(ppci)
	lw	$25, 16(ppci)
	lw	pglyph5, glyphBitmap($24)
	lw	pglyph4, glyphBitmap($25)
#endif
#if GLYPHWIDTH <= 16
LPG32:	lw	$24, 12(ppci)
	lw	$25, 8(ppci)
	lw	pglyph3, glyphBitmap($24)
	lw	pglyph2, glyphBitmap($25)
#endif
LPG10:	lw	$24, 4(ppci)
	lw	$25, 0(ppci)
	lw	pglyph1, glyphBitmap($24)
	j	$31
LPGEVEN:lw	pglyph0, glyphBitmap($25)

#if GLYPHWIDTH == 8
LPG65:	lw	$24, 24(ppci)
	lw	$25, 20(ppci)
	lw	pglyph6, glyphBitmap($24)
	lw	pglyph5, glyphBitmap($25)
LPG43:	lw	$24, 16(ppci)
	lw	$25, 12(ppci)
	lw	pglyph4, glyphBitmap($24)
	lw	pglyph3, glyphBitmap($25)
#endif
#if GLYPHWIDTH <= 16
LPG21:	lw	$24, 8(ppci)
	lw	$25, 4(ppci)
	lw	pglyph2, glyphBitmap($24)
	lw	pglyph1, glyphBitmap($25)
#endif
LPG0:	lw	$24, 0(ppci)
	j	$31
LPGODD:	lw	pglyph0, glyphBitmap($24)

#if GLYPHWIDTH == 8
LGB7:	lbu	$25, 0(pglyph7)
	addu	pglyph7, 1
	sll	$24, width
	or	$24, $25
LGB6:	lbu	$25, 0(pglyph6)
	addu	pglyph6, 1
	sll	$24, width
	or	$24, $25
LGB5:	lbu	$25, 0(pglyph5)
	addu	pglyph5, 1
	sll	$24, width
	or	$24, $25
LGB4:	lbu	$25, 0(pglyph4)
	addu	pglyph4, 1
	sll	$24, width
	or	$24, $25
LGB3:	lbu	$25, 0(pglyph3)
	addu	pglyph3, 1
	sll	$24, width
	or	$24, $25
LGB2:	lbu	$25, 0(pglyph2)
	addu	pglyph2, 1
	sll	$24, width
	or	$24, $25
LGB1:	lbu	$25, 0(pglyph1)
	addu	pglyph1, 1
	sll	$24, width
	or	$24, $25
LGB0:	lbu	$25, 0(pglyph0)
	addu	pglyph0, 1
	sll	$24, width
	j	$31
LGB:	or	$24, $25
#endif

#if GLYPHWIDTH == 16
LGB3:	lhu	$25, 0(pglyph3)
	addu	pglyph3, 2
	sll	$24, width
	or	$24, $25
LGB2:	lhu	$25, 0(pglyph2)
	addu	pglyph2, 2
	sll	$24, width
	or	$24, $25
LGB1:	lhu	$25, 0(pglyph1)
	addu	pglyph1, 2
	sll	$24, width
	or	$24, $25
LGB0:	lhu	$25, 0(pglyph0)
	addu	pglyph0, 2
	sll	$24, width
	j	$31
LGB:	or	$24, $25
#endif
	
#if GLYPHWIDTH == 32
LGB1:	lwl	$25, 0(pglyph1)
	lwr	$25, 3(pglyph1)
	addu	pglyph1, glyphWidth
	and	$25, glyphMask
	sll	$24, width
	or	$24, $25
LGB0:	lwl	$25, 0(pglyph0)
	lwr	$25, 3(pglyph0)
	addu	pglyph0, glyphWidth
	and	$25, glyphMask
	sll	$24, width
	j	$31
LGB:	or	$24, $25
#endif
	
	.set	reorder

$130:
/* Fill pglyph0, pglyph1, etc. */
	andi	$24, tnt, 1
	la	$25, LPGEVEN			# Assume tn even
	beq	$24, 0, $140
	la	$25, LPGODD			# Oops, tn is odd
$140:
	sll	$24, tnt, 3
	subu	$25, $24
	move	pdst, pdstBase			# pdst = pdstBase;
	jal	$25

	sll	$24, tnt, 2			# ppci += tn
	addu	ppci, $24
	subu	nglyph, tnt			# nglyph -= tn

/* Compute tnt = entry point into glyphbits loading code */
#if GLYPHWIDTH <= 16
	sll	$24, tnt, 4			# $24 = 4 inst * 4 bytes * tn
#else
	sll	$24, tnt, 1			# $24 = 6 inst * 4 bytes * tn
	addu	$24, tnt
	sll	$24, 3
#endif
	la	tnt, LGB
	subu	tnt, $24

/* Now for each scanline load up a word full of bits and paint it */

$150:						# do {
	li	$24, 0				# fetch glyphbits
	jal	tnt

#ifdef SFBIMAGETEXT
	sw	threshhold_mask, SFBPIXELMASK(sfb)	# write pixel mask
#endif
	sll	$24, newres			# write glyphbits << align
	StippleEntireWord(pdst, $24)
	addu	pdst, widthDst			# pdst += widthDst
	bne	pdst, pdstEnd, $150		# while (pdst != pdstEnd)

	addu	pdstBase, SFBBUSBITS * SFBPIXELBYTES
	addu	pdstEnd, SFBBUSBITS * SFBPIXELBYTES
	beq	nglyph, 0, $300

/****************************************************************
Phase 2: Paint groups of glyphs so as to always paint SFBBUSBITS
   Leave loop with conditions:
   (1) pdstBase & SFBALIGNMASK == 0    (pdstBase is SFB-aligned)
   (2) 0..width-1 bits left to paint in each row of last glyph painted
*****************************************************************/

	la	$24, divTable			# n = divTable[width].ndiv
	sll	$25, width, 1
	addu	$25, $24
	lbu	n, NDIV($25)
	lbu	nwidth, NWIDTH($25)		# nwidth = divTable[width.nwidth
	li	threshhold_mask, SFBBUSBITS	# threshhold = SFBBUSBITS-nwidth
	subu	threshhold_mask, nwidth

$200:
	lw	$24, -4(ppci)			# load pGlyphResidue
	lw	pGlyphResidue, glyphBitmap($24)
	slt	$25, residue, threshhold_mask
	addu	tnt, n, $25			# tn = n + (residue<threshhold)
	addu	newres, residue, nwidth		# newresidue = residue + nwidth
	beq	tnt, n, $210			# if tn != n
	addu	newres, width			#	newresidue += width
$210:	subu	rightShift, width, residue	# rightShift = width - residue
	blt	nglyph, tnt, PAINTLASTCHARS	# if (tn > nglyph)

/* Fill pglyph0, pglyph1, etc. */
	andi	$24, tnt, 1
	la	$25, LPGEVEN			# Assume tn even
	beq	$24, 0, $240
	la	$25, LPGODD			# Oops, tn is odd
$240:
	sll	$24, tnt, 3
	subu	$25, $24
	move	pdst, pdstBase			# pdst = pdstBase;
	jal	$25

	sll	$24, tnt, 2			# ppci += tn
	addu	ppci, $24
	subu	nglyph, tnt			# nglyph -= tn

/* Now for each scanline load up a word full of bits and paint it */

/* Compute tnt = entry point into glyphbits loading code */
#if GLYPHWIDTH <= 16
	sll	$24, tnt, 4			# $24 = 4*4*tn
#else
	sll	$24, tnt, 1			# $24 = 6*4*tn
	addu	$24, tnt
	sll	$24, 3
#endif
	la	tnt, LGB
	subu	tnt, $24

$250:						# do {
	li	$24, 0				# fetch glyphbits
	jal	tnt

/* Now get residue bits in there, too. */
#if GLYPHWIDTH == 8
	lbu	$25, 0(pGlyphResidue)
	addu	pGlyphResidue, 1
#elif GLYPHWIDTH == 16
	lhu	$25, 0(pGlyphResidue)
	addu	pGlyphResidue, 2
#elif GLYPHWIDTH == 32
	ulw	$25, 0(pGlyphResidue)
	and	$25, glyphMask
	addu	pGlyphResidue, glyphWidth
#endif
	sll	$24, residue
	srl	$25, rightShift
	or	$24, $25

/* Write the word */
	StippleEntireWord(pdst, $24)
	addu	pdst, widthDst			# pdst += widthDst
	bne	pdst, pdstEnd, $250		# while (pdst != dstEnd)
	
	andi	residue, newres, SFBBUSBITSMASK
	addu	pdstBase, SFBBUSBITS * SFBPIXELBYTES
	addu	pdstEnd, SFBBUSBITS * SFBPIXELBYTES
	bne	nglyph, 0, $200			# while (nglyph != 0)

/****************************************************************
Phase 3: Paint the last bits of the last glyph (if any).
*****************************************************************/
 
$300:
	beq	residue, 0, $9999

	lw	$24, -4(ppci)			# load pGlyphResidue
	lw	pGlyphResidue, glyphBitmap($24)
	subu	rightShift, width, residue	# rightShift = width - residue
#ifdef SFBIMAGETEXT
	li	$24, SFBSTIPPLEALL1		# mask = SFBRIGHTMASK(residue)
	SFBRIGHTSTIPPLEMASK(threshhold_mask, residue, $24)
#endif

#if GLYPHWIDTH > SFBSTIPPLEBITS
	subu	residue, SFBSTIPPLEBITS		# residue -= SFBSTIPPLEBITS
	bgt	residue, 0 $320			# if residue <= 0
#endif

$310:						# do {
#if GLYPHWIDTH == 8
	lbu	$25, 0(pGlyphResidue)		# GETGLYPHBITS(pGlyphResidue...
	addu	pGlyphResidue, 1
#elif GLYPHWIDTH == 16
	lhu	$25, 0(pGlyphResidue)
	addu	pGlyphResidue, 2
#elif GLYPHWIDTH == 32
	ulw	$25, 0(pGlyphResidue)
#ifndef SFBIMAGETEXT
	and	$25, glyphMask
#endif
	addu	pGlyphResidue, glyphWidth
#endif
#ifdef SFBIMAGETEXT
	sw	threshhold_mask, SFBPIXELMASK(sfb)
#endif
	srl	$25, rightShift			# glyphbits >>= rightShift
	sw	$25, 0(pdstBase)
	addu	pdstBase, widthDst		# pdstBase += widthDst
	bne	pdstBase, pdstEnd, $310		# } while (pdstBase != pdstEnd)
	j	$9999

#if GLYPHWIDTH > SFBSTIPPLEBITS
$320:						# do {
# if GLYPHWIDTH == 8
	lbu	$25, 0(pGlyphResidue)		# GETGLYPHBITS(pGlyphResidue...
	addu	pGlyphResidue, 1
# elif GLYPHWIDTH == 16
	lhu	$25, 0(pGlyphResidue)
	addu	pGlyphResidue, 2
# elif GLYPHWIDTH == 32
	ulw	$25, 0(pGlyphResidue)
#  ifndef SFBIMAGETEXT
	and	$25, glyphMask
#  endif
	addu	pGlyphResidue, glyphWidth
# endif
	srl	$25, rightShift			# glyphbits >>= rightShift

# ifdef SFBIMAGETEXT
	move	tnt, residue			# m = residue
$330:						# do {
	sw	$25, 0(pdst)			# SFBWRITE(pdstBase, glyphbits)
	addu	pdst, SFBSTIPPLEBYTESDONE	# pdst += SFBSTIPPLEBYTESDONE
	srl	$25, SFBSTIPPLEBITS		# glyphbits >>= SFBSTIPPLEBITS
	subu	tnt, SFBSTIPPLEBITS		# m -= SFBSTIPPLEBITS
	bgt	tnt, 0, $330			# } while m > 0
	sw	threshhold_mask, SFBPIXELMASK(sfb)
	sw	$25, 0(pdst)
# else
	beq	$25, 0, $340			# while glyphbits != 0 {
$330:	sw	$25, 0(pdst)			# SFBWRITE(pdst, glyphbits)
	addu	pdst, SFBSTIPPLEBYTESDONE	# pdst += SFBSTIPPLEBYTESDONE
	srl	$25, SFBSTIPPLEBITS		# glyphbits >>= SFBSTIPPLEBITS
	bne	$25, 0, $330			# } while
$340:
# endif
	addu	pdstBase, widthDst
	bne	pdstBase, pdstEnd, $320		# } while (pdstBase != pdstEnd)
	j	$9999
#endif


/****************************************************************
PAINTLASTCHARS: Get here if not enough chars to round out a word
*****************************************************************/
 
PAINTLASTCHARS:
#ifdef SFBIMAGETEXT
	multu	nglyph, width			# nglyph * width
	li	threshhold_mask, SFBSTIPPLEALL1	# leftMask = SFBSTIPPLEALL1
#endif

#if GLYPHWIDTH == 8
	li	newres, 1			# residueWidthGlyph = 1
#elif GLYPHWIDTH == 16
	li	newres, 2			# residueWidthGlyph = 2
#elif GLYPHWIDTH == 32
	move	newres, glyphWidth		# residueWidthGlyph = glyphWidth
#endif
	lw	tnt, -4(ppci)			# pGlyphResidue = ppci[-1].pPriv
	lw	pGlyphResidue, glyphBitmap(tnt)

PAINTFIRSTANDLASTCHARS:
/* ||| Only compute right mask, keep it in tnt.  Shit. */
#ifdef SFBIMAGETEXT
	li	$24, SFBSTIPPLEALL1		# mask &= SFBRIGHTMASK(...
	mflo	$25				#   ...residue + nglyph*width)
	addu	$25, residue
	SFBRIGHTSTIPPLEMASK(tnt, $25, $24)
	and	threshhold_mask, tnt
#endif
	sub	rightShift, width, residue
/* Fill pglyph0, pglyph1, etc. */
	andi	$24, nglyph, 1
	la	$25, LPGEVEN			# Assume nglyph even
	beq	$24, 0, $440
	la	$25, LPGODD			# Oops, nglyph is odd
$440:
	sll	$24, nglyph, 3
	subu	$25, $24
	jal	$25		# ||| NOP here

/* Compute tnt = entry point into glyphbits loading code */
#if GLYPHWIDTH <= 16
	sll	$24, nglyph, 4			# $24 = 4*4*nglyph
#else
	sll	$24, nglyph, 1			# $24 = 6*4*nglyph
	addu	$24, nglyph
	sll	$24, 3
#endif
	la	tnt, LGB
	subu	tnt, $24

$450:						# do {
	li	$24, 0				# fetch glyphbits
	jal	tnt

/* Now get residue bits in there, too. */
#if GLYPHWIDTH == 8
	lbu	$25, 0(pGlyphResidue)
	addu	pGlyphResidue, newres
#elif GLYPHWIDTH == 16
	lhu	$25, 0(pGlyphResidue)
	addu	pGlyphResidue, newres
#elif GLYPHWIDTH == 32
	ulw	$25, 0(pGlyphResidue)
	and	$25, glyphMask
	addu	pGlyphResidue, newres
#endif
	sll	$24, residue
	srl	$25, rightShift
	or	$24, $25

/* Write the word */
#ifdef SFBIMAGETEXT
	sw	threshhold_mask, SFBPIXELMASK(sfb)
#endif
	sw	$24, 0(pdstBase)
	addu	pdstBase, widthDst		# pdstBase += widthDst
	bne	pdstBase, pdstEnd, $450		# while (pdstBase != dstEnd)
	
$9999:

/* Restore callee's registers */
	lw	$31, 36($sp)
	lw	$30, 32($sp)
#if GLYPHWIDTH == 8
	lw	$23, 28($sp)
	lw	$22, 24($sp)
	lw	$21, 20($sp)
	lw	$20, 16($sp)
#endif
	lw	$19, 12($sp)
	lw	$18, 8($sp)
	lw	$17, 4($sp)
	lw	$16, 0($sp)
	addu	$sp, framesize
 # } /* SFBIMAGEGLYPHS */

	j	$31
	.end	SFBIMAGEGLYPHS

