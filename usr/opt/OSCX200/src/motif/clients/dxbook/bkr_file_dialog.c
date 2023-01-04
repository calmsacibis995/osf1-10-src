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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_FILE_DIALOG.C*/
/* *12    1-FEB-1993 15:45:08 RAMSHAW "Int QAR #14 - Fix default filter extension"*/
/* *11   25-JAN-1993 15:40:03 RAMSHAW "Improve test for no filename in file selection widget"*/
/* *10   30-OCT-1992 18:34:50 BALLENGER "Fix problem with reopening default library."*/
/* *9    19-AUG-1992 14:13:56 KLUM "improve error msg for empty filename string"*/
/* *8    12-AUG-1992 09:43:45 KLUM "fix crash on open book with nul filename"*/
/* *7    19-JUN-1992 20:19:29 BALLENGER "Cleanup for Alpha/OSF port."*/
/* *6     8-JUN-1992 19:10:50 BALLENGER "UCX$CONVERT"*/
/* *5     8-JUN-1992 12:58:20 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *4     3-MAR-1992 16:58:54 KARDON "UCXed"*/
/* *3     1-NOV-1991 13:05:00 BALLENGER "reintegrate  memex support"*/
/* *2    17-SEP-1991 19:59:28 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:39:20 PARMENTER "File dialog box creation"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_FILE_DIALOG.C*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_FILE_DIALOG.C*/
/* *11   25-JAN-1993 15:40:03 RAMSHAW "Improve test for no filename in file selection widget"*/
/* *10   30-OCT-1992 18:34:50 BALLENGER "Fix problem with reopening default library."*/
/* *9    19-AUG-1992 14:13:56 KLUM "improve error msg for empty filename string"*/
/* *8    12-AUG-1992 09:43:45 KLUM "fix crash on open book with nul filename"*/
/* *7    19-JUN-1992 20:19:29 BALLENGER "Cleanup for Alpha/OSF port."*/
/* *6     8-JUN-1992 19:10:50 BALLENGER "UCX$CONVERT"*/
/* *5     8-JUN-1992 12:58:20 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *4     3-MAR-1992 16:58:54 KARDON "UCXed"*/
/* *3     1-NOV-1991 13:05:00 BALLENGER "reintegrate  memex support"*/
/* *2    17-SEP-1991 19:59:28 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:39:20 PARMENTER "File dialog box creation"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_FILE_DIALOG.C*/
#ifndef VMS
 /*
#else
#module BKR_FILE_DIALOG "V03-0002"
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
**	Miscellaneous dialog box creation routines.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     4-Jun-1990
**
**  MODIFICATION HISTORY:
**
**  V03-0002    DLB0002		David L Ballenger	13-Feb-1991
**              Test for pointer to NULL string in addition to a NULL
**              pointer for the target_name.
**
**  V03-0001    DLB0001	David L Ballenger	06-Feb-1991
**              Include <Xm/FileSB.h> to pick up corrrect function 
**              declarations.
**
**--
**/


/*
 * INCLUDE FILES
 */

#include    <Xm/FileSB.h>
#include    <DXm/DECspecific.h>
#include    "br_common_defs.h"  /* common BR #defines */
#include    "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include    "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include    "br_globals.h"      /* BR external variables declared here */
#include    "bkr_file_dialog.h" /* function prototypes for .c module */
#include    "bkr_cursor.h"      /* function prototypes for .c module */
#include    "bkr_error.h"       /* function prototypes for .c module */
#include    "bkr_fetch.h"       /* function prototypes for .c module */
#include    "bkr_library.h"     /* function prototypes for .c module */
#include    "bkr_selection_open.h" /* function prototypes for .c module */
#include    "bkr_window.h"      /* function prototypes for .c module */



/*
 * FORWARD ROUTINES
 */
static unsigned	    create_confirm_open_dialog();
static unsigned	    create_file_selection_dialog();


/*
 * FORWARD DEFINITIONS 
 */

