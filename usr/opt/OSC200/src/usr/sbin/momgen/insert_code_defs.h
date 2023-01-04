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
 * @(#)$RCSfile: insert_code_defs.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 22:10:17 $
 */
 /*
 **++
 **  FACILITY:	MOMGEN
 **
 **  MODULE DESCRIPTION:
 **
 **	 This header file for the MOMGEN builder containing the INSERT-CODE definitions .
 **
 **  AUTHORS:
 **
 **	Gary J. Allison
 **
 **  CREATION DATE:  21-Sep-1991
 **
 **  MODIFICATION HISTORY:
 **
 **--
 */

#define code_insert_start   "/*-insert-code-"
#define code_insert_start_len 15
#define code_insert_end     "-*/"

#define code_default_error_reply "default-error-reply"
#define code_duplicate_error_reply "duplicate-error-reply"
#define code_define_version "define-version"
#define code_perform_action "perform-action"
#define code_get_attr_oid   "get-attr-oid"
#define code_name_oid	    "name-oid"
#define code_import_name_oid "import-name-oid"
#define code_delete_instance "delete-instance"
#define code_map_oid	    "map-oid"
#define code_identifier_attr "identifier-attr"
#define code_map_oid_list   "map-oid-list"
#define code_class_compare  "class-compare"
#define code_list_status    "list-status"
#define code_set_char 	    "set-char"
#define code_define_protos  "define-protos"
#define code_list_char 	    "list-char"
#define code_list_id 	    "list-id"
#define code_list_counters  "list-counters"
#define code_external_routines	  "external-routines"
#define code_action_exception "action-exception"
#define code_action_success_oid "action-success-oid"
#define code_action_import_oids "action-import-oids"
#define code_delete_success_oid "delete-success-oid"
#define code_delete_import_oids "delete-import-oids"
#define code_build_avl_type "build-avl-type"
#define code_descrip_cd	    "descrip-cd"
#define code_descrip_cd_dependency  "descrip-cd-dependency"
#define code_add_check_attributes "add-check-attributes"
#define code_mom_check_attributes "mom-check-attributes"
#define code_descrip_obj_action "descrip-obj-action"
#define code_descrip_dependency "descrip-dependency"
#define code_action_success_reply "action-success-reply"
#define code_action_switch  "action-switch"
#define code_action_case    "action-case"
#define code_init_exports   "init-exports"
#define code_init_imports   "init-imports"
#define code_init_defines   "init-defines"
#define code_iso_dna	    "iso-dna"
#define code_init_dereg	    "dereg-all"
#define code_init_fixup     "fixup-exports"
#define code_register	    "register"
#define code_create_args    "create-arguments"
#define code_get_all_attr   "get-all-attributes"
#define code_get_items	    "get-items"
#define code_add_new_instances "add-new-instance"
#define code_add_new_instances_call "add-new-instance-call"
#define code_get_groups	    "get-groups"
#define code_list_attr	    "list-attr"
#define code_check_types    "check-datatypes"
#define code_check_values   "check-values"
#define code_set_attributes "set-attributes"
#define code_switch_get_candidate "switch-get-candidate"
#define code_select_routine	  "select-routine"
#define code_create_select_locate 	  "create-select-locate"
#define code_create_select_routine	  "create-select-routine"
#define code_reply_select_routine	  "reply-select-routine"
#define code_perform_inits	  "perform-inits"
#define code_link_dependencies	  "link-dependencies"
#define code_compiles		  "compiles"
#define code_define_classes	  "define-classes"
#define code_defs		  "defs"
#define code_commands		  "commands"
#define code_instance_avl	  "instance-avl"
#define code_next_oid		  "getnext-attribute-oid"
#define code_first_oid            "getnext-first-attribute"
#define code_class_code		  "class-code"
#define code_instance_construct   "instance-construct"
#define code_par_inst_construct   "parent-instance-construct"
#define code_classfile_list       "list_of_mom_class_files"
#define code_class_src_list       "mom_class_src_list"
#define code_class_obj_list       "mom_class_obj_list"
#define code_extern_common  	  "code-extern-common"
#define code_init_instance	  "init-instance"
#define code_init_trap            "init-trap"
#define code_trap_export_defs     "trap-export-defs"
#define code_trap_arg_inits       "trap-arg-inits"
#define code_trap_arg_code        "trap_arg_code"
#define code_trap_cond            "trap-cond"
#define code_trap_polling         "trap-polling"
