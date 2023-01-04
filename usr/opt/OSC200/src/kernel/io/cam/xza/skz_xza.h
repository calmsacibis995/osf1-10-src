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
#if !defined(SKZ_XZA_INCLUDE)
#define SKZ_XZA_INCLUDE 1

/************************************************************************
 *									*
 * File:	skz_xza.h						*
 * Date:	November 6, 1991					*
 * Author:	Ron Hegli                                               *
 *									*
 * Description:								*
 *	This file contains XZA hardware specific definitions, such	*
 *	as registers and other hardware descriptive definitions		*
 *									*
 ************************************************************************/


/*
**
** XZA Register Definitions
**
*/
typedef enum xza_registers { 
	ASR, ASRCR, AICR, CICR, AECR, CECR, AMCSR,
	CBRCR, AFAR, XDEV, XBE, XFADR, XFAER, XCOMM,
	XPUD, GCQ0IR, GCQ1IR, NRE, QIR, AFAR0, AFAR1,
	XPD1, XPD2
} XZA_REGISTERS;

typedef struct xza_reg_addrs {
    vm_offset_t xdev;
    vm_offset_t xbe;
    vm_offset_t xfadr;
    vm_offset_t xcomm;
    vm_offset_t xfaer;
    vm_offset_t xpd1;
    vm_offset_t xpd2;
    vm_offset_t afar1;
    vm_offset_t nre;
    vm_offset_t afar0;
    vm_offset_t xpud;
    vm_offset_t asr;
    vm_offset_t gcq0ir;
    vm_offset_t gcq1ir;
    vm_offset_t qir;
} XZA_REG_ADDRS;

/*
** XZA Register Address Offsets
*/
#define XDEV_OFFSET	0x0
#define XBE_OFFSET	0x4
#define XFADR_OFFSET	0x8
#define XCOMM_OFFSET	0x10
#define XFAER_OFFSET	0x2c
#define XPD1_OFFSET	0x100
#define XPD2_OFFSET	0x104
#define AFAR1_OFFSET	0x104
#define NRE_OFFSET	0x104
#define AFAR0_OFFSET	0x108
#define XPUD_OFFSET	0x10c
#define ASR_OFFSET	0x10c
#define GCQ0IR_OFFSET	0x114
#define GCQ1IR_OFFSET	0x118
#define QIR_OFFSET	0x11c

typedef struct xdev_reg {
	unsigned short		device_type;
	unsigned char		firm_rev;
	unsigned char		hard_rev;
} XDEV_REG;

typedef struct xbe_reg {
	unsigned int		resrvd	: 1;	/* Reserved */
	unsigned int		emp	: 1;	/* enable more protocol */
	unsigned int		dxto	: 1;	/* disable XMI timeout */
	unsigned int		notimp3	: 1;	/* not implemented */
	unsigned int		fcid	: 6;	/* failing commander id */
	unsigned int		stf	: 1; 	/* self-test fail */
	unsigned int 		notimp11: 1;	/* not implemented */
	unsigned int		notimp12: 1;	/* not implemented */
	unsigned int		tto	: 1;	/* transaction timeout */
	unsigned int		rsrvd14	: 1;	/* reserved */
	unsigned int		cnak	: 1;	/* command no ack */
	unsigned int		rer	: 1;	/* read error response */
	unsigned int		rse	: 1;	/* read sequence error */
	unsigned int		nrr	: 1;	/* no read response */
	unsigned int		crd	: 1;	/* corrected read data */
	unsigned int		wdnak	: 1;	/* write data no ack */
	unsigned int		ridnak	: 1;	/* read/indent data no ack */
	unsigned int		wse	: 1;	/* write sequence error */
	unsigned int		pe	: 1;	/* parity error */
	unsigned int		notimp24: 1; 	/* not implemented */
	unsigned int		notimp25: 1;	/* not implemented */
	unsigned int		notimp26: 1;	/* not implemented */
	unsigned int		cc	: 1;	/* corrected confirmation */
	unsigned int		notimp28: 1;	/* not implemented */
	unsigned int		nhalt	: 1;	/* node halt */
	unsigned int		nrst	: 1;	/* node reset */
	unsigned int		es	: 1;	/* error summary */
} XBE_REG;

typedef struct xfadr_reg {
	unsigned int		fa	: 30;	/* failing address */
	unsigned int		fln	: 2;	/* failing length */
} XFADR_REG;

typedef struct xfaer_reg {
	unsigned short		mask;		/* */
	unsigned int		ae 	: 10;	/* address extension */
	unsigned int		mbz	: 2;	/* must be zero */
	unsigned int		fcmd	: 4;	/* */
} XFAER_REG;

