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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_MENU.C*/
/* *7     8-NOV-1992 19:17:13 BALLENGER "Fix problem with updating view menu."*/
/* *6    13-AUG-1992 15:09:58 GOSSELIN "updating with necessary A/OSF changes"*/
/* *5     9-JUN-1992 09:57:15 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *4     3-MAR-1992 17:00:56 KARDON "UCXed"*/
/* *3     1-NOV-1991 12:58:20 BALLENGER "Reintegrate  memex support"*/
/* *2    18-SEP-1991 13:51:37 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:39:40 PARMENTER "Popup menus"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_MENU.C*/
#ifndef VMS
 /*
#else
#module BKR_MENU "V03-0000"
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
**	Popup menu support routines.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     21-Jan-1990
**
**  MODIFICATION HISTORY:
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
#include "bkr_menu.h"        /* function prototypes for .c module */
#include "bkr_error.h"       /* function prototypes for .c module */
#include "bkr_library.h"   /* function prototypes for .c module */
#include "bkr_selection.h"   /* function prototypes for .c module */
#include  <ctype.h>


/*
 * LOCAL ROUTINES
 */
static void  	    	
add_mnemonics_to_buttons
    PROTOTYPE((BKR_WINDOW *window,
               int        num_dirs
               ));
static Widget	    	
create_pulldown_button
    PROTOTYPE((Widget  	      parent_widget,
               char    	      *button_label,
               XtCallbackProc callback_proc,
               XtPointer      callback_data
               ));

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_menu_unmap 
**
** 	Cleans up the current window when a popup menu is 
**  	being unmapped.
**
**  FORMAL PARAMETERS:
**
**	widget      	- id of the widget to be saved
**	tag 	    	- pointer to the tag passed from the UIL file
**	reason  	- pointer to callback data (not used)
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
bkr_menu_unmap PARAM_NAMES((widget,tag,reason))
    Widget	    	    widget PARAM_SEP
    int 	    	    *tag PARAM_SEP
    XmAnyCallbackStruct     *reason PARAM_END	/* unused */

