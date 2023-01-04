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
 *	@(#)$RCSfile: debugf.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:13:02 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1990
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
#ifndef _H_DEBUGF
#define _H_DEBUGF
/*
 * COMPONENT_NAME: INC/SYS debugf.h Debugging Utility Define's
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#ifdef DEBUG
#define DEBUGF(cond,stmt) if (cond) stmt	/* do the stmt (printf) */
#else
#define DEBUGF(cond,stmt) 		/* do nothing if not debugging */
#endif
/*
 * Usage:
 * DEBUGF(fddebug & DEBUG_OPEN,
 *	printf("fdopen: Entering, Hi, I am %s, %d 0x%x\n", here, dec, hex));
 */
#define DEBUG_ALL		0xffffffff
#define DEBUG_NONE		0x00000000

/*
 * Bit 0-15 are used to debug entry points found in most device drivers.
 * Bit 16-30 are used to debug component specific routines.
 * Bit 31 is used to debug error conditions.
 */
#define DEBUG_CONFIG		BIT0
#define DEBUG_OPEN		BIT1
#define DEBUG_CLOSE		BIT2
#define DEBUG_READ		BIT3
#define DEBUG_WRITE		BIT4
#define DEBUG_IOCTL		BIT5
#define DEBUG_STRATEGY		BIT6
#define DEBUG_SELECT		BIT7
#define DEBUG_PRINT		BIT8
#define DEBUG_DUMP		BIT9
#define DEBUG_MPX		BIT10
#define DEBUG_REVOKE		BIT11
#define DEBUG_INTR		BIT12
#define DEBUG_BIT13		BIT13		/* still free */
#define DEBUG_BIT14		BIT14		/* still free */
#define DEBUG_ABIOS		BIT15

#define DEBUG_ERR_COND		BIT31

#ifdef DEBUG
extern void debug_handy_brkpt(char * msg);
#else
#define debug_handy_brkpt(msg)
#endif
/*
 * Usage: use debugger to set a break point at debug_handy_brkpt(),
 *	  put this line in your program where you wish to stop:
 * debug_handy_brkpt("I am here");
 */

#ifdef DEBUG
extern void dump_abios_rb(caddr_t addr, int flag);
#else
#define dump_abios_rb(addr, flag)
#endif
/*
 * Usage: dump content of abios request block.
 * dump_abios_rb((caddr_t)&this_request_block, DUMP_HEADER_ONLY);
 * or
 * dump_abios_rb((caddr_t)&this_request_block, DUMP_ENTIRE_BLOCK);
 */
#define DUMP_HEADER_ONLY		BIT0
#define DUMP_ENTIRE_BLOCK		BIT1

/*
 * Little-Endian bit convention
 */
#define BIT0				0x00000001
#define BIT1				0x00000002
#define BIT2				0x00000004
#define BIT3				0x00000008
#define BIT4				0x00000010
#define BIT5				0x00000020
#define BIT6				0x00000040
#define BIT7				0x00000080
#define BIT8				0x00000100
#define BIT9				0x00000200
#define BIT10				0x00000400
#define BIT11				0x00000800
#define BIT12				0x00001000
#define BIT13				0x00002000
#define BIT14				0x00004000
#define BIT15				0x00008000
#define BIT16				0x00010000
#define BIT17				0x00020000
#define BIT18				0x00040000
#define BIT19				0x00080000
#define BIT20				0x00100000
#define BIT21				0x00200000
#define BIT22				0x00400000
#define BIT23				0x00800000
#define BIT24				0x01000000
#define BIT25				0x02000000
#define BIT26				0x04000000
#define BIT27				0x08000000
#define BIT28				0x10000000
#define BIT29				0x20000000
#define BIT30				0x40000000
#define BIT31				0x80000000

#endif /* _H_DEBUGF */
