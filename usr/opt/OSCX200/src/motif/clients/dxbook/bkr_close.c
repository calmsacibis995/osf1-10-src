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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_CLOSE.C*/
/* *17   22-MAR-1993 14:37:47 BALLENGER "Fix problems with polygon hotspots."*/
/* *16   20-JUL-1992 13:33:34 BALLENGER "Character cell support."*/
/* *15   17-JUN-1992 21:01:42 BALLENGER "Use NODISPLAY to test for bad handle"*/
/* *14   11-JUN-1992 09:44:58 ROSE "Do not unmanage main window when closing topic windows; it destroys keyboard traversal when*/
/*the window is re-used"*/
/* *13    8-JUN-1992 18:59:11 BALLENGER "UCX$CONVERT"*/
/* *12    8-JUN-1992 11:33:24 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *11    9-MAY-1992 09:21:48 FITZELL "ifdef copy around initializing copy ctx"*/
/* *10    8-MAY-1992 16:20:19 FITZELL "initialize copy ctx on close"*/
/* *9     8-APR-1992 16:32:00 GOSSELIN "fixed bug on ALPHA"*/
/* *8     3-APR-1992 17:15:26 FITZELL "decworld hooks"*/
/* *7    14-MAR-1992 14:17:31 BALLENGER "Fix problems with window and icon titles..."*/
/* *6     8-MAR-1992 19:16:02 BALLENGER "  Add topic data and text line support"*/
/* *5     5-MAR-1992 14:24:53 PARMENTER "adding simple search"*/
/* *4     3-MAR-1992 16:57:23 KARDON "UCXed"*/
/* *3     1-NOV-1991 13:06:38 BALLENGER "Reintegrate  memex support"*/
/* *2    17-SEP-1991 18:13:16 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:38:54 PARMENTER "Window deletation and cleanup"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_CLOSE.C*/
#ifndef VMS
 /*
#else
#module BKR_CLOSE "V03-0002"
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
**	Window deletation and cleanup routines
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     16-Nov-1989
**
**  MODIFICATION HISTORY:
**
**  V03-0002  DLB0001	David L Ballenger	19-Feb-1991
**            Add support for BMD_CHUNK_RAGS_NO_FILL for PIC graphics 
**            support.
**
**  V03-0001  DLB0001	David L Ballenger	11-Feb-1991
**            Cache one dummy parent by leaving the book "open" when all
**            topic windows are closed, so that "reopening" the book
**            will be faster when coming in through HyperHelp.
**
**--
**/


/*
 * INCLUDE FILES
 */
#include <stdlib.h>          /* defines EXIT_SUCCESS */
#include "br_common_defs.h"  /* common BR #defines */
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_globals.h"      /* BR external variables declared here */
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "bkr_close.h"       /* function prototypes for .c module */
#include "bkr_image.h"       /* Image handling routines */
#include "bkr_library.h"     /* Library window routines */
#ifdef SEARCH
#include "bkr_search.h"      /* Search routines */
#endif /* SEARCH */
#include "bkr_selection.h"   /* Selection window routnes */
#include "bkr_shell.h"       /* Shell handling routines */
#ifdef MEMEX
#include "bmi_user_interface.h"
#endif
#ifdef USE_TEXT_LINES
#include "bkr_topic_data.h"
#endif 



/*
 * MACRO DEFINITIONS
 */
#define REMOVE_TOPIC_WINDOW( win_ptr )				\
{  XtUnmapWidget( (win_ptr)->appl_shell_id );			\
   XWithdrawWindow( bkr_display,				\
		    XtWindow( (win_ptr)->appl_shell_id ),	\
		    XDefaultScreen( bkr_display ) );		\
}

#define REMOVE_SELECTION_WINDOW( win_ptr )			\
{  if ( XtIsManaged( (win_ptr)->widgets[W_MAIN_WINDOW] ) )	\
       XtUnmanageChild( (win_ptr)->widgets[W_MAIN_WINDOW] );	\
   XtUnmapWidget( (win_ptr)->appl_shell_id );			\
   XWithdrawWindow( bkr_display,				\
		    XtWindow( (win_ptr)->appl_shell_id ),	\
		    XDefaultScreen( bkr_display ) );		\
}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_close_all_but_default_topic
**
** 	Closes all Topic windows and returns the "default" window if one
**  	exists otherwise returns the first window found.
**
**  FORMAL PARAMETERS:
**
**	sel_window - pointer to Selection window.
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
bkr_close_all_but_default_topic PARAM_NAMES((shell))
    BKR_SHELL *shell PARAM_END

