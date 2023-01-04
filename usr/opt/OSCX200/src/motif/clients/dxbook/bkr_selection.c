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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_SELECTION.C*/
/* *17   17-NOV-1992 22:51:28 BALLENGER "Special handling for Space and Return."*/
/* *16    8-NOV-1992 19:21:36 BALLENGER "Use work in progress box instead of wait cursor on character cell."*/
/* *15   20-OCT-1992 15:43:46 KLUM "conditionalized ENTRY_TARGET_ERROR msg for char-cell"*/
/* *14    5-OCT-1992 11:29:56 KLUM "rename print widget id constants"*/
/* *13   13-AUG-1992 15:10:08 GOSSELIN "updating with necessary A/OSF changes"*/
/* *12   20-JUL-1992 13:49:39 BALLENGER "Character cell support"*/
/* *11   22-JUN-1992 19:24:11 BALLENGER "Don't call bkr_menu_update_view_pulldown() when opening a directory."*/
/* *10    9-JUN-1992 09:58:24 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *9     3-APR-1992 17:16:45 FITZELL "decworld hooks"*/
/* *8    13-MAR-1992 15:19:01 KLUM "forgot to UCX"*/
/* *7    13-MAR-1992 11:23:54 PARMENTER "grays out 'search topics' when neccesary"*/
/* *6    13-MAR-1992 11:02:11 KLUM "gray ""Print Topic"" button if no selection"*/
/* *5     5-FEB-1992 10:38:00 FITZELL "fixes accvio when no directories are found"*/
/* *4     7-JAN-1992 16:50:44 PARMENTER "adding CBR/Search"*/
/* *3     1-NOV-1991 12:58:30 BALLENGER "Reintegrate  memex support"*/
/* *2    17-SEP-1991 21:23:06 BALLENGER "change use of major and minor in version number to avoid conflict with sys/types.h*/
/*definition on ULTRIX"*/
/* *1    16-SEP-1991 12:40:24 PARMENTER "SVN callbacks for the Selection window"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_SELECTION.C*/
#ifndef VMS
 /*
#else
#module BKR_SELECTION "V03-0005"
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
**	SVN callback routines for the Selection window.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     26-Jun-1990
**
**  MODIFICATION HISTORY:
**
**  V03-1000    MJF	Michael Fitzell			31-JAN-1992
**		Need to report back to BKR that a directory can't be opened
**		it was being dropped in add_entries
**
**  V03-0005	DLB0005		David L Ballenger	22-May-1991
**              Fix problems when double clicking on a directory entry
**              with the LinkWorks highlight icon.
**
**  V03-0004	DLB0004		David L Ballenger	30-Apr-1991
**              Fix problems with select destination when following links
**              and with doing a visit when double clicking on the LinkWorks
**              highlight icon.
**
**  V03-0003	DLB0003		David L Ballenger	11-Apr-1991
**		Clean up handling of surrogate object highlight so
**              common code can be used for the Navigation and Library
**              windows.
**
**  V03-0002	DLB0002		David L Ballenger	07-Apr-1991
**		Fix problems with links in the navigation window.
**
**  V03-0001	DLB0001		David L Ballenger	06-Feb-1991
**		Fix include of <DXm/DXmSvn.h>.
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
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "bkr_selection.h"   /* function prototypes for .c module */
#include "bkr_cursor.h"      /* function prototypes for .c module */
#include "bkr_directory.h"   /* function prototypes for .c module */
#include "bkr_error.h"       /* function prototypes for .c module */
#include "bkr_fetch.h"       /* function prototypes for .c module */
#include "bkr_menu.h"        /* function prototypes for .c module */
#include "bkr_object.h"      /* function prototypes for .c module */
#include "bkr_selection_create.h"   /* function prototypes for .c module */
#include "bkr_window.h"      /* function prototypes for .c module */
#include "bri_dir.h"
#ifdef MEMEX
#include "bmi_directory.h"
#include "bmi_user_interface.h"
#endif 
#include <stdio.h>
#include <DXm/DECspecific.h>
#ifdef DECWORLD
#include "bkr_decworld.h"
#endif
 
#define	BELL_VOLUME	4

#define IS_EXPANDED(svn,entry) (DXmSvnGetEntryNumber(svn,(XtPointer)((entry)->children)) != 0)

/*
 * LOCAL ROUTINES
 */


unsigned int 	    	add_entries_to_display
    PROTOTYPE((Widget  	     svn_widget,
               BKR_DIR_ENTRY *entry,
               int	     entry_number,
               BKR_WINDOW     *window
               ));
static void  	    	expand_to_lowest_level
    PROTOTYPE((BKR_WINDOW     *window,
               BKR_DIR_ENTRY *entry,
               int	     entry_number
               ));
static void  	    	fetch_icon_pixmaps
    PROTOTYPE((Widget  svn_widget
               ));
static void  	    	remove_entries_from_display
    PROTOTYPE((Widget  	     svn_widget,
               BKR_DIR_ENTRY *entry,
               int	     entry_number,
               BKR_WINDOW     *window
               ));
static void  	    	set_pulldown_menu_buttons
    PROTOTYPE((BKR_WINDOW *window
               ));
static void
open_entry_target PROTOTYPE((BKR_WINDOW *window));

#ifdef DEBUG
static void  	    	print_dir_node();
#endif /* DEBUG */


/*
 * FORWARD DEFINITIONS 
 */

static Pixmap	    	    expandable_icon = 0;
static Pixmap	    	    single_target_expandable_icon = 0;
static int  	    	    pixmap_width = 0;
static int  	    	    pixmap_height = 0;
static XmString             expandable_entry = NULL;
static XmString             expanded_entry = NULL;
static XmString             non_expandable_entry = NULL;
static XmFontList           charcell_svn_font_list = NULL;

static Widget	    	    *popup_menu_widgets = NULL;


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  	bkr_selection_collapse_expand
**
** 	Callback routine for handling collapse and expand operations
**  	from either the VIEW pulldown menu or the popup menu.
**
**  	Collapse Entry - Collapse All - Expand Entry - Expand All
**
**  FORMAL PARAMETERS:
**
**  	widget	    - id of the push button widget.
**  	data	    - user data.
**  	reason	    - push button callback data.
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
bkr_selection_collapse_expand PARAM_NAMES((widget,data,reason))
    Widget	    	    widget PARAM_SEP
    caddr_t 	    	    data PARAM_SEP
    XmAnyCallbackStruct     *reason PARAM_END /* Not used */

