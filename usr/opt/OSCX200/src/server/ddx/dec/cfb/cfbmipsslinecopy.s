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

	.globl	CFBLINES1

 # CFBLINES1(addrl, nlwidth, fgandbits, fgxorbits,
 # 	     nels, elInit, capEnds, pbox)

	.ent	CFBLINES1 2
CFBLINES1:
/* Offsets of local variables from sp
----------------------------------------
|                : args to CFBLINES1   |
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
	clipargsize   = 40
	framesize     = clipargsize + clipsavesize + localsize + linesavesize

	lineargoff    = clipargsize + clipsavesize + localsize + linesavesize
	linesaveoff   = clipargsize + clipsavesize + localsize
	localoff      = clipargsize + clipsavesize
	clipsaveoff   = clipargsize

	.mask	0xC0FF0000, -localsize-4
	.frame	$sp, framesize, $31

/* Scratch temporaries */
#define addr		$2
#define verticalish	$3

/* Register parameters.  */
#define addrl		$4
#define nlwidth 	$5
#define nels		$6
#define fgxorbits	$7

/* Scratch temporaries */
#define dx		$8
#define dy		$9
#define	pt2x		$10
#define pt2y		$11
#define e		$12
#define e1		$13
#define e2		$14
#define snlwidth	$15

/* Must save if used registers */
#define clipx1		$16
#define clipy1		$17
#define clipx2		$18
#define clipy2		$19
#define pt1x		$20
#define pt1y		$21
#define pel		$22
#ifdef SEGMENTS
#define capEnds		$23
#else
#define pt1InRect	$23
#endif

/* Scratch temporaries */
#define len		$25

/* Must save if used registers */
#define signdx		$30
#define signdy		$31

/* Offsets of arguments */
#define linearg_nels	lineargoff+16
#define linearg_elInit	lineargoff+20
#define linearg_capEnds lineargoff+24
#define linearg_pbox	lineargoff+28

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

 # Load additional arguments
	lw	nels, linearg_nels($sp)
#ifdef SEGMENTS
	lw	capEnds, linearg_capEnds($sp)
#else
	li	pt1InRect, 0			# if 1, pt1 is in clip rect
						# if 0, it may or may not be
#endif

 # Load clip registers.
	lw	$24, linearg_pbox($sp)
	lh	clipx1, 0($24)			# clipx1 = pbox->x1;
	lh	clipy1, 2($24)			# clipy1 = pbox->y1;
	lh	clipx2, 4($24)			# clipx2 = pbox->x2;
	lh	clipy2, 6($24)			# clipy2 = pbox->y2;

	lw	pel, linearg_elInit($sp)	# pel = elInit;
 # while (nels) {
	beq	nels, 0, $86
$32:
	subu	nels, 1				# nels--;
$90:
#ifdef SEGMENTS
	lh	pt1x, 0(pel)			# pt1x = pel->x1;
	lh	pt1y, 2(pel)			# pt1y = pel->y1;
	lh	pt2x, 4(pel)			# pt2x = pel->x2;
	lh	pt2y, 6(pel)			# pt2y = pel->y2;
	addu	pel, 8				# pel++;
#else
	lh	pt1x, 0(pel)			# pt1x = pel->x;
	lh	pt1y, 2(pel)			# pt1y = pel->y;
	addu	pel, 4				# pel++;
	lh	pt2x, 0(pel)			# pt2x = pel->x;
	lh	pt2y, 2(pel)			# pt2y = pel->y;

#endif
	subu	dx, pt2x, pt1x			# dx = pt2x - pt1x;
	subu	dy, pt2y, pt1y			# dy = pt2y - pt1y;
 #     if (dx == 0) {
	bne	dx, 0, $37
 #         /* force line from top to bottom, keeping endpoint semantics */
