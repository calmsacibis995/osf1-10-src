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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_MENU_CREATE.C*/
/* *11   17-NOV-1992 22:52:39 BALLENGER "Create CC-specific help menus."*/
/* *10    5-AUG-1992 21:34:54 BALLENGER "Remove the !@%^&$ BOOKREADER_CC conditionals"*/
/* *9    27-JUL-1992 15:33:05 KARDON "Change build literal to BOOKREADER_CC"*/
/* *8    20-JUL-1992 13:49:10 BALLENGER "Character cell support"*/
/* *7     9-JUN-1992 09:57:26 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *6     8-MAY-1992 16:01:19 FITZELL "add edit/copy pulldown to formal topic"*/
/* *5     3-MAR-1992 17:01:07 KARDON "UCXed"*/
/* *4     6-FEB-1992 10:10:39 KLUM "to add cut to buffer"*/
/* *3     7-JAN-1992 16:49:53 PARMENTER "adding CBR/Search"*/
/* *2    18-SEP-1991 14:03:13 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:39:53 PARMENTER "Pulldown and popup menu creation"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_MENU_CREATE.C*/
#ifndef VMS
 /*
#else
#module BKR_MENU_CREATE "V03-0000"
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
**	Pulldown and popup menu creation routines.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     18-Jun-1990
**
**  MODIFICATION HISTORY:
**
**  V03-0002	DLB0001		David L Ballenger	06-Feb-1991
**		Add "extern" declaration for bkr_fetch_literal().
**
**              Pass correct number of arguments to bkr_error_modal().
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
#include "bkr_menu_create.h" /* function prototypes for .c module */
#include "bkr_error.h"       /* Error reporting */
#include "bkr_fetch.h"       /* Resource fetching */
#include "bkr_topic_init.h"  /* Topic window initialization */
#include  <ctype.h>




/*
 * FORWARD DEFINITIONS 
 */

static MrmRegisterArg	    tag_reglist[] = { { "tag", (caddr_t) 0 } };



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  	bkr_menu_create_file
**
** 	Fetchs the appropriate FILE pulldown menu.
**
**  FORMAL PARAMETERS:
**
**	shell	- pointer to the window shell.
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
**
**--
**/
Boolean
bkr_menu_create_file PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END
{
    unsigned	    status;
    char    	    *index_name;
    Arg     	    arglist[5];
    int     	    argcnt;
    MrmType	    dummy_class;

    switch ( window->type )
    {
    	case BKR_LIBRARY :
    	    index_name = "libraryFileMenu";
    	    break;
    	case BKR_SELECTION :
    	    index_name = "selectionFileMenu";
    	    break;
    	case BKR_STANDARD_TOPIC :
    	    index_name = "standardTopicFileMenu";
    	    break;
    	case BKR_FORMAL_TOPIC :
    	    index_name = "formalTopicFileMenu";
    	    break;
    }	  /* end of switch */

    /* Pulldown menu already fetched? */

    if ( window->widgets[W_FILE_MENU] != NULL )
    	return TRUE;

    /* Update the DRM tag and fetch the FILE pulldown menu */

    BKR_UPDATE_TAG( window );

    status = MrmFetchWidget(
    	    	bkr_hierarchy_id,
    	    	index_name, 	    	    	    /* index into UIL */
    	    	window->widgets[W_MENU_BAR],	    /* parent widget  */
    	    	&window->widgets[W_FILE_MENU],	    /* widget fetched */
    	    	&dummy_class);	    	    	    /* unused class   */
    if ( status != MrmSUCCESS )
    {
    	char	*error_string;
	error_string = (char *) bkr_fetch_literal( "FETCH_WIDGET_ERROR", ASCIZ );
    	if ( error_string != NULL )
    	{
	    sprintf( errmsg, error_string, index_name );
	    bkr_error_modal( errmsg, NULL );
	    XtFree( error_string );
    	}
    	return FALSE;
    }

    /* Associate the pulldown menu with the pulldown menu entry */

    argcnt = 0;
    SET_ARG( XmNsubMenuId, window->widgets[W_FILE_MENU] );
    XtSetValues( window->widgets[W_FILE_PULLDOWN_ENTRY], arglist, argcnt );

    /* Post-fetching setup */

    switch ( window->type )
    {
    	case BKR_SELECTION :
    	    	/* Do nothing for now */
    	    break;
    	case BKR_STANDARD_TOPIC :
    	case BKR_FORMAL_TOPIC :
    	    break;
    }	/* end switch */

    return TRUE;

};  /* end bkr_menu_create_file */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  	bkr_menu_create_edit
**
** 	Fetchs the appropriate EDIT pulldown menu.
**
**  FORMAL PARAMETERS:
**
**	shell	- pointer to the window shell.
**
**--
**/
Boolean
bkr_menu_create_edit PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END

