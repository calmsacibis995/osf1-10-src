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

#include "sfbasm.h"

	.globl	SFBLINES1

 # sfbLineS1(sfb, addrl, nlwidth, pel, nel, pbox);
 # sfbSegS1 (sfb, addrl, nlwidth, pel, nel, pbox, capEnds);

	.ent	SFBLINES1 2
SFBLINES1:
/* Offsets of local variables from sp
----------------------------------------
|                : args to SFBLINES1   |
----------------------------------------  <- lineargoff
| linesavesize   : caller-save regs    |
----------------------------------------  <- linesaveoff
| localsize      : local vars          |
----------------------------------------  <- localoff
| clipsavesize   : callee-save regs    |
----------------------------------------  <- clipsaveoff
| clipargsize    : args to cfbClipLine |
----------------------------------------  <- $sp
*/

	linesavesize  = 40
	localsize     =  8
	clipsavesize  = 36
	clipargsize   = 36
	framesize     = clipargsize + clipsavesize + localsize + linesavesize

	lineargoff    = clipargsize + clipsavesize + localsize + linesavesize
	linesaveoff   = clipargsize + clipsavesize + localsize
	localoff      = clipargsize + clipsavesize
	clipsaveoff   = clipargsize

	.mask	0xC0FF0000, -localsize-4
	.frame	$sp, framesize, $31

/* Scratch temporaries */
#define addr		$2
#define clip		$3

/* Register parameters.  */
#define sfb		$4
#define addrl		$5
#define nlwidth 	$6
#define pel		$7

/* Scratch temporaries */
#define dx		$8
#define dy		$9
#define	pt2x		$10
#define pt2y		$11
#define e		$12
#define pbox		$13
#define e2		$14
#define snlwidth	$15

/* Must save if used registers */
#define clipul		$16
#define cliplr		$17
#define ones		$18
#define signbits	$19
#define pt1x		$20
#define pt1y		$21
#define elEnd		$22
#ifdef SEGMENTS
#define capEnds		$23
#else
#define pt1MayBeOut	$23
#endif

/* Scratch temporaries */
#define len		$25

/* Must save if used registers */
#define signdx		$30
#define signdy		$31

/* Offsets of arguments */
#define linearg_nels	lineargoff+16
#define linearg_pbox	lineargoff+20
#ifdef SEGMENTS
#define linearg_capEnds lineargoff+24
#endif

/* Offsets of locals */
#define	local_clippt1x	localoff+0
#define local_clippt1y	localoff+2
#define local_clippt2x	localoff+4
#define local_clippt2y	localoff+6


	subu	$sp, framesize

 # Save registers
	sw	$16, linesaveoff+ 0($sp)
	sw	$17, linesaveoff+ 4($sp)
	sw	$18, linesaveoff+ 8($sp)
	sw	$19, linesaveoff+12($sp)
	sw	$20, linesaveoff+16($sp)
	sw	$21, linesaveoff+20($sp)
	sw	$22, linesaveoff+24($sp)
	sw	$23, linesaveoff+28($sp)
	sw	$30, linesaveoff+32($sp)
	sw	$31, linesaveoff+36($sp)

#ifndef SEGMENTS
 # Load pt1x, pt1y and start multiply
	lh	pt1y, 2(pel)
	lh	pt1x, 0(pel)
	mult	pt1y, nlwidth
#endif
	
 # Load additional arguments
	lw	$24, linearg_nels($sp)
	beq	$24, 0, $86
	li	signbits, 0x80008000
	sub	$24, 1
#ifdef SEGMENTS
	sll	$24, 3
	addu	elEnd, pel, $24
	lw	capEnds, linearg_capEnds($sp)
#else
	sll	$24, 2
	addu	elEnd, pel, $24
	li	pt1MayBeOut, 1			# if 1, don't know.
						# if 0, definitely in.
