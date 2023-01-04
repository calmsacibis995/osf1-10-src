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
 * Alpha page table entry
 *
 * There are two major kinds of pte's: those which have ever existed
 * (and are thus either now in core or on the swap device), and those
 * which have never existed, but which will be filled on demand at first
 * reference.  There is a structure describing each.  There is also an
 * ancillary structure used in page clustering.
 */

/*----------------------------------------------------------------------
 * Modification History
 *
 *  1-Oct-90 -- jmartin
 *	Change int to long in union pte_words.
 *
 * 25-Sep-90 -- jmartin
 *	Define forkutl.
 *----------------------------------------------------------------------
 */
#ifndef _PTE_H_
#define _PTE_H_

#ifdef __LANGUAGE_C__
struct pte
{
unsigned int	pg_v:1,		/* valid */
		pg_flt_on:3,	/* fault on read/write/execute */
		pg_asm:1,	/* address space match */
		pg_gh:2,	/* granularity hint */
		:1,		/* reserved for hardware future */
		pg_prot:8,	/* R/W kernel/executive/supervisor/user */
		pg_cow:1,	/* (SOFTWARE) copy-on-write */
		pg_sav_flt_on:3,/* (SOFTWARE) saved when c_ref is cleared;
				 	      restored when c_ref is set */
		pg_m:1,		/* (SOFTWARE) modify (moving to cmap) */
		:9,
		pg_swapm:1,	/* (SOFTWARE) have to write back to swap */
		pg_fod:1,	/* (SOFTWARE) fill on demand (==0) */
		pg_pfnum:32;	/* page frame number */
};

struct fpte
{
unsigned int	pg_v:1,		/* valid (==0) */
		:7,
		pg_prot:8,	/* R/W kernel/executive/supervisor/user */
		:14,
		pg_fileno:1,	/* file mapped from or TEXT or ZERO */
		pg_fod:1,	/* (SOFTWARE) fill on demand (==1) */
		pg_blkno:32;	/* file system block number */
};

union pte_words {
	long hardware_word;
	struct pte whole_pte;
};
#endif /* __LANGUAGE_C__ */

#define	PG_V		0x00000001L
#define PG_ASM		0x00000010L
#define PG_GH		0x00000060L
#define	PG_PROT		0x0000ff00L
#define	PG_M		0x00100000L
#define	PG_FOD		0x80000000L
#define	PG_PFNUM	0xffffffff00000000

#define	PG_FZERO	0L
#define	PG_FTEXT	1L
#define	PG_FMAX		(PG_FTEXT)

#define	PG_NOACC	0x00000000L
#define	PG_KW		0x00001100L
#define	PG_KR		0x00000100L

#define	PG_UW		0x00003300L
#define	PG_URKW		0x00001300L
#define	PG_URKR		0x00000300L

/* these are dummy args for mips compatability */
#define DO_CACHE	0x0
#define NO_CACHE	0x0

#define	PROT_NOACC	0x00
#define	PROT_KW		0x11
#define	PROT_KR		0x01
#define	PROT_UW		0x99
#define	PROT_URKW	0x19
#define	PROT_URKR	0x09

/* Virtual address mask constants */
#define VA_BYTEOFFS	(NBPG - 1)	/* jmfix: should be defined elsewhere */
#define VA_USER		(0x40000000)	/* jmfix: bogus, should be eliminated */
#define VA_SYS		(0x80000000)	/* jmfix: bogus, should be eliminated */
#define VA_SPACE	(VA_SYS | VA_USER)

#define PTE_PFNSHIFT	32
#define NOTaPFN		((unsigned long)(~0L))

#ifdef __LANGUAGE_C__
/*
 * Pte related macros
 */
#define	dirty(pte)	((pte)->pg_fod == 0 && (pte)->pg_pfnum && \
			    ((pte)->pg_m || (pte)->pg_swapm))

#define SET_SWDIRTY(pte) ((pte)->pg_m = 1)

#define CLEAR_DIRTY(pte) ((pte)->pg_m = 0)

#ifdef KERNEL
struct	pte *vtopte();

/* utilities defined in locore.s */
#define Sysmap	((struct pte *)0xfffffffe00000000)
struct pte *Usrptmap;
struct pte *usrpt;
#define forkutl	(*(struct user *)((char *)(UADDR - MAX_NBPG)))
#define Forkmap	(u.u_procp->p_addr - (1 << (MAX_PGSHIFT - PGSHIFT)))
extern	struct pte Swapmap[];
#ifdef notyet
extern	struct pte Xswapmap[];
extern	struct pte Xswap2map[];
extern	struct pte Pushmap[];
extern	struct pte mmap[];
extern	struct pte vmbinfomap[];
extern	struct pte Nexmap[][16];
extern	struct pte Ioamap[][1];
extern	struct pte scsmemptmap[];
extern  struct pte scsmempt[];
extern  struct pte eUsrptmap[];
#endif
struct pte *kmempt;
struct pte *ekmempt;

#ifdef ALPHAADU
struct pte *tv_cnfg_pt;		/* map for tvbus configuration area */
struct pte *adu_tv_io_pt;	/* map for tvbus register space	*/
#endif


#ifdef	KDEBUG
extern	struct pte Kdbmap[];
#endif /* KDEBUG */
#endif /* KERNEL */
#endif /* __LANGUAGE_C__ */

#endif /* _PTE_H_ */
