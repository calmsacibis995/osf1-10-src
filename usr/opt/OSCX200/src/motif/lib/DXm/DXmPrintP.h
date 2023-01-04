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
/*								 	*/
/*   FACILITY:								*/
/*									*/
/*        Print Widget 							*/
/*									*/
/*   ABSTRACT:								*/
/*									*/
/*	This module contains private definitions for the print widget.  */
/*									*/
/*   AUTHORS:								*/
/*									*/
/*	Print Wgt Team	Winter-1990                                     */
/*									*/
/*   CREATION DATE:     Winter-1990					*/
/*									*/
/*   MODIFICATION HISTORY:						*/
/*									*/
/*	020	Will Walker		06-Feb-1991			*/
/*		Add "Default" print format.				*/
/*	019	Will Walker		21-Jan-1991			*/
/*		Make Boolean resources really be Boolean resources.	*/
/*	018	Will Walker		11-Jan-1991			*/
/*		Make DXmN{ok|cancel}selectedCallbackList map to		*/
/*		XmN{ok|cancel}Callback for consistency issues.		*/
/*		Also make DXmNunmapOn{Ok|Cancel}Selected be		*/
/*		DXmNunmanageOn{Ok|Cancel} for more consistency.		*/
/*	017	Will Walker		07-Jan-1991			*/
/*		Add private resources for .UID and Help filenames.      */
/*	016	Will Walker		19-Nov-1990			*/
/*		More Style Guide Compliancy.				*/
/*	015	Will Walker		19-Nov-1990			*/
/*		More Style Guide Compliancy.				*/
/*	014	Will Walker		20-Sep-1990			*/
/*		Add augment list capability.				*/
/*	013	Will Walker		17-Aug-1990			*/
/*		Add extra constants for label widgets.			*/
/*	012	Will Walker		15-Aug-1990			*/
/*		Make output tray option objects be specific to OS.	*/
/*	011	Will Walker		09-Aug-1990			*/
/*		Work on I14Y (add some new constants)			*/
/*	010	Will Walker		16-Jul-1990			*/
/*		Add additional params for DXmCvt...			*/
/*	009	Will Walker		15-Jul-1990			*/
/*		Adjust indeces for widgets.				*/
/*	008	Will Walker		13-Jul-1990			*/
/*		I18N work.						*/
/*	007	Will Walker		06-Jul-1990			*/
/*		Put DXmCSText in interface instead of XmText.  Make	*/
/*		resources be XmString's instead of char *'s.		*/
/*	006	Will Walker		28-Jun-1990			*/
/*		Add "wait" cursor support.				*/
/*	005	Will Walker		21-Jun-1990			*/
/*		Make printer list resources public.			*/
/*	004	Will Walker		18-Jun-1990			*/
/*		Add "DXm" prefix to all of the resources.		*/
/*	003	Will Walker		31-May-1990			*/
/*		Add help support.					*/
/*	002	Will Walker		27-Mar-1990			*/
/*		Reorganize.						*/
/*	001	Will Walker		19-Mar-1990			*/
/*		Add modification history.  Change names of resources to	*/
/* 		match Xm naming conventions.				*/
/*									*/
/************************************************************************/
#ifndef _DXmPrintP_h
#define _DXmPrintP_h
#if defined (VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#define PW_TOUPPER(c)	((c) >= 'a' && (c) <= 'z' ? (c) & 0x5F:(c))

/************************************************************************/
/*									*/
/* INCLUDE FILES							*/
/*									*/
/************************************************************************/
#include <DXm/DXmPrint.h>

#define DXmIsPrintWidget(w) \
    (XtIsSubclass(w, dxmPrintWgtWidgetClass)) 


/************************************************************************/
/*									*/
/* Names for widgets in the .UID file.					*/
/*									*/
/************************************************************************/
#define K_PW_NOW_STRING_NAME		"now_string"
#define K_PW_ERROR_MESSAGE_TABLE_NAME	"error_message_table"

#define	K_PW_PRIMARY_BOX_NAME	"pw_primary_box"
#define	K_PW_2NDARY_BOX_NAME	"pw_2ndary_box"
#define K_PW_2NDARY_TITLE_NAME	"k_pw_print_options"
#define K_PW_MESSAGE_BOX_NAME	"pw_message"
#define K_PW_HELP_BOX_NAME	"pw_help_box"
#define K_PW_DROP_HELP_NAME     "pw_drop_help_box"

/************************************************************************/
/*									*/
/* The default queue list variable is used for setting the user's base	*/
/* queue list (e.g. suppose they don't want the queue list for the	*/
/* entire system...)  The default queue variable is what the print 	*/
/* widget thinks is the default printer for the user's environment.	*/
/*									*/
/************************************************************************/
#ifdef VMS
#    define K_PW_DEFAULT_QUEUE_LIST_VARIABLE_NAME "vms_default_queue_list_variable"
#    define K_PW_DEFAULT_QUEUE_VARIABLE_NAME	  "vms_default_queue_variable"
#else
#    define K_PW_DEFAULT_QUEUE_LIST_VARIABLE_NAME "ultrix_default_queue_list_varia"
#    define K_PW_DEFAULT_QUEUE_VARIABLE_NAME	  "ultrix_default_queue_variable"
#endif

