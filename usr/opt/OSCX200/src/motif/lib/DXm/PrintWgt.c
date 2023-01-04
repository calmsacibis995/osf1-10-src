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
/*  DEC/CMS REPLACEMENT HISTORY, Element PRINTWGT.C */
/************************************************************************/
/*									*/
/*	Copyright (c) Digital Equipment Corporation, 1990  		*/
/*	All Rights Reserved.  Unpublished rights reserved		*/
/*	under the copyright laws of the United States.			*/
/*									*/
/*	The software contained on this media is proprietary		*/
/*	to and embodies the confidential technology of 			*/
/*	Digital Equipment Corporation.  Possession, use,		*/
/*	duplication or dissemination of the software and		*/
/*	media is authorized only pursuant to a valid written		*/
/*	license from Digital Equipment Corporation.			*/
/*									*/
/*	RESTRICTED RIGHTS LEGEND   Use, duplication, or 		*/
/*	disclosure by the U.S. Government is subject to			*/
/*	restrictions as set forth in Subparagraph (c)(1)(ii)		*/
/*	of DFARS 252.227-7013, or in FAR 52.227-19, as			*/
/*	applicable.							*/
/*									*/
/************************************************************************/
/************************************************************************/
/*									*/
/*   FACILITY:								*/
/*									*/
/*	  Print Widget							*/
/*									*/
/*   ABSTRACT:								*/
/*									*/
/*	This module is the main module for the print widget.  It	*/
/*	contains the following public interfaces:			*/
/*									*/
/*	DXmCreatePrintBox-- performs the print widget initialization	*/
/*		and the interactive operation of the screen as seen by	*/
/*		the user.  This is the non-popup version.	       	*/
/*									*/
/*	DXmCreatePrintDialog-- performs the print widget initialization	*/
/*		and the interactive operation of the screen as seen by	*/
/*		the user.  This is the popup version.			*/
/*									*/
/*	DXmPrintWgtPrintJob -- performs the actual submission of files	*/
/*		to the print services of the underlying operation	*/
/*		system, in accordance with the submission information	*/
/*		gathered by the DXmPrintWidget.				*/
/*									*/
/*	DXmPrintWgtAugmentList -- Adds something to the user interface. */
/*									*/
/*	DXmCreatePrintWgt-- performs the print widget initialization	*/
/*		and the interactive operation of the screen as seen by	*/
/*		the user. (Obsolete - Use DXmCreatePrintDialog instead.)*/
/*									*/
/*									*/
/*   AUTHORS:								*/
/*									*/
/*	Print Wgt Team							*/
/*									*/
/*   CREATION DATE:							*/
/*									*/
/*	Winter/Spring 1990						*/
/*									*/
/*   MODIFICATION HISTORY:						*/
/*									*/
/*	050	WDW			16-Jun-1993			*/
/*		CLD CFS.4031.  Make cs choice values be pointers into	*/
/*		the CS list rather than copies.				*/
/*      049     AD			2-Feb-1993                      */
/*              convert static warning and error messages to            */
/*		character arrays					*/
/*      048     PAR			6-Jan-1992                      */
/*              add drag and drop to printer list field                 */
/*	047	CHS			14-Jul-1991			*/
/*		CLD UVO01299.  Fix pw_handle_suppress_options_mask	*/
/*		so that it set the sizes of the forms in the primary	*/
/*		print box correctly when an option is suppressed.	*/
/*		Remove the call to pw_handle_suppress_options_mask from	*/
/*		pw_expose and add a call to the suppress function in	*/
/*		pw_initialize.						*/
/*	046	WDW			15-Jul-1991			*/
/*		During a call to XtSetValues, the print widget should	*/
/*		check both the contents and values of the pointers of	*/
/*		the old and new values for compound strings.		*/
/*	045	WDW			20-May-1991			*/
/*		Remove defines of PRINTWGT_DEBUG and NO_FREE_MRM	*/
/*		Add fix for setting print format which results in	*/
/*		getting a different printer list.			*/
/*	044	WDW			11-Apr-1991			*/
/*		Remove obsolete resources.				*/
/*	043	WDW			03-Apr-1991			*/
/*		Add checks for cs text widget based resources.		*/
/*	042	WDW			29-Mar-1991			*/
/*		Work on function prototypes.				*/
/*	041	WDW			18-Mar-1991			*/
/*		Fix bug where application sets value of print format	*/
/*		list after they have popped it up and before the user	*/
/*		hits the "Cancel" button.				*/
/*	040	WDW			14-Mar-1991			*/
/*		Add DXmCreatePrintBox and move code from create to	*/
/*		initialize.  This is to accomodate VUIT's requests.	*/
/*	039	WDW			21-Feb-1991			*/
/*		Add /passall /feed behavior as recommeded by DECprint.	*/
/*	038	WDW			06-Feb-1991			*/
/*		Add "Default" print format.				*/
/*	037	WDW			05-Feb-1991			*/
/*		Fix ULTRIX compile warnings.				*/
/*	036	WDW			28-Jan-1991			*/
/*		Make ULTRIX suppress options not include JOB NAME.	*/
/*	035	WDW			21-Jan-1991			*/
/*		Make Boolean resources really be Boolean resources.	*/
/*	034	WDW			11-Jan-1991			*/
/*		Make DXmN{ok|cancel}selectedCallbackList map to		*/
/*		XmN{ok|cancel}Callback for consistency issues.		*/
/*		Also make DXmNunmapOn{Ok|Cancel}Selected be		*/
/*		DXmNunmanageOn{Ok|Cancel} for more consistency.		*/
/*	033	WDW			10-Jan-1991			*/
/*		Remove XmNokCallback and XmNcancelCallback.		*/
/*	032	WDW			07-Jan-1991			*/
/*		Add private resources for .UID and Help filenames.      */
/*		Also add public resource for name of 2ndary box.	*/
/*	031	WDW			19-Nov-1990			*/
/*		More Style Guide Compliancy.				*/
/*	030	WDW			19-Nov-1990			*/
/*		More Style Guide Compliancy.				*/
/*	029	WDW			02-Nov-1990			*/
/*		Style Guide Compliancy.					*/
/*	028	WDW			17-Oct-1990			*/
/*		Work on memory leaks.					*/
/*		Add callback reasons to callbacks.			*/
/*	027	WDW			09-Oct-1990			*/
/*		Convert to use DXMAPPWARNING and DXMAPPERROR		*/
/*	026	WDW			09-Oct-1990			*/
/*		Make 2ndary box always be unmanaged on OK or Cancel.    */
/*		The window manager seems to get confused if we don't.	*/
/*		Also, use XtPopdown and XtPopup when handling the form	*/
/*		dialog box for 2ndary box.				*/
/*	025	WDW			08-Oct-1990			*/
/*		Conditionalize compilation of debug code.		*/
/*	024	WDW			20-Sep-1990			*/
/*		Add augment list capability.				*/
/*	023	WDW			18-Sep-1990			*/
/*		Fix up known print format list stuff.			*/
/*	022	WDW			18-Sep-1990			*/
/*		Add "Print After" capability to ULTRIX.			*/
/*	021	WDW			31-Aug-1990			*/
/*		Put free memory stuff back in for DXmCSTextGetString	*/
/*	020	WDW			30-Aug-1990			*/
/*		Add default button code.				*/
/*	019	WDW			28-Aug-1990			*/
/*		Remove help and cancel buttons from message box.	*/
/*	018	WDW			22-Aug-1990			*/
/*		Make sure now string is copied on validation of		*/
/*		initial values instead of referenced.  Also, since	*/
/*	        DXmCSTextGetString does give back memory to the user	*/
/*		it is necessary to avoid freeing this memory and	*/
/*		it is necessary to copy it if it is to be used later.	*/
/*	017	WDW			17-Aug-1990			*/
/*		Add ability to suppress certain options.		*/
/*	016	WDW			16-Aug-1990			*/
/*		Move wait cursor support to DXmMisc.c.			*/
/*	015	WDW			15-Aug-1990			*/
/*		Add definition of MIN for ULTRIX.			*/
/*		Reduce flash on screen from I14Y stuff.			*/
/*	014	WDW			09-Aug-1990			*/
/*		Add validate values on startup.  This is a must in order*/
/*		close a QAR (those whining babies :-)).			*/
/*	013	WDW			09-Aug-1990			*/
/*		Work on I14Y.  The primary box doesn't seem to want to	*/
/*		work, but the 2ndary works fine.  The secondary box was	*/
/*		modified to be a FormDialog which controls a scrolled	*/
/*		window which controls a Form.  Upon manage, the scrolled*/
/*		window's size is set to match the screen size.		*/
/*	012	WDW			20-Jul-1990			*/
/*		Make pw_xm_string_compare as sub for XmStringCompare.	*/
/*	011	WDW			18-Jul-1990			*/
/*		Remove DXmPrintWgtInitForMrm.				*/
/*	010	WDW			17-Jul-1990			*/
/*		Fix a couple resource initialization problems.		*/
/*	009	WDW			16-Jul-1990			*/
/*		Add additional params for DXmCvt...			*/
/*	008	WDW			13-Jul-1990			*/
/*		I18N work.  Add DXmCreatePrintDialog.			*/
/*	007	WDW			06-Jul-1990			*/
/*		Put DXmCSText in interface instead of XmText.  Make	*/
/*		resources be XmString's instead of char *'s.		*/
/*	006	WDW			28-Jun-1990			*/
/*		Add "wait" cursor support.				*/
/*	005	WDW			08-Jun-1990			*/
/*		Remove type casting for inherit translations.		*/
/*	004	WDW			31-May-1990			*/
/*		Add help support.					*/
/*	003	WDW			21-May-1990			*/
/*		Add XtInheritParentProcess to Manager Class.		*/
/*	002	WDW			27-Mar-1990			*/
/*		Reorganize.						*/
/*	001	WDW			19-Mar-1990			*/
/*		Add modification history.  Change names of resources to	*/
/*		match Xm naming conventions.				*/
/*									*/
/************************************************************************/
#define PRINTWGT


/************************************************************************/
/*									*/
/* INCLUDE FILES							*/
/*									*/
/************************************************************************/
#include <Xm/XmP.h>
#include <Mrm/MrmPublic.h>
#include <X11/StringDefs.h>
#include <X11/ShellP.h>
#include <Xm/BulletinBP.h>
#include <Xm/DialogSP.h>
#include <Xm/FormP.h>
#include <Xm/List.h>
#include <Xm/RowColumnP.h>
#include <Xm/AtomMgr.h>
#include <Xm/DragDrop.h>
#include <DXm/DXmHelpB.h>
#include <DXm/DXmPrintP.h>
#include "DXmMessI.h"
#include "DXmPrivate.h"

/************************************************************************/
/*									*/
/* Message definitions for character arrays				*/
/*									*/
/************************************************************************/
#define PWGTNOPARAM	_DXmMsgPrintWgt_0000
#define PWGTNOPARAMD	_DXmMsgPrintWgt_0001
#define PWGTNOCONVCSFL	_DXmMsgPrintWgt_0002
#define PWGTNOMRMFETCH	_DXmMsgPrintWgt_0003
#define PWGTNOCONVCSPR	_DXmMsgPrintWgt_0004
#define PWGTNOCONVCSPRF	_DXmMsgPrintWgt_0005
#define PWGTNOCONVCSTM	_DXmMsgPrintWgt_0006
#define PWGTNOMRMOPEN	_DXmMsgPrintWgt_0007

#define PWMSGNAME0	_DXmMsgPWgtName_0000
#define PWMSGNAME1	_DXmMsgPWgtName_0001
#define PWMSGNAME2	_DXmMsgPWgtName_0002
#define PWMSGNAME3	_DXmMsgPWgtName_0003
#define PWMSGNAME4	_DXmMsgPWgtName_0004
#define PWMSGNAME5	_DXmMsgPWgtName_0005
#define PWMSGNAME6	_DXmMsgPWgtName_0006
/************************************************************************/
/*									*/
/* MACRO DEFINITIONS							*/
/*									*/
/************************************************************************/

#ifndef MAX
#define MAX(x,y)        ((x) > (y) ? (x) : (y))
#endif

#define INHERIT_COMPOSITE(to,from,what) \
    ((to)->composite_class.what = (from)->composite_class.what)


/************************************************************************/
/*									*/
/* Locations where we expect to find the print widget definitions and	*/
/* HELP file on either VMS or Ultrix.  (When debugging, look in current */
/* directory.)								*/
/*									*/
/************************************************************************/
#ifdef VMS
#   define PW_UID_FILE_LOCATION		"DECW$DXmPrintWgt_UI12"       	/* logical or file */
#   define PW_HELP_FILE_LOCATION	"DECW$DXmPrintWgt_Help"
#else
#   define PW_UID_FILE_LOCATION		"DXmPrintWgt_UI_1_2"
#   define PW_HELP_FILE_LOCATION	"DXmPrintWgt_Help"
#endif

static Opaque help_context = (Opaque) NULL;


/************************************************************************/
/*									*/
/* FORWARD DECLARATIONS							*/
/*									*/
/************************************************************************/
/* Routines for dealing with widgets */
static void		pw_class_initialize();
static void		pw_destroy();
static void		pw_expose();
static void		pw_initialize();
static Boolean		pw_set_values();

/* Routines for dealing with Mrm */
static Cardinal		pw_fetch_mrm_resources();
static Cardinal		pw_fetch_pulldown_menu_os_list();
static unsigned int	pw_activate_proc();
static unsigned int	pw_create_proc();
static unsigned int	pw_pulldown_menu_create_proc();
static unsigned int	pw_help_proc();
static void		pw_help_error();
static unsigned int	pw_resource_activate_proc();
static unsigned int	pw_pulldown_menu_activate_proc();

/* Utility routines */
static Boolean		pw_required_arguments();
static void		pw_copy_memory_on_create();

static void		pw_set_value_boolean();
static void		pw_save_value_boolean();
static void		pw_restore_value_boolean();

static Boolean		pw_in_bounds();

static Boolean		pw_validate_value_int();
static void		pw_set_value_int();
static void		pw_save_value_int();
static void		pw_restore_value_int();

static Boolean		pw_validate_value_text_length();
static Boolean		pw_validate_value_text_contents();
static void		pw_set_value_cs();
static void		pw_set_value_2ndary_title();
static void		pw_save_value_cs();
static void		pw_restore_value_cs();

static Boolean		pw_validate_value_print_format_list();
static void		pw_set_value_print_format_list();
static Boolean		pw_validate_value_filename_list();
static void		pw_set_value_filename_list();
static void		pw_set_value_readonly_cs_list();

static Boolean		pw_validate_value_cs_choice();
static void		pw_set_value_cs_choice();
static void		pw_save_value_cs_choice();
static void		pw_restore_value_cs_choice();

static Boolean		pw_validate_value_time();

static void		pw_listbox_activate();
static void		pw_radiobox_activate();
static void		pw_toggle_activate();
static void		pw_optionmenu_activate();

static void		pw_save_values();
static void		pw_restore_values();

static void		pw_make_widgets_match_resources();
static void		pw_make_resources_match_widgets();

static void		pw_display_a_message();
static Boolean		pw_validate_cs_widget_based_resources();

static int		pw_get_queue_list_from_variable();

static Boolean		pw_decide_printer_queue_choice();
static void		pw_populate_printer_queue_listbox();
static void		pw_propagate_print_format_choice();
static Boolean		pw_guess_print_format_choice();
static Boolean		pw_decide_print_format_choice();
static void		pw_populate_print_format_listbox();

static void		pw_handle_suppress_options_mask();

static void		pw_turn_on_wait_cursor();
static void		pw_turn_off_wait_cursor();
    
static void		pw_handle_form_dialog();
static Boolean		pw_validate_startup_values();

#if 0
static void 		pw_set_listbox_widget_selection_from_resource();
#endif
static void		pw_set_listbox_widget_contents_from_resource();

static void		pw_adjust_application_child_location();

static void		pw_tabify_primary_box();
static void		pw_tabify_2ndary_box();

static void		pw_fetch_2ndary_box();

#ifdef _NO_PROTO
static void		pw_drop_proc_callback();
static void             pw_drop_help_ok_callback();
static void             pw_drop_help_cancel_callback();
#else
static void pw_drop_proc_callback(Widget w, XtPointer client, XtPointer call);
static void pw_drop_help_ok_callback(Widget w, XtPointer client, XtPointer call);
static void pw_drop_help_cancel_callback(Widget w, XtPointer client, XtPointer call);
#endif

static void		pw_register_drop_site();

typedef struct _DropTransferRec
{
     Widget widget;
} DropTransferRec, *DropTransfer;

typedef struct _DropHelpCallbackStruct {
     Widget       dragContext;
     Widget       w;
} DropHelpCallbackStruct;




/************************************************************************/
/*									*/
/* The names and addresses of callback routines.  Note: we keep the	*/
/* names in lexicographic order so that the resource manager can do	*/
/* a quick lookup at initialization time.				*/
/*									*/
/************************************************************************/
#define ARRAY_SIZE(array)  ((int) (sizeof(array)/sizeof(array[0])))

/* Mrm arguments */
static MrmRegisterArg	ar_mrm_register_args[] =
{
    {"pw_activate_proc",		(XtPointer)pw_activate_proc},
    {"pw_create_proc",			(XtPointer)pw_create_proc},
    {"pw_pulldown_menu_create_proc",	(XtPointer)pw_pulldown_menu_create_proc},
    {"pw_help_proc",			(XtPointer)pw_help_proc},
    {"pw_resource_activate_proc",	(XtPointer)pw_resource_activate_proc},
    {"pw_pulldown_menu_activate_proc",	(XtPointer)pw_pulldown_menu_activate_proc}
};
static const MrmCount	l_mrm_register_args_count = ARRAY_SIZE(ar_mrm_register_args);

static char *ar_mrm_filename_vec[1]; 
static const MrmCount	l_mrm_filename_count = ARRAY_SIZE(ar_mrm_filename_vec);


/************************************************************************/
/*									*/
/* Values for handling the pulldown menus.				*/
/*									*/
/* This must have the same number of pulldowns as the Print Widget	*/
/* instance, *and* they must be in the same order.			*/
/*									*/
/*        [[[Needs documenting]]]					*/
/*									*/
/************************************************************************/
static pw_r_pulldown_menu_data_struct pw_ar_pulldown_menu_data_table[K_PW_NUM_PULLDOWN_MENUS] =
{
    { /* Page Size */
	K_PW_PAGE_SIZE_MAP,
	K_PW_PAGE_SIZE_OBJECTS_NAME
    },
    { /* Sides */
	K_PW_SIDES_MAP,
	K_PW_SIDES_OBJECTS_NAME
    },
    { /* File Start Sheet */
	K_PW_FILE_START_SHEET_MAP,
	K_PW_FILE_START_SHEET_OBJECTS_NAME
    },
    { /* File End Sheet */
	K_PW_FILE_END_SHEET_MAP,
	K_PW_FILE_END_SHEET_OBJECTS_NAME
    },
    { /* File Burst Sheet */
	K_PW_FILE_BURST_SHEET_MAP,
	K_PW_FILE_BURST_SHEET_OBJECTS_NAME
    },
    { /* Message Log */
	K_PW_MESSAGE_LOG_MAP,
	K_PW_MESSAGE_LOG_OBJECTS_NAME
    },
    { /* Sheet Size */
	K_PW_SHEET_SIZE_MAP,
	K_PW_SHEET_SIZE_OBJECTS_NAME
    },
    { /* Input Tray */
	K_PW_INPUT_TRAY_MAP,
	K_PW_INPUT_TRAY_OBJECTS_NAME
    },
    { /* Output Tray */
	K_PW_OUTPUT_TRAY_MAP,
	K_PW_OUTPUT_TRAY_OBJECTS_NAME
    },
};


/************************************************************************/
/*									*/
/* Values for verifying the validity of resources.			*/
/*									*/
/* This must have the same number of resources as the Print Widget	*/
/* instance, *and* they must be in the same order.			*/
/*									*/
/*        [[[Needs documenting]]]					*/
/*									*/
/************************************************************************/
static const pw_r_validate_value_struct pw_ar_validation_value[K_PW_NUM_RESOURCES] =
{
    { /* Number Copies */
	K_PW_BOUNDED_MIN | K_PW_BOUNDED_MAX,
	0,
	255
    },
    { /* Page Range From */
	0,
	0,
	0
    },
    { /* Page Range To */
	0,
	0,
	0
    },
    { /* Print Format Contents */
	0,
	0,
	0
    },
    { /* Print Format Choice */
	0,
	0,
	0
    },
    { /* Orientation */
	K_PW_BOUNDED_MIN | K_PW_BOUNDED_MAX,
	0,
	2
    },
    { /* Printer Contents */
	0,
	0,
	0
    },
    { /* Printer Choice */
	0,
	0,
	0
    },
    { /* Print After */
	0,
	0,
	0
    },
    { /* Delete File */
	0,
	0,
	0
    },
    { /* Page Size */
	K_PW_BOUNDED_MIN | K_PW_BOUNDED_MAX,
	0,
	29 /* Matches max allowed spots for push buttons */
    },
    { /* Sides */
	K_PW_BOUNDED_MIN | K_PW_BOUNDED_MAX,
	0,
	9 /* Matches max allowed spots for push buttons */
    },
    { /* Number Up */
	K_PW_BOUNDED_MIN | K_PW_BOUNDED_MAX,
	0,
	100
    },
    { /* Sheet Count */
	K_PW_BOUNDED_MIN | K_PW_BOUNDED_MAX,
	0,
	10000
    },
    { /* File Start Sheet */
	K_PW_BOUNDED_MIN | K_PW_BOUNDED_MAX,
	0,
	4 /* Matches max allowed spots for push buttons */
    },
    { /* File End Sheet */
	K_PW_BOUNDED_MIN | K_PW_BOUNDED_MAX,
	0,
	4 /* Matches max allowed spots for push buttons */
    },
    { /* File Burst Sheet */
	K_PW_BOUNDED_MIN | K_PW_BOUNDED_MAX,
	0,
	4 /* Matches max allowed spots for push buttons */
    },
    { /* Message Log */
	K_PW_BOUNDED_MIN | K_PW_BOUNDED_MAX,
	0,
	4 /* Matches max allowed spots for push buttons */
    },
    { /* Hold Job */
	0,
	0,
	0
    },
    { /* Notify When Done */
	0,
	0,
	0
    },
    { /* Sheet Size */
	K_PW_BOUNDED_MIN | K_PW_BOUNDED_MAX,
	0,
	29 /* Matches max allowed spots for push buttons */
    },
    { /* Input Tray */
	K_PW_BOUNDED_MIN | K_PW_BOUNDED_MAX,
	0,
	9 /* Matches max allowed spots for push buttons */
    },
    { /* Output Tray */
	K_PW_BOUNDED_MIN | K_PW_BOUNDED_MAX,
	0,
	9 /* Matches max allowed spots for push buttons */
    },
    { /* Job Name */
	K_PW_BOUNDED_MAX,
	0,
        39
    },
    { /* Operator Message */
	K_PW_BOUNDED_MAX,
	0,
        255
    },
    { /* Header */
	0,
	0,
	0
    },
    { /* Automatic Pagination */
	0,
	0,
	0
    },
    { /* Double Spacing */
	0,
	0,
	0
    },
    { /* Layup Definition */
	K_PW_BOUNDED_MAX,
	0,
        79
    },
    { /* Start Sheet Comment */
	K_PW_BOUNDED_MAX,
	0,
        255
    },
    { /* Pass All */
	0,
	0,
	0
    },
    { /* Printer Form Contents */
	0,
	0,
	0
    },
    { /* Printer Form Choice */
	0,
	0,
	0
    },
    { /* Priority */
	K_PW_BOUNDED_MIN | K_PW_BOUNDED_MAX,
	0,
	255
    },
    { /* Setup */
	K_PW_BOUNDED_MAX,
	0,
        79
    },
    { /* Debug */
	0,
	0,
	0
    },
    { /* OK Callback List */
	0,
	0,
	0
    },
    { /* Unmanage on OK Selected */
	0,
	0,
	0
    },
    { /* Cancel Callback List */
	0,
	0,
	0
    },
    { /* Unmanage on Cancel Selected */
	0,
	0,
	0
    },
    { /* Filenames */
	0,
	0,
	0
    },
    { /* Known Print Formats */
	0,
	0,
	0
    },
    { /* Known Print Format Variables */
	0,
	0,
	0
    },
    { /* Known Print Format Op Sys Mappings */
	0,
	0,
	0
    },       
    { /* Known Print Format Guesser Mappings */
	0,
	0,
	0
    },       
    { /* Default Printer */
	0,
	0,
	0
    },       
    { /* Suppress Options */
	0,
	0,
	0
    },       
};


/************************************************************************/
/*									*/
/* Definition of the resource control structure				*/
/*									*/
/* This must have the same number of resources as the Print Widget	*/
/* instance, *and* they must be in the same order.			*/
/*									*/
/*        [[[Needs documenting]]]					*/
/*									*/
/************************************************************************/
static pw_r_resource_control_struct pw_ar_resource_control[K_PW_NUM_RESOURCES] =
{
    { /* Number Copies */
	K_PW_RESOURCE_TYPE_CS_INT,			/* type 		*/
	K_PW_NUMBER_COPIES,				/* widget index 	*/
	NULL,						/* widget activate proc */
	pw_set_value_int,				/* set value proc 	*/
	pw_save_value_int,				/* save value proc 	*/
	pw_restore_value_int,				/* restore value proc 	*/
	pw_validate_value_int,				/* validate value proc 	*/
    },
    { /* Page Range From */
	K_PW_RESOURCE_TYPE_CS,				/* type 		*/
	K_PW_PAGE_RANGE_FROM,				/* widget index 	*/
	NULL,						/* widget activate proc */
	pw_set_value_cs,				/* set value proc 	*/
	pw_save_value_cs,				/* save value proc 	*/
	pw_restore_value_cs,				/* restore value proc 	*/
	NULL,						/* validate value proc 	*/
    },
    { /* Page Range To */
	K_PW_RESOURCE_TYPE_CS,				/* type 		*/
	K_PW_PAGE_RANGE_TO,				/* widget index 	*/
	NULL,						/* widget activate proc */
	pw_set_value_cs,				/* set value proc 	*/
	pw_save_value_cs,				/* save value proc 	*/
	pw_restore_value_cs,				/* restore value proc 	*/
	NULL,						/* validate value proc 	*/
    },
    { /* Print Format List and Count */
	K_PW_RESOURCE_TYPE_CS_LIST,			/* type 		*/
	K_PW_PRINT_FORMAT,				/* widget index 	*/
	pw_listbox_activate,				/* widget activate proc */
	pw_set_value_print_format_list,			/* set value proc 	*/
	NULL,						/* save value proc 	*/
	NULL,						/* restore value proc 	*/
	pw_validate_value_print_format_list,		/* validate value proc 	*/
    },
    { /* Print Format Choice */
	K_PW_RESOURCE_TYPE_CS_CHOICE,			/* type 		*/
	K_PW_PRINT_FORMAT,				/* widget index 	*/
	NULL,						/* widget activate proc */
	pw_set_value_cs_choice,				/* set value proc 	*/
	pw_save_value_cs_choice,			/* save value proc 	*/
	pw_restore_value_cs_choice,			/* restore value proc 	*/
	pw_validate_value_cs_choice,			/* validate value proc 	*/
    },
    { /* Orientation */
	K_PW_RESOURCE_TYPE_RADIOBOX,			/* type 		*/
	K_PW_ORIENTATION,				/* widget index 	*/
	pw_radiobox_activate,				/* widget activate proc */
	pw_set_value_int,				/* set value proc 	*/
	pw_save_value_int,				/* save value proc 	*/
	pw_restore_value_int,				/* restore value proc 	*/
	pw_validate_value_int,				/* validate value proc 	*/
    },
    { /* Printer List and Count */
	K_PW_RESOURCE_TYPE_CS_LIST,			/* type 		*/
	K_PW_PRINTER,					/* widget index 	*/
	pw_listbox_activate,				/* widget activate proc */
	NULL,						/* set value proc 	*/ /* No-one should be setting this... */
	NULL,						/* save value proc 	*/
	NULL,						/* restore value proc 	*/
	NULL,						/* validate value proc 	*/
    },
    { /* Printer Choice */
	K_PW_RESOURCE_TYPE_CS_CHOICE,			/* type 		*/
	K_PW_PRINTER,					/* widget index 	*/
	NULL,						/* widget activate proc */
	pw_set_value_cs_choice,				/* set value proc 	*/
	pw_save_value_cs_choice,			/* save value proc 	*/
	pw_restore_value_cs_choice,			/* restore value proc 	*/
	pw_validate_value_cs_choice,			/* validate value proc 	*/
    },
    { /* Print After */
	K_PW_RESOURCE_TYPE_CS,				/* type 		*/
	K_PW_PRINT_AFTER,				/* widget index 	*/
	NULL,						/* widget activate proc */
	pw_set_value_cs,				/* set value proc 	*/
	pw_save_value_cs,				/* save value proc 	*/
	pw_restore_value_cs,				/* restore value proc 	*/
	pw_validate_value_time,				/* validate value proc 	*/
    },
    { /* Delete File */
	K_PW_RESOURCE_TYPE_TOGGLE,			/* type 		*/
	K_PW_DELETE_FILE,				/* widget index 	*/
	pw_toggle_activate,				/* widget activate proc */
	pw_set_value_boolean,				/* set value proc 	*/
	pw_save_value_boolean,				/* save value proc 	*/
	pw_restore_value_boolean,			/* restore value proc 	*/
	NULL,						/* validate value proc 	*/
    },
    { /* Page Size */
	K_PW_RESOURCE_TYPE_OPTIONMENU,			/* type 		*/
	K_PW_PAGE_SIZE,					/* widget index 	*/
	pw_optionmenu_activate,				/* widget activate proc */
	pw_set_value_int,				/* set value proc 	*/
	pw_save_value_int,				/* save value proc 	*/
	pw_restore_value_int,				/* restore value proc 	*/
	pw_validate_value_int,				/* validate value proc 	*/
    },
    { /* Sides */
	K_PW_RESOURCE_TYPE_OPTIONMENU,			/* type 		*/
	K_PW_SIDES,					/* widget index 	*/
	pw_optionmenu_activate,				/* widget activate proc */
	pw_set_value_int,				/* set value proc 	*/
	pw_save_value_int,				/* save value proc 	*/
	pw_restore_value_int,				/* restore value proc 	*/
	pw_validate_value_int,				/* validate value proc 	*/
    },
    { /* Number Up */
	K_PW_RESOURCE_TYPE_CS_INT,			/* type 		*/
	K_PW_NUMBER_UP,					/* widget index 	*/
	NULL,						/* widget activate proc */
	pw_set_value_int,				/* set value proc 	*/
	pw_save_value_int,				/* save value proc 	*/
	pw_restore_value_int,				/* restore value proc 	*/
	pw_validate_value_int,				/* validate value proc 	*/
    },
    { /* Sheet Count */
	K_PW_RESOURCE_TYPE_CS_INT,			/* type 		*/
	K_PW_SHEET_COUNT,				/* widget index 	*/
	NULL,						/* widget activate proc */
	pw_set_value_int,				/* set value proc 	*/
	pw_save_value_int,				/* save value proc 	*/
	pw_restore_value_int,				/* restore value proc 	*/
	pw_validate_value_int,				/* validate value proc 	*/
    },
    { /* File Start Sheet */
	K_PW_RESOURCE_TYPE_OPTIONMENU,			/* type 		*/
	K_PW_FILE_START_SHEET,				/* widget index 	*/
	pw_optionmenu_activate,				/* widget activate proc */
	pw_set_value_int,				/* set value proc 	*/
	pw_save_value_int,				/* save value proc 	*/
	pw_restore_value_int,				/* restore value proc 	*/
	pw_validate_value_int,				/* validate value proc 	*/
    },
    { /* File End Sheet */
	K_PW_RESOURCE_TYPE_OPTIONMENU,			/* type 		*/
	K_PW_FILE_END_SHEET,				/* widget index 	*/
	pw_optionmenu_activate,				/* widget activate proc */
	pw_set_value_int,				/* set value proc 	*/
	pw_save_value_int,				/* save value proc 	*/
	pw_restore_value_int,				/* restore value proc 	*/
	pw_validate_value_int,				/* validate value proc 	*/
    },
    { /* File Burst Sheet */
	K_PW_RESOURCE_TYPE_OPTIONMENU,			/* type 		*/
	K_PW_FILE_BURST_SHEET,				/* widget index 	*/
	pw_optionmenu_activate,				/* widget activate proc */
	pw_set_value_int,				/* set value proc 	*/
	pw_save_value_int,				/* save value proc 	*/
	pw_restore_value_int,				/* restore value proc 	*/
	pw_validate_value_int,				/* validate value proc 	*/
    },
    { /* Message Log */
	K_PW_RESOURCE_TYPE_OPTIONMENU,			/* type 		*/
	K_PW_MESSAGE_LOG,				/* widget index 	*/
	pw_optionmenu_activate,				/* widget activate proc */
	pw_set_value_int,				/* set value proc 	*/
	pw_save_value_int,				/* save value proc 	*/
	pw_restore_value_int,				/* restore value proc 	*/
	pw_validate_value_int,				/* validate value proc 	*/
    },
    { /* Hold Print Job */
	K_PW_RESOURCE_TYPE_TOGGLE,			/* type 		*/
	K_PW_HOLD_JOB,					/* widget index 	*/
	pw_toggle_activate,				/* widget activate proc */
	pw_set_value_boolean,				/* set value proc 	*/
	pw_save_value_boolean,				/* save value proc 	*/
	pw_restore_value_boolean,			/* restore value proc 	*/
	NULL,						/* validate value proc 	*/
    },
    { /* Notify */
	K_PW_RESOURCE_TYPE_TOGGLE,			/* type 		*/
	K_PW_NOTIFY,					/* widget index 	*/
	pw_toggle_activate,				/* widget activate proc */
	pw_set_value_boolean,				/* set value proc 	*/
	pw_save_value_boolean,   			/* save value proc 	*/
	pw_restore_value_boolean,			/* restore value proc 	*/
	NULL,						/* validate value proc 	*/
    },
    { /* Sheet Size */
	K_PW_RESOURCE_TYPE_OPTIONMENU,			/* type 		*/
	K_PW_SHEET_SIZE,				/* widget index 	*/
	pw_optionmenu_activate,				/* widget activate proc */
	pw_set_value_int,				/* set value proc 	*/
	pw_save_value_int,				/* save value proc 	*/
	pw_restore_value_int,				/* restore value proc 	*/
	pw_validate_value_int,				/* validate value proc 	*/
    },
    { /* Input Tray */
	K_PW_RESOURCE_TYPE_OPTIONMENU,			/* type 		*/
	K_PW_INPUT_TRAY,				/* widget index 	*/
	pw_optionmenu_activate,				/* widget activate proc */
	pw_set_value_int,				/* set value proc 	*/
	pw_save_value_int,				/* save value proc 	*/
	pw_restore_value_int,				/* restore value proc 	*/
	pw_validate_value_int,				/* validate value proc 	*/
    },
    { /* Ouput Tray */
	K_PW_RESOURCE_TYPE_OPTIONMENU,			/* type 		*/
	K_PW_OUTPUT_TRAY,				/* widget index 	*/
	pw_optionmenu_activate,				/* widget activate proc */
	pw_set_value_int,				/* set value proc 	*/
	pw_save_value_int,				/* save value proc 	*/
	pw_restore_value_int,				/* restore value proc 	*/
	pw_validate_value_int,				/* validate value proc 	*/
    },
    { /* Job Name */
	K_PW_RESOURCE_TYPE_CS,				/* type 		*/
	K_PW_JOB_NAME,					/* widget index 	*/
	NULL,						/* widget activate proc */
	pw_set_value_cs,				/* set value proc 	*/
	pw_save_value_cs,				/* save value proc 	*/
	pw_restore_value_cs,				/* restore value proc 	*/
	pw_validate_value_text_length,			/* validate value proc 	*/
    },
    { /* Operator Message */
	K_PW_RESOURCE_TYPE_CS,				/* type 		*/
	K_PW_OPERATOR_MESSAGE,				/* widget index 	*/
	NULL,						/* widget activate proc */
	pw_set_value_cs,				/* set value proc 	*/
	pw_save_value_cs,				/* save value proc 	*/
	pw_restore_value_cs,				/* restore value proc 	*/
	pw_validate_value_text_length,			/* validate value proc 	*/
    },
    { /* Header */
	K_PW_RESOURCE_TYPE_TOGGLE,			/* type 		*/
	K_PW_HEADER,					/* widget index 	*/
	pw_toggle_activate,				/* widget activate proc */
	pw_set_value_boolean,				/* set value proc 	*/
	pw_save_value_boolean,				/* save value proc 	*/
	pw_restore_value_boolean,			/* restore value proc 	*/
	NULL,						/* validate value proc 	*/
    },
    { /* Automatic Pagination */
	K_PW_RESOURCE_TYPE_TOGGLE,			/* type 		*/
	K_PW_AUTOMATIC_PAGINATION,			/* widget index 	*/
	pw_toggle_activate,				/* widget activate proc */
	pw_set_value_boolean,				/* set value proc 	*/
	pw_save_value_boolean,				/* save value proc 	*/
	pw_restore_value_boolean,			/* restore value proc 	*/
	NULL,						/* validate value proc 	*/
    },
    { /* Double Spacing */
	K_PW_RESOURCE_TYPE_TOGGLE,			/* type 		*/
	K_PW_DOUBLE_SPACING,				/* widget index 	*/
	pw_toggle_activate,				/* widget activate proc */
	pw_set_value_boolean,				/* set value proc 	*/
	pw_save_value_boolean,				/* save value proc 	*/
	pw_restore_value_boolean,			/* restore value proc 	*/
	NULL,						/* validate value proc 	*/
    },
    { /* Layup Definition */
	K_PW_RESOURCE_TYPE_CS,				/* type 		*/
	K_PW_LAYUP_DEFINITION,				/* widget index 	*/
	NULL,						/* widget activate proc */
	pw_set_value_cs,				/* set value proc 	*/
	pw_save_value_cs,				/* save value proc 	*/
	pw_restore_value_cs,				/* restore value proc 	*/
	pw_validate_value_text_contents,		/* validate value proc 	*/
    },
    { /* Start Sheet Comment */
	K_PW_RESOURCE_TYPE_CS,				/* type 		*/
	K_PW_START_SHEET_COMMENT,			/* widget index 	*/
	NULL,						/* widget activate proc */
	pw_set_value_cs,				/* set value proc 	*/
	pw_save_value_cs,				/* save value proc 	*/
	pw_restore_value_cs,				/* restore value proc 	*/
	pw_validate_value_text_length,			/* validate value proc 	*/
    },
    { /* Pass All */
	K_PW_RESOURCE_TYPE_TOGGLE,			/* type 		*/
	K_PW_PASS_ALL,					/* widget index 	*/
	pw_toggle_activate,				/* widget activate proc */
	pw_set_value_boolean,				/* set value proc 	*/
	pw_save_value_boolean,				/* save value proc 	*/
	pw_restore_value_boolean,			/* restore value proc 	*/
	NULL,						/* validate value proc 	*/
    },
    { /* Printer Form List and Count */
	K_PW_RESOURCE_TYPE_CS_LIST,			/* type 		*/
	K_PW_PRINTER_FORM,				/* widget index 	*/
	pw_listbox_activate,				/* widget activate proc */
	pw_set_value_readonly_cs_list,			/* set value proc 	*/ /* No-one should be setting this... */
	NULL,						/* save value proc 	*/
	NULL,						/* restore value proc 	*/
	NULL,						/* validate value proc 	*/
    },
    { /* Printer Form Choice */
	K_PW_RESOURCE_TYPE_CS_CHOICE,			/* type 		*/
	K_PW_PRINTER_FORM,				/* widget index 	*/
	NULL,						/* widget activate proc */
	pw_set_value_cs_choice,				/* set value proc 	*/
	pw_save_value_cs_choice,			/* save value proc 	*/
	pw_restore_value_cs_choice,			/* restore value proc 	*/
	pw_validate_value_cs_choice,			/* validate value proc 	*/
    },
    { /* Priority */
	K_PW_RESOURCE_TYPE_CS_INT,			/* type 		*/
	K_PW_PRIORITY,					/* widget index 	*/
	NULL,						/* widget activate proc */
	pw_set_value_int,				/* set value proc 	*/
	pw_save_value_int,				/* save value proc 	*/
	pw_restore_value_int,				/* restore value proc 	*/
	pw_validate_value_int,				/* validate value proc 	*/
    },
    { /* Setup */
	K_PW_RESOURCE_TYPE_CS,				/* type 		*/
	K_PW_SETUP,					/* widget index 	*/
	NULL,						/* widget activate proc */
	pw_set_value_cs,				/* set value proc 	*/
	pw_save_value_cs,				/* save value proc 	*/
	pw_restore_value_cs,				/* restore value proc 	*/
	pw_validate_value_text_contents,		/* validate value proc 	*/
    },
    { /* Debug */
	K_PW_RESOURCE_TYPE_INT,				/* type 		*/
	0,						/* widget index 	*/
	NULL,						/* widget activate proc */
	NULL,						/* set value proc 	*/
	NULL,						/* save value proc 	*/
	NULL,						/* restore value proc 	*/
	NULL,						/* validate value proc 	*/
    },
    { /* OK Selected Callback List */
	K_PW_RESOURCE_TYPE_CALLBACK_LIST,		/* type 		*/
	0,						/* widget index 	*/
	NULL,						/* widget activate proc */
	NULL,						/* set value proc 	*/
	NULL,						/* save value proc 	*/
	NULL,						/* restore value proc 	*/
	NULL,						/* validate value proc 	*/
    },
    { /* Unmanage on OK selected */
	K_PW_RESOURCE_TYPE_INT,				/* type 		*/
	0,						/* widget index 	*/
	NULL,						/* widget activate proc */
	NULL,						/* set value proc 	*/
	NULL,						/* save value proc 	*/
	NULL,						/* restore value proc 	*/
	NULL,						/* validate value proc 	*/
    },
    { /* Cancel Selected Callback List */
	K_PW_RESOURCE_TYPE_CALLBACK_LIST,		/* type 		*/
	0,						/* widget index 	*/
	NULL,						/* widget activate proc */
	NULL,						/* set value proc 	*/
	NULL,						/* save value proc 	*/
	NULL,						/* restore value proc 	*/
	NULL,						/* validate value proc 	*/
    },
    { /* Unmanage on Cancel selected */
	K_PW_RESOURCE_TYPE_INT,				/* type 		*/
	0,						/* widget index 	*/
	NULL,						/* widget activate proc */
	NULL,						/* set value proc 	*/
	NULL,						/* save value proc 	*/
	NULL,						/* restore value proc 	*/
	NULL,						/* validate value proc 	*/
    },
    { /* File Name List and Count */
	K_PW_RESOURCE_TYPE_CS_LIST,			/* type 		*/
	0,						/* widget index 	*/
	NULL,						/* widget activate proc */
	pw_set_value_filename_list,			/* set value proc 	*/
	NULL,						/* save value proc 	*/
	NULL,						/* restore value proc 	*/
	pw_validate_value_filename_list,		/* validate value proc 	*/
    },
    { /* Known Print Formats */
	K_PW_RESOURCE_TYPE_CS_LIST,			/* type 		*/
	0,						/* widget index 	*/
	NULL,						/* widget activate proc */
	NULL,						/* set value proc 	*/
	NULL,						/* save value proc 	*/
	NULL,						/* restore value proc 	*/
	NULL,						/* validate value proc 	*/
    },
    { /* Known Print Format Variables */
	K_PW_RESOURCE_TYPE_CS_LIST,			/* type 		*/
	0,						/* widget index 	*/
	NULL,			       			/* widget activate proc */
	NULL,						/* set value proc 	*/
	NULL,						/* save value proc 	*/
	NULL,						/* restore value proc 	*/
	NULL,						/* validate value proc 	*/
    },
    { /* Known Print Format Op Sys Map */
	K_PW_RESOURCE_TYPE_CS_LIST,			/* type 		*/
	0,						/* widget index 	*/
	NULL,						/* widget activate proc */
	NULL,						/* set value proc 	*/
	NULL,						/* save value proc 	*/
	NULL,						/* restore value proc 	*/
	NULL,						/* validate value proc 	*/
    },
    { /* Known Print Format Guesser Mappings */
	K_PW_RESOURCE_TYPE_INT_LIST,			/* type 		*/
	0,						/* widget index 	*/
	NULL,						/* widget activate proc */
	NULL,						/* set value proc 	*/
	NULL,						/* save value proc 	*/
	NULL,						/* restore value proc 	*/
	NULL,						/* validate value proc 	*/
    },
    { /* Default Printer */
	K_PW_RESOURCE_TYPE_CS,				/* type 		*/
	0,						/* widget index 	*/
	NULL,						/* widget activate proc */
	pw_set_value_cs,				/* set value proc 	*/
	NULL,						/* save value proc 	*/
	NULL,						/* restore value proc 	*/
	NULL,						/* validate value proc 	*/
    },
    { /* Suppress Options Mask */
	K_PW_RESOURCE_TYPE_INT,				/* type 		*/
	NULL,						/* widget index 	*/
	NULL,						/* widget activate proc */
	pw_set_value_int,				/* set value proc 	*/
	NULL,						/* save value proc 	*/
	NULL,						/* restore value proc 	*/
	NULL,						/* validate value proc 	*/
    },
    { /* UID Filename */
	K_PW_RESOURCE_TYPE_CS,				/* type 		*/
	0,						/* widget index 	*/
	NULL,						/* widget activate proc */
	pw_set_value_cs,				/* set value proc 	*/
	NULL,						/* save value proc 	*/
	NULL,						/* restore value proc 	*/
	NULL,						/* validate value proc 	*/
    },
    { /* Help filename (library spec) */
	K_PW_RESOURCE_TYPE_CS,				/* type 		*/
	0,						/* widget index 	*/
	NULL,						/* widget activate proc */
	pw_set_value_cs,				/* set value proc 	*/
	NULL,						/* save value proc 	*/
	NULL,						/* restore value proc 	*/
	NULL,						/* validate value proc 	*/
    },
    { /* 2ndary (options) dialog box title */
	K_PW_RESOURCE_TYPE_CS,				/* type 		*/
	K_PW_2NDARY_BOX,				/* widget index 	*/
	NULL,						/* widget activate proc */
	pw_set_value_2ndary_title,			/* set value proc 	*/
	NULL,						/* save value proc 	*/
	NULL,						/* restore value proc 	*/
	NULL,						/* validate value proc 	*/
    }
};
    

