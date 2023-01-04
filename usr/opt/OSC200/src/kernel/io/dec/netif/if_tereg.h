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
  
#ifndef _IF_TEREG_H_
#define _IF_TEREG_H_

/*
 * Digital TGEC NI Adapter. This module handles architectures which
 * implement Network Interfaces by a direct programmatic interface to
 * the DIGITAL TGEC chip set. 
 */
  
/*
 * TGEC ring descriptors
 */
struct te_ring  {
   u_short	te_flag;	/* rcv/xmt status flags */
   unsigned	te_com:15;	/* receive length ; transmit TDR */
   unsigned	te_own:1;	/* ownership bit */
   u_short	te_resv;	/* reserved */
   u_short	te_info;	/* rcv/xmt info for TGEC */
   u_short	te_pg_off;	/* page offset */
   u_short	te_bsize;	/* rcv/xmt buffer size */
   u_int	te_bfaddr;	/* buffer address */
};

#define TEDESC struct te_ring

#define	TE_nTRING	16		/* Size of xmt ring: # of desc */
#define	TE_nRRING	16		/* Size of rcv ring: # of desc */

#define	TE_BFSIZE	1536		/* Size of xmt/rcv buffers */
#define	TE_SETUPSIZE	128		/* Size of setup frame buffer */
#define	TE_DIAGBFSIZE	18		/* Size of diagnostic buffer */

/*
 * Total buffer requirements: used by steal_mem.c
 */
#define	TE_BUFFER_REQ	(\
			((TE_nTRING + TE_nRRING) * TE_BFSIZE) +\
			((TE_nTRING + TE_nRRING + 2) * sizeof(TEDESC)) +\
			(TE_SETUPSIZE + TE_DIAGBFSIZE)\
			)

/*
 * TGEC receive status (ne_flag RDES0<00:15>)
 */
#define TE_OF			0x0001	/* Overflow			*/
#define TE_CE			0x0002	/* CRC error			*/
#define TE_DB			0x0004	/* Dribbling Bits		*/
#define TE_RTN			0x0008	/* Translation Not Valid 	*/
#define TE_FT			0x0020	/* Frame Type 			*/
#define TE_CS			0x0040	/* Collision Seen 		*/
#define TE_RTL			0x0080	/* Frame Too Long		*/
#define TE_RLS			0x0100	/* Last Segment			*/
#define TE_RFS			0x0200	/* First Segment		*/
#define TE_BO			0x0400	/* Buffer Overflow 		*/
#define TE_RF			0x0800	/* Runt Frame	 		*/
#define TE_RLE			0x4000	/* Length Error 		*/
#define TE_ES			0x8000	/* Error Summary 		*/

/* 
 * TGEC Own bit (ne_own RDES0<31> & TDES0<31>)  
 */
#define TE_OWN			0x0001	/* Own bit=1 TGEC owns ,=0 host owns */

/* 
 * TGEC info (ne_info RDES1<30:31> & TDES1<30:31>)
 */
#define TE_RVT			0x2000	/* Virtual Type */
#define TE_VA			0x4000	/* Virtual Address */
#define TE_CA			0x8000	/* Chain Address */

/* 
 * TGEC packet Data type 
 */
#define TE_DT			0x3000   /* Mask for the Data type      */
  /* Receive Data Type */
#define TE_DT_RSRF		0x0000   /* Serial Received Frame       */
#define TE_DT_RILF		0x1000   /* Internally Looped back Frame*/
#define TE_DT_RELF		0x2000   /* External looped back Frame  */
  /* Transmit Data Type */
#define TE_DT_TNOR		0x0000   /* Normal Transmit Frame Data  */
#define TE_DT_TSET		0x2000   /* Setup Frame	                */
#define TE_DT_TDIA		0x3000   /* Diagnostic Frame            */

  
/*
 * SGEC transmit status (ne_flag TDES0<00:15>)
 */