/************************************************************************/
/*									*/
/* The print format UI table is the table used for the list box		*/
/*									*/
/* The print format OS table is the table used for passing stuff to	*/
/* the operating system.						*/
/*									*/
/* The print format guesser table is used for determing what format to	*/
/* use once a file's type has been guessed.				*/
/* 									*/
/* The print format variable table is used for determining what queues	*/
/* to display for different formats.					*/
/*									*/
/************************************************************************/
#ifdef VMS
#define K_PW_PRINT_FORMAT_UI_TABLE_NAME		"vms_print_format_UI_table"
#define K_PW_PRINT_FORMAT_OS_TABLE_NAME		"vms_print_format_OS_table"
#define K_PW_PRINT_FORMAT_GUESSER_TABLE_NAME	"vms_print_format_guesser_table"
#define K_PW_PRINT_FORMAT_VARIABLE_TABLE_NAME	"vms_print_format_variable_table"
#else
#define K_PW_PRINT_FORMAT_UI_TABLE_NAME		"ultrix_print_format_UI_table"
#define K_PW_PRINT_FORMAT_OS_TABLE_NAME		"ultrix_print_format_OS_table"
#define K_PW_PRINT_FORMAT_GUESSER_TABLE_NAME	"ultrix_print_format_guesser_tab"
#define K_PW_PRINT_FORMAT_VARIABLE_TABLE_NAME	"ultrix_print_format_variable_ta"
#endif

/************************************************************************/
/*									*/
/* Numbers for offsetting resources in widget instance.			*/
/*									*/
/************************************************************************/
#define K_PW_PRIMARY_RESOURCES_START	0	/* Start of resource in Primary Box */
#define K_PW_NUMBER_COPIES_MAP		0
#define K_PW_PAGE_RANGE_FROM_MAP	1
#define K_PW_PAGE_RANGE_TO_MAP		2
#define K_PW_PRINT_FORMAT_CONTENTS_MAP	3
#define K_PW_PRINT_FORMAT_CHOICE_MAP	4
#define K_PW_ORIENTATION_MAP		5
#define K_PW_PRINTER_CONTENTS_MAP	6
#define K_PW_PRINTER_CHOICE_MAP		7
#define K_PW_PRINT_AFTER_MAP		8
#define K_PW_DELETE_FILE_MAP		9
#define K_PW_PRIMARY_RESOURCES_FINISH	9	/* End of resources in primary box */

#define K_PW_2NDARY_RESOURCES_START	10	/* Start of resources in 2ndary box */
#define K_PW_PAGE_SIZE_MAP	        10
#define K_PW_SIDES_MAP			11
#define K_PW_NUMBER_UP_MAP		12
#define K_PW_SHEET_COUNT_MAP		13
#define K_PW_FILE_START_SHEET_MAP	14
#define K_PW_FILE_END_SHEET_MAP		15
#define K_PW_FILE_BURST_SHEET_MAP	16
#define K_PW_MESSAGE_LOG_MAP		17
#define K_PW_HOLD_JOB_MAP		18
#define K_PW_NOTIFY_MAP			19
#define K_PW_SHEET_SIZE_MAP		20
#define K_PW_INPUT_TRAY_MAP		21
#define K_PW_OUTPUT_TRAY_MAP		22
#define K_PW_JOB_NAME_MAP		23
#define K_PW_OPERATOR_MESSAGE_MAP	24
#define K_PW_HEADER_MAP			25
#define K_PW_AUTOMATIC_PAGINATION_MAP	26
#define K_PW_DOUBLE_SPACING_MAP		27
#define K_PW_LAYUP_DEFINITION_MAP	28
#define K_PW_START_SHEET_COMMENT_MAP	29
#define K_PW_PASS_ALL_MAP		30
#define K_PW_PRINTER_FORM_CONTENTS_MAP	31
#define K_PW_PRINTER_FORM_CHOICE_MAP	32
#define K_PW_PRIORITY_MAP		33
#define K_PW_SETUP_MAP			34
#define K_PW_2NDARY_RESOURCES_FINISH	34	/* End of resources in 2ndary box */

#define K_PW_DEBUG_MAP			35	/* Resources which do not have a widget */
#define K_PW_OK_CALLBACK_LIST_MAP	36
#define K_PW_UNMANAGE_ON_OK_MAP		37
#define K_PW_CANCEL_CALLBACK_LIST_MAP	38
#define K_PW_UNMANAGE_ON_CANCEL_MAP	39
#define K_PW_FILENAMES_MAP		40
#define K_PW_KNOWN_FORMAT_UI_LIST_MAP	41
#define K_PW_KNOWN_FORMAT_VARIABLE_LIST_MAP	42
#define K_PW_KNOWN_FORMAT_OS_LIST_MAP	43
#define K_PW_KNOWN_FORMAT_GUESSER_LIST_MAP	44
#define K_PW_DEFAULT_PRINTER_MAP	45
#define K_PW_SUPPRESS_OPTIONS_MASK_MAP	46
#define K_PW_UID_SPEC_MAP		47
#define K_PW_LIBRARY_SPEC_MAP		48
#define K_PW_2NDARY_TITLE_MAP		49
#define K_PW_NUM_RESOURCES		50	/* Total number of resources */

/************************************************************************/
/*									*/
/* Define strings which return tables that each of the option menus	*/
/* will use for determining what to create for the push buttons.	*/
/*									*/
/************************************************************************/
#define K_PW_PROTO_OPTIONMENU_PB_NAME			"proto_optionmenu_pb"
#define K_PW_PROTO_OPTIONMENU_LABEL_NAME		"pw_optionmenu_pb_label"
#define K_PW_PROTO_OPTIONMENU_HELP_TAG_NAME		"pw_optionmenu_pb_help_tag"
#define K_PW_PROTO_OPTIONMENU_ACTIVATE_TAG_NAME		"pw_optionmenu_pb_activate_tag"

