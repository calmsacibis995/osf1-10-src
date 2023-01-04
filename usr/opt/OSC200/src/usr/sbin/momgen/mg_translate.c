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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: mg_translate.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/06/15 11:55:47 $";
#endif
/*
**+
**  Copyright (c) Digital Equipment Corporation, 1991
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**
**++
**  FACILITY:  VMS MOM Generator
**
**  MODULE DESCRIPTION:
**
**      This module implements the VMS MOM Generator.  It provides
**	a method for generating the basic framework code for a 
**	Managed Object Module (MOM), based on a description of
**	the MOM and its entity classes.
**
**  AUTHORS:
**
**	Rich Bouchard, Jr.
**	VMS System Management Engineering
**
**  CREATION DATE:  April 17, 1991
**
**  MODIFICATION HISTORY:
**
**	F-19		    Marc Nozell     	    15-Jun-1992
**		Remove references to module.
**
**	F-6		    Mike Densmore	    21-May-1992
**		Modified for use on U**X platforms.
**
**	F-5		    Gary J. Allison	    19-Aug-1991
**		Update for 1H Agent. Restructured files.
**
**	F-4	RJB1055	    Richard J. Bouchard Jr. 23-Jul-1991
**		Update for 1G Agent
**
**	F-3		    Marc Nozell		    24-May-1991
**                            
**	Support multiple classes (insert_init_exports)                    
**
**	Make GET.C to compile (insert_select_routine)
**
**	Changed mom structure's current_class to point to the current CLASS_DEF. 
**
**	Resolved possible name conflicts of attributes across multiple classes (insert_init_exports
**	+ 1 small bug, insert_init_fixup, insert_import_attr, insert_get_items)
**
**	Fixed declaration of perform_{get|set} routine. (insert_external_routines)
**
**	F-2	Marc Nozell	16-May-1991
**
**	Change the location of the "common" include file. 
**
**	Change case of the ERROR_CONDITION macro. (various)
**
**	DESCRIP.MMS dependencies for <<entity>>_perform_set.c
**	<<entity>>_perform_init.c, <<entity>>_perform_get_candidate.c
**	(insert_compiles) 
**
**	Add more debugging information (build_mom)
**
**	Return status of type momgen_status from all routines.
**
**	25-Sep-1992	Mike Densmore	Modified for ANSI Standard C
**--
*/

#ifdef VMS
#include "vaxcshr.h"		/* translate from VAX C to DEC C RTL names */
#endif 
#include "mom_defs.h"
#include "string_defs.h"
#include "insert_code_defs.h"
#include "mg_prototypes.h"
#include <string.h>

