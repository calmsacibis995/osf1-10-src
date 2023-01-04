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
	.rdata
	.asciiz "@(#)$RCSfile: lwu.s,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/19 19:44:19 $"
	.text

#include <machine/pal.h>
#include <machine/asm.h>
#include <machine/regdef.h>
#include <machine/machparam.h>

/* a0            pte pointer
 * a1            start va
 * a2            n_pages
 * a3            users buffer
 * a4            transaction id
 * t3            pte limit  (beg)
 * t4            pte limit (middle) 
 * a4		 pte limit (middle - noprefetch)
 * t5            pte lim (end) 
 * t1            the constant (VALID_MASK|SEG_MASK)
 * t2            the constant LWW_MASK
 * v0            ipl
 * s0-s6,a5                   8 pte's
 * t6, t7, a1, a2, t9, t0             prefteched data (maybe)
 * t11, t12, t8, t10, t8      temps
 */



LEAF(pmap_lw_unwire_ass)
.align 3

	lda	sp, -192(sp)     # or whetever 
	stq	s0, 8(sp)
	stq	s1, 16(sp)
	stq	s2, 24(sp)
	stq	s3, 32(sp)
	stq	s4, 40(sp)
	stq	s5, 48(sp)
	stq     s6, 56(sp)
	stq	a0, 64(sp)
	bis	zero,zero,zero

/*	lda	t0,lww_spot1
 *	ldq	t9, 0(t0)
 *	addq	t9, 0x1, t9
 *	stq	t9, 0(t0)
 */
	
	stq	ra, 0(sp)        # may be able to delete this
	addq	zero, 0x4, a0

	call_pal PAL_swpipl
	bis	zero,zero,zero  # add lock_pmap(map) 

	stq	v0, 72(sp)
	ornot   zero,zero,t1
	srl	t1, 0x15, t1
	and     a1, t1, a1        # and startva and mask
	srl	a1, 0xd, a1
	ornot	zero,zero,t2
	sll	t2, 0xf, t2
	bis	zero,zero,zero

	ldah	t2, -32768(zero)
	s8addq	a1, 0, a1         
	s8addq	t2, zero, t2
	addq	a1, t2, a0       # a0 has the pte pointer

	bis	zero, 0x1, t2
	sll	t2, 0x11, t2      # lww mask
	ornot	zero, t2, t2
	bis	zero, 0x1, t1
	sll	t1, 0x12, t1
	bis	t1, 0x1, t1	  # valid | seg
	

	s8addq	a2, a0, t5        # end limit
	ornot	zero, 0x3f, t8
	addq	a0, 0x3f, t3
	and	t3, t8, t3        # beg limit
	and	t5, t8, t4	  # middle limit
	subq	t4, 0x40, a4      # middle prefetch limit

	cmpult  a0, t3, t8
	bne	t8, top_first_init
	cmpult  a0, t4, t8
	bne	t8, start_aligned
	cmpult	a0, t5, t8
	bne	t8, top_last
	br	zero, end_good
	bis	zero,zero,zero

.align 3

top_first_init:

	cmpult	t3, t5, t8
	bne	t8, top_first
	bis	t5, t5, t3	# copy end limit to beg limit
	bis	t5, t5, t4	# copy end limit to middle limit

top_first:

	ldq    s0, 0(a0)         # load pte into scratch area
	and    s0, t1, t11       # check valid bit and not segment
	subq   t11, 0x1, t8
	bne    t8, error_ret1

	and    s0, t2, t8         # dual,  set lww bit in pte
	stq    t8, 0(a0)          # update pte

	addq	a0, 0x8, a0
	cmpult	a0, t3, t8
	bne	t8, top_first
	bis	zero,zero,zero

	cmpult  a0, t4, t8
	bne	t8, start_aligned
	cmpult	a0, t5, t8
	bne	t8, top_last
	br	zero, end_good
	bis	zero, zero, zero


.align 3

start_aligned:

	ldq	t6, 0(a0)        # prefetch
	ldq	t7, 32(a0)       # prefetch
	bis	t6, t6, s0
	bis	t7, t7, s4
	cmpule	a4, a0, t8
	bne	t8, skip1

top_aligned:

