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
 * @(#)$RCSfile: kn_bzero.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 17:09:28 $
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
 * The routine in this module implements the memory intensive routine bzero().
 * In order to optimize the performance of this routine it is
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
 *		b z e r o
 *
 * Functional Description:
 *
 *	Zero out a block of memory. This routine assumes that it will mostly be
 *	used to zero out large blocks of memory.
 *
 * Inputs:
 *
 *	void bzero( dst, length )
 *
 *	dst	- pointer to block to be zeroed
 *	length	- number of bytes to be zeroed
 *
 * Outputs:
 *
 *	The block of memory will be filled with zeroes.
 *
 * Return Value:
 *
 *	None
 *
 */
#define MINZERO	12

LEAF(bzero)
XLEAF(blkclr)
XLEAF(kn_bzero)

	.set	noreorder

	blt	a1,MINZERO,bz1		# for small blocks - byte at a time
	subu	t0,zero,a0		# number of bytes until alignment
	and	t0,NBPW-1
	beqz	t0,bz128		# if zero, already word aligned
	subu	a1,t0			# reduce remaining byte count
	SWS	zero,(a0)		# force alignment
	addu	a0,t0			# update pointer

/*
 * Zero out 128 byte, aligned block.
 */
bz128:	and	t0,a1,~127		# get # of 128 byte blocks
	beqz	t0,bz32			# if zero, none
	subu	a1,t0			# reduce byte count
	addu	t0,a0			# compute dst endpoint

1:	sw	zero,0(a0)		# start write buffer going
	addu	a0,128			# move on pointer
	sw	zero,-124(a0)		# zero out the rest of
	sw	zero,-120(a0)		#   the block
	sw	zero,-116(a0)
	sw	zero,-112(a0)
	sw	zero,-108(a0)
	sw	zero,-104(a0)
	sw	zero,-100(a0)
	sw	zero,-96(a0)
	sw	zero,-92(a0)
	sw	zero,-88(a0)
	sw	zero,-84(a0)
	sw	zero,-80(a0)
	sw	zero,-76(a0)
	sw	zero,-72(a0)
	sw	zero,-68(a0)
	sw	zero,-64(a0)
	sw	zero,-60(a0)
	sw	zero,-56(a0)
	sw	zero,-52(a0)
	sw	zero,-48(a0)
	sw	zero,-44(a0)
	sw	zero,-40(a0)
	sw	zero,-36(a0)
	sw	zero,-32(a0)
	sw	zero,-28(a0)
	sw	zero,-24(a0)
	sw	zero,-20(a0)
	sw	zero,-16(a0)
	sw	zero,-12(a0)
	sw	zero,-8(a0)
	bne	a0,t0,1b		# loop till done
	sw	zero,-4(a0)

/*
 * Zero out 32 byte, aligned block.
 */
bz32:	and	t0,a1,96		# get # of 32 byte blocks
	beqz	t0,bz4			# if zero, none
	subu	a1,t0			# reduce byte count
	addu	t0,a0			# compute dst endpoint

1:	sw	zero,0(a0)		# start write buffer going
	addu	a0,32			# move on pointer
	sw	zero,-28(a0)		# zero out the rest of
	sw	zero,-24(a0)		#   the block
	sw	zero,-20(a0)
	sw	zero,-16(a0)
	sw	zero,-12(a0)
	sw	zero,-8(a0)
	bne	a0,t0,1b		# loop tile done
	sw	zero,-4(a0)

/*
 * Zero out 4 byte (longword), aligned block.
 */
bz4:	and	t0,a1,~(NBPW-1)		# get # of words
	beqz	t0,bz1			# if zero, none
	subu	a1,t0			# reduce byte count
	addu	t0,a0			# compute dst endpoint

1:	addu	a0,NBPW			# move on pointer
	bne	a0,t0,1b		# loop till done
	sw	zero,-NBPW(a0)		# zero out the word

/*
 * Run out the rest of the block 1 byte at a time.
 */
bz1:	beqz	a1,bzdone		# if zero, all done
	addu	t0,a1,a0		# compute dst endpoint

1:	addu	a0,1			# move on pointer
	bne	a0,t0,1b		# loop till done
	sb	zero,-1(a0)		# zero out a byte

bzdone:	j	ra			# done
	nop

	.set	reorder

	END(bzero)

