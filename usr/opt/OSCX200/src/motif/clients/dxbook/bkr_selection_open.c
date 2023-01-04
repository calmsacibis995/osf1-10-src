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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_SELECTION_OPEN.C*/
/* *7     8-NOV-1992 19:21:41 BALLENGER "Use work in progress box instead of wait cursor on character cell."*/
/* *6     9-JUN-1992 09:58:38 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *5     3-MAR-1992 17:04:01 KARDON "UCXed"*/
/* *4     5-FEB-1992 10:39:18 FITZELL "fixes acvio when no directorie are found"*/
/* *3     1-NOV-1991 12:58:37 BALLENGER "Reintegrate  memex support"*/
/* *2    18-SEP-1991 18:01:14 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:40:32 PARMENTER "Sets up Selection window"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_SELECTION_OPEN.C*/
#ifndef VMS
 /*
#else
#module BKR_SELECTION_OPEN "V03-0000"
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
**	Routines for setting up a Selection window to display
**  	the contents of a directory from a book.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     1-May-1990
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
#include "br_globals.h"      /* BR external variables declared here */
#include "bkr_selection_open.h" /* function prototypes for .c module */
#include "bkr_book.h"        /* Book acces routines */
#include "bkr_close.h"       /* Close routines */
#include "bkr_directory.h"   /* Directory access routines */
#include "bkr_error.h"       /* Error reporting routines */
#include "bkr_selection.h"   /* Selection window access routines */
#include "bkr_selection_create.h" /* Selection window creation routines */
#include "bkr_shell.h"       /* Shell access routines */


static Boolean open_selection PROTOTYPE((BKR_SHELL *shell,BMD_OBJECT_ID dir_id_to_open));



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_selection_open
**
** 	Opens the Selection window, initializes the toplevel 
**  	directories, and opens to the directory specified.
**
**  FORMAL PARAMETERS:
**
**  	topic_shell 	- pointer to a Topic shell.
**  	dir_id_to_open	- id of the directory to open or 0.
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
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
Boolean
bkr_selection_open PARAM_NAMES((shell,dir_id_to_open))
    BKR_SHELL *shell PARAM_SEP
    BMD_OBJECT_ID dir_id_to_open PARAM_END
{
    Boolean status;

    /* The open_selection() routine does the real work. This is the
     * public interface which handles turning on and off the 
     * work-in-progress box.
     */
    if (bkrplus_g_charcell_display) {
        bkr_cc_working(ON);
    }

    status = open_selection(shell,dir_id_to_open);

    if (bkrplus_g_charcell_display) {
        bkr_cc_working(OFF);
    }
    return status;
};  /* end of bkr_selection_open */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_selection_open_book
**
** 	Opens the default directory within a book in either the default 
**  	or a new Selection window.
**
**  FORMAL PARAMETERS:
**
**  	filename    	    - string name of book to open.
**  	shelf_id    	    - id of shelf book is located on.
**  	entry_id    	    - books position within the shelf.
**  	use_default_window  - Boolean: open in default Selection window.
**  	node_id	    	    - generic node data passed when book is opened
**  	    	    	    	from the Library window. Data is only stored
**  	    	    	    	if not NULL.
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
**	Returns:    SHELL pointer of selection window created, or
**  	    	    NULL if an error occurred.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
BKR_SHELL *
bkr_selection_open_book PARAM_NAMES((filename,shelf_id,entry_id,use_default_shell,library_node))
    char    	    *filename PARAM_SEP
    BMD_SHELF_ID    shelf_id PARAM_SEP
    unsigned	    entry_id PARAM_SEP
    Boolean 	    use_default_shell PARAM_SEP
    BKR_NODE        *library_node PARAM_END

