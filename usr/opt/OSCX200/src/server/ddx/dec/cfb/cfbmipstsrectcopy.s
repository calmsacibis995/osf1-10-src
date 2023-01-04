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
**                   COPYRIGHT (c) 1988, 1989, 1990 BY                      *
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

	.globl	cfbFillSolidRectCopy

 # void cfbFillStippledRectCopy(fgandbits, fgxorbits,
 #				pdstBase, dstwidth, prect, nrect)

	.ent	cfbFillSolidRectCopy 2
cfbFillSolidRectCopy:
	solidframesize = 0
	.mask	0x0000000, -4
	.frame	$sp, solidframesize, $31

/*
 * Register allocation.  Temporary registers t8-t9 ($24-$25) used as much as
 * possible as actual temporaries and limited-span temporaries in order to keep
 * temporaries t0-t7 ($8-$15) for variables.
 */

/* Scratch temporaries */
#define x		$2
#define y		$3

/* Register parameters.  */
#define prect		$4
#define fgxorbits	$5
#define pdstBase	$6
#define dstwidth	$7

/* Scratch temporaries */
#define nrect		$8
#define pdst		$9
#define p		$10
#define align		$11
#define w		$12
#define h		$13
#define tw		$14
#define four		$15

/* Scratch temporaries.  $24 used as a general temporary. */
#define leftpixels	$25


/* Record offset definitions. */
#define	xfield		0
#define yfield		2
#define widthfield	4
#define heightfield	6

/* Don't save callee's registers...we only use scratch registers */


/* Load up arguments currently in memory */
	lw	prect, solidframesize+16($sp)
	lw	nrect, solidframesize+20($sp)
	li	four, 4

/* do { /* Each rectangle */
$50:

/* Compute upper left corner. */
 #	pdst = pdstBase + prect->y * dstwidth + prect->x;
	lh	y, yfield(prect)
	multu	y, dstwidth	
	lh	x, xfield(prect)	
	lh	w, widthfield(prect)
	lh	h, heightfield(prect)
	addu	pdst, pdstBase, x

	subu	nrect, 1

/* Compute alignment. */
	andi	align, x, 3
	addu	tw, w, align
	addu	tw, -4

	mflo	$24
	addu	pdst, $24

/* Does entire rectangle fit inside single destination word? */
	bgt	tw, 0, $700

/* Fits into high pixels of word? */
	bne	tw, 0, $200

/* Do rectangle using swr */
$100:
	swr	fgxorbits, (pdst)
	addu	pdst, dstwidth
	subu	h, 1
	bne	h, 0, $100
	b	$1500

$200:
	bne	align, 0, $400

/* Do rectangle using swl */
	addu	pdst, w
$300:
	swl	fgxorbits, -1(pdst)
	addu	pdst, dstwidth
	subu	h, 1
	bne	h, 0, $300
	b	$1500
	
$400:
	bne	w, 1, $600
/* Paint one-pixel wide rectangle */
$500:
	sb	fgxorbits, (pdst)
	addu	pdst, dstwidth
	subu	h, 1
	bne	h, 0, $500
	b	$1500

$600:
/* Paint two-pixel wide rectangle in center of word */
	sb	fgxorbits, (pdst)
	sb	fgxorbits, 1(pdst)
	addu	pdst, dstwidth
	subu	h, 1
	bne	h, 0, $600
	b	$1500

$700:
/* Rectangle spans more than one word */
	subu	leftpixels, four, align
	subu	tw, 4
	bgt	tw, 0, $900

/* Exactly two words to write to */
$800:
	swr	fgxorbits, (pdst)
	addu	p, pdst, w
	swl	fgxorbits, -1(p)
	addu	pdst, dstwidth
	subu	h, 1
	bne	h, 0, $800
	b 	$1500

$900:
	bgt	tw, 4, $1100

/* Exactly three words to write to */

$1000:
	swr	fgxorbits, (pdst)
	addu	p, pdst, leftpixels
	sw	fgxorbits, (p)
	addu	p, pdst, w
	swl	fgxorbits, -1(p)
	addu	pdst, dstwidth
	subu	h, 1
	bne	h, 0, $1000
	b	$1500

