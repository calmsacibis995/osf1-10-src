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
 *	@(#)$RCSfile: ldr_symbol.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:37:12 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* ldr_symbol.h
 * Definitions for loader import symbol table
 *
 * This routine defines the common symbol table format, used for
 * both the import symbol table and the export symbol table.
 * The symbol table lists all the imported or exported symbols for a
 * module, the package each symbol comes from, and the symbol's
 * value.
 *
 * The import symbol table is built by the format-dependent
 * manager, with all symbols unresolved.  The symbol values are
 * filled in by the format-independent symbol resolution and
 * symbol value computation routines.  The entire import symbol
 * table is then passed to the format-dependent manager for its
 * use during relocation.
 *
 * The export symbol table is also built by the format-dependent
 * manager.  It is normally only used when pre-loading a library.
 *
 * This file depends on: ldr_machdep.h ldr_package.h
 * OSF/1 Release 1.0
 */

#ifndef _H_LDR_SYMBOL
#define _H_LDR_SYMBOL


#define	LDR_SYMBOL_VERSION	1	/* current structure version */

typedef struct ldr_symbol_rec {
	int		ls_version;	/* version number of structure */
	char		*ls_name;	/* symbol name */
	int		ls_packageno;	/* package number symbol belongs to */
	univ_t		ls_module;	/* module exporting symbol */
	ldr_symval	ls_value;	/* symbol's value */
	char		ls_reserved[40]; /* reserved */
} ldr_symbol_rec;


/* Create a list of symbol records large enough to hold
 * count records, initialize it, and return it in *syms.  Returns
 * LDR_SUCCESS on success, negative error status on error.
 */

extern int ldr_symbols_create __((int count, int version, ldr_symbol_rec **retval));

/* Free a list of symbol packages containing count records.  Returns
 * LDR_SUCCESS on success, negative error status on error.
 */

extern int ldr_symbols_free __((int count, ldr_symbol_rec *val));

#endif /* _H_LDR_SYMBOL */