#define	TE_DE			0x0001	/* Deferred - network busy	*/
#define	TE_UF			0x0002	/* Underflow Error		*/
#define	TE_TTN			0x0004	/* Translation Not Valid	*/
#define	TE_CC			0x0078	/* Mask for Collision Count	*/
#define	TE_HF			0x0080	/* Heartbeat Fail		*/
#define	TE_EC			0x0100	/* Excessive Collisions		*/
#define	TE_LC			0x0200	/* Late Collision		*/
#define	TE_NC			0x0400	/* No Carrier			*/
#define	TE_LO			0x0800	/* Loss of Carrier 		*/
#define	TE_TLE			0x1000	/* Length Error			*/
#define	TE_TO			0x4000	/* Transmit Watchdog Timeout    */

/* transmit info (ne_info, TDES0<29:16>) */
#define TE_TDR			0x7fff	/* Time domain reflectometer	*/  

#define	TE_TVT			0x0080	/* Transmit Virtual Type	*/ 
#define	TE_IC			0x0100	/* Interrupt on Completion 	*/ 
#define	TE_TLS			0x0200	/* Last Segment			*/ 
#define	TE_TFS			0x0400	/* First Segment		*/ 
#define	TE_AC			0x0800	/* Add CRC disable		*/ 
#define TE_TDT			0x3000	/* Transmit Data Type		*/
#define TE_TVA			0x4000	/* Transmit Virtual Addressing	*/
#define TE_CA			0x8000	/* Chain Address		*/

/* Page Offset (ne_pg_off<8:0>) */
#define TE_PF			0x01ff	/* Page Offset		        */

/*
 * TGEC Vector Address, IPL, Sync/Asynch (CSR0)
 */
#define TE_CSR0_IV	0x0000fffc	/* Interrupt Vector		*/
#define TE_CSR0_SA	0x20000000	/* Sync/Asynch Mode		*/
#define TE_CSR0_IP	0xc0000000	/* Interrupt Priority		*/
#define TE_CSR0_INIT 	~(NE_CSR_IV | NE_CSR_SA | NE_CSR_IP)
#define TE_CSR0_MB1	0x1fff0003	/* must be 1 bits in csr0	*/

/* 
 * TGEC Transmit Polling Demand (CSR1)
 */
#define TE_CSR1_PD	0x00000001	/* Tx Polling Demand            */ 

/* 
 * TGEC Receive Polling Demand (CSR2)
 */
#define TE_CSR2_PD	0x00000001	/* Rx Polling Demand 		*/

/*
 * TGEC Descriptor List addresses
 *	CSR3 - Start of receive list
 *	CSR4 - Start of send list
 *  For best performance, descriptor lists should be octaword aligned, 
 *    they must be longword aligned.
 *  Transmit list must have at least two descriptors to use chaining
 *  Initially, CSR3 and CSR4 must be written before the start command
 *  These are physical addresses (??).
 */

/* 
 * TGEC Status Register (CSR5)
 */
#define TE_CSR5_IS	0x00000001	/* Interrupt Summary		*/
#define TE_CSR5_TI	0x00000002	/* Transmit Interrupt		*/
#define TE_CSR5_RI	0x00000004	/* Receive Interrupt            */
#define TE_CSR5_RU	0x00000008	/* Receive Buffer Unavailable   */
#define TE_CSR5_ME	0x00000010	/* Memory Error			*/
#define TE_CSR5_RW	0x00000020	/* Rx Watchdog Timer Interrupt  */
#define TE_CSR5_TW	0x00000040	/* Tx Watchdog Timer Interrupt  */
#define TE_CSR5_BO	0x00000080	/* Boot_Message			*/
#define TE_CSR5_TC	0x00000100	/* Transmit Completed		*/
#define TE_CSR5_IE	(TE_CSR5_TW | TE_CSR5_ME) 

#define TE_CSR5_DN	0x00010000	/* Done				*/
#define TE_CSR5_OM	0x00060000	/* Operating Mode		*/
/* Operation Mode */
#define TE_OM5_NOR	0x00000000	/* Normal Operation Mode        */
#define TE_OM5_INL	0x00020000	/* Internal Loopback            */
#define TE_OM5_EXL	0x00040000	/* External Loopback            */
#define TE_OM5_DIA	0x00060000	/* Diagnostic Mode	        */

