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
 * @(#)$RCSfile: kn02ca.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/01/28 17:14:52 $
 */

/************************************************************************
 *									*
 *			Copyright (c) 1989 by				*
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

/*
 * Modification History:
 *
 * 10-May-91    Joe Szczypek
 *      Spun off this file from kn02ba.h for MAXine.
 *
 * 27-Apr-91	Mark Shepard
 *	Added defs for MAXine baseboard DSKBUS hardware.
 *
 * 15-Oct-90	Randall Brown
 *	Added error handling code.
 *
 * 23-Feb-90	Randall Brown
 *	Created file for support of 3MIN (DS5000_100).
 *
 */


#define	KN02CA_ADDR_ERR		0x0e000004	/* Address Error Register */
#define KN02CA_BOOT_0_REG	0x0e000008	/* Boot 0 Register */
#define KN02CA_INTR_REG		0x0e00000c	/* CPU Interrupt Register */

#define	KN02CA_MEM_ERR		0x0c400000	/* Memory Error Register */
#define PAGE_BOUNDRY_ERR	0x00010000	/*   Page Boundry Error */
#define	TRANSFER_LEN_ERR	0x00008000	/*   Transfer Length Error */
#define MEM_ERR_DISABLE		0x00004000	/*   Memory Error Disable */
#define	LAST_BYTE_ERR_MASK	0x00000f00	/*   Last Byte Error Mask */

#define KN02CA_MEM_SIZE		0x0c800000	/* Memory Size Register */
#define	KN02CA_16MB_MEM		0x00002000	/* 16MB if set, 4MB otherwise */

#define KN02CA_SSR_ADDR		0x1c040100	/* System Support Register */
#define	KN02CA_SIR_ADDR		0x1c040110	/* System Interrupt Register */
#define	KN02CA_SIRM_ADDR	0x1c040120	/* System Intr Mask Register */

#define KN02CA_SL_0_ADDR 0x10000000
#define KN02CA_SL_1_ADDR 0x14000000
#define KN02CA_SL_2_ADDR 0x18000000
#define KN02CA_SL_3_ADDR 0x1c300000
#define KN02CA_SL_4_ADDR 0x1c0c0000
#define KN02CA_SL_5_ADDR 0x1c100000
#define KN02CA_SL_6_ADDR 0x0            /* baseboard video base address */
#define KN02CA_SL_7_ADDR 0x1c240000     /* baseboard ISDN registers */
#define KN02CA_SL_8_ADDR 0x1c340000     /* baseboard Floppy DMA registers */
#define KN02CA_SL_9_ADDR 0x1c280000	/* baseboard DTI pseudo-slot */
#define KN02CA_SL_10_ADDR 0x0

#define KN02CA_SCSI_ADDR	0x1c300000
#define KN02CA_LN_ADDR		0x1c0c0000
#define KN02CA_SCC_ADDR		0x1c100000
#define	KN02CA_CLOCK_ADDR	0x1c200000	/* Base Address of Clock */
#define KN02CA_DTI_ADDR		0x1c280000	/* DTI controller data reg.*/
#define KN02CA_VIDEO_ADDR	0x0690		/* MAXine IO space for video */
#define KN02CA_BBA_ADDR		0x1c240000      /* baseboard ISDN registers */
#define KN02CA_FDI_ADDR		0x1c2c0000      /* baseboard Floppy DMA registers */

#define KN02CA_SCSI_INDEX	3		/* Base board SCSI port */
#define KN02CA_LN_INDEX		4		/* Base board ethernet port */
#define KN02CA_SCC_INDEX	5		/* Base board serial line */
#define KN02CA_BBA_INDEX	7		/* MAXine base board audio */	
#define KN02CA_FDI_INDEX	8		/* MAXine base board FDI */
#define KN02CA_DTI_INDEX	9		/* MAXine base board DB */
#define KN02CA_VIDEO_INDEX     10		/* MAXine base board video */ 


/* SIR Bit defines */

#define COMM1_XMIT	0x80000000	/* Comm Port 1 Xmit Intr 	*/
#define COMM1_XMIT_DMA	0x40000000	/* Comm Port 1 Xmit DMA Error	*/
#define COMM1_RECV	0x20000000	/* Comm Port 1 Recv Intr 	*/
#define COMM1_RECV_DMA	0x10000000	/* Comm Port 1 Recv DMA Error	*/

#define DTI_TX_PEND	0x08000000	/* DTi Transmit Page End Int   	*/
#define DTI_TX_DMA_ERR	0x04000000	/* DTi Transmit DMA Mem. rd err.*/
#define DTI_RX_HPINT	0x02000000	/* DTi Receive Halt Page Int.  	*/
#define DTI_DMA_POVR	0x01000000	/* DTi Receive DMA Page Overrun	*/

#define FDI_DMA_ERROR   0x00800000      /* FDI DMA error interrupt      */
#define ISDN_DMA_TXINT  0x00400000      /* ISDN DMA Transmit interrupt  */
#define ISDN_DMA_RXINT  0x00200000      /* ISDN DMA Receive interrupt   */
#define ISDN_DMA_ERROR  0x00100000      /* IDSN DMA error interrupt     */

#define SCSI_DMA_INTR	0x00080000	/* SCSI DMA buffer ptr loaded	*/
#define SCSI_DMA_ORUN	0x00040000	/* SCSI DMA Overrun Error	*/
#define SCSI_DMA_MEM	0x00020000	/* SCSI DMA Mem Read Error	*/
#define LANCE_DMA_MEM	0x00010000	/* LANCE DMA Mem Read Error	*/

#define FDI_INT         0x00008000      /* FDI Interrupt                */
#define	UNSCUR_JMPR	0x00004000	/* Security Mode Jumper		*/
#define POWER_ON_RESET	0x00002000	/* Reset on Power On    	*/
#define TC0_INTR	0x00001000	/* TC Option 0 interrupt      	*/

#define ISDN_INT        0x00000800      /* ISDN interrupt               */
#define NRMOD_JMPR	0x00000400	/* Manufacturing Mode Jumper	*/
#define	SCSI_CHIP_INTR	0x00000200	/* SCSI 53c94 Chip Interrupt	*/
#define	LANCE_CHIP_INTR	0x00000100	/* LANCE Chip Interrupt		*/

#define	FLOPPY_HDSTATUS	0x00000080	/* Floppy Status            	*/
#define	SCC0_INTR	0x00000040	/* SCC(0) Intr          	*/
#define TC1_INTR	0x00000020	/* TC Option 1 interrupt	*/
#define	FLOPPY_XDSTATUS	0x00000010	/* Floppy Status      		*/
#define	PSWARN		0x00000010	/* Power Supply Warning		*/

#define	VIDEO_INT	0x00000008	/* Video Frame interrupt	*/
#define NVIDEO_INT	0x00000004	/* Not Video frame interrupt    */
#define DTI_TXINT	0x00000002	/* DTI Xmit-Rdy interrupt    */
#define DTI_RXINT	0x00000001	/* DTI Recv-Avail. interrupt */

/* Combinations of SIR bits */

#define RESERVED_BITS	( 0 )
#define SLU_INTR	(COMM1_XMIT | COMM1_XMIT_DMA | COMM1_RECV | \
			 COMM1_RECV_DMA | SCC0_INTR )
