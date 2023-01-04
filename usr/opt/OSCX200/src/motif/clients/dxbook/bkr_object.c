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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_OBJECT.C*/
/* *7     9-JUN-1992 09:57:41 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *6    24-APR-1992 16:40:04 BALLENGER "Support window positioning through the API."*/
/* *5     3-MAR-1992 17:01:26 KARDON "UCXed"*/
/* *4     2-JAN-1992 10:18:07 FITZELL "Xbook references need their own shell in exref _dispatch"*/
/* *3    26-SEP-1991 17:13:15 BALLENGER "Fix problems with reopening directories with API."*/
/* *2    17-SEP-1991 21:27:31 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:39:57 PARMENTER "Object Id Dispatch"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_OBJECT.C*/
#ifndef VMS
 /*
#else
#module BKR_OBJECT "V03-0000"
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
**	Object Id dispatch routines. 
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     11-Oct-1990
**
**  MODIFICATION HISTORY:
**
**  V03-0001	DBL001		David L Ballenger	06-Feb-1991
**      	Add "extern" declarations for bkr_selection_create(),
**		bkr_selection_open(), and bkr_selection_find_entry_by_id().
**
**--
**/


/*
 * INCLUDE FILES
 */

#include "br_common_defs.h"       /* common BR #defines */
#include "br_meta_data.h"         /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"          /* BR high-level typedefs and #defines */
#include "br_globals.h"           /* BR external variables declared here */
#include "bri_dir.h"
#include "bkr_object.h"           /* function prototypes for .c module */
#include "bkr_book.h"             /* Book access routines */
#include "bkr_cursor.h"           /* Cursor manipulation routines */
#include "bkr_error.h"            /* Error reporting */
#include "bkr_fetch.h"            /* Resource fetching */
#include "bkr_pointer.h"          /* Pointer manipulation routines */
#include "bkr_selection.h"        /* Selection window manipulation routines */
#include "bkr_selection_create.h" /* Selection window creation routines */
#include "bkr_selection_open.h"   /* Selection window open routines */
#include "bkr_shell.h"            /* Shell access routines */
#include "bkr_topic_open.h"       /* Topic window open routines */
#include "bkr_window.h"           /* Window manipulation routines


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_object_exref_dispatch
**
** 	Calls the appropriate topic or directory display routine based on
**  	the object id of the external reference.
**
**  FORMAL PARAMETERS:
**
**	exref_book_id	- id of the book that contains the external reference.
**  	exref_object_id - id of the external reference (in source book).
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
**	Returns the window of the opened object or NULL.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
BKR_WINDOW *
bkr_object_exref_dispatch PARAM_NAMES((exref_book_id,exref_object_id))
    BMD_BOOK_ID	    	exref_book_id PARAM_SEP
    BMD_OBJECT_ID	exref_object_id PARAM_END

{
    char    	    	*book_file;
    BKR_BOOK_CTX    	*book;
    BKR_SHELL           *shell;
    BKR_WINDOW 	    	*window = NULL;
    char    	    	*symbol_name;
    BMD_OBJECT_ID	object_id;
    BMD_OBJECT_TYPE 	exref_type = BMD_C_NO_OBJECT_TYPE;
    BMD_OBJECT_TYPE 	object_type = BMD_C_NO_OBJECT_TYPE;
    char	    	*book_title;
    unsigned    	*exref_timestamp;

    if ( ( exref_book_id == NULL ) || ( exref_object_id == 0 ) )
    	return NULL;

    /*  Get the exref type and info  
     */
    exref_type = bri_get_exref_info( exref_book_id, exref_object_id, 
	    	    &book_file, &book_title, &exref_timestamp, 
    	    	    &symbol_name, &object_id );

    if ( ( exref_type != BMD_C_SYMBOL_EXREF_OBJECT )
	    && ( exref_type != BMD_C_STATIC_EXREF_OBJECT ) )
    	return NULL;

    if ( book_file == NULL )
    	return NULL;

    /* Open the book
     */
    book = bkr_book_get( book_file, NULL, 0 );
    if ( book == NULL ) {
        return NULL;
    }

    /*  For symbol ref's get the symbol name and the object id from the
     *  symbol ref. if the symbol exists, otherwise get the object id 
     *  for the static reference.
     */
    if ( exref_type == BMD_C_SYMBOL_EXREF_OBJECT ) {
	if ( symbol_name != NULL ) {
	    object_id = bri_get_symbol_object( book->book_id, symbol_name );
	    object_type = bri_get_object_type( book->book_id, object_id );
	} else {
	    object_id = 0;
        }
    } else {
        /* BMD_C_STATIC_EXREF_OBJECT
         *
	 * Compare the timestamp from the book context with
    	 * the one stored in the external reference.
    	 */
	if ((book->timestamp[0] == exref_timestamp[0])
            && (book->timestamp[1] == exref_timestamp[1])
            ){
	    object_type = bri_get_object_type( book->book_id, object_id );
        } else {
	    object_id = 0;
        }
    }

    /*  No symbol or object so assume the reference is to the whole book 
     */
    if ( object_id == 0 ) {
    	object_type = BMD_C_BOOK_OBJECT;
    }

    /* Get a shell to use for this set of windows.
     */
    shell = bkr_shell_get(book,NULL,TRUE);
    if (shell == NULL) {

        /* We're through with this book so "close" it. The close
         * routine won't close it out from under other users though.
         */
        bkr_book_free( book );
        return NULL;
    }

    /*  Validate the object type
     */
    switch ( object_type ) 
    {
    	case BMD_C_BOOK_OBJECT : {
            window = shell->selection;
            if (window) {
                bkr_window_setup(window->widgets[W_MAIN_WINDOW],NULL,TRUE );
            } else if (bkr_selection_open(shell,0)) {
                window = shell->selection;
            }
            break;
        }
    	case BMD_C_DIRECTORY_OBJECT :
    	case BMD_C_DIRECTORY_ENTRY_OBJECT : {
            window = bkr_object_id_dispatch(shell,NULL,object_id);
            break;
        }
    	case BMD_C_CHUNK_OBJECT :
    	case BMD_C_TOPIC_OBJECT : {
            window = bkr_object_id_dispatch(shell,shell->default_topic,object_id);
            break;
         }
    	case BMD_C_SHELF_OBJECT :
    	case BMD_C_SHELF_ENTRY_OBJECT :
    	case BMD_C_SYMBOL_EXREF_OBJECT :
    	case BMD_C_STATIC_EXREF_OBJECT :
    	case BMD_C_NO_OBJECT_TYPE :
    	default: {
            char		*error_string;
            error_string = 
            (char *) bkr_fetch_literal( "OBJECT_TYPE_ERROR", ASCIZ );
            if ( error_string != NULL ) {
                sprintf( errmsg, error_string, object_type );
                bkr_error_modal( errmsg, NULL );
                XtFree( error_string );
            }
            break;
        }
     }
    if (window == NULL) {
        
        /* We weren't successful, so close the shell that we created.
         * This may also result in the book being closed if there are
         * no other shells.
         */
        bkr_shell_free(shell,TRUE);
    }
    return window;
};  /* end of bkr_object_exref_dispatch */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_object_id_dispatch
**
** 	Calls the appropriate topic or directory display routine based on
**  	the object id passed.
**
**  FORMAL PARAMETERS:
**
**	window	    	    - pointer to the Shell.
**  	object_id   	    - id of the object to open.
**  	create_new_window   - boolean: whether to create new window.
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
**	Returns the window of the opened object or NULL.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
BKR_WINDOW *
bkr_object_id_dispatch PARAM_NAMES((shell,window,object_id))
    BKR_SHELL           *shell PARAM_SEP
    BKR_WINDOW          *window PARAM_SEP
    BMD_OBJECT_ID       object_id PARAM_END