{
    BKR_WINDOW	*window;
    BKR_WINDOW	*next_window;

    window = shell->other_topics;
    while (window) {
    	next_window = window->u.topic.sibling;
        bkr_close_window( window, TRUE );
    	window = next_window;
    }
    shell->other_topics = NULL;

    XFlush( bkr_display );  	/* get rid of the windows right away */

}  /* end of bkr_close_all_but_default_topic */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_close_shell
**
** 	Closes all windows for a shell
**
**  FORMAL PARAMETERS:
**
**  	shell              - pointer to the shell
**      unconditional_free - if true the shell will be freed even
**                           if it belongs to a client, otherwise
**                           it will be freed only if it doesn't
**                           belong to a client.
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
**	Virtual memory is freed.
**
**--
**/
void
bkr_close_shell PARAM_NAMES((shell,unconditional_free))
    BKR_SHELL *shell PARAM_SEP
    Boolean unconditional_free PARAM_END
{
    BKR_WINDOW *window;

    if (shell) {

        /* Close the selection window
         */
        bkr_close_window(shell->selection,unconditional_free);
        
        /* Close all of the topic windows
         */
        bkr_close_all_but_default_topic(shell);
        bkr_close_window(shell->default_topic,unconditional_free);

        /* Now see if we can free the shell
         */
        bkr_shell_free(shell,unconditional_free);
    }

};  /* end of bkr_close_shell */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_close_selection_window
**
** 	Callback routine for closing a single Selection window.
**
**  	The "default" Selection window is always cached when closed 
**  	and at least one other Selection window will be cached.
**
**  FORMAL PARAMETERS:
**
**  	widget	    	- id of the widget that caused the event.
**  	sel_window   	- pointer to the Selection window.
**  	callback_data	- pointer to the callback data (unused).
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
**	Virtual memory is freed.
**
**--
**/
void
bkr_close_selection_window PARAM_NAMES((widget,sel_window,callback_data))
    Widget		    	widget PARAM_SEP
    BKR_WINDOW	    	    	*sel_window PARAM_SEP
    XmPushButtonCallbackStruct  *callback_data 	PARAM_END /* not used */

{
    BKR_WINDOW	*window = NULL;

#ifdef DECWORLD
    bkr_decworld_check();
#endif

    if ( sel_window->type == BKR_SELECTION ) {
        bkr_close_shell(sel_window->shell,FALSE);
    }

};  /* end of bkr_close_selection_window */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_close_topic_data
**
**  	Frees all the allocated data associated with an open topic.
**
**  	    - chunk_list data which includes:
**  	    	    FTEXT, RAGS, images, POINT and XPOINT polygon data.
**  	    - closes the open BRI data page if only referenced once.
**  	    - zeros the book_id, page_id, and grop_pending fields.
**
**  FORMAL PARAMETERS:
**
**  	topic_window - pointer to the Topic window which 
**  	    	    	contains the topic to be closed.
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
**	Virtual memory is freed.
**
**--
**/
void
bkr_close_topic_data PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END
{
    BKR_BOOK_CTX        *book = window->shell->book;
    BMD_CHUNK	    	*chunk;
    int	    	    	page_open_cnt;

    if ( ( window->type != BKR_STANDARD_TOPIC ) &&
    	 ( window->type != BKR_FORMAL_TOPIC ) )
    	return;

#ifdef USE_TEXT_LINES
    bkr_topic_data_free(book,window->u.topic.page_id);
#else
    if ( window->u.topic.page_id == 0 )
	return;

    /* Free the chunk list if it exists */

    if ( window->u.topic.chunk_list != NULL )
    {
	int	cknum;
	for ( cknum = 0; cknum < window->u.topic.num_chunks; cknum++ )
	{
	    BMD_CHUNK  *chunk = &window->u.topic.chunk_list[cknum];

	    switch ( chunk->data_type )
	    {
	    	case BMD_CHUNK_FTEXT:
		    break;

	    	case BMD_CHUNK_RAGS:
	    	case BMD_CHUNK_RAGS_NO_FILL:
#ifndef NO_RAGS
		    if ( ( chunk->handle ) && ( chunk->handle != NODISPLAY ) )
		    	bkr_rags_close( chunk->handle );
#endif 
		    break;

	    	case BMD_CHUNK_IMAGE:
	    	case BMD_CHUNK_IMAGE75:		/* NOTE: Version 1.0 books ONLY */
		    if ( chunk->handle )
		    	bkr_image_close( chunk );
		    break;  
    	    }	/* end of switch */

	    if ( chunk->xpoint_vec != NULL )
	    {
		BKR_CFREE( chunk->xpoint_vec );
	    }
            if (chunk->segments) {
                BKR_FREE(chunk->segments);
            }
	    if ( chunk->region )
		XDestroyRegion( chunk->region );

	}   /* end for loop */

	BKR_CFREE( window->u.topic.chunk_list );
    	window->u.topic.num_chunks = 0;

    }	    /* end "if chunk_list" */

    /* Close the page, the bri routines keep track of usage,
     * and will only do a real close if this window is the last
     * user.
     */
    bri_page_close( book->book_id, window->u.topic.page_id );
#endif 
    window->u.topic.page_id = 0;
    window->u.topic.grop_pending = FALSE;

};  /* end of bkr_close_topic_data */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_close_topic_window
**
** 	Callback routine for closing a single Topic window.
**
**  	The "default" Standard Topic window is always cached when closed 
**  	and at least one Standard Topic window will be cached.
**
**  FORMAL PARAMETERS:
**
**  	widget	    	- id of the widget that caused the event.
**  	data	    	- generic user data.
**  	callback_data	- pointer to the callback data (unused).
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
**	Virtual memory is freed.
**
**--
**/
void
bkr_close_topic_window PARAM_NAMES((widget,data,callback_data))
    Widget		    	widget PARAM_SEP
    caddr_t 	    	    	data PARAM_SEP
    XmPushButtonCallbackStruct *callback_data PARAM_END	/* not used */

