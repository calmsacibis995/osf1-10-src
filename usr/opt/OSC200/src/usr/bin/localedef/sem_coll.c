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
static char     *sccsid = "@(#)$RCSfile: sem_coll.c,v $ $Revision: 1.1.5.8 $ (DEC) $Date: 1993/12/20 21:32:57 $";
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
 * 1.13  com/cmd/nls/sem_coll.c, cmdnls, bos320, 9137320a 9/6/91 11:41:08
 */

#include <sys/localedef.h>
#include <sys/method.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "symtab.h"
#include "semstack.h"
#include "locdef.h"
#include "err.h"

/*
  Regular expression special characters.
*/
#define REGX_SPECIALS  "[*+.?({|^$"

/*
  Collation weight counter.  nxt_coll_wgt() returns it and then
  increments it.

  Defining the counter outside of nxt_coll_wgt() lets us control the
  weights that nxt_coll_wgt() returns.  We need to do that when
  inserting undefined characters into the beginning or the middle of
  the character order.
*/
static wchar_t nxt_coll_val = 0x01010101;	/* No null bytes permitted */

/*
*  FUNCTION: nxt_coll_wgt
*
*  DESCRIPTION:
*  Collation weights cannot have a zero in either the first or second
*  byte (assuming two byte wchar_t).
*/
static
wchar_t nxt_coll_wgt(void)
{
    wchar_t collval;
    extern	_LC_collate_t	collate;

    collval = nxt_coll_val;
    nxt_coll_val++;
    if ((nxt_coll_val & 0xff00) == 0)
	INTERNAL_ERROR;
    
    if ((nxt_coll_val & 0x00ff) == 0)
	nxt_coll_val |= 0x0001;

    if (collval > collate.co_col_max)
	collate.co_col_max = collval;		/* Remember biggest */
    
    return collval;
}


/*
*  FUNCTION: set_coll_wgt
*
*  DESCRIPTION:
*  Sets the collation weight for order 'ord' in weight structure 'weights'
*  to 'weight'.  If 'ord' is -1, all weights in 'weights' are set to 
*  'weight'.
*/
void set_coll_wgt(_LC_weight_t *weights, wchar_t weight, int ord)
{
    extern _LC_collate_t collate;
    int i;

    if (collate.co_nord < _COLL_WEIGHTS_INLINE) {

	if (ord < 0) {
	    for (i=0; i <= collate.co_nord; i++)
		weights->n[i] = weight;
	} else
	    if (ord >= 0 && ord <= collate.co_nord)
		weights->n[ord] = weight;
	    else
		INTERNAL_ERROR;

    } else {
	int i;

	/* check if weights array has been allocated yet */
	if (weights->p == NULL) {
	    weights->p = MALLOC(wchar_t, collate.co_nord + 1);
	    for (i=0; i<=collate.co_nord; i++)
		weights->p[i] = UNDEFINED;
	}

	if (ord < 0) {
	    for (i=0; i <= collate.co_nord; i++)
		weights->p[i] = weight;
	} else
	    if (ord >= 0 && ord <= collate.co_nord)
		weights->p[ord] = weight;
	    else
		INTERNAL_ERROR;
    }
}


/*
*  FUNCTION: get_coll_wgt
*
*  DESCRIPTION:
*  Gets the collation weight for order 'ord' in weight structure 'weights'.
*/
wchar_t get_coll_wgt(_LC_weight_t *weights, int ord)
{
    extern _LC_collate_t collate;

    if (collate.co_nord < _COLL_WEIGHTS_INLINE) {

	if (ord >= 0 && ord <= collate.co_nord)
	    return weights->n[ord];
	else
	    error(ERR_TOO_MANY_ORDERS);

    } else {

	/* check if weights array has been allocated yet */
	if (weights->p == NULL)
	    INTERNAL_ERROR;

	if (ord >= 0 && ord <= collate.co_nord)
	    return weights->p[ord];
	else
	    error(ERR_TOO_MANY_ORDERS);
    }
/*NOTREACHED*/
    return 0;
}


/* 
*  FUNCTION: sem_init_colltbl
*
*  DESCRIPTION:
*  Initialize the collation table.  This amounts to setting all collation
*  values to IGNORE, assigning the default collation order (which is 1), 
*  allocating memory to contain the table.
*/
void
sem_init_colltbl(void)
{
    extern wchar_t max_wchar_enc;
    extern _LC_collate_t collate;

    
    /* initialize collation attributes to defaults */
    collate.co_nord   = 0;		/* potentially modified by 'order' */
    collate.co_wc_min = 0;		/* always 0                        */
    collate.co_wc_max = max_wchar_enc;	/* always max_wchar_enc            */
    
    /* allocate and zero fill memory to contain collation table */
    collate.co_coltbl = MALLOC(_LC_coltbl_t, max_wchar_enc+1);
    if (collate.co_coltbl == NULL)
	INTERNAL_ERROR;

    /* set default min and max collation weights */
    collate.co_col_min = collate.co_col_max = 0;

    /* initialize substitution strings */
    collate.co_nsubs = 0;
    collate.co_subs  = NULL;
}


/* 
*  FUNCTION: sem_push_collel();
*  DESCRIPTION:
*  Copies a symbol from the symbol stack to the semantic stack.
*/
void
sem_push_collel()
{
    symbol_t *s;
    item_t   *i;

    s = sym_pop();
    i = create_item(SK_SYM, s);
    sem_push(i);
}


/*
*  FUNCTION: loc_collel
*
*  DESCRIPTION: 
*  Locates a collation element in an array of collation elements.  This
*  function returns the first collation element which matches 'sym'.
*/
_LC_collel_t *
loc_collel(char *sym, wchar_t pc)
{
    extern _LC_collate_t collate;
    _LC_collel_t *ce;

    for (ce = collate.co_coltbl[pc].ct_collel; ce->ce_sym != NULL; ce++) {
	if (strcmp(sym, ce->ce_sym)==0)
	    return ce;
    }
    
    INTERNAL_ERROR;
/*NOTREACHED*/
}