{
    BMD_OBJECT_TYPE 	object_type;
    BMD_BOOK_ID	    	book_id = shell->book->book_id;

    if ( ( book_id == NULL ) || ( object_id == 0 ) )
    	return NULL;

    bkr_cursor_display_wait( ON );

    object_type = bri_get_object_type( book_id, object_id );
    switch ( object_type )
    {
    	case BMD_C_TOPIC_OBJECT :
    	case BMD_C_CHUNK_OBJECT : {

            BMD_OBJECT_ID   	    page_id;
            BMD_OBJECT_ID   	    chunk_id;

    	    if ( object_type == BMD_C_TOPIC_OBJECT ) {
    	    	page_id = BRI_LOCAL_OBJECT_NUMBER( object_id );
                if (page_id == 0) {
                    page_id = window->shell->book->first_page;
                }
    	    	chunk_id = 0;
    	    } else {
    	    	page_id = 0;
    	    	chunk_id = object_id;
    	    }

    	    if ( window && (window->u.topic.outlined_chunk != NULL)) {
                bkr_pointer_outline_hot_spot(window,
                                             window->u.topic.outlined_chunk,
                                             OFF );
            }
    	    window = bkr_topic_open_to_position(
    	    	shell,	    	    	/* Parent Selection shell */
    	    	window,  	    	/* Topic shell to use	  */
    	    	page_id,	    	/* Page id 	    	  */
    	    	0, 0,	    	    	/* x, y, position   	  */
    	    	chunk_id,   	    	/* Chunk id 	    	  */
    	    	(window != NULL) );	/* add to history list?   */
    	    break;
        }
    	case BMD_C_SYMBOL_EXREF_OBJECT :
    	    window = bkr_object_exref_dispatch( book_id, object_id );
    	    break;

    	case BMD_C_DIRECTORY_OBJECT :
    	case BMD_C_DIRECTORY_ENTRY_OBJECT :
    	{
    	    BMD_OBJECT_ID   dir_id = BRI_DIRECTORY_ID( object_id );
    	    int	    	    entry_num  = BRI_DIRECTORY_ENTRY_NUMBER( object_id );

            /* Make sure that we have a selection window.
             */
            if ((shell->selection == NULL) ||
                (shell->selection->active == NULL)) {
                if (bkr_selection_open(shell,dir_id) == NULL) {
                    bkr_cursor_display_wait( OFF );
                    return NULL;
                }
            }
            window = shell->selection;

    	    /*  Make the specific entry visible within the display
    	     */
            bkr_selection_display_entry(window, dir_id, entry_num );
    	    bkr_window_setup( window->widgets[W_MAIN_WINDOW], NULL, TRUE );
    	    break;
    	}   	/* end case directory objects */

    	case BMD_C_BOOK_OBJECT :
    	case BMD_C_SHELF_OBJECT :
    	case BMD_C_SHELF_ENTRY_OBJECT :
    	case BMD_C_NO_OBJECT_TYPE :
    	default :
    	    printf( "Invalid object type: %d\n", object_type );
    	    return NULL;
    }

    bkr_cursor_display_wait( OFF );

    return window;

};  /* end of bkr_object_id_dispatch */



