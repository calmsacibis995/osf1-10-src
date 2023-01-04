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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_COPYRIGHT.C*/
/* *27   17-NOV-1992 22:48:16 BALLENGER "Distinguish active/inactive windows."*/
/* *26   22-SEP-1992 22:00:21 BALLENGER "Fix LMF version checkingfor character cell."*/
/* *25   21-SEP-1992 22:10:23 BALLENGER "Check BOOKREADER-CC license."*/
/* *24    5-AUG-1992 21:34:28 BALLENGER "Remove the !@%^&$ BOOKREADER_CC conditionals"*/
/* *23    3-AUG-1992 10:19:45 KARDON "Conditionalize LMF call (only in CC product)"*/
/* *22   17-JUL-1992 09:28:39 KARDON "Always allow everything but Character Cell"*/
/* *21   18-JUN-1992 15:06:34 KARDON "Always allow Copy and simple search (in basic Bookreader)"*/
/* *20   18-JUN-1992 13:38:45 BALLENGER "Use correct function interfaces for event handlers."*/
/* *19    8-JUN-1992 19:00:55 BALLENGER "UCX$CONVERT"*/
/* *18    8-JUN-1992 11:35:41 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *17    6-MAY-1992 17:19:02 GOSSELIN "ifdef NO_LMF"*/
/* *16    9-APR-1992 17:34:11 ROSE "Done"*/
/* *15   24-MAR-1992 10:17:16 ROSE "Done"*/
/* *14   23-MAR-1992 17:16:04 ROSE "Done"*/
/* *13   20-MAR-1992 09:53:30 ROSE "Done"*/
/* *12   19-MAR-1992 18:33:30 ROSE "Done"*/
/* *11   19-MAR-1992 17:14:40 ROSE "Left status commented out until it can be debugged"*/
/* *10   18-MAR-1992 16:59:42 ROSE "Done"*/
/* *9    17-MAR-1992 21:42:03 ROSE "routine added to display needs-license message"*/
/* *8    14-MAR-1992 14:16:43 BALLENGER "Fix problems with window and icon titles..."*/
/* *7     9-MAR-1992 17:33:09 ROSE "Done"*/
/* *6     6-MAR-1992 17:59:32 ROSE "LMF checking added for VMS, need binary time on Ultrix"*/
/* *5     3-MAR-1992 16:57:31 KARDON "UCXed"*/
/* *4    12-FEB-1992 11:52:14 PARMENTER "Using I18n lib for title and icon names"*/
/* *3     1-NOV-1991 13:06:45 BALLENGER "Reintegrate  memex support"*/
/* *2    17-SEP-1991 19:32:00 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:38:57 PARMENTER "Copyright Handling"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_COPYRIGHT.C*/
#ifndef VMS
 /*
#else
# module BKR_COPYRIGHT "V03-0000"
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
**      Bookreader User Interface (bkr)
**
**  ABSTRACT:
**	Bookreader copyright display routines (new for V3).
**
**  AUTHORS:
**      James A. Ferguson
**
**  CREATION DATE:
**	23-Nov-1990
**
**  MODIFICATION HISTORY:
**
**      V01-02	Tom Rose	17-Mar-92
**	    Add routine to display "need license to allow function" box.
**
**      V01-01	Tom Rose	05-Mar-92
**	    Add LMF check for Plus license.  Certain functionality globals 
**	    are set depending on whether or not the Plus license is installed.
**
**--
**/

                               
/*******************
 *  INCLUDE FILES  *
 *******************/
#include <stdio.h>
#include "br_application_info.h"/* Version and name information */
#include "br_common_defs.h"	/* common BR #defines */
#include "br_meta_data.h"      	/* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"	/* BR high-level typedefs and #defines */
#include "br_globals.h"		/* BR external variables declared here */
#include "bkr_copyright.h"	/* function prototypes for .c module */
#include "bkr_fetch.h"		/* Routines for fetching resource literals */
#ifdef VMS
#ifndef NO_LMF
#include <lmfdef.h>		/* VMS license checking literals */
#endif
#include <descrip.h>		/* VMS descriptor definition */
#include <ssdef.h>		/* Status codes for SYS$ calls */
#else
#ifndef NO_LMF
#include <lmf.h>		/* Ultrix license checking literals */
#endif
#endif


/**********************
 *  LOCAL STRUCTURES  *
 **********************/
typedef struct {
    short	buffer_length;
    short	item_code;
    char	*buffer_address;
    int		*ret_len_address;
} ITMLST;      



/**********************
 *  FORWARD ROUTINES  *
 **********************/
