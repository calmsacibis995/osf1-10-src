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

/* This file can be compiled for several different scenarios.  In all cases 
   except GLYPH32 we are dealing with fixed-width and height glyphs, minimum
   width 4, maximum width 29.  If GLYPH32, we may either have a big fixed-width
   font, or just about any size variable-pitch font.  We deal with both cases
   as if they were variable-pitch.  So we have the following posibilities:

	define   NGLYPHS   WIDTHGLYPH	possible glyph widths
	-----------------------------------------------------
	GLYPH6	    5	  	1		 4..6
	GLYPH8	    4		1		 7..8
	GLYPH10     3		2		 9..10
	GLYPH16     2		2		11..16
	GLYPH32	    1         1..4     		 4..29
*/

#if defined(GLYPH6) || defined(GLYPH8) || defined(GLYPH10) || defined(GLYPH16)
#else
#define GLYPH32
#endif

	.globl	CFBSPLATGLYPHS

 # void CFBSPLATGLYPHS(pdstBase, widthDst, nglyph, ppci, fgandbits, fbxorbits)

	.ent	CFBSPLATGLYPHS 2
CFBSPLATGLYPHS:
#ifdef GLYPH6
	framesize = 24
	.mask	0x003f000, -4
#define NGLYPHS 5
#endif
#ifdef GLYPH8
	framesize = 20
	.mask	0x001f0000, -4
#define NGLYPHS 4
#endif
#ifdef GLYPH10
	framesize = 16
	.mask	0x000f0000, -4
#define NGLYPHS 3
#endif
#ifdef GLYPH16
	framesize = 12
	.mask	0x00070000, -4
#define NGLYPHS 2
#endif
#ifdef GLYPH32
	framesize = 0
	.mask	0x00000000, -4
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

/* Register parameters.  */
#define pdstBase	$4
#define widthDst	$5
#define nglyph		$6
#define ppci		$7

/* Scratch temporaries */
#define pglyph0		$8
#define pci		$9
#define align		$10
#define casetable	$11
#define h		$12
#define fgxorbits	$13

#ifdef GLYPH32
#define glyphMask	$14
#define widthGlyph	$15
#else
#define height		$14
#define width		$15
#endif

/* Must save if used registers */

#if defined(GLYPH6) || defined(GLYPH8) || defined(GLYPH10) || defined(GLYPH16)
#define widthn		$16
#define align4		$17
#define pglyph1		$18
#endif

#if defined(GLYPH6) || defined(GLYPH8) || defined(GLYPH10)
#define pglyph2		$19
#endif
#if defined(GLYPH6) || defined(GLYPH8)
#define pglyph3		$20
#endif
#if defined(GLYPH6)
#define pglyph4		$21
#endif

/* Record offset definitions. */
#define leftSideBearing	 0
#define rightSideBearing 2
#define characterWidth	 4
#define ascent		 6
#define descent		 8
#define glyphBitmap	12

/* Load up arguments currently in memory */
	lw	fgxorbits, 20($sp)

 #  while (nglyph != 0) {
	beq	nglyph, 0, $9999

/* Save callee's registers */
#if defined(GLYPH6) || defined(GLYPH8) || defined(GLYPH10) || defined(GLYPH16)
	subu	$sp, framesize
	sw	$16, 0($sp)
	sw	$17, 4($sp)
	sw	$18, 8($sp)
#endif
#if defined(GLYPH6) || defined(GLYPH8) || defined(GLYPH10)
	sw	$19, 12($sp)
#endif
#if defined(GLYPH6) || defined(GLYPH8)
	sw	$20, 16($sp)
#endif
#if defined(GLYPH6)
	sw	$21, 20($sp)
#endif