{
    BKR_WINDOW	            *window = NULL;
    BKR_DIR_ENTRY   	    *entry;
    BKR_DIR_ENTRY   	    *entry_to_position = NULL;
    int	    	    	    i;
    XtPointer	    	    entry_tag;
    int	    	    	    entry_number;
    Arg	    	    	    arglist[5];
    int	    	    	    argcnt;
    Boolean 	    	    selection_popup = FALSE;

    if (!bkrplus_g_charcell_display) 
    {
        bkr_cursor_display_wait( ON );
    }
    /* 
     *  Get the user data, if non-NULL then the callback is from the Selection
     *  popup menu, otherwise its from one of the Selection window pulldown menus.
     */

    if ( widget != NULL ) {
    	argcnt = 0;
    	SET_ARG( XmNuserData, &window );
    	XtGetValues( widget, arglist, argcnt );
    }

    if ( window != NULL )
    	selection_popup = TRUE;
    else
    	window = (BKR_WINDOW *) data;


    DXmSvnDisableDisplay( window->widgets[W_SVN] );

    if ( ( widget == window->widgets[W_EXPAND_ALL_ENTRY] )
    	 || ( widget == window->widgets[W_EXPAND_ENTRY] ) 
    	 || ( widget == window->widgets[W_COLLAPSE_ENTRY] ) 
        ){
    	for ( i = 0; i < window->u.selection.num_selected; i++ ) {
    	    entry_tag = (XtPointer)window->u.selection.selected_entry_tags[i];
    	    entry = (BKR_DIR_ENTRY *) entry_tag;
    	    entry_number = DXmSvnGetEntryNumber( window->widgets[W_SVN], 
			    entry_tag );
    	    if ( widget == window->widgets[W_EXPAND_ALL_ENTRY] ) {
    	    	expand_to_lowest_level( window, entry, entry_number );

    	    } else if ( ENTRY_IS_EXPANDABLE( entry ) ) {

    	    	if ( widget == window->widgets[W_EXPAND_ENTRY] ) 
    	    	    (void)add_entries_to_display( window->widgets[W_SVN], entry, 
    	    	    	    	    	    entry_number, window );
    	    	else
    	    	    remove_entries_from_display( window->widgets[W_SVN], entry, 
    	    	    	    	    	    	 entry_number, window );
    	    }
    	}

        if ( widget != window->widgets[W_COLLAPSE_ENTRY] ) {
            entry_to_position = (BKR_DIR_ENTRY *)window->u.selection.selected_entry_tags[0];
        }

    } else if ( widget == window->widgets[W_COLLAPSE_ALL_ENTRY] ) {
    	XmAnyCallbackStruct 	reason;

    	for ( i = 1, entry = window->shell->book->directories; entry != NULL;
    	    	i++, entry = entry->sibling )
    	    remove_entries_from_display( window->widgets[W_SVN], entry, 
    	    	    	    	    	 i, window );

    	/* Scroll the first directory to the Top of the SVN display */
    	if ( window->shell->book->directories != NULL )
    	    bkr_selection_position_entry( window->widgets[W_SVN], 
    	    	    	    	    	   window->shell->book->directories, 
    	    	    	    	    	   POSITION_TOP );
    } else if ( selection_popup ) {
    	entry = window->u.selection.btn3down_entry_node;
    	entry_number = DXmSvnGetEntryNumber( window->widgets[W_SVN], 
			(XtPointer) entry );
    	if ( entry != NULL ) {
            if ( IS_EXPANDED(window->widgets[W_SVN],entry) ) {
                remove_entries_from_display( window->widgets[W_SVN], entry, 
                                            entry_number, window );
            } else {
                (void)add_entries_to_display(window->widgets[W_SVN],entry,entry_number,window);
                entry_to_position = entry;
            }
            window->u.selection.btn3down_entry_node = NULL;
        }
    }

    /*  Update the selected entry tags just in case the selections have changed.
     */
    {
    	DXmSvnCallbackStruct    data;

    	bkr_selection_transitions_done(window->widgets[W_SVN], window, &data );
    }
    if ( entry_to_position) {
        bkr_selection_position_entry(window->widgets[W_SVN],entry_to_position,POSITION_TOP);
    }

    DXmSvnEnableDisplay( window->widgets[W_SVN] );

    if (!bkrplus_g_charcell_display) 
    {
        bkr_cursor_display_wait( OFF );
    }
};  /* end of bkr_selection_collapse_expand */


void
bkr_selection_detach_from_src PARAM_NAMES((svn_widget,window,data))
    Widget	    	    svn_widget PARAM_SEP
    BKR_WINDOW	    	    *window PARAM_SEP
    DXmSvnCallbackStruct    *data PARAM_END

{

};  /* end of bkr_selection_detach_from_src */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  	bkr_selection_double_click
**
** 	Callback routine for handling a double click on an entry.
**  	Double-click actions:
**
**  	    <entry-icon>    	- makes children visible if entry is expandable.
**  	    <highlight-icon>	- performs a MEMEX Visit operation.
**  	    <entry-title>   	- opens to Topic.
**
**  FORMAL PARAMETERS:
**
**  	svn_widget - id of the SVN widget which received the double click.
**  	window  - pointer to Selection window which contains the SVN widget.
**  	data	   - pointer to SVN callback data.
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
bkr_selection_double_click PARAM_NAMES((svn_widget,window,data))
    Widget	    	    svn_widget PARAM_SEP
    BKR_WINDOW	    	    *window PARAM_SEP
    DXmSvnCallbackStruct    *data PARAM_END

{
    BKR_DIR_ENTRY           *entry;
    int	    	            component_number;

#ifdef DECWORLD
    bkr_decworld_reset_timer = TRUE;
#endif

    entry = (BKR_DIR_ENTRY *) data->entry_tag;
    if ( entry == NULL )
    {
    	entry = (BKR_DIR_ENTRY *) DXmSvnGetEntryTag( svn_widget, 
		    (XtPointer)(data->entry_number) );
    	if ( entry == NULL )
    	    return;
    }

    if (!bkrplus_g_charcell_display) 
    {
        bkr_cursor_display_wait( ON );
    }
    /*  If its a ButtonRelease then use the component # in the callback data, 
     *  otherwise if its a KeyPress force the component # to 2 or 3 depending 
     *  upon if the entry is highlighted.  A KeyPress event means that we
     *  were called from a KActivate, so we force the component # so the target 
     *  Topic is displayed.  This is because the user can expand/collapse 
     *  the entry by first selecting it and then using the VIEW pulldown menu.
     *  Also, because the component_number is undefined by SVN.
     */
    if ( data->event->type == ButtonRelease )
    	component_number = data->component_number;
    else if ( data->event->type == KeyPress )
    {
#ifdef MEMEX
    	if ( entry->highlight )
    	    component_number = 3;
    	else
#endif
    	    component_number = 2;
    }

    /* Do the appropriate action based on the component and the state of
     * the entry.  What the component actually is depends on the state.
     */
    switch (component_number) {

        case 1: {
            /* If the entry is expandable, then the user clicked on the
             * expand/collapse icon.
             */
            if ( ENTRY_IS_EXPANDABLE( entry ) ) { 	/* Has children */
                if ( IS_EXPANDED(svn_widget,entry) ) {
                    remove_entries_from_display( svn_widget, 
                                                entry, 
                                                data->entry_number, 
                                                window );
                } else {
                    (void)add_entries_to_display(svn_widget,
                                           entry,
                                           data->entry_number,
                                           window);
                    bkr_selection_position_entry(svn_widget,entry,POSITION_TOP);
                }
                break;
#ifdef MEMEX
            } else if ( entry->highlight ) {
                /* The double-click was on the highlight icon which is the  
                 * first component in if the entry is not expandable, so do 
                 * a MEMEX Visit.
                 */
                bmi_visit( window );
                break;
#endif 
	    }
            /* Fall through. The component 1 is actually the title.
             */
        }
        case 2: {
#ifdef MEMEX
            if ( ENTRY_IS_EXPANDABLE( entry ) && entry->highlight ) {
                /* The double-click was on the highlight icon so 
                 * do a MEMEX Visit.
                 */
                bmi_visit( window );
                break;
            }
#endif
            /* Fall through. Component 2, or maybe 1, was actually the title.
             */
        }
        default: {
            open_entry_target(window);
        }
    }
    if (!bkrplus_g_charcell_display) 
    {
        bkr_cursor_display_wait( OFF );
    }
    BKR_FLUSH_EVENTS;

};  /* end of bkr_selection_double_click */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_selection_display_entry
**
** 	Positions an entry at the top of the Selection window 
**  	even if its parent entries are not currently open.
**
**  FORMAL PARAMETERS:
**
**	window     - pointer to the Selection window.
**  	dir_id 	      - id of the directory containing the entry.
**  	entry_number  - number of the entry within the directory.
**
**  IMPLICIT INPUTS:
**
**	It is assumed that the toplevel directory entries have already
**  	been initialized. ie, window->shell->book->directories is not NULL.
**  	The routine bkr_selection_new_hierarchy() could be called
**  	to initialize the toplevel directory entries.
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:                               
**
**  	Returns:    the display entry if it could be displayed,
**  	    	    NULL if an error occurs.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
BKR_DIR_ENTRY *
bkr_selection_display_entry PARAM_NAMES((window,dir_id,entry_number))
    BKR_WINDOW	   *window PARAM_SEP
    BMD_OBJECT_ID  dir_id PARAM_SEP
    unsigned 	   entry_number PARAM_END