#define K_PW_SIDES_OBJECTS_NAME			"sides_objects"
#define K_PW_FILE_START_SHEET_OBJECTS_NAME	"file_start_sheet_objects"
#define K_PW_FILE_END_SHEET_OBJECTS_NAME	"file_end_sheet_objects"
#define K_PW_FILE_BURST_SHEET_OBJECTS_NAME	"file_burst_sheet_objects"
#ifdef VMS
#define K_PW_PAGE_SIZE_OBJECTS_NAME		"vms_page_size_objects"
#define K_PW_MESSAGE_LOG_OBJECTS_NAME		"vms_message_log_objects"
#define K_PW_SHEET_SIZE_OBJECTS_NAME		"vms_sheet_size_objects"
#define K_PW_INPUT_TRAY_OBJECTS_NAME		"vms_input_tray_objects"
#define K_PW_OUTPUT_TRAY_OBJECTS_NAME		"vms_output_tray_objects"
#else
#define K_PW_PAGE_SIZE_OBJECTS_NAME		"ultrix_page_size_objects"
#define K_PW_MESSAGE_LOG_OBJECTS_NAME		"ultrix_message_log_objects"
#define K_PW_SHEET_SIZE_OBJECTS_NAME		"ultrix_sheet_size_objects"
#define K_PW_INPUT_TRAY_OBJECTS_NAME		"ultrix_input_tray_objects"
#define K_PW_OUTPUT_TRAY_OBJECTS_NAME		"ultrix_output_tray_objects"
#endif

/************************************************************************/
/*									*/
/* Numbers for getting to pulldown menus and their data.		*/
/*									*/
/************************************************************************/
#define K_PW_PAGE_SIZE_PULLDOWN_MAP		DXmPAGE_SIZE
#define K_PW_SIDES_PULLDOWN_MAP			DXmSIDES
#define K_PW_FILE_START_SHEET_PULLDOWN_		DXmFILE_START_SHEET
#define K_PW_FILE_END_SHEET_PULLDOWN_MA		DXmFILE_END_SHEET
#define K_PW_FILE_BURST_SHEET_PULLDOWN_		DXmFILE_BURST_SHEET
#define K_PW_MESSAGE_LOG_PULLDOWN_MAP		DXmMESSAGE_LOG
#define K_PW_SHEET_SIZE_PULLDOWN_MAP		DXmSHEET_SIZE
#define K_PW_INPUT_TRAY_PULLDOWN_MAP		DXmINPUT_TRAY
#define K_PW_OUTPUT_TRAY_PULLDOWN_MAP		DXmOUTPUT_TRAY

#define K_PW_NUM_PULLDOWN_MENUS			K_PW_OUTPUT_TRAY_PULLDOWN_MAP + 1

/************************************************************************/
/*									*/
/* Numbers for getting to widget IDs.					*/
/*									*/
/************************************************************************/
#define	K_PW_PRIMARY_BOX	       	1

#define	K_PW_NUMBER_COPIES	       	2
#define	K_PW_PAGE_RANGE_FROM	       	3
#define	K_PW_PAGE_RANGE_TO	       	4
#define	K_PW_PRINT_FORMAT	       	5
#define	K_PW_ORIENTATION	       	6
#define	K_PW_DEFAULT_ORIENTATION       	7
#define	K_PW_PORTRAIT		       	8
#define	K_PW_LANDSCAPE		       	9
#define	K_PW_PRINTER		       	10
#define	K_PW_PRINT_AFTER	       	11
#define	K_PW_DELETE_FILE	       	12

#define	K_PW_OK			       	13
#define	K_PW_CANCEL		       	14
#define	K_PW_OPTIONS		       	15

#define	K_PW_2NDARY_BOX		       	16

#define	K_PW_PAGE_SIZE		       	17 	/*Reserve 30 spots for buttons*/
#define	K_PW_SIDES		       	48 	/*Reserve 10 spots for buttons*/
#define	K_PW_NUMBER_UP		       	59
#define	K_PW_SHEET_COUNT	       	60
#define	K_PW_FILE_START_SHEET	       	61 	/*Reserve 5 spots for buttons*/
#define	K_PW_FILE_END_SHEET	       	67 	/*Reserve 5 spots for buttons*/
#define	K_PW_FILE_BURST_SHEET	       	73 	/*Reserve 5 spots for buttons*/
#define	K_PW_MESSAGE_LOG	       	79 	/*Reserve 5 spots for buttons*/
#define	K_PW_HOLD_JOB		       	85
#define	K_PW_NOTIFY		       	86
#define	K_PW_SHEET_SIZE		       	87 	/*Reserve 30 spots for buttons*/
#define	K_PW_INPUT_TRAY		       	118	/*Reserve 10 spots for buttons*/
#define	K_PW_OUTPUT_TRAY	       	129	/*Reserve 10 spots for buttons*/
#define	K_PW_JOB_NAME		       	140
#define	K_PW_OPERATOR_MESSAGE	       	141
#define	K_PW_HEADER		       	142
#define	K_PW_AUTOMATIC_PAGINATION     	143
#define	K_PW_DOUBLE_SPACING	       	144
#define	K_PW_LAYUP_DEFINITION	       	145
#define	K_PW_START_SHEET_COMMENT       	146
#define	K_PW_PASS_ALL		       	147
#define	K_PW_PRINTER_FORM	       	153
#define	K_PW_PRIORITY		       	154
#define	K_PW_SETUP		       	155
	
#define	K_PW_2NDARY_OK		       	156
#define	K_PW_2NDARY_CANCEL	       	157

#define	K_PW_MSG		       	158
#define	K_PW_MSG_YES		       	159
#define K_PW_HELP_WIDGET	       	160

#define	K_PW_PRIMARY_WINDOW		161
#define	K_PW_PRIMARY_FORM		162
#define	K_PW_2NDARY_WINDOW		163
#define	K_PW_2NDARY_FORM		164

