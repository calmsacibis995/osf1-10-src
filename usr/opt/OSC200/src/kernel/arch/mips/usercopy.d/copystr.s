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
 * @(#)$RCSfile: copystr.s,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/06/03 11:26:49 $
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
 * Copy a null terminated string from one point to another in 
 * kernel address space.
 *
 * copystr(src, dest, maxlength, &lencopied)
 *	returns:
 *		0	- success
 *		EFAULT	- bogus length
 *		ENOENT	- string exceeded maxlength
 */
LEAF(copystr)
	/* blez	a2,cstrerror	# length must be positive */
	bgtz	a2, $cpystrt
	j	cstrerror
$cpystrt:
	/*
	 * start up first word
	 * adjust pointers so that a0 points to next word
	 * t7 = a1 adjusted by same amount minus one
	 * t0,t1,t2,t3 are filled with 4 consecutive bytes
	 * t4 is filled with the same 4 bytes in a single word
	 */
	.set noreorder
	ble	a2,4,$dumbcpy	# not enough for a word
	move	v0,a2		# BDSLOT save copy of maxlength
	lb	t0,0(a0)
	nop			# LDSLOT
	beq	t0,zero,$cpy1ch
	or	t5,a1,3		# LDSLOT get an early start
	lb	t1,1(a0)
	subu	t6,t5,a1	# LDSLOT number of char in 1st word of dst - 1
	beq	t1,zero,$cpy2ch
	addu	t7,a0,t6	# BDSLOT offset starting pt for source string
	lb	t2,2(a0)
	nop			# LDSLOT
	beq	t2,zero,$cpy3ch
	LWS	t4,0(a0)	# BDSLOT safe: always in same word as 0(a0)
	lb	t3,3(a0)	# LDSLOT
	LWB	t4,3(a0)	# LDSLOT fill out word
	beq	t3,zero,$cpy4ch	# LDSLOT
	addu	t6,1		# BDSLOT chars stored by SWS
	blt	a2,t6,$cpy4ch	# out of space
	addu	a0,t6		# adjust source pointer
	SWS	t4,0(a1)	# store entire or part word
	subu	a1,t5,3		# adjust destination ptr
	subu	a2,t6		# decr maxlength

	/*
	 * inner loop
	 * at this point the destination is word aligned and t7
	 * points 1 byte before the corresponding source location
	 */
1:	ble	a2,4,$dumbcpy
	addu	a1,4		# BDSLOT
	lb	t0,1(t7)
	addu	t7,4		# LDSLOT
	beq	t0,zero,$cpy1ch
	nop			# BDSLOT
	lb	t1,1+1-4(t7)
	nop			# LDSLOT
	beq	t1,zero,$cpy2ch
	nop			# BDSLOT
	lb	t2,2+1-4(t7)
	addu	a0,4		# LDSLOT adjust source pointer
	beq	t2,zero,$cpy3ch
	LWS	t4,0+1-4(t7)	# BDSLOT
	subu	a2,4		# LDSLOT
	bltz	a2,$nsp4ch	# no room for 4
	lb	t3,3+1-4(t7)	# BDSLOT
	LWB	t4,3+1-4(t7)	# LDSLOT
	bne	t3,zero,1b	# LDSLOT
	sw	t4,0(a1)	# BDSLOT
	b	$cpyok
	nop			# BDSLOT

$cpy4ch:
	/*
	 * 4 bytes left to store
	 */
	subu	a2,4
	bgez	a2,$do4ch	# room left
	nop			# BDSLOT
	b	$nsp3ch		# try 3 characters
	addu	a2,1		# BDSLOT

$do4ch:
	SWS	t4,0(a1)
	b	$cpyok
	SWB	t4,3(a1)	# BDSLOT

$cpy3ch:	
	/*
	 * 3 bytes left to store
	 */
	subu	a2,3
	bgez	a2,$do3ch	# room left
	nop			# BDSLOT
	b	$nsp2ch		# no space for 3, see if 2 will fit
	addu	a2,1		# BDSLOT

$cpy2ch:
	/*
	 * 2 bytes left to store
	 */
	subu	a2,2
	bgez	a2,$do2ch	# room left
	nop			# BDSLOT
	b	$nsp1ch		# no space for 2, see if 1 will fit
	addu	a2,1		# BDSLOT

$cpy1ch:
	/*
	 * 1 last byte to store
	 */
	subu	a2,1
	bgez	a2,$do1ch	# room left
	nop			# BDSLOT
	b	$nospace	# no space at all
	addu	a2,1		# BDSLOT

$do3ch: sb	t2,2(a1)
$do2ch:	sb	t1,1(a1)
$do1ch:	sb	t0,0(a1)

$cpyok:
	/*
	 * copy complete, calculate length copied if necessary and
	 * return
	 */
	subu	a2,v0,a2		# bytes copied = maxlength - rem
	move	v0,zero			# success return code

$cpyexit:
	.set	reorder
	beq	a3,zero,2f		# &lencopied == 0 ?
	sw	a2,0(a3)		# no, return lencopied
2:	j	ra
	.set	noreorder

/*
 * not enough room to move one word, do stupid byte copy
 */
$dumbcpy:
	beq	a2,zero,$nospace	# no room
	nop				# BDSLOT
	lbu	t0,0(a0)
	subu	a2,1			# LDSLOT decr count
	addu	a0,1			# bump source ptr
	sb	t0,0(a1)
	bne	t0,zero,$dumbcpy	# not null terminator
	addu	a1,1			# BDSLOT bump dest ptr
	b	$cpyok
	nop				# BDSLOT

/*
 * ran out of space, copy as many characters as possible
 */
$nsp4ch:
	addu	a2,1
$nsp3ch:
	bgez	a2,$ndo3ch		# room for 3
	addu	a2,1			# BDSLOT
$nsp2ch:
	bgez	a2,$ndo2ch		# room for 2
	addu	a2,1			# BDSLOT
$nsp1ch:
	bgez	a2,$ndo1ch		# room for 1
	nop				# BDSLOT
	b	$nospace
	nop				# BDSLOT

$ndo3ch:sb	t2,2(a1)
$ndo2ch:sb	t1,1(a1)
$ndo1ch:sb	t0,0(a1)
$nospace:
	/*
	 * Ran out of space, length copied is always maxlength
	 */
	move	a2,v0			# copied max length
	b	$cpyexit
	li	v0,ENOENT		# BDSLOT string too big
	.set	reorder
	END(cpystr)

