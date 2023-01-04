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
/* DEC/CMS REPLACEMENT HISTORY, Element BRI_BOOK.C*/
/* *8    24-JUL-1992 12:26:41 KLUM "repress print if NO_PRINT alt-prod-name"*/
/* *7    19-JUN-1992 20:16:08 BALLENGER "Cleanup for Alpha/OSF port."*/
/* *6     9-JUN-1992 10:01:43 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *5     3-MAR-1992 17:10:13 KARDON "UCXed"*/
/* *4     4-FEB-1992 14:28:26 FITZELL "alpha alignment issues"*/
/* *3     2-JAN-1992 10:01:20 FITZELL "Xbook references in OAF level B"*/
/* *2    17-SEP-1991 21:08:59 BALLENGER "change use of major and minor in version number to avoid conflict with sys/types.h*/
/*definition on ULTRIX"*/
/* *1    16-SEP-1991 12:43:56 PARMENTER "Book management routines"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BRI_BOOK.C*/
#ifndef VMS
 /*
#else
# module BRI_BOOK "V03-0002"
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
**   BRI -- Book Reader Interface
**
** ABSTRACT:
**
**   This module implements the book management routines for BRI
**
** FUNCTIONS:
**
**  	bri_book_close
**  	bri_book_copyright_chunk
**	FtextToAscii
**  	bri_book_copyright_info
**  	bri_book_first_page
**  	bri_book_directory_count
**  	bri_book_directory_contents
**  	bri_book_directory_index
**  	bri_book_directory_default
**  	bri_book_font_count
**  	bri_book_font_max_id
**  	bri_book_font_name
**  	bri_book_timestamp
**  	OpenBook
**  	bri_book_open_file
**  	bri_book_open
**  	bri_book_title
**  	bri_book_file_spec
**  	bri_book_version
**  	bri_book_chunk_count
**  	bri_book_no_print
**  	bri_logical
**
** AUTHORS:
**
**   Joseph M. Kraetsch
**
** CREATION DATE: 30-SEP-1987
**
** MODIFICATION HISTORY:
**
**   V03-0003	DAA 	    Debbie A. Ackerman	    	23-Apr-1991 
**  	    	Replace newline character with space for copyright info 
**              so entire string displays in title bar.
**
**   V03-0002	JMK 	    Joe M. Kraetsch 	    	29-Nov-1990
**  	    	Add new routines FtextToAscii and bri_book_copyright_info.
**
**  V03-0001    DLB         David L Ballenger           29-Aug-1990
**              Use RMS for file access on VMS.
**
**    V03-0000	JAF0000	    James A. Ferguson   	16-Aug-1990 
**  	      	Create new module, modify revision history format and
**  	    	update copyright.
**
**  	    	DLB 	    David L Ballenger	    	05-Jul-1990 
**  	    	Incorporate changes from Jim Ferguson to allow 
**              specificaton of a home directory when opening a
**              book by file name.
**
**    	    	DLB 	    David L Ballenger	    	30-May-1990 
**              Cleanup (i.e. remove most contionaliztaion) include
**              files for new VMS standards.
**
**    	    	DLB 	    David L Ballenger	    	12-Sep-1989 
**          	Add changes from V2 of Bookreader on VMS.
**
*/

/*  
**  INCLUDE FILES  
*/
#include "bri_private_def.h"
#ifdef vms
#include <descrip>
#include <ssdef>
#include <lnmdef>
#endif


/*
**  bri_book_close -- Close a book
*/
void
bri_book_close PARAM_NAMES((bkid))
    BMD_BOOK_ID bkid PARAM_END
{
    /* Closing a book is simply a matter of "deleting" the context
     * specified by the book id.
     */
    BriContextDelete((BRI_CONTEXT *)bkid);
};	/*  end of bri_book_close*/


/*
**  bri_book_copyright_chunk -- Get copyright chunk id 
*/
BMD_OBJECT_ID
bri_book_copyright_chunk PARAM_NAMES((bkid))
    BMD_BOOK_ID bkid PARAM_END
{

    return BriBookPtr(bkid)->vbh.copyright_chunk ;

};	/*  end of bri_book_copyright_chunk  */


char *
FtextToAscii PARAM_NAMES((context,data_addr,data_len))
    BRI_CONTEXT *context PARAM_SEP
    BR_UINT_8   *data_addr PARAM_SEP	/*  address of ftext chunk  */
    BR_UINT_32   data_len PARAM_END	/*  length of data  */
             
