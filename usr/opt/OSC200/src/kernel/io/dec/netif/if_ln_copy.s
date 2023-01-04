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
 *	@(#)$RCSfile: if_ln_copy.s,v $ $Revision: 1.2.4.2 $ (DEC) $Date: 1992/02/12 18:44:42 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from if_ln_copy.s	1.1      (OSF)  2/26/91";
 */
/************************************************************************
 *
 *	Modification History: if_ln_copy.s
 *
 * 03-Nov-91	Fred Canter
 *	Added Vasu Subramanian's assembly language fast copy routines.
 *
 * 19-Jan-91	Randall Brown
 *	Created file for copy routines written in assembly for performance
 *	reasons.
 */

#include <machine/asm.h>
#include <machine/reg.h>
#include <machine/regdef.h>
#include <machine/cpu.h>
#include <assym.s>


LEAF(ln_cpyout4x4)
	.set 	noreorder

/*
 * caddr_t
 * ln_cpyout4x4(from,to,len,off)
 * caddr_t from;				# a0 = from
 * caddr_t to;					# a1 = to
 * int len, off;				# a2 = len, a3 = off
 * {
 * 	register tmp1;
 * 	register caddr_t dp = from;
 * 	register int i;
 * 	int end4word;
 * 	register caddr_t lrbptr = (to+off);
 */
 	addu	a1, a1, a3

/*
 * 	if ((u_long) lrbptr & 0xf) {
 * 
 * 		end4word = (0x10 - ((u_long)lrbptr & 0xf));
 * 		tmp1 = ((len > end4word) ? end4word : len);
 * 
 * 		len -= tmp1;
 * 		while (tmp1--)
 * 		    *lrbptr++ = *dp++;
 * 
 *              if (((u_long)lrbptr & 0xf) == 0)
 *                  lrbptr += 0x10;
 * 	}
 */
 	andi	t6,a1,0xf	# is lrbptr on 4 word boundary

 	beq	t6,zero,5f	# if yes, do 4 word copy
 	nop

 	li	t7,16		# determine number of bytes
 	subu	t7,t7,t6	# to get to 4 word boundary
 	slt	t5,t7,a2	# if number is less than len
 	beq	t5,zero,2f	# only copy len
 	move	t8,a2
 	move	t8,t7

2:	subu	a2,a2,t8	# decrement total len

 	beq	t8,zero,4f	
	nop
3: 	lb	t9,0(a0)	# copy 1 byte at a time
	addiu	t8,t8,-1	# until tmp1(t8) is zero
 	sb	t9,0(a1)	
 	addiu	a0,a0,1
 	addiu	a1,a1,1
 	bne	t8,zero,3b
	nop

4:	andi	t3,a1,0xf	# if lrbptr on 4 word boundary
	bne	t3,zero,6f	# skip 4 word hole
	sra	t8,a2,4
	addiu	a1,a1,16

/*
 * 
 * 	tmp1 = (len >> 4);
 * 
 * 	len -= (tmp1 << 4);
 */
5:	sra	t8,a2,4		# t8 = len >> 4
6:	sll	t0,t8,4		
	subu	a2,a2,t0

/*
 *	NOTE: if dp is not aligned to a word boundary, then
 *	      lwl and lwr is used to fill in the bytes.
 *
 * 	for (; tmp1; tmp1--) {
 * 
 *         	register int temp0, temp1, temp2, temp3;
 * 
 * 	        temp0 = ((u_int *)dp)[0];
 * 		temp1 = ((u_int *)dp)[1];
 * 		temp2 = ((u_int *)dp)[2];
 * 		temp3 = ((u_int *)dp)[3];
 * 		((u_int *)lrbptr)[0] = temp0;
 * 		((u_int *)lrbptr)[1] = temp1;
 * 		((u_int *)lrbptr)[2] = temp2;
 * 		((u_int *)lrbptr)[3] = temp3;
 * 
 * 		lrbptr += 32;
 * 		dp += 16;
 * 	}
 */
 	beq	t8,zero,9f	# tmp1 == 0, then get fragment
	andi	t6,a0,0x3	# is dp word aligned ?
	bne	t6,zero,1f	# no, do unaligned copy
	andi	t7,t8,0x3	# number of 4 word blocks to get 
	beq	t7,zero,8f	#     16 word aligned
	subu	t8,t8,t7	# decrement total count 
7:	lw	t0,0(a0)	# do 4 word block until we can
	lw	t1,4(a0)	#     do 16 words blocks
	lw	t2,8(a0)
	lw	t3,12(a0)
	sw	t0,0(a1)
	sw	t1,4(a1)
	sw	t2,8(a1)
	sw	t3,12(a1)
	addiu	t7,t7,-1	# decrement counter
	addiu	a0,a0,16	# increment dp by 16
	bne	t7,zero,7b
	addiu	a1,a1,32	# increment lrbptr by 32
	beq	t8,zero,9f
	nop
8:	lw	t0,0(a0)	# do copy 16 words at a time
	lw	t1,4(a0)
	lw	t2,8(a0)
	lw	t3,12(a0)
	lw	t4,16(a0)	# no hole to skip
	lw	t5,20(a0)
	lw	t6,24(a0)
	lw	t7,28(a0)
	sw	t0,0(a1)
	sw	t1,4(a1)
	sw	t2,8(a1)
	sw	t3,12(a1)
	sw	t4,32(a1)	# skip 4 word hole
	sw	t5,36(a1)
	sw	t6,40(a1)
	sw	t7,44(a1)
	lw	t0,32(a0)
	lw	t1,36(a0)
	lw	t2,40(a0)
	lw	t3,44(a0)
	lw	t4,48(a0)
	lw	t5,52(a0)
	lw	t6,56(a0)
	lw	t7,60(a0)
	sw	t0,64(a1)
	sw	t1,68(a1)
	sw	t2,72(a1)
	sw	t3,76(a1)
	sw	t4,96(a1)
	sw	t5,100(a1)
	sw	t6,104(a1)
	sw	t7,108(a1)
	addiu	t8,t8,-4	# decrement tmp1 by 4
	addiu	a0,a0,64	# add 64 to dp
	bne	t8,zero,8b	# repeat if tmp != 0
	addiu	a1,a1,128	# add 128 to lrbptr
	b	9f
	nop
1:	lwr	t0,0(a0)
	lwl	t0,3(a0)
	lwr	t1,4(a0)
	lwl	t1,7(a0)
	lwr	t2,8(a0)
	lwl	t2,11(a0)
	lwr	t3,12(a0)
	lwl	t3,15(a0)
	sw	t0,0(a1)
	sw	t1,4(a1)
	sw	t2,8(a1)
	sw	t3,12(a1)
	addiu	a0,a0,16
	addiu	t8,t8,-1
	bne	t8,zero,1b
	addiu	a1,a1,32

/*
 * 
 *
 * 	if (len) {
 * 	        tmp1 = len;		
 * 		while (tmp1--)
 * 		    *lrbptr++ = *dp++;	
 * 	}
 * 	return(lrbptr);
 */
9:	beq	a2,zero,2f
	nop
1:	lb	t0,0(a0)
	addiu	a2,a2,-1
	sb	t0,0(a1)
	addiu	a0,a0,1
	addiu	a1,a1,1
	bne	a2,zero,1b
	nop

2:	move	v0,a1
	j	ra
	nop

	.set	reorder
	END(ln_cpyout4x4)

/*
 * ln_clean_dache4x4(addr, len)
 *	a0 = addr
 *	a1 = len
 */
LEAF(ln_clean_dcache4x4)
	.set noreorder
	nop
	mfc0	t3,C0_SR		# save sr
	nop

	li	v0,SR_ISC		# disable interrupts, isolate caches
	mtc0	v0,C0_SR

	nop
	nop
	nop
	nop				# cache must be isolated by now

1:	sb	zero,0(a0)
	sb	zero,4(a0)
	sb	zero,8(a0)
	sb	zero,12(a0)
	addiu	a1,a1,-1
	bne	a1,zero,1b
	addiu	a0,a0,32		# skip 4 word hole

	nop				# insure isolated stores out of pipe
	nop
	nop
	mtc0	t3,C0_SR		# un-isolate, enable interrupts
	nop				# insure cache unisolated
	nop				# insure cache unisolated
	nop				# insure cache unisolated
	j	ra
	nop
	.set	reorder
	END(ln_clean_dcache)

LEAF(ln_cpyin4x4s)
	.set 	noreorder

/*
 * caddr_t
 * ln_cpyin4x4s(lrbptr,dp,len)
 * caddr_t lrbptr;				# a0 = from
 * caddr_t dp;					# a1 = to
 * int len;					# a2 = len
 * {
 * 	register tmp1;
 * 	int end4word;
 */

/*
 * 	if ((u_long) lrbptr & 0xf) {
 * 
 * 		end4word = (0x10 - ((u_long)lrbptr & 0xf));
 * 		tmp1 = ((len > end4word) ? end4word : len);
 * 
 * 		len -= tmp1;
 * 		while (tmp1--)
 * 		    *dp++ = *lrbptr++;
 * 
 *              if (((u_long)lrbptr & 0xf) == 0)
 *                  lrbptr += 0x10;
 * 	}
 */
 	andi	t6,a0,0xf	# is lrbptr on 4 word boundary

 	beq	t6,zero,5f	# if yes, do 4 word copy
 	nop

 	li	t7,16		# determine number of bytes
 	subu	t7,t7,t6	# to get to 4 word boundary
 	slt	t5,t7,a2	# if number is less than len
 	beq	t5,zero,2f	# only copy len
 	move	t8,a2
 	move	t8,t7

2:	subu	a2,a2,t8	# decrement total len

 	beq	t8,zero,4f	
	nop
3: 	lb	t9,0(a0)	# copy 1 byte at a time
	addiu	t8,t8,-1	# until tmp1(t8) is zero
 	sb	t9,0(a1)	
 	addiu	a0,a0,1
 	addiu	a1,a1,1
 	bne	t8,zero,3b
	nop

4:	andi	t3,a0,0xf	# if lrbptr on 4 word boundary
	bne	t3,zero,6f	# skip 4 word hole
	sra	t8,a2,4
	addiu	a0,a0,16

/*
 * 
 * 	tmp1 = (len >> 4);
 * 
 * 	len -= (tmp1 << 4);
 */
5:	sra	t8,a2,4		# t8 = len >> 4
6:	sll	t0,t8,4		
	subu	a2,a2,t0

/*
 *	NOTE: if dp is not aligned to a word boundary, then
 *	      swl and swr is used to fill in the bytes.
 *
 * 	for (; tmp1; tmp1--) {
 * 
 *         	register int temp0, temp1, temp2, temp3;
 * 
 * 	        temp0 = ((u_int *)dp)[0];
 * 		temp1 = ((u_int *)dp)[1];
 * 		temp2 = ((u_int *)dp)[2];
 * 		temp3 = ((u_int *)dp)[3];
 * 		((u_int *)lrbptr)[0] = temp0;
 * 		((u_int *)lrbptr)[1] = temp1;
 * 		((u_int *)lrbptr)[2] = temp2;
 * 		((u_int *)lrbptr)[3] = temp3;
 * 
 * 		lrbptr += 32;
 * 		dp += 16;
 * 	}
 */
 	beq	t8,zero,9f	# tmp1 == 0, then get fragment
	andi	t6,a1,0x3	# is dp word aligned ?
	bne	t6,zero,1f	# no, do unaligned copy
	andi	t7,t8,0x3	# number of 4 word blocks to get 
	beq	t7,zero,8f	#     16 word aligned
	subu	t8,t8,t7	# decrement total count 
7:	lw	t0,0(a0)	# do 4 word block until we can
	lw	t1,4(a0)	#     do 16 words blocks
	lw	t2,8(a0)
	lw	t3,12(a0)
	sw	t0,0(a1)
	sw	t1,4(a1)
	sw	t2,8(a1)
	sw	t3,12(a1)
	addiu	t7,t7,-1	# decrement counter
	addiu	a1,a1,16	# increment dp by 16
	bne	t7,zero,7b
	addiu	a0,a0,32	# increment lrbptr by 32
	beq	t8,zero,9f
	nop
8:	lw	t0,0(a0)	# do copy 16 words at a time
	lw	t1,4(a0)
	lw	t2,8(a0)
	lw	t3,12(a0)
	lw	t4,32(a0)	# no hole to skip
	lw	t5,36(a0)
	lw	t6,40(a0)
	lw	t7,44(a0)
	sw	t0,0(a1)
	sw	t1,4(a1)
	sw	t2,8(a1)
	sw	t3,12(a1)
	sw	t4,16(a1)	# skip 4 word hole
	sw	t5,20(a1)
	sw	t6,24(a1)
	sw	t7,28(a1)
	lw	t0,64(a0)
	lw	t1,68(a0)
	lw	t2,72(a0)
	lw	t3,76(a0)
	lw	t4,96(a0)
	lw	t5,100(a0)
	lw	t6,104(a0)
	lw	t7,108(a0)
	sw	t0,32(a1)
	sw	t1,36(a1)
	sw	t2,40(a1)
	sw	t3,44(a1)
	sw	t4,48(a1)
	sw	t5,52(a1)
	sw	t6,56(a1)
	sw	t7,60(a1)
	addiu	t8,t8,-4	# decrement tmp1 by 4
	addiu	a1,a1,64	# add 64 to dp
	bne	t8,zero,8b	# repeat if tmp != 0
	addiu	a0,a0,128	# add 128 to lrbptr
	b	9f
	nop
1:	lw	t0,0(a0)
	lw	t1,4(a0)
	lw	t2,8(a0)
	lw	t3,12(a0)
	swr	t0,0(a1)
	swl	t0,3(a1)
	swr	t1,4(a1)
	swl	t1,7(a1)
	swr	t2,8(a1)
	swl	t2,11(a1)
	swr	t3,12(a1)
	swl	t3,15(a1)
	addiu	a1,a1,16
	addiu	t8,t8,-1
	bne	t8,zero,1b
	addiu	a0,a0,32

/*
 * 
 *
 * 	if (len) {
 * 	        tmp1 = len;		
 * 		while (tmp1--)
 * 		    *dp++ = *lrbptr++;	
 * 	}
 * 	return(lrbptr);
 */
9:	beq	a2,zero,2f
	nop
1:	lb	t0,0(a0)
	addiu	a2,a2,-1
	sb	t0,0(a1)
	addiu	a0,a0,1
	addiu	a1,a1,1
	bne	a2,zero,1b
	nop

2:	move	v0,a0
	j	ra
	nop

	.set	reorder
	END(ln_cpyin4x4s)


/*
 * Specialized "bcopy" to move len bytes into
 * 16-bit wide by 32-bit aligned local RAM buffer.
 * Loop does up to 32 short-word moves in a single
 * shot until the buffer is depleted. "ln_hold" is
 * used for MIPS to short-word align buffers.

* Arguments:
 *      from (src)      The address to copy from.
 *      to (dst)        The address to copy to.
 *      len             The number of bytes to copy.
 *      off             The offset, if any.
 *
 * Return Value:
 *      The lrb pointer incremented by the number of bytes copied.
 */

LEAF(as_ln_cpyout16)

	subu	sp, 56
	sw	ra, 36(sp)
	sd	a0, 56(sp)
	sd	a2, 64(sp)
	sw	s3, 32(sp)
	sw	s2, 28(sp)
	sw	s1, 24(sp)
	sw	s0, 20(sp)
 
	lw	s2, 56(sp)

 	                                  /* If offset is not zero  */
                                          /* Change the DST address accordingly */   
	lw	t6, 60(sp)                /* Load DST address */
	lw	t8, 68(sp)                /* Load the offset value */
	and	t7, t6, 1
	mul	t9, t8, 2
	rem	t1, t8, 2

        .set noreorder
	beq	t7, $0, 1f
	addu	t0, t6, t9
        .set reorder

        .set noreorder
	b	2f
	addu	s3, t0, t1
        .set reorder
1:
	subu	s3, t0, t1
2:
	lw	s0, 64(sp)
                                	/* Is the DST odd alligned */
	and	t6, s3, 1
	beq	t6, $0, 3f
	
 					/* Start LRB on even short-word boundary */

	addu	t9, s3, -1
	lw	t8, 0(t9)
	lb	t0, 0(s2)
	and	t1, t0, 255
	sll	t2, t1, 8
	and	t4, t2, 65535
	or	t3, t8, t4
	sw	t3, 0(t9)
	addu	s2, s2, 1
	addu	s3, s3, 3
 
	addu	s0, s0, -1
3:
	
                         	/* Is SRC unalligned */
	beq	s0, $0, 4f      /* Is LEN == 0 */
	and	t7, s2, 1
	beq	t7, $0, 4f
 		               /* copy SRC to ln_hold which is 
                                  an pre-defined unalligned area */     
	move	a0, s2
	la	a1, ln_hold
	move	a2, s0
	jal	bcopy
 		                  
	la	s2, ln_hold    /* make SRC to be ln_hold */
4:
	

/*

This is where the transfers from USER space to the LRB is done. 
REG s0 has the length of the transfer
REG s2 has the user space address 
REG s3 is the LRBPTR

First check if LEN == 0. 
Convert LEN ( in bytes) to word count. Length in word is in REG s1. 
If Word Count >= 33, start at the top of case statement. 
In consecutive memory locations are the addresses of labels to branch 
to if <= 33 Word Counts have to be transferred. 
The label cpyout_begin_case is the start address.  
In consecutive locations of rdata section  are stored the addresses 
to branch to depending on the LEN to be transferred. 

*/
                                	/* While LEN > 0 */
	beq	s0, $0, ln_cpyout16_done
ln_cpyout16_start:
                                       /* switch to word count */
                                       /* REG s1 has word count */
	sra	s1, s0, 1
	bgeu	s1, 33, 33f            /* If LEN >= 33 start at top 
                                          of case statement */
	sll	t6, s1, 2
	lw	t6, cpyout_begin_case(t6)
	j	t6

	.rdata	
cpyout_begin_case:
	.word	cpyout_end_case
	.word	1f
	.word	2f
	.word	3f
	.word	4f
	.word	5f
	.word	6f
	.word	7f
	.word	8f
	.word	9f
	.word	10f
	.word	11f
	.word	12f
	.word	13f
	.word	14f
	.word	15f
	.word	16f
	.word	17f
	.word	18f
	.word	19f
	.word	20f
	.word	21f
	.word	22f
	.word	23f
	.word	24f
	.word	25f
	.word	26f
	.word	27f
	.word	28f
	.word	29f
	.word	30f
	.word	31f
	.word	32f
	.text	
 	    					/*default:*/
33:
 		   			/*	case 32:  */
32:
	li	s1, 32
 
	lhu	t0, 62(s2)
	sw	t0, 124(s3)
 	    				/*	case 31:  */
31:
	lhu	t8, 60(s2)
	sw	t8, 120(s3)
 	    				/*	case 30:  */
30:
	lhu	t5, 58(s2)
	sw	t5, 116(s3)
 	    				/*	case 29:  */
29:
	lhu	t9, 56(s2)
	sw	t9, 112(s3)
 	    				/*	case 28:  */
28:
	lhu	t2, 54(s2)
	sw	t2, 108(s3)
 	    				/*	case 27:  */
27:
	lhu	t3, 52(s2)
	sw	t3, 104(s3)
 	    				/*	case 26:  */
26:
	lhu	t6, 50(s2)
	sw	t6, 100(s3)
 	    				/*	case 25:  */
25:
	lhu	t1, 48(s2)
	sw	t1, 96(s3)
 	    				/*	case 24:  */
24:
	lhu	t4, 46(s2)
	sw	t4, 92(s3)
 	    				/*	case 23:  */
23:
	lhu	t7, 44(s2)
	sw	t7, 88(s3)
 	    				/*	case 22:  */
22:
	lhu	t0, 42(s2)
	sw	t0, 84(s3)
 	    				/*	case 21:  */
21:
	lhu	t8, 40(s2)
	sw	t8, 80(s3)
 	    				/*	case 20:  */
20:
	lhu	t5, 38(s2)
	sw	t5, 76(s3)
 	    				/*	case 19:  */
19:
	lhu	t9, 36(s2)
	sw	t9, 72(s3)
 	    				/*	case 18:  */
18:
	lhu	t2, 34(s2)
	sw	t2, 68(s3)
 	    				/*	case 17:  */
17:
	lhu	t3, 32(s2)
	sw	t3, 64(s3)
 	    				/*	case 16:  */
16:
	lhu	t6, 30(s2)
	sw	t6, 60(s3)
 	    				/*	case 15:  */
15:
	lhu	t1, 28(s2)
	sw	t1, 56(s3)
 	    				/*	case 14:  */
14:
	lhu	t4, 26(s2)
	sw	t4, 52(s3)
 	    				/*	case 13:  */
13:
	lhu	t7, 24(s2)
	sw	t7, 48(s3)
 	    				/*	case 12:  */
12:
	lhu	t0, 22(s2)
	sw	t0, 44(s3)
 	    				/*	case 11:  */
11:
	lhu	t8, 20(s2)
	sw	t8, 40(s3)
 	    				/*	case 10:  */
10:
	lhu	t5, 18(s2)
	sw	t5, 36(s3)
 	    				/*	case 9:  */
9:
	lhu	t9, 16(s2)
	sw	t9, 32(s3)
 	    				/*	case 8:  */
8:
	lhu	t2, 14(s2)
	sw	t2, 28(s3)
 	    				/*	case 7:  */
7:
	lhu	t3, 12(s2)
	sw	t3, 24(s3)
 	    				/*	case 6:  */
6:
	lhu	t6, 10(s2)
	sw	t6, 20(s3)
 	    				/*	case 5:  */
5:
	lhu	t1, 8(s2)
	sw	t1, 16(s3)
 	    				/*	case 4:  */
4:
	lhu	t4, 6(s2)
	sw	t4, 12(s3)
 	    				/*	case 3:  */
3:
	lhu	t7, 4(s2)
	sw	t7, 8(s3)
 	    				/*	case 2:  */
2:
	lhu	t0, 2(s2)
	sw	t0, 4(s3)
 	    				/*	case 1:  */
1:
	lhu	t2, 0(s2)
	sw	t2, 0(s3)
 		    		/* Actually did some word moves */
                                /* Adjust SRC and DST pointers */
	sll	t7, s1, 2
	addu	s3, s3, t7
	
	sll	t6, s1, 1
	addu	s2, s2, t6
 		    		/* adjust LEN */	
        .set noreorder
	bne	s0, $0, ln_cpyout16_start  /* LEN > 0; loop back */
	subu	s0, s0, t6
        .set reorder

 	    				/*	case 0:  */
cpyout_end_case:
                     			/* Was LEN odd bytes */
	and	t8, s0, 1
	beq	t8, $0, ln_cpyout16_done

 					/* One lousy byte left over! */
	lb	t4, 0(s2)
	addu	s3, s3, 1
	and	t3, t4, 255
	sw	t3, -1(s3)

ln_cpyout16_done:
	move	v0, s3           /* return SRC */
	lw	s0, 20(sp)
	lw	s1, 24(sp)
	lw	s2, 28(sp)
	lw	s3, 32(sp)
	lw	ra, 36(sp)
	addu	sp, 56
	j	ra
	END(as_ln_cpyout16)



/*
 * Specialized "bcopy" to move len bytes from
 * 16-bit wide by 32-bit aligned local RAM buffer.
 * Loop does up to 32 short-word moves in a single
 * shot until the buffer is depleted. Off is non-zero
 * if we wish to begin copying off-many bytes beyond
 * "from".
 *
 * Arguments:
 *      sc              The softc structure for the interface.
 *      from            The address to copy from.
 *      to              The address to copy to.
 *      len             The number of bytes to copy.
 *      off             The offset, if any.
 *
 * Return Value:
 *      The local RAM buffer pointer (lrbptr).
 */


LEAF(as_ln_cpyin16)

	subu	sp, 64
	sw	ra, 36(sp)
	sd	a0, 64(sp)
	sd	a2, 72(sp)
	sw	s4, 32(sp)
	sw	s2, 28(sp)
	sw	s1, 24(sp)
	sw	s0, 20(sp)
 				/* Calculate the new SRC address given 
                                    from and offset */
	lw	t6, 68(sp)      /* load start, REG a1 stored above */
	lw	t8, 80(sp)      /* load offset */
	and	t7, t6, 1
	mul	t9, t8, 2
	rem	t1, t8, 2

        .set noreorder
	beq	t7, $0, 1f
	addu	t0, t6, t9
        .set reorder
                                
        .set noreorder
	b	2f
	addu	s0, t0, t1      /* REG so has the new SRC address */
        .set reorder

1:
	subu	s0, t0, t1
2:
	lw	t6, 64(sp)
	lw	s1, 72(sp)        /* DST address is in REG s1 */
	lw	t9, 2900(t6)
	
	lw	s2, 76(sp)        /* LEN is in REG s2 */
				  /* no read thru caching */
	lw	t8, 36(t9)
	beq	t8, $0, 3f

 	/*	if((svatophys(lrbptr, &phy)) == KERN_INVALID_ADDRESS) */

	move	a0, s0
	addu	a1, sp, 44
	jal	pmap_svatophys
	bne	v0, 1, 4f
	
        PANIC(" ln_cpyin16: Invalid physical address!\n");
4:
 			/* 	clean_dcache(PHYS_TO_K0(phy),len*2);   */
	lw	a0, 44(sp)
	or	a0, a0, -2147483648
	lw	a1, 76(sp)
	mul	a1, a1, 2
	jal	clean_dcache
3:
 					/* Is the SRC odd alligned */
	and	t0, s0, 1
	beq	t0, $0, 5f
 				/* Start LRB on even short-word boundary */

	lb	t1, 0(s0)
	addu	s1, s1, 1
	sb	t1, -1(s1)
	
	addu	s0, s0, 3
	addu	s2, s2, -1
5:


/*

This is where the transfers from LRB to USER space is done. 
REG s2 has the length of the transfer
REG s1 has the user space address 
REG s0 is the LRBPTR

First check if LEN == 0. 
Convert LEN ( in bytes) to word count. Length in word is in REG s4. 
If Word Count >= 33, start at the top of case statement. 
In consecutive memory locations are the addresses of labels to branch 
to if <= 33 Word Counts have to be transferred. 
The label cpyin_begin_case is the start address.  
In consecutive locations of rdata section  are stored the addresses 
to branch to depending on the LEN to be transferred. 

*/
                                	/* While LEN > 0 */
 					    	/*	while LEN > 0 */
	beq	s2, $0, ln_cpyin16_done
ln_cpyin16_start:
         					/*  shift to word count */
                                                /* REG s4 has word count */
	sra	s4, s2, 1
	bgeu	s4, 33, 33f                     /* If LEN is >= 33 start at 
                                                   top of case statement */
	sll	t2, s4, 2
	lw	t2, cpyin_begin_case(t2)        /* jump into appropriate point 
                                                   of case statement */
	j	t2

	.rdata	
cpyin_begin_case:
	.word	cpyin_end_case
	.word	1f
	.word	2f
	.word	3f
	.word	4f
	.word	5f
	.word	6f
	.word	7f
	.word	8f
	.word	9f
	.word	10f
	.word	11f
	.word	12f
	.word	13f
	.word	14f
	.word	15f
	.word	16f
	.word	17f
	.word	18f
	.word	19f
	.word	20f
	.word	21f
	.word	22f
	.word	23f
	.word	24f
	.word	25f
	.word	26f
	.word	27f
	.word	28f
	.word	29f
	.word	30f
	.word	31f
	.word	32f
	.text	
 					/*	default:  */
33:
 					/*	case 32: */
32:
	li	s4, 32
 
	lw	t3, 124(s0)
	sh	t3, 62(s1)
 					/*	case 31:  */
31:
	lw	t6, 120(s0)
	sh	t6, 60(s1)
 					/*	case 30:  */
30:
	lw	t0, 116(s0)
	sh	t0, 58(s1)
 				       /*	case 29:  */
29:
	lw	t4, 112(s0)
	sh	t4, 56(s1)
 					/*	case 28:  */
28:
	lw	t7, 108(s0)
	sh	t7, 54(s1)
 					/*	case 27:  */
27:
	lw	t8, 104(s0)
	sh	t8, 52(s1)
 				       /*	case 26:   */
26:
	lw	t2, 100(s0)
	sh	t2, 50(s1)
 					/*	case 25:   */
25:
	lw	t5, 96(s0)
	sh	t5, 48(s1)
 					/*	case 24:   */
24:
	lw	t9, 92(s0)
	sh	t9, 46(s1)
 					/*	case 23:   */
23:
	lw	t1, 88(s0)
	sh	t1, 44(s1)
 					/*	case 22:   */
22:
	lw	t3, 84(s0)
	sh	t3, 42(s1)
 					/*	case 21:  */
21:
	lw	t6, 80(s0)
	sh	t6, 40(s1)
 					/*	case 20:   */
20:
	lw	t0, 76(s0)
	sh	t0, 38(s1)
					/*	case 19:   */
19:
	lw	t4, 72(s0)
	sh	t4, 36(s1)
 					/*	case 18:   */
18:
	lw	t7, 68(s0)
	sh	t7, 34(s1)
	 				/*	case 17:    */
17:
	lw	t8, 64(s0)
	sh	t8, 32(s1)
 				       /*	case 16:  */
16:
	lw	t2, 60(s0)
	sh	t2, 30(s1)
 					/*	case 15:   */
15:
	lw	t5, 56(s0)
	sh	t5, 28(s1)
 					/*	case 14:   */
14:
	lw	t9, 52(s0)
	sh	t9, 26(s1)
 					/*	case 13:   */
13:
	lw	t1, 48(s0)
	sh	t1, 24(s1)
	
 					/*	case 12:   */
12:
	lw	t3, 44(s0)
	sh	t3, 22(s1)
	
 				       /*	case 11:  */
11:
	lw	t6, 40(s0)
	sh	t6, 20(s1)
					/*	case 10:   */
10:
	lw	t0, 36(s0)
	sh	t0, 18(s1)
	 				/*	case  9:   */
9:
	lw	t4, 32(s0)
	sh	t4, 16(s1)
	 				/*	case  8:   */
8:
	lw	t7, 28(s0)
	sh	t7, 14(s1)
	 				/*	case  7:   */
7:
	lw	t8, 24(s0)
	sh	t8, 12(s1)
 				       /*	case  6:  */
6:
	lw	t2, 20(s0)
	sh	t2, 10(s1)
	 				/*	case  5:  */
5:
	lw	t5, 16(s0)
	sh	t5, 8(s1)
 				       /*	case  4:  */
4:
	lw	t9, 12(s0)
	sh	t9, 6(s1)
 				       /*	case  3:  */
3:
	lw	t1, 8(s0)
	sh	t1, 4(s1)
 					/*	case  2:  */
2:
	lw	t3, 4(s0)
	sh	t3, 2(s1)
 					/*	case  1:  */
1:
	lw	t7, 0(s0)
	sh	t7, 0(s1)
 					/* Actually did some word moves */
                                        /* adjust the SRC and DST pointers */
	sll	t8, s4, 2
	addu	s0, s0, t8
 
	sll	t0, s4, 1
	addu	s1, s1, t0

                                        /* adjust the LEN */
        .set noreorder
	bne	s2, $0, ln_cpyin16_start     
	subu	s2, s2, t0  		/* LEN > 0 ; loop back */ 
        .set reorder

 					/*	case 0:   */
cpyin_end_case:
 					/*   was the LEN odd */
	and	t6, s2, 1
	beq	t6, $0, ln_cpyin16_done
 					/* One lousy byte left over! */
	lb	t9, 0(s0)
	addu	s0, s0, 1
	sb	t9, 0(s1)

ln_cpyin16_done:
	move	v0, s0                  /* return the SRC pointer */
	lw	s0, 20(sp)
	lw	s1, 24(sp)
	lw	s2, 28(sp)
	lw	s4, 32(sp)
	lw	ra, 36(sp)
	addu	sp, 64
	j	ra

	END(as_ln_cpyin16)
