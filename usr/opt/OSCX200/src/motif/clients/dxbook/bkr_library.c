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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_LIBRARY.C*/
/* *15   17-NOV-1992 22:51:13 BALLENGER "Special handling for Space and Return."*/
/* *14   31-OCT-1992 01:06:05 KLUM "search book titles fix"*/
/* *13   30-OCT-1992 12:43:42 BALLENGER "Use PARAM_NAMES"*/
/* *12   29-OCT-1992 16:13:35 KLUM "QAR 298"*/
/* *11    5-OCT-1992 11:27:32 KLUM "rename print widget id constants"*/
/* *10   13-AUG-1992 15:09:46 GOSSELIN "updating with necessary A/OSF changes"*/
/* *9    20-JUL-1992 13:48:58 BALLENGER "Character cell support"*/
/* *8     9-JUN-1992 09:57:01 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *7    24-MAR-1992 10:27:56 KLUM "fix print book grayinh"*/
/* *6     5-MAR-1992 14:25:13 PARMENTER "adding simple search"*/
/* *5     3-MAR-1992 17:00:16 KARDON "UCXed"*/
/* *4     1-NOV-1991 13:05:15 BALLENGER "reintegrate  memex support"*/
/* *3    17-SEP-1991 20:26:54 BALLENGER "include function prototype headers"*/
/* *2    17-SEP-1991 20:21:29 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:39:43 PARMENTER "Library window routines for interacting with SVN"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_LIBRARY.C*/
#ifndef VMS
 /*
#else
#module BKR_LIBRARY "V03-0004"
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
**	Library window routines for interacting with SVN.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     4-Jan-1990
**
**  MODIFICATION HISTORY:
**
**  V03-0004 DLB0004		David L Ballenger	17-May-1991
**           Allocate entry tags for SVN using BKR_CALLOC rather than
**           on the stack, this allows large shelf files and fixes a bug
**           that caused crashes for shelves of more than 100 entries.
**
**           Also, free child nodes and close shelf files when closing
**           a shelf entry in the library window.  This allows users
**           to pick up changes in shelves simply by closing and reopening
**           shelf entries, rather than having to restart Bookreader.
**
**  V03-0003 DLB0003		David L Ballenger	02-May-1991
**           Fix nil pointer dereference in bkr_library_free_node().
**
**  V03-0002 DLB0002		David L Ballenger	30-Apr-1991
**           Fix problems with handling of surrogate objects in the
**           library window.
**
**  V03-0001 DLB0001		David L Ballenger	06-Feb-1991
**	     Fix include of <DXm/DXmSvn.h>.
**
**--
**/


/*
 * INCLUDE FILES
 */
#
#include "br_common_defs.h"  /* common BR #defines */
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_globals.h"      /* BR external variables declared here */
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "bkr_library.h"     /* function prototypes for .c module */
#include "bkr_close.h"       /* close routines */
#include "bkr_cursor.h"      /* cursor routines */
#include "bkr_error.h"       /* error display routines */
#include "bkr_fetch.h"       /* resource fetching routines */
#include "bkr_selection_open.h" /* selection window open routines */
#ifdef MEMEX
#include "bmi_library.h"
#include "bmi_surrogate.h"
#include "bmi_user_interface.h"
#endif
#include <stdio.h>

#define IS_EXPANDED(svn,entry) (DXmSvnGetEntryNumber(svn,(XtPointer)((entry)->children)) != 0)
 
#define	BELL_VOLUME	4

/*
 * FORWARD ROUTINES
 */

static void  	    add_shell_to_list
                         PROTOTYPE((BKR_NODE *node,
                                    BKR_SHELL *shell));
static void  	    add_toplevel_entries();
static void  	    close_node();
static void  	    expand_all();
static BKR_NODE_PTR  library_shelf_initialize();
static void  	    open_node();
static unsigned	    open_shelf();
static void  	    remove_shell_from_list
                        PROTOTYPE((BKR_NODE *node, 
                                   BKR_SHELL *shell));
static void  	    set_pulldown_menu_buttons();
static void  	    setup_shelf_book_icons();


/*
 * FORWARD DEFINITIONS 
 */

static Widget    	    svn_widget = NULL;
static int  	    	    bkr_num_source_entries = 0;
static char 	    	    num_books_open_in_library = 0;

static int  	    	    lib_component_1_width = 0;
static int  	    	    lib_component_1_height = 0;
static Pixmap 	    	    shelf_pixmap = 0;
static Pixmap 	    	    book_pixmap = 0;
static XmString             expandable_entry = NULL;
static XmString             expanded_entry = NULL;
static XmString             non_expandable_entry = NULL;
static XmFontList           charcell_svn_font_list = NULL;

#ifdef TRACE_SVN
static char	    	    debug_svn_callbacks = FALSE;
static char 	    	    debug_expand_all = FALSE;
#endif 


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_library_attach_to_source
**
** 	Saves the SVN widget id so local routines can access it.
**
**  FORMAL PARAMETERS:
**
**  	widget	    - id of the widget.
**  	unused_tag  - user data.
**  	data	    - callback data.
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
bkr_library_attach_to_source PARAM_NAMES((widget,unused_tag,data))
    Widget	    	 widget PARAM_SEP
    int 	    	 unused_tag PARAM_SEP
    DXmSvnCallbackStruct *data PARAM_END

{

    /* Save the Svn widget id so local routines can access it */

    if ( svn_widget == NULL )
    	svn_widget = widget;

};  /* end of bkr_library_attach_to_source */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  	bkr_library_collapse_expand
**
** 	Callback routine that handles:
**
**  	Collapse Entry - Collapse All - Expand Entry -
**  	    Expand All (removed for V3 because of cycles in hierarchy)
**
**  FORMAL PARAMETERS:
**
**  	widget	- id of the widget that caused the callback.
**  	tag 	- user data.
**  	reason	- callback data.
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
bkr_library_collapse_expand PARAM_NAMES((widget,tag,reason))
    Widget	    	   widget PARAM_SEP
    int 	    	   *tag PARAM_SEP
    XmAnyCallbackStruct    *reason PARAM_END

{
    int	    	 index = (int) *tag;
    BKR_NODE_PTR node;
    BKR_NODE_PTR node_to_position = NULL;
    int	    	 i;
    int	    	 entry_number;
    XtPointer	 entry_tag;

    bkr_cursor_display_wait( ON );
    DXmSvnDisableDisplay( svn_widget );

    switch ( index )
    {
    	case W_COLLAPSE_ENTRY :
    	    for ( i = 0; i < bkr_library->u.library.num_selected; i++ )
    	    {
    	    	entry_tag = (XtPointer)bkr_library->u.library.selected_entry_tags[i];
    	    	node = (BKR_NODE_PTR) entry_tag;
    	    	entry_number = (int) DXmSvnGetEntryNumber( svn_widget, 
		    entry_tag );
    	    	if ( NODE_IS_EXPANDABLE( node ) )
    	    	    close_node( node, entry_number );

#ifdef TRACE_SVN
    	    	if ( debug_svn_callbacks )
    	    	    printf( "collapse_entry=%3d, \"%s\" entry_tag=%X \n", 
    	    	    	entry_number, node->title, node );
#endif
    	    }

    	    /* Get the selected entries again just in case they have changed
    	     */
    	    {
    	    	DXmSvnCallbackStruct	reason;
    	    	int 	    	    	tag;

    	    	bkr_library_transitions_done( svn_widget, tag, &reason );
    	    }
    	    break;

    	case W_COLLAPSE_ALL_ENTRY :
            if (bkr_library->u.library.root_of_tree) {
                for ( i = 1, node = bkr_library->u.library.root_of_tree->u.shelf.children;
                     ( node != NULL );  i++, node = node->sibling 
                     ) {
                    close_node( node, i );
                }
            }

    	    /* Scroll the first Toplevel shelf to the Top of the SVN display .
             */
    	    if ( bkr_library->u.library.root_of_tree != NULL )
    	    {
                node_to_position = bkr_library->u.library.root_of_tree->u.shelf.children ;
    	    }

    	    /* Get the selected entries again just in case they have changed
    	     */
    	    {
    	    	DXmSvnCallbackStruct	reason;
    	    	int 	    	    	tag;

    	    	bkr_library_transitions_done( svn_widget, tag, &reason );
    	    }
    	    break;

    	case W_EXPAND_ENTRY :
    	    for ( i = 0; i < bkr_library->u.library.num_selected; i++ ) {
    	    	entry_tag = (XtPointer)bkr_library->u.library.selected_entry_tags[i];
    	    	node = (BKR_NODE_PTR) entry_tag;
    	    	entry_number = DXmSvnGetEntryNumber( svn_widget, entry_tag );
    	    	if ( NODE_IS_EXPANDABLE( node ) ) {
                    open_node(node,entry_number);
                }
#ifdef TRACE_SVN
    	    	if ( debug_svn_callbacks )
    	    	    printf( "expand_entry=%3d, \"%s\" entry_tag=%X \n", 
    	    	    	entry_number, node->title, node );
#endif
    	    }
            node_to_position = (BKR_NODE_PTR)bkr_library->u.library.selected_entry_tags[0];
    	    break;

    	case W_EXPAND_ALL_ENTRY :
    	    if ( bkr_library->u.library.root_of_tree != NULL ) {
                node_to_position = bkr_library->u.library.root_of_tree->u.shelf.children;
                if (node_to_position) {
                    expand_all( node_to_position, 1 );
                }
    	    }

    	    break;
    }

    if (node_to_position) {
        entry_number = DXmSvnGetEntryNumber( svn_widget, (XtPointer)node_to_position );
        DXmSvnPositionDisplay( svn_widget, entry_number, DXmSvnKpositionTop );
    }
    DXmSvnEnableDisplay( svn_widget );
    bkr_cursor_display_wait( OFF );

};  /* end of bkr_library_collapse_expand */

