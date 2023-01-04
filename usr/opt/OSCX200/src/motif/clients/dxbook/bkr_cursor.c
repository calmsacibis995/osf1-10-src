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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_CURSOR.C*/
/* *10   18-NOV-1992 18:41:39 BALLENGER "Center CC WIP box."*/
/* *9    17-NOV-1992 22:49:22 BALLENGER "Just use title bar for CC WIP dialog."*/
/* *8     9-NOV-1992 19:14:34 BALLENGER "Fix working... message so it doesn't get focus."*/
/* *7     8-NOV-1992 19:21:29 BALLENGER "Use work in progress box instead of wait cursor on character cell."*/
/* *6     5-OCT-1992 11:27:11 KLUM "rename print widget id constants"*/
/* *5    10-JUL-1992 17:13:00 ROSE "Set/clear cursors on all dialog boxes"*/
/* *4     8-JUN-1992 19:02:45 BALLENGER "UCX$CONVERT"*/
/* *3     8-JUN-1992 12:38:35 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *2     3-MAR-1992 16:57:46 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:39:00 PARMENTER "Cursor Handling"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_CURSOR.C*/
#ifndef VMS
 /*
#else
#module BKR_CURSOR "V03-0002"
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
**	Cursor creation and display routines for the wait and inactive cursors.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     30-May-1990
**
**  MODIFICATION HISTORY:
**
**  V03-0001  DLB0001        David L Ballenger            04-Apr-1991
**            Fix reference to <DXm/DECspecific.h>.
**
**
**--
**/


/*
 * INCLUDE FILES
 */
#if FALSE
#include "br_api.h"          /* api (help) typedefs and #defines */
#endif
#include "br_common_defs.h"  /* common BR #defines (ON,OFF) */
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_globals.h"      /* BR external variables declared here */
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "bkr_cursor.h"      /* function prototypes for .c module */
#include "bkr_fetch.h"
#include <X11/Intrinsic.h>
#include <Xm/MwmUtil.h>
#ifdef __osf__
#include "XctStrings.h"
#else
#include <Xct/XctStrings.h>
#endif
/*
 * 
 */
typedef struct _INACTIVE_SHELL_ITEM
{
    	struct _INACTIVE_SHELL_ITEM 	*next;
    	Widget	    	    	    	shell;
}  INACTIVE_SHELL_ITEM;


/*
 * EXTERNAL ROUTINES
 */


/*
 * FORWARD DEFINITIONS 
 */
static void add_shell_to_inactive_list PROTOTYPE((Widget  shell));
static void remove_shell_from_inactive_list PROTOTYPE((Widget  shell));
static void toggle_inactive_for_dialogs ();

static Cursor  	    	    inactive_cursor_id = 0;
/* static Cursor    	    wait_cursor_id = 0; print_extract */
externaldef(bkrdata) Cursor wait_cursor_id = (Cursor)0;
static INACTIVE_SHELL_ITEM  *inactive_shell_list = NULL;

/* Variables for dealing with the work-in-progress dialog box.
 */

typedef struct {
    Boolean active;
    Widget  shell;
} WORK_IN_PROGRESS_REC;

static WORK_IN_PROGRESS_REC wip = { FALSE, NULL };

