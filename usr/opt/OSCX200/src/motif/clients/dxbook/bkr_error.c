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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_ERROR.C*/
/* *17   17-NOV-1992 22:49:49 BALLENGER "Fix error dialog so that previous window regains focus."*/
/* *16   13-AUG-1992 15:09:37 GOSSELIN "updating with necessary A/OSF changes"*/
/* *15   24-JUL-1992 10:15:19 ROSE "Xt warning messages accumulated during Search"*/
/* *14   19-JUN-1992 19:04:43 BALLENGER "Put int missing } in if statement."*/
/* *13   19-JUN-1992 13:26:30 BALLENGER "Conditionalize use of Rags."*/
/* *12   17-JUN-1992 16:59:22 ROSE "No longer display error messages during Search; oinstead, store messages away until Search*/
/*completes"*/
/* *11    8-JUN-1992 19:08:06 BALLENGER "UCX$CONVERT"*/
/* *10    8-JUN-1992 12:53:14 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *9    16-MAR-1992 15:56:02 BALLENGER "run ucx$convert"*/
/* *8    10-MAR-1992 14:57:23 GOSSELIN "fixed one last problem - really"*/
/* *7    10-MAR-1992 14:45:06 GOSSELIN "added RAGS support"*/
/* *6    10-MAR-1992 13:23:17 GOSSELIN "fixing screwup"*/
/* *5    10-MAR-1992 11:05:13 GOSSELIN "finished"*/
/* *4     3-MAR-1992 16:58:32 KARDON "UCXed"*/
/* *3    13-FEB-1992 18:32:19 BALLENGER "Fix problems with error messages for API."*/
/* *2    17-SEP-1991 19:50:19 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:39:15 PARMENTER "Error handling"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_ERROR.C*/
#ifndef VMS
 /*
#else
# module BKR_ERROR "V03-0003"
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
**	Bookreader error handling routines.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     25-Apr-1990
**
**  MODIFICATION HISTORY:
**
**      V03-0003    DLB0002     David L Ballenger       04-Apr-1991
**                  Fix reference to <Xm/Xm.h>.
**
**      V03-0002    DLB0001     David L Ballenger       06-Feb-1991
**                  Include <Xm/MessageB.h> to pick up correct
**                  function declarations.  
**
**                  Modify bkr_error_modal to expect two arguments 
**                  rather than a variable number, since va_count()
**                  is not portable.
**
**	V03-0001    JAF0001	James A. Ferguson   	25-Apr-1990
**  	    	    Extracted V2 routines and created new module.
**
**--
**/


/*
 * INCLUDE FILES
 */
#include <X11/Intrinsic.h>
#include <X11/Vendor.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <Xm/Label.h>
#include <Xm/MessageB.h>
#include <string.h>
#include <DXm/DECspecific.h>
#include <Mrm/MrmPublic.h>
#include <stdio.h>
#include "br_common_defs.h"  /* common BR #defines */
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_globals.h"      /* BR external variables declared here */
#include "bkr_error.h"       /* function prototypes for .c module */
#include "bkr_fetch.h"      /* BR external variables declared here */
#include "bkr_cursor.h"      /* Cursor routines */
#include "bkr_window.h"      /* Window handling routines */
#include "br_resource.h"     /* typedefs and #defines for Xrm customizable
                                resources used in BR */
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "bkr_client_server.h" /* function prototypes for .c module */
#ifndef NO_RAGS
#include    "ge.h"		/* RAGS globals */
#endif 





/*
 * FORWARD ROUTINES
 */

static void	    error_dialog_client_msg();


/*
 * FORWARD DEFINITIONS
 */

