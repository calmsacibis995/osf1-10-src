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
 * @(#)pq.h	5.1	(ULTRIX)	6/19/91
 */
/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/* $Header: /usr/sde/alpha/rcs/alpha/src/./kernel/io/dec/ws/pq.h,v 1.1.2.4 1993/01/22 15:18:45 Bob_Berube Exp $
 *
 * 13-Aug-90	Sam Hsu
 *	Convert to multiscreen.  Based on io/tc/gq.h.
 */
#ifndef _PQ_H_
#define _PQ_H_

/*
 * 128/256KB of SRAM on graphics accelerator option board.
 *
 * This stuff should EXACTLY correspond to the ucode layout, please...
 * We define the major organs here.  Internals are left with the ucode.
 * ANY changes here MUST be coordinated with the N10 ucode.
 */
#define N10_RAM_SIZE		(0x20000)
#define N10_PAGE		(0x1000)
#define N10_OUTPUT_BUFCNT	(2)

#define N10_OUTPUT_BUFFERS_OFF	(0 * N10_PAGE)
#define N10_OUTPUT_BUFFER_SIZE	(N10_PAGE)
#define N10_OUTPUT_BUFFERS_SIZE	(N10_OUTPUT_BUFCNT * N10_OUTPUT_BUFFER_SIZE)
#define N10_IMAGE_BUFFER_OFF	(N10_OUTPUT_BUFFERS_OFF+N10_OUTPUT_BUFFERS_SIZE)
#define N10_IMAGE_BUFFER_SIZE	(((2*(1279+1)+4+7) & ~7) * sizeof(int))
#define N10_INTR_R3K_OFF	(N10_IMAGE_BUFFER_OFF+N10_IMAGE_BUFFER_SIZE)
#define N10_INTR_N10_OFF	(N10_INTR_R3K_OFF+sizeof(int)+sizeof(int))
#define N10_SAVE_OFF		(N10_INTR_N10_OFF+sizeof(int)+sizeof(int))

typedef int pq_reqbuf[2*N10_OUTPUT_BUFFER_SIZE/sizeof(int)];

typedef struct _pq_ram {
    /* graphics console packet area */
    pq_reqbuf reqbuf[N10_OUTPUT_BUFCNT];
    /* read/write spans image buffer + trailing WIDTH for readspans */
    int pixbuf[2*N10_IMAGE_BUFFER_SIZE/sizeof(int)];
    /* begin 2 interrupt words quadword aligned */
    int intr_host; int _pad0; int intr_host_high; int _pad1;
    int intr_coproc; int _pad2; int intr_coproc_high; int _pad3;
    /* rest is a mystery */
    char memory[2*(N10_RAM_SIZE - N10_SAVE_OFF)];
    char hi_mem[2*(N10_RAM_SIZE)];
} pq_ram;

typedef int pq_reqbufd[N10_OUTPUT_BUFFER_SIZE/sizeof(int)];

typedef struct _pq_ramd {
    /* graphics console packet area */
    pq_reqbufd reqbuf[N10_OUTPUT_BUFCNT];
    /* read/write spans image buffer + trailing WIDTH for readspans */
    int pixbuf[N10_IMAGE_BUFFER_SIZE/sizeof(int)];
    /* begin 2 interrupt words quadword aligned */
    int intr_host; int intr_host_high;
    int intr_coproc; int intr_coproc_high;
    /* rest is a mystery */
    char memory[N10_RAM_SIZE - N10_SAVE_OFF];
    char hi_mem[N10_RAM_SIZE];
} pq_ramd;

#define PQ_REQBUF(X,N)	(PQ_RAMD(X)->reqbuf[(N)%N10_OUTPUT_BUFCNT])

/* convert a system virtual address to SRAM physical address */
#ifdef __alpha
/* 
 * Sparse space definition
 */
#define PQ_SYS_TO_PHYS(C,A)	((u_long)(A) - (u_long)PQ_RAMD(C))
#else /* __alpha */
#define PQ_SYS_TO_PHYS(C,A)	((u_long)(A) - (u_long)PQ_RAM(C))
#endif /* __alpha */


#if defined(KERNEL) || defined(PQ_DEBUG)
/*
 * Template for accessing various fields on PQ board.  #if'd to avoid
 * having to pull in Bt459 definitions for user programs that aren't
 * debugging the board.
 */
typedef struct _pq_map {
#ifdef __alpha
    /* 0x000000: STIC polling register (initiate DMA read) */
    int		stic_poll_reg;  char __pad05[0x300000-sizeof(int)];

    /* 0x300000: STIC control registers */
#define PQ_STIC_OFFSET	0x300000
#define PQ_STAMP_OFFSET	0x180000
    stic_regs	stic_reg;  char __pad10[0x400000-0x300000-sizeof(stic_regs)];

    /* 0x400000: SRAM */
    pq_ram	ram;  char __pad15[0x500000-0x400000-sizeof(pq_ram)];

    /*
     * low 17 bits specify where in SRAM the write will appear (see also
     * pq_ram structure definition).
     */
    char __pad20[2*N10_INTR_R3K_OFF];

    /* 0x..500000 N10 interrupt host / host clear interrupt */
    /* 10 100x xxxx xxxx xxxx xxxx */
    int		intr_host;
    int		_pad0;
    int		intr_host_high;
    char __pad25[0x580000-0x500000-(3*sizeof(int))
					-(2*N10_INTR_R3K_OFF)];
    char __pad30[2*N10_INTR_N10_OFF];

    /* 0x..580000: host interrupt N10 / N10 clear interrupt */
    /* 10 110x xxxx xxxx xxxx xxxx */
    int 	intr_coproc;
    int		_pad1;
    int		intr_coproc_high;
    char __pad35[0x600000-0x580000-(3*sizeof(int))
					  -(2*N10_INTR_N10_OFF)];

    /* 0x600000: Brooktree 459 VDAC */
#define PQ_BT459_OFFSET	0x300000
    struct bt459 vdac;  char __pad45[0x680000-0x600000-sizeof(struct bt459)];
    /* Bt459 chip reset */
#define PQ_BT459_RESET	0x340000
    int		vdac_reset;  char __pad40[0x700000-0x680000-sizeof(int)];

    /* start/reset N10 write-only (also ROM read-only) */
    int		start_coproc; char __pad50[0x780000-0x700000-sizeof(int)];
    int		reset_coproc;		/* 0x780000 */

#else /* __alpha */

    /* 0x000000: STIC polling register (initiate DMA read) */
    int		stic_poll_reg;  char __pad05[0x180000-sizeof(int)];

    /* 0x180000: STIC control registers */
#define PQ_STIC_OFFSET	0x180000
#define PQ_STAMP_OFFSET	0x0C0000
    stic_regs	stic_reg;  char __pad10[0x200000-0x180000-sizeof(stic_regs)];

    /* 0x200000: SRAM */
    pq_ram	ram;  char __pad15[0x280000-0x200000-sizeof(pq_ram)];

    /*
     * low 17 bits specify where in SRAM the write will appear (see also
     * pq_ram structure definition).
     */		      char __pad20[N10_INTR_R3K_OFF];

    /* 0x..280000 N10 interrupt host / host clear interrupt */
    /* 10 100x xxxx xxxx xxxx xxxx */
    int		intr_host;  char __pad25[0x2c0000-0x280000-sizeof(int)
					-N10_INTR_R3K_OFF];
                      char __pad30[N10_INTR_N10_OFF];

    /* 0x..2c0000: host interrupt N10 / N10 clear interrupt */
    /* 10 110x xxxx xxxx xxxx xxxx */
    int 	intr_coproc;  char __pad35[0x300000-0x2c0000-sizeof(int)
					  -N10_INTR_N10_OFF];

    /* 0x300000: Brooktree 459 VDAC */
#define PQ_BT459_OFFSET	0x300000
    struct bt459 vdac;  char __pad45[0x380000-0x340000-sizeof(struct bt459)];
    /* Bt459 chip reset */
#define PQ_BT459_RESET	0x340000
    int		vdac_reset;  char __pad40[0x340000-0x300000-sizeof(int)];

    /* start/reset N10 write-only (also ROM read-only) */
    int		start_coproc; char __pad50[0x3c0000-0x380000-sizeof(int)];
    int		reset_coproc;		/* 0x3c0000 */
#endif /* __alpha */
} pq_map;

#ifdef __alpha
typedef struct _pq_mapd {
    /* 0x000000: STIC polling register (initiate DMA read) */
    int		stic_poll_reg;  char __pad05[0x180000-sizeof(int)];

    /* 0x180000: STIC control registers */
#define PQ_STIC_OFFSETD		0x180000
#define PQ_STAMP_OFFSETD	0x0C0000
    char __pad10[0x200000-0x180000];

    /* 0x200000: SRAM */
    pq_ramd	ram;  char __pad15[0x280000-0x200000-sizeof(pq_ramd)];

    /*
     * low 17 bits specify where in SRAM the write will appear (see also
     * pq_ram structure definition).
     */		      char __pad20[N10_INTR_R3K_OFF];

    /* 0x..280000 N10 interrupt host / host clear interrupt */
    /* 10 100x xxxx xxxx xxxx xxxx */
    int		intr_host;
    int		intr_host_high;

    char __pad25[0x2c0000-0x280000-sizeof(int)-sizeof(int)
					-N10_INTR_R3K_OFF];
    char __pad30[N10_INTR_N10_OFF];

    /* 0x..2c0000: host interrupt N10 / N10 clear interrupt */
    /* 10 110x xxxx xxxx xxxx xxxx */
    int 	intr_coproc;
    int		intr_coproc_high;
    char __pad35[0x300000-0x2c0000-sizeof(int)-sizeof(int)
					  -N10_INTR_N10_OFF];

    /* 0x300000: Brooktree 459 VDAC */
#define PQ_BT459_OFFSETD	0x300000
    char __pad45[0x340000-0x300000];
    /* Bt459 chip reset */
#define PQ_BT459_RESETD	0x340000
    int		vdac_reset;  char __pad40[0x380000-0x340000-sizeof(int)];

    /* start/reset N10 write-only (also ROM read-only) */
    int		start_coproc; char __pad50[0x3c0000-0x380000-sizeof(int)];
    int		reset_coproc;		/* 0x3c0000 */
} pq_mapd;
#endif /* __alpha */

#endif /* kernel || pq_debug */


/*
 * for px_info's dev_closure
 */
typedef struct _pq_info {
    unsigned int bufselect;
    unsigned int n10K;
    unsigned int unwedge_stic;
    unsigned int dropped_packet;
    unsigned int stic_timeout;
} pq_info;

typedef struct _pq_ptpt {
    unsigned long
#ifdef __alpha
	pq_pg_pfnum : 20,
        pq_pg_v : 1,
        pq_pg_m : 1,
        pq_pg_tag_lo : 10,
        pq_pg_tag_hi : 32;
#else /* __alpha */
        pq_pg_tag : 9,
        pq_pg_v : 1,
        pq_pg_m : 1,
      	unused : 1,
        pq_pg_pfnum : 20;
#endif /* __alpha */
} pq_ptpt;

#define PQ_PTPT_ENTRIES	4096
#define PQ_PTPT_SIZE	(PQ_PTPT_ENTRIES * sizeof(pq_ptpt))
#define PQ_STLB(_vpn)	((_vpn) & (PQ_PTPT_ENTRIES-1))

#ifdef __alpha
#define PQ_PTPT_PPN	0x00000000000fffff	/* ptp ppn */
#define PQ_PTPT_VSN	0xffffffffffc00000	/* vsn for this ppn */
#define PQ_PTPT_VSN_LO	0x00000000ffc00000	/* tag_lo */
#define PQ_PTPT_VSN_HI	0xffffffff00000000	/* tag_hi */
#define pq_vtovsn(_va)	(((unsigned long) (_va) & PQ_PTPT_VSN ) >> 22 )
#define pq_vtovsn_lo(_va) (((unsigned long) (_va) & PQ_PTPT_VSN_LO ) >> 22 )
#define pq_vtovsn_hi(_va) (((unsigned long) (_va) & PQ_PTPT_VSN_HI ) >> 32 )
#else /* __alpha */
#define PQ_PTPT_PPN	0xfffff000	/* ptp ppn */
#define PQ_PTPT_VSN	0x000001ff	/* vsn for this ppn */
#define pq_vtovsn(_va)	(((_va) >> 22) & PQ_PTPT_VSN)
#endif /* __alpha */

/******************************************************************************
 * R3K intr N10:
 *
 ******	Invalidate one PTE (paging):
 *	intr_coproc = (vpn & PQ_INTR_MASK) | PQ_INTR_INV1;
 *	while (intr_coproc) ;
 *  N10:
 *	intr_coproc = 0;
 *	<invalidate VTLB entry>
 *
 ****** Invalidate all PTEs (swapping):
 *	<clear all PTPT entries>
 *	intr_coproc = PQ_INTR_INVA;
 *	while (intr_coproc) ;
 *  N10:
 *	intr_coproc = 0;
 *	bzero(VPTP, sizeof(VPTP));
 *	bzero(VTLB, sizeof(VTLB));
 *
 ****** PTE ceil (brk'ing):
 *	<clear PTPT entry, if necessary>
 *	intr_coproc = (vpn & PQ_INTR_MASK) | PQ_INTR_CEIL;
 *	while (intr_coproc) ;
 *  N10:
 *	intr_coproc = 0;
 *	<invalidate all VTLB/VPTP entries >= vpn>
 *	<N10 has option to collapse this into invalidate-all>
 *
 ****** Halt N10:
 *	intr_coproc = PQ_INTR_HALT;
 *	while (intr_coproc) ;
 *  N10:
 *	intr_coproc = 0;
 *	halt();
 *
 ******************************************************************************
 * N10 intr R3:
 *
 ****** PageIn (pte invalid):
 *	while (intr_host == HST_INTR_VSYN) ;
 *	intr_host = (VA & HST_INTR_MASK) | HST_INTR_PGIN;
 *	while (intr_host) ;
 *  R3K:
 *	intr_host = intr_host;
 *	signal(server);
 *	[...in server...]
 *	int tmp = *(volatile int *)(intr_host & HST_INTR_MASK);
 *	intr_host = 0;
 *
 ****** Xlate (can't complete address translation):
 *	while (intr_host == HST_INTR_VSYN) ;
 *	intr_host = tmp = (VA & HST_INTR_MASK) | HST_INTR_XLAT;
 *	while ((reg = intr_host) == tmp) ;
 *	if (reg == 0)
 *		<retry address translation>;
 *	else
 *		PA = reg | (VA & 0xfff);
 *  R3K:
 *	vpn = fill_ptpt_entry(ptpt, intr_host);
 *	pte = vtopte(server, vpn);
 *	if (pte->pq_pg_v)
 *		intr_host = pte->pq_pg_pfnum << PQ_INTR_SHFT;
 *	else
 *		intr_host = intr_host;
 *		signal(server);
 *	[...in server...]
 *	int tmp = *(volatile int *)(intr_host & HST_INTR_MASK);
 *	intr_host = 0;
 *
 ****** Vertical blank (install colormap or cursor):
 *	intr_host = HST_INTR_VSYN;
 *  R3K:
 *	intr_host = 0;
 *	if (new_colormap)
 *		install new colormap;
 */

#define HST_INTR_MASK	0xfffff000
#define HST_INTR_SHFT	0x0000000c	/* 12. */

#define HST_INTR_WHAT	0x0000000f
#define HST_INTR_WSHF	0x00000000
#define HST_INTR_PGIN	0x00000001	/* page in */
#define HST_INTR_XLAT	0x00000002	/* translate */
#define HST_INTR_VSYN	0x00000003	/* vblank sync */
#define HST_INTR_PMSK	0x00000f00	/* args mask */
#define HST_INTR_PADD	0x00000700	/* add'l pages */
#define HST_INTR_DRTY	0x00000800	/* dirty mask */
#define HST_INTR_PSHF	0x00000010	/* 16. */

#define PQ_INTR_MASK	HST_INTR_MASK
#define PQ_INTR_SHFT	HST_INTR_SHFT

#define PQ_INTR_WHAT	0x000000f0
#define PQ_INTR_WSHF	0x00000004
#define PQ_INTR_INV1	0x00000010	/* invalidate 1 pte */
#define PQ_INTR_INVA	0x00000020	/* invalidate all ptes */
#define PQ_INTR_CEIL	PQ_INTR_INVA
#define PQ_INTR_PAUS	0x00000030	/* pause N10 */
#define PQ_INTR_BUF0	0x00000001
#define PQ_INTR_BUF1	0x00000002
#define PQ_INTR_HALT	0x000000f0	/* unused */


#ifdef KERNEL
/*
 * px closure to pq field ptr
 */
#define PQ_MAP(C)		((pq_map *)PX_BASE(C))
#define PQ_POLL(G)		(& (PQ_MAP(G)->stic_poll_reg) )
#define PQ_VDAC(G)		(& (PQ_MAP(G)->vdac) )
#define PQ_VDACRESET(G)		(& (PQ_MAP(G)->vdac_reset) )
#define PQ_RAM(G)		(& (PQ_MAP(G)->ram) )
#define PQ_STIC(G)		( (stic_regs *)((G)->stic) )
#define PQ_STAMP(G)		( (int *)((G)->stamp) )
#define PQ_INTRC(G)		(& (PQ_MAP(G)->intr_coproc) )
#define	PQ_INTRC_HIGH(G)	(& (PQ_RAM(G)->intr_coproc_high) )
#define PQ_INTRH(G)		(& (PQ_MAP(G)->intr_host) )
#define	PQ_INTRH_HIGH(G)	(& (PQ_RAM(G)->intr_host_high) )

#ifdef __alpha
#define PQ_MAPD(C)		((pq_mapd *)PX_BASED(C))
#define	PQ_RAMD(G)		(& (PQ_MAPD(G)->ram) )
#define PQ_START(G)		(& (PQ_MAPD(G)->start_coproc) )
#define PQ_RESET(G)		(& (PQ_MAPD(G)->reset_coproc) )
#else /* __alpha */
#define PQ_START(G)		(& (PQ_MAP(G)->start_coproc) )
#define PQ_RESET(G)		(& (PQ_MAP(G)->reset_coproc) )
#endif

#else
/*
 * user-level debug of board
 */
#ifdef PQ_DEBUG
#define PQO_MAP(X)	 	(  ( pq_map *)(((pxInfo *)(X))->dev.pq.pxo))
#define PQO_POLL(X)	 	(& (PQO_MAP(X)->stic_poll_reg) )
#define PQO_VDAC(X)	 	(& (PQO_MAP(X)->vdac) )
#define PQO_VDACRESET(X) 	(& (PQO_MAP(X)->vdac_reset) )
#define PQO_RAM(X)	 	(& (PQO_MAP(X)->ram) )
#define PQO_STIC(X)	 	(& (PQO_MAP(X)->stic_reg) )
#define PQO_STAMP(X)	      ((int)PQO_MAP(X)+PQ_STAMP_OFFSET)
#define PQO_INTRC(G)		(& (PQO_MAP(G)->intr_coproc) )
#define PQO_INTRH(G)		(& (PQO_MAP(G)->intr_host) )
#define PQO_START(G)		(& (PQO_MAP(G)->start_coproc) )
#define PQO_RESET(G)		(& (PQO_MAP(G)->reset_coproc) )
#define PQO_REQBUF(X,N)		(   PQO_MAP(X)->ram.reqbuf[(N)%N10_OUTPUT_BUFCNT] )
#define PQO_IMGBUF(X)		(   PQO_MAP(X)->ram.pixbuf )
#define PQO_SYS_TO_SRAM(X,A)	((int)(A) - (int)PQO_RAM(X))
#endif

#endif /* kernel */

/*
 * ALL PXG ioctl's MUST begin with (struct px_ioc).
 */

#define PQ_IOC_RESET	_IOW ('w', (0|IOC_S), px_ioc)
#define PQ_N10_RESET	1

#define PQ_IOC_START	_IOW ('w', (0|IOC_S), px_ioc)
#define PQ_N10_START	2

#define PQ_IOC_INTR	_IOWR('w', (0|IOC_S), px_ioc)
#define PQ_N10_INTR	3

/* for backward compatibility - see also PX_IOC_MAP */
#define PQ_IOC_MAP	_IOWR('w', (0|IOC_S), px_ioc)
#define PQ_N10_MAP	4

#ifdef KERNEL
int	pq_attach();
int	pq_bootmsg();
int	pq_setup();
int	pq_ioctl();
void	pq_close();
int	pq_intr_coproc();
int	pq_map_screen();
int *	pq_getPacket();
void	pq_sendPacket();
void	pq_getImageBuffer();
void	pq_interrupt();
int     pq_invalidate_gcp_tlb();
#endif
#endif /* _PQ_H_ */

