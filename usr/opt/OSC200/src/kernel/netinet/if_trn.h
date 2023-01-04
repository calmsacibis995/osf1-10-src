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
 * @(#)$RCSfile: if_trn.h,v $ $Revision: 1.1.3.5 $ (DEC) $Date: 1993/10/06 13:51:45 $
 */
/* ---------------------------------------------------------------------
 *
 * Modification History:
 * 17-Dec-92 Uttam Shikarpur
 * 	Added Julia's changes for source routing.
 *
 * 13-Nov-92	Uttam Shikarpur	
 * 	Created module for Token Ring.
 * ------------------------------------------------------------------- */

#ifndef _IF_TOKENRING_H_DEFINED
#define _IF_TOKENRING_H_DEFINED

#define MAC_ADDR_LEN	6		/* Token Ring MAC address length */
#define SR_RD_SIZE	8		/* TRN SR Route Designator size */
#define RIIndicator	0x01		/* RII bit - canonical form */

/*
 * Structure  of a Token Ring header.
 */
struct	trn_preamble {
	u_char  trn_ac;	 
	u_char	trn_fc;
	u_char	trn_dhost[MAC_ADDR_LEN];
	u_char	trn_shost[MAC_ADDR_LEN];
};

/*
 * Source Routing Information structure
 */
  struct  trn_rif {
	  u_char  rif_lth:5,
	   	  rif_rt:3;
	  u_char  rif_res:4,
		  rif_lf:3,
		  rif_dir:1;
	  u_short rif_rd[SR_RD_SIZE];
};

/*
 * Structure of a Token Ring header.
 */
  struct  trn_header {                /* Extended (with SR Info) header */
	u_char  trn_ac;	 
	u_char	trn_fc;
	u_char	trn_dhost[MAC_ADDR_LEN];
	u_char	trn_shost[MAC_ADDR_LEN];
	struct trn_rif trn_hdr_rif;  /* Routing Information */
};

/*
 * These are the maximum MTUs that can be supported on a 4 and 16 Mb/s token
 * ring support(for IP) - see RFC 1042.
 */
#define TRN4_RFC1042_IP_MTU   4092
#define TRN16_RFC1042_IP_MTU  8188
#define TRN4_RFC1042_MAC_MTU  4136
#define TRN16_RFC1042_MAC_MTU 8232
#define TRN4_MAX_MTU  4472
#define TRN16_MAX_MTU 17800

/* 
 * The Largest Frame - for multi-ring network Routing Information Field
 * Per P802.5PD-2
 */
#define TRN_LF_516		0x0
#define TRN_LF_1500		0x1	/* TRN_LF_1470 */
#define TRN_LF_2052		0x2
#define TRN_LF_4472		0x3	/* TRN_LF_4399 */
#define TRN_LF_8144		0x4	/* TRN_LF_8130 */
#define TRN_LF_11407		0x5
#define TRN_LF_17800		0x6	/* TRN_LF_17749 */
#define TRN_LF_65535		0x7	/* TRN_LF_41600 */

/*
 * Values used for access control and frame control fields.
 */
#define TRN_AC	 	0x00
#define TRN_FC	 	0x40
#define TRN_FRAME     	0x0e

/*
 * TRN Bandwidths.
 */
#define TRN_BANDWIDTH_4MB          40000000        /* Token ring - 4Mbs */
#define TRN_BANDWIDTH_16MB         16000000        /* Token ring - 16Mbs */

#endif /* _IF_TOKENRING_H_DEFINED */