{
    Widget  	       svn_widget = window->widgets[W_SVN];
    BKR_DIR_ENTRY      *parent_dir;
    BKR_DIR_ENTRY      *entry_list;
    BKR_DIR_ENTRY      *entry_to_display;
    BKR_DIR_ENTRY_PTR  *ancestors = NULL;
    BKR_DIR_ENTRY      *entry;
    BKR_DIR_ENTRY      *current_child;
    unsigned	       num_ancestors;
    unsigned	       i;
    int	    	       svn_entry_number;

    if ( window->shell->book->directories == NULL )
    	return ( NULL );

    /* Find the toplevel entry containing the directory */

    parent_dir = bkr_selection_find_entry_by_id( window->shell->book->directories, dir_id );
    if ( parent_dir == NULL )
    	return ( NULL );

    /* Initialize the directory if its not already open */

    if ( parent_dir->children == NULL )
    {
        Boolean status;
        
        if (bkrplus_g_charcell_display) 
        {
            bkr_cc_working(ON);
        }

    	status = bkr_directory_open( window->shell->book->book_id, parent_dir );
        if (bkrplus_g_charcell_display) 
        {
            bkr_cc_working(ON);
        }
        if (!status) 
        {
            return ( NULL );
        }
    }
    entry_list = parent_dir->children;

    /* Check the range of the entry_number */

    if (entry_number == 0) {
        
        entry_to_display = parent_dir;
        current_child = (BKR_DIR_ENTRY_PTR) &entry_list[0];

    } else if (( entry_number < 0 ) || ( entry_number > parent_dir->u.directory.num_entries )) {
    	return ( NULL );
    } else {
        entry_to_display = (BKR_DIR_ENTRY_PTR) &entry_list[entry_number - 1];
        current_child = entry_to_display;
    }

    /*  Setup an array of all the ancestor entries we must display before
     *  displaying the entry we were passed.  
     *  NOTES:
     *      - we handle the toplevel directory separately.
     *      - we store the entries into a 1-based array in reverse
     *	      order but tell SVN to display them in ascending order.
     */

    num_ancestors = current_child->level;
    ancestors = (BKR_DIR_ENTRY_PTR *) BKR_CALLOC(num_ancestors, sizeof( BKR_DIR_ENTRY_PTR ) );
    entry = current_child;
    for ( i = num_ancestors - 1; i > 0; i-- )	/* 1-based array */
    {
    	while ( current_child->level == entry->level ) 
    	{
    	    entry--;
    	}
    	if ( current_child->level == ( entry->level + 1 ) )
    	{
    	    ancestors[i] = entry;
    	    current_child = entry;
    	}
    	else
    	{   /* Invalid parent found, so just return an error */
    	    if ( ancestors != NULL )
    	    	BKR_CFREE( ancestors );
    	    return ( NULL );
    	}
    }

    DXmSvnDisableDisplay( svn_widget );

    /* Display the children of the toplevel directory */

    svn_entry_number = DXmSvnGetEntryNumber( svn_widget, 
    	    	    	    (XtPointer) parent_dir );
    (void)add_entries_to_display( svn_widget, parent_dir, svn_entry_number, 
    	    	    	    window );

    /* Now open the rest of the entries */

    for ( i = 1; i < num_ancestors; i++ )  	/* ancestors[0] never used */
    {
    	svn_entry_number = DXmSvnGetEntryNumber( svn_widget, 
    	    	    	    	(XtPointer) ancestors[i] );
    	(void)add_entries_to_display( svn_widget, ancestors[i], 
    	    	    	    	svn_entry_number, window );
    }

    if ( ancestors != NULL ) {
        BKR_CFREE( ancestors );
    }

    /* Finally position the specified entry */

    bkr_selection_position_entry( svn_widget, entry_to_display, POSITION_TOP );

    DXmSvnEnableDisplay( svn_widget );

    return ( entry_to_display );

};  /* end of bkr_selection_display_entry */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_selection_find_entry_by_id
**
** 	Searchs a directory list for a directory given its id.
**
**  FORMAL PARAMETERS:
**
**	dir_list    	- pointer to the directory list to search.
**  	dir_id_to_find  - id of the directory to find.
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
**	Returns:    - pointer to the entry containing the directory,
**  	    	    - NULL if no directory found.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
BKR_DIR_ENTRY *
bkr_selection_find_entry_by_id PARAM_NAMES((dir_list,object_id))
    BKR_DIR_ENTRY   *dir_list PARAM_SEP
    BMD_OBJECT_ID   object_id PARAM_END

{
    BKR_DIR_ENTRY   *entry = NULL;
    BMD_OBJECT_ID   dir_id = BRI_DIRECTORY_ID(object_id);
    BMD_OBJECT_ID   entry_num = BRI_DIRECTORY_ENTRY_NUMBER(object_id);

    for ( entry = dir_list; entry != NULL; entry = entry->sibling )
    	if ( entry->object_id == dir_id )
        {
            if (entry_num != 0) {
                if (entry->children && (entry_num <= entry->u.directory.num_entries)) {
                    entry = &entry->children[entry_num-1];
                } else {
                    entry = NULL;
                }
            }
    	    break;
        }

    return ( entry );

};  /* end of bkr_selection_find_entry_by_id */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  	bkr_selection_free_hierarchy
**
** 	Removes all of the data displayed in a single SVN widget.
**
**  FORMAL PARAMETERS:
**
**  	window - pointer to the Selection window
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
bkr_selection_free_hierarchy PARAM_NAMES((window))
    BKR_WINDOW	*window PARAM_END
{
    Widget svn_widget = window->widgets[W_SVN];

    if ( svn_widget == NULL )
    	return;

    /* Free the selected entry tags array first */

    if ( window->u.selection.selected_entry_tags != NULL )
    	BKR_CFREE( window->u.selection.selected_entry_tags );
    window->u.selection.num_selected = 0;

    /* Tell SVN to delete all the toplevel node's children */

    bkr_selection_collapse_expand( window->widgets[W_COLLAPSE_ALL_ENTRY],
    	    	    	    	   (caddr_t)window, NULL );

    /* 
     *  Tell SVN to delete the toplevel entries 
     */

    DXmSvnDisableDisplay( svn_widget );
    DXmSvnDeleteEntries( svn_widget, 0, window->shell->book->n_directories );
    DXmSvnEnableDisplay( svn_widget );

};  /* end of bkr_selection_free_hierarchy */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  	bkr_selection_get_entry
**
** 	Callback routine for initializing the components of an entry.
**  	Entries can have any of the following formats: 
**
**  	    <entry-icon> <entry-title>	    	    	  <-- most common
**  	    	         <entry-title>	    	    	  <-- filler entry
**  	<highlight-icon> <entry-title>  	    	  <-- filler w/ connection
**  	    <entry-icon> <highlight-icon> <entry-title>	  <-- connection
**
**  	NOTE: entry-titles are always aligned unless the entry is highlighted.
**
**  FORMAL PARAMETERS:
**
**  	svn_widget - id of the SVN widget which needs data for an entry.
**  	window  - pointer to Selection window which contains the SVN widget.
**  	data	   - pointer to SVN callback data.
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
bkr_selection_get_entry PARAM_NAMES((svn_widget,window,data))
    Widget	    	    svn_widget PARAM_SEP
    BKR_WINDOW	    	    *window PARAM_SEP
    DXmSvnCallbackStruct    *data PARAM_END