#define	K_PW_NUMBER_COPIES_LABEL	165
#define	K_PW_PAGE_RANGE_FROM_LABEL	166
#define	K_PW_PAGE_RANGE_TO_LABEL	167
#define	K_PW_PRINT_FORMAT_LABEL		168
#define	K_PW_ORIENTATION_LABEL		169
#define	K_PW_PRINTER_LABEL		170
#define	K_PW_PRINT_AFTER_LABEL		171
#define	K_PW_NUMBER_UP_LABEL		172
#define	K_PW_SHEET_COUNT_LABEL		173
#define	K_PW_JOB_NAME_LABEL		174
#define	K_PW_OPERATOR_MESSAGE_LABEL	175
#define	K_PW_LAYUP_DEFINITION_LABEL	176
#define	K_PW_START_SHEET_COMMENT_LABEL	177
#define	K_PW_PRINTER_FORM_LABEL		178
#define	K_PW_PRIORITY_LABEL		179
#define	K_PW_SETUP_LABEL		180

#define K_PW_TOP_PRIMARY_INFO_FORM	181
#define K_PW_MIDDLE_PRIMARY_INFO_FORM	182
#define K_PW_BOTTOM_PRIMARY_INFO_FORM	183
#define K_PW_VIRTUAL_PAGE_BOX		184
#define K_PW_PHYSICAL_PAGE_BOX		185
#define K_PW_PAGE_LAYOUT_BOX		186
#define K_PW_JOB_PARAMETERS_BOX		187
#define K_PW_PAGE_SIZE_LABEL		188
#define K_PW_SIDES_LABEL		189
#define K_PW_SHEET_SIZE_LABEL		190
#define K_PW_INPUT_TRAY_LABEL		191
#define K_PW_OUTPUT_TRAY_LABEL		192
#define K_PW_FILE_START_SHEET_LABEL	193
#define K_PW_FILE_END_SHEET_LABEL	194
#define K_PW_FILE_BURST_SHEET_LABEL	195
#define K_PW_MESSAGE_LOG_LABEL		196

#define K_PW_HELP			197
#define K_PW_2NDARY_HELP		198

#define	K_PW_VIRTUAL_PAGE_LABEL	       	199
#define	K_PW_PHYSICAL_PAGE_LABEL       	200
#define	K_PW_PAGE_LAYOUT_LABEL	       	201
#define	K_PW_JOB_PARAMETERS_LABEL      	202
#define K_PW_DROP_HELP                  203

#define	K_PW_NUM_WIDGETS		204 /* number of above indices */


/************************************************************************/
/*									*/
/* Print widget private resource names					*/
/*									*/
/************************************************************************/
#define	DXmNprintWgtDebug			"DXmprintWgtDebug"			/* debugging resource 	*/
#define	DXmCPrintWgtDebug			"DXmPrintWgtDebug"
#define DXmNknownPrintFormatUIList		"DXmknownPrintFormatUIList"		/* List of print known formats */
#define DXmCKnownPrintFormatUIList  		"DXmKnownPrintFormatUIList"
#define DXmNknownPrintFormatCount		"DXmknownPrintFormatCount" 		/* Number of known print formats */
#define DXmCKnownPrintFormatCount		"DXmKnownPrintFormatCount"
#define DXmNknownPrintFormatVariableList	"DXmknownPrintFormatVariableList"	/* Variable to find q's	*/
#define DXmCKnownPrintFormatVariableList	"DXmKnownPrintFormatVariableList"
#define DXmNknownPrintFormatOSList		"DXmknownPrintFormatOSList" 		/* Op Sys Print Format	*/
#define DXmCKnownPrintFormatOSList		"DXmKnownPrintFormatOSList"
#define DXmNknownPrintFormatGuesserList		"DXmknownFormatGuesserList" 		/* Guesser List	*/
#define DXmCKnownPrintFormatGuesserList		"DXmKnownFormatGuesserList"

#define	DXmNprintWgtWidgetIds			"DXmprintWgtWidgetIds"			/* Pointer to widget ids array 	*/
#define	DXmCPrintWgtWidgetIds			"DXmPrintWgtWidgetIds"

#define	DXmNuidSpec				"DXmuidSpec"				/* Name of UID file			*/
#define	DXmCUidSpec				"DXmUidSpec"				/* (Use DXmNlibrarySpec for Help file)	*/


/************************************************************************/
/*									*/
/* Print Widget Structures						*/
/*									*/
/************************************************************************/

/************************************************************************/
/*									*/
/* Print Queue List Structure						*/
/*									*/
/************************************************************************/
typedef struct _pw_r_variable_list_struct
{
    XmString	ar_variable_name;
    XmString	*ar_cs_list;
    int		l_cs_count;
} pw_r_variable_list_struct;

/************************************************************************/
/*									*/
/* The resource control structure is a static structure which helps the */
/* print widget control its resources.  				*/
/*									*/
/************************************************************************/
#define K_PW_RESOURCE_TYPE_BOOLEAN		0
#define	K_PW_RESOURCE_TYPE_INT			1
#define	K_PW_RESOURCE_TYPE_CS_INT		2
#define K_PW_RESOURCE_TYPE_RADIOBOX		3
#define K_PW_RESOURCE_TYPE_TOGGLE		4
#define K_PW_RESOURCE_TYPE_OPTIONMENU		5
#define	K_PW_RESOURCE_TYPE_TEXT			6
#define K_PW_RESOURCE_TYPE_CS			7
#define K_PW_RESOURCE_TYPE_CS_CHOICE		8
#define K_PW_RESOURCE_TYPE_CS_LIST		9
#define K_PW_RESOURCE_TYPE_CALLBACK_LIST	10
#define K_PW_RESOURCE_TYPE_INT_LIST		11