/************************************************************************/
/*									*/
/* Definition of the resources.						*/
/*									*/
/************************************************************************/
static const unsigned long int	l_resource_0 = 0;
static const unsigned long int	l_resource_minus_1 = -1;

static XtResource ar_resources[] =
{
    {
	DXmNnumberCopies, DXmCNumberCopies,
	XtRInt, sizeof(int),
	XtOffset(DXmPrintWgt,printwgt.r_number_copies.r_int_res.l_current_value),
	XtRInt, (XtPointer) &l_resource_0
    },
    {
	DXmNpageRangeFrom, DXmCPageRangeFrom,
	XmRXmString, sizeof(XmString),
	XtOffset(DXmPrintWgt,printwgt.r_page_range_from.r_cs_res.ar_current_cs),
	XmRXmString, NULL
    },
    {
	DXmNpageRangeTo, DXmCPageRangeTo,
	XmRXmString, sizeof(XmString),
	XtOffset(DXmPrintWgt,printwgt.r_page_range_to.r_cs_res.ar_current_cs),
	XmRXmString, NULL
    },
    {
	DXmNprintFormatList, DXmCPrintFormatList,
	XmRXmStringTable, sizeof(XmString *),
	XtOffset(DXmPrintWgt,printwgt.r_print_format_contents.r_cs_list_res.ar_cs_list),
	XmRXmStringTable, NULL
    },
    {
	DXmNprintFormatCount, DXmCPrintFormatCount,
	XtRInt, sizeof(int),
	XtOffset(DXmPrintWgt,printwgt.r_print_format_contents.r_cs_list_res.l_cs_count),
	XtRInt, (XtPointer) &l_resource_minus_1
    },
    {
	DXmNprintFormatChoice, DXmCPrintFormatChoice,
	XmRXmString, sizeof(XmString),
	XtOffset(DXmPrintWgt,printwgt.r_print_format_choice.r_cs_res.ar_current_cs),
	XmRString, NULL
    },
    {
	DXmNorientation, DXmCOrientation,
	XtRInt, sizeof(int),
	XtOffset(DXmPrintWgt,printwgt.r_orientation.r_int_res.l_current_value),
	XtRInt,	(XtPointer) &l_resource_0
    },
    {
	DXmNprinterList, DXmCPrinterList,
	XmRXmStringTable, sizeof(XmString *),
	XtOffset(DXmPrintWgt,printwgt.r_printer_contents.r_cs_list_res.ar_cs_list),
	XmRXmStringTable, NULL
    },
    {
	DXmNprinterCount, DXmCPrinterCount,
	XtRInt, sizeof(int),
	XtOffset(DXmPrintWgt,printwgt.r_printer_contents.r_cs_list_res.l_cs_count),
	XtRInt, (XtPointer) &l_resource_0
    },
    {
	DXmNprinterChoice, DXmCPrinterChoice,
	XmRXmString, sizeof(XmString),
	XtOffset(DXmPrintWgt,printwgt.r_printer_choice.r_cs_res.ar_current_cs),
	XmRString, NULL
    },
    {
	DXmNprintAfter, DXmCPrintAfter,
	XmRXmString, sizeof(XmString),
	XtOffset(DXmPrintWgt,printwgt.r_print_after.r_cs_res.ar_current_cs),
	XmRXmString, NULL
    }, 
    {
	DXmNdeleteFile, DXmCDeleteFile,
	XmRBoolean, sizeof(Boolean),
	XtOffset(DXmPrintWgt,printwgt.r_delete_file.r_boolean_res.b_current_value),
	XmRImmediate, (XtPointer) FALSE
    },

    {
	DXmNpageSize, DXmCPageSize, 
	XtRInt, sizeof(int),
	XtOffset(DXmPrintWgt,printwgt.r_page_size.r_int_res.l_current_value),
	XtRInt,	(XtPointer) &l_resource_0
    },
    {
	DXmNsides, DXmCSides, 
	XtRInt, sizeof(int),
	XtOffset(DXmPrintWgt,printwgt.r_sides.r_int_res.l_current_value),
	XtRInt,	(XtPointer) &l_resource_0
    },
    {
	DXmNnumberUp, DXmCNumberUp,
	XtRInt, sizeof(int),
	XtOffset(DXmPrintWgt,printwgt.r_number_up.r_int_res.l_current_value),
	XtRInt, (XtPointer) &l_resource_0
    },
    {
	DXmNsheetCount, DXmCSheetCount,
	XtRInt, sizeof(int),
	XtOffset(DXmPrintWgt,printwgt.r_sheet_count.r_int_res.l_current_value),
	XtRInt, (XtPointer) &l_resource_0
    },
    {
	DXmNfileStartSheet, DXmCFileStartSheet, 
	XtRInt, sizeof(int),
	XtOffset(DXmPrintWgt,printwgt.r_file_start_sheet.r_int_res.l_current_value),
	XtRInt,	(XtPointer) &l_resource_0
    },
    {
	DXmNfileEndSheet, DXmCFileEndSheet, 
	XtRInt, sizeof(int),
	XtOffset(DXmPrintWgt,printwgt.r_file_end_sheet.r_int_res.l_current_value),
	XtRInt,	(XtPointer) &l_resource_0
    },
    {
	DXmNfileBurstSheet, DXmCFileBurstSheet, 
	XtRInt, sizeof(int),
	XtOffset(DXmPrintWgt,printwgt.r_file_burst_sheet.r_int_res.l_current_value),
	XtRInt,	(XtPointer) &l_resource_0
    },
    {
	DXmNmessageLog, DXmCMessageLog, 
	XtRInt, sizeof(int),
	XtOffset(DXmPrintWgt,printwgt.r_message_log.r_int_res.l_current_value),
	XtRInt,	(XtPointer) &l_resource_0
    },
    {
	DXmNholdJob, DXmCHoldJob,
	XmRBoolean, sizeof(Boolean),
	XtOffset(DXmPrintWgt,printwgt.r_hold_job.r_boolean_res.b_current_value),
	XmRImmediate, (XtPointer) FALSE
    },
    {
	DXmNnotify, DXmCNotify,
	XmRBoolean, sizeof(Boolean),
	XtOffset(DXmPrintWgt,printwgt.r_notify.r_boolean_res.b_current_value),
	XmRImmediate, (XtPointer) TRUE
    },
    {
	DXmNsheetSize, DXmCSheetSize, 
	XtRInt, sizeof(int),
	XtOffset(DXmPrintWgt,printwgt.r_sheet_size.r_int_res.l_current_value),
	XtRInt,	(XtPointer) &l_resource_0
    },
    {
	DXmNinputTray, DXmCInputTray, 
	XtRInt, sizeof(int),
	XtOffset(DXmPrintWgt,printwgt.r_input_tray.r_int_res.l_current_value),
	XtRInt,	(XtPointer) &l_resource_0
    },
    {
	DXmNoutputTray, DXmCOutputTray, 
	XtRInt, sizeof(int),
	XtOffset(DXmPrintWgt,printwgt.r_output_tray.r_int_res.l_current_value),
	XtRInt,	(XtPointer) &l_resource_0
    },
    {
	DXmNjobName, DXmCJobName,
	XmRXmString, sizeof(XmString),
	XtOffset(DXmPrintWgt,printwgt.r_job_name.r_cs_res.ar_current_cs),
	XmRXmString, NULL
    },
    {
	DXmNoperatorMessage, DXmCOperatorMessage,
	XmRXmString, sizeof(XmString),
	XtOffset(DXmPrintWgt,printwgt.r_operator_message.r_cs_res.ar_current_cs),
	XmRXmString, NULL
    },
    {
	DXmNheader, DXmCHeader,
	XmRBoolean, sizeof(Boolean),
	XtOffset(DXmPrintWgt,printwgt.r_header.r_boolean_res.b_current_value),
	XmRImmediate, (XtPointer) FALSE
    },
    {
	DXmNautoPagination, DXmCAutoPagination,
	XmRBoolean, sizeof(Boolean),
	XtOffset(DXmPrintWgt,printwgt.r_automatic_pagination.r_boolean_res.b_current_value),
	XmRImmediate, (XtPointer) FALSE
    },
    {
	DXmNdoubleSpacing, DXmCDoubleSpacing,
	XmRBoolean, sizeof(Boolean),
	XtOffset(DXmPrintWgt,printwgt.r_double_spacing.r_boolean_res.b_current_value),
	XmRImmediate, (XtPointer) FALSE
    },
    {
	DXmNlayupDefinition, DXmCLayupDefinition,
	XmRXmString, sizeof(XmString),
	XtOffset(DXmPrintWgt,printwgt.r_layup_definition.r_cs_res.ar_current_cs),
	XmRXmString, NULL
    },
    {
	DXmNstartSheetComment, DXmCStartSheetComment,
	XmRXmString, sizeof(XmString),
	XtOffset(DXmPrintWgt,printwgt.r_start_sheet_comment.r_cs_res.ar_current_cs),
	XmRXmString, NULL
    },
    {
	DXmNpassAll, DXmCPassAll, 
	XmRBoolean, sizeof(Boolean),
	XtOffset(DXmPrintWgt,printwgt.r_pass_all.r_boolean_res.b_current_value),
	XmRImmediate, (XtPointer) FALSE
    },
    {
	DXmNprinterFormList, DXmCPrinterFormList,
	XmRXmStringTable, sizeof(XmString *),
	XtOffset(DXmPrintWgt,printwgt.r_printer_form_contents.r_cs_list_res.ar_cs_list),
	XmRXmStringTable, NULL
    },
    {
	DXmNprinterFormCount, DXmCPrinterFormCount,
	XtRInt, sizeof(int),
	XtOffset(DXmPrintWgt,printwgt.r_printer_form_contents.r_cs_list_res.l_cs_count),
	XtRInt, (XtPointer) &l_resource_minus_1
    },
    {
	DXmNprinterFormChoice, DXmCPrinterFormChoice,
	XmRXmString, sizeof(XmString),
	XtOffset(DXmPrintWgt,printwgt.r_printer_form_choice.r_cs_res.ar_current_cs),
	XmRString, NULL
    },
    {
	DXmNpriority, DXmCPriority,
	XtRInt, sizeof(int),
	XtOffset(DXmPrintWgt,printwgt.r_priority.r_int_res.l_current_value),
	XtRInt, (XtPointer) &l_resource_0
    },
    {
	DXmNsetup, DXmCSetup,
	XmRXmString, sizeof(XmString),
	XtOffset(DXmPrintWgt,printwgt.r_setup.r_cs_res.ar_current_cs),
	XmRXmString, NULL
    },
    {
	DXmNprintWgtDebug, DXmCPrintWgtDebug, 
	XtRInt, sizeof(int),
	XtOffset(DXmPrintWgt,printwgt.r_debug.r_int_res.l_current_value),
	XtRInt,	(XtPointer) &l_resource_0
    },
    {
	XmNokCallback, XtCCallback, 
	XtRCallback,sizeof(XtCallbackList),
	XtOffset(DXmPrintWgt,printwgt.r_ok_callback_list.r_callback_list_res.ar_callback_list),
	XtRCallback, (XtPointer) NULL
    },
    {
	DXmNunmanageOnOk, DXmCUnmanageOnOk, 
        XmRBoolean, sizeof (Boolean),
	XtOffset(DXmPrintWgt,printwgt.r_unmanage_on_ok.r_boolean_res.b_current_value),
	XmRImmediate, (XtPointer) FALSE
    },
    {
	XmNcancelCallback, XtCCallback, 
	XtRCallback,sizeof(XtCallbackList),
	XtOffset(DXmPrintWgt,printwgt.r_cancel_callback_list.r_callback_list_res.ar_callback_list),
	XtRCallback, (XtPointer) NULL
    },
    {
	DXmNunmanageOnCancel, DXmCUnmanageOnCancel, 
        XmRBoolean, sizeof (Boolean),
	XtOffset(DXmPrintWgt,printwgt.r_unmanage_on_cancel.r_boolean_res.b_current_value),
	XmRImmediate, (XtPointer) FALSE
    },
    {
	DXmNfileNameList, DXmCFileNameList,
	XmRXmStringTable, sizeof(XmString *),
	XtOffset(DXmPrintWgt,printwgt.r_filenames.r_cs_list_res.ar_cs_list),
	XmRXmStringTable, NULL
    },
    {
	DXmNfileNameCount, DXmCFileNameCount,
	XtRInt, sizeof(int),
	XtOffset(DXmPrintWgt,printwgt.r_filenames.r_cs_list_res.l_cs_count),
	XtRInt, (XtPointer) &l_resource_0
    },
    {
	DXmNknownPrintFormatUIList, DXmCKnownPrintFormatUIList,
	XmRXmStringTable, sizeof(XmString *),
	XtOffset(DXmPrintWgt,printwgt.r_known_format_UI_list.r_cs_list_res.ar_cs_list),
	XmRXmStringTable, NULL
    },
    {
	DXmNknownPrintFormatCount, DXmCKnownPrintFormatCount,
	XtRInt, sizeof(int),
	XtOffset(DXmPrintWgt,printwgt.r_known_format_UI_list.r_cs_list_res.l_cs_count),
	XtRInt, (XtPointer) &l_resource_minus_1
    },
    {
	DXmNknownPrintFormatVariableList, DXmCKnownPrintFormatVariableList,
	XmRXmStringTable, sizeof(XmString *),
	XtOffset(DXmPrintWgt,printwgt.r_known_format_variable_list.r_cs_list_res.ar_cs_list),
	XmRXmStringTable, NULL
    },
    {
	DXmNknownPrintFormatOSList, DXmCKnownPrintFormatOSList,
	XmRXmStringTable, sizeof(XmString *),
	XtOffset(DXmPrintWgt,printwgt.r_known_format_OS_list.r_cs_list_res.ar_cs_list),
	XmRXmStringTable, NULL
    },
    {
	DXmNknownPrintFormatGuesserList, DXmCKnownPrintFormatGuesserList,
	XtRInt, sizeof(int *),
	XtOffset(DXmPrintWgt,printwgt.r_known_format_guesser_list.r_int_list_res.al_int_list),
	XtRInt, (XtPointer) &l_resource_0
    },
    {
	DXmNdefaultPrinter, DXmCDefaultPrinter,
	XmRXmString, sizeof(XmString),
	XtOffset(DXmPrintWgt,printwgt.r_default_printer.r_cs_res.ar_current_cs),
	XmRXmString, (XtPointer) &l_resource_minus_1
    },
    {
	DXmNsuppressOptionsMask,DXmCSuppressOptionsMask,
	XtRInt, sizeof(int),
	XtOffset(DXmPrintWgt,printwgt.r_suppress_options_mask.r_int_res.l_current_value),
	XtRInt, (XtPointer) &l_resource_0
    },
    {
	DXmNuidSpec, DXmCUidSpec,
	XmRXmString, sizeof(XmString),
	XtOffset(DXmPrintWgt,printwgt.r_uid_spec.r_cs_res.ar_current_cs),
	XmRString, PW_UID_FILE_LOCATION
    },
    {
	DXmNlibrarySpec, XmCXmString,
	XmRXmString, sizeof(XmString),
	XtOffset(DXmPrintWgt,printwgt.r_library_spec.r_cs_res.ar_current_cs),
	XmRString, PW_HELP_FILE_LOCATION
    },
    {
	DXmNoptionsDialogTitle, DXmCOptionsDialogTitle,
	XmRXmString, sizeof(XmString),
	XtOffset(DXmPrintWgt,printwgt.r_2ndary_title.r_cs_res.ar_current_cs),
	XmRXmString, NULL
    },

    {
	DXmNprintWgtWidgetIds, DXmCPrintWgtWidgetIds,
	XtRInt, sizeof(int *),
	XtOffset(DXmPrintWgt,printwgt.ar_widget_ids),
	XtRInt, (XtPointer) &l_resource_0
    },
    {	
        XmNnoResize, XmCNoResize, 
        XmRBoolean, sizeof (Boolean),
	XtOffset (XmBulletinBoardWidget, bulletin_board.no_resize),
	XmRImmediate, (XtPointer) TRUE
    },
};


/************************************************************************/
/*									*/
/* The initialization of the widget class record.			*/
/*									*/
/************************************************************************/
externaldef(dxmprintwgtclassrec) DXmPrintWgtClassRec dxmPrintWgtClassRec =
{
    { /* core class record						*/
	/* superclass		*/	(WidgetClass) &xmBulletinBoardClassRec,
	/* class_name		*/	"DXmPrintWgt",
	/* widget_size		*/	sizeof(DXmPrintWgtRec),
	/* class_initialize	*/	pw_class_initialize,
	/* class init part proc */	NULL,
	/* class_inited		*/	FALSE,
	/* initialize		*/	pw_initialize,
	/* initialize hook	*/	NULL,
	/* realize		*/	(XtRealizeProc)_XtInherit,
	/* actions		*/	NULL,
	/* num_actions		*/	0,
	/* resources		*/	ar_resources,
	/* num_resources	*/	XtNumber(ar_resources),
	/* xrm_class		*/	NULLQUARK,
	/* compress_motion	*/	TRUE,
	/* compress_exposure	*/	TRUE,
	/* compress-enter/exit	*/	TRUE,
	/* visible_interest	*/	FALSE,
	/* destroy		*/	pw_destroy,
	/* resize		*/	XtInheritResize, /* should these be */
	/* expose		*/	pw_expose,
	/* set_values		*/	pw_set_values,
	/* set_values hook	*/	NULL,
	/* set_values almost	*/	XtInheritSetValuesAlmost,
	/* widget get val hook	*/	NULL,
	/* accept_focus		*/	XtInheritAcceptFocus,
	/* version		*/	XtVersion,
	/* callback offset	*/	NULL,
	/* default translations */	XtInheritTranslations,
	/* Query Geometry proc	*/	XtInheritQueryGeometry,
	/* disp accelerator	*/	NULL,
	/* extension		*/	NULL,
    },

    { /* composite class record						*/

	/* child geo mgr proc	*/	NULL,
	/* set changed proc	*/	XtInheritChangeManaged,
	/* add a child		*/	XtInheritInsertChild,
	/* remove a child	*/	XtInheritDeleteChild,
	/* extension		*/	NULL,
    },

    { /* constraint class record					*/
	/* constraint resources */	NULL,
	/* num addtnl resources */	0,
	/* size of constr rec	*/	0,
	/* constraint_initialize*/	NULL,				    
	/* constraint_destroy	*/	NULL,				    
	/* constraint_setvalue	*/	NULL,				    
	/* extension		*/	NULL,
    },

    { /* manager class record						*/
	/* default translations */	XtInheritTranslations,
	/* get resources	*/	NULL,
	/* num get resources	*/	0,
	/* get cont resources	*/	NULL,
	/* num get cont resource*/	0,
        /* parent process	*/	XmInheritParentProcess,
	/* extension		*/	NULL,
    }, 

    { /* bulletin board class record					*/     
	/* always inst accel	*/	FALSE,
        /* geo_matrix_create	*/	XmInheritGeoMatrixCreate,
        /* focus moved proc     */	XtInheritFocusMovedProc,
	/* extension		*/	NULL,
    },	

    { /* print widget class record					*/
	/* extension		*/	NULL,
    }
};

externaldef(dxmprintwgtwidgetclass) WidgetClass dxmPrintWgtWidgetClass
		= (WidgetClass) &dxmPrintWgtClassRec;


/************************************************************************/
/*									*/
/* DXmCreatePrintBox							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine is the low level create routine for the print	*/
/*	widget. (Non-popup version.)					*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	parent		Widget (to be the parent)			*/
/*									*/
/*      name		Name to associate with this widget.		*/
/*									*/
/*	args		Argument list.					*/
/*									*/
/*	arg_count	Number of entries in args.			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	An ID to a new print widget instance.				*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
#ifdef _NO_PROTO
extern Widget DXmCreatePrintBox (ar_parent, at_name, ar_args, l_arg_count)
    Widget	ar_parent;
    char	*at_name;
    ArgList	ar_args;
    int		l_arg_count;
#else
extern Widget DXmCreatePrintBox (Widget 	ar_parent, 
				 char 		*at_name, 
				 ArgList 	ar_args, 
				 int 		l_arg_count)
#endif /* _NO_PROTO */
{
    DXmPrintWgt		ar_pw;

    /********************************************************************/
    /*									*/
    /* Make sure that the required arguments have been passed in.	*/
    /*									*/
    /********************************************************************/
    if (pw_required_arguments(ar_args,l_arg_count)
	!= TRUE)
    {
	DXMAPPWARNING(XtWidgetToApplicationContext(ar_parent),
		      PWMSGNAME0,
		      PWGTNOPARAM,
		      NULL,NULL);
	return ((Widget) NULL);
    }

    /********************************************************************/
    /*									*/
    /* Create the printwgt as a child of the parent                     */
    /*									*/
    /********************************************************************/
    ar_pw = (DXmPrintWgt) XtCreateWidget (at_name,
					  dxmPrintWgtWidgetClass,
					  ar_parent,
					  ar_args,
					  l_arg_count);
    
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("Inside DXmCreatePrintBox.  The print widget has been created.\n");
#endif
    
    return ((Widget)ar_pw);
}


/************************************************************************/
/*									*/
/* DXmCreatePrintDialog							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine is the low level create routine for the print	*/
/*	widget.  (Popup version.)					*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	parent		Widget (to be the parent)			*/
/*									*/
/*      name		Name to associate with this widget.		*/
/*									*/
/*	args		Argument list.					*/
/*									*/
/*	arg_count	Number of entries in args.			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	An ID to a new print widget instance.				*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
#ifdef _NO_PROTO
extern Widget DXmCreatePrintDialog (ar_parent, at_name, ar_args, l_arg_count)
    Widget	ar_parent;
    char	*at_name;
    ArgList	ar_args;
    int		l_arg_count;
#else
extern Widget DXmCreatePrintDialog (Widget 	ar_parent, 
				    char 	*at_name, 
				    ArgList 	ar_args, 
				    int 	l_arg_count)
#endif /* _NO_PROTO */
{
    Widget		ar_dialog_shell;
    Arg			ar_al[2];
    int			l_ac = 0;
    DXmPrintWgt		ar_pw;
    char           	*at_ds_name;

    /********************************************************************/
    /*									*/
    /* Make sure that the required arguments have been passed in.	*/
    /*									*/
    /********************************************************************/
    if (pw_required_arguments(ar_args,l_arg_count)
	!= TRUE)
    {
	DXMAPPWARNING(XtWidgetToApplicationContext(ar_parent),
		      PWMSGNAME0,
		      PWGTNOPARAMD,
		      NULL,NULL);
	return ((Widget) NULL);
    }

    /********************************************************************/
    /*									*/
    /* Create a popup shell for the printwgt                            */
    /*									*/
    /********************************************************************/
    at_ds_name = XtCalloc(strlen(at_name)+XmDIALOG_SUFFIX_SIZE+1,sizeof(char));
    strcpy(at_ds_name,at_name);
    strcat(at_ds_name,XmDIALOG_SUFFIX);

    XtSetArg (ar_al[l_ac], XtNallowShellResize, TRUE); 	l_ac++;

    ar_dialog_shell = (Widget) XmCreateDialogShell (ar_parent,
						    at_ds_name,
						    ar_al,
						    l_ac);

    XtFree(at_ds_name);
    
    /********************************************************************/
    /*									*/
    /* Create the printwgt as a child of the shell                      */
    /*									*/
    /********************************************************************/
    ar_pw = (DXmPrintWgt) XtCreateWidget (at_name,
					  dxmPrintWgtWidgetClass,
					  ar_dialog_shell,
					  ar_args,
					  l_arg_count);
    
    XtAddCallback ((Widget)ar_pw, XmNdestroyCallback, _XmDestroyParentCallback, NULL);

#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("Inside DXmCreatePrintDialog.  The print widget has been created.\n");
#endif
    
    return ((Widget)ar_pw);
}


/************************************************************************/
/*									*/
/* DXmCreatePrintWgt							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine is the low level create routine for the print	*/
/*	widget.	(Obsolete - Use DXmCreatePrintDialog instead.)		*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	parent		Widget (to be the parent)			*/
/*									*/
/*      name		Name to associate with this widget.		*/
/*									*/
/*	args		Argument list.					*/
/*									*/
/*	arg_count	Number of entries in args.			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	An ID to a new print widget instance.				*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
#ifdef _NO_PROTO
extern Widget DXmCreatePrintWgt (ar_parent, at_name, ar_args, l_arg_count)
    Widget	ar_parent;
    char	*at_name;
    ArgList	ar_args;
    int		l_arg_count;
#else
extern Widget DXmCreatePrintWgt (Widget 	ar_parent, 
				 char 		*at_name, 
				 ArgList 	ar_args, 
				 int 		l_arg_count)
#endif /* _NO_PROTO */
{
    return (DXmCreatePrintDialog(ar_parent,at_name,ar_args,l_arg_count));

} /* DXmCreatePrintWgt */


