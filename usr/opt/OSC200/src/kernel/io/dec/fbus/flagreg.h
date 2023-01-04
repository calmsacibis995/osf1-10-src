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

#ifndef FLAGREG_H
#define FLAGREG_H

	/* FLAG CSR's */
#define FLAG_BASE 	0xfffca000
#define FLAG_FINT	0xfffca000
#define FLAG_FCTL_V_ERRINT_ENABLE	0xf7dfe
#define FLAG_FCTL_V_ERRINT_DISABLE	0xf7c0e
#define FLAG_NID	0xfffca004
#define FLAG_STO	0xfffca008
#define FLAG_ERRHI	0xfffca00C
#define FLAG_ERRHI_V_NXA	0x04000000
#define FLAG_ERRHI_V_CMD_PE	0x01000000
#define FLAG_ERRHI_V_DA_PE	0x00800000
#define FLAG_ERRHI_V_PROTO_ERR	0x00400000
#define FLAG_ERRHI_V_USE	0x00080000
#define FLAG_ERRHI_V_MASK	\
	( FLAG_ERRHI_V_NXA | FLAG_ERRHI_V_CMD_PE | FLAG_ERRHI_V_DA_PE \
	  | FLAG_ERRHI_V_PROTO_ERR | FLAG_ERRHI_V_USE )
#define FLAG_ERRLO	0xfffca010
#define FLAG_FADDRHI	0xfffca014
#define FLAG_FADDRLO	0xfffca018
#define FLAG_TTO	0xfffca01C
#define FLAG_BZRTRY	0xfffca020
#define FLAG_FCTL	0xfffca024
#define FLAG_DIAG	0xfffca028
#define FLAG_FGPR	0xfffca02C
#define FLAG_FERR	0xfffca030
#define FLAG_IBR	0xfffca034
#endif /* FLAGREG_H */