#ifdef SEGMENTS
 #        /* Assume already top to bottom for better scheduling */
	addu	pt2y, capEnds		# pt2y += capEnds;
 #         if (dy < 0) {
	bge	dy, 0, $33
	addu	pt2y, pt1y, 1		# pt2y = pt1y + 1;
	addu	pt1y, pt2y, dy		# pt1y = pt2y + dy - capEnds;
	subu	pt1y, capEnds
 #         } /* end if dy < 0 */
#else
 #         if (dy < 0) {
	bge	dy, 0, $33
	addu	pt2y, pt1y, 1		# pt2y = pt1y + 1;
	addu	pt1y, pt2y, dy		# pt1y = pt2y + dy;
 #         } /* end if dy < 0 */
#endif


$33:
 # ******* NOTE WELL.  SET NOREORDER TO AVOID NOPS *************
	.set	noreorder
 # This is really out of sequence, but the idea is to get pt1y settled as
 # quickly as possible, so we can multiply as early as possible.

	slt	$24, pt1y, clipy1	# if (pt1y < clipy1) pt1y = clipy1;
	beq	$24, 0, $34
	slt	$24, pt1x, clipx1
	move	pt1y, clipy1
#ifndef SEGMENTS
	li	pt1InRect, 0
#endif

$34:
	mult	pt1y, nlwidth		# Kick off multiply for below.
 #         if (pt1x >= clipx1 && pt1x < clipx2) {
	bne	$24, 0, UnknownClip1
	slt	$24, pt1x, clipx2
	beq	$24, 0, UnknownClip1
 #             /* Infinite vertical line intersects clip box */
	slt	$24, clipy2, pt2y	# if (pt2y > clipy2) pt2y = clipy2;
	beq	$24, 0, $35
	addu	addr, addrl, pt1x	# addr = addrl + pt1x;
	move	pt2y, clipy2
#ifndef SEGMENTS
	li	pt1InRect, 0
#endif

$35:
 #             if (pt1y < pt2y) {
	slt	$24, pt1y, pt2y
	beq	$24, 0, $85
	mflo	$24
 # 	           /* The actual segment intersects clip box, draw it */
	addu	addr, $24
$36:
 #                 do {
	sb	fgxorbits, 0(addr)	# MYCFBFILL(addr);
	addu	pt1y, 1			# pt1y++;
 # 	           } while (pt1y != pt2y);
	bne	pt1y, pt2y, $36
	addu	addr, nlwidth		# addr += nlwidth;
 #             } /* if anything to paint vertically */
 #         } /* if vertical intersection */

	bne	nels, 0, $90
	subu	nels, 1
	b	$87
	lw	$31, linesaveoff+36($sp)

 #     } else if (dy == 0) {
$37:
	bne	dy, 0, $45
 	li	signdx, 1		# signdx = 1;
 #         /* force line from left to right, keeping endpoint semantics */
#ifdef SEGMENTS
 #         if (dx < 0) {
	addu	pt2x, capEnds		# pt2x += capEnds;
	bge	dx, 0, $38
	slt	$24, pt1y, clipy1	# for clipping later
	addu	pt2x, pt1x, 1		# pt2x = pt1x + 1;
	addu	pt1x, pt2x, dx		# pt1x = pt2x + dx - capEnds;
	subu	pt1x, capEnds
 #         } /* if dx < 0 */
#else
 #         if (dx < 0) {
	bge	dx, 0, $38
	slt	$24, pt1y, clipy1	# for clipping later
	addu	pt2x, pt1x, 1		# pt2x = pt1x + 1;
	addu	pt1x, pt2x, dx		# pt1x = pt2x + dx;
 #         } /* if dx < 0 */
#endif

$38:
	mult	pt1y, nlwidth		# Kick off mult for  below
 #         if (pt1y >= clipy1 && pt1y < clipy2) {
	bne	$24, 0, UnknownClip1
	slt	$24, pt1y, clipy2
	beq	$24, 0, UnknownClip1
 # 	       /* Infinite horizontal line intersects clip box */
	slt	$24, pt1x, clipx1	# if (pt1x < clipx1) pt1x = clipx1;
	beq	$24, 0, $39
	slt	$24, clipx2, pt2x
	move	pt1x, clipx1