#define SCSI_INTR	(SCSI_DMA_INTR | SCSI_DMA_ORUN | SCSI_DMA_MEM | \
			 SCSI_CHIP_INTR )
#define LANCE_INTR	(LANCE_DMA_MEM | LANCE_CHIP_INTR)

/* MAXine SSR defines */

#define	LED		0x00000001	/* LED on/off bit */
#define	RSVD		0x0000001e	/* Reserved */
#define	DTI_RST		0x00000020	/* Reset for SERIAL.bus */
#define VDAC_RESET	0x00000040	/* Reset for video subsystem */
#define	FDI_RESET	0x00000080	/* Reset for FDI controller */
#define LANCE_RESET	0x00000100	/* Reset for network controller */
#define	SCSI_RESET	0x00000200	/* Reset for SCSI subsystem */
#define	RTC_RESET	0x00000400	/* Reset for TOY */
#define	SCC_RESET	0x00000800	/* Reset for serial line */
#define	ISDN_RESET	0x00001000	/* Reset for ISDN subsystem */
#define	LANCE_DMA_EN	0x00010000	/* LANCE DMA enable */
#define SCSI_DMA_EN	0x00020000	/* SCSI DMA enable */
#define	SCSI_DMA_DIR	0x00040000	/* SCSI DMA direction */
#define	ISDN_RX_DMA	0x00080000	/* ISDN Receive DMA enable */
#define	ISDN_TX_DMA	0x00100000	/* ISDN Transmit DMA enable */
#define	FDI_DMA_EN	0x00200000	/* FDI DMA enable */
#define	FDI_DMA_DIR	0x00400000	/* FDI DMA direction */
#define RSVD_1		0x0f800000	/* Reserved */
#define	DTI_DMA_RX	0x10000000	/* SERIAL.bus RX DMA enable */
#define DTI_DMA_TX	0x20000000	/* SERIAL.bus TX DMA enable */
#define	COMM_DMA_RX	0x40000000	/* Comm. Port RX DMA enable */
#define COMM_DMA_TX	0x80000000	/* Comm. Port TX DMA enable */

