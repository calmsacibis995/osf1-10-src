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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_TOPIC_OPEN.C*/
/* *23   24-FEB-1993 17:47:49 BALLENGER "Fixes for large topic and Region memory leak."*/
/* *22   25-NOV-1992 18:36:20 BALLENGER "Fix problem with going to chunks in CC mode."*/
/* *21    8-NOV-1992 19:21:51 BALLENGER "Use work in progress box instead of wait cursor on character cell."*/
/* *20   12-OCT-1992 17:39:29 BALLENGER "Fix problems with formal topics in CC mode and clean up window creation."*/
/* *19   21-SEP-1992 22:13:51 BALLENGER "Focus highlight in topic window."*/
/* *18    5-AUG-1992 21:48:46 BALLENGER "Hotspot traversal and handling for character cell support."*/
/* *17    9-JUN-1992 09:59:57 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *16   24-APR-1992 16:43:25 BALLENGER "Support window positioning through the API."*/
/* *15   23-APR-1992 14:52:44 BALLENGER "make sure titlebar and icon titles reset in default window reuse."*/
/* *14   10-APR-1992 13:57:27 ROSE "Fixed hot spot bug for real"*/
/* *13    6-APR-1992 18:09:00 ROSE "Done"*/
/* *12    3-APR-1992 17:21:04 FITZELL "decworld hooks"*/
/* *11   30-MAR-1992 20:50:14 GOSSELIN "conditionalized bkr_un_highlight"*/
/* *10   29-MAR-1992 16:33:04 FITZELL "added un_highlight to open_to_position "*/
/* *9    14-MAR-1992 14:20:17 BALLENGER "Have bkr_topic_data_get open the page"*/
/* *8     8-MAR-1992 19:16:22 BALLENGER "  Add topic data and text line support"*/
/* *7     5-MAR-1992 14:26:20 PARMENTER "adding simple search"*/
/* *6     3-MAR-1992 17:05:41 KARDON "UCXed"*/
/* *5    12-FEB-1992 12:14:31 PARMENTER "i18n support for titles and icons"*/
/* *4     8-NOV-1991 17:30:32 BALLENGER "Fix open nav window menu item bug."*/
/* *3     1-NOV-1991 12:58:55 BALLENGER "Reintegrate  memex support"*/
/* *2    17-SEP-1991 21:23:15 BALLENGER "change use of major and minor in version number to avoid conflict with sys/types.h*/
/*definition on ULTRIX"*/
/* *1    16-SEP-1991 12:40:59 PARMENTER "Setup Topic Windows"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_TOPIC_OPEN.C*/
#ifndef VMS
 /*
#else
#module BKR_TOPIC_OPEN "V03-0000"
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
**	"Open Topic" callback routines.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     18-Jun-1990
**
**  MODIFICATION HISTORY:
**
**
**--
**/


/*
 * INCLUDE FILES
 */

#include "br_common_defs.h"    /* common BR #defines */
#include "br_meta_data.h"      /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"       /* BR high-level typedefs and #defines */
#include "br_globals.h"        /* BR external variables declared here */
#include "br_malloc.h"         /* BKR_MALLOC, etc defined here */
#ifdef SEARCH
#include "bkr_search.h"        /* function prototypes for .c module */
#endif /* SEARCH */
#include "bkr_close.h"         /* function prototypes for .c module */
#include "bkr_copyright.h"     /* function prototypes for .c module */
#include "bkr_cursor.h"        /* function prototypes for .c module */
#include "bkr_font.h"          /* Font manipulation routines */
#include "bkr_object.h"        /* function prototypes for .c module */
#include "bkr_scroll.h"        /* function prototypes for .c module */
#include "bkr_shell.h"         /* function prototypes for .c module */
#include "bkr_topic_create.h"  /* function prototypes for .c module */
#include "bkr_topic_init.h"    /* function prototypes for .c module */
#include "bkr_topic_open.h"    /* function prototypes for .c module */
#ifdef USE_TEXT_LINES
#include "bkr_topic_data.h"    /* function prototypes for .c module */
#endif 
#include "bkr_window.h"        /* function prototypes for .c module */
#ifdef MEMEX
#include "bmi_topic.h"
#endif
#include <DXm/DECspecific.h>
#include "bkr_button.h"
#ifdef DECWORLD
#include "bkr_decworld.h"
#endif