#endif

 # Load clip registers.
	lw	pbox, linearg_pbox($sp)
	lw	clipul, 0(pbox)			# clipul = ((int*)pbox)[0];
	lw	cliplr, 4(pbox)			# cliplr = ((int*)pbox)[1]...
	li	ones, 0xfffeffff
	addu	cliplr, ones			# ... - 0x00010001
#if SFBSTIPPLEBITS == 32
	li	ones, 0xffffffff
#elif SFBSTIPPLEBITS == 16
	li	ones, 0x0000ffff
#endif
#if (SFBSTIPPLEBITS < SFBLINEBITS)
	Code won't work properly
#endif


#ifndef SEGMENTS
 # Cram starting line address into sfb register
	SFBPIXELSTOBYTES(pt1x)
	addu	addr, addrl, pt1x
	mflo	$24
	addu	addr, $24
	sw	addr, SFBADDRESS(sfb)
#endif

 # while (pel != elEnd) {
$32:
	lw	pt1x, 0(pel)			# ((int *)pel)[0];
	sra	pt1y, pt1x, 16			# pt1y = pt1x >> 16
#ifdef SEGMENTS
	mult	pt1y, nlwidth			# start multiply for later
#endif
	lw	pt2x, 4(pel)			# ((int *)pel)[1];
	sra	pt2y, pt2x, 16			# pt2y = pt2x >> 16

 # Are both points wholly within clip box?
 # clipCheck = (pt2x-clipul) | (cliplr-pt2x);
	subu	e, pt2x, clipul
	subu	$24, cliplr, pt2x
	or	e, $24
#ifndef SEGMENTS
 # if (pt1MayBeOut != 0) {
	beq	pt1MayBeOut, 0, $37		# Skip pt1 test if already know
#endif

 # clipCheck |= (pt1x-clipul) | (cliplr-pt1x)
	subu	$24, pt1x, clipul
	or	e, $24
	subu	$24, cliplr, pt1x
	or	e, $24

$37:
	sll	snlwidth, nlwidth, 16		# fill jump slot
#ifdef SEGMENTS
	and	e, signbits
	bne	e, 0, CLIPPED
#else
	and	pt1MayBeOut, e, signbits
	bne	pt1MayBeOut, 0, CLIPPED
#endif

	and	pt1x, 0xffff			# get pt1x
	and	pt2x, 0xffff			# get pt2x
	subu	dx, pt2x, pt1x			# dx = pt2x - pt1x;
	subu	dy, pt2y, pt1y			# dy = pt2y - pt1y;

 #     if (dy == 0) {
	bne	dy, 0, $45

#ifndef SEGMENTS
	mult	pt1y, nlwidth
#endif
	li	$24, TRANSPARENTSTIPPLE		# change mode for a moment

 #         /* force line from left to right, keeping endpoint semantics */
#ifdef SEGMENTS
 #         if (dx <= 0) {
	addu	pt2x, capEnds		# pt2x += capEnds;
	bgt	dx, 0, $38
	addu	pt2x, pt1x, 1		# pt2x = pt1x + 1;
	beq	dx, capEnds, $44	# no line if dx, capEnds = 0
	addu	pt1x, pt2x, dx		# pt1x = pt2x + dx - capEnds;
	subu	pt1x, capEnds
$38:
	sw	$24, SFBMODE(sfb)
	SFBPIXELSTOBYTES(pt1x)
	addu	addr, addrl, pt1x	# addr = addrl + pt1x*SFBPIXELBYTES +
	mflo	$24			# ... + pt1y*nlwidth 
	addu	addr, $24
 #         } /* if dx < 0 */
#else
 #         if (dx <= 0) {
	bgt	dx, 0, $38
	addu	pt2x, pt1x, 1		# pt2x = pt1x + 1;
	beq	dx, 0, $44		# no line if dx = 0
	addu	pt1x, pt2x, dx		# pt1x = pt2x + dx;
	li	dx, -1