/************************************************************************/
/*									*/
/* DXmPrintWgtPrintJob							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine accepts a print widget and a filename list.	*/
/*	It then submits the print job by a call to 			*/
/*	pw_send_files_to_print_queue.					*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/*	cs_filenames	Compound string list of filenames		*/
/*									*/
/*	filename_count	Number of filenames				*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
#ifdef _NO_PROTO
extern unsigned long int DXmPrintWgtPrintJob (ar_w,ar_cs_filenames,l_filename_count)
    Widget		ar_w;
    XmString		ar_cs_filenames[];
    int			l_filename_count;
#else
extern unsigned long int DXmPrintWgtPrintJob (Widget 	ar_w, 
					      XmString 	ar_cs_filenames [], 
					      int 	l_filename_count)
#endif /* _NO_PROTO */
{
    DXmPrintWgt		ar_pw = (DXmPrintWgt) ar_w;
    Opaque      	*ar_os_filenames;
    int			l_i;
    int			l_pulldown;
    char		**aat_objects_list;
    MrmCode	        r_literal_type;
    long		l_size,l_status;

    /********************************************************************/
    /*									*/
    /* Check for no printers or no printer selected.			*/
    /*									*/
    /********************************************************************/
    if (!PWr_cs_list_res(ar_pw,K_PW_PRINTER_CONTENTS_MAP).l_cs_count)
	return (0);
    
    if (!PWr_cs_res(ar_pw,K_PW_PRINTER_CHOICE_MAP).ar_current_cs)
	return (0);
    
    /********************************************************************/
    /*									*/
    /* Check for no filenames given.					*/
    /*									*/
    /********************************************************************/
    if (!ar_cs_filenames || !l_filename_count)
	return (0);
    
    /********************************************************************/
    /*									*/
    /* Convert the filenames to OS strings.				*/
    /*									*/
    /********************************************************************/
    ar_os_filenames = (Opaque *) XtMalloc(sizeof(Opaque) * l_filename_count);
    for (l_i = 0; l_i < l_filename_count; l_i++)
    {
	if (!(ar_os_filenames[l_i] = (Opaque) DXmCvtCStoOS(ar_cs_filenames[l_i],
                                                           &l_size,
                                                           &l_status)))
	{
	    DXMAPPWARNING(XtWidgetToApplicationContext((Widget)ar_pw),
			  PWMSGNAME1,
			  PWGTNOCONVCSFL,
			  NULL,NULL);
	    return (0);
	}
    }
    
    /********************************************************************/
    /*									*/
    /* If the OS versions of the stuff in the option menus has not been	*/
    /* found yet, find it now.						*/
    /*									*/
    /********************************************************************/
    for (l_pulldown = 0; l_pulldown < K_PW_NUM_PULLDOWN_MENUS; l_pulldown++)
    {
	if (!PWar_pulldown_menu_os_table(ar_pw,l_pulldown).ar_os_list)
	{    
	    /************************************************************/
	    /*								*/
	    /* Fetch the object list definition.			*/
	    /*								*/
	    /************************************************************/
	    if (MrmFetchLiteral(PWar_mrm_hierarchy(ar_pw),
				pw_ar_pulldown_menu_data_table[l_pulldown].at_objects_list_name, /* index (name) of literal */
				NULL,				/* display */
				(XtPointer *) &aat_objects_list,	/* return value */
				&r_literal_type)		/* return type */
		!= MrmSUCCESS)
	    {
		String params = pw_ar_pulldown_menu_data_table[l_pulldown].at_objects_list_name;
		Cardinal num_params = 1;
	
		DXMAPPWARNING(XtWidgetToApplicationContext((Widget)ar_pw),
			      PWMSGNAME2,
			      PWGTNOMRMFETCH,
			      &params,&num_params);

		return MrmFAILURE;
	    }

#ifdef PRINTWGT_DEBUG	    
	    if (PWr_debug(ar_pw).l_current_value)
		for (l_i = 0; l_i < 3; l_i++)
		    printf("object list element: %s\n",aat_objects_list[l_i]);
#endif

	    /************************************************************/
	    /*								*/
	    /* Fetch the null-terminated OS list.			*/
	    /*								*/
	    /************************************************************/
	    pw_fetch_pulldown_menu_os_list(ar_pw,
					   aat_objects_list[1], /* 1 is the index of OS table name */
					   l_pulldown);
#ifndef NO_FREE_MRM
	    XtFree((char *)aat_objects_list);
#endif

	} /* if pulldown os list does not exist */
    } /* for which loops through pulldown stuff */

    /********************************************************************/
    /*									*/
    /* Find the OS str versions of the printer and printer form, and th */
    /* binary form of the time.						*/
    /*									*/
    /********************************************************************/

    /********************************************************************/
    /*									*/
    /* Printer Choice							*/
    /*									*/
    /********************************************************************/
    if (PWr_cs_res(ar_pw,K_PW_PRINTER_CHOICE_MAP).ar_current_cs)
    {
	if (!(PWr_os_printer_choice(ar_pw) = 
	      (Opaque) DXmCvtCStoOS(PWr_cs_res(ar_pw,
					       K_PW_PRINTER_CHOICE_MAP).ar_current_cs,
                                    &l_size,
                                    &l_status)))
	{
	    DXMAPPWARNING(XtWidgetToApplicationContext((Widget)ar_pw),
			  PWMSGNAME3,
			  PWGTNOCONVCSPR,
			  NULL,NULL);
	    return (0);
	}
    }
    
    /********************************************************************/
    /*									*/
    /* Printer Form Choice						*/
    /*									*/
    /********************************************************************/
    if (PWr_cs_res(ar_pw,K_PW_PRINTER_FORM_CHOICE_MAP).ar_current_cs)
    {
	if (!(PWr_os_printer_form_choice(ar_pw) = 
	      (Opaque) DXmCvtCStoOS(PWr_cs_res(ar_pw,
					       K_PW_PRINTER_FORM_CHOICE_MAP).ar_current_cs,
                                    &l_size,
                                    &l_status)))
	{
	    DXMAPPWARNING(XtWidgetToApplicationContext((Widget)ar_pw),
			  PWMSGNAME4,
			  PWGTNOCONVCSPRF,
			  NULL,NULL);
	    return (0);
	}
    }
    
    /********************************************************************/
    /*									*/
    /* Print After - If there isn't any time specified, set the first	*/
    /* 		     and second elements to -1.				*/
    /*									*/
    /********************************************************************/
    if (pw_xm_string_compare(PWr_cs_res(ar_pw,K_PW_PRINT_AFTER_MAP).ar_current_cs,
			     PWar_now_string(ar_pw)))
    {
	PWal_binary_start_time(ar_pw)[0] = -1;
	PWal_binary_start_time(ar_pw)[1] = -1;
    }
    else
    {
	if (!pw_convert_cs_to_time(PWr_cs_res(ar_pw,K_PW_PRINT_AFTER_MAP).ar_current_cs,
				     PWal_binary_start_time(ar_pw)))
	{
	    DXMAPPWARNING(XtWidgetToApplicationContext((Widget)ar_pw),
			  PWMSGNAME5,
			  PWGTNOCONVCSTM,
			  NULL,NULL);
	    return (0);
	}
    }
    
    /********************************************************************/
    /*									*/
    /* Print the Job							*/
    /*									*/
    /********************************************************************/
/* Kanji printer */
#ifdef VMS
  {
    char *print_format;
    long len, status;

    print_format = (char *)DXmCvtCStoFC(
                      PWr_cs_res(ar_pw,
			         K_PW_PRINT_FORMAT_CHOICE_MAP).ar_current_cs,
                      &len, &status);
    /* BOGUS
     * These strings shouldn't be hardcoded.
     */
    if (strcmp(print_format, "LN80 (Text)") == 0 ||
        strcmp(print_format, "LN80 (Sixel)") == 0)
	l_status = pw_send_files_to_kprint (ar_pw,
					    ar_os_filenames,
					    l_filename_count);
    else
	l_status = pw_send_files_to_print_queue(ar_pw,
						ar_os_filenames,
						l_filename_count);
    XtFree(print_format);
  }
#else /* VMS */
    l_status = pw_send_files_to_print_queue(ar_pw,
					    ar_os_filenames,
					    l_filename_count);
#endif /* VMS */
    
    /********************************************************************/
    /*									*/
    /* Free up the memory used.						*/
    /*									*/
    /********************************************************************/
    for (l_i = 0; l_i < l_filename_count; l_i++)
	XtFree(ar_os_filenames[l_i]);
    
    XtFree((char *)ar_os_filenames);
    
    if (PWr_os_printer_choice(ar_pw))
    {
	XtFree(PWr_os_printer_choice(ar_pw));
	PWr_os_printer_choice(ar_pw) = NULL;
    }
    
    if (PWr_os_printer_form_choice(ar_pw))
    {
	XtFree(PWr_os_printer_form_choice(ar_pw));
	PWr_os_printer_form_choice(ar_pw) = NULL;
    }
    
    return(l_status);
    
} /* DXmPrintWgtPrintJob */


/************************************************************************/
/*									*/
/* DXmPrintWgtAugmentList						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine will augment a list of the print widget.  An	*/
/*	application would use this to augment the option menus or	*/
/*	print formats.							*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/*	list		Constant which specifies what list to augment.	*/
/*									*/
/*	data		Data structure pointing to list information.	*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	The position where the element was added or 0 if it fails.	*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
#ifdef _NO_PROTO
extern unsigned long int DXmPrintWgtAugmentList(ar_w,l_list,ar_data)
    Widget			ar_w;
    int				l_list;
    XtPointer			ar_data;
#else
extern unsigned long int DXmPrintWgtAugmentList (Widget 	ar_w, 
						 int 		l_list, 
						 XtPointer 	ar_data)
#endif /* _NO_PROTO */
{
    DXmPrintWgt			ar_pw = (DXmPrintWgt) ar_w;
    DXmPrintOptionMenuStruct	*ar_added_option = (DXmPrintOptionMenuStruct *) ar_data;
    DXmPrintFormatStruct	*ar_added_format = (DXmPrintFormatStruct *) ar_data;

    unsigned long int		l_return_status = 0;

    MrmType	        	ar_dummy_class;
    Arg				ar_al[3];
    int				l_resource_index;
    int				l_widget_index;
    int				l_add_pos;
    long 			l_size,l_status;
    
    switch (l_list)
    {
	case DXmPRINT_FORMAT:
	{
	    /************************************************************/
	    /*							    	*/
	    /* If they have given us valid strings, augment the		*/
	    /* contents and count of the following lists:		*/
	    /*								*/
	    /*	      - Known format UI list.				*/
	    /*	      - Known format OS list.				*/
	    /*	      - Known format VAR list.				*/
	    /*	      - Known format GUESSER MAPPING list.		*/
	    /*	      - Current known format queue choice list.	        */
	    /*	      - Print format list.				*/
	    /*								*/
	    /************************************************************/
	    if (ar_added_format->ui_string && 
		ar_added_format->os_string && 
		ar_added_format->var_string)
	    {
		pw_add_cs_list_element(&PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).ar_cs_list,
				       PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).l_cs_count,
				       ar_added_format->ui_string);
		PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).l_cs_count++;
		
		pw_add_cs_list_element(&PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_OS_LIST_MAP).ar_cs_list,
				       PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_OS_LIST_MAP).l_cs_count,
				       ar_added_format->os_string);
		PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_OS_LIST_MAP).l_cs_count++;
		
		pw_add_cs_list_element(&PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_VARIABLE_LIST_MAP).ar_cs_list,
				       PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_VARIABLE_LIST_MAP).l_cs_count,
				       ar_added_format->var_string);
		PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_VARIABLE_LIST_MAP).l_cs_count++;
		
		pw_add_int_list_element(&PWr_int_list_res(ar_pw,K_PW_KNOWN_FORMAT_GUESSER_LIST_MAP).al_int_list,
					PWr_int_list_res(ar_pw,K_PW_KNOWN_FORMAT_GUESSER_LIST_MAP).l_int_count,
					DXmPRINT_FORMAT_DEFAULT);
		PWr_int_list_res(ar_pw,K_PW_KNOWN_FORMAT_GUESSER_LIST_MAP).l_int_count++;
		
		pw_add_int_list_element(&PWal_current_format_queue_choices(ar_pw),
					PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).l_cs_count - 1,
					-1);  /* means format hasn't been selected yet */
		
		pw_add_cs_list_element(&PWr_cs_list_res(ar_pw,K_PW_PRINT_FORMAT_CONTENTS_MAP).ar_cs_list,
				       PWr_cs_list_res(ar_pw,K_PW_PRINT_FORMAT_CONTENTS_MAP).l_cs_count,
				       ar_added_format->ui_string);
		PWr_cs_list_res(ar_pw,K_PW_PRINT_FORMAT_CONTENTS_MAP).l_cs_count++;
		
		/********************************************************/
		/* 							*/
		/* Populate the print format box.			*/
		/* 							*/
		/********************************************************/	
		pw_populate_print_format_listbox(ar_pw);

		l_return_status = PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).l_cs_count;
	    }
	    
	    break;
	}

	case DXmPAGE_SIZE	:
	case DXmSIDES		:
	case DXmFILE_START_SHEET:
	case DXmFILE_END_SHEET	:
	case DXmFILE_BURST_SHEET:
	case DXmMESSAGE_LOG	:
        case DXmSHEET_SIZE	:
	case DXmINPUT_TRAY	:
	case DXmOUTPUT_TRAY	:
	{
	    /************************************************************/
	    /*								*/
	    /* If the 2ndary box has not been created, create it.	*/
	    /* We need to create it so we have the option menus and	*/
	    /* stuff to add the push button to.				*/
	    /*								*/
	    /************************************************************/
	    if (!PWar_widget_id(ar_pw,K_PW_2NDARY_BOX))
		pw_fetch_2ndary_box(ar_pw);

	    /************************************************************/
	    /*								*/
	    /* Now create an extra button and put it in the option	*/
	    /* menu.  Note that the application can only add a finite 	*/
	    /* number of buttons since the widget ID array is static.	*/
	    /* If the position to add to is not empty, return a 0.	*/
	    /*								*/
	    /************************************************************/
	    l_resource_index = pw_ar_pulldown_menu_data_table[l_list].l_resource_index;
	    l_widget_index = pw_ar_resource_control[l_resource_index].l_widget_index;

	    l_add_pos = l_widget_index + PWar_pulldown_menu_os_table(ar_pw,l_list).l_os_count + 1;

	    if (!PWar_widget_id(ar_pw,l_add_pos))
	    {
		/********************************************************/
		/*							*/
		/* Create the push button.				*/
		/*							*/
		/********************************************************/
		XtSetArg(ar_al[0],
			 K_PW_PROTO_OPTIONMENU_ACTIVATE_TAG_NAME,
			 l_add_pos +
			 (l_resource_index << 16));
		XtSetArg(ar_al[1],
			 K_PW_PROTO_OPTIONMENU_LABEL_NAME,
			 ar_added_option->ui_string);
		XtSetArg(ar_al[2],
			 K_PW_PROTO_OPTIONMENU_HELP_TAG_NAME,
			 NULL);

		MrmRegisterNames((MrmRegisterArglist)ar_al,(MrmCount)3);

		if (MrmFetchWidget (PWar_mrm_hierarchy(ar_pw),			/* MRM heirarchy	*/
				    K_PW_PROTO_OPTIONMENU_PB_NAME,		/* Name of widget	*/
			            XtParent(PWar_widget_id(ar_pw,l_add_pos-1)),/* Parent		*/
				    &PWar_widget_id(ar_pw,l_add_pos), 		/* Push Button		*/
				    &ar_dummy_class) 				/* Class		*/
		    != MrmSUCCESS)
		{
		    String params = K_PW_PROTO_OPTIONMENU_PB_NAME;
		    Cardinal num_params = 1;
	
		    DXMAPPWARNING(XtWidgetToApplicationContext((Widget)ar_pw),
				  PWMSGNAME2,
				  PWGTNOMRMFETCH,
				  &params,&num_params);
		    return (0);
		}

		XtManageChild(PWar_widget_id(ar_pw,l_add_pos));

		/********************************************************/
		/*							*/
		/* Augment the OS list.					*/
		/*							*/
		/********************************************************/
		pw_add_opaque_list_element(&PWar_pulldown_menu_os_table(ar_pw,l_list).ar_os_list,
					   PWar_pulldown_menu_os_table(ar_pw,l_list).l_os_count,
					   DXmCvtCStoOS(ar_added_option->os_string,
							&l_size,
							&l_status));
		PWar_pulldown_menu_os_table(ar_pw,l_list).l_os_count++;
		
		l_return_status = PWar_pulldown_menu_os_table(ar_pw,l_list).l_os_count - 1;
	    }
	    
	    break;
	}
	
	default:	break;
	
    } /* end switch */

    return l_return_status;
    
} /* DXmPrintWgtAugmentList */




/************************************************************************
 *
 * drop_failure
 *
 ***********************************************************************/

static void
#ifdef _NO_PROTO
drop_failure(w)
Widget w;
#else
drop_failure(Widget w)
#endif /* _NO_PROTO */
{
    Arg                         args[2];

    XtSetArg(args[0], XmNtransferStatus, XmTRANSFER_FAILURE);
    XtSetArg(args[1], XmNnumDropTransfers, 0);

    /* we need to start the drop transfer to cancel the transfer */
    XmDropTransferStart(w, args, 2);

}

/************************************************************************
 *
 * pw_receive_transfer
 *
 *   receives transfered data from drag
 *
 ************************************************************************/

static void
#ifdef _NO_PROTO
pw_receive_transfer(w, closure, seltype, type, value, length, format)
    Widget w;
    XtPointer closure;
    Atom *seltype;
    Atom *type;
    XtPointer value;
    unsigned long *length;
    int format;
#else
pw_receive_transfer(
    Widget w,
    XtPointer closure,
    Atom *seltype,
    Atom *type,
    XtPointer value,
    unsigned long *length,
    int format)
#endif
{

    Atom     ct;
    XmString str;
    XmString *listptr;
    XmString *newlist;
    Arg      al[10];
    int      count;
    int      n, i;
    XmListWidget lw;
    char     *text;
    XmString newstr, tmpstr;
    XmString str1, str2, str3;
    XmStringContext context;
    XmStringCharSet charset;
    XmStringDirection direction;
    Boolean  separator;
    Boolean  done;
    Boolean  cont;

    DropTransfer    dropdata = (DropTransfer) closure;
    Widget widget = dropdata->widget;

    lw = (XmListWidget)widget;
    ct = XmInternAtom (XtDisplay(widget), "COMPOUND_TEXT", False);

    if (*type == ct)
        str = XmCvtCTToXmString(value);
    XtFree(value);

    /* Check out what we got.  Since we have a single selection, single
       line item list box, make sure the data fits that. */

    if (XmStringLineCount(str) > 1)
    {

        cont = XmStringInitContext(&context, str);
        if (!cont)
        {
            return;
        }


        newstr = NULL;
        done = FALSE;
        while (!done)
        {
           if (XmStringGetNextSegment(context, &text, &charset, &direction,
                                  &separator))
           {
                str1 = XmStringDirectionCreate(direction);
                str2 = XmStringCreate(text, charset);
                str3 = XmStringConcat(str1, str2);
                XmStringFree(str1);
                XmStringFree(str2);
           }
           if (newstr)
           {
                tmpstr = XmStringConcat(newstr, str3);
                newstr = tmpstr;
                XmStringFree(str3);
           }
           else newstr = str3;
           if (separator)
                done = TRUE;
        }

        XmStringFreeContext(context);
        XmStringFree(str);
        str = newstr;
    }
    XmListAddItemsUnselected(widget, &str, 1, 0); 
    return;

}



/************************************************************************
 *
 *  GetTargets
 *
 *  Get the targets that the initiator supports to see if we can handle
 *  what it's sending.
 *
 ************************************************************************/

static Boolean
#ifdef _NO_PROTO
GetTargets(w, t)
Widget   w;
Atom     t; 
#else
GetTargets(Widget w, Atom t)
#endif
{

    Arg 			al[3];
    Atom        		*exportTargets;
    Cardinal    		numExportTargets;
    int                         n;

    XtSetArg(al[0], XmNexportTargets, &exportTargets);
    XtSetArg(al[1], XmNnumExportTargets, &numExportTargets);
    XtGetValues(w, al, 2);

    /* search through the export targets */
    for (n = 0; n < numExportTargets; n++)
    {
        if (exportTargets[n] == t)
            return(TRUE);
    }
    return (FALSE);

}

/************************************************************************
 *
 *  DropDestroyCB
 *
 *  Free the transfer structure when the transfer has completed
 *
 ************************************************************************/

static void
#ifdef _NO_PROTO
DropDestroyCB(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
#else
DropDestroyCB(Widget w, XtPointer client, XtPointer call)
#endif
{
      XtFree((char *) client);
}



/************************************************************************
 *
 * pw_do_drop_transfer
 *
 ***********************************************************************/

static void
#ifdef _NO_PROTO
pw_do_drop_transfer(w, client, call)
   Widget w;
   XtPointer client;
   XtPointer call;
#else
pw_do_drop_transfer(Widget w, XtPointer client, XtPointer call)
#endif /* _NO_PROTO */
{
    XmDropTransferEntryRec 	transferEntries[4];
    XmDropTransferEntry 	transferList;
    DropTransfer 		transferData;
    Arg 			al[10];
    Atom 			ct;
    int 			n = 0;

    static XtCallbackRec dropDestroyCB[] = {
        {DropDestroyCB, NULL},
        {NULL, NULL}
    };

    ct = XmInternAtom(XtDisplay(w),"COMPOUND_TEXT", False);

    /* create transfer data structure, and arrange to have memory
       freed after the transfer */

    transferData = (DropTransfer) XtMalloc(sizeof(DropTransferRec));
    transferData->widget = w;
    transferEntries[0].target = ct;
    transferEntries[0].client_data = (XtPointer) transferData;
    transferList = transferEntries;
    dropDestroyCB[0].closure = (XtPointer) transferData;

    XtSetArg (al[n], XmNdropTransfers, transferList); n++;
    XtSetArg (al[n], XmNnumDropTransfers, 1); n++;
    XtSetArg (al[n], XmNtransferProc, pw_receive_transfer); n++;

    XmDropTransferStart(client, al, n);
}

/************************************************************************
 *
 * pw_drop_help_ok_callback
 *
 ***********************************************************************/

static void
#ifdef _NO_PROTO
pw_drop_help_ok_callback(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
#else
pw_drop_help_ok_callback(Widget w, XtPointer client, XtPointer call)
#endif /* _NO_PROTO */
{
    DropHelpCallbackStruct *helpStruct;

    helpStruct = (DropHelpCallbackStruct *) client;

    pw_do_drop_transfer(helpStruct->w, 
                        (XtPointer) helpStruct->dragContext, 
                        (XtPointer) call);

}


/************************************************************************
 *
 * pw_drop_help_cancel_callback
 *
 ***********************************************************************/

static void
#ifdef _NO_PROTO
pw_drop_help_cancel_callback(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
#else
pw_drop_help_cancel_callback(Widget w, XtPointer client, XtPointer call)
#endif /* _NO_PROTO */
{

    DropHelpCallbackStruct *helpStruct;

    helpStruct = (DropHelpCallbackStruct *) client;

    drop_failure(helpStruct->dragContext);

}


/************************************************************************
 *
 * pw_drag_proc_callback
 *
 *     Handles drop site messages and does drag under effects for the
 *     printer list box
 *
 ************************************************************************/

static void
#ifdef _NO_PROTO
pw_drop_proc_callback(w, client, call)
   Widget 	w;
   XtPointer	client;
   XtPointer	call;
#else
pw_drop_proc_callback(Widget w, XtPointer client, XtPointer call)
#endif
{

    XmDropProcCallback DropData;
    static DropHelpCallbackStruct helpData;
    Arg al[10];
    int n;
    DXmPrintWgt pw;
    MrmType ar_dummy_class;
    static Boolean  haveHelpDialog = FALSE;

    n = 0;
    DropData = (XmDropProcCallback) call;


    DropData->dropSiteStatus = XmVALID_DROP_SITE;        
    if (DropData->dropAction != XmDROP)
    {

        /* Doing help.  Find the printwgt, then get the message box from
           the printwgt UID file.  */


        while ((pw) && (!DXmIsPrintWidget((Widget)pw)))
            pw = (DXmPrintWgt) XtParent(pw);
        if (pw == NULL)
        {
	    drop_failure(DropData->dragContext);
            return;
        }             

        if (!haveHelpDialog)
        {

            /* Go fetch it from UID */
            if (MrmFetchWidget (PWar_mrm_hierarchy(pw),
                                K_PW_DROP_HELP_NAME,
                                (Widget) pw,
                                &PWar_widget_id(pw, K_PW_DROP_HELP),
                                &ar_dummy_class) != MrmSUCCESS)
            {
                /* This could perhaps do something more, like explain that
                   there isn't any help, but where would one get the
                   message box for that? */
		drop_failure(DropData->dragContext);
                return;
            }

            XtAddCallback(PWar_widget_id(pw, K_PW_DROP_HELP),
                          XmNokCallback,
                          (XtCallbackProc) pw_drop_help_ok_callback,
                          (XtPointer) &helpData);
            XtAddCallback(PWar_widget_id(pw, K_PW_DROP_HELP),
                          XmNcancelCallback,
                          (XtCallbackProc) pw_drop_help_cancel_callback,
                          (XtPointer) &helpData);


            XtUnmanageChild(
                  (Widget) XmMessageBoxGetChild(
                                PWar_widget_id(pw, K_PW_DROP_HELP), 
                                XmDIALOG_HELP_BUTTON));
            XtRealizeWidget(PWar_widget_id(pw, K_PW_DROP_HELP));  
          
        }

        /* set up the callback data to be passed */


        helpData.w = w;  
        helpData.dragContext = DropData->dragContext;

        XtManageChild(PWar_widget_id(pw, K_PW_DROP_HELP));
        haveHelpDialog = TRUE;
    }
    else {
        /* Cancel the drop on invalid drop operations or incompatible
           target types */

        Atom ct;
        ct = XmInternAtom(XtDisplay(w),"COMPOUND_TEXT", False);

        if ((!DropData->operations & XmDROP_COPY) || 
           (!GetTargets(DropData->dragContext,ct))) 
        {
            DropData->operation = XmDROP_NOOP;
            DropData->dropSiteStatus = XmINVALID_DROP_SITE;
            drop_failure(DropData->dragContext);
         }
         else pw_do_drop_transfer(w, DropData->dragContext, call);
    }
}

/************************************************************************
 *
 * pw_register_drop_site
 *
 *     Registers the printer list box as a drop site
 *
 ************************************************************************/

static void
#ifdef _NO_PROTO
pw_register_drop_site(w)
   Widget w;
#else
pw_register_drop_site(Widget w)
#endif
{
    Display *display = XtDisplay(w);
    Atom    targets[2];
    Arg     args[5];
    int     n = 0;

    /* Only accept moves or copies */
    XtSetArg(args[n], XmNdropSiteOperations, XmDROP_COPY | XmDROP_MOVE); n++;

    /* set all possible targets for any of the nested drop sites */
    targets[0] = XmInternAtom(display, "COMPOUND_TEXT", False);
    XtSetArg(args[n], XmNimportTargets, targets); n++;
    XtSetArg(args[n], XmNnumImportTargets, 1); n++;
    XtSetArg(args[n], XmNdropProc, pw_drop_proc_callback); n++;
    XmDropSiteRegister(w, args, n);



}


/************************************************************************/
/*									*/
/* pw_class_initialize							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine initializes the class instance for the print       */
/*	widget class.							*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	none								*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	The dxmPrintWgtWidgetClass static variable.			*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	void								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	The print widget class is initialized.				*/
/*									*/
/************************************************************************/
static void pw_class_initialize()
{
    XmBulletinBoardWidgetClass	superclass;
    DXmPrintWgtClass      	myclass;

    /********************************************************************/
    /*									*/
    /* inherit super-class routines needed by print widget              */
    /*									*/
    /********************************************************************/
    myclass 	= (DXmPrintWgtClass) dxmPrintWgtWidgetClass;
    superclass 	= (XmBulletinBoardWidgetClass) myclass->core_class.superclass;

    myclass->core_class.resize = superclass->core_class.resize;

    INHERIT_COMPOSITE(myclass, superclass, geometry_manager);

} /* pw_class_initialize */


/************************************************************************/
/*									*/
/* pw_destroy								*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine destroys this instance of the print widget.        */
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	w		Widget						*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	void								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_destroy(ar_w)
    Widget ar_w;
{
    DXmPrintWgt			ar_pw = (DXmPrintWgt) ar_w;
    pw_ar_res_union		ar_resources;
    unsigned long int		l_i,l_j;

#ifdef PRINTWGT_DEBUG    
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_destroy\n");
#endif

    /********************************************************************/
    /*									*/
    /* Close the UID file hierarchy 					*/
    /*									*/
    /********************************************************************/
    MrmCloseHierarchy(PWar_mrm_hierarchy(ar_pw));
    
    /********************************************************************/
    /*									*/
    /* Free the memory associated with the resource			*/
    /*									*/
    /********************************************************************/
    ar_resources = PWar_resources(ar_pw);
    
    for (l_i = 0; l_i < K_PW_NUM_RESOURCES; l_i++)
    {
	switch (pw_ar_resource_control[l_i].l_type)
	{
	    case K_PW_RESOURCE_TYPE_INT_LIST:
	    {
		if (ar_resources[l_i].r_int_list_res.al_int_list)
		    XtFree((char *)ar_resources[l_i].r_int_list_res.al_int_list);	

		break;
	    }
	    
	    case K_PW_RESOURCE_TYPE_CS:
	    {
		if ((ar_resources[l_i].r_cs_res.ar_current_cs) &&
		    ((int) ar_resources[l_i].r_cs_res.ar_current_cs != -1))
		    XmStringFree(ar_resources[l_i].r_cs_res.ar_current_cs);
		if ((ar_resources[l_i].r_cs_res.ar_previous_cs) &&
		    ((int) ar_resources[l_i].r_cs_res.ar_previous_cs != -1))
		    XmStringFree(ar_resources[l_i].r_cs_res.ar_previous_cs);
		break;
	    }
	    
	    case K_PW_RESOURCE_TYPE_CS_LIST:
	    {
		if (ar_resources[l_i].r_cs_list_res.ar_cs_list)
		{
		    pw_free_cs_list_ref_memory(ar_resources[l_i].r_cs_list_res.ar_cs_list,
					       ar_resources[l_i].r_cs_list_res.l_cs_count);
		    XtFree((char *)ar_resources[l_i].r_cs_list_res.ar_cs_list);	
		}

		break;
	    }

	    case K_PW_RESOURCE_TYPE_CALLBACK_LIST:
	    {
		if (ar_resources[l_i].r_callback_list_res.ar_callback_list)
		{
		    XtRemoveAllCallbacks ((Widget)ar_pw, 
					  ar_resources[l_i].r_callback_list_res.at_callback_list_name);
		}

		break;
	    }

	    default:	break;

	} /* switch */
    } /* for */
    
    /********************************************************************/
    /*									*/
    /* Free the memory associated with the now string and the error	*/
    /* message table.							*/
    /*									*/
    /********************************************************************/
    if (PWar_now_string(ar_pw))
	XmStringFree(PWar_now_string(ar_pw));
    
    if (PWr_error_messages(ar_pw).l_cs_count)
    {
	pw_free_cs_list_ref_memory(PWr_error_messages(ar_pw).ar_cs_list,
				   PWr_error_messages(ar_pw).l_cs_count);
	XtFree((char *)PWr_error_messages(ar_pw).ar_cs_list);
    }

    /********************************************************************/
    /*									*/
    /* Free the memory associated with current format queue choices.	*/
    /*									*/
    /********************************************************************/
    if (PWal_current_format_queue_choices(ar_pw))
	XtFree((char *)PWal_current_format_queue_choices(ar_pw));
    
    /********************************************************************/
    /*									*/
    /* Free the memory associated with the known queue lists.		*/
    /*									*/
    /********************************************************************/
    if (PWar_known_queue_lists(ar_pw))
    {
	for (l_i = 0; 
	     l_i < PWl_num_known_queue_lists(ar_pw);
	     l_i++)
	{
	    XmStringFree((XmString)PWar_known_queue_lists(ar_pw)[l_i].ar_variable_name);

	    if (PWar_known_queue_lists(ar_pw)[l_i].ar_cs_list)
	    {
		pw_free_cs_list_ref_memory(PWar_known_queue_lists(ar_pw)[l_i].ar_cs_list,
					   PWar_known_queue_lists(ar_pw)[l_i].l_cs_count);
		XtFree((char *)PWar_known_queue_lists(ar_pw)[l_i].ar_cs_list);
	    }
	}
	
	XtFree((char *)PWar_known_queue_lists(ar_pw));
    }

    /********************************************************************/
    /*									*/
    /* Free memory associated with pulldown menu OS mapping tables.	*/
    /*									*/
    /********************************************************************/
    for (l_i = 0; l_i < K_PW_NUM_PULLDOWN_MENUS; l_i++)
    {
	if (PWar_pulldown_menu_os_table(ar_pw,l_i).ar_os_list)
	{
	    for (l_j = 0; l_j < PWar_pulldown_menu_os_table(ar_pw,l_i).l_os_count; l_j++)
		XtFree(PWar_pulldown_menu_os_table(ar_pw,l_i).ar_os_list[l_j]);

	    XtFree((char *)PWar_pulldown_menu_os_table(ar_pw,l_i).ar_os_list);
	}
    }

    /********************************************************************/
    /*									*/
    /* Free the memory associated with the default queue list variable.	*/
    /*								        */
    /********************************************************************/
    if (PWar_default_queue_list_variable(ar_pw))
	XmStringFree(PWar_default_queue_list_variable(ar_pw));

    if (PWar_default_queue_variable(ar_pw))
	XmStringFree(PWar_default_queue_variable(ar_pw));

    /********************************************************************/
    /*									*/
    /* Since nobody but the print widget knows about the shell widget	*/
    /* which holds the print widget, the print widget (who's the one	*/
    /* who created it in the first place), is responsible for killing	*/
    /* it.								*/
    /*									*/
    /********************************************************************/
    if (PWar_dialog_shell(ar_pw) != NULL)
    {
	if (PWar_dialog_shell(ar_pw)->shell.popped_up)
	    XtPopdown((Widget)PWar_dialog_shell(ar_pw));
	XtDestroyWidget((Widget)PWar_dialog_shell(ar_pw));
    }

    /********************************************************************/
    /*									*/
    /* Free the widget id array of the print widget.			*/
    /*									*/
    /********************************************************************/
    XtFree((char *)PWar_widget_ids(ar_pw));

} /* pw_destroy */


/************************************************************************/
/*									*/
/* pw_expose								*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine is called when this instance of the print widget	*/
/*	is exposed.						        */
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/*	event		Event						*/
/*									*/
/*	region		Region						*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	void								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	l_primary_saved variable in widget instance may get set.	*/
/*									*/
/************************************************************************/
static void pw_expose(ar_pw,ar_event,ar_region)
    DXmPrintWgt ar_pw;
    XEvent 	*ar_event;
    Region 	ar_region;
{

#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
    {
	printf("pw_expose: ");
        if (PWl_primary_saved(ar_pw))
	    printf("primary variables have been saved.\n");
	else
	    printf("primary variables need to be saved.\n");
    }
#endif

    /********************************************************************/
    /*									*/
    /* If the widget was just created, make the widgets match the	*/
    /* resources, and handle the suppress options mask.			*/
    /*									*/
    /********************************************************************/
    if (PWl_primary_just_created(ar_pw))
    {
	/****************************************************************/
	/*								*/
	/* If the application added a child, and it's x,y position has	*/
	/* not been set, set the x,y position to be below the 		*/
	/* orientation menu.						*/
	/*								*/
	/****************************************************************/
	pw_adjust_application_child_location(ar_pw);
	
	/****************************************************************/
	/*								*/
	/* [[[HACK HACK HACK]]]						*/
	/*								*/
	/*	We need to make it so that if an application adds	*/
	/*	children to this widget, the OK button will be invoked. */
	/*		 						*/
	/****************************************************************/
	ar_pw->bulletin_board.default_button = (PWar_widget_id(ar_pw,
							       K_PW_OK));
	
	/****************************************************************/
	/*								*/
	/* Make the widgets match the values of the resources.		*/
	/*								*/
	/****************************************************************/
	pw_make_widgets_match_resources(ar_pw,
					K_PW_PRIMARY_RESOURCES_START,
					K_PW_PRIMARY_RESOURCES_FINISH);
		    
	PWl_primary_just_created(ar_pw) = FALSE;
    }
	
    /********************************************************************/
    /*									*/
    /* Make the widgets match the values of the resources.		*/
    /*									*/
    /********************************************************************/
    if (!PWl_primary_saved(ar_pw))
    {
	pw_save_values(ar_pw,
		       K_PW_PRIMARY_RESOURCES_START,
		       K_PW_PRIMARY_RESOURCES_FINISH);

	PWl_primary_saved(ar_pw) = TRUE;
    }
    
} /* pw_expose */


/************************************************************************/
/*									*/
/* pw_initialize							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine initializes this instance of the print widget.     */
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	requested_w	Widget (as requested)				*/
/*									*/
/*	resulting_w	Widget (as it will be)				*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	void								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	The widget instance is initialized.				*/
/*									*/
/************************************************************************/
static void pw_initialize(ar_requested_w,ar_resulting_w)
    Widget ar_requested_w;
    Widget ar_resulting_w;
{
    DXmPrintWgt		ar_pw = (DXmPrintWgt) ar_resulting_w;
    pw_ar_res_union	ar_resources;
    unsigned long int	l_i;
    MrmType	        ar_dummy_class;
    Arg			ar_al[2];
    int			l_ac = 0;
    char		*at_temp;
    long		l_size,l_status;
    int			l_choice_index;
    

#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_initialize\n");
#endif

    /********************************************************************/
    /*									*/
    /* Create the wait cursor.						*/
    /*									*/
    /********************************************************************/
    PWr_wait_cursor(ar_pw) = _DXmCreateWaitCursor(ar_pw);
		
    /********************************************************************/
    /*									*/
    /* Initialize the ar_widget_ids array so the print widget will know */
    /* that none of them have been created yet.				*/
    /*									*/
    /********************************************************************/
    PWar_widget_ids(ar_pw) = (Widget *) XtMalloc(sizeof(Widget) * K_PW_NUM_WIDGETS);

    for (l_i = 0; l_i < K_PW_NUM_WIDGETS; l_i++)
	PWar_widget_id(ar_pw,l_i) = (Widget) NULL;
    
    PWar_dialog_shell(ar_pw) = NULL;
    
    /********************************************************************/
    /*									*/
    /* Initialize previous value associated with each copied resource	*/
    /*									*/
    /********************************************************************/
    ar_resources = PWar_resources(ar_pw);
    
    for (l_i = 0; l_i < K_PW_NUM_RESOURCES; l_i++)
    {
	switch (pw_ar_resource_control[l_i].l_type)
	{
	    case K_PW_RESOURCE_TYPE_CS:
	    case K_PW_RESOURCE_TYPE_CS_CHOICE:
	    {
		ar_resources[l_i].r_cs_res.ar_previous_cs = NULL;
		break;
	    }

	    default:	break;
	}
    }

    /********************************************************************/
    /*									*/
    /* Initialize the ascii values of the printer and			*/
    /* printer form choices to NULL.					*/
    /*									*/
    /********************************************************************/
    PWr_os_printer_choice(ar_pw)       	= (Opaque) NULL;
    PWr_os_printer_form_choice(ar_pw) 	= (Opaque) NULL;

    /********************************************************************/
    /*									*/
    /* Initialize the values of the now string and error messages 	*/
    /* to NULL.								*/
    /*									*/
    /********************************************************************/
    PWar_now_string(ar_pw) = NULL;    
    PWr_error_messages(ar_pw).ar_cs_list = NULL;
    PWr_error_messages(ar_pw).l_cs_count = -1;

    /********************************************************************/
    /*									*/
    /* Initialize the callback list names.				*/
    /*									*/
    /********************************************************************/
    ar_resources[K_PW_OK_CALLBACK_LIST_MAP].r_callback_list_res.at_callback_list_name =
	XmNokCallback;
    
    ar_resources[K_PW_CANCEL_CALLBACK_LIST_MAP].r_callback_list_res.at_callback_list_name = 
	XmNcancelCallback;

    /********************************************************************/
    /*									*/
    /* Initialize the primary saved variable to false.			*/
    /*									*/
    /********************************************************************/
    PWl_primary_saved(ar_pw) = 0;

    PWl_primary_just_created(ar_pw) = FALSE;
    PWl_2ndary_just_created(ar_pw) = FALSE;

    /********************************************************************/
    /*									*/
    /* Initialize the pulldown menu (option menu) OS mapping tables to	*/
    /* NULL and set their counts to 0.					*/
    /*									*/
    /********************************************************************/
    for (l_i = 0; l_i < K_PW_NUM_PULLDOWN_MENUS; l_i++)
    {
	PWar_pulldown_menu_os_table(ar_pw,l_i).ar_os_list = NULL;
	PWar_pulldown_menu_os_table(ar_pw,l_i).l_os_count = 0;
    }

    /********************************************************************/
    /*									*/
    /* Initialize default queue list variable to NULL.  This variable,	*/
    /* if it exists, can be used as a short-cut to finding queues on	*/
    /* the system.							*/
    /********************************************************************/
    PWar_default_queue_list_variable(ar_pw) = NULL;
    PWar_default_queue_variable(ar_pw) = NULL;
    
    PWl_print_format_choice(ar_pw) = 0;

    /********************************************************************/
    /*									*/
    /* Copy user's memory on create - don't want to stomp on user's	*/
    /* memory.								*/
    /*									*/
    /********************************************************************/
    pw_copy_memory_on_create(ar_pw);
    
    /********************************************************************/
    /*									*/
    /* Open the print widget hierarchy					*/
    /*									*/
    /********************************************************************/
    ar_mrm_filename_vec[0] = (char *) DXmCvtCStoOS(PWr_cs_res(ar_pw,K_PW_UID_SPEC_MAP).ar_current_cs,
						   &l_size,
						   &l_status);
    
    if (MrmOpenHierarchy(l_mrm_filename_count, 		/* number of files	*/
			 ar_mrm_filename_vec, 		/* files     	        */
			 NULL,				/* ancillary structures	*/
			 &PWar_mrm_hierarchy(ar_pw))	/* ptr to returned id   */
	!= MrmSUCCESS)
    {	
	DXMAPPWARNING(XtWidgetToApplicationContext((Widget)ar_pw),
		      PWMSGNAME6,
		      PWGTNOMRMOPEN,
		      NULL,NULL);
    }

    /********************************************************************/
    /*									*/
    /* Fetch some literals from the MRM data base for the print widget. */
    /*									*/
    /********************************************************************/
    pw_fetch_mrm_resources(ar_pw);

    /********************************************************************/
    /*									*/
    /* Register our callback routines so that the resource manager can  */
    /* resolve them at widget-creation time.                            */
    /*                                                                  */
    /********************************************************************/
    MrmRegisterNames(ar_mrm_register_args,
		     (MrmCount) l_mrm_register_args_count);

    /********************************************************************/
    /*									*/
    /* Fetch from MRM the entire printwgt hierarchy of widgets.  	*/
    /* They are all unmanaged at this point.  The widget id of the root	*/
    /* of the hieriarchy is returned in pqw.  This call will, in turn, 	*/
    /* provoke callbacks to the pw_create_proc for every widget created.*/
    /* It is in this callback routine where we can make last-minute 	*/
    /* changes to the widgets before anyone sees them on the screen.	*/
    /*									*/
    /********************************************************************/
    if (MrmFetchWidget (PWar_mrm_hierarchy(ar_pw),	/* MRM heirarchy	*/
			K_PW_PRIMARY_BOX_NAME,		/* Name of widget	*/
			(Widget) ar_pw,				/* Parent		*/
			&PWar_widget_id(ar_pw,
					K_PW_PRIMARY_BOX), /* Primary Box		*/
			&ar_dummy_class) 		/* Class		*/
	!= MrmSUCCESS)
    {
	String params = K_PW_PRIMARY_BOX_NAME;
	Cardinal num_params = 1;
	
	DXMAPPWARNING(XtWidgetToApplicationContext((Widget)ar_pw),
		      PWMSGNAME2,
		      PWGTNOMRMFETCH,
		      &params,&num_params);
    }

    PWl_primary_just_created(ar_pw) = TRUE;

    /********************************************************************/
    /*									*/
    /* Fix up the tab groups since the Motif defaults really suck.	*/
    /* I don't mean to be abusive or anything, but whoever grouped	*/
    /* option menus, toggle buttons, and push buttons in the same 	*/
    /* tab group was really a moron who should be shot on sight.	*/
    /*									*/
    /********************************************************************/
    pw_tabify_primary_box(ar_pw);
    
    /********************************************************************/
    /*									*/
    /* Get the form names from the system (if they already haven't been	*/
    /* set - they shouldn't be since this is a read only resource).	*/
    /*									*/
    /* Also, if there is a current print format choice, make sure it 	*/
    /* points to an element in the list.				*/
    /*									*/
    /********************************************************************/
    PWr_cs_list_res(ar_pw,K_PW_PRINTER_FORM_CONTENTS_MAP).l_cs_count = -1;
    pw_get_print_form_names(&(PWr_cs_list_res(ar_pw,K_PW_PRINTER_FORM_CONTENTS_MAP).ar_cs_list),
  			    &(PWr_cs_list_res(ar_pw,K_PW_PRINTER_FORM_CONTENTS_MAP).l_cs_count));

    if (PWr_cs_res(ar_pw,K_PW_PRINTER_FORM_CHOICE_MAP).ar_current_cs)
    {
	l_choice_index = pw_get_cs_list_element_index(PWr_cs_res(ar_pw,K_PW_PRINTER_FORM_CHOICE_MAP).ar_current_cs,
						      PWr_cs_list_res(ar_pw,K_PW_PRINTER_FORM_CONTENTS_MAP).ar_cs_list,
						      PWr_cs_list_res(ar_pw,K_PW_PRINTER_FORM_CONTENTS_MAP).l_cs_count);
	if (l_choice_index >= 0)
	    PWr_cs_res(ar_pw,K_PW_PRINTER_FORM_CHOICE_MAP).ar_current_cs = 
		PWr_cs_list_res(ar_pw,K_PW_PRINTER_FORM_CONTENTS_MAP).ar_cs_list[l_choice_index];
	else
	    PWr_cs_res(ar_pw,K_PW_PRINTER_FORM_CHOICE_MAP).ar_current_cs = NULL;
    }

    /********************************************************************/
    /*									*/
    /* Initialize the current format queue choices to -1 so we know	*/
    /* a queue hasn't been selected by the user, yet.			*/
    /*									*/
    /********************************************************************/
    PWal_current_format_queue_choices(ar_pw) = 
	(int *) XtMalloc(sizeof(int) * 
			 PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).l_cs_count);

    for (l_i = 0; l_i < PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).l_cs_count; l_i++)
	PWal_current_format_queue_choices(ar_pw)[l_i] = -1;

    /********************************************************************/
    /*									*/
    /* Initialize the known queue lists variables.  We need to have the */
    /* zeroeth element be the default element.  If the default queue	*/
    /* list variable has been defined, get its value and make that be	*/
    /* the default list.  Otherwise, it will be necessary to ask the 	*/
    /* system for all of its queues.  By setting the count to -1, we 	*/
    /* can delay this query until it is really necessary.		*/
    /*									*/
    /********************************************************************/
    PWl_num_known_queue_lists(ar_pw) = 1;
    PWar_known_queue_lists(ar_pw) = 
	(pw_r_variable_list_struct *) XtMalloc(K_PW_CHUNK_INCR *
					       sizeof(pw_r_variable_list_struct));

    PWar_known_queue_lists(ar_pw)[0].ar_variable_name = 
	XmStringCopy(PWar_default_queue_list_variable(ar_pw));
    
    if (pw_get_variable_value(PWar_known_queue_lists(ar_pw)[0].ar_variable_name,
			      &at_temp))
    {
	PWar_known_queue_lists(ar_pw)[0].ar_cs_list = NULL;
	PWar_known_queue_lists(ar_pw)[0].l_cs_count = 0;	    
	pw_parse_comma_string(at_temp,
			      &PWar_known_queue_lists(ar_pw)[0].ar_cs_list,
			      &PWar_known_queue_lists(ar_pw)[0].l_cs_count);

	XtFree(at_temp);
    }
    else
    {
	PWar_known_queue_lists(ar_pw)[0].l_cs_count = -1;
	PWar_known_queue_lists(ar_pw)[0].ar_cs_list = NULL;
    }
        
    PWl_current_queue_list_index(ar_pw) = -1;
    
    /********************************************************************/
    /*									*/
    /* It may be necessary to find out what the default printer     	*/
    /* is.  This will be indicated by the default_printer resource  	*/
    /* being set to -1.							*/
    /*									*/
    /********************************************************************/
    if ((int)PWr_cs_res(ar_pw,K_PW_DEFAULT_PRINTER_MAP).ar_current_cs == -1)
    {
	char *at_asciz_default_printer;
        long l_size,l_status;
	    
	if (pw_get_variable_value(PWar_default_queue_variable(ar_pw),
			          &at_asciz_default_printer))
	{
	    PWr_cs_res(ar_pw,K_PW_DEFAULT_PRINTER_MAP).ar_current_cs =
	        (XmString) DXmCvtOStoCS(at_asciz_default_printer,
                                        &l_size,
                			&l_status);
	    XtFree(at_asciz_default_printer);
	}
	else
	    PWr_cs_res(ar_pw,K_PW_DEFAULT_PRINTER_MAP).ar_current_cs = NULL;
    }

    /********************************************************************/
    /*									*/
    /* Validate/Change startup values.					*/
    /*									*/
    /********************************************************************/
    pw_validate_startup_values(ar_pw,
 			       K_PW_PRIMARY_RESOURCES_START,
 			       K_PW_2NDARY_RESOURCES_FINISH);

    /********************************************************************/
    /*									*/
    /* populate the print format list box				*/
    /*									*/
    /********************************************************************/
    pw_populate_print_format_listbox(ar_pw);

    /********************************************************************/
    /*									*/
    /* Suppress the desired options.					*/
    /*									*/
    /********************************************************************/
    pw_handle_suppress_options_mask(ar_pw);

    /********************************************************************/
    /*									*/
    /* Manage the box.							*/
    /*									*/
    /********************************************************************/
    XtManageChild(PWar_widget_id(ar_pw,K_PW_PRIMARY_BOX));

} /* pw_initialize */


/************************************************************************/
/*									*/
/* pw_set_values							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine is called whenever a XtSetValues call is made	*/
/*	which modifies a public resource fiels within the print widget. */
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	old_w		Widget (as it used to be)			*/
/*									*/
/*	requested_w	Widget (as it wants to be)			*/
/*									*/
/*	resulting_w	Widget (as it will be)				*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	Boolean		FALSE - nothing in the print widget itself	*/
/*			needs to be redrawn.				*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static Boolean pw_set_values(ar_old_w,ar_requested_w,ar_new_w)
    Widget ar_old_w;
    Widget ar_requested_w;
    Widget ar_new_w;
{
    DXmPrintWgt			ar_requested_pw = (DXmPrintWgt) ar_requested_w;
    DXmPrintWgt			ar_old_pw = (DXmPrintWgt) ar_old_w;
    DXmPrintWgt			ar_new_pw = (DXmPrintWgt) ar_new_w;
    unsigned long int		l_i;

#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_requested_pw).l_current_value)
	printf("pw_set_values\n");
#endif

    /********************************************************************/
    /*									*/
    /* Loop through each of the resources and call the appropriate	*/
    /* set values routine.						*/
    /*									*/
    /********************************************************************/
    for (l_i = 0; l_i < K_PW_NUM_RESOURCES; l_i++)
    {
	if (pw_ar_resource_control[l_i].set_value_proc)
	    (*(pw_ar_resource_control[l_i].set_value_proc))(ar_old_w,ar_new_w,l_i);
    };
    
    return (Boolean) FALSE;
    
} /* pw_set_values */