typedef struct xcomm_reg {
	unsigned char		char1;	
	unsigned int		node_id1: 4;	/* node id 1 */
	unsigned int		mbz1	: 3;	/* must be zero */
	unsigned int		busy1	: 1;	/* busy */
	unsigned char		char2;	
	unsigned int		node_id2: 4;	/* node id 2 */
	unsigned int		mbz2	: 3;	/* must be zero */
	unsigned int		busy2	: 1;	/* busy */
} XCOMM_REG;

typedef struct asr_reg {
	unsigned int		afqe	: 1;	/* adapter free q exhausted */
	unsigned int		mbz1	: 1;	/* must be zero */
	unsigned int		dse	: 1;	/* data structure error */
	unsigned int		mbz3	: 1;	/* must be zero */
	unsigned int		ac	: 1;	/* abnormal condition */
	unsigned int		mbz5	: 1;	/* must be zero */
	unsigned int		cic	: 1;	/* channel init complete */
	unsigned int		cec	: 1;	/* channel enable complete */
	unsigned int		sbz8	: 1;	/* should be zero */
	unsigned int		unin	: 1;	/* uninitialized */
	unsigned int		sbz10	: 4;	/* should be zero */
	unsigned int		c0	: 1;	/* channel zero flag */
	unsigned int		c1	: 1;	/* channel one flag */
	unsigned int		adsec	: 8;	/* data struct error code */
	unsigned int		accamec	: 7;	/* cond. code, maint. code */
	unsigned int		ame	: 1;	/* adapter maintenance error */
} ASR_REG;

typedef struct xpud_reg {
	unsigned int		fic	: 1;	/* firmware init complete */
	unsigned int		unused1	: 1;	/* unused */
	unsigned int		eefl	: 1;	/* eeprom firmware loaded */
	unsigned int		efl	: 1;	/* eprom firmware loaded */
	unsigned int		dpti	: 1;	/* diag patch table invalid */
	unsigned int		ehe	: 1;	/* error history exists */
	unsigned int		unused6	: 3;	/* unused */
	unsigned int		unin	: 1;	/* uninitialized */
	unsigned int		unused10: 2;	/* unused */
	unsigned int		s0vid	: 1;	/* SIOP0 valid id */
	unsigned int		s1vid	: 1;	/* SIOP1 valid id */
	unsigned int		xnaga	: 1;	/* xnaga test passed */
	unsigned int		s0p	: 1;	/* SIOP0 passed */
	unsigned int		s1p	: 1;	/* SIOP1 passed */
	unsigned int		sprt	: 1;	/* shared parity ram tst pass */
	unsigned int		srmt	: 1;	/* shared ram march test pass */
	unsigned int		xnadalt	: 1;	/* xnadal timeout logic test */
	unsigned int		xnadalr	: 1;	/* xnadal readback test pass */
	unsigned int		eetp	: 1;	/* eeprom test passed */
	unsigned int		unused22: 1;	/* unused */
	unsigned int		cctp	: 1;	/* CVAX chip test passed */
	unsigned int		cprtp	: 1;	/* CVAX parity ram test pass */
	unsigned int		crmtp	: 1;	/* CVAX ram march test passed */
	unsigned int		cdtp	: 1;	/* console drivers test pass */
	unsigned int		stp	: 1;	/* SSC test passed */
	unsigned int		drtp	: 1;	/* diag reg test passed */
	unsigned int		ciltp	: 1;	/* CVAX int lines test passed */
	unsigned int		brtp	: 1;	/* boot rom test passed */
	unsigned int		stc	: 1;	/* self test complete */
} XPUD_REG;

typedef struct xpd1_reg {
	unsigned char		rcode;		/* register emulation code */
	unsigned int		c0	: 1;	/* channel 0 mask flag */
	unsigned int		c1	: 1;	/* channel 1 mask flag */
	unsigned int		reserved: 3;	/* reserved */
	unsigned int		pax	: 3;	/* physical address extension */
	unsigned int		slot	: 4;	/* XMI node id of lamb */
	unsigned int		vector	: 10;	/* interrupt vector */
	unsigned int		ipl	: 2;	/* interrupt level */
} XPD1_REG;

struct amcsr {
		unsigned int	unused	: 2;	/* unused */
		unsigned int	dqe	: 1;	/* dqe error */
		unsigned int	unused3	: 29;	/* unused */
};