$38:
	sw	$24, SFBMODE(sfb)
	SFBPIXELSTOBYTES(pt1x)
	addu	addr, addrl, pt1x	# addr = addrl + pt1x*SFBPIXELBYTES +
	mflo	$24			# ... + pt1y*nlwidth 
	addu	addr, $24
	SFBPIXELSTOBYTES(dx)
	addu	pt2y, addr, dx		# next = addr + dx*SFBPIXELBYTES
 #         } /* if dx < 0 */
#endif

 # Paint the line as a span
	subu	dx, pt2x, pt1x		# dx = pt2x - pt1x
	andi	$24, addr, SFBALIGNMASK	# align = addr & SFBALIGNMASK
	sub	addr, $24		# addr -= align
	SFBBYTESTOPIXELS($24)
	add	dx, $24			# len += align
	SFBLEFTSTIPPLEMASK(e, $24, ones) # leftMask
	SFBRIGHTSTIPPLEMASK(e2, dx, ones) # rightMask
	slt	$24, dx, (SFBSTIPPLEBITS+1) # if (width <= SFBSTIPPLEBITS)
	bne	$24, 0, $42

 # Mask requires two words 
	subu	dx, (2*SFBSTIPPLEBITS)	# width -= 2*SFBSTIPPLEBITS;
	sw	e, (addr)		# SFBWRITE(addr, leftMask)
	ble	dx, 0, $40
$39:	addu	addr, SFBSTIPPLEBYTESDONE # addr += SFBSTIPPLEBYTESDONE
	subu	dx, SFBSTIPPLEBITS	# width -= SFBSTIPPLEBITS
	sw	ones, (addr)		# SFBWRITE(addr, ALL1)
	bgt	dx, 0, $39		# while width > 0
$40:	# SFBWRITE(addr+SFBBYTESDONE, rightMask)
	sw	e2, SFBSTIPPLEBYTESDONE(addr)
	j	$43

 # Mask fits in one word
$42:					# SFBWRITE(addr, leftmask & rightMask)
#ifdef TLBFAULTS
	sw	addr, SFBADDRESS(sfb)
	and	$24, e, e2
	sw	$24, SFBSTART(sfb)
#else
	and	$24, e, e2
	sw	$24, (addr)
#endif

 # Put back in line mode, restore pixel address if PolyLine
$43:
#ifndef SEGMENTS
	sw	pt2y, SFBADDRESS(sfb)
#endif
	li	$24, TRANSPARENTLINE
	sw	$24, SFBMODE(sfb)
$44:
	.set	noreorder
	bne	pel, elEnd, $32			# while (pel != elEnd)
#ifdef SEGMENTS
	addu	pel, 8				# pel++;
#else
	addu	pel, 4				# pel++;
#endif
	.set	reorder
	lw	$31, linesaveoff+36($sp)
	b	$87

 #     } else {	/* sloped line */
$45:

/* Now things get pretty gross.  To get maximum speed, we branch into one of
   4 cases based upon 3 tests: dx < 0, dy < 0, and horizontalish/verticalish.
   Further, to avoid as many delay slots and unnecessary negu's as possible,
   we have very similar, but nonetheless separate code for polyline vs.
   polysegment. */

 #	sll	snlwidth, nlwidth, 16	# in jump slot way above

#ifdef SEGMENTS
	SFBPIXELSTOBYTES(pt1x)		# addr = addrl + pt1x*SFBPIXELBYTES +
	addu	addr, addrl, pt1x
	mflo	$24			# ... + pt1y*nlwidth
	addu	addr, $24
	sw	addr, SFBADDRESS(sfb)

 #         if (dx < 0) {
	bge	dx, 0, $57
	negu	dx, dx			# dx = -dx;
	li	signdx, -(SFBPIXELBYTES << 16)

 #             if (dy < 0) {
	bge	dy, 0, $53
 # dx < 0, dy < 0 case
	negu	dy, dy			# dy = - dy;
	subu	e2, dx, dy		# e2 = dx - dy
 #                 if (e2 >= 0) {
	subu	$24, signdx, snlwidth	# SFBBRES2(sfb, (snlwidth+signdx) | e2)
	blt	e2, 0, $50