/************************************************************************/
/*									*/
/* pw_fetch_mrm_resources						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine fetches local sensitive resources from MRM.	*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	unsigned int		1					*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static Cardinal pw_fetch_mrm_resources(ar_pw)
    DXmPrintWgt		ar_pw;
{
    int			*al_temp_int_table;
    XmStringTable	r_temp_cs_table;
    MrmCode	        r_literal_type;
    int			l_i;
    long		l_size,l_status;

#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_fetch_mrm_resources\n");
#endif

    /********************************************************************/
    /*									*/
    /* Fetch the definition for "now"					*/
    /*									*/
    /********************************************************************/
    if (MrmFetchLiteral(PWar_mrm_hierarchy(ar_pw),
			K_PW_NOW_STRING_NAME,			/* index (name) of literal */
			NULL,					/* display */
			(XtPointer *) &PWar_now_string(ar_pw),	/* return value */
			&r_literal_type)			/* return type */
	!= MrmSUCCESS)
    {
	String params = K_PW_NOW_STRING_NAME;
	Cardinal num_params = 1;
	
	DXMAPPWARNING(XtWidgetToApplicationContext((Widget)ar_pw),
		      PWMSGNAME2,
		      PWGTNOMRMFETCH,
		      &params,&num_params);

	return MrmFAILURE;
    }
        
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_fetch_mrm_resources %s is %s\n",
	       K_PW_NOW_STRING_NAME,
	       DXmCvtCStoOS(PWar_now_string(ar_pw),
                            &l_size,
                            &l_status));
#endif

    /*
    ** Make sure we let the resource know about this...
    */
    if (!PWr_cs_res(ar_pw,K_PW_PRINT_AFTER_MAP).ar_current_cs)
        PWr_cs_res(ar_pw,K_PW_PRINT_AFTER_MAP).ar_current_cs = XmStringCopy(PWar_now_string(ar_pw));

    /********************************************************************/
    /*									*/
    /* Fetch the definition for default queue list variable		*/
    /*									*/
    /********************************************************************/
    if (MrmFetchLiteral(PWar_mrm_hierarchy(ar_pw),
			K_PW_DEFAULT_QUEUE_LIST_VARIABLE_NAME,			/* index (name) of literal */
			NULL,							/* display */
			(XtPointer *) &PWar_default_queue_list_variable(ar_pw),	/* return value */
			&r_literal_type)					/* return type */
	!= MrmSUCCESS)
    {
	String params = K_PW_DEFAULT_QUEUE_LIST_VARIABLE_NAME;
	Cardinal num_params = 1;
	
	DXMAPPWARNING(XtWidgetToApplicationContext((Widget)ar_pw),
		      PWMSGNAME2,
		      PWGTNOMRMFETCH,
		      &params,&num_params);

	return MrmFAILURE;
    }
        
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_fetch_mrm_resources %s is %s\n",
	       K_PW_DEFAULT_QUEUE_LIST_VARIABLE_NAME,
	       DXmCvtCStoOS(PWar_default_queue_list_variable(ar_pw),
                            &l_size,
                            &l_status));
#endif

    /********************************************************************/
    /*									*/
    /* Fetch the definition for default queue variable			*/
    /*									*/
    /********************************************************************/
    if (MrmFetchLiteral(PWar_mrm_hierarchy(ar_pw),
			K_PW_DEFAULT_QUEUE_VARIABLE_NAME,		/* index (name) of literal */
			NULL,						/* display */
			(XtPointer *) &PWar_default_queue_variable(ar_pw),/* return value */
			&r_literal_type)				/* return type */
	!= MrmSUCCESS)
    {
	String params = K_PW_DEFAULT_QUEUE_VARIABLE_NAME;
	Cardinal num_params = 1;
	
	DXMAPPWARNING(XtWidgetToApplicationContext((Widget)ar_pw),
		      PWMSGNAME2,
		      PWGTNOMRMFETCH,
		      &params,&num_params);

	return MrmFAILURE;
    }
        
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_fetch_mrm_resources %s is %s\n",
	       K_PW_DEFAULT_QUEUE_VARIABLE_NAME,
	       DXmCvtCStoOS(PWar_default_queue_variable(ar_pw),
                            &l_size,
                            &l_status));
#endif

    /********************************************************************/
    /*									*/
    /* Fetch the error message table					*/
    /*									*/
    /********************************************************************/
    if (MrmFetchLiteral(PWar_mrm_hierarchy(ar_pw),
			K_PW_ERROR_MESSAGE_TABLE_NAME,	/* index (name) of literal */
			NULL,				/* display */
			(XtPointer *) &r_temp_cs_table,	/* return value */
			&r_literal_type)		/* return type */
	!= MrmSUCCESS)
    {
	String params = K_PW_ERROR_MESSAGE_TABLE_NAME;
	Cardinal num_params = 1;
	
	DXMAPPWARNING(XtWidgetToApplicationContext((Widget)ar_pw),
		      PWMSGNAME2,
		      PWGTNOMRMFETCH,
		      &params,&num_params);

	return MrmFAILURE;
    }

    pw_copy_cs_list(r_temp_cs_table,
		    &PWr_error_messages(ar_pw).ar_cs_list,
		    &PWr_error_messages(ar_pw).l_cs_count);

#ifndef NO_FREE_MRM
    XtFree((char *)r_temp_cs_table);
#endif

#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
    {
	printf("pw_fetch_mrm_resources %s has %d elements\n",
	       K_PW_ERROR_MESSAGE_TABLE_NAME,
	       PWr_error_messages(ar_pw).l_cs_count);

	for (l_i = 0; l_i < PWr_error_messages(ar_pw).l_cs_count; l_i++)
	    printf("%d--->%s\n",
	       l_i,
	       DXmCvtCStoOS(PWar_error_message(ar_pw,l_i),
                            &l_size,
                            &l_status));
    }
#endif
    
    /********************************************************************/
    /*									*/
    /* Fetch the print format tables if they already haven't been	*/
    /* defined.								*/
    /*									*/
    /********************************************************************/
    if (PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).l_cs_count < 0)
    {
	/****************************************************************/
	/*								*/
	/* User interface.						*/
	/*								*/
	/****************************************************************/
	if (MrmFetchLiteral(PWar_mrm_hierarchy(ar_pw),
			    K_PW_PRINT_FORMAT_UI_TABLE_NAME,	/* index (name) */
			    NULL,				/* display */
			    (XtPointer *) &r_temp_cs_table,	/* return value */
			    &r_literal_type)			/* return type */
	    != MrmSUCCESS) 
	{
	    String params = K_PW_PRINT_FORMAT_UI_TABLE_NAME;
	    Cardinal num_params = 1;
	
	    DXMAPPWARNING(XtWidgetToApplicationContext((Widget)ar_pw),
			  PWMSGNAME2,
			  PWGTNOMRMFETCH,
			  &params,&num_params);

	    return MrmFAILURE;
	}

	pw_copy_cs_list(r_temp_cs_table,
			&PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).ar_cs_list,
			&PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).l_cs_count);

#ifndef NO_FREE_MRM
	XtFree((char *)r_temp_cs_table);
#endif

#ifdef PRINTWGT_DEBUG
	if (PWr_debug(ar_pw).l_current_value)
	{
	    printf("pw_fetch_mrm_resources %s has %d elements\n",
		   K_PW_PRINT_FORMAT_UI_TABLE_NAME,
		   PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).l_cs_count);

	    for (l_i = 0; l_i < PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).l_cs_count; l_i++)
		printf("%d--->%s\n",
		       l_i,
		       DXmCvtCStoOS(PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).ar_cs_list[l_i],
                                    &l_size,
                                    &l_status));
	}
#endif
    
	/****************************************************************/
	/*								*/
	/* Strings to pass to operating system.				*/
	/*								*/
	/****************************************************************/
	if (MrmFetchLiteral(PWar_mrm_hierarchy(ar_pw),
			    K_PW_PRINT_FORMAT_OS_TABLE_NAME,	/* index (name) */
			    NULL,				/* display */
			    (XtPointer *) &r_temp_cs_table,	/* return value */
			    &r_literal_type)       		/* return type */
	    != MrmSUCCESS) 
	{
	    String params = K_PW_PRINT_FORMAT_OS_TABLE_NAME;
	    Cardinal num_params = 1;
	
	    DXMAPPWARNING(XtWidgetToApplicationContext((Widget)ar_pw),
			  PWMSGNAME2,
			  PWGTNOMRMFETCH,
			  &params,&num_params);

	    return MrmFAILURE;
	}

	PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_OS_LIST_MAP).l_cs_count = 
	    PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).l_cs_count;
	
	pw_copy_cs_list(r_temp_cs_table,
			&PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_OS_LIST_MAP).ar_cs_list,
			&PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_OS_LIST_MAP).l_cs_count);

#ifndef NO_FREE_MRM
	XtFree((char *)r_temp_cs_table);
#endif

#ifdef PRINTWGT_DEBUG
	if (PWr_debug(ar_pw).l_current_value)
	{
	    printf("pw_fetch_mrm_resources %s has %d elements\n",
		   K_PW_PRINT_FORMAT_OS_TABLE_NAME,
		   PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_OS_LIST_MAP).l_cs_count);

	    for (l_i = 0; l_i < PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_OS_LIST_MAP).l_cs_count; l_i++)
		printf("%d--->%s\n",
		       l_i,
		       DXmCvtCStoOS(PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_OS_LIST_MAP).ar_cs_list[l_i],
                                    &l_size,
                                    &l_status));
	}
#endif
    
	/****************************************************************/
	/*								*/
	/* Strings for finding envronment variable values.		*/
	/*								*/
	/****************************************************************/
	if (MrmFetchLiteral(PWar_mrm_hierarchy(ar_pw),
			    K_PW_PRINT_FORMAT_VARIABLE_TABLE_NAME,	/* index (name) */
			    NULL,				/* display */
			    (XtPointer *) &r_temp_cs_table,	/* return value */
			    &r_literal_type)			/* return type */
	    != MrmSUCCESS) 
	{
	    String params = K_PW_PRINT_FORMAT_VARIABLE_TABLE_NAME;
	    Cardinal num_params = 1;
	
	    DXMAPPWARNING(XtWidgetToApplicationContext((Widget)ar_pw),
			  PWMSGNAME2,
			  PWGTNOMRMFETCH,
			  &params,&num_params);

	    return MrmFAILURE;
	}

	PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_VARIABLE_LIST_MAP).l_cs_count = 
	    PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).l_cs_count;
	
	pw_copy_cs_list(r_temp_cs_table,
			&PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_VARIABLE_LIST_MAP).ar_cs_list,
			&PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_VARIABLE_LIST_MAP).l_cs_count);

#ifndef NO_FREE_MRM
	XtFree((char *)r_temp_cs_table);
#endif

#ifdef PRINTWGT_DEBUG
	if (PWr_debug(ar_pw).l_current_value)
	{
	    printf("pw_fetch_mrm_resources %s has %d elements\n",
		   K_PW_PRINT_FORMAT_VARIABLE_TABLE_NAME,
		   PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_VARIABLE_LIST_MAP).l_cs_count);

	    for (l_i = 0; l_i < PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_VARIABLE_LIST_MAP).l_cs_count; l_i++)
		printf("%d--->%s\n",
		       l_i,
		       DXmCvtCStoOS(PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_VARIABLE_LIST_MAP).ar_cs_list[l_i],
                                    &l_size,
                                    &l_status));
	}
#endif
    
	/****************************************************************/
	/*								*/
	/* Guesser integers.						*/
	/*								*/
	/****************************************************************/
	if (MrmFetchLiteral(PWar_mrm_hierarchy(ar_pw),
			    K_PW_PRINT_FORMAT_GUESSER_TABLE_NAME,	/* index (name) */
			    NULL,					/* display */
			    (XtPointer *) &al_temp_int_table,		/* return value */
			    &r_literal_type)				/* return type */
	    != MrmSUCCESS) 
	{
	    String params = K_PW_PRINT_FORMAT_GUESSER_TABLE_NAME;
	    Cardinal num_params = 1;
	
	    DXMAPPWARNING(XtWidgetToApplicationContext((Widget)ar_pw),
			  PWMSGNAME2,
			  PWGTNOMRMFETCH,
			  &params,&num_params);

	    return MrmFAILURE;
	}

	PWr_int_list_res(ar_pw,K_PW_KNOWN_FORMAT_GUESSER_LIST_MAP).l_int_count = 
	    PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).l_cs_count;

	PWr_int_list_res(ar_pw,K_PW_KNOWN_FORMAT_GUESSER_LIST_MAP).al_int_list = al_temp_int_table;
	
#ifdef PRINTWGT_DEBUG
	if (PWr_debug(ar_pw).l_current_value)
	{
	    printf("pw_fetch_mrm_resources %s has %d elements\n",
		   K_PW_PRINT_FORMAT_GUESSER_TABLE_NAME,
		   PWr_int_list_res(ar_pw,K_PW_KNOWN_FORMAT_GUESSER_LIST_MAP).l_int_count);

	    for (l_i = 0; l_i < PWr_int_list_res(ar_pw,K_PW_KNOWN_FORMAT_GUESSER_LIST_MAP).l_int_count; l_i++)
		printf("%d--->%d\n",
		       l_i,
		       PWr_int_list_res(ar_pw,K_PW_KNOWN_FORMAT_GUESSER_LIST_MAP).al_int_list[l_i]);
	}
#endif
    
    } /* Get known print format tables. */
    
    /********************************************************************/
    /*									*/
    /* Fetch the default 2ndary dialog box title if it has not been set.*/
    /*									*/
    /********************************************************************/
    if (!PWr_cs_res(ar_pw,K_PW_2NDARY_TITLE_MAP).ar_current_cs)
    {
	if (MrmFetchLiteral(PWar_mrm_hierarchy(ar_pw),
			    K_PW_2NDARY_TITLE_NAME,					/* index (name) of literal */
			    NULL,							/* display */
			    (XtPointer *) &PWr_cs_res(ar_pw,K_PW_2NDARY_TITLE_MAP).ar_current_cs,	/* return value */
			    &r_literal_type)						/* return type */
	    != MrmSUCCESS)
	{
	    String params = K_PW_2NDARY_TITLE_NAME;
	    Cardinal num_params = 1;
	
	    DXMAPPWARNING(XtWidgetToApplicationContext((Widget)ar_pw),
			  PWMSGNAME2,
			  PWGTNOMRMFETCH,
			  &params,&num_params);

	    return MrmFAILURE;
	}
    }
    
    return MrmSUCCESS;

} /* pw_fetch_mrm_resources */

/************************************************************************/
/*									*/
/* pw_fetch_pulldown_menu_os_list					*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine fetches a pulldown menu OS list from MRM		*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	unsigned int		1					*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static Cardinal pw_fetch_pulldown_menu_os_list(ar_pw,at_list_name,l_pulldown)
    DXmPrintWgt		ar_pw;
    char		*at_list_name;
    int			l_pulldown;
{
    XmStringTable	r_temp_cs_table;
    MrmCode	        r_literal_type;
    int			l_i;
    int			l_count;
    long		l_size,l_status;

    /********************************************************************/
    /*									*/
    /* Fetch it only if necessary.					*/
    /*									*/
    /********************************************************************/
    if (!PWar_pulldown_menu_os_table(ar_pw,l_pulldown).ar_os_list)
    {
#ifdef PRINTWGT_DEBUG
	if (PWr_debug(ar_pw).l_current_value)
	    printf("pw_fetch_pulldown_menu_os_list %d\n",l_pulldown);
#endif

	/****************************************************************/
	/*								*/
	/* Fetch the null terminated CS OS table			*/
	/*								*/
	/****************************************************************/
	if (MrmFetchLiteral(PWar_mrm_hierarchy(ar_pw),
			    at_list_name,			/* index (name) of literal */
			    NULL,				/* display */
			    (XtPointer *) &r_temp_cs_table,	/* return value */
			    &r_literal_type)			/* return type */
	    != MrmSUCCESS)
	{
	    String params = at_list_name;
	    Cardinal num_params = 1;
	
	    DXMAPPWARNING(XtWidgetToApplicationContext((Widget)ar_pw),
			  PWMSGNAME2,
			  PWGTNOMRMFETCH,
			  &params,&num_params);

	    return MrmFAILURE;
	}

	/****************************************************************/
	/*					       			*/
	/* Convert the list to OS strings and save it.			*/
	/*								*/
	/****************************************************************/
	l_count = 0;
	for (l_i = 0; r_temp_cs_table[l_i]; l_i++)
	    l_count++;
	
	if (l_count)
	{		
	    PWar_pulldown_menu_os_table(ar_pw,l_pulldown).ar_os_list =
		(Opaque *) XtMalloc(sizeof(Opaque) * l_count);

	    for (l_i = 0; l_i < l_count; l_i++)
	    {
		PWar_pulldown_menu_os_table(ar_pw,l_pulldown).ar_os_list[l_i] =
		    (Opaque) DXmCvtCStoOS(r_temp_cs_table[l_i],
                                          &l_size,
                                          &l_status);

#ifdef PRINTWGT_DEBUG
		if (PWr_debug(ar_pw).l_current_value)
		    printf("os list element %d: %s\n",
			   l_i,
			   PWar_pulldown_menu_os_table(ar_pw,l_pulldown).ar_os_list[l_i]);
#endif
	    }
	}

	PWar_pulldown_menu_os_table(ar_pw,l_pulldown).l_os_count = l_count;

#ifndef NO_FREE_MRM
	XtFree((char *)r_temp_cs_table);
#endif
    }
    
    return MrmSUCCESS;

} /* pw_fetch_pulldown_menu_os_list */

/************************************************************************/
/*									*/
/* pw_activate_proc							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine is called whenever one of the print widget control */
/*	buttons (i.e. OK, CANCEL, OPTIONS..., OK, CANCEL) is pressed.	*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	w		Widget (which was activated)			*/
/*									*/
/*	tag		Widget (index into ra_widget_ids array)		*/
/*									*/
/*	callback_data	Callback data for activation.			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	unsigned int		1					*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static unsigned int pw_activate_proc(ar_w,al_tag,ar_callback_data)
    Widget		ar_w;
    int			*al_tag;
    XmAnyCallbackStruct	*ar_callback_data;
{
    DXmPrintWgt		ar_pw;

    /********************************************************************/
    /*									*/
    /* Find the print widget (it is guaranteed to be a parent of this	*/
    /* widget).								*/
    /*									*/
    /********************************************************************/
    ar_pw = (DXmPrintWgt) ar_w;
    while (!XtIsSubclass((Widget)ar_pw,dxmPrintWgtWidgetClass))   
	ar_pw = (DXmPrintWgt)XtParent(ar_pw);

#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_activate_proc: %d\n",*al_tag);
#endif

    /********************************************************************/
    /*								       	*/
    /* Figure out which widget was activated and handle it.		*/
    /*									*/
    /********************************************************************/
    switch (*al_tag)
    {
	/****************************************************************/
	/*								*/
	/* Primary box OK button					*/
	/*								*/
	/****************************************************************/
	case K_PW_OK:
	{
	    /************************************************************/
	    /*								*/
	    /* If the 2ndary box is up, we must check those values,	*/
	    /* too.							*/
	    /*								*/
	    /************************************************************/
	    if (PWar_widget_id(ar_pw,K_PW_2NDARY_BOX) &&
		XtIsRealized(PWar_widget_id(ar_pw,K_PW_2NDARY_BOX)))
	    {
		pw_make_resources_match_widgets(ar_pw,
						K_PW_2NDARY_RESOURCES_START,
						K_PW_2NDARY_RESOURCES_FINISH);

		if (!pw_validate_cs_widget_based_resources(ar_pw,
							   K_PW_2NDARY_RESOURCES_START,
							   K_PW_2NDARY_RESOURCES_FINISH))
		    break;
	    }
		
	    /************************************************************/
	    /*								*/
	    /* Update and validate the cs-widget-based resources	*/
	    /*								*/
	    /************************************************************/
	    pw_make_resources_match_widgets(ar_pw,
					    K_PW_PRIMARY_RESOURCES_START,
					    K_PW_PRIMARY_RESOURCES_FINISH);	    
	    
	    if (!pw_validate_cs_widget_based_resources(ar_pw,
							 K_PW_PRIMARY_RESOURCES_START,
							 K_PW_PRIMARY_RESOURCES_FINISH))
		    break;

	    /************************************************************/
	    /*								*/
	    /* Signal that it is necessary to save the primary variables*/
	    /* once the interface is brought back up.			*/
	    /*								*/
	    /************************************************************/
	    PWl_primary_saved(ar_pw) = 0;
	    
	    /************************************************************/
	    /*								*/
	    /* Unmanage the widgets if they so wish			*/
	    /*								*/
	    /************************************************************/
	    if (PWar_widget_id(ar_pw,K_PW_2NDARY_BOX))
		XtUnmanageChild(PWar_widget_id(ar_pw,K_PW_2NDARY_BOX));
	    if (PWar_widget_id(ar_pw,K_PW_MSG))
		XtUnmanageChild(PWar_widget_id(ar_pw,K_PW_MSG));
	    if (PWar_widget_id(ar_pw,K_PW_HELP_WIDGET))
		XtUnmanageChild(PWar_widget_id(ar_pw,K_PW_HELP_WIDGET));
	    if (PWr_boolean_res(ar_pw,K_PW_UNMANAGE_ON_OK_MAP).b_current_value)
 		XtUnmanageChild((Widget)ar_pw);
	    
	    /************************************************************/
	    /*								*/
	    /* Call the OK callback routine				*/
	    /*								*/
	    /************************************************************/
	    ar_callback_data->reason = XmCR_OK;
	    XtCallCallbacks((Widget)ar_pw,
			    XmNokCallback,
			    ar_callback_data);
	    break;
	} /* case K_PW_OK */
	
	/****************************************************************/
	/*								*/
	/* Primary box CANCEL button					*/
	/*								*/
	/****************************************************************/
	case K_PW_CANCEL:
	{
	    if (PWar_widget_id(ar_pw,K_PW_2NDARY_BOX))
		XtUnmanageChild(PWar_widget_id(ar_pw,K_PW_2NDARY_BOX));
	    if (PWar_widget_id(ar_pw,K_PW_MSG))
		XtUnmanageChild(PWar_widget_id(ar_pw,K_PW_MSG));
	    if (PWar_widget_id(ar_pw,K_PW_HELP_WIDGET))
		XtUnmanageChild(PWar_widget_id(ar_pw,K_PW_HELP_WIDGET));
	    if (PWr_boolean_res(ar_pw,K_PW_UNMANAGE_ON_CANCEL_MAP).b_current_value)
		XtUnmanageChild((Widget)ar_pw);
		
	    /************************************************************/
	    /*								*/
	    /* Restore the interface to what it used to be.		*/
	    /*								*/
	    /************************************************************/
	    pw_restore_values(ar_pw,
			      K_PW_PRIMARY_RESOURCES_START,
			      K_PW_PRIMARY_RESOURCES_FINISH);

	    /************************************************************/
	    /*								*/
	    /* Call the cancel callback routine				*/
	    /*								*/
	    /************************************************************/
	    ar_callback_data->reason = XmCR_CANCEL;
	    XtCallCallbacks((Widget)ar_pw,
			    XmNcancelCallback,
			    ar_callback_data);
	    break;
	} /* case K_PW_CANCEL */
	
	/****************************************************************/
	/*								*/
	/* Primary box OPTIONS... button				*/
	/*								*/
	/****************************************************************/
	case K_PW_OPTIONS:
	{
	    unsigned long int	l_mask = CWStackMode;
	    XWindowChanges	r_value;
	    Arg			ar_al[2];
	    unsigned long int	l_ac = 0;
	    Position		l_current_x,l_current_y;
	    
	    /************************************************************/
	    /*								*/
	    /* If the 2ndary box has not been created, create it.	*/
	    /*								*/
	    /************************************************************/
	    if (!PWar_widget_id(ar_pw,K_PW_2NDARY_BOX))
	    {
		/********************************************************/
		/*							*/
		/* Turn on the wait cursor.				*/
		/*							*/
		/********************************************************/
		pw_turn_on_wait_cursor(ar_pw,PWar_widget_id(ar_pw,K_PW_PRIMARY_BOX));

		/********************************************************/
		/*							*/
		/* Fetch the 2ndary box.				*/
		/*							*/
		/********************************************************/
		pw_fetch_2ndary_box(ar_pw);
            }

            /************************************************************/
	    /*								*/
	    /* Save the state of the 2ndary variables.			*/
	    /*								*/
	    /************************************************************/
	    pw_save_values(ar_pw,
			   K_PW_2NDARY_RESOURCES_START,
			   K_PW_2NDARY_RESOURCES_FINISH);
	    
	    /************************************************************/
	    /*								*/
	    /* Make the widgets match the values of the resources.	*/
	    /* Also, make sure the dialog box fits the screen.		*/
	    /*								*/
	    /* [[[I wanted to put this above where the box was created,	*/
	    /* but the toolkit kept on bombing out on me; so I guess 	*/
	    /* making the widgets match the resources will have to wait	*/
	    /* until here.]]]						*/
	    /*								*/
	    /************************************************************/
            if (PWl_2ndary_just_created(ar_pw))
            {
	        XtRealizeWidget(PWar_widget_id(ar_pw,K_PW_2NDARY_BOX));

		/********************************************************/
		/*							*/
		/* Force the x,y coordinates of the 2ndary form to match*/
		/* the x,y coordinates of the print widget itself.	*/
		/* Also, resize the scolled window to fit the screen.	*/
		/*							*/
		/*	[[[This has been moved so it only happens	*/
		/*	on the create.  Something wierd happens when	*/
		/*	we try to do this every time.]]]		*/
		/*							*/
		/********************************************************/
    	    	pw_handle_form_dialog(ar_pw,
                                      PWar_widget_id(ar_pw,K_PW_2NDARY_FORM),
                                      PWar_widget_id(ar_pw,K_PW_2NDARY_WINDOW));

		l_ac = 0;
		XtSetArg(ar_al[l_ac], XmNx, &l_current_x);	l_ac++;
		XtSetArg(ar_al[l_ac], XmNy, &l_current_y);	l_ac++;
		XtGetValues((Widget)ar_pw,ar_al,l_ac);
		
		l_ac = 0;
		XtSetArg(ar_al[l_ac], XmNx, l_current_x);	l_ac++;
		XtSetArg(ar_al[l_ac], XmNy, l_current_y);	l_ac++;
                XtSetValues(PWar_widget_id(ar_pw,K_PW_2NDARY_BOX),
			    ar_al,
			    l_ac);    

		/********************************************************/
		/*							*/
		/* Make the widgets match the resources.		*/
		/*							*/
		/********************************************************/
		pw_make_widgets_match_resources(ar_pw,
						K_PW_2NDARY_RESOURCES_START,
						K_PW_2NDARY_RESOURCES_FINISH);

		/********************************************************/
		/* 							*/
		/* Suppress the desired options.			*/
		/*							*/
		/********************************************************/
		pw_handle_suppress_options_mask(ar_pw);

		/********************************************************/
		/*							*/
		/* Turn off the wait cursor.				*/
		/*							*/
		/********************************************************/
                pw_turn_off_wait_cursor(ar_pw,PWar_widget_id(ar_pw,K_PW_PRIMARY_BOX));

		PWl_2ndary_just_created(ar_pw) = FALSE;
            }

	    /************************************************************/
	    /*								*/
	    /* Force the 2ndary form widget to the top of the	 	*/
	    /* stacking order among its siblings.			*/
	    /*								*/
	    /************************************************************/
	    r_value.stack_mode = Above;
	    
	    if (XtIsRealized(PWar_widget_id(ar_pw,K_PW_2NDARY_BOX)))
		XConfigureWindow(XtDisplay(PWar_widget_id(ar_pw,K_PW_2NDARY_BOX)),
				  XtWindow(PWar_widget_id(ar_pw,K_PW_2NDARY_BOX)),
				  l_mask,
				  &r_value);
	    
	    /************************************************************/
	    /*								*/
	    /* Manage the 2ndary form.					*/
	    /*								*/
	    /************************************************************/	    
	    XtManageChild(PWar_widget_id(ar_pw,K_PW_2NDARY_BOX));

	    break;
	} /* case K_PW_OPTIONS */
	
	/****************************************************************/
	/*								*/
	/* 2ndary box OK button						*/
	/*								*/
	/****************************************************************/
      	case K_PW_2NDARY_OK:
	{
	    /************************************************************/
	    /*								*/
	    /* Update and validate the cs-widget-based resources	*/
	    /*								*/
	    /************************************************************/
	    pw_make_resources_match_widgets(ar_pw,
					    K_PW_2NDARY_RESOURCES_START,
					    K_PW_2NDARY_RESOURCES_FINISH);

	    if (!pw_validate_cs_widget_based_resources(ar_pw,
						       K_PW_2NDARY_RESOURCES_START,
						       K_PW_2NDARY_RESOURCES_FINISH))
		    break;

	    XtUnmanageChild(PWar_widget_id(ar_pw,K_PW_2NDARY_BOX));

	    break;
	} /* case K_PW_2NDARY_OK */
	
	/****************************************************************/
	/*								*/
	/* 2ndary box CANCEL button					*/
	/*								*/
	/****************************************************************/
      	case K_PW_2NDARY_CANCEL:
	{
	    XtUnmanageChild(PWar_widget_id(ar_pw,K_PW_2NDARY_BOX));

	    /************************************************************/
	    /*								*/
	    /* Restore the interface to what it used to be.		*/
	    /*								*/
	    /************************************************************/
	    pw_restore_values(ar_pw,
			      K_PW_2NDARY_RESOURCES_START,
			      K_PW_2NDARY_RESOURCES_FINISH);

	    break;
	} /* case K_PW_CANCEL */
	
	default:	break;
       
    } /* switch (*al_tag) */

    return TRUE;
    
} /* pw_activate_proc */


