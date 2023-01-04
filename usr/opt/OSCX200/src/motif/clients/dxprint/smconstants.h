/*
*****************************************************************************
**                                                                          *
**  COPYRIGHT (c) 1988, 1989, 1991, 1992 BY                                 *
**  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.                  *
**  ALL RIGHTS RESERVED.                                                    *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED   *
**  ONLY IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE   *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER   *
**  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY   *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE IS  HEREBY   *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE   *
**  AND  SHOULD  NOT  BE  CONSTRUED AS  A COMMITMENT BY DIGITAL EQUIPMENT   *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS   *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
**                                                                          *
*****************************************************************************
**
** FACILITY:  PrintScreen
**
** ABSTRACT:
**
**	This header files of UI constants
**
** ENVIRONMENT:
**
**      VAX/VMS operating system.
**
** AUTHOR:  Karen Brouillette October 1989
**
** Modified by:
**
**	29-Jan-1992	Edward P Luwish
**		Add MAXFILSTR constant so that length of permanent file name
**		is same length as volatile file name
**
**	09-Apr-1991	Edward P Luwish
**		Port to Motif UI, add help messages
**
**
*/

#define MAXFILSTR 512

/* acceration values */


#define	end_session_number  0
#define resource_changed_number 1

/* UIL defs */

/*
 * Application message ID's  must match those in SM_DEFS.UIL
 */

#define k_sm_title_msg 0
#define k_sm_copytitle_msg 1
#define k_sm_iconname_msg 2
#define k_system_resource_msg 3
#define k_resource_create_msg 4
#define k_help_about_msg 5
#define k_help_ultrix_msg 6
#define k_help_vms_msg 7
#define k_help_overview_msg 8
#define k_print_device_msg 9
#define k_print_window_msg 10
#define k_print_aspect_msg 11
#define k_print_color_msg 12
#define k_print_dest_msg 13
#define k_print_queue_msg 14
#define k_print_reverse_msg 15
#define k_print_storage_msg 16
#define k_print_form_msg 17
#define k_print_code_msg 18
#define k_print_fatal_msg 19
#define k_print_image_msg 20
#define k_print_file_msg 21
#define k_print_check_msg 22
#define k_print_alloc_msg 23
#define k_print_intx_msg 24
#define k_print_func_msg 25
#define k_print_unknown_msg 26
#define k_resource_nochange_msg 27
#define k_resource_change_msg 28
#define k_resource_nosave_msg 29
#define k_resource_save_msg 30
#define k_sm_putdatabase_msg 31
#define k_sm_putproperty_msg 32
#define k_print_noddif_msg  33
#define k_print_nodepth_msg 34
#define k_help_window_msg 35
#define k_help_version_msg 36
#define k_help_context_msg 37
#define k_help_on_help_msg 38

#define k_help_on_window_value	1
#define k_help_on_version_value	2
#define k_help_on_context_value	3
#define k_help_on_help_value	4