$47:
 # Paint horizontalish line, dx < 0 case
	or	$24, e2
	sw	$24, SFBBRES2(sfb)
 # e = (dy - e2 - 1) << 14
	subu	e, dy, e2
	subu	e, 1
	sll	e, 14
	or	$24, signdx, dy		# SFBBRES1(sfb, signdx | dy)
	sw	$24, SFBBRES1(sfb)
	addu	dx, capEnds		# dx += capEnds;
	or	$24, e, dx		# SFBBRES3(sfb, e | dx)
	sw	$24, SFBBRES3(sfb)
	
$48:	sw	ones, SFBBRESCONTINUE(sfb)
	sub	dx, SFBLINEBITS
	bgt	dx, 0, $48

	.set	noreorder
	bne	pel, elEnd, $32			# while (pel != elEnd)
	addu	pel, 8				# pel++;
	.set	reorder
	lw	$31, linesaveoff+36($sp)
	b	$87

$50:
 # Paint verticalish line, dy < 0 case
	negu	e2, e2			# e2 = -e2
	or	$24, e2
	sw	$24, SFBBRES2(sfb)
	subu	e, dx, e2		# e = (dx - e2 - 1) << 14
	subu	e, 1
	sll	e, 14
	negu	snlwidth, snlwidth
	or	$24, snlwidth, dx	# SFBBRES1(sfb, snlwidth | dx)
	sw	$24, SFBBRES1(sfb)
	addu	dy, capEnds		# dy += capEnds;
	or	$24, e, dy		# SFBBRES3(sfb, e | dy)
	sw	$24, SFBBRES3(sfb)
	
$51:	sw	ones, SFBBRESCONTINUE(sfb)
	sub	dy, SFBLINEBITS
	bgt	dy, 0, $51

	.set	noreorder
	bne	pel, elEnd, $32			# while (pel != elEnd)
	addu	pel, 8				# pel++;
	.set	reorder
	lw	$31, linesaveoff+36($sp)
	b	$87

$53:
 # dx < 0, dy > 0 case
	subu	e2, dx, dy		# e2 = dx - dy
 #                 if (e2 >= 0) {
	addu	$24, snlwidth, signdx	# SFBBRES2(sfb, (snlwidth+signdx) | e2)
	bge	e2, 0, $47

 # Paint verticalish line, dy > 0 case
	negu	e2, e2
$54:
	or	$24, e2
	sw	$24, SFBBRES2(sfb)
	subu	e, dx, e2		# e = (dx - e2) << 14
	sll	e, 14
	or	$24, snlwidth, dx	# SFBBRES1(sfb, snlwidth | dx)
	sw	$24, SFBBRES1(sfb)
	addu	dy, capEnds		# dy += capEnds;
	or	$24, e, dy		# SFBBRES3(sfb, e | dy)
	sw	$24, SFBBRES3(sfb)
	
$55:	sw	ones, SFBBRESCONTINUE(sfb)
	sub	dy, SFBLINEBITS
	bgt	dy, 0, $55

	.set	noreorder
	bne	pel, elEnd, $32			# while (pel != elEnd)
	addu	pel, 8				# pel++;
	.set	reorder
	lw	$31, linesaveoff+36($sp)
	b	$87

$57: # we know that dx >= 0
 	li	signdx, (SFBPIXELBYTES << 16)
 #             if (dy < 0) {
	bge	dy, 0, $62
 # dx >= 0, dy < 0 case
	negu	dy, dy			# dy = - dy;
	subu	e2, dx, dy		# e2 = dx - dy
 #                 if (e2 >= 0) {
	subu	$24, signdx, snlwidth	# SFBBRES2(sfb, (snlwidth+signdx) | e2)
	blt	e2, 0, $50

$59:
 # Paint horizontalish line, dx >= 0 case
	or	$24, e2
	sw	$24, SFBBRES2(sfb)
 # e = (dy - e2) << 14
	subu	e, dy, e2
	sll	e, 14
	or	$24, signdx, dy		# SFBBRES1(sfb, signdx | dy)
	sw	$24, SFBBRES1(sfb)
	addu	dx, capEnds		# dx += capEnds;
	or	$24, e, dx		# SFBBRES3(sfb, e | dx)
	sw	$24, SFBBRES3(sfb)
	
