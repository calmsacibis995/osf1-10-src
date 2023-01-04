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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: mmax_ast.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:41:55 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
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
 *	mmax/ast.c - multimax support for remote ast_check invocation.
 */

#include <kern/processor.h>
#include <sys/table.h>

extern int ast_check();

/*
 *	init_ast_check - initialize for remote invocation of ast_check.
 *		Allocate the vector exactly once, and fill in cpu fields.
 */

init_ast_check(processor)
processor_t	processor;
{
	static ast_check_t	ast_vector;
	static boolean_t	ast_initialized = FALSE;

	/*
	 *	Allocate vector exactly once.
	 */

	if (!ast_initialized) {
	    if(alloc_vector(&ast_vector, ast_check, 0, INTR_OTHER))
	    	panic("init_ast_check");

	    ast_vector.f.v_class = SLAVE_CLASS;
	    ast_initialized = TRUE;
	}

	/*
	 *	Copy vector to processor structure and fill in processor.
	 */
#define CPU_TO_SLOT(cpu)        ((cpu & 0x3c) >> 2)
#define CPU_TO_DEV(cpu)         (cpu & 0x3)
	processor->ast_check_data = ast_vector;
	processor->ast_check_data.f.v_slot = CPU_TO_SLOT(processor->slot_num);
	processor->ast_check_data.f.v_device = CPU_TO_DEV(processor->slot_num);
}

/*
 *	cause_ast_check - cause remote invocation of ast_check.  Caller
 *		is at splsched().  Just send the vector to the processor.
 */

cause_ast_check(processor)
processor_t	processor;
{
    send_vector(&(processor->ast_check_data));
}