#ifndef SEGMENTS
	li	pt1InRect, 0
#endif

$39:
	beq	$24, 0, $40		# if (pt2x > clipx2) pt2x = clipx2;
	addu	addr, addrl, pt1x	# addr = addrl + pt1y*nlwidth + pt1x...
	move	pt2x, clipx2
#ifndef SEGMENTS
	li	pt1InRect, 0
#endif

$40:
 #	       if (dx > 0) {
	subu	dx, pt2x, pt1x
	ble	dx, 0, $85
 #	           /* The actual segment intersects clip box, draw it */
	mflo	$24			# ...addr = addrl + pt1y*nlwidth + pt1x
	addu	addr, $24
 # 	           /* Write bytes until aligned to word boundary */
 # 	           while (((int) addr) & 3) {
	and	$24, addr, 3
	beq	$24, $0, $42
	addu	dx, -1			# dx--; (Predec makes loop faster)
$41:
 # 		       if (dx == 0) goto HORIZDONE0;
	beq	dx, 0, $85
	sb	fgxorbits, 0(addr)	# MYCFBFILL(addr);
 # 	           } /* end while not word aligned */
	addu	addr, 1			# addr++;
	andi	$24, addr, 3
	bne	$24, $0, $41
	addu	dx, -1
$42:
	subu	dx, 3			# dx -= 3; (Really -1-3 = -4)
 #                 while (dx > 0) {
	ble	dx, 0, $44
	addu	$24, addr, dx
$43:
	sw	fgxorbits, 0(addr)	# MYCFBFILL((Pixel32 *)addr);
	subu	dx, 4			# dx -= 4;
 # 	           } /* while paint words */
	bgt	dx, 0, $43
	addu	addr, 4			# addr += 4;

$44:
 #                 /* Write final 1-4 bytes */
	swl	fgxorbits, 3($24)	# CFBFILLLEFT(addr + 3 + dx);
 #             } /* if anything to paint horizontally */
HORIZDONE0:
 #         } /* if horizontal intersection */
	bne	nels, 0, $90
	subu	nels, 1
	b	$87
	lw	$31, linesaveoff+36($sp)

 #     } else {	/* sloped line */

 # ******* NOTE WELL.  SET REORDER TO AVOID BRAIN DAMAGE *************
	.set	reorder
$45:
	mult	pt1y, nlwidth		# Kick off multiply for below

 #	li	signdx, 1		# signdx = 1; (in jump slot way above)
 #         if (dx < 0) {
	bge	dx, 0, $46
	negu	dx, dx			# dx = -dx;
	li	signdx, -1		# signdx = -1;
 #         } /* if dx < 0 */
$46:
	li	signdy, 1		# signdy = 1;
	move	snlwidth, nlwidth	# snlwidth = nlwidth;
 #         if (dy < 0) {
	bge	dy, 0, $47
	negu	dy, dy			# dy = - dy;
	li	signdy, -1		# signdy = -1;
	negu	snlwidth, snlwidth	# snlwidth = -snlwidth;
 #         } /* if dy < 0 */

$47:
 #         if (pt1x >= clipx1 & pt2x >= clipx1 &
 # 		pt1x < clipx2 & pt2x < clipx2 &
 # 		pt1y >= clipy1 & pt2y >= clipy1 &
 # 		pt1y < clipy2 & pt2y < clipy2) {

 # ******* NOTE WELL.  SET NOREORDER TO AVOID NOPS *************
	.set	noreorder

	slt	$24, pt2x, clipx1
	bne	$24, 0, $74
	slt	$24, pt2x, clipx2
	beq	$24, 0, $74
	slt	$24, pt2y, clipy1
	bne	$24, 0, $74
	slt	$24, pt2y, clipy2
	beq	$24, 0, $74