/*
 * IO ASIC's System Support Register
 *   Bit definitions
 */

#ifdef  DTI_DMA
#define DTI_INTR	(DTI_TX_PEND | DTI_TX_DMA_ERR, | DTI_RX_HPINT | DTI_DMA_POVR)
#else
#define DTI_INTR	(DTI_TXINT | DTI_RXINT)
#endif  DTI_DMA

#define VIDEO_INTR	(VIDEO_INT)
#define BBA_INTR  	(ISDN_INT | ISDN_DMA_ERROR | ISDN_DMA_RXINT | ISDN_DMA_TXINT)
#define FDI_INTR        (FDI_INT | FDI_DMA_ERROR)

/* SR_IBIT7 allows halt to always come in */
#define KN02CA_HALT         SR_IBIT7

#define SR_FPU        SR_IBIT8
#define SR_HALT       SR_IBIT7
#define SR_SYSTEM     SR_IBIT6
#define SR_MEM        SR_IBIT5
#define SR_RTC        SR_IBIT4
#define SR_INTR       SR_IBIT3
#define SR_SNET       SR_IBIT2
#define SR_SOFTC      SR_IBIT1

#define KN02CA_SR_IMASK0	SR_IEC | SR_FPU | SR_HALT | SR_SYSTEM | SR_MEM | SR_RTC | SR_SNET | SR_SOFTC
#define KN02CA_SR_IMASK1	SR_IEC | SR_FPU | SR_HALT | SR_SYSTEM | SR_MEM | SR_RTC | SR_SNET 
#define KN02CA_SR_IMASK2	SR_IEC | SR_FPU | SR_HALT | SR_SYSTEM | SR_MEM | SR_RTC
#define KN02CA_SR_IMASK3	SR_IEC | SR_FPU | SR_HALT | SR_MEM | SR_RTC
#define KN02CA_SR_IMASK4	SR_IEC | SR_FPU | SR_HALT | SR_MEM
#define KN02CA_SR_IMASK5	SR_IEC | SR_FPU | SR_HALT  
#define KN02CA_SR_IMASK6	SR_IEC | SR_FPU | SR_HALT | SR_MEM | SR_RTC
#define KN02CA_SR_IMASK7	SR_IEC | SR_FPU
#define KN02CA_SR_IMASK8	SR_IEC | SR_HALT

#define	KN02CA_SIRM_K1ADDR	0xbc040120

/* rpbfix: don't mask HALT */
/*#define KN02CA_SPL_MASK		(SR_IMASK8 | SR_IEC)*/
#define KN02CA_SPL_MASK		(0)

/* Base board slot number for MAXine. */
#define KN02CA_BASESLOT	3

#define KN02CA_ESRPKT 1
#define KN02CA_MEMPKT 2

/*
 * These defines, macros, and variables are for memory parity errors.
 * Format of "memreg" for logging memory parity errors.
 */
#define SIMMOFF 28
#define TYPEOFF 26
#define BYTEOFF 25
#define DPOFF 24
#define TCOUNTOFF 16
#define SCOUNTOFF 8
#define HCOUNTOFF 0
#define MEMREGFMT(simm, type, byte, dp, tcount, scount, hcount) \
(simm << SIMMOFF | type << TYPEOFF | byte << BYTEOFF | dp << DPOFF | \
tcount << TCOUNTOFF | scount << SCOUNTOFF | hcount << HCOUNTOFF)

#define KN02CA_TRANSINTVL (60*15)	/* time delta to enable parity log - 15 mins */

#ifdef LANGUAGE_C
struct trans_errcnt {             /* trans parity errors */
    long   trans_last;	/* time of most recent trans err */
    long   trans_prev;	/* time of previous trans err */
};

#define TRANSPAR 0x1
#define SOFTPAR 0x2
#define HARDPAR 0x3
#define MAXSIMM 16

struct kn02ca_consinfo_esr_t {
    u_int cause;	/* from the exception frame */
    u_int epc;		/* from the exception frame */
    u_int status;	/* from the exception frame */
    u_int badva;	/* from the exception frame */
    u_int sp;		/* from the exception frame */
    u_int ssr;     	/* system support reg */
    u_int sir;     	/* system interrupt reg */	
    u_int sirm;    	/* system interrupt mask */
};

struct kn02ca_consinfo_mem_t {
    u_int memreg;
    u_int pa;
    u_int epc;
    u_int badva;
};

struct kn02ca_consinfo_t {
    int	pkt_type;
    union {
	struct kn02ca_consinfo_esr_t esrp;
	struct kn02ca_consinfo_mem_t memp;
    } pkt;
};
    
#endif /* LANGUAGE_C */