/*
*  FUNCTION: sem_coll_sym_ref
*
*  DESCRIPTION:
*  checks that the symbol referenced has a collation weights structure
*  attached.  If one is not yet present, one is allocated as necessary 
*  for the symbol type.
*/
void
sem_coll_sym_ref(void)
{
    extern _LC_collate_t collate;
    _LC_collel_t *ce;
    symbol_t     *s;

    /* Pop symbol specified off of symbol stack
    */
    s = sym_pop();

    /* 
      Check that this element has a weights array with
      the correct number of orders.  If the element does not, create
      one and initialize the contents to UNDEFINED.
      */
    switch (s->sym_type) {
      case ST_CHR_SYM:
	if (s->data.chr->wgt == NULL)
	    s->data.chr->wgt = 
		&(collate.co_coltbl[s->data.chr->wc_enc].ct_wgt);

	if (collate.co_nord >= _COLL_WEIGHTS_INLINE) {
	    if (s->data.chr->wgt->p == NULL) {
		s->data.chr->wgt->p = MALLOC(wchar_t, collate.co_nord);
		set_coll_wgt(s->data.chr->wgt, UNDEFINED, -1);
	    }
	}
	break;

      case ST_COLL_ELL:
	ce = loc_collel(s->data.collel->sym, s->data.collel->pc);
	if (collate.co_nord >= _COLL_WEIGHTS_INLINE) {
	    if (ce->ce_wgt.p == NULL) {
		ce->ce_wgt.p = MALLOC(wchar_t, collate.co_nord);
		set_coll_wgt(&(ce->ce_wgt), UNDEFINED, -1);
	    }
	}
	break;

      case ST_COLL_SYM:
	if (collate.co_nord >= _COLL_WEIGHTS_INLINE) {
	    if (s->data.collsym->p == NULL) {
		    s->data.collsym->p = MALLOC(wchar_t, collate.co_nord);
		set_coll_wgt(s->data.collsym, UNDEFINED, -1);
	    }
	}
	break;
      default:
	INTERNAL_ERROR;
    }

    sym_push(s);
}


/*
*  FUNCTION: sem_coll_literal_ref
*  
*  DESCRIPTION:
*  A character literal is specified as a collation element.  Take this
*  element and create a dummy symbol for it.  The dummy symbol is pushed
*  onto the symbol stack, but is not added to the symbol table.
*/
void
sem_coll_literal_ref(void)
{
    extern int max_disp_width;
    symbol_t *dummy;
    item_t   *it;
    wchar_t  pc;
    int      rc;
    int      fc;
    

    /* Pop the file code to use as character off the semantic stack. */
    it = sem_pop();
    fc = it->value.int_no;

    /* Create a dummy symbol with this byte list as its encoding. */
    dummy = MALLOC(symbol_t, 1);
    dummy->sym_type = ST_CHR_SYM;
    dummy->sym_scope = 0;
    dummy->data.chr = MALLOC(chr_sym_t, 1);

    /* save file code for character */
    dummy->data.chr->fc_enc = fc;
    
    /* use hex translation of file code for symbol id (for errors) */
    dummy->sym_id = MALLOC(char, 8*2+3);	/* '0x' + 8 digits + '\0' */
    sprintf(dummy->sym_id, "0x%x", fc);

    /* save length of character */
    dummy->data.chr->len = mbs_from_fc(dummy->data.chr->str_enc, fc);

    /* check if characters this long are valid */
    if (dummy->data.chr->len > mb_cur_max)
	error(ERR_CHAR_TOO_LONG, dummy->sym_id);

    /* define process code for character literal */
    rc = INT_METHOD(METH_OFFS(CHARMAP_MBTOWC))(&pc,
					      dummy->data.chr->str_enc,
					      MB_LEN_MAX,
					      NULL);
    if (rc < 0)
	error(ERR_UNSUP_ENC, dummy->sym_id);

    dummy->data.chr->wc_enc = pc;
    
    /* define width for character */
    rc = INT_METHOD(METH_OFFS(CHARMAP_WCWIDTH))(pc, NULL);
    if (rc > max_disp_width)
	max_disp_width = rc;

    dummy->data.chr->width = rc;

    /* clear out wgt and subs_str pointers */
    dummy->data.chr->wgt = NULL;
    dummy->data.chr->subs_str = NULL;

    /* mark character as defined */
    define_wchar(pc);

    destroy_item(it);
    sym_push(dummy);
}


/* 
*  FUNCTION: sem_def_substr
*
*  DESCRIPTION:
*  Defines a substitution string.
*/
void
sem_def_substr(void)
{
    extern _LC_collate_t collate;
    item_t     *it0, *it1;
    char       *src, *tgt;
    _LC_subs_t *subs;
    int        i, j;
    int        flag;

    it1 = sem_pop();		/* target string */
    it0 = sem_pop();		/* source string */

    if (it1->type != SK_STR || it0->type != SK_STR)
	INTERNAL_ERROR;

    /* allocate space for new substitution string */
    subs = MALLOC(_LC_subs_t, collate.co_nsubs+1);

    /* Translate and allocate space for source string */
    src = copy_string(it0->value.str);
    
    /* Translate and allocate space for target string */
    tgt = copy_string(it1->value.str);

    /* Initialize substitution flag */
    flag = _SUBS_ACTIVE;
    
    /* check source and destination strings for regular expression
       special characters. If special characters are found then enable
       regular expressions in substitution string.
       */
    if (strpbrk(src, REGX_SPECIALS) != NULL)
      flag |= _SUBS_REGEXP;
    else if (strpbrk(tgt, REGX_SPECIALS) != NULL)
      flag |= _SUBS_REGEXP;

    /* Add source and target strings to newly allocated substitute list */
    for (i=0,j=0; i<collate.co_nsubs; i++,j++) {
	int   c;

	c = strcmp(src, collate.co_subs[i].ss_src);
	if (c < 0 && i == j) {
	    subs[j].ss_src = src;
	    subs[j].ss_tgt = tgt;
	    set_coll_wgt(&(subs[j].ss_act), flag, -1);
	    j++;
	} 
	subs[j].ss_src = collate.co_subs[i].ss_src;
	subs[j].ss_tgt = collate.co_subs[i].ss_tgt;
	subs[j].ss_act = collate.co_subs[i].ss_act;
    }
    if (i==j) {			
	/* either subs was empty or new substring is greater than any other
	   to date */
	subs[j].ss_src = src;
	subs[j].ss_tgt = tgt;
	set_coll_wgt(&(subs[j].ss_act), flag, -1);
    }

    /* increment substitute string count */
    collate.co_nsubs++;

    /* free space occupied by old list */
    free(collate.co_subs);
    
    /* attach new list to coll table */
    collate.co_subs = subs;

    destroy_item(it0);
    destroy_item(it1);
}


