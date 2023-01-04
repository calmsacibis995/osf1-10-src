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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_BOOK.C*/
/* *7     8-JUN-1992 18:58:14 BALLENGER "UCX$CONVERT"*/
/* *6     8-JUN-1992 11:21:20 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *5     8-MAR-1992 19:15:56 BALLENGER "  Add topic data and text line support"*/
/* *4     3-MAR-1992 16:56:16 KARDON "UCXed"*/
/* *3     1-NOV-1991 13:06:29 BALLENGER "Reintegrate  memex support"*/
/* *2    17-SEP-1991 19:00:28 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:38:42 PARMENTER "Initialization routines for opening a book"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_BOOK.C*/
#ifndef VMS
 /*
#else
# module BKR_BOOK "V03-0000"
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
*/


/*
**++
**  FACILITY:
**
**      Bookreader User Interface (bkr)
**
**  ABSTRACT:
**
**	Initialization routines for opening a book.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     26-Apr-1990
**
**  MODIFICATION HISTORY:
**
**--
**/


/*
 * INCLUDE FILES
 */
#include "br_common_defs.h"  /* common BR #defines */ 
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "br_globals.h"      /* BR external variables declared here */
#include "bkr_book.h"        /* function prototypes for .c module */
#include "bri_dir.h"
#ifdef MEMEX
#include "bmi_surrogate.h"
#endif
#ifdef USE_TEXT_LINES
#include "bkr_topic_data.h"
#endif /* USE_TEXT_LINES */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_book_get
**
** 	Initialization routine for opening a book.  
**  	If a NULL is passed for the "filename" parameter then
**  	the shelf and entry ids are used to open the book, otherwise
**  	the book is opened by "filename" and the shelf and entry ids
**  	are ignored.
**
**  FORMAL PARAMETERS:
**
**  	filename    	    - string name of file to open.
**  	shelf_id    	    - id of shelf which contains book.
**  	entry_id    	    - entry number within parent shelf.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	Returns:    1 - if the book was successfully opened.
**  	    	    0 - if the open failed.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
BKR_BOOK_CTX *
bkr_book_get PARAM_NAMES((filename,shelf_id,entry_id))
    char *filename PARAM_SEP
    BMD_SHELF_ID shelf_id  PARAM_SEP
    unsigned int entry_id PARAM_END
    
