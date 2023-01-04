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
 *	@(#)$RCSfile: locopy_ns32k.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:41:17 $
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
 * Mach Operating System
 * Copyright (c) 1989 Encore Computer Corporation
 */
#include "assym.s"

#include "mmax_xpc.h"
#include "mmax_apc.h"
#include "mmax_dpc.h"

#include "sys/errno.h"
#include "mmax/mlmacros.h"
#include "mmax/cpudefs.h"

/*
 * locopy_ns32k.s:  byte copying/clearing routines for the ns32k family.
 */

/*
 * Copy a null terminated string from the user address space into
 * the kernel address space.
 *
 * copyinstr(fromaddr, toaddr, maxlength, &lencopied)
 */
ENTRY(copyinstr)
	enter	[r3,r4,r5], $0
	movd	r0,r2			/* r2 = user addr */
					/* r1 = kernel addr */
	movqd	$0,r4			/* r4 = match char (null) */
	movd	8(fp),r5		/* r5 = remaining count */

	/*
	 *	Fault in quad loop causes attempt to recover by moving
	 *	individual bytes.
	 */
	GETCPUID(r3)
	movd	_active_threads[r3:d],r3	/* This CPU's thread */
	addr	cinstr_recover,THREAD_RECOVER(r3)

	/*
	 * Set up to do the quad loop
	 */
	cmpqd	$0,r5			/* Any chars to copy in ? */
	bge	cinstrdone		/* No - we are done */

cinstrqloop:
	cmpqd	$7,r5			/* space for another quad ? */
	bge	cinstrbstart		/* branch if not */
	movusd	0(r2),0(r1)		/* get 4 bytes */
	movusd	4(r2),4(r1)		/* get 4 more bytes */
	addr	8,r0			/* 8 bytes to scan */
	addd	r0,r2			/* bump r2 because scan won't */
	skpsb	[u]			/* scan for null and bump r1 */
	bfs	cinstrqdone		/* br if null found */
	acbd	$-8,r5,cinstrqloop	/* update count and continue */
	br	cinstrfail		/* out of space */

cinstrqdone:
	/*
	 *	skpsb subtracted the bytes before the null from 8 in r0.
	 *	We need to account for the null it found.  Hence bytes
	 *	moved = (8 - r0) + 1 = (9 - r0) and this needs to be
	 *	subtracted from the count (in r5).
	 */
	subd	$9,r5
	addd	r0,r5
	br	cinstrdone

	/*
	 *	Move rest of string 1 byte at a time.
	 */
cinstr_recover:
cinstrbstart:
	addr	cinstrfault, THREAD_RECOVER(r3)	/* faults here are fatal */

cinstrbloop:
	movusb	0(r2),0(r1)		/* Perform the actual move */
	cmpqb	$0,0(r1)		/* End of string? */
	beq	cinstrbdone
	addqd	$1,r2			/* useraddr++ */
	addqd	$1,r1			/* kerneladdr++ */
	acbd	$-1,r5,cinstrbloop	/* keep going */
	br	cinstrfail		/* out of space */

cinstrbdone:
	addqd	$-1,r5			/* account for null */

	/*
	 * We are done: return the number of bytes copied
	 */
cinstrdone:
	movd	12(fp),r2		/* &lenstr copied */
	cmpqd	$0,r2			/* return nothing if null */
	beq	cinstrnull
	movd	8(fp),r1		/* original count */
	subd	r5,r1			/* subract count remaining */
	movd	r1,0(r2)		/* No. bytes copied */

cinstrnull:
	movd	r4,r0			/* Set return code */
	movqd	$0,THREAD_RECOVER(r3)
	exit	[r3,r4,r5]
	ret	$0

	/*
	 *	On error, put code in r4.  It's already 0 (success code),
	 *	so nothing need be done in success case.
	 */

cinstrfail:
	addr	ENOENT,r4
	br	cinstrdone

cinstrfault:
	addr	EFAULT,r4
	br	cinstrdone
	
/*
 * Copy a null terminated string from the kernel
 * address space to the user address space.
 *
 * copyoutstr(fromaddr, toaddr, maxlength, &lencopied)
 */
ENTRY(copyoutstr)
	enter	[r3,r4,r5], $0
	movd	r1,r2			/* r2 = user addr */
	movd	r0,r1			/* r1 = kernel addr */
	movqd	$0,r4			/* r4 = match char (null) */
	movd	8(fp),r5		/* r5 = remaining count */

	/*
	 *	Fault in quad loop causes attempt to recover by moving
	 *	individual bytes.
	 */
	GETCPUID(r3)
	movd	_active_threads[r3:d],r3	/* This CPU's thread */
	addr	coutstr_recover,THREAD_RECOVER(r3)

	/*
	 * Set up to do the quad loop
	 */
	cmpqd	$0,r5			/* Any chars to copy out ? */
	bge	coutstrdone		/* No - we are done */

coutstrqloop:
	cmpqd	$7,r5			/* space for another quad ? */
	bge	coutstrbstart		/* branch if not */
	movsud	0(r1),0(r2)		/* put 4 bytes */
	movsud	4(r1),4(r2)		/* put 4 more bytes */
	addr	8,r0			/* 8 bytes to scan */
	addd	r0,r2			/* bump r2 because scan won't */
	skpsb	[u]			/* scan for null and bump r1 */
	bfs	coutstrqdone		/* br if null found */
	acbd	$-8,r5,coutstrqloop	/* update count and continue */
	br	coutstrfail		/* out of space */

coutstrqdone:
	/*
	 *	skpsb subtracted the bytes before the null from 8 in r0.
	 *	We need to account for the null it found.  Hence bytes
	 *	moved = (8 - r0) + 1 = (9 - r0) and this needs to be
	 *	subtracted from the count (in r5).
	 */
	subd	$9,r5
	addd	r0,r5
	br	coutstrdone

	/*
	 *	Move rest of string 1 byte at a time.
	 */
coutstr_recover:
coutstrbstart:
	addr	coutstrfault, THREAD_RECOVER(r3)  /* faults here are fatal */

coutstrbloop:
	movsub	0(r1),0(r2)		/* Perform the actual move */
	cmpqb	$0,0(r1)		/* End of string? */
	beq	coutstrbdone
	addqd	$1,r2			/* useraddr++ */
	addqd	$1,r1			/* kerneladdr++ */
	acbd	$-1,r5,coutstrbloop	/* keep going */
	br	coutstrfail		/* out of space */

coutstrbdone:
	addqd	$-1,r5			/* account for null */

	/*
	 * We are done: return the number of bytes copied
	 */
coutstrdone:
	movd	12(fp),r2		/* &lenstr copied */
	cmpqd	$0,r2			/* return nothing if null */
	beq	coutstrnull
	movd	8(fp),r1		/* original count */
	subd	r5,r1			/* subract count remaining */
	movd	r1,0(r2)		/* No. bytes copied */

coutstrnull:
	movd	r4,r0			/* Set return code */
	movqd	$0,THREAD_RECOVER(r3)
	exit	[r3,r4,r5]
	ret	$0

	/*
	 *	On error, put code in r4.  It's already 0 (success code),
	 *	so nothing need be done in success case.
	 */

coutstrfail:
	addr	ENOENT,r4
	br	coutstrdone

coutstrfault:
	addr	EFAULT,r4
	br	coutstrdone
	
/*
 * Copy a null terminated string from one point to another in
 * the kernel address space.
 *
 * copystr(fromaddr, toaddr, maxlength, &lencopied)
 */
ENTRY(copystr)
	movd	r4,tos			/* save scratch reg */
	movd	r1,r2			/* r2 = toaddr */
	movd	r0,r1			/* r1 = fromaddr */
	movd	8(sp),r0		/* r0 = count */
	movqd	$0,r4			/* r4 = null to match */

	cmpqd	$0,r0			/* Anything to copy ? */
	bgt	cstrdone		/* No - we are done */

	movsb	[u]			/* move the string */

	cmpqd	$0,r0			/* room for null terminator ? */
	beq	cstrfail		/* no - ran out of space */
	movqb	$0,0(r2)		/* terminate string */
	addqd	$-1,r0			/* account for terminator */
	br	cstrdone

	/*
	 *	On failure, set r4 to error code.  0 (its old value) is
	 *	success, so nothing needs to be done in that case.
	 */
cstrfail:
	addr	ENOENT, r4

	/*
	 *	Return number of bytes copied if requested.
	 */
cstrdone:
	movd	12(sp),r2		/* &lenstr copied */
	cmpqd	$0,r2			/* return nothing if null */
	beq	cstrnull
	movd	8(sp),r1		/* original count */
	subd	r0,r1			/* subract count remaining */
	movd	r1,0(r2)		/* No. bytes copied */
cstrnull:
	movd	r4,r0			/* set return code */
	movd	tos,r4
	ret	$0

/*
.* copyin - copy a certain number of bytes from user space to kernel space.
 *
int simple_copyin (uaddr, kerneladdr, count)
caddr_t uaddr, kerneladdr;
int count;
 */

ENTRY(simple_copyin)
	save	[r3,r7]			/* save temp regs */
	movd	12(sp),r7		/* Get count */

	GETCPUID(r3)
	movd	_active_threads[r3:d],r3	/* This CPU's thread */
	addr	copy_fail,THREAD_RECOVER(r3)

	/*
	 * Set up to do the movusd loop
	 */

	cmpqd	$7,r7			/* Do we do the 1st 'movusd'? */
	bgt	cinbstart		/* No - go to 'movusb' code */

	movd	r1,r2			/* get the "to" address */
	andd	$3,r2			/* check alignment */
	cmpqd	$0,r2
	beq	cindstart		/* already aligned -- great! */
	addqd	$-4,r2			/* -(bytes to move to align) */
	addd	r2,r7			/* adjust count */

cinaloop:
	movusb	r0,r1			/* move a byte */
	addqd	$1,r0			/* uaddr++ */
	addqd	$1,r1			/* kerneladdr++ */
	acbd	$1,r2,cinaloop		/* continue until aligned */

	/*
	 * Loop doing as may movusd's as possible
	 */

cindstart:
	movd	r7,r2			/* get the remaining count */
	andd	$3,r7			/* mask to leave leftover bytes */
	ashd	$-2,r2			/* convert to d-words */
cindloop:
	movusd	r0,r1			/* Perform the actual move */
	addqd	$4,r0			/* useraddr++ */
	addqd	$4,r1			/* kerneladdr++ */
	acbd	$-1,r2,cindloop		/* continue until done movusd's */

	/*
	 * Set up to do the movusb loop
	 */

cinbstart:
	cmpqd	$0,r7			/* Do we do the 1st 'movusb'? */
	beq	cindone			/* No - we are done */

	/*
	 * Loop doing as may movusb's as possible
	 */

cinbloop:
	movusb	r0,r1			/* Perform the actual move */
	addqd	$1,r0			/* useraddr++ */
	addqd	$1,r1			/* kerneladdr++ */
	acbd	$-1,r7,cinbloop		/* count-- */

cindone:
	movqd	$0,r0
	movqd	$0,THREAD_RECOVER(r3)
	restore	[r3,r7]
	ret	$0			/* Return to caller */

/*
.* copyout - copy a number of bytes from kernel space to user space.
 *
int simple_copyout (kerneladdr, uaddr, count)
caddr_t kerneladdr, uaddr;
int count;
/*/

ENTRY(simple_copyout)
	save	[r3,r7]			/* save registers */
	movd	12(sp),r7		/* Get count */

	GETCPUID(r3)
	movd	_active_threads[r3:d],r3	/* This CPU's thread */
	addr	copy_fail,THREAD_RECOVER(r3)

	/*
	 * Set up to do the movsud loop
	 */

	cmpqd	$7,r7			/* Do we perform the 1st 'movsud' */
	bgt	coutbstart		/* No - go to 'movusb' code */
	
	movd	r1,r2			/* get the "to" address */
	andd	$3,r2			/* check alignment */
	cmpqd	$0,r2
	beq	coutdstart		/* already aligned -- great! */
	addqd	$-4,r2			/* -(bytes to move to align) */
	addd	r2,r7			/* adjust count */

coutaloop:
	movsub	r0,r1			/* move a byte */
	addqd	$1,r0			/* uaddr++ */
	addqd	$1,r1			/* kernel_addr ++ */
	acbd	$1,r2,coutaloop		/* continue until aligned */

	/*
	 * Loop doing as may movusd's as possible
	 */

coutdstart:
	movd	r7,r2			/* get the remaining count */
	andd	$3,r7			/* mask to leave leftover bytes */
	ashd	$-2,r2			/* convert to d-words */
coutdloop:
	movsud	r0,r1			/* Perform the actual move */
	addqd	$4,r0			/* kerneladdr++ */
	addqd	$4,r1			/* uaddr++ */
	acbd	$-1,r2,coutdloop	/* continue until d-moves done */

	/*
	 * Set up to do the movusb loop
	 */

coutbstart:
	cmpqd	$0,r7			/* Do we perform the 1st 'movusb'? */
	beq	coutdone		/* No - we are done */
					/* from prior 'movusd' failure) */

	/*
	 * Loop doing as many movusb's as possible
	 */

coutbloop:
	movsub	r0,r1			/* Perform the actual move */
	addqd	$1,r0			/* kerneladdr++ */
	addqd	$1,r1			/* uaddr++ */
	acbd	$-1,r7,coutbloop	/* continue until done */

coutdone:
	movqd	$0,r0
	movqd	$0,THREAD_RECOVER(r3)
	restore [r3,r7]
	ret	$0			/* Return to caller */

/* Stub which is jumped to if a fault occurs in the copyin/copyout routines */

copy_fail:				# jump to here if access fails
	addr	EFAULT,r0
	movqd	$0,THREAD_RECOVER(r3)
	restore	[r3,r7]
	ret	$0

/* ovbcopy (*s1, *s2, n) -
 *	copy from s1 to s2 for n characters, possible overlapping.

 *	This determines if the address of s1 is less than the address of s2,
 *	if it is then it copies in the forward direction, if the address of s2
 *	is less than the address of s1 then it copies in the backward
 *	direction.  The reason that this matters is the possibility of the
 *	source and the destination overlapping.
 */

ENTRY(ovbcopy)
	movd	r1,r2	#Address of string 2 in r2
	movd	r0,r1	#Address of string 1 in r1
	movd	4(sp),r0	#Count in r0
	cmpqd	$0,r0		#Test for 0 count
	beq	ovret
	cmpd	r1,r2		#Determine direction of the copy.
	blo	ovback		#	- Backward direction.
	beq	ovret		#	- dest = src
				#	- Forward direction
				#Copy in the forward direction...
	movsb
ovret:
	ret	$0

ovback:				#Copy in the backward direction...
	addd	r0,r1
	addqd	$-1,r1
	addd	r0,r2
	addqd	$-1,r2
	movsb	[b]
	ret	$0

/* 
 * void bcopy (from, to, bytes)
 * 	caddr_t from, to;
 * 	int bytes;
 * 
 *	copy from s1 to s2 for n characters, possible overlapping.
 *
.* ASSUMPTIONS:
 *	Unpredictable results will occur if the 'from' address range
 *	and the 'to' addresss range overlap. Both the 'from' address range
 *	and the 'to' address range are valid. Although the routine
 *	will work no matter how the from and to addresses are aligned, the
 *	routine will be at its best if the addresses are aligned on word
 *	boundaries.
/*/

	.globl	_blkcpy
_blkcpy:
ENTRY(bcopy)
	movd	r7,tos			/* Save the scratch register */
	movd	r1,r2			/* Get the to address */
	movd	r0,r1			/* Get the from address */
	movd 	8(sp),r7		/* Get the length */

	/*
	 * If less than 12 bytes are to be moved, then don't even try to
	 * optimize using a movsd instruction.
	 */

	cmpd	$12,r7			/* Is the length < 12 bytes? */
	bgt	bcopsngl		/* Yes - only do a 'movsb' */

	movd	r2,r0			/* align to "to" address */
	andd	$3,r0			/* if "from" aligns, big win,
					else aligning "to" is more efficient
					because writes are aligned. */
	cmpqd	$0,r0			/* if already aligned, great */
	beq	bcopch
	addqd	$-4,r0			/* -(bytes to move to align) */
	addd	r0,r7			/* decrement remaining count */
	absd	r0,r0			/* make it positive for movsb */
	movsb				/* first move to align */

	/*
	 *	Move as many 64-byte chunks as possible.  Unrolled
	 *	movd's are faster than movsd.
	 */
bcopch:
	cmpd	$64,r7			/* any chunks ?? */
	bge	bcopdbl			/* branch if not */
	movd	r7,r0
	ashd	$-6,r0			/* number of chunks */
	andd	$0x3f,r7		/* remove from remaining length */

bcopchloop:
	movd	0(r1),0(r2)
	movd	4(r1),4(r2)
	movd	8(r1),8(r2)
	movd	12(r1),12(r2)
	movd	16(r1),16(r2)
	movd	20(r1),20(r2)
	movd	24(r1),24(r2)
	movd	28(r1),28(r2)
	movd	32(r1),32(r2)
	movd	36(r1),36(r2)
	movd	40(r1),40(r2)
	movd	44(r1),44(r2)
	movd	48(r1),48(r2)
	movd	52(r1),52(r2)
	movd	56(r1),56(r2)
	movd	60(r1),60(r2)
	addr	64(r1),r1		# update this address
	addr	64(r2),r2		# and this one
	acbd	$-1,r0,bcopchloop	# continue until chunks done.

	/*
	 * Do an aligned movsd instruction for the maximum number of words
	 */

bcopdbl:
	cmpqd	$4,r7			/* any doubles left ? */
	bgt	bcopsngl
	movd	r7,r0			/* copy remaining length */
	andd	$3,r7			/* mask off bytes for movsb */
	ashd	$-2,r0			/* convert to d-words */
	movsd				/* Perform the actual copy */

	/*
	 * Do a movsb instruction for any remaining bytes to be copied.
	 */

bcopsngl:
	cmpqd	$0,r7			/* Anything left ?? */
	beq	bcopdone		/* if not, done */
	movd	r7,r0			/* Set # bytes to be copied */
	movsb				/* Perform the actual copy */

	/*
	 * Return to the caller
	 */

bcopdone:
	movd	tos,r7			/* Restore scratch register */
	ret	$0			/* Return to caller */

/*2
.* kclear:
 *	This routine is used to clear an area of kernel space to be
 *	all zeroes.
 *
.* ARGUMENTS:
 *
.* kerneladdr: address in kernel (supervisor) space where clearing
 *	is to start
 *
.* bytes: number or bytes to clear
 *
.* USAGE:
 *	This routine is synonymous with bzero and bzeroba.
 *
.* ASSUMPTIONS:
 *	The range to be cleared is valid. Although the routine
 *	will work no matter how the address is aligned, the
 *	routine will be at its best if the address is aligned on 
 *	a word boundary.

kclear (kerneladdr, bytes)
	caddr_t kerneladdr;
	int bytes;

bzero (kerneladdr, bytes)
	caddr_t kerneladdr;
	int bytes;

bzeroba (kerneladdr, bytes)
	caddr_t kerneladdr;
	int bytes;
/*/

	.globl	_kclear
_kclear:
	.globl	_blkclr
_blkclr:
	.globl	_bzeroba
_bzeroba:
ENTRY(bzero)
					# length to clear in r1
					# start addr in r0
	cmpd	$16,r1			# Clear less than 16 bytes?
	bgt	bzerbstart		# Clear bytes
	movqd	$3,r2			# Mask for 4-byte alignment.
	andd	r0,r2			# mask off all but bottom bits
	cmpqb	$0,r2			# Are we double word aligned?
	beq	bzerchstart		# yes, clear 64 byte chunks
	addqd	$-4,r2			# -(number of bytes to clear)
	addd	r2,r1			# no, adjust count and clear 1-3 bytes

bzeraloop:
	movqb	$0,0(r0)		# clear byte
	addqd	$1,r0			# addr++
	acbd	$1,r2,bzeraloop		# increment count and loop

bzerchstart:
	cmpd	$64,r1			# any chunks
	bgt	bzerqstart		# nope, try quads.
	movd	r1,r2			# number of bytes
	ashd	$-6,r2			# number of 64 byte chunks
	andd	$0x3f,r1		# remove chunks from remaining count

bzerchloop:
	movqd	$0,0(r0)		# clear 4 bytes
	movqd	$0,4(r0)		# clear 4 bytes
	movqd	$0,8(r0)		# clear 4 bytes
	movqd	$0,12(r0)		# clear 4 bytes
	movqd	$0,16(r0)		# clear 4 bytes
	movqd	$0,20(r0)		# clear 4 bytes
	movqd	$0,24(r0)		# clear 4 bytes
	movqd	$0,28(r0)		# clear 4 bytes
	movqd	$0,32(r0)		# clear 4 bytes
	movqd	$0,36(r0)		# clear 4 bytes
	movqd	$0,40(r0)		# clear 4 bytes
	movqd	$0,44(r0)		# clear 4 bytes
	movqd	$0,48(r0)		# clear 4 bytes
	movqd	$0,52(r0)		# clear 4 bytes
	movqd	$0,56(r0)		# clear 4 bytes
	movqd	$0,60(r0)		# clear 4 bytes
	addr	64(r0),r0		# update address
	acbd	$-1,r2,bzerchloop	# again and again

bzerqstart:
	cmpd	$8,r1			# any quads left ?
	bgt	bzerbstart		# branch if not
	movd	r1,r2
	ashd	$-3,r2			# convert to doubles
	andd	$7,r1			# remove doubles from remaining count

bzerqloop:
	movqd	$0,0(r0)		# clear 4 bytes
	movqd	$0,4(r0)		# clear 4 bytes
	addr	8(r0),r0		# bump address
	acbd	$-1,r2,bzerqloop

bzerbstart:
	cmpqd	$0,r1			# any more bytes?
	beq	bzerdone		# nope

bzerbloop:
	movqb	$0,0(r0)		# clear byte
	addqd	$1,r0			# addr++
	acbd	$-1,r1,bzerbloop	# decrement count and loop

bzerdone:
	ret	$0


/*
 *	Random string manipulation routines.
 */

/*
 * strcmp(s1,s2):  Compare strings:  s1>s2: >0  s1==s2: 0  s1<s2: <0
 */
ENTRY(strcmp)
	movd	r4,tos		# save temporary.
	movd	r1,r2		# r2 = s2
	movd	r0,r1		# r1 = s1
	movd	$0x7fffffff,r0	# Ridiculously large number
	movqd	$0,r4		# Look for null terminator
	cmpsb	[u]		# compare the strings
	movzbd	0(r1),r0	# last elt of s1
	movzbd	0(r2),r1	# last elt of s2
	subd	r1,r0		# return (*s1 - *s2)
	movd	tos,r4		# restore temporary
	ret	$0

/*
 * char *strcpy(s1, s2):  Copy string s2 to s1.  s1 must be large enough.
 * return s1
 */
ENTRY(strcpy)
	enter	[r0,r4], $0	# save temporary, original s1
	movd	r0,r2		# r1 = s2 [from], r2 = s1 [to]
	movd	0x7fffffff,r0	# Ridiculously large number
	movqd	$0,r4		# Look for null terminator
	movsb	[u]		# move the strings
	movqb	$0,0(r2)	# null-terminate result
	exit	[r0,r4]		# restore temporary, original s1 (returned)
	ret	$0

/*
 * char *strncpy(s1, s2, n): Copy s2 to s1, truncating or null-padding to
 * 	always copy n bytes.  return s1
 */
ENTRY(strncpy)
	enter	[r0,r4]		# save temporary, original s1
	movd	r0,r2		# r1 = s2 [from], r2 = s1 [to]
	movd	8(fp),r0	# number of elements
	movqd	$0,r4		# Look for null terminator
	movsb	[u]		# move strings
	bfc	cpydone		# If no terminator, done.
	movd	r0,r1		# remaining bytes in s1
	movd	r2,r0		# address of what's left
	jsr	_bzero		# Clear it.
cpydone:
	exit	[r0,r4]		# restore temporary, original s1
	ret	$0