$60:	sw	ones, SFBBRESCONTINUE(sfb)
	sub	dx, SFBLINEBITS
	bgt	dx, 0, $60

	.set	noreorder
	bne	pel, elEnd, $32			# while (pel != elEnd)
	addu	pel, 8				# pel++;
	.set	reorder
	lw	$31, linesaveoff+36($sp)
	b	$87

$62:
 # dx >= 0, dy > 0 case
	subu	e2, dx, dy		# e2 = dx - dy
 #                 if (e2 >= 0) {
	addu	$24, snlwidth, signdx	# SFBBRES2(sfb, (snlwidth+signdx) | e2)
	bge	e2, 0, $59
 # Paint verticalish line, dy > 0 case
	negu	e2, e2
	b	$54

#else /* lines */

 	li	signdx, (SFBPIXELBYTES << 16)
 #         if (dx < 0) {
	bge	dx, 0, $57

	negu	dx, dx			# dx = -dx;
	negu	signdx, signdx		# signdx = -signdx;

 #             if (dy < 0) {
	bge	dy, 0, $53
 # dx < 0, dy < 0 case
	negu	dy, dy			# dy = - dy;
	negu	snlwidth, snlwidth	# snlwidth = -snlwidth;

	subu	e2, dx, dy		# e2 = dx - dy
 #                 if (e2 >= 0) {
	addu	$24, snlwidth, signdx	# SFBBRES2(sfb, (snlwidth+signdx) | e2)
	blt	e2, 0, $50

$47:
 # Paint horizontalish line, dx < 0 case
	or	$24, e2
	sw	$24, SFBBRES2(sfb)
 # e = (dy - e2 - 1) << 14
	subu	e, dy, e2
	subu	e, 1
	sll	e, 14
	or	$24, signdx, dy		# SFBBRES1(sfb, signdx | dy)
	sw	$24, SFBBRES1(sfb)
	or	$24, e, dx		# SFBBRES3(sfb, e | dx)
	sw	$24, SFBBRES3(sfb)
	
$48:	sw	ones, SFBBRESCONTINUE(sfb)
	sub	dx, SFBLINEBITS
	bgt	dx, 0, $48

	.set	noreorder
	bne	pel, elEnd, $32			# while (pel != elEnd)
	addu	pel, 4				# pel++;
	.set	reorder
	lw	$31, linesaveoff+36($sp)
	b	$87

$50:
 # Paint verticalish line, dy < 0 case
	negu	e2, e2			# e2 = -e2
	or	$24, e2
	sw	$24, SFBBRES2(sfb)
	subu	e, dx, e2		# e = (dx - e2 - 1) << 14
	subu	e, 1
	sll	e, 14
	or	$24, snlwidth, dx	# SFBBRES1(sfb, snlwidth | dx)
	sw	$24, SFBBRES1(sfb)
	or	$24, e, dy		# SFBBRES3(sfb, e | dy)
	sw	$24, SFBBRES3(sfb)
	
$51:	sw	ones, SFBBRESCONTINUE(sfb)
	sub	dy, SFBLINEBITS
	bgt	dy, 0, $51

	.set	noreorder
	bne	pel, elEnd, $32			# while (pel != elEnd)
	addu	pel, 4				# pel++;
	.set	reorder
	lw	$31, linesaveoff+36($sp)
	b	$87

$53:
 # dx < 0, dy > 0 case
	subu	e2, dx, dy		# e2 = dx - dy
 #                 if (e2 >= 0) {
	addu	$24, snlwidth, signdx	# SFBBRES2(sfb, (snlwidth+signdx) | e2)
	bge	e2, 0, $47

 # Paint verticalish line, dy > 0 case
	negu	e2, e2
