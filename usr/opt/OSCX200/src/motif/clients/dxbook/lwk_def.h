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
/**/
/******************************************************************************/
/**                                                                          **/
/**  Copyright (c) 1992                                                      **/
/**  by DIGITAL Equipment Corporation, Maynard, Mass.                        **/
/**                                                                          **/
/**  This software is furnished under a license and may be used and  copied  **/
/**  only  in  accordance  with  the  terms  of  such  license and with the  **/
/**  inclusion of the above copyright notice.  This software or  any  other  **/
/**  copies  thereof may not be provided or otherwise made available to any  **/
/**  other person.  No title to and ownership of  the  software  is  hereby  **/
/**  transferred.                                                            **/
/**                                                                          **/
/**  The information in this software is subject to change  without  notice  **/
/**  and  should  not  be  construed  as  a commitment by DIGITAL Equipment  **/
/**  Corporation.                                                            **/
/**                                                                          **/
/**  DIGITAL assumes no responsibility for the use or  reliability  of  its  **/
/**  software on equipment which is not supplied by DIGITAL.                 **/
/**                                                                          **/
/******************************************************************************/
/********************************************************************************************************************************/
/* Created 14-JUL-1992 18:32:53 by VAX SDL V3.2-12     Source: 14-JUL-1992 18:30:49 MEMEX$BUILD:[HIS.BL24A.SOURCE]LWK_DEF.SDL;1 */
/********************************************************************************************************************************/
 