/*
*  FUNCTION: sem_collel_list
*
*  DESCRIPTION:
*  Process the set of symbols now on the semantic stack for the character 
*  for this particular order.
*/
void
sem_collel_list(_LC_weight_t *w, symbol_t *tgt, int order)
{
    extern _LC_collate_t collate;
    item_t       *i;		/* count item - # of items which follow */
    item_t       *si;		/* symbol item - item containing symbol */
    _LC_collel_t *ce;
    wchar_t	 weight;
    int          j;

    i = sem_pop();		/* pop count item */
    if (i == NULL || i->type != SK_INT)
	INTERNAL_ERROR;

    /*
      If the user specified more weights than orders, ignore the
      extra weights.  They would overwrite the character order.
    */
    if (order >= collate.co_nord) {
	diag_error(ERR_TOO_MANY_WEIGHTS, tgt->sym_id);
	for (j=0; j < i->value.int_no; j++) {
	    si = sem_pop();
	    if (si == NULL || si->type != SK_SYM)
		INTERNAL_ERROR;
	    destroy_item(si);
	}

	return;
    }

    if (i->value.int_no==1) {
	/* character gets collation value of symbol */
	si = sem_pop();
	if (si == NULL || si->type != SK_SYM)
	    INTERNAL_ERROR;

	switch (si->value.sym->sym_type) {

	  case ST_CHR_SYM:		/* character */
	    weight=get_coll_wgt(si->value.sym->data.chr->wgt,order);
	    if (weight == UNDEFINED) {

		/*
		  a symbol with UNDEFINED collation can only appear on the
		  RHS if it is also the target on the LHS
		*/
		if (si->value.sym == tgt) {
		    /* assign collation weight for self reference */
		    set_coll_wgt(si->value.sym->data.chr->wgt, 
				 nxt_coll_wgt(), -1);
		} else {
		    diag_error(ERR_FORWARD_REF, si->value.sym->sym_id);
		    return;
		}
	    }
	    set_coll_wgt(w, 
			 get_coll_wgt(si->value.sym->data.chr->wgt,
				      order), 
			 order);
	    break;
	    
	  case ST_COLL_ELL:	/* collation element */
	    ce = loc_collel(si->value.sym->data.collel->sym,
			    si->value.sym->data.collel->pc);
	    
	    weight=get_coll_wgt(&(ce->ce_wgt),order);
	    if (weight == UNDEFINED) {
		/*
		  a symbol with UNDEFINED collation can only appear on the
		  RHS if it is also the target on the LHS
		*/
		if (si->value.sym == tgt)
		    set_coll_wgt(&(ce->ce_wgt), nxt_coll_wgt(), -1);
		else {
		    diag_error(ERR_FORWARD_REF, si->value.sym->sym_id);
		    return;
		}
	    }
	    set_coll_wgt(w, get_coll_wgt(&(ce->ce_wgt), order), order);
	    break;

	  case ST_COLL_SYM:	/* collation symbol */
	    weight=get_coll_wgt(si->value.sym->data.collsym,order);
	    if (weight == UNDEFINED) {
		/*
		  a symbol with UNDEFINED collation can only appear on the
		  RHS if it is also the target on the LHS
		*/
		if (si->value.sym == tgt) {
		    set_coll_wgt(tgt->data.collsym, nxt_coll_wgt(), -1);

		} else {
		    diag_error(ERR_FORWARD_REF, si->value.sym->sym_id);
		    return;
		}
	    }
	    set_coll_wgt(w,
			 get_coll_wgt(si->value.sym->data.collsym,
				      order),
			 order);
	    break;
	  default:
	    INTERNAL_ERROR;
	}

    } else {
	/* 
	  collation substitution, i.e. <eszet>   <s><s>; <eszet>
	*/
	item_t **buf;
	item_t *xi;
	int    n;
	char   *subs;
	char   *srcs;
	char   *srcs_temp;
	_LC_weight_t *tgt_wgts;

	/* 
	  pop all of the collation elements on the semantic stack and
	  create a string from each of their encodings.
	*/
	subs = MALLOC(char, (i->value.int_no * MB_LEN_MAX) + 1);
	buf = MALLOC(item_t *, i->value.int_no);
	for (n=0; n < i->value.int_no; n++)
	    buf[n] = sem_pop();

	for (n=i->value.int_no-1; n >= 0; n--) {
	    if (buf[n]->type == SK_SYM) 
		strncat(subs, 
			(char *)buf[n]->value.sym->data.chr->str_enc, MB_LEN_MAX);
	    else
		INTERNAL_ERROR;

	    destroy_item(buf[n]);
	}
	free(buf);

	/* 
	  Get source string from target symbol.

	  tgt->data.chr->str_enc must be run through copy_string so that
          it is in the same format as collate substring (which is also
          run through copy_string

	*/
	if (tgt->sym_type == ST_COLL_ELL) {
	    srcs_temp = MALLOC(char, strlen(tgt->data.collel->str)+1);
	    strcpy(srcs_temp, tgt->data.collel->str);
	} else {
	    srcs_temp = MALLOC(char, MB_LEN_MAX+1);
	    strncpy(srcs_temp, (char *)tgt->data.chr->str_enc, MB_LEN_MAX);
	}

	srcs = copy_string(srcs_temp);

	/*
	  This section has been changed so that one-to-many mappings
	  can be used in bracket expressions, for example, "[<eszet>]"
	  or "[a-<eszet>]".

	  That didn't work before because one-to-many mappings were
	  being assigned a weight of SUB_STRING (ULONG_MAX - 1) in the
	  collation table, and SUB_STRING isn't a valid collation
	  weight.

	  When regcomp() handles bracket expressions, it gets weights
	  for the bracketed characters from the collation table.  It
	  would get a weight of SUB_STRING for a one-to-many mapping
	  like <eszet>, and complain that <eszet> was an invalid
	  collating element.

	  To fix this, the weight now used for one-to-many mappings is
	  the weight of the character being mapped.  So, for example,
	  the weight used for <eszet> is now <eszet>, not SUB_STRING.
	 */

	if (tgt->sym_type == ST_COLL_ELL) {
	    ce = loc_collel(tgt->data.collel->sym,
			    tgt->data.collel->pc);
	    tgt_wgts = &ce->ce_wgt;
	} else {
	    tgt_wgts = tgt->data.chr->wgt;
	}

	weight = get_coll_wgt(tgt_wgts, order);
	if (weight == UNDEFINED) {
	    weight = nxt_coll_wgt();
	    set_coll_wgt(tgt_wgts, weight, -1);
	}

	set_coll_wgt(w,weight,order);

	/* 
	  look for the src string in the set of collation substitution
	  strings alread defined.  If it is present, then just enable
	  it for this order.
	*/
	for (n=0; n< collate.co_nsubs; n++) {
	    _LC_weight_t *w;

	    w = &(collate.co_subs[n].ss_act);

	    if (strcmp(srcs, collate.co_subs[n].ss_src)==0) {
		set_coll_wgt(w, 
			     get_coll_wgt(w,order) | _SUBS_ACTIVE,
			     order);
		free(srcs);
		free(srcs_temp);
		free(subs);

		return;
	    }
	}
		
	/*
	  If this substitution has never been used before, then we
          need to create a new one.  Push source and substitution
	  strings on semantic stack and then call semantic action to
	  process substitution strings.  Reset active flag for all
	  except current order.
	*/
	xi = create_item(SK_STR, srcs_temp);
	sem_push(xi);

	xi = create_item(SK_STR, subs);
	sem_push(xi);

	sem_def_substr();
	
	/*
	  locate source string in substitution string array.  After
	  you locate it, fix the substitution flags to indicate which
	  order the thing is valid for.
        */
	for (n=0; n < collate.co_nsubs; n++) {
	    if (strcmp(collate.co_subs[n].ss_src, srcs)==0) {
		
		/* 
		  allocate weights array if not already done.
		*/
		if (collate.co_nord >= _COLL_WEIGHTS_INLINE 
		    && collate.co_subs[n].ss_act.p==NULL) {
		    collate.co_subs[n].ss_act.p = 
			MALLOC(wchar_t, collate.co_nord+1);
		}

		/*
		  turn off substitution for all but current order.
		*/
		set_coll_wgt(&(collate.co_subs[n].ss_act), 0, -1);
		set_coll_wgt(&(collate.co_subs[n].ss_act), 
			     _SUBS_ACTIVE, order);

		break;
	    }
	}
	free(srcs);
	free(srcs_temp);
	free(subs);

    } /* .....end collation substitution..... */
}


