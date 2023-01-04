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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_STARTUP.C*/
/* *49   22-MAR-1993 14:43:20 BALLENGER "Conditionalize call character cell initialization on ULTRIX."*/
/* *48    9-NOV-1992 19:15:50 BALLENGER "No working message at startup."*/
/* *47    8-NOV-1992 19:21:45 BALLENGER "Use work in progress box instead of wait cursor on character cell."*/
/* *46   30-OCT-1992 18:50:33 BALLENGER "handle new searchList and defaultLibraryName resources"*/
/* *45   20-OCT-1992 18:56:04 BALLENGER "Fix missing Can't Open Display message."*/
/* *44   24-SEP-1992 17:06:15 KLUM ""*/
/* *43   22-SEP-1992 22:02:26 BALLENGER "Use PID for CC server number on ULTRIX"*/
/* *42   21-SEP-1992 22:11:32 BALLENGER "CC startup for ultrix"*/
/* *41   13-AUG-1992 23:13:06 BALLENGER "Move character cell startup to DECW$BOOKREADER_CC on VMS."*/
/* *40    5-AUG-1992 22:19:02 BALLENGER "Clean up of single image handling"*/
/* *39   29-JUL-1992 13:14:13 BALLENGER "clean up of single image"*/
/* *38   28-JUL-1992 13:37:38 KARDON "Update to one image"*/
/* *37   28-JUL-1992 13:26:20 BALLENGER "Character cell work."*/
/* *36   19-JUN-1992 20:19:58 BALLENGER "Cleanup for Alpha/OSF port."*/
/* *35   17-JUN-1992 16:45:30 ROSE "New callback routine for search message box registered"*/
/* *34   15-JUN-1992 16:19:34 ROSE "Added abort callback routine to MRM vector"*/
/* *33    4-JUN-1992 16:28:46 KLUM "cleaned out linked PS stuff"*/
/* *32   29-MAY-1992 17:35:56 BALLENGER "Uncomment reference to  bmi_no_linkworks_msg_display callback."*/
/* *31   20-MAY-1992 11:28:06 KLUM "UI rework + printing from topic windows"*/
/* *30   10-MAY-1992 21:12:53 BALLENGER "Add LinkWorks not installed message."*/
/* *29    7-MAY-1992 10:26:34 KARDON "Change status for exit if char_Cell requested"*/
/* *28   27-APR-1992 19:19:35 BALLENGER "Add not implemeted yet message for char cell support."*/
/* *27    9-APR-1992 15:34:49 ROSE "Command line processing for version option finished"*/
/* *26    7-APR-1992 16:26:28 ROSE "Ultrix version command line option (-v) added"*/
/* *25    3-APR-1992 17:18:19 FITZELL "decworld hooks"*/
/* *24   30-MAR-1992 16:05:49 BALLENGER "Change #ifdef foo && bar to #if defined(foo) && defined(bar)"*/
/* *23   20-MAR-1992 10:18:18 FITZELL "Conditionalize copy_clipboard"*/
/* *22   19-MAR-1992 11:58:30 FITZELL "put mb1&2 motion callbacks in trans table"*/
/* *21   18-MAR-1992 15:57:57 KARDON "Take out VMS-only literal reference"*/
/* *20   18-MAR-1992 12:11:18 KARDON "Add command line processing"*/
/* *19    8-MAR-1992 19:16:15 BALLENGER "  Add topic data and text line support"*/
/* *18    6-MAR-1992 18:01:21 ROSE "Done"*/
/* *17    5-MAR-1992 14:40:46 PARMENTER "callbacks are correct now"*/
/* *16    5-MAR-1992 14:26:12 PARMENTER "adding simple search"*/
/* *15    3-MAR-1992 17:04:45 KARDON "UCXed"*/
/* *14   17-FEB-1992 17:15:22 KLUM "added mb1-motion handler to action table"*/
/* *13    6-FEB-1992 10:13:22 KLUM "to add cut-to-buffer"*/
/* *12   23-JAN-1992 16:26:44 KLUM "post bl1"*/
/* *11   21-JAN-1992 14:57:45 KLUM "bl1"*/
/* *10    8-JAN-1992 16:27:48 KLUM "print changes"*/
/* *9     7-JAN-1992 16:51:28 PARMENTER "adding CBR/Search"*/
/* *8     2-JAN-1992 09:44:53 KLUM "BR Print - register new callbacks"*/
/* *7    12-DEC-1991 14:48:32 BALLENGER "Fix LinkWorks coldstart timing  problems."*/
/* *6    14-NOV-1991 00:38:17 KLUM "Green devel work"*/
/* *5     8-NOV-1991 10:09:01 GOSSELIN "added ifdef PRINT"*/
/* *4     1-NOV-1991 12:58:41 BALLENGER "Reintegrate  memex support"*/
/* *3    14-OCT-1991 12:12:40 BALLENGER "Clean up coldstart handling"*/
/* *2    17-SEP-1991 21:27:37 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:40:42 PARMENTER "Startup and initialization for the Bookreader UI"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_STARTUP.C*/
#ifndef VMS
 /*
#else
#module BKR_STARTUP "V03-0002"
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
**	Startup and initialization routines for the Bookreader user interface
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     25-Oct-1989
**
**  MODIFICATION HISTORY:
**
**	V01-0001    Tom Rose				5-Mar-1992
**  	    	    Add call to license checking routine.
**
**	V03-0002    JAF0002	James A. Ferguson   	1-Feb-1991
**  	    	    Fix bug in initialize_library_window so that if a
**  	    	    bogus file is passed on the command line the library
**  	    	    window won't start iconified.  Also, rearrange code to
**  	    	    help with cold-start performance by using a work proc.
**
**	V03-0001    JAF0001	James A. Ferguson   	25-Oct-1989
**  	    	    Extracted V2 routines and created new module.
**
**--
**/


/*
 * INCLUDE FILES
 */
#ifdef vms
#include <ssdef.h>
#include <descrip.h>
#include <chfdef.h>
#include <clidef.h>
#include <climsgdef.h>
#include <dvidef.h>
#include <lnmdef.h>
#include <psldef.h>
#else
#include <unistd.h>
#ifdef __osf__
#include <Xm/XmStrDefs.h>
#include <curses.h>
#else
#include <cursesX.h>
#endif /* alpha osf */
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include "br_common_defs.h"	/* common BR #defines */
#include "br_meta_data.h"	/* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"	/* BR high-level typedefs and #defines */
#include "br_globals.h"		/* BR external variables declared here */
#include "br_malloc.h"		/* BKR_MALLOC, etc defined here */
#include "br_application_info.h"/* application name and version information */

