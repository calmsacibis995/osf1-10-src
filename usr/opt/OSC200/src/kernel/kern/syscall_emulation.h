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
 *	@(#)$RCSfile: syscall_emulation.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:27:25 $
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

#ifndef	_KERN_SYSCALL_EMULATION_H_
#define _KERN_SYSCALL_EMULATION_H_

#define EML_MIN_SYSCALL		-9

#ifndef	ASSEMBLER
typedef	 unsigned long 	eml_routine_t;

typedef struct eml_dispatch {
	int		eml_ref;	/* reference count */
	int 		disp_count; 	/* count of entries in vector */
	eml_routine_t	disp_vector[1];	/* first entry in array (array is really disp_count large) */
					/* of dispatch routines */
} *eml_dispatch_t;

typedef struct syscall_val {
	int	rv_val1;
	int	rv_val2;
} syscall_val_t;


#define EML_ROUTINE_NULL	(eml_routine_t)0
#define EML_DISPATCH_NULL	(eml_dispatch_t)0

#define EML_SUCCESS		(0)

#define EML_MOD			(err_kern|err_sub(2))
#define EML_BAD_TASK		(EML_MOD|0x0001)
#define EML_BAD_CNT		(EML_MOD|0x0002)

#define EML_OFFSET(x)		((x)-EML_MIN_SYSCALL)

#endif	/* ASSEMBLER */
#endif	/* _KERN_SYSCALL_EMULATION_H_ */