/*
 * LOCAL ROUTINES
 */
static void  	    	    add_topic_to_history_list();
static BKR_WINDOW    *topic_already_open
    PROTOTYPE((BKR_SHELL       *parent_shell,
               BMD_OBJECT_ID   page_id));

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_topic_open_in_default
**
** 	Callback routine for opening a Topic in the "default" window.
**
**  FORMAL PARAMETERS:
**
**  	widget	    	- widget id of the push button that caused the event.
**  	data	    	- generic user data.
**	callback_data	- pointer to callback data (unused).
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
bkr_topic_open_in_default PARAM_NAMES((widget,data,callback_data))
    Widget		    widget PARAM_SEP
    caddr_t 	    	    data PARAM_SEP
    XmAnyCallbackStruct     *callback_data PARAM_END

{
    BKR_SHELL	            *parent;
    BKR_WINDOW              *window;
    int	    	    	    i;
    BKR_DIR_ENTRY   	    *entry;
    unsigned	    	    target_id = 0;
    Arg	    	    	    arglist[5];
    int	    	    	    argcnt;
    Boolean 	    	    popup_menu_callback = FALSE;

    /* 
     *  Get the user data, if non-NULL then the callback is from either the 
     *  Selection or Topic popup menus, otherwise its from a Topic shell 
     *  pulldown or a Selection pulldown menu.
     */

    argcnt = 0;
    SET_ARG( XmNuserData, &window );
    XtGetValues( widget, arglist, argcnt );
    if ( window != NULL ) {
    	popup_menu_callback = TRUE;
    } else {
        window = (BKR_WINDOW *) data;
    }

    parent = window->shell;

    switch ( window->type ) {
    	case BKR_SELECTION :
            entry = NULL ;
    	    if ( popup_menu_callback ) {
    	    	entry = window->u.selection.btn3down_entry_node;
    	    	window->u.selection.btn3down_entry_node = NULL;	    /* clear target id */
    	    	if ( entry->u.entry.target == NULL )
    	    	    return;
    	    } else if ( window->u.selection.selected_entry_tags)  {

                /* Find the first Topic with targets in the selected list */
                
                for ( i = 0; i < window->u.selection.num_selected; i++ ) {
                    entry = (BKR_DIR_ENTRY *) window->u.selection.selected_entry_tags[i];
                    if ( entry->u.entry.target) {
                        break;
                    }
                }
            }
    	    if ( entry == NULL ) {   /* NO entry exists! */
    	    	return;
            }

    	    /* Open the Topic 
             */
    	    if ( entry->u.entry.target ) {
    	    	bkr_object_id_dispatch(parent,parent->default_topic,entry->u.entry.target);
    	    }
    	    break;  /* end case BKR_SELECTION */

    	case BKR_STANDARD_TOPIC :
    	case BKR_FORMAL_TOPIC :

    	    if ( popup_menu_callback )
    	    {
    	    	BMD_CHUNK   *target_chunk;

    	    	target_chunk = window->u.topic.btn3down_popup_chunk;
    	    	window->u.topic.btn3down_popup_chunk = NULL; 	/* clear target chunk */
    	    	if ( target_chunk == NULL )
    	    	    return;
    	    	target_id = target_chunk->target;
    	    }
    	    else if ( widget == window->widgets[W_OPEN_TOPIC_IN_DEFAULT] ) 
    	    {
    	    	if ( window->u.topic.selected_chunk != NULL )
    	    	    target_id = window->u.topic.selected_chunk->target;
    	    	else
    	    	    return; 	/* No hotspots are selected! */
    	    }
    	    if ( target_id == 0 )
    	    	return;

    	    bkr_object_id_dispatch(parent,parent->default_topic,target_id);

    	    break;  /* end case BKR_STANDARD_TOPIC and BKR_FORMAL_TOPIC */

    }	/* end of switch */

};  /* end of bkr_topic_open_in_default */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_topic_open_in_new
**
** 	Callback routine for opening a Topic in a new window.
**
**  FORMAL PARAMETERS:
**
**  	widget	    	- widget id of the push button that caused the event.
**  	data	    	- generic user data.
**	callback_data	- pointer to callback data (unused).
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
bkr_topic_open_in_new PARAM_NAMES((widget,data,callback_data))
    Widget		    widget PARAM_SEP
    caddr_t 	    	    data PARAM_SEP
    XmAnyCallbackStruct     *callback_data PARAM_END

