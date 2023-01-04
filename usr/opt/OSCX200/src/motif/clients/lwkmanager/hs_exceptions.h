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
** COPYRIGHT (c) 1989, 1990, 1991 BY
** DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
** ALL RIGHTS RESERVED.
**
** THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
** ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
** INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
** COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
** OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
** TRANSFERRED.
**
** THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
** AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
** CORPORATION.
**
** DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
** SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
*/

/*
**++
**  Subsystem:
**	LinkWorks Manager
**
**  Version: V1.0
**
**  Abstract:
**	Header file for Exception Handling Service
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	André Pavanello
**
**  Creation Date: 2-Nov-89
**
**  Modification History:
**  X0.7-1   pa   17-Nov-89  add error messages
**--
*/


/*
**  Include Files
*/

#include <setjmp.h>

/*
**  Type Definitions
*/

typedef unsigned long int _Exception;

/*
**  Error messages definitions
*/

typedef enum _hs_status {
    hs_s_min = 0,
    hs_s_unknown = 0,
    hs_s_success,
    hs_s_failure,
    hs_s_not_yet_impl,
    hs_s_alloc_error,
    hs_s_dealloc_error,   
    hs_s_list_empty,
    hs_s_drm_open_error,
    hs_s_drm_fetch_error,
    hs_s_drm_close_error,
    hs_s_inv_widget_id,
    hs_s_inv_operation,
    hs_s_inv_domain,
    hs_s_no_such_property,
    hs_s_property_is_readonly,
    hs_s_abstract_object,
    hs_s_inv_argument,
    hs_s_inv_comp_string,
    hs_s_single_valued_property,
    hs_s_property_not_deleted,
    hs_s_inv_set_operation,
    hs_s_no_such_element,
    hs_s_inv_object,
    hs_s_inv_currency,
    hs_s_cur_set_failure,
    hs_s_lb_not_exist,
    hs_s_new_lb,
    hs_s_retrieve_error,
    hs_s_wnd_open_error,
    hs_s_wnd_disp_error,
    hs_s_startup_failed,
    hs_s_set_value_failed,
    hs_s_get_value_failed,
    hs_s_objdsc_encode_error,
    hs_s_objdsc_decode_error,
    hs_s_get_objdsc_failed,
    hs_s_add_ele_failed,
    hs_s_create_list_failed,
    hs_s_close_lb_err,
    hs_s_open_lb_err,
    hs_s_store_failed,
    hs_s_save_failed,
    hs_s_open_lb_failed,
    hs_s_quit_failed,
    hs_s_map_menu_failed,
    hs_s_activate_failed,
    hs_s_record_failed,
    hs_s_show_prop_failed,
    hs_s_expand_failed,
    hs_s_save_lb_failed,
    hs_s_paste_failed,
    hs_s_cut_failed,
    hs_s_copy_failed,
    hs_s_delete_failed,
    hs_s_get_domain_failed,
    hs_s_persobj_creation_failed,
    hs_s_persobj_deletion_failed,
    hs_s_encode_deletion_failed,
    hs_s_envmgr_deletion_failed,
    hs_s_iterate_failed,
    hs_s_svn_entry_not_found,
    hs_s_object_not_found,
    hs_s_copy_from_clip_failed,
    hs_s_clip_locked,
    hs_s_no_timestamp,
    hs_s_lb_already_open,
    hs_s_new_wnd_not_open,
    hs_s_sel_conf_cb_failed,
    hs_s_select_cb_failed,
    hs_s_dwui_creation_failed,
    hs_s_no_self_insert,
    hs_s_create_lb_err,
    hs_s_update_err,
    hs_s_delete_err,
    hs_s_save_attr_failed,
    hs_s_restore_attr_failed,
    hs_s_restore_sys_attr_failed,
    hs_s_object_retrieve_failed,
    hs_s_duplicate_obj,
    hs_s_lb_to_be_saved,
    hs_s_obj_name_to_be_changed,
    hs_s_cannot_test_recursive,
    hs_s_recursive_list,
    hs_s_cannot_load_path,
    hs_s_cannot_load_attr,
    hs_s_cannot_load_trail,
    hs_s_version_error,
    hs_s_get_selection_failed,
    hs_s_unexpected_error,
    hs_s_max
    } hs_status;

/*
**  Macro Definitions
*/

#define _StartExceptionBlock \
    { /* Start of Exception Handling Block */ \
	_Status volatile _code_; \
	EnvExcBeginException(); \
	_code_ = (_Status) setjmp(EnvExcContexts[EnvExcContext]); \
	if (_code_ == hs_s_unknown) {  /* Normal return from setjmp */ \

#define _Exceptions \
	    EnvExcLeaveExceptionNormally(); \
	} \
	else {  /* longjmp return from setjmp */ \
	    EnvExcLeaveExceptionRaised(); \
	    if (_False) {

#define _When(Code) \
	    } \
	    else if (_code_ == _StatusCode(Code)) {

#define _WhenOthers \
	    } \
	    else if (_code_ == _code_) {

#define _EndExceptionBlock \
	    } \
	    else \
		_Reraise; \
	} \
    } /* End of Exception Handling Block */

