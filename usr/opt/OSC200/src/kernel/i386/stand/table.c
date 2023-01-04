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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: table.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:15:39 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */

/* 
 * Mach Operating System
 * Copyright (c) 1990 Carnegie-Mellon University
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * 			INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *	This software is supplied under the terms of a license  agreement or 
 *	nondisclosure agreement with Intel Corporation and may not be copied 
 *	nor disclosed except in accordance with the terms of that agreement.
 *
 *	Copyright 1988 Intel Corporation
 * Copyright 1988, 1989 by Intel Corporation
 */

#define NGDTENT		6
#define GDTLIMIT	48	/* NGDTENT * 8 */

/*  Segment Descriptor
 *
 * 31          24         19   16                 7           0
 * ------------------------------------------------------------
 * |             | |B| |A|       | |   |1|0|E|W|A|            |
 * | BASE 31..24 |G|/|0|V| LIMIT |P|DPL|  TYPE   | BASE 23:16 |
 * |             | |D| |L| 19..16| |   |1|1|C|R|A|            |
 * ------------------------------------------------------------
 * |                             |                            |
 * |        BASE 15..0           |       LIMIT 15..0          |
 * |                             |                            |
 * ------------------------------------------------------------
 */

struct seg_desc {
	unsigned short	limit_15_0;
	unsigned short	base_15_0;
	unsigned char	base_23_16;
	unsigned char	bit_15_8;
	unsigned char	bit_23_16;
	unsigned char	base_31_24;
	};


struct seg_desc	Gdt[NGDTENT] = {
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0},		/* 0x0 : null */
	{0xFFFF, 0x1000, 0x0, 0x9E, 0x40, 0x0},	/* 0x8 : boot code */
	{0xFFFF, 0x1000, 0x0, 0x92, 0x40, 0x0},	/* 0x10 : boot data */
	{0xFFFF, 0x1000, 0x0, 0x9E, 0x0, 0x0},	/* 0x18 : boot code, 16 bits */
	{0xFFFF, 0x0, 0x0, 0x92, 0xCF, 0x0},	/* 0x20 : init data */
	{0xFFFF, 0x0, 0x0, 0x9E, 0xCF, 0x0}	/* 0x28 : init code */
	};


struct pseudo_desc {
	unsigned short	limit;
	unsigned short	base_low;
	unsigned short	base_high;
	};

struct pseudo_desc Gdtr = { GDTLIMIT, 0x1200, 0 }; 
			/* boot is loaded at 4k, Gdt is located at 4k+512 */
