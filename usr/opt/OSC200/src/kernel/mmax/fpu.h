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
 *	@(#)$RCSfile: fpu.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:40:40 $
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
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */

/*

 *
 */



#define FSR_TT		0x00000007	/* Trap type */
#define FSR_NOTRAP	0		/* No trap (glitch?) */
#define FSR_FLTUND	1		/* Floating underflow */
#define FSR_FLTOVF	2		/* Floating overflow */
#define FSR_FLTDIV	3		/* Floating divide by zero */
#define FSR_ILLINST	4		/* Illegal FPU instruction */
#define FSR_INVLOP	5		/* Invalid operation */
#define FSR_INEXACT	6		/* Inexact result */
#define FSR_OPRNDERR	7		/* Operand Error */

#define FSR_RM_NEAREST	0		/* Round to nearest value */
#define FSR_RM_TRUNC	0x080		/* Round by truncating */
#define FSR_RM_UP	0x100		/* Round upward */
#define FSR_RM_DOWN	0x180		/* Round downward */
#define FSR_UEN_ENABLE	0x008		/* Underflow trap enable */
#define FSR_IEN_ENABLE  0x020		/* Inexact result trap enable */
