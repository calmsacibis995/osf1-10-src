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
 *	@(#)$RCSfile: if_pc586.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:08:47 $
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/* 
 * OSF/1 Release 1.0
 */
 
/* 
 *	Olivetti PC586 Mach Ethernet driver v1.0
 *	Copyright Ing. C. Olivetti & C. S.p.A. 1988, 1989
 *	All rights reserved.
 *
 */ 

/*
               All Rights Reserved

   Permission to use, copy, modify, and distribute this software and
 its documentation for any purpose and without fee is hereby
 granted, provided that the above copyright notice appears in all
 copies and that both the copyright notice and this permission notice
 appear in supporting documentation, and that the name of Olivetti
 not be used in advertising or publicity pertaining to distribution
 of the software without specific, written prior permission.

   OLIVETTI DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
 INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
 IN NO EVENT SHALL OLIVETTI BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
 CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
 NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUR OF OR IN CONNECTION
 WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 *         INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *     This software is supplied under the terms of a license 
 *    agreement or nondisclosure agreement with Intel Corpo-
 *    ration and may not be copied or disclosed except in
 *    accordance with the terms of that agreement.
 *    Copyright 1983, 1984, 1986, 1987  Intel Corporation.
 */

#define	ushort	u_short
#define uchar   u_char

#include	<i386/AT386/i82586.h>	/* chip/board specific defines	*/

#define	STATUS_TRIES	15000
#define	ETHER_ADD_SIZE	6	/* size of a MAC address 		*/
#define ETHER_PCK_SIZE	1500	/* maximum size of an ethernet packet 	*/

/*
 * 	Board Specific Defines:
 */

#define OFFSET_NORMMODE	0x3000
#define OFFSET_CHANATT	0x3002
#define OFFSET_RESET   	0x3004
#define OFFSET_INTENAB	0x3006
#define OFFSET_XFERMODE	0x3008
#define OFFSET_SYSTYPE	0x300a
#define OFFSET_INTSTAT	0x300c
#define OFFSET_PROM	0x2000 

#define EXTENDED_ADDR	0x20000
#define OFFSET_SCP      0x7ff6
#define OFFSET_ISCP     0x7fee
#define OFFSET_SCB      0x7fde
#define OFFSET_RU       0x4000
#define OFFSET_RBD	0x4228
#define OFFSET_CU       0x7814

#define OFFSET_TBD      0x7914
#define OFFSET_TBUF     0x7924
#define N_FD              25  
#define N_RBD             25 
#define RCVBUFSIZE       532
#define DL_DEAD         0xffff

#define CMD_0           0     
#define CMD_1           0xffff

#define PC586NULL	0xffff			/* pc586 NULL for lists	*/

#ifndef TRUE
#define TRUE            1     
#endif 	TRUE

#define	DSF_LOCK	1
#define DSF_RUNNING	2
#define	DSF_SETADDR	3

#define MOD_ENAL 1
#define MOD_PROM 2

/*
 * Driver (not board) specific defines and structures:
 */

typedef union {
		char	a[4];
		unsigned long b;
}  pack_ulong_t;

#define ubyte	unsigned char

typedef	struct	{
	rbd_t	r;
	char	rbd_pad[2];
	char	rbuffer[RCVBUFSIZE];
} ru_t;

