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
 *	@(#)$RCSfile: seg.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:19:53 $
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/* 
 * OSF/1 Release 1.0
 */

/*
 * Copyright 1988, 1989 by Intel Corporation
 */

/* access rights for data segments */ 
#define UDATA_ACC1	0xF2 	/* present dpl=3 writable */
#define KDATA_ACC1	0x92	/* present dpl=0 writable */
#define KSTACK_ACC1	0x96	/* present dpl=0 writable expand down */
#define DATA_ACC2	0xC	/* 1byte gran. 4Gb limit avl=0 */
#define ZERO_ACC2	0x0	/* 1byte gran. 64K limit avl=0 */
#define UTEXT_ACC1	0xFA 	/* present dpl=3 readable */
#define KTEXT_ACC1	0x9A 	/* present dpl=0 readable */
#define TEXT_ACC2	0xC	/* 1byte gran., 32 bit operands avl=0 */
#define LDT_UACC1	0xE2	/* present dpl=3 type=ldt */
#define LDT_KACC1	0x82	/* present dpl=0 type=ldt */
#define LDT_ACC2	0x0	/* G=0 avl=0*/
#define TSS3_KACC1	0x89 	/* present dpl=0 type=available 386 TSS */
#define	TSS2_KACC1	0x81 	/* present dpl=0 type=available 286 TSS */
#define TSS3_UACC1	0xE9 	/* present dpl=3 type=available 386 TSS */
#define	TSS2_UACC1	0xE1 	/* present dpl=3 type=available 286 TSS */
#define TSS_ACC2	0x0	/* g=0 avl=0 */

/* selector definitions */
#define LDTSEL		0x140	/* LDT for the current process */
#define UTSSSEL		0x148	/* TSS for the current process */
#define KTSSSEL 	0x150	/* TSS for the scheduler */
#define KCSSEL		0x158	/* kernel code segment selector */
#define KDSSEL		0x160	/* kernel data segment selector */
#define KSSSEL		0x20	/* kernel stack selector for limit checking */
#define DFTSSSEL	0x168	/* TSS for double fault handler */
#define JTSSSEL		0x170
#define MON1SEL		0x178	/* Selector to get to monitor int 1 handler */
#define MON3SEL		0x180	/* Selector to get to monitor int 3 handler */
#ifdef PS2
/* ABIOS selectors */
#define ABIOS_RET	0x18	/* 16 bit return selector for ABIOS (GDT) */
#endif

/* user selectors */
#define	USER_SCALL	0x07	/* call gate for system calls */
#define	USER_SIGCALL	0x0F	/* call gate for sigreturn */
#define	USER_CS		0x17	/* user's code segment */
#define	USER_DS		0x1F	/* user's data segment */
#define USER_ZERO	0x27	/* zero location 0 */
#define USER_FP		0x2f	/* selector for fp emulator to access state info */
#define USER_FPSTK	0x37	/* alias 32 bit stack selectore for emulator */

#define MONIDTSZ	16
#define KTBASE          0xC0008000
#define KDBASE          0xC0068000

/* access rights field for gates */

#define GATE_UACC	0xE0		/* present and dpl = 3 */
#define GATE_KACC	0x80		/* present and dpl = 0 */
#define GATE_386CALL	0xC		/* 386 call gate */
#define GATE_386INT	0xE		/* 386 int gate */
#define GATE_386TRP	0xF		/* 386 trap gate */
#define GATE_TSS	0x5		/* Task gate */

#define KVBASE          ((unsigned)0xC0000000L)     /* base address of the kernel    */

#ifdef PS2
#define GDTSZ		0x300	/* size of gdt table */
#else
#define GDTSZ		0x60	/* size of gdt table */
#endif /* PS2 */
#define IDTSZ		0x100	/* size of idt table */
#define LDTSZ		0x07	/* size of ldt table */

/*
 * fakedesc is a structure that contains the information to construct
 * a descriptor entry in either the gdt or idt tables. Since the base
 * address for the segment can't be resolved at compile time, it can't
 * be broken up into the appropriate descriptor bit fields. fakedesc
 * is munged into a real descriptor.
 *
 * f_base contains the 32 bit base field of the descriptor.
 * f_limacc contains 20 limit bits and 12 accbits in this order
 *
 * 0                      19  20  21      22   23   24  25,26 27-31
 * _____________________________________________________________________________
 * |                                                                           |
 * |   20 bit limit field   | G | B or D | 0 | AVL | P | DPL | Other Attributes|
 * |___________________________________________________________________________|
 * 
 */

#ifndef ASSEMBLER
struct fakedesc {
	unsigned long f_base;		/* segment base */
	unsigned long f_limacc;		/* 4 limit and access bytes */
};

/*
 * fakegate is similar to fakedesc. However it is for a gate descriptor.
 */

struct fakegate {
	unsigned long  f_off;		/* offset */
	unsigned short f_sel;		/* selector */
	unsigned char  f_wcount;	/* word count */
	unsigned char  f_type;		/* type of gate and access rights */
};

/*
 * desc is a real segment descriptor entry.
 */

struct desc {
	unsigned int	d_lim0015:16,
	 		d_base0015:16,
	  		d_base1623:8,
			d_acc0007:8,
			d_lim1619:4,
			d_acc0811:4,
			d_base2431:8;
};

/*
 * gate is a real gate descriptor.
 */

struct gate {
	unsigned int	g_off0015:16,
			g_select:16,
			g_wordc:8,
			g_acc:8,
			g_off1631:16;
};
#endif ASSEMBLER
			

/*
 * Mkdesc converts a segment base address, limit, access1, and access2
 * fileds into a fakedesc entry.
 */

#define mkdesc(base, limit, acc1, acc2) \
	{(unsigned long)base,((unsigned long)limit|(acc2<<20)|(acc1<<24))}

/* interrupt gate */
#define mkintg(a) \
	{(unsigned long)(a),(unsigned short)KCSSEL,(unsigned char)0,(unsigned char)(GATE_KACC|GATE_386INT)} 

/* trap gate from kernel */
#define mktrpg(a) \
	{(unsigned long)(a),(unsigned short)KCSSEL,(unsigned char)0,(unsigned char)(GATE_KACC|GATE_386TRP)} 

/* trap gate from user */
#define mkusrtrpg(a) \
 	{(unsigned long)(a),(unsigned short)KCSSEL,(unsigned char)0,(unsigned char)(GATE_UACC|GATE_386TRP)} 

/*
 * Supervisor mode text, data, tss descriptors from mkdesc
 */

#define	ktextdesc(x)  mkdesc((x),0xfffffL,KTEXT_ACC1,TEXT_ACC2) 
#define	kdatadesc(x)  mkdesc((x),0xfffffL,KDATA_ACC1,DATA_ACC2) 
#define	kldtdesc(x)  \
	        mkdesc((x),LDTSZ*sizeof (struct fakedesc),LDT_KACC1,LDT_ACC2) 
#define	ktssdesc(x)   mkdesc((x),sizeof(struct tss386),TSS3_KACC1,TSS_ACC2) 

#define nulldesc()    mkdesc(0,0,0,0)

/* 
 * Given a selector, convert it to an index into the appropriate
 * descriptor table.
 */

#define	seltoi(sel)		((unsigned short)(sel) >> 3)