#define K_PW_BOUNDLESS				0
#define K_PW_BOUNDED_MIN			1
#define K_PW_BOUNDED_MAX			2

typedef struct _pw_r_resource_control_struct
{
    int		l_type;
    int		l_widget_index;
    void	(*widget_activate_proc)();
    void	(*set_value_proc)();
    void	(*save_value_proc)();
    void	(*restore_value_proc)();
    Boolean	(*validate_value_proc)();
} pw_r_resource_control_struct, *pw_ar_resource_control_struct;

typedef struct _pw_r_validate_value_struct
{
    unsigned long int	l_mask;
    int			l_min;
    int			l_max;
} pw_r_validate_value_struct, *pw_ar_validate_value_struct;


typedef struct _pw_r_boolean_res_struct
{
    Boolean	b_current_value;
    Boolean	b_previous_value;
} pw_r_boolean_res_struct, *pw_ar_boolean_res_struct;

typedef struct _pw_r_int_res_struct
{
    int		l_current_value;
    int		l_previous_value;
} pw_r_int_res_struct, *pw_ar_int_res_struct;

typedef struct _pw_r_int_list_res_struct
{
    int		*al_int_list;
    int		l_int_count;
} pw_r_int_list_res_struct, *pw_ar_int_list_res_struct;

typedef struct _pw_r_cs_res_struct
{
    XmString	ar_current_cs;
    XmString	ar_previous_cs;
} pw_r_cs_res_struct, *pw_ar_cs_res_struct;

typedef struct _pw_r_cs_list_res_struct
{
    XmStringTable	ar_cs_list;
    int			l_cs_count;
} pw_r_cs_list_res_struct, *pw_ar_cs_list_res_struct;

typedef struct _pw_r_callback_list_res_struct
{
    XtCallbackRec	*ar_callback_list;
    char		*at_callback_list_name;
} pw_r_callback_list_res_struct, *pw_ar_callback_list_res_struct;

typedef union _pw_r_res_union
{
    pw_r_boolean_res_struct		r_boolean_res;
    pw_r_int_res_struct			r_int_res;
    pw_r_int_list_res_struct		r_int_list_res;
    pw_r_cs_res_struct			r_cs_res;
    pw_r_cs_list_res_struct		r_cs_list_res;
    pw_r_callback_list_res_struct	r_callback_list_res;
} pw_r_res_union, *pw_ar_res_union;

/************************************************************************/
/*									*/
/* Structures for Pulldown menu data.					*/
/*									*/
/************************************************************************/
typedef struct _pw_r_pulldown_menu_data_struct
{
    int			l_resource_index;
    char		*at_objects_list_name;
} pw_r_pulldown_menu_data_struct, *pw_ar_pulldown_menu_data_struct;

typedef struct _pw_r_pulldown_menu_os_table_struct
{
    Opaque		*ar_os_list;
    int			l_os_count;
} pw_r_pulldown_menu_os_table_struct, *pw_ar_pulldown_os_table_struct;


/************************************************************************/
/*									*/
/* DXmPrintWgtClassPart	and DXmPrintWgtClassRec				*/
/*									*/
/************************************************************************/
typedef struct 
{
    XtPointer	extension;				/* pointer to extension record */
} DXmPrintWgtClassPart;

typedef struct _DXmPrintWgtClassRec
{
    CoreClassPart	    core_class;			/* basic widget     	*/
    CompositeClassPart	    composite_class;	    	/* composite portion    */
    ConstraintClassPart	    constraint_class;		/* constraint portion	*/
    XmManagerClassPart	    manager_class;		/* manager portion	*/
    XmBulletinBoardClassPart bulletin_board_class;	/* bullentin board ptn	*/
    DXmPrintWgtClassPart    printwgt_class; 	    	/* printwgt portion   	*/

} DXmPrintWgtClassRec, *DXmPrintWgtClass;

#ifndef _DXmPrintWgt_c
externalref DXmPrintWgtClassRec dxmPrintWgtClassRec;
#endif