void		bkr_check_license ();
static int	bkr_get_binary_time ();
Boolean		bkr_display_need_license_box ();


/*************************
 *  EXTERNAL REFERENCES  *
 *************************/
BKR_GLOBAL_DATA int	bkrplus_g_allow_charcell;
BKR_GLOBAL_DATA int	bkrplus_g_allow_copy;
BKR_GLOBAL_DATA int	bkrplus_g_allow_cbr;
BKR_GLOBAL_DATA int	bkrplus_g_allow_search;
BKR_GLOBAL_DATA int	bkrplus_g_allow_print;

                               


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_copyright_display
**
** 	Gets the copyright information from the book, if any and 
**  	displays it in the window title.
**
**  FORMAL PARAMETERS:
**
**  	window - pointer to the bookreader window information
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
**	TRUE if copyright successfully displayed, FALSE if not.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
Boolean
bkr_copyright_display PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END
{
    BMD_BOOK_ID  book_id;
    Arg     	 arglist[1];
    char    	 *copyright_msg = NULL;
    XmString	 cs_copyright_msg;
    long         cs_length;
    long         cs_status;

    switch (window->type) 
    {
        case BKR_LIBRARY: {
            copyright_msg = (char *) bkr_fetch_literal("s_copyright_notice",
                                                       MrmRtypeChar8 );
            break;
        }
        case BKR_SELECTION:
        case BKR_STANDARD_TOPIC:
        case BKR_FORMAL_TOPIC: {
            if ((window->shell) && (window->shell->book))
            {
                copyright_msg = bri_book_copyright_info( window->shell->book->book_id );
            }
            else 
            {
                return FALSE;
            }
            break;
        }
        default: {
            return FALSE;
        }
    }
    
    if ( copyright_msg ) 
    {
	/*
	 * use I18N routine to set title
	 */
        cs_copyright_msg = DXmCvtFCtoCS( copyright_msg, &cs_length, &cs_status);
	DWI18n_SetTitle( window->appl_shell_id, cs_copyright_msg );
	COMPOUND_STRING_FREE( cs_copyright_msg );

        /* Add event handlers to remove the copyright notice.
         */
        XtAddEventHandler( window->widgets[W_MENU_BAR], ButtonPressMask, 
                          TRUE, bkr_copyright_event_handler, window);
        XtAddEventHandler( window->widgets[W_MAIN_WINDOW], KeyPressMask, 
                          TRUE, bkr_copyright_event_handler, window);
        
        if ((window->type == BKR_LIBRARY) || (window->type == BKR_SELECTION)) 
        {
            Widget      svn_primary_window;

            XtVaGetValues(bkr_library->widgets[W_SVN],
                          DXmSvnNprimaryWindowWidget,
                          &svn_primary_window,
                          NULL);

            XtAddEventHandler( window->widgets[W_SVN], ButtonPressMask, 
                              TRUE, bkr_copyright_event_handler, window );
            XtAddEventHandler( svn_primary_window, KeyPressMask, 
                              TRUE, bkr_copyright_event_handler, window);
	} 
        else 
        {
            XtAddEventHandler( window->widgets[W_WINDOW], ButtonPressMask, 
                              TRUE, bkr_copyright_event_handler, window );
            XtAddEventHandler( window->widgets[W_WINDOW], KeyPressMask, 
                              TRUE, bkr_copyright_event_handler, window);
            XtAddEventHandler( window->widgets[W_BUTTON_BOX], ButtonPressMask, 
                              TRUE, bkr_copyright_event_handler, window);
            XtAddEventHandler( window->widgets[W_BUTTON_BOX], KeyPressMask, 
                              TRUE, bkr_copyright_event_handler, window);
        }
        return TRUE;
    }
    return FALSE;
}

/*
 * Copyright notice event handlers.  If a proper event occurs, remove the
 * event handlers, put the proper name in the title bar, and then
 * return.
 */

static void
bkr_copyright_event_handler PARAM_NAMES((w,user_data,event,continue_to_dispatch))
    Widget 	w PARAM_SEP
    XtPointer   user_data PARAM_SEP
    XEvent 	*event PARAM_SEP
    Boolean     *continue_to_dispatch PARAM_END