{
    unsigned	    status;
    char    	    *index_name;
    Arg     	    arglist[5];
    int     	    argcnt;
    MrmType	    dummy_class;

    if ( (window->type == BKR_SELECTION) ||
	 (window->type == BKR_LIBRARY) )
        return (FALSE);

    index_name = "standardTopicEditMenu";

    /* Pulldown menu already fetched? */

    if ( window->widgets[W_EDIT_MENU] != NULL )
    	return TRUE;

    /* Update the DRM tag and fetch the FILE pulldown menu */

    BKR_UPDATE_TAG( window );

    status = MrmFetchWidget(
    	    	bkr_hierarchy_id,
    	    	index_name, 	    	    	    /* index into UIL */
    	    	window->widgets[W_MENU_BAR],	    /* parent widget  */
    	    	&window->widgets[W_EDIT_MENU],	    /* widget fetched */
    	    	&dummy_class);	    	    	    /* unused class   */
    if ( status != MrmSUCCESS )
    {
    	char	*error_string;
	error_string = (char *) bkr_fetch_literal( "FETCH_WIDGET_ERROR", ASCIZ );
    	if ( error_string != NULL )
    	{
	    sprintf( errmsg, error_string, index_name );
	    bkr_error_modal( errmsg, NULL );
	    XtFree( error_string );
    	}
    	return FALSE;
    }

    /* Associate the pulldown menu with the pulldown menu entry */

    argcnt = 0;
    SET_ARG( XmNsubMenuId, window->widgets[W_EDIT_MENU] );
    XtSetValues( window->widgets[W_EDIT_PULLDOWN_ENTRY], arglist, argcnt );

    return TRUE;

};  /* end bkr_menu_create_edit */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  	bkr_menu_create_view
**
** 	Fetchs the appropriate VIEW pulldown menu.
**
**  FORMAL PARAMETERS:
**
**	shell	- pointer to the window shell.
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
bkr_menu_create_view PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END

{
    unsigned	    status;
    char    	    *index_name;
    Arg     	    arglist[5];
    int     	    argcnt;
    MrmType	    dummy_class;

    switch ( window->type )
    {
    	case BKR_LIBRARY :
    	    index_name = "libraryViewMenu";
    	    break;
    	case BKR_SELECTION :
    	    index_name = "selectionViewMenu";
    	    break;
    	case BKR_STANDARD_TOPIC :
    	    index_name = "standardTopicViewMenu";
    	    break;
    	case BKR_FORMAL_TOPIC :
    	    index_name = "formalTopicViewMenu";
    	    break;
    }	  /* end of switch */

    /* Pulldown menu already fetched? */

    if ( window->widgets[W_VIEW_MENU] != NULL )
    	return TRUE;

    /* Update the DRM tag and fetch the VIEW pulldown menu */

    BKR_UPDATE_TAG( window );

    status = MrmFetchWidget(
    	    	bkr_hierarchy_id,
    	    	index_name, 	    	    	    /* index into UIL */
    	    	window->widgets[W_MENU_BAR],	    /* parent widget  */
    	    	&window->widgets[W_VIEW_MENU],	    /* widget fetched */
    	    	&dummy_class);	    	    	    /* unused class   */
    if ( status != MrmSUCCESS )
    {
    	char	*error_string;
	error_string = (char *) bkr_fetch_literal( "FETCH_WIDGET_ERROR", ASCIZ );
    	if ( error_string != NULL )
    	{
	    sprintf( errmsg, error_string, index_name );
	    bkr_error_modal( errmsg, NULL );
	    XtFree( error_string );
    	}
    	return FALSE;
    }

    /* Associate the pulldown menu with the pulldown menu entry */

    argcnt = 0;
    SET_ARG( XmNsubMenuId, window->widgets[W_VIEW_MENU] );
    XtSetValues( window->widgets[W_VIEW_PULLDOWN_ENTRY], arglist, argcnt );

    /* Post-fetching setup */

    switch ( window->type )
    {
    	case BKR_LIBRARY :
	    /* No post-fetching setup for now */		    
    	    break;

    	case BKR_SELECTION :
	    /* No post-fetching setup for now */		    
    	    break;

    	case BKR_STANDARD_TOPIC :
    	    bkr_topic_init_sensitivity( window );

    	    /* WARNING!! Falling through to next case */

    	case BKR_FORMAL_TOPIC :
    	    	/* Set the initial state for the toggle buttons */
    	    if ( window->widgets[W_HOTSPOTS_ENTRY] != NULL )
    	    {
    	    	argcnt = 0;
    	    	SET_ARG( XmNset, bkr_topic_resources.show_hot_spots );
    	    	XtSetValues( window->widgets[W_HOTSPOTS_ENTRY], arglist, argcnt );
    	    }
    	    if ( window->widgets[W_EXTENSIONS_ENTRY] != NULL )
    	    {
    	    	argcnt = 0;
    	    	SET_ARG( XmNset, bkr_topic_resources.show_extensions );
                if (bkrplus_g_charcell_display) {
                    SET_ARG( XmNsensitive,FALSE);
                }
    	    	XtSetValues( window->widgets[W_EXTENSIONS_ENTRY], arglist, argcnt );

    	    }
    	    break;
    }	/* end switch */

    return TRUE;

};  /* end of bkr_menu_create_view */



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  	bkr_menu_create_search
**
** 	Fetchs the appropriate SEARCH pulldown menu.
**
**  FORMAL PARAMETERS:
**
**	shell	- pointer to the window shell.
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
#ifdef SEARCH
Boolean
bkr_menu_create_search PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END