$1100:
/* Four or more words to write to */
	
	swr	fgxorbits, (pdst)
	addu	p, pdst, leftpixels
	move	w, tw
$1200:
	sw	fgxorbits, (p)
	addu	p, 4
	subu	w, 4
	bgt	w, 0, $1200

	addu	p, w
	swl	fgxorbits, 3(p)
	addu	pdst, dstwidth
	subu	h, 1
	bne	h, 0, $1100

$1500:
	addu	prect, 8
	bne	nrect, 0, $50

	j	$31
	.end	cfbFillSolidRectCopy

#undef x
#undef y
#undef prect
#undef fgxorbits
#undef pdstBase
#undef dstwidth
#undef nrect
#undef pdst
#undef p
#undef align
#undef w
#undef h
#undef tw
#undef four
#undef leftpixels


	.globl	cfbFillStippledRectCopy

 # void cfbFillStippledRectCopy(psrcBase, srcheight, fgandbits, fgxorbits,
 #				xorg, yorg, pdstBase, dstwidth, prect, nrect)

	.ent	cfbFillStippledRectCopy 2
cfbFillStippledRectCopy:
	framesize = 40
	.mask	0xC0FF0000, -4
	.frame	$sp, framesize, $31

/*
 * Register allocation.  Temporary registers t8-t9 ($24-$25) used as much as
 * possible as actual temporaries and limited-span temporaries in order to keep
 * temporaries t0-t7 ($8-$15) for variables.
 */

/* Scratch temporaries */
#define srcbits		$2
#define tsrcbits	$3

/* Register parameters.  */
#define psrcBase	$4
#define srcheight	$5
#define xorg		$6
#define fgxorbits	$7

/* Scratch temporaries */
#define yorg		$8
#define pdst		$9
#define align		$10
#define dstwidth	$11
#define width		$12
#define height		$13
#define ix32		$14
#define w		$15

/* Must save if used registers */
#define	p		$16
#define casetable	$17
#define endmask		$18
#define ix		$19
#define iy		$20
#define pdstBase	$21
#define prect		$22
#define nrect		$23

/* Scratch temporaries.  $24 used as a general temporary. */
#define tp		$25

/* Must save if used registers specific to large rectangles */
#define w3		$30
#define i		$31


/* Record offset definitions. */
#define	xfield		0
#define yfield		2
#define widthfield	4
#define heightfield	6

/* Save callee's registers */
	subu	$sp, framesize
	sw	$16, 0($sp)
	sw	$17, 4($sp)
	sw	$18, 8($sp)
	sw	$19, 12($sp)
	sw	$20, 16($sp)
	sw	$21, 20($sp)
	sw	$22, 24($sp)
	sw	$23, 28($sp)
	sw	$30, 32($sp)
	sw	$31, 36($sp)

/* Load up arguments currently in memory */
	lw	xorg, framesize+16($sp)
	lw	yorg, framesize+20($sp)
	lw	pdstBase, framesize+24($sp)
	lw	dstwidth, framesize+28($sp)
	lw	prect, framesize+32($sp)
	lw	nrect, framesize+36($sp)

/* Turn srcheight into byte offsets.  iy also maintained as byte offset. */
	sll	srcheight, 2

/* do { /* Each rectangle */
$33:

/* Compute upper left corner.  Offset back so pdst is word-aligned. */
 #	pdst = pdstBase + prect->y * dstwidth + prect->x;
	lh	ix, xfield(prect)	
	lh	iy, yfield(prect)
	multu	iy, dstwidth	
	lh	width, widthfield(prect)
	lh	height, heightfield(prect)

	addu	pdst, pdstBase, ix
	mflo	$24
	addu	pdst, $24

	/* Compute offsets into stipple. */
	subu	ix, xorg		# ix = (prect->x - xorg);
	subu	iy, yorg		# iy = (prect->y - yorg) % stippleHeight
	sll	iy, 2
	divu	$0, iy, srcheight	# $0 supposedly says no checks

	subu	nrect, 1

/* Compute alignment, readjust values based on it */
	andi	align, pdst, 3
	addu	width, align
	subu	pdst, align
	subu	ix, align

