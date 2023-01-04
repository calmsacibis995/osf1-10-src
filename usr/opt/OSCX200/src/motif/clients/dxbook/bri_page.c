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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BRI_PAGE.C*/
/* *10   24-FEB-1993 17:40:53 BALLENGER "Fix problems with large topics."*/
/* *9    15-OCT-1992 16:50:17 BALLENGER "Accumulate x,y values before scaling to lessen affects of scaling."*/
/* *8    19-JUN-1992 20:16:41 BALLENGER "Cleanup for Alpha/OSF port."*/
/* *7     9-JUN-1992 10:02:29 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *6     3-MAR-1992 17:11:10 KARDON "UCXed"*/
/* *5     2-JAN-1992 12:41:21 FITZELL "use OAF version to determine what case the symbol table check should be in"*/
/* *4    22-NOV-1991 17:00:34 BALLENGER "cast timestamp appropriately"*/
/* *3    13-NOV-1991 14:51:28 GOSSELIN "alpha checkins"*/
/* *2    17-SEP-1991 21:09:11 BALLENGER "change use of major and minor in version number to avoid conflict with sys/types.h*/
/*definition on ULTRIX"*/
/* *1    16-SEP-1991 12:44:24 PARMENTER "Page Management"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BRI_PAGE.C*/
#ifndef VMS
 /*
#else
# module BRI_PAGE "V03-0002"
#endif
#ifndef VMS
  */
#endif

/*
***************************************************************
**  Copyright (c) Digital Equipment Corporation, 1988, 1990  **
**  All Rights Reserved.  Unpublished rights reserved	     **
**  under the copyright laws of the United States.  	     **
**  	    	    	    	    	    	    	     **
**  The software contained on this media is proprietary	     **
**  to and embodies the confidential technology of  	     **
**  Digital Equipment Corporation.  Possession, use,	     **
**  duplication or dissemination of the software and	     **
**  media is authorized only pursuant to a valid written     **
**  license from Digital Equipment Corporation.	    	     **
**  	    	    	    	    	    	    	     **
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 	     **
**  disclosure by the U.S. Government is subject to 	     **
**  restrictions as set forth in Subparagraph (c)(1)(ii)     **
**  of DFARS 252.227-7013, or in FAR 52.227-19, as  	     **
**  applicable.	    	    	    	    	    	     **
***************************************************************
**
**  FACILITY:
**
**   BRI -- VOILA Reader Interface
**
** ABSTRACT:
**
**   This module implements the page management routines for BRI
**
** FUNCTIONS:
**
**  	bri_page_chunk_count
**  	bri_page_chunk_page_id
**  	bri_page_chunk_symbol
**  	bri_get_object_symbol
**  	bri_get_symbol_object
**  	bri_symbol_to_chunk
**  	bri_page_chunk_symbol
**  	bri_get_exref_info
**  	bri_page_chunk_title
**  	bri_page_close
**  	bri_page_data
**  	bri_page_next
**  	bri_page_open
**  	bri_page_previous
**
** AUTHORS:
**
**   Joseph M. Kraetsch
**
** CREATION DATE: 30-SEP-1987
**
** MODIFICATION HISTORY:
**
**  V03-0002	JMK 	    Joe M. Kraetsch 	    	2-Dec-1990
**  	    	Add new routine bri_get_exref_info.
**
**  V03-0001	JAF0001	    James A. Ferguson	    	1-Oct-1990
**  	    	Fix access violation in bri_get_symbol_object and change
**  	    	bri_page_chunk_symbol to call bri_get_object_symbol.
**
**  V03-0000	JAF0000	    James A. Ferguson	    	16-Aug-1990 
**  	      	Create new module, modify revision history format and
**  	    	update copyright.
**
**  	    	JAF 	    James A. Ferguson	    	3-Aug-1990 
**  	       	Fix bri_symbol_to_chunk to upper case the symbol
**  	       	passed before doing comparisons.
**
**  	    	DLB 	    David L Ballenger	    	05-Jul-1990 
**             	Add bri_symbol_to_chunk routine.
**
**  	    	DLB 	    David L Ballenger	    	30-May-1990 
**             	Cleanup (i.e. remove most contionaliztaion) include
**             	files for new VMS standards.
**
*/