{
    BKR_WINDOW	*window = (BKR_WINDOW *)user_data;
    Arg 	 arglist[1];
    XmString	 cs_title_bar_name;
    long         cs_length;
    long         cs_status;

    if ( window == NULL )
	return;

    switch ( window->type )
    {
        case BKR_LIBRARY: 
	case BKR_SELECTION : 
        {
            Widget      svn_primary_window;

            XtVaGetValues(bkr_library->widgets[W_SVN],
                          DXmSvnNprimaryWindowWidget,
                          &svn_primary_window,
                          NULL);

            XtRemoveEventHandler( window->widgets[W_SVN], ButtonPressMask,
                                 TRUE, bkr_copyright_event_handler, window );
            XtRemoveEventHandler(svn_primary_window, KeyPressMask,
                                 TRUE, bkr_copyright_event_handler, window );
            break;
        }
	case BKR_STANDARD_TOPIC :
    	case BKR_FORMAL_TOPIC :
        {
            XtRemoveEventHandler( window->widgets[W_WINDOW], ButtonPressMask,
                                 TRUE, bkr_copyright_event_handler, window );
            XtRemoveEventHandler( window->widgets[W_WINDOW], KeyPressMask,
                                 TRUE, bkr_copyright_event_handler, window );
            XtRemoveEventHandler( window->widgets[W_BUTTON_BOX], ButtonPressMask, 
                                 TRUE, bkr_copyright_event_handler, window );
            XtRemoveEventHandler( window->widgets[W_BUTTON_BOX], KeyPressMask, 
                                 TRUE, bkr_copyright_event_handler, window );
            break;
        }
    }		
    XtRemoveEventHandler( window->widgets[W_MAIN_WINDOW], KeyPressMask, 
                         TRUE, bkr_copyright_event_handler, window );
    XtRemoveEventHandler( window->widgets[W_MENU_BAR], ButtonPressMask, 
                         TRUE, bkr_copyright_event_handler, window );
    /*
     * use I18N routine to set title
     */
    if (bkrplus_g_charcell_display) 
    {
        char         buffer[1024];

        /* Highlight the active window in character cell mode by pre- and 
         * post-pending the tilte with '*'s.
         */
        sprintf(buffer," *%s* ",window->title_bar_name);
        cs_title_bar_name= DXmCvtFCtoCS( buffer, &cs_length, &cs_status);
    }
    else 
    {
        cs_title_bar_name= DXmCvtFCtoCS(window->title_bar_name , &cs_length, &cs_status);        
    }
    DWI18n_SetTitle( window->appl_shell_id, cs_title_bar_name);
    COMPOUND_STRING_FREE( cs_title_bar_name );
}