/*
*  FUNCTION: sem_set_collwgt
*
*  DESCRIPTION:
*  Assigns the collation weights in the argument 'weights' to the character
*  popped off the symbol stack.
*/
void
sem_set_collwgt(_LC_weight_t *weights, int last_order)
{
    extern _LC_collate_t collate;
    symbol_t     *s;
    int          i;
    _LC_weight_t *tgt;
    _LC_collel_t *ce;
    int next_order;
    wchar_t wgt;

    /*
      If the user specified more weights than orders, ignore the
      extra weights.  They would overwrite the character order.
     */
    if (last_order >= collate.co_nord)
	last_order = collate.co_nord - 1;

    s = sym_pop();
    switch (s->sym_type) {

      case ST_CHR_SYM:
	tgt = s->data.chr->wgt;
	if (tgt == NULL)
	    s->data.chr->wgt = 
		&(collate.co_coltbl[s->data.chr->wc_enc].ct_wgt);
	break;

      case ST_COLL_ELL:
	ce = loc_collel(s->data.collel->sym, s->data.collel->pc);
	tgt = &(ce->ce_wgt);
	break;

      case ST_COLL_SYM:
	tgt = s->data.collsym;
	break;

      default:
	INTERNAL_ERROR;
    }

    /*
     * The user may not have specified weights for all the orders.  If
     * that's the case, set the weights for the remaining orders to
     * the weight of the character itself.  For example, the following
     * order statement:
     *
     *     <a>    <a>;<a>;<a>
     *
     * should be identical to either of these two:
     *
     *     <a>    <a>;<a>
     *     <a>    <a>
     *
     * sem_collel_list() has already set the remaining weights if the 
     * character was specified as one of its own weights.  If it
     * wasn't, for example:
     *
     *     <A>    <a>
     *
     * then the weights for the remaining orders are all undefined.
     * In that case, set them to the next available weight. 
     */

    for (i=0; i<=last_order; i++)
	set_coll_wgt(tgt, get_coll_wgt(weights,i), i);

    if (last_order < collate.co_nord) {
	next_order = last_order + 1;
	if (get_coll_wgt(tgt, next_order) == UNDEFINED) {
	    wgt = nxt_coll_wgt();
	    for (i=next_order; i<=collate.co_nord; i++)
		set_coll_wgt(tgt, wgt, i);
	}
    }
}


/*
*  FUNCTION: sem_get_coll_tgt
*
*  DESCRIPTION:
*  Returns a pointer to the symbol on top of the symbol stack.
*/
symbol_t *sem_get_coll_tgt(void)
{
    symbol_t *s;

    s = sym_pop();

    sym_push(s);
    return s;
}


/* 
*  FUNCTION: sem_set_dflt_collwgt
*
*  DESCRIPTION:
*  Assign collation weight to character - set weight in symbol table
*  entry and in coltbl weight array.
*
*  The collation weight assigned is the next one available, i.e. the
*  default collation weight.
*/
void
sem_set_dflt_collwgt(void)
{
    extern _LC_collate_t collate;
    wchar_t      weight;
    symbol_t     *sym;
    wchar_t      pc;
    _LC_collel_t *ce;
    
    
    sym = sym_pop();
    if (sym->sym_type != ST_CHR_SYM 
	&& sym->sym_type != ST_COLL_SYM && sym->sym_type != ST_COLL_ELL)
	INTERNAL_ERROR;

    pc = sym->data.chr->wc_enc;

    switch (sym->sym_type) {	/* handle character */
      case ST_CHR_SYM:
	/* check if character already specified elswhere */
	if (get_coll_wgt(&(collate.co_coltbl[pc].ct_wgt),0) != UNDEFINED) {
	    diag_error(ERR_DUP_COLL_SPEC, sym->sym_id);
	    return;
	}

	/* get next available collation weight */
	weight = nxt_coll_wgt();

	/* place weight in colltbl */
	set_coll_wgt(&(collate.co_coltbl[pc].ct_wgt), 
		     weight, -1);

	/* put weight in symbol table entry for character. */
	sym->data.chr->wgt = 
	    &(collate.co_coltbl[pc].ct_wgt);
	break;

      case ST_COLL_ELL:
	ce = loc_collel(sym->data.collel->sym, sym->data.collel->pc);
	
	/* check if character already specified elswhere */
	if (get_coll_wgt(&(ce->ce_wgt), 0) != UNDEFINED) {
	    diag_error(ERR_DUP_COLL_SPEC, sym->sym_id);
	    return;
	}
	
	/* get next available collation weight */
	weight = nxt_coll_wgt();
	
	/* put weights in symbol table entry for character. */
	set_coll_wgt(&(ce->ce_wgt), weight, -1);
	
	break;

      case ST_COLL_SYM:
	/* check if character already specified elswhere */
	if (get_coll_wgt(sym->data.collsym, 0) != UNDEFINED) {
	    diag_error(ERR_DUP_COLL_SPEC, sym->sym_id);
	    return;
	}

	weight = nxt_coll_wgt();
	set_coll_wgt(sym->data.collsym, weight, -1);
	break;
      default:
	INTERNAL_ERROR;
    }
}