#if defined(sun) || defined(sparc)
int	insert_perform_action ();
int	insert_set_char ();
int	insert_name_oid ();
int	insert_action_exception ();
int	insert_action_success_oid ();
int	insert_define_protos ();
int	insert_duplicate_error_reply ();
int	insert_default_error_reply ();
int	insert_delete_success_oid ();
int	insert_instance_avl ();
int	insert_map_oid ();
int	insert_identifier_attr ();
int	insert_map_oid_list ();
int	insert_descrip_cd ();
int	insert_action_success_reply ();
int	insert_descrip_cd_dependency ();
int	insert_descrip_obj_action ();
int	insert_descrip_dependency ();
int	insert_action_switch ();
int	insert_commands ();
int	insert_get_attr_oid ();
int	insert_define_version ();
int	insert_init_exports ();
int	insert_init_defines ();
int	insert_init_imports ();
int	insert_iso_dna ();
int	insert_dereg_all ();
int	insert_init_register ();
int	insert_mom_check_attributes ();
int	insert_add_check_attributes ();
int	insert_create_args ();
int	insert_get_all_attr ();
int	insert_get_items ();
int	insert_get_groups ();
int	insert_list_attr ();
int	insert_list_status ();
int	insert_class_compare ();
int	insert_build_avl_type ();
int	insert_list_id ();
int	insert_list_char ();
int	insert_list_counters ();
int	insert_check_types ();
int	insert_extern_common ();
int	insert_check_values ();
int	insert_set_attributes ();
int	insert_switch_get_candidate ();
int	insert_select_routine ();
int	insert_create_select_routine ();
int	insert_external_routines ();
int	insert_reply_select_routine ();
int	insert_create_select_locate ();
int	insert_perform_inits ();
int	insert_link_dependencies ();
int	insert_compiles ();
int	insert_define_classes ();
int	insert_add_new_instances ();
int	insert_add_new_instances_call ();
int	insert_delete_instance ();
int	insert_defs ();
int	insert_next_oid ();
int	insert_first_oid ();
int	insert_class_code ();
int	insert_instance_construct ();
int	insert_par_inst_construct ();
int	insert_makefile_macro_file_list ();
int	insert_makefile_macro_src ();
int	insert_makefile_macro_obj ();
int	insert_init_instance ();
int	insert_init_trap ();
int	insert_trap_cond ();
int	insert_trap_polling ();
#else
int	insert_perform_action (MOM_BUILD *, FILE *);
int	insert_set_char (MOM_BUILD *, FILE *);
int	insert_name_oid (MOM_BUILD *, FILE *);
int	insert_action_exception (MOM_BUILD *, FILE *);
int	insert_action_success_oid (MOM_BUILD *, FILE *);
int	insert_define_protos (MOM_BUILD *, FILE *);
int	insert_duplicate_error_reply (MOM_BUILD *, FILE *);
int	insert_default_error_reply (MOM_BUILD *, FILE *);
int	insert_delete_success_oid (MOM_BUILD *, FILE *);
int	insert_instance_avl (MOM_BUILD *, FILE *);
int	insert_map_oid (MOM_BUILD *, FILE *);
int	insert_identifier_attr (MOM_BUILD *, FILE *);
int	insert_map_oid_list (MOM_BUILD *, FILE *);
int	insert_descrip_cd (MOM_BUILD *, FILE *);
int	insert_action_success_reply (MOM_BUILD *, FILE *);
int	insert_descrip_cd_dependency (MOM_BUILD *, FILE *);
int	insert_descrip_obj_action (MOM_BUILD *, FILE *);
int	insert_descrip_dependency (MOM_BUILD *, FILE *);
int	insert_action_switch (MOM_BUILD *, FILE *);
int	insert_commands (MOM_BUILD *, FILE *);
int	insert_get_attr_oid (MOM_BUILD *, FILE *);
int	insert_define_version (MOM_BUILD *, FILE *);
int	insert_init_exports (MOM_BUILD *, FILE *);
int	insert_init_defines (MOM_BUILD *, FILE *);
int	insert_init_imports (MOM_BUILD *, FILE *);
int	insert_iso_dna (MOM_BUILD *, FILE *);
int	insert_dereg_all (MOM_BUILD *, FILE *);
int	insert_init_register (MOM_BUILD *, FILE *);
int	insert_mom_check_attributes (MOM_BUILD *, FILE *);
int	insert_add_check_attributes (MOM_BUILD *, FILE *);
int	insert_create_args (MOM_BUILD *, FILE *);
int	insert_get_all_attr (MOM_BUILD *, FILE *);
int	insert_get_items (MOM_BUILD *, FILE *);
int	insert_get_groups (MOM_BUILD *, FILE *);
int	insert_list_attr (MOM_BUILD *, FILE *);
int	insert_list_status (MOM_BUILD *, FILE *);
int	insert_class_compare (MOM_BUILD *, FILE *);
int	insert_build_avl_type (MOM_BUILD *, FILE *);
int	insert_list_id (MOM_BUILD *, FILE *);
int	insert_list_char (MOM_BUILD *, FILE *);
int	insert_list_counters (MOM_BUILD *, FILE *);
int	insert_check_types (MOM_BUILD *, FILE *);
int	insert_extern_common (MOM_BUILD *, FILE *);
int	insert_check_values (MOM_BUILD *, FILE *);
int	insert_set_attributes (MOM_BUILD *, FILE *);
int	insert_switch_get_candidate (MOM_BUILD *, FILE *);
int	insert_select_routine (MOM_BUILD *, FILE *);
int	insert_create_select_routine (MOM_BUILD *, FILE *);
int	insert_external_routines (MOM_BUILD *, FILE *);
int	insert_reply_select_routine (MOM_BUILD *, FILE *);
int	insert_create_select_locate (MOM_BUILD *, FILE *);
int	insert_perform_inits (MOM_BUILD *, FILE *);
int	insert_link_dependencies (MOM_BUILD *, FILE *);
int	insert_compiles (MOM_BUILD *, FILE *);
int	insert_define_classes (MOM_BUILD *, FILE *);
int	insert_add_new_instances (MOM_BUILD *, FILE *);
int	insert_add_new_instances_call (MOM_BUILD *, FILE *);
int	insert_delete_instance (MOM_BUILD *, FILE *);
int	insert_defs (MOM_BUILD *, FILE *);
int	insert_next_oid (MOM_BUILD *, FILE *);
int	insert_first_oid (MOM_BUILD *, FILE *);
int	insert_class_code (MOM_BUILD *, FILE *);
int	insert_instance_construct (MOM_BUILD *, FILE *);
int	insert_par_inst_construct (MOM_BUILD *, FILE *);
int	insert_makefile_macro_file_list (MOM_BUILD *, FILE *);
int	insert_makefile_macro_src (MOM_BUILD *, FILE *);
int	insert_makefile_macro_obj (MOM_BUILD *, FILE *);
int	insert_init_instance (MOM_BUILD *, FILE *);
int	insert_init_trap (MOM_BUILD *, FILE *);
int	insert_trap_cond (MOM_BUILD *, FILE *);
int	insert_trap_polling (MOM_BUILD *, FILE *);
#endif

