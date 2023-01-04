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
 /***********************************************************************
  * kzqreg.h	04/11/89
  *
  * Modification History
  *
  * 06-Jun-90   Charles Richmond - cloned siireg.h into kzqreg.h and
  *		added the kzq_ registers.
  *
  ***********************************************************************/

#ifndef _KZQREG_H_
#define _KZQREG_H_
/*
 * SII and KZQSA registers
 */
struct kzq_regs
{
	u_short sii_sdb;	/* SCSI Data Bus and Parity		*/
	u_short sii_sc1;	/* SCSI Control Signals One		*/
	u_short sii_sc2;	/* SCSI Control Signals Two		*/
	u_short sii_csr;	/* Control/Status register		*/
	u_short sii_id;		/* Bus ID register			*/
	u_short sii_slcsr;	/* Select Control and Status Register	*/
	u_short sii_destat;	/* Selection Detector Status Register	*/
	u_short sii_dstmo;	/* DSSI Timeout Register		*/
	u_short sii_data;	/* Data Register			*/
	u_short sii_dmctrl;	/* DMA Control Register			*/
	u_short sii_dmlotc;	/* DMA Length of Transfer Counter	*/
	u_short sii_dmaddrl;	/* DMA Address Register Low		*/
	u_short sii_dmaddrh;	/* DMA Address Register High		*/
	u_short sii_dmabyte;	/* DMA Initial Byte Register		*/
	u_short sii_stlp;	/* DSSI Short Target List Pointer	*/
	u_short sii_ltlp;	/* DSSI Long Target List Pointer	*/
	u_short sii_ilp;	/* DSSI Initiator List Pointer		*/
	u_short sii_dsctrl;	/* DSSI Control Register		*/
	u_short sii_cstat;	/* Connection Status Register		*/
	u_short sii_dstat;	/* Data Transfer Status Register	*/
	u_short sii_comm;	/* Command Register			*/
	u_short sii_dictrl;	/* Diagnostic Control Register		*/
	u_short sii_clock;	/* Diagnostic Clock Register		*/
	u_short sii_bhdiag;	/* Bus Handler Diagnostic Register	*/
	u_short sii_sidiag;	/* SCSI IO Diagnostic Register		*/
	u_short sii_dmdiag;	/* Data Mover Diagnostic Register	*/
	u_short sii_mcdiag;	/* Main Control Diagnostic Register	*/
	u_short kzq_dmacsr;	/* DMA Control/Status Reg. for KZQSA	*/
	u_short kzq_qbar;	/* Qbus Address (high 5 bits in dmacsr) */
	u_short kzq_lbar;	/* Local Address for Qbus DMA.		*/
	u_short kzq_wc;		/* Word Count for DMA.			*/
	u_short kzq_vector;	/* Intr vector & 128k memory base.	*/
};
#define KZQ_REG                register volatile struct kzq_regs

#define KZQ_WAIT_COUNT		10000   /* Delay count used for the SII chip  */
#define KZQ_MAX_DMA_XFER_LENGTH	8192  	/* Max DMA transfer length for SII    */
#define QVEC_BCNT    0x80    /* The number of vectors possible       */

#endif