/************************************************************************/
/*									*/
/* pw_create_proc							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine is called once for every widget of interest in the */
/*	print widget interface at the tim eit is initially created by   */
/*	the resource manager (if a create callback was specified). 	*/
/*									*/
/*	Note that not every widget makes this callback; only those of  	*/
/*	interest make this callback.  For example, it is not necessary  */
/*	to save the widget id of some option menu popups.  These ID's	*/
/*	are handed to us in the activation callback when they are	*/
/*	selected.							*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	w		Widget (which was created)			*/
/*									*/
/*	tag		Widget (index into ra_widget_ids array)		*/
/*									*/
/*	callback_data	Callback data for creation.			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	unsigned int		1					*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static unsigned int pw_create_proc(ar_w,al_tag,ar_callback_data)
    Widget		ar_w;
    int			*al_tag;
    XmAnyCallbackStruct	*ar_callback_data;
{
    DXmPrintWgt		ar_pw;

    /********************************************************************/
    /*									*/
    /* Find the print widget (it is guaranteed to be a parent of this	*/
    /* widget).								*/
    /*									*/
    /********************************************************************/
    ar_pw = (DXmPrintWgt) ar_w;
    while (!XtIsSubclass((Widget)ar_pw,dxmPrintWgtWidgetClass))   
	ar_pw = (DXmPrintWgt)XtParent(ar_pw);

#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_create_proc: %d\n",*al_tag);
#endif

    /********************************************************************/
    /*									*/
    /* Save the id of the widget for future use.			*/
    /*									*/
    /********************************************************************/
    PWar_widget_id(ar_pw,*al_tag) = ar_w;

    /*
     *   add drag and drop to the printer list widget
     */


    if (*al_tag == K_PW_PRINTER)
    {
         pw_register_drop_site(ar_w);
    }
    
    return 1;
    
} /* pw_create_proc */


/************************************************************************/
/*									*/
/* pw_pulldown_menu_create_proc						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine is called for when an option menu in the interface */
/*	is created.  It is used for fetching and creating the push	*/
/*	buttons for the pulldown menu.					*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	w		Widget (which was created)			*/
/*									*/
/*	tag		Widget (index into ra_widget_ids array)		*/
/*									*/
/*	callback_data	Callback data for creation.			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	unsigned int		1					*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static unsigned int pw_pulldown_menu_create_proc(ar_w,al_tag,ar_callback_data)
    Widget		ar_w;
    int			*al_tag;
    XmAnyCallbackStruct	*ar_callback_data;
{
    DXmPrintWgt		ar_pw;
    char		**aat_objects_list;
    XmStringTable	ar_ui_list = NULL;
    XmString		ar_help_key = NULL;
    MrmCode	        r_literal_type;
    int			l_i;
    Arg			ar_al[3];
    MrmType	        ar_dummy_class;
    int			l_resource_index;
    int			l_widget_index;
    
    /********************************************************************/
    /*									*/
    /* Find the print widget (it is guaranteed to be a parent of this	*/
    /* widget).								*/
    /*									*/
    /********************************************************************/
    ar_pw = (DXmPrintWgt) ar_w;
    while (!XtIsSubclass((Widget)ar_pw,dxmPrintWgtWidgetClass))   
	ar_pw = (DXmPrintWgt)XtParent(ar_pw);

#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_pulldown_menu_create_proc: %d\n",*al_tag);
#endif

    /********************************************************************/
    /*									*/
    /* Save the id of the widget for future use.			*/
    /*									*/
    /********************************************************************/
    l_resource_index = pw_ar_pulldown_menu_data_table[*al_tag].l_resource_index;
    l_widget_index = pw_ar_resource_control[l_resource_index].l_widget_index;

    PWar_widget_id(ar_pw,l_widget_index) = ar_w;
    
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_pulldown_menu_create_proc: %d --> resource index = %d --> widget index = %d\n",
	       *al_tag,
	       l_resource_index,
	       l_widget_index);
#endif

    /********************************************************************/
    /*									*/
    /* Now create the push buttons for this particular pulldown menu.	*/	
    /* 									*/
    /* The method for doing this is as follows:				*/
    /*									*/
    /* 	1) Fetch the 3 element asciz table for this pulldown.		*/
    /*									*/
    /*	   The first element points to the list of UI strings.		*/
    /*	   The second element points to the list of OS strings.		*/
    /*	   The third element points to the help key.			*/
    /*									*/
    /*	2) Save the OS strings in the allocated structure.		*/
    /*									*/
    /*	3) Set up the callback data structures for activate and help.	*/
    /*									*/
    /*	4) Loop through each of the UI strings and create a pushbutton.	*/
    /*									*/
    /********************************************************************/
    /********************************************************************/
    /*									*/
    /* Fetch the object list definition.				*/
    /*									*/
    /********************************************************************/
    if (MrmFetchLiteral(PWar_mrm_hierarchy(ar_pw),
			pw_ar_pulldown_menu_data_table[*al_tag].at_objects_list_name,	/* index (name) of literal */
			NULL,				/* display */
			(XtPointer *) &aat_objects_list,	/* return value */
			&r_literal_type)		/* return type */
	!= MrmSUCCESS)
    {
	String params = pw_ar_pulldown_menu_data_table[*al_tag].at_objects_list_name;
	Cardinal num_params = 1;
	
	DXMAPPWARNING(XtWidgetToApplicationContext((Widget)ar_pw),
		      PWMSGNAME2,
		      PWGTNOMRMFETCH,
		      &params,&num_params);

	return MrmFAILURE;
    }
    
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
        for (l_i = 0; l_i < 3; l_i++)
	    printf("object list element: %s\n",aat_objects_list[l_i]);
#endif

    /********************************************************************/
    /*									*/
    /* Fetch the OS list.						*/
    /*									*/
    /********************************************************************/
    pw_fetch_pulldown_menu_os_list(ar_pw,aat_objects_list[1],*al_tag);    

    /********************************************************************/
    /*									*/
    /* Fetch the help key.						*/
    /*									*/
    /********************************************************************/
    if (MrmFetchLiteral(PWar_mrm_hierarchy(ar_pw),
			aat_objects_list[2],		/* index (name) of literal */
			NULL,				/* display */
			(XtPointer *) &ar_help_key,	/* return value */
			&r_literal_type)		/* return type */
	!= MrmSUCCESS)
    {
	String params = aat_objects_list[2];
	Cardinal num_params = 1;
	
	DXMAPPWARNING(XtWidgetToApplicationContext((Widget)ar_pw),
		      PWMSGNAME2,
		      PWGTNOMRMFETCH,
		      &params,&num_params);

	return MrmFAILURE;
    }

    /********************************************************************/
    /*									*/
    /* Fetch the UI list.						*/
    /*									*/
    /********************************************************************/
    if (MrmFetchLiteral(PWar_mrm_hierarchy(ar_pw),
			aat_objects_list[0],		/* index (name) of literal */
			NULL,				/* display */
			(XtPointer *) &ar_ui_list,	/* return value */
			&r_literal_type)		/* return type */
	!= MrmSUCCESS)
    {
	String params = aat_objects_list[0];
	Cardinal num_params = 1;
	
	DXMAPPWARNING(XtWidgetToApplicationContext((Widget)ar_pw),
		      PWMSGNAME2,
		      PWGTNOMRMFETCH,
		      &params,&num_params);

	return MrmFAILURE;
    }

    /********************************************************************/
    /*									*/
    /* Now create the push buttons as children of the pulldown.		*/
    /*									*/
    /********************************************************************/
    for (l_i = 0; ar_ui_list[l_i]; l_i++)
    {
	XtSetArg(ar_al[0],
		 K_PW_PROTO_OPTIONMENU_ACTIVATE_TAG_NAME,
		 l_widget_index + l_i + 1 +
		 (l_resource_index << 16));
	XtSetArg(ar_al[1],
		 K_PW_PROTO_OPTIONMENU_LABEL_NAME,
		 ar_ui_list[l_i]);
	XtSetArg(ar_al[2],
		 K_PW_PROTO_OPTIONMENU_HELP_TAG_NAME,
		 ar_help_key);

	MrmRegisterNames((MrmRegisterArglist) ar_al,(MrmCount)3);

	if (MrmFetchWidget (PWar_mrm_hierarchy(ar_pw),		/* MRM heirarchy	*/
			    K_PW_PROTO_OPTIONMENU_PB_NAME,	/* Name of widget	*/
			    ar_w,				/* Parent		*/
			    &PWar_widget_id(ar_pw,
					    l_widget_index + l_i + 1), 	/* Push Button	*/
			    &ar_dummy_class) 			/* Class		*/
	!= MrmSUCCESS)
	{
	    String params = K_PW_PROTO_OPTIONMENU_PB_NAME;
	    Cardinal num_params = 1;
	
	    DXMAPPWARNING(XtWidgetToApplicationContext((Widget)ar_pw),
			  PWMSGNAME2,
			  PWGTNOMRMFETCH,
			  &params,&num_params);

	    return (0);
	}

	XtManageChild(PWar_widget_id(ar_pw,l_widget_index + l_i + 1));
    }
    
    /********************************************************************/
    /*									*/
    /* Free up the memory used.						*/
    /*									*/
    /********************************************************************/
#ifndef NO_FREE_MRM
    XtFree((char *)aat_objects_list);
    XtFree((char *)ar_ui_list);
    XtFree((char *)ar_help_key);
#endif
	
    return 1;
    
} /* pw_pulldown_menu_create_proc */

/* 
 * These will probably become part of a public include file eventually,
 * remove these definitions when they do.
 */

#define Bkr_Success                 0
#define Bkr_Busy                    1
#define Bkr_Send_Event_Failure      2
#define Bkr_Startup_Failure         3
#define Bkr_Create_Client_Failure   4
#define Bkr_Invalid_Object          5
#define Bkr_Get_Data_Failure        6
#define Bkr_Bad_Filename            7

/* 
 * This routine is called whenever one of the HyperHelp routines returns
 * a failure status.
 */

static void pw_help_error (problem_string, status)
    char    *problem_string;               
    int     status;

{
        switch (status)
        {
            case Bkr_Send_Event_Failure:
		printf("HyperHelp error: Failure sending event to bookreader, status code %d\n", status);
		break;

            case Bkr_Startup_Failure:
		printf("HyperHelp error: Failed to start bookreader, status code %d\n", status);
		break;

            case Bkr_Create_Client_Failure:
		printf("HyperHelp error: Can't create client context, status code %d\n", status);
		break;

            case Bkr_Invalid_Object:
		printf("HyperHelp error: Invalid object passed to help routines, status code %d\n", status);
		break;

            case Bkr_Get_Data_Failure:
		printf("HyperHelp error: Can't retrieve information from help context, status code %d\n", status);
		break;

            case Bkr_Bad_Filename:
		printf("HyperHelp error: Bad filename passed to DXmHelpSystemOpen, status code %d\n", status);
		break;
	}
}


/************************************************************************/
/*									*/
/* pw_help_proc								*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine is called whenever help is requested on any object	*/
/*	within the print widget display.				*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	w		Widget (which for getting help on)		*/
/*									*/
/*	tag		Widget (index into ra_widget_ids array)		*/
/*									*/
/*	callback_data	Callback data for help.				*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	unsigned int		1					*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/

#ifndef WIN32
/* Remove the above conditional when HyperHelp becomes available on Unix */

static unsigned int pw_help_proc(ar_w,al_tag,ar_callback_data)
    Widget		ar_w;
    int			*al_tag;
    XmAnyCallbackStruct	*ar_callback_data;
{
    DXmPrintWgt		ar_pw;
    Arg			ar_al[2];
    int			l_ac = 0;

    /********************************************************************/
    /*									*/
    /* Find the print widget (it is guaranteed to be a parent of this	*/
    /* widget).								*/
    /*									*/
    /********************************************************************/
    ar_pw = (DXmPrintWgt) ar_w;
    while (!XtIsSubclass((Widget)ar_pw,dxmPrintWgtWidgetClass))   
	ar_pw = (DXmPrintWgt)XtParent(ar_pw);

#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_help_proc: %d\n",al_tag);
#endif

    /* 
     * Although the print widget supports context sensitive help on
     * several of it's components, the OSF style guide recommends that 
     * invoking context sensitive help anywhere in a secondary dialog
     * box bring up a help screen for the dialog as a whole, after which
     * the user can access individual component help manually.
     * This means I pass "overview" in the call to DXmHelpSystemDisplay
     * instead of an ASCII string extracted from al_tag.
     */ 

    if (help_context == (Opaque) NULL)
	DXmHelpSystemOpen (&help_context, ar_pw, 
#ifdef VMS
	     "DECW$DXMPRINTWGT_HELP",
#else
	     "DXmPrintWgt_Help",
#endif
             pw_help_error, (Opaque) NULL);    

    DXmHelpSystemDisplay (help_context, 
#ifdef VMS
	     "DECW$DXMPRINTWGT_HELP",
#else
	     "DXmPrintWgt_Help",
#endif
	    "topic", "overview", pw_help_error, (Opaque) NULL);
    
    return TRUE;
    
} /* pw_help_proc */

#else

static unsigned int pw_help_proc(ar_w,al_tag,ar_callback_data)
    Widget		ar_w;
    int			*al_tag;
    XmAnyCallbackStruct	*ar_callback_data;
{
    DXmPrintWgt		ar_pw;
    Arg			ar_al[2];
    int			l_ac = 0;

    /********************************************************************/
    /*									*/
    /* Find the print widget (it is guaranteed to be a parent of this	*/
    /* widget).								*/
    /*									*/
    /********************************************************************/
    ar_pw = (DXmPrintWgt) ar_w;
    while (!XtIsSubclass((Widget)ar_pw,dxmPrintWgtWidgetClass))   
	ar_pw = (DXmPrintWgt)XtParent(ar_pw);

#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_help_proc: %d\n",al_tag);
#endif
    
    /********************************************************************/
    /*									*/
    /* Turn on the wait cursor.						*/
    /*									*/
    /********************************************************************/
    pw_turn_on_wait_cursor(ar_pw,PWar_widget_id(ar_pw,K_PW_PRIMARY_BOX));
    pw_turn_on_wait_cursor(ar_pw,PWar_widget_id(ar_pw,K_PW_2NDARY_BOX));
		
    /********************************************************************/
    /*									*/
    /* If the help box has not been created, create it.			*/
    /*									*/
    /********************************************************************/
    if (!PWar_widget_id(ar_pw,K_PW_HELP_WIDGET))
    {
	/****************************************************************/
	/*								*/
	/* Fetch the help box.						*/
	/*								*/
	/****************************************************************/
	MrmType	ar_dummy_class;

	XtSetArg(ar_al[l_ac],
		 DXmNlibrarySpec,
		 PWr_cs_res(ar_pw,K_PW_LIBRARY_SPEC_MAP).ar_current_cs); l_ac++;

	XtSetArg(ar_al[l_ac],
		 DXmNfirstTopic,
		 al_tag); l_ac++;
    
	if (MrmFetchWidgetOverride (PWar_mrm_hierarchy(ar_pw),	/* MRM heirarchy	*/
			    	    K_PW_HELP_BOX_NAME,		/* Name of widget	*/
			            (Widget) ar_pw,			/* Parent		*/
				    NULL,			/* Override name	*/
				    ar_al,			/* Override args	*/
				    l_ac,			/* Number of arguments	*/
				    &PWar_widget_id(ar_pw,
						    K_PW_HELP_WIDGET),	/* help box		*/
				    &ar_dummy_class) 		/* Class		*/
	    != MrmSUCCESS)
	{
	    String params = K_PW_HELP_BOX_NAME;
	    Cardinal num_params = 1;
	
	    DXMAPPWARNING(XtWidgetToApplicationContext((Widget)ar_pw),
			  PWMSGNAME2,
			  PWGTNOMRMFETCH,
			  &params,&num_params);
	}		
    }
    else
    {
	/****************************************************************/
	/*                                                          	*/
	/* Set the topic to be displayed				*/
	/*                                                              */
	/****************************************************************/
	XtSetArg(ar_al[l_ac],
		 DXmNfirstTopic,
		 al_tag); l_ac++;
	XtSetValues(PWar_widget_id(ar_pw,K_PW_HELP_WIDGET),
		    ar_al,
		    l_ac);
    }

    /********************************************************************/
    /*								        */
    /* Manage the help box if necessary.				*/
    /*								        */
    /********************************************************************/
    if (!XtIsManaged(PWar_widget_id(ar_pw,K_PW_HELP_WIDGET)))
	XtManageChild(PWar_widget_id(ar_pw,K_PW_HELP_WIDGET));

    /********************************************************************/
    /*									*/
    /* Turn off the wait cursor.					*/
    /*									*/
    /********************************************************************/
    pw_turn_off_wait_cursor(ar_pw,PWar_widget_id(ar_pw,K_PW_PRIMARY_BOX));
    pw_turn_off_wait_cursor(ar_pw,PWar_widget_id(ar_pw,K_PW_2NDARY_BOX));
		
    return TRUE;
    
} /* pw_help_proc */
#endif


/************************************************************************/
/*									*/
/* pw_resource_activate_proc						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine is called whenever a widget-based resource is	*/
/*	activated.							*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	w		Widget 						*/
/*									*/
/*	tag		This contains two numbers.  The first (which is */
/*			shifted left 16 bits) is the controlling widget */
/*			(e.g. an option menu) widget, and the second is */
/*			the actual widget index (e.g. a pushbutton	*/
/*			inside an option menu.				*/
/*									*/
/*	callback_data	Callback data for help.				*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	unsigned int		1					*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static unsigned int pw_resource_activate_proc(ar_w,al_tag,ar_callback_data)
    Widget		ar_w;
    int			*al_tag;
    XmAnyCallbackStruct	*ar_callback_data;
{
    DXmPrintWgt		ar_pw;
    unsigned long int	l_controller;
    unsigned long int	l_controllee;
    
    /********************************************************************/
    /*									*/
    /* Find the print widget (it is guaranteed to be a parent of this	*/
    /* widget).								*/
    /*									*/
    /********************************************************************/
    ar_pw = (DXmPrintWgt) ar_w;
    while (!XtIsSubclass((Widget)ar_pw,dxmPrintWgtWidgetClass))   
	ar_pw = (DXmPrintWgt)XtParent(ar_pw);

    /********************************************************************/
    /*									*/
    /* Convert the *al_tag into the control widget map (i.e. which 	*/
    /* widget controls everyting, such as an option menu) and the widget*/
    /* map (i.e. what widget was actually activated, such as a push	*/
    /* button inside of an option menu.					*/
    /*									*/
    /********************************************************************/
    l_controller = (*al_tag) >> 16;
    l_controllee = *al_tag - (l_controller << 16);
    
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_resource_activate_proc: tag %d, controller %d, controllee: %d\n",
	       *al_tag,
	       l_controller,
	       l_controllee);
#endif

    /********************************************************************/
    /*									*/
    /* Now call the approriate activate routine.		 	*/
    /*									*/
    /********************************************************************/
    if (pw_ar_resource_control[l_controller].widget_activate_proc)
	    (*(pw_ar_resource_control[l_controller].widget_activate_proc))(ar_pw,
									   l_controller,
									   l_controllee,
									   ar_callback_data);

    return TRUE;
    
} /* pw_resource_activate_proc */


/************************************************************************/
/*									*/
/* pw_pulldown_menu_activate_proc					*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine is called whenever a optionmenu widget-based	*/
/*	resource is activated.						*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	w		Widget 						*/
/*									*/
/*	tag		This contains two numbers.  The first (which is */
/*			shifted left 16 bits) is the controlling widget */
/*			(e.g. an option menu) widget, and the second is */
/*			the actual widget index (e.g. a pushbutton	*/
/*			inside an option menu.				*/
/*									*/
/*	callback_data	Callback data for help.				*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	unsigned int		1					*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static unsigned int pw_pulldown_menu_activate_proc(ar_w,l_tag,ar_callback_data)
    Widget		ar_w;
    int			l_tag;
    XmAnyCallbackStruct	*ar_callback_data;
{
    pw_resource_activate_proc(ar_w,&l_tag,ar_callback_data);
    
    return TRUE;

} /* pw_pulldown_menu_activate_proc */


/************************************************************************/
/*									*/
/* pw_required_arguments						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine returns TRUE if the argument list contains the	*/
/*	arguments required to create the print widget.			*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	args		Argument list.					*/
/*									*/
/*	arg_count	Number of entries in args.			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	Boolean								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static Boolean pw_required_arguments (ar_args, l_arg_count)
    ArgList	ar_args;
    int		l_arg_count;
{
    return (TRUE);

} /* pw_required_arguments */


/************************************************************************/
/*									*/
/* pw_copy_memory_on_create						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine copies any memory the user may have given to the	*/
/*	print widget during the create routine.				*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	w		Widget						*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	void								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_copy_memory_on_create(ar_w)
    Widget ar_w;
{
    DXmPrintWgt			ar_pw = (DXmPrintWgt) ar_w;
    pw_ar_res_union		ar_resources;
    unsigned long int		l_i,l_j;
    int				*al_copied_int_list;
    XmString			ar_copied_cs;
    XmStringTable		r_copied_cs_table;
    int				l_choice_index;
    
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_copy_memory_on_create\n");
#endif

    /********************************************************************/
    /*									*/
    /*	Set the known format list counts to be the same as the UI list	*/
    /*	count.  The reason for doing so is that the UI list count is 	*/
    /*	the resource seen to the outside world, whereas the rest are	*/
    /*	strictly internal.						*/
    /*									*/
    /********************************************************************/
    PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_OS_LIST_MAP).l_cs_count = 
	PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).l_cs_count;
	
    PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_VARIABLE_LIST_MAP).l_cs_count = 
	PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).l_cs_count;
	
    PWr_int_list_res(ar_pw,K_PW_KNOWN_FORMAT_GUESSER_LIST_MAP).l_int_count = 
	PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).l_cs_count;

    /********************************************************************/
    /*									*/
    /* Go through each of the resources and copy the associated memory.	*/
    /*									*/
    /********************************************************************/
    ar_resources = PWar_resources(ar_pw);
    
    for (l_i = 0; l_i < K_PW_NUM_RESOURCES; l_i++)
    {
	switch (pw_ar_resource_control[l_i].l_type)
	{
	    case K_PW_RESOURCE_TYPE_INT_LIST:
	    {
		if (ar_resources[l_i].r_int_list_res.al_int_list) 
		{			    
		    al_copied_int_list = 
			(int *) XtMalloc (sizeof(int) *
					  ar_resources[l_i].r_int_list_res.l_int_count);
		
		    for (l_j = 0; l_j < ar_resources[l_i].r_int_list_res.l_int_count; l_j++)
			al_copied_int_list[l_j] = 
			    ar_resources[l_i].r_int_list_res.al_int_list[l_j];

		    ar_resources[l_i].r_int_list_res.al_int_list = al_copied_int_list;
		}
		
		break;
	    }
	    
	    case K_PW_RESOURCE_TYPE_CS:
	    {
		if ((ar_resources[l_i].r_cs_res.ar_current_cs) &&
		    ((int) ar_resources[l_i].r_cs_res.ar_current_cs != -1))
		{
		    ar_copied_cs = (XmString) XmStringCopy(ar_resources[l_i].r_cs_res.ar_current_cs);
		    ar_resources[l_i].r_cs_res.ar_current_cs = ar_copied_cs;
		}

		break;
	    }
	    
	    case K_PW_RESOURCE_TYPE_CS_LIST:
	    {
		if (ar_resources[l_i].r_cs_list_res.ar_cs_list)
		{
		    r_copied_cs_table = 
			(XmStringTable) XtMalloc(sizeof(XmString) *
						 ar_resources[l_i].r_cs_list_res.l_cs_count);
		    
		    for (l_j = 0; l_j < ar_resources[l_i].r_cs_list_res.l_cs_count; l_j++)
			r_copied_cs_table[l_j] = 
			    XmStringCopy(ar_resources[l_i].r_cs_list_res.ar_cs_list[l_j]);

		    ar_resources[l_i].r_cs_list_res.ar_cs_list = r_copied_cs_table;
		}

		break;
	    }

	    default:	break;

	} /* switch */	
    } /* for */
    
} /* pw_copy_memory_on_create */


/************************************************************************/
/*									*/
/* pw_set_value_boolean							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine sets the value of a widget based boolean		*/
/*	resource.							*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	old_pw		Original Widget					*/
/*									*/
/*	new_pw		Contains what is requested			*/
/*									*/
/*      resource_index	The index of the resource     			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_set_value_boolean(ar_old_pw,ar_new_pw,l_resource_index)
    DXmPrintWgt		ar_old_pw;
    DXmPrintWgt		ar_new_pw;
    int			l_resource_index;
{   
    pw_ar_res_union	ar_old_resources;
    pw_ar_res_union	ar_new_resources;

    ar_old_resources = PWar_resources(ar_old_pw);
    ar_new_resources = PWar_resources(ar_new_pw);

    if (ar_new_resources[l_resource_index].r_boolean_res.b_current_value !=
	ar_old_resources[l_resource_index].r_boolean_res.b_current_value)
    {	    
#ifdef PRINTWGT_DEBUG
	if (PWr_debug(ar_new_pw).l_current_value)
	    printf("pw_set_value_int %d value changed from %d to %d\n",
		   l_resource_index,
		   ar_old_resources[l_resource_index].r_boolean_res.b_current_value,
		   ar_new_resources[l_resource_index].r_boolean_res.b_current_value);
#endif

	/****************************************************************/
	/* 								*/
	/* Set the value in the widget.					*/
	/* 								*/
	/****************************************************************/
	PW_SET_TOGGLE_WIDGET_FROM_RESOURCE(ar_new_pw,
					   l_resource_index);

    } /* if */

} /* pw_set_value_boolean */
    

/************************************************************************/
/*									*/
/* pw_save_value_boolean						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine saves the value of a widget based resource.	*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/*      resource_index	The index of the resource			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_save_value_boolean(ar_pw,l_resource_index)
    DXmPrintWgt		ar_pw;
    int			l_resource_index;
{   
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_save_value_boolean %d\n",l_resource_index);
#endif

    /********************************************************************/
    /* 									*/
    /* Save value of the Boolean resource.				*/
    /* 									*/
    /********************************************************************/
    PWr_boolean_res(ar_pw,l_resource_index).b_previous_value =
	PWr_boolean_res(ar_pw,l_resource_index).b_current_value;

} /* pw_save_value_boolean */
    

/************************************************************************/
/*									*/
/* pw_restore_value_boolean						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine restores the value of a widget based boolean	*/
/*	resource.							*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/*      resource_index	The index of the resource			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_restore_value_boolean(ar_pw,l_resource_index)
    DXmPrintWgt		ar_pw;
    int			l_resource_index;
{   
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_restore_boolean %d\n",l_resource_index);
#endif

    /********************************************************************/
    /* 									*/
    /* Retore the value and set the value in the widget.		*/
    /* 									*/
    /********************************************************************/
    PWr_boolean_res(ar_pw,l_resource_index).b_current_value =
	PWr_boolean_res(ar_pw,l_resource_index).b_previous_value;

    /********************************************************************/
    /* 									*/
    /* Set the value in the widget.					*/
    /* 									*/
    /********************************************************************/
    PW_SET_TOGGLE_WIDGET_FROM_RESOURCE(ar_pw,
				       l_resource_index);

} /* pw_restore_value_boolean */
    

/************************************************************************/
/*									*/
/* pw_in_bounds								*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine checks to see if an integer value is in bounds.	*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/*	value		Integer Value					*/
/*									*/
/*      validation	Validation Value Structure			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	Boolean								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static Boolean pw_in_bounds(ar_pw,l_value,ar_validation_value)
    DXmPrintWgt			ar_pw;
    int				l_value;
    pw_ar_validate_value_struct	ar_validation_value;
{
    if (ar_validation_value->l_mask & K_PW_BOUNDED_MIN)
    {
	if (l_value < ar_validation_value->l_min)
	    return FALSE;
    }

    if (ar_validation_value->l_mask & K_PW_BOUNDED_MAX)
    {
	if (l_value > ar_validation_value->l_max)
	    return FALSE;
    }
    
    return TRUE;

} /* pw_in_bounds */


/************************************************************************/
/*									*/
/* pw_validate_value_int						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine validates the value of a widget based resource.	*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/*      resource_index	The index of the resource			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static Boolean pw_validate_value_int(ar_pw,l_resource_index)
    DXmPrintWgt		ar_pw;
    int			l_resource_index;
{   
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_validate_value_int %d\n",l_resource_index);
#endif

    /********************************************************************/
    /* 									*/
    /* Validate value of the integer resource.				*/
    /* 									*/
    /********************************************************************/
    return (pw_in_bounds(ar_pw,
			 PWr_int_res(ar_pw,l_resource_index).l_current_value,
			 &pw_ar_validation_value[l_resource_index]));
    
} /* pw_validate_value_int */
    

/************************************************************************/
/*									*/
/* pw_set_value_int							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine sets the value of a widget based integer		*/
/*	resource.							*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	old_pw		Original Widget					*/
/*									*/
/*	new_pw		Contains what is requested			*/
/*									*/
/*      resource_index	The index of the resource     			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_set_value_int(ar_old_pw,ar_new_pw,l_resource_index)
    DXmPrintWgt		ar_old_pw;
    DXmPrintWgt		ar_new_pw;
    int			l_resource_index;
{   
    pw_ar_res_union	ar_old_resources;
    pw_ar_res_union	ar_new_resources;

    ar_old_resources = PWar_resources(ar_old_pw);
    ar_new_resources = PWar_resources(ar_new_pw);

    if (ar_new_resources[l_resource_index].r_int_res.l_current_value !=
	ar_old_resources[l_resource_index].r_int_res.l_current_value)
    {	    
#ifdef PRINTWGT_DEBUG
	if (PWr_debug(ar_new_pw).l_current_value)
	    printf("pw_set_value_int %d value changed from %d to %d\n",
		   l_resource_index,
		   ar_old_resources[l_resource_index].r_int_res.l_current_value,
		   ar_new_resources[l_resource_index].r_int_res.l_current_value);
#endif

	/****************************************************************/
	/*								*/
	/* Make sure the value given is OK.				*/
	/*								*/
	/****************************************************************/
	if (!pw_validate_value_int(ar_new_pw,l_resource_index))
	{
#ifdef PRINTWGT_DEBUG
	    if (PWr_debug(ar_new_pw).l_current_value)
		printf("pw_set_value_int:  Resource out of range.  Not changing old value.\n");
#endif
	    ar_new_resources[l_resource_index].r_int_res.l_current_value = 
		ar_old_resources[l_resource_index].r_int_res.l_current_value;
	}
	
	/****************************************************************/
	/* 								*/
	/* If this is the suppress options resource, we must handle it	*/
	/* specially.							*/
	/* 								*/
	/****************************************************************/
	if (l_resource_index == K_PW_SUPPRESS_OPTIONS_MASK_MAP)
	    pw_handle_suppress_options_mask(ar_new_pw);

	/****************************************************************/
	/* 								*/
	/* Set the value in the text widget.				*/
	/* 								*/
	/****************************************************************/
	switch(pw_ar_resource_control[l_resource_index].l_type)
	{
	    case K_PW_RESOURCE_TYPE_CS_INT:
	    {
		PW_SET_CS_INT_WIDGET_FROM_RESOURCE(ar_new_pw,
						     l_resource_index);
		break;
	    }

	    case K_PW_RESOURCE_TYPE_RADIOBOX:	
	    {
		PW_SET_RADIOBOX_WIDGET_FROM_RESOURCE(ar_new_pw,
						     l_resource_index);
		break;
	    }

	    case K_PW_RESOURCE_TYPE_TOGGLE:
	    {
		PW_SET_TOGGLE_WIDGET_FROM_RESOURCE(ar_new_pw,
						   l_resource_index);
		break;
	    }

	    case K_PW_RESOURCE_TYPE_OPTIONMENU:
	    {
		PW_SET_OPTIONMENU_WIDGET_FROM_RESOURCE(ar_new_pw,
						       l_resource_index);
		break;
	    }

	    default:	break;
	    
	} /* switch */
    } /* if */

} /* pw_set_value_int */
    

/************************************************************************/
/*									*/
/* pw_save_value_int							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine saves the value of a widget based resource.	*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/*      resource_index	The index of the resource			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_save_value_int(ar_pw,l_resource_index)
    DXmPrintWgt		ar_pw;
    int			l_resource_index;
{   
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_save_value_int %d\n",l_resource_index);
#endif

    /********************************************************************/
    /* 									*/
    /* Save value of the integer resource.				*/
    /* 									*/
    /********************************************************************/
    PWr_int_res(ar_pw,l_resource_index).l_previous_value =
	PWr_int_res(ar_pw,l_resource_index).l_current_value;

} /* pw_save_value_int */
    

/************************************************************************/
/*									*/
/* pw_restore_value_int							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine restores the value of a widget based integer	*/
/*	resource.							*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/*      resource_index	The index of the resource			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	TRUE or FALSE							*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_restore_value_int(ar_pw,l_resource_index)
    DXmPrintWgt		ar_pw;
    int			l_resource_index;
{   
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_restore_int %d\n",l_resource_index);
#endif

    /********************************************************************/
    /* 									*/
    /* Retore the value and set the value in the text widget.		*/
    /* 									*/
    /********************************************************************/
    PWr_int_res(ar_pw,l_resource_index).l_current_value =
	PWr_int_res(ar_pw,l_resource_index).l_previous_value;

    /********************************************************************/
    /* 									*/
    /* Set the value in the text widget.				*/
    /* 									*/
    /********************************************************************/
    switch(pw_ar_resource_control[l_resource_index].l_type)
    {
	case K_PW_RESOURCE_TYPE_CS_INT:
	{
	    PW_SET_CS_INT_WIDGET_FROM_RESOURCE(ar_pw,
					       l_resource_index);
	    break;	
	}

	case K_PW_RESOURCE_TYPE_RADIOBOX:	
	{
	    PW_SET_RADIOBOX_WIDGET_FROM_RESOURCE(ar_pw,
						 l_resource_index);
	    break;	
	}

	case K_PW_RESOURCE_TYPE_TOGGLE:
	{
	    PW_SET_TOGGLE_WIDGET_FROM_RESOURCE(ar_pw,
					       l_resource_index);
	    break;
	}

	case K_PW_RESOURCE_TYPE_OPTIONMENU:
	{
	    PW_SET_OPTIONMENU_WIDGET_FROM_RESOURCE(ar_pw,
						   l_resource_index);
	    break;
	}

	default:	break;
	
    } /* switch */

} /* pw_restore_value_int */
    

/************************************************************************/
/*									*/
/* pw_validate_value_text_length					*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine performs validations on the length of a text	*/
/*	widget based resource.  If they are OK, it returns true.	*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/*      resource_index	The index of the resource			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	TRUE or FALSE							*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static Boolean pw_validate_value_text_length(ar_pw,l_resource_index)
    DXmPrintWgt		ar_pw;
    int			l_resource_index;
{  
    char	*r_temp_os_string;
    long	l_size,l_status;
    Boolean	b_ret_status = TRUE;
    
    r_temp_os_string = (char *) DXmCvtCStoOS(PWr_cs_res(ar_pw,l_resource_index).ar_current_cs,
					     &l_size,
					     &l_status);

    /********************************************************************/
    /* 									*/
    /* Validate value of the string resource.				*/
    /* 									*/
    /********************************************************************/
    if (r_temp_os_string)
    {
	b_ret_status = pw_in_bounds(ar_pw,
				    strlen(r_temp_os_string),
				    &pw_ar_validation_value[l_resource_index]);
	XtFree(r_temp_os_string);
    }

    return (b_ret_status);

} /* pw_validate_value_text_length */


