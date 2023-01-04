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
/* ------------------------------------------------------------------------
 * Modification History: /sys/io/xmi/sspxmi.h
 *
 *	20-Jul-89 -- map (Mark Parenti)
 *		Original version
 *
 * ------------------------------------------------------------------------
 */

#ifndef _SSPXMI_H_
#define _SSPXMI_H_

/*	SSP PD Interrupt Level - Used for XMI				*/
#define	UQ_XMI_LEVEL14	0x10000		/* XMI Interrupt level 14 */
#define	UQ_XMI_LEVEL15	0x20000		/* XMI Interrupt level 15 */
#define	UQ_XMI_LEVEL16	0x40000		/* XMI Interrupt level 16 */
#define	UQ_XMI_LEVEL17	0x80000		/* XMI Interrupt level 17 */

/*	SSP PSI(Page Size Indicator) - Used for XMI			*/
#define	SSP_PSI_512	0	/* 512 byte pages			*/
#define	SSP_PSI_1024	1	/* 1024 byte pages			*/
#define	SSP_PSI_2048	2	/* 2048 byte pages			*/
#define	SSP_PSI_4096	3	/* 4096 byte pages			*/
#define	SSP_PSI_8192	4	/* 8192 byte pages			*/

/*	SSP PFN Mask Value - Used for XMI				*/
#define	SSP_PFN_MASK	0x1FF	/* 25 bits of PFN are significant	*/
                                /* We get 16 by default			*/


#endif
