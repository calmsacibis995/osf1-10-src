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
#ifdef SEARCH
#ifndef VMS
 /*
#else
#module BKR_SIMPLE_SEARCH "V03-0000"
#endif
#ifndef VMS
  */
#endif

/*
***************************************************************
**  Copyright (c) Digital Equipment Corporation, 1988, 1991  **
**  All Rights Reserved.  Unpublished rights reserved	     **
**  under the copyright laws of the United States.  	     **
**  	    	    	    	    	    	    	     **
**  The software contained on this media is proprietary	     **
**  to and embodies the confidential technology of  	     **
**  Digital Equipment Corporation.  Possession, use, 	     **
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
**      Bookreader User Interface ( bkr )
**
**  ABSTRACT:
**
**	Simple search code
**
**  AUTHORS:
**
**      David Parmenter 
**
**  CREATION DATE:     17-feb-1992
**
**  MODIFICATION HISTORY:
**
**--
**/


/*
 * INCLUDE FILES
 */

#include "br_common_defs.h"  	/* common BR #defines */
#include "br_meta_data.h"    	/* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     	/* BR high-level typedefs and #defines */
#include "br_globals.h"      	/* BR external variables declared here */
#include "br_malloc.h"       	/* BKR_MALLOC, etc defined here */
#include "bkr_button.h"		/* function prototypes for .c module */
#include "bkr_book.h"		/* function prototypes for .c module */
#include "bkr_cursor.h"		/* function prototypes for .c module */
#include "bkr_window.h"		/* function prototypes for .c module */
#include "bkr_topic_data.h"	/* topic data structures */
#include "bkr_search.h"		/* function prototypes for .c module */
#include "bkr_simple_search.h"	/* function prototypes for .c module */
#include "bkr_cbrresults.h"	/* function prototypes for .c module */
#include "bkr_library.h"	/* function prototypes for .c module */

#include <X11/Xlib.h>
#include <ctype.h>
#include <stdio.h>
#include <Xm/Text.h>
#include <Xm/DialogS.h>
#include <Xm/SelectioB.h>
#ifdef __osf__
#include "XctStrings.h"
#include "XcmStrings.h"
#else
#include <Xct/XctStrings.h>
#include <Xcm/XcmStrings.h>
#endif



/*********** 
 * DEFINES *
 ***********/
#define NEW_LINE	'\n'
#define NULL_CHAR	'\0'
#define SPACE		" "



/************************
 * FORWARD DECLARATIONS *
 ************************/
static void		simple_search_client_msg();
static void		search_results_client_msg();
static void    		do_simple_search();
static void    		setup_simple_results();
static void		search_books();
static void		expand_search_books();
static void		search_book();
static void		expand_search_directory();
static int		substringequal();
static int		search_line();
static int		search_chunk();
static void		search_topic();
static void		search_topics();
static void		search_chunks();
static void		search_library();
static void		search_selection_window();
static void		expand_search_selection_window();
static void		fetch_pixmaps();
static void		add_result();
static int 		add_result_filename();
static BMD_OBJECT_ID	get_good_target();
static void		search_library_window();
static void		expand_search_library_window();
static unsigned		open_shelf();
static void		close_shelf();
void			bkr_accumulate_search_messages ();
static void		process_button_event ();
static char  		*filter_search_string();
static void             setposfromparent();


static FILE *fp;

/********************************************
 * STATIC VARIABLES and EXTERNAL REFERENCES *
 ********************************************/
static MrmRegisterArg   tag_reglist[] = { { "tag", (caddr_t) 0 } };
static char    	    	*error_string;	/* generic error string ptr */

/* pixmaps are global for convenience.  [[fix]] */
static Pixmap	   	topic_pixmap = 0;
static Pixmap	   	shelf_pixmap = 0;
static Pixmap		book_pixmap = 0;

static Boolean		STATIC_abort_search;
static BKR_WINDOW	*STATIC_search_window;
static Boolean		STATIC_message_during_search;

static char STATIC_list_translations [] = 
	"<Btn1Down>:		 	MB1DOWN()";

static XtActionsRec STATIC_list_actions [] = 
    {                                                       
       	{"MB1DOWN",		(XtActionProc) process_button_event},
	{NULL,			NULL}
    };

static XtTranslations	STATIC_parsed_table = 0;

externalref Cursor	wait_cursor_id;


#define PENDING 2

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_setup_simple_search()
**
** 	this routine gets called for all simple searches.  'tag' is used
**	to distinguish between the various searches.
**
**  FORMAL PARAMETERS:
**
**	window	
**	tag 	-- used to figure out what kind of query to do
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	search_context
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
bkr_setup_simple_search PARAM_NAMES((window,tag))
    BKR_WINDOW 		*window PARAM_SEP
    Opaque		*tag  PARAM_END
{
    unsigned	    status;
    MrmType 	    dummy_class;
    caddr_t 	    uil_data;
    int	    	    min_width, min_height;
    int	    	    argcnt = 0;
    Arg	    	    arglist[15];
    Position        x_off, y_off;
    Widget          search_dialog;


    static char *dialog_title[] = { NULL, NULL, NULL, NULL, NULL,
                                    NULL, NULL, NULL, NULL, NULL };

    static char *title_name[] = { NULL,
                                  "s_search_library_title",
                                  "s_search_books_title",
                                  "s_search_book_title",
                                  "s_search_topics_title",
                                  NULL,
                                  "s_search_library_window_title",
                                  "s_search_selection_window_title",
                                  NULL };

    if (window == NULL) 
        return;

    bkr_cursor_display_wait( ON );

    /* 
     * set the query type field.  the types are defined in bkr_widget_values.h
     */

    window->search.query_type = (int) *tag;

    /*
     * now fetch the widget, if need be
     */
    if ( window->widgets[W_SEARCH_BOX] == NULL )
    {
    	argcnt = 0;
        SET_ARG( XmNdeleteResponse, XmUNMAP );

        BKR_UPDATE_TAG( window );
	status = MrmFetchWidgetOverride(
	    	    	bkr_hierarchy_id, 
	    	    	"SimpleSearchDialog",  		/* widget's UIL name */
	    	    	window->appl_shell_id,	        /* parent widget */
                        NULL,
                        arglist, argcnt,
	    	    	&window->widgets[W_SEARCH_BOX], /*widget pointer */
	    	    	&dummy_class );	    		/* unused class */

    	if ( status != MrmSUCCESS ) {
            bkr_error_simple_msg(window,"SEARCH_BOX_ERROR");
	    bkr_cursor_display_wait( OFF );
    	    return;
    	}
        window->widgets[W_SEARCH_BOX_OK] 
            = XmSelectionBoxGetChild(window->widgets[W_SEARCH_BOX],XmDIALOG_OK_BUTTON);
        window->widgets[W_SEARCH_BOX_CANCEL] 
            = XmSelectionBoxGetChild(window->widgets[W_SEARCH_BOX],XmDIALOG_CANCEL_BUTTON);
        window->widgets[W_SEARCH_BOX_HELP] 
            = XmSelectionBoxGetChild(window->widgets[W_SEARCH_BOX],XmDIALOG_HELP_BUTTON);
        window->widgets[W_SEARCH_BOX_TEXT] 
            = XmSelectionBoxGetChild(window->widgets[W_SEARCH_BOX],XmDIALOG_TEXT);
        window->widgets[W_SEARCH_BOX_BUTTON_BOX] = XtParent(window->widgets[W_SEARCH_BOX_OK]);

        XtManageChild(window->widgets[W_SEARCH_BOX]);
    }
    else 
    {
        if (!XtIsManaged(window->widgets[W_SEARCH_BOX])) 
        {
            XtManageChild(window->widgets[W_SEARCH_BOX]);
        }
    }
    search_dialog = XtParent(window->widgets[W_SEARCH_BOX]);


    /* Set the title of the search dialog box based on the type of Search
     * that was chosen 
     */
    if (dialog_title[window->search.query_type] == NULL) 
    {
        dialog_title[window->search.query_type] 
        	= (char *)bkr_fetch_literal(title_name[window->search.query_type], ASCIZ);
        if (dialog_title[window->search.query_type] == NULL) 
        {
            /* This shouldn't happen
             */
            dialog_title[window->search.query_type] = "Untitled";
        }
    }
    XtVaSetValues(search_dialog,XmNtitle,dialog_title[window->search.query_type],NULL);

    bkrplus_g_search_in_progress = FALSE;

    if (XtIsRealized(search_dialog)) 
    {
        XtMapWidget(search_dialog);
    }
    else {
        XtRealizeWidget(search_dialog);
    }

    XmProcessTraversal(window->widgets[W_SEARCH_BOX_BUTTON_BOX],
                       XmTRAVERSE_NEXT_TAB_GROUP);
    XmProcessTraversal(window->widgets[W_SEARCH_BOX_TEXT],XmTRAVERSE_CURRENT);

    bkr_cursor_display_wait( OFF );
}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_simple_search_ok
**
**	generic callback for 'search' from simple search box.
**
**  FORMAL PARAMETERS:
**
**	widget  	- the OK/Search button (not used)
**	tag 		- BKR_WINDOW pointer of window that the search
**                        dialog belongs to.
**	data		- callback data (not used)
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
**	causes search to be performed, and results to be displayed.
**
**--
**/