/************************************************************************/
/*									*/
/* pw_validate_value_text_contents					*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine performs validations on the contents of a text	*/
/*	widget based resource.  If they are OK, it returns true.  NULL	*/
/*	strings are viewed as OK.					*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/*      resource_index	The index of the resource			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	TRUE or FALSE							*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static Boolean pw_validate_value_text_contents(ar_pw,l_resource_index)
    DXmPrintWgt		ar_pw;
    int			l_resource_index;
{
#if 0
    [[[ This check is not valid for all languages and may return 
        false negatives for those languages which allow other characters.
        Thus, it has been removed.  WDW ]]]
    char	*r_temp_os_string;
    long	l_size,l_status;
    int		l_i;
    Boolean	b_ret_status = TRUE;
    
    r_temp_os_string = (char *)	DXmCvtCStoOS(PWr_cs_res(ar_pw,l_resource_index).ar_current_cs,
					     &l_size,
					     &l_status);

    /********************************************************************/
    /* 									*/
    /* Validate value of the string resource.				*/
    /* 									*/
    /********************************************************************/
    if (r_temp_os_string)
    {
	for (l_i = 0; l_i < strlen(r_temp_os_string); l_i++)
	    if (!(((r_temp_os_string[l_i] >= 'a') && (r_temp_os_string[l_i] <= 'z')) ||
		  ((r_temp_os_string[l_i] >= 'A') && (r_temp_os_string[l_i] <= 'Z')) ||
		  ((r_temp_os_string[l_i] >= '0') && (r_temp_os_string[l_i] <= '9')) ||
#ifdef VMS
		  (r_temp_os_string[l_i] == '$'                           ) ||
#else
		  (r_temp_os_string[l_i] == '.'                           ) ||
		  (r_temp_os_string[l_i] == '/'                           ) ||
#endif	
		  (r_temp_os_string[l_i] == '_'                           ) ||
		  (r_temp_os_string[l_i] == '-'                           )))
		b_ret_status = FALSE;

	XtFree(r_temp_os_string);
    }	

    return (b_ret_status);
#else
    return (TRUE);
#endif

} /* pw_validate_value_text_contents */


/************************************************************************/
/*									*/
/* pw_set_value_cs							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine sets the value of a widget based CS		*/
/*	resource.							*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	old_pw		Original Widget					*/
/*									*/
/*	new_pw		Contains what is requested			*/
/*									*/
/*      resource_index	The index of the resource     			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_set_value_cs(ar_old_pw,ar_new_pw,l_resource_index)
    DXmPrintWgt		ar_old_pw;
    DXmPrintWgt		ar_new_pw;
    int			l_resource_index;
{   
    pw_ar_res_union	ar_old_resources;
    pw_ar_res_union	ar_new_resources;

    ar_old_resources = PWar_resources(ar_old_pw);
    ar_new_resources = PWar_resources(ar_new_pw);

    if ((ar_new_resources[l_resource_index].r_cs_res.ar_current_cs !=
	 ar_old_resources[l_resource_index].r_cs_res.ar_current_cs) ||
	(!pw_xm_string_compare(ar_new_resources[l_resource_index].r_cs_res.ar_current_cs,
			       ar_old_resources[l_resource_index].r_cs_res.ar_current_cs)))
    {	    
#ifdef PRINTWGT_DEBUG
	if (PWr_debug(ar_new_pw).l_current_value)
	    printf("pw_set_value_cs %d value changed\n",l_resource_index);
#endif

	/****************************************************************/
	/*								*/
	/* Validate the value if applicable.				*/
	/*								*/
	/****************************************************************/
	if ((pw_ar_resource_control[l_resource_index].validate_value_proc) &&
	    !(*(pw_ar_resource_control[l_resource_index].validate_value_proc))(ar_new_pw,
									       l_resource_index))
	{
#ifdef PRINTWGT_DEBUG
	    if (PWr_debug(ar_new_pw).l_current_value)
		printf("pw_set_value_cs:  CS resource invalid.  Not changing old value.\n");
#endif
	    ar_new_resources[l_resource_index].r_cs_res.ar_current_cs = 
		ar_old_resources[l_resource_index].r_cs_res.ar_current_cs;
	}
	else
	{
	    /************************************************************/
	    /* 								*/
	    /* Free up the old memory used.				*/
	    /* 								*/
	    /************************************************************/
	    if (ar_old_resources[l_resource_index].r_cs_res.ar_current_cs)
		XmStringFree(ar_old_resources[l_resource_index].r_cs_res.ar_current_cs);

	    /************************************************************/
	    /* 								*/
	    /* Copy the value and set the value in the text widget.	*/
	    /* 								*/
	    /************************************************************/
	    ar_new_resources[l_resource_index].r_cs_res.ar_current_cs = 
		XmStringCopy(ar_new_resources[l_resource_index].r_cs_res.ar_current_cs);

	    PW_SET_CS_WIDGET_FROM_RESOURCE(ar_new_pw,
					   l_resource_index);    
	}
    }

} /* pw_set_value_cs */
    

/************************************************************************/
/*									*/
/* pw_set_value_2ndary_title						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine sets the value of a the 2ndary dialog box.		*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	old_pw		Original Widget					*/
/*									*/
/*	new_pw		Contains what is requested			*/
/*									*/
/*      resource_index	The index of the resource     			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_set_value_2ndary_title(ar_old_pw,ar_new_pw,l_resource_index)
    DXmPrintWgt		ar_old_pw;
    DXmPrintWgt		ar_new_pw;
    int			l_resource_index;
{   
    Arg			ar_al[2];
    pw_ar_res_union	ar_old_resources;
    pw_ar_res_union	ar_new_resources;

    ar_old_resources = PWar_resources(ar_old_pw);
    ar_new_resources = PWar_resources(ar_new_pw);

    if ((ar_new_resources[l_resource_index].r_cs_res.ar_current_cs !=
	 ar_old_resources[l_resource_index].r_cs_res.ar_current_cs) ||
	(!pw_xm_string_compare(ar_new_resources[l_resource_index].r_cs_res.ar_current_cs,
			       ar_old_resources[l_resource_index].r_cs_res.ar_current_cs)))
    {	    
#ifdef PRINTWGT_DEBUG
	if (PWr_debug(ar_new_pw).l_current_value)
	    printf("pw_set_value_2ndary_title %d value changed\n",l_resource_index);
#endif

	/****************************************************************/
	/*								*/
	/* Validate the value if applicable.				*/
	/*								*/
	/****************************************************************/
	if ((pw_ar_resource_control[l_resource_index].validate_value_proc) &&
	    !(*(pw_ar_resource_control[l_resource_index].validate_value_proc))(ar_new_pw,
									       l_resource_index))
	{
#ifdef PRINTWGT_DEBUG
	    if (PWr_debug(ar_new_pw).l_current_value)
		printf("pw_set_value_2ndary_title:  CS resource invalid.  Not changing old value.\n");
#endif
	    ar_new_resources[l_resource_index].r_cs_res.ar_current_cs = 
		ar_old_resources[l_resource_index].r_cs_res.ar_current_cs;
	}
	else
	{
	    /************************************************************/
	    /* 								*/
	    /* Free up the old memory used.				*/
	    /* 								*/
	    /************************************************************/
	    if (ar_old_resources[l_resource_index].r_cs_res.ar_current_cs)
		XmStringFree(ar_old_resources[l_resource_index].r_cs_res.ar_current_cs);

	    /************************************************************/
	    /* 								*/
	    /* Copy the value and set the value in the text widget.	*/
	    /* 								*/
	    /************************************************************/
	    ar_new_resources[l_resource_index].r_cs_res.ar_current_cs = 
		XmStringCopy(ar_new_resources[l_resource_index].r_cs_res.ar_current_cs);

	    if (PWar_widget_id(ar_new_pw,
			       pw_ar_resource_control[l_resource_index].l_widget_index))
	    {
		XtSetArg(ar_al[0],XmNdialogTitle,ar_new_resources[l_resource_index].r_cs_res.ar_current_cs);
		XtSetValues(PWar_widget_id(ar_new_pw,
					   pw_ar_resource_control[l_resource_index].l_widget_index),
			    ar_al,
			    1);
	    }
	}
    }

} /* pw_set_value_2ndary_title */
    

/************************************************************************/
/*									*/
/* pw_save_value_cs							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine saves the value of a widget based cs resource.	*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/*      resource_index	The index of the resource			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_save_value_cs(ar_pw,l_resource_index)
    DXmPrintWgt		ar_pw;
    int			l_resource_index;
{   
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_save_value_cs %d\n",l_resource_index);
#endif

    /********************************************************************/
    /* 									*/
    /* Free up the old memory used.			       	      	*/
    /* 									*/
    /********************************************************************/
    if (PWr_cs_res(ar_pw,l_resource_index).ar_previous_cs)
	XmStringFree(PWr_cs_res(ar_pw,l_resource_index).ar_previous_cs);

    /********************************************************************/
    /* 									*/
    /* Save value of the cs resource.					*/
    /* 									*/
    /********************************************************************/
    PWr_cs_res(ar_pw,l_resource_index).ar_previous_cs =
	XmStringCopy(PWr_cs_res(ar_pw,l_resource_index).ar_current_cs);

} /* pw_save_value_cs */


/************************************************************************/
/*									*/
/* pw_restore_value_cs							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine restores the value of a cs widget based		*/
/*	resource.							*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/*      resource_index	The index of the resource			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_restore_value_cs(ar_pw,l_resource_index)
    DXmPrintWgt		ar_pw;
    int			l_resource_index;
{   
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_restore_value_cs %d\n",l_resource_index);
#endif

    /********************************************************************/
    /* 									*/
    /* Free up the old memory used.			       	      	*/
    /* 									*/
    /********************************************************************/
    if (PWr_cs_res(ar_pw,l_resource_index).ar_current_cs)
	XmStringFree(PWr_cs_res(ar_pw,l_resource_index).ar_current_cs);

    /********************************************************************/
    /* 									*/
    /* Set the value in the cs widget.					*/
    /* 									*/
    /********************************************************************/
    PWr_cs_res(ar_pw,l_resource_index).ar_current_cs =
	XmStringCopy(PWr_cs_res(ar_pw,l_resource_index).ar_previous_cs);

    PW_SET_CS_WIDGET_FROM_RESOURCE(ar_pw,
				   l_resource_index);

} /* pw_restore_value_cs */
    

/************************************************************************/
/*									*/
/* pw_validate_value_print_format_list					*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine validates the value of the print format list	*/
/*	by comparing its contents to those of the known print formats.	*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/*      resource_index	The index of the resource			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static Boolean pw_validate_value_print_format_list(ar_pw,l_resource_index)
    DXmPrintWgt		ar_pw;
    int			l_resource_index;
{   
    int			l_i;
    
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_validate_value_print_format_list %d\n",l_resource_index);
#endif

    /********************************************************************/
    /*									*/
    /* Must have at least one entry.					*/
    /*									*/
    /********************************************************************/
    if (!PWr_cs_list_res(ar_pw,K_PW_PRINT_FORMAT_CONTENTS_MAP).l_cs_count)
	return FALSE;
    
    /********************************************************************/
    /*									*/
    /* Go through each element in the list.  If it can't be found in	*/
    /* known print format lists, then return FALSE.			*/
    /*									*/
    /********************************************************************/
    for (l_i = 0;
	 l_i < PWr_cs_list_res(ar_pw,K_PW_PRINT_FORMAT_CONTENTS_MAP).l_cs_count;
	 l_i++)
	if (pw_get_cs_list_element_index(PWr_cs_list_res(ar_pw,K_PW_PRINT_FORMAT_CONTENTS_MAP).ar_cs_list[l_i],
					 PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).ar_cs_list,
					 PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).l_cs_count) < 0)
	    return FALSE;
    
    /********************************************************************/
    /*									*/
    /* If we've gotten this far, everything must be OK.			*/
    /*									*/
    /********************************************************************/
    return TRUE;
    
} /* pw_validate_value_print_format_list */


/************************************************************************/
/*									*/
/* pw_set_value_print_format_list					*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine sets the value of a compound string		*/
/*	list resource.							*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	old_pw		Original Widget					*/
/*									*/
/*	new_pw		Contains what is requested			*/
/*									*/
/*      resource_index	The index of the resource     			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_set_value_print_format_list(ar_old_pw,ar_new_pw,l_resource_index)
    DXmPrintWgt		ar_old_pw;
    DXmPrintWgt		ar_new_pw;
    int			l_resource_index;
{
    pw_ar_res_union	ar_old_resources;
    pw_ar_res_union	ar_new_resources;
    XmString		*ar_temp_list;

    ar_old_resources = PWar_resources(ar_old_pw);
    ar_new_resources = PWar_resources(ar_new_pw);

    if (ar_new_resources[l_resource_index].r_cs_list_res.ar_cs_list !=
	ar_old_resources[l_resource_index].r_cs_list_res.ar_cs_list)
    {	    
#ifdef PRINTWGT_DEBUG
	if (PWr_debug(ar_new_pw).l_current_value)
	    printf("pw_set_value_print_format_list %d value changed\n",l_resource_index);
#endif

	/****************************************************************/
	/* 								*/
	/* Make sure everything's OK.				      	*/
	/* 								*/
	/****************************************************************/
	if (!pw_validate_value_print_format_list(ar_new_pw,l_resource_index))
	{
#ifdef PRINTWGT_DEBUG
	    if (PWr_debug(ar_new_pw).l_current_value)
		printf("pw_set_value_print_format_list:  Print format list is invalid.  Not changing value.\n");
#endif
	    ar_new_resources[l_resource_index].r_cs_list_res.ar_cs_list = 
		ar_old_resources[l_resource_index].r_cs_list_res.ar_cs_list;
	    ar_new_resources[l_resource_index].r_cs_list_res.l_cs_count = 
		ar_old_resources[l_resource_index].r_cs_list_res.l_cs_count;
	}
	else
	{
	    /************************************************************/
	    /* 								*/
	    /* Free up the old memory used.			      	*/
	    /* 								*/
	    /************************************************************/
	    if (ar_old_resources[l_resource_index].r_cs_list_res.ar_cs_list)
	    {
		pw_free_cs_list_ref_memory(ar_old_resources[l_resource_index].r_cs_list_res.ar_cs_list,
					   ar_old_resources[l_resource_index].r_cs_list_res.l_cs_count);
		XtFree((char *)ar_old_resources[l_resource_index].r_cs_list_res.ar_cs_list);
	    }

	    /************************************************************/
	    /* 								*/
	    /* Copy the list and set the contents of the list box      	*/
	    /* 								*/
	    /************************************************************/	
	    if (ar_new_resources[l_resource_index].r_cs_list_res.l_cs_count)
	    {
		ar_temp_list = ar_new_resources[l_resource_index].r_cs_list_res.ar_cs_list;

		pw_copy_cs_list(ar_temp_list,
				&ar_new_resources[l_resource_index].r_cs_list_res.ar_cs_list,
				&ar_new_resources[l_resource_index].r_cs_list_res.l_cs_count);
	    }
	    else
		ar_new_resources[l_resource_index].r_cs_list_res.ar_cs_list = NULL;

	    /************************************************************/
	    /* 								*/
	    /* Populate the print format box.				*/
	    /* 								*/
	    /************************************************************/	
	    pw_populate_print_format_listbox(ar_new_pw);
	    
	}
    }
    
} /* pw_set_value_print_format_list */


/************************************************************************/
/*									*/
/* pw_validate_value_filename_list					*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine validates the value of the filename list.		*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/*      resource_index	The index of the resource			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static Boolean pw_validate_value_filename_list(ar_pw,l_resource_index)
    DXmPrintWgt		ar_pw;
    int			l_resource_index;
{   
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_validate_value_filename_list %d\n",l_resource_index);
#endif
    
    /********************************************************************/
    /*									*/
    /* If we've gotten this far, everything must be OK.			*/
    /*									*/
    /********************************************************************/
    return TRUE;
    
} /* pw_validate_value_filename_list */


/************************************************************************/
/*									*/
/* pw_set_value_filename_list						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine sets the value of a compound string		*/
/*	list resource.							*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	old_pw		Original Widget					*/
/*									*/
/*	new_pw		Contains what is requested			*/
/*									*/
/*      resource_index	The index of the resource     			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_set_value_filename_list(ar_old_pw,ar_new_pw,l_resource_index)
    DXmPrintWgt		ar_old_pw;
    DXmPrintWgt		ar_new_pw;
    int			l_resource_index;
{   
    pw_ar_res_union	ar_old_resources;
    pw_ar_res_union	ar_new_resources;
    XmString		*ar_temp_list;

    ar_old_resources = PWar_resources(ar_old_pw);
    ar_new_resources = PWar_resources(ar_new_pw);

    if (ar_new_resources[l_resource_index].r_cs_list_res.ar_cs_list !=
	ar_old_resources[l_resource_index].r_cs_list_res.ar_cs_list)
    {	    
#ifdef PRINTWGT_DEBUG
	if (PWr_debug(ar_new_pw).l_current_value)
	    printf("pw_set_value_filename_list %d value changed\n",l_resource_index);
#endif

	/****************************************************************/
	/* 								*/
	/* Make sure everything's OK.				      	*/
	/* 								*/
	/****************************************************************/
	if (!pw_validate_value_filename_list(ar_new_pw,l_resource_index))
	{
#ifdef PRINTWGT_DEBUG
	    if (PWr_debug(ar_new_pw).l_current_value)
		printf("pw_set_value_filename_list:  Filename list is invalid.  Not changing value.\n");
#endif
	    ar_new_resources[l_resource_index].r_cs_list_res.ar_cs_list = 
		ar_old_resources[l_resource_index].r_cs_list_res.ar_cs_list;
	    ar_new_resources[l_resource_index].r_cs_list_res.l_cs_count = 
		ar_old_resources[l_resource_index].r_cs_list_res.l_cs_count;
	}
	else
	{
	    /************************************************************/
	    /* 								*/
	    /* Free up the old memory used.			      	*/
	    /* 								*/
	    /************************************************************/
	    if (ar_old_resources[l_resource_index].r_cs_list_res.ar_cs_list)
	    {
		pw_free_cs_list_ref_memory(ar_old_resources[l_resource_index].r_cs_list_res.ar_cs_list,
					   ar_old_resources[l_resource_index].r_cs_list_res.l_cs_count);
		XtFree((char *)ar_old_resources[l_resource_index].r_cs_list_res.ar_cs_list);
	    }

	    /************************************************************/
	    /* 								*/
	    /* Copy the list and set the contents of the list box      	*/
	    /* 								*/
	    /************************************************************/	
	    if (ar_new_resources[l_resource_index].r_cs_list_res.l_cs_count)
	    {
		ar_temp_list = ar_new_resources[l_resource_index].r_cs_list_res.ar_cs_list;

		pw_copy_cs_list(ar_temp_list,
				&ar_new_resources[l_resource_index].r_cs_list_res.ar_cs_list,
				&ar_new_resources[l_resource_index].r_cs_list_res.l_cs_count);
	    }
	    else
		ar_new_resources[l_resource_index].r_cs_list_res.ar_cs_list = NULL;

	    /************************************************************/
	    /* 								*/
	    /* Determine the new print format from the filename list.	*/
	    /* 								*/
	    /************************************************************/
            PWr_cs_res(ar_new_pw,K_PW_PRINT_FORMAT_CHOICE_MAP).ar_current_cs = NULL;
            pw_decide_print_format_choice(ar_new_pw);

	    PW_SET_LISTBOX_WIDGET_SELECTION_FROM_RESOURCE(ar_new_pw,
							  K_PW_PRINT_FORMAT_CHOICE_MAP);

	    pw_propagate_print_format_choice(ar_new_pw);	    
	}
    }
    
} /* pw_set_value_filename_list */


/************************************************************************/
/*									*/
/* pw_set_value_readonly_cs_list					*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine inhibits the setting of the value of a 		*/
/*	read only compound string list resource.			*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	old_pw		Original Widget					*/
/*									*/
/*	new_pw		Contains what is requested			*/
/*									*/
/*      resource_index	The index of the resource     			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_set_value_readonly_cs_list(ar_old_pw,ar_new_pw,l_resource_index)
    DXmPrintWgt		ar_old_pw;
    DXmPrintWgt		ar_new_pw;
    int			l_resource_index;
{   
    pw_ar_res_union	ar_old_resources;
    pw_ar_res_union	ar_new_resources;

    ar_old_resources = PWar_resources(ar_old_pw);
    ar_new_resources = PWar_resources(ar_new_pw);

    if ((ar_new_resources[l_resource_index].r_cs_list_res.ar_cs_list !=
	 ar_old_resources[l_resource_index].r_cs_list_res.ar_cs_list) ||
        (ar_new_resources[l_resource_index].r_cs_list_res.l_cs_count !=
 	 ar_old_resources[l_resource_index].r_cs_list_res.l_cs_count))
    {	
#ifdef PRINTWGT_DEBUG
	if (PWr_debug(ar_new_pw).l_current_value)
	    printf("pw_set_value_readonly_cs_list %d value changed\n",l_resource_index);
#endif

	ar_new_resources[l_resource_index].r_cs_list_res.ar_cs_list = 
	    ar_old_resources[l_resource_index].r_cs_list_res.ar_cs_list;

	ar_new_resources[l_resource_index].r_cs_list_res.l_cs_count = 
	    ar_old_resources[l_resource_index].r_cs_list_res.l_cs_count;
    }

} /* pw_set_value_readonly_cs_list */


/************************************************************************/
/*									*/
/* pw_validate_value_cs_choice						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine validate the cs choice value of a list box.	*/
/*	The resources are laid out in such a way that the list		*/
/*	resource is just prior to the choice in the resource list.	*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/*      resource_index	The index of the resource			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	Boolean								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static Boolean pw_validate_value_cs_choice(ar_pw,l_resource_index)
    DXmPrintWgt		ar_pw;
    int			l_resource_index;
{   
    Boolean	c_found;
    
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_validate_value_cs_choice %d\n",l_resource_index);
#endif

    /********************************************************************/
    /*									*/
    /* If there is no choice, it is valid.  If there is a choice, look	*/
    /* at each string in the list and do a string compare.		*/
    /*									*/
    /********************************************************************/
    c_found = FALSE;

    if (PWr_cs_res(ar_pw,l_resource_index).ar_current_cs)
    {
	if (pw_get_cs_list_element_index(PWr_cs_res(ar_pw,l_resource_index).ar_current_cs,
					 PWr_cs_list_res(ar_pw,l_resource_index - 1).ar_cs_list,
					 PWr_cs_list_res(ar_pw,l_resource_index - 1).l_cs_count) >= 0)
	    c_found = TRUE;
    }
    else
    {
	c_found = TRUE;
    }
    
    return c_found;
	
} /* pw_validate_value_cs_choice */


/************************************************************************/
/*									*/
/* pw_set_value_cs_choice						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine sets the value of a widget based text		*/
/*	resource.							*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	old_pw		Original Widget					*/
/*									*/
/*	new_pw		Contains what is requested			*/
/*									*/
/*      resource_index	The index of the resource     			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_set_value_cs_choice(ar_old_pw,ar_new_pw,l_resource_index)
    DXmPrintWgt		ar_old_pw;
    DXmPrintWgt		ar_new_pw;
    int			l_resource_index;
{   
    pw_ar_res_union	ar_old_resources;
    pw_ar_res_union	ar_new_resources;
    int			l_choice_index;
    
    ar_old_resources = PWar_resources(ar_old_pw);
    ar_new_resources = PWar_resources(ar_new_pw);

    if ((ar_new_resources[l_resource_index].r_cs_res.ar_current_cs !=
	 ar_old_resources[l_resource_index].r_cs_res.ar_current_cs) ||
	(!pw_xm_string_compare(ar_new_resources[l_resource_index].r_cs_res.ar_current_cs,
			       ar_old_resources[l_resource_index].r_cs_res.ar_current_cs)))
    {
#ifdef PRINTWGT_DEBUG
	if (PWr_debug(ar_new_pw).l_current_value)
	    printf("pw_set_value_cs_choice %d value changed\n",l_resource_index);
#endif

	/****************************************************************/
	/*								*/
	/* Make sure the value given is OK.				*/
	/*								*/
	/****************************************************************/
	l_choice_index = pw_get_cs_list_element_index(PWr_cs_res(ar_new_pw,l_resource_index).ar_current_cs,
						      PWr_cs_list_res(ar_new_pw,l_resource_index - 1).ar_cs_list,
						      PWr_cs_list_res(ar_new_pw,l_resource_index - 1).l_cs_count);
	
	/* Reject it if it is not NULL and it is not in the new list 
	 */
	if (PWr_cs_res(ar_new_pw,l_resource_index).ar_current_cs && (l_choice_index < 0))
	{
#ifdef PRINTWGT_DEBUG
	    if (PWr_debug(ar_new_pw).l_current_value)
		printf("pw_set_value_cs_choice:  List choice not found.  Not changing old value.\n");
#endif
	    ar_new_resources[l_resource_index].r_cs_res.ar_current_cs = 
		ar_old_resources[l_resource_index].r_cs_res.ar_current_cs;
	}
	else
	{    
	    /************************************************************/
	    /* 								*/
	    /* Point to the element in the list if the new value is	*/
	    /* not NULL.						*/
	    /* 								*/
	    /************************************************************/
	    if (ar_new_resources[l_resource_index].r_cs_res.ar_current_cs)
		ar_new_resources[l_resource_index].r_cs_res.ar_current_cs = 
		    PWr_cs_list_res(ar_new_pw,l_resource_index - 1).ar_cs_list[l_choice_index];

	    PW_SET_LISTBOX_WIDGET_SELECTION_FROM_RESOURCE(ar_new_pw,
							  l_resource_index);

	    /************************************************************/
	    /*								*/
	    /* If this is the print format choice, make sure it gets	*/
	    /* propagated to all the other widgets.			*/
	    /*								*/
	    /************************************************************/
	    if (l_resource_index == K_PW_PRINT_FORMAT_CHOICE_MAP)
		pw_propagate_print_format_choice(ar_new_pw);

	    /************************************************************/
	    /*								*/
	    /* If this is the printer queue, make sure the choice for	*/
	    /* this particular print format is saved.  The choice is 	*/
	    /* saved by encoding the current queue list index and the 	*/
	    /* selected position in the same number.			*/
	    /*								*/
	    /* Make sure we check to see if this CS string is NULL, if  */
	    /* so dont' even go into pw_get_cs_list_element_index,      */
	    /* because it will only return a -1, which is the default.  */
	    /*								*/
	    /************************************************************/
	    if ((PWr_cs_list_res(ar_new_pw,K_PW_PRINTER_CONTENTS_MAP).ar_cs_list != NULL) &&
		(l_resource_index == K_PW_PRINTER_CHOICE_MAP))
		{
		PWal_current_format_queue_choices(ar_new_pw)[PWl_print_format_choice(ar_new_pw)] = 
		    (PWl_current_queue_list_index(ar_new_pw) << 16) +
		    pw_get_cs_list_element_index(PWr_cs_res(ar_new_pw,K_PW_PRINTER_CHOICE_MAP).ar_current_cs,
						 PWr_cs_list_res(ar_new_pw,K_PW_PRINTER_CONTENTS_MAP).ar_cs_list,
						 PWr_cs_list_res(ar_new_pw,K_PW_PRINTER_CONTENTS_MAP).l_cs_count);
                }
	}
    }
} /* pw_set_value_cs_choice */


/************************************************************************/
/*									*/
/* pw_save_value_cs_choice						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine saves the value of a widget based comp string	*/
/*	resource.							*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/*      resource_index	The index of the resource			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_save_value_cs_choice(ar_pw,l_resource_index)
    DXmPrintWgt		ar_pw;
    int			l_resource_index;
{
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_save_value_cs_choice %d\n",l_resource_index);
#endif

    /********************************************************************/
    /* 									*/
    /* Save value of the cs resource.					*/
    /* 									*/
    /********************************************************************/
    PWr_cs_res(ar_pw,l_resource_index).ar_previous_cs = 
	PWr_cs_res(ar_pw,l_resource_index).ar_current_cs;

} /* pw_save_value_cs_choice */


/************************************************************************/
/*									*/
/* pw_restore_value_cs_choice						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine restores the value of a compound string		*/
/*	resource.							*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/*      resource_index	The index of the resource			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_restore_value_cs_choice(ar_pw,l_resource_index)
    DXmPrintWgt		ar_pw;
    int			l_resource_index;
{   
    int			l_choice_index;

#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_restore_value_cs_choice %d\n",l_resource_index);
#endif
	
    /********************************************************************/
    /*									*/
    /* Since the application may have modified the list since last save,*/
    /* make sure the previous choice actually is in the current list.  	*/
    /* If it isn't, don't restore the value to the previous choice.	*/
    /*									*/
    /********************************************************************/
    l_choice_index = pw_get_cs_list_element_index(PWr_cs_res(ar_pw,l_resource_index).ar_previous_cs,
						  PWr_cs_list_res(ar_pw,l_resource_index - 1).ar_cs_list,
						  PWr_cs_list_res(ar_pw,l_resource_index - 1).l_cs_count);

    if (l_choice_index >= 0)
	PWr_cs_res(ar_pw,l_resource_index).ar_current_cs = 
	    PWr_cs_list_res(ar_pw,l_resource_index - 1).ar_cs_list[l_choice_index];
    
    /********************************************************************/
    /* 									*/
    /* Set the value in the listbox.					*/
    /* 									*/
    /********************************************************************/
    PW_SET_LISTBOX_WIDGET_SELECTION_FROM_RESOURCE(ar_pw,
						  l_resource_index);

    /********************************************************************/
    /*									*/
    /* If this is the print format choice, make sure it gets		*/
    /* propagated to all the other widgets.				*/
    /*									*/
    /********************************************************************/
    if (l_resource_index == K_PW_PRINT_FORMAT_CHOICE_MAP)
	pw_propagate_print_format_choice(ar_pw);

} /* pw_restore_value_cs_choice */


/************************************************************************/
/*									*/
/* pw_validate_value_time						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine validates the value of a text widget based time	*/
/*	resource.							*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Widget						*/
/*									*/
/*      resource_index	The index of the resource     			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	TRUE (valid) FALSE (invalid)					*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static Boolean pw_validate_value_time(ar_pw,l_resource_index)
    DXmPrintWgt		ar_pw;
    int			l_resource_index;
{   
    int	al_temp_time[2];

#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_validate_value_time %d\n",l_resource_index);
#endif

    /********************************************************************/
    /*									*/
    /* It's OK to have a "Now" in the print after box.			*/
    /*									*/
    /********************************************************************/
    if (l_resource_index == K_PW_PRINT_AFTER_MAP)
    {
	if (pw_xm_string_compare(PWr_cs_res(ar_pw,l_resource_index).ar_current_cs,
	 	                 PWar_now_string(ar_pw)))
	    return(TRUE);
    }

    /********************************************************************/
    /*									*/
    /* Otherwise, it's necessary to check the time.			*/
    /*									*/
    /********************************************************************/
    return(pw_convert_cs_to_time(PWr_cs_res(ar_pw,l_resource_index).ar_current_cs,
				 al_temp_time));

} /* pw_validate_value_time */
    

/************************************************************************/
/*									*/
/* pw_listbox_activate							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine is called whenever a list box element is selected. */
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/*	list_box	Listbox widget resource index			*/
/*									*/
/*	unused		Unused						*/
/*									*/
/*	callback_data	Callback data for mapping.			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_listbox_activate(ar_pw,l_listbox,l_unused,ar_callback_data)
    DXmPrintWgt			ar_pw;
    int				l_listbox;
    int				l_unused;
    XmListCallbackStruct	*ar_callback_data;
{
    int	l_choice_index;

#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_listbox_activate: %d  reason:  %d\n",l_listbox,ar_callback_data->reason);
#endif

    /********************************************************************/
    /*									*/
    /* Determine which element in the list has been selected.		*/
    /*									*/
    /********************************************************************/
    l_choice_index = pw_get_cs_list_element_index(ar_callback_data->item,
						  PWr_cs_list_res(ar_pw,l_listbox).ar_cs_list,
						  PWr_cs_list_res(ar_pw,l_listbox).l_cs_count);

    /********************************************************************/
    /*									*/
    /* If this is a browse select, set the current selection.		*/
    /*									*/
    /* The resources are laid out in such a way that the resource	*/
    /* following the list box contents is guaranteed to be the list box	*/
    /* choice.								*/
    /*									*/
    /********************************************************************/
    if (ar_callback_data->reason == XmCR_BROWSE_SELECT)
	PWr_cs_res(ar_pw,l_listbox + 1).ar_current_cs =
	    PWr_cs_list_res(ar_pw,l_listbox).ar_cs_list[l_choice_index];

    /********************************************************************/
    /*									*/
    /* If this is an single select, do some really wierd stuff.	        */
    /* This is so users can deselect printer forms if they want to.	*/
    /*									*/
    /* The resources are laid out in such a way that the resource	*/
    /* following the list box contents is guaranteed to be the list box	*/
    /* choice.								*/
    /*									*/
    /********************************************************************/
    if (ar_callback_data->reason == XmCR_SINGLE_SELECT)
    {
        /****************************************************************/
        /*								*/
	/* If they selected the same string again, then deselect it.    */
	/* Otherwise, select it.					*/
	/*								*/
	/****************************************************************/
	if (pw_xm_string_compare(PWr_cs_res(ar_pw,l_listbox + 1).ar_current_cs,
		                 ar_callback_data->item))
	    PWr_cs_res(ar_pw,l_listbox + 1).ar_current_cs = NULL;
	else
	    PWr_cs_res(ar_pw,l_listbox + 1).ar_current_cs =
		PWr_cs_list_res(ar_pw,l_listbox).ar_cs_list[l_choice_index];
    }

    /********************************************************************/
    /*									*/
    /* If this is the print format, make sure the choice is propagated.	*/
    /*									*/
    /********************************************************************/
    if (l_listbox == K_PW_PRINT_FORMAT_CONTENTS_MAP)
	pw_propagate_print_format_choice(ar_pw);
    
    /********************************************************************/
    /*									*/
    /* If this is the printer queue, make sure the choice for this 	*/
    /* particular print format is saved.  The choice is saved by 	*/
    /* encoding the current queue list index and the selected position	*/
    /* in the same number.						*/
    /*									*/
    /********************************************************************/
    if (l_listbox == K_PW_PRINTER_CONTENTS_MAP)
	PWal_current_format_queue_choices(ar_pw)[PWl_print_format_choice(ar_pw)] = 
	    (PWl_current_queue_list_index(ar_pw) << 16) + (ar_callback_data->item_position - 1);
    
} /* pw_listbox_activate */


/************************************************************************/
/*									*/
/* pw_radiobox_activate							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine is called whenever a radio box element is selected */
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/*	radiobox	Radio widget resource index			*/
/*									*/
/*	pushbutton	Widget index of button pressed.			*/
/*									*/
/*	callback_data	Callback data for mapping.			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_radiobox_activate(ar_pw,l_radiobox,l_pushbutton,ar_callback_data)
    DXmPrintWgt				ar_pw;
    int					l_radiobox;
    int					l_pushbutton;
    XmToggleButtonCallbackStruct	*ar_callback_data;
{
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_radiobox_activate:  radiobox %d, button %d, reason %d\n",
	       l_radiobox,
	       l_pushbutton,
	       ar_callback_data->reason);
#endif

    if (ar_callback_data->set)
	PWr_int_res(ar_pw,l_radiobox).l_current_value =
	    l_pushbutton - (pw_ar_resource_control[l_radiobox].l_widget_index + 1);
        
} /* pw_radiobox_activate */


/************************************************************************/
/*									*/
/* pw_toggle_activate							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine is called whenever a toggle button is pressed 	*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/*	toggle		Optionmenu resource index			*/
/*									*/
/*	unused		Not Used		       			*/
/*									*/
/*	callback_data	Callback data for mapping.			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_toggle_activate(ar_pw,l_toggle,l_unused,ar_callback_data)
    DXmPrintWgt				ar_pw;
    int					l_toggle;
    int					l_unused;
    XmToggleButtonCallbackStruct	*ar_callback_data;
{
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_toggle_activate: toggle %d, reason %d\n",
	       l_toggle,
	       ar_callback_data->reason);
#endif

    PWr_boolean_res(ar_pw,l_toggle).b_current_value = ar_callback_data->set; 

} /* pw_toggle_activate */

/************************************************************************/
/*									*/
/* pw_optionmenu_activate						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine is called whenever an optionmenu button is pressed */
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/*	optionmenu	Optionmenu resource index			*/
/*									*/
/*	pushbutton	Widget index of button pressed.			*/
/*									*/
/*	callback_data	Callback data for mapping.			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_optionmenu_activate(ar_pw,l_optionmenu,l_pushbutton,ar_callback_data)
    DXmPrintWgt			ar_pw;
    int				l_optionmenu;
    int				l_pushbutton;
    XmAnyCallbackStruct		*ar_callback_data;
{
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_optionmenu_activate:  optionmenu %d, button %d, reason %d\n",
	       l_optionmenu,
	       l_pushbutton,
	       ar_callback_data->reason);
#endif

    PWr_int_res(ar_pw,l_optionmenu).l_current_value =
	l_pushbutton - (pw_ar_resource_control[l_optionmenu].l_widget_index + 1);
        
} /* pw_optionmenu_activate */


/************************************************************************/
/*									*/
/* pw_save_values							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine saves the state of the resource values specified	*/
/*	by the from and to indeces.				       	*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		print widget					*/
/*			     						*/
/*	from		beginning resource index			*/
/*									*/
/* 	to		end resource index (inclusive)			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	The values of the variables are saved.				*/
/*									*/
/************************************************************************/
static void pw_save_values(ar_pw,l_from,l_to)
    DXmPrintWgt		ar_pw;
    int			l_from;
    int			l_to;
{
    int	l_i;
    
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_save_values from %d to %d\n",l_from,l_to);
#endif

    for (l_i = l_from; l_i <= l_to; l_i++)
    {
	if (pw_ar_resource_control[l_i].save_value_proc)
	    (*(pw_ar_resource_control[l_i].save_value_proc))(ar_pw,l_i);
    }
    
} /* pw_save_values */