/* function prototypes for callbacks from .c modules */
#include "bkr_book.h"
#include "bkr_button.h"
#include "bkr_scroll.h"
#include "bkr_error.h"
#include "bkr_close.h"
#include "bkr_file_dialog.h"
#include "bkr_help.h"
#include "bkr_menu.h"
#include "bkr_pointer.h"
#include "bkr_scroll.h"
#include "bkr_library.h"
#include "bkr_library_create.h"
#include "bkr_pointer.h"
#ifdef PRINT
#include "bkr_printextract.h"
#endif 
#ifdef SEARCH
#include "bkr_search.h"
#include "bkr_simple_search.h"
#endif /* SEARCH */
#include "bkr_selection.h"
#include "bkr_selection_create.h"
#include "bkr_selection_open.h"
#include "bkr_scroll.h"
#include "bkr_topic_open.h"
#include "bkr_topic_create.h"
#include "bkr_topic_callbacks.h"
#include "bkr_window.h"
#include "bkr_cbrresults.h"
#ifdef COPY
#include "bkr_copy_clipboard.h"
#endif
#ifdef MEMEX
#include "bmi_user_interface.h"
#endif 
#ifdef DECWORLD
#include "bkr_decworld.h"
#endif

#define XK_MISCELLANY
#include <X11/keysymdef.h>

/*
 * FORWARD ROUTINES
 */
void		    bkr_save_widgets();
static void  	    initialize_globals();
static void  	    initialize_library_window();
static unsigned     initialize_toolkit();
static int          bkr_get_comline();

#if defined(VMS)
static int          bkr_get_dcl_comline();
static int          bkr_comline_error_handler();
#endif

static void  	    bkr_dummy();
static void  	    bkr_null_callback();

void 	    	    synchronize();  	    /* debug routine */
void 	    	    sync(); 	  	    /* debug routine */


/* #if !defined(NO_CC) */
#ifndef vms
static char  **start_charcell_server PROTOTYPE((int *argc, char **argv));
static void  kill_charcell_server PROTOTYPE((void));
static pid_t charcell_server_id = 0;
static pid_t charcell_wm_id = 0;
#endif 
static char                 charcell_display_string[250] = { '\0' };
/* #endif */


/*
 * FORWARD DEFINITIONS 
 */

#ifdef VMS
static unsigned	    	    old_cond_handler;
#endif 

static char	    	    *uid_file[] = { BKR_UID_FILE };

static MrmRegisterArg	    tag_reglist[] = { { "tag", (caddr_t) 0 } };

static MrmRegisterArg	    callback_reglist[] =
 {
    { "bkr_close_selection_window",  	(caddr_t) bkr_close_selection_window },
    { "bkr_close_topic_window",	    	(caddr_t) bkr_close_topic_window },
    { "bkr_close_quit_callback",	(caddr_t) bkr_close_quit_callback },
    { "bkr_error_unmap",	    	(caddr_t) bkr_error_unmap },
    { "bkr_file_dialog_cancel",	    	(caddr_t) bkr_file_dialog_cancel },
    { "bkr_file_dialog_confirm_ok",  	(caddr_t) bkr_file_dialog_confirm_ok },
    { "bkr_file_dialog_create",    	(caddr_t) bkr_file_dialog_create },
    { "bkr_file_dialog_ok", 	    	(caddr_t) bkr_file_dialog_ok },
    { "bkr_file_dialog_open_default",	(caddr_t) bkr_file_dialog_open_default },
    { "bkr_help_on_context",	    	(caddr_t) bkr_help_on_context },
    { "bkr_help_on_widget", 	    	(caddr_t) bkr_help_on_widget },
    { "bkr_menu_unmap",	    	    	(caddr_t) bkr_menu_unmap },
    { "bkr_save_widgets",    	    	(caddr_t) bkr_save_widgets },
    { "bkr_scroll_horizontal_callback",	(caddr_t) bkr_scroll_horizontal_callback },
    { "bkr_scroll_vertical_callback",	(caddr_t) bkr_scroll_vertical_callback },
    { "bkr_library_attach_to_source",   (caddr_t) bkr_library_attach_to_source },
    { "bkr_library_collapse_expand",	(caddr_t) bkr_library_collapse_expand },
    { "bkr_library_detach_from_source", (caddr_t) bkr_library_detach_from_source },
    { "bkr_library_double_click",   	(caddr_t) bkr_library_double_click },
    { "bkr_library_get_entry",   	(caddr_t) bkr_library_get_entry },
    { "bkr_library_open_close_node",	(caddr_t) bkr_library_open_close_node },
    { "bkr_library_popup_menu",	    	(caddr_t) bkr_library_popup_menu },
    { "bkr_library_save_widgets",    	(caddr_t) bkr_library_save_widgets },
    { "bkr_library_transitions_done",   (caddr_t) bkr_library_transitions_done },
    { "bkr_selection_create_save_ids",	(caddr_t) bkr_selection_create_save_ids },
    { "bkr_selection_collapse_expand",	(caddr_t) bkr_selection_collapse_expand },
    { "bkr_selection_detach_from_src",	(caddr_t) bkr_selection_detach_from_src },
    { "bkr_selection_double_click", 	(caddr_t) bkr_selection_double_click },
    { "bkr_selection_get_entry",    	(caddr_t) bkr_selection_get_entry },
    { "bkr_selection_open_default_dir",	(caddr_t) bkr_selection_open_default_dir },
    { "bkr_selection_popup_menu",   	(caddr_t) bkr_selection_popup_menu },
    { "bkr_selection_transitions_done",	(caddr_t) bkr_selection_transitions_done },

#ifdef REMOVED_FOR_V3	/* Feature could be added back for V4 */
    { "bkr_shell_make_default",	    	(caddr_t) bkr_shell_make_default },
#endif  /* REMOVED_FOR_V3 */

    { "bkr_topic_bottom",    	    	(caddr_t) bkr_topic_bottom },
    { "bkr_topic_create_save_ids",  	(caddr_t) bkr_topic_create_save_ids },
    { "bkr_topic_extensions_onoff",  	(caddr_t) bkr_topic_extensions_onoff },
    { "bkr_topic_goback",    	    	(caddr_t) bkr_topic_goback },
    { "bkr_topic_hot_spots_onoff",   	(caddr_t) bkr_topic_hot_spots_onoff },
    { "bkr_topic_next",	    	    	(caddr_t) bkr_topic_next },
    { "bkr_topic_next_screen",	    	(caddr_t) bkr_topic_next_screen },
    { "bkr_topic_open_in_default",   	(caddr_t) bkr_topic_open_in_default },
    { "bkr_topic_open_in_new",	    	(caddr_t) bkr_topic_open_in_new },
    { "bkr_topic_previous",  	    	(caddr_t) bkr_topic_previous },
    { "bkr_topic_previous_screen",   	(caddr_t) bkr_topic_previous_screen },
    { "bkr_topic_top",    	    	(caddr_t) bkr_topic_top },
    { "bkr_window_expose",   	    	(caddr_t) bkr_window_expose },
#ifdef COPY
    { "bkr_copy_clipboard",    	    	(caddr_t) bkr_copy_clipboard },
#endif
    { "bkr_dummy",	    	    	(caddr_t) bkr_dummy },
#ifdef MEMEX
    { "bmi_save_connection_menu",	(caddr_t) bmi_save_connection_menu },
    { "bmi_no_linkworks_msg_display",	(caddr_t) bmi_no_linkworks_msg_display },
#else
    { "bmi_save_connection_menu", 	(caddr_t) bkr_null_callback },
    { "bmi_no_linkworks_msg_display",	(caddr_t) bkr_null_callback },
#endif
#ifdef PRINT
    { "bkr_print_book", 		(caddr_t) bkr_print_book },
    { "bkr_print_topic", 		(caddr_t) bkr_print_topic },
    { "bkr_printtopic_window", 		(caddr_t) bkr_printtopic_window },
    { "bkr_printpopup", 		(caddr_t) bkr_printpopup },
    { "bkr_print_otherps", 		(caddr_t) bkr_print_otherps },
    { "bkr_pe_options_button",          (caddr_t) bkr_pe_options_button },
    { "bkr_pe_ok_button",        	(caddr_t) bkr_pe_ok_button },
    { "bkr_pe_cancel_button",           (caddr_t) bkr_pe_cancel_button },
    { "bkr_pe_printdialog_ok", 		(caddr_t) bkr_pe_printdialog_ok },
    { "bkr_pe_infodialog_ok",   	(caddr_t) bkr_pe_infodialog_ok },
    { "bkr_pe_append_tb",       	(caddr_t) bkr_pe_append_tb },
    { "bkr_pe_format_radiobox",         (caddr_t) bkr_pe_format_radiobox },
    { "bkr_pe_radio2",                  (caddr_t) bkr_pe_radio2 },
    { "bkr_pe_filename_text",           (caddr_t) bkr_pe_filename_text },
    { "bkr_pe_printers_buttons",        (caddr_t) bkr_pe_printers_buttons },
#endif
#ifdef SEARCH
    { "bkr_no_op_cb",	    		(caddr_t) bkr_no_op_cb },
    { "bkr_search_everything_cb",	(caddr_t) bkr_search_everything_cb },
    { "bkr_search_concept_list_cb",	(caddr_t) bkr_search_concept_list_cb },
    { "bkr_search_edit_concept_cb",	(caddr_t) bkr_search_edit_concept_cb },
    { "bkr_simple_search_ok",	    	(caddr_t) bkr_simple_search_ok },
    { "bkr_simple_search_abort",	(caddr_t) bkr_simple_search_abort },
    { "bkr_simple_search_cancel",	(caddr_t) bkr_simple_search_cancel },
    { "bkr_search_message_ok",		(caddr_t) bkr_search_message_ok },
    { "BkrCbrOK_button",		(caddr_t) BkrCbrOK_button },
    { "BkrCbrApply_button",		(caddr_t) BkrCbrApply_button },
    { "BkrCbrCancel_button",		(caddr_t) BkrCbrCancel_button },
    { "BkrCbrSelect_list",		(caddr_t) BkrCbrSelect_list },
    { "BkrCbr_double_click",		(caddr_t) BkrCbr_double_click },
#endif SEARCH
#ifdef DECWORLD
    { "bkr_decworld_popup",             (caddr_t) bkr_decworld_popup },
    { "bkr_decworld_widg_save",         (caddr_t) bkr_decworld_widg_save },
    { "bkr_decworld_data_save",         (caddr_t) bkr_decworld_data_save },
#endif
    { "tag", 	    	    	    	(caddr_t) 0 }
 };