void
bkr_simple_search_ok PARAM_NAMES((widget,tag,data))     
    Widget			widget PARAM_SEP
    Opaque			*tag PARAM_SEP
    XmAnyCallbackStruct		*data  PARAM_END
{
    BKR_WINDOW *window = (BKR_WINDOW *)tag;

    /* If a search is ongoing, we know the user has cancelled it */
    if (bkrplus_g_search_in_progress) {
	STATIC_abort_search = TRUE;
	return;
    }

    bkr_cursor_display_wait( ON );

    bkr_reset_search_context( window );

    /* 
     * call s_s_r() to do the actual searching
     */
    setup_simple_results(window);	

    bkr_cursor_display_wait( OFF );

    /* Now check for any messages that may have been accumulated during the
     * search.  If the flag has been set, manage the dialog box and set
     * the do not enter cursor 
     */
    if (STATIC_message_during_search) {
	bkr_cursor_display_inactive_all (ON);
	XtManageChild (window->widgets[W_SEARCH_MESSAGE_BOX]);
    }

}   /* end of bkr_simple_search_ok */



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_simple_search_cancel()
**
**	search box cancel callback 
**
**  FORMAL PARAMETERS:
**
**	widget  - the cancel button (not used)
**	tag 	- BKR_WINDOW pointer of window that the search
**                dialog belongs to.
**	data	- callback data (not used)
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
**	unmanages search box
**
**--
**/

void
bkr_simple_search_cancel PARAM_NAMES((widget,tag,data))
    Widget			widget PARAM_SEP
    Opaque			*tag PARAM_SEP
    XmAnyCallbackStruct		*data PARAM_END
{
    BKR_WINDOW *window = (BKR_WINDOW *)tag;

    if (window) 
        XtUnmapWidget(XtParent(window->widgets[W_SEARCH_BOX]));

}   /* end of bkr_simple_search_cancel */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	search_results_client_msg ()
**
** 	Callback routine which handles DELETE_WINDOW Client Messages
**  	for the search results dialog box.
**
**  FORMAL PARAMETERS:
**
**	dialog_shell		- id of dialog shell widget which received message.
**	local_simple_search	- id of search results dialog box.
**	event			- pointer to the X event.
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
**	may unmap the search results box
**
**--
**/
static void 
search_results_client_msg PARAM_NAMES((results_shell,window,event,test))
    Widget		results_shell PARAM_SEP
    BKR_WINDOW		*window PARAM_SEP
    XEvent		*event PARAM_SEP
    Boolean		*test PARAM_END
{
    XClientMessageEvent *cmevent = (XClientMessageEvent *) event;
  
    if (event->type != ClientMessage)
	return;

    if ( cmevent->message_type == WM_PROTOCOLS_ATOM )
    {
    	if ( cmevent->data.l[0] == WM_DELETE_WINDOW_ATOM )
    	{
	    /* For some reason, choosing the Close button causes the Results
	       widget to be destroyed (like calling XtDestroyWidget), which
	       will result in an ACCVIO the next time the Results box is
	       set up.  So, zero out the field and force a new instance to be
	       fetched */
	    window->widgets [W_SEARCH_RESULTS_BOX] = 0;
    	}
    }
};




/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	setup_simple_results()
**      
** 	sets up the results widget, does the search, displays the results
** 	
**
**  FORMAL PARAMETERS:
**
**	window
**
**  IMPLICIT INPUTS:
**
**	topic_pixmap
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
**	may unmap the search box
**
**--
**/