{
    BR_INT_32			prev_y = 0;
    char		*text_str;	/*  output string  */
    BR_INT_32			count = 0;

    BMD_FTEXT_PKT       *packet = (BMD_FTEXT_PKT *) data_addr;
    BMD_TEXT_PKT        text;
    BMD_TEXT_PKT        *text_ptr = (BMD_TEXT_PKT *) &text;
    BMD_WORD_PKT	*word;
    BMD_WORD_PKT	*word_end;

    BR_UINT_8           *data_end = (BR_UINT_8 *) &data_addr[data_len];

#define TEXT_PTR( pkt )     ( (BMD_TEXT_PKT *) &( (BMD_FTEXT_PKT *) (pkt) )->value[0] )
#define WORD_PTR( txt )     ( (BMD_WORD_PKT *) &(txt)->data[0] )
#define WORD_START( pkt )   WORD_PTR( TEXT_PTR( (pkt) ) )
#define WORD_END( pkt )     ( (BMD_WORD_PKT *) &(pkt)->value[(pkt)->len - 2] )
 
    if ( data_addr == 0 || data_len == 0 )
	return 0;

    text_str = (char *)BriMalloc( data_len, context );

    while ( packet < (BMD_FTEXT_PKT *) data_end )
    {
	switch (packet->tag)
	{
	case BMD_FTEXT_TEXT300:
	case BMD_FTEXT_TEXT400:
#if defined(VMS) && !defined(ALPHA)
	    text_ptr = (BMD_TEXT_PKT *) &packet->value[0];
#else
	    memcpy( &text, TEXT_PTR( packet ), sizeof( text ) );
#endif

	    /*  check for line break  */
	    if ( ( count > 0 ) && ( text_ptr->y > prev_y ) )
		/*  replace the last space with a newline  */		
/*		text_str[count - 1] = '\n';*/
  		text_str[count - 1] = ' ';
  
	    prev_y = text_ptr->y;
	    word = WORD_START( packet );
	    word_end = WORD_END( packet );

	    while ( word < word_end )
	    {
		BR_INT_32 ch;

		/*  copy word to text string  */
		for ( ch = 0; ch < word->count; ch++ )
		    if ( word->chars[ch] != '\0' )   /* skip NULs */
			text_str[count++] = word->chars[ch];

		/* separate words with a space  */
		text_str[count++] = ' ';  

		/*  next word  */
		word = (BMD_WORD_PKT *) &word->chars[word->count];
	    }
	    break;	/* end case FTEXT_TEXT*  */

	default:	/*  ignore everything but text  */
	    break;	
	}

	/*  next packet  */
	packet = (BMD_FTEXT_PKT *) 
	    &packet->value[packet->len - BMD_FTEXT_PKT_LENGTH ];
	if (packet->len == 0)
	    break;
    }

    /* nul-terminate the string and strip trailing blanks  */
    do
    {
	text_str[count] = '\0';    /* nul-terminate the string  */
	count--;
    } while (count >= 0 && text_str[count] == ' ');

    /*  if the string is empty, free it and return NULL  */

    if ( strlen (text_str) > 0 )
	return text_str;
    BriFree (text_str, context);

    return 0;

}	/* end FtextToAscii */


