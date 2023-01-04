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
 *	@(#)$RCSfile: dpcdefs.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:40:28 $
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
 * Copyright (c) 1989 Encore Computer Corporation
 */

#ifndef _DPCDEFS_H_
#define _DPCDEFS_H_
#include <mmax_dpc.h>

#if	MMAX_DPC

/*
 * dpcdefs.h
 *
 *	DPC register definitions.
 */

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

#define DPCREG_BASE	0xfffffe00
#define DPCREG_TOP	0xffffffff

#define DPCROM_BASE	0xffff0000
#define DPCROM_TOP	0xffffffff


/*
 * DPC vector register. When read it delivers the current pending interrupt
 * vector number (FIFO or TSE) without clocking the fifo.
 */
#define DPCREG_VECTOR	0xfffffe00	/* Private, byte read */


/*
 * DPC hydrabus interface register
 */
#ifndef	LOCORE
typedef	union dpchbusdata {
	struct {
		unsigned int	d_lowdata;			/* High */
		unsigned int	d_highdata;			/* High */
	} f;
	long l;
} dpchbusdata_t;
#endif	LOCORE

#define DPCREG_HBUSDATA	0xfffffe18	/* Shared, double long read */


/*
 * DPC priority register.
 */
#ifndef	LOCORE
typedef union dpcprior {
	struct {
		unsigned int	p_pri:5,			/* High */
				p_class:2,			/* High */
				:1;
	} f;
	char	c;
} dpcprior_t;
#endif	LOCORE

#define DPCPRIOR_FIX	0x00

#define DPCPRIOR_PRI		0x1f
#define DPCPRIOR_CLASS		0x60

#define DPCREG_PRIOR	0xfffffe22	/* Private, byte read/write */


/*
 * DPC send vector register.
 */
#ifndef	LOCORE
typedef	union dpcsendvec {
	struct {
		unsigned int	v_class:2,			/* High */
				v_device:2,			/* High */
				v_slot:4,			/* High */
				v_lamplo:2,			/* High */
				v_lampreqlo:1,			/* High */
				:5,				/* reserved */
				v_vector:8,			/* High */
				v_lamphi:2,			/* High */
				v_lampreqhi:1,			/* High */
				:5;				/* reserved */
	} f;
	long l;
} dpcsendvec_t;

typedef	dpcsendvec_t	board_vbxmit_t;
#endif	LOCORE

#define DPCSENDVEC_FIX	0x00000000

#define DPCSENDVEC_CLASS	0x00000003
#define DPCSENDVEC_DEVICE	0x0000000c
#define DPCSENDVEC_SLOT		0x000000f0
#define DPCSENDVEC_LAMPLO	0x00000300
#define DPCSENDVEC_LAMPREQLO	0x00000400
#define DPCSENDVEC_VECTOR	0x00ff0000
#define DPCSENDVEC_LAMPHI	0x03000000
#define DPCSENDVEC_LAMPREQHI	0x04000000

#define DPCREG_SENDVEC	0xfffffe24	/* Private, long write */


/*
 * DPC fifo register. When read, the next vector is delivered and the fifo
 * is clocked.
 */
#define DPCREG_FIFO	0xfffffe28
						/* Private, byte read */
#define DPC_FIFOSIZE 17


/*
 * DPC sbx registers (serial lines 0/1)
 */
#ifndef	LOCORE
typedef union dpcsbxctl {
	struct	{
		unsigned int	c_rxrdy:1,		/* High */
				c_zerocount:1,		/* High */
				c_txrdy:1,		/* High */
				c_dcd:1,		/* High */
				c_sync:1,		/* High */
				c_cts:1,		/* High */
				c_txunr:1,		/* High */
				c_break:1;		/* High */
	} f;
	char c;
} dpcsbxctl_t;
#endif	LOCORE

#define DPCSBXCTL_RXRDY		0x01
#define DPCSBXCTL_RXRDY_BIT	0x00
#define DPCSBXCTL_ZEROCOUNT	0x02
#define DPCSBXCTL_TXRDY		0x04
#define DPCSBXCTL_TXRDY_BIT	0x02
#define DPCSBXCTL_DCD		0x08
#define DPCSBXCTL_SYNC		0x10
#define DPCSBXCTL_CTS		0x20
#define DPCSBXCTL_TXUNR		0x40
#define DPCSBXCTL_BREAK		0x80

#define DPCREG_SBXCTL0		0x0fffffec1 /* Shared, byte read/write*/
#define DPCREG_SBXDATA0		0x0fffffec5 /* Shared, byte read/write*/
#define DPCREG_SBXCTL1		0x0fffffec9 /* Shared, byte read/write*/
#define DPCREG_SBXDATA1		0x0fffffecd /* Shared, byte read/write*/


/*
 * DPC board history register
 */
#ifndef	LOCORE
typedef	union dpcbrdhist {
	struct {
		unsigned short	b_unused;		/* Unused */
		char		b_id;			/* Id byte */
		char		b_rev;			/* Revision byte */
	} f;
	long l;
} dpcbrdhist_t;

typedef struct dpcidprom {
	dpcbrdhist_t	b_boardhist[32];	/* Board history array */
} dpcidprom_t;
#endif	LOCORE

#define DPCREG_BRDHIST	0xffffff00	/* Shared, long read */


/*
 * DPC time slice end registers.
 */
#ifndef	LOCORE
typedef	union dpctsevec {
	struct {
		unsigned int	v_readvec:8,			/* High */
				:16,				/* reserved */
				v_writevec:8;			/* High */
	} f;
	long l;
} dpctsevec_t;
#endif	LOCORE

#define DPCTSEVEC_READVEC	0x000000ff
#define DPCTSEVEC_WRITEVEC	0xff000000

#define DPCREG_TSEVECREAD	0xfffffe2c
						 /* Private, BYTE read */
#define DPCREG_TSEVECWRITE	0xfffffe2f
						 /* Private, BYTE write */


/* 
 * DPC time slice end counters
 */
#ifndef	LOCORE
typedef	union	dpctsectl {
	struct {
		unsigned int	c_bcd:1,			/* High */
				c_mode:3,			/* High */
				c_rdwr:2,			/* High */
				c_select:2;			/* High */
	} f;
	char c;
} dpctsectl_t;
#endif	LOCORE

#define DPCTSECTRL_BCD		0x01
#define DPCTSECTRL_MODE		0x0e
#define DPCTSECTRL_RDWR		0x30
#define DPCTSECTRL_SELECT	0xc0

#define DPCREG_TSECNT0	0xfffffe31
					/* Private, byte read/write */
#define DPCREG_TSECNT1	0xfffffe35
					/* Private, byte read/write */
#define DPCREG_TSECNT2	0xfffffe39
					/* Private, byte read/write */
#define DPCREG_TSECTL	0xfffffe3d
					/* Private, byte write */


/*
 * DPC diagnostic parity register.
 */
#ifndef	LOCORE
typedef union dpcdiagpar {
	struct {
		unsigned int	d_wrong_addr_par:4,		/* High */
				d_wrong_data_par:2,		/* High */
				:2;				/* reserved */
	} f;
	char c;
} dpcdiagpar_t;
#endif	LOCORE

#define DPCDIAGPAR_WRONG_ADDR_PAR	0x000f
#define DPCDIAGPAR_WRONG_DATA_PAR	0x0030

#define DPCREG_DIAGPAR	0xfffffe10	/* Shared, byte write */

/*
 * DPC control register.
 */
#ifndef	LOCORE
typedef	union	dpcctl {
	struct	{
		unsigned int	:8,				/* reserved */
				c_block_a_req:1,		/* High */
				c_block_b_req:1,		/* Low */
				:2,				/* reserved */
				c_nmi_disable:1,		/* Low */
				c_etlb_off:1,			/* Low */
				:7,				/* reserved */
				c_gen_sysnmi:1,			/* Low */
				c_flush_req:1,			/* High */
				c_bus_soft_err_dis:1,		/* Low */
				c_csr_force_miss:1,		/* Low */
				c_switch_nmi_a:1;		/* Low */
	} f;
	long	l;
} dpcctl_t;
#endif	LOCORE

#define DPCCTL_FIX	0x03a03200

#define DPCCTL_BLOCK_A_REQ	0x00000100
#define DPCCTL_BLOCK_B_REQ	0x00000200
#define DPCCTL_NMI_DISABLE	0x00001000
#define DPCCTL_ETLB_OFF		0x00002000
#define DPCCTL_GEN_SYSNMI	0x00200000
#define DPCCTL_FLUSH_REQ	0x00400000
#define DPCCTL_BUS_SOFT_ERR_DIS	0x00800000
#define DPCCTL_CSR_FORCE_MISS	0x01000000
#define DPCCTL_SWITCH_NMI_A	0x02000000

#define DPCREG_CTL		0xfffffe48


/*
 * DPC Status Registers
 */
#ifndef	LOCORE
typedef	union	dpcsts	{
	struct	{
		unsigned int	s_cpuid:1,			/* High */
				:1,				/* reserved */
				s_slotid:4,			/* High */
				:2,				/* reserved */
				s_block_a_req:1,		/* High */
				s_block_b_req:1,		/* Low */
				:2,				/* reserved */
				s_nmi_disable:1,		/* Low */
				s_etlb_off:1,			/* Low */
				:13,				/* reserved */
				s_vecbus_txreq:1,		/* High */
				s_output_ready:1,		/* High */
				s_bus_unfreeze:1,		/* Low */
				s_isbx_present:1,		/* Low */
				s_fpa_present:1;		/* Low */
	} f;
	long	l;
} dpcsts_t;
#endif	LOCORE

#define DPCSTS_FIX	0xe0003200

#define DPCSTS_CPUID		0x00000001
#define DPCSTS_SLOTID		0x0000003c
#define DPCSTS_BLOCK_A_REQ	0x00000100
#define DPCSTS_BLOCK_B_REQ	0x00000200
#define DPCSTS_NMI_DISABLE	0x00001000
#define DPCSTS_ETLB_OFF		0x00002000
#define DPCSTS_VECBUS_TXREQ	0x08000000
#define DPCSTS_OUTPUT_READY	0x10000000
#define DPCSTS_BUS_UNFREEZE	0x20000000
#define DPCSTS_ISBX_PRESENT	0x40000000
#define DPCSTS_FPA_PRESENT	0x80000000

#define DPCREG_STS		0xfffffffc

#define FIFO_NOT_EMPTY	((*(long *)DPCREG_STS) & DPCSTS_OUTPUT_READY)
#define VB_BUSY ((*((long *)DPCREG_STS) ^ DPCSTS_FIX) & DPCSTS_VECBUS_TXREQ)

#define GETDPCSLOT	((*((long *)DPCREG_STS) & DPCSTS_SLOTID) >> 2)
#define GETDPCDEV	(*((long *)DPCREG_STS) & DPCSTS_CPUID)


/*
 * NMI Status Register
 */
#ifndef	LOCORE
typedef	union	dpcnmi {
	struct	{
		unsigned int	n_data_cache_parity:1,		/* High */
				n_ctag_parity:1,		/* High */
				n_vbit_even_bank_parity:1,	/* High */
				n_vbit_odd_bank_parity:1,	/* High */
				:1,				/* reserved */
				n_bus_error:3,			/* Low */
				:1,				/* reserved */
				n_vecbus_vec_not_taken:1,	/* High */
				n_vecbus_parity:1,		/* High */
				n_received_data_parity:1,	/* High */
				n_etlb_parity:1,		/* High */
				:1,				/* reserved */
				n_external_switch:1,		/* High */
				n_system_nmi:1,			/* High */
				n_bus_powerfail:1,		/* High */
				n_btag_even_bank_parity:1,	/* High */
				n_btag_odd_bank_parity:1,	/* High */
				n_received_address_parity:1,	/* High */
				n_vecbus_out_of_synch:1;	/* High */
	} f;
	long	l;
} dpcnmi_t;
#endif	LOCORE

#define DPCNMI_FIX	0x000000e0
#define DPCNMI_MASK	0x001fdeef
#define DPCNMI_HARDERR	0x00080080

#define DPCNMI_DATA_CACHE_PARITY	0x00000001
#define DPCNMI_CTAG_PARITY		0x00000002
#define DPCNMI_VBIT_EVEN_BANK_PARITY	0x00000004
#define DPCNMI_VBIT_ODD_BANK_PARITY	0x00000008
#define DPCNMI_BUS_ERROR		0x000000e0
#define DPCNMI_VECBUS_VEC_NOT_TAKEN	0x00000200
#define DPCNMI_VECBUS_PARITY		0x00000400
#define DPCNMI_RECEIVED_DATA_PARITY	0x00000800
#define DPCNMI_ETLB_PARITY		0x00001000
#define DPCNMI_EXTERNAL_SWITCH		0x00004000
#define DPCNMI_SYSTEM_NMI		0x00008000
#define DPCNMI_BUS_POWERFAIL		0x00010000
#define DPCNMI_BTAG_EVEN_BANK_PARITY	0x00020000
#define DPCNMI_BTAG_ODD_BANK_PARITY	0x00040000
#define DPCNMI_RECEIVED_ADDRESS_PARITY	0x00080000
#define DPCNMI_VECBUS_OUT_OF_SYNCH	0x00100000

#define DPCNMI_PROTO_ERR		7
#define DPCNMI_WRTPAR_ERR		6
#define DPCNMI_ADRPAR_ERR		5
#define DPCNMI_BUSTMO_ERR		4

#define DPCREG_NMI	0xfffffe08


/*
 * DPC diagnostic control register
 */
#ifndef	LOCORE
typedef	union	dpcdiagctl {
	struct	{
		unsigned int	dc_rom_overlay:1,		/* Low */
				dc_force_hbus_disable:1,	/* Low */
				dc_force_hbus_not_busy:1,	/* Low */
				dc_force_not_mysel:1,		/* Low */
				dc_force_vbit_cmp:1,		/* Low */
				dc_vbit_flush_level:1,		/* High */
				dc_vecbus_receive_enable:1,	/* Low */
				dc_vecbus_loopback:1,		/* High */
				dc_force_vecbus_grant:1,	/* Low */
				dc_force_wrong_data_parity:1,	/* High */
				dc_force_wrong_vbit_parity:1,	/* Low */
				dc_force_wrong_etlb_parity:1;	/* Low */
	} f;
	long	l;
} dpcdiagctl_t;
#endif	LOCORE

#define DPCDIAGCTL_FIX	0x00000d5f

#define DPCDIAGCTL_ROM_OVERLAY			0x00000001
#define DPCDIAGCTL_FORCE_HBUS_DISABLE		0x00000002
#define DPCDIAGCTL_FORCE_HBUS_NOT_BUSY		0x00000004
#define DPCDIAGCTL_FORCE_NOT_MYSEL		0x00000008
#define DPCDIAGCTL_FORCE_VBIT_CMP		0x00000010
#define DPCDIAGCTL_VBIT_FLUSH_LEVEL		0x00000020
#define DPCDIAGCTL_VECBUS_RECEIVE_ENABLE	0x00000040
#define DPCDIAGCTL_VECBUS_LOOPBACK		0x00000080
#define DPCDIAGCTL_FORCE_VECBUS_GRANT		0x00000100
#define DPCDIAGCTL_FORCE_WRONG_DATA_PARITY	0x00000200
#define DPCDIAGCTL_FORCE_WRONG_VBIT_PARITY	0x00000400
#define DPCDIAGCTL_FORCE_WRONG_ETLB_PARITY	0x00000800

#define DPCREG_DIAGCTL	0xfffffe50


/*
 * Diagnostic Status Registers
 */
#ifndef	LOCORE
typedef	union	dpcdiagsts {
	struct	{
		unsigned int	ds_rom_overlay:1,		/* Low */
				ds_force_hbus_disable:1,	/* Low */
				ds_force_hbus_not_busy:1,	/* Low */
				:9,				/* reserved */
				ds_board_in_test_rack:1,	/* Low */
				ds_diagnostic_mode:1,		/* Low */
				ds_pending_bit:1,		/* High */
				ds_hit:1,			/* Low */
				ds_vbit_even_bank:1,		/* High */
				ds_vbit_odd_bank:1,		/* High */
				ds_ctag_compare:1,		/* High */
				:13;				/* reserved */
	} f;
	long	l;
} dpcdiagsts_t;
#endif	LOCORE

#define DPCDIAGSTS_FIX	0x0000b007

#define DPCDIAGSTS_ROM_OVERLAY			0x00000001
#define DPCDIAGSTS_FORCE_HBUS_DISABLE		0x00000002
#define DPCDIAGSTS_FORCE_HBUS_NOT_BUSY		0x00000004
#define DPCDIAGSTS_BOARD_IN_TEST_RACK		0x00001000
#define DPCDIAGSTS_DIAGNOSTIC_MODE		0x00002000
#define DPCDIAGSTS_PENDING_BIT			0x00004000
#define DPCDIAGSTS_HIT				0x00008000
#define DPCDIAGSTS_VBIT_EVEN_BANK		0x00010000
#define DPCDIAGSTS_VBIT_ODD_BANK		0x00020000
#define DPCDIAGSTS_CTAG_COMPARE			0x00040000

#define DPCREG_DIAGSTS	0xfffffe4c

#define DPCLED_LED	    0x000000ff		/* Low.	0=on. Shared	*/
#define DPCREG_LED	    0xfffffe07
#define LED_REG		    DPCREG_LED
#define LED_LED		    DPCLED_LED
#if	xxx
#define LED(X)		    *(LED_REG) = (LED_LED ^ (X))
#else	xxx
#define LED(X)
#endif	xxx

/*
 * Macro that can be used to obtain the CPU number directly from the
 * hardware.
 */

/* Original version --
 *
 * #define getcpuid()
 *     ((*(long *)DPCREG_STS ^ DPCSTS_FIX) & (DPCSTS_CPUID | DPCSTS_SLOTID))
 *
 * Optimized version of getcpuid() --
 *
 *	DPCSTS_PROCID = DPCSTS_CPUID | DPCSTS_SLOTID
 */
#define DPCSTS_PROCID		0x3d

#define getcpuid() ((*(long *)DPCREG_STS) & DPCSTS_PROCID)

#endif	MMAX_DPC
#endif	_DPCDEFS_H_
