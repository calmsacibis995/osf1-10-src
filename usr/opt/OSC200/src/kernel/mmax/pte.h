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
 *	@(#)$RCSfile: pte.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:42:42 $
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
 *        Copyright 1985 Encore Computer Corporation
 *
 * ALL RIGHTS RESERVED. Licensed Material - Property of Encore Computer
 * Corporation. This software is made available solely pursuant to the
 * terms of a software license agreement which governs its use. 
 * Unauthorized duplication, distribution or sale are strictly prohibited.
 *
 * Include file description:
 *	Contains definitions related to page table entries as defined
 *	by the NS32000 series hardware.
 *
 */
/*
 *	Three types of PTEs are defined:
 *
 *		1) The pte structure describes resident pages.
 *		2) The fpte structure describes non-resident pages.
 *		3) The cpte structure is used for copying PTE attribute
 *		   bits from one PTE in a cluster to the others.
 */
#ifndef	__MMAX_PTE_H
#define __MMAX_PTE_H

#ifdef	KERNEL
#include <mmax_xpc.h>
#include <mmax_apc.h>
#include <mmax_dpc.h>
#endif


#ifndef	LOCORE

#if	MMAX_XPC || MMAX_APC
struct pte {

/*
 *                                        ----------
 *                                        |Software|
 *   -----------------------------------------------------------------------
 *   |                  pfn               |  |  |  |mod|ref|ci| un| prot |v|
 *   -----------------------------------------------------------------------
 *                       20                 1  1  1  1   1  1    3    2   1
 */
unsigned int	pg_v	: 1,	/* Valid bit.				*/
		pg_prot	: 2,	/* Protection bits              	*/
		pg_un1	: 3,	/* Unused.				*/
		pg_ci	: 1,	/* Cache inhibit bit.			*/
		pg_r	: 1,	/* Page referenced bit.			*/
		pg_m    : 1,	/* Page modified.			*/
		pg_wired: 1,	/* Page is wired	(Software)	*/
        	pg_un2	: 2,	/* Unused       	(Software)	*/
		pg_pfnum: 20;	/* Physical page frame number.		*/
};


struct cpte {                           /* Special for clustering */
  unsigned int    pg_info:12,             /* Informational bits */
                  pg_pfnum:20;            /* Page frame number */
};

struct fpte {				/* Non-resident PTE */
unsigned int	pg_v:1,			/* Valid bit */
		pg_prot:2,		/* Protection bits */
		:7,			/* unused */
		pg_fs:1,		/* Page from file system */
		pg_share:1,		/* Shared page */
		pg_blkno:20;		/* File system block number */
};
#endif	/* MMAX_XPC || MMAX_APC */

#if	MMAX_DPC
struct pte {				/* Resident PTE */
unsigned int	pg_v:1,			/* Page is valid */
		pg_prot:2,		/* Protection */
		pg_r:1,			/* Page has been referenced */
		pg_m:1,			/* Page has been modified */
		:2,			/* Reserved */
		pg_wired:1,		/* Page wired */
		:1,			/* Not used */
		pg_pfnum:23;		/* Page frame number */
};

struct cpte {				/* Special for clustering */
unsigned int	pg_info:9,		/* Informational bits */
		pg_pfnum:23;		/* Page frame number */
};

struct fpte {				/* Non-resident PTE */
unsigned int	pg_v:1,			/* Valid bit */
		pg_prot:2,		/* Protection bits */
		:4,			/* unused */
		pg_fs:1,		/* Page from file system */
		pg_share:1,		/* Shared page */
		pg_blkno:23;		/* File system block number */
};
#endif	/* MMAX_DPC */

/*
 * Typedefs
 */
typedef	struct pte	PTE;
typedef struct fpte	FPTE;
typedef struct cpte	CPTE;
#endif	/* LOCORE */

/*
 *	The Multimax has hardware that cleanly extends the physical address
 *	without the split pte hack.
 */
#define PTETOPHYS(pte)		(pte & PG_PFNUM)
#define PHYSTOPTE(paddr) 	(paddr & PG_PFNUM)


/* Field masks and bits */

#if	MMAX_XPC || MMAX_APC
#define PG_V_BIT	0
#define PG_V		(1 << PG_V_BIT)
#define PG_PROT		0x00000006
#define PG_CI		0x00000040
#define PG_CI_BIT	6
#define PG_R		0x00000080
#define PG_M		0x00000100
#define PG_WIRED	0x00000200
#define PG_PFNUM	0xfffff000
#define PG_BLKNO	0xfffff000
#endif	/* MMAX_XPC || MMAX_APC */

#if	MMAX_DPC
#define PG_V_BIT	0
#define PG_V		(1 << PG_V_BIT)
#define PG_PROT		0x00000006
#define PG_R		0x00000008
#define PG_M		0x00000010
#define PG_WIRED	0x00000080
#define PG_PFNUM	0xfffffe00
#define PG_BLKNO	0xfffffe00
#endif	/* MMAX_DPC */

/* Protection values for cast-as-int operations */

#define PG_RONA		0x00000000
#define PG_NOACC	0x00000000
#define PG_KR		0x00000000
#define PG_RWNA		0x00000002
#define PG_KW		0x00000002
#define PG_RWRO		0x00000004
#define PG_URKW		0x00000004
#define PG_URKR		0x00000004
#define PG_RWRW		0x00000006
#define PG_UWKW		0x00000006
#define PG_UW		0x00000006

/* Bit-field value definitions */

#define PG_VALID	0x1
#define PG_KERNELREAD	0x0
#define PG_KERNELWRITE	0x1
#define PG_USERREAD	0x2
#define PG_USERWRITE	0x3
#define PG_REFERENCED	0x1
#define PG_MODIFIED	0x1
#define PG_ISWIRED	0x1

/* Masks for computing page table indices */

#if	MMAX_XPC || MMAX_APC
#define VA_OFFMASK	0x00000fff	/* Virtual address offset	*/
#define VA_L2MASK	0x003ff000	/* Virtual address level2 mask	*/
#define VA_L1MASK	0xffc00000	/* Virtual address level1	*/
#define VA_PTBMASK      0xfffff000      /* L1 page table on 4k boundary */
#define VA_PT2MASK      0xfffff000      /* L2 page table on 4k boundary */
#define VA_SIZE		32		/* Bits in a Virtual Address	*/
#endif	/* MMAX_XPC || MMAX_APC */

#if	MMAX_DPC
#define VA_OFFMASK	0x000001ff
#define VA_L2MASK	0x0000fe00
#define VA_L1MASK	0xffff0000
#define VA_PTBMASK	0xfffffc00
#define VA_PT2MASK	0xfffffe00
#endif	/* MMAX_DPC */

/* Bit offsets */

#if	MMAX_XPC || MMAX_APC
#define PG_VOFF		0
#define PG_PROTOFF	1
#define PG_ROFF		7
#define PG_MOFF		8
#define PG_WIREDOFF	9
#define PG_PFOFF	12
#define PG_BLKOFF	12
#define PG_PFLTH	20
#define PG_L2OFF	12
#define PG_L2LTH	10
#define PG_L1OFF	22
#define PG_L1LTH	10
#define VA_L2SHIFT	PG_L2OFF
#define VA_L1SHIFT	PG_L1OFF
#endif	/* MMAX_XPC || MMAX_APC */

#if	MMAX_DPC
#define PG_VOFF		0
#define PG_PROTOFF	1
#define PG_ROFF		3
#define PG_MOFF		4
#define PG_WIREDOFF	7
#define PG_PFOFF	9
#define PG_BLKOFF	9
#define PG_PFLTH	23
#define PG_L2OFF	9
#define PG_L2LTH	7
#define PG_L1OFF	16
#define PG_L1LTH	16
#define VA_L2SHIFT	PG_L2OFF
#define VA_L1SHIFT	PG_L1OFF
#endif	/* MMAX_DPC */

/* Random junk */

#define PG_FMAX		(NOFILE)

#if	MMAX_XPC || MMAX_APC
#define PTE1_SIZE       4096            /* Byte size of Level 1 page table  */
#define PTE2_SIZE       4096            /* Byte size of Level 2 page table. */
#define PTE1_ENTRIES    1024            /* Entries in Level 1 page table.   */
#define PTE2_ENTRIES    1024            /* Entries in Level 2 page table.   */
#define MAPPED_PER_PTE2 4096            /* Memory bytes mapped by a L2 PTE  */
#define BYTES_PER_PTE2     4            /* # Bytes in a Level 2 PTE         */
#endif	/* MMAX_XPC || MMAX_APC */

#if	MMAX_DPC
#define PTE1_SIZE	1024		/* Byte size of Level 1 page table. */
#define PTE2_SIZE	512		/* Byte size of Level 2 page table. */
#define PTE1_ENTRIES	256		/* Entries in Level 1 page table.   */
#define PTE2_ENTRIES	128		/* Entries in Level 2 page table.   */
#define MAPPED_PER_PTE2 512             /* Memory bytes mapped by a L2 PTE  */
#define BYTES_PER_PTE2    4             /* # Bytes in a Level 2 PTE         */
#endif	/* MMAX_DPC */

/*
 * Pte related macros
 */
#define L1IDX(vaddr)	((unsigned long int)(vaddr & VA_L1MASK) >> VA_L1SHIFT)
#define L2IDX(vaddr)	((unsigned long int)(vaddr & VA_L2MASK) >> VA_L2SHIFT)

#ifndef	LOCORE
#define DIRTY(pte)	((pte) -> pg_v && (pte) -> pg_m)

#define PHYSADDR(pte, vaddr) (PTETOPHYS(*pte) | (vaddr & PGOFSET))

#define ns32k_pfn(addr)	((unsigned long int)((addr) & PG_PFNUM) >> PG_PFOFF)
#endif	/* LOCORE */

#endif	/* __MMAX_PTE_H */