static void
setup_simple_results PARAM_NAMES((window))
    BKR_WINDOW	*window PARAM_END
{
    MrmType 	dummy_class;
    Arg		arglist[8];
    int         argcnt;
    int		status;
    char	*search_string;			/* receives the search string */
    char	*text_string;			/* holds the  search string */
    XmString	query_cs;			/* passes search string to results widget */
    XmString	query_ccs;			/* passes search string to results widget */
    XmString	item;
    Widget      widgetlist[8];
    XmString    x_string;
    long        byte_cnt,l_status;
    int         i;
    int         width,height;
    Position    x_off, y_off;
    caddr_t 	    uil_data;
    Widget      search_dialog;
    Widget      results_dialog;


    static XmString search_button_string = NULL;
    static XmString stop_search_string = NULL;
    static XmString ok_button_string = NULL;

    static char *dialog_title[] = { NULL, NULL, NULL, NULL, NULL,
                                    NULL, NULL, NULL, NULL, NULL };

    static char *title_name[] = { NULL,
                                  "s_results_library_title",
                                  "s_results_books_title",
                                  "s_results_book_title",
                                  "s_results_topics_title",
                                  NULL,
                                  "s_results_library_window_title",
                                  "s_results_selection_window_title",
                                  NULL };

    if (window == NULL) 
    {
        return;
    }

    /* Get the search dialog that invoked us.
     */
    search_dialog = XtParent(window->widgets[W_SEARCH_BOX]);

    /*
     * first fetch the results widget if need be
     */
    if ( window->widgets[W_SEARCH_RESULTS_BOX] == NULL )
    {
    	argcnt = 0;
        SET_ARG( XmNdeleteResponse, XmUNMAP );

        BKR_UPDATE_TAG( window );

	status = MrmFetchWidgetOverride(bkr_hierarchy_id, 
                                        "SimpleResultsDialog",
                                        window->appl_shell_id,
                                        NULL,
                                        arglist, argcnt,
                                        &window->widgets[W_SEARCH_RESULTS_BOX],
                                        &dummy_class );

    	if ( status != MrmSUCCESS ) 
        {
            bkr_error_simple_msg(window,"SEARCH_BOX_ERROR");
	    bkr_cursor_display_wait( OFF );
    	    return;
        }

        window->widgets[W_SEARCH_R_GOTO_BUTTON] 
            = XmSelectionBoxGetChild(window->widgets[W_SEARCH_RESULTS_BOX],
                                     XmDIALOG_OK_BUTTON);
        window->widgets[W_SEARCH_R_VISIT_BUTTON] 
            = XmSelectionBoxGetChild(window->widgets[W_SEARCH_RESULTS_BOX],
                                     XmDIALOG_APPLY_BUTTON);
        window->widgets[W_SEARCH_R_CANCEL_BUTTON] 
            = XmSelectionBoxGetChild(window->widgets[W_SEARCH_RESULTS_BOX],
                                     XmDIALOG_CANCEL_BUTTON);
        window->widgets[W_SEARCH_R_HELP_BUTTON] 
            = XmSelectionBoxGetChild(window->widgets[W_SEARCH_RESULTS_BOX],
                                     XmDIALOG_HELP_BUTTON);
        window->widgets[W_SEARCH_R_LIST] 
            = XmSelectionBoxGetChild(window->widgets[W_SEARCH_RESULTS_BOX],
                                     XmDIALOG_LIST);
        window->widgets[W_SEARCH_R_BUTTON_BOX] 
            = XtParent(window->widgets[W_SEARCH_R_GOTO_BUTTON]);

        XtUnmanageChild(XmSelectionBoxGetChild(window->widgets[W_SEARCH_RESULTS_BOX],
                                               XmDIALOG_SELECTION_LABEL));

        XtUnmanageChild(XmSelectionBoxGetChild(window->widgets[W_SEARCH_RESULTS_BOX],
                                               XmDIALOG_TEXT));

        XtVaSetValues(window->widgets[W_SEARCH_RESULTS_BOX],
                      XmNdefaultButton,window->widgets[W_SEARCH_R_VISIT_BUTTON],
                      NULL);

        XtAddCallback(window->widgets[W_SEARCH_R_LIST],
                      XmNbrowseSelectionCallback,
                      (XtCallbackProc)BkrCbrSelect_list,
                      (XtPointer)window);

        XtManageChild(window->widgets[W_SEARCH_RESULTS_BOX]);
    }
    else 
    {
        /* Delete all of the items for the XmList widget.
         */
        XmListDeleteAllItems(window->widgets[W_SEARCH_R_LIST]);
        XmListDeselectAllItems(window->widgets[W_SEARCH_R_LIST]);

        /* Make sure the Results box is managed.
         */
        if (!XtIsManaged(window->widgets[W_SEARCH_RESULTS_BOX])) 
        {
            XtManageChild(window->widgets[W_SEARCH_RESULTS_BOX]);
        }
    }
    results_dialog = XtParent(window->widgets[W_SEARCH_RESULTS_BOX]);

    /* Set the title of the search dialog box based on the type of Search
     * that was chosen 
     */
    if (dialog_title[window->search.query_type] == NULL) 
    {
        dialog_title[window->search.query_type] 
        	= (char *)bkr_fetch_literal(title_name[window->search.query_type], ASCIZ);
        if (dialog_title[window->search.query_type] == NULL) 
        {
            /* This shouldn't happen
             */
            dialog_title[window->search.query_type] = "Untitled";
        }
    }
    XtVaSetValues(results_dialog,XmNtitle,dialog_title[window->search.query_type],NULL);

    /*
     * get search string
     * do the search
     */
    XtVaGetValues(window->widgets[W_SEARCH_BOX],
                  XmNtextString,&query_cs,
                  NULL);

    text_string = (char *)DXmCvtCStoFC (query_cs, &byte_cnt, &status);
    
    if((status = DXmCvtStatusOK)  && strlen( text_string ) )
    {
	search_string  = (char *)BKR_MALLOC(strlen(text_string)+ 1);
        strcpy(search_string, text_string);
	window->search.search_string_length = strlen(text_string);	
    }
    else
    {
	if( text_string )
        {
	    XtFree(text_string);
        }
        bkr_error_simple_msg(window,"SEARCH_NO_STRING");
	bkr_cursor_display_wait( OFF );
    	return;
    }

    XtVaSetValues(window->widgets[W_SEARCH_RESULTS_BOX],
                  XmNlistLabelString,query_cs,
                  NULL);        
    XmStringFree( query_cs );
    XtFree(text_string);

    if (stop_search_string == NULL) 
    {
        stop_search_string = (XmString)bkr_fetch_literal("s_abort_search_label", 
                                                         MrmRtypeCString);
    }

    /* Make Cancel & Help buttons insensitive so they can't be
     * clicked on 
     */
    XtSetSensitive(window->widgets[W_SEARCH_BOX_CANCEL],FALSE);
    XtSetSensitive(window->widgets[W_SEARCH_BOX_HELP],FALSE);
    XtSetSensitive(window->widgets[W_SEARCH_BOX_TEXT],FALSE);

    /* Set the label of the Search button to Stop Search 
     */
    XtVaSetValues(window->widgets[W_SEARCH_BOX_OK],
                  XmNlabelString, stop_search_string,
                  XmNfillOnArm, FALSE,
                  NULL);

    XtVaSetValues(window->widgets[W_SEARCH_R_GOTO_BUTTON],
                  XmNlabelString, stop_search_string,
                  XmNfillOnArm, FALSE,
                  NULL);

    /* Make Apply, Cancel, and Help buttons sensitive so they can
     * be clicked on 
     */
    XtSetSensitive(window->widgets[W_SEARCH_R_VISIT_BUTTON],FALSE);
    XtSetSensitive(window->widgets[W_SEARCH_R_CANCEL_BUTTON],FALSE);
    XtSetSensitive(window->widgets[W_SEARCH_R_HELP_BUTTON],FALSE);


    /* Fetch the message widget so the error routine can use the ID of the
     * message list box to fill it with messages 
     */
    if (window->widgets[W_SEARCH_MESSAGE_BOX] == 0)
    {
	status = MrmFetchWidget (bkr_hierarchy_id,
				 "entryPopupDialogBox",
				 window->appl_shell_id,
				 &window->widgets[W_SEARCH_MESSAGE_BOX],
				 &dummy_class);
    	if ( status != MrmSUCCESS )
        {
            bkr_error_simple_msg(window,"SEARCH_BOX_ERROR");
        }
    }

    /* Keep track of search window */
    STATIC_search_window = window;

    /* Set abort search flag to FALSE */
    STATIC_abort_search = FALSE;

    /* Set message during search flag to FALSE */
    STATIC_message_during_search = FALSE;

    /* Set search in progress flag to TRUE */
    bkrplus_g_search_in_progress = TRUE;

    if (XtIsRealized(results_dialog)) 
    {
        XtMapWidget(results_dialog);
    }
    else {
        XtRealizeWidget(results_dialog);
    }

    /* Turn off the wait cursor in the Search box, to let the user know
     * that they can stop the search 
     */
    XUndefineCursor(bkr_display,XtWindow(window->widgets[W_SEARCH_BOX]));
    XUndefineCursor(bkr_display,XtWindow(window->widgets[W_SEARCH_RESULTS_BOX]));

    XFlush(bkr_display);

    do_simple_search( window, search_string );

    /* Set the label of the OK button back to OK 
     */
    if (search_button_string == NULL) 
    {
        search_button_string = (XmString)bkr_fetch_literal("s_search_button_label", 
                                                           MrmRtypeCString);
    }
    XtVaSetValues(window->widgets[W_SEARCH_BOX_OK],
                  XmNlabelString, search_button_string,
                  XmNfillOnArm, TRUE,
                  NULL);

    /* Make Search, Cancel, and Help buttons sensitive so they can
     * be clicked on 
     */
    XtSetSensitive(window->widgets[W_SEARCH_BOX_CANCEL],TRUE);
    XtSetSensitive(window->widgets[W_SEARCH_BOX_HELP],TRUE);
    XtSetSensitive(window->widgets[W_SEARCH_BOX_TEXT],TRUE);

    /* Set the OK button in the results dialog back.
     */
    if(ok_button_string == NULL)
    {
        ok_button_string = (XmString) bkr_fetch_literal ("s_ok_button_label", MrmRtypeCString);
    }
    XtVaSetValues(window->widgets[W_SEARCH_R_GOTO_BUTTON],
                  XmNlabelString, ok_button_string,
                  XmNfillOnArm, TRUE,
                  NULL);

    /* Make Apply, Cancel, and Help buttons sensitive so they can
     * be clicked on 
     */
    XtSetSensitive(window->widgets[W_SEARCH_R_VISIT_BUTTON],TRUE);
    XtSetSensitive(window->widgets[W_SEARCH_R_CANCEL_BUTTON],TRUE);
    XtSetSensitive(window->widgets[W_SEARCH_R_HELP_BUTTON],TRUE);

    XmProcessTraversal(window->widgets[W_SEARCH_R_BUTTON_BOX],
                       XmTRAVERSE_HOME);

    XmProcessTraversal(window->widgets[W_SEARCH_R_BUTTON_BOX],
                       XmTRAVERSE_NEXT);

    /* Clear the search in progress flag 
     */
    bkrplus_g_search_in_progress = FALSE;

    if(search_string)
    {
	BKR_FREE(search_string);
    }

    /* If there were no matches, inform the user 
     */
    if (window->search.n_results == 0)
    {
	/* Unmap the Results dialog since there were no matches 
         */
	XtUnmapWidget(results_dialog);

        if (STATIC_abort_search) 
        {
            bkr_error_simple_msg(window,"SEARCH_NO_FIND_BEFORE_CANCEL");
        }
        else 
        {
            bkr_error_simple_msg(window,"SEARCH_NO_ENTRIES_ERROR");            
        }
    }
}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_simple_results_select()
**      
** 	results widget 'select' callback
** 	
**
**  FORMAL PARAMETERS:
**
**	widget
**	tag
**	data
**
**  IMPLICIT INPUTS:
**
**	topic_pixmap
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
**	may unmap the search box
**
**--
**/
void 
bkr_simple_results_select PARAM_NAMES((window,position))
    BKR_WINDOW 	*window PARAM_SEP
    int         position PARAM_END
{
    BKR_WINDOW	*topic_window;
    int		file_index,
		line_index,
		char_index;
    char	*text_string;
    int		length;


    /* If a search is ongoing, we know the user has cancelled it 
     */
    if (bkrplus_g_search_in_progress) {
	STATIC_abort_search = TRUE;
	return;
    }

    bkr_cursor_display_wait (ON);

    /* Before doing anything, make sure a topic in the list box is selected;
     * if not, display the error message and return 
     */
    if (position == 0) {
	bkr_error_simple_msg(window,"RESULTS_LIST_ERROR");
	bkr_cursor_display_wait( OFF );
	return;
    }

    /*
     * open the book to this chunk.  
     * position tells us the result #, then we look in the results
     * array for this result.
     *
     * NOTE!: CBR results list is 1-based
     */ 

    switch ( window->search.results[ position - 1 ].result_type )
	{
	case K_SHELF_RESULT:
		/* [MIKE FITZELL] here's where the shelf display routine
		 * would go.
		 */
		printf( "selecting a shelf is not implemented yet.\n" );
		break;

	case K_BOOK_RESULT:
		/* 
		 * get the filename number */

		file_index = 
			window->search.results[ position - 1 ].file_index;

		/* open this filename
		 */

    	    	(void) bkr_selection_open_book( 
			window->search.filenames[ file_index ],
			NULL, 
			0, 
			FALSE, 
			NULL );
		break;

	case K_TOPIC_RESULT:
		topic_window = NULL;
		/* 
		 * get the filename number */
    
		file_index = 
			window->search.results[ position - 1 ].file_index;
		/* 
		 * now open the book to the correct chunk */
		        
		topic_window = bkr_book_open_to_chunk(
			window->search.filenames[ file_index ],
			window->search.results[position - 1].chunk_id,
			FALSE );

		/*
		 * [[MIKE FITZELL]]
		 * [[FIX ME!!!] b_b_o_t_c() calls bkr_topic_open_to_position
		 * which creates a new window.  We need this window id in order
		 * to call b_h_t_w().
		 * this code is here to show you how to get this going
		 *
		 * y value stores the number of lines down,
		 * x values stores the number of chars into the line.
		 */

		line_index = window->search.results[ position - 1 ].y;
		char_index = window->search.results[ position - 1 ].x;

#ifdef COPY

		if ( topic_window )
		    bkr_highlight_text_word( 
				topic_window,
				line_index,
				char_index,
				window->search.search_string_length );
#endif

		break;
	}

    bkr_cursor_display_wait( OFF );
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	filter_search_string
**
**	filter multiple contiguous whitespace, hyphens from search string
**
**  FORMAL PARAMETERS:
**
**  	search_string
**--
**/
#define MEM_ERROR 0

static char *
filter_search_string PARAM_NAMES((search_string))
  char	*search_string PARAM_END
{
  char        *srch_string;
  char        *cp1,*cp2;
  int         len;

  len = strlen(search_string);

  if(!(srch_string = (char *)BKR_CALLOC(1,len)))
    return((char *)0);

  for(cp1 = srch_string, cp2 = search_string; *cp2; )
    {
    *cp1 = tolower(*cp2);
    ++cp2;
    /* filter multiple white-space down to one */
    if((*cp1 != ' ') || (*cp2 != ' '))
      ++cp1;
    }
  *cp1 = '\0';

  return(srch_string);

}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	do_simple_search
**
**	figure out what kind of search to do, then do it
**
**  FORMAL PARAMETERS:
**
**  	window, search_string
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

static void
do_simple_search PARAM_NAMES((window,search_string))
    BKR_WINDOW	*window PARAM_SEP
    char	*search_string PARAM_END
{
    char        *srch_string;




    if(!(srch_string = filter_search_string(search_string)))
      return;

  /* printf("\n srch_string = %s\n",srch_string); */


    switch( window->search.query_type ) 
	{
	case K_SEARCH_LIBRARY:
		search_library( 
			window, 
			srch_string );
		break;

	case K_SEARCH_BOOKS:
                /* fp = fopen("topic_data.txt","w"); */
		search_books( 
			window, 
			srch_string );

                /* fclose(fp); */
		break;

	case K_SEARCH_BOOK:
		search_book(
			window,
			window->shell->book,
		    	srch_string );
		break;

	case K_SEARCH_TOPICS:
		search_topics(
			window,
			window->shell->book,
		    	srch_string );
		break;

	case K_SEARCH_LIBRARY_WINDOW: 
		search_library_window(
			window,
		    	search_string );
		break;

	case K_SEARCH_SELECTION_WINDOW: 
		search_selection_window(
			window,
			window->shell->book,
		    	search_string );

		break;
	}

    BKR_CFREE(srch_string);

}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	search_topic
**
**	search a single topic
**
**
**  FORMAL PARAMETERS:
**
**	window, book, page_id, search_string, chunk_flags
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
static void
search_topic PARAM_NAMES((window,book,page_id,search_string,chunk_flags))
    BKR_WINDOW	      	*window PARAM_SEP
    BKR_BOOK_CTX        *book PARAM_SEP    		/* current book */
    BMD_OBJECT_ID       page_id PARAM_SEP
    char		*search_string PARAM_SEP
    int			chunk_flags[]  PARAM_END
{
    BKR_TEXT_LINE	*line, *tline;
    BKR_TOPIC_DATA	*topic;
    char                *last_chunk_title;	/* keeps previous hit */
    char                *chunk_title;		/* array of chunk titles */
    unsigned int 	i;       		/* utility integers */
    int			line_index,
			char_index,
			status,
			new_index;
    int                 len;
    char		*new_string;

    char                *cp1,*topic_cp,*s_cp,*t_cp;
    char                *topic_buf;
    int                 n_lines,bufsize;
    int                 match;
    char                ch;
    int                 st_len;
    char                *line_txt;
    int                 n_matches;
    int                 topic_ch;
    int                 t_ch;
    int                 keep_searching;
    int                 space_char, t_space_char;

    /*
     * open the topic
     */

  last_chunk_title = "ZZZZZZZZZZZZ";

  topic = bkr_topic_data_get( book, page_id );

  if (topic == NULL)
      return;

  /*--- loop through topic data buffer for start match positions ---*/

  if(!(line = topic->text_lines))
    return;

  for(topic_cp = t_cp = (char *)line->chars,
       line_index = char_index = 1, n_matches = 0, keep_searching = TRUE;
       keep_searching; ++topic_cp, ++char_index)
    {
    space_char = FALSE;

    /* skip blank lines (just have single newline) */
    if(!(*(topic_cp+1)))
      {
      space_char = TRUE;
      while(!(*(topic_cp+1)))
        {
        if(!(line = line->next))
          {
          keep_searching = FALSE;
          break;
          }
        line_index++;
        char_index = 1;
        topic_cp = (char *)line->chars;
        }

      if(!keep_searching)
        break;

      while(*topic_cp == ' ')
        {
        ++topic_cp;
        ++char_index;
        }
      --topic_cp;
      --char_index;
      }

    if(!space_char)
      {
      topic_ch = (int)*topic_cp;
      if(topic_ch == 173)
        topic_ch = '-';
      if(topic_ch == '-')  /* char is hyphen */
        {
        if(!(*(topic_cp+2))) /* hyphen is at end-of-line */
          {  /* a legitimately hyphenated word, parse forward to get next
                compare char */
          space_char = FALSE;
          do
            {  /* go forward over possible empty lines (just newline char) */
            if(!(line = line->next))
              {
              keep_searching = FALSE;
              break;
              }
            line_index++;
            char_index = 1;
            topic_cp = (char *)line->chars;
            } while(!(*(topic_cp+1)));
          if(!keep_searching)
            break;

          /* we are now at a populated line after the hyphen, get the first
             non-space char */
          while(*topic_cp && (*topic_cp == ' '))
            {
            ++topic_cp;
            ++char_index;
            }
          /* (we assume here that there are no lines with just space chars) */
          /* glue the hyphenated word back together */
          }
        /* hyphens that have stuff after them, we leave alone */
        }  /* end if hyphen char */

      else  /* char is NOT hyphen, filter multiple spaces */
        {
        while(*topic_cp == ' ')
          {
          space_char = TRUE;
          ++topic_cp;
          ++char_index;
          }
        }
      }   /* end if !space_char */

    if(!(*topic_cp))
      break;

    /* loop through the search string, filtering topic data, try to match */
    for(match=TRUE, s_cp=search_string, t_cp = topic_cp, tline = line;
         *s_cp; ++s_cp, ++t_cp)
      {
      t_space_char = FALSE;

      /* skip blank lines (just have single newline) */
      if(!(*(t_cp+1)))
        {
        t_space_char = TRUE;
        while(!(*(t_cp+1)))
          {
          if(!(tline = tline->next))
            {
            keep_searching = FALSE;
            break;
            }
          t_cp = (char *)tline->chars;
          }

        if(!keep_searching)
          break;

        if(*t_cp == ' ')
          {
          while(*t_cp == ' ')
            ++t_cp;
          }
        --t_cp;
        }
        
      if(!t_space_char)
        {
        t_ch = (int)*t_cp;
        if(t_ch == 173)
          t_ch = '-';
        if((t_ch == '-'))  /* char is hyphen */
          {
          if(!(*(t_cp+2)))
            {  /* a legitimately hyphenated word, parse forward to get next
                  compare char */
            do
              {  /* go forward over possible empty lines (just newline char) */
              if(!(tline = tline->next))
                {
                keep_searching = FALSE;
                break;
                }
              t_cp = (char *)tline->chars;
              } while(!(*(t_cp+1)));
            if(!keep_searching)
              break;
            /* we are now at a populated line after the hyphen, get the first
               non-space char */
            while(*t_cp && (*t_cp == ' '))
              ++t_cp;
            t_ch = *t_cp;
            /* (we assume here that there are no lines with just space chars) */
            /* glue the hyphenated word back together */
            }
          /* hyphens that have stuff after them, we leave alone */
          }  /* end if hyphen char */

        else  /* char is NOT hyphen, filter multiple spaces */
          {
          if(*t_cp == ' ')
            {
            while(*t_cp == ' ')
              {
              t_space_char = TRUE;
              ++t_cp;
              }
            --t_cp;
            }
          t_ch = *t_cp;
          }

        t_ch = tolower(t_ch);
        }   /* end if !space_char */
      else
        {
        t_ch = ' ';
        }

      /* we are now at a match comparison char in topic data */
      if(!t_ch || (*s_cp != t_ch))
        break;
      }

    if(*s_cp)  /* didn't reach the end of the search string */
      match = FALSE;

    if(match)
      {
      chunk_title = bri_page_chunk_title( book->book_id, 
					line->parent_chunk->id );
      if (strcmp( chunk_title, last_chunk_title ))
        {
	/* this title is different from the last hit,
	 * so we add this result 
	 */
        n_matches = 1;
	add_result(
		window, 
		K_TOPIC_RESULT,			
		book->title,
		chunk_title, 
		line->parent_chunk->id,
		0,
		char_index,
		line_index,
		NULL,
                1);
	last_chunk_title = chunk_title;
	}
      else
        {
        n_matches++;
        /* update_n_matches */
        }
      }
    }

  bkr_topic_data_free( book, page_id );

  return;
  }



/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	search_library
**
**	search each book in the library, call expand_search_library to open
**	shelves
**
**
**  FORMAL PARAMETERS:
**
**  	window, search_string
**  	
**  	
**  IMPLICIT INPUTS:
**
**	bkr_library->u.library.root_of_tree
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
**	calls expand_search_books() which recursively traverses the
**	tree.
**
**--
**/

static void
search_library PARAM_NAMES((window,search_string))
    BKR_WINDOW		*window PARAM_SEP
    char		*search_string PARAM_END
{
    BKR_NODE_PTR 	node_to_position;

    if ( bkr_library->u.library.root_of_tree != NULL ) 
	expand_search_books(
			window,
			bkr_library->u.library.root_of_tree,
			search_string );
}




/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	search_books
**
**	search the selected list of books and or shelves.  
**
**
**  FORMAL PARAMETERS:
**
**	window, search_string
**  	
**  IMPLICIT INPUTS:
**
**	bkr_library->u.library.num_selected
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
**	Calls expand_search_books to recursively search each book and shelf
**                                                         
**
**--
**/
static void
search_books PARAM_NAMES((window,search_string))
    BKR_WINDOW		*window PARAM_SEP
    char		*search_string PARAM_END
{
    BKR_NODE            *node;
    char                *error_string;
    BKR_BOOK_CTX 	*book;
    int			i;

    /*
     * traverse the selected entry list, and expand or search each book
     */

    for (i = 0; i < bkr_library->u.library.num_selected; i++)
	{
	node = (BKR_NODE_PTR)bkr_library->u.library.selected_entry_tags[i];
	expand_search_books( window, node, search_string );
	}
}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	expand_search_books
**
**	recursive routine to search shelves and books
**
**
**  FORMAL PARAMETERS:
**
**  	window, node, search_string
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
**	recursive routine
**
**--
**/
static void
expand_search_books PARAM_NAMES((window,node,search_string))
    BKR_WINDOW		*window PARAM_SEP
    BKR_NODE            *node PARAM_SEP
    char		*search_string  PARAM_END
{
    BKR_BOOK_CTX 	*book;
    BKR_NODE		*child_node;
    int	    		i;
    int			shelf_opened_during_search = FALSE;

    /* If the user had aborted the search, return */
    if (STATIC_abort_search)
	return;

    if (! NODE_IS_EXPANDABLE( node ) )
        {
	/* node is a book
	 */

	book = bkr_book_get(
		NULL,
		node->parent->u.shelf.id, 
		node->entry_id );

	if ( book == NULL )
	    return;

        search_book( 
		window, 
		book, 
		search_string );

	bkr_book_free( book );
	}
    else
	{
	/* node is a Shelf.  Open it and initialize its children
	 */

	if ( node->u.shelf.id == NULL )	    /* siblings not yet initialized */
	    {
	    if ( ! open_shelf( node ) )
    	    	return; 	    	    /* the open failed */
	    shelf_opened_during_search = TRUE;
	    }
	
	child_node = node->u.shelf.children;
    	for ( i = 1; i <= node->u.shelf.num_children; i++ )
	    {
	    expand_search_books( 
				window,
				child_node, 
				search_string );
    	    child_node = child_node->sibling;
	    }
	if ( shelf_opened_during_search )
	    close_shelf( node );
	}
}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	poll_next_event
**
**	handle event loop, checking for Cancel Search
**
**
**  FORMAL PARAMETERS:
**
**  	window, search_string
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
static void
poll_next_event PARAM_NAMES((window))
    BKR_WINDOW		*window PARAM_END
{
    XEvent		event;
    XtInputMask           status;

    /* Process X events at this point to see if the user has chosen
     * to abort the search 
     */
    while(status = XtAppPending(bkr_app_context))
    {
        if(status != XtIMXEvent)
        {
            XtAppProcessEvent(bkr_app_context,status);
            continue;
        }
        XtAppNextEvent (bkr_app_context, &event);

        /* If the event is a button event, only allow if it's in the
         * Search window 
         */
        if(event.type == ButtonPress || event.type == ButtonRelease ||
           event.type == MotionNotify)
        {
            if ((event.xbutton.window ==
                 XtWindow(window->widgets[W_SEARCH_BOX_BUTTON_BOX]))
                || (event.xbutton.window ==
                    XtWindow(window->widgets[W_SEARCH_R_BUTTON_BOX]))
                )
            {
                XtDispatchEvent (&event);
            }
        }
        else if (event.type == ClientMessage)
        {
            /* Process client messages 
             */
            Widget search_dialog = XtParent(window->widgets[W_SEARCH_BOX]);

            if ((event.xclient.message_type == WM_PROTOCOLS_ATOM)
                && (event.xclient.data.l[0] == WM_DELETE_WINDOW_ATOM)
                && (event.xclient.window == XtWindow(search_dialog))
                )
            {
                /* If it is a delete window, allow only if it's on the Search
                 * box 
                 */
                STATIC_abort_search = TRUE;
                XtUnmapWidget(search_dialog);
            }
            else
            {
                /* If the message is not of the delete window type, which
                 * indicates that Close was chosen, process it 
                 */ 
                XtDispatchEvent (&event);
            }
        }
        else        /* Process all other events */
        {
            XtDispatchEvent (&event);
        }
    }   /* end while(status = XtAppPending(bkr_app_context)) */

    return;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	search_library_window
**
**	search each entry in the library window
**
**
**  FORMAL PARAMETERS:
**
**  	window, search_string
**  	
**  	
**  IMPLICIT INPUTS:
**
**	bkr_library->u.library.root_of_tree ...
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
**	calls expand_search_library_window() which recursively searchs the 
**	nodes in this window
**
**--
**/
static void
search_library_window PARAM_NAMES((window,search_string))
  BKR_WINDOW		*window PARAM_SEP
  char		        *search_string PARAM_END
{
  BKR_NODE_PTR 	child = NULL;
  int			i,num_sibs;
  int                 n_selected;
  XEvent		event;
  XtInputMask           status;

  n_selected = bkr_library->u.library.num_selected;
  for (i = 0; (i < n_selected) && !STATIC_abort_search; i++)
    {
    if(!(child = (BKR_NODE_PTR)bkr_library->u.library.selected_entry_tags[i]))
      continue;


    /* Process X events at this point to see if the user has chosen
       to abort the search */

    poll_next_event(window);

    if(STATIC_abort_search)
      {
      return;
      }

    expand_search_library_window(window,child,search_string );
    }

  return;
  }




/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	expand_search_library_window
**
**	recursively search the titles of each book and shelf
**
**
**  FORMAL PARAMETERS:
**
**	window, node, search_string
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
**	recursive routine
**
**      adds results to the results list
**--
**/
static void
expand_search_library_window PARAM_NAMES((window,node,search_string))
    BKR_WINDOW	*window PARAM_SEP
    BKR_NODE	*node PARAM_SEP
    char	*search_string PARAM_END
{
    BMD_BOOK_ID 	book_id;
    BKR_NODE		*child_node;
    int	    		i;
    Pixmap		temp_pixmap;
    int			shelf_opened_during_search = FALSE;
    char                *filespec;
    XEvent		event;
    XtInputMask           status;

    poll_next_event(window);

    if(STATIC_abort_search)
      {
      return;
      }

    if (! NODE_IS_EXPANDABLE( node ) )
        {
	/* node is a book 
	 */



	if ( search_line ( node->title, search_string, NULL ))
	    {
	    /* we've got a match. need to open the book, to get filespec
	     */



	    book_id = bri_book_open(
			node->parent->u.shelf.id, 
			node->entry_id );
	    if ( book_id == NULL )
		return;

	    /* add this filename to the filename list,
	     * then add the result if filename is new
	     */

            filespec = bri_book_found_file_spec(book_id);

            if(add_result_filename( window, filespec ))
                 {
                 add_result(
                 window,
                 K_BOOK_RESULT,
                 node->title,
                 NULL,
                 (BMD_OBJECT_ID)0,
                 0,
                 0,
                 0,
                 filespec,
                 1 );
                 }

            bri_book_close(book_id);

	    }
	}
    else
	{
	/* node is a shelf.  Open it and initialize its children
	 */

	if ( node->u.shelf.id == NULL )	    /* siblings not yet initialized */
	    {
	    if ( ! open_shelf( node ) )
    	    	return; 	    	    /* the open failed */
	    shelf_opened_during_search = TRUE;
	    }
	
	/* now search the title */
	/* [MIKE FITZELL] node->path is supposed to contain the shelf names
	 * of all the parent shelves up to this point.  But I can't get it
	 * to work.  pass this in to add_result in the 'name' paraemeter (the
	 * last one, and then bkr_simple_results_select() will need some code
	 * copied from or merged with the memex code to navigate to that shelf.
 	 */

/* not implemented so don't show we can't do it */
/*	if ( search_line ( node->title, search_string, NULL ))
	    add_result(
		window,
		K_SHELF_RESULT,
		node->title,
		NULL,
		(BMD_OBJECT_ID)0,
		0,
		0,
		0,
		node->title,
                1 );
*/
	/* now recursively traverse the node */

	child_node = node->u.shelf.children;
    	for ( i = 1; i <= node->u.shelf.num_children; i++ )
	    {
	    expand_search_library_window( 
				window,
				child_node, 
				search_string );
    	    child_node = child_node->sibling;
	    }
	if ( shelf_opened_during_search )
	    close_shelf( node );
	}

    return;
}   /* end of expand_search_library_window */



/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	close_shelf
**
**	close a shelf.  [[stolen from bkr_library.c -- this code *could*
**			    be unified with that code. ]]
**
**  FORMAL PARAMETERS:
**
**  	node
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
**	closes the shelf
**
**--
**/
static void
close_shelf PARAM_NAMES(( node ))
    BKR_NODE	*node PARAM_END
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
                close_shelf( child_node );
            }
            /* Now free the child node because the next time the
             * parent node is opened we'll reread the shelf in case
             * there have been any changes.
             */
            bkr_library_free_node(child_node,FALSE);
            child_node = sibling;
        }

        /* Mark the node closed and update the global number of source
         * entries and the 
         */
        node->u.shelf.opened = FALSE;
        node->u.shelf.children = NULL;
        node->u.shelf.num_children = 0;
        bri_shelf_close( node->u.shelf.id );
        node->u.shelf.id = NULL;
    }
};  /* end of close_shelf */