#ifndef SEGMENTS
	addu	addr, addrl, pt1x	# expect pt1InRect to be 1, and thus
	bne	pt1InRect, 0, $475	# line is completely in clip rect
	mflo	$24
#endif
	slt	$24, pt1x, clipx1
	bne	$24, 0, $74
	slt	$24, pt1x, clipx2
	beq	$24, 0, $74
	slt	$24, pt1y, clipy1
	bne	$24, 0, $74
	slt	$24, pt1y, clipy2
	beq	$24, 0, $74
#ifdef SEGMENTS
	addu	addr, addrl, pt1x
#else
	li	pt1InRect, 1
#endif
	mflo	$24


 # ******* NOTE WELL.  SET REORDER TO AVOID BRAIN DAMAGE *************
	.set	reorder
$475:
	addu	addr, $24

 #             if (dx >= dy) {
	blt	dx, dy, $65
	addu	e1, dy, dy		# e1 = 2*dy;
	negu	e, dx			# e = -dx;
	subu	e2, e, dx		# e2 = -2*dx

#ifdef SEGMENTS
	addu	dx, capEnds		# dx += capEnds;
#endif

DRAWHORIZONTALISH:
	subu	dx, 1			# dx--;
	and	$24, dx, 3		# tlen = dx & 3;

 #                 if (signdx > 0) {
	ble	signdx, 0, $56
 # 		       /* Horizontal increments 1 pixel,
 # 			   vertical increments 0 or 1*/
 # 		       /* Get to where we can do 4 at a time */
	subu	dx, $24			# dx -= tlen;
 # 		       while (tlen > 0) {
	ble	$24, 0, $50
$48:
	sb	fgxorbits, 0(addr)	# CPHW(0);
	addu	e, e1
	blt	e, 0, $49
	addu	addr, snlwidth
	addu	e, e2
$49:
	addu	addr, 1			# addr++;
	addu	$24, -1			# tlen--;
 # 		       } /* end while tlen > 0 */
	bgt	$24, 0, $48
$50:
 # 		       /* Do 4 bytes at a time now */
 # 		       while (dx > 0) {
	ble	dx, 0, $555
$51:
 # 			   CPHW(0); CPHW(1); CPHW(2); CPHW(3);
	sb	fgxorbits, 0(addr)
	addu	e, e1
	blt	e, 0, $52
	addu	addr, snlwidth
	addu	e, e2
$52:
	sb	fgxorbits, 1(addr)
	addu	e, e1
	blt	e, 0, $53
	addu	addr, snlwidth
	addu	e, e2
$53:
	sb	fgxorbits, 2(addr)
	addu	e, e1
	blt	e, 0, $54
	addu	addr, snlwidth
	addu	e, e2
$54:
	sb	fgxorbits, 3(addr)
	addu	e, e1
	blt	e, 0, $55
	addu	addr, snlwidth
	addu	e, e2
$55:
	addu	addr, 4			# addr += 4;
	subu	dx, 4			# dx -= 4;
 # 		       } while (dx > 0);
	bgt	dx, 0, $51

$555:
	sb	fgxorbits, 0(addr)	# do last pixel in line
	bne	nels, 0, $32
	lw	$31, linesaveoff+36($sp)
	b	$87
 # 		   } else {
$56:
 # 		       /* Horizontal decrements 1 pixel,
 # 			vertical increments 0 or 1 */
	subu	dx, $24			# dx -= tlen;
 # 		       while (tlen > 0) {
	ble	$24, 0, $59
$57:
	sb	fgxorbits, 0(addr)	# CNHW(0);
	addu	e, e1
	ble	e, 0, $58
	addu	addr, snlwidth
	addu	e, e2
$58:
	subu	addr, 1			# addr--;
	subu	$24, 1			# tlen--;
 # 		       } /* while tlen > 0 */
	bgt	$24, 0, $57
$59:
 # 		       while (dx > 0) {
	ble	dx, 0, $645