$54:
	or	$24, e2
	sw	$24, SFBBRES2(sfb)
	subu	e, dx, e2		# e = (dx - e2) << 14
	sll	e, 14
	or	$24, snlwidth, dx	# SFBBRES1(sfb, snlwidth | dx)
	sw	$24, SFBBRES1(sfb)
	or	$24, e, dy		# SFBBRES3(sfb, e | dy)
	sw	$24, SFBBRES3(sfb)
	
$55:	sw	ones, SFBBRESCONTINUE(sfb)
	sub	dy, SFBLINEBITS
	bgt	dy, 0, $55

	.set	noreorder
	bne	pel, elEnd, $32			# while (pel != elEnd)
	addu	pel, 4				# pel++;
	.set	reorder
	lw	$31, linesaveoff+36($sp)
	b	$87

$57: # we know that dx >= 0
 #             if (dy < 0) {
	bge	dy, 0, $62
 # dx >= 0, dy < 0 case
	negu	dy, dy			# dy = - dy;
	negu	snlwidth, snlwidth	# snlwidth = -snlwidth;

	subu	e2, dx, dy		# e2 = dx - dy
 #                 if (e2 >= 0) {
	addu	$24, snlwidth, signdx	# SFBBRES2(sfb, (snlwidth+signdx) | e2)
	blt	e2, 0, $50

$59:
 # Paint horizontalish line, dx >= 0 case
	or	$24, e2
	sw	$24, SFBBRES2(sfb)
 # e = (dy - e2) << 14
	subu	e, dy, e2
	sll	e, 14
	or	$24, signdx, dy		# SFBBRES1(sfb, signdx | dy)
	sw	$24, SFBBRES1(sfb)
	or	$24, e, dx		# SFBBRES3(sfb, e | dx)
	sw	$24, SFBBRES3(sfb)
	
$60:	sw	ones, SFBBRESCONTINUE(sfb)
	sub	dx, SFBLINEBITS
	bgt	dx, 0, $60

	.set	noreorder
	bne	pel, elEnd, $32			# while (pel != elEnd)
	addu	pel, 4				# pel++;
	.set	reorder
	lw	$31, linesaveoff+36($sp)
	b	$87

$62:
 # dx >= 0, dy > 0 case
	subu	e2, dx, dy		# e2 = dx - dy
 #                 if (e2 >= 0) {
	addu	$24, snlwidth, signdx	# SFBBRES2(sfb, (snlwidth+signdx) | e2)
	bge	e2, 0, $59
 # Paint verticalish line, dy > 0 case
	negu	e2, e2
	b	$54

#endif /* if segments else lines */


 # 	   } else if (((oc1 = OUTCODES(oc1, pt1x, pt1y, pbox)) &
 # 			(oc2 = OUTCODES(oc2, pt2x, pt2y, pbox))) == 0) {
CLIPPED:

	lh	pt1x, 0(pel)		# Need signed values, may by < 0
	lh	pt2x, 4(pel)

 # For now, we'll compute oc1 and oc2 into e and e2.

	and	clip, clipul, 0xffff	# Clip against pbox->x1
	slt	$24, pt1x, clip
	sll	e, $24, 3
	slt	$24, pt2x, clip
	sll	e2, $24, 3

	sra	clip, clipul, 16	# Clip against pbox->y1
	slt	$24, pt1y, clip
	sll	$24, 1
	or	e, $24
	slt	$24, pt2y, clip
	sll	$24, 1
	or	e2, $24

	and	clip, cliplr, 0xffff	# Clip against pbox->x2
	slt	$24, clip, pt1x
	sll	$24, 2
	or	e, $24
	slt	$24, clip, pt2x
	sll	$24, 2
	or	e2, $24

	sra	clip, cliplr, 16	# Clip against pbox->y2
	slt	$24, clip, pt1y
	or	e, $24
	slt	$24, clip, pt2y
	or	e2, $24

	and	$24, e, e2		# Trivial clip reject?
	li	signdx, 1		# signdx = +/- 1
	bne	$24, 0, $85

	sh	pt1x, local_clippt1x($sp)	# clippt1.x = pt1x;
	sh	pt1y, local_clippt1y($sp)	# clippt1.y = pt1y;
	sh	pt2x, local_clippt2x($sp)	# clippt2.x = pt2x;
	sh	pt2y, local_clippt2y($sp)	# clippt2.y = pt2y;

	subu	dx, pt2x, pt1x			# dx = pt2x - pt1x;
	subu	dy, pt2y, pt1y			# dy = pt2y - pt1y;

	bge	dx, 0, $70
	negu	dx, dx
	negu	signdx, signdx