/************************************************************************/
/*									*/
/* pw_restore_values							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine restores the state of the values of the resources	*/
/*	specified by the from and to indeces.				*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		print widget					*/
/*			     						*/
/*	from		beginning resource index			*/
/*									*/
/* 	to		end resource index (inclusive)			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	The values of the variables are restored.			*/
/*									*/
/************************************************************************/
static void pw_restore_values(ar_pw,l_from,l_to)
    DXmPrintWgt		ar_pw;
    int			l_from;
    int			l_to;
{
    int l_i;
    
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_restore_values from %d to %d\n",l_from,l_to);
#endif

    for (l_i = l_from; l_i <= l_to; l_i++)
    {
	if (pw_ar_resource_control[l_i].restore_value_proc)
	    (*(pw_ar_resource_control[l_i].restore_value_proc))(ar_pw,l_i);
    }
    
} /* pw_restore_values */


/************************************************************************/
/*									*/
/* pw_make_widgets_match_resources					*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine sets the state of the widgets based upon the	*/
/*	values of the resources.					*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		print widget					*/
/*			     						*/
/*	from		beginning resource index			*/
/*									*/
/* 	to		end resource index (inclusive)			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	The widgets are set according to the value of the resources.	*/
/*									*/
/************************************************************************/
static void pw_make_widgets_match_resources(ar_pw,l_from,l_to)
    DXmPrintWgt		ar_pw;
    int			l_from;
    int			l_to;
{
    int 	l_i;
    
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_make_widgets_match_resources from %d to %d\n",l_from,l_to);
#endif

    for (l_i = l_from; l_i <= l_to; l_i++)
    {
#ifdef PRINTWGT_DEBUG
	if (PWr_debug(ar_pw).l_current_value)
	    printf("pw_make_widgets_match_resources %d\n",l_i);
#endif

	switch (pw_ar_resource_control[l_i].l_type)
	{
	    case K_PW_RESOURCE_TYPE_CS_INT:
	    {
		PW_SET_CS_INT_WIDGET_FROM_RESOURCE(ar_pw,l_i);
		break;
	    }

	    case K_PW_RESOURCE_TYPE_RADIOBOX:
	    {
		PW_SET_RADIOBOX_WIDGET_FROM_RESOURCE(ar_pw,l_i);
		break;
	    }
	    
	    case K_PW_RESOURCE_TYPE_TOGGLE:
	    {
		PW_SET_TOGGLE_WIDGET_FROM_RESOURCE(ar_pw,l_i);
		break;
	    }

	    case K_PW_RESOURCE_TYPE_OPTIONMENU:
	    {
		PW_SET_OPTIONMENU_WIDGET_FROM_RESOURCE(ar_pw,l_i);
		break;
	    }

	    case K_PW_RESOURCE_TYPE_CS:
	    {
		PW_SET_CS_WIDGET_FROM_RESOURCE(ar_pw,l_i);
		break;
	    }
	    
	    case K_PW_RESOURCE_TYPE_CS_CHOICE:
	    {
		PW_SET_LISTBOX_WIDGET_SELECTION_FROM_RESOURCE(ar_pw,l_i);
		break;
	    }

	    case K_PW_RESOURCE_TYPE_CS_LIST:
	    {
		PW_SET_LISTBOX_WIDGET_CONTENTS_FROM_RESOURCE(ar_pw,l_i);
		break;
	    }
		
	    default: break;
		
	} /* switch */
    } /* for which loops through resources */
} /* pw_make_widgets_match_resources */


/************************************************************************/
/*									*/
/* pw_make_resources_match_widgets					*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine examines the values of the text widgets and makes	*/
/*	the values of the resources match accordingly.			*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		print widget					*/
/*			     						*/
/*	from		beginning resource index			*/
/*									*/
/* 	to		end resource index (inclusive)			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	The values of the text widget based resources are set.		*/
/*									*/
/************************************************************************/
static void pw_make_resources_match_widgets(ar_pw,l_from,l_to)
    DXmPrintWgt		ar_pw;
    int			l_from;
    int			l_to;
{
    int 	l_i;
    XmString	ar_cs_value;
    long	l_size,l_status;

#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_make_resources_match_widgets from %d to %d\n",l_from,l_to);
#endif

    for (l_i = l_from; l_i <= l_to; l_i++)
    {
	if (PWar_widget_id(ar_pw,
			   pw_ar_resource_control[l_i].l_widget_index))
	{
	    switch (pw_ar_resource_control[l_i].l_type)
	    {
		case K_PW_RESOURCE_TYPE_CS_INT:
		{
		    /****************************************************/
		    /*							*/
		    /* Scan the value as an integer.			*/
		    /*							*/
		    /****************************************************/
		    ar_cs_value = 
			(XmString) DXmCSTextGetString(PWar_widget_id(ar_pw,
								     pw_ar_resource_control[l_i].l_widget_index));

		    PWr_int_res(ar_pw,l_i).l_current_value = (int) _DXmCvtCStoI(ar_cs_value,
                              							&l_size,
                              							&l_status);
		    XmStringFree(ar_cs_value);
		    
		    /****************************************************/
		    /*							*/
		    /* While we have the value, we might as well set	*/
		    /* the string to what we think it is.  This will at */
		    /* least let the user get some feedback as to what	*/
		    /* we think the number is.				*/
		    /*							*/
		    /****************************************************/
		    PW_SET_CS_INT_WIDGET_FROM_RESOURCE(ar_pw,l_i);

		    break;
		} /* case K_PW_RESOURCE_TYPE_CS_INT */
		
		case K_PW_RESOURCE_TYPE_CS:
		{		    
		    /****************************************************/
		    /*							*/
		    /* Scan the value as a string.  Free up old memory	*/
		    /* first.						*/
		    /*							*/
		    /****************************************************/
		    if (PWr_cs_res(ar_pw,l_i).ar_current_cs)
			XmStringFree(PWr_cs_res(ar_pw,l_i).ar_current_cs);
		    
		    PWr_cs_res(ar_pw,l_i).ar_current_cs =
			(XmString) DXmCSTextGetString(PWar_widget_id(ar_pw,
								     pw_ar_resource_control[l_i].l_widget_index));
		    break;
		} /* case K_PW_RESOURCE_TYPE_CS */
		
		default: break;
		
	    } /* switch */
	} /* if widget exists */
    } /* for which loops through widgets */
} /* pw_make_resources_match_widgets */


/************************************************************************/
/*									*/
/* pw_display_a_message							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine displays a message in a message box.		*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*			     						*/
/*	resource_index	Index of resource				*/
/*									*/
/*	message_index	Index of message in error message table.	*/
/*			     						*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_display_a_message(ar_pw,l_resource_index,l_message_index)
    DXmPrintWgt		ar_pw;
    int			l_resource_index;
    int			l_message_index;
{
    unsigned long int	l_mask = CWStackMode;
    XWindowChanges	r_value;
    Arg			ar_al[2];
    XtCallbackRec	ar_help_callback[2];
    XtCallbackList	ar_help_callback_list = ar_help_callback;
    
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_display_a_message\n");
#endif

    /********************************************************************/
    /*									*/
    /* Turn on the wait cursor.						*/
    /*									*/
    /********************************************************************/
    pw_turn_on_wait_cursor(ar_pw,PWar_widget_id(ar_pw,K_PW_PRIMARY_BOX));
    pw_turn_on_wait_cursor(ar_pw,PWar_widget_id(ar_pw,K_PW_2NDARY_BOX));

    /********************************************************************/
    /*									*/
    /* If the message box has not been created, create it.		*/
    /*									*/
    /********************************************************************/
    if (!PWar_widget_id(ar_pw,K_PW_MSG))
    {
	/****************************************************************/
	/*								*/
	/* Fetch the message box.					*/
	/*								*/
	/****************************************************************/
	MrmType	ar_dummy_class;

	if (MrmFetchWidget (PWar_mrm_hierarchy(ar_pw),	/* MRM heirarchy	*/
			    K_PW_MESSAGE_BOX_NAME,	/* Name of widget	*/
			    (Widget) ar_pw,			/* Parent		*/
			    &PWar_widget_id(ar_pw,
					    K_PW_MSG),	/* Message box		*/
			    &ar_dummy_class) 		/* Class		*/
	    != MrmSUCCESS)
	{
	    String params = K_PW_MESSAGE_BOX_NAME;
	    Cardinal num_params = 1;
	
	    DXMAPPWARNING(XtWidgetToApplicationContext((Widget)ar_pw),
			  PWMSGNAME2,
			  PWGTNOMRMFETCH,
			  &params,&num_params);
	}

	/****************************************************************/
	/*								*/
	/* Unmanage the help and cancel buttons				*/
	/*								*/
	/****************************************************************/
	XtUnmanageChild((Widget)XmMessageBoxGetChild(PWar_widget_id(ar_pw,K_PW_MSG),
					     XmDIALOG_CANCEL_BUTTON));
    }
    
    /********************************************************************/
    /*									*/
    /* Set the message from the error message table.			*/
    /*									*/
    /********************************************************************/

    /********************************************************************/
    /*									*/
    /* Set the label and help callbacks then manage the message box.	*/
    /* Make sure the help callback on the message box gives help on	*/
    /* the thing which caused us to display this message.		*/
    /*									*/
    /********************************************************************/
    XtSetArg(ar_al[0],XmNhelpCallback,&ar_help_callback_list);
    XtGetValues(PWar_widget_id(ar_pw,pw_ar_resource_control[l_resource_index].l_widget_index),
		ar_al,
		1);

    XtSetArg(ar_al[0],XmNhelpCallback,ar_help_callback_list);
    XtSetArg(ar_al[1],XmNmessageString,PWar_error_message(ar_pw,l_message_index));
    XtSetValues(PWar_widget_id(ar_pw,K_PW_MSG),
		ar_al,
		2);

    XtManageChild(PWar_widget_id(ar_pw,K_PW_MSG));

    /********************************************************************/
    /*									*/
    /* Force the message widget to the top of the	 		*/
    /* stacking order among its siblings.				*/
    /*									*/
    /********************************************************************/
    r_value.stack_mode = Above;
	    
    if (XtIsRealized(PWar_widget_id(ar_pw,K_PW_MSG)))
	XConfigureWindow(XtDisplay(PWar_widget_id(ar_pw,K_PW_MSG)),
			 XtWindow(PWar_widget_id(ar_pw,K_PW_MSG)),
			 l_mask,
			 &r_value);
	    	    
    /********************************************************************/
    /*									*/
    /* Turn off the wait cursor.       					*/
    /*									*/
    /********************************************************************/
    pw_turn_off_wait_cursor(ar_pw,PWar_widget_id(ar_pw,K_PW_PRIMARY_BOX));
    pw_turn_off_wait_cursor(ar_pw,PWar_widget_id(ar_pw,K_PW_2NDARY_BOX));

} /* pw_display_a_message */


/************************************************************************/
/*									*/
/* pw_validate_cs_widget_based_resources				*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine performs validations on the cs widget based	*/
/*	resources.  If they are OK, it returns true.			*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*			     						*/
/*	from		beginning resource index			*/
/*									*/
/* 	to		end resource index (inclusive)			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static Boolean pw_validate_cs_widget_based_resources(ar_pw,l_from,l_to)
    DXmPrintWgt		ar_pw;
    int			l_from;
    int			l_to;
{
    int	l_i;
    
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_validate_cs_widget_based_resources from %d to %d\n",l_from,l_to);
#endif

    for (l_i = l_from; l_i <= l_to; l_i++)
    {
	switch (l_i)
	{
	    case K_PW_NUMBER_COPIES_MAP:
	    {
		if (!pw_validate_value_int(ar_pw,l_i))
		{
		    pw_display_a_message(ar_pw,l_i,K_PW_INVALID_NUMBER_COPIES);
		    return(FALSE);
		}
		break;
	    }
	    
	    case K_PW_PAGE_RANGE_FROM_MAP:
	    case K_PW_PAGE_RANGE_TO_MAP:
	    {
		break;
	    }
	    
	    case K_PW_PRINT_AFTER_MAP:
	    {
		if (!pw_validate_value_time(ar_pw,l_i))
		{
		    pw_display_a_message(ar_pw,l_i,K_PW_INVALID_START_AFTER_TIME);
		    return(FALSE);
		}
		break;
	    }

	    case K_PW_NUMBER_UP_MAP:
	    {
		if (!pw_validate_value_int(ar_pw,l_i))
		{
		    pw_display_a_message(ar_pw,l_i,K_PW_INVALID_NUMBER_UP);
		    return(FALSE);
		}
		break;
	    }
	    
	    case K_PW_SHEET_COUNT_MAP:
	    {
		if (!pw_validate_value_int(ar_pw,l_i))
		{
		    pw_display_a_message(ar_pw,l_i,K_PW_INVALID_SHEET_COUNT);
		    return(FALSE);
		}
		break;
	    }
	    
	    case K_PW_JOB_NAME_MAP:
	    {		
		if (!pw_validate_value_text_length(ar_pw,l_i))
		{
		    pw_display_a_message(ar_pw,l_i,K_PW_INVALID_JOB_NAME);
		    return(FALSE);
		}
		break;
	    }
	    
	    case K_PW_OPERATOR_MESSAGE_MAP:
	    {
		break;
	    }

	    case K_PW_LAYUP_DEFINITION_MAP:
	    {
		if (!pw_validate_value_text_contents(ar_pw,l_i))
		{
		    pw_display_a_message(ar_pw,l_i,K_PW_INVALID_LAYUP_NAME);
		    return(FALSE);
		}
		break;
	    }
	    
	    case K_PW_START_SHEET_COMMENT_MAP:
	    {
		break;
	    }
	    	    
	    case K_PW_PRIORITY_MAP:
	    {
		
		if (!pw_validate_value_int(ar_pw,l_i))
		{
		    pw_display_a_message(ar_pw,l_i,K_PW_INVALID_PRIORITY);
		    return(FALSE);
		}
		break;
	    }

	    case K_PW_SETUP_MAP:
	    {
		break;
	    }

	    default:	break;
	    
	} /* switch */
    } /* for */
		
    return (TRUE);
 
} /* pw_validate_cs_widget_based_resources */


/************************************************************************/
/*									*/
/* pw_get_queue_list_from_variable					*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*      Determines and returns index to list of Compound Strings which 	*/
/*	correspond to a variable (VMS logical, ULTRIX environment var)	*/
/*	which is defined to be a comma list.				*/
/*                                                                      */
/*	NOTE:  If the variable is not found on the system, all the	*/
/*	       queues will be found and returned.  The first element	*/
/*	       of the print widget queue list array is reserved for	*/
/*	       all the queues on the system.  If its count is set to	*/
/*	       -1, it means all the queues have not been searched for.	*/
/*                                                                      */
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		The print widget.				*/
/*									*/
/*	variable_name	System variable to be found.			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	1								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static int pw_get_queue_list_from_variable(ar_pw,ar_variable_name)
    DXmPrintWgt	ar_pw;
    XmString	ar_variable_name;
{
    int		l_mem_allocation = 0;
    Boolean	c_found;
    int		l_found;
    int		l_i;
    long	l_size,l_status;

#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_get_queue_list_from_variable: %s\n",DXmCvtCStoOS(ar_variable_name,
                                                                    &l_size,
                                                                    &l_status));
#endif

    /********************************************************************/
    /*									*/
    /* See if we have already asked the system for this list.		*/
    /*									*/
    /********************************************************************/
    c_found = FALSE;
    for (l_i = 0; 
	 (l_i < PWl_num_known_queue_lists(ar_pw)) &&
	 !c_found; 
	 l_i++)
    {
	c_found = pw_xm_string_compare(PWar_known_queue_lists(ar_pw)[l_i].ar_variable_name,
				       ar_variable_name);
	l_found = l_i;
    }
    
    /********************************************************************/
    /*									*/
    /* Nope, we must find it.						*/
    /*									*/
    /********************************************************************/
    if (!c_found)
    {
	char	*at_variable_value = NULL;

        /****************************************************************/
	/*								*/
	/* If the variable exists on the system, then use it, even if	*/
	/* the value is NULL.  Otherwise, use the system's.		*/
	/*								*/
        /****************************************************************/
	if (pw_get_variable_value(ar_variable_name,&at_variable_value))
	{
	    l_found = PWl_num_known_queue_lists(ar_pw);
	    /************************************************************/
	    /*							  	*/
	    /* Determine the memory allocation (in longwords) of the	*/
	    /* vector passed in.  (This list's memory will be reused 	*/
	    /* via realloc calls.)					*/
	    /*								*/
	    /************************************************************/
	    l_mem_allocation = 
		((l_found + K_PW_CHUNK_INCR - 1) / 
		 K_PW_CHUNK_INCR) * 
		     K_PW_CHUNK_INCR;

	    /************************************************************/
	    /*								*/
	    /* Convert value into a compound string list.  Allocate more*/
	    /* memory if necessary.					*/
	    /*								*/
	    /************************************************************/
	    (PWl_num_known_queue_lists(ar_pw))++;

	    if (PWl_num_known_queue_lists(ar_pw) > l_mem_allocation)
	    {
		PWar_known_queue_lists(ar_pw) =
		    (pw_r_variable_list_struct *) XtRealloc((char *)PWar_known_queue_lists(ar_pw),
							    sizeof(pw_r_variable_list_struct) *
							    (l_mem_allocation + K_PW_CHUNK_INCR));
	    }
	    
	    PWar_known_queue_lists(ar_pw)[l_found].ar_variable_name = 
		XmStringCopy(ar_variable_name);
	    
	    PWar_known_queue_lists(ar_pw)[l_found].ar_cs_list = NULL;
	    PWar_known_queue_lists(ar_pw)[l_found].l_cs_count = 0;	    
	    pw_parse_comma_string(at_variable_value,
				  &PWar_known_queue_lists(ar_pw)[l_found].ar_cs_list,
				  &PWar_known_queue_lists(ar_pw)[l_found].l_cs_count);

	    XtFree(at_variable_value);
	}
	else
	    l_found = 0; /* use the system's entire list. */	    
    }
    
    /********************************************************************/
    /*									*/
    /* If this is element 0 and we haven't actually gone out to the 	*/
    /* system to ask it for *all* of the queues, we should do so 	*/
    /* now.								*/
    /*    								*/
    /********************************************************************/
    if ((l_found == 0) &&
	(PWar_known_queue_lists(ar_pw)[l_found].l_cs_count < 0))
	pw_get_print_queue_names(&PWar_known_queue_lists(ar_pw)[l_found].ar_cs_list,
				 &PWar_known_queue_lists(ar_pw)[l_found].l_cs_count);
    
    return l_found;

} /* pw_get_queue_list_from_variable */


/************************************************************************/
/*									*/
/* pw_decide_printer_queue_choice					*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*      Decides on a printer queue for the current queue list.		*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		The print widget.				*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	TRUE (a new choice was made) FALSE (current choice is valid)	*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static Boolean pw_decide_printer_queue_choice(ar_pw)
    DXmPrintWgt	ar_pw;
{
    int		l_queue_list;
    int		l_queue_index;
    int		l_found_index,l_choice_index;
    XmString 	ar_new_choice = NULL;
    Boolean	b_return_value;
    
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_decide_printer_queue_choice\n");
#endif

    /********************************************************************/
    /*									*/
    /* If this is on startup, try to find the queue choice given by	*/
    /* the arglist sent to create.					*/
    /*									*/
    /********************************************************************/
    if (PWl_primary_just_created(ar_pw))
	ar_new_choice = PWr_cs_res(ar_pw,K_PW_PRINTER_CHOICE_MAP).ar_current_cs;

    /********************************************************************/
    /*									*/
    /* If the user has previously selected a printer for the particular	*/
    /* print format, choose this one first.				*/
    /*									*/
    /********************************************************************/
    if (PWal_current_format_queue_choices(ar_pw)[PWl_print_format_choice(ar_pw)] != -1)
    {	
	l_queue_list = PWal_current_format_queue_choices(ar_pw)[PWl_print_format_choice(ar_pw)] >> 16;
	l_queue_index = PWal_current_format_queue_choices(ar_pw)[PWl_print_format_choice(ar_pw)] -
	                (l_queue_list << 16);
	
#ifdef PRINTWGT_DEBUG
	if (PWr_debug(ar_pw).l_current_value)
	    printf("pw_decide_printer_queue_choice:  queue_list %d, queue_index %d\n",
		   l_queue_list,
		   l_queue_index);
#endif

	ar_new_choice = PWar_known_queue_lists(ar_pw)[l_queue_list].ar_cs_list[l_queue_index];
    }
    
    /********************************************************************/
    /*									*/
    /* If there is a current printer choice, make sure it is in the	*/
    /* list.  If it isn't, nullify the choice.				*/
    /*									*/
    /********************************************************************/
    if (ar_new_choice)
    {
	l_found_index = pw_get_cs_list_element_index(ar_new_choice,
						     PWr_cs_list_res(ar_pw,K_PW_PRINTER_CONTENTS_MAP).ar_cs_list,
						     PWr_cs_list_res(ar_pw,K_PW_PRINTER_CONTENTS_MAP).l_cs_count);
	if (l_found_index < 0)
	    ar_new_choice = NULL;
    }
    
    /********************************************************************/
    /*									*/
    /* If there isn't a current printer choice, try to find the default	*/
    /* printer (i.e. SYS$PRINT on VMS or PRINTER on ULTRIX)		*/
    /*									*/
    /********************************************************************/
    if (!ar_new_choice)
    {
	/****************************************************************/
	/*								*/
	/* If there is a default printer, look for it in the queue list.*/
	/*								*/
	/****************************************************************/
	if (PWr_cs_res(ar_pw,K_PW_DEFAULT_PRINTER_MAP).ar_current_cs)
	{
	    l_found_index = pw_get_cs_list_element_index(PWr_cs_res(ar_pw,K_PW_DEFAULT_PRINTER_MAP).ar_current_cs,
							 PWr_cs_list_res(ar_pw,K_PW_PRINTER_CONTENTS_MAP).ar_cs_list,
							 PWr_cs_list_res(ar_pw,K_PW_PRINTER_CONTENTS_MAP).l_cs_count);
	    if (l_found_index >= 0)
		ar_new_choice = PWr_cs_list_res(ar_pw,K_PW_PRINTER_CONTENTS_MAP).ar_cs_list[l_found_index];
	}
    }
    
    /********************************************************************/
    /*									*/
    /* If there still isn't a current printer choice, default to the	*/
    /* first element in the list (if it exists).			*/
    /*									*/
    /********************************************************************/
    if (!ar_new_choice)
    {
	if (PWr_cs_list_res(ar_pw,K_PW_PRINTER_CONTENTS_MAP).l_cs_count)
	{
	    ar_new_choice = PWr_cs_list_res(ar_pw,K_PW_PRINTER_CONTENTS_MAP).ar_cs_list[0];
	    l_found_index = 0;
	}
	else
	    ar_new_choice = NULL;	
    }

    /********************************************************************/
    /*									*/
    /* If the user hasn't previously selected a printer queue for this	*/
    /* print format, make sure we save it now.				*/
    /*									*/
    /********************************************************************/
    if ((PWal_current_format_queue_choices(ar_pw)[PWl_print_format_choice(ar_pw)] == -1) &&
	ar_new_choice)
	PWal_current_format_queue_choices(ar_pw)[PWl_print_format_choice(ar_pw)] = 
	    (PWl_current_queue_list_index(ar_pw) << 16) + l_found_index;
        
    /********************************************************************/
    /*									*/
    /* If the strings are the same, return FALSE which indicates a new	*/
    /* choice wasn't made.  Otherwise, return TRUE.			*/
    /*									*/
    /********************************************************************/
    if (pw_xm_string_compare(ar_new_choice,PWr_cs_res(ar_pw,K_PW_PRINTER_CHOICE_MAP).ar_current_cs))
	b_return_value = FALSE;
    else
	b_return_value = TRUE;
    
    /********************************************************************/
    /*									*/
    /* Make sure the choice points to an element in the printer queue	*/
    /* list.								*/
    /*									*/
    /********************************************************************/

    if (ar_new_choice)
    {
	l_choice_index = pw_get_cs_list_element_index(ar_new_choice,
						  PWr_cs_list_res(ar_pw,K_PW_PRINTER_CONTENTS_MAP).ar_cs_list,
						  PWr_cs_list_res(ar_pw,K_PW_PRINTER_CONTENTS_MAP).l_cs_count);
	
	PWr_cs_res(ar_pw,K_PW_PRINTER_CHOICE_MAP).ar_current_cs = 
	    PWr_cs_list_res(ar_pw,K_PW_PRINTER_CONTENTS_MAP).ar_cs_list[l_choice_index];
    }

    return b_return_value;
    
} /* pw_decide_printer_queue_choice */


/************************************************************************/
/*									*/
/* pw_populate_printer_queue_listbox					*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*      Sets the contents and choice of the printer queue list box.	*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		The print widget.				*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_populate_printer_queue_listbox(ar_pw)
    DXmPrintWgt	ar_pw;
{
    int	l_i;
    
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_populate_printer_queue_list_box\n");
#endif

    /********************************************************************/
    /*									*/
    /* Free up the old list.						*/
    /*									*/
    /********************************************************************/
    if (PWr_cs_list_res(ar_pw,K_PW_PRINTER_CONTENTS_MAP).ar_cs_list)
    {
	pw_free_cs_list_ref_memory(PWr_cs_list_res(ar_pw,K_PW_PRINTER_CONTENTS_MAP).ar_cs_list,
				   PWr_cs_list_res(ar_pw,K_PW_PRINTER_CONTENTS_MAP).l_cs_count);
	XtFree((char *)PWr_cs_list_res(ar_pw,K_PW_PRINTER_CONTENTS_MAP).ar_cs_list);
    }

    /********************************************************************/
    /*									*/
    /* Use the new list.  The new list is set by the			*/
    /* l_current_queue_list_index field in the print widget.		*/
    /*									*/
    /********************************************************************/
    l_i = PWl_current_queue_list_index(ar_pw);
    pw_copy_cs_list(PWar_known_queue_lists(ar_pw)[l_i].ar_cs_list,
		    &PWr_cs_list_res(ar_pw,K_PW_PRINTER_CONTENTS_MAP).ar_cs_list,
		    &PWar_known_queue_lists(ar_pw)[l_i].l_cs_count);
    
    PWr_cs_list_res(ar_pw,K_PW_PRINTER_CONTENTS_MAP).l_cs_count = 
	PWar_known_queue_lists(ar_pw)[l_i].l_cs_count;
    
    /********************************************************************/
    /*									*/
    /* Decide upon the correct printer queue.				*/
    /*									*/
    /********************************************************************/
    pw_decide_printer_queue_choice(ar_pw);
    
    /********************************************************************/
    /*									*/
    /* Populate the printer queue list box.				*/
    /*									*/
    /********************************************************************/
#if 0
    [[[ You've got me why this macro doesn't work... ]]]

    PW_SET_LISTBOX_WIDGET_CONTENTS_FROM_RESOURCE(ar_pw,K_PW_PRINTER_CONTENTS_MAP)
#else
    pw_set_listbox_widget_contents_from_resource(ar_pw,K_PW_PRINTER_CONTENTS_MAP);
#endif

} /* pw_populate_printer_queue_listbox */


/************************************************************************/
/*									*/
/* pw_propagate_print_format_choice					*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*      Looks at the print format choice and sets the queue list box,	*/
/*	pass all, and automatic pagination choices.			*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		The print widget.				*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_propagate_print_format_choice(ar_pw)
    DXmPrintWgt	ar_pw;
{
    int		l_new_queue_list_index;
    int		l_new_print_format_choice;
    Boolean	b_print_format_changed;
    
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_propagate_print_format_choice\n");
#endif

    /********************************************************************/
    /*									*/
    /* Find the index of the choice in the list of known formats.	*/
    /*									*/
    /********************************************************************/
    l_new_print_format_choice =
	pw_get_cs_list_element_index(PWr_cs_res(ar_pw,K_PW_PRINT_FORMAT_CHOICE_MAP).ar_current_cs,
				     PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).ar_cs_list,
				     PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).l_cs_count);

    b_print_format_changed = (PWl_print_format_choice(ar_pw) != l_new_print_format_choice);
    PWl_print_format_choice(ar_pw) = l_new_print_format_choice;
    
    /********************************************************************/
    /*									*/
    /* Use the print format variable to find the printer queue list	*/
    /*									*/
    /********************************************************************/
    l_new_queue_list_index = 
	pw_get_queue_list_from_variable(ar_pw,
					PWr_cs_list_res(ar_pw,
							K_PW_KNOWN_FORMAT_VARIABLE_LIST_MAP).ar_cs_list[PWl_print_format_choice(ar_pw)]);
        
    /********************************************************************/
    /*									*/
    /* If the queue lists are different, re_populate the printer_queue	*/
    /* listbox.  Otherwise, free up the memory used.			*/
    /*									*/
    /********************************************************************/
    if (l_new_queue_list_index != PWl_current_queue_list_index(ar_pw))
    {
	PWl_current_queue_list_index(ar_pw) = l_new_queue_list_index;
	pw_populate_printer_queue_listbox(ar_pw);
    }    
    else
    {
	/****************************************************************/
	/*								*/
	/* Decide upon the correct printer queue.			*/
	/*								*/
	/****************************************************************/
	if (pw_decide_printer_queue_choice(ar_pw))
	    PW_SET_LISTBOX_WIDGET_SELECTION_FROM_RESOURCE(ar_pw,K_PW_PRINTER_CHOICE_MAP);
    }

    /********************************************************************/
    /*									*/
    /* Populate pass all and autopage from format type if the format	*/
    /* has changed since last time.					*/
    /*									*/
    /********************************************************************/
    if (b_print_format_changed)
    {
	switch (PWl_print_format_choice(ar_pw))
	{
	    case DXmPRINT_FORMAT_TEXT:
	    case DXmPRINT_FORMAT_LINE_PRINTER:
	    case DXmPRINT_FORMAT_TERMINAL:
	    {
		PWr_boolean_res(ar_pw,K_PW_PASS_ALL_MAP).b_current_value = FALSE;
		PWr_boolean_res(ar_pw,K_PW_AUTOMATIC_PAGINATION_MAP).b_current_value = TRUE;
		break;
	    }
	
	    case DXmPRINT_FORMAT_ANSI2:
	    case DXmPRINT_FORMAT_ANSI:
	    case DXmPRINT_FORMAT_TEKTRONIX:
	    case DXmPRINT_FORMAT_DDIF:
	    {
		PWr_boolean_res(ar_pw,K_PW_PASS_ALL_MAP).b_current_value = TRUE;
		PWr_boolean_res(ar_pw,K_PW_AUTOMATIC_PAGINATION_MAP).b_current_value = FALSE;
		break;
	    }
	
	    case DXmPRINT_FORMAT_DEFAULT:
	    case DXmPRINT_FORMAT_POSTSCRIPT:
	    case DXmPRINT_FORMAT_REGIS:
	    {
		PWr_boolean_res(ar_pw,K_PW_PASS_ALL_MAP).b_current_value = FALSE;
		PWr_boolean_res(ar_pw,K_PW_AUTOMATIC_PAGINATION_MAP).b_current_value = FALSE;
		break;
	    }

	} /* switch on print format type */

	PW_SET_TOGGLE_WIDGET_FROM_RESOURCE(ar_pw,K_PW_PASS_ALL_MAP);
	PW_SET_TOGGLE_WIDGET_FROM_RESOURCE(ar_pw,K_PW_AUTOMATIC_PAGINATION_MAP);

    } /* if print format changed */
    
} /* pw_propagate_print_format_choice */


/************************************************************************/
/*									*/
/* pw_guess_print_format_choice						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*      Use first filename in filename list to guess on a print format.	*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		The print widget.				*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	TRUE (a new choice was made) FALSE (no choice made)	       	*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static Boolean pw_guess_print_format_choice(ar_pw,aar_format_choice)
    DXmPrintWgt	ar_pw;
    XmString	*aar_format_choice;
{
    Boolean	c_return_status = FALSE;
    int		l_guessed_format_mapping;  /* one of DXmPRINT_FORMAT_xxx */
    int		l_i;
    int		l_known_format_index;
    
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_guess_print_format_choice\n");
#endif

    /********************************************************************/
    /*									*/
    /* Only guess if there are filenames.				*/
    /*									*/
    /********************************************************************/
    if ((PWr_cs_list_res(ar_pw,K_PW_FILENAMES_MAP).l_cs_count) &&
	(PWr_cs_list_res(ar_pw,K_PW_FILENAMES_MAP).ar_cs_list))
    {
	/****************************************************************/
	/*								*/
	/* Call pw_get_guesser_format_mapping which returns an integer 	*/
	/* which represents the format (i.e. text,terminal,line,etc.) 	*/
	/* to choose.							*/
        /*							 	*/
	/****************************************************************/
	pw_get_guesser_format_mapping(PWr_cs_list_res(ar_pw,K_PW_FILENAMES_MAP).ar_cs_list[0],
				      &l_guessed_format_mapping);
	
	/****************************************************************/
	/*								*/
	/* Go through each of the formats in the displayed format list	*/
	/* and see if we can find one that has a guess format index 	*/
	/* which is the same as the guessed format index.  If it does,	*/
	/* return true and gove a pointer to the format choice comp.	*/
	/* string.							*/
	/*								*/
	/****************************************************************/
	for (l_i = 0;
	     (l_i < PWr_cs_list_res(ar_pw,K_PW_PRINT_FORMAT_CONTENTS_MAP).l_cs_count) &&
	     !c_return_status;
	     l_i++)
	{
	    /************************************************************/
	    /*								*/
	    /* Find out which known format this one is.			*/
	    /*								*/
	    /************************************************************/
	    l_known_format_index = 
		pw_get_cs_list_element_index(PWr_cs_list_res(ar_pw,K_PW_PRINT_FORMAT_CONTENTS_MAP).ar_cs_list[l_i],
					     PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).ar_cs_list,
					     PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).l_cs_count);
	    
	    /************************************************************/
	    /*								*/
	    /* See if the guessed format mapping matched the guesser	*/
	    /* format mapping for the known print format.  If it does,	*/
	    /* then the current print format is the one!		*/
	    /*								*/
	    /************************************************************/
	    if (PWr_int_list_res(ar_pw,K_PW_KNOWN_FORMAT_GUESSER_LIST_MAP).al_int_list[l_known_format_index] ==
		l_guessed_format_mapping)
	    {
#ifdef PRINTWGT_DEBUG
		if (PWr_debug(ar_pw).l_current_value)
		    printf("pw_guess_print_format_choice: matches with %d\n",l_i);
#endif
		c_return_status = TRUE;
		*aar_format_choice = PWr_cs_list_res(ar_pw,K_PW_PRINT_FORMAT_CONTENTS_MAP).ar_cs_list[l_i];
	    }
	}
    }

    return c_return_status;
	
} /* pw_guess_print_format_choice */


/************************************************************************/
/*									*/
/* pw_decide_print_format_choice					*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*      Decides on a print format for the current format list.		*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		The print widget.				*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	TRUE (a new choice was made) FALSE (current choice is valid)	*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static Boolean pw_decide_print_format_choice(ar_pw)
    DXmPrintWgt	ar_pw;
{
    XmString ar_new_choice = PWr_cs_res(ar_pw,K_PW_PRINT_FORMAT_CHOICE_MAP).ar_current_cs;
    int	     l_found_index,l_choice_index;
    Boolean  b_return_value;
    
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_decide_print_format_choice\n");
#endif

    /********************************************************************/
    /*									*/
    /* If there is a current print format choice, see if it is in the 	*/
    /* list.  If it isn't, nullify the choice.				*/
    /*									*/
    /********************************************************************/
    if (ar_new_choice)
	if (pw_get_cs_list_element_index(ar_new_choice,
					 PWr_cs_list_res(ar_pw,K_PW_PRINT_FORMAT_CONTENTS_MAP).ar_cs_list,
					 PWr_cs_list_res(ar_pw,K_PW_PRINT_FORMAT_CONTENTS_MAP).l_cs_count) < 0)
	    ar_new_choice = NULL;
    
    /********************************************************************/
    /*									*/
    /* If there isn't a current print format choice, try to guess it	*/
    /* from the filenames resource.					*/
    /*									*/
    /********************************************************************/
    if (!ar_new_choice)
	pw_guess_print_format_choice(ar_pw,&ar_new_choice);
    
    /********************************************************************/
    /*									*/
    /* If there still isn't a current print format choice, go for the	*/
    /* zeroeth element.							*/
    /*									*/
    /********************************************************************/
    if (!ar_new_choice)
	ar_new_choice = PWr_cs_list_res(ar_pw,K_PW_PRINT_FORMAT_CONTENTS_MAP).ar_cs_list[0];

    /********************************************************************/
    /*									*/
    /* If the strings are the same, return FALSE which indicates a new	*/
    /* choice wasn't made.  Otherwise, return TRUE.			*/
    /*									*/
    /********************************************************************/
    if (pw_xm_string_compare(ar_new_choice,PWr_cs_res(ar_pw,K_PW_PRINT_FORMAT_CHOICE_MAP).ar_current_cs))
	b_return_value = FALSE;
    else
	b_return_value = TRUE;
	
    /********************************************************************/
    /*									*/
    /* Make sure the choice points to an element in the print format	*/
    /* list.								*/
    /*									*/
    /********************************************************************/
    l_found_index = pw_get_cs_list_element_index(ar_new_choice,
						 PWr_cs_list_res(ar_pw,K_PW_PRINT_FORMAT_CONTENTS_MAP).ar_cs_list,
						 PWr_cs_list_res(ar_pw,K_PW_PRINT_FORMAT_CONTENTS_MAP).l_cs_count);
	
    PWr_cs_res(ar_pw,K_PW_PRINT_FORMAT_CHOICE_MAP).ar_current_cs = 
	PWr_cs_list_res(ar_pw,K_PW_PRINT_FORMAT_CONTENTS_MAP).ar_cs_list[l_found_index];

    return b_return_value;

} /* pw_decide_print_format_choice */