{
    BKR_DIR_ENTRY   *entry = (BKR_DIR_ENTRY *) data->entry_tag;
    int	    	    num_components;
    Pixmap  	    entry_pixmap;
    XmString        entry_prefix;
    XmString	    cs_title;
    int	    	    component_number = 1;
    int	    	    x_position = 0;
    long            byte_cnt, stat;

    if ( entry == NULL )
    	return;

    if ( pixmap_width == 0 )
    	fetch_icon_pixmaps( svn_widget );

    /* Tell SVN about the structure of an entry */

    if ( ( entry->entry_type == DIRECTORY_ENTRY )
    	  && ( ! ENTRY_IS_EXPANDABLE( entry ) )
    	  && ( entry->u.entry.target == NULL ) )
    	num_components = 1; 	/* implies => no icon for entry */
    else
    	num_components = 2;
#ifdef MEMEX
    /* 2nd component is the memex highlight icon */

    if ( entry->highlight )
    	num_components++;
#endif

    /* Decide which icon to use for the entry, may be none */
    /* If no pixmap is needed, reset the num_components */

    entry_pixmap = NULL;
    entry_prefix = NULL;
    if ( ENTRY_IS_EXPANDABLE( entry ) )
    {
        if (bkrplus_g_charcell_display) 
        {
            if (IS_EXPANDED(svn_widget,entry) ) 
            {
                entry_prefix = expanded_entry;
            }
            else 
            {
                entry_prefix = expandable_entry;
            }
        }
        else 
        {
            if (IS_EXPANDED(svn_widget,entry) )
            {
                entry_pixmap = expandable_icon;
            }
            else 
            {
                entry_pixmap = single_target_expandable_icon;
            }
        }
    }
    else 
    {
	num_components--;
    }

    if ( num_components == 0 )	
	num_components++;

    DXmSvnSetEntryNumComponents( svn_widget, data->entry_number, num_components );

    /* Tell SVN about the PIXMAP data component */

    if ( num_components >= 2 )
    {
        if ( entry_pixmap != NULL )
        {
            DXmSvnSetComponentPixmap( svn_widget, 
                                     data->entry_number, 
                                     component_number,
                                     x_position,
                                     0, 	    	    	/* y position       */
                                     entry_pixmap, 
                                     pixmap_width, pixmap_height );

            component_number++;
        }
        else if (entry_prefix != NULL)
        {
            DXmSvnSetComponentText( svn_widget, 
                                   data->entry_number, 
                                   component_number,
                                   x_position,
                                   0,  	      /* y position       */
                                   entry_prefix,
                                   charcell_svn_font_list );

            component_number++;
        }
    }

#ifdef MEMEX
    /* Tell SVN about the highlighted entry pixmap */

    if ( entry->highlight ) {
        if (entry_pixmap != NULL) {
            x_position += pixmap_width + 4;
        }
        bmi_insert_highlight_icon(svn_widget,
                                  data->entry_number, 
                                  component_number,
                                  x_position
                                 );
        component_number++;
    }
#endif

    /* Tell SVN about the TEXT data component */

    x_position += pixmap_width + 4;

    cs_title = DXmCvtFCtoCS( entry->title, &byte_cnt, &stat );
    DXmSvnSetComponentText( svn_widget, 
    	data->entry_number, 
    	component_number,    	    	      /* component number */
    	x_position,                           /* x position       */
    	0,  	    	    	    	      /* y position       */
    	cs_title,   	    	    	      /* entry text       */
    	charcell_svn_font_list );	    	    	    	      /* font 	       	  */
    COMPOUND_STRING_FREE( cs_title );

};  /* end of bkr_selection_get_entry */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  	bkr_selection_highlight_entry
**
** 	Highlights/Unhighlights an entry in an SVN widgets current display.
**
**  	The xbutton_event parameter is only dereferenced if a zero is
**  	passed for the position_number.  In this case, the position within
**  	the current display is determined by using the coordinates contained
**  	in the mouse button event.
**
**  FORMAL PARAMETERS:
**
**  	svn_widget  	- id of the SVN widget containing the entry.
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
void
bkr_selection_highlight_entry PARAM_NAMES((svn_widget,xbutton_event,position_number,highlight))
    Widget  	    	    svn_widget PARAM_SEP
    XButtonPressedEvent	    *xbutton_event PARAM_SEP
    int	    	    	    position_number PARAM_SEP
    Boolean 	    	    highlight PARAM_END

{
    int	    	entry_number;
    unsigned	entry_tag;

#ifdef DECWORLD
    bkr_decworld_reset_timer = TRUE;
#endif

    /* If the position_number is 0 then we need to map the position first */

    entry_number = (int) position_number;
    if ( entry_number == 0 )
    {
    	DXmSvnMapPosition( svn_widget, xbutton_event->x, xbutton_event->y,
    	    	        &entry_number, NULL, &entry_tag );
    }

    DXmSvnDisableDisplay( svn_widget );
    if ( highlight )
    {
    	DXmSvnHighlightEntry( svn_widget, entry_number );
    	DXmSvnShowHighlighting( svn_widget );
    }
    else
    	DXmSvnClearHighlight( svn_widget, entry_number );
    DXmSvnEnableDisplay( svn_widget );

};  /* end of bkr_selection_highlight_entry */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_selection_new_hierarchy
**
** 	Initializes the SVN widget with a directory hierarchy.
**
**  FORMAL PARAMETERS:
**
**	window   - pointer to the Selection window containing the SVN widget.
**	dir_to_open - id of the first directory to open if non-zero.
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
**	Returns:    status of initialization.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
unsigned
bkr_selection_new_hierarchy PARAM_NAMES((window,dir_to_open))
    BKR_WINDOW *window PARAM_SEP 
    BMD_OBJECT_ID  dir_to_open PARAM_END
{
    BKR_DIR_ENTRY   	    entry;
    Widget  	    	    svn_widget = window->widgets[W_SVN];
    BKR_DIR_ENTRY   	    *dir_entry = NULL;
    unsigned int	    rtn_status = 0;

    if ( window->shell->book->directories == NULL )
    {
    	return ( 0 );
    }

#ifdef MEMEX
    bmi_create_directory_ui( window );
#endif

    /* Add the toplevel directories to the SVN display */

    DXmSvnDisableDisplay( svn_widget );
    entry.num_children	= window->shell->book->n_directories;
    entry.children  	= window->shell->book->directories;
    entry.level	    	= -1;
    entry.entry_type	= DIRECTORY;
    entry.object_id  	= 0;
    rtn_status = add_entries_to_display( svn_widget, 
					&entry, 
					BKR_TOPLEVEL_NODE, 
					window );

    DXmSvnEnableDisplay( svn_widget );

    if( ! rtn_status)
    	return( 0 );


    /*  Open to the specified directory if supplied */

    if ( dir_to_open != 0 )
    {
    	dir_entry = bkr_selection_find_entry_by_id(window->shell->book->directories , 
    	    	    	    	    	    	    dir_to_open );
    	if ( dir_entry != NULL )
    	{
    	    DXmSvnDisableDisplay( svn_widget );
	    rtn_status = add_entries_to_display( svn_widget, dir_entry, 
    	    	    	    	    DXmSvnGetEntryNumber( svn_widget, 
    	    	    	       	    	    (XtPointer) dir_entry ),
    	    	    	    	    window );

    	    DXmSvnEnableDisplay( svn_widget );

	    if( ! rtn_status)
		return( 0 ); 

    	}
    	else
    	    return ( 0 );
    }

    return ( 1 );

};  /* end of bkr_selection_new_hierarchy */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_selection_popup_menu
**
** 	Callback routine which handles displaying the appropriate
**  	popup menu.  The popup menu is shared between ALL of the Selection
**  	windows.
**
**  FORMAL PARAMETERS:
**
**  	svn_widget  - id of the widget that caused the callback.
**  	window   - pointer to Selection window which contains the SVN widget.
**  	data	    - pointer to SVN callback data.
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
bkr_selection_popup_menu PARAM_NAMES((svn_widget,window,data))
    Widget		    svn_widget PARAM_SEP
    BKR_WINDOW	    	    *window PARAM_SEP
    DXmSvnCallbackStruct    *data PARAM_END