{
    BKR_WINDOW	*window;
    BKR_SHELL   *temp_shell;
	
    if (widget != NULL) {
        /* 
         *  Get the user data, if non-NULL then the callback is from a Topic popup 
         *  menu, otherwise its from a Topic window pulldown menu.
         */
        Arg	    	arglist[5];
        int	    	argcnt;
        argcnt = 0;
        SET_ARG( XmNuserData, &window );
        XtGetValues( widget, arglist, argcnt );
        if ( window == NULL ) {
            window = (BKR_WINDOW *) data;
        }
    } else {
        /*
         * No widget, called directly from bookreader, so get the
         * window from the user data.
         */
        window = (BKR_WINDOW *) data;
    }
    if (window) {

	temp_shell = window->shell;

        bkr_close_window(window,FALSE);

        /*  Attempt to free the shell if it doesn't belong to a client.
         */
        bkr_shell_free(temp_shell,FALSE);
    }
};  /* end of bkr_close_topic_window */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_close_window
**
** 	Routine for closing a single window.
**
**  	The "default" Standard Topic window is always cached when closed 
**  	and at least one Standard Topic window will be cached.
**
**  FORMAL PARAMETERS:
**
**      window - pointer to the BKR_WINDOW for the window being closed
**      unconditional_destroy - destroy the window regardless of whether
**               it belongs to the default shell or not
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
**	Virtual memory is freed.
**
**--
**/
void
bkr_close_window PARAM_NAMES((window,unconditional_destroy))
    BKR_WINDOW	*window PARAM_SEP
    Boolean unconditional_destroy PARAM_END

