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
 * @(#)px.h	5.2	(ULTRIX)	6/19/91
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
#ifndef _PXDEFS_H_
#define _PXDEFS_H_

/*
 * Common definitions for PixelStamp devices.
 *
 * Modification history:
 *
 * 13-Nov-1991 -- Lloyd Wheeler
 *		Tweaked includes for OSF kernel build environment.
 *
 * 13-Aug-1990 -- Sam Hsu
 *		Modified for PX, PXG, PXG-T.
 *
 * 27-May-1990 -- Win Treese
 *		Created, with heavy influence from gx.h
 */

#ifdef KERNEL
#      include <sys/types.h>
#      include <sys/ioctl.h>
#      include <io/dec/ws/stamp.h>
#else /* KERNEL */
#      include <sys/param.h>
#      include <sys/ioctl.h>
#endif /* KERNEL */

/*
 * Fixup some MIPS-only macros for Alpha environment
 */
#ifdef __alpha
#ifdef PHYS_TO_K0
#undef PHYS_TO_K0
#endif /* PHYS_TO_K0 */
#ifdef PHYS_TO_K1
#undef PHYS_TO_K1
#endif /* PHYS_TO_K1 */
#ifdef K0_TO_PHYS
#undef K0_TO_PHYS
#endif /* K0_TO_PHYS */
#ifdef K1_TO_PHYS
#undef K1_TO_PHYS
#endif /* K1_TO_PHYS */
#define PHYS_TO_K0(_a)		PHYS_TO_KSEG(_a)
#define	PHYS_TO_K1(_a)		PHYS_TO_KSEG(_a)
#define K0_TO_PHYS(_a)		KSEG_TO_PHYS(_a)
#define	K1_TO_PHYS(_a)		KSEG_TO_PHYS(_a)
#endif /* __alpha */

/*
 * Add some macros to walk through the PixelStamp packet formatting.
 * The base pointers for doing this *must* be 32bits.
 */
#ifdef __alpha
#define STORE_STAMP_PTR( _p, _val )	*(unsigned int *) (_p) = (unsigned int) (_val)
#define INC_STAMP_PTR( _p )		_p++;
#else /* __alpha */ 
#define STORE_STAMP_PTR( _p, _val )	*(unsigned int *) (_p) = (unsigned int) (_val)
#define INC_STAMP_PTR( _p )		_p++;
#endif /* __alpha */

#define PX_LOAD(A,B) 	((A)&0xfff) | (((B)&0xfff) << 16)
#define PX_GET2ROWS(C)	PX_LOAD((int)(*(C)),(int)(*((C)+1)));(C)+=2

/*
 * Process virtual address manipulation macros
 */
#ifdef __alpha
#define	PX_PVA_VTLB_INDEX( _pva )	(((_pva) & 0x00000000003fe000 ) >> 13 )
#define	PX_PVA_STLB_INDEX( _pva )	(((_pva) & 0x0000000001ffe000 ) >> 13 )
#define PX_PVA_TAG( _pva )		(((_pva) & 0xffffffffffc00000 ) >> 22 )
#define	PX_PVA_PAGE_OFFSET( _pva )	((_pva)  & 0x0000000000001fff )
#else /* __alpha */
#define	PX_PVA_VTLB_INDEX( _pva )	(((_pva) & 0x003ff000 ) >> 12 )
#define	PX_PVA_STLB_INDEX( _pva )	(((_pva) & 0x00fff000 ) >> 12 )
#define PX_PVA_TAG( _pva )		(((_pva) & 0x7fc00000 ) >> 22 )
#define	PX_PVA_PAGE_OFFSET( _pva )	((_pva)  & 0x00000fff )
#endif /* __alpha */

/*
 * STamp Interface Chip
 */
typedef struct _stic_regs {
#ifdef __alpha
    int __pad0;			/* 0x..300000 */
    int __pad1;
    int __pad2;			/* 0x..300008 */
    int __pad3;
    int	hsync;			/* 0x..300010 */
    int __pad4;
    int	hsync2;			/* 0x..300018 */
    int __pad5;
    int	hblank;			/* 0x..300020 */
    int __pad6;
    int	vsync;			/* 0x..300028 */
    int __pad7;
    int	vblank;			/* 0x..300030 */
    int __pad8;
    int	vtest;			/* 0x..300038 */
    int __pad9;
    int	ipdvint;		/* 0x..300040 */
    int __pad10;
    int	__pad11;		/* 0x..300048 */
    int __pad12;
    int	sticsr;			/* 0x..300050 */
    int __pad13;
    int	busdat;			/* 0x..300058 */
    int __pad14;
    int	busadr;			/* 0x..300060 */
    int __pad15;
    int	__pad16;		/* 0x..300068 */
    int __pad17;
    int	buscsr;			/* 0x..300070 */
    int __pad18;
    int	modcl;			/* 0x..300078 */
    int __pad19;
#else /* __alpha */
    int __pad0;			/* 0x..180000 */
    int __pad1;			/* 0x..180004 */
    int	hsync;			/* 0x..180008 */
    int	hsync2;			/* 0x..18000c */
    int	hblank;			/* 0x..180010 */
    int	vsync;			/* 0x..180014 */
    int	vblank;			/* 0x..180018 */
    int	vtest;			/* 0x..18001c */
    int	ipdvint;		/* 0x..180020 */
    int	__pad2;			/* 0x..180024 */
    int	sticsr;			/* 0x..180028 */
    int	busdat;			/* 0x..18002c */
    int	busadr;			/* 0x..180030 */
    int	__pad3;			/* 0x..180034 */
    int	buscsr;			/* 0x..180038 */
    int	modcl;			/* 0x..18003c */
#endif /* __alpha */
} stic_regs;

typedef struct _stic_csr {
    unsigned tstfnc : 2;
    unsigned checkpar : 1;
    unsigned startvt : 1;
    unsigned start : 1;
    unsigned reset : 1;
    unsigned autoread : 1;
    unsigned startst : 1;
    unsigned : 24;
} stic_csr;

/* Masks for STIC CSR register */

#define STIC_CSR_TSTFNC		0x00000003
#define STIC_CSR_TSTFNC_NORMAL 	0
#define STIC_CSR_TSTFNC_PARITY 	1
#define STIC_CSR_TSTFNC_CNTPIX 	2
#define STIC_CSR_TSTFNC_TSTDAC 	3
#define STIC_CSR_CHECKPAR	0x00000004
#define STIC_CSR_STARTVT	0x00000008
#define STIC_CSR_START		0x00000010
#define STIC_CSR_RESET		0x00000020
#define STIC_CSR_AUTOREAD	0x00000040
#define STIC_CSR_STARTST	0x00000080

typedef struct _stic_intr {
    unsigned	eint_en : 1;	     /* 0 */
    unsigned	eint : 1;	     /* 1 */
    unsigned	eint_mask : 1;	     /* 2 */
    unsigned	: 5;
    unsigned	vint_en : 1;	     /* 8 */
    unsigned	vint : 1;	     /* 9 */
    unsigned	vint_mask : 1;	     /* 10 */
    unsigned	: 5;
    unsigned	pint : 1;	     /* 16 */
    unsigned	pint_en : 1;	     /* 17 */
    unsigned	pint_mask : 1;	     /* 18 */
} stic_intr;

/* Masks for %int register */

#define STIC_INT_E_EN 0x00000001
#define STIC_INT_E    0x00000002
#define STIC_INT_E_WE 0x00000004
#define STIC_INT_V_EN 0x00000100
#define STIC_INT_V    0x00000200
#define STIC_INT_V_WE 0x00000400
#define STIC_INT_P_EN 0x00010000
#define STIC_INT_P    0x00020000
#define STIC_INT_P_WE 0x00040000

#define STIC_INT_WE   (STIC_INT_E_WE|STIC_INT_V_WE|STIC_INT_P_WE)
#define STIC_INT_CLR  (STIC_INT_E_EN|STIC_INT_E_WE|STIC_INT_V_WE|STIC_INT_P_WE)

typedef struct _stic_cf {
    unsigned	: 8;
    unsigned	vdac : 1;
    unsigned	yconfig : 2;
    unsigned	xconfig : 1;
    unsigned	option : 3;
    unsigned	: 9;
    unsigned	revision : 8;
} stic_cf;


/* Masks for %modcl bits. */

#define STIC_CF_VDAC_T		0x00000100 /* <8> */
#define STIC_CF_CONFIG_Y	0x00000600 /* <10:9> */
#define STIC_CF_CONFIG_X	0x00000800 /* <11> */
#define STIC_CF_CONFIG_OPTION	0x00007000 /* <14:12> */
#define STIC_CF_PLANES		0x00004000 /* <14> */
#define STIC_CF_ZPLANES		0x00001000 /* <12> */
#define STIC_CF_REV		0xff000000 /* <31:24> */

/* STIC option types (derived from the STIC modtype option field) */

#define STIC_OPT_2DA		0x0		/* 2D Accelerator */
#define STIC_OPT_2DA_SH		0x0		/* 2D Accelerator */
#define STIC_OPT_3DA_SH		STIC_CF_CONFIG_OPTION

/*
 * 2DA STIC polling address:
 *
 *   For 2DA, <15:17> aren't connected at all.  Ditto for <24:26>.
 *   Input <2:14+15:20> become the STIC's <2:14+18:23> and likewise
 *	for <21:22> to STIC <27:28>.
 *
 *   This means you can specify (input) 32K of physically contiguous
 *	memory to the STIC before bumping against some tie-lines, but...
 *
 *   The STIC can only "see" 23 bits of address (2da), and only if you form
 *	the polling address such that some bits get shift up into the
 *	STIC's <18:23> & <27:28>, since they're tied to <15:20> & <21:22>
 *	on the bus.  This means the packet buffers must be in the 1st
 *	8MB of physical memory.
 */

#define _0to14  (0x00007fff)	/* bits 00-14 set */
#define _0to23	(0x00ffffff)	/* bits 00-23 set */
#define _2to21  (0x003ffffc)	/* bits 02-21 set */
#define _6to8	(0x000001c0)	/* bits 00-08 set */
#define _11to28 (0x1ffff800)	/* bits 11-28 set */
#define _15to17	(0x00038000)	/* bits 15-17 set */
#define _15to20 (0x001f8000)	/* bits 15-20 set */
#define _18to23 (0x00fc0000)	/* bits 18-23 set */
#define _18to28 (0x1ffc0000)	/* bits 18-28 set */
#define _21to22 (0x00600000)	/* bits 21-22 set */
#define _24to28 (0x1f000000)	/* bits 24-28 set */
#define _27to28	(0x18000000)	/* bits 27-28 set */

/* Convert a system physical address to a STIC space physical address. */

#define PX_SYS2STIC(A) ((((A)&_21to22)<<6)|(((A)&_15to20)<<3)|((A)&_0to14))
#define PX_SYS_TO_STIC(A) PX_SYS2STIC(((u_long)(A)))

/* Convert a STIC space physical address to a system physical address. */

#define PX_STIC2SYS(A) ((((A)&_27to28)>>6)|(((A)&_18to23)>>3)|((A)&_0to14))
#define PX_STIC_TO_SYS(A) PX_STIC2SYS(((u_long)(A)))

/* Convert a system physical address to STIC DMA encoding - architecture spec */
#define PX_PHYS_TO_DMA(A) ((((u_long)(A)) & _11to28) >> 9)

/* Convert a system physical address to a DMA encoding - implementation spec */

#define PX_SYS2DMA(A)	( (((A)&~_0to14)<<3) | ((A)&_0to14) )
#define PX_SYS_TO_DMA(A)  PX_PHYS_TO_DMA(PX_SYS2DMA(((u_long)(A))))

/*
 * Information passed back 'n forth betwix kernel and user.  Procedure
 * is to MAP_SCREEN_AT_DEPTH, then GET_DEPTH_INFO.  After former, pixmap
 * will contain a pointer to a shared structure (below) as well as a mapped
 * ringbuffer (locked down memory for packet dispatch to STIC/N10).
 */
typedef struct _painfo {
    int *pxo;
#ifdef __alpha
    int *pxod;
#endif
    int *stic_dma_rb;
    stic_regs *stic_regs;
} paInfo;

typedef struct _pqinfo {
    int  *pxo;			/* board mapped at this virtual addr */
#ifdef __alpha
    int  *pxod;
#endif
    int  *ram;			/* SRAM on graphics board */
#ifdef __alpha
    int  *ramd;
#endif
    int   ptpt_phys;		/* phys addr of ptpt to pass to N10 */
    int   ptpt_size;
    int  *pgin_vaddr;
    short pgin_pages;		/* add'l pages */
    short pgin_dirty;		/* dirty the pages */
    int   ram_size;		/* SRAM size (128/256) */
} pqInfo;

/*
 * If you add/change a component in struct _pxinfo, be sure to add/change
 * a macro defintion for the same component name in the list of px__* macros
 * which follow.
 */
typedef struct _pxinfo {
    short	stamp_width;
    short	stamp_height;
    short	n10_present;		/* geometry accelerator		*/
    char	zplanes;		/* Z buffer			*/
    char	xplanes;		/* extra buffer - high-end only */
    int		rb_size;		/* ring buffer char length	*/
    u_long	rb_phys;		/* ring buffer phys addr	*/
    int *	rb_addr;		/* ring buffer virt addr	*/
    int		ib_size;		/* image buffer char length	*/
    int		idle_count;		/* nominally for PEX		*/
    int		idle_sample;		/* ditto			*/
    union {
	paInfo	pa;
	pqInfo	pq;
    } dev;
} pxInfo;

#ifdef ultrix
#define __px_prefix px.
#else /* ultrix */
#define __px_prefix px->
#endif /* ultrix */

#define px__stamp_width 	__px_prefix stamp_width
#define px__stamp_height 	__px_prefix stamp_height 
#define px__n10_present 	__px_prefix n10_present 
#define px__zplanes 		__px_prefix zplanes 
#define px__xplanes 		__px_prefix xplanes 
#define px__rb_size 		__px_prefix rb_size 
#define px__rb_phys 		__px_prefix rb_phys
#define px__rb_addr 		__px_prefix rb_addr 
#define px__ib_size 		__px_prefix ib_size 
#define px__idle_count 		__px_prefix idle_count 
#define px__idle_sample 	__px_prefix idle_sample 
#define px__dev			__px_prefix dev

#define PX_CPU_IDLE	3

#define PX_DWN_(X)	(((int)(X)) & ~(CLBYTES-1))
#define PX_RND_(X)	PX_DWN_(((int)(X)) + CLBYTES-1)

/*
 * Ioctl header.
 */
typedef struct {
    short         screen;
    short         cmd;
    unsigned long data;
} px_ioc;

#define PX_IOC_MAP	_IOWR('w', (0|IOC_S), px_ioc)
#define PX_MAP_OPTION	4


#ifdef KERNEL
#      include <io/dec/ws/pa.h>		/* 2DA driver */
#      include <io/dec/ws/pq.h>		/* 3DA driver */
#endif /* KERNEL */
#endif /* _PXDEFS_H_ */

