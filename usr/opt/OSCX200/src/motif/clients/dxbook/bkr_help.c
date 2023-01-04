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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_HELP.C*/
/* *11    3-SEP-1992 13:01:36 GOSSELIN "added Keyboard and Contents items to Help Menus"*/
/* *10    8-JUN-1992 19:13:20 BALLENGER "UCX$CONVERT"*/
/* *9     8-JUN-1992 13:01:40 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *8    22-APR-1992 22:11:20 GOSSELIN "made I18N changes"*/
/* *7    22-APR-1992 21:15:06 BALLENGER "Use XtResolvePathname and remove wait cursor support."*/
/* *6    20-MAR-1992 14:31:14 KARDON "Put in PLUS help file as conditional"*/
/* *5     3-MAR-1992 16:59:26 KARDON "UCXed"*/
/* *4    11-DEC-1991 11:25:34 GOSSELIN "finished adding help Index support"*/
/* *3    10-NOV-1991 21:17:08 BALLENGER "Fix help file references"*/
/* *2    17-SEP-1991 20:08:03 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:39:30 PARMENTER "Help"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_HELP.C*/
#ifndef VMS
 /*
#else
#module BKR_HELP "V03-0000"
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
**	Help callback routines.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     2-Dec-1990
**
**  MODIFICATION HISTORY:
**
**  06-Feb-1991 David L Ballenger
**              Correct spelling/capitalization of <DXm/DXmHelpB.h> for
**              ULTRIX compatility.
**
**              Pass correct number of arguments to bkr_error_modal().
**
**              Check event type correctly in bkr_help_on_context().
**
**
**--
**/


/*
 * INCLUDE FILES
 */

#include    <DXm/DXmHelpB.h>
#include    <DXm/DECspecific.h>

#include "br_common_defs.h"  /* common BR #defines */
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_globals.h"      /* BR external variables declared here */
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "bkr_api.h"         /* BR api (help) typedefs and #defines */
#include "bkr_api_util.h"    /* BR api (help) typedefs and #defines */
#include "bkr_help.h"        /* function prototypes for .c module */
#include "bkr_cursor.h"      /* cursor manipulation routines */
#include "bkr_error.h"       /* error reporting routines */
#include "bkr_fetch.h"       /* resource fetching routines */
#include "bkr_window.h"      /* Window access routines */


/*
** DEFINES
*/


/*
 * FORWARD ROUTINES
 */

static void  	    create_icon_pixmaps();
static void  	    display_help();
static void  	    unmap_help();


/*
 * FORWARD DEFINITIONS 
 */

static BkrClientStruct      *bkr_help_context = NULL;
static Widget	    	    help_widget = NULL;
static XtCallbackRec	    unmap_help_cb[2] = { { (XtCallbackProc) unmap_help, NULL },
    	    	    	    	    	    	 { NULL, NULL } };
static Pixmap	    	    iconify_pixmap = 0;
static Pixmap	    	    icon_pixmap = 0;
static Boolean	    	    icon_pixmap_created = FALSE;


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_help_on_context
**
**  FORMAL PARAMETERS:
**
**  	widget	- id of the widget that caused the help callback.
**  	tag 	- unused tag.
**  	reason  - callback data.
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
bkr_help_on_context PARAM_NAMES((widget,tag,reason))
    Widget  	    	widget PARAM_SEP
    int	    	    	tag PARAM_SEP
    XmAnyCallbackStruct	*reason PARAM_END

{
    BKR_WINDOW      *window = NULL;
    Widget  	    appl_shell;

    window = bkr_window_find(widget);
    if ( window == NULL )
    {
    	/*  Assume its the Library window.
    	 */
    	appl_shell = bkr_library->appl_shell_id;
    }
    else
    	appl_shell = window->appl_shell_id;
    if ( appl_shell == NULL )
    	return;

    DXmHelpOnContext( appl_shell, FALSE );

};  /* end of bkr_help_on_context */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_help_on_widget
**
**  FORMAL PARAMETERS:
**
**  	widget	- id of the widget that caused the help callback.
**  	topic	- pointer to Help Topic to display.
**  	reason  - callback data.
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
bkr_help_on_widget PARAM_NAMES((widget,topic,reason))
    Widget  	    	widget PARAM_SEP
    char    	    	*topic PARAM_SEP
    XmAnyCallbackStruct	*reason PARAM_END