$60:
 # 			   CNHW(0);  CNHW(-1);  CNHW(-2);  CNHW(-3);
	sb	fgxorbits, 0(addr)
	addu	e, e1
	ble	e, 0, $61
	addu	addr, snlwidth
	addu	e, e2
$61:
	sb	fgxorbits, -1(addr)
	addu	e, e1
	ble	e, 0, $62
	addu	addr, snlwidth
	addu	e, e2
$62:
	sb	fgxorbits, -2(addr)
	addu	e, e1
	ble	e, 0, $63
	addu	addr, snlwidth
	addu	e, e2
$63:
	sb	fgxorbits, -3(addr)
	addu	e, e1
	ble	e, 0, $64
	addu	addr, snlwidth
	addu	e, e2
$64:
	addu	addr, -4		# addr -= 4;
	addu	dx, -4			# dx -= 4;
 # 		       } /* while dx > 0 */;
	bgt	dx, 0, $60

$645:
	sb	fgxorbits, 0(addr)	# do last pixel in line
	bne	nels, 0, $32
	lw	$31, linesaveoff+36($sp)
	b	$87

 # 		   } /* end backwards horizontalish line */
 # 	       } /* end horizontalish line */
 # 	   } else {

 #	       /* Verticalish line */
$65:
	addu	e1, dx, dx		# e1 = 2*dx;
	negu	e, dy			# e = -dy;
	subu	e2, e, dy		# e2 = -2*dy;

#ifdef SEGMENTS
	addu	dy, capEnds		# dy += capEnds;
#endif

DRAWVERTICALISH:
	subu	dy, 1			# dy--;

 # 	       /* Set up to use same test for both signdx */
 # 263			    
	sgt	$24, signdx, 0		# e += (signdx > 0);
	addu	e, signdx
 # 	       /* Horizontal move 0 or +/- 1 pixels,
 # 		       vertical increments 1 */
	and	$24, dy, 3		# tlen = dy & 3;
	subu	dy, $24			# dy -= tlen;
 # 		   while (tlen > 0) {
	ble	$24, 0, $68
$66:
	sb	fgxorbits, 0(addr)	# CVW();
	addu	addr, snlwidth
	addu	e, e1
	ble	e, 0, $67
	addu	addr, signdx
	addu	e, e2
$67:
	addu	$24, -1			# tlen--;
 # 		   } /* end while tlen > 0 */
	bgt	$24, 0, $66
$68:
 # 	           while (dy > 0) {
	ble	dy, 0, $735
$69:
 # 		       CVW();  CVW();  CVW();  CVW();
	sb	fgxorbits, 0(addr)
	addu	addr, snlwidth
	addu	e, e1
	ble	e, 0, $70
	addu	addr, signdx
	addu	e, e2
$70:
	sb	fgxorbits, 0(addr)
	addu	addr, snlwidth
	addu	e, e1
	ble	e, 0, $71
	addu	addr, signdx
	addu	e, e2
$71:
	sb	fgxorbits, 0(addr)
	addu	addr, snlwidth
	addu	e, e1
	ble	e, 0, $72
	addu	addr, signdx
	addu	e, e2
$72:
	sb	fgxorbits, 0(addr)
	addu	addr, snlwidth
	addu	e, e1
	ble	e, 0, $73
	addu	addr, signdx
	addu	e, e2
$73:
	addu	dy, -4			# dy -= 4;
 # 		   } /* end while (dy > 0) */;
	bgt	dy, 0, $69
 # 	       } /* end if unclipped sloped line */

$735:
	sb	fgxorbits, 0(addr)	# do last pixel in line
	bne	nels, 0, $32
	lw	$31, linesaveoff+36($sp)
	b	$87

 # 	   } else if (((oc1 = OUTCODES(oc1, pt1x, pt1y, pbox)) &
 # 			(oc2 = OUTCODES(oc2, pt2x, pt2y, pbox))) == 0) {
