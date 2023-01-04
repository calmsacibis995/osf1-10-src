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

#ifndef _SIOPDEFS_H_ 
#define _SIOPDEFS_H_ 


/* DEFINITIONS OF ADDRESSES OF SIOP REGISTERS IN LBUS SPACE AS SEEN FROM
 * THE SIOP'S.  THIS IS USED TO MOVE DATA FROM CHIP REGISTERS TO MEMORY
 * WITH THE MEMORY MOVE INSTRUCTION.
 */

/* The BCR53C710 register set as defined by the NCR data sheet. */
#define	SIOP_REG_SCNTL0   	0x00
#define	SIOP_REG_SCNTL1   	0x01
#define	SIOP_REG_SDID   	0x02
#define	SIOP_REG_SIEN   	0x03
#define	SIOP_REG_SCID   	0x04
#define	SIOP_REG_SXFER   	0x05
#define	SIOP_REG_SODL   	0x06
#define	SIOP_REG_SOCL   	0x07
#define	SIOP_REG_SFBR   	0x08
#define	SIOP_REG_SIDL   	0x09
#define	SIOP_REG_SBDL   	0x0a
#define	SIOP_REG_SBCL   	0x0b
#define	SIOP_REG_DSTAT   	0x0c
#define	SIOP_REG_SSTAT0   	0x0d
#define	SIOP_REG_SSTAT1   	0x0e
#define	SIOP_REG_SSTAT2   	0x0f
#define	SIOP_REG_DSA0   	0x10
#define	SIOP_REG_DSA1   	0x11
#define	SIOP_REG_DSA2   	0x12
#define	SIOP_REG_DSA3   	0x13
#define	SIOP_REG_CTEST0   	0x14
#define	SIOP_REG_CTEST1   	0x15
#define	SIOP_REG_CTEST2   	0x16
#define	SIOP_REG_CTEST3   	0x17
#define	SIOP_REG_CTEST4   	0x18
#define	SIOP_REG_CTEST5   	0x19
#define	SIOP_REG_CTEST6   	0x1a
#define	SIOP_REG_CTEST7   	0x1b
#define	SIOP_REG_TEMP0   	0x1c
#define	SIOP_REG_TEMP1   	0x1d
#define	SIOP_REG_TEMP2   	0x1e
#define	SIOP_REG_TEMP3   	0x1f
#define	SIOP_REG_DFIFO   	0x20
#define	SIOP_REG_ISTAT   	0x21
#define	SIOP_REG_CTEST8   	0x22
#define	SIOP_REG_LCRC   	0x23
#define	SIOP_REG_DBC0   	0x24
#define	SIOP_REG_DBC1   	0x25
#define	SIOP_REG_DBC2   	0x26
#define	SIOP_REG_DCMD   	0x27
#define	SIOP_REG_DNAD0   	0x28
#define	SIOP_REG_DNAD1   	0x29
#define	SIOP_REG_DNAD2   	0x2a
#define	SIOP_REG_DNAD3   	0x2b
#define	SIOP_REG_DSP0   	0x2c
#define	SIOP_REG_DSP1   	0x2d
#define	SIOP_REG_DSP2   	0x2e
#define	SIOP_REG_DSP3   	0x2f
#define	SIOP_REG_DSPS0   	0x30
#define	SIOP_REG_DSPS1   	0x31
#define	SIOP_REG_DSPS2   	0x32
#define	SIOP_REG_DSPS3   	0x33
#define	SIOP_REG_SCRATCH0   	0x34
#define	SIOP_REG_SCRATCH1   	0x35
#define	SIOP_REG_SCRATCH2   	0x36
#define	SIOP_REG_SCRATCH3   	0x37
#define	SIOP_REG_DMODE   	0x38
#define	SIOP_REG_DIEN   	0x39
#define	SIOP_REG_DWT   		0x3a
#define	SIOP_REG_DCNTL   	0x3b
#define	SIOP_REG_ADDER0   	0x3c
#define	SIOP_REG_ADDER1   	0x3d
#define	SIOP_REG_ADDER2   	0x3e
#define	SIOP_REG_ADDER3   	0x3f

/* Register bit definitions. */

/* SCNTL0 */
#define SCNTL0_ARB1	0x80
#define SCNTL0_ARB0	0x40
#define SCNTL0_START	0x20
#define SCNTL0_WATN	0x10
#define SCNTL0_EPC	0x08
#define SCNTL0_EPG	0x04
#define SCNTL0_AAP	0x02
#define SCNTL0_TRG	0x01

/* SCNTL1 */
#define SCNTL1_EXC	0x80
#define SCNTL1_ADB	0x40
#define SCNTL1_ESR	0x20
#define SCNTL1_CON	0x10
#define SCNTL1_RST	0x08
#define SCNTL1_AESP	0x04
#define SCNTL1_SND	0x02
#define SCNTL1_RCV	0x01

/* SIEN */
#define SIEN_MA		0x80
#define SIEN_FCMP	0x40
#define SIEN_STO	0x20
#define SIEN_SEL	0x10
#define SIEN_SGE	0x08
#define SIEN_UDC	0x04
#define SIEN_RST	0x02
#define SIEN_PAR	0x01

/* SXFER */
#define SXFER_DHP	0x80
#define SXFER_TP	0x70
#define SXFER_MO	0xf