/************************************************************************/
/*									*/
/* DXmPrintWgtPart and DXmPrintWgtRec					*/
/*									*/
/************************************************************************/
typedef struct
{
    /********************************************************************/
    /*									*/
    /* Print widget resources						*/
    /*									*/
    /********************************************************************/
    pw_r_res_union	r_number_copies;		/* Number of copies			*/
    pw_r_res_union	r_page_range_from;		/* Page range from 			*/
    pw_r_res_union	r_page_range_to;		/* Page range to 			*/
    pw_r_res_union	r_print_format_contents;	/* Print format list box contents	*/
    pw_r_res_union	r_print_format_choice;		/* Selection in list box		*/
    pw_r_res_union	r_orientation;			/* Orientation				*/
    pw_r_res_union	r_printer_contents;		/* Printer list box contents		*/
    pw_r_res_union	r_printer_choice;		/* Selection in list box		*/
    pw_r_res_union	r_print_after;			/* Print after	 			*/
    pw_r_res_union	r_delete_file;			/* Delete file when printed		*/
    
    pw_r_res_union	r_page_size;			/* Page Size				*/
    pw_r_res_union	r_sides;			/* Sides				*/
    pw_r_res_union	r_number_up;			/* Number up				*/
    pw_r_res_union	r_sheet_count;			/* Sheet count				*/
    pw_r_res_union	r_file_start_sheet;		/* File Start Sheet			*/
    pw_r_res_union	r_file_end_sheet;		/* File End Sheet			*/
    pw_r_res_union	r_file_burst_sheet;		/* File Burst Sheet			*/
    pw_r_res_union	r_message_log;			/* Message Log				*/
    pw_r_res_union	r_hold_job;			/* Hold Job				*/
    pw_r_res_union	r_notify;			/* Notify when done			*/
    pw_r_res_union	r_sheet_size;			/* Sheet Size				*/
    pw_r_res_union	r_input_tray;			/* Input Tray				*/
    pw_r_res_union	r_output_tray;			/* Output Tray				*/
    pw_r_res_union	r_job_name;			/* Job	 Name				*/
    pw_r_res_union	r_operator_message;		/* Operator message 			*/
    pw_r_res_union	r_header;			/* Header				*/
    pw_r_res_union	r_automatic_pagination;		/* Automatic pagination			*/
    pw_r_res_union	r_double_spacing;		/* Double Spacing	      		*/
    pw_r_res_union	r_layup_definition;		/* Layup definition 			*/
    pw_r_res_union	r_start_sheet_comment;		/* Start sheet comment 			*/
    pw_r_res_union	r_pass_all;			/* Pass All Control Characters		*/
    pw_r_res_union	r_printer_form_contents;	/* Print form list box contents		*/
    pw_r_res_union	r_printer_form_choice;		/* Selection in list box		*/
    pw_r_res_union	r_priority;			/* Priority				*/
    pw_r_res_union	r_setup;			/* Setup				*/

    pw_r_res_union 	r_debug;			/* Resource for turning on debugging 	*/
    pw_r_res_union	r_ok_callback_list;		/* Callback for OK selected		*/
    pw_r_res_union	r_unmanage_on_ok;		/* Unmanage widget when OK selected	*/
    pw_r_res_union	r_cancel_callback_list;		/* Callback for CANCEL selected		*/
    pw_r_res_union	r_unmanage_on_cancel;		/* Unmanage widget when cancel selected	*/
    pw_r_res_union	r_filenames;			/* Filenames for print widget		*/
    pw_r_res_union	r_known_format_UI_list;		/* Known print formats.			*/
    pw_r_res_union	r_known_format_variable_list;	/* Array which matches the one above.	*/
    pw_r_res_union	r_known_format_OS_list;		/* Array which matches the one above.	*/    
    pw_r_res_union	r_known_format_guesser_list;	/* Array which matches the one above.	*/    
    pw_r_res_union	r_default_printer;		/* Default printer to use.		*/
    pw_r_res_union	r_suppress_options_mask;	/* Mask for suppressing options.	*/
    pw_r_res_union	r_uid_spec;			/* Name of UID file.			*/
    pw_r_res_union	r_library_spec;			/* Name of Help file.			*/
    pw_r_res_union	r_2ndary_title;			/* Name of 2ndary dialog box.		*/
    
    /********************************************************************/
    /*									*/
    /* Data private to the widget instance				*/
    /*									*/
    /********************************************************************/
    MrmHierarchy	ar_mrm_hierarchy;		/* Print Wgt MRM file hierarchy 	*/
    XmDialogShellWidget	ar_dialog_shell;		/* The shell which holds the print wgt	*/
    Widget		*ar_widget_ids;			/* IDs of widgets in interface		*/
    unsigned long int	l_primary_just_created;		/* If true the primary are saved 	*/
    unsigned long int	l_2ndary_just_created;		/* If true the primary are saved 	*/
    unsigned long int	l_primary_saved;		/* TRUE if primary box just created 	*/
    int			l_print_format_choice;		/* The print format choice index.	*/
    Opaque		r_os_printer_choice;		/* Converted from CS string		*/
    Opaque		r_os_printer_form_choice;	/* Converted from CS string		*/
    int			al_binary_start_time[2];	/* Converted from Asciiz string		*/
    XmString		ar_now_string;			/* "Now"				*/
    pw_r_cs_list_res_struct    				/* Error messages.			*/
	r_error_messages;				/* 					*/
    pw_r_variable_list_struct 				/* Array of queue list by print		*/
                        *ar_known_queue_lists;		/* ..format variable.			*/
    int			l_num_known_queue_lists;	/* Number of known queue lists.		*/
    int			l_current_queue_list_index;	/* Which queues are currently being used*/
    int			*al_current_format_queue_choices; /* Current queue choice by format.	*/
    Cursor		r_wait_cursor;			/* Wait cursor                          */
    XmString		ar_default_queue_list_variable;	/* Variable for default queue list.	*/
    XmString		ar_default_queue_variable;	/* Variable for default queue.		*/
    pw_r_pulldown_menu_os_table_struct	       		/* Pulldown menu data.			*/
	ar_pulldown_menu_os_tables[K_PW_NUM_PULLDOWN_MENUS]; /*					*/
    int			l_page_range_from;		/* Converted from CS string.		*/
    int			l_page_range_to;		/* Converted from CS string.		*/
} DXmPrintWgtPart;


typedef struct _DXmPrintWgtRec
{
    CorePart 			core;			/* basic widget		*/
    CompositePart		composite;		/* composite widget	*/
    ConstraintPart		constraint;		/* constraint widget	*/
    XmManagerPart		manager;		/* manager part		*/
    XmBulletinBoardPart		bulletin_board;		/* bulletin board widget*/
    DXmPrintWgtPart		printwgt;		/* print widget		*/
} DXmPrintWgtRec, *DXmPrintWgt;


/************************************************************************/
/*									*/
/* Macros for getting at each field in the print widget			*/
/*									*/
/************************************************************************/
#define PWar_resources(w)		(&w->printwgt.r_number_copies)
#define PWr_boolean_res(w,i)		((PWar_resources(w))[i].r_boolean_res)
#define PWr_int_res(w,i)		((PWar_resources(w))[i].r_int_res)
#define PWr_int_list_res(w,i)		((PWar_resources(w))[i].r_int_list_res)
#define PWr_cs_res(w,i)			((PWar_resources(w))[i].r_cs_res)
#define PWr_cs_list_res(w,i)		((PWar_resources(w))[i].r_cs_list_res)