{
    BKR_DIR_ENTRY   	    *entry;
    XButtonPressedEvent	    *xbutton;
    XButtonPressedEvent	    dummy_xbutton;
    Arg	    	    	    arglist[5];
    int	    	    	    argcnt;
    Boolean                 opened = FALSE;

    entry = (BKR_DIR_ENTRY *) data->entry_tag;
    if ( entry == NULL )
    {
    	entry = (BKR_DIR_ENTRY *) DXmSvnGetEntryTag( svn_widget, 
		    (XtPointer)(data->entry_number) );
    	if ( entry == NULL )
    	    return;
    }

    /*  Create the popup menu.
     */
    if ( popup_menu_widgets == NULL )
    	bkr_selection_create_popups( &popup_menu_widgets );
    if ( popup_menu_widgets == NULL )
    	return;

    /* Manage/Unmanage the appropriate push buttons for this mapping */

    if ( ( ! ENTRY_IS_EXPANDABLE( entry ) ) && ( entry->u.entry.target == NULL) )
    	return;
    if ( entry->u.entry.target == NULL )
    {
	if ( XtIsManaged( popup_menu_widgets[W_TOPIC_POPUP_ENTRY] ) )
	    XtUnmanageChild( popup_menu_widgets[W_TOPIC_POPUP_ENTRY] );
	if ( XtIsManaged( popup_menu_widgets[W_TOPIC_IN_NEW_POPUP_ENTRY] ) )
	    XtUnmanageChild( popup_menu_widgets[W_TOPIC_IN_NEW_POPUP_ENTRY] );

    }
    else 
    {
	if ( ! XtIsManaged( popup_menu_widgets[W_TOPIC_POPUP_ENTRY] ) )
	    XtManageChild( popup_menu_widgets[W_TOPIC_POPUP_ENTRY] );
	if ( ! XtIsManaged( popup_menu_widgets[W_TOPIC_IN_NEW_POPUP_ENTRY] ) )
	    XtManageChild( popup_menu_widgets[W_TOPIC_IN_NEW_POPUP_ENTRY] );
	if ( XtIsManaged( popup_menu_widgets[W_TOPIC_MULTI_POPUP_ENTRY] ) )
	    XtUnmanageChild( popup_menu_widgets[W_TOPIC_MULTI_POPUP_ENTRY] );
    }
    if ( ENTRY_IS_EXPANDABLE( entry ) )
    {
    	if ( ! XtIsManaged( popup_menu_widgets[W_EXPAND_POPUP_ENTRY] ) )
    	    XtManageChild( popup_menu_widgets[W_EXPAND_POPUP_ENTRY] );
    	if ( ! XtIsManaged( popup_menu_widgets[W_COLLAPSE_POPUP_ENTRY] ) )
    	    XtManageChild( popup_menu_widgets[W_COLLAPSE_POPUP_ENTRY] );

        opened = IS_EXPANDED(svn_widget,entry);
    }
    else
    {
    	if ( XtIsManaged( popup_menu_widgets[W_EXPAND_POPUP_ENTRY] ) )
    	    XtUnmanageChild( popup_menu_widgets[W_EXPAND_POPUP_ENTRY] );
    	if ( XtIsManaged( popup_menu_widgets[W_COLLAPSE_POPUP_ENTRY] ) )
    	    XtUnmanageChild( popup_menu_widgets[W_COLLAPSE_POPUP_ENTRY] );
    }

    /* Set the userData resource so the callbacks can access the window */

    argcnt = 0;
    SET_ARG( XmNuserData,   (caddr_t) window );
    SET_ARG( XmNsensitive,  ! opened );
    XtSetValues( popup_menu_widgets[W_EXPAND_POPUP_ENTRY], arglist, argcnt );
    argcnt = 0;
    SET_ARG( XmNuserData,   (caddr_t) window );
    SET_ARG( XmNsensitive,  opened );
    XtSetValues( popup_menu_widgets[W_COLLAPSE_POPUP_ENTRY], arglist, argcnt );
    XtSetArg( arglist[0], XmNuserData, (caddr_t) window );
    XtSetValues( popup_menu_widgets[W_TOPIC_POPUP_ENTRY],   	 arglist, 1 );
    XtSetValues( popup_menu_widgets[W_TOPIC_IN_NEW_POPUP_ENTRY], arglist, 1 );
    XtSetValues( popup_menu_widgets[W_TOPIC_MULTI_POPUP_ENTRY],  arglist, 1 );
    XtSetValues( popup_menu_widgets[W_SELECTION_POPUP],     	 arglist, 1 );

    /* Update the NODE pointer for this mapping and highlight the entry node */

    window->u.selection.btn3down_entry_node = entry;
    bkr_selection_highlight_entry( svn_widget, NULL, data->entry_number, ON );

    /*  If its a ButtonPress then use the event to position the popup
     *  menu, otherwise if its a KeyPress build a dummy XButton event.
     */
    if ( data->event->type == ButtonPress )
    	xbutton = (XButtonPressedEvent *) &data->event->xbutton;
    else if ( data->event->type == KeyPress )
    {
    	Window	junk_window;

    	xbutton = (XButtonPressedEvent *) &dummy_xbutton;
	xbutton->type	     = ButtonPress;
	xbutton->serial	     = 0;
	xbutton->send_event  = False;
	xbutton->display     = XtDisplay( svn_widget );
	xbutton->window	     = XtWindow( svn_widget );
	xbutton->root	     = XDefaultRootWindow( XtDisplay( svn_widget ) );
	xbutton->subwindow   = XtWindow( svn_widget );
	xbutton->time	     = data->event->xkey.time;
	xbutton->x   	     = 0;
    	xbutton->y   	     = 0;
	xbutton->state	     = 0;
	xbutton->button	     = Button3;
	xbutton->same_screen = True;
    	XTranslateCoordinates( XtDisplay( svn_widget ), xbutton->window, 
    	    	xbutton->root, xbutton->x, xbutton->y,
    	    	&xbutton->x_root, &xbutton->y_root, 
    	    	&junk_window );
    }

    /* Position the popup menu then manage it. */

    if ( ! XtIsRealized( popup_menu_widgets[W_SELECTION_POPUP] ) )
    	XtRealizeWidget( popup_menu_widgets[W_SELECTION_POPUP] );
    XmMenuPosition( popup_menu_widgets[W_SELECTION_POPUP], xbutton );
    XtManageChild( popup_menu_widgets[W_SELECTION_POPUP] );

};  /* end of bkr_selection_popup_menu */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_selection_position_entry
**
** 	Scrolls the specified entry to the top of the current SVN 
**  	display within a particular Selection window.
**
**  FORMAL PARAMETERS:
**
**  	svn_widget  	- id of the SVN widget to scroll.
**  	entry	    	- pointer to the entry make visible.
**  	new_position	- location to scroll entry to.
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
bkr_selection_position_entry PARAM_NAMES((svn_widget,entry,new_position))
    Widget	    svn_widget PARAM_SEP
    BKR_DIR_ENTRY   *entry PARAM_SEP
    int	    	    new_position PARAM_END

