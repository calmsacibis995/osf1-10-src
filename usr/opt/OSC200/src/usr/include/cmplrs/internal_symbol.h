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
**++
**  PRODUCT:
***
**	DEC C++ Demangler
**
**  FILE:  
**
**      internal_symbol.h
**
**  ABSTRACT:
**
**	This file contains info necessary for recognizing an internal C++ symbol.
**
**  ENVIRONMENT:
**
**	User Mode
**
**  AUTHORS:
**
**
**  MODIFICATION HISTORY:
**
**
**--
**/


#ifndef _INTERNAL_SYMBOL_H_
#define _INTERNAL_SYMBOL_H_

/*
*** 
*** Misc.
***
*/

#define	MLD_ERROR	-1
#define MLD_SOMETHING_RECOGNIZED 1
#define MLD_NOTHING_RECOGNIZED 0
#define MLD_FALSE 0
#define MLD_TRUE 1


/*
**
** Internal Symbol Recognition Function
**
*/

int /* recognition status */
MLD_internal_symbol(char* s,                /* IN:  string to be parsed */
                        int start,      /* IN:  index of first unparsed character */
                        int *symbol);   /* OUT: enumeration value of symbol */


/*
***
*** Internal Symbol Definitions
***
*/

#define MLD_NUMBER_OF_INTERNAL_SYMBOLS                      22

enum MLD_INTERNAL_SYMS {

MLD_INTER_INTERNAL_SYM,
MLD_BPTR_INTERNAL_SYM,
MLD_CONTROL_INTERNAL_SYM,
MLD_VPTR_INTERNAL_SYM,
MLD_BRACKETED_CONTROL_INTERNAL_SYM,
MLD_RESULT_POINTER_INTERNAL_SYM,
MLD_BRACKETED_RESULT_POINTER_INTERNAL_SYM,
MLD_BRACKETED_RESULT_INTERNAL_SYM,
MLD_RESULT_INTERNAL_SYM,
MLD_THIS_INTERNAL_SYM,
MLD_VTBL_INTERNAL_SYM,
MLD_BTBL_INTERNAL_SYM,
MLD_EVDW_INTERNAL_SYM,
MLD_EXTERNAL_DESTRUCTOR_FUNCTOR_LIST_INTERNAL_SYM,
MLD_FN_INTERNAL_SYM,
MLD_FW_INTERNAL_SYM,
MLD_INIT_INTERNAL_SYM,
MLD_IVIW_INTERNAL_SYM,
MLD_THUNK_INTERNAL_SYM,
MLD_CTYPE_INTERNAL_SYM,
MLD_T_INTERNAL_SYM,
MLD_ELLIPSES_INTERNAL_SYM

};

#define MLD_INTER_INTERNAL_SYMBOL					"__INTER__"
#define MLD_BPTR_INTERNAL_SYMBOL					"__bptr"
#define MLD_CONTROL_INTERNAL_SYMBOL					"__control"
#define MLD_VPTR_INTERNAL_SYMBOL					"__vptr"
#define MLD_BRACKETED_CONTROL_INTERNAL_SYMBOL			"<control>"
#define MLD_RESULT_POINTER_INTERNAL_SYMBOL				"__result_pointer"
#define MLD_BRACKETED_RESULT_POINTER_INTERNAL_SYMBOL		"<result_pointer>"
#define MLD_BRACKETED_RESULT_INTERNAL_SYMBOL			"<RESULT>"
#define MLD_RESULT_INTERNAL_SYMBOL					"__result"
#define MLD_THIS_INTERNAL_SYMBOL					"this"
#define MLD_VTBL_INTERNAL_SYMBOL					"__vtbl"
#define MLD_BTBL_INTERNAL_SYMBOL					"__btbl"
#define MLD_EVDW_INTERNAL_SYMBOL					"evdw"
#define MLD_EXTERNAL_DESTRUCTOR_FUNCTOR_LIST_INTERNAL_SYMBOL	"__external_destructor_functor_list"
#define MLD_FN_INTERNAL_SYMBOL					"__fn"
#define MLD_FW_INTERNAL_SYMBOL					"__fw"
#define MLD_INIT_INTERNAL_SYMBOL					"__init"
#define MLD_IVIW_INTERNAL_SYMBOL					"__iviw"
#define MLD_THUNK_INTERNAL_SYMBOL					"_thunk"
#define MLD_CTYPE_INTERNAL_SYMBOL					"_ctype__"
#define MLD_T_INTERNAL_SYMBOL					"__t"
#define MLD_ELLIPSES_INTERNAL_SYMBOL				"..."

typedef enum { MLD_all, MLD_pre, MLD_post } MLD_internalPos;

struct MLD_internalNameEntry {
     char* s;
     MLD_internalPos ip;
     int arg;
};

static struct MLD_internalNameEntry MLD_internalNameTable[] = {
     { MLD_INTER_INTERNAL_SYMBOL,                            MLD_pre,  MLD_FALSE },
     { MLD_BPTR_INTERNAL_SYMBOL,                             MLD_all,  MLD_FALSE },
     { MLD_CONTROL_INTERNAL_SYMBOL,                          MLD_all,  MLD_TRUE },
     { MLD_VPTR_INTERNAL_SYMBOL,                             MLD_all,  MLD_FALSE },
     { MLD_BRACKETED_CONTROL_INTERNAL_SYMBOL,                MLD_all,  MLD_TRUE },
     { MLD_RESULT_POINTER_INTERNAL_SYMBOL,                   MLD_all,  MLD_TRUE },
     { MLD_BRACKETED_RESULT_POINTER_INTERNAL_SYMBOL,         MLD_all,  MLD_TRUE },
     { MLD_BRACKETED_RESULT_INTERNAL_SYMBOL,                 MLD_all,  MLD_TRUE },
     { MLD_RESULT_INTERNAL_SYMBOL,                           MLD_all,  MLD_TRUE },
     { MLD_THIS_INTERNAL_SYMBOL,                             MLD_all,  MLD_TRUE },
     { MLD_VTBL_INTERNAL_SYMBOL,                             MLD_pre,  MLD_FALSE },
     { MLD_BTBL_INTERNAL_SYMBOL,                             MLD_pre,  MLD_FALSE },
     { MLD_EVDW_INTERNAL_SYMBOL,                             MLD_post, MLD_FALSE },
     { MLD_EXTERNAL_DESTRUCTOR_FUNCTOR_LIST_INTERNAL_SYMBOL, MLD_all,  MLD_FALSE },
     { MLD_FN_INTERNAL_SYMBOL,                               MLD_all,  MLD_FALSE },
     { MLD_FW_INTERNAL_SYMBOL,                               MLD_all,  MLD_FALSE },
     { MLD_INIT_INTERNAL_SYMBOL,                             MLD_pre,  MLD_FALSE },
     { MLD_IVIW_INTERNAL_SYMBOL,                             MLD_post, MLD_FALSE },
     { MLD_THUNK_INTERNAL_SYMBOL,                            MLD_post, MLD_FALSE },
     { MLD_CTYPE_INTERNAL_SYMBOL,                            MLD_all,  MLD_FALSE },
     { MLD_T_INTERNAL_SYMBOL,                                MLD_pre,  MLD_FALSE },
     { MLD_ELLIPSES_INTERNAL_SYMBOL,                         MLD_all,  MLD_TRUE },
};


#endif /* _INTERNAL_SYMBOL_H_ */
/* DON'T ADD ANYTHING AFTER THIS #endif */