{
    BKR_WINDOW              *window;
    int	    	    	    entry_number;
    XtPointer	    	    entry_tag;
    Arg	    	    	    arglist[5];
    int	    	    	    argcnt;

    if ( tag == NULL )
    	return;

    /* If the contents of the tag's 1, its the Library shell */

    if ( *tag == K_LIBRARY_WINDOW )
    {
    	bkr_library_update_node( bkr_library->u.library.btn3down_entry_node, NULL, CLOSE );
    	return;
    }

    /* 
     *  Get the user data, if non-NULL then the callback is from the 
     *  Selection popup menu, otherwise its probably a bogus callback.
     */

    argcnt = 0;
    SET_ARG( XmNuserData, &window );
    XtGetValues( widget, arglist, argcnt );
    if ( window == NULL )
    	return;

    if ( window->type == BKR_SELECTION )
    {
    	/* Unhighlight the entry
         */
    	entry_tag = (XtPointer) window->u.selection.btn3down_entry_node;
    	entry_number = DXmSvnGetEntryNumber( window->widgets[W_SVN], 
                                            entry_tag );
#ifdef MEMEX
    	if ( ! window->u.selection.btn3down_entry_node->highlight )
#endif 
    	    bkr_selection_highlight_entry( window->widgets[W_SVN], 
    	    	    	    	    	   NULL, entry_number, OFF );
    }

};  /* end of bkr_menu_unmap */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_menu_update_view_pulldown
**
** 	Updates the push buttons in the VIEW pulldown menu after a book
**  	is opened within a Selection shell.  One push button is created
**  	for each directory in the book.
**
**  FORMAL PARAMETERS:
**
**  	window   	 - pointer to the Selection window containing the menu.
**  	toplevel_entries - pointer to the directory list.
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
bkr_menu_update_view_pulldown PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END
{
    BKR_DIR_ENTRY        *toplevel_entries; 
    BKR_DIR_ENTRY   	 *entry;
    int	    	    	 i;
    WidgetList	    	 widgetlist;
    Widget	    	 button;
    int	    	         num_dirs;

    if ((window->type != BKR_SELECTION) ||
        (window->widgets[W_VIEW_MENU] == NULL))
    {
    	return;
    }

    /* Destroy the current directory buttons first 
     */
    if ( ( window->u.selection.view_menu_dir_buttons != NULL )
    	 && ( window->u.selection.num_view_dir_buttons != 0 ) )
    {
    	for ( i = 0; i < window->u.selection.num_view_dir_buttons; i++ )
        {
    	    XtDestroyWidget(window->u.selection.view_menu_dir_buttons[i]);
        }
    	BKR_CFREE( window->u.selection.view_menu_dir_buttons );
    	window->u.selection.num_view_dir_buttons = 0;
    }

    toplevel_entries = window->shell->book->directories; 
    num_dirs = window->shell->book->n_directories;

    if ((num_dirs > 0) && (toplevel_entries) )
    {
        /* Create the directory push buttons for the VIEW menu 
         */
        widgetlist = (WidgetList) BKR_CALLOC( num_dirs, sizeof( Widget ) );
        window->u.selection.view_menu_dir_buttons = widgetlist;
        window->u.selection.num_view_dir_buttons = num_dirs;

        for ( i = 0, entry = (BKR_DIR_ENTRY_PTR) toplevel_entries; 
             i < num_dirs; 
             i++, entry++ 
             )
        {
            widgetlist[i] = create_pulldown_button( window->widgets[W_VIEW_MENU], 
                                                   entry->title,
                                                   (XtCallbackProc) bkr_selection_view_directory,
                                                   (XtPointer) entry );
        }
        add_mnemonics_to_buttons( window, num_dirs );
    }
    else 
    {
        window->u.selection.view_menu_dir_buttons = NULL;
        window->u.selection.num_view_dir_buttons = 0;
    }

};  /* end of bkr_menu_update_view_pulldown */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	add_mnemonics_to_buttons
**
** 	Adds mnemonics to the push buttons which are created dynamically
**  	for the VIEW menu.
**
**  FORMAL PARAMETERS:
**
**  	sel_window - pointer to the Selection window containin the VIEW menu.
**  	num_dirs   - number of Directory push buttons.
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
add_mnemonics_to_buttons PARAM_NAMES((window,num_dirs))
    BKR_WINDOW    *window PARAM_SEP
    int	    	  num_dirs PARAM_END