{
    int	    entry_number;
    int	    position;

    switch ( new_position )
    {
    	case POSITION_TOP :
    	    position = DXmSvnKpositionTop;
    	    break;
    	case POSITION_MIDDLE :
    	    position = DXmSvnKpositionMiddle;
    	    break;
    	case POSITION_BOTTOM :
    	    position = DXmSvnKpositionBottom;
    	    break;
    	default :
    	    position = DXmSvnKpositionTop;
    }
    entry_number = DXmSvnGetEntryNumber( svn_widget, (XtPointer) entry );
    if (entry_number > 0) {
        DXmSvnPositionDisplay( svn_widget, entry_number, position );
    }

};  /* end of bkr_selection_position_entry */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_selection_transitions_done
**
** 	Callback routine for handling the change of entries from
**  	selected to unselected or unselected to selected.
**
**  FORMAL PARAMETERS:
**
**  	svn_widget - id of the SVN widget which has selected entries.
**  	window  - pointer to Selection window which contains the SVN widget.
**  	data	   - pointer to SVN callback data.
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
bkr_selection_transitions_done PARAM_NAMES((svn_widget,window,data))
    Widget		    svn_widget PARAM_SEP
    BKR_WINDOW	    	    *window PARAM_SEP
    DXmSvnCallbackStruct    *data PARAM_END

{
    int	    	    	    num_selected;
    int	    	    	    *entry_numbers;
    XtPointer	    	    *entry_tags;
    int	    	    	    i;
    BKR_DIR_ENTRY   	    *entry;

    /* Discard the old entry tag arrays and zero the count */

    if ( window->u.selection.selected_entry_tags != NULL )
    {
    	BKR_CFREE( window->u.selection.selected_entry_tags );
    	window->u.selection.num_selected = 0;
    }

    /* Get the number of selected entries from SVN */

    num_selected = DXmSvnGetNumSelections( svn_widget );
    if ( num_selected == 0 )
    {
    	set_pulldown_menu_buttons( window );
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

    /* 
     * Save the entry tags array and the number of selected entries and
     * free the entry_numbers array because we don't need it anymore.
     */

    window->u.selection.selected_entry_tags = entry_tags;
    window->u.selection.num_selected = num_selected;
    BKR_CFREE( entry_numbers );

    /* Set the appropriate pulldown menu entries */

    set_pulldown_menu_buttons( window );

};  /* end of bkr_selection_transitions_done */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_selection_view_directory
**
** 	Callback routine for scrolling the DIRECTORY specified to the 
**  	top of SVN display within a particular Selection window.
**  	The directory will be opened if it isn't already.
**
**  FORMAL PARAMETERS:
**
**  	widget	- id of the widget that caused the event.
**  	entry	- pointer to the DIRETORY entry to make visible.
**  	reason	- pointer to the callback data. (unused)
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
bkr_selection_view_directory PARAM_NAMES((widget,entry,reason))
    Widget	    	    widget PARAM_SEP
    BKR_DIR_ENTRY   	    *entry PARAM_SEP
    XmAnyCallbackStruct	    *reason PARAM_END

{
    BKR_WINDOW	*window = NULL;
    Widget  	svn_widget;
    int	    	entry_number;

    if (!bkrplus_g_charcell_display) 
    {
        bkr_cursor_display_wait( ON );
    }
    /* Get the Selection window associated with the push button
     */

    window = bkr_window_find( widget );
    if ( window == NULL )
    {
        if (!bkrplus_g_charcell_display) 
        {
            bkr_cursor_display_wait( OFF );
        }
    	return;
    }
    svn_widget = window->widgets[W_SVN];
    entry_number = DXmSvnGetEntryNumber( svn_widget, (XtPointer) entry );

    DXmSvnDisableDisplay( svn_widget );

    /* Display the first level entries within the directory.
     */
    (void)add_entries_to_display( svn_widget, entry, entry_number, window );

    /* Now make the Directory entry visible.
     */
    bkr_selection_position_entry( svn_widget, entry, POSITION_TOP );

    DXmSvnEnableDisplay( svn_widget );

    if (!bkrplus_g_charcell_display) 
    {
        bkr_cursor_display_wait( OFF );
    }
};  /* end of bkr_selection_view_directory */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_selection_update_entry
**
** 	Forces an entry within the SVN display to be refetched.
**  	When the MEMEX highlighting changes on an entry this routine
**  	should be called to update the entry.
**
**  FORMAL PARAMETERS:
**
**	svn_widget  - id of the widget containing the entry.
**  	entry	    - pointer to the entry to update.
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
**	DXmSvnGetEntry callback will be called to update the entry.
**
**--
**/
void
bkr_selection_update_entry PARAM_NAMES((svn_widget,entry))
    Widget svn_widget PARAM_SEP
    BKR_DIR_ENTRY *entry PARAM_END
{
    int	    entry_number;

    entry_number = DXmSvnGetEntryNumber( svn_widget, (XtPointer) entry );
    if ( entry_number > 0 )
    {
    	DXmSvnDisableDisplay( svn_widget );
    	DXmSvnInvalidateEntry( svn_widget, entry_number );
    	DXmSvnEnableDisplay( svn_widget );
    }

};  /* end of bkr_selection_update_entry */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_selection_keyboard_actions
**
** 	Key input action handler routine for the selection window
**
**  FORMAL PARAMETERS:
**
**	svn_widget     	- id of the widget that caused the event,
**	xbutton_event	- X event associated with the Btn1Up event
**	params	    	- address of a list of strings passed as arguments
**	num_params  	- address of the number of arguments passed
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
bkr_selection_keyboard_actions PARAM_NAMES((svn_primary_window,event,params,num_params))
    Widget		    svn_primary_window PARAM_SEP
    XKeyEvent  	            *event PARAM_SEP
    String  	    	    *params PARAM_SEP
    Cardinal	    	    *num_params PARAM_END

{
    BKR_WINDOW 	            *window = NULL;
    Widget                  svn_widget;

    /* Get the window from the userData of the SVN widget.
     */
    XtVaGetValues(svn_primary_window,XmNuserData,&window,NULL);

    if ((window == NULL) 
        || (window->type != BKR_SELECTION)
        || (window->widgets[W_SVN] == NULL)
        ) {
    	return;
    }

    if (window->u.selection.num_selected == 0) 
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
        BKR_DIR_ENTRY  *entry;
        BKR_DIR_ENTRY  *entry_to_position = NULL;
        int	       i;
        XtPointer      entry_tag;
        int	       entry_number;

        static int beepcount = 0;


        if (!bkrplus_g_charcell_display) 
        {
            bkr_cursor_display_wait( ON );
        }
        DXmSvnDisableDisplay(svn_widget);

    	for ( i = 0; i < window->u.selection.num_selected; i++ ) 
        {
    	    entry_tag = (XtPointer)window->u.selection.selected_entry_tags[i];
    	    entry = (BKR_DIR_ENTRY *) entry_tag;
            
    	    if ( ENTRY_IS_EXPANDABLE(entry) ) 
            {
                entry_number = DXmSvnGetEntryNumber(svn_widget,entry_tag);
                
                if (IS_EXPANDED(svn_widget,entry)) 
                {
                    remove_entries_from_display(svn_widget,entry,entry_number,window);
                }
                else 
                {
                    (void)add_entries_to_display(svn_widget,entry,entry_number,window);
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
                    if (entry->u.entry.target) 
                    {
                        if(bkrplus_g_charcell_display) 
                        {
                            bkr_error_simple_msg(window,"CC_ENTRY_NO_SUBTOPICS");
                        }
                        else 
                        {
                            bkr_error_simple_msg(window,"MF_ENTRY_NO_SUBTOPICS");
                        }
                    }
                    else 
                    {
                        bkr_error_simple_msg(window,"ENTRY_NO_TOPIC_NO_SUBTOPICS");
                    }
                    beepcount = 0;
                }
            }
        }
        /*  Update the selected entry tags just in case the selections have changed.
         */
        {
            DXmSvnCallbackStruct    data;

            bkr_selection_transitions_done(svn_widget, window, &data );
        }
        if ( entry_to_position) {
            bkr_selection_position_entry(svn_widget,entry_to_position,POSITION_TOP);
        }

        DXmSvnEnableDisplay(svn_widget);

        if (!bkrplus_g_charcell_display) 
        {
            bkr_cursor_display_wait( OFF );
        }
    }
    else if (strcmp(params[0],"OpenClose") == 0 )
    {
        open_entry_target(window);
    }
};  /* end of bkr_selection_keyboard_actions */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	add_entries_to_display
**
** 	Adds entries to the SVN display given the parent entry.  If the
**  	entry is a directory and the directory is not yet initialized, then
**  	the directory is first opened.  After the entries have been added
**  	the source entry count is updated. (mjf -1-31-1992) Need to return a 
**	status in case of bogus directories 
**  FORMAL PARAMETERS:
**
**  	svn_widget   - id of the SVN widget to add the entries to.
**  	entry	     - pointer to the parent entry.
**  	entry_number - number of the entry within the SVN display.
**  	window    - pointer to Selection window data.
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
**	This routine assumes that DXmSvnDisableDisplay has already been called
**  	before adding new entries to the SVN display.
**
**--
**/
unsigned int
add_entries_to_display PARAM_NAMES((svn_widget,entry,entry_number,window))
    Widget  	    svn_widget PARAM_SEP
    BKR_DIR_ENTRY   *entry PARAM_SEP
    int	    	    entry_number PARAM_SEP
    BKR_WINDOW	    *window PARAM_END