{
    BKR_SHELL	    	    *parent;
    BKR_WINDOW              *window;
    int	    	    	    i;
    BKR_DIR_ENTRY   	    *entry;
    unsigned	    	    target_id = 0;
    Arg	    	    	    arglist[5];
    int	    	    	    argcnt;
    Boolean 	    	    popup_menu_callback = FALSE;

    /* 
     *  Get the user data, if non-NULL then the callback is from either the 
     *  Selection or Topic popup menus, otherwise its from a Topic shell 
     *  pulldown or a Selection pulldown menu.
     */

    argcnt = 0;
    SET_ARG( XmNuserData, &window );
    XtGetValues( widget, arglist, argcnt );
    if ( window != NULL ) {
    	popup_menu_callback = TRUE;
    } else {
    	window = (BKR_WINDOW *) data;
    }
    parent = window->shell;

    switch ( window->type )
    {
    	case BKR_SELECTION :
    	    if ( popup_menu_callback )
    	    {
    	    	entry = window->u.selection.btn3down_entry_node;
    	    	window->u.selection.btn3down_entry_node = NULL;	    /* clear target id */
    	    	if ( entry->u.entry.target == NULL )
    	    	    return;
    	    }
    	    else
    	    {
    	    	if ( window->u.selection.selected_entry_tags == NULL )
    	    	    return;

    	    	/* Find the first Topic with targets in the selected list */
    
    	    	for ( i = 0; i < window->u.selection.num_selected; i++ )
    	    	{
    	    	    entry = (BKR_DIR_ENTRY *) window->u.selection.selected_entry_tags[i];
    	    	    if ( entry->u.entry.target )
    	    	    	break;
    	    	}
    	    }
    	    if ( entry == NULL )    /* NO selected entries have targets! */
    	    	return;

    	    /*  Open the topic in a "new" window
             */
    	    if ( entry->u.entry.target ) {
    	    	bkr_object_id_dispatch(parent,NULL,entry->u.entry.target);
    	    }
    	    break;  /* end case BKR_SELECTION */

    	case BKR_STANDARD_TOPIC :
    	case BKR_FORMAL_TOPIC :

    	    if ( popup_menu_callback )
    	    {
    	    	BMD_CHUNK   *target_chunk;

    	    	target_chunk = window->u.topic.btn3down_popup_chunk;
    	    	window->u.topic.btn3down_popup_chunk = NULL; 	/* clear target chunk */
    	    	if ( target_chunk == NULL )
    	    	    return;
    	    	target_id = target_chunk->target;
    	    }
    	    else if ( widget == window->widgets[W_OPEN_TOPIC_IN_NEW] ) 
    	    {
    	    	if ( window->u.topic.selected_chunk != NULL )
    	    	    target_id = window->u.topic.selected_chunk->target;
    	    	else
    	    	    return; 	/* No hotspots are selected! */
    	    }
    	    if ( target_id != 0 ) {
                bkr_object_id_dispatch(parent,NULL,target_id);
            }

    	    break;  /* end case BKR_STANDARD_TOPIC and BKR_FORMAL_TOPIC */
    }	/* end switch */

};  /* end of bkr_topic_open_in_new */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_topic_open_to_chunk
**
** 	Opens a topic to the specified chunk but doesn't create
**  	the Selection shell.
**
**  FORMAL PARAMETERS:
**
**	book_id	     - id of the book containing the chunk.
**	book_version - version of the book.
**  	page_id	     - id of the topic to open.
**	chunk_id     - id of the chunk to open.
**
**  IMPLICIT INPUTS:
**
**	It is assumed that the book has already been successfully opened.
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	Returns the opened window or NULL.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
BKR_WINDOW *
bkr_topic_open_to_chunk PARAM_NAMES((book,page_id,chunk_id))
    BKR_BOOK_CTX    *book PARAM_SEP
    BMD_OBJECT_ID   page_id PARAM_SEP
    BMD_OBJECT_ID   chunk_id PARAM_END