typedef struct xpd2_reg {
    union {
	unsigned int		pa;		/* physical address */
	struct	amcsr		amcsr;
    } xpd2_un;
} XPD2_REG;

typedef struct afar0 {
	unsigned int		pa;
} AFAR0_REG;

typedef struct afar1 {
	unsigned char		pax;
	unsigned char		sbz[7];
} AFAR1_REG;

/*
** Register Emulation 'rcode's
*/
#define ASRCR_EMULATION	01
#define AICR_EMULATION	02
#define CICR_EMULATION	02
#define AECR_EMULATION	03
#define CECR_EMULATION	03
#define AMCSR_EMULATION	04
#define CBRCR_EMULATION	05

typedef struct xza_sg_elem {
	unsigned long		pa	: 32;	/* physical addrs */
	unsigned long		pax	: 16;	/* physical addrs */
	unsigned long		length	: 16;	/* SG segment length */
} XZA_SG_ELEM;

/*
** IPL level definitions
*/
#define IPL_14	0
#define IPL_15	1
#define IPL_16	2
#define IPL_17	3

/* for OSFPal */
#define DEV_LEVEL_0 	IPL_14
#define DEV_LEVEL_1 	IPL_15
#define CLOCK_AND_IP	IPL_16


/*
** XZA state
*/
typedef enum xza_states {
	UNINITIALIZED, DISABLED, ENABLED, DIAGNOSTIC
} XZA_STATE;

/*
** Major definitions
*/

#define XZA_DEVICE_TYPE	0x0c36	/* 0c = XMI I/O w/XCOMM, 36 = XZA/SCSI */
#define XZA_CHANNELS	2
#define XZA_PORT_PAGE_SIZE	(8 * 1024)	/* 8K */
#define XZA_MAX_TRANSFER	(8 * 1024 * 1024) /* 8 MB */
#define XZA_MAX_SG_ELEMENTS	1024	/* 8 bytes each, up to 8K port page */

/*
** Timing
*/
#define XZA_STATE_CHANGE_TIME	2	/* State Change wait, in seconds */
#define XZA_EMUL_TIME		50	/* Wait for ASRCR Emul, in tenths */
#define XZA_BUS_RESET_TIME	10	/* Bus reset wait, in seconds */
#define XZA_ADAPTER_RESET_TIME	10	/* Adapter Reset time, in seconds */
#define XZA_ADAPTER_HALT_TIME	10	/* Adapter halt time, in seconds */
#define XZA_BUS_DEV_RESET_TIME	10 	/* Bus Device Reset time, in seconds */
#define XZA_RESPONSE_TIME	2000	/* During boot, in 0.001 secs */


/*
** Error Block Structures
*/
#define XZA_MAX_ERROR_RECORDS 5
#define XZA_ERR_BUF_SIZE      640

/* Error Types */
#define XZA_ET_MACH_CHECK	1
#define XZA_ET_NODE_HALT	2
#define XZA_ET_AME		3
#define XZA_ET_FW_UPDATE	4

/* Error Sub-Types */
#define XZA_EST_PEEK_TRANS	1
#define XZA_EST_DATAMOVE	2
#define XZA_EST_HOST_INTR	3
#define	XZA_EST_XBE_RELATED	4
#define XZA_EST_SIOP		5

/*
** Error Block Header
*/
typedef struct xza_eb_header {
	unsigned int	uptime_100ms;	/* adapter uptime, in 100ms */
	unsigned int	xdev_reg;	/* XDEV register contents */
	unsigned short	err_type;	/* error type */
	unsigned short	sub_type;	/* error sub-type */
	unsigned int	xbe_reg;	/* XBE register contents */
	unsigned int	xfadr_reg;	/* XFADR register contents */
	unsigned int	xfaer_reg;	/* XFAER register contents */
	unsigned int	asr_reg;	/* ASR register contents */
	unsigned int	fw_pc;		/* firmware error PC */
	unsigned int	gacsr_reg;	/* GACSR register contents */
	unsigned int	diag_reg;	/* XZA diag register contents */
} XZA_EB_HEADER;

