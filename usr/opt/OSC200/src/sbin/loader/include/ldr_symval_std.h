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
 *	@(#)$RCSfile: ldr_symval_std.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:37:22 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * ldr_symval_std.h
 * This file contains the declarations for the standard loader symbol
 * value structure.  The loader symbol value structure is machine-
 * dependent, but most machines will be able to use the standard
 * declaration contained in this file.
 *
 * Depends on: ldr_types.h
 *
 * OSF/1 Release 1.0
 */

#ifndef _H_LDR_SYMVAL_STD
#define _H_LDR_SYMVAL_STD

#include "ldr_macro_help.h"


/* Is symbol a function or data symbol?  May be used to determine kind
 * of handling if symbol is unresolved.
 */

typedef enum {				/* symbol kind */
	ldr_sym_unknown,		/* unknown symbol kind */
	ldr_sym_function,		/* code symbol */
	ldr_sym_data			/* data symbol */
	} ldr_symbol_kind_t;


/* Kind of symbol value, for tag in tagged union */

typedef enum ldr_symval_kind {
	ldr_sym_unres,			/* currently-unresolved symbol */
	ldr_sym_abs,			/* absolute (unsigned) value */
	ldr_sym_regrel			/* region/offset in current module */
	} ldr_symval_kind;

/* The loader symbol value structure itself -- a tagged union tagged
 * by ldr_symval_kind.
 */

typedef struct ldr_symval {
	ldr_symbol_kind_t	ls_kind; /* function or data */
	ldr_symval_kind		ls_tag;	/* tag for union */
	union {

		/* No union entry for ldr_sym_unres tag */

		univ_t		ls_un_abs; /* ldr_sym_abs -- must be first in
					    * the union for initialization to work
					    */

		struct {		/* ldr_sym_regrel, ldr_sym_modrel */
			long	ls_un_offset; /* offset from reg. start */
			int	ls_un_regno; /* region number */
		} ls_un_rel;

	} ls_union;
} ldr_symval;

#define	ls_abs		ls_union.ls_un_abs
#define	ls_offset	ls_union.ls_un_rel.ls_un_offset
#define	ls_regno	ls_union.ls_un_rel.ls_un_regno


/* The following macros and functions provide the machine-independent
 * interfaces to the machine-dependent symbol value structures.
 * Any representation of symbol values must support these operations.
 */

/* Initialize a static ldr_symval_t to contain an absolute value of
 * the specified symbol kind.  This depends on the ability of the
 * compiler to initialize a union.
 */
#define	ldr_symval_init_abs(k, n) { \
	k, \
	ldr_sym_abs, \
	{ n } \
	}

/* Construct a symbol value representing an unresolved symbol.
 *   extern void ldr_symval_make_unres(ldr_symval *sym);
 */

#define ldr_symval_make_unres(sym)	MACRO_BEGIN \
	(sym)->ls_tag = ldr_sym_unres; \
	(sym)->ls_kind = ldr_sym_unknown; \
	MACRO_END

/* Construct a symbol value representing an absolute symbol value,
 * of unknown kind.
 *   extern void ldr_symval_make_abs(ldr_symval *sym, univ_t val);
 */

#define ldr_symval_make_abs(sym, val)	MACRO_BEGIN \
	(sym)->ls_tag = ldr_sym_abs; \
	(sym)->ls_kind = ldr_sym_unknown; \
	(sym)->ls_abs = (val); \
	MACRO_END;

/* Construct a symbol value representing a region-relative symbol value,
 * of unknown kind.
 *   extern void ldr_symval_make_regrel(ldr_symval *sym, int regno, long offset);
 */

#define ldr_symval_make_regrel(sym, regno, offset)	MACRO_BEGIN \
	(sym)->ls_tag = ldr_sym_regrel; \
	(sym)->ls_kind = ldr_sym_unknown; \
	(sym)->ls_offset = (offset); \
	(sym)->ls_regno = (regno); \
	MACRO_END;

/* Make the specified symbol value represent a function symbol.
 *    extern void ldr_symval_make_function(ldr_symval *sym);
 */

#define ldr_symval_make_function(sym)	(sym)->ls_kind = ldr_sym_function

/* Make the specified symbol value represent a date symbol.
 *    extern void ldr_symval_make_data(ldr_symval *sym);
 */

#define ldr_symval_make_data(sym)	(sym)->ls_kind = ldr_sym_data

/* Is the specified symbol value unresolved?
 *   extern int ldr_symval_is_unres(ldr_symval *sym);
 */

#define ldr_symval_is_unres(sym)	((sym)->ls_tag == ldr_sym_unres)

/* Is the specified symbol value an absolute symbol value?
 *   extern int ldr_symval_is_abs(ldr_symval *sym);
 */

#define ldr_symval_is_abs(sym)	((sym)->ls_tag == ldr_sym_abs)

/* Is the specified symbol value a region-relative value?
 *   extern int ldr_symval_is_regrel(ldr_symval *sym);
 */

#define ldr_symval_is_regrel(sym)	((sym)->ls_tag == ldr_sym_regrel)

/* Is the specified symbol value a function symbol?
 *   extern int ldr_symval_is_function(ldr_symval *sym);
 */

#define ldr_symval_is_function(sym)	((sym)->ls_kind == ldr_sym_function)

/* Is the specified symbol value a data symbol?
 *   extern int ldr_symval_is_data(ldr_symval *sym);
 */

#define ldr_symval_is_data(sym)	((sym)->ls_kind == ldr_sym_data)

/* Return the absolute value of the specified symbol, if it's absolute.
 * Caller is responsible for ensuring that symbol is absolute (eg. by calling
 * ldr_symval_is_abs()).
 *   extern univ_t ldr_symval_abs(ldr_symval *sym)
 */

#define ldr_symval_abs(sym)	((sym)->ls_abs)

/* Convert the specified symbol value to an absolute address, if possible.
 * If already absolute, no work is needed.  Otherwise we need to look up
 * the symbol's region number in the supplied region table, and add the
 * symbol offset to the region start address to get the absolute
 * value.  The symbol's value is converted in place.  Caller must use
 * ldr_symval_is_abs afterward to check for success.
 *   extern void ldr_symval_cvt_abs(ldr_symval *sym, ldr_region_rec *regions,
 *				    int count);
 */

#define	ldr_symval_cvt_abs(sym, regions, count)	MACRO_BEGIN \
	if (((sym)->ls_tag == ldr_sym_regrel) && \
	    ((sym)->ls_regno >= 0) && \
	    ((sym)->ls_regno < (count)) ) { \
		(sym)->ls_tag = ldr_sym_abs; \
		(sym)->ls_abs = (univ_t)((char *)(regions[(sym)->ls_regno].lr_vaddr) \
				       + (sym)->ls_offset); \
		} \
	MACRO_END


/* Check the symbol types of two symbols for equality.  Returns success
 * if either symbol type is unknown, or if both are the same (function or
 * data).  Else returns error.
 *
 *	extern int ldr_symval_type_check(ldr_symval *sym1, ldr_symval *sym2)
 */

#define	ldr_symval_type_check(sym1, sym2) \
	(((sym1)->ls_tag == (sym2)->ls_tag) || \
	 ((sym1)->ls_tag == ldr_sym_unknown) || \
	 ((sym2)->ls_tag == ldr_sym_unknown) )

#endif  /* _H_LDR_SYMVAL_STD */