#ifndef GLYPH32
/* Permanently displace pdstBase to upper left corner of destination... */
/* ...and compute constant height and width for all characters 		*/

	lw	pci, 0(ppci)			# pci = *ppci;
 #   	    /* find pixel for top left bit of glyph */
 #  	    pdst = pdstBase - (pci->metrics.ascent * widthDst);
	lh	$24, ascent(pci)		# $24 used below again
	mul	$25, $24, widthDst
	subu	pdstBase, $25

	lh	glyphbits, descent(pci)		# use glyphbits improves sched.
	addu	height, $24, glyphbits
	lh	width, characterWidth(pci)

/****************************************************************
Phase 1: Paint glpyhs in groups as long as there are enough left
*****************************************************************/
 
	subu	nglyph, NGLYPHS			# make loop test faster
	blt	nglyph, 0, $2000

	la	casetable, $1600

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

$1100:
/* main loop on NGLYPHS at a time */
	lw	pci, 0(ppci)			# pci = ppci[0];
	lw	pglyph0, glyphBitmap(pci) 	# pglyph0 = pci->picture;

	lw	pci, 4(ppci)			# pci = ppci[1];
	lw	pglyph1, glyphBitmap(pci) 	# pglyph1 = pci->picture;

#if defined(GLYPH6) || defined(GLYPH8) || defined(GLYPH10)
	lw	pci, 8(ppci)			# pci = ppci[2];
	lw	pglyph2, glyphBitmap(pci) 	# pglyph2 = pci->picture;
#endif

#if defined(GLYPH6) || defined(GLYPH8)
	lw	pci, 12(ppci)			# pci = ppci[3];
	lw	pglyph3, glyphBitmap(pci) 	# pglyph3 = pci->picture;
#endif
#if defined(GLYPH6)
	lw	pci, 16(ppci)			# pci = ppci[4];
	lw	pglyph4, glyphBitmap(pci) 	# pglyph4 = pci->picture;
#endif

	move	h, height
	move	pdst, pdstBase

 #  	    /* Align pdst to word boundary.  Keep track of how many extra
 #  	       pixels this adds...we compensate for these pixels by shifting
 #  	       glyphbits left by this amount.  Since we shift in 0's, this
 #  	       doesn't cause anything bad to get written. */
	and	align, pdst, 3			# align = ((int) pdst) & 3;
	subu	pdst, align			# pdst = pdst & ~3;
	li	align4, 4			# align4 = 4 - align
	subu	align4, align
	addu	align, 5			# 32 bytes per case

/*
This is so gross I hate to describe it.  Nonetheless...the tops of terminal
emulator fonts are often 0, due to interline spacing at the top, as well as
lower-case letter that don't go very high.  Also, glyph information always
starts on an aligned word.  So we can read up a whole word from each glyph, or
them all together, and quickly find out how many of the top 4 (GLYPH6 and
GLYPH8) or top 2 (GLYPH10 and GLYPH16) scanlines are all 0.

In all cases 2 is a good expected number for the number of blank lines.  The
tests below try to establish this as quickly as possible.  Also note that we
can andi against 0xff and 0xffff directly, and we don't need to test against
0xffffffff.  Testing against 0xffffff, though, isn't a single instruction.
*/

#if defined(GLYPH6) || defined(GLYPH8)
	ulw	glyphbits, 0(pglyph0)
	ulw	$24, 0(pglyph1)
	or	glyphbits, $24
	ulw	$25, 0(pglyph2)
	or	glyphbits, $25
	ulw	$24, 0(pglyph3)
	or	glyphbits, $24
#if defined(GLYPH6)
	ulw	$25, 0(pglyph4)
	or	glyphbits, $25
#endif
	and	$24, glyphbits, 0xffff		# line 0 & 1 all 0's?
	li	$25, 2				# hope for the best
	beq	$24, 0, $1220

	and	$24, glyphbits, 0xff		# line 0 all 0's?
	bne	$24, 0, $1400
	li	$25, 1
	addu	pdst, widthDst
	j	$1300

