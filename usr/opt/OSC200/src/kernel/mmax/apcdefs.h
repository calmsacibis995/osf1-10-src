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
 *	@(#)$RCSfile: apcdefs.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:39:32 $
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
 * Copyright (c) 1989 Encore Computer Corporation
 */

#ifndef _APCDEFS_H_
#define	_APCDEFS_H_
#include "mmax_apc.h"

#if	MMAX_APC

/*
 * apcdefs.h
 *
 *	APC register definitions.
 */

#define APCREG_BASE     0xfffff000
#define APCREG_TOP      0xffffffff

#define APCRAM_BASE     0xfff90000
#define APCRAM_TOP      0xfffa0000

#define APCROM_BASE     0xfffa0000
#define APCROM_TOP      0xfffb0000

/*
 *      APC Control and Status Register
 */

#ifndef	LOCORE
typedef union apccsr {
    struct {
        unsigned int
            s_cpuid:2,          /* High     */
            s_slotid:4,         /* High     */
            :2,                 /* reserved */
            s_cache_bypass:1,   /* Low      */
            s_burst_enable:1,   /* Low      */
            s_vbclass:2,        /* High     */
            s_vbbusy:1,         /* Low      */
            s_vbfifo:1,         /* High     */
            s_tty:1,            /* Low      */
            s_fpa:1,            /* Low      */
            :16;                /* reserved */
    } f;
    long l;
} apccsr_t;
#endif	LOCORE

#define APCCSR_CPUID            0x00000003              /* High         */
#define APCCSR_SLOTID           0x0000003c              /* High         */
#define APCCSR_CACHE_BYPASS     0x00000100              /* High         */
#define APCCSR_BURST_ENABLE     0x00000200              /* Low          */
#define APCCSR_BURST_DISABLE    APCCSR_BURST_ENABLE     /* high alias   */
#define APCCSR_VECBUS_CLASS     0x00000c00              /* High         */
#define APCCSR_WRITE_MASK       0x00000f00              /* Writeable bits.  */
#define APCCSR_VECBUS_BUSY      0x00001000              /* Low          */
#define APCCSR_VB_BUSY_BIT      12
#define APCCSR_VECBUS_FREE      APCCSR_VECBUS_BUSY      /* high alias   */
#define APCCSR_VECBUS_FIFO      0x00002000              /* High= !empty */
#define APCCSR_VB_FIFO_BIT      13
#define APCCSR_TTY_PRES         0x00004000              /* Low          */
#define APCCSR_NO_TTY_PRES      APCCSR_TTY_PRES         /* high alias   */
#define APCCSR_TTY_PRES_BIT     14                      /* bit position */
#define APCCSR_FPA_PRES         0x00008000              /* Low          */
#define APCCSR_32081_PRES       APCCSR_FPA_PRES         /* high alias   */
#define APCCSR_FPA_PRES_BIT     15                      /* bit position */
#define APCCSR_32081_PRES_BIT   APCCSR_FPA_PRES_BIT     /* high alias   */
#define	APCCSR_CONE_READY	0x00010000		/* low (weitek)	*/
#define	APCCSR_CONE_BUSY	APCCSR_CONE_READY

#ifndef	LOCORE
#define APCREG_CSR              (long *)0xffffff54      /* Private          */

#define FIFO_NOT_EMPTY		(*APCREG_CSR & APCCSR_VECBUS_FIFO)

#define APCCSR_CLASSSHFT        10

#define GETCPUSLOT              ((*APCREG_CSR & APCCSR_SLOTID) >> 2)
#define GETCPUDEV               (*APCREG_CSR & APCCSR_CPUID)
#define GETCPUERR               (*APCREG_ERR & APCREG_ERR_MASK)
#endif	LOCORE


/*
 *      APC NMI Control Register
 */

#define APCREG_NMI	0xffffff50	/* Generate SYSNMI (write only).*/
#define APCNMI_VALUE	0		/* Value written to cause NMI   */



/*
 *      APC Vector Bus Transmit Register
 */

#ifndef	LOCORE
typedef union apcvbxmit {
    struct {
        unsigned int
            v_class:2,                  /* High */
            v_device:2,                 /* High */
            v_slot:4,                   /* High */
            v_lamplo:2,                 /* High */
            v_lampreqlo:1,              /* High */
            :5,                         /* reserved */
            v_vector:8,                 /* High */
            v_lamphi:2,                 /* High */
            v_lampreqhi:1,              /* High */
            :5;                         /* reserved */
    } f;
    long l;
} apcvbxmit_t;

typedef	apcvbxmit_t	board_vbxmit_t;
#endif	LOCORE

#define APCVBXMIT_CLASS         0x00000003  /* 0 == direct, 1-3 == classed. */
#define APCVBXMIT_DEVICE        0x0000000c  /* 0 == CPU A, 1 == CPU B.      */
#define APCVBXMIT_SLOT          0x000000f0  /* Directed vector to this slot.*/
#define APCVBXMIT_LAMPLO        0x00000300  /* External lamp bits 0:1.      */
#define APCVBXMIT_LAMPREQLO     0x00000400  /* 1 == Direct to external lamp.*/
#define APCVBXMIT_VECTOR        0x00ff0000  /* The 8-bit vector to be sent. */
#define APCVBXMIT_LAMPHI        0x03000000  /* External lamp bits 2:3.      */
#define APCVBXMIT_LAMPREQHI     0x04000000  /* 1 == Direct to external lamp.*/

#ifndef	LOCORE
#define APCREG_VBXMIT	       (long *)0xffffff4c  /* Private, long write */
#endif	LOCORE
#define	VBXMIT_REG		APCREG_VBXMIT
#define	VB_BUSY			(!(*APCREG_CSR & APCCSR_VECBUS_FREE))


/*
 *      APC Vector Bus FIFO Register
 */

#ifndef	LOCORE
#define APCREG_VBFIFO       (char *)0xffffff48    /* Private, 32 bit read */
#endif	LOCORE

#define APC_VBFIFOSIZE      17
#define APC_VBFIFOMASK      0x0000000ff         /* <7:0> current ID     */



/*
 *      APC Error Status Register
 */

#ifndef	LOCORE
typedef union apcerr {
    struct {
        unsigned int
            e_cpuerr:1,         /* cpu error    High    */
            e_lock:1,           /* interlock    Low     */
            e_ctagcmp:2,        /* ctag compare Low     */
            e_vbits:4,          /* vbit         High    */
            e_ctagpe:2,         /* ctag parity  High    */
            e_vbitpe:4,         /* vbit parity  Low     */
            e_cache:2,          /* cache addr           */
            e_datape:4,         /* data parity  Low     */
            e_rdbus:1,          /* bus read     Low     */
            :11;                /* undefined            */
    } f;
    long l;
} apcerr_t;
#endif	LOCORE

#define APCERR_CPU      0x00000001      /* CPU error (High)             */
#define APCERR_LOCK     0x00000002      /* Interlock cycle (Low)        */

#define APCERR_CTAGCMP  0x0000000c      /* Ctag compare (Low)           */
#define APCERR_CTAG_CMP_SHFT     2
#define APCERR_CTAGCMP0 0x00000004
#define APCERR_CTAGCMP1 0x00000008

#define APCERR_VBITS    0x000000f0      /* Vbits                        */
#define APCERR_VBIT_SHFT         4
#define APCERR_VBIT0    0x00000010
#define APCERR_VBIT1    0x00000020
#define APCERR_VBIT2    0x00000040
#define APCERR_VBIT3    0x00000080

#define APCERR_CTAGPE   0x00000300      /* Ctag parity error (Low)      */
#define APCERR_CTAGPE0  0x00000100
#define APCERR_CTAGPE1  0x00000200

#define APCERR_VBITPE   0x00003c00      /* Vbit parity error            */
#define APCERR_VBITPE0  0x00000400
#define APCERR_VBITPE1  0x00000800
#define APCERR_VBITPE2  0x00001000
#define APCERR_VBITPE3  0x00002000

#define APCERR_CACHE    0x0000c000      /* cache address                */
#define APCERR_CACHE2   0x00004000
#define APCERR_CACHE3   0x00008000

#define APCERR_DATAPE   0x000f0000      /* Data parity error (Low)      */
#define APCERR_DATAPE0  0x00010000
#define APCERR_DATAPE1  0x00020000
#define APCERR_DATAPE2  0x00040000
#define APCERR_DATAPE3  0x00080000

#define APCERR_BUSERR   0x00100000      /* Bus error (Low)              */

#ifndef	LOCORE
#define APCREG_ERR      (long *)0xffffff44
#endif	LOCORE
#define APCREG_ERR_MASK 0x001fffff      /* Mask off unused bits         */


/*
 *      APC Physical Address Register
 */

#ifndef	LOCORE
typedef union apcpa {
    struct {
        unsigned int
            p_cycle:1,          /* Cycle type       */
            p_cpu:1,            /* Cpu indicator    */
            p_pa:30;            /* Physical address */
    } f;
    long l;
} apcpa_t;
#endif	LOCORE

#define APCPA_CYCLE     0x00000001      /* set if write else read       */
#define APCPA_CPU       0x00000002
#define APCPA_PA        0xfffffffc

#ifndef	LOCORE
#define APCREG_PA       (long *)0xffffff40
#endif	LOCORE



/*
 *      APC DUART Registers     (Signetics SCN2681-24)
 */

#define	APCDUART_AMODE		0xffffff00
#define	APCDUART_ASTATUS	0xffffff04
#define	APCDUART_ACLKSEL	0xffffff04
#define	APCDUART_ACMD		0xffffff08
#define	APCDUART_ARCVDATA	0xffffff0c
#define	APCDUART_AXMTDATA	0xffffff0c
#define	APCDUART_AUXCTL		0xffffff10
#define	APCDUART_INTRSTAT	0xffffff14
#define	APCDUART_INTRMASK	0xffffff14
#define	APCDUART_BMODE		0xffffff20
#define	APCDUART_BSTATUS	0xffffff24
#define	APCDUART_BCLKSEL	0xffffff24
#define	APCDUART_BCMD		0xffffff28
#define	APCDUART_BRCVDATA	0xffffff2c
#define	APCDUART_BXMTDATA	0xffffff2c

#ifndef	LOCORE
#define APCDUART_ADDR   ((duart_ctl_t *)APCDUART_AMODE)
#endif	LOCORE

/*
 *      APC ICU Registers
 */

/*
 * Register offsets are defined in "icu.h".
 */
#define APCICU_SLAVE        (char *)0xfffffe80
#define APCICU_MASTER       (char *)0xfffffe00


/*
 *      APC Diagnostic Control and Status Register
 */

#ifndef	LOCORE
typedef union apcdiag {
    struct {
        unsigned int
            dc_force_vbit_cmp:1,        /* Low  */
            dc_force_ctag_cmp:1,        /* Low  */
            dc_force_btag_cmp:1,        /* High */
            dc_vecbus_receive_enable:1, /* Low  */
            dc_vecbus_loopback:1,       /* High */
            dc_force_vecbus_grant:1,    /* Low  */
            dc_force_nbus_disable:1,    /* Low  */
            dc_force_nbus_diag:1,       /* Low  */
            dc_force_fdpmp:1,           /* Low  */
            dc_force_not_cycle:1,       /* Low  */
            dc_rom_overlaya:1,          /* Low  */
            dc_rom_overlayb:1,          /* Low  */
            dc_buserr_enable:1,         /* Low  */
            dc_block_a_req:1,           /* High */
            dc_block_b_req:1,           /* Low  */
            dc_diag_mode:1,             /* Low  */
            dc_test_rack:1,             /* Low  */
            dc_btag_cmp:1,              /* Low  */
            dc_dis_cbypass:1,           /* Low  */
            :13;
    } f;
    long l;
} apcctl_t;
#endif	LOCORE

#define apcdiag_t               apcctl_t

#ifndef	LOCORE
#define APCREG_CTL      (long *)0xfffeffbc
#define APCREG_DIAG     APCREG_CTL
#endif	LOCORE

#define APCCTL_FRC_VBIT_CMP     0x00000001      /* Low  Shared  */
#define APCCTL_FRC_CTAG_CMP     0x00000002      /* Low  Shared  */
#define APCCTL_FRC_BTAG_CMP     0x00000004      /* High Shared  */
#define APCCTL_VB_RCV_ENBL      0x00000008      /* Low  Shared  */
#define APCCTL_VB_LPBK_ENBL     0x00000010      /* High Shared  */
#define APCCTL_FRC_VB_GRANT     0x00000020      /* Low  Shared  */
#define APCCTL_FRC_NBI_DSBL     0x00000040      /* Low  Shared  */
#define APCCTL_NBI_DIAG         0x00000080      /* Low  Shared  */
#define APCCTL_FRC_DBL_PUMP     0x00000100      /* Low  Shared  */
#define APCCTL_FRC_NOT_MY_CYCLE 0x00000200      /* Low  Shared  */
#define APCCTL_ROM_OVERLAY_A    0x00000400      /* Low  Shared  */
#define APCCTL_ROM_OVERLAY_B    0x00000800      /* Low  Shared  */
#define APCCTL_BUS_ERR_ENBL     0x00001000      /* Low  Shared  */
#define APCCTL_BLOCK_A_REQ      0x00002000      /* High Shared  */
#define APCCTL_BLOCK_B_REQ      0x00004000      /* Low  Shared  */
#define APCCTL_DIAG_MODE        0x00008000      /* Low  Shared  */
#define APCCTL_TEST_RACK        0x00010000      /* Low  Shared  */
#define APCCTL_BTAG_CMP         0x00020000      /* Low  Private */
#define APCCTL_WRITE_MASK       0x00007fff      /* Writeable bits   */
#define APCCTL_NORMAL_BITS      0x00000fe3
#define APCCTL_INIT_VALUE       0x00024ffa      /* Used to set up each cpu */
                                                /*  at boot time           */



/*
 *      APC Wrong Parity Control Register
 */

#ifndef	LOCORE
typedef union apcwpar {
    struct {
        unsigned int
            w_fwap:4,           /* addr  parity - High  */
            w_fwcp:2,           /* ctag  parity - High  */
            w_fwdp:4,           /* data  parity - Low   */
            w_fwvp:1,           /* vbit  parity - Low   */
            w_fwaid:1,          /* addid parity - High  */
            w_fwbp:1,           /* byte  parity - High  */
            :19;
    } f;
    long l;
} apcwpar_t;
#endif	LOCORE

#define APCPAR_FRC_ADDR_PAR         0x0000000f  /* High Shared  */
#define APCPAR_FRC_CTAG_PAR         0x00000030  /* High Shared  */
#define APCPAR_FRC_DATA_PAR         0x000003c0  /* Low  Shared  */
#define APCPAR_FRC_VBIT_PAR         0x00000400  /* Low  Shared  */
#define APCPAR_FRC_ADDID_PAR        0x00000800  /* High Shared  */
#define APCPAR_FRC_BYTE_PAR         0x00001000  /* High Shared  */

#ifndef	LOCORE
#define APCREG_WPAR         (long *)0xfffeffb8
#endif	LOCORE



/*
 *      APC DESTSEL Control Register
 */

#define APCDEST_NOCACHE             0x00800000
#define APCDEST_DPMPRTN             0x01000000
#define APCDEST_DESTSELP            0x02000000
#define APCDEST_DESTSEL             0xfc000000

#ifndef	LOCORE
#define APCREG_DESTSEL      (long *)0xfffeffb4
#endif	LOCORE



/*
 *      APC LED Control Register
 */

#define APCLED_LED          0x000000ff          /* Low. 0=on. Shared    */
#ifndef	LOCORE
#define APCREG_LED          0xfffeffb0
#endif	LOCORE
#define LED_REG		    APCREG_LED
#define LED_LED		    APCLED_LED
#define LED(X)		    movb X,0xfffeffb0



/*
 *      APC NanoBus Interface Register
 */

#ifndef	LOCORE
typedef union apcnbusdata {
    struct {
        unsigned int
            d_lowdata,          /* High */
            d_highdata;         /* High */
    } f;
    long l[2];                  /* dwm 0 = low, 1 = high        */
} apcnbusdata_t;
#endif	LOCORE

#ifndef	LOCORE
#define APCREG_NBUSDATAH        (long *)0xfffeffac
#define APCREG_NBUSDATAL        (long *)0xfffeffa8
#endif	LOCORE



/*
 *      APC Board ID PROM Registers
 */

#ifndef	LOCORE
typedef union apcbrdhist {
    struct {
        char
            b_id,                       /* Id byte */
            b_rev;                      /* Revision byte */
    } f;
    short s;
} apcbrdhist_t;

typedef struct apcidprom {
    apcbrdhist_t b_boardhist[32];       /* Board history array */
} apcidprom_t;
#endif	LOCORE

#ifndef	LOCORE
#define APCREG_BRDHIST  (long *)0xfffeffc0      /* Shared, long read */
#endif	LOCORE

/*
 * Cone/Weitek
 */
#define APC_FPA_RMB             0x80000000      /* FSR Register Mod Bit */
#define APC_FPA_RMB_BIT         31              /*                      */

#ifndef	LOCORE
/*
 * Macro that can be used to obtain the CPU number directly from the
 * hardware.
 */

/* Original version --
 *
 * #define getcpuid() (*APCREG_CSR & (APCCSR_CPUID | APCCSR_SLOTID))
 *
 * Optimized version of getcpuid() --
 *
 *       Non-relevent bits of low order byte guaranteed to be zero
 */

#define getcpuid()              ((*((unsigned char *)0xffffff54)))
#endif	LOCORE

#endif	MMAX_APC
#endif	_APCDEFS_H_
