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
 * Modification History: /sys/machine/alpha/psl.h
 *
 * 10-Sep-90 -- rjl
 *	Created this file for Alpha support.
 */

#ifndef _PSL_H_
#define _PSL_H_

#ifdef	ASSEMBLER
#define PSL_CURMOD	8			/* k/u			*/
#define PSL_IPL		7			/* mask for 3 ipl bits	*/
#define PSL_K		0			/* in bit[3], k=0	*/
#define PSL_U		8			/*       and  u=1	*/
#else
#define PSL_CURMOD	0x8L			/* k/u			*/
#define PSL_IPL		0x7L			/* mask for 3 ipl bits	*/
#define PSL_K		0x0L			/* in bit[3], k=0	*/
#define PSL_U		0x8L			/*       and  u=1	*/
#endif
#define PSL_IPL_LOW	0	/* level used for no hw/sw intr lockout */

#endif
