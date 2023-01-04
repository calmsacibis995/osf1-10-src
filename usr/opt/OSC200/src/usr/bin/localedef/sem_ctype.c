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
static char     *sccsid = "@(#)$RCSfile: sem_ctype.c,v $ $Revision: 1.1.5.4 $ (DEC) $Date: 1993/12/20 21:31:02 $";
#endif
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
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.9  com/cmd/nls/sem_ctype.c, , bos320, 9135320l 8/22/91 13:17:26
 */

#include <sys/types.h>
#include <sys/localedef.h>
#include "semstack.h"
#include "symtab.h"
#include "ctype.h"
#include "err.h"

/* 
** GLOBAL variables 
*/
extern wchar_t max_wchar_enc;	/* the maximum wchar encoding for this codeset */
int next_bit = 0x1000;		/* first available bit for ctype classes */

/*
*  FUNCTION: add_classname
*
*  DESCRIPTION:
*  Adds a classname and associated mask to the ctype.classnms table.  
*/
void add_classname(_LC_ctype_t *ctype, char *s, int mask) 
{
    int i,j;

    /* check if classnms NULL and allocate space for 32 classes. */
    if (ctype->classnms == NULL) {
	ctype->classnms = MALLOC(_LC_classnm_t, 32);
	ctype->nclasses = 0;
    }
	
    /* search for class name in names array */
    for (i=0, j=-1; 
	 i<ctype->nclasses && 
	 (j=strcmp(s, ctype->classnms[i].name))>0;
	 i++);
    
    /* insert new class name unless already present */
    if (j != 0) {
	for (j=ctype->nclasses; j > i; j--)
	    ctype->classnms[j] = ctype->classnms[j-1];
	
	ctype->nclasses++;
	
	ctype->classnms[i].name = MALLOC(char, strlen(s)+1);
	strcpy(ctype->classnms[i].name, s);
	ctype->classnms[i].mask = mask;
    }
}


/*
*  FUNCTION: add_ctype
*
*  DESCRIPTION:
*  Creates a new character classification from the list of characters on
*  the semantic stack.  The productions using this sem-action implement
*  statements of the form:
*
*      print    <A>;...;<Z>;<a>;<b>;<c>;...;<z>
*
*  The function checks if the class is already defined and if so uses the
*  mask in the table, otherwise a new mask is allocated and used.  This 
*  allows seeding of the table with the POSIX classnames and preassignment
*  of masks to those classes.
*/
void add_ctype(_LC_ctype_t *ctype)
{

    extern symtab_t cm_symtab;
    extern wchar_t  max_wchar_enc;
    
    item_t   *it;
    symbol_t *sym0;
    symbol_t *sym1;
    int      i, j;
    int      n_ranges;
    uint     mask;
    wchar_t  wc;

    /* check if mask array has been defined yet, and if not allocate memory */
    if (ctype->_mask == NULL) {
	ctype->_mask = (uint_t *)calloc(sizeof(unsigned int), max_wchar_enc+1);
	if (ctype->_mask == NULL) 
	    INTERNAL_ERROR;

	/* make sure <tab> and <space> are blanks */
	ctype->_mask[9] |= _ISBLANK;
	ctype->_mask[32] |= _ISBLANK;

    }

    /* get class name off symbol stack.*/
    sym0 = sym_pop();
    
    /* check if class name already defined */
    sym1 = loc_symbol(&cm_symtab, sym0->sym_id, 0);
    if (sym1 != NULL && sym1->sym_type == ST_CLS_NM) {
	sym0 = sym1;

    } else {
	/* add new mask and type to symbol */
	diag_error(CHARCLASS_NOT_DECLARED, sym0->sym_id);
	add_predef_classname(sym0->sym_id, next_bit);
	next_bit <<= 1;
	sym0 = loc_symbol(&cm_symtab, sym0->sym_id, 0);
    }

    mask = sym0->data.cls->mask;

    /* handle derived properties */
    switch (mask) {
      case _ISUPPER:
      case _ISLOWER:
	mask |= _ISPRINT | _ISALPHA | _ISALNUM | _ISGRAPH;
	break;

      case _ISDIGIT:
	mask |= _ISPRINT | _ISALNUM | _ISGRAPH | _ISXDIGIT;
	break;

      case _ISPUNCT:
      case _ISXDIGIT:
	mask |= _ISGRAPH | _ISPRINT;
	break;

      case _ISBLANK:
	mask |= _ISSPACE;
	break;

      case _ISALPHA:
	mask |= _ISPRINT | _ISGRAPH | _ISALNUM;
	break;

      case _ISGRAPH:
	mask |= _ISPRINT;
	break;

      default:
	break;
    };
	

    /* for each range on stack - add mask to class mask for character */
    while ((it = sem_pop()) != NULL) {

	for (j=it->value.range->min; j <= it->value.range->max; j++) {
	    int wc;

	    wc = wc_from_fc(j);
	    /* only set masks for characters which are actually defined */
	    if (wc >= 0 && wchar_defined(wc)) 
		ctype->_mask[wc] |= mask;

	}
	
	destroy_item(it);
    }
}	      


/*
*  FUNCTION: push_char_sym
*
*  DESCRIPTION:
*  Create character range from character symbol.  Routine expects 
*  character symbol on semantic stack.  This symbol is used to create a
*  simple character range which is then pushed back on semantic stack.
*  This production 
*
*  Treatment of single characters as ranges allows a uniform handling of 
*  characters and character ranges in class definition statements.
*/
void push_char_sym(void)
{
    item_t   *it0, *it1;
    
    it0 = sem_pop();
    
    if (it0->type == SK_CHR)
	it1 = create_item(SK_RNG, 
			  it0->value.chr->fc_enc,
			  it0->value.chr->fc_enc);
    else if (it0->type == SK_INT)
	it1 = create_item(SK_RNG, 
			  it0->value.int_no,
			  it0->value.int_no);
    else
	error(ERR_ILL_RANGE_SPEC);

    destroy_item(it0);

    sem_push(it1);
}


/*
*  FUNCTION: push_char_range
*
*  DESCRIPTION:
*  Modifies end point of range with character on top of stack. This rule is
*  used by productions implementing expressions of the form:
*
*       <A>;...;<Z>
*/
void push_char_range(void)
{
    item_t   *it0, *it1;
    
    it1 = sem_pop();		/* from character at end of range   */
    it0 = sem_pop();		/* from character at start of range */
    if (it1->type == SK_CHR && it0->type == SK_RNG) {
        /* make sure min is less than max */
        if (it1->value.chr->fc_enc > it0->value.range->max)
	    it0->value.range->max = it1->value.chr->fc_enc;
        else
	    it0->value.range->min = it1->value.chr->fc_enc;
    }
    else if (it1->type == SK_INT && it0->type == SK_RNG) {
	if (it1->value.int_no > it0->value.range->max) 
	    it0->value.range->max = it1->value.int_no;
	else
	    it0->value.range->min = it1->value.int_no;
    }
    else
	INTERNAL_ERROR;
    
    destroy_item(it1);
    
    sem_push(it0);
}