/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	open_shelf
**
**	open up a shelf.  [[stolen from bkr_library.c -- this code *could*
**			    be unified with that code. ]]
**
**  FORMAL PARAMETERS:
**
**  	node
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
**	opens up a shelf
**
**--
**/
static unsigned
open_shelf PARAM_NAMES(( node ))
    BKR_NODE	*node  PARAM_END
{
    unsigned	status;

    /* Open the shelf file */

    node->u.shelf.id = bri_shelf_open(node->parent->u.shelf.id,
                                      node->entry_id, 	            /* entry id in parent shelf */
                                      &node->u.shelf.num_children );
    if ( node->u.shelf.id == NULL )
    	return( FALSE );

    /* Initialize the shelf entries */

    status = bkr_library_sibling_initialize( node );
    if ( ! status ) {
    	return( FALSE );
    }
    /* Successful open, so update fields */

    return( TRUE );

};  /* end of open_shelf */




/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	search_topics
**
**	search a selected list of topics
**
**
**  FORMAL PARAMETERS:
**
**	window, book, search_string
**	
**  	
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**  	none
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
search_topics PARAM_NAMES((window,book,search_string))
    BKR_WINDOW		*window  PARAM_SEP
    BKR_BOOK_CTX 	*book PARAM_SEP
    char		*search_string PARAM_END
{
    int			*chunk_flags;
    int			num_selected;
    int			i;


    /*
     * add the filename 
     */

    add_result_filename( window, book->filespec );

    /* 
     * get memory for chunk_flags 
     */

    if(!(chunk_flags = (int *) 
	BKR_CALLOC(book->n_chunks + 1, sizeof(int))))
    	{
    	printf("bkr_search: error on calloc of chunk_flags\n");
    	return;
    	}

    /*
     * now traverse the svn tree looking for selections
     */

    num_selected = window->u.selection.num_selected;

    for (i=0; i < num_selected; i++) 
	{
	expand_search_directory(
		(BKR_DIR_ENTRY *) window->u.selection.selected_entry_tags[i],
		chunk_flags );
	}

    /* 
     * now search the chunks
     */

    search_chunks(
		window,
		book,
		search_string,
		chunk_flags );

    /*
     * free the flags
     */

    BKR_CFREE(chunk_flags);
}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	search_book
**
**	search a single book
**
**
**  FORMAL PARAMETERS:
**
**	window, book, search_string
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
**      adds results to the results list
**
**--
**/
static void
search_book PARAM_NAMES((window,book,search_string))
    BKR_WINDOW		*window PARAM_SEP
    BKR_BOOK_CTX 	*book PARAM_SEP
    char		*search_string PARAM_END
{
    int			*chunk_flags;
    int			i;


    /*
     * first see if we have already searched this book
     */

    for( i = 0; i < window->search.n_filenames; i++ )
	{
	if (! strcmp ( book->filespec, window->search.filenames[ i ] ))
	    return;
	}
    /*
     * it's a new book. add it to the list 
     */

    add_result_filename( window, book->filespec );

    /* 
     * get memory for chunk_flags 
     */

    if(!(chunk_flags = (int *) 
	BKR_CALLOC(book->n_chunks + 1, sizeof(int))))
    	{
    	printf("bkr_search: error on calloc of chunk_flags\n");
    	return;
    	}

    /*
     * now get relevant chunks from each directory
     */

/*we are doing a whole frigging book so set all the chunk_flags */
/*    for (i=0; i < book->n_directories; i++) 
	{
        BKR_DIR_ENTRY *toplevel;

        toplevel = &book->directories[i];

        if (toplevel->children == NULL) 
            bkr_directory_open(book->book_id,toplevel);

        expand_search_directory( toplevel, chunk_flags );
    	}
*/
    for (i=0; i < book->n_chunks; i++) {
	chunk_flags[i] = 1;
	}
    
    /* 
     * now search the chunks
     */

    search_chunks(
		window,
		book,
		search_string,
		chunk_flags );

    /*
     * free the flags
     */

    BKR_CFREE(chunk_flags);
}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	search_selection_window
**
**	search the entries in the selection window
**
**
**  FORMAL PARAMETERS:
**
**	window, book, search_string
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
**      adds results to the results list
**
**--
**/
static void
search_selection_window PARAM_NAMES((window,book,search_string))
    BKR_WINDOW		*window PARAM_SEP
    BKR_BOOK_CTX	*book PARAM_SEP
    char		*search_string PARAM_END
{
    int			i;

    add_result_filename( window, book->filespec );

    /*
     * now get relevant chunks from each directory
     */

    for (i=0; i < book->n_directories; i++) 
	{
        BKR_DIR_ENTRY *toplevel;

        toplevel = &book->directories[i];

        if (toplevel->children == NULL) 
            bkr_directory_open(book->book_id,toplevel);

        expand_search_selection_window( 
				window, 
				toplevel,
				search_string );
    	}
}