#define TE_CSR5_RS	0x00c00000	/* Reception process State      */
/* Reception process State */
#define TE_RS_STP	0x00000000	/* Stoppped		        */
#define TE_RS_RUN	0x00400000	/* Running		        */
#define TE_RS_SUP	0x00800000	/* Suspended                    */

#define TE_CSR5_TS	0x03000000	/* Transmission process State   */
/* Transmit process State */
#define TE_TS_STP	0x00000000	/* Stoppped		        */
#define TE_TS_RUN	0x01000000	/* Running		        */
#define TE_TS_SUP	0x02000000	/* Suspended                    */

#define TE_CSR5_SS	0x3c000000	/* Self Test Status		*/
/* Reception process State */
#define TE_SS_ROM	0x04000000	/* ROM Error 		        */
#define TE_SS_RAM	0x08000000	/* RAM Error 		        */
#define TE_SS_AFR	0x0c000000	/* Address filter RAM Error	*/
#define TE_SS_TFF	0x10000000	/* Transmit FIFO Error	        */
#define TE_SS_RFF	0x14000000	/* Receive FIFO Error	        */
#define TE_SS_SLE	0x18000000	/* Self test Loopback Error     */

#define TE_CSR5_SF	0x40000000	/* Self Test Failed 		*/
#define TE_CSR5_ID	0x80000000	/* Initialization Done		*/

/*
 * TGEC Command and Mode Register (CSR6)
 */
#define TE_CSR6_SW	0x00000001	/* Swapping mode                */
#define TE_CSR6_AF 	0x00000006   	/* Address Filtering Mode       */
/* Address Filtering Mode */
#define TE_AF_NOR 	0x00000000 	/* Normal			*/
#define TE_AF_PRO 	0x00000002 	/* Promiscuous			*/
#define TE_AF_MUL 	0x00000004 	/* ALL Multicast  		*/  

#define TE_CSR6_PB	0x00000008	/* Pass Bad Frames Mode		*/
#define TE_CSR6_TR	0x00000030	/* Threshold control bits	*/
/* Threshold control bits */
#define TE_TC_72	0x00000000	/* 72 bytes			*/
#define TE_TC_96	0x00000010	/* 96 bytes			*/
#define TE_TC_128	0x00000020	/* 128 bytes			*/
#define TE_TC_160	0x00000030	/* 160 bytes			*/


#define TE_CSR6_FC	0x00000040	/* Force Collision Mode		*/
#define TE_CSR6_DC	0x00000080	/* Disable Data Chaining Mode	*/
#define TE_CSR6_OM	0x00000300	/* Operating Mode 		*/
/* Operation Mode */
#define TE_OM6_NOR	0x00000000	/* Normal Operation Mode        */
#define TE_OM6_INL	0x00000100	/* Internal Loopback            */
#define TE_OM6_EXL	0x00000200	/* External Loopback            */
#define TE_OM6_DIA	0x00000300	/* Diagnostic Mode	        */

#define TE_CSR6_SR	0x00000400	/* Start/Stop Rx Command	*/
#define TE_CSR6_ST	0x00000800	/* Start/Stop Tx Command	*/
#define TE_CSR6_HW	0x00010000	/* Hexaword alignment mode	*/
#define TE_CSR6_CD	0x00060000	/* CCTL/CDMR modes		*/
#define TE_CSR6_SE	0x00080000	/* Single Cycle Enable Mode	*/
#define TE_CSR6_BE	0x00100000	/* Boot Message Enable Mode     */
#define TE_CSR6_BL	0x3e000000	/* Burst Limit Mode		*/
#define TE_CSR6_IE	0x40000000	/* Interrupt Enable Mode        */
#define TE_CSR6_RE	0x80000000	/* Reset Command	        */

/*
 * TGEC CSR7 - System Base Register
 */

/*
 * TGEC CSR8 - Reserved Register
 */

/*
 * TGEC CSR9 - Watch Dog Timers
 */
#define TE_CSR9_TT	0x0000ffff	/* Transmit watchdog timeout	*/
#define TE_CSR9_RT	0xffff0000	/* Recieve watchdog timeout	*/

/*
 * TGEC Revision Number and Missed Frame Count (CSR10)
 *	pass 1 DIN=2, HRN=1, FRN=1
 */
