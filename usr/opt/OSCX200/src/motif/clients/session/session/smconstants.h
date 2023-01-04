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
/* acceration values */

#define none_num -1
#define none_den -1
#define none_t -1

#define slow_num -1
#define slow_den -1
#define slow_t   -1

#define med_num	 -1
#define med_den -1
#define med_t   -1

#define fast_num -1
#define fast_den -1
#define fast_t   -1

#define	default_pattern	    0	/* first 3 patterns are special cased */
#define	solid_background    1
#define solid_foreground    2
#define gray_width 16
#define gray_height 16

#define	end_session_number  0
#define resource_changed_number 1

/* UIL defs */

/*
 * Application message ID's  must match those in SM_DEFS.UIL
 */

#define k_sm_title_msg 0
#define k_sm_copytitle_msg 1
#define k_sm_iconname_msg 2
#define k_security_error_msg 3
#define k_system_resource_msg 4
#define k_resource_create_msg 5
#define k_color_mapfull_msg 6
#define k_sm_winmgr_msg 7
#define k_help_about_msg 8
#define k_help_ultrix_msg 9
#define k_help_vms_msg 10
#define k_help_overview_msg 11
#define k_keymap_default_msg 12
#define k_print_device_msg 13
#define k_print_window_msg 14
#define k_print_aspect_msg 15
#define k_print_color_msg 16
#define k_print_dest_msg 17
#define k_print_queue_msg 18
#define k_print_reverse_msg 19
#define k_print_storage_msg 20
#define k_print_form_msg 21
#define k_print_code_msg 22
#define k_print_fatal_msg 23
#define k_print_image_msg 24
#define k_print_file_msg 25
#define k_print_check_msg 26
#define k_print_alloc_msg 27
#define k_print_intx_msg 28
#define k_print_func_msg 29
#define k_print_unknown_msg 30
#define k_sm_process_msg 31
#define k_sm_control_msg 32
#define k_sm_opkey_msg 33
#define k_process_complete_msg 34
#define k_process_start_msg 35
#define k_resource_nochange_msg 36
#define k_resource_change_msg 37
#define k_resource_nosave_msg 38
#define k_resource_save_msg 39
#define k_sm_putdatabase_msg 40
#define k_sm_putproperty_msg 41
#define k_verb_missing_msg 42
#define k_label_missing_msg 43
#define k_appdef_missing_msg 44
#define k_appdef_system_msg 45
#define k_passfail_msg 46
#define k_cust_window_msg 47
#define k_cust_iconbox_msg 48
#define k_wrong_pass_msg 49
#define k_error_pass_msg 50
#define k_cust_both_msg 51
#define k_print_noddif_msg  52
#define k_print_nodepth_msg 53
#define k_lg_gbl_msg 54
#define k_lg_gblpages_msg 55
#define k_lg_gblsections_msg 56
#define k_resource_norestore_msg 57
#define k_filesystem_delay_msg 58
#define k_security_noserver_msg 59
#define k_appdef_invalid_msg 60
