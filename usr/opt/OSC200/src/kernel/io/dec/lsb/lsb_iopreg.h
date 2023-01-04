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
/*	"@(#)lsb_iopreg.h	9.2	(ULTRIX/OSF)	10/23/91" */

#ifndef LSB_IOPREG_H
#define LSB_IOPREG_H


#define bus_node private[1]

#ifdef DEC7000
#define nHOSES 4
#define MAX_HOSE (nHOSES - 1)
#endif /* DEC7000 */

/* remote adapters */
#define	LAMB    0x102a
#define FLAG	0x2f00

struct iopreg {
	/* LASER required registers */
	unsigned int ldev;             /* device type */
	char         pad_1[60];
	unsigned int lber;             /* bus error */
	char         pad_2[60];
	unsigned int lcnr;             /* config reg */
	char         pad_3[1404];
	unsigned int lbesr0;           /* bus ecc error syndrome (0-3) */
	char         pad_4[60];
	unsigned int lbesr1;
	char         pad_5[60];
	unsigned int lbesr2;
	char         pad_6[60];
	unsigned int lbesr3;
	char         pad_7[60];
	unsigned int lbecr0;            /* bus err command (0-1) */
	char         pad_8[60];
	unsigned int lbecr1;
	char         pad_9[700];
	unsigned int lild0;             /* int level 0 ident */
	char         pad_10[60];
	unsigned int lild1;             /* int level 1 ident */
	char         pad_11[60];
	unsigned int lild2;             /* int level 2 ident */
	char         pad_12[60];
	unsigned int lild3;             /* int level 3 ident */
	char         pad_13[60];
	unsigned int lcpumask;          /* cpu int mask */
	char         pad_14[252];
	unsigned int lmbpr;             /* mailbox */
	char         pad_15[5116];
	/* IOP specific registers */
	unsigned int ipcnse;            /* port chip err */
	char         pad_16[60];
	unsigned int ipcvr;             /* port chip int vector */
	char         pad_17[60];
	unsigned int ipcmsr;            /* port chip mode select */
	char         pad_18[60];
	unsigned int ipchst;            /* port chip hose status */
	char         pad_19[60];
	unsigned int ipcdr;             /* port chip diag */
	char         pad_21[7868];
};

/* ipcnse register bit definitions */
#define INTR_NSES       0x80000000
#define MULT_INTR_ERR   0x00100000
#define DN_VRTX_ERR     0x00080000
#define UP_VRTX_ERR     0x00040000
#define IPC_IE          0x00020000
#define UP_HIC_IE       0x00010000
#define UP_HOSE_PAR_ERR 0x0000f000
#define UP_HOSE_PKT_ERR 0x00000f00
#define UP_HOSE_OFLO    0x000000f0
#define MBX_TIP         0x0000000f

/* some ipcnse bit pos shifts */
#define UP_HOSE_PAR_SHFT     12
#define UP_HOSE_PKT_SHFT     8
#define UP_HOSE_OFLO_SHFT    4

/* ipchst register bit definitions */
#define IPCHST_HOSE_STAT  0xF
#define IPCHST_HOSE_RESET 0x10000000

/* hose status bits */
#define IPCHST_PWROKT    0x8           /* power ok transitioned */
#define IPCHST_CBLOK     0x4           /* cblok current */
#define IPCHST_PWROKC    0x2           /* power ok current */
#define IPCHST_ERROR     0x1           /* error transitioned */

#endif /* LSB_IOPREG_H */