/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	expand_search_selection_window
**
**	recursively search the entries in the selection window
**
**
**  FORMAL PARAMETERS:
**
**  	window, parent, search_string
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
**	recursive routine.  
**	 
**      adds results to the results list
**--
**/
static void
expand_search_selection_window PARAM_NAMES((window,parent,search_string))
    BKR_WINDOW    	*window PARAM_SEP
    BKR_DIR_ENTRY 	*parent PARAM_SEP
    char		*search_string  PARAM_END
{
    BKR_DIR_ENTRY 	*child;
    BMD_OBJECT_ID  	target;

    child = parent->children;
    while (child) 
	{
	if ( search_line ((char *)child->title, search_string, NULL ))
	    {
	    target = child->u.entry.target;
	    if( target == 0 )
		{
		/* can't have a null target, so get target from children */

		target = get_good_target( child );
		}
	    add_result(
		window,
		K_TOPIC_RESULT,
		window->shell->book->title,
		child->title,
		target,
		0,
		0,
		0,
		NULL,
                1 );
	    }

        if (child->children)
	    expand_search_selection_window( window, child, search_string );
        child = child->sibling;
	}
}   /* end of search__expand_directory */



/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	get_good_target
**
**	recursively look for a good entry.target for a parent from 
**  	its list of children
**
**
**  FORMAL PARAMETERS:
**
**  	parent
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
static BMD_OBJECT_ID
get_good_target PARAM_NAMES((parent)) 
    BKR_DIR_ENTRY 	*parent  PARAM_END
{
    BKR_DIR_ENTRY 	*child;
    BMD_OBJECT_ID  	target;

    child = parent->children;
    while (child) 
	{
	if ( child->u.entry.target != 0)
	    return( child->u.entry.target );
        if (child->children)
	    target = get_good_target (child->children );
	if (target != 0)
	    return( target );
        child = child->sibling;
	}
    return((BMD_OBJECT_ID) 0 );
}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	search_chunks
**  	
**	given a list of chunks, get the topics for these chunks, and then
**	search [[note, extra chunks are being searched right now, need to
**	be revisited after new data strucutures are in place.]]
**
**
**  FORMAL PARAMETERS:
**
**	window, book, search_string, chunk_flags
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
static void
search_chunks PARAM_NAMES((window,book,search_string,chunk_flags)) 
  BKR_WINDOW		*window PARAM_SEP
  BKR_BOOK_CTX 	        *book PARAM_SEP
  char                  *search_string PARAM_SEP
  int			chunk_flags[]  PARAM_END
{
  int			*topic_flags;
  int			i;
  XEvent		event;
  XtInputMask           status;

  /*
   * allocate an array of topic_flags, one for each topic 
   */
  if(!(topic_flags = (int *) BKR_CALLOC(book->n_pages + 1, sizeof(int))))
    {
    printf("bkr_search: error on calloc of topic_flags\n");
    return;
    }

  /*
   * now get the topic for these chunks
   */
  for(i = 1; i <= book->n_chunks; i++)
    {
    if(chunk_flags[i])
      topic_flags[ bri_page_chunk_page_id( book->book_id, i )] = TRUE;
    }

  /*
   * now search these topics
   */

  for(i=1; (i <= book->n_pages) && !STATIC_abort_search; ++i)
    {
    if(!topic_flags[i])
      continue;

    /* Process X events at this point to see if the user has chosen
       to abort the search */

    poll_next_event(window);

    if(STATIC_abort_search)
      {
      break;
      }

    search_topic(window,book,i,search_string,chunk_flags);
    }

  /* free the flags */
  BKR_CFREE(topic_flags);

  return;
  }
        