static int	    	callback_reglist_cnt = ( sizeof(callback_reglist) /
    	    	    	    	    	    	  sizeof(MrmRegisterArg) );

/* Literals for DCL command line processing
 */
#ifdef VMS
#define 	CLI$_SYNTAX	0X000310FC

char		DCL_INTERFACE_QUAL []		= "INTERFACE";
char		DCL_INTERFACE_VAL_DECW []	= "D";
char		DCL_INTERFACE_VAL_CHAR []	= "C";
char		DCL_BOOK_PARAM []		= "P1";
char		DCL_SECTION_PARAM []		= "P2";
#endif

/* Literals for command line processing
 */
char		UNIX_INTERFACE_PREFIX []	= "-";
char		UNIX_INTERFACE_VAL_DECW []	= "m";
char		UNIX_INTERFACE_VAL_CHAR []	= "c";
char		UNIX_INTERFACE_VERSION_CHAR []	= "v";


/*
 *  Action table for widget translations.
 */
static XtActionsRec action_routines[] =
{
    /* Action routines for the topic window drawing area.
     */
    {"TopicKeyboard", 		(XtActionProc) bkr_topic_keyboard_actions},
    {"TopicFocus", 		(XtActionProc) bkr_topic_focus},
    {"MB1_ACTION",  		(XtActionProc) bkr_button_btn1},
    {"MB3_PRESSED", 		(XtActionProc) bkr_button_btn3_popup},
    {"GRAPHICS_EXPOSE",		(XtActionProc) bkr_window_graphics_expose},
    {"LEAVE_WINDOW",		(XtActionProc) bkr_pointer_leave_window},
    {"MOTION",	    		(XtActionProc) bkr_pointer_motion},
    {"MB1_MOTION", 		(XtActionProc) bkr_mb1_motion},
    {"MB2_MOTION",      	(XtActionProc) bkr_mb2_motion},
    {"MB2_ACTION",      	(XtActionProc) bkr_button_btn2},

    /* Action routines for the topic window work area.
     */
    {"CONFIGURE",		(XtActionProc) bkr_window_resize_work_area},

    /* Action routines for the standard topic window button box.
     */
    {"TopicBtnBoxFocus",	(XtActionProc) bkr_topic_btn_box_focus},

    {NULL, NULL}
};

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	main
**
** 	Main entry point for starting the Bookreader.
**
**  FORMAL PARAMETERS:
**
**	argc - command line parameter count
**	argv - address of a vector of pointers to command line parameters
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
**	Routine never returns.  
**
**  SIDE EFFECTS:
**
**	Virtual memory is allocated.
**
**--
**/
main( argc, argv )
    int	    argc;
    char    **argv;
{
    unsigned	status;
    char	book_name_buffer    [MAX_FILESPEC_LEN];
    char	section_name_buffer [MAX_FILESPEC_LEN];

#ifdef VMS_LATER
    /* Establish VMS condition handler */

    old_cond_handler = LIB$ESTABLISH (BRI_ERROR_HANDLER);

#endif  /* VMS */

    /* Parses the command line used to invoke the product
     */
    status = bkr_get_comline (argc, argv, 
                              book_name_buffer, sizeof (book_name_buffer),
                              section_name_buffer, sizeof (section_name_buffer) );

    /* These are now part of Basic Bookreader 
     */
    bkrplus_g_allow_print = 1;
    bkrplus_g_allow_copy = 1;
    bkrplus_g_allow_search = 1;

    /* Perform license checking, which sets globals that indicate what
     * functionality should be disabled or enabled 
     */
    bkr_check_license ();


/* #if !defined(NO_CC) */
    if (bkrplus_g_charcell_display) 
    {

        if ( ! bkrplus_g_allow_charcell) 
        {
            fprintf(stderr,
                    "\nCharacter cell mode requires a BOOKREADER-CC license.\n");
            exit( EXIT_FAILURE);
        }
#ifndef VMS
        else 
        {
            argv = start_charcell_server(&argc,argv);
        }
#endif 
    }
/* #endif */ /* !defined(NO_CC) */

    /* Initialize the toolkit to get things started 
     */
    status = initialize_toolkit( argc, argv );
    if ( status == 0 )
    	exit( EXIT_FAILURE );

    /* Setup globals */
    initialize_globals();

    /* Initialize the Library window */
    initialize_library_window( book_name_buffer, section_name_buffer );

    /* Process events forever! */

    XtAppMainLoop( bkr_app_context );

    /* should never get here */

};  /* end of main */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_save_widgets
**
** 	Save the individual widget ids during widget creation from a fetch
**
**  FORMAL PARAMETERS:
**
**	widget      	- id of the widget to be saved
**	tag 	    	- pointer to the tag passed from the UIL file
**	reason  	- pointer callback data (not used)
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
bkr_save_widgets( widget, tag, reason )
    Widget	    	    widget;
    int 	    	    *tag;
    XmAnyCallbackStruct    *reason;
{
    int	    	    index;
    BKR_WINDOW	    *window = NULL;

    index = (int) *tag;

    window = bkr_window_find( widget ); 

    /* If the shells's not found assume its the Library shell */

    if ( window == NULL )
    {
    	bkr_library_save_widgets( widget, tag, reason );
    	return;
    }

#ifdef TRACE_OUTPUT
    printf( "saved %31s = %2d\n", XtName( widget ), index );
#endif

    window->widgets[index] = widget;	/* save it!! */

};  /* end bkr_save_widgets */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	initialize_globals
**
** 	Defines global variables used in the Bookreader.
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
initialize_globals()
{
    int	screen;

    bkr_display	    	= XtDisplay( bkr_toplevel_widget );
    screen  	    	= XDefaultScreen( bkr_display );
    bkr_display_width 	= XDisplayWidth( bkr_display, screen );
    bkr_display_height	= XDisplayHeight( bkr_display, screen );

    /*
     * Get the monitor resolution in dots per inch (DPI).
     */
    if (bkrplus_g_charcell_display) 
        bkr_monitor_resolution = 75;
    else
        bkr_monitor_resolution = 
            (int) ((float) bkr_display_width
                    / ((float) XDisplayWidthMM( bkr_display, screen )
                        / MILLIMETERS_PER_INCH 
                       )
                   );

    /*
     *  KLUDGE: JAF 23-May-1989
     *  If the monitor resolution is within 10dpi either side of 75 
     *  assume 75 or if the it is 10dpi either side of 100 assume 100.  This
     *  is because PC's are really 67dpi not 75 and the PC server uses the
     *  75 dpi fonts.  Also, the VMS server in the past has changed its mind
     *  about 75dpi monitors and thought they were 78dpi.
     */

    if ( abs( bkr_monitor_resolution - 75) <= 10 )
	bkr_monitor_resolution = 75;
    else if ( abs( bkr_monitor_resolution - 100 ) <= 10 )
	bkr_monitor_resolution = 100;

    /* Initialize default font information 
     */
    bkr_default_font = XLoadQueryFont(bkr_display,"fixed");
    bkr_default_space_width = XTextWidth(bkr_default_font," ",1);
    bkr_default_line_height = bkr_default_font->ascent + bkr_default_font->descent;

    /* Create the cursors */
    bkr_cursor_create_cursors();

#ifdef DECWORLD
    bkr_decworld_start_timer = TRUE;
#endif
    
};  /* end of initialize_globals */
#ifdef VMS

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	define_vms_search_list
**
**  	Defines the serach list for finding books and shelves on vms.
**
**  FORMAL PARAMETERS:
**
**	None
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
define_vms_search_list()
{
    typedef struct _ITEM_LIST { 
        unsigned short ITM$W_BUFSIZ;
        unsigned short ITM$W_ITMCOD;
        unsigned char  *ITM$L_BUFADR;
        unsigned long  ITM$L_RETLEN;
    } ITEM_LIST;
    
    static ITEM_LIST default_search_list[] = { { (sizeof("DECW$BOOK:") - 1),
                                                 LNM$_STRING,
                                                 "DECW$BOOK:",
                                                 0 },
                                               0 };
    static $DESCRIPTOR(name_table,"LNM$PROCESS");
    static $DESCRIPTOR(logical_name,"DECW$BOOK_SEARCHLIST");

    ITEM_LIST *items = NULL;
    int n_items = 0;
    int attributes = 0;
    unsigned char access_mode = PSL$C_USER;
    int status;

    if (bkr_library_resources.searchList) 
    {
        /* The search list resource was defined, so look for the items in it
         * and use them to define the search list logical name.
         */
        char *ptr = bkr_library_resources.searchList;

        while (*ptr) 
        {
            /* Skip initial spaces.
             */
            while (*ptr && isspace(*ptr)) 
            {
                ptr++;
            }
            if (*ptr) 
            {
                /* Found something add it to the item list for creating the logical
                 * name.
                 */
                if (n_items == 0) {
                    items = (ITEM_LIST *)BKR_CALLOC(n_items+2,sizeof(ITEM_LIST));
                } else {
                    items = (ITEM_LIST *)BKR_REALLOC(items,(n_items+2) * sizeof(ITEM_LIST));
                }
                items[n_items].ITM$W_ITMCOD = LNM$_STRING;
                items[n_items].ITM$L_BUFADR = ptr;
                
                /* Find the end of the string.
                 */
                while (*ptr && ! isspace(*ptr)) {
                    ptr++;
                }
                items[n_items].ITM$W_BUFSIZ = ptr - items[n_items].ITM$L_BUFADR;
                
                n_items++;
            }
        }
    }
    if (n_items) 
    {
        /* We found some items in the resource string, so terminate that item
         * list.
         */
        items[n_items].ITM$W_ITMCOD = 0;
        items[n_items].ITM$W_BUFSIZ = 0;
        items[n_items].ITM$L_BUFADR = NULL;
    }
    else
    {
        /* Didn't find any items so use the default search list.
         */
        items = default_search_list;
    }

    /* Define the logical name.
     */
    status = SYS$CRELNM(&attributes,&name_table,&logical_name,&access_mode,items);
    if (n_items) {
        BKR_FREE(items);
    }
    if ((status & 1) == 0) {
        exit(status);
    }
} /* end define_vms_search_list() */
#endif /* VMS */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	initialize_library
**
** 	Creates the Library window by determining what toplevel
**  	library shelf to use and opens the book specified on the
**  	command line.
**
**  FORMAL PARAMETERS:
**
**	book		- pointer to buffer into which the book name
**			  should be placed.
**	section		- pointer to buffer into which the section name
**			  should be placed.
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
initialize_library_window( book, section )
    char		*book;
    char		*section;