/* 
*  FUNCTION: sem_set_dflt_collwgt_range
*
*  DESCRIPTION:
*  Assign collation weights to a range of characters.  The functions
*  expects to find two character symbols on the semantic stack.
*
*  The collation weight assigned is the next one available, i.e. the
*  default collation weight.
*/
void
sem_set_dflt_collwgt_range(void)
{
    extern _LC_collate_t collate;
    symbol_t *s1, *s0;
    int      start, end;
    wchar_t  weight;
    int      wc;
    int      i;
    
    /* 
      Issue warning message that using KW_ELLIPSES results in the use of
      codeset encoding assumptions by localedef. 

      - required by POSIX.
    */
    diag_error(ERR_CODESET_DEP);

    /* pop symbols of symbol stack */
    s1 = sym_pop();
    s0 = sym_pop();
    
    /* 
      ensure that both of these symbols are characters and not collation
      symbols or elements 
    */
    if (s1->sym_type != ST_CHR_SYM || s0->sym_type != ST_CHR_SYM) {
	diag_error(ERR_INVAL_COLL_RANGE, s0->sym_id, s1->sym_id);
	return;
    }

    /* get starting and ending points in file code */
    start = s0->data.chr->fc_enc;
    end = s1->data.chr->fc_enc;

    /* invalid symbols in range ?*/
    if (start > end)
	error(ERR_INVAL_COLL_RANGE, s0->sym_id, s1->sym_id);
	
    for (i=start; i <= end; i++) {

	if ((wc = wc_from_fc(i)) >= 0) {
	    /* check if already defined elsewhere in map */
	    if (get_coll_wgt(&(collate.co_coltbl[wc].ct_wgt), 
			     0) != UNDEFINED) {
		diag_error(ERR_DUP_COLL_RNG_SPEC, "<???>"  ,s0->sym_id, s1->sym_id);
		return;
	    }
	    /* get next available collation weight */
	    weight = nxt_coll_wgt();

	    /* collation weights for symbols assigned weights in a range
	       are not accessible from the symbol , i.e.

	       s->data.chr->wgt[x] = weight;

	       cannot be assigned here since we don't have the symbol
	       which refers to the file code.
	    */

	    /* put weight in coll table at spot for wchar encoding */
	    set_coll_wgt(&(collate.co_coltbl[wc].ct_wgt), weight, -1);
	    
	}
    }
}


/*
*  FUNCTION: sem_sort_spec
*
*  DESCRIPTION:
*  This function decrements the global order by one to compensate for the 
*  extra increment done by the grammar, and then copies the sort modifier
*  list to each of the substrings defined thus far.
*/
void
sem_sort_spec(void)
{
    extern _LC_collate_t collate;
    extern symtab_t cm_symtab;
    extern wchar_t max_wchar_enc;
    symbol_t *s;
    item_t   *it;
    int      i,j;
    wchar_t    *buf;
    _LC_collel_t *ce;

    /*
      Add an extra order for the character order.  It will save
      the order of the characters between the order_start and
      order_end keywords, and will be used for range expressions
      in regular expressions.
     */
    it = create_item(SK_INT, _COLL_CHAR_ORDER_MASK);
    sem_push(it);
    collate.co_nord++;

    /*
     * The number of collation orders is one-based (at this point)
     * We change it to zero based, which is what the runtime wants
     */
    collate.co_nord--;

    /* collate.co_nord ready, time to set collating-element weights default */
    for (i=0; i<=max_wchar_enc; i++) {
      if (wchar_defined(i)) {
	if (collate.co_coltbl[i].ct_collel != NULL) {
	  for (j=0, ce=&(collate.co_coltbl[i].ct_collel[j]); 
	       ce->ce_sym != NULL; 
	       j++, ce=&(collate.co_coltbl[i].ct_collel[j])) {
	    /* this was originally done in sem_def_collel() */
	    set_coll_wgt(&(ce->ce_wgt), UNDEFINED, -1);
	  }
	}
      }
    }

    /*
      Get sort values from top of stack and assign to collate.co_sort
    */
    if (collate.co_nord < _COLL_WEIGHTS_INLINE) {
	for (i = collate.co_nord; i>=0; i--) {
	    it = sem_pop();
	    collate.co_sort.n[i] = it->value.int_no;
	    destroy_item(it);
	}
    } else {
	collate.co_sort.p = MALLOC(wchar_t, collate.co_nord+1);
	for (i = collate.co_nord; i>=0; i--) {
	    it = sem_pop();
	    collate.co_sort.p[i] = it->value.int_no;
	    destroy_item(it);
	}

	buf = MALLOC(wchar_t, (collate.co_nord+1)*(max_wchar_enc+1));
	for (i=0; i<=max_wchar_enc; i++) {
	    collate.co_coltbl[i].ct_wgt.p = buf;
	    buf += (collate.co_nord+1);
	    set_coll_wgt(&(collate.co_coltbl[i].ct_wgt), UNDEFINED, -1);
	}
    }

    /*
      Turn off the _SUBS_ACTIVE flag for substitution strings in orders
      where this capability is disabled.
      This is now done in setup_substr called from the grammar.
    */
    /* 
      seed the symbol table with IGNORE and UNDEFINED
    */

    /* 
      IGNORE gets a special collation value .  The xfrm and coll
      logic must recognize zero and skip a character possesing this collation
      value.
    */
    s = create_symbol("IGNORE", 0);
    s->sym_type = ST_COLL_SYM;
    s->data.collsym = MALLOC(_LC_weight_t, 1);
    set_coll_wgt(s->data.collsym, IGNORE, -1);
    add_symbol(&cm_symtab, s);
    
    /*
      Seed the symbol table with UNDEFINED to avoid a warning if the
      user does specify it in the collation order.  Without the seed,
      localedef would think UNDEFINED was a character, and warn that
      it hadn't been defined in the charmap.
    */
    s = create_symbol("UNDEFINED", 0);
    s->sym_type = ST_COLL_SYM;
    s->data.collsym = MALLOC(_LC_weight_t, 1);
    set_coll_wgt(s->data.collsym, UNDEFINED, -1);
    add_symbol(&cm_symtab, s);

    /*
      Set the minimum collation weight.  nxt_coll_wgt() updates the
      maximum weight each time it's called.
    */
    collate.co_col_min = nxt_coll_wgt();
}