/*** MODULE lwk_definitions IDENT V1.0 ***/
#ifndef lwk__def
#define lwk__def 1
#ifndef lwk_ptr
#ifdef MSDOS
#define lwk_ptr _far *
#else /* !MSDOS */
#define lwk_ptr *
#endif /* MSDOS */
#endif /* !lwk_ptr */
#ifndef lwk_entry
#ifdef MSDOS
#ifdef MSWINDOWS
#define lwk_entry _far PASCAL
#else /* MSDOS && !MSWINDOWS */
#define lwk_entry _far
#endif /* MSWINDOWS */
#else /* !MSDOS */
#define lwk_entry
#endif /* MSDOS */
#endif /* !lwk_entry */
typedef void lwk_ptr lwk_any_pointer;
typedef long int lwk_boolean;
typedef unsigned long int lwk_closure;
typedef char lwk_ptr lwk_ddif_string;
typedef char lwk_ptr lwk_date;
typedef char lwk_ptr lwk_encoding;
typedef float lwk_float;
typedef long int lwk_integer;
typedef char lwk_ptr lwk_string;
typedef unsigned long int lwk_termination;
typedef lwk_termination (lwk_entry *lwk_callback)();
typedef lwk_any_pointer lwk_ptr lwk_any_pointer_ptr;
typedef lwk_boolean lwk_ptr lwk_boolean_ptr;
typedef lwk_closure lwk_closure_ptr;
typedef lwk_ddif_string lwk_ptr lwk_ddif_string_ptr;
typedef lwk_date lwk_ptr lwk_date_ptr;
typedef lwk_encoding lwk_ptr lwk_encoding_ptr;
typedef lwk_float lwk_ptr lwk_float_ptr;
typedef lwk_integer lwk_ptr lwk_integer_ptr;
typedef lwk_string lwk_ptr lwk_string_ptr;
typedef lwk_termination lwk_ptr lwk_termination_ptr;
typedef lwk_callback (lwk_ptr lwk_callback_ptr)();
typedef lwk_any_pointer lwk_object;
typedef lwk_object lwk_list;
typedef lwk_object lwk_set;
typedef lwk_object lwk_object_id;
typedef lwk_object lwk_object_descriptor;
typedef lwk_object lwk_ui;
typedef lwk_object lwk_dxm_ui;
typedef lwk_object lwk_persistent;
typedef lwk_object lwk_surrogate;
typedef lwk_object lwk_link;
typedef lwk_object lwk_linknet;
typedef lwk_object lwk_composite_linknet;
typedef lwk_object lwk_step;
typedef lwk_object lwk_path;
typedef lwk_object lwk_composite_path;
typedef lwk_object lwk_linkbase;
typedef lwk_object lwk_ptr lwk_object_ptr;
typedef lwk_list lwk_ptr lwk_list_ptr;
typedef lwk_set lwk_ptr lwk_set_ptr;
typedef lwk_object_id lwk_ptr lwk_object_id_ptr;
typedef lwk_object_descriptor lwk_ptr lwk_object_descriptor_ptr;
typedef lwk_ui lwk_ptr lwk_ui_ptr;
typedef lwk_dxm_ui lwk_ptr lwk_dxm_ui_ptr;
typedef lwk_persistent lwk_ptr lwk_persistent_ptr;
typedef lwk_surrogate lwk_ptr lwk_surrogate_ptr;
typedef lwk_link lwk_ptr lwk_link_ptr;
typedef lwk_linknet lwk_ptr lwk_linknet_ptr;
typedef lwk_composite_linknet lwk_ptr lwk_composite_linknet_ptr;
typedef lwk_step lwk_ptr lwk_step_ptr;
typedef lwk_path lwk_ptr lwk_path_ptr;
typedef lwk_composite_path lwk_ptr lwk_composite_path_ptr;
typedef lwk_linkbase lwk_ptr lwk_linkbase_ptr;
#define lwk_c_false ((lwk_boolean) 0)
#define lwk_c_true ((lwk_boolean) 1)
#define lwk_c_hl_none 0
#define lwk_c_hl_on 1
#define lwk_c_hl_sources 2
#define lwk_c_hl_targets 4
#define lwk_c_hl_orphans 8
#define lwk_c_hl_pending_source 16
#define lwk_c_hl_pending_target 32
#define lwk_c_hl_destination_of_follow 64
#define lwk_c_hl_select_destination 128
#define lwk_c_null_object 0
typedef enum _lwk_status {
    lwk_s_min = 0,
    lwk_s_unknown = 0,
    lwk_s_success,
    lwk_s_failure,
    lwk_s_bad_logic,
    lwk_s_not_yet_impl,
    lwk_s_alloc_error,
    lwk_s_dealloc_error,
    lwk_s_inv_object,
    lwk_s_inv_argument,
    lwk_s_inv_operation,
    lwk_s_no_such_property,
    lwk_s_inv_domain,
    lwk_s_list_empty,
    lwk_s_property_is_readonly,
    lwk_s_dupl_element,
    lwk_s_no_such_element,
    lwk_s_inv_set_operation,
    lwk_s_property_not_deleted,
    lwk_s_single_valued_property,
    lwk_s_cannot_move_surrogate,
    lwk_s_cannot_move_link,
    lwk_s_cannot_move_step,
    lwk_s_cannot_delete_surrogate,
    lwk_s_inv_date,
    lwk_s_inv_ddif_string,
    lwk_s_inv_string,
    lwk_s_drm_open_error,
    lwk_s_drm_fetch_error,
    lwk_s_drm_close_error,
    lwk_s_get_surrogate_cb_error,
    lwk_s_create_surrogate_cb_error,
    lwk_s_apply_cb_error,
    lwk_s_close_view_cb_error,
    lwk_s_complete_link_cb_error,
    lwk_s_env_change_cb_error,
    lwk_s_inv_encoding,
    lwk_s_internal_encoding_error,
    lwk_s_internal_decoding_error,
    lwk_s_inv_widget_id,
    lwk_s_stored_elsewhere,
    lwk_s_no_such_linkbase,
    lwk_s_db_not_open,
    lwk_s_no_such_object,
    lwk_s_db_read_error,
    lwk_s_db_write_error,
    lwk_s_db_lock_error,
    lwk_s_db_commit_error,
    lwk_s_db_version_error,
    lwk_s_linkbase_in_use,
    lwk_s_not_linked,
    lwk_s_no_recording_linknet,
    lwk_s_no_pending_source,
    lwk_s_no_pending_target,
    lwk_s_no_history,
    lwk_s_history_is_empty,
    lwk_s_no_active_cpath,
    lwk_s_no_more_steps,
    lwk_s_inv_query_expr,
    lwk_s_abstract_object,
    lwk_s_highlight_error,
    lwk_s_go_to_error,
    lwk_s_visit_error,
    lwk_s_show_history_error,
    lwk_s_step_forward_error,
    lwk_s_go_back_error,
    lwk_s_start_link_error,
    lwk_s_dummy_1,
    lwk_s_complete_link_error,
    lwk_s_annotate_error,
    lwk_s_show_link_error,
    lwk_s_delete_link_error,
    lwk_s_show_links_error,
    lwk_s_link_update_error,
    lwk_s_show_step_error,
    lwk_s_delete_step_error,
    lwk_s_step_update_error,
    lwk_s_unexpected_error,
    lwk_s_db_access_error,
    lwk_s_apply_error,
    lwk_s_invocation_error,
    lwk_s_normal_return,
    lwk_s_not_registered,
    lwk_s_max
    } lwk_status, lwk_ptr lwk_status_ptr;
