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
 * @(#)$RCSfile: kn_bcopy.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 17:07:26 $
 */
#include <machine/machparam.h>
#include <machine/cpu.h>
#include <machine/asm.h>
#include <machine/reg.h>
#include <machine/regdef.h>
#include <sys/errno.h>
#include <assym.s>

#if	MIPSEB		/* Unaligned acceses */
#    define LWS lwl
#    define LWB lwr
#    define SWS swl
#    define SWB swr
#else	/* MIPSEL */
#    define LWS lwr
#    define LWB lwl
#    define SWS swr
#    define SWB swl
#endif

/*
 * Copyright (C) 1990 by
 * Digital Equipment Corporation, Maynard, Mass.
 * Telecommunications and Networks Software Engineering
 *
 * Notes:
 * -----
 *
 * The routine in this module implements the memory intensive routine
 * bcopy(). In order to optimize the performance of this routine it is
 * important to understand how the memory subsystem operates, especially with
 * respect to memory writes. Typically, memory writes can be retired from the
 * write buffer every clock cycle (page mode writes as they will be for these
 * routines), however, there are some additional start-up and shut-down costs
 * for the write buffer: (3MAX numbers)
 *
 *	  2	cycles latency before the memory controller receives
 *		the request
 *	  5	cycles start-up time in the write buffer
 *	  4	cycles pipe-line flush at shutdown
 *	-----
 *	 11	overhead
 *
 * In order to amortize this overhead we want to keep the write buffer busy
 * for long sequences of instructions wherever possible. This routine will
 * therefore unroll the loops into large pieces and tradeoff I-cache usage
 * and code size against write buffer usage.
 */
/*
 *		b c o p y
 *
 * Functional Description:
 *
 *	High speed replacement routine for the standard kernel block copy.
 *	This routine assumes that it will mostly be used for copying large
 *	blocks of memory.
 *
 * Inputs:
 *
 *	bcopy( src, dst, len )
 *
 *	src	- pointer to source buffer
 *	dst	- pointer to destination buffer
 *	len	- number of bytes to copy
 *
 * Outputs:
 *
 *	The source buffer is copied to the destination. It is assumed that
 *	these buffers do not overlap.
 *
 * Return Value:
 *
 *	None
 */
#define MINCOPY	8

LEAF(bcopy)
XLEAF(kn_bcopy)

	.set	noreorder

	blt	a2,MINCOPY,bytecopy	# short block, just byte copy
	xor	v0,a0,a1		# check for possible alignment
	and	v0,NBPW-1		# can we align src and dst?
	bnez	v0,unaligncopy		# if non-zero, no
	subu	v1,zero,a0		# compute # of bytes
	and	v1,NBPW-1		#   to alignment
	beqz	v1,blockcopy		# if zero, already aligned
	subu	a2,v1			# reduce remaining count
	LWS	v0,(a0)			# copy unaligned data
	addu	a0,v1
	SWS	v0,(a1)
	addu	a1,v1

/*
 * Perform a block copy 64 bytes (16 longwords) at a time.
 */
blockcopy:
	and	v0,a2,~63		# 64 byte chunks
	beqz	v0,wordcopy		# if zero, < 1 block to copy
	subu	a2,v0			# update byte count
	addu	v0,a0			# compute src endpoint

/*
 * Note that the following code assumes that the cache line size is 4
 * longwords and optimizes for the case where source data is cached
 * although it is no worse for uncached data with 32 byte block copy.
 */
1:	lw	t0,(a0)			# preload the cache
	lw	t1,16(a0)		#   with all the data
	lw	t2,32(a0)
	lw	t3,48(a0)

	lw	t4,4(a0)		# load rest of first half
	lw	t5,8(a0)		#   of block
	lw	t6,12(a0)
	lw	t7,20(a0)
	lw	t8,24(a0)
	lw	t9,28(a0)
	sw	t0,(a1)			# start write buffer going
	sw	t4,4(a1)
	addu	a0,64			# update src and dst pointers
	addu	a1,64
	sw	t5,-56(a1)
	sw	t6,-52(a1)
	sw	t1,-48(a1)
	sw	t7,-44(a1)
	sw	t8,-40(a1)
	sw	t9,-36(a1)

	lw	t4,-28(a0)		# load rest of second half
	lw	t5,-24(a0)
	lw	t6,-20(a0)
	lw	t7,-12(a0)
	lw	t8,-8(a0)
	lw	t9,-4(a0)
	sw	t2,-32(a1)		# store the second half
	sw	t4,-28(a1)
	sw	t5,-24(a1)
	sw	t6,-20(a1)
	sw	t3,-16(a1)
	sw	t7,-12(a1)
	sw	t8,-8(a1)
	bne	a0,v0,1b		# loop till done
	sw	t9,-4(a1)

/*
 * Word copy loop
 */
wordcopy:
	and	v0,a2,~(NBPW-1)		# word chunks
	beqz	v0,bytecopy		# if zero, < 1 word to copy
	subu	a2,v0			# update byte count
	addu	v0,a0			# compute src endpoint

1:	lw	t0,(a0)			# copy word at a time
	addu	a0,NBPW
	sw	t0,(a1)
	bne	a0,v0,1b		# if NE, another word to move
	addu	a1,NBPW

	b	bytecopy		# copy remaining bytes
	nop

/*
 * Worst case. Source and destination cannot be simultaneously aligned.
 * Align the destination so that we can use the full word access on part
 * of the copy.
 */
unaligncopy:
	subu	v1,zero,a1		# compute # of bytes
	and	v1,NBPW-1		#   to alignment
	beqz	v1,1f			# if zero, dst already aligned
	subu	a2,v1			# reduce remaining count
	LWS	v0,(a0)			# get the next longword
	LWB	v0,3(a0)
	addu	a0,v1
	SWS	v0,(a1)			# store "right" piece
	addu	a1,v1

/*
 * When we get here, the source is unaligned and the destination is aligned.
 * NOTE: for this to work, MINCOPY must be >= 7.
 */
1:	and	v0,a2,~(NBPW-1)		# word chunks
	subu	a2,v0			# update byte count
	addu	v0,a0			# compute src endpoint

2:	LWS	t0,(a0)			# load next longword
	LWB	t0,3(a0)		#   the slow way
	addu	a0,NBPW
	sw	t0,(a1)
	bne	a0,v0,2b		# if NE, another word to move
	addu	a1,NBPW

/*
 * Here comes the brute force, byte-by-byte copy code. This code is used for
 * small copies (< MINCOPY bytes) and for the final few bytes for the higher
 * performance loops.
 */
bytecopy:
	blez	a2,done			# if <= zero, all done
	addu	v0,a0,a2		# compute src endpoint

1:	lb	t0,(a0)			# copy byte at a time
	addu	a0,1
	sb	t0,(a1)
	bne	a0,v0,1b		# if NE, another byte to move
	addu	a1,1

done:	j	ra
	nop

	.set	reorder

	END(bcopy)

