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

#ifndef XLAMBREG_H
#define XLAMBREG_H

#define LAMB_NODE_ID    0xf0000000

struct lambreg {
	/* xmi specific registers */
	unsigned int xdev;                    /* device type reg */
	unsigned int xber;                    /* bus error */
	unsigned int xfadr;                   /* fail addr */
	char         pad_0[32];
	unsigned int xfaer;                   /* failing extension */
	char         pad_1[16];
	/* lamb registers */
	unsigned int ldiag;                   /* lamb diag */
	unsigned int imsk;                    /* lamb int mask */
	unsigned int levr;                    /* lamb err vector */
	unsigned int lerr;                    /* lamb error */
	unsigned int lgpr;                    /* general purpose */
	unsigned int ipr1;                    /* int pending 1 */
	unsigned int ipr2;                    /* int pending 2 */
	unsigned int iipr;                    /* int in progress */
};

/* lamb error register (lerr) bits */
#define LERR_XMIPE    0xe0000000    /* faild parity bit if xber<23> == 1, RO */
#define LERR_DHDPE    0x10000000    /* down hose parity err <28>, w1c */
#define LERR_IVID     0x00078000    /* ivintr source node id, RO */
#define LERR_MBPE     0x00004000    /* mail box parity err <14>, w1c */
#define LERR_MBIC     0x00002000    /* mail box illegal command <13>, w1c */
#define LERR_MBIA     0x00001000    /* mail box illegal address <12>, w1c */
#define LERR_DFDPE    0x00000040    /* data fifo data field par err <6>, w1c */
#define LERR_RBDPE    0x00000020    /* read buffer data par error <5>, w1c */
#define LERR_MBOF     0x00000010    /* mail box overflow <4>, w1c */
#define LERR_FE       0x00000008    /* fatal error <3>, w1c */

/* lamb interrupt mask register (imsk) bits */
#define IMSK_ICC        0x08000000
#define IMSK_IWEI       0x02000000
#define IMSK_IIPE       0x01000000
#define IMSK_IXPE       0x00800000
#define IMSK_IWSE       0x00400000
#define IMSK_IRIDNAK    0x00200000
#define IMSK_IWDNAK     0x00100000
#define IMSK_ICRD       0x00080000
#define IMSK_INRR       0x00040000
#define IMSK_IRSE       0x00020000
#define IMSK_IRER       0x00010000
#define IMSK_ICNAK      0x00008000
#define IMSK_ITTO       0x00002000
#define IMSK_IDFDPE     0x00000040
#define IMSK_IRBDPE     0x00000020
#define IMSK_IMBER      0x00000010

#endif /* XLAMBREG_H */