{
    BKR_SHELL	    *parent;

    parent = bkr_shell_get(book,NULL,FALSE);
    if (parent == NULL) {
        return NULL;
    } else {
        /* Open the Topic to the specified chunk
         */
        return (bkr_topic_open_to_position(parent,NULL,page_id,0,0,chunk_id,FALSE));
    }
};  /* end of bkr_topic_open_to_chunk */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_topic_open_to_position
**
** 	Opens a topic either in the default window or a new window.
**
**  FORMAL PARAMETERS:
**
**  	parent_shell	- pointer to the Selection shell which will 
**  	    	    	    become the parent for the Topic shell.
**  	this_shell  	- pointer to the Topic shell to display the topic.
**  	pg_id	    	- id of the page to open.  
**  	x_pos	    	- initial x position within topic when opened.
**  	y_pos	    	- initial y position within topic when opened.
**  	chunk_id    	- id of the chunk to open.
**  	open_in_new 	- Boolean: whether to open the entry a "new" window.
**  	save_for_history - Boolean: whether to add the previously open 
**  	    	    	    	topic to the history list for the Topic window.
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
**	Returns the opened window or NULL.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
BKR_WINDOW *
bkr_topic_open_to_position PARAM_NAMES((shell,window,pg_id,x_pos,y_pos,chunk_id,save_for_history))
    BKR_SHELL	    *shell PARAM_SEP
    BKR_WINDOW	    *window PARAM_SEP
    BMD_OBJECT_ID   pg_id PARAM_SEP
    int	    	    x_pos PARAM_SEP
    int	    	    y_pos PARAM_SEP
    BMD_OBJECT_ID   chunk_id PARAM_SEP
    Boolean 	    save_for_history PARAM_END