$74:
#ifndef	SEGMENTS
	li	pt1InRect, 0
#endif

 # For now, we'll compute oc1 and oc2 into e1 and e2.

	slt	$24, pt1x, clipx1
	sll	e1, $24, 3
	sge	$24, pt1x, clipx2
	sll	$24, 2
	or	e1, $24
	slt	$24, pt1y, clipy1
	sll	$24, 1
	or	e1, $24
	sge	$24, pt1y, clipy2
	or	e1, $24
	slt	$24, pt2x, clipx1
	sll	e2, $24, 3
	sge	$24, pt2x, clipx2
	sll	$24, 2
	or	e2, $24
	slt	$24, pt2y, clipy1
	sll	$24, 1
	or	e2, $24
	sge	$24, pt2y, clipy2
	or	e2, $24
	and	$24, e1, e2
	bne	$24, 0, $84

	sh	pt1x, local_clippt1x($sp)	# clippt1.x = pt1x;
	sh	pt1y, local_clippt1y($sp)	# clippt1.y = pt1y;
	sh	pt2x, local_clippt2x($sp)	# clippt2.x = pt2x;
	sh	pt2y, local_clippt2y($sp)	# clippt2.y = pt2y;

	slt	verticalish, dx, dy		# verticalish = (dx < dy);

 # 	       if (cfbClipLine(pbox, &clippt1, &clippt2,
 # 			oc1, oc2, dx, dy, signdx, signdy, verticalish)> 0) {

 # Save any registers in 2-15, 24-25, 31 that contain valuable data
	sw	$4, clipsaveoff+ 0($sp)		# addrl
	sw	$5, clipsaveoff+ 4($sp)		# nlwidth
	sw	$6, clipsaveoff+ 8($sp)		# nels
	sw	$7, clipsaveoff+12($sp)		# fgxorbits
	sw	$8, clipsaveoff+16($sp)		# dx
	sw	$9, clipsaveoff+20($sp)		# dy
	sw	$15,clipsaveoff+24($sp)		# snlwidth

 # Save registers that we'll need later, but not restore immediately
	sw	e1, clipsaveoff+28($sp)		# oc1
	sw	e2, clipsaveoff+32($sp)		# oc2

 # Load up arguments to cfbClipLine
	lw	$4, linearg_pbox($sp)		# pbox
	addu	$5, $sp, local_clippt1x		# &clippt1
	addu	$6, $sp, local_clippt2x		# &clippt2
	move	$7, e1				# oc1
	sw	e2, 16($sp)			# oc2
	sw	dx, 20($sp)			# dx
	sw	dy, 24($sp)			# dy
	sw	signdx, 28($sp)			# signdx
	sw	signdy, 32($sp)			# signdy
	sw	verticalish, 36($sp)		# verticalish
	jal	cfbClipLine

 # Reload saved registers that should always be valid
	lw	$4, clipsaveoff+ 0($sp)		# addrl
	lw	$5, clipsaveoff+ 4($sp)		# nlwidth
	lw	$6, clipsaveoff+ 8($sp)		# nels
	lw	$7, clipsaveoff+12($sp)		# fgxorbits

 # Is the line at least partially contained?
	ble	$2, 0, $84

 # Reload a few more registers (all but oc1, oc2)
	lw	$8, clipsaveoff+16($sp)		# dx
	lw	$9, clipsaveoff+20($sp)		# dy
	lw	$15,clipsaveoff+24($sp)		# snlwidth

	lh	pt2x, local_clippt1x($sp)	# pt2x = clippt1.x
	lh	pt2y, local_clippt1y($sp)	# pt2y = clippt1.y
	multu	pt2y, nlwidth			# start multiply

	slt	verticalish, dx, dy		# verticalish = dx < dy again
 #                 if (verticalish) {
	beq	verticalish, 0, $77
	sll	e1, dx, 1			# e1 = 2*dx
	negu	e, dy				# e = -dy
	subu	e2, e, dy			# e2 = -2*dy
	lh	len, local_clippt2y($sp)	# len = cliptt2.y - pt2y
	subu	len, pt2y
	b	$78