$1220:						# Lines 0 & 1 are all 0's
	addu	pdst, widthDst
	addu	pdst, widthDst
	srl	glyphbits, 16

	and	$24, glyphbits, 0xff		# Line 3 all 0's?
	bne	$24, 0, $1300
	li	$25, 3
	addu	pdst, widthDst
	
	bne	glyphbits, 0, $1300		# line 3 all 0's?
	addu	pdst, widthDst
	li	$25, 4

$1300:
 # skip down as many scanlines as we've recorded in $25
	subu	h, $25
#if defined(GLYPH6)
	addu	pglyph4, $25
#endif
	addu	pglyph3, $25
	addu	pglyph2, $25
	addu	pglyph1, $25
	addu	pglyph0, $25
#endif

#if defined(GLYPH10) || defined(GLYPH16)
$1200:
	ulw	glyphbits, 0(pglyph0)
	ulw	$24, 0(pglyph1)
	or	glyphbits, $24
#if defined(GLYPH10)
	ulw	$25, 0(pglyph2)
	or	glyphbits, $25
#endif

	bne	glyphbits, 0, $1220		# line 0 and 1 all 0's?
	addu	pdst, widthDst
	addu	pdst, widthDst
	subu	h, 2
#if defined(GLYPH10)
	addu	pglyph2, 4
#endif
	addu	pglyph1, 4
	addu	pglyph0, 4
	b	$1400

$1220:
	and	$24, glyphbits, 0xffff		# line 0 all 0's?
	bne	$24, 0, $1400
	addu	pdst, widthDst
	subu	h, 1
#if defined(GLYPH10)
	addu	pglyph2, 2
#endif
	addu	pglyph1, 2
	addu	pglyph0, 2
#endif

/* Loop over scanlines of glyph */
$1400:
 #	    do {
 #  	    	glyphbits = Data from pglyph0..pglyph4;
/* Load up n glyphs for this scanline, bump pointers */
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
#endif
#if defined(GLYPH16)
	lhu	glyphbits, 0(pglyph1)
	addu	pglyph1, 2
	lhu	$24, 0(pglyph0)
	addu	pglyph0, 2
#endif
	sll	glyphbits, width
	or	glyphbits, $24

	addu	h, -1			# h--;
	beq	glyphbits, 0, $1700	# Get out as soon as possible if 0
	move	$25, pdst		# p = pdst

 # 	    	$24 = address of instruction block offset from $1600
	sll	$24, glyphbits, align
	and	$24, (15 << 5)
	addu	$24, casetable
	srl	glyphbits, align4	# glyphbits >>= align4;
	.set	noreorder
	j	$24
	and	$24, glyphbits, 15	# Sleazy.  Do next $r24 in case we loop
	
$1500:
	sll	$24, 5
	addu	$24, casetable
	srl	glyphbits, 4		# glyphbits >>= 4;
	.set	noreorder
	j	$24
	and	$24, glyphbits, 15	# Sleazy.  Do next $r24 in case we loop

/*
 * Individual cases [0..15].  Each is exactly 8 instructions long for fast
 * dispatching.  Each case writes to exactly those bytes that have a
 * corresponding 1 set in glyphbits.  Further, each case contains the 
 * ``while glyphbits !=0'' loop termination code to minimize extra jumps.
 */

$1600:	bne 	glyphbits, 0, $1500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $1400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$1800
	subu	nglyph, NGLYPHS		# nglyph -= NGLYPHS
	nop
	nop
$1601:	sb	fgxorbits, 0($25)
	bne 	glyphbits, 0, $1500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $1400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$1800
	subu	nglyph, NGLYPHS		# nglyph -= NGLYPHS
	nop
$1602:	sb	fgxorbits, 1($25)
	bne 	glyphbits, 0, $1500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $1400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$1800
	subu	nglyph, NGLYPHS		# nglyph -= NGLYPHS
	nop
