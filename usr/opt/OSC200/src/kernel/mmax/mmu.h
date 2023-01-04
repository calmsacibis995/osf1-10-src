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
 *	@(#)$RCSfile: mmu.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:41:48 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *     Copyright (C) 1984 Hydra Computer Systems, Inc.
 *
 * ALL RIGHTS RESERVED. Licensed Material - Property of Hydra 
 * Computer Systems, Inc. This software is made available solely 
 * pursuant to the terms of a software license agreement which governs 
 * its use. Unauthorized duplication, distribution or sale are strictly 
 * prohibited.
 *
 * Include file description:
 *
 * Original Author: Tony Anzelmo	Created on: 12/19/84
 */

#include <mmax_xpc.h>
#include <mmax_apc.h>
#include <mmax_dpc.h>

#if	MMAX_XPC
/*
 *	NS32532 Memory management register definitions.
 *	The MMU lives on-chip.
 */

/*
 *	MCR:  Memory Management Control Register
 */

#define	MCR_TU		0x0001		/* Translate user addresses	    */
#define	MCR_TU_BIT	0
#define MCR_TS		0x0002		/* Translate system addresses	    */
#define	MCR_TS_BIT	1
#define	MCR_DS		0x0004		/* Dual address space		    */
#define	MCR_DS_BIT	2
#define	MCR_AO		0x0008		/* Address override		    */
#define	MCR_MAGIC	0x0000		/* Turns off translation	    */

/*
 *	MSR:  Memory Management Status Register
 */

#define MSR_TEX_MASK	0x00000003	/* Translation Exception	    */
#define MSR_PTE1FLT	0x00000001	/*  table fault (1st level)	    */
#define MSR_PTE2FLT	0x00000002	/*  page  fault (2nd level)	    */
#define	MSR_PROTFLT	0x00000003	/*  protection fault		    */
#define MSR_DDI_MASK	0x00000004	/* Data direction indicator	    */
#define	MSR_DDI_BIT		 2	/* DDT bit position		    */
#define MSR_USF_MASK	0x00000008	/* Mode (user/super) flag	    */
#define MSR_USF_BIT		 3	/* Mode flag bit position	    */
#define MSR_STT_MASK	0x000000f0	/* CPU status bits		    */
#define MSR_STT_SHFT		 4

/* CPU Status at Translation Exception */
#define MSR_ST_T_IDLE          0x0      /* Bus idle - nothing to do     */
#define MSR_ST_T_WAIT          0x1      /* Bus idle - wait instruction  */
#define MSR_ST_T_RESVD         0x2      /* Reserved for future use      */
#define MSR_ST_T_SLAVE         0x3      /* Waiting for slave processor  */
#define MSR_ST_T_IACKM         0x4      /* Int. acknowledge, master     */
#define MSR_ST_T_IACKC         0x5      /* Int. acknowledge, cascaded   */
#define MSR_ST_T_ENDIM         0x6      /* End of interrupt, master     */
#define MSR_ST_T_ENDIC         0x7      /* End of interrupt, cascaded   */
#define MSR_ST_T_IFSEQ         0x8      /* Sequential instruction fetch */
#define MSR_ST_T_IFNSQ         0x9      /* Non-sequential instr. fetch  */
#define MSR_ST_T_XFER          0xa      /* Data transfer                */
#define MSR_ST_T_RDRMW         0xb      /* Read RMW Operand             */
#define MSR_ST_T_RDADR         0xc      /* Read for address calculation */
#define MSR_ST_T_SLVOP         0xd      /* Transfer slave proc. operand */
#define MSR_ST_T_SLVST         0xe      /* Read slave processor status  */
#define MSR_ST_T_SLVID         0xf      /* Broadcast Slave ID           */

/*
 *	Ptrace arg value for the user's MMU MSR
 *	and AART registers.
 */

#define UMSR	-200
#define UAART	-201

/*
 *	DCR:  Debug Condition Register
 */

#define DCR_CBE0	0x00000001	/* Compare byte enable 0	    */
#define DCR_CBE1	0x00000002	/* Compare byte enable 1	    */
#define DCR_CBE2	0x00000004	/* Compare byte enable 2	    */
#define DCR_CBE3	0x00000008	/* Compare byte enable 3	    */
#define DCR_VNP		0x00000010	/* Compare virt (1) or phys (0) addr*/
#define DCR_CWR		0x00000020	/* Compare enable write references  */
#define DCR_CRD		0x00000040	/* Compare enable read references   */
#define DCR_CAE		0x00000080	/* Address compare enable	    */
#define DCR_TR		0x00080000	/* Enable Trap (DBG) on debug cond. */
#define DCR_PCE		0x00100000	/* PC-match enable		    */
#define DCR_UD		0x00200000	/* Enable debug cond. in User-Mode  */
#define DCR_SD		0x00400000	/* Enable debug cond. in Supr-Mode  */
#define DCR_DEN		0x00800000	/* Enable debug conditions	    */
#define DCR_BF		0x00000100	/* Bus interface unit FIFO disable  */
#define DCR_SI		0x00020000	/* Single-Instruction mode enable   */
#define DCR_BCP		0x00040000	/* Branch Condition Prediction dsbl */

/*
 *	DSR:  Debug Status Register
 */

#define DSR_RD		0x80000000	/* Last addr compare RD = 1, WR = 0 */
#define DSR_BPC		0x40000000	/* PC-match condition detected	    */
#define DSR_BEX		0x20000000	/* External condition detected	    */
#define DSR_BCA		0x10000000	/* Address-compare cond. detected   */

#endif	MMAX_XPC

#if	MMAX_APC

/*
 *	NS32382 Memory management register definitions
 */

/*
 * FEW: Feature Enable Word. 16 bit register.
 */

#define FEW_TU          0x0001          /* Translate user addresses     */
#define FEW_TU_BIT      0
#define FEW_TS          0x0002          /* Translate system addresses   */
#define FEW_TS_BIT      1
#define FEW_DS          0x0004          /* Dual address space           */
#define FEW_DS_BIT      2
#define FEW_AO          0x0008          /* Address override             */
#define FEW_BAE         0x00f0          /* Breakpoint address enable    */
#define FEW_BR          0x0010          /* Breakpoint read enable       */
#define FEW_BW          0x0020          /* Breakpoint write enable      */
#define FEW_BX          0x0040          /* Breakpoint execute enable    */
#define FEW_BS          0x0080          /* Breakpoint address select    */
#define FEW_MAGIC       0x0000          /* Turns off translation        */

/*
 * ASR: Abort Status Register. 19 Bits.
 */
#define ASR_TYPE_MASK   0x00000003      /* Translation exception field  */
#define ASR_PTE1FLT     0x00000001      /*  table fault (1st level)     */
#define ASR_PTE2FLT     0x00000002      /*  page  fault  (2nd level)    */
#define ASR_PROTFLT     0x00000003      /*  prot  fault                 */
#define ASR_AB_MASK     0x00000E00      /* Abort indicator mask         */
#define ASR_AB0         0x00000200      /* Abort indicator 0 - BPT      */
#define ASR_AB0_BIT	9		/* Abort indicator 0 - BPT      */
#define ASR_AB1         0x00000400      /* Abort indicator 1 - CPU      */
#define ASR_AB2         0x00000800      /* Abort indicator 2 - MMU      */

        /* The following bits are clocked on a bus error */
#define ASR_DDI_E       0x00001000      /* Data direction indicator     */
#define ASR_USF_E       0x00002000      /* Mode (user/super) flag       */
#define ASR_ST_EMASK    0x0003c000      /* CPU status bits              */
#define ASR_MC_E        0x00040000      /* CPU Multiple cycle bit       */

        /* The following bits are clocked on a translation exception */
#define ASR_DDI_T       0x00000004      /* Data direction indicator     */
#define ASR_DDI_T_BIT           02      /* Direction bit position       */
#define ASR_USR_MODE    0x00000008      /* Mode (user/super) flag       */
#define ASR_USR_MODE_BIT        03      /* User/super bit position      */
#define ASR_ST_TMASK    0x000000f0      /* CPU status bits              */
#define ASR_MC_T        0x00000100      /* CPU Multiple cycle bit       */

        /* CPU Status at Translation Exception */

#define ASR_ST_T_IDLE          0x0      /* Bus idle - nothing to do     */
#define ASR_ST_T_WAIT          0x1      /* Bus idle - wait instruction  */
#define ASR_ST_T_RESVD         0x2      /* Reserved for future use      */
#define ASR_ST_T_SLAVE         0x3      /* Waiting for slave processor  */
#define ASR_ST_T_IACKM         0x4      /* Int. acknowledge, master     */
#define ASR_ST_T_IACKC         0x5      /* Int. acknowledge, cascaded   */
#define ASR_ST_T_ENDIM         0x6      /* End of interrupt, master     */
#define ASR_ST_T_ENDIC         0x7      /* End of interrupt, cascaded   */
#define ASR_ST_T_IFSEQ         0x8      /* Sequential instruction fetch */
#define ASR_ST_T_IFNSQ         0x9      /* Non-sequential instr. fetch  */
#define ASR_ST_T_XFER          0xa      /* Data transfer                */
#define ASR_ST_T_RDRMW         0xb      /* Read RMW Operand             */
#define ASR_ST_T_RDADR         0xc      /* Read for address calculation */
#define ASR_ST_T_SLVOP         0xd      /* Transfer slave proc. operand */
#define ASR_ST_T_SLVST         0xe      /* Read slave processor status  */
#define ASR_ST_T_SLVID         0xf      /* Broadcast Slave ID           */

/*
 * ptrace arg value for the users'
 * mmu asr and aart registers.
 */

#define UASR    -200
#define UAART   -201

#endif	MMAX_APC

#if	MMAX_DPC

/*
 *	NS32082 Memory management register definitions
 */

/*
 * The EIA register - returns address of memory fault.  Also used to 
 * invalidate an address in the translation buffer.
 */

#define EIA_ADDR	0x00ffffff	/* address causing memory fault */
#define EIA_PTB1	0x80000000	/* translated by PTB1 (user space) */
#define EIA_PTB1_BIT	31		/* Bit number */
#define EIA_MASK	0x80ffffff	/* mask to clear reserved EIA bits */

#define EIA_VA		0x00ffffff	/* Virtual Address */
#define EIA_AS		0x80000000	/* Address-Space */

#define EIA_USER	0x80000000	/* For User Mode */
#define EIA_KERNEL	0x00000000	/* For kernel Mode */

/*
 * The MSR register - The MMU status register.  Gives reasons for an abort,
 * and is used to control the MMU.
 */

#define MSR_BE_BIT	02		/* Bpt bit in Error Class Field */
#define MSR_TU_BIT	16		/* Translate user bit number */
#define MSR_TS_BIT	17		/* Translate super bit number */
#define MSR_DS_BIT	18		/* Dual space */
#define MSR_NT_BIT	25		/* Non-sequential trace bit */
#define MSR_BEN_BIT	20		/* Breakpoint enable bit	*/

#define MSR_TRANSERR	0x00000001	/* error due to address translation */
#define MSR_MAGIC	0x00000002	/* magic bit to clear msr */
#define MSR_BRKERR	0x00000004	/* error due to breakpoint */
#define MSR_PROT_MASK	0x00000008	/* protection level error */
#define MSR_PROT_BIT	3
#define MSR_TABLE_MASK	0x00000010	/* invalid level-1 page table entry */
#define MSR_TABLE_BIT	4
#define MSR_PAGE_MASK	0x00000020	/* invalid level-2 page table entry */
#define MSR_PAGE_BIT	5
#define MSR_BPT1	0x00000040	/* breakpoint was BPR register 1 */
#define MSR_READERR	0x00000100	/* 1 = read error, 0 = write failure */
#define MSR_READBPT	0x00000200	/* 1 = read breakpoint, 0 = write */
#define MSR_TRANSSTAT	0x00001c00	/* bus cycle at translation error */
#define MSR_BPTSTAT	0x0000e000	/* bus cycle at breakpoint */
#define MSR_TRANSUSER	0x00010000	/* translate user addresses */
#define MSR_TRANSSUP	0x00020000	/* translate supervisor addresses */
#define MSR_USEPTB1	0x00040000	/* use PTB1 for user translations */
#define MSR_OVERRIDE	0x00080000	/* use supervisor prot for user */
#define MSR_BRKENABLE	0x00100000	/* breakpoints enabled */
#define MSR_BRKUSER	0x00200000	/* breakpoint only in user mode */
#define MSR_AI		0x00400000	/* abort or NMI trap for debug blk */
#define MSR_FLOWENABLE	0x00800000	/* flow tracing is enabled */
#define MSR_FLOWUSER	0x01000000	/* flow trace only in user mode */
#define MSR_SEQENABLE	0x02000000	/* nonsequential flow trap enabled */

/*
 * ptrace arg value for the users'
 * mmu msr and eia registers.
 */

#define UMSR	-200
#define UEIA	-201


#define MSR_PROT	3
#define MSR_TABLE	4
#define MSR_PAGE	5

/* Some more msr field definitions */

#define MSR_ERC_ATE	0x00000001	/* Address Translation Error */
#define MSR_ED		0x00000100	/* Error Data Direction */
#define MSR_TU		0x00010000	/* Translate User */
#define MSR_TS		0x00020000	/* Translate System */
#define MSR_DS		0x00040000	/* Dual Space */

 /* Value to init msr registers with.  */

#define INITMSR		(MSR_TU|MSR_TS|MSR_DS)

#endif	MMAX_DPC