{
    char		*file_name;
    char		*symbol_name;
    Boolean		start_iconified		= FALSE;
    Boolean		open_to_book		= TRUE;
    BMD_ENTRY_TYPE	file_type		= BMD_C_UNKNOWN;
    char		*library_logname	= BKR_DECW_BOOKSHELF_LOG;

#ifdef MEMEX
    /*  Initialize the hyperinformation services before creating this window.
     *  If the initialization succeeds and the application was hyperinvoked
     *  the routine will return TRUE, and we will want to create the window
     *  iconified.
     */
    if (bmi_initialize_his()) {
        start_iconified = TRUE;
    }
#endif

    /* Fetch the shell resources for the library window.
     */
    bkr_resource_fetch_library();

#ifdef VMS
    define_vms_search_list();
#else
    if (bkr_library_resources.searchList) 
    {
        setenv("DECW_BOOK",bkr_library_resources.searchList,TRUE);
    }
#endif


    /* Determine if the command line had book/shelf and section parameters */

    if ( strlen (book) > 0 )
    {
    	file_type = bri_file_type( book );
    	file_name = book;
    }
    if ( strlen (section) > 0 )
    {
    	open_to_book = FALSE;
    	symbol_name = section;
    }

    /* Determine what file to use for the toplevel shelf */

    if ( file_type == BMD_C_SHELF )
    {
    	bkr_library_resources.defaultLibraryName = file_name;
    }
    else if (bkr_library_resources.defaultLibraryName == NULL)
    {
        /* See if the Library logical name is defined 
         */
    	char 	*tmp_library = NULL;

    	tmp_library = bri_logical( library_logname );
    	if ( tmp_library == library_logname )	    /* No translation */
    	{
            bkr_library_resources.defaultLibraryName = BKR_TOPLEVEL_LIBRARY;
    	}
    	else
    	{
#ifdef VMS
    	    if ( tmp_library )
    	    	free( tmp_library );	/* bri_logical uses malloc */
    	    /* Use logical name -- not translation 
             */
            bkr_library_resources.defaultLibraryName = library_logname;
#else
            bkr_library_resources.defaultLibraryName = tmp_library;
#endif 
    	}
    }

    if (bkr_server_coldstart()) 
    {
        start_iconified = TRUE;
    } 
    else if ( file_type == BMD_C_BOOK )
    {
    	start_iconified = TRUE;
    }

    bkr_library_window_create( start_iconified );

/* #if !defined(NO_CC) */
    if (bkrplus_g_charcell_display)
    {
        int cell_height = 0;
        int cell_width = 0;
        int argcnt = 0;
        Arg arglist[2];

        SET_ARG("cellheight", &cell_height);
        SET_ARG("cellwidth",  &cell_width);
        XtGetValues(bkr_library->widgets[W_MAIN_WINDOW],arglist,argcnt);

        if (cell_height > 0) {
            bkr_default_line_height = cell_height;
        }
        if (cell_width > 0) {
            bkr_default_space_width = cell_width;
        }
    }
/* #endif */ /* NO_CC */

    bkr_server_initialize();


    /* See if we need to open a particular book or topic */

    if ( file_type == BMD_C_BOOK )
    {
    	start_iconified = TRUE;
    	if ( open_to_book ) {
    	    if ( bkr_selection_open_book( file_name, NULL, 0, TRUE, NULL ) ) {
    	    	XFlush( bkr_display );
    	    }
    	} else {

    	    BKR_BOOK_CTX *book;
            Boolean book_opened = FALSE;
    	    BMD_OBJECT_ID   	chunk_id = 0;

    	    book = bkr_book_get(file_name, NULL, 0);
    	    if ( book ) {
    	    	chunk_id = bri_symbol_to_chunk( book->book_id, symbol_name );
    	    	if ( (chunk_id > 0)
                    && bkr_topic_open_to_chunk( book, 0, chunk_id )
                    ) {
                    XFlush( bkr_display );
                } else {
                    /* No target, so just close the book if there are
                     * no other shells.
                     */
                    bkr_book_free(book);
                }
            }
    	}
    }

    /* Open the specified library.
     */
    bkr_library_open_new_library(bkr_library_resources.defaultLibraryName);

#ifdef MEMEX
    /* Check to see if we were coldstarted by LinkWorks and do an apply on the
     * surrogate that caused the coldstart if we were.
     */
    bmi_apply_coldstart_surrogate();
#endif 

    XFlush( bkr_display );

};  /* end of initialize_library_window */