{
    XtPointer	    	    *entry_tags = NULL;
    int	    	    	    entry_num;
    BKR_DIR_ENTRY   	    *child_entry;
    unsigned	    	    new_level = entry->level + 1;

    /* Directory not open yet */

    if ( ( entry->entry_type == DIRECTORY ) && ( entry->num_children == 0 ) )
    {
    	 if ( entry->object_id != 0 ) {
    	    if( ! bkr_directory_open(window->shell->book->book_id, entry ))
	        return(FALSE);
		
        } else {
            return(TRUE);
        }
    }

    if ( ! ENTRY_IS_EXPANDABLE( entry ) )
    	return(TRUE);

    /* Entry is already opened */

    if ( IS_EXPANDED(svn_widget,entry) )
    	return(TRUE);

    entry_tags = (XtPointer *) BKR_CALLOC( entry->num_children, 
		    sizeof( XtPointer ) );

    /* Build tag array from entries and tell SVN to add them */

    child_entry = entry->children;
    for ( entry_num = 0; entry_num < entry->num_children; entry_num++ )
    {
    	entry_tags[entry_num] = (XtPointer) child_entry;
    	child_entry = child_entry->sibling;
    }
    DXmSvnAddEntries( 
    	svn_widget,
    	entry_number,	    	    	    	/* Insert after this entry  	 */
    	entry->num_children,	    	    	/* Number of entries to add 	 */
    	new_level,   	    	    	    	/* Level of new entries	    	 */
    	entry_tags, 	    	    	    	/* Entry tag array  	    	 */
    	( new_level <= 2 ) ? TRUE : FALSE );	/* Display entry in index window */

    if ( entry_tags != NULL )
    	BKR_CFREE( entry_tags );

    /* 
     *  Update the number of source entries visible and set the button 
     *  sensitivity. 
     */

    window->u.selection.num_source_entries += entry->num_children;
    set_pulldown_menu_buttons( window );

    bkr_selection_update_entry( svn_widget, entry );

#ifdef MEMEX
    /*  Update the highlighting, since things may have changed 
     *  since we last looked at it.
     */
    bmi_update_dir_highlighting( window, FALSE );
#endif 
    return(TRUE);

};  /* end of add_entries_to_display */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	expand_to_lowest_level
**
** 	Recursive routine for expanding an entry and all of its ancestor
**  	entries below it until a leaf entry is reached.  The entries are
**  	added to the SVN display as they are opened.
**
**  FORMAL PARAMETERS:
**
**  	window    - pointer to Selection window data.
**  	entry	     - pointer to the starting entry to expand.
**  	entry_number - number of the entry within the SVN display.
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
expand_to_lowest_level PARAM_NAMES((window,entry,entry_number))
    BKR_WINDOW	    *window PARAM_SEP
    BKR_DIR_ENTRY   *entry PARAM_SEP
    int	    	    entry_number PARAM_END