static Widget	    	    file_selection_dialog = NULL;
static Widget	    	    confirm_open_dialog = NULL;
static unsigned char	    previous_fileselect_type = 0;
static unsigned char	    fileselect_type = 0;
static XmString	    	    open_library_title = NULL;
static XmString	    	    open_book_title = NULL;
static char 	    	    *target_file = NULL;
static Boolean	    	    confirm_in_progress = FALSE;


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_file_dialog_cancel
**
** 	Routine for handling the "Cancel" callback for the file selection widget
**  	or the confirm dialog box widget.
**
**  FORMAL PARAMETERS:
**
**  	widget	    - id of the file selection or confirm dialog box widgets.
**  	tag 	    - pointer to the user data.
**	fileselect  - pointer to push button callback data.
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
bkr_file_dialog_cancel PARAM_NAMES((widget,tag,fileselect))
    Widget		    	     widget PARAM_SEP
    int	    	    	    	     *tag PARAM_SEP
    XmFileSelectionBoxCallbackStruct *fileselect PARAM_END

{

    confirm_in_progress = FALSE;

    if ( target_file != NULL )
    {
    	XtFree( target_file );
    	target_file = NULL;
    }
    XtUnmanageChild( widget );	    /* either file selection or confirm dialog */
    bkr_cursor_display_inactive( bkr_library->appl_shell_id, OFF );

};  /* end of bkr_file_dialog_cancel */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_file_dialog_confirm_ok
**
** 	Callback routine for the "ok" button of the confirm dialog box.
**
**  FORMAL PARAMETERS:
**
**  	widget	    	- id of the push button.  NOTE: NULL is sometimes passed.
**  	tag 	        - user data.
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
bkr_file_dialog_confirm_ok PARAM_NAMES((widget,tag,callback_data))
    Widget		    widget PARAM_SEP
    int	    	    	    *tag PARAM_SEP   	    	/* unused */
    XmAnyCallbackStruct     *callback_data PARAM_END 	/* unused */
{
    unsigned	    	status;

    confirm_in_progress = FALSE;

    bkr_cursor_display_inactive( bkr_library->appl_shell_id, OFF );

    if ( (target_file == NULL) || (*target_file == '\000') )
    	return;

    status = bkr_library_open_new_library( target_file );

    /* Opened successfully, so gray out the appropriate buttons */

    if ( status )
    {
    	if ( fileselect_type == K_OPEN_DEFAULT_LIBRARY )
    	    XtSetSensitive( bkr_library->widgets[W_OPEN_DEFAULT_LIBRARY_ENTRY], FALSE );
    	else if ( fileselect_type == K_OPEN_LIBRARY_FILE )
    	    XtSetSensitive( bkr_library->widgets[W_OPEN_DEFAULT_LIBRARY_ENTRY], TRUE );
    }

    if ( target_file != NULL )
    {
    	XtFree( target_file );
    	target_file = NULL;
    }

    if ( widget != NULL )
    	XtUnmanageChild( widget );

};  /* end of bkr_file_dialog_confirm_ok */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_file_dialog_create
**
** 	Callback routine which configures the file selection dialog box
**  	used to open a book or a new library.
**
**  FORMAL PARAMETERS:
**
**  	widget	    	- widget id of the push button that caused the event.
**  	tag 	        - user data.
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
bkr_file_dialog_create PARAM_NAMES((widget,tag,callback_data))
    Widget		    widget PARAM_SEP
    int	    	    	    *tag PARAM_SEP
    XmAnyCallbackStruct     *callback_data PARAM_END 	/* unused */
{
    int	    	    type = (int) *tag;
    char    	    *dirmask;
    XmString	    cs_dirmask;
    XmString	    cs_title;
    int	    	    argcnt;
    Arg	    	    arglist[5];
    long     	    byte_cnt, stat;

    bkr_cursor_display_wait( ON );

    if ( file_selection_dialog == NULL )
    {
    	if ( ! create_file_selection_dialog( bkr_library->appl_shell_id ) )
    	{
    	    bkr_cursor_display_wait( OFF );
    	    return;
    	}
    }

    /* Set the directory mask for the dialog box */

    switch ( type )
    {
    	case K_OPEN_BOOK_FILE :
    	    dirmask = BOOK_DIRMASK;
    	    if ( open_book_title == NULL )
    	    	open_book_title = (XmString) bkr_fetch_literal( "s_open_book_dialog_title",
    	    	    	    	    	    	    MrmRtypeCString );
    	    cs_title = open_book_title;
    	    break;
    	case K_OPEN_LIBRARY_FILE :
    	    dirmask = SHELF_DIRMASK;
    	    if ( open_library_title == NULL )
    	    	open_library_title = (XmString) bkr_fetch_literal( "s_open_library_dialog_title",
    	    	    	    	    	    	    MrmRtypeCString );
    	    cs_title = open_library_title;
    	    break;
    	default : 
    	    bkr_cursor_display_wait( OFF );
    	    return;
    }
    cs_dirmask = DXmCvtFCtoCS( dirmask, &byte_cnt, &stat );
    argcnt = 0;

/* Refilter only if this usage is different from last time.
 * Note: If the book/shelf list has changed since the last time but the
 * usage hasn't changed then no re-filter is performed.
 * It is assumed that the list is static under normal circumstances and
 * a re-filter could be time consuming.
 */
    if ( previous_fileselect_type != type )
    {
    	SET_ARG( XmNdirMask, cs_dirmask );
	previous_fileselect_type = type;
    }
    SET_ARG( XmNdialogTitle, cs_title );
    XtSetValues( file_selection_dialog, arglist, argcnt );
    COMPOUND_STRING_FREE( cs_dirmask );

    /* Save the callback data type; used later by "ok" and "cancel" callbacks */
    fileselect_type = type;

    bkr_cursor_display_wait( OFF );
    BKR_FLUSH_EVENTS;
    bkr_cursor_display_inactive( bkr_library->appl_shell_id, ON );
    bkr_window_setup( file_selection_dialog, bkr_library->appl_shell_id, TRUE );

};  /* end of bkr_file_dialog_create */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_file_dialog_ok
**
** 	Routine for handling the "OK" callback for the file selection widget.
**
**  FORMAL PARAMETERS:
**
**  	widget	    - id of the OK push button widget.
**  	tag 	    - pointer to the user data.
**	fileselect  - pointer to push button callback data.
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
bkr_file_dialog_ok PARAM_NAMES((widget,tag,fileselect))
    Widget		    	     widget PARAM_SEP
    int	    	    	    	     *tag PARAM_SEP
    XmFileSelectionBoxCallbackStruct *fileselect PARAM_END
{
    long    	    cvt_status;
    long    	    byte_cnt;
    unsigned	    status;

    /*  If the confirm dialog box is already mapped then ignore extraneous
     *  callbacks from the FileSelection widget.
     */
    if ( confirm_in_progress )
    	return;

    if ( fileselect_type == 0 )
    	return;

    /* Free the old filename */

    if ( target_file != NULL )
    {
    	XtFree( target_file );
    	target_file = NULL;
    }

    target_file = (char *) DXmCvtCStoOS( fileselect->value, 
    	    	    	    	    	 &byte_cnt, &cvt_status );

    /* Give up if no string entered, or if the string is null, or if
     * whole thing is shorter than the directory spec (i.e. no filename
     * was entered)
     */
    if ( !target_file || !target_file[0] || (fileselect->length <= fileselect->dir_length))
    {
    	char	*error_string;
	error_string = (char *) bkr_fetch_literal( "CS_IS_NULL_ERROR", ASCIZ );
    	if ( error_string != NULL )
    	{
	    sprintf( errmsg, error_string );
	    bkr_error_modal( errmsg, XtWindow( bkr_library->appl_shell_id ) );
	    XtFree( error_string );
    	}
    	return;
    }

    if ( ! cvt_status )
    {
    	char	*error_string;
	error_string = (char *) bkr_fetch_literal( "CS_TO_OS_CVT_ERROR", ASCIZ );
    	if ( error_string != NULL )
    	{
	    sprintf( errmsg, error_string );
	    bkr_error_modal( errmsg, XtWindow( bkr_library->appl_shell_id ) );
	    XtFree( error_string );
    	}
    	return;
    }

    if ( fileselect_type == K_OPEN_BOOK_FILE )
    {
    	bkr_cursor_display_inactive( bkr_library->appl_shell_id, OFF );
    	bkr_cursor_display_wait( ON );
    	status = (unsigned) bkr_selection_open_book( target_file, 
    	    	    	    	    	    NULL, 0, FALSE, NULL );
    	bkr_cursor_display_wait( OFF );
    	if ( status )
    	    XtUnmanageChild( widget );	    	/* file selection */
    	else
    	    bkr_cursor_display_inactive( bkr_library->appl_shell_id, ON );	/* open failed */
    	BKR_FLUSH_EVENTS;
    }
    else if ( fileselect_type == K_OPEN_LIBRARY_FILE )
    {
    	XtUnmanageChild( widget );  	    	/* file selection */

    	/* No books open, so just switch to the new library.
    	 */
    	if ( bkr_library_number_books_open() == 0 )
    	    bkr_file_dialog_confirm_ok( NULL, NULL, NULL );
    	else
    	{
    	    if ( confirm_open_dialog == NULL )
    	    {
    	    	if ( ! create_confirm_open_dialog( bkr_library->appl_shell_id ) )
    	    	{
    	    	    /* Can't create confirm, so just do it!
    	    	     */
    	    	    bkr_file_dialog_confirm_ok( NULL, NULL, NULL );
    	    	    return;
    	    	}
    	    }
    	    if ( confirm_open_dialog != NULL )
    	    {
    	    	bkr_window_setup(confirm_open_dialog, 
                                 bkr_library->appl_shell_id, 
                                 TRUE );
    	    	confirm_in_progress = TRUE;
    	    }
    	}
    }

};  /* end of bkr_file_dialog_ok */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_file_dialog_open_default
**
** 	Callback routine for opening the "default" library shelf.
**
**  FORMAL PARAMETERS:
**
**  	widget	    	- widget id of the push button that caused the event.
**  	tag 	        - user data.
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
bkr_file_dialog_open_default PARAM_NAMES((widget,tag,callback_data))
    Widget		    widget PARAM_SEP
    int	    	    	    *tag PARAM_SEP
    XmAnyCallbackStruct     *callback_data PARAM_END