#define _Others _code_

#define _Signal(Code) EnvExcSignalException(_ExceptionCode(Code))
#define _Raise(Code) EnvExcRaiseException(_ExceptionCode(Code))
#define _Reraise EnvExcRaiseException((_Exception) _code_)

/*
**  Define Exception Codes
*/

#if defined(__osf__)
#define _ExceptionCode(Code) hs_x_ ## Code
#else
#define _ExceptionCode(Code) hs_x_/**/Code
#endif /* __osf__ */

#define _ExceptionCodes \
    _X_(failure), \
    _X_(not_yet_impl), \
    _X_(alloc_error), \
    _X_(dealloc_error), \
    _X_(list_empty), \
    _X_(drm_open_error), \
    _X_(drm_fetch_error), \
    _X_(drm_close_error), \
    _X_(inv_widget_id), \
    _X_(inv_operation), \
    _X_(inv_domain), \
    _X_(no_such_property), \
    _X_(property_is_readonly), \
    _X_(abstract_object), \
    _X_(inv_argument), \
    _X_(inv_comp_string), \
    _X_(single_valued_property), \
    _X_(property_not_deleted), \
    _X_(inv_set_operation), \
    _X_(no_such_element), \
    _X_(inv_object), \
    _X_(inv_currency), \
    _X_(cur_set_failure), \
    _X_(lb_not_exist), \
    _X_(new_lb), \
    _X_(retrieve_error), \
    _X_(wnd_open_error), \
    _X_(wnd_disp_error), \
    _X_(startup_failed), \
    _X_(set_value_failed), \
    _X_(get_value_failed), \
    _X_(objdsc_encode_error), \
    _X_(objdsc_decode_error), \
    _X_(get_objdsc_failed), \
    _X_(add_ele_failed), \
    _X_(create_list_failed), \
    _X_(close_lb_err), \
    _X_(open_lb_err), \
    _X_(store_failed), \
    _X_(save_failed), \
    _X_(open_lb_failed), \
    _X_(quit_failed), \
    _X_(map_menu_failed), \
    _X_(activate_failed), \
    _X_(record_failed), \
    _X_(show_prop_failed), \
    _X_(expand_failed), \
    _X_(save_lb_failed), \
    _X_(paste_failed), \
    _X_(cut_failed), \
    _X_(copy_failed), \
    _X_(delete_failed), \
    _X_(get_domain_failed), \
    _X_(persobj_creation_failed), \
    _X_(persobj_deletion_failed), \
    _X_(encode_deletion_failed), \
    _X_(envmgr_deletion_failed), \
    _X_(iterate_failed), \
    _X_(svn_entry_not_found), \
    _X_(object_not_found), \
    _X_(copy_from_clip_failed), \
    _X_(clip_locked), \
    _X_(no_timestamp), \
    _X_(lb_already_open), \
    _X_(new_wnd_not_open), \
    _X_(sel_conf_cb_failed), \
    _X_(select_cb_failed), \
    _X_(dwui_creation_failed), \
    _X_(no_self_insert), \
    _X_(create_lb_err), \
    _X_(update_err), \
    _X_(delete_err), \
    _X_(save_attr_failed), \
    _X_(restore_attr_failed), \
    _X_(restore_sys_attr_failed), \
    _X_(object_retrieve_failed), \
    _X_(duplicate_obj), \
    _X_(lb_to_be_saved), \
    _X_(obj_name_to_be_changed), \
    _X_(cannot_test_recursive), \
    _X_(recursive_list), \
    _X_(cannot_load_path), \
    _X_(cannot_load_attr), \
    _X_(cannot_load_trail), \
    _X_(version_error), \
    _X_(get_selection_failed), \
    _X_(unexpected_error)
        
/*
**  External Routine Declarations
*/

_DeclareFunction(_Void EnvExcBeginException, (void));
_DeclareFunction(_Void EnvExcLeaveExceptionNormally, (void));
_DeclareFunction(_Void EnvExcLeaveExceptionRaised, (void));
_DeclareFunction(_Void EnvExcSignalException, (_Exception code));
_DeclareFunction(_Void EnvExcRaiseException, (_Exception code));

/*
**  External Data Definitions
*/

_External jmp_buf EnvExcContexts[];
_External int     EnvExcContext;

#define _X_(Code) _ExceptionCode(Code)

_External _Exception _Constant
    _ExceptionCodes;

#undef _X_