$1603:	sh	fgxorbits, 0($25)
	bne 	glyphbits, 0, $1500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $1400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$1800
	subu	nglyph, NGLYPHS		# nglyph -= NGLYPHS
	nop
$1604:	sb	fgxorbits, 2($25)
	bne 	glyphbits, 0, $1500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $1400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$1800
	subu	nglyph, NGLYPHS		# nglyph -= NGLYPHS
	nop
$1605:	sb	fgxorbits, 0($25)
	sb	fgxorbits, 2($25)
	bne 	glyphbits, 0, $1500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $1400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$1800
	subu	nglyph, NGLYPHS		# nglyph -= NGLYPHS
$1606:	sb	fgxorbits, 1($25)
	sb	fgxorbits, 2($25)
	bne 	glyphbits, 0, $1500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $1400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$1800
	subu	nglyph, NGLYPHS		# nglyph -= NGLYPHS
$1607:	swl	fgxorbits, 2($25)
	bne 	glyphbits, 0, $1500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $1400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$1800
	subu	nglyph, NGLYPHS		# nglyph -= NGLYPHS
	nop
$1608:	sb	fgxorbits, 3($25)
	bne 	glyphbits, 0, $1500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $1400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$1800
	subu	nglyph, NGLYPHS		# nglyph -= NGLYPHS
	nop
$1609:	sb	fgxorbits, 0($25)
	sb	fgxorbits, 3($25)
	bne 	glyphbits, 0, $1500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $1400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$1800
	subu	nglyph, NGLYPHS		# nglyph -= NGLYPHS
$1610:	sb	fgxorbits, 1($25)
	sb	fgxorbits, 3($25)
	bne 	glyphbits, 0, $1500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $1400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$1800
	subu	nglyph, NGLYPHS		# nglyph -= NGLYPHS
$1611:	sh	fgxorbits, 0($25)
	sb	fgxorbits, 3($25)
	bne 	glyphbits, 0, $1500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $1400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$1800
	subu	nglyph, NGLYPHS		# nglyph -= NGLYPHS
$1612:	sh	fgxorbits, 2($25)
	bne 	glyphbits, 0, $1500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $1400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$1800
	subu	nglyph, NGLYPHS		# nglyph -= NGLYPHS
	nop
$1613:	sb	fgxorbits, 0($25)
	sh	fgxorbits, 2($25)
	bne 	glyphbits, 0, $1500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $1400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$1800
	subu	nglyph, NGLYPHS		# nglyph -= NGLYPHS
$1614:	swr	fgxorbits, 1($25)
	bne 	glyphbits, 0, $1500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $1400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$1800
	subu	nglyph, NGLYPHS		# nglyph -= NGLYPHS
	nop
$1615:	sw	fgxorbits, 0($25)
	bne 	glyphbits, 0, $1500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $1400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$1800
	subu	nglyph, NGLYPHS		# nglyph -= NGLYPHS
	nop
	.set	reorder
$1700:
	addu	pdst, widthDst		# pdst += widthDst;
	bgt	h, 0, $1400		# while (h > 0);
	subu	nglyph, NGLYPHS		# nglyph -= NGLYPHS;

$1800:
	addu	pdstBase, widthn	# pdstBase += NGLYPHS * width
	addu	ppci, NGLYPHS*4		# ppci += NGLYPHS
	bge	nglyph, 0, $1100 	# while nglyph >= 0


$2000:	addu	nglyph, NGLYPHS
#endif ndef GLYPH32


/****************************************************************
Phase 2: Paint glyphs one at a time as long as there are any left
*****************************************************************/

	beq	nglyph, 0, $9900

	la	casetable, $2600

$2100:
/* main loop on glyphs one at a time*/
	lw	pci, 0(ppci)			# pci = *ppci;
	lw	pglyph0, glyphBitmap(pci)	# pglyph0 = pci->picture;

 #  	    h = pci->metrics.ascent + pci->metrics.descent;