typedef enum _lwk_domain {
    lwk_c_domain_min = 0,
    lwk_c_domain_unknown = 0,
    lwk_c_domain_integer,
    lwk_c_domain_float,
    lwk_c_domain_date,
    lwk_c_domain_string,
    lwk_c_domain_ddif_string,
    lwk_c_domain_boolean,
    lwk_c_domain_routine,
    lwk_c_domain_closure,
    lwk_c_domain_object,
    lwk_c_domain_list,
    lwk_c_domain_set,
    lwk_c_domain_property,
    lwk_c_domain_object_id,
    lwk_c_domain_object_desc,
    lwk_c_domain_ui,
    lwk_c_domain_dxm_ui,
    lwk_c_domain_persistent,
    lwk_c_domain_surrogate,
    lwk_c_domain_link,
    lwk_c_domain_linknet,
    lwk_c_domain_comp_linknet,
    lwk_c_domain_step,
    lwk_c_domain_path,
    lwk_c_domain_comp_path,
    lwk_c_domain_linkbase,
    lwk_c_domain_environment_state,
    lwk_c_domain_max
    } lwk_domain, lwk_ptr lwk_domain_ptr;
typedef enum _lwk_query_operator {
    lwk_c_and = 1,
    lwk_c_or,
    lwk_c_not,
    lwk_c_any,
    lwk_c_identity,
    lwk_c_has_properties,
    lwk_c_is_the_source_of,
    lwk_c_is_the_target_of,
    lwk_c_is_some_source_of,
    lwk_c_is_some_target_of,
    lwk_c_is_the_origin_of,
    lwk_c_is_the_destination_of,
    lwk_c_is_some_origin_of,
    lwk_c_is_some_destination_of,
    lwk_c_has_source,
    lwk_c_has_target,
    lwk_c_has_origin,
    lwk_c_has_destination,
    lwk_c_string_is_eql,
    lwk_c_string_is_neq,
    lwk_c_string_contains,
    lwk_c_string_contains_no,
    lwk_c_ddif_string_is_eql,
    lwk_c_ddif_string_is_neq,
    lwk_c_ddif_string_contains,
    lwk_c_ddif_string_contains_no,
    lwk_c_date_is_eql,
    lwk_c_date_is_neq,
    lwk_c_date_is_lss,
    lwk_c_date_is_gtr,
    lwk_c_date_is_leq,
    lwk_c_date_is_geq,
    lwk_c_integer_is_eql,
    lwk_c_integer_is_neq,
    lwk_c_integer_is_lss,
    lwk_c_integer_is_gtr,
    lwk_c_integer_is_leq,
    lwk_c_integer_is_geq,
    lwk_c_float_is_eql,
    lwk_c_float_is_neq,
    lwk_c_float_is_lss,
    lwk_c_float_is_gtr,
    lwk_c_float_is_leq,
    lwk_c_float_is_geq,
    lwk_c_boolean_is_eql,
    lwk_c_boolean_is_neq,
    lwk_c_boolean_is_true,
    lwk_c_boolean_is_false,
    lwk_c_property_value,
    lwk_c_some_property_value,
    lwk_c_string_literal,
    lwk_c_ddif_string_literal,
    lwk_c_date_literal,
    lwk_c_integer_literal,
    lwk_c_float_literal,
    lwk_c_boolean_literal
    } lwk_query_operator, lwk_ptr lwk_query_operator_ptr;