$70:	li	signdy, 1		# signdy = +/- 1
	# sll	snlwidth, nlwidth, 16	# snlwidth = nlwidth << 16
	bge	dy, 0, $71
	negu	dy, dy
	negu	signdy, signdy
	negu	snlwidth, snlwidth

$71:
 # 	       if (cfbClipLine(pbox, &clippt1, &clippt2,
 # 			oc1, oc2, dx, dy, signdx, signdy)> 0) {

 # Save any registers in 2-15, 24-25, 31 that contain valuable data
	sw	$4, clipsaveoff+ 0($sp)	# sfb
	sw	$5, clipsaveoff+ 4($sp)	# addrl
	sw	$6, clipsaveoff+ 8($sp)	# nlwidth
	sw	$7, clipsaveoff+12($sp)	# pel
	sw	$8, clipsaveoff+16($sp)	# dx
	sw	$9, clipsaveoff+20($sp)	# dy
	sw	$15,clipsaveoff+24($sp)	# snlwidth

 # Save registers that we'll need later, but not restore immediately
	sw	e, clipsaveoff+28($sp)	# oc1
	sw	e2, clipsaveoff+32($sp)	# oc2

 # Load up arguments to cfbClipLine
	lw	$4, linearg_pbox($sp)	# pbox
	addu	$5, $sp, local_clippt1x	# &clippt1
	addu	$6, $sp, local_clippt2x	# &clippt2
	move	$7, e			# oc1
	sw	e2, 16($sp)		# oc2
	sw	dx, 20($sp)		# dx
	sw	dy, 24($sp)		# dy
	sw	signdx, 28($sp)		# signdx
	sw	signdy, 32($sp)		# signdy
	jal	cfbClipLine

 # Reload saved registers that should always be valid
	lw	$4, clipsaveoff+ 0($sp)	# sfb
	lw	$5, clipsaveoff+ 4($sp)	# addrl
	lw	$6, clipsaveoff+ 8($sp)	# nlwidth
	lw	$7, clipsaveoff+12($sp)	# pel
	lw	$13, linearg_pbox($sp)	# pbox

 # Is the line at least partially contained?
	ble	$2, 0, $85

 # Reload a few more registers (all but oc1, oc2)
	lw	$8, clipsaveoff+16($sp)	# dx
	lw	$9, clipsaveoff+20($sp)	# dy
	lw	$15,clipsaveoff+24($sp)	# snlwidth

	lh	pt2x, local_clippt1x($sp)	# pt2x = clippt1.x
	lh	pt2y, local_clippt1y($sp)	# pt2y = clippt1.y
	multu	pt2y, nlwidth			# start multiply

	sll	signdx, SFBLINESHIFT	# signdx <<= SFBLINESHIFT
	subu	e2, dx, dy		# e2 = dx - dy
	blt	e2, 0, $77		# if e2 >= 0

 # Horizontalish line
	or	$24, signdx, dy		# SFBBRES1(sfb, signdx | dy)
	sw	$24, SFBBRES1(sfb)
	addu	$24, snlwidth, signdx	# SFBBRES2(sfb, (snlwidth+signdx) | e2)
	or	$24, e2
	sw	$24, SFBBRES2(sfb)
	subu	e, dy, e2		# e = dy - e2
	lw	$24, clipsaveoff+32($sp)	# Get ready for (oc2 != 0)...
	lh	len, local_clippt2x($sp)	# len = cliptt2.x - pt2x
	subu	len, pt2x
	bge	len, 0, $78		# if (len < 0)
	subu	e, 1			# e--
	negu	len, len		# len = -len
	b	$78