char temp_buffer[128];


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_code
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file.
**
**	This routine generates the code to replace a "code marker" in a build
**	file.  It determines which marker was present, and dispatches to
**	the appropriate "insert_" routine above.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**	buffer		The buffer from the build file containing the "code marker".
**	outfile		File handle of the current output file.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**
**--
*/
int insert_code( mom,
		 buffer,
		 outfile)

MOM_BUILD *mom;
char *buffer;
FILE *outfile;

{
    char    *code_marker;
    char    *tmp;
    int	    status;

    code_marker = buffer + code_insert_start_len;

    tmp = (char *)strstr(code_marker, code_insert_end);
    if (tmp == NULL)
    {
	fprintf(stderr,"Error - invalid format for code marker '%s'\n", buffer);
	return;
    }

    *tmp = '\0';

    if (mom->debug)
        fprintf(mom->log_file,"Inserting code %s\n",code_marker);

    if (strcmp(code_marker, code_perform_action) == 0)
	insert_perform_action(mom, outfile);
    else if (strcmp(code_marker, code_set_char) == 0)
	insert_set_char(mom, outfile);
    else if (strcmp(code_marker, code_name_oid) == 0)
	insert_name_oid(mom, outfile);
    else if (strcmp(code_marker, code_action_exception) == 0)
	insert_action_exception(mom, outfile);
    else if (strcmp(code_marker, code_action_success_oid) == 0)
	insert_action_success_oid(mom, outfile);
    else if (strcmp(code_marker, code_define_protos) == 0)
	insert_define_protos(mom, outfile);
    else if (strcmp(code_marker, code_duplicate_error_reply) == 0)
	insert_duplicate_error_reply(mom, outfile);
    else if (strcmp(code_marker, code_default_error_reply) == 0)
	insert_default_error_reply(mom, outfile);
    else if (strcmp(code_marker, code_delete_success_oid) == 0)
	insert_delete_success_oid(mom, outfile);
    else if (strcmp(code_marker, code_instance_avl) == 0)
	insert_instance_avl(mom, outfile);
    else if (strcmp(code_marker, code_map_oid) == 0)
	insert_map_oid(mom, outfile);
    else if (strcmp(code_marker, code_identifier_attr) == 0)
	insert_identifier_attr(mom, outfile);
    else if (strcmp(code_marker, code_map_oid_list) == 0)
	insert_map_oid_list(mom, outfile);
    else if (strcmp(code_marker, code_descrip_cd) == 0)
	insert_descrip_cd(mom, outfile);
    else if (strcmp(code_marker, code_action_success_reply) == 0)
	insert_action_success_reply(mom, outfile);
    else if (strcmp(code_marker, code_descrip_cd_dependency) == 0)
	insert_descrip_cd_dependency(mom, outfile);
    else if (strcmp(code_marker, code_descrip_obj_action) == 0)
	insert_descrip_obj_action(mom, outfile);
    else if (strcmp(code_marker, code_descrip_dependency) == 0)
	insert_descrip_dependency(mom, outfile);
    else if (strcmp(code_marker, code_action_switch) == 0)
	insert_action_switch(mom, outfile);
    else if (strcmp(code_marker, code_commands) == 0)
	insert_commands(mom, outfile);
    else if (strcmp(code_marker, code_get_attr_oid) == 0)
	insert_get_attr_oid(mom, outfile);
    else if (strcmp(code_marker, code_define_version) == 0)
	insert_define_version(mom, outfile);
    else if (strcmp(code_marker, code_init_exports) == 0)
	insert_init_exports(mom, outfile);
    else if (strcmp(code_marker, code_init_defines) == 0)
	insert_init_defines(mom, outfile);
    else if (strcmp(code_marker, code_init_imports) == 0)
	insert_init_imports(mom, outfile);
    else if (strcmp(code_marker, code_iso_dna) == 0)
	insert_iso_dna(mom, outfile);
    else if (strcmp(code_marker, code_init_dereg) == 0)
	insert_dereg_all(mom, outfile);
    else if (strcmp(code_marker, code_register) == 0)
	insert_init_register(mom, outfile);
    else if (strcmp(code_marker, code_mom_check_attributes) == 0)
	insert_mom_check_attributes(mom, outfile);
    else if (strcmp(code_marker, code_add_check_attributes) == 0)	
	insert_add_check_attributes(mom, outfile);
    else if (strcmp(code_marker, code_create_args) == 0)
	insert_create_args(mom, outfile);
    else if (strcmp(code_marker, code_get_all_attr) == 0)
	insert_get_all_attr(mom, outfile);
    else if (strcmp(code_marker, code_get_items) == 0)
	insert_get_items(mom, outfile);
    else if (strcmp(code_marker, code_get_groups) == 0)
	insert_get_groups(mom, outfile);
    else if (strcmp(code_marker, code_list_attr) == 0)
	insert_list_attr(mom, outfile);
    else if (strcmp(code_marker, code_list_status) == 0)
	insert_list_status(mom, outfile);
    else if (strcmp(code_marker, code_class_compare) == 0)
	insert_class_compare(mom, outfile);
    else if (strcmp(code_marker, code_build_avl_type) == 0)
	insert_build_avl_type(mom, outfile);
    else if (strcmp(code_marker, code_list_id) == 0)
	insert_list_id(mom, outfile);
    else if (strcmp(code_marker, code_list_char) == 0)
	insert_list_char(mom, outfile);
    else if (strcmp(code_marker, code_list_counters) == 0)
	insert_list_counters(mom, outfile);
    else if (strcmp(code_marker, code_check_types) == 0)
	insert_check_types(mom, outfile);
    else if (strcmp(code_marker, code_extern_common) == 0)
	insert_extern_common(mom, outfile);
    else if (strcmp(code_marker, code_check_values) == 0)
	insert_check_values(mom, outfile);
    else if (strcmp(code_marker, code_set_attributes) == 0)
	insert_set_attributes(mom, outfile);
    else if (strcmp(code_marker, code_switch_get_candidate) == 0)
	insert_switch_get_candidate(mom, outfile);
    else if (strcmp(code_marker, code_select_routine) == 0)
	insert_select_routine(mom, outfile);
    else if (strcmp(code_marker, code_create_select_routine) == 0)
	insert_create_select_routine(mom, outfile);
    else if (strcmp(code_marker, code_external_routines) == 0)
	insert_external_routines(mom, outfile);
    else if (strcmp(code_marker, code_reply_select_routine) == 0)
	insert_reply_select_routine(mom, outfile);
    else if (strcmp(code_marker, code_create_select_locate) == 0)
	insert_create_select_locate(mom, outfile);
    else if (strcmp(code_marker, code_perform_inits) == 0)
	insert_perform_inits(mom, outfile);
    else if (strcmp(code_marker, code_link_dependencies) == 0)
	insert_link_dependencies(mom, outfile);
    else if (strcmp(code_marker, code_compiles) == 0)
	insert_compiles(mom, outfile);
    else if (strcmp(code_marker, code_define_classes) == 0)
	insert_define_classes(mom, outfile);
    else if (strcmp(code_marker, code_add_new_instances ) == 0)
	insert_add_new_instances(mom, outfile);
    else if (strcmp(code_marker, code_add_new_instances_call ) == 0)
	insert_add_new_instances_call(mom, outfile);
    else if (strcmp(code_marker, code_delete_instance ) == 0)
	insert_delete_instance(mom, outfile);
    else if (strcmp(code_marker, code_defs) == 0)
	insert_defs(mom, outfile);
    else if (strcmp(code_marker, code_next_oid) == 0)
	insert_next_oid(mom, outfile);
    else if (strcmp(code_marker, code_first_oid) == 0)
	insert_first_oid(mom, outfile);
    else if (strcmp(code_marker, code_class_code) == 0)
	insert_class_code(mom, outfile);
    else if (strcmp(code_marker, code_instance_construct) == 0)
	insert_instance_construct(mom, outfile);
    else if (strcmp(code_marker, code_par_inst_construct) == 0)
	insert_par_inst_construct(mom, outfile);
    else if (strcmp(code_marker, code_classfile_list) == 0)
        insert_makefile_macro_file_list(mom, outfile);
    else if (strcmp(code_marker, code_class_src_list) == 0)
        insert_makefile_macro_src(mom, outfile);
    else if (strcmp(code_marker, code_class_obj_list) == 0)
        insert_makefile_macro_obj(mom, outfile);
    else if (strcmp(code_marker, code_init_instance) == 0)
        insert_init_instance(mom, outfile);
    else if (strcmp(code_marker, code_init_trap) == 0)
        insert_init_trap(mom, outfile);
    else if (strcmp(code_marker, code_trap_export_defs) == 0)
        insert_trap_export_defs(mom, outfile);
    else if (strcmp(code_marker, code_trap_arg_inits) == 0)
        insert_trap_arg_inits(mom, outfile);
    else if (strcmp(code_marker, code_trap_arg_code) == 0)
        insert_trap_arg_code(mom, outfile);
    else if (strcmp(code_marker, code_trap_cond) == 0)
        insert_trap_cond(mom, outfile);
    else if (strcmp(code_marker, code_trap_polling) == 0)
        insert_trap_polling(mom, outfile);
    else
    {
	fprintf(stderr,"Error - code marker '%s' not recognized.\n", code_marker);
	return;
    }

    return SS$_NORMAL;
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	get_marker
**
**	This routine perform a text substitution for a "marker" in a
**	build file.  The marker specifies a piece of data, such as 
**	"MOM Author", and this routine returns the string to be
**	be substituted in its place in the output file.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**	marker		String containing the marker
**	translation	(output) the expansion of the marker.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Unrecognized marker
**
**--
*/
int get_marker( mom,
	        marker,
	        translation)

MOM_BUILD *mom;
char *marker;
char **translation;

{
    int	    status;
    
    if (mom->debug)
        fprintf(mom->log_file,"Translating %s",marker);

    if (strcmp(marker, string_author) == 0)
	*translation = mom->author;

    else if (strcmp(marker, string_oid_type) == 0)
	switch (mom->oid_type) 
	    {
	    case DNA_CMIP_OID :
		*translation = DNA_CMIP_OID_STR;
		break;
	    case OSI_CMIP_OID :
		*translation = OSI_CMIP_OID_STR;
		break;
	    case SNMP_OID :
		*translation = SNMP_OID_STR;
		break;                 
	    case OID :
		*translation = OID_STR;
		break;
	    default:
		*translation = " ";
	    }	

    else if (strcmp(marker, string_organization) == 0)
	*translation = mom->organization;

    else if (strcmp(marker, string_num_threads) == 0)
	{
	sprintf(temp_buffer, "%d", mom->num_threads);
	*translation = temp_buffer;
	}

    else if (strcmp(marker, string_system) == 0)
	*translation = mom->system;

    else if (strcmp(marker, string_copyright_date) == 0)
	*translation = mom->copyright_date;

    else if (strcmp(marker, string_copyright_owner) == 0)
	*translation = mom->copyright_owner;

    else if (strcmp(marker, string_creation_date) == 0)
	*translation = mom->creation_date;

    else if (strcmp(marker, string_mom_name) == 0)
	*translation = mom->mom_name;

    else if (strcmp(marker, string_facility) == 0)
	*translation = mom->facility;

    else if (strcmp(marker, string_include_file) == 0)
	*translation = mom->include_file;

    else if (strcmp(marker, string_include_file_inc) == 0)
	*translation = mom->include_file_inc;

    else if (strcmp(marker, string_msl_file) == 0)
	*translation = mom->msl_file;

    else if (strcmp(marker, string_extern_common) == 0)
        *translation = mom->extern_common;

    else if (strcmp(marker, string_class) == 0)
	*translation = mom->current_class->prefix;

    else if (strcmp(marker, string_class_name) == 0)
	*translation = mom->current_class->class_name;

    else if (strcmp(marker, string_class_name_ptr) == 0)
	*translation = mom->current_class->class_name_ptr;

    else if (strcmp(marker, string_class_void_name) == 0)
	*translation = mom->current_class->class_void_name;

    else if (strcmp(marker, string_action_directive) == 0)
	*translation = mom->current_class->current_action->action_name;

    else if (strcmp(marker, string_class_prefix) == 0)
	if (!mom->multiclass)
	    *translation = "\0";
	else
	{
	    sprintf(temp_buffer, "%s", mom->current_class->prefix);
	    *translation = temp_buffer;
	}
    else if (strcmp(marker, string_version) == 0)
	*translation = builder_version;
    else if (strcmp(marker, string_sp) == 0)
	*translation = mom->spaces;
    else if (strcmp(marker, string_mom_target) == 0)
	*translation = mom->mom_target;

    else
    {
	fprintf(stderr,"Unrecognized marker %s\n", marker);
	return SS$_BADPARAM;
    }

    if (mom->debug)
        fprintf(mom->log_file," into %s\n", *translation);

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	transfer_file
**
**	This routine transfers an input build file to an output result
**	file.  It searches for "markers" and "code markers" and performs
**	the requires text and code substitutions.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**	infile		File handle of the current input file
**	outfile		File handle of the current output file.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BARPARAM	Invalidly constructed marker found.
**
**--
*/
int transfer_file( mom,
		   infile,
		   outfile)

MOM_BUILD *mom;
FILE *infile;
FILE *outfile;

{
#define buffer_size 256
#define marker_size 64

    char    buffer[buffer_size];
    char    *buffer_pos;
    char    *insert_start;
    char    *insert_end;
    char    *marker_word;
    char    *translated_string;
    int	    marker_len;
    int	    status;

    while (NULL != fgets(buffer, buffer_size, infile))
    {
	buffer_pos = buffer;

	if (NULL != (insert_start = (char *)strstr(buffer_pos, code_insert_start)))
	    insert_code(mom, insert_start, outfile);
        else
        {
	    while (NULL != (insert_start = (char *)strstr(buffer_pos, mark_insert_start)))
	    {
	        insert_end = (char *)strstr(insert_start, mark_insert_end);
	        if (insert_end == NULL)
		    return SS$_BADPARAM;	    /* ?? */

	        marker_word = insert_start + mark_insert_start_len;
	        *insert_end = '\0';

	        status = get_marker(mom, marker_word, &translated_string);
	        if ERROR_CONDITION(status)
		    return status;

	        /*
	         * Write text before the marker.
	         */
	        *insert_start = '\0';
	        fputs(buffer_pos, outfile);

	        /*
	         * Write the marker
	         */
	        fputs(translated_string, outfile);

	        /*
	         * Advance past marker
	         */
	        buffer_pos = insert_end + mark_insert_end_len;
            }
	    fputs(buffer_pos, outfile);
        }
    }

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	move_file_to_output
**
**	This routine transfers an input build file to an output file
**	specified by name.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**	new_file	Name of the output file to create.
**	outfile		File handle of the current output file.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**
**--
*/
momgen_status move_file_to_output( mom,
				   new_file,
				   outfile)

MOM_BUILD *mom;
char *new_file;
FILE *outfile;

{
  char    *in_filename;
  FILE    *infile;
  int	    status;
  
  in_filename = (char *)malloc( strlen( mom->mom_in ) + strlen( new_file ) + 1);
  strcpy(in_filename, mom->mom_in );
  strcat(in_filename, new_file );

  infile = fopen(in_filename, "r" );
  if (infile == NULL) {
    fprintf(stderr, "Can't read %s\n", in_filename);
    return MOMGEN_C_OPENREADERROR;
  }
  
  status = transfer_file(mom, infile, outfile);
  
  fclose(infile);
  
  return status;
}    

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	create_output_file
**
**	This routine transfers an input build file to an output
**	result file, both of which are specified by name.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**	input_filename	Name of the input file
**	output_filename Name of the output file
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**
**--
*/
momgen_status create_output_file( mom,
				  input_filename,
				  output_filename)

MOM_BUILD *mom;
char *input_filename;
char *output_filename;

{
    char *out_filename;
    FILE    *outfile;
    momgen_status status;

    mom->input_filename = input_filename;
    out_filename = (char *)malloc( strlen( mom->mom_out_src ) + strlen( output_filename ) + 1);
    strcpy(out_filename, mom->mom_out_src );
    strcat(out_filename, output_filename );

    outfile = fopen(out_filename, "w" );
    if (outfile == NULL) 
      {
      fprintf(stderr, "Can't write to %s\n", out_filename);
      return MOMGEN_C_OPENWRITEERROR;
      }

    status = move_file_to_output(mom, input_filename, outfile);
    if (status == MOMGEN_C_OPENREADERROR)
      return status;

    fclose(outfile);

    return status;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	create_output_file_build
**
**	This routine transfers an input build file to an output
**	result file, both of which are specified by name. THis
**	routine prefixes the name with the MOM_OUT_BUILD logical.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**	input_filename	Name of the input file
**	output_filename Name of the output file
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**
**--
*/
momgen_status create_output_file_build( mom,
				  	input_filename,
				  	output_filename)

MOM_BUILD *mom;
char *input_filename;
char *output_filename;

{
    char  *out_filename;
    FILE    *outfile;
    momgen_status status;

    out_filename = (char *)malloc( strlen( mom->mom_out_build ) + strlen( output_filename ) + 1);
    strcpy(out_filename, mom->mom_out_build );
    strcat(out_filename, output_filename );

    outfile = fopen(out_filename, "w" );
    if (outfile == NULL) 
      {
      fprintf(stderr, "Can't write to %s\n", out_filename);
      return MOMGEN_C_OPENWRITEERROR;
      }

    status = move_file_to_output(mom, input_filename, outfile);
    if (status == MOMGEN_C_OPENREADERROR)
      return status;

    fclose(outfile);

    return status;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	create_output_per_class
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file.
**
**	This routine transfers an input build file to a set of
**	output result files, one per supported class.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**	input_filename	Name of the input file
**	output_filename	Base name of the output file
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**
**--
*/
momgen_status create_output_per_class( mom,
				       input_filename,
				       output_filename)

MOM_BUILD *mom;
char *input_filename;
char *output_filename;

{
    CLASS_DEF	*class;
    int		class_num;
    char	outfile_name[256];
    char	blank_spaces[256];
    momgen_status status;
    int		tmp;

    class = mom->class;

    for (class_num=0; class_num < mom->num_classes; class_num++)
    {
	mom->current_class = class;
	for (tmp=0; tmp <= strlen(mom->current_class->prefix); tmp++)
	    blank_spaces[tmp] = ' ';
        blank_spaces[tmp] = '\0';
	mom->spaces = blank_spaces;

	sprintf(outfile_name, output_filename, mom->current_class->prefix);

	status = create_output_file(mom, input_filename, outfile_name);
	if (status != MOMGEN_C_SUCCESS)
	  return status;

	class = class->next;
    }

    mom->current_class = mom->class;

    return status;
}
