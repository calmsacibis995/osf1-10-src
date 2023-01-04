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
#ifndef _DKIO_H_
#define _DKIO_H_

#define NO_ACCESS -13	/* No access mscp command suport */

/* Structure for DKIOCDOP ioctl - disk operation command */
struct	dkop	{
	short	dk_op;			/* Operations defined below	*/
	daddr_t dk_count;		/* How many of them		*/
};

/* Structure for DKIOCGET ioctl - disk get status command */
struct	dkget	{
	short	dk_type;		/* Type of device defined below */
	short	dk_dsreg;		/* ``drive status'' register	*/
	short	dk_erreg;		/* ``error'' register		*/
	short	dk_resid;		/* Residual count		*/
};

/* IOCTL DKIOACC opcodes */
#define ACC_SCAN	1
#define ACC_CLEAR	2
#define ACC_REVEC	3
#define ACC_UNPROTECT	4
#define ACC_PRINT	5

/* Structure for DKIOCACC ioctl - disk access command */
struct	dkacc	{
	short	dk_opcode;		/* Operation code for access	*/
	long	dk_lbn;			/* Disk sector number		*/
	long	dk_length;		/* Length of transfer		*/
	unsigned dk_status;		/* Status of operation		*/
	unsigned dk_flags;		/* Status flags			*/
};

#endif
