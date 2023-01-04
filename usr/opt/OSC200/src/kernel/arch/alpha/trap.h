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
 * Modification History: /sys/vax/trap.h
 *
 * 15-Oct-90 --	rjl
 *	Initial ALPHA version
 */

#ifndef _TRAP_H_
#define _TRAP_H_

#define T_ARITH		0x10		/* Arithmentic			*/
#define T_MMANG		0x20		/* memory management faults	*/
#define T_ALIGN		0x30		/* unaligned access faults	*/

#define T_IFAULT	0x40		/* Instruction faults		*/
#define T_IFAULT_BPT	0		/* Break point trap		*/
#define T_IFAULT_BUGCK	1		/* Bug Check			*/
#define T_IFAULT_GENT	2		/* Gen trap			*/
#define T_IFAULT_FEN	3		/* Floating point enable	*/
#define T_IFAULT_OPDEC	4		/* Opcode reserved to DEC	*/


#define T_MMANG_TNV	0		/* Translation not valid	*/
#define T_MMANG_ACV	1		/* Access Violation		*/
#define T_MMANG_FOR	2		/* Fault on Read		*/
#define T_MMANG_FOE	3		/* Fault on Execute		*/
#define T_MMANG_FOW	4		/* Fault on Write		*/

#define	T_AST		0x50		/* Asynchronous System Trap	*/

#define USERFAULT	0x1		/* flag to note user/kernel 	*/

#endif
