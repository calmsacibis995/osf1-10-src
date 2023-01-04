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
/*	
 *	@(#)$RCSfile: Messages.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:00:03 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if SEC_BASE

/*
    filename:
        Messages.h
        
    copyright:
        Copyright (c) 1989-1990 SKM, L.P.
        Copyright (c) 1989-1990 SecureWare Inc.
        Copyright (c) 1989-1990 MetaMedia Inc.
        ALL RIGHTS RESERVED

    contains:
	variables used my message subsystem

*/

#include "gl_defs.h"

/* Error messages */
GLOBAL char 
	**msg_not_user INIT1 (NULL),
	**msg_no_prdb_entry INIT1 (NULL),
	**msg_acct_retired INIT1 (NULL),
	**msg_not_isso INIT1 (NULL),
	**msg_is_isso INIT1 (NULL),
	**msg_null_user INIT1 (NULL),
	**msg_no_default_entry INIT1 (NULL),
	**msg_cant_update_user INIT1 (NULL),
	**msg_cant_update INIT1 (NULL),
	**msg_cant_read_audit INIT1 (NULL),
	**msg_null_device INIT1 (NULL),
	**msg_no_device_entry INIT1 (NULL),
	**msg_no_terminal_entry INIT1 (NULL),
	**msg_cant_update_devices INIT1 (NULL),
	**msg_cant_update_terminal INIT1 (NULL),
	**msg_null_terminal INIT1 (NULL),
	**msg_no_host_entry INIT1 (NULL),
	**msg_cant_update_host INIT1 (NULL),
	**msg_null_host INIT1 (NULL),
	**msg_devices_error_1 INIT1 (NULL),
	**msg_devices_error_2 INIT1 (NULL),
	**msg_devices_error_3 INIT1 (NULL),
	**msg_devices_error_4 INIT1 (NULL),
	**msg_devices_error_5 INIT1 (NULL),
	**msg_devices_error_6 INIT1 (NULL),
	*msg_not_user_text INIT1 (NULL),
	*msg_no_prdb_entry_text INIT1 (NULL),
	*msg_acct_retired_text INIT1 (NULL),
	*msg_not_isso_text INIT1 (NULL),
	*msg_is_isso_text INIT1 (NULL),
	*msg_null_user_text INIT1 (NULL),
	*msg_no_default_entry_text INIT1 (NULL),
	*msg_cant_update_user_text INIT1 (NULL),
	*msg_cant_update_text INIT1 (NULL),
	*msg_cant_read_audit_text INIT1 (NULL),
	*msg_null_device_text INIT1 (NULL),
	*msg_no_device_entry_text INIT1 (NULL),
	*msg_no_terminal_entry_text INIT1 (NULL),
	*msg_cant_update_devices_text INIT1 (NULL),
	*msg_cant_update_terminal_text INIT1 (NULL),
	*msg_null_terminal_text INIT1 (NULL),
	*msg_no_host_entry_text INIT1 (NULL),
	*msg_cant_update_host_text INIT1 (NULL),
	*msg_null_host_text INIT1 (NULL),
	*msg_devices_error_1_text INIT1 (NULL),
	*msg_devices_error_2_text INIT1 (NULL),
	*msg_devices_error_3_text INIT1 (NULL),
	*msg_devices_error_4_text INIT1 (NULL),
	*msg_devices_error_5_text INIT1 (NULL),
	*msg_devices_error_6_text INIT1 (NULL);

GLOBAL char
	**msg_make_group_value INIT1 (NULL),
	**msg_make_group_toggle INIT1 (NULL),
	**msg_cant_access_group INIT1 (NULL),
	**msg_invalid_group INIT1 (NULL),
	**msg_invalid_gid INIT1 (NULL),
	**msg_group_exists INIT1 (NULL),
	**msg_gid_exists INIT1 (NULL),
	**msg_cant_update_group INIT1 (NULL),

	*msg_make_group_value_text INIT1 (NULL),
	*msg_make_group_toggle_text INIT1 (NULL),
	*msg_cant_access_group_text INIT1 (NULL),
	*msg_invalid_group_text INIT1 (NULL),
	*msg_invalid_gid_text INIT1 (NULL),
	*msg_group_exists_text INIT1 (NULL),
	*msg_gid_exists_text INIT1 (NULL),
	*msg_cant_update_group_text INIT1 (NULL);