/*
**  bri_book_copyright_info -- Get copyright text
*/
char *
bri_book_copyright_info PARAM_NAMES((bkid))
    BMD_BOOK_ID bkid PARAM_END
{
    BRI_CONTEXT *env = (BRI_CONTEXT *) bkid;
    char	*copyright = NULL;

    /*  for V2 and older books, extract text from the copyright chunk  */
    if ( env->data.book->vbh.version.major_num < 3 ) 
    {
	BMD_OBJECT_ID	ckid;
	BMD_OBJECT_ID	pgid;
	BR_UINT_8	page_opened = FALSE;
	BR_UINT_32	num_chunks;
	BMD_CHUNK	*chunk_list;
        BMD_CHUNK	*chunk;
        BR_INT_32		cknum;
 
	ckid = BriBookPtr(bkid)->vbh.copyright_chunk ;
	if (ckid == 0)
	    return NULL;
	pgid = bri_page_chunk_page_id( bkid, ckid ) ;
	if ( BriBookPtr(bkid)->page_map[pgid] == NULL ) 
	{
	    bri_page_open( bkid, pgid ) ;
	    page_opened = TRUE;
	}

	num_chunks = bri_page_chunk_count( bkid, pgid );
	chunk_list = (BMD_CHUNK *) BriCalloc( BMD_CHUNK, num_chunks, env ) ;
	memset( chunk_list, 0, num_chunks * sizeof( BMD_CHUNK ) );
	bri_page_data( bkid, pgid, chunk_list );
        chunk = (BMD_CHUNK *) &chunk_list[0];
        for ( cknum = 0; cknum < num_chunks; cknum++, chunk++ )
            if ( ( chunk->parent == 0 ) && ( chunk->id == ckid ) )
            {
		if (chunk->data_type == BMD_CHUNK_FTEXT)
		    copyright = FtextToAscii (env, chunk->data_addr, chunk->data_len);
                break;
            }
	BriFree( chunk_list, bkid);
	/*  close the page if we opened it  */
	if ( page_opened )
	    bri_page_close( bkid, pgid );
    }
    else    /*  for V3, get strings from book header  */
    {
	char	*c_date = env->data.book->copyright_date;
	char	*c_corp = env->data.book->copyright_corp;
	char	*c_text = env->data.book->copyright_text;
    	BR_INT_32 	c_date_len = 0;
    	BR_INT_32 	c_corp_len = 0;
    	BR_INT_32 	c_text_len = 0;
	BR_INT_32 	size = 0;

    	if ( c_date ) 
    	{
    	    c_date_len = strlen( c_date );
    	    size += c_date_len + 1; 	/* 1 for NUL or space */
    	}
    	if ( c_corp )
    	{
    	    c_corp_len = strlen( c_corp );
    	    size += c_corp_len + 1; 	/* 1 for NUL or \n */
    	}
    	if ( c_text )
    	{
    	    c_text_len = strlen( c_text );
    	    size += c_text_len + 1; 	/* 1 for NUL */
    	}

    	copyright = NULL;
    	if ( size > 0 )
    	{
    	    char	*segment_ptr;

	    copyright = (char *)BriMalloc( size, env ) ;
    	    segment_ptr = copyright;
    	    if ( c_date )
    	    {
    	    	strncpy( segment_ptr, c_date, c_date_len );
    	    	segment_ptr += c_date_len;
    	    }
    	    if ( c_corp )
    	    {
 	    	/* add space separator */
    	    	if ( c_date )
    	    	{
    	    	    strncpy( segment_ptr, " ", 1 );
    	    	    segment_ptr++;
    	    	}
    	    	strncpy( segment_ptr, c_corp, c_corp_len );
    	    	segment_ptr += c_corp_len;
    	    }
    	    if ( c_text )
    	    {
    	    	/* add newline separator */
    	    	if ( c_date || c_corp )
    	    	{
/*    	    	    sprintf( segment_ptr, "\n" );*/
    	    	    strncpy( segment_ptr, " ", 1 );
    	    	    segment_ptr++;
    	    	}
    	    	strncpy( segment_ptr, c_text, c_text_len );
    	    }
    	    copyright[size] = '\0';
    	}
    }

    return copyright;

};	/*  end of bri_book_copyright_info  */


/*
**  bri_book_first_page -- Get page id of page to open with book
*/
BMD_OBJECT_ID
bri_book_first_page PARAM_NAMES((bkid))
    BMD_BOOK_ID bkid PARAM_END

{

    return BriBookPtr(bkid)->vbh.first_data_page;

};	/*  end of bri_book_first_page*/

/*
**  bri_book_page_count returns the number of pages in the book
*/
BR_UINT_32
bri_book_page_count PARAM_NAMES((bkid))
    BMD_BOOK_ID bkid PARAM_END

{

    return BriBookPtr(bkid)->vbh.n_pages;

};	/*  end of bri_book_page_count */


/*
**  bri_book_directory_count -- number of directories in book */
BR_UINT_32
bri_book_directory_count PARAM_NAMES((bkid))
    BMD_BOOK_ID bkid PARAM_END

{

    return BriBookPtr(bkid)->vbh.n_directories ;

};	/*  end of bri_book_directory_count  */

/*
**  bri_book_directory_contents -- return directory id of TOC directory
*/
BMD_OBJECT_ID
bri_book_directory_contents PARAM_NAMES((bkid))
    BMD_BOOK_ID bkid PARAM_END

