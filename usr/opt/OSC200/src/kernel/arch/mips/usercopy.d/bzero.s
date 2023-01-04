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
 * @(#)$RCSfile: bzero.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 15:57:48 $
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
 * bzero(dst, bcount)
 * Zero block of memory
 *
 * Calculating MINZERO, assuming 50% cache-miss on non-loop code:
 * Overhead =~ 18 instructions => 63 (81) cycles
 * Byte zero =~ 16 (24) cycles/word for 08M44 (08V11)
 * Word zero =~ 3 (6) cycles/word for 08M44 (08V11)
 * If I-cache-miss nears 0, MINZERO ==> 4 bytes; otherwise, times are:
 * breakeven (MEM) = 63 / (16 - 3) =~ 5 words
 * breakeven (VME) = 81 / (24 - 6)  =~ 4.5 words
 * Since the overhead is pessimistic (worst-case alignment), and many calls
 * will be for well-aligned data, and since Word-zeroing at least leaves
 * the zero in the cache, we shade these values (18-20) down to 12
 */
#define	MINZERO	12
LEAF(bzero)
XLEAF(blkclr)
	subu	v1,zero,a0		# number of bytes til aligned
	blt	a1,MINZERO,bytezero
	and	v1,NBPW-1
	subu	a1,v1
	beq	v1,zero,blkzero		# already aligned
	SWS	zero,0(a0)
	addu	a0,v1

/*
 * zero 32 byte, aligned block
 */
blkzero:
	and	a3,a1,~31		# 32 byte chunks
	subu	a1,a3
	beq	a3,zero,wordzero
	addu	a3,a0			# dst endpoint
1:	sw	zero,0(a0)
	sw	zero,4(a0)
	sw	zero,8(a0)
	sw	zero,12(a0)
	addu	a0,32
	sw	zero,-16(a0)
	sw	zero,-12(a0)
	sw	zero,-8(a0)
	sw	zero,-4(a0)
	bne	a0,a3,1b

wordzero:
	and	a3,a1,~(NBPW-1)		# word chunks
	subu	a1,a3
	beq	a3,zero,bytezero
	addu	a3,a0			# dst endpoint
1:	addu	a0,NBPW
	sw	zero,-NBPW(a0)
	bne	a0,a3,1b

bytezero:
	ble	a1,zero,zerodone
	addu	a1,a0			# dst endpoint
1:	addu	a0,1
	sb	zero,-1(a0)
	bne	a0,a1,1b
zerodone:
	j	ra
	END(bzero)