static Widget	    	    modal_error_dialog = NULL;
static Widget	    	    current_parent_error_shell = NULL;


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_error_modal
**
** 	A simplified interface to bkr_error_modal for simple errors,
**	that only need to be fetch to be displayed.
**
**  FORMAL PARAMETERS:
**
**	window - the BKR_WINDOW pointer for the parent window or NULL
**
**  	error  - the error to fetch from the uid file
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
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
void
bkr_error_simple_msg PARAM_NAMES((window,error))
    BKR_WINDOW *window PARAM_SEP
    char *error  PARAM_END
{
    char	*error_string;
    Window      window_id = NULL;

    if (window) {
        window_id = XtWindow( window->appl_shell_id  );
    }
    error_string = bkr_fetch_literal(error,ASCIZ) ;

    if (error_string) 
    {
        bkr_error_modal( error_string,window_id);
        XtFree( error_string );
    }
    else 
    {
        char buffer[100];

        sprintf(buffer,"Missing descriptive text for error: %s\n",error);
        bkr_error_modal(buffer,window_id);
    }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_error_modal
**
** 	Displays a modal informational dialog box given an error message.
**
**  FORMAL PARAMETERS:
**
**	Variable list of arguments, the first is always:
**
**  	    error_msg - pointer to the error message to display.
**
**  	The second parameter (optional) is:
**
**  	    window_id - id of a window to used as the parent of the message box.
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
bkr_error_modal PARAM_NAMES((error_msg,window_id))
    char    	    *error_msg PARAM_SEP
    Window  	    window_id PARAM_END

{
    Widget  	    parent_widget = NULL;
    unsigned	    status;
    MrmType 	    dummy_class;
    int	    	    argcnt;
    Arg	    	    arglist[15];
    XmString	    cs_error_msg;
    Widget  	    cancel_button;
    Widget  	    help_button;
    long     	    byte_cnt, stat;
    BKR_WINDOW	    *book_window;

    /* If a search is in progress, don't display this box */
#ifdef SEARCH
    if (bkrplus_g_search_in_progress) {

	/* Add the error message to the Search message box */
	bkr_accumulate_search_messages (error_msg);

	return;
    }
#endif /* SEARCH */

    argcnt = 0;

    /* Get the error message and optional window id 
     */
    if (window_id) 
    {
        parent_widget = XtWindowToWidget( bkr_display, window_id );
        RAISE_WINDOW(parent_widget);
        SET_ARG( XmNdialogStyle,XmDIALOG_PRIMARY_APPLICATION_MODAL);
    } 
    else 
    {
        parent_widget = current_parent_error_shell;
        if (parent_widget) 
        {
            RAISE_WINDOW(parent_widget);
            SET_ARG( XmNdialogStyle,XmDIALOG_PRIMARY_APPLICATION_MODAL);
        }
        else 
        {
            SET_ARG( XmNdialogStyle,XmDIALOG_FULL_APPLICATION_MODAL);
            parent_widget = bkr_toplevel_widget;
            if ( parent_widget == NULL ) 
            {
                parent_widget = XtWindowToWidget( bkr_display,
                                                 DefaultRootWindow(bkr_display) );
            }
        }
    }

    cs_error_msg = DXmCvtFCtoCS( error_msg, &byte_cnt, &stat );
    SET_ARG( XmNmessageString, cs_error_msg );
    SET_ARG( XmNsaveUnder, TRUE );
#ifdef  FOO
    SET_ARG( XmNoverrideRedirect, TRUE );
#endif 
    status = MrmFetchWidgetOverride(
    	    	bkr_hierarchy_id,
    	    	"modalMessageBox",  	    	/* index into UIL       */
    	    	parent_widget,	    	    	/* parent widget        */
    	    	NULL,	    	    	    	/* don't override name	*/
    	    	arglist,
    	    	argcnt,
    	    	&modal_error_dialog,	    	/* widget being fetched */
    	    	&dummy_class );	    	    	/* unused class         */
    COMPOUND_STRING_FREE( cs_error_msg );
    if ( status != MrmSUCCESS )
    {
	fprintf( stderr, "Can't fetch message box \n" );
    	return;
    }

    /* Get rid of the Cancel and Help buttons */

    if ( cancel_button = XmMessageBoxGetChild( modal_error_dialog, 
    	    	    	    	    XmDIALOG_CANCEL_BUTTON ) )
    	XtUnmanageChild( cancel_button );
    if ( help_button = XmMessageBoxGetChild( modal_error_dialog, 
    	    	    	    	    XmDIALOG_HELP_BUTTON ) )
    	XtUnmanageChild( help_button );

    bkr_window_setup( modal_error_dialog, parent_widget, TRUE );

    /*  Manage the dialog box and set all windows cursors to inactive.
     */
    bkr_cursor_display_inactive_all( ON );
    XtManageChild( modal_error_dialog );

    /*  Setup to receive client messages just in case the user closes
     *  the dialog box by using the window menu instead of the "OK" 
     *  button.  We need to do some cleanup.
     */
    XtAddEventHandler( XtParent( modal_error_dialog ), NoEventMask, 
		       TRUE, error_dialog_client_msg, modal_error_dialog );   
                                              
};  /* end of bkr_error_modal */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_error_set_parent_shell
**
** 	Saves the id of a shell to be used when creating messages boxes.
**
**  FORMAL PARAMETERS:
**
**	shell_id - id of the shell to be saved.
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
bkr_error_set_parent_shell PARAM_NAMES((shell_id))
    Widget  shell_id PARAM_END
{

    if ( shell_id != NULL )
    {
    	if ( XtWindow( shell_id ) != 0 )
    	    current_parent_error_shell = shell_id;
    	else
    	    current_parent_error_shell = NULL;
    }
    else
    	current_parent_error_shell = NULL;

};  /* end of bkr_error_set_parent_shell */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_error_unmap
**
** 	Callback routine for handling the "OK" callback from an
**  	error dialog box.
**
**  FORMAL PARAMETERS:
**
**	widget  - id of the widget that caused the event.
**	tag 	- user data.
**	reason	- pointer to callback data (not used).
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
*/
void
bkr_error_unmap PARAM_NAMES((widget,tag,reason))
    Widget  	    	widget PARAM_SEP
    caddr_t 	    	tag PARAM_SEP
    XmAnyCallbackStruct	*reason PARAM_END

{
    XtDestroyWidget( widget );
    bkr_cursor_display_inactive_all( OFF );

};  /* end of bkr_error_unmap */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_error_xlib_non_fatal
**
** 	Xlib non-fatal error handler routine.
**
**  FORMAL PARAMETERS:
**
**	display - pointer to the display variable for the application.
**  	event   - pointer to the event which contains the error.
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
**	Returns:    Routine MUST return 0. 
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
int
bkr_error_xlib_non_fatal PARAM_NAMES((display,event))
    Display	    *display PARAM_SEP
    XErrorEvent	    *event PARAM_END

{
    char    errbuff[ERROR_BUFFER_SIZE];
    char    *mtype = "XlibMessage";

    /* Set up error notification so that the RAGS code is aware of errors */
#ifndef NO_RAGS
    geErr = event->error_code;
#endif

#ifdef DO_LATER  /********** what to do for BadAlloc errors ???? ******/
    /*
     *  BadAlloc error received;  we will close all fonts and then re-initialize
     *  our font data structure because the close free's our structure and
     *  then zeros the pointer.
     */

    if ( event->error_code == BadAlloc )
    {
	voila$font_data_close ();
	voila$font_data_init ();
	return (0);
    }
#endif /* DO_LATER */

    /* Write out the error message */

    XGetErrorText( display, event->error_code, errmsg, ERROR_BUFFER_SIZE );
    XGetErrorDatabaseText( display, mtype, "XError", 
    	"X Error event received", errbuff, ERROR_BUFFER_SIZE );
    fprintf( stderr, "%s:  %s \n  ", errbuff, errmsg );

    XGetErrorDatabaseText( display, mtype, "MajorCode", 
    	"Request Major op code %d", errmsg, ERROR_BUFFER_SIZE );
    fprintf( stderr, errmsg, event->request_code );
    sprintf( errbuff, "%d", event->request_code );
    XGetErrorDatabaseText( display, "XRequest", errbuff, 
    	"", errmsg, ERROR_BUFFER_SIZE );
    fprintf( stderr, " (%s)", errmsg );
    fputs( "\n  ", stderr );

    XGetErrorDatabaseText( display, mtype, "MinorCode", 
    	"Request Minor op code %d", errmsg, ERROR_BUFFER_SIZE );
    fprintf( stderr, errmsg, event->minor_code );
    fputs( "\n  ", stderr );

    XGetErrorDatabaseText( display, mtype, "ResourceID", 
    	"ResourceID 0x%x", errmsg, ERROR_BUFFER_SIZE );
    fprintf( stderr, errmsg, event->resourceid );
    fputs( "\n  ", stderr );

    XGetErrorDatabaseText( display, mtype, "ErrorSerial", 
    	"Error Serial #%d", errmsg, ERROR_BUFFER_SIZE );
    fprintf( stderr, errmsg, event->serial );
    fputs( "\n  ", stderr );

#ifdef VMS /* waiting for fix from ALPHA/OSF */
    XGetErrorDatabaseText( display, mtype, "CurrentSerial", 
    	"Current Serial #%d", errmsg, ERROR_BUFFER_SIZE );
    fprintf( stderr, errmsg, display->request );
    fputs( "\n", stderr );
#endif /* waiting for fix from ALPHA/OSF */

    return ( 0 );	    /* must return 0 */

};  /* end of bkr_error_xlib_non_fatal */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_error_xt_warning_handler
**
** 	Callback routine for displaying Xt warning messages.
**
**  FORMAL PARAMETERS:
**
**	error_msg - pointer to the error message to display.
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
**	We set a flag to avoid displaying recursive message boxes.
**
**--
**/
void
bkr_error_xt_warning_handler PARAM_NAMES((error_msg))
    char    *error_msg PARAM_END

{
    unsigned	    status;
    MrmType 	    dummy_class;
    Arg	    	    arglist[5];
    int	    	    argcnt;
    XmString	    cs_error_msg;
    static Boolean  display_in_progess = FALSE;
    Widget  	    cancel_button;
    Widget  	    help_button;
    long     	    byte_cnt, stat;

    /* If a search is in progress, don't display this box, simply add
       the message to the Search Messages box */
#ifdef SEARCH
    if (bkrplus_g_search_in_progress) {
                                                   
	/* Add the error message to the Search message box */
	bkr_accumulate_search_messages (error_msg);

	return;
    }
#endif /* SEARCH */

    if ( display_in_progess )
    	return;
    display_in_progess = TRUE;

    cs_error_msg = DXmCvtFCtoCS( error_msg, &byte_cnt, &stat );
    argcnt = 0;
    SET_ARG( XmNmessageString, cs_error_msg );
    status = MrmFetchWidgetOverride(
    	    	bkr_hierarchy_id,
    	    	"modalMessageBox",  	    	/* index into UIL       */
    	    	bkr_toplevel_widget,	    	/* parent widget        */
    	    	NULL,	    	    	    	/* don't override name	*/
    	    	arglist,
    	    	argcnt,
    	    	&modal_error_dialog,	    	/* widget being fetched */
    	    	&dummy_class );	    	    	/* unused class         */
    COMPOUND_STRING_FREE( cs_error_msg );

    if ( status != MrmSUCCESS )
    {
	fprintf( stderr, "Can't fetch message box \n" );
    	return;
    }

    /* Get rid of the Cancel and Help buttons */

    if ( cancel_button = XmMessageBoxGetChild( modal_error_dialog, 
    	    	    	    	    XmDIALOG_CANCEL_BUTTON ) )
    	XtUnmanageChild( cancel_button );
    if ( help_button = XmMessageBoxGetChild( modal_error_dialog, 
    	    	    	    	    XmDIALOG_HELP_BUTTON ) )
    	XtUnmanageChild( help_button );

    bkr_window_setup( modal_error_dialog, bkr_toplevel_widget, FALSE );

    XtManageChild( modal_error_dialog );

    /* The message has been displayed */

    display_in_progess = FALSE;

};  /* end of bkr_error_xt_warning_handler */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	error_dialog_client_msg
**
** 	Callback routine which handles DELETE_WINDOW Client Messages
**  	for Error dialog boxes.
**
**  FORMAL PARAMETERS:
**
**	dialog_shell - id of the dialog shell widget which received the message.
**	error_dialog - id of the error dialog box.
**	event	     - pointer to the X event.
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
error_dialog_client_msg PARAM_NAMES(( dialog_shell, error_dialog, event ))
    Widget  	    dialog_shell PARAM_SEP
    Widget 	    error_dialog PARAM_SEP
    XEvent  	    *event PARAM_END
{
    XClientMessageEvent	*cmevent = (XClientMessageEvent *)event;

    if ( event->type != ClientMessage )
    	return;

    if ( cmevent->message_type == WM_PROTOCOLS_ATOM )
    {
    	if ( cmevent->data.l[0] == WM_DELETE_WINDOW_ATOM )
    	{
    	    XmAnyCallbackStruct	reason;
	    caddr_t 	    	tag;

    	    bkr_error_unmap( error_dialog, tag, &reason );
    	}
    }

};  /* end of error_dialog_client_msg */