/* Do we do stupid code for little rects, or smart code for big rects? */
	bgt	width, 99, $2000

	la	casetable, $10100

/* Finish computing ix from align, then compute ix32 from ix.  Sleaze
   knowing shift only pays attention to bottom 5 bits, so 32-ix == -ix */
	
	andi	ix, 0x1f
	subu	ix32, $0, ix

/* Precompute alignment mask for first 32 bits of rectangle */
	li	$24, -1
	srl	$24, align
	sll	align, $24, align

/* Precompute mask for last 1-32 bits of rectangle.  Note that shift only uses
   bottom 5 bits of shift amount, so we can really sleaze here. */
	li	endmask, 1
	sll	endmask, width
	addu	endmask, -1
	
/* Finally, get iy back from multiplier/divide unit */
	mfhi	iy

$82:
	move	w, width
	move	p, pdst
 # srcbits = *(psrcBase + iy);
	addu	$24, psrcBase, iy
	lw	srcbits, 0($24)
	addu	iy, 4
 # 336		if (iy == srcheight) {
	bne	iy, srcheight, $1077
 # 337		    iy = 0;
	move	iy, $0
$1077:

 # if srcbits is 0, skip this entire row in the rectangle
	beq	srcbits, 0, $106

 # /* Do one-time alignment of srcbits for entire scan line */
 # srcbits = (srcbits << ix32) | (srcbits >> ix);
	sll	$24, srcbits, ix32
	srl	srcbits, ix
	or	srcbits, $24

 # /* We bumped back p to beginning of word, so make sure bottom
 #    bits of word are 0. */
	and	tsrcbits, srcbits, align

$83:
 # for (;;) {
	addu	w, -32
 #     if (w < 32-32) {
	bge	w, 0, $84
 # 	   /* ran out of rectangle, so mask out high stipple bits */
 #         tsrcbits &= endmask;
	and	tsrcbits, endmask
 #     }

$84:
	move	tp, p
	addu	p, 32
 #     SplatAllFGBits(tp, tsrcbits, fgandbits, fgxorbits);
 # $24 = address of instruction block offset from $10100
$85:
	and	$24, tsrcbits, 15
	sll	$24, 5
	addu	$24, casetable

	.set	noreorder
	j	$24			# jump to proper instruction block
	srl	tsrcbits, 4		# tsrcbits >> 4
	
/*
 * Individual cases [0..15].  Each is exactly 8 instructions long for fast
 * dispatching.  Each case writes to exactly those bytes that have a
 * corresponding 1 set in tsrcbits.  Further, each case contains the 
 * ``while tsrcbits !=0'' and other loop termination code to minimize
 *  extra jumps.
 */

$10100:	bne 	tsrcbits, 0, $85	# end while tsrcbits
	addu	tp, 4			# tp += 4
	bgt	w, 0, $83		# end while w > 0
	move	tsrcbits, srcbits	# tsrcbits = srcbits
	b	$107
	addu	height, -1		# height--
	nop
	nop
$10101:	sb	fgxorbits, 0(tp)
	bne 	tsrcbits, 0, $85	# end while tsrcbits
	addu	tp, 4			# tp += 4
	bgt	w, 0, $83		# end while w > 0
	move	tsrcbits, srcbits	# tsrcbits = srcbits
	b	$107
	addu	height, -1		# height--
	nop
$10102:	sb	fgxorbits, 1(tp)
	bne 	tsrcbits, 0, $85	# end while tsrcbits
	addu	tp, 4			# tp += 4
	bgt	w, 0, $83		# end while w > 0
	move	tsrcbits, srcbits	# tsrcbits = srcbits
	b	$107
	addu	height, -1		# height--
	nop
$10103:	sh	fgxorbits, 0(tp)
	bne 	tsrcbits, 0, $85	# end while tsrcbits
	addu	tp, 4			# tp += 4
	bgt	w, 0, $83		# end while w > 0
	move	tsrcbits, srcbits	# tsrcbits = srcbits
	b	$107
	addu	height, -1		# height--
	nop
