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
 *	@(#)$RCSfile: cfg.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:39:48 $
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
 * Copyright 1989 Encore Computer Corporation
 */
/*
 * cfg.h:  ns32532 configuration register definitions.
 */


#define CFG_I	0x0001		/* Interrupt Vectoring (ICU present).	    */
#define CFG_F	0x0002		/* Floating-Point Instruction set enable.   */
#define CFG_M	0x0004		/* MMU instruction set enable.		    */
#define CFG_C	0x0008		/* Custom instruction set enable.	    */
#define CFG_DE	0x0100		/* Direct-Exception mode enable.	    */
#define CFG_DC	0x0200		/* Data Cache enable.			    */
#define CFG_LDC	0x0400		/* Lock Data Cache.			    */
#define CFG_IC	0x0800		/* Instruction Cache enable.		    */
#define CFG_LIC	0x1000		/* Lock Instruction Cache.		    */
#define CFG_PF	0x2000		/* FPU Instruction FIFO enable.		    */

#define CFG_I_BIT	0	/* Interrupt Vectoring (ICU present).	    */
#define CFG_F_BIT	1	/* Floating-Point Instruction set enable.   */
#define CFG_M_BIT	2	/* MMU instruction set enable.		    */
#define CFG_C_BIT	3	/* Custom instruction set enable.	    */
#define CFG_DE_BIT	8	/* Direct-Exception mode enable.	    */
#define CFG_DC_BIT	9	/* Data Cache enable.			    */
#define CFG_LDC_BIT	10	/* Lock Data Cache.			    */
#define CFG_IC_BIT	11	/* Instruction Cache enable.		    */
#define CFG_LIC_BIT	12	/* Lock Instruction Cache.		    */
#define CFG_PF_BIT	13	/* FPU Instruction FIFO enable.		    */

#ifndef	LOCORE
typedef union xpc_cfg {
    struct {
	unsigned int
	    cfg_i	:1,
	    cfg_f	:1,
	    cfg_m	:1,
	    cfg_c	:1,
			:4,
	    cfg_de	:1,
	    cfg_dc	:1,
	    cfg_ldc	:1,
	    cfg_ic	:1,
	    cfg_lic	:1,
	    cfg_pf	:1,
			:18;
    } f;
    unsigned int
	l;
} xpc_cfg_t;
#endif	LOCORE