{

    bkr_cursor_display_inactive( bkr_library->appl_shell_id, ON );

    /* Free the old filename */

    if ( target_file != NULL )
    {
    	XtFree( target_file );
    	target_file = NULL;
    }

    /* Set the target filename to the default library name
     */
    target_file = (char *)XtMalloc(strlen(bkr_library_resources.defaultLibraryName) + 1 );
    strcpy( target_file, bkr_library_resources.defaultLibraryName);

    /* Set the callback type; used later by "ok" and "cancel" callbacks */

    fileselect_type = K_OPEN_DEFAULT_LIBRARY;

    /* 
     *  Confirm with the user if books are open in the library otherwise
     *  just open the "default" library.
     */
    if ( bkr_library_number_books_open() > 0 )
    {
    	if ( confirm_open_dialog == NULL )
    	{
    	    /*  Can't confirm operation, so just do it!
    	     */
            if ( ! create_confirm_open_dialog( bkr_library->appl_shell_id ) )
    	    	bkr_file_dialog_confirm_ok( NULL, NULL, NULL );
    	}
    	if ( confirm_open_dialog != NULL )
    	{
    	    bkr_window_setup(confirm_open_dialog, 
                             bkr_library->appl_shell_id, 
                             TRUE );
    	    confirm_in_progress = TRUE;
    	}
    }
    else
    	bkr_file_dialog_confirm_ok( NULL, NULL, NULL );

};  /* end of bkr_file_dialog_open_default */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	create_confirm_open_dialog
**
** 	Creates the confirm dialog box when a new library is to be opened.
**
**  FORMAL PARAMETERS:
**
**	parent_widget - id of the parent widget for the file selection dialog.
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
**	Returns:    1 - if the dialog box was created,
**  	    	    0 - otherwise.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
static unsigned
create_confirm_open_dialog( parent_widget )
    Widget  parent_widget;
{
    MrmType 	    dummy_class;
    unsigned	    status;
    char    	    *index_name = "confirmOpenNewLibraryDialog";

    if ( confirm_open_dialog != NULL )
    	return( 1 );

    status = MrmFetchWidget(
    	    	bkr_hierarchy_id,
    	    	index_name, 	    	    	/* index into UIL       */
    	    	parent_widget,  	    	/* parent widget        */
    	    	&confirm_open_dialog,	    	/* widget being fetched */
    	    	&dummy_class );	    	    	/* unused class         */
    if ( status != MrmSUCCESS )
    {
    	char	*error_string;
	error_string = 
		(char *) bkr_fetch_literal( "FETCH_WIDGET_ERROR", ASCIZ );
    	if ( error_string != NULL )
    	{
	    sprintf( errmsg, error_string, index_name );
	    bkr_error_modal( errmsg, NULL );
	    XtFree( error_string );
    	}
    	return( status );
    }

    /*  Setup to receive client messages just in case the user closed 
     *  the confirm dialog box widget by using the window menu instead 
     *  of the "cancel" button.  We need to do some cleanup on the 
     *  Library window.  Pass NULL for the user_data so that we'll
     *  know this isn't from our regular windows.
     */
    XtAddEventHandler( XtParent( confirm_open_dialog ), NoEventMask, TRUE, 
    	    	    	bkr_window_client_message, NULL );

    return( status );

};  /* end of create_confirm_open_dialog */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	create_file_selection_dialog
**
** 	Creates the file selection dialog used for the following callbacks:
**
**  	    "Open Book..."
**  	    "Switch Library..."
**
**  FORMAL PARAMETERS:
**
**	parent_widget - id of the parent widget for the file selection dialog.
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
**	Returns:    1 - if the dialog box was created,
**  	    	    0 - otherwise.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
static unsigned
create_file_selection_dialog( parent_widget )
    Widget  parent_widget;
{
    MrmType 	    dummy_class;
    unsigned	    status;
    char    	    *index_name = "openBookOrShelfFileSelection";

    if ( file_selection_dialog != NULL )
    	return( 1 );

    status = MrmFetchWidget(
    	    	bkr_hierarchy_id,
    	    	index_name, 	    	    	/* index into UIL       */
    	    	parent_widget,  	    	/* parent widget        */
    	    	&file_selection_dialog,    	/* widget being fetched */
    	    	&dummy_class );	    	    	/* unused class         */
    if ( status != MrmSUCCESS )
    {
    	char	*error_string;
	error_string = 
		(char *) bkr_fetch_literal( "FETCH_WIDGET_ERROR", ASCIZ );
    	if ( error_string != NULL )
    	{
	    sprintf( errmsg, error_string, index_name );
	    bkr_error_modal( errmsg, NULL );
	    XtFree( error_string );
    	}
    	return( status );
    }

    /*  Make sure we don't get called twice by disabling multi-clicks.
     */
    {
    	Arg 	arglist[5];
    	Widget 	ok_button;
    	Widget 	apply_button;

    	XtSetArg( arglist[0], XmNmultiClick, XmMULTICLICK_DISCARD );
    	if ( ok_button = XmFileSelectionBoxGetChild( file_selection_dialog, 
    	    	    	    	    	 XmDIALOG_OK_BUTTON ) )
    	    XtSetValues( ok_button, arglist, 1 );
    	if ( apply_button = XmFileSelectionBoxGetChild( file_selection_dialog, 
    	    	    	    	    	 XmDIALOG_APPLY_BUTTON ) )
    	    XtSetValues( apply_button, arglist, 1 );
    }

    /*  Setup to receive client messages just in case the user closed 
     *  the file selection widget by using the window menu instead 
     *  of the "cancel" button.  We need to do some cleanup on the 
     *  Library window.
     */

    XtAddEventHandler( XtParent( file_selection_dialog ), NoEventMask, TRUE, 
    	    	    	bkr_window_client_message, NULL);
    
    return( status );

};  /*end of create_file_selection_dialog */