$10104:	sb	fgxorbits, 2(tp)
	bne 	tsrcbits, 0, $85	# end while tsrcbits
	addu	tp, 4			# tp += 4
	bgt	w, 0, $83		# end while w > 0
	move	tsrcbits, srcbits	# tsrcbits = srcbits
	b	$107
	addu	height, -1		# height--
	nop
$10105:	sb	fgxorbits, 0(tp)
	sb	fgxorbits, 2(tp)
	bne 	tsrcbits, 0, $85	# end while tsrcbits
	addu	tp, 4			# tp += 4
	bgt	w, 0, $83		# end while w > 0
	move	tsrcbits, srcbits	# tsrcbits = srcbits
	b	$107
	addu	height, -1		# height--
$10106:	sb	fgxorbits, 1(tp)
	sb	fgxorbits, 2(tp)
	bne 	tsrcbits, 0, $85	# end while tsrcbits
	addu	tp, 4			# tp += 4
	bgt	w, 0, $83		# end while w > 0
	move	tsrcbits, srcbits	# tsrcbits = srcbits
	b	$107
	addu	height, -1		# height--
$10107:	swl	fgxorbits, 2(tp)
	bne 	tsrcbits, 0, $85	# end while tsrcbits
	addu	tp, 4			# tp += 4
	bgt	w, 0, $83		# end while w > 0
	move	tsrcbits, srcbits	# tsrcbits = srcbits
	b	$107
	addu	height, -1		# height--
	nop
$10108:	sb	fgxorbits, 3(tp)
	bne 	tsrcbits, 0, $85	# end while tsrcbits
	addu	tp, 4			# tp += 4
	bgt	w, 0, $83		# end while w > 0
	move	tsrcbits, srcbits	# tsrcbits = srcbits
	b	$107
	addu	height, -1		# height--
	nop
$10109:	sb	fgxorbits, 0(tp)
	sb	fgxorbits, 3(tp)
	bne 	tsrcbits, 0, $85	# end while tsrcbits
	addu	tp, 4			# tp += 4
	bgt	w, 0, $83		# end while w > 0
	move	tsrcbits, srcbits	# tsrcbits = srcbits
	b	$107
	addu	height, -1		# height--
$10110:	sb	fgxorbits, 1(tp)
	sb	fgxorbits, 3(tp)
	bne 	tsrcbits, 0, $85	# end while tsrcbits
	addu	tp, 4			# tp += 4
	bgt	w, 0, $83		# end while w > 0
	move	tsrcbits, srcbits	# tsrcbits = srcbits
	b	$107
	addu	height, -1		# height--
$10111:	sh	fgxorbits, 0(tp)
	sb	fgxorbits, 3(tp)
	bne 	tsrcbits, 0, $85	# end while tsrcbits
	addu	tp, 4			# tp += 4
	bgt	w, 0, $83		# end while w > 0
	move	tsrcbits, srcbits	# tsrcbits = srcbits
	b	$107
	addu	height, -1		# height--
$10112:	sh	fgxorbits, 2(tp)
	bne 	tsrcbits, 0, $85	# end while tsrcbits
	addu	tp, 4			# tp += 4
	bgt	w, 0, $83		# end while w > 0
	move	tsrcbits, srcbits	# tsrcbits = srcbits
	b	$107
	addu	height, -1		# height--
	nop
$10113:	sb	fgxorbits, 0(tp)
	sh	fgxorbits, 2(tp)
	bne 	tsrcbits, 0, $85	# end while tsrcbits
	addu	tp, 4			# tp += 4
	bgt	w, 0, $83		# end while w > 0
	move	tsrcbits, srcbits	# tsrcbits = srcbits
	b	$107
	addu	height, -1		# height--
$10114:	swr	fgxorbits, 1(tp)
	bne 	tsrcbits, 0, $85	# end while tsrcbits
	addu	tp, 4			# tp += 4
	bgt	w, 0, $83		# end while w > 0
	move	tsrcbits, srcbits	# tsrcbits = srcbits
	b	$107
	addu	height, -1		# height--
	nop
