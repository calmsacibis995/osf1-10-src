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
 */

#ifndef lint
static char     *sccsid = "@(#)$RCSfile: vwi-symbol.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:52:30 $";
#endif
/*  DEC/CMS REPLACEMENT HISTORY, Element VWI-SYMBOL.C */
/*  *2    11-AUG-1989 15:05:58 DECWBUILD "Initial entry" */
/*  *1    11-AUG-1989 11:10:44 DECWBUILD "Initial entry of V5.1 sources" */
/*  DEC/CMS REPLACEMENT HISTORY, Element VWI-SYMBOL.C */


/*
**  FACILITY:
**
**   VWI -- VOILA Writer Interface
**
** ABSTRACT:
**
**   This module implements the symbol table management routines for VWI
**
** AUTHORS:
**
**   Joseph M. Kraetsch
**
** CREATION DATE: 27-APR-1988
**
** MODIFICATION HISTORY:
*/

/*  
**  INCLUDE FILES  
*/

# include   "vxi-def.h"
#ifdef vms
# include  <stdlib.h>
#endif

# define PME__L_REF_COUNT PME__L_CHUNK_LIST

unsigned    VWI_SYMBOL_LOOKUP ();
void	    VWI_SYMBOL_DEFINE ();
void	    VWI_SYMBOL_UNDEF ();
void	    VWI_SYMBOL_PATCH ();
UNDEFSYM    *VWI_SYMBOL_COMPLAIN ();

#ifdef vms
globalref int  VWI__GL_PREV_PAGE;
#else
extern    int  VWI__GL_PREV_PAGE;
#endif

unsigned VWI_SYMBOL_LOOKUP (bkb, symbol)
BKB  *bkb;	    /*  Book id (from VWI__CREATE_BOOK)		    */
char *symbol;	    /*  Symbol to find  */
{
    VBH		*vbh;		/*  VOILA book header		*/
    PME		*pgmap;		/*  page map  */
    PME		*pme;
    unsigned	ckid;
    char	*testsym;
    
    vbh = &bkb->BKB__V_VBH;
    pgmap = bkb->BKB__L_PG_MAP;
    pme = pgmap + vbh->VBH__L_SYMBOL_PGID;
    testsym = (char *) &pme->PME__L_PG_BUFF->PAGE__V_DATA[6];

    for (ckid = 1; ckid <= vbh->VBH__L_NUM_CHUNKS; ckid++)
	if (strcmp (symbol, testsym) == 0)
	    return ckid;
	else
	    testsym += 32;

    return 0;
}


void VWI_SYMBOL_DEFINE (bkb, ckid, symbol)
BKB        *bkb;	    /*  Book id (from VWI__CREATE_BOOK)		    */
unsigned    ckid;	    /*  Chunk id of new symbol  */
char	   *symbol;	    /*  Symbol to find  */
{
    UNDEF	*testsym;
    UNDEF	*lastsym;
    
    testsym =  bkb->BKB__L_SYMBOLS;

    while (testsym != 0)
    {
	if (strcmp (testsym->name, symbol) == 0)    /*  found one  */
	{
	    /* patch the reference  */

	    VWI_SYMBOL_PATCH (bkb, testsym->pgid, testsym->offset, ckid);

	    /* remove it from the list  */

	    if ( testsym == bkb->BKB__L_SYMBOLS )
		    bkb->BKB__L_SYMBOLS = testsym->next;
	    else
	    	    lastsym->next = testsym->next;

/*	    free (testsym); */

	}
	else	lastsym = testsym;

	testsym = testsym->next;
    }
}

void VWI_SYMBOL_UNDEF (bkb, pgid, offset, ckid, name)
BKB        *bkb;
unsigned    pgid;
unsigned    offset;
unsigned    ckid;
char	    *name;
{
    PME		*pgmap;		/*  page map  */
    PME		*pme;
    UNDEF	*symbol;
    
    symbol = (UNDEF *)malloc (sizeof (UNDEF));
    symbol->next = bkb->BKB__L_SYMBOLS;
    symbol->pgid = pgid;
    symbol->offset = offset;
    symbol->parent_ckid = ckid;
    strncpy (symbol->name, name, 32);
    bkb->BKB__L_SYMBOLS = symbol;

    pgmap = bkb->BKB__L_PG_MAP;
    pme = pgmap + pgid;
    pme->PME__L_REF_COUNT += 1;
}

void VWI_SYMBOL_PATCH (bkb, pgid, offset, ckid)
BKB        *bkb;
unsigned    pgid;
unsigned    offset;
unsigned    ckid;
{
    PME		*pgmap;		/*  page map  */
    PME		*pme;
    char	*patch;		/*  location to patch  */
    
    pgmap = bkb->BKB__L_PG_MAP;
    pme = pgmap + pgid;

    patch = (char *) &pme->PME__L_PG_BUFF->PAGE__V_DATA[offset];
    put_int ( patch, ckid );
    pme->PME__L_REF_COUNT -= 1;

    if (pme->PME__L_REF_COUNT == 0 && pme->PME__B_FLAGS.PME__B_BITS.PME__V_CLOSE
	    && pgid != VWI__GL_PREV_PAGE)
	VWI_TOPIC_CLOSE (bkb, pgid);
}

UNDEFSYM *VWI_SYMBOL_COMPLAIN (bkb)
BKB *bkb;	    /*  Book id (from VWI__CREATE_BOOK)		    */
{
    UNDEF	*testsym;
    UNDEFSYM	*symbol, *lastsym;
        
    testsym = bkb->BKB__L_SYMBOLS;
    if (testsym == NULL)
	return FALSE;

    symbol = (UNDEFSYM *)malloc (sizeof (UNDEFSYM));
    symbol->next = NULL;

    while (testsym != NULL)
    {
	symbol->ckid = testsym->parent_ckid;
	strncpy (symbol->name, testsym->name, 32);
	testsym = testsym->next;
	if(testsym != NULL) {
	    lastsym = symbol;
	    symbol = (UNDEFSYM *)malloc (sizeof (UNDEFSYM));
	    symbol->next = lastsym;
	    }
	    
    }
    return(symbol);
}
