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
 * the CPYBF addresses need to be
 * aligned to a 32-byte boundary.
 *
 * NOTE: if changing register addresses here, remember
 * to make the corresponding change in "sfbcontrol.eqn"
 */
#define CPYBF0_ADDRESS    0x100000
#define CPYBF1_ADDRESS    0x100004
#define CPYBF2_ADDRESS    0x100008
#define CPYBF3_ADDRESS    0x10000c
#define CPYBF4_ADDRESS    0x100010
#define CPYBF5_ADDRESS    0x100014
#define CPYBF6_ADDRESS    0x100018
#define CPYBF7_ADDRESS    0x10001c
#define FG_ADDRESS        0x100020
#define BG_ADDRESS        0x100024
#define PLANEMASK_ADDRESS 0x100028
#define PIXMSK_ADDRESS    0x10002c
#define MODE_ADDRESS	  0x100030
#define BOOLOP_ADDRESS	  0x100034
#define PIXSHFT_ADDRESS   0x100038
#define ADDRREG_ADDRESS   0x10003c
#define BRES1_ADDRESS     0x100040
#define BRES2_ADDRESS     0x100044
#define BRES3_ADDRESS     0x100048
#define BCONT_ADDRESS     0x10004c
#define DEEP_ADDRESS      0x100050
#define START_ADDRESS     0x100054

#define REGMASK		0xffff80
#define REGVAL		0x100000

#define NOP	0
#define READ	1
#define WRITE	2
#define FLUSH	3
#define SHIFT	4

#define COLMASK		(0x1ff)
#define ROWMASK		(~COLMASK)