bkr_library_open_entry(entry)
    BKR_NODE_PTR entry;
{
    open_node(entry,DXmSvnGetEntryNumber( svn_widget, (XtPointer)entry));
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  	bkr_library_detach_from_source
**
** 	Callback routine when an SVN widget is being destroyed.
**
**  FORMAL PARAMETERS:
**
**  	widget	    - id of the widget that caused the callback.
**  	unused_tag  - user data.
**  	reason	    - callback data.
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
bkr_library_detach_from_source PARAM_NAMES((widget,unused_tag,data))
    Widget	    	 widget PARAM_SEP
    int 	    	 unused_tag PARAM_SEP
    DXmSvnCallbackStruct *data PARAM_END

{

    bkr_library_free_hierarchy( &bkr_library->u.library.root_of_tree->u.shelf.children, FALSE );

};  /* end of bkr_library_detach_from_source */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  	bkr_library_double_click
**
** 	Callback routine for handling double clicks in the Library window.
**
**  FORMAL PARAMETERS:
**
**  	widget	    - id of the widget that caused the callback.
**  	unused_tag  - user data.
**  	reason	    - callback data.
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
bkr_library_double_click PARAM_NAMES((widget,unused_tag,data))
    Widget	    	 widget PARAM_SEP
    int 	    	 unused_tag PARAM_SEP
    DXmSvnCallbackStruct *data PARAM_END

{
    BKR_NODE	*node;
    BKR_SHELL   *tmp_shell;

    bkr_cursor_display_wait( ON );
    bkr_error_set_parent_shell( bkr_library->appl_shell_id );

    node = (BKR_NODE *) data->entry_tag;
    if ( node == NULL ) {
    	node = (BKR_NODE *) DXmSvnGetEntryTag( widget, (XtPointer)data->entry_number );
    	if ( node == NULL ) {
            bkr_error_set_parent_shell( NULL );
            bkr_cursor_display_wait( OFF );
            BKR_FLUSH_EVENTS;
            return;
        }
    }

#ifdef TRACE_SVN
    if ( debug_svn_callbacks ) 
    	printf( "bkr_library_double_click -- entry_number=%d, entry_tag=%x \n",
    	    data->entry_number, data->entry_tag );
#endif

#ifdef MEMEX
    if ((data->component_number == 2) && node->highlight) {
        bmi_visit(bkr_library);
        bkr_error_set_parent_shell( NULL );
        bkr_cursor_display_wait( OFF );
        BKR_FLUSH_EVENTS;
        return;
    }
#endif
    if ( node->entry_type == BKR_BOOK_FILE ) {

	/* User double clicked on node a second time
         */
	if ( node->u.book.shells != NULL ) {
	    /* Close the "default" shell if this is the default node
             * or close the first in the list.
             */
            tmp_shell = node->u.book.shells;
	    if (bkr_library->u.library.default_node == node) {
                while (tmp_shell && (tmp_shell != bkr_default_shell)) {
                    tmp_shell = tmp_shell->library_shells;
                }
                if (tmp_shell == NULL) {
                    tmp_shell = node->u.book.shells;
                }
            }
            if (tmp_shell) {
                /* Update the node and close the shell
                 */
                bkr_library_update_node( node, tmp_shell, CLOSE );
                bkr_close_shell(tmp_shell,FALSE);
            }
	} else {   	/* No book's open for this node */

            if (bkr_default_shell) {
                bkr_close_shell(bkr_default_shell,FALSE);
	        if (bkr_library->u.library.default_node) {
                    bkr_library_update_node(bkr_library->u.library.default_node, 
                                            bkr_default_shell, 
                                            CLOSE );
                }
            }
	    tmp_shell = bkr_selection_open_book(NULL,
                                                node->parent->u.shelf.id, 
    	    	    	    	    	    	node->entry_id, 
                                                TRUE,
					    	node );
	    if ( tmp_shell == NULL ) {
    	    	bkr_cursor_display_wait( OFF );
    	    	BKR_FLUSH_EVENTS;
    	    	bkr_error_set_parent_shell( NULL );
    	    	return;
    	    }

    	    /* Opened successfully, update the node
             */
	    bkr_library_update_node( node, tmp_shell, OPEN );

    	}   	/* end of if node->u.book.shells */
    } else if ( node->entry_type == BKR_SHELF_FILE ) {
    	if ( NODE_IS_EXPANDABLE( node ) ) {
    	    if ( node->u.shelf.opened ) {
    	    	close_node( node, data->entry_number );
    	    } else {
    	    	open_node( node, data->entry_number );
                DXmSvnPositionDisplay(svn_widget,
                                      data->entry_number,
                                      DXmSvnKpositionTop );
            }
        }
    }

    bkr_error_set_parent_shell( NULL );
    bkr_cursor_display_wait( OFF );
    BKR_FLUSH_EVENTS;

};  /* end of bkr_library_double_click */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	bkr_library_free_hierarchy
**
** 	Frees a Library hierarchy.
**
**  FORMAL PARAMETERS:
**
**  	node_ptr     - address of a pointer to a hierarchy to free.
**  	close_books  - Boolean: whether or not to close open books.
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
bkr_library_free_hierarchy PARAM_NAMES((node_ptr,close_books))
    BKR_NODE_PTR    *node_ptr PARAM_SEP
    Boolean 	    close_books PARAM_END

{
    BKR_NODE	*node;

    for ( node = *node_ptr; ( node != NULL ); node = node->sibling )
    {
    	bkr_library_free_node( node, close_books );
    }
    *node_ptr = NULL;

    if ( bkr_library->u.library.selected_entry_tags != NULL )
    {
    	BKR_CFREE( bkr_library->u.library.selected_entry_tags );
    	bkr_library->u.library.num_selected = 0;
    }

};  /* end of bkr_library_free_hierarchy */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_library_update_entry
**
** 	Forces an entry within the SVN display to be refetched.
**  	When the MEMEX highlighting changes on an entry this routine
**  	should be called to update the entry.
**
**  FORMAL PARAMETERS:
**
**  	entry	    - pointer to the entry to update.
**
**  IMPLICIT INPUTS:
**
**	svn_widget  - svn widget of the library
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
**	DXmSvnGetEntry callback will be called to update the entry.
**
**--
**/
void
bkr_library_update_entry PARAM_NAMES((entry))
    BKR_NODE *entry PARAM_END
{
    int	    entry_number;

    entry_number = DXmSvnGetEntryNumber( svn_widget, (XtPointer) entry );
    if ( entry_number > 0 )
    {
    	DXmSvnDisableDisplay( svn_widget );
    	DXmSvnInvalidateEntry( svn_widget, entry_number );
    	DXmSvnEnableDisplay( svn_widget );
    }

};  /* end of bkr_library_update_entry */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  	bkr_library_get_entry
**
** 	Callback routine for initializing the data of an entry needed by SVN.
**
**  FORMAL PARAMETERS:
**
**  	widget	    - id of the widget.
**  	unused_tag  - user data.
**  	data	    - callback data.
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
bkr_library_get_entry PARAM_NAMES((widget,unused_tag,data))
    Widget	    	 widget PARAM_SEP
    int 	    	 unused_tag PARAM_SEP
    DXmSvnCallbackStruct *data PARAM_END

{
    BKR_NODE	*node = (BKR_NODE *) data->entry_tag;
    Pixmap  	entry_pixmap;
    XmString	cs_title;
    long	byte_cnt, stat;
    int	    	    num_components = BKR_SOURCE_NUM_COMPONENTS ;
    int	    	    highlight_icon_x_pos = 0;

#ifdef TRACE_SVN
    if ( debug_svn_callbacks ) 
    	printf( "bkr_library_get_entry -- entry_number=%d, entry_tag=%x \n",
    	    data->entry_number, data->entry_tag );
#endif

    /* Setup the icon pixmaps */

    if ( lib_component_1_width == 0 )
    	setup_shelf_book_icons( widget );

    /*
     * Set the entry information that both children and parent nodes
     * have in common.
     */
#ifdef MEMEX
    if (node->highlight) {
        num_components++;
    }
#endif

    DXmSvnSetEntryNumComponents( widget, data->entry_number, num_components );

    /* Add ONLY shelf file entries to the index window */

    if ( ( node->entry_type == BKR_SHELF_FILE ) && ( node->level <= 2 ) )
    	DXmSvnSetEntryIndexWindow( widget, data->entry_number, TRUE );

    if (bkrplus_g_charcell_display) 
    {
        XmString entry_type_string;

        if (node->entry_type == BKR_SHELF_FILE) 
        {
            if (node->u.shelf.opened) 
            {
                entry_type_string = expanded_entry;
            }
            else 
            {
                entry_type_string = expandable_entry;
            }
        }
        else 
        {
            entry_type_string = non_expandable_entry;
        }

        DXmSvnSetComponentText( widget, 
                               data->entry_number, 
                               1,
                               0,        /* x position     	*/
                               0,        /* y position      	*/
                               entry_type_string,
                               charcell_svn_font_list );

    }
    else 
    {
        switch ( node->entry_type )
        {
            case BKR_BOOK_FILE : {
                entry_pixmap = book_pixmap;
                break;
    	    }
            case BKR_SHELF_FILE : {
                entry_pixmap = shelf_pixmap;
                break;
            }
        }
        DXmSvnSetComponentPixmap( widget, 
                                 data->entry_number, 
                                 1, 	    	    /* component number */
                                 0, 	    	    /* x position   	*/
                                 0, 	    	    /* y position   	*/
                                 entry_pixmap, 
                                 lib_component_1_width, lib_component_1_height );
    }
#ifdef MEMEX
    /* Tell SVN about the highlighted entry pixmap */

    if ( node->highlight )
    {
        highlight_icon_x_pos = lib_component_1_width + 4;

        bmi_insert_highlight_icon(widget,
                                  data->entry_number, 
                                  2,                  /* Second component */
                                  highlight_icon_x_pos
                                 );
    }
#endif
    cs_title = DXmCvtFCtoCS( node->title, &byte_cnt, &stat );
    DXmSvnSetComponentText( widget, 
    	data->entry_number, 
    	num_components,
    	lib_component_1_width+4+highlight_icon_x_pos, /* x position     	*/
    	0,  	    	    	                 /* y position      	*/
    	cs_title,   	    	                 /* entry text      	*/
    	charcell_svn_font_list );

    COMPOUND_STRING_FREE( cs_title );

    /* Highlight the entry if it was previously highlighted */

    if ( ( node->entry_type == BKR_BOOK_FILE ) && ( node->u.book.shells != NULL ) )
    	bkr_library_highlight_entry( widget, NULL, data->entry_number, ON );

};  /* end of bkr_library_get_entry */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  	bkr_library_highlight_entry
**
** 	Highlights/Unhighlights an entry given either the button 
**  	press event or position within the SVN structure.
**
**  FORMAL PARAMETERS:
**
**  	widget	    	- id of the SVN widget containing the entry.
**  	xbutton_event	- pointer to xbutton event used to map the position.
**  	position_number	- position number with the SVN display.
**  	highlight   	- whether to highlight or unhighlight an entry.
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
int 
bkr_library_highlight_entry PARAM_NAMES((widget,xbutton_event,
                                         position_number,highlight))

    Widget  	    	    widget PARAM_SEP
    XButtonPressedEvent	    *xbutton_event PARAM_SEP
    int	    	    	    position_number PARAM_SEP
    Boolean 	    	    highlight PARAM_END

{
    int	    	entry_number;
    int	    	entry_tag;

    /* If the position_number is 0 then we need to map the position first */

    entry_number = (int) position_number;
    if ( entry_number == 0 )
    {
    	DXmSvnMapPosition( widget, xbutton_event->x, xbutton_event->y,
    	    &entry_number, NULL, &entry_tag );
    }

    DXmSvnDisableDisplay( widget );
    if ( highlight )
    {
    	DXmSvnHighlightEntry( widget, entry_number );
    	DXmSvnShowHighlighting( widget );
    }
    else
    	DXmSvnClearHighlight( widget, entry_number );
    DXmSvnEnableDisplay( widget );

    return( entry_number );

};  /* end of bkr_library_highlight_entry */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  	bkr_library_number_books_open
**
**  	Returns the number of open books in the Library window.
**
**  FORMAL PARAMETERS:
**
**	none
**
**  IMPLICIT INPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	Returns:    number of open books in the Library window.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
int
bkr_library_number_books_open(VOID_PARAM)
{
    return 0;
};  /* end of bkr_library_number_books_open */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  	bkr_library_open_close_node
**
** 	Callback routine that opens books in either the
**  	default or in a new Selection window.
**
**  FORMAL PARAMETERS:
**
**  	widget	- id of the widget that caused the event.
**  	tag 	- user data.
**  	data	- callback data.
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
bkr_library_open_close_node PARAM_NAMES((widget,tag,data))
    Widget		    widget PARAM_SEP
    int	    	    	    *tag PARAM_SEP
    XmAnyCallbackStruct     *data PARAM_END

{
    Widget  	*widget_array = (Widget *) bkr_library->widgets;
    int	    	index = *tag;
    BKR_NODE	*node = NULL;
    BKR_NODE	*tmp_node = NULL;
    int	    	entry_number;
    BKR_SHELL   *tmp_shell;
    Boolean 	open_in_default = FALSE;
    int	    	i;

    if ( ( widget != widget_array[W_OPEN_BOOK_IN_DEFAULT] ) 
    	 && ( widget != widget_array[W_OPEN_BOOK_IN_NEW] ) )
    {
    	node = bkr_library->u.library.btn3down_entry_node;    	    
    }
    else
    {
    	if ( bkr_library->u.library.selected_entry_tags == NULL )
    	    return;

    	/* Open ONLY the first Book in the selected list */

    	for ( i = 0; i < bkr_library->u.library.num_selected; i++ )
    	{
    	    tmp_node = (BKR_NODE_PTR) bkr_library->u.library.selected_entry_tags[i];
    	    if ( tmp_node->entry_type == BKR_BOOK_FILE )
    	    {
    	    	node = tmp_node;
    	    	break;
    	    }
    	}
    }
    if ( node == NULL )
    	return;

    bkr_cursor_display_wait( ON );
    DXmSvnDisableDisplay( svn_widget );
    bkr_error_set_parent_shell( bkr_library->appl_shell_id );

    entry_number = (int) DXmSvnGetEntryNumber( svn_widget, (XtPointer) node );
    switch ( index )
    {
    	case W_OPEN_BOOK_IN_DEFAULT :
    	case W_OPEN_IN_DEFAULT_POPUP_ENTRY : {
            open_in_default = TRUE;
            if (bkr_default_shell) {
                bkr_close_shell(bkr_default_shell,FALSE);
	        if (bkr_library->u.library.default_node) {
                    bkr_library_update_node(bkr_library->u.library.default_node, 
                                            bkr_default_shell, 
                                            CLOSE );
                }
            }
            /* Fall through to open the book.
             */
        }
    	case W_OPEN_BOOK_IN_NEW :
    	case W_OPEN_IN_NEW_POPUP_ENTRY : {
    	    tmp_shell = bkr_selection_open_book(NULL, 
                                                node->parent->u.shelf.id, 
    	    	    	    	    	    	node->entry_id, 
    	    	    	    	    	    	open_in_default,
                                                node );
    	    if ( tmp_shell == NULL ) {
    	    	break;
    	    }

    	    /* Opened successfully, update the node
             */
    	    bkr_library_update_node( node, tmp_shell, OPEN );

    	    break;  /* end case OPEN_IN_NEW or OPEN_IN_DEFAULT */
        }
    	case W_OPEN_SHELF_POPUP_ENTRY :
    	    if ( NODE_IS_EXPANDABLE( node ) ) {
    	    	open_node( node, entry_number );
                DXmSvnPositionDisplay(svn_widget,entry_number,DXmSvnKpositionTop );
            }

    	    break;

    	case W_CLOSE_SHELF_POPUP_ENTRY :
    	    if ( NODE_IS_EXPANDABLE( node ) )
    	    	close_node( node, entry_number );

    	    /* Get the selected entries again just in case they have changed
    	     */
    	    {
    	    	DXmSvnCallbackStruct	reason;
    	    	int 	    	    	tag;

    	    	bkr_library_transitions_done( svn_widget, tag, &reason );
    	    }
    	    break;
    }

    /* Clear the entry_node that btn3down was over */

    bkr_library->u.library.btn3down_entry_node = NULL;

    bkr_error_set_parent_shell( NULL );
    DXmSvnEnableDisplay( svn_widget );
    bkr_cursor_display_wait( OFF );
    BKR_FLUSH_EVENTS;

};  /* end of bkr_library_open_close_node */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_library_open_new_library
**
** 	Opens a hierarchy within an SVN widget given the toplevel library name.
**
**  FORMAL PARAMETERS:
**
**	new_library_name - pointer to the Library to open.
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
**	Returns:    1 - if the library could be opened,
**  	    	    0 - for failure.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
unsigned
bkr_library_open_new_library PARAM_NAMES((new_library_name))
    char    *new_library_name PARAM_END
{
    BKR_NODE	    	*new_root_of_tree;
    int	    	    	num_entries;
    XmAnyCallbackStruct	reason;
    int	    	    	function = W_COLLAPSE_ALL_ENTRY;
    BKR_NODE	    	*node;

    bkr_cursor_display_wait( ON );

    /* Attempt to open the new Library shelf */

    new_root_of_tree = library_shelf_initialize( new_library_name) ;
    if ( new_root_of_tree == NULL  )
    {
    	bkr_cursor_display_wait( OFF );
    	BKR_FLUSH_EVENTS;
    	return ( 0 );
    }

    /* Tell SVN to delete all the toplevel node's children */

    bkr_library_collapse_expand( svn_widget, &function, &reason );

    /* 
     *  Tell SVN to delete the toplevel entries 
     */

    DXmSvnDisableDisplay( svn_widget );
    DXmSvnDeleteEntries( svn_widget, 0, bkr_num_source_entries );
    DXmSvnEnableDisplay( svn_widget );

    /* Disable pulldown menu push buttons */

    set_pulldown_menu_buttons( FALSE, FALSE );

    /* Free the current library hierarachy, if there is one, and close the Toplevel shelf. 
     */
#ifdef MEMEX
    if (bkr_library->u.library.root_of_tree) {
        bmi_delete_shelf_surrogates(bkr_library->u.library.root_of_tree);
    }
#endif 
    bkr_library_free_hierarchy( &bkr_library->u.library.root_of_tree, FALSE );
    if (bkr_library->u.library.root_of_tree && (bkr_library->u.library.root_of_tree->u.shelf.id != NULL)) {
    	bri_shelf_close( bkr_library->u.library.root_of_tree->u.shelf.id );
    }

    /* Save the new hierarchy, tell SVN to add and display the new entries */
    bkr_library->u.library.root_of_tree = new_root_of_tree;
    bkr_num_source_entries = new_root_of_tree->u.shelf.num_children;

    DXmSvnDisableDisplay( svn_widget );
    add_toplevel_entries( bkr_library->u.library.root_of_tree,bkr_num_source_entries );
#ifdef MEMEX
    new_root_of_tree->surrogates = NULL;
    new_root_of_tree->highlight = FALSE;
    new_root_of_tree->u.shelf.path = "";
    bkr_library->u.library.shelves_queried = FALSE;
    bkr_library->u.library.name = (char *)BKR_MALLOC(strlen(new_library_name)+1);
    strcpy(bkr_library->u.library.name,new_library_name);
    bmi_create_shelf_context(bkr_library,new_root_of_tree);
#endif 
    if ( bkr_library_resources.shelf_to_open == FIRST_SHELF )
    	open_node( bkr_library->u.library.root_of_tree->u.shelf.children, 1 );
    else if ( bkr_library_resources.shelf_to_open == ALL_SHELVES )
    {
    	BKR_NODE    *node;
    	int 	    entry_number;

    	for (node = bkr_library->u.library.root_of_tree->u.shelf.children ; 
             node != NULL ;
             node = node->sibling 
            ) {
    	    entry_number = (int) DXmSvnGetEntryNumber( svn_widget, 
    	    	    	    	    	(XtPointer) node );
    	    open_node( node, entry_number );
    	}
    }
    DXmSvnEnableDisplay( svn_widget );

    bkr_cursor_display_wait( OFF );
    BKR_FLUSH_EVENTS;

    return ( 1 );

};  /* end of bkr_library_open_new_library */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  	bkr_library_transitions_done
**
** 	Callback routine for storing the currently selected entries.
**
**  FORMAL PARAMETERS:
**
**  	widget	    - id of the widget that caused the event.
**  	unused_tag  - user data.
**  	data	    - callback data.
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
bkr_library_transitions_done PARAM_NAMES((widget,unused_tag,data))
    Widget	    	 widget PARAM_SEP
    int 	    	 unused_tag PARAM_SEP
    DXmSvnCallbackStruct *data PARAM_END

{
    BKR_NODE	*node;
    int	    	num_selected;
    int	    	*entry_numbers;
    XtPointer	*entry_tags;
    int	    	i;
    Boolean 	selected_book = FALSE;
    Boolean 	selected_shelf = FALSE;

    /* Discard the old entry tag arrays and zero the count */

    if ( bkr_library->u.library.selected_entry_tags != NULL )
    {
    	BKR_CFREE( bkr_library->u.library.selected_entry_tags );
    	bkr_library->u.library.num_selected = 0;
    }

    /* Get the number of selected entries from SVN */

    num_selected = DXmSvnGetNumSelections( svn_widget );
    if ( num_selected == 0 )
    {
    	set_pulldown_menu_buttons( FALSE, FALSE );
    	return;
    }

    /* 
     * Allocate some memory to hold the new selected entry tags array
     * and the unused entry numbers array.
     */

    entry_numbers = (int *) BKR_CALLOC( num_selected, sizeof( int ) );
    entry_tags = (XtPointer *) BKR_CALLOC( num_selected, sizeof( XtPointer ) );

    DXmSvnGetSelections( 
    	svn_widget, 
    	entry_numbers,	/* entry numbers; not used but we can't pass a NULL */
    	NULL,	    	/* component numbers; not used */
    	entry_tags, 	/* entry tags array 	       */
    	num_selected );	/* size of arrays being passed */

#ifdef TRACE_SVN
    if ( debug_svn_callbacks )
    {
    	int i;
    	for ( i = 0; i < num_selected; i++ )
    	    printf( "selected_entry_number=%3d, \"%s\" entry_tag=%X \n", 
    	    	entry_numbers[i], ((BKR_NODE_PTR) entry_tags[i])->title, 
    	    	entry_tags[i] );
    }
#endif

    /* 
     * Save the entry tags array and the number of selected entries and
     * free the entry_numbers array because we don't need it anymore.
     */

    bkr_library->u.library.selected_entry_tags = entry_tags;
    bkr_library->u.library.num_selected = num_selected;
    BKR_CFREE( entry_numbers );

    /* Decide which push buttons should be set sensitive */

    for ( i = 0; i < bkr_library->u.library.num_selected; i++ )
    {
    	node = (BKR_NODE_PTR) bkr_library->u.library.selected_entry_tags[i];
    	if ( node->entry_type == BKR_BOOK_FILE )
    	    selected_book = TRUE;
    	if ( node->entry_type == BKR_SHELF_FILE )
    	    selected_shelf = TRUE;
    }
    set_pulldown_menu_buttons( selected_book, selected_shelf );

};  /* end of bkr_library_transitions_done */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  	bkr_library_update_node
**
** 	Updates a book nodes shell list as the book the node points to
**  	is either opened or closed.
**
**  FORMAL PARAMETERS:
**
**  	node	 - pointer to the node to update.
**  	shell_id - pointer to the shell to add/delete from the node.
**  	open	 - Boolean: whether or not a book node is being added or deleted.
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
bkr_library_update_node PARAM_NAMES((node,shell,open))
    BKR_NODE *node PARAM_SEP
    BKR_SHELL *shell PARAM_SEP
    Boolean   open PARAM_END
{
    int position;

    position = DXmSvnGetEntryNumber( svn_widget, (XtPointer) node );
    if ( open )
    {
    	if ( position > 0 )
    	    bkr_library_highlight_entry( svn_widget, NULL, position, ON );
    	add_shell_to_list( node, shell );
    }
    else
    {
    	if ( node->entry_type == BKR_BOOK_FILE )
    	{
    	    /* Clear the shell */

    	    if ( shell != NULL )
    	    {
    	    	remove_shell_from_list( node, shell );
    	    	if ( ( bkr_library->u.library.default_node == node )
    	    	     && ( bkr_default_shell == shell ) )
    	    	{
    	    	    bkr_library->u.library.default_node = NULL;
    	    	}
    	    }

    	    /* Only clear the highlight, if the BOOK isn't open in some shell */

    	    if ( ( position > 0 ) && ( node->u.book.shells == NULL ) )
    	    	bkr_library_highlight_entry( svn_widget, NULL, position, OFF );
    	}
    	else if ( node->entry_type == BKR_SHELF_FILE )
    	{
    	    if ( position > 0 )
    	    	bkr_library_highlight_entry( svn_widget, NULL, position, OFF );
    	}
    }

};  /* end of bkr_library_update_node */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_library_keyboard_actions
**
** 	Key input action handler routine for the library window
**
**  FORMAL PARAMETERS:
**
**	svn_primary_window - id of the widget that caused the event,
**	xbutton_event	   - X event associated with the Btn1Up event
**	params	    	   - address of a list of strings passed as arguments
**	num_params  	   - address of the number of arguments passed
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
bkr_library_keyboard_actions PARAM_NAMES((svn_primary_window,event,params,num_params))
    Widget		    svn_primary_window PARAM_SEP
    XKeyEvent  	            *event PARAM_SEP
    String  	    	    *params PARAM_SEP
    Cardinal	    	    *num_params PARAM_END

{
    BKR_WINDOW 	            *window = NULL;
    Widget                  svn_widget;

    static int beepcount = 0;

    /* Get the window from the userData of the SVN widget.
     */
    XtVaGetValues(svn_primary_window,XmNuserData,&window,NULL);

    if ((window == NULL) 
        || (window->type != BKR_LIBRARY)
        || (window->widgets[W_SVN] == NULL)
        ) {
    	return;
    }
    
    if (window->u.library.num_selected == 0) 
    {
        bkr_error_simple_msg(window,"NOTHING_SELECTED");
    	return;
    }
    
    svn_widget = window->widgets[W_SVN];

    if (*num_params < 1) 
    {
        char                    buffer[50];
        XComposeStatus          compose;
        KeySym                  keysym;

        switch (event->type) 
        {
            case KeyPress: 
            {
                fprintf(stderr,"KeyPress:   ");
                break;
            }
            case KeyRelease: 
            {
                fprintf(stderr,"KeyRelease: ");
                break;
            }
            default:
            {
                return;
            }
        }
        (void)XLookupString(event,buffer,50,&keysym,&compose);
        fprintf(stderr,"%04x, %s\n",keysym,buffer);
    }
    else if (strcmp(params[0],"CollapseExpand") == 0 )
    {
        BKR_NODE  *entry;
        BKR_NODE  *entry_to_position = NULL;
        int	       i;
        XtPointer      entry_tag;
        int	       entry_number;

        if (!bkrplus_g_charcell_display) 
        {
            bkr_cursor_display_wait( ON );
        }
        DXmSvnDisableDisplay(svn_widget);
        
        for ( i = 0; i < window->u.library.num_selected; i++ ) 
        {
            entry_tag = (XtPointer)window->u.library.selected_entry_tags[i];
            entry = (BKR_NODE *) entry_tag;
            if ( entry->entry_type == BKR_SHELF_FILE ) 
            {
                entry_number = DXmSvnGetEntryNumber(svn_widget,entry_tag);
                
                if (entry->u.shelf.opened ) 
                {
                    close_node(entry,entry_number);
                }
                else 
                {
                    open_node(entry,entry_number);
                    if (entry_to_position == NULL) 
                    {
                        entry_to_position = entry;
                    }
                }
                
                beepcount = 0;
            }
            else 
            {
                /* if entry is a directory with no target, beep three times
                 * before putting out an error message	     
                 */
                XBell( bkr_display, BELL_VOLUME );
                beepcount++;
                
                if ( beepcount == 2 ) 
                { 	
                    if(bkrplus_g_charcell_display) 
                    {
                        bkr_error_simple_msg(window,"CC_ENTRY_NOT_SHELF");
                    }
                    else 
                    {
                        bkr_error_simple_msg(window,"MF_ENTRY_NOT_SHELF");
                    }
                    beepcount = 0;
                }
            }
        }
        if ( entry_to_position) 
        {
            DXmSvnPositionDisplay(svn_widget,
                                  DXmSvnGetEntryNumber(svn_widget,(XtPointer)entry_to_position), 
                                  DXmSvnKpositionTop);
        }

        DXmSvnEnableDisplay(svn_widget);
        
        if (!bkrplus_g_charcell_display) 
        {
            bkr_cursor_display_wait( OFF );
        }
    }
    else if (strcmp(params[0],"OpenClose") == 0 )
    {
        BKR_NODE	*node;
        BKR_SHELL   *tmp_shell;

        bkr_error_set_parent_shell( bkr_library->appl_shell_id );
        
        node = (BKR_NODE *)window->u.library.selected_entry_tags[0];
        if ( node->entry_type == BKR_BOOK_FILE ) 
        {
            if (!bkrplus_g_charcell_display) 
            {
                bkr_cursor_display_wait( ON );
            }
            if ( node->u.book.shells != NULL ) 
            {
                /* Close the "default" shell if this is the default node
                 * or close the first in the list.
                 */
                tmp_shell = node->u.book.shells;
                if (bkr_library->u.library.default_node == node) 
                {
                    while (tmp_shell && (tmp_shell != bkr_default_shell)) 
                    {
                        tmp_shell = tmp_shell->library_shells;
                    }
                    if (tmp_shell == NULL) 
                    {
                        tmp_shell = node->u.book.shells;
                    }
                }
                if (tmp_shell) 
                {
                    /* Update the node and close the shell
                     */
                    bkr_library_update_node( node, tmp_shell, CLOSE );
                    bkr_close_shell(tmp_shell,FALSE);
                }
            } else {   	/* No book's open for this node */

                if (bkr_default_shell) 
                {
                    bkr_close_shell(bkr_default_shell,FALSE);
                    if (bkr_library->u.library.default_node) 
                    {
                        bkr_library_update_node(bkr_library->u.library.default_node, 
                                                bkr_default_shell, 
                                                CLOSE );
                    }
                }
                tmp_shell = bkr_selection_open_book(NULL,
                                                    node->parent->u.shelf.id, 
                                                    node->entry_id, 
                                                    TRUE,
                                                    node );
                if ( tmp_shell == NULL ) 
                {
                    bkr_cursor_display_wait( OFF );
                    BKR_FLUSH_EVENTS;
                    bkr_error_set_parent_shell( NULL );
                    return;
                }

                /* Opened successfully, update the node
                 */
                bkr_library_update_node( node, tmp_shell, OPEN );
                
            }   	/* end of if node->u.book.shells */
            if (!bkrplus_g_charcell_display) 
            {
                bkr_cursor_display_wait( OFF );
            }
        } 
        else if ( node->entry_type == BKR_SHELF_FILE ) 
        {
            /* if entry is a directory with no target, beep three times
             * before putting out an error message	     
             */
            XBell( bkr_display, BELL_VOLUME );
            beepcount++;
            
            if ( beepcount == 2 ) 
            { 	
                if(bkrplus_g_charcell_display) 
                {
                    bkr_error_simple_msg(window,"CC_ENTRY_NOT_BOOK");
                }
                else 
                {
                    bkr_error_simple_msg(window,"MF_ENTRY_NOT_BOOK");
                }
                beepcount = 0;
            }
        }
    }
};  /* end of bkr_library_keyboard_actions */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	add_shell_to_list
**
** 	Adds a new entry to a nodes shell list of open books.
**
**  FORMAL PARAMETERS:
**
**  	node - pointer to the node containing the list to modify.
**  	id   - pointer to the shell to add to the list.
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
static void
add_shell_to_list PARAM_NAMES((node,shell))
    BKR_NODE *node PARAM_SEP
    BKR_SHELL *shell PARAM_END
{
    BKR_SHELL_PTR   *shell_ptr;


    /* Put the shell at the end of the list of shells open for this node.
     */
    shell_ptr = &node->u.book.shells;
    while (*shell_ptr != NULL) {
        shell_ptr = &((*shell_ptr)->library_shells);
    }
    *shell_ptr = shell;
    shell->library_shells = NULL;

    /* Update the library book count
     */
    num_books_open_in_library++;

};  /* end of add_shell_to_list */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	add_toplevel_entries
**
** 	Adds the toplevel nodes to the SVN display.  One toplevel
**  	node represents one directory in the DECW$BOOK search list.
**
**  FORMAL PARAMETERS:
**
**  	new_root_node - pointer to the root of the new hierarchy to add.
**  	num_entries   - number of entries to add to the SVN display.
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
static void
add_toplevel_entries( new_root_node, num_entries )
    BKR_NODE	*new_root_node;
    int	    	num_entries;
{
    XtPointer	*entry_tags;
    int	    	entry_num;
    BKR_NODE	*node;

    /* Build tag array from Toplevel entries and tell SVN to add them 
     */
    entry_tags = (XtPointer *)BKR_CALLOC(num_entries,sizeof(XtPointer));
    if (entry_tags == NULL) {
        return;
    }


    node = new_root_node->u.shelf.children;
    for ( entry_num = 0; entry_num < num_entries; entry_num++ )
    {
    	entry_tags[entry_num] = (XtPointer) node;
    	node = node->sibling;
    }
    DXmSvnAddEntries( 
    	svn_widget,
    	BKR_TOPLEVEL_NODE,  	    	/* insert after entry number  */
    	num_entries,	    	    	/* number of entries to add   */
    	new_root_node->level + 1,    	/* level of entry   	      */
    	entry_tags, 	    	    	/* entry tag array  	      */
    	FALSE ); 	    	    	/* add entry to index window? */

    BKR_CFREE(entry_tags);
};  /* end of add_toplevel_entries */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	close_node
**
** 	Recursive routine to remove entries from the SVN display.
**
**  FORMAL PARAMETERS:
**
**  	node	    - pointer to the node to start removing entries.
**  	node_number - number of the node within the SVN display.
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
static void
close_node( node, node_number )
    BKR_NODE	*node;
    int	    	node_number;
{
    BKR_NODE	*child_node;
    int	    	i;

    /* Node is NOT expandable */

    if (NODE_IS_EXPANDABLE( node ) && node->u.shelf.opened ) {

        /* Recursively call close_node to close each child node
         */
        child_node = node->u.shelf.children;
        for ( i = 1; (i <= node->u.shelf.num_children) && child_node ; i++ ) {
            BKR_NODE	*sibling = child_node->sibling;
            
            /* Close any open child shelf entries.
             */
            if (NODE_IS_EXPANDABLE( node ) && node->u.shelf.opened ) {
                close_node( child_node, node_number );
            }
            /* Now free the child node because the next time the
             * parent node is opened we'll reread the shelf in case
             * there have been any changes.
             */
            bkr_library_free_node(child_node,FALSE);
            child_node = sibling;
        }

        /* Tell SVN to remove its children
         */
        DXmSvnDeleteEntries( svn_widget, node_number, node->u.shelf.num_children );

        /* Mark the node closed and update the global number of source
         * entries and the 
         */
        bkr_num_source_entries -= node->u.shelf.num_children;
        node->u.shelf.opened = FALSE;
        node->u.shelf.children = NULL;
        node->u.shelf.num_children = 0;
        bri_shelf_close( node->u.shelf.id );
        node->u.shelf.id = NULL;
    }

    bkr_library_update_entry(node);

};  /* end of close_node */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	expand_all
**
** 	Recursive routine to add all entries to the SVN display.
**
**  FORMAL PARAMETERS:
**
**  	node	    - pointer to the node to start adding entries.
**  	node_number - number of the node within the SVN display.
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
static void
expand_all( node, node_number )
    BKR_NODE	*node;
    int	    	node_number;
{
    BKR_NODE	*child_node;
    int	    	i;
    XtPointer	tag;

#ifdef TRACE_SVN
    if ( debug_expand_all )
    	print_node_short( node, node_number );
#endif

    /* Node is NOT expandable */

    if ( ! NODE_IS_EXPANDABLE( node ) )
    	return;

    /* Expand the node now! */

    open_node( node, node_number );

    /* recursively call expand_all to open each child node */

    child_node = node->u.shelf.children;
    for ( i = 1; i <= node->u.shelf.num_children; i++ )
    {
    	tag = (XtPointer) child_node;
    	expand_all( child_node, DXmSvnGetEntryNumber( svn_widget, tag ) );
    	child_node = child_node->sibling;
    }

}   /* end of expand_all */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  	bkr_library_free_node
**
** 	Recursive routine to free the data for nodes within a hierarchy.
**
**  FORMAL PARAMETERS:
**
**  	node	    	 - pointer to the node to start freeing entries.
**  	close_open_books - Boolean: whether to close open books before
**  	    	    	    freeing a particular nodes data.
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
extern void
bkr_library_free_node PARAM_NAMES(( node, close_open_books ))
    BKR_NODE	*node PARAM_SEP
    Boolean 	close_open_books PARAM_END
{
    BKR_NODE	*child_node;
    int	    	i;

#ifdef TRACE_SVN
    if ( debug_expand_all )
    	print_node_short( node, -1 );
#endif

    /* Recursively call bkr_library_free_node to close each child node */

    child_node = node->u.shelf.children;
    while (child_node) {
        BKR_NODE *sibling = child_node->sibling;
    	bkr_library_free_node( child_node, close_open_books );
    	child_node = sibling;
    }

    /* Close the shelf if it was open */

    if ( node->entry_type == BKR_SHELF_FILE ) {
    	if ( node->u.shelf.id != NULL )
    	{
    	    bri_shelf_close( node->u.shelf.id );
    	    node->u.shelf.id = NULL;
    	}
#ifdef MEMEX
        if (node->parent && node->u.shelf.path) {
            BKR_FREE(node->u.shelf.path);
        }
#endif 
    }

    /* Free the NODE's shell list
     */
    if ( node->entry_type == BKR_BOOK_FILE )
    {
    	BKR_SHELL *shell = node->u.book.shells;
    	
        if (node == bkr_library->u.library.default_node) {
            bkr_library->u.library.default_node = NULL;
        }

        while (shell) {
            BKR_SHELL *next_shell = shell->library_shells;
            shell->library_shells = NULL;
            if (close_open_books) {
		bkr_close_selection_window( NULL, shell->selection, NULL );
            }
            shell = next_shell;
        }
    }

#ifdef MEMEX
    if (node->surrogates) {
        bmi_clear_surrogate_list(&node->surrogates,1);
    }
#endif 

    /* Finally free the node itself */

    BKR_FREE( node );

};  /* end of bkr_library_free_node */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	library_shelf_initialize
**
** 	Opens a Toplevel library shelf and initializes its children.
**
**  FORMAL PARAMETERS:
**
**  	library_name - name of the shelf file to open.
**  	library_shelf_id_rtn - address to return the id of the shelf opened.
**  	new_shelf    - address to return pointer to new toplevel hierarchy.
**  	num_entries  - address to return number of entries in shelf.
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
static BKR_NODE_PTR
library_shelf_initialize( library_name)
    char    	    *library_name;
{
    BKR_NODE	*new_library;
    static char	*default_title = NULL;

    new_library = (BKR_NODE *) BKR_MALLOC( sizeof( BKR_NODE ) );
    if (new_library == NULL) {
        return NULL;
    }
    memset( new_library, 0, sizeof( BKR_NODE ) );

    /* Attempt to open the Library shelf file */

    if (default_title == NULL)
    {
	default_title = (char *) bkr_fetch_literal( "s_library_default_title", 
	    ASCIZ );
	if (default_title == NULL)
	    default_title = "";
    }
    bkr_error_set_parent_shell( bkr_library->appl_shell_id );
    new_library->u.shelf.id = bri_shelf_openlib( library_name, 
                                                &new_library->u.shelf.num_children, 
                                                default_title );
    if ( new_library->u.shelf.id == NULL )
    {
        BKR_FREE(new_library);
    	bkr_error_set_parent_shell( NULL );
    	return( NULL );
    }

    /* Finish initializing library node.
     */
    new_library->level = ((BKR_TOPLEVEL_NODE) - 1);
    new_library->title = default_title;
    new_library->entry_type = BKR_SHELF_FILE;

    /* Toplevel library opened successfully, so initialize its entries */

    if ( ! bkr_library_sibling_initialize( new_library) )
    {
        BKR_FREE(new_library);
    	bkr_error_set_parent_shell( NULL );
    	return ( NULL );
    }

    new_library->u.shelf.opened = TRUE;

    return( new_library );

};  /* end of library_shelf_initialize */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	open_node
**
** 	Add the entries for the given shelf to the SVN display.
**
**  FORMAL PARAMETERS:
**
**  	node	    - pointer to the shelf node to open.
**  	node_number - node number within the SVN display.
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
static void
open_node( node, node_number )
    BKR_NODE	*node;
    int	    	node_number;
{
    BKR_NODE	*child_node;
    int	    	entry_num;
    XtPointer	*entry_tags;
    unsigned	new_level = node->level + 1;

    /* Node is NOT expandable */

    if ( ! NODE_IS_EXPANDABLE( node ) )
    	return;

    /* Node is already opened */

    if ( node->u.shelf.opened )
    	return;

    /* Open the Shelf file and initialize its children */

    if ( node->u.shelf.id == NULL )	    	/* siblings not yet initialized */
    	if ( ! open_shelf( node ) )
    	    return; 	    	    	/* the open failed */

    /* Success! Node is now open */

    node->u.shelf.opened = TRUE;

    /* Build tag array for children and add the entries 
     */
    entry_tags = (XtPointer *)BKR_CALLOC(node->u.shelf.num_children,
		    sizeof(XtPointer));
    if (entry_tags == NULL) {
        return;
    }

    child_node = node->u.shelf.children;
    for ( entry_num = 0; entry_num < node->u.shelf.num_children; entry_num++ )
    {
#ifdef TRACE_SVN
    	if ( debug_svn_callbacks )
    	    print_node_short( child_node, bkr_num_source_entries + entry_num );
#endif
    	entry_tags[entry_num] = (XtPointer) child_node;
    	child_node = child_node->sibling;
    }
    DXmSvnAddEntries( 
    	svn_widget,
    	node_number,	    	    /* Insert after this node 	     */
    	node->u.shelf.num_children, /* Number of entries to add	     */
    	new_level,    	    	    /* Level of new entries 	     */
    	entry_tags, 	    	    /* Entry tag array	    	     */
    	FALSE );    	    	    /* Display entry in index window */

    BKR_CFREE(entry_tags);

    /* Update the global number of source entries */

    bkr_num_source_entries += node->u.shelf.num_children;

    /* Now position the node at the top so we can see as many of its entries 
     * as possible.
     */
    bkr_library_update_entry(node);

};  /* end of open_node */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  	open_shelf
**
**  	Opens the shelf and initializes the shelf entry hierarchy.
**
**  FORMAL PARAMETERS:
**
**  	node - pointer to the node to open.
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
static unsigned
open_shelf( node )
    BKR_NODE	*node;
{
    unsigned	status;

    /* Open the shelf file */

    node->u.shelf.id = bri_shelf_open(node->parent->u.shelf.id,
                                      node->entry_id, 	            /* entry id in parent shelf */
                                      &node->u.shelf.num_children );
    if ( node->u.shelf.id == NULL )
    	return( 0 );

    /* Initialize the shelf entries */

    status = bkr_library_sibling_initialize( node );
    if ( ! status ) {
    	return( 0 );
    }
    /* Successful open, so update fields */

#ifdef MEMEX
    bmi_create_shelf_context(bkr_library,node);
#endif

    return( 1 );

};  /* end of open_shelf */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	remove_shell_from_list
**
** 	Removes an entry from a nodes shell list of open books.
**
**  FORMAL PARAMETERS:
**
**  	node - pointer to the node containing the list to modify.
**  	id   - pointer to the shell to remove from the list.
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
static void
remove_shell_from_list PARAM_NAMES((node,shell))
    BKR_NODE  *node PARAM_SEP
    BKR_SHELL *shell PARAM_END
{
    BKR_SHELL_PTR *shell_ptr;

    if ( node->u.book.shells == NULL )
    	return;

    /* Loop until we find the shell */

    shell_ptr = &node->u.book.shells;
    while (*shell_ptr) {
        if (*shell_ptr == shell) {
            *shell_ptr = shell->library_shells;
            shell->library_shells = NULL;
            break;
        }
        shell_ptr = &((*shell_ptr)->library_shells);
    }

    /* Update the library book count
     */
    num_books_open_in_library--;

};  /* end of remove_shell_from_list */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  	set_pulldown_menu_buttons
**
** 	Sets the sensitivity of the push buttons in the pulldown menus.
**
**  FORMAL PARAMETERS:
**
**  	selected_book  - sensitivity of the book push buttons.
**  	selected_shelf - sensitivity of the shelf push buttons.
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
static void
set_pulldown_menu_buttons( selected_book, selected_shelf )
    Boolean 	selected_book;
    Boolean 	selected_shelf;
{
#ifdef SEARCH
    Boolean	search_books_sensitive = 
			( bkr_library->u.library.num_selected > 0 );
#endif /* SEARCH */

    if ( bkr_library->widgets[W_FILE_MENU] != NULL ) 
    {
    	XtSetSensitive( bkr_library->widgets[W_OPEN_BOOK_IN_DEFAULT], selected_book );
    	XtSetSensitive( bkr_library->widgets[W_OPEN_BOOK_IN_NEW],     selected_book );
    }
    if ( bkr_library->widgets[W_VIEW_MENU] != NULL )
    {
    	XtSetSensitive( bkr_library->widgets[W_COLLAPSE_ENTRY], selected_shelf );
    	XtSetSensitive( bkr_library->widgets[W_EXPAND_ENTRY],   selected_shelf );
    }
#ifdef SEARCH
    if ( bkr_library->widgets[W_SEARCH_MENU] != NULL )
    {
        XtSetSensitive( 
		bkr_library->widgets[W_SEARCH_BOOKS_ENTRY], 
		search_books_sensitive );
        XtSetSensitive( 
		bkr_library->widgets[W_SEARCH_LIBRARY_WINDOW_ENTRY], 
		search_books_sensitive );
    }
#endif /* SEARCH */

#ifdef PRINT
    /*--- set "Print Book" button to gray if nothing selected ---*/
    if ( bkr_library->widgets[W_FILE_MENU] )
    {
        XtSetSensitive( 
		bkr_library->widgets[W_PR_PRBOOK_BUTTON], 
		bkr_library->u.library.num_selected );
    }
#endif /* PRINT */
#ifdef FOO
    /*--- commented out until the widget values are resolved ---*/
#endif /* FOO */
};  /* end of set_pulldown_menu_buttons */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  	setup_shelf_book_icons
**
** 	Creates the shelf and book pixmaps.
**
**  FORMAL PARAMETERS:
**
**  	widget - id of the widget to use for the foreground/background colors
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	Initializes:	book_pixmap 	    shelf_pixmap 
**  	    	    	lib_component_1_width    lib_component_1_height
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
static void
setup_shelf_book_icons( widget )
    Widget widget;
{
    int	    screen = XDefaultScreen( bkr_display );
    Pixel   background_pixel;
    Pixel   foreground_pixel;
    Arg     arglist[5];
    int	    argcnt;
    caddr_t data;

    /* If we've already done this, then return. */

    if ( lib_component_1_width != 0 ) 
    	return;

    if (bkrplus_g_charcell_display) 
    {
        expandable_entry = (XmString)bkr_fetch_literal("ExpandableEntry",
                                                       MrmRtypeCString );
        expanded_entry = (XmString)bkr_fetch_literal("ExpandedEntry",
                                                     MrmRtypeCString );
        non_expandable_entry = (XmString)bkr_fetch_literal("NonExpandableEntry",
                                                           MrmRtypeCString );

        charcell_svn_font_list = XmFontListCreate(bkr_default_font,
                                                  XmSTRING_DEFAULT_CHARSET);

        lib_component_1_width = 2 * bkr_default_space_width;
    }
    else 
    {
        /*
         *  Get the foreground/background colors of Svn and match them 
         *  for the icons.
         */
        
        argcnt = 0;
        SET_ARG( XmNforeground, &foreground_pixel );
        SET_ARG( XmNbackground, &background_pixel );
        XtGetValues( widget, arglist, argcnt );
        
        /* Fetch the pixmaps */
        
        shelf_pixmap = bkr_fetch_icon_literal( "SHELF_ENTRY_ICON",
                                              foreground_pixel, background_pixel );
        
        book_pixmap = bkr_fetch_icon_literal( "BOOK_ENTRY_ICON",
                                             foreground_pixel, background_pixel );
        
        data = bkr_fetch_literal( "LIBRARY_PIXMAP_WIDTH", MrmRtypeInteger );
        lib_component_1_width = *( (int *) data );
        data = bkr_fetch_literal( "LIBRARY_PIXMAP_HEIGHT", MrmRtypeInteger );
        lib_component_1_height = *( (int *) data );
    }
};  /* end of setup_shelf_book_icons */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  	bkr_library_sibling_initialize
**
** 	Initializes the entries given the id of the shelf.
**
**  FORMAL PARAMETERS:
**
**  	parent          - pointer to the parent shelf node
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
**	Returnes:   status of the initialization.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
extern unsigned
bkr_library_sibling_initialize PARAM_NAMES(( parent ))
    BKR_NODE_PTR    parent PARAM_END
{
    BKR_NODE	*new_node   = NULL;
    BKR_NODE	*prev_node  = NULL;
    int	    	entry_num;
    int         new_level = parent->level + 1;
    int         num_siblings = parent->u.shelf.num_children;

    for ( entry_num = 1; entry_num <= num_siblings; entry_num++ )
    {
    	new_node = (BKR_NODE *) BKR_MALLOC( sizeof( BKR_NODE ) );
    	memset( new_node, 0, sizeof( BKR_NODE ) );
    	if ( entry_num == 1 )
    	{
    	    parent->u.shelf.children = new_node;
    	    prev_node = new_node;
    	}
    	else
    	{
    	    prev_node->sibling = new_node;
    	    prev_node = new_node;
    	}
    	new_node->entry_type = BKR_NULL_TYPE;
    	bri_shelf_entry (
	    parent->u.shelf.id,     /* shelf id from bri_shelf_open   */
	    entry_num,	    	    /* entry id; MUST start at 1      */
	    &new_node->entry_type,  /* 1 = BOOK; 2 = SHELF  	      */
	    NULL,                   /* entry file name      	      */
	    NULL,   	    	    /* width of entry 	    	      */
	    NULL,   	    	    /* height of entry 	    	      */
	    NULL,   	    	    /* formatted entry data address   */
	    NULL,   	    	    /* length of formatted entry data */
    	    NULL,   	    	    /* type of formatted entry data   */
	    &new_node->title );

    	new_node->level     	    = new_level;
    	new_node->sibling   	    = NULL;
        new_node->parent            = parent;
    	new_node->entry_id  	    = entry_num;
    	switch ( new_node->entry_type )
    	{
    	    case BKR_SHELF_FILE :
                new_node->u.shelf.opened = FALSE;   /* shelf not open yet */
    	    	new_node->u.shelf.id = NULL;  	    
    	    	new_node->u.shelf.children = NULL;
                new_node->u.shelf.num_children = 0; /* always zero, until entry is opened */
                new_node->u.shelf.path = NULL;
    	    	break;

    	    case BKR_BOOK_FILE :
    	    	new_node->u.book.shells	 = NULL;
    	    	break;
    	}
#ifdef MEMEX
        new_node->surrogates = NULL;
        new_node->highlight = FALSE;
#endif 
    } 	    /* end for loop */

    return( 1 );

};  /* end bkr_library_sibling_initialize */


#ifdef TRACE_SVN
print_node_short( node_ptr, node_number )
    BKR_NODE_PTR node_ptr;
    int	    	 node_number;
{
    print_indent( node_ptr->level );
    if ( node_number > -1 )
    	printf( "node_number=%d ", node_number );

    printf( "title=\"%s\", entry_tag=%X \n", node_ptr->title, node_ptr );

};

print_node( node_ptr )
    BKR_NODE_PTR node_ptr;
{
    print_indent( node_ptr->level );
      printf( "level        = %d \n", node_ptr->level );
    print_indent( node_ptr->level );
      printf( "title        = %s \n", node_ptr->title );
    print_indent( node_ptr->level );
      printf( "entry_type   = %d \n", node_ptr->entry_type );
    print_indent( node_ptr->level );
      printf( "shelf_id     = %x \n", node_ptr->u.shelf.id );
    print_indent( node_ptr->level );
      printf( "opened       = %d \n", node_ptr->u.shelf.opened );
    print_indent( node_ptr->level );
      printf( "num_children = %d \n", node_ptr->u.shelf.num_children );

    printf( "\n" );
};


print_indent( level )
    int level;
{
    int i;
    for ( i = 0; i < level * 4; i++ )	 /* 4 spaces indent per level */
    	printf( " " );	/* <--- one space */
};
#endif /* TRACE_SVN */