{
    BKR_BOOK_CTX        *book_ctx ;
    BMD_BOOK_ID         book_id;
    char                *filespec;
    BMD_OBJECT_ID  	dir_id_ctx;
    BMD_OBJECT_ID       default_dir_id;
    BMD_OBJECT_ID       toc_id;
    BMD_OBJECT_ID       index_id;
    BKR_DIR_ENTRY_PTR   dir_ptr;
    int	    	    	cnt;

    /* Open the book and get the structure level */

    if ( filename != NULL )
    	book_id = bri_book_open_file( filename, HOME_DIRECTORY );
    else
    	book_id = bri_book_open( shelf_id, entry_id );

    if ( book_id == NULL ) {
	return( NULL );
    }

    /* See if we already have the book open.
     */
    filespec = bri_book_found_file_spec(book_id);
    book_ctx = bkr_open_books;
    while ( book_ctx ) {

        /* Yep its open, just use the previous context
         */
    	if ( strcmp( book_ctx->filespec, filespec ) == 0 ) {
            bri_book_close(book_id);
    	    return book_ctx;
    	}
    	book_ctx = book_ctx->next;
    }

    /* Create a new book context
     */
    book_ctx = (BKR_BOOK_CTX *)BKR_MALLOC(sizeof(BKR_BOOK_CTX));
    if (book_ctx == NULL) {
        bri_book_close(book_id);
        return book_ctx;
    }
    memset(book_ctx,0, sizeof(BKR_BOOK_CTX));

    book_ctx->next = bkr_open_books;
    bkr_open_books = book_ctx;
    book_ctx->shells = NULL;

    book_ctx->book_id = book_id;
    book_ctx->filename = bri_book_file_name(book_id);
    book_ctx->filespec = bri_book_found_file_spec(book_id);
    book_ctx->title = bri_book_title(book_id);
    bri_book_version( book_id, &book_ctx->version );
    bri_book_timestamp(book_id, book_ctx->timestamp);
    book_ctx->n_chunks = bri_book_chunk_count(book_id);

    toc_id = bri_book_directory_contents( book_id );
    index_id = bri_book_directory_index( book_id );
    default_dir_id = bri_book_directory_default( book_id );

    /* 
     *  Get all of the directory ids in the book and store them in an array
     *  with the TOC directory id first and the INDEX id last and the
     *  rest in the middle.  
     *  NOTE:  This is the exact order they will be displayed using SVN 
     *         in the Selection window.
     */

    book_ctx->n_directories = (int) bri_book_directory_count( book_id );
    book_ctx->directories = (BKR_DIR_ENTRY_PTR)BKR_CALLOC(book_ctx->n_directories, sizeof( BKR_DIR_ENTRY ) ); 
    memset(book_ctx->directories,0,(book_ctx->n_directories * sizeof(BKR_DIR_ENTRY)));

    cnt = 0;
    if ( index_id != 0 ) {
        book_ctx->index = &book_ctx->directories[book_ctx->n_directories-1];
        book_ctx->index->object_id = index_id;
        cnt++;
    }
    dir_ptr = &book_ctx->directories[0];
    if ( toc_id != 0 ){
        book_ctx->toc = dir_ptr;
        book_ctx->toc->object_id = toc_id;
        cnt++;
        dir_ptr++;
    }
    dir_id_ctx = bri_directory_next( book_id, 0 );
    while ( ( dir_id_ctx != 0 ) && ( cnt < book_ctx->n_directories ) )
    {
        if ((dir_id_ctx != toc_id) && (dir_id_ctx != index_id)) {
            dir_ptr->object_id = dir_id_ctx;
            cnt++;
            dir_ptr++;
        }
        dir_id_ctx = bri_directory_next( book_id, dir_id_ctx ); 
    }

    /* Determine the default directory if one doesn't already exist */

    if ((bkr_resources.first_directory_to_open == TOC_DIR) && toc_id)
    {
        default_dir_id = toc_id;
    }
    else if ((bkr_resources.first_directory_to_open == INDEX_DIR) && index_id)
    {
        default_dir_id = index_id;
    }
    else if (default_dir_id == 0 )
    {
    	if ( toc_id != 0 )
    	    default_dir_id = toc_id;
    	else if ( index_id != 0 )  /* No TOC, use Index  */
    	    default_dir_id = index_id;
    	else    	    	    	     /* No Index, use first we find  */
    	    default_dir_id = bri_directory_next( book_id, 0 );
    }


    for (dir_ptr = book_ctx->directories, cnt = 0 ;
         cnt < book_ctx->n_directories ; 
         dir_ptr++, cnt++) 
    {
        dir_ptr->entry_type = DIRECTORY;
        dir_ptr->level = 0;
        dir_ptr->sibling = &book_ctx->directories[cnt+1]; 
        dir_ptr->children = NULL;
        dir_ptr->title = bri_directory_name(book_id,dir_ptr->object_id );
        if (dir_ptr->object_id == default_dir_id) {
            book_ctx->default_directory = dir_ptr;
        }
#ifdef MEMEX
        dir_ptr->highlight = FALSE;
        dir_ptr->u.directory.surrogates = NULL;
#endif 
    }
    book_ctx->directories[book_ctx->n_directories - 1].sibling = NULL;

    book_ctx->first_page =  bri_book_first_page( book_id );
    book_ctx->n_pages    =  bri_book_page_count( book_id );

    /* Initialize the font data structures for the book.
     */
    book_ctx->max_font_id = bri_book_font_max_id(book_id);
    bkr_font_data_init(book_ctx);

    return( book_ctx );

}   /* end of bkr_book_get */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_book_free
**
**
**  FORMAL PARAMETERS:
**
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	Returns:    1 - if the book was successfully opened.
**  	    	    0 - if the open failed.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
void
bkr_book_free 
    PARAM_NAMES((book))
    BKR_BOOK_CTX *book PARAM_END
{
    BKR_BOOK_CTX_PTR *ctx_ptr;
    BKR_DIR_ENTRY_PTR dir_ptr;

    if ((book == NULL) || (book->shells != NULL))
	return;

#ifdef USE_TEXT_LINES
    while (book->open_topics) {
        BKR_TOPIC_DATA *next = book->open_topics->next;
        bkr_topic_data_delete(book,book->open_topics);
        book->open_topics = next;
    }
#endif 

    /* Free the fonts for the book.
     */
    bkr_font_data_close(book);

    /* Tell BRI to close the book now! */

    bri_book_close( book->book_id );
#ifdef MEMEX
    if (book->chunk_surrogates) {
        bmi_delete_surrogate_tree(&book->chunk_surrogates);
    }
#endif 

    dir_ptr = book->directories;
    if (dir_ptr) 
    {
        while (dir_ptr)
        {
            if (dir_ptr->children) {
                BKR_CFREE(dir_ptr->children);
            }
#ifdef MEMEX
            if (dir_ptr->u.directory.surrogates) {
                bmi_delete_surrogate_tree(&dir_ptr->u.directory.surrogates);
            }
#endif 
            dir_ptr = dir_ptr->sibling;
        }
        BKR_CFREE(book->directories);
    }

    ctx_ptr = &bkr_open_books;
    while (*ctx_ptr) {
        BKR_BOOK_CTX *ctx = *ctx_ptr;
        if (book == ctx) {
            *ctx_ptr = book->next;
            break;
        }
        ctx_ptr = &ctx->next;
    }

    BKR_FREE(book);

}   /* end of bkr_book_free */