typedef struct xza_mc_nh_body {
	unsigned int	r0;		/* R0 register contents */
	unsigned int	r1;		/* R1 register contents */
	unsigned int	r2;		/* R2 register contents */
	unsigned int	r3;		/* R3 register contents */
	unsigned int	r4;		/* R4 register contents */
	unsigned int	r5;		/* R5 register contents */
	unsigned int	r6;		/* R6 register contents */
	unsigned int	r7;		/* R7 register contents */
	unsigned int	r8;		/* R8 register contents */
	unsigned int	r9;		/* R9 register contents */
	unsigned int	r10;		/* R10 register contents */
	unsigned int	r11;		/* R11 register contents */
	unsigned int	ap;		/* AP register contents */
	unsigned int	fp;		/* FP register contents */
	unsigned int	sp;		/* (SP) register contents */
	unsigned int	sp4;		/* 4(SP) register contents */
	unsigned int	sp8;		/* 8(SP) register contents */
	unsigned int	sp12;		/* 12(SP) register contents */
	unsigned int	sp16;		/* 16(SP) register contents */
	unsigned int	sp20;		/* 20(SP) register contents */
	unsigned int	sp24;		/* 24(SP) register contents */
	unsigned int	sp28;		/* 28(SP) register contents */
} XZA_MC_NH_BODY;

typedef struct xza_peek_trans_body {
	unsigned int	pkxmil0_reg;	/* PKXMIL0 register contents */
	unsigned int	pkxmih0_reg;	/* PKXMIH0 register contents */
	unsigned int	pkdata0_reg;	/* PKDATA0 register contents */
	unsigned int	pkdatb0_reg;	/* PKDATB0 register contents */
	unsigned int	pkxmil1_reg;	/* PKXMIL1 register contents */
	unsigned int	pkxmih1_reg;	/* PKXMIH1 register contents */
	unsigned int	pkdata1_reg;	/* PKDATA1 register contents */
	unsigned int	pkdatb1_reg;	/* PKDATB1 register contents */
} XZA_PEEK_TRANS_BODY;


typedef struct xza_datamove_body {
	unsigned int	dmpor0_reg;	/* DMPOR0 register contents */
	unsigned int	dmcsr0_reg;	/* DMCSR0 register contents */
	unsigned int	dmxmi0_reg;	/* DMXMI0 register contents */
	unsigned int	dmnpa0_reg;	/* DMNPA0 register contents */
	unsigned int	dmpor1_reg;	/* DMPOR0 register contents */
	unsigned int	dmcsr1_reg;	/* DMCSR0 register contents */
	unsigned int	dmxmi1_reg;	/* DMXMI0 register contents */
	unsigned int	dmnpa1_reg;	/* DMNPA0 register contents */
	unsigned int	dmpor2_reg;	/* DMPOR0 register contents */
	unsigned int	dmcsr2_reg;	/* DMCSR0 register contents */
	unsigned int	dmxmi2_reg;	/* DMXMI0 register contents */
	unsigned int	dmnpa2_reg;	/* DMNPA0 register contents */
	unsigned int	dmpor3_reg;	/* DMPOR0 register contents */
	unsigned int	dmcsr3_reg;	/* DMCSR0 register contents */
	unsigned int	dmxmi3_reg;	/* DMXMI0 register contents */
	unsigned int	dmnpa3_reg;	/* DMNPA0 register contents */
} XZA_DATAMOVE_BODY;

typedef struct xza_host_intr_body {
	unsigned int	gahir_reg;	/* GAHIR register contents */
	unsigned int	gaivr_reg;	/* GAIVR register contents */
	unsigned int	gatmr_reg;	/* GATMR register contents */
} XZA_HOST_INTR_BODY;

typedef struct xza_xbe_related_body {
	unsigned int	unused;		/* no body */
} XZA_XBE_RELATED_BODY;

typedef struct xza_siop_body {
	unsigned int	channel_mask;	
	unsigned int	reserved;
	unsigned int	dstat_reg;	/* DSTAT register contents */
	unsigned int	istat_reg;	/* ISTAT register contents */
	unsigned long	siop_regs[8];	/* SIOP registers */
} XZA_SIOP_BODY;

typedef struct xza_fw_update_body {
	unsigned int	ascii_fw_image_rev;	/* FW image revision */
	unsigned char	ascii_fw_rev_date[12];	/* FW image rev date */
} XZA_FW_UPDATE_BODY;

typedef union xza_error_body {
	XZA_MC_NH_BODY		mc_nh_body;
	XZA_PEEK_TRANS_BODY	peek_trans_body;
	XZA_DATAMOVE_BODY	datamove_body;
	XZA_HOST_INTR_BODY	host_intr_body;
	XZA_XBE_RELATED_BODY	xbe_related_body;
	XZA_SIOP_BODY		siop_body;
	XZA_FW_UPDATE_BODY	fw_update_body;
} XZA_EB_BODY;

typedef struct error_block {
	XZA_EB_HEADER	eb_header;
	XZA_EB_BODY	eb_body;
} EB;


#endif