{
    BRI_CONTEXT *env = (BRI_CONTEXT *)bkid;
    BRI_BOOK_BLOCK *bkb = env->data.book ;
    BR_UINT_32 dir_cnt;
    BRI_DIRECTORY_BLOCK *drb;

    dir_cnt = 0;
    while (dir_cnt < bkb->vbh.n_directories) {

    	drb = bkb->drb_list[dir_cnt];
        if (drb->vdh.flags.bits.contents != 0) {
            return (BMD_OBJECT_ID)drb->directory_object_id;
        }
    	dir_cnt++;
    }

    return 0 ;
}

/*
**  bri_book_directory_index -- return directory id of INDEX directory
*/
BMD_OBJECT_ID
bri_book_directory_index PARAM_NAMES((bkid))
    BMD_BOOK_ID bkid PARAM_END

{
    BRI_CONTEXT *env = (BRI_CONTEXT *)bkid;
    BRI_BOOK_BLOCK *bkb = env->data.book ;
    BR_UINT_32 dir_cnt;
    BRI_DIRECTORY_BLOCK *drb;

    dir_cnt = 0;
    while (dir_cnt < bkb->vbh.n_directories) {

    	drb = bkb->drb_list[dir_cnt];
        if (drb->vdh.flags.bits.index != 0) {
            return (BMD_OBJECT_ID)drb->directory_object_id;
        }
    	dir_cnt++;
    }

    return 0 ;
}

/*
**  bri_book_directory_default -- return directory id of default directory
*/
BMD_OBJECT_ID
bri_book_directory_default PARAM_NAMES((bkid))
    BMD_BOOK_ID bkid PARAM_END

{
    BRI_CONTEXT *env = (BRI_CONTEXT *)bkid;
    BRI_BOOK_BLOCK *bkb = env->data.book ;
    BR_UINT_32 dir_cnt;
    BRI_DIRECTORY_BLOCK *drb;

    dir_cnt = 0;
    while (dir_cnt < bkb->vbh.n_directories) {

    	drb = bkb->drb_list[dir_cnt];
        if (drb->vdh.flags.bits.default_directory != 0) {
            return (BMD_OBJECT_ID)drb->directory_object_id;
        }
    	dir_cnt++;
    }

    return 0 ;
};

/*
**  bri_book_font_count -- number of fonts in book
*/
BR_UINT_32
bri_book_font_count PARAM_NAMES((bkid))
    BMD_BOOK_ID bkid PARAM_END

{
    BRI_CONTEXT *env = (BRI_CONTEXT *)bkid;

    return env->data.book->vbh.n_fonts ;

};	/*  end of bri_book_font_count  */

/*
**  bri_book_font_max_id -- highest font id used in book
*/
BR_UINT_32
bri_book_font_max_id PARAM_NAMES((bkid))
    BMD_BOOK_ID bkid PARAM_END

{
    BRI_CONTEXT *env = (BRI_CONTEXT *)bkid;

    return env->data.book->vbh.max_font_id ;

};	/*  end of bri_book_font_max_id  */

/*
**  bri_book_font_name -- name of font given id
*/
char *
bri_book_font_name PARAM_NAMES((bkid,font_id))
    BMD_BOOK_ID bkid PARAM_SEP
    BR_UINT_16 font_id PARAM_END
{
    BRI_CONTEXT *env = (BRI_CONTEXT *)bkid;

    return env->data.book->font_list[font_id];

};	/*  end of bri_book_font_name  */

/*
**  bri_book_timestamp - get the timestamp for a book
*/
void
bri_book_timestamp PARAM_NAMES((bkid,timestamp_rtn))
    BMD_BOOK_ID bkid PARAM_SEP
    BR_UINT_32 timestamp_rtn[2] PARAM_END
{
    BRI_CONTEXT *env = (BRI_CONTEXT *)bkid;

    if (env->data.book->vbh.version.major_num < 2) {
        timestamp_rtn[0] = 0;
        timestamp_rtn[1] = 0;
    } else {
        timestamp_rtn[0] = env->data.book->vbh.book_build_date[0];
        timestamp_rtn[1] = env->data.book->vbh.book_build_date[1];
    }
};	/*  end of bri_book_timestamp  */