/* 
*  FUNCTION: sem_def_collel
*
*  DESCRIPTION:
*  Defines a collation ellement. Creates a symbol for the collation element
*  in the symbol table, creates a collation element data structure for
*  the element and populates the element from the string on the semantic 
*  stack.
*/
void
sem_def_collel(void)
{
    extern _LC_collate_t collate;
    extern symtab_t cm_symtab;
    symbol_t     *sym_name;	/* symbol to be defined                 */
    item_t       *it;		/* string which is the collation symbol */
    wchar_t      pc;		/* process code for collation symbol    */
    _LC_collel_t *coll_sym;	/* collation symbol pointer             */
    char         *sym;		/* translated collation symbol          */
    int          n_syms;	/* no. of coll syms beginning with char */
    int          rc;
    int          i, j, skip;
    char	 *temp_ptr;


    sym_name = sym_pop();	/* get coll symbol name off symbol stack */
    it = sem_pop();		/* get coll symbol string off of stack */

    if (it->type != SK_STR)
	INTERNAL_ERROR;

    /* Create symbol in symbol table for coll symbol name */
    sym_name->sym_type = ST_COLL_ELL;
    sym_name->data.collel = MALLOC(coll_ell_t,1);
    add_symbol(&cm_symtab, sym_name);
    
    temp_ptr = copy(it->value.str); /* Expand without making printable */

    /* Translate collation symbol to file code */
    sym = copy_string(it->value.str);

    /* 
      Determine process code for collation symbol.  The process code for
      a collation symbol is that of the first character in the symbol.
    */
    rc = INT_METHOD(METH_OFFS(CHARMAP_MBTOWC))(&pc, temp_ptr, MB_LEN_MAX, NULL);

    if (rc < 0) {
	diag_error(ERR_ILL_CHAR, it->value.str);
	return;
    }
    skip = 0;
    for (i = 0; i < rc; i++) {
        if ((unsigned char)temp_ptr[i] < 128)
	    skip++;
	else
	    skip +=6;	/* Leave space for \\xnn\0 */
    }

    /* Now finished with the temp array, free it */
    free(temp_ptr);

    /* save process code and matching source str in symbol */
    /* do not put the first character in the src str */
    sym_name->data.collel->pc = pc;
    sym_name->data.collel->str = MALLOC(char, strlen(sym)+1);
    sym_name->data.collel->sym = sym_name->data.collel->str + skip;
    strcpy(sym_name->data.collel->str, sym);
    sym += skip;

    if (collate.co_coltbl[pc].ct_collel != NULL) {
	/* 
	  At least one collation symbol exists already --
	  Count number of collation symbols with the process code 
	*/
	for (i=0;
	     collate.co_coltbl[pc].ct_collel[i].ce_sym != NULL;
	     i++);
    
	/* 
	  Allocate memory for 
	     current number + new symbol + terminating null symbol
	*/
	coll_sym = calloc(i+2,sizeof(_LC_collel_t));
	n_syms = i;
    } else {
	/* 
	  This is the first collation symbol, allocate for 

	  new symbol + terminating null symbol
	*/
	coll_sym = calloc(2,sizeof(_LC_collel_t));
	n_syms = 0;
    }
    
    if (coll_sym == NULL)
	INTERNAL_ERROR;
    
    /* Add collation symbols to list in sorted order */
    for (i=j=0; i < n_syms; i++,j++) {
	int   c;

	c = strcmp(sym, collate.co_coltbl[pc].ct_collel[i].ce_sym);
	if (c < 0 && i == j) {
	    coll_sym[j].ce_sym = sym;
	    /*collate_cord not yet set, thus incorrect to call set_coll_wgt() */
	    /*set_coll_wgt(&(coll_sym[j].ce_wgt), UNDEFINED, -1);*/
	    coll_sym[j].ce_wgt.p = NULL;
	    j++;
	} 
	coll_sym[j].ce_sym = collate.co_coltbl[pc].ct_collel[i].ce_sym;
	/* collate_cord not yet set, thus incorrect to call set_coll_wgt() */
	/* set_coll_wgt(&(coll_sym[j].ce_wgt), UNDEFINED, -1);*/
	coll_sym[j].ce_wgt.p = NULL;
    }
    if (i==j) {
	/* 
	  either subs was empty or new substring is greater than any other
	  to date 
	*/
	coll_sym[j].ce_sym = sym;
	/* collate_cord not yet set, thus incorrect to call set_coll_wgt() */
	/*set_coll_wgt(&(coll_sym[j].ce_wgt), UNDEFINED, -1);*/
	coll_sym[j].ce_wgt.p = NULL;
	j++;
    }
    /* Add terminating NULL symbol */
    coll_sym[j].ce_sym = NULL;

    /* free space occupied by old list */
    if (n_syms>0)
	free(collate.co_coltbl[pc].ct_collel);
    
    /* attach new list to coll table */
    collate.co_coltbl[pc].ct_collel = coll_sym;

    destroy_item(it);
}


/* 
*  FUNCTION: sem_spec_collsym
*
*  DESCRIPTION:
*  Defines a placeholder collation symbol name.  These symbols are typically
*  used to assign collation values to a set of characters.
*/
void
sem_spec_collsym(void)
{
    extern symtab_t cm_symtab;
    symbol_t *sym,*t;

    sym = sym_pop();		/* get coll symbol name off symbol stack */

    t = loc_symbol(&cm_symtab,sym->sym_id,0);
    if (t != NULL)
	diag_error(ERR_DUP_COLL_SYM,sym->sym_id);
    else {
        /* Create symbol in symbol table for coll symbol name */
        sym->sym_type = ST_COLL_SYM;
        sym->data.collsym = calloc(1, sizeof(_LC_weight_t));
        set_coll_wgt(sym->data.collsym, UNDEFINED, -1);
        add_symbol(&cm_symtab, sym);
    }
}