/*	Function Name: bkrAppInitialize
 *	Description: A convience routine for Initializing the toolkit.
 *	Arguments: app_context_return - The application context of the
 *                                      application
 *                 application_class  - The class of the application.
 *                 options            - The option list.
 *                 num_options        - The number of options in the above list
 *                 argc_in_out, argv_in_out - number and list of command line
 *                                            arguments.
 *                 fallback_resource  - The fallback list of resources.
 *                 args, num_args     - Arguements to use when creating the 
 *                                      shell widget.
 *	Returns: The shell widget.
 */
	
static Widget
bkrAppInitialize(app_context_return, application_class, options, num_options,
		argc_in_out, argv_in_out, fallback_resources, 
		args_in, num_args_in)
XtAppContext * app_context_return;
String application_class;
XrmOptionDescRec *options;
Cardinal num_options, *argc_in_out, num_args_in;
String *argv_in_out, * fallback_resources;     
ArgList args_in;
{
    XtAppContext app_con;
    Display * dpy;
    String *saved_argv;
    register int i, saved_argc = *argc_in_out;
    Widget root;
    Arg args[3], *merged_args;
    Cardinal num = 0;
    
    XtToolkitInitialize();
    
/*
 * Save away argv and argc so we can set the properties later 
 */
    
    saved_argv = (String *)
	BKR_MALLOC( (Cardinal)((*argc_in_out + 1) * sizeof(String)) );

    for (i = 0 ; i < saved_argc ; i++) saved_argv[i] = argv_in_out[i];
    saved_argv[i] = NULL;	/* NULL terminate that sucker. */


    app_con = XtCreateApplicationContext();

    if (fallback_resources != NULL) /* save a procedure call */
	XtAppSetFallbackResources(app_con, fallback_resources);

    dpy = XtOpenDisplay(app_con, (String) NULL, NULL, application_class,
			options, num_options, argc_in_out, argv_in_out);

    if (dpy == NULL) 
    {
	XtErrorMsg("invalidDisplay","xtInitialize","XtToolkitError",
                   "Can't Open display", (String *) NULL, (Cardinal *)NULL);
        exit(1);
    }
#if !defined(vms) && !defined(NO_CC)
    if (bkrplus_g_charcell_display) 
    {
        DXcmInitializeExtensions(dpy);
    }
#endif 

    XtSetArg(args[num], XtNscreen, DefaultScreenOfDisplay(dpy)); num++;
    XtSetArg(args[num], XtNargc, saved_argc);	                 num++;
    XtSetArg(args[num], XtNargv, saved_argv);	                 num++;

    merged_args = XtMergeArgLists(args_in, num_args_in, args, num);
    num += num_args_in;

    root = XtAppCreateShell(NULL, application_class, 
			    applicationShellWidgetClass,dpy, merged_args, num);
    
    if (app_context_return != NULL)
	*app_context_return = app_con;

    XtFree((XtPointer)merged_args);
    BKR_FREE(saved_argv);
    return(root);
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	initialize_toolkit
**
** 	Initializes Mrm, Xt toolkit, and opens 
**  	the Mrm hierarchy UID file.
**
**  FORMAL PARAMETERS:
**
**	argc - command line parameter count
**	argv - address of a vector of pointers to command line parameters
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
**	TRUE if initialization successful otherwise FALSE
**
**  SIDE EFFECTS:
**
**	virtual memory is allocated.
**
**--
**/
static unsigned
initialize_toolkit( argc, argv )
    int	    argc;
    char    **argv;
{
    unsigned	status;
    /* Initialize Mrm and DXm before we initialize the Xt toolkit */

    MrmInitialize();
    DXmInitialize();

    /*
     * Initialize the toolkit before we open the Mrm Hierarchy so the toolkit
     * will default the rest of the UID file name for us.
     */
    bkr_toplevel_widget = bkrAppInitialize(
    	    	    	    &bkr_app_context,  	    /* Application context  */
		    	    BKR_APPLICATION_CLASS,  /* Application class    */
		    	    NULL, 0,		    /* Options, Count       */
    	    	    	    &argc, argv,            /* Command line 	    */
    	    	    	    NULL,   	    	    /* Fallback resources   */
    	    	    	    NULL, 0 );	    	    /* Override Arg, cnt    */

    if ( bkr_toplevel_widget == 0 )
	return( FALSE );

    /* Define the Mrm hierarchy */

    status = MrmOpenHierarchy(
    	    	    1,		    	    /* number of files      */
		    uid_file, 		    /* files     	    */
		    NULL,		    /* os_ext_list (null)   */
	    	    &bkr_hierarchy_id );    /* hierarchy id	    */

    if ( status != MrmSUCCESS )
	return( FALSE );

    /* Register the callback routines with Mrm */

    status = MrmRegisterNames( callback_reglist, callback_reglist_cnt );
    if ( status != MrmSUCCESS )
	return( FALSE );

    /* Setup the Xlib error handler and the Xt warning handler */

    XSetErrorHandler( bkr_error_xlib_non_fatal );
    XtSetWarningHandler( bkr_error_xt_warning_handler );

    /* Add the actions for our translation tables 
     */
    XtAppAddActions( bkr_app_context, action_routines, XtNumber(action_routines));
	/* add support for tear off menus */
	XmRepTypeInstallTearOffModelConverter();


    return( TRUE );

};	/* end initialize_toolkit */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_get_comline
**
**	This routine gets values from the command line used to invoke
**	the product.
**
**  FORMAL PARAMETERS:
**
**      argc		- command line parameter count.
**      argv		- address of a vector of pointers to command line
**			  parameters.
**
**		NOTE: The ARGC and ARGV are only used when Bookreader[Plus]
**		      is invoked without using the VMS DCL command line.
**
**	book		- pointer to buffer into which the book name
**			  should be placed.
**	book_len	- length of the book name buffer.
**	section		- pointer to buffer into which the section name
**			  should be placed.
**	section_len	- length of the section name buffer.
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**	bkrplus_g_charcell_display
**			- flag indicating if the user wanted a character
**			  cell display.
**
**  FUNCTION VALUE:
**
**	always returns 1
**
**  SIDE EFFECTS:
**
**	none
**--
**/
static int bkr_get_comline (argc, argv, book, book_len, section, section_len)

    int				argc;
    char			**argv;
    char			*book;
    int				book_len;
    char			*section;
    int				section_len;

{
    int				status 		= 0;
    int				arg_index	= 1;
    char			*param_ptr;

    /* Initialize parameter values 
     */
    book[0] = 0;
    section[0] = 0;

#if defined(VMS)
    /* Try to parse as if invoked from DCL */
    status = bkr_get_dcl_comline (book, book_len, section, section_len);
#endif

    /* Not called from VMS DCL command */
    if (status == 0)
	{

	if ( ( argc > arg_index ) && ( argv[arg_index] ) && ( *argv[arg_index] != NULLCHAR ) )
	    {                          
	    param_ptr = argv[arg_index];

	    /* Check to see if first parameter is a hyphen, indicating options */
	    if (param_ptr [0] == UNIX_INTERFACE_PREFIX [0])
		{
		/* Check to see if the user just wants to see the version number */
		if (param_ptr [1] == UNIX_INTERFACE_VERSION_CHAR [0])
		    {
			printf ("\n%s Version %s\n\n",BKR_APP_NAME,BKR_VERSION_STRING);
			exit (EXIT_SUCCESS);
		    }
		/* Check the value of the the INTERFACE switch */
		else if (param_ptr [1] == UNIX_INTERFACE_VAL_DECW [0])
		    {
		    bkrplus_g_charcell_display = 0;
		    }
		else if (param_ptr [1] == UNIX_INTERFACE_VAL_CHAR [0])
		    {
		    bkrplus_g_charcell_display = 1;
		    }

		if (strlen (param_ptr) > 2)
		    /* Check to see if the user just wants to see the version number */
		    if (param_ptr [2] == UNIX_INTERFACE_VERSION_CHAR [0])
			{
			printf("\n%s Version %s\n\n",BKR_APP_NAME,BKR_VERSION_STRING);
			exit(EXIT_SUCCESS);
			}
		/* Look at next parameter (only if current one was INTERFACE switch) */
		arg_index = arg_index + 1;
		}
	    }

	/* Check to see if BOOK parameter exists (1st if 1st wasn't INTERFACE, else 2nd) */
	if ( ( argc > arg_index ) && ( argv[arg_index] ) && ( *argv[arg_index] != NULLCHAR ) )
	    {
	    param_ptr = argv[arg_index];

	    strncpy (book, param_ptr, book_len);

	    arg_index = arg_index + 1;
	    }

	/* Check to see if SECTION parameter exists (2nd if 1st wasn't INTERFACE, else 3rd) */
	if ( ( argc > arg_index ) && ( argv[arg_index] ) && ( *argv[arg_index] != NULLCHAR ) )
	    {
	    param_ptr = argv[arg_index];

	    strncpy (section, param_ptr, section_len);
	    }
	}

    return (1);
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_get_dcl_comline
**
**	This routine gets values from the VMS DCL command line that
**	was used to invoke the product.
**
**  FORMAL PARAMETERS:
**
**	book		- pointer to buffer into which the book name
**			  should be placed.
**	book_len	- length of the book name buffer.
**	section		- pointer to buffer into which the section name
**			  should be placed.
**	section_len	- length of the section name buffer.
**
**  IMPLICIT INPUTS:
**
**	bkrplus_g_charcell_display
**			- flag indicating if the user wanted a character
**			  cell display.
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**	1		- DCL command line successfully parsed.
**	0		- DCL command line not successfully parsed.
**
**  SIDE EFFECTS:
**
**	none
**--
**/

#if defined(VMS) 

static int bkr_get_dcl_comline (book, book_len, section, section_len)

    char			*book;
    int				book_len;
    char			*section;
    int				section_len;

{
    int				status;

    struct dsc$descriptor	keyword;		/* String descriptor pointing to the above buffer	*/
    struct dsc$descriptor	keyword_value;
    short int			keyword_len;		/* Return length of a qualifier or parameter value	*/
    char			keyword_buffer [5];	/* Buffer to obtain qualifier and parameter value	*/
    

    /* Set up descriptors for /INTERFACE qualifier value */
    keyword.dsc$b_dtype = DSC$K_DTYPE_T;
    keyword.dsc$b_class = DSC$K_CLASS_S;

    keyword_value.dsc$b_dtype   = DSC$K_DTYPE_T;
    keyword_value.dsc$b_class   = DSC$K_CLASS_S;
    keyword_value.dsc$w_length  = sizeof (keyword_buffer);
    keyword_value.dsc$a_pointer = &keyword_buffer;


    /* Set condition handler (in case we were not invoked from DCL verb */
    LIB$ESTABLISH (bkr_comline_error_handler);


    /* Determine if /INTERFACE qualifier is present */
    keyword.dsc$w_length = strlen (DCL_INTERFACE_QUAL);
    keyword.dsc$a_pointer = DCL_INTERFACE_QUAL;

    status = CLI$PRESENT (&keyword);

    if (status == CLI$_PRESENT)
	{
	if (CLI$GET_VALUE (&keyword, &keyword_value, &keyword_len))
	    {
	    if (keyword_buffer [0] == DCL_INTERFACE_VAL_DECW [0])
		bkrplus_g_charcell_display = 0;
	    else if (keyword_buffer [0] == DCL_INTERFACE_VAL_CHAR [0])
		bkrplus_g_charcell_display = 1;
	    }
	}


    /* Set up descriptors for P1 parameter value */
    keyword.dsc$w_length = strlen (DCL_BOOK_PARAM);
    keyword.dsc$a_pointer = DCL_BOOK_PARAM;

    keyword_value.dsc$w_length  = book_len;
    keyword_value.dsc$a_pointer = book;

    if (CLI$PRESENT (&keyword) == CLI$_PRESENT)
	{
	if (CLI$GET_VALUE (&keyword, &keyword_value, &keyword_len))
	    book[keyword_len] = 0;
	else
	    return (1);
	}

    /* Set up descriptors for P2 parameter value */
    keyword.dsc$w_length = strlen (DCL_SECTION_PARAM);
    keyword.dsc$a_pointer = DCL_SECTION_PARAM;

    keyword_value.dsc$w_length  = section_len;
    keyword_value.dsc$a_pointer = section;

    if (CLI$PRESENT (&keyword) == CLI$_PRESENT)
	{
	if (CLI$GET_VALUE (&keyword, &keyword_value, &keyword_len))
	    section[keyword_len] = 0;
	}

    return (1);
}
#endif

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_comline_error_handler
**
**	This routine is an exception handler that is invoked when a request
**	to the CLI callback routines for command line information fails.  The
**	most common reason for the CLI$... routines to cause an exception is
**	the image was invoked with RUN rather than through the DCL command
**	tables.  In this case, we return (a zero value) to the calling
**	routine.
**
**  FORMAL PARAMETERS:
**
**      signal_arg		Array of signal arguments (unsigned longs)
**	mech_arg		Array of mechanism arguments (unsigned longs)
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**	0			- CLI$_SYNTAX encountered (return 0 to
**				  calling routine).
**      SS$_UNWIND		- Already in the process of unwinding (take
**				  no action).
**	SS$_RESIGNAL		- Unhandled condition encountered (resignal
**				  and process).
**
**  SIDE EFFECTS:
**
**	none
**--
**/

#if defined(VMS) 

static int bkr_comline_error_handler (signal_arg, mech_arg)
unsigned long int	signal_arg [];
unsigned long int	mech_arg [];
{

    int depth = 0;

    /*  If the user condition handler is in the UNWIND state, any 	*/
    /*  return status is ignored by the VAX condition handler; so	*/
    /*  just return.							*/

    switch (signal_arg [1])
	{
	case SS$_UNWIND :
	    return (SS$_UNWIND);	/* Currently unwinding, just return.	*/

	case CLI$_SYNTAX :
	    mech_arg [3] = 0;

	    return (SYS$UNWIND (0, 0));

	default :
	    return (SS$_RESIGNAL);
	}				/* End of switch on signal condition	*/

}
#endif

/* #if !defined(NO_CC) */
#ifndef vms
/* exit_on_signal
 *
 * Simple signal handler routine to do an exit on certain signals
 * (e.g. user types CTRL/C, SIGINT).  This gives us a chance to kill
 * off the character cell server and window manager, and reset the
 * terminal.  The exit will indirectly invoke the kill_charcell_server
 * exit handler.
 */
static void 
exit_on_signal PARAM_NAMES((signal_number))
    int signal_number PARAM_END
{
    exit(EXIT_FAILURE);
}

/* kill_charcell_server
 *
 * Exit handler to kill the character cell server and window manger and
 * reset the terminal.
 */
static void
kill_charcell_server(VOID_PARAM)
{
    if (charcell_server_id) 
    {
        kill(charcell_server_id,SIGKILL);
        
        if (charcell_wm_id) 
        {
            kill(charcell_wm_id,SIGKILL);
        }

        /* Make sure the window gets cleared after killing the server.
         * But we only do this if we tried to kill the server
         */
#if !defined(NO_CC)
        if (clear() == OK) {
            (void)refresh();
        }
#endif
    }
#if !defined(NO_CC)
    (void)resetty();
    (void)endwin();
#endif
}
#endif 
/* #endif */  /* NO_CC */

#ifndef VMS
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	define_terminfo
**
**	A ULTRIX / OSF only routine to make sure that the server will use
**      the right TERMINFO database.
**
**  FORMAL PARAMETERS:
**
**      none
**
**  IMPLICIT INPUTS:
**
**      The envrionment variables.
**
**  IMPLICIT OUTPUTS:
**
**      The environment variables.
**
**  FUNCTION VALUE:
**
**	None
**
**  SIDE EFFECTS:
**
**	May create a TERMINFO envrionment variable.
**--
**/

static void
define_terminfo()
{
    struct stat stat_buffer;
    char *dxbook_terminfo = "/usr/lib/dxbook_terminfo";

    if ((getenv("TERMINFO") == NULL) 
        && (stat(dxbook_terminfo,&stat_buffer) == 0)
        )
    {
        putenv("TERMINFO=/usr/lib/dxbook_terminfo");
    }
}
#endif 

/* #if !defined(NO_CC) */
#ifndef VMS
static char **
start_charcell_server PARAM_NAMES((argc,argv))
    int *argc PARAM_SEP
    char **argv PARAM_END
{
    char    *tty_name;
    char    display_string[20];
    int     num_offset;
    int     new_argc;
    char    **new_argv;
    char    **saved_argv = argv;
    Boolean reverseVideo = FALSE;

    static char display_variable[50];

    /* We may need to change the arguments handed off
     * to the toolkit for character cell mode.
     */
    new_argv = (char **)BKR_CALLOC(*argc+1,sizeof(char *));
    if (new_argv == NULL) 
    {
        exit(EXIT_FAILURE);
    }
    new_argc = 0;
    while (*argv) 
    {
        if (strcmp(*argv,"-display") == 0) 
        {
            /* In this case we want to return the original
             * argument vector unchanged.
             */
            BKR_FREE(new_argv);
            return saved_argv;
        }

        /* The -rv is passed through to the server, but we
         * need to remove it from the argument list before
         * initializing the toolkit, since the serve will be
         * handling the reverse video and passing it to the
         * toolkit will tend to cancel out what the server is
         * doing.
         */
        if (strcmp(*argv,"-rv") == 0)
        {
            reverseVideo = TRUE;
        }
        else 
        {
            /* Copy all arguments except the -rv.
             */
            new_argv[new_argc++] = *argv;
        }
        argv++;
    }
    *argc = new_argc;

#ifndef USE_TTY_NUM
    sprintf(display_string,":%d",(int)getpid());
#else
    tty_name = ttyname(fileno(stdin));
    num_offset = strcspn(tty_name,"0123456789");
    sprintf(display_string,":%s",&tty_name[num_offset]);
#endif 
    sprintf(display_variable,"DISPLAY=%s.0",display_string);
    putenv(display_variable);

    define_terminfo();

    /* Initialize and save the state of the screen so we can make sure
     * it is reset when we exit.
     */
#if !defined(NO_CC)
    (void)initscr();
    (void)savetty();
#endif

    /* Set up handler to do clean exit for certain signals.
     */
    signal(SIGHUP,exit_on_signal);
    signal(SIGINT,exit_on_signal);
    signal(SIGTERM,exit_on_signal);

    /* Set up exit handler to kill off server and window manager, and
     * reset the terminal.
     */
    (void)atexit(kill_charcell_server);

    charcell_server_id = vfork();
    if (charcell_server_id == 0) 
    {
        char *Xc_argv[6];
        
        Xc_argv[0] = "Xc";
        Xc_argv[1] = display_string;
        Xc_argv[2] = "-config";
        Xc_argv[3] = "/usr/lib/X11/XcServerConfig";
        if (reverseVideo) 
        {
            Xc_argv[4] = "-rv";
            Xc_argv[5] = NULL;
        }
        else 
        {
            Xc_argv[4] = NULL;
        }
        execvp("Xc",Xc_argv);
        fprintf(stderr,"\nCan not execute the character cell server (Xc).\n");
        exit( EXIT_FAILURE );
    }

    /* As long as the server proccess is still alive, keep trying
     * to open the display
     */
    while (XOpenDisplay(display_string) == NULL) 
    {
        if (kill(charcell_server_id,0) == 0)
        {
            sleep(1);
        }
        else 
        {
            charcell_server_id = 0;
            break;
        }
    }

    charcell_wm_id = vfork();
    if (charcell_wm_id == 0) 
    {
        execlp("xcwm",
              "xcwm",
              NULL);
        fprintf(stderr,"\nCan not execute the character cell window manager (xcwm).\n");
        exit( EXIT_FAILURE );
    }

    return new_argv;
}
#endif
/* #endif *//* NO_CC */

/*
 * Dummy routine for testing
 */
static void
bkr_dummy( widget, tag, reason )
    Widget	    	    widget;
    int 	    	    *tag;
    XmAnyCallbackStruct    *reason;
{

    if ( tag != NULL )
    	printf(" TAG passed was %d \n", *tag );

};  /* end of bkr_dummy */


#ifndef MEMEX
/*
 * Null callback
 */
static void
bkr_null_callback( widget, tag, reason )
    Widget	    	    widget;
    int 	    	    *tag;
    XmAnyCallbackStruct    *reason;
{
};  /* end of bkr_null_callback */
#endif

/*
 * debugging routines
 */

void
synchronize( onoff )
    int	    onoff;
{
    XSynchronize( bkr_display, onoff );
    printf( "Display output synchronization %s.\n",
	onoff ? "enabled" : "disabled" );
};
void
sync( discard )
    int	    discard;
{
    XSync( bkr_display, discard );
    printf( "Buffer flushed%s.\n", 
    	discard ? "--input events discarded" : "" );
};

void
get_values( widget )
    Widget  widget;
{
    int	    	argcnt;
    Arg	    	arglist[5];
    char    	value_name[50];
    short int	value;

    argcnt = 0;
    SET_ARG( value_name, &value );
    XtGetValues( widget, arglist, argcnt );

    printf( "%s for %s = %d\n", value_name, XtName( widget ), value );

};

void
set_values( w, value )
    Widget  	w;
    short int	value;
{
    int	    	argcnt;
    Arg	    	arglist[5];
    char    	value_name[50];

    argcnt = 0;
    SET_ARG( value_name, value );
    XtSetValues( w, arglist, argcnt );

    printf( "%s set to be %d on %s\n", value_name, value, XtName( w ) );

};

show_map_state( map_state )
    int map_state;
{

    switch ( map_state )
    {
    	case IsUnmapped :
    	    printf( " IsUnmapped \n" );
    	    break;
    	case IsUnviewable :
    	    printf( " IsUnviewable \n" );
    	    break;
    	case IsViewable :
    	    printf( " IsViewable \n" );
    	    break;
    }
};
