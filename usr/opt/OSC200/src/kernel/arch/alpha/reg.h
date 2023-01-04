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
 * 07-sep-90	rjl
 *
 *	Register offsets into exception frame, given a pointer to the
 * 	exception frame (long *).  The first 8 registers are saved by
 *	the palcode exception handler, the rest is saved by Ultrix
 */

/*
 * Location of the users' stored
 * registers in exception frame.
 * Usage is u.u_ar0[XX] or USER_REG(x).
 *
 * Space to home arguments is now in the callee instead of the
 * caller.
 */


#ifndef _REG_H_
#define _REG_H_
/*
 * Software portion of the exception frame
 */
#define	EF_V0		0		/* r0: return address	*/
#define	EF_T0		1		/* r1: caller saved 0	*/
#define EF_T1		2		/* r2: caller saved 1	*/
#define EF_T2		3		/* r3: caller saved 2	*/
#define EF_T3		4		/* r4: caller saved 3	*/
#define EF_T4		5		/* r5: caller saved 4	*/
#define EF_T5		6		/* r6: caller saved 5	*/
#define EF_T6		7		/* r7: caller saved 6	*/
#define	EF_T7		8		/* r8: caller saved 7	*/
#define	EF_S0		9		/* r9: callee saved 0	*/
#define	EF_S1		10		/* r10: callee saved 1	*/
#define	EF_S2		11		/* r11: callee saved 2	*/
#define	EF_S3		12		/* r12: callee saved 3	*/
#define	EF_S4		13		/* r13: callee saved 4	*/
#define	EF_S5		14		/* r14: callee saved 5	*/
#define EF_S6		15		/* r15:	callee saved 6	*/
#define	EF_A3		16		/* r19: argument 3	*/
#define	EF_A4		17		/* r20: argument 4	*/
#define	EF_A5		18		/* r21: argument 5	*/
#define	EF_T8		19		/* r22: caller saved 8	*/
#define	EF_T9		20		/* r23: caller saved 9	*/
#define	EF_T10		21		/* r24: caller saved 10	*/
#define	EF_T11		22		/* r25: caller saved 11	*/
#define	EF_RA		23		/* r26: return address	*/
#define	EF_T12		24		/* r27: caller saved 12	*/
#define	EF_AT		25		/* r28: assembler temp	*/
#define	EF_SP		26		/* r30: stack pointer	*/

/*
 * Hardware portion of the exception frame
 */
#define EF_PS		27		/* processor status 	*/
#define EF_PC		28		/* program counter	*/
#define	EF_GP		29		/* r29: global pointer	*/
#define	EF_A0		30		/* r16: argument 0	*/
#define	EF_A1		31		/* r17: argument 1	*/
#define	EF_A2		32		/* r18: argument 2	*/

#define EF_SIZE		(33*8)		/* size of exception frame */
#define HWEF_SIZE	(6*8)		/* size of hardware frame */

/*
 * EF_SSIZE is the size of the software portion of the exception frame
 */
#define EF_SSIZE	(EF_SIZE-HWEF_SIZE) /* size of software saved part */

/*
 * Macro to find user registers in core files
 * for use by debuggers, etc.
 * Note the USER_REG macro is located in mach/mips/thread_status.h
 */
#define	CORE_REG(reg, ubase)	\
	(((unsigned *)((unsigned)(ubase)+UPAGES*NBPG-EF_SIZE))[reg])
/*
 * jmp_buf offsets
 * WARNING:
 * if this changes, label_t in types.h must change
 */
#define	JB_S0		0	/* callee saved regs.... */
#define	JB_S1		1
#define	JB_S2		2
#define	JB_S3		3
#define	JB_S4		4
#define	JB_S5		5
#define	JB_S6		6
#define	JB_SP		7	/* stack pointer */
#define	JB_PC		8	/* program counter */
#define	JB_PS		9	/* Not really status register, used for IPL */
#define	NJBREGS		10

#endif