/*
*  FUNCTION: count_undefined
*
*  DESCRIPTION:
*  Counts the number of characters and collating elements with
*  undefined collation weights.
*/
static int
count_undefined(void)
{
    extern _LC_collate_t collate;
    extern wchar_t max_wchar_enc;
    int num_undefined;
    int i, j;
    _LC_collel_t *ce;

    num_undefined = 0;

    if (collate.co_nord < _COLL_WEIGHTS_INLINE) {
	for (i=0; i<=max_wchar_enc; i++) {
	    if (wchar_defined(i)) {
		if (collate.co_coltbl[i].ct_wgt.n[0] == UNDEFINED)
		    num_undefined++;

		if (collate.co_coltbl[i].ct_collel != NULL) {
		    for (j=0, ce=&(collate.co_coltbl[i].ct_collel[j]);
		         ce->ce_sym != NULL;
		         j++, ce=&(collate.co_coltbl[i].ct_collel[j])) {
			if (ce->ce_wgt.n[0] == UNDEFINED)
			    num_undefined++;
		    }
		}
	    }
	}
    }
    else {
	for (i=0; i<=max_wchar_enc; i++) {
	    if (wchar_defined(i)) {
		if (collate.co_coltbl[i].ct_wgt.p[0] == UNDEFINED)
		    num_undefined++;

		if (collate.co_coltbl[i].ct_collel != NULL) {
		    for (j=0, ce=&(collate.co_coltbl[i].ct_collel[j]);
		         ce->ce_sym != NULL;
		         j++, ce=&(collate.co_coltbl[i].ct_collel[j])) {
			if (ce->ce_wgt.p[0] == UNDEFINED)
			    num_undefined++;
		    }
		}
	    }
	}
    }

    return num_undefined;
}


/*
*  FUNCTION: shift_wgts
*
*  DESCRIPTION:
*  Shifts up all the collation weights from 'initial_wgt' through the
*  maximum weight by 'shift_amt'.
*
*  The reason we can't just add 'shift_amt' to each weight is that it
*  might produce weights containing bytes of 0x00, and those weights
*  are invalid.  So instead, we use nxt_coll_wgt() to create a
*  mapping from the original weights to their new, shifted values.
*/
static wchar_t
shift_wgts(wchar_t initial_wgt, int shift_amt)
{
    extern _LC_collate_t collate;
    extern wchar_t max_wchar_enc;
    int  num_shifted_wgts;
    wchar_t *shifted_wgts;
    wchar_t col_max;
    wchar_t wgt;
    int i, j, k;
    _LC_collel_t *ce;

    /* Allocate a table for the mapping. */
    num_shifted_wgts = collate.co_col_max - initial_wgt + 1;
    shifted_wgts = MALLOC(wchar_t, num_shifted_wgts);

    /*
      Mark the valid weights in the table; they're the only ones we
      need to shift.  nxt_coll_wgt() updates collate.co_col_max, so if
      we didn't use a copy of it in the loop test, the loop would loop
      forever on the "last" weight.
    */
    col_max = collate.co_col_max;
    nxt_coll_val = initial_wgt;
    while ((wgt = nxt_coll_wgt()) <= col_max)
	shifted_wgts[wgt - initial_wgt] = 1;

    /* Advance the weight counter by the shift amount. */
    nxt_coll_val = initial_wgt;
    for (i=0; i<shift_amt; i++)
	(void) nxt_coll_wgt();

    /* Fill in the table with the shifted weights. */
    for (i=0; i<num_shifted_wgts; i++)
	if (shifted_wgts[i])
	    shifted_wgts[i] = nxt_coll_wgt();

    /* Update the weights in the locale's collation table. */
    if (collate.co_nord < _COLL_WEIGHTS_INLINE) {
	for (i=0; i<=max_wchar_enc; i++) {

	    if (wchar_defined(i)){
	        for (j=0; j<=collate.co_nord; j++) {
		    wgt = collate.co_coltbl[i].ct_wgt.n[j];
		    if ((wgt >= initial_wgt) &&
			(wgt != UNDEFINED) &&
			(wgt != IGNORE))
			collate.co_coltbl[i].ct_wgt.n[j] =
			    shifted_wgts[wgt - initial_wgt];
		}

		if (collate.co_coltbl[i].ct_collel != NULL) {

		    for (j=0, ce=&(collate.co_coltbl[i].ct_collel[j]);
		         ce->ce_sym != NULL;
		         j++, ce=&(collate.co_coltbl[i].ct_collel[j])) {

			for (k=0; k<=collate.co_nord; k++) {
			    wgt = ce->ce_wgt.n[k];
			    if ((wgt >= initial_wgt) &&
				(wgt != UNDEFINED) &&
				(wgt != IGNORE))
				ce->ce_wgt.n[k] =
				    shifted_wgts[wgt - initial_wgt];
			}
		    }
		}
	    }
	}
    } else {
	for (i=0; i<=max_wchar_enc; i++) {

	    if (wchar_defined(i)){
	        for (j=0; j<=collate.co_nord; j++) {
		    wgt = collate.co_coltbl[i].ct_wgt.p[j];
		    if ((wgt >= initial_wgt) &&
			(wgt != UNDEFINED) &&
			(wgt != IGNORE))
			collate.co_coltbl[i].ct_wgt.p[j] =
			    shifted_wgts[wgt - initial_wgt];
		}

		if (collate.co_coltbl[i].ct_collel != NULL) {

		    for (j=0, ce=&(collate.co_coltbl[i].ct_collel[j]);
		         ce->ce_sym != NULL;
		         j++, ce=&(collate.co_coltbl[i].ct_collel[j])) {

			for (k=0; k<=collate.co_nord; k++) {
			    wgt = ce->ce_wgt.p[k];
			    if ((wgt >= initial_wgt) &&
				(wgt != UNDEFINED) &&
				(wgt != IGNORE))
				ce->ce_wgt.p[k] =
				    shifted_wgts[wgt - initial_wgt];
			}
		    }
		}
	    }
	}
    }

    /* Return the new, shifted value of the initial weight. */
    initial_wgt = shifted_wgts[0];
    free(shifted_wgts);
    return initial_wgt;
}


