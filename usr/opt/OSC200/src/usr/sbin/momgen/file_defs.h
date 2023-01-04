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
 * @(#)$RCSfile: file_defs.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 22:10:12 $
 */
 /*
 **++
 **  FACILITY:	MOMGEN
 **
 **  MODULE DESCRIPTION:
 **
 **	 This header file for the MOMGEN builder.
 **
 **  AUTHORS:
 **
 **	Rich Bouchard
 **
 **  CREATION DATE:  18-March-1991
 **
 **  MODIFICATION HISTORY:
 **
 **	X-2	Marc Nozell	21-May-1991
 **	
 ** 	Added GROUP_* defines
 **
 ** 	Created enumerated type for the momgen return status. More will be
 ** 	addeded later.
 **
 ** 	Always include ssdef and rmsdef.
 **
 **	Changed the logicals used to find the file templates when momgen is run.
 **	The logicals are as follows:
 **
 ** 		- mom_in - Location of the template files, normally FS$:[MOMGEN.SRC]
 **
 ** 		- mom_out_src - Where to put the generated source files. Normally
 **	 	DEV:[DIR.ENTITY.SRC] 
 **
 ** 		- mom_out_mms - Where to put the MMS description file. Normally
 **	 	DEV:[DIR.ENTITY]  
 **
 ** 		- mom_out_obj - Where to put the linker options file, normally where
 ** 		the objects will go.  Normally DEV:[DIR.ENTITY.OBJ]
 **
 **	Bumped up ident.
 **
 **	X-3	Marc Nozell	28-May-1991
 **	Changed the mom structure's current_class to a pointer to the current class structure. 
 **--
 */
#define file_prototypes_input		"build_prototypes.h"
#define file_prototypes_output		"mom_prototypes.h"
#define file_oid_export_input		"build_export_oids.h"
#define file_oid_export_output		"export_oids.h"
#define file_oid_import_input		"build_import_oids.h"
#define file_oid_import_output		"import_oids.h"
#define file_util_input	    		"build_utility_routines.c"
#define file_util_output    		"utility_routines.c"
#define file_get_input	    		"build_get.c"
#define file_get_output	    		"get.c"               
#define file_get_null_input		"build_get_null.c"
#define file_cancel_get_null_input	"build_cancel_null.c"
#define file_cancel_get_output		"cancel.c"
#define file_set_input	    		"build_set.c"
#define file_set_output	    		"set.c"
#define file_set_null_input		"build_set_null.c"
#define file_extern_common_input	"build_extern_common.h"
#define file_extern_common_output	"extern_common.h"
#define file_find_input   		"build_find_instance.c"
#define file_find_output  		"find_instance.c"
#define file_multi_find_output  	"%sfind_instance.c"
#define file_delete_input   		"build_delete.c"
#define file_delete_output  		"delete.c"
#define file_delete_null_input  	"build_delete_null.c"
#define file_create_input   		"build_create.c"
#define file_create_output  		"create.c"
#define file_create_null_input  	"build_create_null.c"
#define file_action_input   		"build_action.c"
#define file_action_null_input 		"build_action_null.c"
#define file_action_action  		"build_action_action.c"
#define file_action_create  		"build_action_create.c"
#define file_action_delete  		"build_action_delete.c"
#define file_action_output  		"action.c"
#define file_perform_action		"build_perform_action.c"
#define file_class_perform_action 	"%sperform_%s.c"
#define file_access_input   		"build_access.c"
#define file_access_output  		"access.c"
#define file_directive_input  		"build_directive.c"
#define file_directive_output 		"directive.c"
#define file_init_input	    		"build_init.c"
#define file_init_output    		"init.c"
#define file_command_input		"build_command.com"
#define file_command_mp_input		"build_multi_command.com"
#define file_command_output		"mom.com"
#define file_makefile_input             "build_mom.make"
#define file_makefile_output            "Makefile"
#define file_descrip_input  		"build_descrip.mms"
#define file_multi_descrip_input    	"build_multi_descrip.mms"
#define file_descrip_output 		"descrip.mms"
#define file_options_input  		"build_options.opt"
#define file_options_mp_input  		"build_multi_options.opt"
#define file_options_output 		"mom_options.opt"
#define file_get_candidate_inst_input 	"build_get_candidate_instance.c"
#define file_switch_get_candidate_input "build_switch_get_candidate.c"
#define file_get_candidate_output 	"get_instance.c"
#define file_class_get_candidate_output "%sget_instance.c"
#define file_perform_init_input   	"build_perform_init.c"
#define file_perform_init_inst_input 	"build_init_perform_instance.c"
#define file_perform_init_output    	"perform_init.c"
#define file_class_perform_init_output	"%sperform_init.c"
#define file_perform_get_input   	"build_perform_get.c"
#define file_perform_snmp_get_input   	"build_perform_snmp_get.c"
#define file_perform_get_output    	"perform_get.c"
#define file_class_perform_get_output	"%sperform_get.c"
#define file_perform_set_input   	"build_perform_set.c"
#define file_perform_set_output    	"perform_set.c"
#define file_class_perform_set_output	"%sperform_set.c"
#define file_perform_create_inst_input 	"build_perform_create_instance.c"
#define file_perform_create_input   	"build_perform_create.c"
#define file_perform_create_output    	"perform_create.c"
#define file_class_perform_create_output "%sperform_create.c"
#define file_perform_delete_input   	"build_perform_delete.c"
#define file_perform_delete_output    	"perform_delete.c"
#define file_class_perform_delete_output "%sperform_delete.c"
#define file_common_input		"build_common.h"
#define file_common_output		"common.h"
#define file_instance_input		"build_instance_avl.c"
#define file_instance_null_input	"build_instance_avl_null.c"
#define file_getnext_input		"build_getnext.c"
#define file_getnext_null_input		"build_action_null.c"
#define file_getnext_output		"getnext.c"
#define file_perform_table_getnext_input "build_perform_table_getnext.c"
#define file_perform_nontable_getnext_input "build_perform_nontable_getnext.c"
#define file_perform_getnext_output	"%sperform_getnext.c"
#define file_trap_input			"build_trap.c"
#define file_trap_output		"trap.c"