{
    BMD_BOOK_ID	    book_id = shell->book->book_id;
    unsigned	    num_chunks;
    int	    	    topic_width;
    int	    	    topic_height;
    BKR_WINDOW_TYPE new_page_type;
    Boolean         open_by_chunk;
    Boolean 	    update_topic;
    int		    top_x = x_pos;
    int		    top_y = y_pos;
    Arg             arglist[20];
    int             argcnt;
    Pixel           backgroundColor;
    Pixmap          backgroundPixmap;
    Boolean         new_window;
    BKR_TOPIC_DATA  *topic_data;
    XmString        cs_title;
    long	    byte_cnt, stat;	
    long            cs_length;
    long            cs_status;



#ifdef COPY
    if(bkr_copy_ctx.startline)
	bkr_un_highlight(bkr_copy_ctx.window);
#endif

    if (bkrplus_g_charcell_display) 
    {
        bkr_cc_working( ON );
    }
    else 
    {
        bkr_cursor_display_wait( ON );
    }

    if (pg_id == 0) {
        open_by_chunk = TRUE;
        pg_id = bri_page_chunk_page_id(book_id,chunk_id);
    } else {
        open_by_chunk = FALSE;
    }
    /* Open the page and see if we can reuse the window passed in or
     * the default window. 
     */
    topic_data = bkr_topic_data_get(shell->book,pg_id);
    if (topic_data == NULL) {
        return NULL;
    }
    new_page_type = topic_data->type;

    switch (new_page_type) {

        case BKR_STANDARD_TOPIC: {

            /* New page is a regular topic, so we can use the window
             * passed in if it is a standard topic, or we can use the
             * default topic if it exists and is not active.
             */
            if (window) {
                if (window->type != BKR_STANDARD_TOPIC) {
                    window = NULL;
                }
            } else if (shell->default_topic 
                       && (shell->default_topic->active == FALSE)
                       ) {
                window = shell->default_topic;
            }
            break;
        }

        case BKR_FORMAL_TOPIC: {

            /* Formal topics never reuse other topic windows.
             */
            window = NULL;
            break;
        }
        
        default: {
            /* Unknown page type so return null.
             */
            return NULL;
        }
    }

    if (window == NULL) {
        update_topic = FALSE;
        window = (BKR_WINDOW *) BKR_MALLOC( sizeof( BKR_WINDOW ) );
        if (window == NULL) {
            return NULL;
        }
        memset( window, 0, sizeof( BKR_WINDOW ) );

        window->widgets = (WidgetList)BKR_CALLOC(K_MAX_STANDARD_TOPIC_WIDGETS,sizeof(Widget));
        if (window->widgets == NULL) {
            BKR_FREE(window);
            return NULL;
        }
        memset(window->widgets,0,(K_MAX_STANDARD_TOPIC_WIDGETS * sizeof(Widget)));
        
        window->shell = shell;
        window->type = new_page_type;
#ifdef SEARCH
        bkr_initialize_search_context(window);
#endif 
        if ((new_page_type == BKR_FORMAL_TOPIC) 
            || (shell->default_topic)
            ) {
            window->u.topic.sibling = shell->other_topics;
            shell->other_topics = window;
        } else {
            shell->default_topic = window;
        }
    } else {
        update_topic = TRUE;
        if ( save_for_history && window->active ) {
            add_topic_to_history_list(window);
        }
    }
    
    if (window->u.topic.page_id != 0) {
        bkr_close_topic_data(window);
    }

    /* Get the topic display information for the newly opened topic.
     */
    window->u.topic.page_id = pg_id;
    window->u.topic.data = topic_data;
    window->u.topic.title = topic_data->title;
    window->u.topic.width = topic_data->width;
    window->u.topic.height = topic_data->height;
    window->u.topic.num_chunks = topic_data->num_chunks;
    window->u.topic.chunk_list = topic_data->chunk_list;

    if ( open_by_chunk ) {

        /* Find the Y-coordinate of the chunk passed
         */
    	BMD_CHUNK   *chunk;
    	int 	    cknum;

    	top_y = 0;
    	chunk = (BMD_CHUNK *) &window->u.topic.chunk_list[0];
    	for ( cknum = 0; cknum < window->u.topic.num_chunks; cknum++, chunk++ )
    	    if ( ( chunk->parent == 0 ) && ( chunk->id == chunk_id ) )
    	    {
                if (bkrplus_g_charcell_display && chunk->first_line) 
                {
                    /* In charcter cell mode we have to position to the
                     * first line of the chunk, if there is a first
                     * line, rather that the chunk->y.  That's because
                     * we don't modify the chunk coordinates since they
                     * are used by the PostScript printing code.
                     */
                    top_y = -chunk->first_line->y;
                }
                else 
                {
                    top_y = - chunk->rect.top;
                }
    	    	break;
    	    }
    }
    
    /*
     *  For version 1.0 books, we adjust the x position if the topic type 
     *  is a STANDARD topic because the "default" topic window width is now 
     *  narrower than it was in the version 1 Bookreader.
     */
    if ( ( window->shell->book->version.major_num == VERSION_V1 ) 
        && ( window->type == BKR_STANDARD_TOPIC ) ) {
	top_x = V1_TOPIC_ADJUST;
    }

    /* Fill in the rest of the values in the topic window structure
     */
    window->title_bar_name          = window->shell->book->title;
    window->icon_name               = window->u.topic.title;
    window->u.topic.x 	    	    = top_x;
    window->u.topic.y	    	    = top_y;
    window->u.topic.internal_x 	    = top_x;
    window->u.topic.internal_y	    = top_y;
    window->u.topic.outlined_chunk  = NULL;
    window->u.topic.selected_chunk  = NULL;
    window->u.topic.grop_pending    = FALSE;

    new_window = (window->appl_shell_id == NULL);
    if (new_window) {
        if ( ! bkr_topic_create_window(window)) {
            bkr_cursor_display_wait( OFF );
            return NULL ;
        }
    }
    else 
    {
	XClearWindow( bkr_display, XtWindow( window->widgets[W_WINDOW] ) );
	bkr_window_expose( window->widgets[W_WINDOW], window, NULL );

    }

    /* Put the topic title in the label
     */
    cs_title = DXmCvtFCtoCS( window->u.topic.title, &byte_cnt, &stat );
    XtVaSetValues( window->widgets[W_LABEL], XmNlabelString, cs_title, NULL );
    COMPOUND_STRING_FREE( cs_title );

    /* Make sure the titlebar and icon names are set,
     */
    bkr_window_set_names(window);

    /* If the main window is not active yet, then this is a new
     * window, or we are reusing one that had been closed. In either
     * case we need to do some additional set up.
     */
    if (window->active == FALSE) {

        /* Display the Copyright message from the book if any.  This should
         * happen whenever ther are no active windows.
         */
        if (shell->n_active_windows == 0) {
            bkr_copyright_display(window);
        }

        /* If the navigation/selection window is not open then
         * manage the "Open Navigation Window" item in the file
         * menu.
         */
        if ((shell->selection == NULL)
            || (shell->selection->appl_shell_id == NULL)
            || (shell->selection->active == FALSE)
            ){
            if ( ! XtIsManaged( window->widgets[W_OPEN_DEFAULT_DIR_ENTRY] ) ) {
                XtManageChild( window->widgets[W_OPEN_DEFAULT_DIR_ENTRY] );
            }
        }

	/* Set / reset the hot spot  field to the default resource value.
         */
	window->u.topic.show_hot_spots  = bkr_topic_resources.show_hot_spots;

        /* Manage the main window.
         */
        XtManageChild(window->widgets[W_MAIN_WINDOW] );

        /* We use the drawing window border as the focus highlight rectangle,
         * so the color of the border has to be the same as the background
         * color initially.  When and if the drawing window gets focus the
         * border will be changed.
         */
        bkr_topic_focus_highlight(window,FALSE);

        window->active = TRUE;
        shell->n_active_windows++;
    }

#ifdef FOO
    /* Only allow keyboard traversal into the drawing window if there are
     * hotspots.
     */
    argcnt = 0;
    SET_ARG(XmNtraversalOn, (window->u.topic.data->hot_spots ? TRUE : FALSE));
    XtSetValues( window->widgets[W_WINDOW], arglist, argcnt );
#endif 


    if (bkrplus_g_charcell_display) 
    {
        /* We have to turn this off before realizing the new window or
         * raising the old one so that any window that this is on top of
         * get the expose event before the new topic window comes up.
         */
        bkr_cc_working(OFF);
    }
    if (new_window) 
    {
        /* Make sure we have a window before setting the iconify
         * pixmap and creating the memex ui.
         */
        XtRealizeWidget( window->appl_shell_id );
    
        /* Set the icons only after we have realized the shell
         */
        bkr_window_set_icons(window);

#ifdef MEMEX
        bmi_create_topic_ui( window );
#endif 
    }
    else 
    {
        /* Just make sure the prexisting window is on screen.
         */
        RAISE_WINDOW( window->appl_shell_id );
    }


    /* Make the button box push button sensitive 
     */
    if ( window->type == BKR_STANDARD_TOPIC )
    {	
    	bkr_topic_init_sensitivity( window );
       	bkr_screen_init_sensitivity( window );
    }

    /* Adjust the scrollbars 
     */
    bkr_scroll_adjust_scrollbars( window, window->u.topic.x, window->u.topic.y, 
    	    	    	    	    window->u.topic.width, window->u.topic.height );

#ifdef MEMEX
    bmi_update_chunk_highlighting( window, FALSE );
#endif 

    if ( ! bkrplus_g_charcell_display) 
    {
        bkr_cursor_display_wait( OFF );
    }

#ifdef DECWORLD
    if(bkr_decworld_start_timer){
	bkr_decworld_reset( window->widgets[W_MAIN_WINDOW], NULL );
	}
    bkr_decworld_reset_timer = TRUE;
#endif

    return window;

};  /* end of bkr_topic_open_to_position */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	add_topic_to_history_list
**
** 	Adds a topic to the history list for the given topic shell.
**
**  FORMAL PARAMETERS:
**
**  	topic_shell - a pointer to the Topic shell.
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
**	Virtual memory is allocated.
**
**--
**/
static void
add_topic_to_history_list PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END
{
    BKR_BACK_TOPIC  	*new_back_topic;
    BKR_BACK_TOPIC  	*back_topic;

    if ( window->type != BKR_STANDARD_TOPIC ) {
    	return;
    }

    /*  Ignore duplicates.
     */
    if ( ( back_topic = window->u.topic.back_topic ) != NULL )  {
    	if ( ( window->u.topic.page_id == back_topic->page_id )
	    	&& ( window->u.topic.x == back_topic->x )
	    	&& ( window->u.topic.y == back_topic->y ) )
    	{
	    return;
    	}
    }

    new_back_topic = (BKR_BACK_TOPIC *) BKR_MALLOC( sizeof(BKR_BACK_TOPIC ) );
    if (new_back_topic == NULL) {
        return;
    }

    new_back_topic->page_id = window->u.topic.page_id;
    new_back_topic->x	    = window->u.topic.x;
    new_back_topic->y	    = window->u.topic.y;

    new_back_topic->next    = window->u.topic.back_topic;
    window->u.topic.back_topic = new_back_topic;


    if ( window->widgets[W_VIEW_MENU] != NULL )
    {
	XtSetSensitive( window->widgets[W_GOBACK_ENTRY],  TRUE );
    }
    XtSetSensitive( window->widgets[W_GOBACK_BUTTON], TRUE );

};  /* end of add_topic_to_history_list */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_display_chunk_at_top
**
** 	Given a chunk id, the chunk is positioned at the top 
**  	of the Topic window and then displayed.
**
**  FORMAL PARAMETERS:
**
**  	window - a pointer to the Topic window
**  	chunk_id    - id of the chunk to display at the top.
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
bkr_display_chunk_at_top PARAM_NAMES((window,chunk_id))
    BKR_WINDOW *window PARAM_SEP
    BMD_OBJECT_ID   chunk_id PARAM_END
{
    int	    	    	top_y;
    BMD_CHUNK	    	*chunk;
    int	    	    	cknum;

    /* Position the topic at 0, 0 for starters */

    window->u.topic.internal_x = 0;
    window->u.topic.internal_y = 0;

    /* Make sure no graphics operations are pending */

    window->u.topic.grop_pending = FALSE;

    /*
     *  For version 1.0 books we scroll the STANDARD topic to the 
     *  left n pixels because the topic window in V2 is narrower than 
     *  the version 1 Bookreader.  This is done so the right side 
     *  of the topic will not get chopped off.
     */

    if ( ( window->shell->book->version.major_num == VERSION_V1 ) && 
    	 ( window->type == BKR_STANDARD_TOPIC ) )
    {
    	window->u.topic.internal_x = V1_TOPIC_ADJUST;
    }

    /* Find the "top" chunk to display */

    top_y = 0;
    chunk = (BMD_CHUNK *) &window->u.topic.chunk_list[0];
    for ( cknum = 0; cknum < window->u.topic.num_chunks; cknum++, chunk++ )
    	if ( ( chunk->parent == 0 ) && ( chunk->id == chunk_id ) )
    	if ( chunk->id == chunk_id )
    	{
    	    top_y = - chunk->rect.top;
    	    break;
    	}
    window->u.topic.internal_y = top_y;

    /* Update the display! */

    bkr_scroll_adjust_display( window );
    bkr_scroll_adjust_scrollbars(window,
                                 window->u.topic.internal_x, 
                                 window->u.topic.internal_y, 
                                 window->u.topic.width,
                                 window->u.topic.height );

};  /* end of bkr_display_chunk_at_top */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	topic_already_open
**
** 	Checks to see if a topic is already open in a given shell by
**  	looping through all the topic given a parent Selection shell.
**
**  FORMAL PARAMETERS:
**
**  	parent_shell	- pointer to parent Selection shell.
**  	this_shell  	- pointer to Topic shell to check.
**  	page_id	    	- id of page to check for being open.
**  	topic_shell_rtn	- address to return
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
**	Returns:    TRUE   if the given page was open in a Topic shell
**  	    	    FALSE, otherwise
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
static BKR_WINDOW *
topic_already_open PARAM_NAMES((shell,page_id))
    BKR_SHELL	    *shell PARAM_SEP
    BMD_OBJECT_ID   page_id PARAM_END

{
    BKR_WINDOW	*window;
    
    if (shell->default_topic
        && (shell->default_topic->active)
        && (shell->default_topic->u.topic.page_id == page_id)) {
        return shell->default_topic;
    }
    window = shell->other_topics;
    while (window) {
        if (window->active && (window->u.topic.page_id == page_id)) {
            break;
        }
        window = window->u.topic.sibling;
    }
    return window;

};  /* end of topic_already_open */