#define TE_CSR10_MFC	0x0000ffff	/* Mask for Missed Frame Count  */
#define TE_CSR10_FRN	0x000f0000	/* Firmware Revision Number     */
#define TE_CSR10_HRN	0x00f00000	/* Hardware Revision Number     */
#define TE_CSR10_DIN	0xf0000000	/* Chip Identification Number   */

/* 
 * TGEC Boot Message (CSR11,12,13)
 */
#define TE_CSR13_PRC	0x0000007f	/* Boot Message Processor ID    */

/* 
 * TGEC Diagnostic Registers (CSR14,15)
 */

/* 
 * TGEC setup frame error indicator
 */
/* SDES0 */
#define TE_SETUP_SE	0x00002000	/* Setup frame length error	*/
#define TE_SETUP_ES	0x00008000	/* Setup frame length error	*/
#define TE_SETUP_OW	0x80000000	/* Ownership bit		*/
/* SDES1 */
#define TE_SETUP_IC	0x01000000	/* Interrupt on Completion 	*/
#define TE_SETUP_HP	0x02000000	/* Hash/Perfect Filtering	*/
#define TE_SETUP_IF	0x04000000	/* Inverse Filtering	 	*/
#define TE_SETUP_DT	0x30000000	/* Data Type (=2 for setup)	*/
/* SDES2 */
#define TE_SETUP_BS	0x7fff0000	/* Buffer Size		 	*/
/* SDES3 */
#define TE_SETUP_PA	0x3ffffffe	/* buffer physical address	*/

/*
 * Misc
 */
#define TE_CC_SHIFT 	3		/* Transmit Collision Count     */ 
#define TE_BL_SHIFT	24		/* Burst limit shift		*/
#define TE_HW_RE_SHIFT	20		/* Hardware Revision shift	*/
#define TE_FW_RE_SHIFT	16		/* Firmware Revision shift	*/
#define TE_BURST_SHIFT 	25		/* Burst limit shift		*/
#define TE_HEXAWORD     0x0000000f      /* octaword alignment		*/
#define TE_OCTAWORD     0x00000007      /* octaword alignment		*/
#define TE_LONGWORD     0x00000003      /* longword alignment		*/

#define TE_WORD     	0x00000001      /* word alignment		*/
#define TE_IPL_14       0x00000000      /* SGEC IPL 14 			*/
#define TE_IPL_15       0x40000000      /* SGEC IPL 15 			*/
#define TE_IPL_16       0x80000000      /* SGEC IPL 16 			*/
#define TE_IPL_17       0xc0000000      /* SGEC IPL 17  		*/
#define TE_BUS_ASYN     0x00000000      /* SGEC Asyn Mode  		*/
#define TE_BUS_SYN      0x20000000      /* SGEC Syn Mode   		*/


#define ETHADDR_CMD		0x00000400
#define ETHADDR_BYTE0		0x00000000L
#define ETHADDR_BYTE32		0x00000080L

/*
 * secondary bus address of TGEC registers (bits 6-2)
 */
#define CSR0_SADDR		0x00000000L
#define CSR1_SADDR		0x00000008L
#define CSR2_SADDR		0x00000010L
#define CSR3_SADDR		0x00000018L
#define CSR4_SADDR		0x00000020L
#define CSR5_SADDR		0x00000028L
#define CSR6_SADDR		0x00000030L
#define CSR7_SADDR		0x00000038L
#define CSR8_SADDR		0x00000040L
#define CSR9_SADDR		0x00000048L
#define CSR10_SADDR		0x00000050L
#define CSR11_SADDR		0x00000058L
#define CSR12_SADDR		0x00000060L
#define CSR13_SADDR		0x00000068L
#define CSR14_SADDR		0x00000070L
#define CSR15_SADDR		0x00000078L

/*
 * secondary bus address of TGEC unit (bits 7)
 */
#define TGEC_UNIT0_ADDR	0x00000000
#define TGEC_UNIT1_ADDR	0x00000080

/*
 * unit 0 bit 33
 * unit 1 bit 34
 */
#define LINT_TGEC0	0x0000000400000000
#define LINT_TGEC1	0x0000000200000000


#endif