typedef struct _lwk_query_node {
    lwk_query_operator lwk_operator;
    lwk_any_pointer lwk_operand_1;
    lwk_any_pointer lwk_operand_2;
    } lwk_query_node;
typedef lwk_query_node lwk_ptr lwk_query_node_ptr;
typedef lwk_query_node_ptr lwk_query_expression;
typedef enum _lwk_set_operation {
    lwk_c_set_property = 1,
    lwk_c_clear_property,
    lwk_c_delete_property,
    lwk_c_add_property,
    lwk_c_remove_property
    } lwk_set_operation, lwk_ptr lwk_set_operation_ptr;
typedef enum _lwk_transaction {
    lwk_c_transact_read = 1,
    lwk_c_transact_read_write,
    lwk_c_transact_commit,
    lwk_c_transact_rollback
    } lwk_transaction, lwk_ptr lwk_transaction_ptr;
typedef enum _lwk_reason {
    lwk_c_reason_goto = 0,
    lwk_c_reason_visit,
    lwk_c_reason_show_links,
    lwk_c_reason_start_link,
    lwk_c_reason_complete_link,
    lwk_c_reason_annotate,
    lwk_c_reason_menu_pulldown,
    lwk_c_reason_env_change
    } lwk_reason, lwk_ptr lwk_reason_ptr;
typedef enum _lwk_environment_change {
    lwk_c_env_change_min = 0,
    lwk_c_env_appl_highlight = 0,
    lwk_c_env_active_comp_linknet,
    lwk_c_env_recording_linknet,
    lwk_c_env_active_path,
    lwk_c_env_active_path_index,
    lwk_c_env_active_comp_path,
    lwk_c_env_recording_path,
    lwk_c_env_followed_step,
    lwk_c_env_new_link,
    lwk_c_env_pending_source,
    lwk_c_env_pending_target,
    lwk_c_env_followed_link,
    lwk_c_env_follow_destination,
    lwk_c_env_default_operation,
    lwk_c_env_default_highlight,
    lwk_c_env_default_relationship,
    lwk_c_env_default_retain_source,
    lwk_c_env_default_retain_target,
    lwk_c_env_retain_source,
    lwk_c_env_retain_target,
    lwk_c_env_change_max
    } lwk_environment_change, lwk_ptr lwk_environment_change_ptr;
typedef enum _lwk_follow_type {
    lwk_c_follow_go_to = 1,
    lwk_c_follow_visit,
    lwk_c_follow_implicit_go_to
    } lwk_follow_type, lwk_ptr lwk_follow_type_ptr;
typedef enum _lwk_dxm_menu_action {
    lwk_c_dxm_menu_go_to = 0,
    lwk_c_dxm_menu_visit,
    lwk_c_dxm_menu_step_forward,
    lwk_c_dxm_menu_show_links,
    lwk_c_dxm_menu_show_history,
    lwk_c_dxm_menu_go_back,
    lwk_c_dxm_menu_start_link,
    lwk_c_dxm_menu_comp_link,
    lwk_c_dxm_menu_comp_link_dialog,
    lwk_c_dxm_menu_annotate,
    lwk_c_dxm_menu_highlight_on_off,
    lwk_c_dxm_menu_highlight_dialog
    } lwk_dxm_menu_action, lwk_ptr lwk_dxm_menu_action_ptr;
