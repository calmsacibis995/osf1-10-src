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
 * @(#)$RCSfile: symtab.h,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/09/03 18:25:49 $
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDLOC) Locale Database Commands
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.4  com/cmd/nls/symtab.h, cmdnls, bos320 6/20/91 01:01:16 
 */

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include <limits.h>
#include <sys/localedef.h>
#include <sys/types.h>

/* symbol table/stack limits */

#define ST_MAX_DEPTH    16384
#define MAX_SYM_LEN	128		/* Worst case for symbol size */

typedef struct {
    wchar_t      pc;
    char         *sym;
    char	 *str;
} coll_ell_t;

typedef struct {
  char *mth_sym;		 /* method symbol name */
  char *lib_name;		 /* name of library */
  void *(*mth_ptr);		 /* pointer to method */
} mth_sym_t;

typedef struct {
    wchar_t      wc_enc;	/* the actual encoding for the character */
    unsigned int fc_enc;	/* character as integral file code */
    unsigned char str_enc[MB_LEN_MAX+1];  /* character as string */
    _LC_weight_t *wgt;		/* collation weights */
    _LC_subs_t   *subs_str;	/* substitution string */
    short        width;		/* the display width of the character */
    short        len;		/* the length of the character in bytes */
} chr_sym_t;

typedef struct {
  uint mask;
} cls_sym_t;

/*
 * type tags for symbol_t symbols
 */
typedef enum {
    ST_UNKNOWN,		   /* symbol type is unknown */
    ST_MTH_SYM,		   /* symbol refers to a method */
    ST_CHR_SYM,		   /* symbol refers to character enc. */
    ST_COLL_SYM,	   /* symbol is a collation symbol */
    ST_COLL_ELL,	   /* symbol is a collation element */
    ST_CLS_NM,		   /* symbol refers to char class */
    ST_STR,          	   /* symbol refers to text string  */
    ST_INT		   /* symbol is an integer constant */
} sym_type_t;

typedef struct _SYMBOL_T symbol_t;

struct _SYMBOL_T {
  char   *sym_id;		   /* the symbol id text */
  sym_type_t    sym_type;	   /* message set, text, ... symbol */
  int    sym_scope;                /* message set in which this symbol */
				   /* is defined */
  union  {
    chr_sym_t   *chr;              /* the character encoding this */
				   /* symbol refers to.  */

    cls_sym_t   *cls;		   /* symbol refers to a character class. */

    mth_sym_t   *mth;		   /* symbol refers to a method */

    coll_ell_t  *collel;	   /* symbol refers to a collation element */

    _LC_weight_t *collsym;	   /* symbol refers to a collation symbol */

    char        *str;

    int         ival;

  } data;

  symbol_t *next;
};


#define HASH_TBL_SIZE   89
typedef struct {

  int      n_symbols;		   /* number of symbols in table */

  symbol_t symbols[HASH_TBL_SIZE]; /* sllist of symbol_t entries sorted */
				   /* alphabetically. */

} symtab_t;

/*
  symbol table utility error values
*/
#define ST_OK          0
#define ST_DUP_SYMBOL  1
#define ST_OVERFLOW    2

int        cmp_symbol(symbol_t *, char *, int);
symbol_t * create_symbol(char *, int);
int        add_symbol(symtab_t *, symbol_t *);
symbol_t * loc_symbol(symtab_t *, char *, int);

int        sym_push(symbol_t *);
symbol_t * sym_pop(void);


#endif /* _SYMTAB_H_ */
