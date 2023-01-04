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
/****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1989 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**/


/*
**  FACILITY:
**
**   BWI -- BOOKREADER Writer Interface
**
** ABSTRACT:
**
**   This module implements the symbol table management routines for BWI
**
** AUTHORS:
**
**   Joseph M. Kraetsch
**   Michael Fitzell
**
** CREATION DATE: 27-APR-1988
**
** MODIFICATION HISTORY:
*/

/*  
**  INCLUDE FILES  
*/

# include   "bxi_def.h"
#ifdef vms
# include  <stdlib.h>
#endif

# define PME$L_REF_COUNT PME$L_CHUNK_LIST
# define    SYMBOL_LENGTH   32

unsigned    BWI_SYMBOL_LOOKUP ();
BWI_ERROR   bwi_symbol_define ();
void	    BWI_SYMBOL_UNDEF ();
void	    BWI_SYMBOL_PATCH ();
UNDEFSYM    *bwi_symbol_complain ();
BWI_ERROR   bwi_symbol_temp();

#ifdef vms
globalref int  VWI$GL_PREV_PAGE;
#else
extern    int  VWI$GL_PREV_PAGE;
#endif

/***********************************************************************/
/*   BWI_SYMBOL_LOOKUP - walks down the temporary and permanent symbol 
/*			 table lists (temp = a-tool generated) looking 
/*			 for a match returns the index if found NULL if 
/*			 the symbol isn't defined yet.
/*
/**********************************************************************/
unsigned BWI_SYMBOL_LOOKUP (bkb, symbol)
BKB  *bkb;	    /*  Book id (from BWI$CREATE_BOOK)		    */
char *symbol;	    /*  Symbol to find  */
{
    BKH		*bkh;		/*  VOILA book header		*/
    BKH_EXT     *bkh_ext;	/* extension header for V2.2	*/
    PME		*pgmap;		/*  page map  */
    PME		*pme;
    unsigned	ckid;
    char	*testsym;
    BWI_SYMBOL_ENTRY        *sym_tab_entry;
    BWI_TEMPSYM     *temp_sym_entry;
    
    bkh = &bkb->BKB$V_BKH;
    bkh_ext = &bkb->BKB$V_BKH_EXT;
    pgmap = bkb->BKB$L_PG_MAP;
    pme = pgmap + bkh->BKH$L_SYMBOL_PGID;
    testsym = (char *) &pme->PME$L_PG_BUFF->PAGE$V_DATA[6];

    for (ckid = 1; ckid <= bkh_ext->BKEH$L_NUM_SYMBOLS; ckid++) {
        sym_tab_entry = (BWI_SYMBOL_ENTRY *) testsym;

        if (strcmp (symbol, sym_tab_entry->name) == 0)
            return sym_tab_entry->id;

        testsym += sym_tab_entry->len;
    }
    /* didn't find it in the permanent symbol table so lets look in the temp*/

    temp_sym_entry = bkb->BKB$L_TEMP_SYMBOLS;
    while(temp_sym_entry != 0)
    {
        if (strcmp (symbol, temp_sym_entry->name) == 0)
                return temp_sym_entry->parent_ckid;

        temp_sym_entry = (BWI_TEMPSYM *)temp_sym_entry->next;
    }
    return 0;
};


BWI_ERROR bwi_symbol_define (bkb, ckid, symbol, flag)
BKB        *bkb;            /*  Book id (from VWI$CREATE_BOOK)              */
unsigned    ckid;           /*  Chunk id of new symbol  */
char       *symbol;         /*  Symbol to find  */
char       flag;            /*  TRUE if it's a non-generated symbol */
{

    UNDEF       *testsym;
    UNDEF       *lastsym;
    BKH		*bkh;
    BKH_EXT     *bkh_ext;
    char	sym_buff[SYMBOL_LENGTH];   /*  for processing symbol names  */
    unsigned int status,len;
    int		i, pad;
    BWI_SYMBOL_ENTRY sym_tab_entry;
    char	*sym_name;

    if(!verify_bkid (bkb))
        return(BwiInvalidBookId);

    if(!verify_ckid(bkb, ckid))         /* check for valid topic_id */
        return(BwiInvalidChunkId);

    if (!symbol[0])
        return(BwiOK);

    /* set up pointer to the book header */
    bkh = &bkb->BKB$V_BKH;
    bkh_ext = &bkb->BKB$V_BKH_EXT;

    /* set up pointer to the undefined symbols list */
    testsym =  bkb->BKB$L_SYMBOLS;

    sym_name = sym_tab_entry.name;

    memset ( sym_name , 0, SYMBOL_LENGTH );

    /*  Convert to lower case for the Ultrix people  */
/* DOUBLE CHECK WE WANT TO DO THIS *MJF*  **/
    for(i=0; i < (strlen(symbol)); i++)
        sym_buff[i] = tolower(symbol[i]);

    sym_buff[i] = '\0';

    /* fill in the symbol table entry struct and make sure it's padded */
    len = strlen(symbol)+1;
    sym_tab_entry.len = 1 + 4 + len;
    pad = 4 - (sym_tab_entry.len % 4);
    sym_tab_entry.len += pad;
    sym_buff[len] = '\0';

    sym_tab_entry.id = ckid;

    strcpy(sym_tab_entry.name,sym_buff);

    if(flag == TRUE)
    {
        if((status = VWI$WRITE_PAGE (bkb, bkh->BKH$L_SYMBOL_PGID,
                                 sym_tab_entry.len, &sym_tab_entry)) != BwiOK)
                return(status);


        /* increment the # of symbols defined */
        bkh_ext->BKEH$L_NUM_SYMBOLS++;
    }
    else
    {
        (void) bwi_symbol_temp(bkb, ckid, sym_buff);
    }

 /* patch the chunk ID of this symbol in case of previous references */
    while (testsym != 0)
        {
            if (strcmp (testsym->name, sym_buff) == 0)    /*  found one  */
            {
                /* patch the reference  */

                BWI_SYMBOL_PATCH (bkb, testsym->pgid, testsym->offset, ckid);

                /* remove it from the list  */

                if ( testsym == bkb->BKB$L_SYMBOLS )
                    bkb->BKB$L_SYMBOLS = testsym->next;
                else
                    lastsym->next = testsym->next;

        /*          free (testsym); */

            }
                else    lastsym = testsym;

            testsym = testsym->next;
        }
    return(BwiOK); 
}