GLOBAL char
	**msg_error_lock_passwd_file INIT1 (NULL),
	**msg_error_cant_update_passwd INIT1 (NULL),
	**msg_error_cant_create_home_dir INIT1 (NULL),
	**msg_error_cant_stat_skel INIT1 (NULL),
	**msg_error_cant_open_skel INIT1 (NULL),
	*msg_error_lock_passwd_file_text INIT1 (NULL),
	*msg_error_cant_update_passwd_text INIT1 (NULL),
	*msg_error_cant_create_home_dir_text INIT1 (NULL),
	*msg_error_cant_stat_skel_text INIT1 (NULL),
	*msg_error_cant_open_skel_text INIT1 (NULL);

GLOBAL char
	**msg_err_dev_already_exists INIT1 (NULL),
	*msg_err_dev_already_exists_text INIT1 (NULL); 

/* Error messages for creating a new directory */
GLOBAL char
	**msg_error_mkdir_eacces INIT1 (NULL),
	**msg_error_mkdir_efault INIT1 (NULL),
	**msg_error_mkdir_eintr INIT1 (NULL),
	**msg_error_mkdir_einval INIT1 (NULL),
	**msg_error_mkdir_enotdir INIT1 (NULL),
	**msg_error_mkdir_enoent INIT1 (NULL),
	**msg_error_mkdir_eremote INIT1 (NULL),
	**msg_error_mkdir_error INIT1 (NULL),
	**msg_error_mkdir_change_sl INIT1 (NULL),
	**msg_error_mkdir_reset_sl INIT1 (NULL),
	**msg_error_mkdir_cant_getslabel INIT1 (NULL),
	*msg_error_mkdir_eacces_text INIT1 (NULL),
	*msg_error_mkdir_efault_text INIT1 (NULL),
	*msg_error_mkdir_eintr_text INIT1 (NULL),
	*msg_error_mkdir_einval_text INIT1 (NULL),
	*msg_error_mkdir_enotdir_text INIT1 (NULL),
	*msg_error_mkdir_enoent_text INIT1 (NULL),
	*msg_error_mkdir_eremote_text INIT1 (NULL),
	*msg_error_mkdir_error_text INIT1 (NULL),
	*msg_error_mkdir_change_sl_text INIT1 (NULL),
	*msg_error_mkdir_reset_sl_text INIT1 (NULL),
	*msg_error_mkdir_cant_getslabel_text INIT1 (NULL);

GLOBAL char
	**msg_make_user_value INIT1 (NULL),
	**msg_make_user_toggle INIT1 (NULL),
	**msg_error_must_list_group INIT1 (NULL),
	**msg_error_cant_access_passwd INIT1 (NULL),
	**msg_error_invalid_name INIT1 (NULL),
	**msg_error_name_exists INIT1 (NULL),
	**msg_error_invalid_uid INIT1 (NULL),
	**msg_error_uid_exists INIT1 (NULL),
	**msg_error_invalid_home_dir INIT1 (NULL),
	**msg_error_invalid_shell INIT1 (NULL),
	**msg_error_invalid_comment INIT1 (NULL),
	**msg_error_cant_update_password INIT1 (NULL),
	**msg_error_cant_create_home INIT1 (NULL),
	*msg_make_user_value_text INIT1 (NULL),
	*msg_make_user_toggle_text INIT1 (NULL),
	*msg_error_must_list_group_text INIT1 (NULL),
	*msg_error_cant_access_passwd_text INIT1 (NULL),
	*msg_error_invalid_name_text INIT1 (NULL),
	*msg_error_name_exists_text INIT1 (NULL),
	*msg_error_invalid_uid_text INIT1 (NULL),
	*msg_error_uid_exists_text INIT1 (NULL),
	*msg_error_invalid_home_dir_text INIT1 (NULL),
	*msg_error_invalid_shell_text INIT1 (NULL),
	*msg_error_invalid_comment_text INIT1 (NULL),
	*msg_error_cant_update_password_text INIT1 (NULL),
	*msg_error_cant_create_home_text INIT1 (NULL);



#endif /* SEC_BASE */