{
    BKR_SHELL *shell;
    Boolean   destroy_window;

    if (window == NULL) {
        return;
    }


    shell = window->shell;

    /* Determine if we can destroy the window after were finished closing
     * it.
     */
    if (unconditional_destroy) {

        /* Yes, by definition.
         */
        destroy_window = TRUE;

    } else if (shell->client || (shell == bkr_default_shell)) {

        /* In these cases we don't destroy the selection window or default
         * topic window because we cache them for later use.
         */
        if ((window == shell->selection) || (window == shell->default_topic)) {
            destroy_window = FALSE;
        } else {
            destroy_window = TRUE;
        }
    } else if ((window == shell->default_topic) && (shell->n_active_windows > 1)) {

        /* If other window are open, then don't destroy the default topic, since
         * we might reuse it soon.
         */
        destroy_window = FALSE;
    } else {
        destroy_window = TRUE;
    }

    /* If the window isn't active, then we already closed it once,
     * so there's no more to do.
     */
    if (window->active) {

        window->active = FALSE;
        shell->n_active_windows--;

        switch (window->type) {
            case BKR_SELECTION: {
                BKR_DIR_ENTRY *entry;
                int	    	  i;
                
                if (window->appl_shell_id) {
                    REMOVE_SELECTION_WINDOW(window);
                }

            
                /* Tell SVN to update the node for this book
                 */
                if ( ( bkr_library != NULL ) && ( shell->library_node != NULL ) ) {
                    bkr_library_update_node( shell->library_node, shell, CLOSE );
                }
                shell->library_node = NULL;
                
                /* First free the current selection hierarchy 
                 */
                bkr_selection_free_hierarchy( window );
                
                /* Zero pointers in case this selection window is re-used 
                 */
                window->u.selection.num_source_entries  = 0;
                window->u.selection.btn3down_entry_node = NULL;
                window->u.selection.num_selected        = 0;
                
                break;
            }
            case BKR_STANDARD_TOPIC:
            case BKR_FORMAL_TOPIC: {

                BKR_BACK_TOPIC  *back_topic;
                BKR_BACK_TOPIC  *next_back_topic;

                if (window->appl_shell_id != NULL) {
                    REMOVE_TOPIC_WINDOW( window ); 	
                }
                
                /* Free the back topics.
                 */
                back_topic = window->u.topic.back_topic;
                while ( back_topic != NULL ) {    	    
                    next_back_topic = back_topic->next;
                    BKR_FREE( back_topic );
                    back_topic = next_back_topic;
                }
                window->u.topic.back_topic = NULL;

                /* Free the data first
                 */
                bkr_close_topic_data( window );
            
                if (destroy_window == FALSE) {
                    
                    Arg	    	arglist[2];
                    int	    	argcnt;

                    XtSetSensitive( window->widgets[W_GOBACK_BUTTON], FALSE );
                    if ( window->widgets[W_VIEW_MENU] != NULL )
                    {
                        XtSetSensitive( window->widgets[W_GOBACK_ENTRY],  FALSE );
                        
                        /* Reset the state of the toggle buttons to the resource default 
                         */
                        if ( window->widgets[W_HOTSPOTS_ENTRY] != NULL )
                        {
                            argcnt = 0;
                            SET_ARG( XmNset, bkr_topic_resources.show_hot_spots );
                            XtSetValues(window->widgets[W_HOTSPOTS_ENTRY], 
                                        arglist, argcnt );
                        }
                        if ( window->widgets[W_EXTENSIONS_ENTRY] != NULL )
                        {
                            argcnt = 0;
                            SET_ARG( XmNset, bkr_topic_resources.show_extensions );
                            XtSetValues(window->widgets[W_EXTENSIONS_ENTRY], 
                                        arglist, argcnt );
                        }
                    }
    
                    window->u.topic.title    	    = NULL;
                    window->u.topic.outlined_chunk   = NULL;
                    window->u.topic.selected_chunk   = NULL;
                    window->u.topic.mb1_up_time	    = 0;
                    window->u.topic.mb1_is_down	    = FALSE;
                    window->u.topic.mb1_double_click = FALSE;
                    window->u.topic.show_hot_spots   = bkr_topic_resources.show_hot_spots;
                    window->u.topic.show_extensions  = bkr_topic_resources.show_extensions;
                    window->title_bar_name = NULL;
                    window->icon_name  	= NULL;

                }
                break;
            }
            default: {
                break;
            }
        }
    }
#ifdef COPY
    if(window == bkr_copy_ctx.window)
	bkr_copy_ctx_init();
#endif
    if (destroy_window) {

#ifdef SEARCH
	/* since this window's really dying, destroy the search context
	 */
	bkr_delete_search_context(window);

#endif /* SEARCH */

        /* Remove the shell's connection to the window
         */
        if (window == shell->selection) {

            shell->selection = NULL;

        } else if (window == shell->default_topic) {

            shell->default_topic = NULL;

        } else {

            /* Remove it form the list of other topics
             */
            BKR_WINDOW_PTR  *window_list_ptr;
            
            window_list_ptr = &shell->other_topics;
            while (*window_list_ptr) {
                BKR_WINDOW *ptr = *window_list_ptr;
                if (ptr == window) {
                    *window_list_ptr = window->u.topic.sibling;
                    break;
                }
                window_list_ptr = &ptr->u.topic.sibling;
            }
        }

#ifdef MEMEX
        bmi_delete_ui(window);
#endif 

        bkr_window_free_pixmaps(window);

        BKR_FREE(window);
    }
};  /* end of bkr_close_window */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_close_quit_callback
**
** 	Quit callback for closing down the Bookreader application.
**
**  FORMAL PARAMETERS:
**
**	none
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
**	Virtual memory is freed.
**
**--
**/
void
bkr_close_quit_callback()
{
    bkr_default_shell = NULL;

    while (bkr_all_shells) {
        bkr_close_shell(bkr_all_shells,TRUE);
    }
#ifdef MEMEX
    bmi_delete_composite_network();
    bmi_delete_ui(bkr_library);
#endif 

    /* Destroy the Library window */

    if ( bkr_library != NULL )
    {
    	if ( bkr_library->appl_shell_id != NULL )
    	    XtDestroyWidget( bkr_library->appl_shell_id );
    	BKR_FREE( bkr_library );
    }

    /* Finally destroy the application context; this will flush the event 
     * queue and free all remaining toolkit resources associated with the 
     * context.
     */

    XtDestroyApplicationContext( XtDisplayToApplicationContext( bkr_display ) );

    exit(EXIT_SUCCESS);

}  	/* end of bkr_close_quit_callback */