#define lwk_c_p_active_comp_linknet "$ActiveCompLinknet"
#define lwk_c_p_active_comp_path "$ActiveCompPath"
#define lwk_c_p_active_path "$ActivePath"
#define lwk_c_p_active_path_index "$ActivePathIndex"
#define lwk_c_p_address "$Address"
#define lwk_c_p_appl_highlight "$ApplHighlight"
#define lwk_c_p_apply_cb "$ApplyCb"
#define lwk_c_p_author "$Author"
#define lwk_c_p_close_view_cb "$CloseViewCb"
#define lwk_c_p_complete_link_cb "$CompleteLinkCb"
#define lwk_c_p_container "$Container"
#define lwk_c_p_container_identifier "$ContainerIdentifier"
#define lwk_c_p_create_surrogate_cb "$CreateSurrogateCb"
#define lwk_c_p_creation_date "$CreationDate"
#define lwk_c_p_default_highlight "$DefaultHighlight"
#define lwk_c_p_default_operation "$DefaultOperation"
#define lwk_c_p_default_relationship "$DefaultRelationship"
#define lwk_c_p_default_retain_source "$DefaultRetainSource"
#define lwk_c_p_default_retain_target "$DefaultRetainTarget"
#define lwk_c_p_description "$Description"
#define lwk_c_p_destination "$Destination"
#define lwk_c_p_destination_operation "$DestinationOperation"
#define lwk_c_p_domain "$Domain"
#define lwk_c_p_element_count "$ElementCount"
#define lwk_c_p_environment_change_cb "$EnvironmentChangeCb"
#define lwk_c_p_environment_manager "$EnvironmentManager"
#define lwk_c_p_environment_state "$EnvironmentState"
#define lwk_c_p_first_step "$FirstStep"
#define lwk_c_p_follow_destination "$FollowDestination"
#define lwk_c_p_follow_type "$FollowType"
#define lwk_c_p_followed_link "$FollowedLink"
#define lwk_c_p_followed_step "$FollowedStep"
#define lwk_c_p_get_surrogate_cb "$GetSurrogateCb"
#define lwk_c_p_identifier "$Identifier"
#define lwk_c_p_inter_links "$InterLinks"
#define lwk_c_p_interval "$Interval"
#define lwk_c_p_keywords "$Keywords"
#define lwk_c_p_last_step "$LastStep"
#define lwk_c_p_linkbase "$Linkbase"
#define lwk_c_p_linkbase_identifier "$LinkbaseIdentifier"
#define lwk_c_p_linkbase_name "$LinkbaseName"
#define lwk_c_p_linknet "$Linknet"
#define lwk_c_p_linknets "$Linknets"
#define lwk_c_p_links "$Links"
#define lwk_c_p_main_widget "$MainWidget"
#define lwk_c_p_message_cb "$MessageCb"
#define lwk_c_p_name "$Name"
#define lwk_c_p_new_link "$NewLink"
#define lwk_c_p_next_step "$NextStep"
#define lwk_c_p_notification_cb "$NotificationCb"
#define lwk_c_p_notifications "$Notifications"
#define lwk_c_p_object_domain "$ObjectDomain"
#define lwk_c_p_object_identifier "$ObjectIdentifier"
#define lwk_c_p_object_name "$ObjectName"
#define lwk_c_p_open "$Open"
#define lwk_c_p_origin "$Origin"
#define lwk_c_p_origin_operation "$OriginOperation"
#define lwk_c_p_path "$Path"
#define lwk_c_p_paths "$Paths"
#define lwk_c_p_pending_source "$PendingSource"
#define lwk_c_p_pending_target "$PendingTarget"
#define lwk_c_p_previous_step "$PreviousStep"
#define lwk_c_p_property_name "$PropertyName"
#define lwk_c_p_recording_linknet "$RecordingLinknet"
#define lwk_c_p_recording_path "$RecordingPath"
#define lwk_c_p_relationship_type "$RelationshipType"
#define lwk_c_p_retain_source "$RetainSource"
#define lwk_c_p_retain_target "$RetainTarget"
#define lwk_c_p_source "$Source"
#define lwk_c_p_source_description "$SourceDescription"
#define lwk_c_p_source_keywords "$SourceKeywords"
#define lwk_c_p_steps "$Steps"
#define lwk_c_p_supported_operations "$SupportedOperations"
#define lwk_c_p_supported_surrogates "$SupportedSurrogates"
#define lwk_c_p_surrogate_sub_type "$SurrogateSubType"
#define lwk_c_p_surrogates "$Surrogates"
#define lwk_c_p_target "$Target"
#define lwk_c_p_target_description "$TargetDescription"
#define lwk_c_p_target_keywords "$TargetKeywords"
#define lwk_c_p_user_data "$UserData"
#define lwk_c_p_value "$Value"
lwk_status lwk_entry  lwk_add_boolean();/*
	lwk_list list;

	lwk_boolean boolean;*/