{
#ifdef USE_HELP_WIDGET

    display_help( topic );        

#else /* use Bkr API */

    Arg	     args[2];
    char    *index_string;
    char    *contents_string;
    Boolean  index_string_matched;
    Boolean  contents_string_matched;

    if (bkr_help_context == NULL)
    {
        XtSetArg( args[0], BkrNfilename, (XtArgVal)BKR_APPLICATION_CLASS );

        (void)BkrOpen((Opaque *)&bkr_help_context,
                      bkr_library->appl_shell_id,
                      args,
                      1
                      );
    }

    index_string = (char *) bkr_fetch_literal( "s_index_help", ASCIZ );
    contents_string = (char *) bkr_fetch_literal( "s_contents_help", ASCIZ );

    /* if topic isn't "Index" or "Contents" */

    if ((( strcmp( topic, index_string )    != 0 )) && 
        (( strcmp( topic, contents_string ) != 0 ))) 
	XtSetArg( args[0], BkrNobject, (XtArgVal)Bkr_Topic );
    else 
	XtSetArg( args[0], BkrNobject, (XtArgVal)Bkr_Directory );
    
    XtFree( index_string );
    XtFree( contents_string );

    XtSetArg( args[1], BkrNobjectName, (XtArgVal)topic );

    (void)BkrDisplay((Opaque)bkr_help_context, args, XtNumber(args));

#endif /* USE_HELP_WIDGET */

};  /* end of bkr_help_on_widget */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  	create_icon_pixmaps
**
**	Creates the icon and iconify pixmaps to set on the HELP widget.
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
static void
create_icon_pixmaps( VOID_PARAM )
{
    unsigned int    icon_size;
    char	    *icon_name;

    if ( ( icon_name = bkr_window_get_icon_index_name( "BOOKREADER_ICON", 
    	    	    &icon_size ) ) != NULL )
    {
    	icon_pixmap = bkr_fetch_window_icon( icon_name );
    	BKR_FREE( icon_name );
    	if ( icon_size == 17 )  	    /* Don't fetch icon twice */
    	    iconify_pixmap = icon_pixmap;
    	else if ( icon_size > 17 )
	  if ( !iconify_pixmap )
    	    iconify_pixmap = bkr_fetch_window_icon( "BOOKREADER_ICON_17X17" );
    }

    icon_pixmap_created = TRUE;

};  /* end of create_icon_pixmaps */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	display_help
**
** 	Routine to create and display HELP within the Bookreader.
**
**  FORMAL PARAMETERS:
**
**	topic - pointer to the HELP frame to display.
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
display_help( topic )
    char    *topic;
{
    Arg	    	arglist[10];
    int	    	argcnt;
    unsigned	status;
    XmString	cs_topic;
    XmString	cs_appl_name;
    MrmType	dummy_class;
    long	byte_cnt, stat;

    /*  Make sure we have a first topic to display.
     */
    if ( *topic == NULL )
	return;

    bkr_cursor_display_wait( ON );

    cs_topic = DXmCvtFCtoCS( topic, &byte_cnt, &stat );

    /*  Just update the Help frame.
     */
    argcnt = 0;
    SET_ARG( DXmNfirstTopic, cs_topic );
    if ( help_widget != NULL )
    {
    	XtSetValues( help_widget, arglist, argcnt );
    	COMPOUND_STRING_FREE( cs_topic );
    }
    else
    {
    	/*  Add the UNMAP callback here because we can't in UIL.
    	 */
    	cs_appl_name = DXmCvtFCtoCS( BKR_APPLICATION_CLASS, &byte_cnt, &stat ); 
    	SET_ARG( DXmNlibrarySpec, cs_appl_name );
    	SET_ARG( XmNautoUnmanage, FALSE );
	SET_ARG( XmNunmapCallback, unmap_help_cb );
    	status = MrmFetchWidgetOverride(
    	    	    bkr_hierarchy_id,
    	    	    "helpWidgetDialog",	    /* index into UIL       */
    	    	    bkr_toplevel_widget,    /* parent widget        */
    	    	    NULL,	    	    /* don't override name  */
    	    	    arglist,
    	    	    argcnt,
    	    	    &help_widget,   	    /* widget being fetched */
    	    	    &dummy_class );	    /* unused class         */
    	COMPOUND_STRING_FREE( cs_topic );
    	COMPOUND_STRING_FREE( cs_appl_name );
    	if ( status != MrmSUCCESS )
    	{
    	    char	*error_string;
	    error_string = (char *) bkr_fetch_literal( "FETCH_WIDGET_ERROR", ASCIZ );
    	    if ( error_string != NULL )
    	    {
	    	sprintf( errmsg, error_string, "helpWidgetDialog" );
	    	bkr_error_modal( errmsg, NULL );
	    	XtFree( error_string );
    	    }
    	    bkr_cursor_display_wait( OFF );
    	    return;
    	}

    	/* Fetch and set the icon and iconify pixmaps for the HELP widget.
    	 */
    	if ( ! icon_pixmap_created )
    	    create_icon_pixmaps();

    	argcnt = 0;
    	if ( icon_pixmap )
    	    SET_ARG( XmNiconPixmap, icon_pixmap );

    	/* Make the window a primary window instead of secondary.
    	 */
    	SET_ARG( XmNtransient, FALSE );
    	XtSetValues( XtParent( help_widget ), arglist, argcnt );

    	/* Popup it up!
    	 */
    	XtManageChild( help_widget );

    	/*  Set the iconify pixmap for the XUI window manager ONLY after
    	 *  the hidden shell has a window.
    	 */
    	if ( iconify_pixmap && ( XtWindow(XtParent(help_widget)) != 0 ) )
    	    bkr_window_set_iconify_pixmap( XtParent(help_widget), iconify_pixmap );
    }

    RAISE_WINDOW( XtParent( help_widget ) );

    bkr_cursor_display_wait( OFF );
    BKR_FLUSH_EVENTS;

};  /* end of display_help */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	unmap_help
**
**  	Callback routine which handles the UNMAP callback.
**
**  FORMAL PARAMETERS:
**
**  	widget - id of the widget that caused the callback.
**  	tag    - unused tag data.
**  	reason - callback data.
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
unmap_help( widget, tag, reason )
    Widget  	    	widget;
    int	    	    	tag;
    XmAnyCallbackStruct	*reason;
{

    XtUnmanageChild( widget );
    XtDestroyWidget( widget );
    help_widget = NULL;

};  /* end of unmap_help */