/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	expand_search_directory
**
**	look for valid chunks in the selection window, given a parent.
**	this routine merely sets the chunk_flag[ chunk_id ].
**
**
**  FORMAL PARAMETERS:
**
**	parent, chunk_flags
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
static void
expand_search_directory PARAM_NAMES((parent,chunk_flags))
    BKR_DIR_ENTRY 	*parent PARAM_SEP
    int		  	chunk_flags[] PARAM_END
{
    BKR_DIR_ENTRY *child;

    if ( ( parent->u.entry.target > 0 ) && (parent->entry_type != DIRECTORY) )
	chunk_flags[ parent->u.entry.target ] = TRUE;
    child = parent->children;
    while (child) 
	{
	if ( child->u.entry.target > 0 )
	    chunk_flags[ child->u.entry.target ] = TRUE;
        if (child->children)
	    expand_search_directory( child, chunk_flags );
        child = child->sibling;
	}
}   /* end of search__expand_directory */



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	substringequal()
**	
**	searches two strings to see if the second one is located at the
**	beginning of the first:  
**
**		'abcd' and 'ab' 	-> TRUE
**		'abcd' and 'abcd'	-> TRUE
**		'abc' and 'abcd' 	-> FALSE
**		'abcd' and 'bc' 	-> FALSE
**		'abcd' and 'xabcd'	-> FALSE
**
**  FORMAL PARAMETERS:
**
**	s1 - the first string
**	s2 - the potential substring
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
**	TRUE or FALSE
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
/* decwrite uses this char as their hyphen */
#define HYPHEN 173