/* SBCL */
#define SBCL_REQ	0x80
#define SBCL_ACK	0x40
#define SBCL_BSY	0x20
#define SBCL_SEL	0x10
#define SBCL_ATN	0x08
#define SBCL_MSG	0x04
#define SBCL_CD		0x02
#define SBCL_IO		0x01
#define SBCL_SSCF1	0x02
#define SBCL_SSCF0	0x01

/* DSTAT */
#define DSTAT_DFE	0x80
#define DSTAT_BF	0x20
#define DSTAT_ABRT	0x10
#define DSTAT_SSI	0x08
#define DSTAT_SIR	0x04
#define DSTAT_WTD	0x02
#define DSTAT_IID	0x01

/* SSTAT0 */
#define SSTAT0_MA	0x80
#define SSTAT0_FCMP	0x40
#define SSTAT0_STO	0x20
#define SSTAT0_SEL	0x10
#define SSTAT0_SGE	0x08
#define SSTAT0_UDC	0x04
#define SSTAT0_RST	0x02
#define SSTAT0_PAR	0x01

/* SSTAT1 */
#define SSTAT1_ILF	0x80
#define SSTAT1_ORF	0x40
#define SSTAT1_OLF	0x20
#define SSTAT1_AIP	0x10
#define SSTAT1_LOA	0x08
#define SSTAT1_WOA	0x04
#define SSTAT1_RST	0x02
#define SSTAT1_SDP	0x01

/* SSTAT1 */
#define SSTAT1_ILF	0x80
#define SSTAT1_ORF	0x40
#define SSTAT1_OLF	0x20
#define SSTAT1_AIP	0x10
#define SSTAT1_LOA	0x08
#define SSTAT1_WOA	0x04
#define SSTAT1_RST	0x02
#define SSTAT1_SDP	0x01

/* SSTAT2 */
#define SSTAT2_FF	0xf0

/* ISTAT */
#define ISTAT_ABRT	0x80
#define ISTAT_RST	0x40
#define ISTAT_SIGP	0x20
#define ISTAT_CON	0x08
#define ISTAT_SIP	0x02
#define ISTAT_DIP	0x01

/* DMODE */
#define DMODE_BL1	0x80
#define DMODE_BL0	0x40
#define DMODE_FC2	0x20
#define DMODE_FC1	0x10
#define DMODE_PD	0x08
#define DMODE_FAM	0x04
#define DMODE_UO	0x02
#define DMODE_MAN	0x01

/* DIEN */
#define DIEN_BF		0x20
#define DIEN_ABRT	0x10
#define DIEN_SSI	0x08
#define DIEN_SIR	0x04
#define DIEN_WTD	0x02
#define DIEN_IID	0x01

/* DCNTL */
#define DCNTL_CF1	0x80
#define DCNTL_CF0	0x40
#define DCNTL_EA	0x20
#define DCNTL_SSM	0x10
#define DCNTL_LLM	0x08
#define DCNTL_STD	0x04
#define DCNTL_FA	0x02
#define DCNTL_COM	0x01

/* CTEST8 */
#define CTEST8_FLF	0x08
#define CTEST8_CLF	0x04

/* Synchronous transfer parameter limits */
#define SIOP_MAX_OFFSET			8	/* max offset allowed */
#define SIOP_SLOW_MIN_PERIOD		50	/* 200 ns / 4 */
#define SIOP_FAST_MIN_PERIOD		25	/* 100 ns / 4 for fast SCSI */

/* the longest periods the 710 can support.  Anything longer must be
 * asynchronous.
 */
#define SIOP_SLOW_MAX_PERIOD		110
#define SIOP_FAST_MAX_PERIOD		55

/* SIOP SCSI clock periods for synchronous xfer period calculations */
#define SIOP_SLOW_SCSI_CLK		40
#define SIOP_FAST_SCSI_CLK		20
#define SIOP_SYNC_ADJUST		4

/* Macros for converting transfer period to values that can be loaded
 * into the SXFER register.
 */
#define SIOP_FAST_PERIOD(P,V) { \
	(V) = (((((P)*4)/(SIOP_FAST_SCSI_CLK))-SIOP_SYNC_ADJUST) + \
	((((P)*4)%SIOP_FAST_SCSI_CLK)?1:0)); \
	(P) = (((V) + SIOP_SYNC_ADJUST) * SIOP_FAST_SCSI_CLK)/4; \
	}


#define SIOP_SLOW_PERIOD(P,V) { \
	(V) = (((((P)*4)/(SIOP_SLOW_SCSI_CLK))-SIOP_SYNC_ADJUST) + \
	((((P)*4)%SIOP_SLOW_SCSI_CLK)?1:0)); \
	(P) = (((V) + SIOP_SYNC_ADJUST) * SIOP_SLOW_SCSI_CLK)/4; \
	}

/* Watchdog timeout value (BCLK = 25 MHz) */
#define SIOP_WATCHDOG		0x9c	/* 100 usec */

/* defines for job terminate state machine */
#define SIOP_KILL_RESET		0
#define SIOP_KILL_DRESET	1
#define SIOP_KILL_ABORT		2
#define SIOP_KILL_AT		3

/* Some general defines */
#define SIOP_SH_ALIGN		256

#endif