/*********************************************************************
/*
/* VWI_SYMBOL_TEMP - Stores temporary symbols in a linked list
/*
/*********************************************************************/
BWI_ERROR bwi_symbol_temp (bkb, ckid, name)
BKB        *bkb;
unsigned    ckid;
char        *name;
{
    PME         *pgmap;         /*  page map  */
    PME         *pme;
    BWI_TEMPSYM     *symbol;

    symbol = (BWI_TEMPSYM *)malloc (sizeof (BWI_TEMPSYM));
    if(symbol == NULL)
        return(BwiFailMalloc);
    symbol->next = (BWI_TEMPSYM *)bkb->BKB$L_TEMP_SYMBOLS;
    symbol->parent_ckid = ckid;
    strcpy (symbol->name, name);
    bkb->BKB$L_TEMP_SYMBOLS = symbol;

    return(BwiOK);
}

void BWI_SYMBOL_UNDEF (bkb, pgid, offset, ckid, name)
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
    symbol->next = bkb->BKB$L_SYMBOLS;
    symbol->pgid = pgid;
    symbol->offset = offset;
    symbol->parent_ckid = ckid;
    strcpy (symbol->name, name);
    bkb->BKB$L_SYMBOLS = symbol;

    pgmap = bkb->BKB$L_PG_MAP;
    pme = pgmap + pgid;
    pme->PME$L_REF_COUNT += 1;
}

void BWI_SYMBOL_PATCH (bkb, pgid, offset, ckid)
BKB        *bkb;
unsigned    pgid;
unsigned    offset;
unsigned    ckid;
{
    PME		*pgmap;		/*  page map  */
    PME		*pme;
    char	*patch;		/*  location to patch  */
    
    pgmap = bkb->BKB$L_PG_MAP;
    pme = pgmap + pgid;

    patch = (char *) &pme->PME$L_PG_BUFF->PAGE$V_DATA[offset];
    put_int ( patch, ckid );
    pme->PME$L_REF_COUNT -= 1;

    if (pme->PME$L_REF_COUNT == 0 && pme->PME$B_FLAGS.PME$B_BITS.PME$V_CLOSE
	    && pgid != VWI$GL_PREV_PAGE)
	BWI_TOPIC_CLOSE (bkb, pgid);
}

UNDEFSYM *bwi_symbol_complain (bkb)
BKB *bkb;	    /*  Book id (from BWI$CREATE_BOOK)		    */
{
    UNDEF	*testsym;
    UNDEFSYM	*symbol, *lastsym;
        
    testsym = bkb->BKB$L_SYMBOLS;
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
};

/***************************************************************************/
/*  bwi_symbol_xref - given a symbolic name for a book and a topic returns 
/*		      an index into the external reference table
/*
/*
/***************************************************************************/
unsigned int  bwi_symbol_xref (bkb, book_sym, object_sym, target_id_rtn)
BKB             *bkb;		/* ptr to book structure */
char            *book_sym;	/* symbolic name of another book */
char            *object_sym;	/* symbolic name of chunk in another book */
BMD_OBJECT_ID   *target_id_rtn;	/* returned value which is really an index */
				/* into the exref symbol table */
{
    BKH_EXT *bkh_ext;
    BMD_OBJECT_ID pgid;
    BWI_BYTE len, len1, len2;
    char     *buffer;
    unsigned status;


    bkh_ext = &bkb->BKB$V_BKH_EXT;
    pgid = bkh_ext->BKEH$L_SYM_XREF_INDEX_PG;

    *target_id_rtn = (bkh_ext->BKEH$L_SYM_XREF_INDEX_PG << 24) & 0xFF000000; 
    *target_id_rtn = *target_id_rtn | (bkh_ext->BKEH$L_NUM_XREFS + 1); 
/*  the +1 is so the bookreader won't get confused by an entry number of 0*/ 

    len1 = strlen(book_sym) + 1;
    len2 = strlen(object_sym) + 1;
    len = len1 + len2 + sizeof(len);    /* add 3 for null terminators and */
					/* sizeof(len)			  */

    if((status = VWI$WRITE_PAGE( bkb, pgid, sizeof(len), &len )) != BwiOK)
        return(status);
    if((status = VWI$WRITE_PAGE( bkb, pgid, len1, book_sym )) != BwiOK)
        return(status);
    if((status = VWI$WRITE_PAGE( bkb, pgid, len2, object_sym )) != BwiOK)
        return(status);

    bkh_ext->BKEH$L_NUM_XREFS++;

   return(status);
 }
