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
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: UilKeyTab.c,v $ $Revision: 1.1.4.2 $ $Date: 1993/05/07 00:34:16 $"
#endif
#endif

/*
*  (c) Copyright 1989, 1990, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */

/*
**++
**  FACILITY:
**
**      User Interface Language Compiler (UIL)
**
**  ABSTRACT:
**
**      This module contains the keyword table used by the lexical analyzer
**	to look up the keywords in the UIL.
**
**--
**/


/*
**
**  INCLUDE FILES
**
**/

#include "UilDefI.h"


/*
**
**  DEFINE and MACRO DEFINITIONS
**
**/


/*
**
**  EXTERNAL VARIABLE DECLARATIONS
**
**/


/*
**
**  GLOBAL VARIABLE DECLARATIONS
**
**/


/*
**
**  OWN VARIABLE DECLARATIONS
**
**/

/*    Keyword table pointer.    */

static key_keytable_entry_type * key_keytable_ptr;


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine searches for a symbol in the compiler's keyword table.
**	There are two arguments to the routine, the length of the symbol and
**      the address of the start of the symbol.  The routine returns the
**	address of the keyword entry found, or a NULL pointer if the
**	symbol is not found in the table.
**
**	The search for the symbol is case sensitive depending upon the
**	keytable binding that was established by the key_initialize routine.
**
**	The require file UilKeyTab.h defines and initializes the keyword
**	tables.  It is built automatically from other files, thus it should
**	not be hand editted.
**
**  FORMAL PARAMETERS:
**
**	symbol_length.rl.v : 	length of symbol to look up
**	symbol_ptr.ra.v : 	address of symbol to look up
**
**  IMPLICIT INPUTS:
**
**      key_keytable_ptr		: current keyword table
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**	NULL		: if the symbol is not in the keyword table
**	otherwise	: the address of the keyword table entry for
**			  the specified symbol.
**
** SIDE EFFECTS:
**
**	none
**
**--
**/
key_keytable_entry_type *
	key_find_keyword (symbol_length, symbol_ptr)

unsigned int	symbol_length;
char		* symbol_ptr;

{
    
    int
	lower_limit,
	upper_limit;
    
/*    Check the arguments.    */

    if (symbol_length > key_k_keyword_max_length)
	return NULL;

/*    Initialize region to search.    */
    
    lower_limit = 0;
    upper_limit = key_k_keyword_count-1;
    
/*    Perform binary search on keyword index.    */
    
    do {
	int		mid_point, result;

	key_keytable_entry_type * keyword_entry_ptr;

	mid_point = (lower_limit + upper_limit) >> 1;	/* divide by 2 */

	keyword_entry_ptr = & key_keytable_ptr [mid_point];

	result = strcmp (symbol_ptr, keyword_entry_ptr -> at_name);

	if (result == 0) {
	    return keyword_entry_ptr;		/*    Found keyword.    */
	} else if (result < 0) {
	    upper_limit = mid_point - 1;	/*    Search lower half.    */
	} else {
	    lower_limit = mid_point + 1;	/*    Search upper half.    */
	}

    } while (lower_limit <= upper_limit);

/*    If we fall out of the bottom of the loop, symbol was not found.    */

    return NULL;

}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine initializes the keyword lookup facility.  It can be
**	called multiple times during a single compilation.  It must be called
**	at least once before the keyword table is accessed.
**
**  FORMAL PARAMETERS:
**
**	none
**
**  IMPLICIT INPUTS:
**
**      uil_v_case_sensitive	: case sensitive switch, determines which
**				: keyword table to use.
**
**  IMPLICIT OUTPUTS:
**
**      key_keytable_ptr	: pointer to the keyword table to
**				  use for keyword lookups.
**
**  FUNCTION VALUE:
**
**	none
**
** SIDE EFFECTS:
**
**	none
**
**--
**/
void
	key_initialize ()

{

/*    Use the correct keyword table based on the global case
      sensitivity.   */

    if (uil_v_case_sensitive) {
	key_keytable_ptr = key_table;
    } else {
	key_keytable_ptr = key_table_case_ins;
    }

}    

