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
 *	@(#)$RCSfile: cpudefs.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:40:15 $
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
 * Copyright (c) 1989 Encore Computer Corporation
 */

#ifndef _CPUDEFS_H
#define	_CPUDEFS_H

/*
 * The slave class is set to zero so that the slaves will only get directed
 * interrupts.  The master cpu will therefore get all undirected device
 * interrupts.
 */

#define SLAVE_CLASS	0
#define MASTER_CLASS	1

#ifdef	KERNEL
#include <mmax_dpc.h>
#include <mmax_apc.h>
#include <mmax_xpc.h>
#endif	KERNEL

#if	MMAX_DPC
#include <mmax/dpcdefs.h>
#endif	MMAX_DPC

#if	MMAX_APC
#include <mmax/apcdefs.h>
#endif	MMAX_APC

#if	MMAX_XPC
#include <mmax/xpcdefs.h>
#endif	MMAX_XPC

#if	MMAX_XPC || MMAX_APC
/*
 * Bit defines for ICU registers.
 */

/*
 * HVCT register
 */
#define ICUHVCT_VEC     0x0f
#define ICUHVCT_BIAS    0xf0

/*
 * SVCT register
 */
#define ICUSVCT_VEC     0x0f
#define ICUSVCT_BIAS    0xf0
#endif	MMAX_XPC || MMAX_APC

#endif	_CPUDEFS_H
