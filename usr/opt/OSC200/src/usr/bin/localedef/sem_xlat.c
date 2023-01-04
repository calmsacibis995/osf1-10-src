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
static char     *sccsid = "@(#)$RCSfile: sem_xlat.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/09/30 20:37:01 $";
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
 * 1.4  com/cmd/nls/sem_xlat.c, cmdnls, bos320, 9138320 9/11/91 16:35:09
 */

#include <sys/types.h>
#include <sys/localedef.h>
#include "semstack.h"
#include "symtab.h"
#include "err.h"


/*
*  FUNCTION: add_upper
*
*  DESCRIPTION:
*  Build the 'upper' character translation tables from the symbols on the
*  semantic stack.
*/
void
add_upper(_LC_ctype_t *ctype)
{
    extern wchar_t max_wchar_enc;
    item_t *it;
    int i;

    /* check if upper array allocated yet - allocate if NULL */
    if (ctype->_upper == NULL)
	ctype->_upper = MALLOC(wchar_t, max_wchar_enc+1);

    /* set up default translations - which is identity */
    for (i=0; i <= max_wchar_enc; i++)
	ctype->_upper[i] = i;

    /* for each range on stack - the min is the FROM pc, and the max is */
    /* the TO pc.*/
    while ((it = sem_pop()) != NULL) {
	ctype->_upper[it->value.range->min] = it->value.range->max;
    }
}    


/*
*  FUNCTION: add_lower
*
*  DESCRIPTION:
*  Build the 'lower' character translation tables from the symbols on the
*  semantic stack.
*/
void
add_lower(_LC_ctype_t *ctype)
{
    extern wchar_t max_wchar_enc;
    item_t *it;
    int i;

    /* check if lower array allocated yet - allocate if NULL */
    if (ctype->_lower == NULL)
	ctype->_lower = MALLOC(wchar_t, max_wchar_enc+1);

    /* set up default translations which is identity */
    for (i=0; i <= max_wchar_enc; i++)
	ctype->_lower[i] = i;

    /* for each range on stack - the min is the FROM pc, and the max is */
    /* the TO pc.*/
    while ((it = sem_pop()) != NULL) {
	ctype->_lower[it->value.range->min] = it->value.range->max;
    }
}	      

/* 
*  FUNCTION: sem_push_xlat
*
*  DESCRIPTION:
*  Creates a character range item from two character reference items.
*  The routine pops two character reference items off the semantic stack.
*  These items represent the "to" and "from" pair for a character case
*  translation.  The implementation uses a character range structure to
*  represent the pair.
*/
void
sem_push_xlat(void)
{
  item_t   *it0, *it1;
  item_t   *it;
  it1 = sem_pop();		/* this is the TO member of the pair */
  it0 = sem_pop();		/* this is the FROM member of the pair */

  /* this creates the item and sets the min and max to wc_enc */

  if (it0->type == it1->type)	/* Same type is easy case */
    switch(it0->type) {
      case SK_CHR:
	it = create_item(SK_RNG, it0->value.chr->wc_enc, it1->value.chr->wc_enc);
	break;
      case SK_INT:
	it = create_item(SK_RNG, it0->value.int_no, it1->value.int_no);
	break;
      default:
	INTERNAL_ERROR;		/* NEVER RETURNS */
    }
  /*
   * Not same types, we can coerce INT and CHR into a valid range
   */
  else if (it0->type == SK_CHR && it1->type == SK_INT)
     it = create_item(SK_RNG, it0->value.chr->wc_enc, it1->value.int_no);
  else if (it0->type == SK_INT && it1->type == SK_CHR)
     it = create_item(SK_RNG, it0->value.int_no, it1->value.chr->wc_enc);
  else
    INTERNAL_ERROR;

  destroy_item(it1);
  destroy_item(it0);

  sem_push(it);
}