static BMD_BOOK_ID
OpenBook(book)
    BRI_CONTEXT *book;
/*
 *
 * Function description:
 *
 *      Utility routine that does the dirty work of opening the book
 *      file and reading the header, etc.
 *
 * Arguments:
 *
 *      book - Book context
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      None
 *
 */

{
    /* Open the  book file.
     */
    BriFileOpen(book);

    /* Get the book header from the file and set up the book block. Again
     */
    BriBookGetHeader(book);

    /* See if the book has license information and if this system is
     * licensed to read it.
     */
    BriBookLmfCheck(book);

    /* Set up the page index and page map for the book.
     */
    BriBookGetPageIndexAndMap(book) ;

    /*  Read the chunk index and connect to book block.
     */
    BriBookGetChunkIndex(book);

    /*  Read the chunk titles and set up title index pointing to them.
     */
    BriBookGetChunkTitles(book);

    /*  Read the font names and set up the font index pointing to them.
     */
    BriBookGetFontDefinitions(book);

    /* Don't read the symbol table until we are sure we need it.
     */
    book->data.book->symbol_table = NULL;

    if(book->data.book->vbh.vbh_extension)
	bri_get_extension_header(book);

    if (book->data.book->vbh.version.major_num == 3)
      BriGetV3DirHeaders(book);
 
    return (BMD_BOOK_ID)book;

} /* end OpenBook */

BMD_BOOK_ID
bri_book_open_file PARAM_NAMES((file_name,home_dir))
    char *file_name PARAM_SEP
    char *home_dir PARAM_END
/*
 *
 * Function description:
 *
 *      Opens a book by file name rather than by shelf and entry ids.
 *
 * Arguments:
 *
 *      file_name - Name of the book file to open.
 *  	home_dir  - Name of the home directory for file parses.
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      None
 *
 */

{
    BRI_CONTEXT *book;
    jmp_buf error_return;

    /* Set up the jump_buffer for error unwinding prior to calling any
     * routines.  From this point on any errors that occur while
     * opening the book will result in a call to BriLongJmp which after
     * processing the error will do a longjmp to "return" from the setjmp().
     */
    if (setjmp(error_return) != 0) {

        /* Whoops!!! An error was detected while trying to open the book.
         * The reason should be in the book context.  Report the error,
         * delete the book context, and return NULL to indicate to the
         * calling routine that an error occurred.
         */
        BriError(book,BriErrBookOpenNum);
        BriContextDelete(book);
        return NULL;
    }

    /* Set up the context for the book.  Note that the address of the
     * book context pointer is passed so that it can be set prior to
     * any errors being reported via BriLongJmp().  
     */
    BriContextNew(&book,file_name,BMD_C_BOOK,error_return);

    /* Use the home directory passed otherwise let it be defaulted.
     */
    if (home_dir != NULL)
        BriStringAlloc(&book->entry.home_directory,home_dir,book);

    OpenBook(book);

    return (BMD_BOOK_ID)book;

} /* end bri_book_open_file */


/*
**  bri_book_open -- open a book.
**
**	The book must be open.
**
**  Returns:  Book id
*/
BMD_BOOK_ID
bri_book_open PARAM_NAMES((shelf_id,entry_id))
    BMD_SHELF_ID shelf_id PARAM_SEP
    BMD_SHELF_ENTRY_ID entry_id PARAM_END
{
    BRI_CONTEXT *shelf = (BRI_CONTEXT *)shelf_id;
    BRI_CONTEXT *book;
    jmp_buf error_return;

    /* Set up the jump_buffer for error unwinding prior to calling any
     * routines.  From this point on any errors that occur while
     * opening the book will result in a call to BriLongJmp which after
     * processing the error will do a longjmp to "return" from the setjmp().
     */
    if (setjmp(error_return) != 0) {

        /* Whoops!!! An error was detected while trying to open the book.
         * The reason should be in the book context.  Report the error,
         * delete the book context, and return NULL to indicate to the
         * calling routine that an error occurred.
         */
        BriError(book,BriErrBookOpenNum);
        BriContextDelete(book);
        return NULL;
    }

    /* Set up the context for the book.  Note that the address of the
     * book context pointer is passed so that it can be set prior to
     * any errors being reported via BriLongJmp().  
     */
    BriContextInherit(&book,shelf,entry_id,BMD_C_BOOK,error_return);

    OpenBook(book);

    return (BMD_BOOK_ID)book;

}	/*  end of bri_book_open  */