/*  
**  INCLUDE FILES  
*/
#include "bri_private_def.h"
#include "br_common_defs.h"
#include <ctype.h>

#define BRI_SYMBOL_MAX_LENGTH   32


/*
**  bri_page_chunk_count -- Get number of data chunks on page
*/
BR_UINT_32
bri_page_chunk_count PARAM_NAMES((bkid,pgid))
    BMD_BOOK_ID bkid PARAM_SEP
    BMD_OBJECT_ID pgid PARAM_END
{
    return BriBookPtr(bkid)->page_map[pgid]->data_page->n_chunks;

};	/*  end of bri_page_chunk_count  */

/*
**  bri_page_chunk_page_id -- Get page id for given chunk
*/
BMD_OBJECT_ID
bri_page_chunk_page_id PARAM_NAMES((bkid,ckid))
    BMD_BOOK_ID bkid PARAM_SEP    /*  Book id (from bri_book_open)  */
    BMD_OBJECT_ID ckid PARAM_END  /*  chunk id  */
{

    return BriBookPtr(bkid)->chunk_index[ckid];

};	/*  end of bri_page_chunk_page_id  */

char *
bri_get_object_symbol PARAM_NAMES((bkid,object_id))
    BMD_BOOK_ID bkid PARAM_SEP
    BMD_OBJECT_ID object_id PARAM_END
/*
 *
 * Function description:
 *
 *      gets the symbolic name associated with an object_id
 *
 * Arguments:
 *
 *      bkid  - Book id
 *	object_id - Object id
 *
 * Return value:
 *
 *      Address of the symbolic name of the object 
 *	NULL if the object has no symbolic name
 *
 * Side effects:
 *
 *      None
 *
 */

{
    BRI_CONTEXT *env = (BRI_CONTEXT *)bkid;
    BRI_BOOK_BLOCK *bkb = BriBookPtr(bkid);
    BRI_SYMBOL_ENTRY *symbol;

    if (bkb->symbol_table == NULL) {
        if (setjmp(env->jump_buffer) != 0) {
            BriError(env,BriErrChkSymNum);
            return NULL ;
        }
        BriBookGetSymbolTable(env);
    }
    symbol = bkb->symbol_table[BriHash(object_id)];
    while (symbol != NULL)
    {
	if (symbol->id == object_id)
	    return symbol->name;
	symbol = symbol->next;
    }

    return NULL;

} /*  end bri_get_object_symbol  */


BMD_OBJECT_ID
bri_get_symbol_object PARAM_NAMES((bkid,symbol))
    BMD_BOOK_ID bkid PARAM_SEP
    char *symbol PARAM_END
/*
 *
 * Function description:
 *
 *      Looks up a symbol and returns the associated object id.
 *
 *  	NOTE: For V1 and V2 books the symbol table was stored in upper case.
 *	      For V3 books the symbol table is lower case.
 *	      Symbols are case-insensitive for multi-platform compatibility
 *
 * Arguments:
 *
 *      bkid  - Book id
 *
 *      symbol - Object symbol
 *
 * Return value:
 *
 *      Object Id or ZERO if the symbol isn't found.
 *
 * Side effects:
 *
 *      None
 *
 */