$10115:	sw	fgxorbits, 0(tp)
	bne 	tsrcbits, 0, $85	# end while tsrcbits
	addu	tp, 4			# tp += 4
	bgt	w, 0, $83		# end while w > 0
	move	tsrcbits, srcbits	# tsrcbits = srcbits
/* NOTE: last instruction block is different here */
	.set	reorder
$106:
	addu	height, -1		# height--

$107:
	addu	pdst, dstwidth	
	bgt	height, 0, $82
	b	$9999


/* Splat4 routine for use below */
Splat4:
 # tp: word address to write to
 # fgxorbits: foreground data to write
 # tsrcbits: 4 bits of stipple

	sll	tsrcbits, 2
	sll	$24, tsrcbits, 1
	addu	tsrcbits, $24
	la	$24, $1900
	addu	$24, tsrcbits
	j	$24

	.set	noreorder		# all entries 3 instructions long

$1900:	j	$31
	nop
	nop
$1901:	j	$31
	sb	fgxorbits, 0(tp)
	nop
$1902:	j	$31
	sb	fgxorbits, 1(tp)
	nop
$1903:	j	$31
	sh	fgxorbits, 0(tp)
	nop
$1904:	j	$31
	sb	fgxorbits, 2(tp)
	nop
$1905:	sb	fgxorbits, 0(tp)
	j	$31
	sb	fgxorbits, 2(tp)
$1906:	sb	fgxorbits, 1(tp)
	j	$31
	sb	fgxorbits, 2(tp)
$1907:	j	$31
	swl	fgxorbits, 2(tp)
	nop
$1908:	j	$31
	sb	fgxorbits, 3(tp)
	nop
$1909:	sb	fgxorbits, 0(tp)
	j	$31
	sb	fgxorbits, 3(tp)
$1910:	sb	fgxorbits, 1(tp)
	j	$31
	sb	fgxorbits, 3(tp)
$1911:	sh	fgxorbits, 0(tp)
	j	$31
	sb	fgxorbits, 3(tp)
$1912:	j	$31
	sh	fgxorbits, 2(tp)
	nop
$1913:	sb	fgxorbits, 0(tp)
	j	$31
	sh	fgxorbits, 2(tp)
$1914:	j	$31
	swr	fgxorbits, 1(tp)
	nop
$1915:	j	$31
	sw	fgxorbits, 0(tp)
	
	.set	reorder

/* Really wide rectangle.  For each 4 bits, get into proper branch
   of case statement, then loop punching the appropriate pixels
   across the dst.  We know we'll loop at least 3 times. */

$2000:

	la	casetable, $2050
	addi	ix, 4			# ix = (ix + 4) & 31
	andi	ix, 0x1f
	subu	ix32, $0, ix		# ix32 = 32-ix, sleaze for shift instr

	andi	w3, width, 3		# w3 = width & 3
	subu	width, 100		# width = width - (96+4) - w3
	subu	width, w3

	li	endmask, 1			# endmask = (1 << w3) - 1
	sll	endmask, w3
	subu	endmask, 1

/* Finally, get iy back from multiplier/divide unit */
	mfhi	iy

 # do { /* For each line of height */
$2010:
/* moved this here because of reordering bug in scheduler */
	subu	height, 1		# height--
/* -------------------------------------------------------*/

	addu	$24, psrcBase, iy	# srcbits = psrcBase[iy]
	lw	srcbits, 0($24)
	addu	iy, 4			# iy++, etc.
	bne	iy, srcheight, $2020
	move	iy, $0

 # if srcbits is 0, skip this entire row in the rectangle
$2020:
	beq	srcbits, 0, $2100

 # /* Do one-time alignment of srcbits for entire scan line */
 # srcbits = (srcbits << ix32) | (srcbits >> ix);
	sll	$24, srcbits, ix32
	srl	srcbits, ix
	or	srcbits, $24

/* Ragged left alignment */
 # tsrcbits = (srcbits >> (28 + align)) << align;
	srl	tsrcbits, srcbits, 28
	srl	tsrcbits, align
	sll	tsrcbits, align
	move	tp, pdst
	jal	Splat4