/*
**++
**  ROUTINE NAME:
**     	bkr_check_license ()
**
**  FUNCTIONAL DESCRIPTION:
**	This routine is called at startup to set the global variables (yuck)
**	that indicate which functionality to allow or disallow based on 
**	whether the BookreaderPlus license is installed.
**    
**  FORMAL PARAMETERS:
**	None
**
**  IMPLICIT INPUTS:
**	BKR_GLOBAL_DATA	int	bkrplus_g_allow_charcell
**	BKR_GLOBAL_DATA	int	bkrplus_g_allow_copy
**	BKR_GLOBAL_DATA	int	bkrplus_g_allow_cbr
**	BKR_GLOBAL_DATA	int	bkrplus_g_allow_search
**	BKR_GLOBAL_DATA	int	bkrplus_g_allow_print
**
**  IMPLICIT OUTPUTS:
**	BKR_GLOBAL_DATA	int	bkrplus_g_allow_charcell
**	BKR_GLOBAL_DATA	int	bkrplus_g_allow_copy
**	BKR_GLOBAL_DATA	int	bkrplus_g_allow_cbr
**	BKR_GLOBAL_DATA	int	bkrplus_g_allow_search
**	BKR_GLOBAL_DATA	int	bkrplus_g_allow_print
**
**  FUNCTION VALUE:
**	None     
**
**  SIDE EFFECTS:
**	None     
**--
**/
void bkr_check_license ()
{
    long		  binary_time_quad [2];
    long		  flags;
    Boolean		  set_globals = FALSE;
#ifndef NO_LMF
#ifdef VMS
    short		  product_version [2];
    struct dsc$descriptor date_dsc, product_name_dsc, producer_dsc;
    ITMLST		  item_list [3];
    int			  status = 0;
#else
    ver_t		  product_version;
    int			  status = -1;
#endif    /* VMS */

#ifdef VMS
    /* VMS specific code */

    /* Set up version number */
    product_version [0] = BKR_CC_MINOR_VERSION;
    product_version [1] = BKR_CC_MAJOR_VERSION;

    /* Convert the date to binary time */
    date_dsc.dsc$a_pointer = BKR_CC_PRODUCT_DATE;
    date_dsc.dsc$w_length = strlen (BKR_CC_PRODUCT_DATE);
    date_dsc.dsc$b_class = DSC$K_CLASS_S;
    date_dsc.dsc$b_dtype = DSC$K_DTYPE_T;
    status = bkr_get_binary_time (&date_dsc, binary_time_quad);
    if (status != SS$_NORMAL)
	return;

    /* Create product name descriptor */
    product_name_dsc.dsc$a_pointer = BKR_CC_PRODUCT_NAME;
    product_name_dsc.dsc$w_length = strlen (BKR_CC_PRODUCT_NAME);
    product_name_dsc.dsc$b_class = DSC$K_CLASS_S;
    product_name_dsc.dsc$b_dtype = DSC$K_DTYPE_T;

    /* Create producer descriptor */
    producer_dsc.dsc$a_pointer = BKR_CC_PRODUCER;
    producer_dsc.dsc$w_length = strlen (BKR_CC_PRODUCER);
    producer_dsc.dsc$b_class = DSC$K_CLASS_S;
    producer_dsc.dsc$b_dtype = DSC$K_DTYPE_T;

    /* Create item list to hold the date and version */
    item_list[0].buffer_length = sizeof (binary_time_quad);
    item_list[0].item_code = LMF$_PROD_DATE;
    item_list[0].buffer_address = (char *) binary_time_quad;
    item_list[0].ret_len_address = 0;    
    item_list[1].buffer_length = sizeof (product_version);
    item_list[1].item_code = LMF$_PROD_VERSION;
    item_list[1].buffer_address = (char *) product_version;
    item_list[1].ret_len_address = 0;    
    item_list[2].buffer_length = 0;
    item_list[2].item_code = 0;

    /* Initialize flags */
    flags = LMF$M_RETURN_FAILURES;

    /* Check for the license */
    status = SYS$LOOKUP_LICENSE (&product_name_dsc,
				 item_list,
		  		 &producer_dsc,
				 &flags,
				 0,
				 0);

    /* If the license is there, enable the Plus functionality */
    if (status == SS$_NORMAL)
	set_globals = TRUE;

#else
    /* Ultrix specific code */

    product_version.v_major = BKR_CC_MAJOR_VERSION;
    product_version.v_minor = BKR_CC_MINOR_VERSION;

    status = lmf_probe_license (BKR_CC_PRODUCT_NAME,
				BKR_CC_PRODUCER,
				&product_version,
		   		(time_t) BKR_CC_PRODUCT_BIN_TIME,
				NULL);
    if (status == 0)
	set_globals = TRUE;   
#endif    /* VMS */
#endif    /* NO_LMF */


    /* Set globals accordingly */
    if (set_globals) {
	bkrplus_g_allow_cbr = 1;
	bkrplus_g_allow_charcell = 1;
    }
    else {
	bkrplus_g_allow_cbr = 0;
	bkrplus_g_allow_charcell = 0;
    }	
}




/*
**++
**  ROUTINE NAME:
**	bkr_get_binary_time (time_dsc, time_addr)
**
**  FUNCTIONAL DESCRIPTION:
**	This routine is called by the check license command to convert the
**	ASCII date string to binary time.
**    
**  FORMAL PARAMETERS:
**	struct dsc$descriptor	*time_dsc;	- A descriptor containing the ASCII date
**	struct dsc$descriptor	*time_addr;	- A pointer to a quadword to receive the binary time
**
**  IMPLICIT INPUTS:
**	None
**
**  IMPLICIT OUTPUTS:
**	None
**
**  FUNCTION VALUE:
**	None
**
**  SIDE EFFECTS:
**	None
**--
**/
static int bkr_get_binary_time (time_dsc, time_addr)
    struct dsc$descriptor *time_dsc;
    long		  *time_addr;
{
    int			  status = 0;

#ifdef VMS
    status = SYS$BINTIM (time_dsc, time_addr);
#endif

    return (status);
}



/*
**++
**  ROUTINE NAME:
**	bkr_display_need_license_box ()
**
**  FUNCTIONAL DESCRIPTION:
**	This routine is called by various routines to display the 
**	"license is required" message box.
**    
**  FORMAL PARAMETERS:
**	None
**
**  IMPLICIT INPUTS:
**	None
**
**  IMPLICIT OUTPUTS:
**	None
**
**  FUNCTION VALUE:
**	None
**
**  SIDE EFFECTS:
**	None
**--
**/
Boolean bkr_display_need_license_box ()
{
    char	*error_string;

    error_string = (char *) bkr_fetch_literal ("FUNCTION_REQUIRES_LICENSE", ASCIZ);

    if (error_string != NULL) {
	sprintf (errmsg, error_string);
	bkr_error_modal (errmsg, NULL);
	XtFree (error_string);
    }
}