$77:
	sll	e1, dy, 1			# e1 = 2*dy
	negu	e, dx				# e = -dx
	subu	e2, e, dx			# e2 = -2*dy
	lh	len, local_clippt2x($sp)	# len = clippt2.x - pt2x;
	subu	len, pt2x
$78:
	lw	$24, clipsaveoff+32($sp)	# Get ready for (oc2 != 0)...
	bge	len, 0, $79			# if (len < 0) len = -len;
	negu	len, len
$79:
#ifdef SEGMENTS
	sne	$24, $24, 0			# len += ((oc2 != 0) | capEnds)
	or	$24, capEnds
	addu	len, $24
#else
	sne	$24, $24, 0			# len += (oc2 != 0);
	addu	len, $24
#endif    
 # 		   if (len) {
	beq	len, 0, $84

	mflo	$24				# addr = addrl + mflo * pt2x
	addu	addr, addrl, $24
	addu	addr, pt2x

 # 	/* unwind bresenham error term to first point */
 # 			if (oc1) {
	lw	$24, clipsaveoff+28($sp)
	beq	$24, 0, $83

	subu	dx, pt2x, pt1x			# dx = pt2x - pt1x;
	subu	dy, pt2y, pt1y			# dy = pt2y - pt1y;
	bge	dx, 0, $80			# dx = abs(dx)
	negu	dx, dx
$80:
	bge	dy, 0, $81			# dy = abs(dy)
	negu	dy, dy
$81:
 #   		       if (verticalish)
	beq	verticalish, 0, $82
	mul	$24, dx, e2			# e += dx*e2 + dy*e1;
	addu	e, $24
	mul	$24, dy, e1
	addu	e, $24
	b	$83
$82: #		        else
	mul	$24, dy, e2			# e += dy*e2 + dx*e1;
	addu	e, $24
	mul	$24, dx, e1
	addu	e, $24

$83:
/* Okay, super-gross branch to drawing code above */
	beq	verticalish, 0, $835
	move	dy, len
	j	DRAWVERTICALISH
$835:
	move	dx, len
	j	DRAWHORIZONTALISH

UnknownClip1:
#ifndef SEGMENTS
	li	pt1InRect, 0
#endif

$84:
 #     } /* end if sloped line */
$85:
 # } /* end while points */;
	bne	nels, 0, $32
$86:
	lw	$31, linesaveoff+36($sp)

$87:

#ifndef SEGMENTS
/* Put cap on last line drawn unless it satisfies various weird protocol
   conditions. */
	lw	$24, linearg_capEnds($sp)
	lh	pt1x, 0(pel)		#     ((pt1x=pel->x) != elInit->x ||
	lh	pt1y, 2(pel)
	beq	$24, 0, $89		# if (capEnds &&
/* For copy mode, don't need to do all these tests
	lw	addr, linearg_elInit($sp)
	lh	pt2x, 0(addr)
	lh	pt2y, 2(addr)
	jne	pt1x, pt2x, $88
	addu	addr, 4
	jne	pt1y, pt2y, $88	#      (pt2x=pel->y) != elInit->y ||
	jne	pel, addr, $89		#      pel == elInit+1))) {
*/
$88:
/* In clip range? */
	.set	noreorder
	mult	pt1y, nlwidth		# Kick off multiply for below
	slt	$24, pt1x, clipx1
	bne	$24, 0, $89
	slt	$24, pt1x, clipx2
	beq	$24, 0, $89
	slt	$24, pt1y, clipy1
	bne	$24, 0, $89
	slt	$24, pt1y, clipy2
	beq	$24, 0, $89
	addu	addr, addrl, pt1x
	mflo	$24
	addu	addr, $24
	sb	fgxorbits, 0(addr)
	.set	reorder
#endif

$89:
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