lwk_status lwk_entry  lwk_add_ddif_string();/*
	lwk_list list;

	lwk_ddif_string compound_string;*/
lwk_status lwk_entry  lwk_add_date();/*
	lwk_list list;

	lwk_date date;*/
lwk_status lwk_entry  lwk_add_element();/*
	lwk_list list;

	lwk_domain domain;

	lwk_any_pointer element;*/
lwk_status lwk_entry  lwk_add_float();/*
	lwk_list list;

	lwk_float floating_point;*/
lwk_status lwk_entry  lwk_add_integer();/*
	lwk_list list;

	lwk_integer integer;*/
lwk_status lwk_entry  lwk_add_object();/*
	lwk_list list;

	lwk_object object;*/
lwk_status lwk_entry  lwk_add_string();/*
	lwk_list list;

	lwk_string string;*/
lwk_status lwk_entry  lwk_apply();/*
	lwk_ui ui_object;

	lwk_string operation;

	lwk_surrogate surrogate;*/
lwk_status lwk_entry  lwk_close();/*
	lwk_linkbase linkbase;*/
lwk_status lwk_entry  lwk_confirm_apply();/*
	lwk_ui ui_object;

	lwk_surrogate surrogate;*/
lwk_status lwk_entry  lwk_copy();/*
	lwk_object existing_object;

	lwk_object_ptr new_object;*/
lwk_status lwk_entry  lwk_copy_aggregate();/*
	lwk_object existing_object;

	lwk_object_ptr new_object;*/
lwk_status lwk_entry  lwk_create();/*
	lwk_domain domain;

	lwk_object_ptr new_object;*/
lwk_status lwk_entry  lwk_create_list();/*
	lwk_domain domain;

	lwk_integer estimated_count;

	lwk_list_ptr new_list;*/
lwk_status lwk_entry  lwk_create_set();/*
	lwk_domain domain;

	lwk_integer estimated_count;

	lwk_set_ptr new_set;*/
lwk_status lwk_entry  lwk_date_to_time();/*
	lwk_date date;

	lwk_any_pointer time;*/
lwk_status lwk_entry  lwk_delete();/*
	lwk_object_ptr object;*/
lwk_status lwk_entry  lwk_delete_ddif_string();/*
	lwk_ddif_string_ptr compound_string;*/
lwk_status lwk_entry  lwk_delete_date();/*
	lwk_date_ptr date;*/
lwk_status lwk_entry  lwk_delete_encoding();/*
	lwk_encoding_ptr encoding;*/
lwk_status lwk_entry  lwk_delete_string();/*
	lwk_string_ptr string;*/
lwk_status lwk_entry  lwk_drop();/*
	lwk_object object;*/
lwk_status lwk_entry  lwk_enumerate_properties();/*
	lwk_object object;

	lwk_set_ptr properties;*/
lwk_status lwk_entry  lwk_export();/*
	lwk_persistent object;

	lwk_encoding_ptr encoding;

	lwk_integer_ptr encoding_size;*/
lwk_status lwk_entry  lwk_get_domain();/*
	lwk_object object;

	lwk_domain_ptr domain;*/
lwk_status lwk_entry  lwk_get_object_descriptor();/*
	lwk_persistent persistent_object;

	lwk_object_descriptor_ptr object_descriptor;*/
lwk_status lwk_entry  lwk_get_object_id();/*
	lwk_persistent persistent_object;

	lwk_object_id_ptr object_id;*/
lwk_status lwk_entry  lwk_get_value();/*
	lwk_object object;

	lwk_string property_name;

	lwk_domain domain;

	lwk_any_pointer value;*/
lwk_status lwk_entry  lwk_get_value_domain();/*
	lwk_object object;

	lwk_string property_name;

	lwk_domain_ptr domain;*/