$77:
 # Verticalish line
	or	$24, snlwidth, dx	# SFBBRES1(sfb, snlwidth | dx)
	sw	$24, SFBBRES1(sfb)
	addu	$24, snlwidth, signdx	# SFBBRES2(sfb, (snlwidth+signdx) | -e2)
	negu	e, e2
	or	$24, e
	sw	$24, SFBBRES2(sfb)
	addu	e, dx, e2		# e = dx + e2
	lw	$24, clipsaveoff+32($sp)	# Get ready for (oc2 != 0)...
	lh	len, local_clippt2y($sp)	# len = clippt2.y - pt2y;
	subu	len, pt2y
	bge	len, 0, $78		# if (len < 0)
	subu	e, 1			# e--
	negu	len, len		# len = -len

$78:
	sra	e, 1			# e >>= 1
#ifdef SEGMENTS
	sne	$24, $24, 0		# len += ((oc2 != 0) | capEnds)
	or	$24, capEnds
	addu	len, $24
#else
	sne	$24, $24, 0		# len += (oc2 != 0);
	addu	len, $24
#endif    

	SFBPIXELSTOBYTES(pt2x)		# addr = addrl + pt2x*SFBPIXELBYTES +
	addu	addr, addrl, pt2x
	mflo	$24			# ... + pt2y*nlwidth
	addu	addr, $24
	lw	$24, clipsaveoff+28($sp)	# oc1
	sw	addr, SFBADDRESS(sfb)		# SFBADDRESS(addr)
	
 # 		   if (len) {
	beq	len, 0, $85

 # 	/* unwind bresenham error term to first point */
 # 			if (oc1) {
	beq	$24, 0, $83

	subu	pt1x, pt2x, pt1x	# pt1x = pt2x - pt1x;
	subu	pt1y, pt2y, pt1y	# pt1y = pt2y - pt1y;
	bge	pt1x, 0, $80		# pt1x = abs(pt1x)
	negu	pt1x, pt1x
$80:
	bge	pt1y, 0, $81		# pt1y = abs(pt1y)
	negu	pt1y, pt1y
$81:
	blt	e2, 0, $82
 # Horizontalish error compensation
	mul	$24, pt1x, dy		# e += pt1x*dy - pt1y*dx;
	addu	e, $24
	mul	$24, pt1y, dx
	subu	e, $24
	b	$83
$82:
 # Verticalish error compensation
	mul	$24, pt1y, dx		# e += pt1y*dx - pt1x*dy;
	addu	e, $24
	mul	$24, pt1x, dy
	subu	e, $24

$83:
/* Okay, finally draw the sucker */
	sll	e, 15			# SFBBRES3((e << 15) | len)
	or	e, len
	sw	e, SFBBRES3(sfb)

$84:
	sw	ones, SFBBRESCONTINUE(sfb)
	subu	len, SFBLINEBITS
	bgt	len, 0, $84

$85:
	.set	noreorder
	bne	pel, elEnd, $32		# while (pel != elEnd)
#ifdef SEGMENTS
	addu	pel, 8			# pel++;
#else
	addu	pel, 4			# pel++;
#endif
	.set	reorder
$86:
	lw	$31, linesaveoff+36($sp)

$87:
	lw	$16, linesaveoff+ 0($sp)
	lw	$17, linesaveoff+ 4($sp)
	lw	$18, linesaveoff+ 8($sp)
	lw	$19, linesaveoff+12($sp)
	lw	$20, linesaveoff+16($sp)
	lw	$21, linesaveoff+20($sp)
	lw	$22, linesaveoff+24($sp)
	lw	$23, linesaveoff+28($sp)
	lw	$30, linesaveoff+32($sp)
	addu	$sp, framesize
	j	$31
	.end	CFBLINES1