{
    unsigned	    status;
    char    	    *index_name;
    Arg     	    arglist[5];
    int     	    argcnt;
    MrmType	    dummy_class;

    switch ( window->type )
    {
    	case BKR_LIBRARY :
    	    index_name = "librarySearchMenu";
    	    break;
    	case BKR_SELECTION :
    	    index_name = "selectionSearchMenu";
    	    break;
    	case BKR_STANDARD_TOPIC :
    	case BKR_FORMAL_TOPIC :
    	    index_name = "topicSearchMenu";
    	    break;
    }	  /* end of switch */

    /* Pulldown menu already fetched? */

    if ( window->widgets[W_SEARCH_MENU] != NULL )
    	return TRUE;

    /* Update the DRM tag and fetch the SEARCH pulldown menu */

    BKR_UPDATE_TAG( window );

    status = MrmFetchWidget(
    	    	bkr_hierarchy_id,
    	    	index_name, 	    	    	    /* index into UIL */
    	    	window->widgets[W_MENU_BAR],	    /* parent widget  */
    	    	&window->widgets[W_SEARCH_MENU],    /* widget fetched */
    	    	&dummy_class);	    	    	    /* unused class   */
    if ( status != MrmSUCCESS )
    {
    	char	*error_string;
	error_string = (char *) bkr_fetch_literal( "FETCH_WIDGET_ERROR", ASCIZ );
    	if ( error_string != NULL )
    	{
	    sprintf( errmsg, error_string, index_name );
	    bkr_error_modal( errmsg, NULL );
	    XtFree( error_string );
    	}
    	return FALSE;
    }

    /* Associate the pulldown menu with the pulldown menu entry */

    argcnt = 0;
    SET_ARG( XmNsubMenuId, window->widgets[W_SEARCH_MENU] );
    XtSetValues( window->widgets[W_SEARCH_PULLDOWN_ENTRY], arglist, argcnt );

    /* Post-fetching setup */

    switch ( window->type )
    {
    	case BKR_SELECTION :
	    /* No post-fetching setup for now */		    
    	    break;

    	case BKR_STANDARD_TOPIC :
	    /* No post-fetching setup for now */		    
    	    break;

    	case BKR_FORMAL_TOPIC :
	    /* No post-fetching setup for now */		    
    	    break;
    }	/* end switch */

    return TRUE;

};  /* end of bkr_menu_create_search */
#endif /* SEARCH */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  	bkr_menu_create_help
**
** 	Fetchs the appropriate HELP pulldown menu.
**
**  FORMAL PARAMETERS:
**
**	shell	- pointer to the window shell.
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
**
**--
**/
void
bkr_menu_create_help PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END
{
    unsigned	    status;
    char    	    *index_name;
    MrmType	    dummy_class;
    Widget          menubar;
    Widget          help_button;
    Widget          help_pulldown;

    menubar = window->widgets[W_MENU_BAR];
    
    XtVaGetValues(menubar,XmNmenuHelpWidget,&help_button,NULL);
    XtVaGetValues(help_button,XmNsubMenuId,&help_pulldown,NULL);
    
    /* Pulldown menu already fetched? 
     */
    if ( help_pulldown != NULL )
    	return;

    switch ( window->type )
    {
        case BKR_LIBRARY: {
            index_name = bkrplus_g_charcell_display 
                       ? "libraryHelpMenuCC" : "libraryHelpMenu";
            break;
        }
    	case BKR_SELECTION : {
            index_name = bkrplus_g_charcell_display 
                       ? "selHelpMenuCC" : "selHelpMenu";
    	    break;
        }
    	case BKR_STANDARD_TOPIC :
    	case BKR_FORMAL_TOPIC : {
            index_name = bkrplus_g_charcell_display 
                       ? "topicHelpMenuCC" : "topicHelpMenu";
    	    break;
        }
    }	  /* end of switch */

    /* Update the DRM tag and fetch the HELP pulldown menu */

    BKR_UPDATE_TAG( window );

    status = MrmFetchWidget(bkr_hierarchy_id,
                            index_name,     /* index into UIL */
                            menubar,	    /* parent widget  */
                            &help_pulldown, /* widget fetched */
                            &dummy_class);  /* unused class   */
    if ( status != MrmSUCCESS )
    {
    	char	*error_string;
	error_string = (char *) bkr_fetch_literal( "FETCH_WIDGET_ERROR", ASCIZ );
    	if ( error_string != NULL )
    	{
	    sprintf( errmsg, error_string, index_name );
	    bkr_error_modal( errmsg, NULL );
	    XtFree( error_string );
    	}
    	return;
    }

    /* Associate the pulldown menu with the pulldown menu entry 
     */
    XtVaSetValues(help_button, XmNsubMenuId,help_pulldown,NULL);

};  /* end bkr_menu_create_help */
