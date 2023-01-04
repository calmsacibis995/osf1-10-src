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

#ifndef	_MACH_ALPHA_EXCEPTION_H_
#define	_MACH_ALPHA_EXCEPTION_H_

/*
 * Codes and subcodes for Alpha exceptions.
 */

/*
 * Arithmetic codes.
 */
#define EXC_ALPHA_FLT_COMPLETE	00L
#define EXC_ALPHA_FLT_INVALID	01L
#define EXC_ALPHA_FLT_DIVIDE0	02L
#define EXC_ALPHA_FLT_FOVERFLOW	03L
#define EXC_ALPHA_FLT_UNDERFLOW	04L
#define EXC_ALPHA_FLT_INEXACT	05L
#define EXC_ALPHA_FLT_IOVERFLOW	06L
#define	EXC_ALPHA_INT_DIVIDE0	07L	/* integer divide by 0 */
#define	EXC_ALPHA_INT_OVERFLOW	010L	/* integer overflow */

/* 
 *	Codes to go with EXC_SOFTWARE
 */
#define	EXC_ALPHA_SOFT_STK	0x10003L
#define	EXC_ALPHA_SOFT_SEGV	0x10004L
#define EXC_ALPHA_INST_TRAP	0x10005L

/*
 *	EXC_BREAKPOINT subcodes
 */
#define	EXC_ALPHA_BPT			1L
#define EXC_ALPHA_TRACE			2L

#endif	/* _MACH_ALPHA_EXCEPTION_H_ */
