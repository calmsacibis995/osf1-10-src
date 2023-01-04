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
 *	@(#)$RCSfile: mach_error.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:16:56 $
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

#ifndef	_LIB_MACH_ERROR_H_
#define	_LIB_MACH_ERROR_H_	1

#include <mach/error.h>

char		*mach_error_string(
/*
 *	Returns a string appropriate to the error argument given
 */
#if	c_plusplus
	mach_error_t error_value
#endif	/* c_plusplus */
				);

void		mach_error(
/*
 *	Prints an appropriate message on the standard error stream
 */
#if	c_plusplus
	char 		*str,
	mach_error_t	error_value
#endif	/* c_plusplus */
				);

char		*mach_error_type(
/*
 *	Returns a string with the error system, subsystem and code
*/
#if	c_plusplus
	mach_error_t	error_value
#endif /*  c_plusplus */
				);

#endif	/* _LIB_MACH_ERROR_H_ */
