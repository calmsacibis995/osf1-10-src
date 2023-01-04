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
 *	@(#)$RCSfile: ldr_types.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:37:19 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 *	ldr_types.h
 *	loader defined types
 *	NOTE: other include files needed in order to use this one :
 *		<sys/types.h> <loader.h>
 *
 * OSF/1 Release 1.0
 */

#ifndef _H_LDR_TYPES
#define _H_LDR_TYPES

/* Externally-visible type declarations */

#ifndef _H_LDR_MAIN_TYPES
#include <loader/ldr_main_types.h>
#endif /* _H_LDR_MAIN_TYPES */

typedef	univ_t	ldr_heap_t;		/* Heap data type (opaque) */

typedef	univ_t	ldr_export_list;	/* opaque type for fmt-dep exports */

typedef	enum {				/* kinds of entry points */
	init_routines,			/* initialization routine */
	term_routines			/* termination routine */
}  entry_pt_kind;

typedef univ_t		ldr_switch_t;

#ifndef NULL
#define NULL	0
#endif /* NULL */

extern ldr_heap_t	ldr_process_heap;

/* Machine-dependent type declarations */

#include <ldr_machdep.h>

#endif /* _H_LDR_TYPES */