static int 
substringequal( s1, s2 )
    unsigned char 	*s1;		/* string to be searched */
    unsigned char	*s2;		/* substring to look for with s1 */
{
    while( *s1 && *s2 )
    	{
	/* compare one letter at a time, lowercasing both */

    	if( _br_tolower( *s1 ) != _br_tolower( *s2 ) )  
      	    return( FALSE );			
	s1++;
    	s2++;

/* quick and dirty fix for qar 208 "- didn't work" Scott */
#if FALSE
	/* we're at the end of a line and the string are still matching */
	/* else we have a hyphen and the next character is a \n */
	if( *s1 == '\n')
	    return(PENDING);
	else if ( (( *s1 == '-' ) || ( *s1 == HYPHEN )) && ( *(s1 + 1) == '\n') )
	    return(PENDING);
#endif

    	}
    return( !*s2 );			/* Should have reached the end of *s2 */

}   /* end of substringequal */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	search_line()
**
**	search a single line of text 
**
**  FORMAL PARAMETERS:
**
**	buffer 		- the line to be searched
**	search_string 	- the string to be searched for
**	index 		- receives the index into this line
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
**	TRUE or FALSE
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/

static int
search_line PARAM_NAMES((buffer,search_string,index)) 
    unsigned char	*buffer PARAM_SEP	/* line to be searched */  
    unsigned char	*search_string PARAM_SEP	/* string to be searched for */
    int			*index PARAM_END		/* receives line index */
{
    int			i = 1;
    int			status = 0;

 if(*search_string) {

/* quick and dirty fix for qar 208 "- didn't work" Scott */
#if FALSE
     /* if index != 0 then we had a line break now we have to strip off*/
    /* all but one space */
    if(index) {
	if(*index != 0) {
	    while( (*buffer == ' ') && ( *(buffer + 1) == ' ') ) 
		buffer++;
	    }
	}
#endif

    while ( *buffer )				/* loop to end of buffer */
        {
	status = substringequal( buffer, search_string );

        if ( status ) {                         /* now compare entire string */
	    if (index)
		*index = i;
	    return( status );			/* if we match, return TRUE */
	    }

        buffer++;				/* move forward */
	i++;
        }
    return( FALSE );				/* couldn't find it */
    }
  else
    return( FALSE ); 
}  /* end of search_line */



/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	add_result_filename
**
**	add a result filename.  The filenames are kept in a separate
**	array within the search context.
**
**
**  FORMAL PARAMETERS:
**
**	window
**	filename       -- filename (if neccesary)
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
**	returns 1 if filename is unique to list, 0 if duplicate (not added)
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
static int
add_result_filename PARAM_NAMES((window,filename))
    BKR_WINDOW		*window PARAM_SEP		/* source window */
    char		*filename PARAM_END		/* optional ptr to data */
{
    int			n_filenames;
    int			i,size;

    char 		**temp_filenames;	/* used for reallocating */

    n_filenames = window->search.n_filenames;

    /*
     * first see if this filename is already in the list
     */

    for( i = 0; i < n_filenames; i++)
	{
	if(! strcmp( window->search.filenames[i], filename))
	    return(0);
	}

    /* 
     * realloc if neccesary.  It doubles each time. 
     * this code reallocs by hand, because realloc() didn't seem to be
     * reliable.
     */

    if ( n_filenames >= window->search.n_filenames_allocated) 
	{
	size = window->search.n_filenames_allocated * 2;
/*	window->search.n_filenames_allocated *= 2; */

	temp_filenames = (char **) BKR_CALLOC( size, sizeof( char * ));

	memcpy(	temp_filenames, window->search.filenames,
	    window->search.n_filenames_allocated * sizeof(char *));

	BKR_CFREE ( window->search.filenames );

	window->search.filenames = temp_filenames;
	window->search.n_filenames_allocated  = size;
	}

    /* 
     * allocate and store the filename, finally increment the counter
     */

    window->search.filenames[ n_filenames ] = (char *)
	BKR_CALLOC( strlen( filename ) + 1, sizeof( char ));
    strcpy( window->search.filenames[ n_filenames ], filename );

    window->search.n_filenames++;

    return(1);
}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	add_result
**
**	add a result:  to the results display, and to the search context.
**	reallocate if neccesary.  Behaves differently, depending on what the
**	result_type is.
**
**
**  FORMAL PARAMETERS:
**
**	window
**	result_type    	-- (K_SHELF_RESULT, K_TOPIC_RESULT, K_BOOK_RESULT)
**	result_title_1 	-- first part of result title
**	result_title_2 	-- second part of result title (if neccesary)
**	chunk_id	-- (if neccesary)
**	page_id		-- (if neccesary)
**	x		-- (if neccesary)
**	y		-- (if neccesary)
**	name		-- (if neccesary)
**	n_matches
**  	
**  IMPLICIT INPUTS:
**
**	topic_pixmap, shelf_pixmap, book_pixmap
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
add_result
PARAM_NAMES((window,result_type,result_title_1,result_title_2,
             chunk_id,page_id,x,y,name,n_matches))
    BKR_WINDOW		*window PARAM_SEP		/* source window */
    int			result_type PARAM_SEP		/* shelf, book, or topic? */
    char		*result_title_1 PARAM_SEP	/* part 1 of res title */
    char		*result_title_2 PARAM_SEP	/* part 2 of res title */
    BMD_OBJECT_ID       chunk_id PARAM_SEP
    unsigned		page_id PARAM_SEP
    int			x PARAM_SEP
    int			y PARAM_SEP
    char		*name PARAM_SEP
    int                 n_matches  PARAM_END
{
    Pixmap		pixmap;
    int			n_results;
    int			i,size;
    long                l_status;
    long                byte_cnt;
    char		final_result_title[1024];	/* [FIX ME!!] */
    char		*final_result_title_ptr;
    BKR_SEARCH_RESULTS	*temp_results;			/* for reallocating */
    char                item_buf[256];
    XmString            item;

    n_results = window->search.n_results;

    /* 
     * realloc if neccesary.  It doubles each time. 
     * this code reallocs by hand, because realloc() didn't seem to be
     * reliable.
     */

    if ( n_results >= window->search.n_results_allocated) 
	{
	size = window->search.n_results_allocated * 2;
	
	temp_results = (BKR_SEARCH_RESULTS *) 
	    BKR_CALLOC( size, sizeof( BKR_SEARCH_RESULTS ));

	memcpy(	temp_results, window->search.results,
	    window->search.n_results_allocated * sizeof(BKR_SEARCH_RESULTS));

	BKR_CFREE ( window->search.results );

	window->search.results = temp_results;
	window->search.n_results_allocated = size;
	}

    /* 
     * load up the results data
     */
 

    window->search.results[ n_results ].result_type	= result_type;
    window->search.results[ n_results ].chunk_id       	= chunk_id;
    window->search.results[ n_results ].page_id		= page_id;
    window->search.results[ n_results ].x		= x;
    window->search.results[ n_results ].y		= y;

    /*
     * [[we assume that the last file in the array is the current file]]
     * note that the filenames array is 0-based
     */

    window->search.results[ n_results ].file_index 	= 
						window->search.n_filenames - 1;

    /*
     * if there's a name, then allocate and store it
     */

    if( name )
	{
	window->search.results[ n_results ].name = (char *)
		BKR_CALLOC( strlen( name ) + 1, sizeof( char ));
	strcpy( window->search.results[ n_results ].name, name );
	}
    else
	window->search.results[ n_results ].name = NULL;

    /*
     * setup the result title and the pixmap
     */

    switch( result_type )
	{
	case K_SHELF_RESULT:
		sprintf( final_result_title, "%s", result_title_1 );
		pixmap = shelf_pixmap;
		break;

	case K_BOOK_RESULT:
		sprintf( final_result_title, "%s", result_title_1 );
		pixmap = book_pixmap;
		break;

	case K_TOPIC_RESULT:
		sprintf( final_result_title, "%s:  %s", result_title_1, result_title_2 );
		pixmap = topic_pixmap;
		break;
	}


    /*
     * now add the item to the list.
     * 
     * [MIKE FITZELL] replace chunk_id with, number of hits, and all
     * will be well.
     */ 

    ++window->search.n_results;

    final_result_title_ptr = final_result_title;

    sprintf(item_buf," %s", final_result_title_ptr);

    /* sprintf(item_buf,"  %d %s", n_matches, final_result_title_ptr); */

    item = DXmCvtOStoCS(item_buf,&byte_cnt,&l_status);

    /* used to call:
         BKrCbrResultsAddItem(window->widgets[W_SEARCH_R_LIST],item,FALSE);
       Now call XmListAddItem directly, with followup to XmListSetBottomPos) */
    
    XmListAddItem(window->widgets[W_SEARCH_R_LIST], item, 0);
    XmListSetBottomPos(window->widgets[W_SEARCH_R_LIST], n_results);

    XmStringFree(item);
}