static XtResource wip_resource_list[] =
{
    { "active",
      "Active",
      XmRBoolean,
      sizeof(Boolean),
      XtOffsetOf(WORK_IN_PROGRESS_REC,active),
      XmRString,
      XtEtrue }
};


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_cc_working
**
** 	Displays or removes the work-in-progress box in character cell mode
**
**  FORMAL PARAMETERS:
**
**	onoff -	Boolean: whether to display or remove the wip box
**
**  IMPLICIT INPUTS:
**
**	wip - the WORK_IN_PROGRESS_REC
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
void bkr_cc_working PARAM_NAMES((onoff))
    Boolean onoff PARAM_END
{
    /* Use of the work-in-progress box can be turned of by setting
     * BookreaderCCWorking.active to false in the resource file.
     */
    if (wip.active) 
    {
        /* We simply realize of unrealize the widget as appropriate.
         * All we're really trying to do is display the titlebar which
         * contains the message/
         */
        if (onoff) 
        {
            XtRealizeWidget(wip.shell);        

            /* We need to make sure this is put on the screen
             * as soon as possible, otherwise some of the usefullness goes 
             * away.
             */
            XSync(bkr_display,FALSE);
        }
        else 
        {
            XtUnrealizeWidget(wip.shell);
        }
    }
}
static void remove_cc_wip_dialog PARAM_NAMES((widget,user_data,callback_data))
    Widget widget PARAM_SEP
    caddr_t user_data PARAM_SEP
    XmAnyCallbackStruct *callback_data PARAM_END
{
    bkr_cc_working(FALSE);
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_cursor_create_cursors
**
** 	Creates the wait and inactive cursors for the Bookreader application,
**      and in character cell mode it creates the work in progress dialog box
**      that is used in place of the cursors.
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
**	none
**
**--
**/
void
bkr_cursor_create_cursors( VOID_PARAM )
{
    if (bkrplus_g_charcell_display) 
    {
        String title;
        int wm_decorations;
        Dimension width;
        Position  x,y;
        

        static XtCallbackRec unmap_callback[] = { 
            { (XtCallbackProc)remove_cc_wip_dialog, (XtPointer) NULL },
            { (XtCallbackProc)NULL, (XtPointer)NULL }
        };

        title = bkr_fetch_literal("s_working",ASCIZ);
        if (title == NULL) {
            title = "Bookreader working...";
        }
        width = strlen(title)+8;

        x = ((bkr_display_width/bkr_default_space_width) - width)/2;
        y = ((bkr_display_height/bkr_default_line_height) - 1)/2;

        wip.shell = XtVaAppCreateShell("BookreaderCCWorking", "BookreaderCCWorking", 
                                       topLevelShellWidgetClass,
                                       bkr_display, 
                                       XmNwidth,     5,
                                       XmNheight,    5,
                                       XctNcx,       x,
                                       XctNcy,       y,
                                       XctNcwidth,   width,
                                       XctNcheight,  1,
                                       XmNiconName,  title,
                                       XmNtitle,     title,
                                       XmNinput,     FALSE,
                                       XmNsaveUnder, TRUE,
                                       XmNmwmDecorations, MWM_DECOR_TITLE,
                                       NULL);
        
        if (wip.shell) 
        {
            /* See if the resource file specifies that the work in progres box
             * is to be used.
             */
            XtGetApplicationResources(wip.shell, 
                                      &wip,
                                      wip_resource_list, 
                                      XtNumber(wip_resource_list), 
                                      NULL, 0 );
        }
    }
    wait_cursor_id     = DXmCreateCursor( bkr_toplevel_widget, 
                                         DXm_WAIT_CURSOR );
    inactive_cursor_id = DXmCreateCursor( bkr_toplevel_widget, 
                                             DXm_INACTIVE_CURSOR );
};  /* end of bkr_cursor_create_cursors */

static void
toggle_wait_cursor PARAM_NAMES((window,onoff))
    BKR_WINDOW *window PARAM_SEP
    Boolean onoff PARAM_END
{
    if ((window && window->active) || (window == bkr_library)) {

	if (window->appl_shell_id && XtIsRealized(window->appl_shell_id) ) {
	    if ( onoff )
		XDefineCursor (bkr_display, XtWindow( window->appl_shell_id ), 
			       wait_cursor_id );
	    else
		XUndefineCursor (bkr_display, XtWindow (window->appl_shell_id));
	}

	/* Set or clear the watch cursor on the top level print dialog */
	if (window->widgets[W_PR_BOX] != NULL &&
	    XtIsRealized (window->widgets[W_PR_BOX]) &&
	    XtIsManaged (window->widgets[W_PR_BOX])) {
    	    if ( onoff )
    	    	XDefineCursor (bkr_display,
			       XtWindow (window->widgets[W_PR_BOX]),
			       wait_cursor_id);
    	    else
    	    	XUndefineCursor (bkr_display,
				 XtWindow (window->widgets[W_PR_BOX]));
	}

	/* Set or clear the watch cursor on the print widget dialog */
	if (window->widgets[W_PR_OPTIONS_BOX] != NULL &&
	    XtIsRealized (window->widgets[W_PR_OPTIONS_BOX]) &&
	    XtIsManaged (window->widgets[W_PR_OPTIONS_BOX])) {
    	    if ( onoff )
    	    	XDefineCursor (bkr_display,
			       XtWindow (window->widgets[W_PR_OPTIONS_BOX]),
			       wait_cursor_id);
    	    else
    	    	XUndefineCursor (bkr_display,
				 XtWindow (window->widgets[W_PR_OPTIONS_BOX]));
	}

	/* Set or clear the watch cursor on the search dialog */
	if (window->widgets[W_SEARCH_BOX] != NULL &&
	    XtIsRealized (window->widgets[W_SEARCH_BOX]) &&
	    XtIsManaged (window->widgets[W_SEARCH_BOX])) {

	    if ( onoff )
		XDefineCursor (bkr_display,
			       XtWindow (window->widgets[W_SEARCH_BOX]),
			       wait_cursor_id);
	    else
		XUndefineCursor (bkr_display,
				 XtWindow (window->widgets[W_SEARCH_BOX]));
	}

	/* Set or clear the watch cursor on the search results dialog */
	if (window->widgets[W_SEARCH_RESULTS_BOX] != NULL &&
	    XtIsRealized (window->widgets[W_SEARCH_RESULTS_BOX]) &&
	    XtIsManaged (window->widgets[W_SEARCH_RESULTS_BOX])) {

	    if ( onoff )
		XDefineCursor (bkr_display,
			       XtWindow (window->widgets[W_SEARCH_RESULTS_BOX]),
			       wait_cursor_id);
	    else
		XUndefineCursor (bkr_display,
				XtWindow (window->widgets[W_SEARCH_RESULTS_BOX]));
	}
    }
}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_cursor_display_wait
**
** 	Displays or removes the wait cursor from all Bookreader active windows.
**
**  FORMAL PARAMETERS:
**
**	onoff -	Boolean: whether to display or remove the wait cursor.
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
bkr_cursor_display_wait PARAM_NAMES((onoff))
    Boolean onoff PARAM_END
{
    BKR_SHELL	    	*shell;
    INACTIVE_SHELL_ITEM	*current;

    if ((wait_cursor_id == 0) || (bkrplus_g_charcell_display)) 
    	return;

    if ( bkr_library != NULL )
	toggle_wait_cursor (bkr_library, onoff);

    shell = bkr_all_shells;
    while (shell) {
        BKR_WINDOW *window;
        
        toggle_wait_cursor(shell->selection,onoff);
        toggle_wait_cursor(shell->default_topic,onoff);
        
        window = shell->other_topics;
        while (window) {
            toggle_wait_cursor(window,onoff);
            window = window->u.topic.sibling;
        }
        shell = shell->all_shells;
    }

    /*  Redisplay the INACTIVE cursors.
     */
    if ( onoff == OFF )
    {
    	for ( current = inactive_shell_list; current != NULL; 
    	    	current = current->next )
    	{
    	    bkr_cursor_display_inactive( current->shell, ON );
    	}
    }

    XFlush( bkr_display );

};  /* end of bkr_cursor_display_wait */

static void
toggle_inactive_cursor PARAM_NAMES((shell_id,onoff))
    Widget shell_id PARAM_SEP 
    Boolean onoff PARAM_END
{
    if ( shell_id != NULL )
    {
    	if ( XtIsRealized( shell_id ) )
    	{
    	    if ( onoff )
    	    {
    	    	XDefineCursor( bkr_display, XtWindow( shell_id ), 
    	    	    	    	    inactive_cursor_id );
    	    	add_shell_to_inactive_list( shell_id );
    	    }
    	    else
    	    {
    	    	XUndefineCursor( bkr_display, XtWindow( shell_id ) );
    	    	remove_shell_from_inactive_list( shell_id );
    	    }
    	}
    }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_cursor_display_inactive
**
** 	Displays or removes the inactive cursor from the Library window.
**
**  FORMAL PARAMETERS:
**
**  	shell_id - id of the widget to display the inactive cursor in.
**	onoff 	 - Boolean: whether to display or remove the inactive cursor.
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
bkr_cursor_display_inactive PARAM_NAMES((shell_id,onoff))
    Widget shell_id PARAM_SEP
    Boolean onoff PARAM_END
{
    if (( inactive_cursor_id == 0 ) || (bkrplus_g_charcell_display)) 
    	return;

    toggle_inactive_cursor(shell_id,onoff);

    XFlush( bkr_display );

};  /* end of bkr_cursor_display_inactive */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_cursor_display_inactive_all
**
** 	Displays or removes the inactive cursor from all Bookreader active windows.
**
**  FORMAL PARAMETERS:
**
**	onoff -	Boolean: whether to display or remove the inactive cursor.
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
bkr_cursor_display_inactive_all PARAM_NAMES((onoff))
    Boolean onoff PARAM_END
{
    BKR_SHELL	    	*shell;
    BKR_SHELL	    	*topic_shell;
    INACTIVE_SHELL_ITEM	*current;

    if ( inactive_cursor_id == 0 )
    	return;

    if ( bkr_library != NULL ) {
        toggle_inactive_cursor(bkr_library->appl_shell_id,onoff);

	/* Set or clear the inactive cursor on all the dialogs */
	toggle_inactive_for_dialogs (bkr_library, onoff);
    }

    shell = bkr_all_shells;
    while (shell) {
        
        BKR_WINDOW *window;

        if (shell->selection) {    
            toggle_inactive_cursor(shell->selection->appl_shell_id,onoff);

	    /* Set or clear the inactive cursor on all the dialogs */
	    toggle_inactive_for_dialogs (shell->selection, onoff);
        }
        if (shell->default_topic) {
            toggle_inactive_cursor(shell->default_topic->appl_shell_id,onoff);

	    /* Set or clear the inactive cursor on all the dialogs */
	    toggle_inactive_for_dialogs (shell->default_topic, onoff);
        }

        window = shell->other_topics;
        while (window) {
            toggle_inactive_cursor(window->appl_shell_id,onoff);

	    /* Set or clear the inactive cursor on all the dialogs */
	    toggle_inactive_for_dialogs (window, onoff);

            window = window->u.topic.sibling;
        }
        shell = shell->all_shells;
    }

    XFlush( bkr_display );

};  /* end of bkr_cursor_display_inactive_all */


static void
add_shell_to_inactive_list PARAM_NAMES((shell))
    Widget  shell PARAM_END
{
    INACTIVE_SHELL_ITEM	    *current;
    INACTIVE_SHELL_ITEM	    *new_shell;

    /*  Only add the shell to the list if its not already there.
     */
    for ( current = inactive_shell_list; current != NULL; 
    	    current = current->next )
    {
    	if ( current->shell == shell )
    	    return;
    }

    new_shell = (INACTIVE_SHELL_ITEM *) BKR_MALLOC( sizeof( INACTIVE_SHELL_ITEM	) );
    new_shell->next = NULL;
        new_shell->shell = shell;

    new_shell->next = inactive_shell_list;
    inactive_shell_list = new_shell;

};  /* end of add_shell_to_inactive_list */


static void
remove_shell_from_inactive_list PARAM_NAMES((shell))
    Widget  shell PARAM_END

{
    INACTIVE_SHELL_ITEM	    *current;
    INACTIVE_SHELL_ITEM	    *previous = NULL;

    current = inactive_shell_list;
    while ( current != NULL )
    {
    	if ( current->shell == shell )
    	{
    	    if ( previous != NULL )
    	    	previous->next = current->next;
    	    else
    	    	inactive_shell_list = current->next;
    	    BKR_FREE( current );
    	    break;
    	}
    	previous = current;
    	current = current->next;
    }

};  /* end of remove_shell_from_inactive_list */



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	toggle_inactive_for_dialogs 
**
** 	This routine toggles the inactive cursor on and off for all of
**	Bookreader's dialog boxes.
**
**  FORMAL PARAMETERS:
**	BKR_WINDOW	*window;	- Pointer to window structure
**	Boolean		onoff;		- Flag indicating on or off
**
**  IMPLICIT INPUTS:
**	Cursor		inactive_cursor_id;
**
**  IMPLICIT OUTPUTS:
**	None
**
**  COMPLETION CODES:
**	None
**
**  SIDE EFFECTS:
**	None
**--
**/
static void toggle_inactive_for_dialogs (window, onoff)
    BKR_WINDOW	*window;
    Boolean	onoff;
{
    if (!window->active && window != bkr_library)
	return;

    /* Set or clear the inactive cursor on the top level print dialog */
    if (window->widgets[W_PR_BOX] != NULL &&
	XtIsRealized (window->widgets[W_PR_BOX]) &&
	XtIsManaged (window->widgets[W_PR_BOX])) {

    	if ( onoff ) {
    	    XDefineCursor (bkr_display,
			   XtWindow (window->widgets[W_PR_BOX]),
			   inactive_cursor_id);
	    add_shell_to_inactive_list (window->widgets[W_PR_BOX]);
	}
    	else {
    	    XUndefineCursor (bkr_display,
			     XtWindow (window->widgets[W_PR_BOX]));
    	    remove_shell_from_inactive_list (window->widgets[W_PR_BOX]);
	}
    }

    /* Set or clear the inactive cursor on the print widget dialog */
    if (window->widgets[W_PR_OPTIONS_BOX] != NULL &&
	XtIsRealized (window->widgets[W_PR_OPTIONS_BOX]) &&
	XtIsManaged (window->widgets[W_PR_OPTIONS_BOX])) {

	if ( onoff ) {
	    XDefineCursor (bkr_display,
			   XtWindow (window->widgets[W_PR_OPTIONS_BOX]),
			   inactive_cursor_id);
	    add_shell_to_inactive_list (window->widgets[W_PR_OPTIONS_BOX]);
	}
    	else {
	    XUndefineCursor (bkr_display,
			     XtWindow (window->widgets[W_PR_OPTIONS_BOX]));
	    remove_shell_from_inactive_list (window->widgets[W_PR_OPTIONS_BOX]);
	}
    }

    /* Set or clear the inactive cursor on the search dialog */
    if (window->widgets[W_SEARCH_BOX] != NULL &&
	XtIsRealized (window->widgets[W_SEARCH_BOX]) &&
	XtIsManaged (window->widgets[W_SEARCH_BOX])) {

	if ( onoff ) {
	    XDefineCursor (bkr_display,
			   XtWindow (window->widgets[W_SEARCH_BOX]),
			   inactive_cursor_id);
	    add_shell_to_inactive_list (window->widgets[W_SEARCH_BOX]);
	}
    	else {
	    XUndefineCursor (bkr_display,
			     XtWindow (window->widgets[W_SEARCH_BOX]));
	    remove_shell_from_inactive_list (window->widgets[W_SEARCH_BOX]);
	}
    }

    /* Set or clear the inactive cursor on the search dialog */
    if (window->widgets[W_SEARCH_RESULTS_BOX] != NULL &&
	XtIsRealized (window->widgets[W_SEARCH_RESULTS_BOX]) &&
	XtIsManaged (window->widgets[W_SEARCH_RESULTS_BOX])) {

	if ( onoff ) {
	    XDefineCursor (bkr_display,
			   XtWindow (window->widgets[W_SEARCH_RESULTS_BOX]),
			   inactive_cursor_id);
	    add_shell_to_inactive_list (window->widgets[W_SEARCH_RESULTS_BOX]);
	}
    	else {
	    XUndefineCursor (bkr_display,
			     XtWindow (window->widgets[W_SEARCH_RESULTS_BOX]));
	    remove_shell_from_inactive_list (window->widgets[W_SEARCH_RESULTS_BOX]);
	}
    }
}