{
    register BRI_CONTEXT *env = (BRI_CONTEXT *)bkid;
    register BRI_BOOK_BLOCK *bkb = BriBookPtr(bkid);
    register int i;    
    char test_sym[BRI_SYMBOL_MAX_LENGTH];
    BRI_SYMBOL_ENTRY	*bri_symbol;

    if (symbol == NULL || symbol[0] == '\0' || strlen (symbol) >= BRI_SYMBOL_MAX_LENGTH)
	return 0;	/*  invalid symbol  */

    /* Read the symbol table if necessary.  */
    if (bkb->symbol_table == NULL) {
        if (setjmp(env->jump_buffer) != 0) {
            BriError(env,BriErrChkSymNum);
            return 0 ;
        }
        BriBookGetSymbolTable(env);
    }

    /*  Uppercase the symbol for comparisons for V1 and V2 books;
     *  lowercase the symbol for V3(not many of these) and V2.2 books.
     *  NOTE: on UNIX the _toupper macro DOESN'T include the islower check.
     */

    if (bkb->vbh.oaf_version.minor_num < 2)
    {
	for (i = 0; symbol[i] != '\0'; i++)
    	{
    	    if (islower(symbol[i]))
	    	test_sym[i] = _toupper( symbol[i] );
    	    else
	    	test_sym[i] = symbol[i];
    	}
    }
    else  
    {
	for (i = 0; symbol[i] != '\0'; i++)
    	{
    	    if (isupper(symbol[i]))
	    	test_sym[i] = _tolower( symbol[i] );
    	    else
	    	test_sym[i] = symbol[i];
    	}
    }
    test_sym[ strlen(symbol) ] = '\0';

    for (i = 0; i < BRI_SYMBOL_HASH_SIZE; i++)
    {
	bri_symbol = bkb->symbol_table[i];
	while (bri_symbol != NULL)
	{
	    if (strcmp(test_sym, bri_symbol->name) == 0)
		return bri_symbol->id;
	    bri_symbol = bri_symbol->next;
	}
    }

    return 0;	    /*  symbol not found  */

} /*  end bri_get_symbol_object  */


BMD_OBJECT_ID
bri_symbol_to_chunk (bkid, symbol)  /***this should go away***/
    BMD_BOOK_ID bkid;
    register char *symbol;
{
    return bri_get_symbol_object (bkid, symbol);
}


char *
bri_page_chunk_symbol (bkid, object_id)  /***this should go away***/
    BMD_BOOK_ID bkid;
    BMD_OBJECT_ID object_id;
{
    return bri_get_object_symbol (bkid, object_id);
}


BMD_OBJECT_TYPE
bri_get_exref_info PARAM_NAMES((bkid,exref_id,
                                book_name,book_title,book_timestamp,
                                object_name,object_id))
    BMD_BOOK_ID bkid PARAM_SEP
    BMD_OBJECT_ID exref_id PARAM_SEP
    char	**book_name PARAM_SEP
    char	**book_title PARAM_SEP
    BR_UINT_32  **book_timestamp PARAM_SEP
    char	**object_name PARAM_SEP
    BMD_OBJECT_ID *object_id PARAM_END
/*
 *
 * Function description:
 *
 *      Gets information about an external cross reference
 *
 * Arguments:
 *
 *      bkid		- input--Book id of source book
 *	exref_id	- input--source exref id 
 *	book_name	- output--target book name  
 *	book_title	- output--target book title  (static exrefs only)
 *	book_timestamp	- output--target book timestamp (static exrefs only)
 *	object_name	- output--target object name  (symbolic exrefs only)
 *	object_id	- output--target object id  (static exrefs only)
 *
 * Return value:
 *
 *      Type of cross reference. One of:
 *
 *	    BMD_C_SYMBOL_EXREF_OBJECT   - symbolic cross reference
 *	    BMD_C_STATIC_EXREF_OBJECT	- static cross reference
 *	    BMD_C_NO_OBJECT_TYPE	- error
 *
 * Side effects:
 *
 *      Reads in the appropriate cross reference table if necessary
 *
 */
{
    register BRI_CONTEXT *env = (BRI_CONTEXT *)bkid;
    register BRI_BOOK_BLOCK *bkb = BriBookPtr(bkid);
    register int    index;
    BODS_STATIC_EXREF_BOOK *exref_book;
    int book_num;

    /*  initialize the return parameters  */

    if ( book_name )
	*book_name = NULL;
    if ( book_title )
	*book_title = NULL;
    if ( book_timestamp )
	*book_timestamp = NULL;
    if ( object_name )
	*object_name = NULL;
    if ( object_id )
	*object_id = (BMD_OBJECT_ID) 0;

    switch ( BRI_DIRECTORY_ID(exref_id) )
    {
    	case BODS_SYMBOL_EXREF_DIR_ID:
	    /* Read the symbolic external ref table if necessary */
	    if (bkb->symbol_exref_table == NULL) 
	    {
		if (setjmp(env->jump_buffer) != 0) 
		{
		    BriError(env,BriErrChkSymNum);
		    return BMD_C_NO_OBJECT_TYPE;
		}
		BriBookGetSymbolExrefTable(env);
		if (bkb->symbol_exref_table == NULL) 
		    return BMD_C_NO_OBJECT_TYPE;
	    }

	    /* Validate the external ref */
	    index = BRI_DIRECTORY_ENTRY_NUMBER (exref_id);
	    if ((index < 0) || (index > bkb->vbh.n_symbol_exrefs))
		return BMD_C_NO_OBJECT_TYPE;

	    if ( book_name )
		*book_name = bkb->symbol_exref_table[index].book_name;

	    if ( object_name )
		*object_name = bkb->symbol_exref_table[index].object_name;

	    return BMD_C_SYMBOL_EXREF_OBJECT;
	    break;

	case BODS_STATIC_EXREF_DIR_ID:
	    /* Read the static external ref table if necessary */
	    if (bkb->static_exref_table == NULL) 
	    {
		if (setjmp(env->jump_buffer) != 0) 
		{
		    BriError(env,BriErrChkSymNum);
		    return BMD_C_NO_OBJECT_TYPE;
		}
		BriBookGetStaticExrefTable(env);
		if ( bkb->static_exref_table == NULL )
		    return BMD_C_NO_OBJECT_TYPE;
	    }

	    /* Validate the external ref */
	    index = BRI_DIRECTORY_ENTRY_NUMBER (exref_id);
	    if ( ( index < 0 ) || ( index > bkb->num_static_exrefs ) )
		return BMD_C_NO_OBJECT_TYPE;

	    book_num = bkb->static_exref_table[index].book_num;
	    exref_book = bkb->static_exref_booklist[book_num];

	    if ( book_name )
		*book_name = &exref_book->book_name[0];
	    if ( book_title )
		*book_title = &exref_book->book_title[0];
	    if ( book_timestamp )
		*book_timestamp = (unsigned *)&exref_book->timestamp[0];
	    if ( object_id )
		*object_id = bkb->static_exref_table[index].object_id;

	    return BMD_C_STATIC_EXREF_OBJECT;
	    break;

    	default:
	    return BMD_C_NO_OBJECT_TYPE;
    }
}