/*
*  FUNCTION: sem_collate
*
*  DESCRIPTION:
*  Post processing for collation table which consists of the location
*  and assignment of specific value for <HIGH> and UNDEFINED collation
*  weights.
*/
void
sem_collate(void)
{
    extern _LC_collate_t collate;
    extern wchar_t max_wchar_enc;
    extern symtab_t cm_symtab;
    _LC_weight_t *undefined;
    _LC_collel_t *ce;
    symbol_t     *s;
    int          i, j, k;
    int          warn=FALSE;		/* Local flag to hide extra errors */
    wchar_t      undefined_wgt;
    wchar_t      nxt_wgt;
    wchar_t      shifted_nxt_wgt;
    int          num_undefined;

    /*
      If the collation order didn't specify UNDEFINED, and it didn't
      specify all the characters in the charmap, then POSIX says to
      print a warning and stick the missing characters at the end of
      the order.
     */
    s = loc_symbol(&cm_symtab, "UNDEFINED", 0);
    if (s==NULL)
	INTERNAL_ERROR;
    undefined = s->data.collsym;
    if (get_coll_wgt(undefined, 0)==UNDEFINED) {
	warn = TRUE;
	set_coll_wgt(undefined, nxt_coll_wgt(), -1);
    }

    /*
      If the collation order specified UNDEFINED at the beginning or
      in the middle, then we need to insert any undefined characters
      at that spot.  If there's only one undefined character, we can
      just give it the weight of the UNDEFINED symbol.  If there are
      others, we need to shift the weights following the UNDEFINED
      symbol up and out of the way, leaving a gap that the undefined
      characters can use.
    */
    shifted_nxt_wgt = 0;
    undefined_wgt = get_coll_wgt(undefined, collate.co_nord);
    if (undefined_wgt != collate.co_col_max) {
	num_undefined = count_undefined();
	if (num_undefined > 1) {
	    nxt_coll_val = undefined_wgt;
	    (void) nxt_coll_wgt();	  /* increments nxt_coll_val */
	    nxt_wgt = nxt_coll_val;
	    shifted_nxt_wgt = shift_wgts(nxt_wgt, num_undefined - 1);
	}
    }

    /*
      This makes the next call to nxt_coll_wgt() return the weight
      of the UNDEFINED symbol.  That's where the weights for the
      undefined characters should start.
    */
    nxt_coll_val = undefined_wgt;

    /* 
      Substitute symbols with UNDEFINED weights
      for the weights ultimately determined for UNDEFINED.
    */
    if (collate.co_nord < _COLL_WEIGHTS_INLINE) {
	for (i=0; i<=max_wchar_enc; i++) {

	    if (wchar_defined(i)) {

	        for (j=0; j <  collate.co_nord; j++) {
		    if (collate.co_coltbl[i].ct_wgt.n[j] == UNDEFINED) {
		        collate.co_coltbl[i].ct_wgt.n[j] = undefined->n[j];
		        if (warn)
			    diag_error(ERR_NO_UNDEFINED);
		        warn = FALSE;
		    }
		if (collate.co_coltbl[i].ct_wgt.n[j] == (wchar_t)IGNORE)
		    collate.co_coltbl[i].ct_wgt.n[j] = 0;
	        }

		if (collate.co_coltbl[i].ct_wgt.n[collate.co_nord] ==
		    UNDEFINED)
		    collate.co_coltbl[i].ct_wgt.n[collate.co_nord] =
			nxt_coll_wgt();

	        if (collate.co_coltbl[i].ct_collel != NULL) {

		    for (j=0, ce=&(collate.co_coltbl[i].ct_collel[j]); 
		         ce->ce_sym != NULL; 
		         j++, ce=&(collate.co_coltbl[i].ct_collel[j])) {

		        for (k=0; k< collate.co_nord; k++) {
			    if (ce->ce_wgt.n[k] == UNDEFINED)
			        ce->ce_wgt.n[k] = undefined->n[k];
			    if (ce->ce_wgt.n[k] == (wchar_t) IGNORE)
				ce->ce_wgt.n[k] = 0;
		        }

			if (ce->ce_wgt.n[collate.co_nord] == UNDEFINED)
			    ce->ce_wgt.n[collate.co_nord] = nxt_coll_wgt();
		    }
	        }
	    }
	}
    } else {
	for (i=0; i<=max_wchar_enc; i++) {

	    if (wchar_defined(i)){
	        for (j=0; j <  collate.co_nord; j++) {
		    if (collate.co_coltbl[i].ct_wgt.p[j] == UNDEFINED) {
		        collate.co_coltbl[i].ct_wgt.p[j] = undefined->p[j];
		        if (warn)
			    diag_error(ERR_NO_UNDEFINED);
		        warn = FALSE;
		    }
		if (collate.co_coltbl[i].ct_wgt.p[j] == (wchar_t) IGNORE)
		    collate.co_coltbl[i].ct_wgt.p[j] = 0;
	        }

		if (collate.co_coltbl[i].ct_wgt.p[collate.co_nord] ==
		    UNDEFINED)
		    collate.co_coltbl[i].ct_wgt.p[collate.co_nord] =
			nxt_coll_wgt();

	        if (collate.co_coltbl[i].ct_collel != NULL) {

		    for (j=0, ce=&(collate.co_coltbl[i].ct_collel[j]); 
		         ce->ce_sym != NULL; 
		         j++, ce=&(collate.co_coltbl[i].ct_collel[j])) {
		        for (k=0; k< collate.co_nord; k++) {
			    if (ce->ce_wgt.p[k] == UNDEFINED)
			        ce->ce_wgt.p[k] = undefined->p[j];
			    if (ce->ce_wgt.p[k] == (wchar_t) IGNORE)
				ce->ce_wgt.p[k] = 0;
		        }

			if (ce->ce_wgt.p[collate.co_nord] == UNDEFINED)
			    ce->ce_wgt.p[collate.co_nord] = nxt_coll_wgt();
		    }
	        }
	    }
        }
    }

    /*
      If we had to shift weights, the weight counter should now be at
      the start of the shifted area.  Check that.
    */
    if (shifted_nxt_wgt && (nxt_coll_val != shifted_nxt_wgt))
	INTERNAL_ERROR;
}

/*
*  FUNCTION: setup_substr
*
*  DESCRIPTION:
*  Set-up the collation weights for the substitute strings defined in
*  the collation section using the keyword "substitute". This is executed
*  after the order keyword (at which time we now know how many orders there
*  are and if any have subs turned off).
*
*/
void
setup_substr(void)
{

	extern _LC_collate_t collate;
	int n, i;
	int flag_subs;
	int flag_nosubs;

	flag_nosubs = 0;
	flag_subs = 1;

	for (n=0; n < collate.co_nsubs; n++) {

	    if (collate.co_nord >= _COLL_WEIGHTS_INLINE) {
   	        collate.co_subs[n].ss_act.p = MALLOC(wchar_t,collate.co_nord+1);

		for (i = 0; i <= collate.co_nord; i++) {

		    if (collate.co_sort.p[i] & _COLL_NOSUBS_MASK)
			collate.co_subs[n].ss_act.p[i] = flag_nosubs;
		    else
			collate.co_subs[n].ss_act.p[i] = flag_subs;
		}
	    }
	    else {

		for (i = 0; i <= collate.co_nord; i++) {

		    if (collate.co_sort.n[i] & _COLL_NOSUBS_MASK)
			collate.co_subs[n].ss_act.n[i] = flag_nosubs;
		    else
			collate.co_subs[n].ss_act.n[i] = flag_subs;
		}
	    }
	}
}