/*
**++
**  FUNCTIONAL DESCRIPTION:
**  	
**  	fetch_pixmaps
**
**	get the three pixmaps, store them in static variables
**
**
**  FORMAL PARAMETERS:
**
**	window
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
**	sets three static variables:  topic_pixmap, book_pixmap, shelf_pixmap
**
**--
**/
static void
fetch_pixmaps PARAM_NAMES((window))
    BKR_WINDOW		*window  PARAM_END
{
    Pixel   foreground_pixel;
    Pixel   background_pixel;
    Arg     arglist[5];
    int	    argcnt;

    /* Get the foreground/background color for the icons 
     */

    argcnt = 0;
    SET_ARG( XmNforeground, &foreground_pixel );
    SET_ARG( XmNbackground, &background_pixel );
    XtGetValues( window->widgets[W_SEARCH_BOX], arglist, argcnt );

    topic_pixmap = bkr_fetch_icon_literal( 
				"TOPIC_ENTRY_PIXMAP",
				foreground_pixel, 
				background_pixel );

    book_pixmap = bkr_fetch_icon_literal( 
				"BOOK_ENTRY_ICON",
				foreground_pixel, 
				background_pixel );

    shelf_pixmap = bkr_fetch_icon_literal( 
				"SHELF_ENTRY_ICON",
				foreground_pixel, 
				background_pixel );


}




/*
**++
**  ROUTINE NAME
**	bkr_simple_search_abort
**
**  FUNCTIONAL DESCRIPTION:
**	Callback routine for aborting the search operation.  When this routine
**	is called, searches will be suspended.
**
**  FORMAL PARAMETERS:
**	widget  	- not used
**	tag 		- used to tell who called us
**	data		- not used 
**
**  IMPLICIT INPUTS:
**
**	STATIC_abort_search		- static variable set by this routine
**
**  IMPLICIT OUTPUTS:
**	STATIC_abort_search		- static variable set by this routine
**
**  COMPLETION CODES:
**	None
**
**  SIDE EFFECTS:
**	Sets static variable which tells search routines to suspend searching.
**
**--
**/
void bkr_simple_search_abort PARAM_NAMES((widget,tag,data))
    Widget			widget PARAM_SEP
    Opaque			*tag PARAM_SEP
    XmAnyCallbackStruct		*data PARAM_END
{
    STATIC_abort_search = TRUE;
}




/*
**++
**  ROUTINE NAME
**	bkr_accumulate_search_messages
**
**  FUNCTIONAL DESCRIPTION:
**	This routine is called by the bkr_error_modal routine during
**	searches to add errors to the search message list box that is
**	displayed after a search is finished or aborted.  The routine converts
**	each line in the error string into separate compound strings because
**	the list box widget can't handle multi-line entries (exposure problems,
**	distorts size of list box, etc...).  A blank line is then added to
**	separate each error. 
**
**  FORMAL PARAMETERS:
**	char		*error_msg;			- pointer to error string
**
**  IMPLICIT INPUTS:
**	BKR_WINDOW	*STATIC_search_window;		- pointer to search window
**
**  IMPLICIT OUTPUTS:
**	Boolean		STATIC_message_during_search;	- static variable set by this routine
**             
**  COMPLETION CODES:
**	None
**
**  SIDE EFFECTS:
**	Sets static variable which tells search routine that a message was
**	issued during the search.
**
**--
**/
void bkr_accumulate_search_messages (error_msg)
    char		*error_msg;
{
    XmString		cs_error_msg;
    char		*cptr;
    long		byte_cnt, status;
    BKR_WINDOW		*window;
    MrmType		dummy_class;
    Boolean		end_of_string = FALSE;
    int			list_position = 1;

    /* Get the window used for the search */
    window = STATIC_search_window;

    /* If the message list box is not created, fetch it */
    if (window->widgets[W_SEARCH_MESSAGE_LIST] == NULL) {
	status = MrmFetchWidget (bkr_hierarchy_id,
				 "entryPopupListBox",
				 window->appl_shell_id,
				 &window->widgets[W_SEARCH_MESSAGE_LIST],
				 &dummy_class);

	if (status != MrmSUCCESS)
	    return;
    }

    /* Add button translation to list box to make it "read-only" */
    if (STATIC_parsed_table == 0) {
	XtAppAddActions (bkr_app_context, STATIC_list_actions, XtNumber (STATIC_list_actions)); 

	STATIC_parsed_table = XtParseTranslationTable (STATIC_list_translations);
	XtOverrideTranslations (window->widgets[W_SEARCH_MESSAGE_LIST], STATIC_parsed_table);
    }

    /* Add the error to the list box */
    cptr = error_msg;
    while (!end_of_string) {

	while (*cptr != NEW_LINE && *cptr != NULL_CHAR)
	    cptr++;

	if (*cptr == NULL_CHAR)
	    end_of_string = TRUE;
	else
	    *cptr = NULL_CHAR;

	/* Convert this line of the message to a compound string */
	cs_error_msg = DXmCvtFCtoCS (error_msg, &byte_cnt, &status);
	
	/* Add the item to the top of the list box */
	XmListAddItem (window->widgets[W_SEARCH_MESSAGE_LIST], cs_error_msg,
		       list_position);

	list_position++;

	/* Free the compound string */
	COMPOUND_STRING_FREE (cs_error_msg);

	if (!end_of_string) {
	    cptr++;
	    error_msg = cptr;
	}
    }

    /* Add a blank line to the top of the list box */
    cs_error_msg = DXmCvtFCtoCS (SPACE, &byte_cnt, &status);

    XmListAddItem (window->widgets[W_SEARCH_MESSAGE_LIST], cs_error_msg, 1);

    COMPOUND_STRING_FREE (cs_error_msg);

    /* Record the fact that there was a message during the search */
    STATIC_message_during_search = TRUE;
}




/*
**++
**  ROUTINE NAME
**	bkr_search_message_ok (widget, tag, cbs)
**
**  FUNCTIONAL DESCRIPTION:
**	This routine gets called when the OK button in the Search Message
**	box is clicked.  It simply deletes all of the messages in the
**	Search list box.
**
**  FORMAL PARAMETERS:
**	Widget			widget;		- Widget ID
**	Opaque			*tag;		- Pointer to user data
**	XmAnyCallbackStruct	*cbs;		- Pointer to callback structure
**
**  IMPLICIT INPUTS:
**	BKR_WINDOW	*STATIC_search_window;	- pointer to search window
**
**  IMPLICIT OUTPUTS:
**	None
**             
**  COMPLETION CODES:
**	None
**
**  SIDE EFFECTS:
**	None
**
**--
**/
void bkr_search_message_ok PARAM_NAMES((widget,tag,cbs))
    Widget			widget PARAM_SEP
    Opaque			*tag PARAM_SEP
    XmAnyCallbackStruct		*cbs PARAM_END
{
    BKR_WINDOW			*window;

    window = STATIC_search_window;

    XtUnmanageChild (window->widgets[W_SEARCH_MESSAGE_BOX]);

    XmListDeleteAllItems (window->widgets[W_SEARCH_MESSAGE_LIST]);

    /* Remove the do not enter cursor */
    bkr_cursor_display_inactive_all (OFF);
}



static void process_button_event (widget, event, params, num_params)
    Widget		widget;
    XButtonEvent	*event;
    char		**params;
    int			num_params;

{
    /* Do nothing, this routine simply grabs button events away from
       the list box so users can't select messages */
}



/*-------------------- setposfromparent ---------------------------------
Description: BR Print - sets pos of child widget translated from
             parent widgets origin.
------------------------------------------------------------------------*/
static void setposfromparent(parent_w,child_w,x,y)
  Widget     parent_w;
  Widget     child_w;
  int        x;
  int        y;
  {
  Position   xpos;
  Position   ypos;
  Arg        arglist[4];
  int        argcnt;

  /* get xy pos of parent */
  argcnt = 0;
  SET_ARG(XmNx, &xpos);
  SET_ARG(XmNy, &ypos);
  XtGetValues (parent_w, arglist, argcnt);

  /* translate */
  xpos += (Position) x;
  ypos += (Position) y;

  /* set xy pos of child */
  argcnt = 0;
  SET_ARG(XmNdefaultPosition, FALSE);
  SET_ARG(XmNx, xpos);
  SET_ARG(XmNy, ypos);
  XtSetValues(child_w,arglist,argcnt);

  return;
  }


#endif /* SEARCH */