/*
**  bri_page_chunk_title -- Get title for given chunk
*/
char * 
bri_page_chunk_title PARAM_NAMES((bkid,ckid))
    BMD_BOOK_ID bkid PARAM_SEP	    /*  Book id (from bri_book_open)  */
    BMD_OBJECT_ID ckid PARAM_END    /*  chunk id  */
{

    return BriBookPtr(bkid)->chunk_titles[ckid];

};	/*  end of bri_page_chunk_title  */

/*
**  bri_page_close -- close a page
*/
void 
bri_page_close PARAM_NAMES((bkid,pgid))
    BMD_BOOK_ID bkid PARAM_SEP	    /*  Book id (from bri_book_open)  */
    BMD_OBJECT_ID pgid PARAM_END    /*  page id  */
{
    BRI_CONTEXT *env = (BRI_CONTEXT *)bkid;

    if (setjmp(env->jump_buffer) != 0) {
        BriError(env,BriErrPageCloseNum);
    } else {
        register BRI_BOOK_BLOCK *bkb = BriBookPtr(bkid);
        bkb->page_map[pgid]->n_users--;
        if (bkb->page_map[pgid]->n_users <= 0) {
            BriClosePage(env,pgid);
        }
    }

};	/*  end of bri_page_close  */

/*
**  bri_page_data -- Get data chunks for this page
*/
void
bri_page_data PARAM_NAMES((bkid,pgid,chunk_list))
    BMD_BOOK_ID bkid PARAM_SEP      /* Book id (from bri_book_open)  */
    BMD_OBJECT_ID pgid PARAM_SEP    /* page id  */
    BMD_CHUNK *chunk_list PARAM_END /* address of array of BMD_CHUNK records */
{
    BRI_BOOK_BLOCK *bkb = BriBookPtr(bkid);    /*  book block  */
    BODS_PAGE_HEADER *page; /*  page block  */
    int	    cknum;

    page = bkb->page_map[pgid]->data_page;

    for (cknum = 0; cknum < page->n_chunks ; cknum++)
    {
        BODS_CHUNK_HEADER *ckb = &page->chunks[cknum];

        if (ckb->tag.rec_type == 0) {
            break;
        }

        /* Fix chunk tag for V1.0 cross-references.
         */
        if ((ckb->tag.rec_type == BODS_C_DATA_SUBCHUNK) && ckb->target) {
            chunk_list[cknum].chunk_type	= BMD_REFERENCE_RECT;
        } else {
            chunk_list[cknum].chunk_type	= ckb->tag.rec_type;
        }
	chunk_list[cknum].id		= ckb->chunk_id;
	chunk_list[cknum].parent	= ckb->parent_chunk;
	chunk_list[cknum].target	= ckb->target;
	chunk_list[cknum].data_type	= ckb->data_type;
	chunk_list[cknum].data_len	= ckb->data_length;
        chunk_list[cknum].handle        = 0;

        /* If it's a polygon chunk, set up the point list.
         */
        if (ckb->tag.rec_type == BMD_REFERENCE_POLY ||
            ckb->tag.rec_type == BMD_EXTENSION_POLY
            ) {
            int *num_points = (int *)ckb->data;
            BMD_POINT *point = (BMD_POINT *)(num_points + 1 );
            int index;
            int min_x, min_y, max_x, max_y;

            chunk_list[cknum].num_points = *num_points;
            chunk_list[cknum].point_vec = point;
            chunk_list[cknum].data_addr = (BR_UINT_8 *)(point + *num_points);

            min_x = max_x = point[0].x;
            min_y = max_y = point[0].y;

            for (index = 1; index < *num_points; index++) {
                min_x = MIN(min_x,point[index].x);
                min_y = MIN(min_y,point[index].y);
                max_x = MAX(max_x,point[index].x);
                max_y = MAX(max_y,point[index].y);
            }
            
            chunk_list[cknum].unscaled_x      = min_x;
            chunk_list[cknum].unscaled_y      = min_y;
            chunk_list[cknum].unscaled_width  = max_x - min_x + 1;
            chunk_list[cknum].unscaled_height = max_y - min_y + 1;
        } else {
            chunk_list[cknum].unscaled_x       = ckb->x;
            chunk_list[cknum].unscaled_y       = ckb->y;
            chunk_list[cknum].unscaled_width   = ckb->width;
            chunk_list[cknum].unscaled_height  = ckb->height;
            chunk_list[cknum].data_addr	= ckb->data ;
            chunk_list[cknum].num_points = 0;
            chunk_list[cknum].point_vec = NULL;
        }
    }
};	/*  end of bri_page_data  */


