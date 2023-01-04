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
 *	@(#)$RCSfile: fixpoint.h,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:08:37 $
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
 * derived from fixpoint.h	2.1	(ULTRIX/OSF)	12/3/90
 */

/*		2.1	fixpoint.h	*/

/*
 * Copyright 1985 by MIPS Computer Systems, Inc.
 */

/*
 * Fix-point arithmetic package
 */

/*
 * Basic fix-point types
 */
/*
 * TODO: should probably move this over to types.h so that avenrun is
 * not defined in vm_sched.c
 */
typedef	int 		fix;
typedef	unsigned int	ufix;

/*
 * Number of fraction bits.
 */
#define FBITS		8

/*
 * Conversion to fix-point representation
 * works with int, float, double, char, ....
 */
#define	TO_FIX(x)	((fix)((x)*(1<<FBITS)))

/*
 * Conversion from fix-point to various integer datatypes
 */
#define	FIX_TO_SHORT(x)		((short)((x)>>FBITS))
#define	FIX_TO_INT(x)		((int)((x)>>FBITS))

/*
 * Conversion from fix-point to double
 */
#define	FIX_TO_DBL(x)	(((double)(x))/(1<<FBITS))

/*
 * Multiplication/division of 2 fix-point values
 */
#define	MUL_2FIX(x, y)	(((x)*(y))>>FBITS)
#define	DIV_2FIX(x, y)	(((x)<<FBITS)/(y))