{
    BKR_DIR_ENTRY   	 *child_entry;
    Widget  	    	 svn_widget = (Widget) window->widgets[W_SVN];

    add_entries_to_display( svn_widget, entry, entry_number, window );

    for ( child_entry = entry->children;
    	    child_entry != NULL;
    	    child_entry = child_entry->sibling )
    {
    	expand_to_lowest_level( window, child_entry, 
    	    	    	    	 DXmSvnGetEntryNumber( svn_widget, 
    	    	    	    	    	(XtPointer) child_entry ) );
    }

};  /* end of expand_to_lowest_level */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	fetch_icon_pixmaps
**
** 	Creates the icon pixmaps used for the pixmap part of an entry.
**
**  FORMAL PARAMETERS:
**
**	svn_widget - id of the svn widget to use for the foreground/background.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**  	Initializes the static variables:
**    	    expandable_icon 	    	    single_target_expandable_icon
**    	    pixmap_width    	    	    pixmap_height
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
fetch_icon_pixmaps PARAM_NAMES((svn_widget))
    Widget  svn_widget PARAM_END
{
    Pixel   foreground_pixel;
    Pixel   background_pixel;
    Arg     arglist[5];
    int	    argcnt;
    caddr_t data;

    if (pixmap_width != 0) {
        return;
    }

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

        pixmap_width = 2 * bkr_default_space_width;
    }
    else 
    {
        /* Get the foreground/background color for the icons */

        argcnt = 0;
        SET_ARG( XmNforeground, &foreground_pixel );
        SET_ARG( XmNbackground, &background_pixel );
        XtGetValues( svn_widget, arglist, argcnt );
        
        expandable_icon = bkr_fetch_icon_literal( "EXPANDABLE_ICON", 
                                                 foreground_pixel, background_pixel );
        single_target_expandable_icon = bkr_fetch_icon_literal( "SINGLE_TARGET_EXPANDABLE_ICON",
                                                               foreground_pixel, background_pixel );
        
        data = bkr_fetch_literal( "SELECTION_PIXMAP_WIDTH", MrmRtypeInteger );
        pixmap_width = *( (int *) data );
        data = bkr_fetch_literal( "SELECTION_PIXMAP_HEIGHT", MrmRtypeInteger );
        pixmap_height = *( (int *) data );
    }
};  /* end of fetch_icon_pixmaps */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	remove_entries_from_display
**
** 	Removes entries from the SVN display given the parent entry.  After 
**  	the entries have been removed, the source entry count is updated.
**
**  FORMAL PARAMETERS:
**
**  	svn_widget   - id of the SVN widget to add the entries to.
**  	entry	     - pointer to the parent entry.
**  	entry_number - number of the entry within the SVN display.
**  	window    - pointer to Selection window data.
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
**	This routine assumes that DXmSvnDisableDisplay has already been called
**  	before removing new entries from the SVN display.
**
**--
**/
static void
remove_entries_from_display PARAM_NAMES((svn_widget,entry,entry_number,window))
    Widget  	    svn_widget PARAM_SEP
    BKR_DIR_ENTRY   *entry PARAM_SEP
    int	    	    entry_number PARAM_SEP
    BKR_WINDOW	    *window PARAM_END

{
    BKR_DIR_ENTRY   *child_entry;
    int	    	    i;

    if ( ! ENTRY_IS_EXPANDABLE( entry ) )
    	return;

    /* Entry is already closed */

    if ( ! IS_EXPANDED(svn_widget,entry) )
    	return;

    /* Recursively close each child node */

    child_entry = entry->children;
    for ( i = 1; i <= entry->num_children; i++ )
    {
    	remove_entries_from_display( svn_widget, child_entry, 
    	    	    	    	    entry_number, window );
    	child_entry = child_entry->sibling;
    }

    /* Tell SVN to remove its children */

    DXmSvnDeleteEntries( svn_widget, entry_number, entry->num_children );

    /* 
     *  Mark the entry closed, update the number of source entries visible
     *  and set the button sensitivity.
     */

    window->u.selection.num_source_entries -= entry->num_children;
    set_pulldown_menu_buttons( window );

    bkr_selection_update_entry( svn_widget, entry );

};  /* end of remove_entries_from_display */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	set_pulldown_menu_buttons
**
** 	Updates the sensitivity of the push buttons in the pulldown menus
**  	based on the selected entries and number of source entries.
**
**  FORMAL PARAMETERS:
**
**  	window   - pointer to Selection window data.
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
set_pulldown_menu_buttons PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END
{
    BKR_DIR_ENTRY   *entry = NULL;
    int	    	    i;
    int             n_selected;
    Boolean 	    open_topic_sensitive = FALSE;
    Boolean 	    collapse_sensitive = FALSE;
    Boolean 	    expand_sensitive = FALSE;
    Boolean 	    expand_all_sensitive = FALSE;
#ifdef SEARCH
    Boolean 	    search_topics_sensitive = 
				( window->u.selection.num_selected > 0);
#endif /* SEARCH */

    n_selected = window->u.selection.num_selected;

    for ( i = 0; i < n_selected; i++ )
    {
    	entry = (BKR_DIR_ENTRY *) window->u.selection.selected_entry_tags[i];
    	if ( entry->u.entry.target )
    	    open_topic_sensitive = TRUE;
    	if ( ENTRY_IS_EXPANDABLE( entry ) )
    	{
    	    expand_all_sensitive = TRUE;
    	    if ( IS_EXPANDED(window->widgets[W_SVN],entry) )
    	    	collapse_sensitive = TRUE;
    	    else
    	    	expand_sensitive = TRUE;
    	}
    }

    if ( window->widgets[W_FILE_MENU] != NULL )
    {
    	XtSetSensitive( window->widgets[W_OPEN_TOPIC_IN_DEFAULT], open_topic_sensitive );
    	XtSetSensitive( window->widgets[W_OPEN_TOPIC_IN_NEW], open_topic_sensitive );
#ifdef PRINT
        /*--- set "Print Book" button to gray if nothing selected ---*/
        XtSetSensitive( bkr_library->widgets[W_PR_PRTOPIC_BUTTON],n_selected );
#endif /* PRINT */
    }

    if ( window->widgets[W_VIEW_MENU] != NULL )
    {
    	XtSetSensitive( window->widgets[W_EXPAND_ENTRY], expand_sensitive );
    	XtSetSensitive( window->widgets[W_EXPAND_ALL_ENTRY], expand_all_sensitive );
    	XtSetSensitive( window->widgets[W_COLLAPSE_ENTRY], collapse_sensitive );

    	if ( window->u.selection.num_source_entries > window->shell->book->n_directories ) 
    	    XtSetSensitive( window->widgets[W_COLLAPSE_ALL_ENTRY], TRUE );
    	else if ( window->u.selection.num_source_entries == window->shell->book->n_directories ) 
    	    XtSetSensitive( window->widgets[W_COLLAPSE_ALL_ENTRY], FALSE );
    }

#ifdef SEARCH
    if ( window->widgets[W_SEARCH_MENU] != NULL )
    {
    	XtSetSensitive( 
		window->widgets[W_SEARCH_TOPICS_ENTRY], 
		search_topics_sensitive );
    }
#endif /* SEARCH */

};  /* end of set_pulldown_menu_buttons */



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	open_entry_target
**
** 	Attempts to open the target for an entry in the selection window.
**
**  FORMAL PARAMETERS:
**
**  	window   - pointer to Selection window data.
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
open_entry_target PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END
{
    BKR_DIR_ENTRY  *entry;
    
    static int beepcount = 0;

    entry = (BKR_DIR_ENTRY *) window->u.selection.selected_entry_tags[0];
        
    if ((entry->entry_type == DIRECTORY_ENTRY) && (entry->u.entry.target)) 
    {
        /* Open the target in the default topic window.
         */
        bkr_object_id_dispatch(window->shell,
                               window->shell->default_topic,
                               entry->u.entry.target);

        /* reset beep counter 
         */
        beepcount = 0;

    } 
    else 
    {
        /* if entry is a directory with no target, beep three times
         * before putting out an error message	     
         */
        XBell( bkr_display, BELL_VOLUME );
        beepcount++;

        /* if ( beepcount == 2 )  */
	/* for osf qar # 10352, change this to beepcount ==1.  Otherwise
	   it appears to be an error that the error message box only
	   appears on every other double click. 
	*/
        if ( beepcount == 1 ) 
        { 	 
            if (ENTRY_IS_EXPANDABLE(entry)) 
            {
                if(bkrplus_g_charcell_display) 
                {
                    bkr_error_simple_msg(window,"CC_ENTRY_NO_TOPIC");
                }
                else 
                {
                    bkr_error_simple_msg(window,"MF_ENTRY_NO_TOPIC");
                }
            }
            else 
            {
                bkr_error_simple_msg(window,"ENTRY_NO_TOPIC_NO_SUBTOPICS");
            }
            beepcount = 0;
        }	
    }
}

