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
 *	@(#)$RCSfile: xpcdefs.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:43:50 $
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
 * Copyright (c) 1989 Encore Computer Corporation.
 */

#ifndef _XPCDEFS_H_
#define	_XPCDEFS_H_

#ifdef	KERNEL
#include <mmax_xpc.h>
#endif	KERNEL

#if	MMAX_XPC
#ifdef	KERNEL
#ifndef	LOCORE
#include <kern/lock.h>			/* for vbga lock array, sigh */
#endif	LOCORE
#endif	KERNEL

/*
 * xpcdefs.h
 *
 *	XPC register definitions.
 */

#ifndef	LOCORE
typedef unsigned short	U_SHORT;
#endif	LOCORE

/*
 *			NOTE
 *
 * This include file allows 2 styles of coding.
 *
 *	o Variables can be defined using the typedefs contained herein, and
 *	   individual fields can be manipulated. It is recommended, however,
 *	   that the actual hardware registers be read/written using longs,
 *	   shorts (words), or chars (bytes).
 *	o Hardware registers can be accessed directly using the mask defini-
 *	   tions.
 */

#define XPCREG_BASE	0xfffff000
#define XPCREG_TOP	0xffffffff

#define XPCRAM_BASE	0xffffe000	/* Each CPU has 4K at this address. */
#define XPCRAM_TOP	0xfffff000	/* It maps to ffffc000 and ffffd000.*/

#define XPCSHARED_RAM	0xffffc000	/* The 8K described above where the */
#define XPCSHARED_RAM_TOP 0xffffe000	/* low half belongs to CPU A.	    */

#define XPCROM_BASE	0xfff00000
#define XPCROM_TOP	0xfff40000

/*
 *	XPC Control and Status Register
 *	8, 16, or 32-bit read; 32-bit write
 */

#ifndef	LOCORE
typedef union xpccsr {
    struct {
	unsigned int
	    s_cpuid:1,		/* High	    */
	    s_cpu_b_master:1,	/* High	    */
	    s_slotid:4,		/* High	    */
	    s_cache_bypass:1,	/* High	    */
	    s_burst_enable:1,	/* High	    */
	    s_tty:1,		/* Low	    */
	    s_fpa:1,		/* Low	    */
	    s_gen_sysnmi:1,	/* High	    */
	    :21;		/* Reserved */
    } f;
    long l;
} xpccsr_t;
#endif	LOCORE

#define XPCCSR_CPUID		0x00000001		/* High		*/
#define XPCCSR_CPUID_BIT		 0
#define XPCCSR_SLOTID		0x0000003c		/* High		*/
#define XPCCSR_CACHE_BYPASS	0x00000040		/* High		*/
#define XPCCSR_CACHE_BYPASS_BIT	 6
#define XPCCSR_BURST_ENABLE	0x00000080		/* High		*/
#define XPCCSR_BURST_ENABLE_BIT	 7
#define XPCCSR_TTY_PRES		0x00000100		/* Low		*/
#define XPCCSR_NO_TTY_PRES	XPCCSR_TTY_PRES	/* High alias	*/
#define XPCCSR_TTY_PRES_BIT		 8		/* bit position	*/
#define XPCCSR_FPA_PRES		0x00000200		/* Low		*/
#define XPCCSR_32381_PRES	XPCCSR_FPA_PRES	/* High alias	*/
#define XPCCSR_FPA_PRES_BIT		 9		/* bit position	*/
#define XPCCSR_32381_PRES_BIT	XPCCSR_FPA_PRES_BIT	/* High alias	*/
#define XPCCSR_NO_SYSNMI	0x00000400
#define XPCCSR_GEN_SYSNMI	0xfffffbff
#define	XPCCSR_SYSNMI_BIT	10
#define XPCCSR_NORMAL_BITS	(XPCCSR_BURST_ENABLE | XPCCSR_NO_SYSNMI)
#ifndef	LOCORE
#define XPCREG_CSR		(long *)0xffffff1c	/* Shared.	*/
#endif	LOCORE
#define XPCREG_NMI		XPCREG_CSR
#define XPCNMI_VALUE		(*XPCREG_NMI & XPCCSR_GEN_SYSNMI)

#define XPCCSR_WRITE_MASK	0x000000c0		/* Writeable bits */

#define GETCPUSLOT  ((*XPCREG_CSR & XPCCSR_SLOTID) >> 2)
#define GETCPUDEV   (*XPCREG_CSR & XPCCSR_CPUID)
#ifndef	LOCORE
#define GETCPUID    (*XPCREG_CSR & (XPCCSR_CPUID | XPCCSR_SLOTID))
#endif	LOCORE

/*
 *	XPC Error Status Register
 */

#ifndef	LOCORE
typedef union xpcerr {
    struct {
	unsigned int
	    e_cpuerr:1,		/* cpu error	High (Low means btag err) */
	    e_lock:1,		/* interlock	Low	*/
	    e_ctagcmp:2,	/* ctag compare	Low	*/
	    e_ctagpe:2,		/* ctag parity	Low	*/
	    e_cache_state:4,
	    e_cache_state_pe:1,
	    e_btagcmp:2,
	    e_btagpe:1,
	    :1,
	    e_btag_state:4,
	    e_btag_state_pe:1,
	    e_stall:1,		/* stalled by btag Low	*/
	    e_cache_addr:2,	/* cache addr		*/
	    e_datape:1,		/* data parity	Low	*/
	    e_rdbus:1,		/* bus read	Low	*/
	    e_sft_err_stat:2,	/* 00 Protocol/Logic error
				 * 01 Write Data Parity error
				 * 10 Retry
				 * 11 No error
				 */
	    e_hrd_err_stat:2,	/* 00 Protocol/Logic error w/timeout
				 * 01 WDPE/No Read Data back w/timeout
				 * 10 Not Accepted/STALL w/timeout
				 * 11 No Grant w/timeout
				 */
	    :3;			/* undefined		*/
    } f;
    long l;
} xpcerr_t;
#endif	LOCORE

#define XPCERR_CPU			0x00000001
#define XPCERR_LOCK			0x00000002
#define XPCERR_CTAGCMP			0x0000000c
#define XPCERR_CTAG_CMP_SHFT		2
#define XPCERR_CTAGCMP0		0x00000004
#define XPCERR_CTAGCMP1		0x00000008
#define XPCERR_CTAGPE			0x00000030
#define XPCERR_CTAGPE0			0x00000010
#define XPCERR_CTAGPE1			0x00000020
#define XPCERR_CACHE_STATE		0x000003c0
#define XPCERR_CACHE_STATE_SHFT		6
#define XPCERR_CACHE_STATE0		0x00000040
#define XPCERR_CACHE_STATE1		0x00000080
#define XPCERR_CACHE_STATE2		0x00000100
#define XPCERR_CACHE_STATE3		0x00000200
#define XPCERR_CACHE_STATE_PE		0x00000400
#define XPCERR_BTAGCMP			0x00001800
#define XPCERR_BTAG_CMP_SHFT		11
#define XPCERR_BTAGCMP0			0x00000800
#define XPCERR_BTAGCMP1			0x00001000
#define XPCERR_BTAGPE			0x00002000
#define XPCERR_BTAG_STATE		0x00078000
#define XPCERR_BTAG_STATE_SHFT		15
#define XPCERR_BTAG_STATE0		0x00008000
#define XPCERR_BTAG_STATE1		0x00010000
#define XPCERR_BTAG_STATE2		0x00020000
#define XPCERR_BTAG_STATE3		0x00040000
#define XPCERR_BTAG_STATE_PE		0x00080000
#define XPCERR_STALL			0x00100000
#define XPCERR_CACHE_ADDR		0x00600000
#define XPCERR_CACHE_ADDR2		0x00200000
#define XPCERR_CACHE_ADDR3		0x00400000
#define XPCERR_DATAPE			0x00800000
#define XPCERR_BUSERR			0x01000000
#define XPCERR_SFT_ERR_STAT		0x06000000
#define XPCERR_SFT_ERR_SHFT		25
#define XPCERR_SFT_PROTOCOL		0x00000000
#define XPCERR_SFT_WDPE			0x02000000
#define XPCERR_SFT_RETRY		0x04000000
#define XPCERR_SFT_NO_ERR		0x06000000
#define XPCERR_HRD_ERR_STAT		0x18000000
#define XPCERR_HRD_ERR_SHFT		27
#define XPCERR_HRD_PROTOCOL		0x00000000
#define XPCERR_HRD_WDPE			0x08000000
#define XPCERR_HRD_NOT_ACCEPT		0x10000000
#define XPCERR_HRD_NO_GRANT		0x18000000

#define XPCREG_ERR_MASK			0x1fffbfff  /* Mask off unused bits */
#ifndef	LOCORE
#define XPCREG_ERR			(long *)0xffffff18
#endif	LOCORE

#define GETCPUERR			(*XPCREG_ERR & XPCREG_ERR_MASK)

/*
 *  XPC DUART Registers (Signetics SCN2681-24)
 *  Note:  Control register and bit definitions are found
 *  in duart.h.
 */

#define XPCDUART_AMODE		(char *)0xfffff000
#define XPCDUART_ASTATUS	(char *)0xfffff004
#define XPCDUART_ACLKSEL	(char *)0xfffff004
#define XPCDUART_ACMD		(char *)0xfffff008
#define XPCDUART_ARCVDATA	(char *)0xfffff00c
#define XPCDUART_AXMTDATA	(char *)0xfffff00c
#define XPCDUART_AUXCTL		(char *)0xfffff010
#define XPCDUART_INTRSTAT	(char *)0xfffff014
#define XPCDUART_INTRMASK	(char *)0xfffff014
#ifndef	LOCORE
#define XPCDUART_BMODE		(char *)0xfffff020
#define XPCDUART_BSTATUS	(char *)0xfffff024
#define XPCDUART_BCLKSEL	(char *)0xfffff024
#define XPCDUART_BCMD		(char *)0xfffff028
#define XPCDUART_BRCVDATA	(char *)0xfffff02c
#endif	LOCORE
#define XPCDUART_BXMTDATA	(char *)0xfffff02c

#ifndef	LOCORE
#define XPCDUART_ADDR		((duart_ctl_t *)XPCDUART_AMODE)
#endif	LOCORE

/*
 * Register offsets are defined in "icu.h".
 */
#define XPCICU_MASTER       (char *)0xfffffe00

/*
 *	XPC Board ID PROM Registers
 */
#ifndef	LOCORE
typedef	union xpcbrdhist {
    struct {
	char
	    b_id,			/* ID byte	 */
	    b_rev;			/* Revision byte */
    } f;
    short s;
} xpc_brdhist_t;

typedef struct xpcidprom {
    xpc_brdhist_t  b_boardhist[32];	/* Board history array */
} xpc_idprom_t;
#endif	LOCORE

#ifndef	LOCORE
#define XPCREG_BRDHIST	(long *)0xffffff80	/* Shared */
#endif	LOCORE

/*
 *	XPC Diagnostic Control and Status Register
 */

#ifndef	LOCORE
typedef union xpcdiag {
    struct {
	unsigned int
	    dc_nbi_loop_addr:1,		/* Low	*/
	    dc_force_cache_hit:1,	/* High	*/
	    dc_force_btag_cmp:1,	/* High */
	    dc_fifo_reset:1,		/* Low	*/
	    dc_force_allocate:1,	/* High	*/
	    dc_diag_state:4,
	    dc_force_btag_state:1,	/* Low	*/
	    dc_force_nbus_disable:1,	/* Low	*/
	    dc_force_nbus_diag:1,	/* Low	*/
	    dc_force_not_cycle:1,	/* Low	*/
	    dc_rom_overlaya:1,		/* Low	*/
	    dc_rom_overlayb:1,		/* Low	*/
	    dc_par_err_disable:1,	/* Low	*/
	    dc_block_a_req:1,		/* Low	*/
	    dc_block_b_req:1,		/* Low	*/
	    dc_force_instr_cyc_a:1,	/* High	*/
	    dc_force_instr_cyc_b:1,	/* High	*/
	    dc_diag_mode_1:1,		/* Low	*/
	    dc_diag_mode_2:1,		/* Low	*/
	    dc_test_rack:1,		/* Low	*/
	    :9;
    } f;
    long l;
} xpcdiag_t;
#endif	LOCORE

#ifndef	LOCORE
#define XPCREG_CTL		(long *)0xffffff14
#endif	LOCORE
#define XPCREG_DIAG		XPCREG_CTL
#define XPCDIAG			XPCCTL

#define XPCCTL_NBI_LOOP_ADDR		0x00000001	/* Low	Shared	RW  */
#define XPCCTL_FRC_CACHE_HIT		0x00000002	/* Low	Shared	RW  */
#define XPCCTL_FRC_BTAG_CMP		0x00000004	/* High Shared	RW  */
#define XPCCTL_FIFO_RESET		0x00000008	/* Low	Shared	RW  */
#define XPCCTL_FRC_ALLOCATE		0x00000010	/* High	Shared	RW  */
#define XPCCTL_DIAG_STATE		0x000001e0	/* High	Shared	RW  */
#define XPCCTL_DIAG_STATE_SHFT		5
#define XPCCTL_FRC_BTAG_STATE		0x00000200	/* Low	Shared	RW  */
#define XPCCTL_FRC_NBI_DSBL		0x00000400	/* Low	Shared	RW  */
#define XPCCTL_FRC_NBI_DIAG		0x00000800	/* Low	Shared	RW  */
#define XPCCTL_FRC_NOT_MY_CYCLE		0x00001000	/* Low	Shared	RW  */
#define XPCCTL_ROM_OVERLAY_A		0x00002000	/* Low	Shared	RW  */
#define XPCCTL_ROM_OVERLAY_B		0x00004000	/* Low	Shared	RW  */
#define XPCCTL_PAR_ERR_DSBL		0x00008000	/* Low	Shared	RW  */
#define XPCCTL_BLOCK_A_REQ		0x00010000	/* Low	Shared	RW  */
#define XPCCTL_BLOCK_A_REQ_BIT		16
#define XPCCTL_BLOCK_B_REQ		0x00020000	/* Low	Shared	RW  */
#define XPCCTL_BLOCK_B_REQ_BIT		17
#define XPCCTL_FRC_INSTR_CYC_A		0x00040000	/* High		RW  */
#define XPCCTL_FRC_INSTR_CYC_B		0x00080000	/* High		RW  */
#define XPCCTL_DIAG_MODE_1		0x00100000	/* Low	Shared	RO  */
#define XPCCTL_DIAG_MODE_2		0x00200000	/* Low	Shared	RO  */
#define XPCCTL_TEST_RACK		0x00400000	/* Low	Shared	RO  */
#define XPCCTL_WRITE_MASK		0x000fffff	/* Writeable bits   */
#define XPCCTL_NORMAL_BITS		0x0003fe08	/* Normal operations*/

/*
 *	XPC Wrong Parity Control Register
 */

#ifndef	LOCORE
typedef union xpcwpar {
    struct {
	unsigned int
	    w_force_wrong_addr_par:4,	/* High	*/
	    w_force_wrong_ctag_par:2,	/* High	*/
	    w_force_wrong_btag_par:2,	/* High	*/
	    w_force_wrong_data_par:1,	/* High	*/
	    w_force_wrong_state_par:1,	/* High	*/
	    w_force_wrong_addid_par:1,	/* High	*/
	    w_force_wrong_byte_par:1,	/* High	*/
	    w_force_wrong_destsel_par:1,/* High	*/
	    :19;
    } f;
    long l;
} xpcwpar_t;
#endif	LOCORE

#define XPCPAR_FRC_ADDR_PAR		0x0000000f	/* High	Shared	*/
#define XPCPAR_FRC_CTAG_PAR		0x00000030	/* High	Shared	*/
#define XPCPAR_FRC_BTAG_PAR		0x000000c0	/* High	Shared	*/
#define XPCPAR_FRC_DATA_PAR		0x00000100	/* High	Shared	*/
#define XPCPAR_FRC_STATE_PAR		0x00000200	/* High	Shared	*/
#define XPCPAR_FRC_ADDID_PAR		0x00000400	/* High	Shared	*/
#define XPCPAR_FRC_BYTE_PAR		0x00000800	/* High	Shared	*/
#define XPCPAR_FRC_DESTSEL_PAR		0x00001000	/* High	Shared	*/

#ifndef	LOCORE
#define XPCREG_WPAR			(long *)0xffffff10
#endif	LOCORE

/*
 *	XPC DESTSEL Control Register
 */

#define XPCDEST_FRC_DIAG_DESTSEL	0x01000000
#define XPCDEST_DPMPRTN			0x02000000
#define XPCDEST_DESTSEL			0xfc000000

#ifndef	LOCORE
#define XPCREG_DESTSEL			(long *)0xffffff0c
#endif	LOCORE

/*
 *		XPC LED Control Register
 */

#ifndef	LOCORE
#define XPCREG_LED	(char *)0xffffff08
#define XPCLED_LED	0x000000ff		/* Low	0 = on.  Shared	*/
#define LED_REG		XPCREG_LED
#define LED_LED		XPCLED_LED
#define LED(X)		*(LED_REG) = (LED_LED ^ (X))
#endif	LOCORE

/*
 *	XPC NanoBus Interface Register
 */

#ifndef	LOCORE
typedef union xpcnbusdata {
    struct {
	unsigned int
	    d_lowdata,		/* High	*/
	    d_highdata;		/* High	*/
    } f;
    long l[2];			/* 0 = low, 1 = high	*/
} xpc_nbusdata_t;
#endif	LOCORE

#ifndef	LOCORE
#define XPCREG_NBUSDATAH	(long *)0xffffff04
#define XPCREG_NBUSDATAL	(long *)0xffffff00
#endif	LOCORE

/*
 *	XPC Vector Bus Registers
 */

/*
 * Base of the VBGA.
 */
#define XPCVB			0xfffff080
#define XPCVB_OFF		4

/*
 * VBGA Diagnostic/Control Register.
 */
#define XPCVB_DIAG_CONTROL	(U_SHORT *)(XPCVB + 0 * XPCVB_OFF)
#define XPCVB_DC_FRC_RCV_PAR    0x0002  /* High */
#define XPCVB_DC_FRC_LST_ARB    0x0004  /* High */
#define XPCVB_DC_ENB_DIAG_ARB   0x0008  /* High */
#define XPCVB_DC_ENB_EXT_DATA   0x0010  /* High, normally on */
#define XPCVB_DC_ENB_VB_LPBK    0x0020  /* High */
#define XPCVB_DC_SLOT_ID	0xf000  /* High */
#define XPCVB_DC_SLOT_ID_SHFT   12
#define XPCVB_DC_NORMAL	    	0x10
#define XPCVB_DC_VBOFF	    	0x0

/*
 * VBGA LAMP Receptor Register.	    (Present only on CPU A)
 */
#define XPCVB_LAMP_RECEPTOR	(U_SHORT *)(XPCVB + 1 * XPCVB_OFF)
#define	XPCVB_LAMPS_TO_ACCEPT   0xffff  /* High */

/*
 * VBGA Arbitration Diagnostic Register.
 */
#define XPCVB_ARBITRATION_DIAG  (U_SHORT *)(XPCVB + 2 * XPCVB_OFF)
#define	XPCVB_ARB_SLOT_ID	0x000f  /* ACTIVE LOW */
#define	XPCVB_ARB_RND_ROBIN	0x0010  /* High */
#define	XPCVB_ARB_FIFO_CNT	0x01e0  /* High */
#define	XPCVB_ARB_RND_ROBIN_ALL	0x0200  /* High */

/*
 * VBGA Last Arbitration Register.
 */
#define XPCVB_LAST_ARBITRATION  (U_SHORT *)(XPCVB + 3 * XPCVB_OFF)
#define	XPCVB_LST_SLOT_ID	0x000f  /* ACTIVE LOW */
#define	XPCVB_LST_RND_ROBIN	0x0010  /* High */
#define	XPCVB_LST_FIFO_CNT	0x01e0  /* High */
#define	XPCVB_LST_RND_ROBIN_ALL 0x0200  /* High */

/*
 * VBGA Class Register.
 */
#define XPCVB_CLASS		(U_SHORT *)(XPCVB + 4 * XPCVB_OFF)
#define	XPCVB_CLASS_ID		0x0003

/*
 * VBGA Transmit Registers.  Word 1 is the lower half of the xpcvbxmit
 * structure listed near the end of this section.  Word 2 is the upper half.
 */
#define XPCVB_XMIT_1		(U_SHORT *)(XPCVB + 5 * XPCVB_OFF)
#define XPCVB_XMIT_2		(U_SHORT *)(XPCVB + 6 * XPCVB_OFF)

/*
 * VBGA Receive Register.
 */
#define XPCVB_RCV		(U_SHORT *)(XPCVB + 7 * XPCVB_OFF)
#define	XPCVB_RCV_LAMP_SELECT	0x0400

#define XPCVB_RCV0_CLASS_ID	0x0003  /* This word is only present if */
#define XPCVB_RCV0_DEVICE_ID    0x000c  /* the LAMP Select bit is set.  */
#define XPCVB_RCV0_LAMP_ID	0x00f0  /* In this case, two reads from */
#define XPCVB_RCV0_SLOT_ID_1_0  0x0300  /* the register are necessary.  */

#define	XPCVB_RCV1_DATA		0x00ff  /* 2nd word if LAMP Vector.	    */
#define	XPCVB_RCV1_SLOT_ID_3_2  0x0300  /* Valid only if LAMP Vector    */

/*
 * VBGA Status Register.
 */
#ifndef	LOCORE
#define XPCVB_STATUS		(U_SHORT *)(XPCVB + 8 * XPCVB_OFF)
#endif	LOCORE
#define	XPCVB_STAT_FIFO_CNT	0x000f
#define	XPCVB_STAT_FIFO_NOT_EMP 0x0010
#define	XPCVB_STAT_FIFO_BIT     4
#define	XPCVB_STAT_RND_ROBIN    0x0020
#define	XPCVB_STAT_VECT_ACK	0x0040
#define XPCVB_ACK_BIT		6
#define XPCVB_STAT_VB_BUSY	0x0080
#define XPCVB_BUSY_BIT		7

#define FIFO_NOT_EMPTY		(*XPCVB_STATUS & XPCVB_STAT_FIFO_NOT_EMP)
#define VB_BUSY			(*XPCVB_STATUS & XPCVB_STAT_VB_BUSY)

/*
 * VBGA Error Register.
 */
#ifndef	LOCORE
typedef union xpcvberr {
	struct {
		unsigned int
		ve_badlamp:1,		/* High */
		ve_fifofull:1,		/* High */
		ve_par:1,		/* High */
		ve_sync:1,		/* High */
		:12,			/* Not used */
		:16;			/* Non-existent */
	} f;
	long l;
} xpcvberr_t;
#endif	LOCORE

#define XPCVB_ERROR		(U_SHORT *)(XPCVB + 9 * XPCVB_OFF)
#define XPCVB_ERR_ILL_LMP_VCT   0x0001
#define	XPCVB_ERR_FIFO_OVERFLOW 0x0002
#define	XPCVB_ERR_PARITY_ERR    0x0004
#define	XPCVB_ERR_OUT_OF_SYNC   0x0008

/*
 * XPC Vector Bus Transmit Register
 */

#ifndef	LOCORE
typedef union xpcvbxmit {
    struct {
	unsigned int
	    v_class:2,			/* High */
	    v_device:2,			/* High */
	    v_slot:4,			/* High */
	    v_lamplo:2,			/* High */
	    v_lampreqlo:1,		/* High */
	    :5,				/* Reserved */
	    v_vector:8,			/* High */
	    v_lamphi:2,			/* High */
	    v_lampreqhi:1,		/* High */
	    :5;				/* Reserved */
    } f;
    long l;
} xpcvbxmit_t;

typedef xpcvbxmit_t	board_vbxmit_t;
#endif	LOCORE

#define XPCVBXMIT_CLASS		0x00000003  /* 0 == direct, 1-3 == classed. */
#define XPCVBXMIT_DEVICE	0x0000000c  /* 0 == CPU A, 1 == CPU B.	    */
#define XPCVBXMIT_SLOT		0x000000f0  /* Directed vector to this slot.*/
#define XPCVBXMIT_LAMPLO	0x00000300  /* External lamp bits 0:1.	    */
#define XPCVBXMIT_LAMPREQLO	0x00000400  /* 1 == Direct to external lamp.*/
#define XPCVBXMIT_VECTOR	0x00ff0000  /* The 8-bit vector to be sent. */
#define XPCVBXMIT_LAMPHI	0x03000000  /* External lamp bits 2:3.	    */
#define XPCVBXMIT_LAMPREQHI	0x04000000  /* 1 == Direct to external lamp.*/

#ifndef	LOCORE
#define XPCREG_VBXMIT   	(long *)XPCVB_XMIT_1
#endif	LOCORE
#define VBXMIT_REG		XPCREG_VBXMIT
#ifndef	LOCORE
#define XPCREG_VBFIFO		(char *)XPCVB_RCV
#define VB_FIFO			XPCREG_VBFIFO
#endif	LOCORE

#define XPCVBFIFOSIZE		17
#define VB_FIFO_SIZE		XPCVBFIFOSIZE
#define XPCVBFIFOMASK		0xff		/* <7:0> current ID	*/

#ifdef	KERNEL
/*
 *	VBGA lock array (due to VBGA oversight)
 *	Filler avoids cache interference by padding to cache line size.
 */

#ifndef	LOCORE
typedef struct {
	simple_lock_data_t	lock;
	long			filler[3];
} vb_xmit_lock_t;

extern vb_xmit_lock_t vb_xmit_lock[];
#endif	LOCORE
#endif	KERNEL

#ifndef	LOCORE
/*
 * Macro that can be used to obtain the CPU number directly from the
 * hardware.
 */

/* Original version --
 *
 * #define getcpuid() (*XPCREG_CSR & (XPCCSR_CPUID | XPCCSR_SLOTID))
 *
 * Optimized version of getcpuid() --
 *
 *       Non-relevent bits of low order byte guaranteed to be zero
 */

#ifdef	notyet
#define	getcpuid()		((*((unsigned char *)0xffffff1c)))
#endif	notyet
#define	getcpuid()		(GETCPUID)
#endif	LOCORE

#endif	MMAX_XPC
#endif	_XPCDEFS_H_