#define PWr_debug(w)			(w->printwgt.r_debug.r_int_res)
#define PWar_mrm_hierarchy(w)		(w->printwgt.ar_mrm_hierarchy)
#define PWar_dialog_shell(w)		(w->printwgt.ar_dialog_shell)
#define PWar_widget_ids(w)		(w->printwgt.ar_widget_ids)
#define PWar_widget_id(w,num)		(w->printwgt.ar_widget_ids[num])
#define PWl_primary_just_created(w)	(w->printwgt.l_primary_just_created)
#define PWl_2ndary_just_created(w)	(w->printwgt.l_2ndary_just_created)
#define PWl_primary_saved(w)		(w->printwgt.l_primary_saved)
#define PWl_print_format_choice(w) 	(w->printwgt.l_print_format_choice)
#define PWr_os_printer_choice(w) 	(w->printwgt.r_os_printer_choice)
#define PWr_os_printer_form_choice(w)	(w->printwgt.r_os_printer_form_choice)
#define PWal_binary_start_time(w)	(w->printwgt.al_binary_start_time)
#define PWar_now_string(w)		(w->printwgt.ar_now_string)
#define PWr_error_messages(w)		(w->printwgt.r_error_messages)
#define PWar_error_message(w,l_i)	(w->printwgt.r_error_messages.ar_cs_list[l_i])
#define PWar_known_queue_lists(w)	(w->printwgt.ar_known_queue_lists)
#define PWl_num_known_queue_lists(w)	(w->printwgt.l_num_known_queue_lists)
#define PWl_current_queue_list_index(w) (w->printwgt.l_current_queue_list_index)
#define PWal_current_format_queue_choices(w) (w->printwgt.al_current_format_queue_choices)
#define PWal_current_format_queue_choices(w) (w->printwgt.al_current_format_queue_choices)
#define PWr_wait_cursor(w) 		(w->printwgt.r_wait_cursor)
#define PWar_default_queue_list_variable(w) (w->printwgt.ar_default_queue_list_variable)
#define PWar_default_queue_variable(w) 	(w->printwgt.ar_default_queue_variable)
#define PWar_pulldown_menu_os_table(w,i) (w->printwgt.ar_pulldown_menu_os_tables[i])
#define PWl_page_range_from(w)		(w->printwgt.l_page_range_from)
#define PWl_page_range_to(w)		(w->printwgt.l_page_range_to)


/************************************************************************/
/*									*/
/* Macros for setting widgets from the value of the resource.		*/
/*									*/
/************************************************************************/
#define PW_SET_CS_INT_WIDGET_FROM_RESOURCE(ar_pw,l_resource_index)\
{\
    if (PWar_widget_id(ar_pw,\
		       pw_ar_resource_control[l_resource_index].l_widget_index))\
    {\
        if (PWr_int_res(ar_pw,l_resource_index).l_current_value)\
	{\
	    XmString 	ar_cs_int; \
            long        l_size,l_status;\
	    ar_cs_int = (XmString) _DXmCvtItoCS(PWr_int_res(ar_pw,l_resource_index).l_current_value,\
                                                &l_size,\
                                                &l_status);\
            DXmCSTextSetString(PWar_widget_id(ar_pw,\
					      pw_ar_resource_control[l_resource_index].l_widget_index),\
			       ar_cs_int);\
	   XmStringFree(ar_cs_int); \
	}\
	else\
	   DXmCSTextSetString(PWar_widget_id(ar_pw,\
			  		     pw_ar_resource_control[l_resource_index].l_widget_index),\
			      NULL);\
    }\
}

#define PW_SET_CS_WIDGET_FROM_RESOURCE(ar_pw,l_resource_index)\
{\
    if (PWar_widget_id(ar_pw,\
		       pw_ar_resource_control[l_resource_index].l_widget_index))\
    {\
	DXmCSTextSetString(PWar_widget_id(ar_pw,\
					  pw_ar_resource_control[l_resource_index].l_widget_index),\
			   PWr_cs_res(ar_pw,l_resource_index).ar_current_cs);\
    }\
}

#define PW_SET_LISTBOX_WIDGET_CONTENTS_FROM_RESOURCE(ar_pw,l_resource_index)\
{\
    if (PWar_widget_id(ar_pw,\
		       pw_ar_resource_control[l_resource_index].l_widget_index))\
    {\
	Arg	ar_al[4];\
	int	l_ac;\
	l_ac = 0;\
	XtSetArg(ar_al[l_ac],\
		 XmNitems,\
		 PWr_cs_list_res(ar_pw,l_resource_index).ar_cs_list); l_ac++;\
	XtSetArg(ar_al[l_ac],\
		 XmNitemCount,\
		 PWr_cs_list_res(ar_pw,l_resource_index).l_cs_count); l_ac++;\
	XtSetArg(ar_al[l_ac],\
		 XmNselectedItemCount,\
		 0); l_ac++;\
	XmListDeselectAllItems(PWar_widget_id(ar_pw,\
					      pw_ar_resource_control[l_resource_index].l_widget_index));\
	XtSetValues(PWar_widget_id(ar_pw,\
				   pw_ar_resource_control[l_resource_index].l_widget_index),\
		    ar_al,\
		    l_ac);\
	if (PWr_cs_res(ar_pw,l_resource_index + 1).ar_current_cs)\
	{\
	    XmListSelectItem(PWar_widget_id(ar_pw,\
					    pw_ar_resource_control[l_resource_index].l_widget_index),\
			     PWr_cs_res(ar_pw,l_resource_index + 1).ar_current_cs,\
			     FALSE);\
	    XmListSetItem(PWar_widget_id(ar_pw,\
					 pw_ar_resource_control[l_resource_index].l_widget_index),\
			  PWr_cs_res(ar_pw,l_resource_index + 1).ar_current_cs);\
	}\
    }\
}