{
#define STANDARD_VIEW_BUTTONS 4

    WidgetList	    button_list = window->u.selection.view_menu_dir_buttons;
    BKR_DIR_ENTRY   *entry;
    int	    	    i;
    Arg	    	    arglist[5];
    KeySym  	    keysym_array[STANDARD_VIEW_BUTTONS];
    KeySym  	    *generated_keysyms;

    if ( num_dirs == 0 )
    	return;

    /* Get the key syms for the standard push buttons
     */
    XtSetArg( arglist[0], XmNmnemonic, &keysym_array[0] );
    XtGetValues( window->widgets[W_EXPAND_ENTRY], arglist, 1 );
    XtSetArg( arglist[0], XmNmnemonic, &keysym_array[1] );
    XtGetValues( window->widgets[W_EXPAND_ALL_ENTRY], arglist, 1 );
    XtSetArg( arglist[0], XmNmnemonic, &keysym_array[2] );
    XtGetValues( window->widgets[W_COLLAPSE_ENTRY], arglist, 1 );
    XtSetArg( arglist[0], XmNmnemonic, &keysym_array[3] );
    XtGetValues( window->widgets[W_COLLAPSE_ALL_ENTRY], arglist, 1 );

    /*  Uppercase the standard push button keysym for comparisons.
     */
    for ( i = 0; i < STANDARD_VIEW_BUTTONS; i++ ) 
    {
    	if ( islower( keysym_array[i] ) )
    	    keysym_array[i] = _toupper( keysym_array[i] );
    }

    /* Setup an array to hold the generated keysyms.
     */
    generated_keysyms = (KeySym *) BKR_CALLOC( num_dirs, sizeof( KeySym ) );
    memset( generated_keysyms, 0, num_dirs * sizeof( KeySym ) );

    /*  Generate the mnemonics for the dynamically created buttons.
     */
    for ( i = 0, entry = window->shell->book->directories;
    	    i < num_dirs; 
    	    i++, entry++ )
    {
    	int 	    j;
    	int 	    k;
    	Boolean     keysym_unique = FALSE;
    	char	    *upcase_title;
    	char	    *button_mnemonic;
    	char	    *ch;

    	if ( entry->title == NULL )
    	    continue;

    	upcase_title = (char *) BKR_MALLOC( strlen( entry->title ) + 1 );
    	for ( j = 0; j < strlen( entry->title ); j++ ) 
        {
    	    if ( islower( entry->title[j] ) )
	    	upcase_title[j] = _toupper( entry->title[j] );
    	    else
    	    	upcase_title[j] = entry->title[j]; /* just copy char */
    	}
    	upcase_title[ strlen( entry->title ) ] = '\0';

    	/*  Loop until we find a unique character to use as the mnemonic.
    	 */
    	for ( ch = upcase_title, button_mnemonic = entry->title;
    	    	( *ch != '\0' ); 
    	    	ch++, button_mnemonic++ )
    	{
    	    if ( isspace( *ch ) || ( ! isalnum( *ch ) ) )
    	    	continue;

    	    j = 0;
    	    while ( ( j < STANDARD_VIEW_BUTTONS ) && ( *ch != keysym_array[j] ) )
    	    	j++;
    	    k = 0;
    	    while ( ( k < i ) && ( *ch != generated_keysyms[k] ) )
    	    	k++;

    	    /* KeySym is unique.
    	     */
    	    if ( ( j == STANDARD_VIEW_BUTTONS ) && ( k == i ) )
    	    {
    	    	keysym_unique = TRUE;
    	    	break;
    	    }
    	}
    	if ( ( keysym_unique ) && ( *ch != '\0' ) )
    	{
    	    XtSetArg( arglist[0], XmNmnemonic, (KeySym) *button_mnemonic );
    	    XtSetValues( button_list[i], arglist, 1 );
    	    generated_keysyms[i] = (KeySym) *ch;    /* save the upper case version */
    	}
    	BKR_FREE( upcase_title );
    }

    BKR_CFREE( generated_keysyms );

};  /* end of add_mnemonics_to_buttons */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	create_pulldown_button
**
** 	Creates a push button for a pulldown menu given the parent
**  	menu, label, and callback routine and any user data.
**
**  FORMAL PARAMETERS:
**
**  	parent_widget - id of the parent pulldown menu.
**  	button_label  - label of the new push button.
**  	callback_proc - callback routine.
**  	callback_data - callback data.
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
static Widget
create_pulldown_button PARAM_NAMES((parent_widget,button_label,
                                    callback_proc,callback_data))
    Widget  	    parent_widget PARAM_SEP
    char    	    *button_label PARAM_SEP
    XtCallbackProc  callback_proc PARAM_SEP
    XtPointer	    callback_data PARAM_END

{
    XtCallbackRec   activate_callback[2];
    XmString	    cs_button_label = NULL;
    Arg     	    arglist[5];
    int     	    argcnt;
    unsigned	    status;
    MrmType 	    dummy_class;
    Widget  	    button;
    long	    byte_cnt, stat;	

    activate_callback[0].callback = (XtCallbackProc) callback_proc;
    activate_callback[0].closure  = (XtPointer) callback_data;
    activate_callback[1].callback = (XtCallbackProc) NULL;
    activate_callback[1].closure  = (XtPointer) NULL;

    cs_button_label = DXmCvtFCtoCS( button_label, &byte_cnt, &stat );
    argcnt = 0;
    SET_ARG( XmNlabelString, cs_button_label );
    SET_ARG( XmNactivateCallback, activate_callback );
    status = MrmFetchWidgetOverride(
    	    	    bkr_hierarchy_id,
    	    	    "pulldownMenuTemplateButton",   /* index in UID   	   */
    	    	    parent_widget,  	    	    /* parent widget  	   */
    	    	    NULL,	    	    	    /* don't override name */
    	    	    arglist,
    	    	    argcnt,
    	    	    &button,	    	    	    /* widget fetched 	   */
    	    	    &dummy_class ); 	    	    /* unused class        */
    if ( cs_button_label != NULL )
    	COMPOUND_STRING_FREE( cs_button_label );
    if ( status )
    {
    	XtManageChild( button );
    	return ( button );
    }
    else
    	return ( NULL );

};  /* end of create_pulldown_button */




