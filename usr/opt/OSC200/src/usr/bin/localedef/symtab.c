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
static char     *sccsid = "@(#)$RCSfile: symtab.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/10 23:13:24 $";
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
 * 1.2  com/cmd/nls/symtab.c, bos320 5/10/91 10:18:07
 */

#include <stdlib.h>
#include <string.h>
#include "err.h"
#include "symtab.h"


/*
*  FUNCTION: cmp_symbol
*
*  DESCRIPTION: 
*  Comparator used to determine equivalence of two symbols.
*/
int
cmp_symbol(symbol_t *sym, char *id, int scope)
{
    int  c;
    
    c = strcmp(sym->sym_id, id);
    if (c == 0)
	return sym->sym_scope - scope;
    else
	return c;
}


/*
*  FUNCTION: create_symbol
*
*  DESCRIPTION:
*  Creates an instance of a symbol_t.  This routine malloc()s the space
*  necessary to contain the symbol_t but not for it's data.
*/
symbol_t *
create_symbol(char *id, int scope)
{
    symbol_t *sym;
    
    sym = MALLOC(symbol_t, 1);
    sym->sym_id = strdup(id);

    sym->sym_scope = scope;
    sym->data.str = NULL;
    sym->next = NULL;
    
    return sym;
}


/*
*  FUNCTION: hash
*
*  DESCRIPTION: 
*  The hash function used to determine in which hash bucket a symbol will
*  go.  This hash function is particularly well suited to symbols which use
*  patterns of numbers, e.g. <j2436>, due to POSIX's recommendation that
*  character names be so defined.  Symbols containing the same numbers in
*  different positions is handled by making characters positionally 
*  significant.
*/
int
hash(char *id)
{
    int i =0;
    int hashval = 0;

    while (*id != '\0') {
	i++;
	hashval += *id * i;
	id++;
    }

    hashval &= 0x7FFFFFFF;
    hashval %= HASH_TBL_SIZE;

    return hashval;
}


/*
*  FUNCTION: add_symbol
*
*  DESCRIPTION:
*  Adds a symbol to the symbol table.  The symbol table is implemented
*  as a hash table pointing to linked lists of symbol entries.
*
*      +--- array of pointers to lists indexed by hash(symbol->sym_id)
*      |
*      |
*      V      /--linked list of symbols --\
*  +------+   +------+-+         +------+-+
*  |    --+-->|      |-+-- ...-->|      |^|
*  +------+   +------+_+         +------+-+
*  |    --+--> . . .
*  +------+   
*     .
*     .
*     .
*  +------+   +------+-+         +------+-+
*  |    --+-->|      |-+-- ...-->|      |^|
*  +------+   +------+-+         +------+-+
*/
int
add_symbol(symtab_t *sym_tab, symbol_t *sym)
{
    symbol_t *p;
    symbol_t *t;
    int      c;
    int      hdx;
    
    hdx = hash(sym->sym_id);
    t = &(sym_tab->symbols[hdx]);
    
    c = -1;
    for (p = t;
	 p->next != NULL && 
	 (c=cmp_symbol(p->next, sym->sym_id, sym->sym_scope)) < 0;
	 p = p->next);
    
    if (c==0)
	return ST_DUP_SYMBOL;
    else {
	sym_tab->n_symbols++;
	sym->next = p->next;
	p->next = sym;
	return ST_OK;
    }
}


/*
*  FUNCTION: loc_symbol
*
*  DESCRIPTION:
*  Locates a symbol with sym_id matching 'id' in the symbol table 'sym_tab'.
*  The functions hashes 'id' and searches the linked list indexed for 
*  a matching symbol.  See comment for add_symbol for detail of symbol
*  table structure.
*/
symbol_t *
loc_symbol(symtab_t *sym_tab, char *id, int scope)
{
    symbol_t *p;

    int      c=-1;
    int      hdx;
    
    hdx = hash(id);
    p = &sym_tab->symbols[hdx];

    for(p = p->next;		/* Top of hash isn't a real symbol_t */
	p != NULL;
	p=p->next) {

	/* Walk thru the hash chain looking for the matching symbol */
	
	c=cmp_symbol(p, id, scope);
	
	if (c==0)		/* MATCH */
	  return (p);
	else if (c > 0)		/* OVER-RUN, none possible */
	  return (NULL);
    }
    
    return NULL;
}


/* static implementing symbol stack */
static int stack_top = ST_MAX_DEPTH;
static symbol_t *stack[ST_MAX_DEPTH];

/*
*  FUNCTION: sym_push
*
*  DESCRIPTION:
*  Pushes a symbol on the symbol stack.
*/
int
sym_push(symbol_t *sym)
{
    if (stack_top > 0) {
	stack[--stack_top] = sym;
	return ST_OK;
    } else
	INTERNAL_ERROR;
}


/*
*  FUNCTION: sym_pop
*
*  DESCRIPTION:
*  Pops a symbol off the symbol stack, returning it's address to the caller.
*/
symbol_t *
sym_pop(void)
{
    if (stack_top < ST_MAX_DEPTH)
	return stack[stack_top++];
    else 
	return NULL;
}