/* Ragged right alignment */
	addi	p, pdst, 4
	beq	w3, 0, $2030
 # tsrcbits = (srcbits >> (width & 0x1c)) & endmask;
	andi	$24, width, 0x1c
	srl	tsrcbits, srcbits, $24
	and	tsrcbits, endmask
	addu	tp, p, width
	addi	tp, 96
	jal	Splat4

$2030:
		/* Now cycle through each 4 bits, splating the pixels as many
		   times as possible.  We know that we splat each 4 bits at
		   least three times. */

 # for (i = 28; i >= 0 && srcbits; i -=4) {
	li	i, 28

$2040:
	addu	w, width, i			# w = (width + i) >> 5
	srl	w, 5
	addu	tp, p, 96

	andi	tsrcbits, srcbits, 0xf		# switch (srcbits & 0xf) {
	sll	tsrcbits, 2
	addu	$24, casetable, tsrcbits
	lw	$24, 0($24)
	j	$24

	.rdata
$2050:
	/* case table */
	.word	loop0
	.word	loop1
	.word	loop2
	.word	loop3
	.word	loop4
	.word	loop5
	.word	loop6
	.word	loop7
	.word	loop8
	.word	loop9
	.word	loop10
	.word	loop11
	.word	loop12
	.word	loop13
	.word	loop14
	.word	loop15
	.text

#define SingleOp(op, offset)			\
	op	fgxorbits, offset(p);		\
	op	fgxorbits, 32+offset(p);	\
	op	fgxorbits, 64+offset(p);	\
	beq	w, 0, endcase;			\
1:						\
	op	fgxorbits, offset(tp);		\
	addi	tp, 32;				\
	subu	w, 1;				\
	bne	w, 0, 1b;			\
	b	endcase

#define DoubleOp(op1, offset1, op2, offset2)	\
	op1	fgxorbits, offset1(p);		\
	op2	fgxorbits, offset2(p);		\
	op1	fgxorbits, 32+offset1(p);	\
	op2	fgxorbits, 32+offset2(p);	\
	op1	fgxorbits, 64+offset1(p);	\
	op2	fgxorbits, 64+offset2(p);	\
	beq	w, 0, endcase;			\
1:						\
	op1	fgxorbits, offset1(tp);		\
	op2	fgxorbits, offset2(tp);		\
	addi	tp, 32;				\
	subu	w, 1;				\
	bne	w, 0, 1b;			\
	b	endcase

loop1:	SingleOp(sb, 0)

loop2:	SingleOp(sb, 1)

loop3:	SingleOp(sh, 0)

loop4:	SingleOp(sb, 2)

loop5:	DoubleOp(sb, 0, sb, 2)

loop6:	DoubleOp(sb, 1, sb, 2)

loop7:	SingleOp(swl, 2)

loop8:	SingleOp(sb, 3)

loop9:	DoubleOp(sb, 0, sb, 3)

loop10:	DoubleOp(sb, 1, sb, 3)

loop11:	DoubleOp(sh, 0, sb, 3)

loop12:	SingleOp(sh, 2)

loop13:	DoubleOp(sb, 0, sh, 2)

loop14: SingleOp(swr, 1)

loop15:	SingleOp(sw, 0)

loop0:
endcase:
	addu	p, 4			# p += 4
	srl	srcbits, 4		# srcbits >>= 4
	beq	srcbits, 0, $2100	# end for loop
	subu	i, 4
	bge	i, 0, $2040

$2100:
	addu	pdst, dstwidth		# pdst += dstwidth
	bne	height, 0, $2010	# end while (height != 0)
		
	

 # 342	} /* cfbFillStippledRectCopy */

$9999:
	addu	prect, 8
	bne	nrect, 0, $33

	lw	$31, 36($sp)		# Restore registers
	lw	$30, 32($sp)
	lw	$23, 28($sp)
	lw	$22, 24($sp)
	lw	$21, 20($sp)
	lw	$20, 16($sp)
	lw	$19, 12($sp)
	lw	$18, 8($sp)
	lw	$17, 4($sp)
	lw	$16, 0($sp)
	addu	$sp, framesize
	j	$31
	.end	cfbFillStippledRectCopy