/*
**  bri_page_next -- Get page id of next page
*/
BMD_OBJECT_ID
bri_page_next PARAM_NAMES((bkid,pgid))
    BMD_BOOK_ID bkid PARAM_SEP	    /*  Book id (from bri_book_open)  */
    BMD_OBJECT_ID pgid PARAM_END    /*  page id  */
{

    return BriBookPtr(bkid)->page_map[pgid]->data_page->next_page_id;

};	/*  end of bri_page_next  */

/*
**  bri_page_open -- Open a page
*/
BR_UINT_32
bri_page_open PARAM_NAMES((bkid,pgid))
    BMD_BOOK_ID bkid PARAM_SEP	    /*  Book id (from bri_book_open)  */
    BMD_OBJECT_ID pgid PARAM_END    /*  page id  */
{
    BRI_CONTEXT *env = (BRI_CONTEXT *)bkid ;
    register BRI_BOOK_BLOCK *bkb ;    /*  book block  */

    bkb = BriBookPtr(bkid);

    if (bkb->page_map[pgid] == NULL) {
        
        if (setjmp(env->jump_buffer) != 0) {
            BriError(env,BriErrPageOpenNum);
            return 0 ;
        }
        BriBookGetDataPage(env,pgid) ;
    }
    bkb->page_map[pgid]->n_users++;
    return bkb->page_map[pgid]->data_page->page_type ;

};	/*  end of bri_page_open  */

/*
**  bri_page_previous -- Get page id of previous page
*/
BMD_OBJECT_ID
bri_page_previous PARAM_NAMES((bkid,pgid))
    BMD_BOOK_ID bkid PARAM_SEP	    /*  Book id (from bri_book_open)  */
    BMD_OBJECT_ID pgid PARAM_END    /*  page id  */
{
    return BriBookPtr(bkid)->page_map[pgid]->data_page->prev_page_id;

};	/*  end of bri_page_previous  */