{
    BKR_BOOK_CTX            *book;
    BKR_SHELL	    	    *shell = NULL;

    if (bkrplus_g_charcell_display) {
        bkr_cc_working(ON);
    }
    /* Open the book
     */
    book = bkr_book_get(filename,shelf_id,entry_id);
    if (book) 
    {
        /* Create a new shell or reuse the default
         */
        shell = bkr_shell_get(book,NULL,(use_default_shell == FALSE));
        if (shell == NULL) 
        {
            /* We couldn't create a shell so free the book.
             */
            bkr_book_free( book );
        }
        else 
        {
            /* Create and/or initialize a new Selection window
             */
            if (open_selection(shell,book->default_directory->object_id)) 
            {
                shell->library_node = library_node;
            } 
            else 
            {
                bkr_shell_free(shell,FALSE);
                shell = NULL;
            }
        }
    }
    if (bkrplus_g_charcell_display) {
        bkr_cc_working(OFF);
    }
    return (shell);

};  /* end of bkr_selection_open_book */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_selection_open_default_dir
**
** 	Callback routine for opening the Selection window given
**  	a Topic window that has a dummy parent Selection window.
**  	This routine should ONLY be called if the topic was opened 
**  	directly without opening a Selection window first.
**
**  FORMAL PARAMETERS:
**
**	widget	      - id of the widget that caused the callback.
**	topic_shell   - pointer to the Topic shell.
**	callback_data - pointer to the callback data. (unused)
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
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
void
bkr_selection_open_default_dir PARAM_NAMES((widget,window,callback_data))
    Widget		    widget PARAM_SEP
    BKR_WINDOW              *window PARAM_SEP
    XmAnyCallbackStruct     *callback_data PARAM_END

{
    if (bkrplus_g_charcell_display) {
        bkr_cc_working(OFF);
    }
    /* Open the new hierarchy to the default directory
     */
    (void)open_selection(window->shell, 
                         window->shell->book->default_directory->object_id );

    if (bkrplus_g_charcell_display) {
        bkr_cc_working(OFF);
    }
};  /* end of bkr_selection_open_default_dir */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	open_selection
**
** 	Opens the Selection window, initializes the toplevel 
**  	directories, and opens to the directory specified.
**
**  FORMAL PARAMETERS:
**
**  	topic_shell 	- pointer to a Topic shell.
**  	dir_id_to_open	- id of the directory to open or 0.
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
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
static Boolean
open_selection PARAM_NAMES((shell,dir_id_to_open))
    BKR_SHELL *shell PARAM_SEP
    BMD_OBJECT_ID dir_id_to_open PARAM_END
{
    BKR_WINDOW    *window;

    /*  We better have a parent but NOT an application shell.
     */
    if ((shell == NULL) 
        || (shell->selection && shell->selection->active)) {
        return FALSE;
    }

    /*  Create the real Selection shell and copy the data
     */
    if (bkr_selection_create(shell) ) {
        /*  Open to the specified directory
         */
        if ( ! bkr_selection_new_hierarchy(shell->selection,dir_id_to_open)) { 
            char	*error_string;
            
            error_string = 
            (char *) bkr_fetch_literal( "DEFAULT_DIRECTORY_ERROR", ASCIZ );
            if ( error_string != NULL )
            {
                sprintf( errmsg, error_string );
                bkr_error_modal( errmsg, NULL );
                XtFree( error_string );
            }
	    bkr_cursor_display_inactive(shell->selection->appl_shell_id,FALSE);
            bkr_close_window(shell->selection,FALSE);
            return FALSE;
        }

        /* Turn off the "Open Default Directory" menu item in all of the
         * topic windows.
         */
        if (shell->default_topic) {
            XtUnmanageChild(shell->default_topic->widgets[W_OPEN_DEFAULT_DIR_ENTRY] );
        }
        window = shell->other_topics;
        while (window) {
            XtUnmanageChild(window->widgets[W_OPEN_DEFAULT_DIR_ENTRY] );
            window = window->u.topic.sibling;
        }
        return TRUE;
    }
    return FALSE;

};  /* end of open_selection */