#ifdef GLYPH32
	lh	$24, ascent(pci)
	lh	$25, descent(pci)
	mult	$24, widthDst			# kick off ascent * widthDst
	addu	h, $24, $25
	lh	$25, leftSideBearing(pci)	# $25 used below again
 #  	    widthGlyph = (pci->metrics.rightSideBearing -
 # 			  pci->metrics.leftSideBearing);
	lh	$24, rightSideBearing(pci)
	subu	widthGlyph, $24, $25
 #  	    /* Compute glyphMask, as high bits of glyphbits are garbage */
	li	glyphMask, 1			# glyphMask = 1's for data
	sll	glyphMask, widthGlyph
	subu	glyphMask, 1

/* Convert widthGlyph from # bits to # bytes */
	addu	widthGlyph, 7
	sra	widthGlyph, 3

 #   	    /* find pixel for top left bit of glyph */
 #  	    pdst = pdstBase - (pci->metrics.ascent * widthDst) 
 #  		+ pci->metrics.leftSideBearing;
	addu	pdst, pdstBase, $25
	mflo	$24				# ascent # widthDst
	subu	pdst, $24
#else
	move	h, height
	move	pdst, pdstBase
#endif

 #  	    /* Align pdst to word boundary.  Keep track of how many extra
 #  	       pixels this adds...we compensate for these pixels by shifting
 #  	       glyphbits left by this amount.  Since we shift in 0's, this
 #  	       doesn't cause anything bad to get written. */

	and	align, pdst, 3			# align = pdst & 3;
	subu	pdst, align			# pdst = pdst & 3;

/* Loop over scanlines of glyph */
$2400:
 #	    do {
#if defined(GLYPH6) || defined(GLYPH8)
	lbu	glyphbits, 0(pglyph0)	# glyphbits = *pglyph0;
	addu	pglyph0, 1		# pglyph0 += widthGlyph;
#endif
#if defined(GLYPH10) || defined(GLYPH16)
	lhu	glyphbits, 0(pglyph0)
	addu	pglyph0, 2
#endif
#if defined(GLYPH32)
	ulw	glyphbits, 0(pglyph0)	# glyphbits = *((Bits32 *) pglyph0);
	and	glyphbits, glyphMask 	# glyphbits &= glyphMask;
	addu	pglyph0, widthGlyph	# pglyph0 += widthGlyph;
#endif
	addu	h, -1			# h--
	beq	glyphbits, 0, $2700	# Get out as soon as possible if 0
 # 	    	/* Adjust glyphbits for unaligned left bits of destination */
	sll	glyphbits, align	# glyphbits <<= align;
	move	$25, pdst		# p = pdst;

 # 		/* Now write 4 bits of glyph at a time to word-aligned *p */
	and	$24, glyphbits, 15
$2500:
	sll	$24, 5
	addu	$24, casetable
	srl	glyphbits, 4		# glyphbits >>= 4;
	.set	noreorder
	j	$24
	and	$24, glyphbits, 15	# Sleazy.  Do next $r24 in case we loop

/*
 * Individual cases [0..15].  Each is exactly 8 instructions long for fast
 * dispatching.  Each case writes to exactly those bytes that have a
 * corresponding 1 set in glyphbits.  Further, each case contains the 
 * ``while glyphbits !=0'' loop termination code to minimize extra jumps.
 */

$2600:	bne 	glyphbits, 0, $2500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $2400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$2800
	addu	nglyph, -1		# nglyph--
	nop
	nop
$2601:	sb	fgxorbits, 0($25)
	bne 	glyphbits, 0, $2500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $2400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$2800
	addu	nglyph, -1		# nglyph--
	nop
$2602:	sb	fgxorbits, 1($25)
	bne 	glyphbits, 0, $2500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $2400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$2800
	addu	nglyph, -1		# nglyph--
	nop
