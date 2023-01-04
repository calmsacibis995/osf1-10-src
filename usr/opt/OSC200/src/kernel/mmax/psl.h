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
 *	@(#)$RCSfile: psl.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:42:38 $
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
 *     Copyright (C) 1984 Hydra Computer Systems, Inc.
 * ALL RIGHTS RESERVED. Licensed Material - Property of Hydra 
 * Computer Systems, Inc. This software is made available solely 
 * pursuant to the terms of a software license agreement which governs 
 * its use. Unauthorized duplication, distribution or sale are strictly 
 * prohibited.
 *
 * Original Author: Tony Anzelmo	Created on: 12/19/84
 *
 */


/*
 * Processor Status Register bit definitons as bit numbers and as masks
 */

#define PSR_C_BIT	0
#define PSR_T_BIT	1
#define PSR_L_BIT	2
#define PSR_V_BIT	4
#define PSR_F_BIT	5
#define PSR_Z_BIT	6
#define PSR_N_BIT	7
#define PSR_U_BIT	8
#define PSR_S_BIT	9
#define PSR_P_BIT	10
#define PSR_I_BIT	11

#define PSR_C	0x0001
#define PSR_T	0x0002
#define PSR_L	0x0004
#define	PSR_V	0x0010
#define PSR_F	0x0020
#define PSR_Z	0x0040
#define PSR_N	0x0080
#define PSR_U	0x0100
#define PSR_S	0x0200
#define PSR_P	0x0400
#define PSR_I	0x0800

#define PSR_ALLCC	(PSR_C|PSR_L|PSR_F|PSR_Z|PSR_N)
#define PSR_RESERVED	0xf018

/* all PSL definitions are for a psrmod pair (psr in high order word) */
#define PSL_C	((1 << PSR_C_BIT) << 16)
#define PSL_T	((1 << PSR_T_BIT) << 16)
#define PSL_L	((1 << PSR_L_BIT) << 16)
#define PSL_F	((1 << PSR_F_BIT) << 16)
#define PSL_Z	((1 << PSR_Z_BIT) << 16)
#define PSL_N	((1 << PSR_N_BIT) << 16)
#define PSL_U	((1 << PSR_U_BIT) << 16)
#define PSL_S	((1 << PSR_S_BIT) << 16)
#define PSL_P	((1 << PSR_P_BIT) << 16)
#define PSL_I	((1 << PSR_I_BIT) << 16)

#define PSL_SUPRBITS	0xff000000
#define PSL_USERBITS	0x00ffffff

#define PSL_ALLCC	(PSR_ALLCC << 16)
#define PSL_RESERVED	(PSR_RESERVED << 16)

#define PSL_USERSET	(PSL_U|PSL_S|PSL_I)
#define PSL_USERCLR	(PSL_P|PSL_RESERVED)