lwk_status lwk_entry  lwk_get_value_list();/*
	lwk_object object;

	lwk_string property_name;

	lwk_list_ptr value_list;*/
lwk_status lwk_entry  lwk_import();/*
	lwk_domain domain;

	lwk_encoding encoding;

	lwk_object_ptr object;*/
lwk_status lwk_entry  lwk_initialize();/*
	lwk_boolean_ptr hyperinvoked;

	lwk_string_ptr operation;

	lwk_surrogate_ptr surrogate;*/
lwk_status lwk_entry  lwk_is_multi_valued();/*
	lwk_object object;

	lwk_string property_name;

	lwk_boolean_ptr answer;*/
lwk_status lwk_entry  lwk_iterate();/*
	lwk_object object;

	lwk_domain domain;

	lwk_closure closure;

	lwk_callback callback_routine;

	lwk_termination_ptr termination;*/
lwk_status lwk_entry  lwk_memory_statistics();
lwk_status lwk_entry  lwk_open();/*
	lwk_string linkbase_identifier;

	lwk_boolean create_option;

	lwk_linkbase_ptr linkbase;*/
lwk_status lwk_entry  lwk_query();/*
	lwk_object object;

	lwk_domain domain;

	lwk_query_expression expression;

	lwk_closure closure;

	lwk_callback callback_routine;

	lwk_termination_ptr termination;*/
lwk_status lwk_entry  lwk_remove_boolean();/*
	lwk_list list;

	lwk_boolean_ptr boolean;*/
lwk_status lwk_entry  lwk_remove_ddif_string();/*
	lwk_list list;

	lwk_ddif_string_ptr compound_string;*/
lwk_status lwk_entry  lwk_remove_date();/*
	lwk_list list;

	lwk_date_ptr date;*/
lwk_status lwk_entry  lwk_remove_element();/*
	lwk_list list;

	lwk_domain domain;

	lwk_any_pointer element;*/
lwk_status lwk_entry  lwk_remove_float();/*
	lwk_list list;

	lwk_float_ptr floating_point;*/
lwk_status lwk_entry  lwk_remove_integer();/*
	lwk_list list;

	lwk_integer_ptr integer;*/
lwk_status lwk_entry  lwk_remove_object();/*
	lwk_list list;

	lwk_object_ptr element;*/
lwk_status lwk_entry  lwk_remove_string();/*
	lwk_list list;

	lwk_string_ptr string;*/
lwk_status lwk_entry  lwk_retrieve();/*
	lwk_object_id object_id;

	lwk_persistent_ptr persistent_object;*/
lwk_status lwk_entry  lwk_set_value();/*
	lwk_object object;

	lwk_string property_name;

	lwk_domain domain;

	lwk_any_pointer value;

	lwk_set_operation operation;*/
lwk_status lwk_entry  lwk_set_value_list();/*
	lwk_object object;

	lwk_string property_name;

	lwk_list value_list;

	lwk_set_operation operation;*/
lwk_status lwk_entry  lwk_status_to_ddif_string();/*
	lwk_status status_code;

	lwk_ddif_string_ptr message;*/
lwk_status lwk_entry  lwk_status_to_string();/*
	lwk_status status_code;

	lwk_string_ptr message;*/
lwk_status lwk_entry  lwk_store();/*
	lwk_persistent object;

	lwk_linkbase linkbase;*/
lwk_status lwk_entry  lwk_surrogate_is_highlighted();/*
	lwk_ui ui_object;

	lwk_surrogate surrogate;

	lwk_boolean_ptr answer;*/
lwk_status lwk_entry  lwk_time_to_date();/*
	lwk_any_pointer time;

	lwk_date_ptr date;*/
lwk_status lwk_entry  lwk_transact();/*
	lwk_linkbase linkbase;

	lwk_transaction transaction;*/
#define lwk_c_view_op_id "View"
#define lwk_c_edit_op_id "Edit"
#define lwk_c_activate_op_id "Activate"
#define lwk_c_annotate_op_id "Annotate"
#endif /* lwk__def */