$2603:	sh	fgxorbits, 0($25)
	bne 	glyphbits, 0, $2500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $2400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$2800
	addu	nglyph, -1		# nglyph--
	nop
$2604:	sb	fgxorbits, 2($25)
	bne 	glyphbits, 0, $2500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $2400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$2800
	addu	nglyph, -1		# nglyph--
	nop
$2605:	sb	fgxorbits, 0($25)
	sb	fgxorbits, 2($25)
	bne 	glyphbits, 0, $2500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $2400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$2800
	addu	nglyph, -1		# nglyph--
$2606:	sb	fgxorbits, 1($25)
	sb	fgxorbits, 2($25)
	bne 	glyphbits, 0, $2500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $2400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$2800
	addu	nglyph, -1		# nglyph--
$2607:	swl	fgxorbits, 2($25)
	bne 	glyphbits, 0, $2500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $2400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$2800
	addu	nglyph, -1		# nglyph--
	nop
$2608:	sb	fgxorbits, 3($25)
	bne 	glyphbits, 0, $2500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $2400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$2800
	addu	nglyph, -1		# nglyph--
	nop
$2609:	sb	fgxorbits, 0($25)
	sb	fgxorbits, 3($25)
	bne 	glyphbits, 0, $2500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $2400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$2800
	addu	nglyph, -1		# nglyph--
$2610:	sb	fgxorbits, 1($25)
	sb	fgxorbits, 3($25)
	bne 	glyphbits, 0, $2500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $2400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$2800
	addu	nglyph, -1		# nglyph--
$2611:	sh	fgxorbits, 0($25)
	sb	fgxorbits, 3($25)
	bne 	glyphbits, 0, $2500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $2400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$2800
	addu	nglyph, -1		# nglyph--
$2612:	sh	fgxorbits, 2($25)
	bne 	glyphbits, 0, $2500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $2400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$2800
	addu	nglyph, -1		# nglyph--
	nop
$2613:	sb	fgxorbits, 0($25)
	sh	fgxorbits, 2($25)
	bne 	glyphbits, 0, $2500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $2400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$2800
	addu	nglyph, -1		# nglyph--
$2614:	swr	fgxorbits, 1($25)
	bne 	glyphbits, 0, $2500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $2400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$2800
	addu	nglyph, -1		# nglyph--
	nop
$2615:	sw	fgxorbits, 0($25)
	bne 	glyphbits, 0, $2500	# end while glyphbits
	addu	$25, 4			# p += 4
	bgt	h, 0, $2400		# end while h
	addu	pdst, widthDst		# pdst += widthDst
	b	$2800
	addu	nglyph, -1		# nglyph--
	nop
	.set	reorder
$2700:
	addu	pdst, widthDst		# pdst += widthDst;
	bgt	h, 0, $2400		# while (h > 0);
	addu	nglyph, -1		# nglyph--;
$2800:
 #	    pdstBase += pci->metrics.characterWidth
#ifdef GLYPH8
	addu	pdstBase, width
#else
	lh	$24, characterWidth(pci)
	addu	pdstBase, $24
#endif

 # 	    /* Move ppci to next character info */
	addu	ppci, 4
	bne	nglyph, 0, $2100		#  while nglyph

$9900:
#if defined(GLYPH6)
	lw	$20, 20($sp)
#endif
#if defined(GLYPH6) || defined(GLYPH8)
	lw	$20, 16($sp)
#endif
#if defined(GLYPH6) || defined(GLYPH8) || defined(GLYPH10)
	lw	$19, 12($sp)
#endif
#if defined(GLYPH6) || defined(GLYPH8) || defined(GLYPH10) || defined(GLYPH16)
	lw	$18, 8($sp)
	lw	$17, 4($sp)
	lw	$16, 0($sp)
	addu	$sp, framesize
#endif

 # 118	} /* CFBSPLATGLYPHS */

$9999:
	j	$31
	.end	CFBSPLATGLYPHS