#define PW_SET_LISTBOX_WIDGET_SELECTION_FROM_RESOURCE(ar_pw,l_resource_index)\
{\
    if (PWar_widget_id(ar_pw,\
		       pw_ar_resource_control[l_resource_index].l_widget_index))\
    {\
        if (PWr_cs_res(ar_pw,l_resource_index).ar_current_cs)\
        {\
	    XmListSelectItem(PWar_widget_id(ar_pw,\
					    pw_ar_resource_control[l_resource_index].l_widget_index),\
			     PWr_cs_res(ar_pw,l_resource_index).ar_current_cs,\
			     FALSE);\
	    XmListSetItem(PWar_widget_id(ar_pw,\
					 pw_ar_resource_control[l_resource_index].l_widget_index),\
			  PWr_cs_res(ar_pw,l_resource_index).ar_current_cs);\
        }\
        else\
        {\
	    XmListDeselectAllItems(PWar_widget_id(ar_pw,\
						  pw_ar_resource_control[l_resource_index].l_widget_index));\
	}\
    }\
}

/* [[[ This setting of all the other toggles to false is a hack to
       get around a radiobox bug. ]]] */

#define PW_SET_RADIOBOX_WIDGET_FROM_RESOURCE(ar_pw,l_resource_index)\
{\
    if (PWar_widget_id(ar_pw,\
		       pw_ar_resource_control[l_resource_index].l_widget_index))\
    {\
	XmToggleButtonSetState(PWar_widget_id(ar_pw,\
					      K_PW_DEFAULT_ORIENTATION),\
			       FALSE,\
			       FALSE);\
	XmToggleButtonSetState(PWar_widget_id(ar_pw,\
					      K_PW_PORTRAIT),\
			       FALSE,\
			       FALSE);\
	XmToggleButtonSetState(PWar_widget_id(ar_pw,\
					      K_PW_LANDSCAPE),\
			       FALSE,\
			       FALSE);\
        XmToggleButtonSetState(PWar_widget_id(ar_pw,\
					      (pw_ar_resource_control[l_resource_index].l_widget_index + 1) +\
					      PWr_int_res(ar_pw,l_resource_index).l_current_value),\
			       TRUE,\
			       FALSE);\
    }\
}

#define PW_SET_TOGGLE_WIDGET_FROM_RESOURCE(ar_pw,l_resource_index)\
{\
    if (PWar_widget_id(ar_pw,\
		       pw_ar_resource_control[l_resource_index].l_widget_index))\
    {\
        XmToggleButtonSetState(PWar_widget_id(ar_pw,\
					      pw_ar_resource_control[l_resource_index].l_widget_index),\
			       PWr_boolean_res(ar_pw,l_resource_index).b_current_value,\
			       FALSE);\
    }\
}

#define PW_SET_OPTIONMENU_WIDGET_FROM_RESOURCE(ar_pw,l_resource_index)\
{\
    if (PWar_widget_id(ar_pw,\
		       pw_ar_resource_control[l_resource_index].l_widget_index))\
    {\
	Arg	ar_al[1];\
	XtSetArg(ar_al[0],\
		 XmNmenuHistory,\
		 PWar_widget_id(ar_pw,\
				(pw_ar_resource_control[l_resource_index].l_widget_index + 1) +\
				PWr_int_res(ar_pw,l_resource_index).l_current_value));\
	XtSetValues(PWar_widget_id(ar_pw,\
				   pw_ar_resource_control[l_resource_index].l_widget_index),\
		    ar_al,\
		    1);\
    }\
}

#define PW_GET_OS_TABLE_STRING(ar_pw,l_pulldown_map,l_int_res_map)\
PWar_pulldown_menu_os_table(ar_pw,l_pulldown_map).ar_os_list[PWr_int_res(ar_pw,l_int_res_map).l_current_value]

/************************************************************************/
/*									*/
/* Utility structures for VMS					        */
/*									*/
/************************************************************************/
#define K_PW_MULTI_JOB_CREATE		1
#define K_PW_MULTI_JOB_ADD_FILE		2
#define K_PW_SINGLE_JOB_ENTRY		3

#define K_PW_CHUNK_INCR			10
#define K_PW_MAX_QUEUE_NAME_LENGTH	31
#define K_PW_MAX_FORM_NAME_LENGTH	31

typedef struct _r_itmbuf_struct
{
    unsigned short 	c_buflen;
    unsigned short 	c_code;
    char 		*at_bufadr;
    long 		*al_retlenadr;
} r_itmbuf_struct;

typedef struct _r_iosb_struct
{
    long l_stat;
    long l_dummy;
} r_iosb_struct;

/************************************************************************/
/*									*/
/* Error Message Codes							*/
/*									*/
/************************************************************************/
#define K_PW_INVALID_NUMBER_COPIES	0
#define K_PW_INVALID_PAGE_RANGE_SPEC	1
#define K_PW_INVALID_SHEET_COUNT	2
#define K_PW_INVALID_NUMBER_UP		3
#define K_PW_INVALID_PRIORITY		4
#define K_PW_INVALID_JOB_NAME		5
#define K_PW_INVALID_LAYUP_NAME		6
#define K_PW_INVALID_START_AFTER_TIME	7
#define K_PW_NO_PRINTERS		8
#define K_PW_FILE_NOT_ACCESSIBLE	9
#define K_PW_NUM_ERROR_MESSAGES		10

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _DXmPrintP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
