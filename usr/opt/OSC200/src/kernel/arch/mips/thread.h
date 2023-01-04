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
 *	@(#)$RCSfile: thread.h,v $ $Revision: 1.2.4.3 $ (DEC) $Date: 1992/03/31 17:16:10 $
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
 *	File:	mips/thread.h
 *
 *	This file defines machine specific, thread related structures,
 *	variables and macros.
 *
*/
#ifndef	_MIPS_THREAD_
#define	_MIPS_THREAD_

/*
 *	Unlike Vaxen et al. we don't really have special scratch
 *	registers that we can use, so we go the canonical way..
 */
#ifdef	ASSEMBLER
#else	/* ASSEMBLER */

#ifdef	KERNEL
#include <machine/pcb.h>
#include <mach/mips/vm_param.h>

/*
 * The following structure definition is intended to hold wired down
 * per-thread mips state.  The ordering of the structures within this
 * structure is VERY important since they are used in calculating where
 * each structure is located within the two pages allocated for the kernel
 * stack.  This scheme was done so that kernel stack overflow is 
 * detectable by trying to access unmapped guard page.

               VM_MIN_KERNEL_ADDRESS 
                      +-------------------------------+ ---\
                      |   struct uthread              |     \
                      +-------------------------------+      \
                      |   struct pcb                  |       \
                      +-------------------------------+ KERNEL_STACK_SIZE
                      |               |               |        /
                      |               |               |       /
                      |               |               |      /
                      |               V               |     /
                      |         Kernel Stack          |    /
 thread->kernel_stack +-------------------------------+ --/
                      |     Future Kstack expansion   |
                      +-------------------------------+
                      |       stack guard page        |
                      +-------------------------------+
               Start K2 space
 */



/* these are the offset of each of the structure elements from 
 * 	thread->kernel_stack
 */
#define UTHREAD_START_OFFSET	KERNEL_STACK_SIZE \
				- sizeof(struct uthread) 

#define	PCB_START_OFFSET	UTHREAD_START_OFFSET \
				- sizeof(struct pcb) 

#define KERNEL_STACK_START_OFFSET	PCB_START_OFFSET

/* 
 * This is the magic wired address to get at the pcb, and uthread;
 */
#define PCB_WIRED_ADDRESS  ((struct pcb *)\
					(VM_MIN_KERNEL_ADDRESS \
					 - sizeof(struct uthread) \
	 				 - sizeof(struct pcb)))

#define u (PCB_WIRED_ADDRESS)->u_address

#endif	/* KERNEL */
#endif	/* ASSEMBLER */
#endif	/* _MIPS_THREAD_ */