/*	rpcc	ra
 *	stq	ra, 80(sp)
 */


	ldq    s1, 8(a0)
	and    s0, t1, t11     # start valid bit check
	ldq    s2, 16(a0)
	addq   zero, t11, t12  # running sum of valid bits

	ldq    s3, 24(a0)      
	and    s4, t1, t11
	ldq    s5, 40(a0)
	addq   t12, t11, t12

	ldq    s6, 48(a0) 
	and    s1, t1, t11 
	ldq    a5, 56(a0)
	addq   t12, t11, t12

	ldq	t6, 64(a0)      # miss
	bis	zero,zero,zero

	and    s2, t1, t11
	addq   t12, t11, t12

	and    s3, t1, t11
	addq   t12, t11, t12
	and    s5, t1, t11
	addq   t12, t11, t12
	and    s6, t1, t11
	addq   t12, t11, t12

	ldq	t7, 96(a0)     # miss
	and    a5, t1, t11

	addq   t12, t11, t12
	subq   t12, 0x8, t8    

	bne    t8, error_ret2
	bis	zero,zero,zero 

/*	rpcc	ra
 *	stq	ra, 88(sp)
 */

	and   s0, t2, s0
	stq   s0, 0(a0)
	and   s1, t2, s1
	stq   s1, 8(a0)
	and   s2, t2, s2
	stq   s2, 16(a0)
	and   s3, t2, s3
	stq   s3, 24(a0)
	and   s4, t2, s4
	stq   s4, 32(a0)
	and   s5, t2, s5
	stq   s5, 40(a0)
	and   s6, t2, s6
	stq   s6, 48(a0)
	and   a5, t2, a5
	stq   a5, 56(a0)


	bis	t6, t6, s0
	bis	t7, t7, s4
	addq	a0, 0x40, a0
	cmpult	a0, t4, t8
	cmpule	a4, a0, s6
	bis	zero, zero, zero


	bne	s6, skip1
	bne	t8, top_aligned
	cmpult	a0, t5, t8
	bne	t8, top_last
	br	zero, end_good

skip1:

	ldq    s1, 8(a0)
	and    s0, t1, t11     # start valid bit check
	ldq    s2, 16(a0)
	addq   zero, t11, t12  # running sum of valid bits

	ldq    s3, 24(a0)      
	and    s4, t1, t11
	ldq    s5, 40(a0)
	addq   t12, t11, t12

	ldq    s6, 48(a0) 
	ldq    a5, 56(a0)

	and    s1, t1, t11 
	addq   t12, t11, t12
	and    s2, t1, t11
	addq   t12, t11, t12

	and    s3, t1, t11
	addq   t12, t11, t12
	and    s5, t1, t11
	addq   t12, t11, t12
	and    s6, t1, t11
	addq   t12, t11, t12

	bis	zero,zero,zero
	and    a5, t1, t11

	addq   t12, t11, t12
	subq   t12, 0x8, t8    

	bne    t8, error_ret2
	bis	zero,zero,zero 

	and   s0, t2, s0
	stq   s0, 0(a0)
	and   s1, t2, s1
	stq   s1, 8(a0)
	and   s2, t2, s2
	stq   s2, 16(a0)
	and   s3, t2, s3
	stq   s3, 24(a0)
	and   s4, t2, s4
	stq   s4, 32(a0)
	and   s5, t2, s5
	stq   s5, 40(a0)
	and   s6, t2, s6
	stq   s6, 48(a0)
	and   a5, t2, a5
	stq   a5, 56(a0)

	addq	a0, 0x40, a0
	cmpult	a0, t5, t8
	bne	t8, top_last
	br	zero, end_good

top_last:

	ldq    s0, 0(a0)
	and    s0, t1, t11
	subq   t11, 0x1, t8
	bne    t8, error_ret3


	and   s0, t2, t8         # dual
	stq   t8, 0(a0)

	addq	a0, 0x8, a0
	cmpult	a0, t5, t9
	bne	t9, top_last
	br      zero,end_good

error_ret1:
	ldq	a0, 72(sp)
	call_pal PAL_swpipl
	subq	zero, 0x1, v0
	br	zero,end_common

error_ret2:
	ldq	a0, 72(sp)
	call_pal PAL_swpipl
	subq	zero, 0x2, v0
	br	zero,end_common

error_ret3:
	ldq	a0, 72(sp)
	call_pal PAL_swpipl
	subq	zero, 0x3, v0
	br	zero,end_common

end_good:
	ldq	a0, 72(sp)
	call_pal PAL_swpipl
	bis	zero,zero,v0

end_common:

				/* unlock pmap */
	ldq	ra, 0(sp)
	ldq	s0, 8(sp)
	ldq	s1, 16(sp)
	ldq	s2, 24(sp)
	ldq	s3, 32(sp)
	ldq	s4, 40(sp)
	ldq	s5, 48(sp)
	ldq	s6, 56(sp)
	lda	sp, 192(sp)

	ret	zero,(ra)
	END(pmap_lw_unwire_ass)