/************************************************************************/
/*									*/
/* pw_populate_print_format_listbox					*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*      Sets the contents and choice of the print format list box.	*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		The print widget.				*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_populate_print_format_listbox(ar_pw)
    DXmPrintWgt	ar_pw;
{
    
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_populate_print_format_listbox\n");
#endif

    /********************************************************************/
    /*									*/
    /* If the print format contents hasn't been set, do so now and	*/
    /* copy it from the known format list.				*/
    /*									*/
    /********************************************************************/
    if (PWr_cs_list_res(ar_pw,K_PW_PRINT_FORMAT_CONTENTS_MAP).l_cs_count < 0)
    {
	pw_copy_cs_list(PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).ar_cs_list,
			&PWr_cs_list_res(ar_pw,K_PW_PRINT_FORMAT_CONTENTS_MAP).ar_cs_list,
			&PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).l_cs_count);

	PWr_cs_list_res(ar_pw,K_PW_PRINT_FORMAT_CONTENTS_MAP).l_cs_count =
	    PWr_cs_list_res(ar_pw,K_PW_KNOWN_FORMAT_UI_LIST_MAP).l_cs_count; 
    }

    /********************************************************************/
    /*									*/
    /* Decide upon a print format					*/
    /*									*/
    /********************************************************************/
    pw_decide_print_format_choice(ar_pw);
    
    /********************************************************************/
    /*									*/
    /* Populate the box with the new choice.				*/
    /*									*/
    /********************************************************************/
#if 0
    [[[ You've got me why this macro doesn't work... ]]]

    PW_SET_LISTBOX_WIDGET_CONTENTS_FROM_RESOURCE(ar_pw,K_PW_PRINT_FORMAT_CONTENTS_MAP)
#else
    pw_set_listbox_widget_contents_from_resource(ar_pw,K_PW_PRINT_FORMAT_CONTENTS_MAP);
#endif

    /********************************************************************/
    /*									*/
    /* Now propagate the new choice.					*/
    /*									*/
    /********************************************************************/
    pw_propagate_print_format_choice(ar_pw);
    
} /* pw_populate_print_format_listbox */


/************************************************************************/
/*									*/
/* pw_handle_suppress_options_mask					*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*      Handles the suppression of print widget options.		*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		The print widget.				*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
#define PW_SUPPRESS_UNSUPPRESS(w,mask,field) \
{\
    if (w)\
    {\
        if (((mask) & (field)) == (field))\
	{\
	    if (XtIsManaged(w))\
		 XtUnmanageChild(w);\
	}\
	else\
	{\
	    if (!XtIsManaged(w))\
		XtManageChild(w);\
	}\
    }\
}
	
static void pw_handle_suppress_options_mask(ar_pw)
    DXmPrintWgt	ar_pw;
{
    unsigned long int	l_local_mask;
    int			form_width;
    XmFormWidgetClass	formclass;
    XmRowColumnWidgetClass rcclass;
    Widget		w;
    
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_handle_suppress_options_mask\n");
#endif

#ifdef VMS
    l_local_mask = PWr_int_res(ar_pw,K_PW_SUPPRESS_OPTIONS_MASK_MAP).l_current_value;
#else
    l_local_mask = PWr_int_res(ar_pw,K_PW_SUPPRESS_OPTIONS_MASK_MAP).l_current_value |
	           DXmSUPPRESS_AUTOMATIC_PAGINATION |
		   DXmSUPPRESS_DOUBLE_SPACING |
		   DXmSUPPRESS_HOLD_JOB |
		   DXmSUPPRESS_SETUP |
		   DXmSUPPRESS_PRIORITY | 
		   DXmSUPPRESS_OPERATOR_MESSAGE |
		   DXmSUPPRESS_FILE_START_SHEET |
		   DXmSUPPRESS_FILE_BURST_SHEET |
		   DXmSUPPRESS_FILE_END_SHEET |
#ifdef __osf__
		   DXmSUPPRESS_PAGE_RANGE |
		   DXmSUPPRESS_PAGE_SIZE |
		   DXmSUPPRESS_NUMBER_UP |
		   DXmSUPPRESS_SHEET_COUNT |
		   DXmSUPPRESS_MESSAGE_LOG |
		   DXmSUPPRESS_SHEET_SIZE |
		   DXmSUPPRESS_INPUT_TRAY |
		   DXmSUPPRESS_OUTPUT_TRAY |
		   DXmSUPPRESS_LAYUP_DEFINITION |
#endif
		   DXmSUPPRESS_PRINTER_FORM;
#endif

    /*
    /*
    **	Fix for CLD UVO01299, forms are not resizing properly when options are suppressed.
    **	Remove the right attachments from the middle and bottom forms so they can
    **	resize independent of the top form.
    */

    XtVaSetValues(PWar_widget_id(ar_pw, K_PW_MIDDLE_PRIMARY_INFO_FORM), XmNrightAttachment, XmATTACH_NONE, NULL);
    XtVaSetValues(PWar_widget_id(ar_pw, K_PW_BOTTOM_PRIMARY_INFO_FORM), XmNrightAttachment, XmATTACH_NONE, NULL);

    /********************************************************************/
    /*									*/
    /* Top Primary Info Box						*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_TOP_PRIMARY_INFO_FORM),
                           l_local_mask,
			   DXmSUPPRESS_NUMBER_COPIES |
			   DXmSUPPRESS_PAGE_RANGE |
			   DXmSUPPRESS_ORIENTATION);

    /********************************************************************/
    /*									*/
    /* Middle Primary Info Box						*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_MIDDLE_PRIMARY_INFO_FORM),
                           l_local_mask,
			   DXmSUPPRESS_PRINT_FORMAT |
			   DXmSUPPRESS_PRINTER);

    /********************************************************************/
    /*									*/
    /* Bottom Primary Info Box						*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_BOTTOM_PRIMARY_INFO_FORM),
                           l_local_mask,
			   DXmSUPPRESS_PRINT_AFTER |
			   DXmSUPPRESS_DELETE_FILE);
    
    /********************************************************************/
    /*									*/
    /* Virtual Page Box							*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_VIRTUAL_PAGE_BOX),
                           l_local_mask,
			   DXmSUPPRESS_PAGE_SIZE |
			   DXmSUPPRESS_SIDES |
			   DXmSUPPRESS_NUMBER_UP |
			   DXmSUPPRESS_SHEET_COUNT |
			   DXmSUPPRESS_PASS_ALL |
			   DXmSUPPRESS_AUTOMATIC_PAGINATION |
			   DXmSUPPRESS_HEADER |
			   DXmSUPPRESS_DOUBLE_SPACING);
        
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_VIRTUAL_PAGE_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_PAGE_SIZE |
			   DXmSUPPRESS_SIDES |
			   DXmSUPPRESS_NUMBER_UP |
			   DXmSUPPRESS_SHEET_COUNT |
			   DXmSUPPRESS_PASS_ALL |
			   DXmSUPPRESS_AUTOMATIC_PAGINATION |
			   DXmSUPPRESS_HEADER |
			   DXmSUPPRESS_DOUBLE_SPACING);
        
    /********************************************************************/
    /*									*/
    /* Physical Page Box						*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_PHYSICAL_PAGE_BOX),
                           l_local_mask,
			   DXmSUPPRESS_SHEET_SIZE |
			   DXmSUPPRESS_INPUT_TRAY |
			   DXmSUPPRESS_OUTPUT_TRAY |
			   DXmSUPPRESS_FILE_START_SHEET |
			   DXmSUPPRESS_FILE_END_SHEET |
			   DXmSUPPRESS_FILE_BURST_SHEET);
        
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_PHYSICAL_PAGE_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_SHEET_SIZE |
			   DXmSUPPRESS_INPUT_TRAY |
			   DXmSUPPRESS_OUTPUT_TRAY |
			   DXmSUPPRESS_FILE_START_SHEET |
			   DXmSUPPRESS_FILE_END_SHEET |
			   DXmSUPPRESS_FILE_BURST_SHEET);
        
    /********************************************************************/
    /*									*/
    /* Page Layout Box							*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_PAGE_LAYOUT_BOX),
                           l_local_mask,
			   DXmSUPPRESS_LAYUP_DEFINITION |
			   DXmSUPPRESS_SETUP |
			   DXmSUPPRESS_PRINTER_FORM);
        
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_PAGE_LAYOUT_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_LAYUP_DEFINITION |
			   DXmSUPPRESS_SETUP |
			   DXmSUPPRESS_PRINTER_FORM);
        
    /********************************************************************/
    /*									*/
    /* Job Parameters Box						*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_JOB_PARAMETERS_BOX),
                           l_local_mask,
			   DXmSUPPRESS_NOTIFY |
			   DXmSUPPRESS_MESSAGE_LOG |
			   DXmSUPPRESS_START_SHEET_COMMENT |
			   DXmSUPPRESS_JOB_NAME |
			   DXmSUPPRESS_OPERATOR_MESSAGE |
			   DXmSUPPRESS_HOLD_JOB |
			   DXmSUPPRESS_PRIORITY);
            
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_JOB_PARAMETERS_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_NOTIFY |
			   DXmSUPPRESS_MESSAGE_LOG |
			   DXmSUPPRESS_START_SHEET_COMMENT |
			   DXmSUPPRESS_JOB_NAME |
			   DXmSUPPRESS_OPERATOR_MESSAGE |
			   DXmSUPPRESS_HOLD_JOB |
			   DXmSUPPRESS_PRIORITY);
            
    /********************************************************************/
    /*									*/
    /* Number Copies							*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_NUMBER_COPIES_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_NUMBER_COPIES);

    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_NUMBER_COPIES),
                           l_local_mask,
			   DXmSUPPRESS_NUMBER_COPIES);

    /********************************************************************/
    /*									*/
    /* Page Range							*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_PAGE_RANGE_FROM_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_PAGE_RANGE);

    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_PAGE_RANGE_FROM),
                           l_local_mask,
			   DXmSUPPRESS_PAGE_RANGE);

    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_PAGE_RANGE_TO_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_PAGE_RANGE);

    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_PAGE_RANGE_TO),
                           l_local_mask,
			   DXmSUPPRESS_PAGE_RANGE);

    /********************************************************************/
    /*									*/
    /* Print Format							*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_PRINT_FORMAT_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_PRINT_FORMAT);

    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_PRINT_FORMAT),
                           l_local_mask,
			   DXmSUPPRESS_PRINT_FORMAT);

    /********************************************************************/
    /*									*/
    /* Orientation							*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_ORIENTATION_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_ORIENTATION);

    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_ORIENTATION),
                           l_local_mask,
			   DXmSUPPRESS_ORIENTATION);

    /********************************************************************/
    /*									*/
    /* Printer								*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_PRINTER_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_PRINTER);

    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_PRINTER),
                           l_local_mask,
			   DXmSUPPRESS_PRINTER);

    /********************************************************************/
    /*									*/
    /* Print After							*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_PRINT_AFTER_LABEL),
			   l_local_mask,
			   DXmSUPPRESS_PRINT_AFTER);

    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_PRINT_AFTER),
			   l_local_mask,
			   DXmSUPPRESS_PRINT_AFTER);

    /********************************************************************/
    /*									*/
    /* Delete File							*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_DELETE_FILE),
                           l_local_mask,
			   DXmSUPPRESS_DELETE_FILE);

    /********************************************************************/
    /*									*/
    /* Page Size							*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_PAGE_SIZE),
                           l_local_mask,
			   DXmSUPPRESS_PAGE_SIZE);

    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_PAGE_SIZE_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_PAGE_SIZE);

    /********************************************************************/
    /*									*/
    /* Sides								*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_SIDES),
                           l_local_mask,
			   DXmSUPPRESS_SIDES);

    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_SIDES_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_SIDES);

    /********************************************************************/
    /*									*/
    /* Number Up							*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_NUMBER_UP_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_NUMBER_UP);

    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_NUMBER_UP),
                           l_local_mask,
			   DXmSUPPRESS_NUMBER_UP);

    /********************************************************************/
    /*									*/
    /* Sheet Count							*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_SHEET_COUNT_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_SHEET_COUNT);

    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_SHEET_COUNT),
                           l_local_mask,
			   DXmSUPPRESS_SHEET_COUNT);

    /********************************************************************/
    /*									*/
    /* File Start Sheet							*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_FILE_START_SHEET),
                           l_local_mask,
			   DXmSUPPRESS_FILE_START_SHEET);

    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_FILE_START_SHEET_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_FILE_START_SHEET);

    /********************************************************************/
    /*									*/
    /* File End Sheet							*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_FILE_END_SHEET),
                           l_local_mask,
			   DXmSUPPRESS_FILE_END_SHEET);

    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_FILE_END_SHEET_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_FILE_END_SHEET);

    /********************************************************************/
    /*									*/
    /* File Burst Sheet							*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_FILE_BURST_SHEET),
                           l_local_mask,
			   DXmSUPPRESS_FILE_BURST_SHEET);

    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_FILE_BURST_SHEET_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_FILE_BURST_SHEET);

    /********************************************************************/
    /*									*/
    /* Message Log							*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_MESSAGE_LOG),
                           l_local_mask,
			   DXmSUPPRESS_MESSAGE_LOG);

    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_MESSAGE_LOG_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_MESSAGE_LOG);

    /********************************************************************/
    /*									*/
    /* Hold Job								*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_HOLD_JOB),
                           l_local_mask,
			   DXmSUPPRESS_HOLD_JOB);

    /********************************************************************/
    /*									*/
    /* Notify								*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_NOTIFY),
                           l_local_mask,
			   DXmSUPPRESS_NOTIFY);

    /********************************************************************/
    /*									*/
    /* Sheet Size							*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_SHEET_SIZE),
                           l_local_mask,
			   DXmSUPPRESS_SHEET_SIZE);

    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_SHEET_SIZE_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_SHEET_SIZE);

    /********************************************************************/
    /*									*/
    /* Input Tray							*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_INPUT_TRAY),
                           l_local_mask,
			   DXmSUPPRESS_INPUT_TRAY);

    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_INPUT_TRAY_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_INPUT_TRAY);

    /********************************************************************/
    /*									*/
    /* Output Tray							*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_OUTPUT_TRAY),
                           l_local_mask,
			   DXmSUPPRESS_OUTPUT_TRAY);

    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_OUTPUT_TRAY_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_OUTPUT_TRAY);

    /********************************************************************/
    /*									*/
    /* Job Name								*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_JOB_NAME_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_JOB_NAME);

    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_JOB_NAME),
                           l_local_mask,
			   DXmSUPPRESS_JOB_NAME);

    /********************************************************************/
    /*									*/
    /* Operator Message							*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_OPERATOR_MESSAGE_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_OPERATOR_MESSAGE);

    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_OPERATOR_MESSAGE),
                           l_local_mask,
			   DXmSUPPRESS_OPERATOR_MESSAGE);

    /********************************************************************/
    /*									*/
    /* Header								*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_HEADER),
                           l_local_mask,
			   DXmSUPPRESS_HEADER);

    /********************************************************************/
    /*									*/
    /* Automatic Pagination						*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_AUTOMATIC_PAGINATION),
                           l_local_mask,
			   DXmSUPPRESS_AUTOMATIC_PAGINATION);

    /********************************************************************/
    /*									*/
    /* Double Spacing							*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_DOUBLE_SPACING),
                           l_local_mask,
			   DXmSUPPRESS_DOUBLE_SPACING);

    /********************************************************************/
    /*									*/
    /* Layup Definition							*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_LAYUP_DEFINITION_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_LAYUP_DEFINITION);

    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_LAYUP_DEFINITION),
                           l_local_mask,
			   DXmSUPPRESS_LAYUP_DEFINITION);

    /********************************************************************/
    /*									*/
    /* Start Sheet Comment						*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_START_SHEET_COMMENT_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_START_SHEET_COMMENT);

    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_START_SHEET_COMMENT),
                           l_local_mask,
			   DXmSUPPRESS_START_SHEET_COMMENT);

    /********************************************************************/
    /*									*/
    /* Pass All								*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_PASS_ALL),
                           l_local_mask,
			   DXmSUPPRESS_PASS_ALL);

    /********************************************************************/
    /*									*/
    /* Printer Form							*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_PRINTER_FORM_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_PRINTER_FORM);

    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_PRINTER_FORM),
                           l_local_mask,
			   DXmSUPPRESS_PRINTER_FORM);

    /********************************************************************/
    /*									*/
    /* Priority								*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_PRIORITY_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_PRIORITY);

    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_PRIORITY),
                           l_local_mask,
			   DXmSUPPRESS_PRIORITY);

    /********************************************************************/
    /*									*/
    /* Setup								*/
    /*									*/
    /********************************************************************/
    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_SETUP_LABEL),
                           l_local_mask,
			   DXmSUPPRESS_SETUP);

    PW_SUPPRESS_UNSUPPRESS(PWar_widget_id(ar_pw,K_PW_SETUP),
                           l_local_mask,
			   DXmSUPPRESS_SETUP);

    /********************************************************************/
    /*									*/
    /* Shrink or grow secondary box as necessary.			*/
    /*									*/
    /********************************************************************/
    pw_handle_form_dialog(ar_pw,
			  PWar_widget_id(ar_pw,K_PW_2NDARY_FORM),
			  PWar_widget_id(ar_pw,K_PW_2NDARY_WINDOW));

    /*
    **	Fix for CLD UVO01299, forms are not resizing properly when options are suppressed.
    **  If the forms are not realized, call the change_managed function to force them to
    **  set their width.
    */
    formclass = (XmFormWidgetClass) xmFormWidgetClass;
    rcclass = (XmRowColumnWidgetClass) xmRowColumnWidgetClass;

    /*
    **	Must force top_primary_label_form to size first because it affects the size of top_primary_info_form.
    */
    w = XtNameToWidget(PWar_widget_id(ar_pw, K_PW_TOP_PRIMARY_INFO_FORM), "top_primary_label_form");
    if (!XtIsRealized(w))
	(*formclass->composite_class.change_managed) ((Widget) w);
    if (!XtIsRealized((Widget) PWar_widget_id(ar_pw, K_PW_ORIENTATION)))
	(*rcclass->composite_class.change_managed) ((Widget) PWar_widget_id(ar_pw, K_PW_ORIENTATION));

    if (!XtIsRealized(PWar_widget_id(ar_pw, K_PW_TOP_PRIMARY_INFO_FORM)))
	(*formclass->composite_class.change_managed) ((Widget) PWar_widget_id(ar_pw, K_PW_TOP_PRIMARY_INFO_FORM));
    if (!XtIsRealized(PWar_widget_id(ar_pw, K_PW_MIDDLE_PRIMARY_INFO_FORM)))
	(*formclass->composite_class.change_managed) ((Widget) PWar_widget_id(ar_pw, K_PW_MIDDLE_PRIMARY_INFO_FORM));
    if (!XtIsRealized(PWar_widget_id(ar_pw, K_PW_BOTTOM_PRIMARY_INFO_FORM)))
	(*formclass->composite_class.change_managed) ((Widget) PWar_widget_id(ar_pw, K_PW_BOTTOM_PRIMARY_INFO_FORM));

    /*
    **	Get the width of the widest form
    */
    form_width = XtWidth(PWar_widget_id(ar_pw, K_PW_TOP_PRIMARY_INFO_FORM));
    form_width = MAX( form_width, XtWidth(PWar_widget_id(ar_pw, K_PW_MIDDLE_PRIMARY_INFO_FORM)) );
    form_width = MAX( form_width, XtWidth(PWar_widget_id(ar_pw, K_PW_BOTTOM_PRIMARY_INFO_FORM)) );

    /*
    **	Set the width of the top form
    */
    if (form_width != XtWidth(PWar_widget_id(ar_pw, K_PW_TOP_PRIMARY_INFO_FORM)))
	XtVaSetValues(PWar_widget_id(ar_pw, K_PW_TOP_PRIMARY_INFO_FORM), XmNwidth, form_width, NULL);

    /*
    **	Reattach the right sides of the middle and bottom forms to the top form.
    */

    XtVaSetValues(PWar_widget_id(ar_pw, K_PW_MIDDLE_PRIMARY_INFO_FORM),
	XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
	XmNrightWidget, PWar_widget_id(ar_pw, K_PW_TOP_PRIMARY_INFO_FORM), 
	NULL);
    XtVaSetValues(PWar_widget_id(ar_pw, K_PW_BOTTOM_PRIMARY_INFO_FORM),
	XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
	XmNrightWidget, PWar_widget_id(ar_pw, K_PW_TOP_PRIMARY_INFO_FORM), 
	NULL);

} /* pw_handle_suppress_options_mask */


/************************************************************************/
/*									*/
/* pw_turn_on_wait_cursor						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*      Turns on the print widget wait cursor.				*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		The print widget.				*/
/*									*/
/*	w		The widget which needs wait cursor.		*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_turn_on_wait_cursor(ar_pw,ar_w)
    DXmPrintWgt	ar_pw;
    Widget	ar_w;
{
		
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_turn_on_wait_cursor\n");
#endif

    if (ar_w)
        if (XtIsManaged(ar_w))
        {
	    XDefineCursor(XtDisplay(ar_w),
		          XtWindow(ar_w),
		          PWr_wait_cursor(ar_pw));
            XFlush(XtDisplay(ar_w));
        }
    
} /* pw_turn_on_wait_cursor */


/************************************************************************/
/*									*/
/* pw_turn_off_wait_cursor						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*      Turns off the print widget wait cursor.				*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		The print widget.				*/
/*									*/
/*	w		The widget which needs wait cursor.		*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_turn_off_wait_cursor(ar_pw,ar_w)
    DXmPrintWgt	ar_pw;
    Widget	ar_w;
{
		
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_turn_off_wait_cursor\n");
#endif

    if (ar_w)
        if (XtIsManaged(ar_w))
	    XUndefineCursor(XtDisplay(ar_w),
		            XtWindow(ar_w));

} /* pw_turn_off_wait_cursor */
    

/************************************************************************/
/*									*/
/* pw_handle_form_dialog						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*      Resizes the scrolled window widget which contains a form	*/
/*	dialog.								*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		The print widget.				*/
/*									*/
/*	form		Form widget.					*/
/*									*/
/*	window		Parent of Form to be resized.			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_handle_form_dialog(ar_pw,ar_form,ar_window)
    DXmPrintWgt	ar_pw;
    Widget	ar_form;
    Widget	ar_window;
{
    Dimension	l_width,l_height;
    Screen	*screen;
    Arg		ar_al[5];

#define    SCROLLWINDOW_DRESSING  2
#define    DISTANCE_TO_EDGE_OF_SCREEN  20

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_handle_form_dialog: ");
#endif

    /********************************************************************/
    /*									*/
    /* Just in case...							*/
    /*									*/
    /********************************************************************/
    if ((!ar_form) || (!ar_window))
	return;
    
    /********************************************************************/
    /*									*/
    /* Find out how big the form widget is.				*/
    /*									*/
    /********************************************************************/
    XtSetArg(ar_al[0], XmNwidth, &l_width);
    XtSetArg(ar_al[1], XmNheight, &l_height);
    XtGetValues(ar_form, ar_al, 2);

    /********************************************************************/
    /*									*/
    /* Make the scrollwindow the correct size.  Add 			*/
    /* SCROLLWINDOW_DRESSING so the scrollbars do not come up by default*/
    /* since the scrollwindow compares it's size (minus thickness) to 	*/
    /* the form to decide whether or not to display the scrollbars.	*/
    /* Also, make sure this fits on the screen (with some margin	*/
    /* "DISTANCE_TO_EDGE_OF_SCREEN").					*/
    /*									*/
    /********************************************************************/
    screen = (Screen *) XtScreen(ar_pw);
    
    l_width = MIN(l_width + (SCROLLWINDOW_DRESSING * 2),
                  WidthOfScreen(screen) -
	          (DISTANCE_TO_EDGE_OF_SCREEN * 2));
    
    l_height = MIN(l_height + (SCROLLWINDOW_DRESSING * 2),
		   HeightOfScreen(screen) -
		   (DISTANCE_TO_EDGE_OF_SCREEN * 2));

    XtSetArg(ar_al[0],XmNwidth,l_width);
    XtSetArg(ar_al[1],XmNheight,l_height);
    XtSetValues(ar_window,ar_al,2);

#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("set width and height to %d,%d\n",l_width,l_height);
#endif

} /* pw_handle_form_dialog */


/************************************************************************/
/*									*/
/* pw_validate_startup_values						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine performs validations on the resources.		*/
/*	It meant to be used at startup time and does not cause a	*/
/*	message box to pop up.  If a resource has an invalid value,	*/
/*	it's value is set to default.					*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*			     						*/
/*	from		beginning resource index			*/
/*									*/
/* 	to		end resource index (inclusive)			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static Boolean pw_validate_startup_values(ar_pw,l_from,l_to)
    DXmPrintWgt		ar_pw;
    int			l_from;
    int			l_to;
{
    Boolean 	l_return_status = TRUE;
    int		l_i;
    
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_validate_startup_values from %d to %d\n",l_from,l_to);
#endif

    for (l_i = l_from; l_i <= l_to; l_i++)
    {
	/****************************************************************/
	/*								*/
	/* Validate the value if applicable.				*/
	/*								*/
	/****************************************************************/
	if ((pw_ar_resource_control[l_i].validate_value_proc) &&
	    !(*(pw_ar_resource_control[l_i].validate_value_proc))(ar_pw,
							          l_i))
	{
	    l_return_status = FALSE;
	    
#ifdef PRINTWGT_DEBUG
	    if (PWr_debug(ar_pw).l_current_value)
		printf("pw_validate_startup_values:  Resource %d invalid.  Setting to default.\n",l_i);
#endif

	    switch (pw_ar_resource_control[l_i].l_type)
	    {
		case K_PW_RESOURCE_TYPE_INT : 
		case K_PW_RESOURCE_TYPE_CS_INT : 
		case K_PW_RESOURCE_TYPE_RADIOBOX : 
		case K_PW_RESOURCE_TYPE_TOGGLE : 
		case K_PW_RESOURCE_TYPE_OPTIONMENU : 
		{
		    if (l_i == K_PW_NOTIFY_MAP)
			PWr_boolean_res(ar_pw,l_i).b_current_value = TRUE;
		    else
			PWr_boolean_res(ar_pw,l_i).b_current_value = FALSE;

		    break;
		}
		    
		case K_PW_RESOURCE_TYPE_CS : 
		{
		    if (PWr_cs_res(ar_pw,l_i).ar_current_cs)
			XmStringFree(PWr_cs_res(ar_pw,l_i).ar_current_cs);

		    if (l_i == K_PW_PRINT_AFTER_MAP)
			PWr_cs_res(ar_pw,l_i).ar_current_cs = XmStringCopy(PWar_now_string(ar_pw));
		    else
			PWr_cs_res(ar_pw,l_i).ar_current_cs = NULL;

		    break;
		}
		    
		case K_PW_RESOURCE_TYPE_CS_CHOICE : 
		{
		    /****************************************************/
		    /*							*/
		    /* Ignore bad format and queue choices since	*/
		    /* these will get caught later.			*/
		    /*							*/
		    /****************************************************/
		    if ((l_i != K_PW_PRINT_FORMAT_CHOICE_MAP) &&
		        (l_i != K_PW_PRINTER_CHOICE_MAP))
			PWr_cs_res(ar_pw,l_i).ar_current_cs = NULL;

		    break;
		}
		
		case K_PW_RESOURCE_TYPE_CS_LIST :
		{
		    if (PWr_cs_list_res(ar_pw,l_i).ar_cs_list)
		    {
			pw_free_cs_list_ref_memory(PWr_cs_list_res(ar_pw,l_i).ar_cs_list,
					           PWr_cs_list_res(ar_pw,l_i).l_cs_count);
			XtFree((char *)PWr_cs_list_res(ar_pw,l_i).ar_cs_list);
			PWr_cs_list_res(ar_pw,l_i).l_cs_count = 0;
		    }

		    if (l_i == K_PW_PRINT_FORMAT_CONTENTS_MAP)
			PWr_cs_list_res(ar_pw,l_i).l_cs_count = -1;

		    break;
		}
		
		default :  break;
		
	    } /* switch */
	}
    } /* for */
		
    return (l_return_status);
 
} /* pw_validate_startup_values */


/************************************************************************/
/*									*/
/* pw_set_listbox_widget_selection_from_resource			*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	Just a way to test a macro.					*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*			     						*/
/*	resource_index	Resource index					*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
#if 0
static void pw_set_listbox_widget_selection_from_resource(ar_pw,l_resource_index)
    DXmPrintWgt ar_pw;
    int		l_resource_index;
{
    if (PWar_widget_id(ar_pw,
                       pw_ar_resource_control[l_resource_index].l_widget_index))
    {
        XmListDeselectAllItems(PWar_widget_id(ar_pw,
			  	              pw_ar_resource_control[l_resource_index].l_widget_index));

        if (PWr_cs_res(ar_pw,l_resource_index).ar_current_cs)
        {
	    XmListSelectItem(PWar_widget_id(ar_pw,
		 	    		    pw_ar_resource_control[l_resource_index].l_widget_index),
			     PWr_cs_res(ar_pw,l_resource_index).ar_current_cs,
			     FALSE);
	    XmListSetItem(PWar_widget_id(ar_pw,
		 		         pw_ar_resource_control[l_resource_index].l_widget_index),
			  PWr_cs_res(ar_pw,l_resource_index).ar_current_cs);
        }
    }
} /* pw_set_listbox_widget_selection_from_resource */
#endif


/************************************************************************/
/*									*/
/* pw_set_listbox_widget_contents_from_resource				*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	Just a way to test a macro.					*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*			     						*/
/*	resource_index	Resource index					*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_set_listbox_widget_contents_from_resource(ar_pw,l_resource_index)
    DXmPrintWgt ar_pw;
    int		l_resource_index;
{
    if (PWar_widget_id(ar_pw,
		       pw_ar_resource_control[l_resource_index].l_widget_index))
    {
	Arg	ar_al[4];
	int	l_ac;
	l_ac = 0;
	XtSetArg(ar_al[l_ac],
		 XmNitems,
		 PWr_cs_list_res(ar_pw,l_resource_index).ar_cs_list); l_ac++;
	XtSetArg(ar_al[l_ac],
		 XmNitemCount,
		 PWr_cs_list_res(ar_pw,l_resource_index).l_cs_count); l_ac++;
	XtSetArg(ar_al[l_ac],
		 XmNselectedItemCount,
		 0); l_ac++;
	XmListDeselectAllItems(PWar_widget_id(ar_pw,
					      pw_ar_resource_control[l_resource_index].l_widget_index));
	XtSetValues(PWar_widget_id(ar_pw,
				   pw_ar_resource_control[l_resource_index].l_widget_index),
		    ar_al,
		    l_ac);
	if (PWr_cs_res(ar_pw,l_resource_index + 1).ar_current_cs)
	{
	    XmListSelectItem(PWar_widget_id(ar_pw,
					    pw_ar_resource_control[l_resource_index].l_widget_index),
			     PWr_cs_res(ar_pw,l_resource_index + 1).ar_current_cs,
			     FALSE);
	    XmListSetItem(PWar_widget_id(ar_pw,
					 pw_ar_resource_control[l_resource_index].l_widget_index),
			  PWr_cs_res(ar_pw,l_resource_index + 1).ar_current_cs);
	}
    }
} /* pw_set_listbox_widget_contents_from_resource */


/************************************************************************/
/*									*/
/* pw_adjust_application_child_location					*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	Handles the insertion of a child.  This will move an		*/
/*	application's child to the point just below the orientation	*/
/*	button.								*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	ar_pw		Print Widget.					*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_adjust_application_child_location(ar_pw)
    DXmPrintWgt	ar_pw;
{   
    int		l_i;
    Arg		ar_al[2];
    
#ifdef PRINTWGT_DEBUG
    if (PWr_debug(ar_pw).l_current_value)
	printf("pw_adjust_application_child_location\n");
#endif

    for (l_i = 0; l_i < ar_pw->composite.num_children; l_i++)
    {
	/****************************************************************/
	/*								*/
	/* See if this child is interesting.  It is interesting if it	*/
	/* it has a name that is not the same as the primary or		*/
	/* secondary dialog boxes.					*/
	/*								*/
	/****************************************************************/
	if (ar_pw->composite.children[l_i]->core.name)
	{
	    if (strcmp(ar_pw->composite.children[l_i]->core.name,K_PW_PRIMARY_BOX_NAME) &&
		strcmp(ar_pw->composite.children[l_i]->core.name,K_PW_2NDARY_BOX_NAME))
	    {
#ifdef PRINTWGT_DEBUG
		if (PWr_debug(ar_pw).l_current_value)
		    printf("found foreign child at %d,%d\n",
			   ar_pw->composite.children[l_i]->core.x,
			   ar_pw->composite.children[l_i]->core.y);
#endif
		
		/********************************************************/
		/*							*/
		/* Move the child if it is at 0,0 or (since this is	*/
		/* being called from the expose routine and the bb will	*/
		/* set its children to fit in the margins) at the	*/
		/* margin boundary.					*/
		/*							*/
		/********************************************************/
		if ((!ar_pw->composite.children[l_i]->core.x &&
		     !ar_pw->composite.children[l_i]->core.y) ||
		    ((ar_pw->composite.children[l_i]->core.x  == ar_pw->bulletin_board.margin_width) &&
		     (ar_pw->composite.children[l_i]->core.y  == ar_pw->bulletin_board.margin_height)))
		{
		    XtSetArg(ar_al[0],XmNx,PWar_widget_id(ar_pw,K_PW_BOTTOM_PRIMARY_INFO_FORM)->core.x +
			                   ar_pw->bulletin_board.margin_width);
		
		    XtSetArg(ar_al[1],XmNy,PWar_widget_id(ar_pw,K_PW_BOTTOM_PRIMARY_INFO_FORM)->core.y +
			                   PWar_widget_id(ar_pw,K_PW_BOTTOM_PRIMARY_INFO_FORM)->core.height +
			                   ar_pw->bulletin_board.margin_height +
			                   12); /* 11 is what is used for delta y in print widget */

		    XtSetValues(ar_pw->composite.children[l_i],ar_al,2);
		}
	    }
	}
    }
    
    return;

} /* pw_adjust_application_child_location */


/************************************************************************/
/*									*/
/* pw_tabify_primary_box						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*      Adjust the tab groups in this box since Motif's default tab	*/
/*	groups just plain suck.						*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	ar_pw		Print Widget.					*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_tabify_primary_box(ar_pw)
    DXmPrintWgt	ar_pw;
{   

    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_NUMBER_COPIES));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_PAGE_RANGE_FROM));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_PAGE_RANGE_TO));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_ORIENTATION));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_PRINT_FORMAT));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_PRINTER));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_PRINT_AFTER));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_DELETE_FILE));

} /* pw_tabify_primary_box */


/************************************************************************/
/*									*/
/* pw_tabify_2ndary_box							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*      Adjust the tab groups in this box since Motif's default tab	*/
/*	groups just plain suck.						*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	ar_pw		Print Widget.					*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_tabify_2ndary_box(ar_pw)
    DXmPrintWgt	ar_pw;
{   

    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_PAGE_SIZE));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_SIDES));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_NUMBER_UP));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_SHEET_COUNT));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_PASS_ALL));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_HEADER));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_DOUBLE_SPACING));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_AUTOMATIC_PAGINATION));

    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_SHEET_SIZE));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_INPUT_TRAY));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_OUTPUT_TRAY));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_FILE_START_SHEET));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_FILE_END_SHEET));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_FILE_BURST_SHEET));

    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_LAYUP_DEFINITION));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_SETUP));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_PRINTER_FORM));

    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_NOTIFY));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_MESSAGE_LOG));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_START_SHEET_COMMENT));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_JOB_NAME));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_OPERATOR_MESSAGE));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_HOLD_JOB));
    XmAddTabGroup(PWar_widget_id(ar_pw,K_PW_PRIORITY));

} /* pw_tabify_2ndary_box */


/************************************************************************/
/*									*/
/* pw_fetch_2ndary_box							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*      Fetch the 2ndary box from UIL.					*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	ar_pw		Print Widget.					*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_fetch_2ndary_box(ar_pw)
    DXmPrintWgt	ar_pw;
{   
    Arg				ar_al[1];
    int				l_ac = 1;
    MrmType	        	ar_dummy_class;

    /********************************************************************/
    /*									*/
    /* Fetch the widget.						*/
    /*									*/
    /********************************************************************/
    XtSetArg(ar_al[0],
	     XmNdialogTitle,
	     PWr_cs_res(ar_pw,K_PW_2NDARY_TITLE_MAP).ar_current_cs);

    if (MrmFetchWidgetOverride (PWar_mrm_hierarchy(ar_pw),		/* MRM heirarchy	*/
				K_PW_2NDARY_BOX_NAME,			/* Name of widget	*/
				(Widget) ar_pw,					/* Parent		*/
				NULL,					/* Override name	*/
				ar_al, l_ac,				/* Override args	*/
				&PWar_widget_id(ar_pw,
						K_PW_2NDARY_BOX),	/* 2ndary box		*/
				&ar_dummy_class) 			/* Class		*/
	!= MrmSUCCESS)
    {
	String params = K_PW_2NDARY_BOX_NAME;
	Cardinal num_params = 1;
	
	DXMAPPWARNING(XtWidgetToApplicationContext((Widget)ar_pw),
		      PWMSGNAME2,
		      PWGTNOMRMFETCH,
		      &params,&num_params);
    }

    /********************************************************************/
    /*									*/
    /* Set the just created flag and then set up the tab groups.	*/
    /*									*/
    /********************************************************************/
    PWl_2ndary_just_created(ar_pw) = TRUE;
    pw_tabify_2ndary_box(ar_pw);

} /* pw_fetch_2ndary_box */