/*
**  bri_book_title -- get title of book
*/
char * 
bri_book_title PARAM_NAMES((bkid))
    BMD_BOOK_ID bkid PARAM_END

{

    return BriBookPtr(bkid)->vbh.book_name;

};	/*  end of bri_book_title*/

/*
**  bri_book_file_name -- get title of book
*/
char * 
bri_book_file_name PARAM_NAMES((bkid))
    BMD_BOOK_ID bkid PARAM_END

{
    
    return ((BRI_CONTEXT *)bkid)->entry.target_file;

};	/*  end of bri_book_title*/

/*
**  bri_book_found_file_spec
*/
char * 
bri_book_found_file_spec PARAM_NAMES((bkid))
    BMD_BOOK_ID bkid PARAM_END

{
    
    return ((BRI_CONTEXT *)bkid)->found_file_spec;

};	/*  end of bri_book_title*/

/*
** bri_book_version -- return the book's version numbers
*/
void
bri_book_version PARAM_NAMES((bkid,version))
    BMD_BOOK_ID bkid PARAM_SEP
    BMD_VERSION *version PARAM_END
{
    BRI_CONTEXT *context = (BRI_CONTEXT *)bkid;

    *version = context->data.book->vbh.version;
}

/*
** bri_book_no_print
*/
BR_UINT_32
bri_book_no_print PARAM_NAMES((bkid))
    BMD_BOOK_ID bkid PARAM_END

{
    BRI_CONTEXT *context;

    context = (BRI_CONTEXT *)bkid;

    return(context->no_print);
}

/*
** bri_book_chunk_count
*/
BR_UINT_32
bri_book_chunk_count PARAM_NAMES((bkid))
    BMD_BOOK_ID bkid PARAM_END

{
    BRI_CONTEXT *context = (BRI_CONTEXT *)bkid;

    return context->data.book->vbh.n_chunks;
}

/*
**  bri_logical -- translate a logical name
*/
char * bri_logical PARAM_NAMES((name))
    char *name PARAM_END
{
#ifdef vms
#pragma nostandard
    $DESCRIPTOR (tablename_dsc, "LNM$FILE_DEV");    /*  table to search  */
#pragma standard

    BR_UINT_32	status;
    BR_UINT_32	lognam_len = 0;
    char	namebuff[LNM$C_NAMLENGTH];/*  buffer for name*/
    char	lognam[LNM$C_NAMLENGTH];/*  buffer for translation  */
    BR_UINT_32  attr_mask;
#pragma nostandard
    struct dsc$descriptor name_dsc = 	/*  logical name dsc.        */
	{strlen (name), DSC$K_DTYPE_T, DSC$K_CLASS_S, name};
#pragma standard

    VMS_ITEM_LIST item_list[2];			/*  array of item list dsc   */

    item_list[0].length =   LNM$C_NAMLENGTH;
    item_list[0].itemcode = LNM$_STRING;
    item_list[0].pointer =  lognam;
    item_list[0].ret_len =  (BR_UINT_16 *) &lognam_len;
    item_list[1].length =   0;		/*  null terminate the item list  */
    item_list[1].itemcode = 0;		

    attr_mask = LNM$M_CASE_BLIND;	/* ignore case  */    

    status = SYS$TRNLNM (&attr_mask, &tablename_dsc, &name_dsc, 0, item_list);

    if (status == SS$_NORMAL)
    {
	BR_INT_32 num;
	for (num = 0; num < 10; num++)	/*  translate iteratively  */
	{
	    name_dsc.dsc$a_pointer = lognam;
	    name_dsc.dsc$w_length = lognam_len;
	    status = SYS$TRNLNM (
		&attr_mask, &tablename_dsc, &name_dsc, 0, item_list);
	    if (status == SS$_NOLOGNAM)
	    {
		char	*trans_name = malloc (lognam_len + 1);
		strncpy (trans_name, lognam, lognam_len);
		trans_name[lognam_len] = 0; 
		return trans_name;
	    }
	}
    }
    return name;
#else

    /* ULTRIX doesn't have logical names, so use environment variables.
     */
    extern char *getenv();

    char *value = getenv(name);

    if (value == NULL) {
        return name ;
    } else {
        return value ;
    }
#endif
}
