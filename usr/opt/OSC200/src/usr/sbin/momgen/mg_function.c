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
static char *rcsid = "@(#)$RCSfile: mg_function.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 22:11:12 $";
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
**  FACILITY:  VMS Framework Services VMS MOM Generator
**
**  MODULE DESCRIPTION:
**
**	This module contains the routines that determine which functions to
**	build based on the /FUNCTIONS qualifier.
**
**  AUTHORS:
**
**      Rich Bouchard, Jr.
**
**  CREATION DATE: 
**
**  MODIFICATION HISTORY:
**
**	F-16		Mike Densmore		    6-Jul-1992
**	added code for building getnext modules
**
**	F-15		Marc Nozell 	    	    15-Jun-1992
**  	Remove references to module.
**
**	F-4		    Mike Densmore	    10-Jun-1992
**
**	added support for snmp-specific build_perform_get
**
**	F-3		    Mike Densmore	    21-May-1992
**
**	Modified to support U**X platforms.
**
**	F-2		    Marc Nozell		    24-May-1991
**                            
**	Support multiple classes.
**
**	25-Sep-1992 Mike Densmore   Modified for ANSI Standard C
**--
*/

#ifdef VMS
#include "vaxcshr.h"		/* translate from VAX C to DEC C RTL names */
#endif 
#include "mom_defs.h"
#include "file_defs.h"
#include "mg_prototypes.h"


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_instance_avl
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-instance-avl tag.
**
**	This includes the instance avl code based on whether class instances
**	are supported.  This is determined in get_attribute_part based on 
**	whether a name identifier attribute is supplied.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**	outfile		File handle of the current output file.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**
**--
*/
int insert_instance_avl( mom,
		         outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    momgen_status status;

    mom->temp = "support_instances";

    if (mom->current_class->support_instances)
        status = move_file_to_output(mom, file_instance_input, outfile);
    else
	status = move_file_to_output(mom, file_instance_null_input, outfile);
      
    return status;
}    

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_get
**
**	This routine creates the "GET" module.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_get( mom )

MOM_BUILD *mom;

{
    momgen_status status;

    mom->temp = "get";

    if (mom->support_get)
        status = create_output_file(mom, file_get_input, file_get_output);
    else
	status = create_output_file(mom, file_get_null_input, file_get_output);
      
    return status;
}    

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_getnext
**
**	This routine creates the "GETNEXT" module.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_getnext( mom )

MOM_BUILD *mom;

{
    momgen_status status;

    mom->temp = "getnext";

    if (mom->support_getnext)
        status = create_output_file(mom, file_getnext_input, file_getnext_output);
    else
	status = create_output_file(mom, file_getnext_null_input, file_getnext_output);
      
    return status;
}    

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_set
**
**	This routine creates the "SET" module.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_set( mom )

MOM_BUILD *mom;

{
    momgen_status status;

    mom->temp = "set";

    if (mom->support_set)
        status = create_output_file(mom, file_set_input, file_set_output);
    else
	status = create_output_file(mom, file_set_null_input, file_set_output);

    return status;
}    

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_delete
**
**	This routine creates the "DELETE" module.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_delete( mom )

MOM_BUILD *mom;

{
    momgen_status status;

    mom->temp = "delete";

    if (mom->support_delete)
        status = create_output_file(mom, file_delete_input, file_delete_output);
    else
	status = create_output_file(mom, file_delete_null_input, file_delete_output);

    return status;
}    

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_find
**
**	This routine creates the "FIND" module (containing user changes).
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_find( mom )

MOM_BUILD *mom;

{
    momgen_status status;

    mom->temp = "find";

    if (!mom->multiclass)
        status = create_output_file(mom, file_find_input, file_find_output);
    else
        status = create_output_per_class(mom, file_find_input, file_multi_find_output);

    return status;
}    

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_create
**
**	This routine creates the "CREATE" module.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_create( mom )

MOM_BUILD *mom;

{
    momgen_status status;

    mom->temp = "create";

    if (mom->support_create)
        status = create_output_file(mom, file_create_input, file_create_output);
    else
	status = create_output_file(mom, file_create_null_input, file_create_output);

    return status;
}    

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_action
**
**	This routine creates the "ACTION" module.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_action( mom )

MOM_BUILD *mom;

{                                          
    momgen_status status;

    if (mom->support_action)
        status = create_output_file(mom, file_action_input, file_action_output);
    else
	status = create_output_file(mom, file_action_null_input, file_action_output);

    return status;
}    

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_access
**
**	This routine creates the "ACCESS" module.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_access( mom )

MOM_BUILD *mom;

{
    momgen_status status;

    status = create_output_file(mom, file_access_input, file_access_output);

    return status;
}    

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_directive
**
**	This routine creates the "DIRECTIVE" module.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_directive( mom )

MOM_BUILD *mom;

{
    momgen_status status;

    status = create_output_file(mom, file_directive_input, file_directive_output);

    return status;
}    

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_init
**
**	This routine creates the "INIT" module.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_init( mom )

MOM_BUILD *mom;

{
    momgen_status    status;

    status = create_output_file(mom, file_init_input, file_init_output);

    return status;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_trap
**
**	This routine creates the "TRAP" module.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_trap( mom )

MOM_BUILD *mom;

{
    momgen_status    status;

    status = create_output_file(mom, file_trap_input, file_trap_output);

    return status;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_makefile
**
**	This routine creates the Makefile to build the MOM for Ultrix.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_makefile( mom )

MOM_BUILD *mom;

{
    momgen_status status;

    status = create_output_file(mom, file_makefile_input,file_makefile_output);

    if (ERROR_CONDITION(status))
	return status;

    return status;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_descrip
**
**	This routine creates the MMS description file to
**	build the MOM for VMS.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_descrip( mom )

MOM_BUILD *mom;

{
    char	outfile_name[256];
    momgen_status status;

    if (!mom->multiclass)
        status = create_output_file_build(mom, file_descrip_input, file_descrip_output);
    else
	status = create_output_file_build(mom, file_multi_descrip_input, file_descrip_output);

    if (ERROR_CONDITION(status))
	return status;

    return status;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_util
**
**	This routine creates the utility routines module.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_util( mom )

MOM_BUILD *mom;

{
    char	outfile_name[256];
    momgen_status status;

    status = create_output_file(mom, file_util_input, file_util_output);

    return status;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_get_candidate
**
**	This routine creates the "GET_CANDIDATE" modules for
**	each class.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_get_candidate( mom )

MOM_BUILD *mom;

{
    momgen_status status;

    status = create_output_file(mom, file_switch_get_candidate_input, file_get_candidate_output);
    if (!ERROR_CONDITION(status))
        status = create_output_per_class(mom, file_get_candidate_inst_input, file_class_get_candidate_output);

    return status;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_perform_get
**
**	This routine creates the "PERFORM_GET" module for
**	each class.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_perform_get( mom )

MOM_BUILD *mom;

{
    momgen_status status;

    if (mom->oid_type != SNMP_OID) {
	if (!mom->multiclass)
	    status = create_output_file(mom, file_perform_get_input, file_perform_get_output);
	else
	    status = create_output_per_class(mom, file_perform_get_input, file_class_perform_get_output);
    }
    else {
	if (!mom->multiclass)
	    status = create_output_file(mom, file_perform_snmp_get_input, file_perform_get_output);
	else
	    status = create_output_per_class(mom, file_perform_snmp_get_input, file_class_perform_get_output);
    }

    return status;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_perform_getnext
**
**	This routine creates the "PERFORM_GETNEXT" module for
**	each class.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_perform_getnext( mom )

MOM_BUILD *mom;

{
    momgen_status status;

    /* step thru all the classes and build the appropriate perform_getnext based upon
       whether or not they are table (have identifier) or non-table (no identifier)
       type objects */

CLASS_DEF   *class;
int	    class_num;
char        tempfile_name[256];

    class = mom->class;
    for (class_num = 0; class_num < mom->num_classes; class_num++) {

	mom->current_class = class;
	sprintf(tempfile_name, file_perform_getnext_output, mom->current_class->prefix);

	if (class->support_instances)
	    status = create_output_file( mom, file_perform_table_getnext_input, tempfile_name );
	else
	    status = create_output_file( mom, file_perform_nontable_getnext_input, tempfile_name );
	if (ERROR_CONDITION(status)) return status;
	class = class->next;
    }
    return status;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_perform_create
**
**	This routine creates the "PERFORM_CREATE" module for
**	each class.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_perform_create( mom )

MOM_BUILD *mom;

{
    momgen_status status;

    if (!mom->multiclass)
        if (mom->support_instances)
            status = create_output_file(mom, file_perform_create_inst_input, file_perform_create_output);
	else
            status = create_output_file(mom, file_perform_create_input, file_perform_create_output);
    else
        if (mom->support_instances)
	    status = create_output_per_class(mom, file_perform_create_inst_input, file_class_perform_create_output);
	else
	    status = create_output_per_class(mom, file_perform_create_input, file_class_perform_create_output);

    return status;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_perform_delete
**
**	This routine creates the "PERFORM_DELETE" module for
**	each class.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_perform_delete( mom )

MOM_BUILD *mom;

{
    momgen_status status;

    if (!mom->multiclass)
        status = create_output_file(mom, file_perform_delete_input, file_perform_delete_output);
    else
	status = create_output_per_class(mom, file_perform_delete_input, file_class_perform_delete_output);

    return status;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_perform_set
**
**	This routine creates the "PERFORM_SET" module for
**	each class.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_perform_set( mom )

MOM_BUILD *mom;

{
    momgen_status status;

    if (!mom->multiclass)
        status = create_output_file(mom, file_perform_set_input, file_perform_set_output);
    else
	status = create_output_per_class(mom, file_perform_set_input, file_class_perform_set_output);
 
    return status;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_perform_init
**
**	This routine creates the "PERFORM_INIT" module for
**	each class.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_perform_init( mom )

MOM_BUILD *mom;

{
    momgen_status status;
    
    if (!mom->multiclass)
        if (mom->support_instances)
            status = create_output_file(mom, file_perform_init_inst_input, file_perform_init_output);
	else
            status = create_output_file(mom, file_perform_init_input, file_perform_init_output);
    else
        if (mom->support_instances)
	    status = create_output_per_class(mom, file_perform_init_inst_input, file_class_perform_init_output);
	else
	    status = create_output_per_class(mom, file_perform_init_input, file_class_perform_init_output);

    return status;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_common
**
**	This routine creates the common include file and the extern include file.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_common ( mom )

MOM_BUILD *mom;

{
    momgen_status status;

    status = create_output_file(mom, file_common_input, file_common_output);

    if (mom->support_instances)
        status = create_output_file(mom, file_extern_common_input, file_extern_common_output);

    return status;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_command_file
**
**	This routine creates the command file used to compile and link the MOM.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_command_file( mom )

MOM_BUILD *mom;

{
    momgen_status status;

    if (!mom->multiclass)
        status = create_output_file_build(mom, file_command_input, file_command_output);
    else
        status = create_output_file_build(mom, file_command_mp_input, file_command_output);
    return status;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_cancel_get
**
**	This routine creates the cancel get entry point which is currently
**	unsupported.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_cancel_get( mom )

MOM_BUILD *mom;

{
    momgen_status status;

    mom->temp = "cancel_get";

    status = create_output_file(mom, file_cancel_get_null_input, file_cancel_get_output);
      
    return status;
}    

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_oids
**
**	This routine creates the OID files (import_oids.h and export_oids.h).
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_oids( mom )

MOM_BUILD *mom;

{
    momgen_status status;

    mom->temp = "build_oids";

    status = create_output_file(mom, file_oid_export_input, file_oid_export_output);

    status = create_output_file(mom, file_oid_import_input, file_oid_import_output);
      
    return status;
}    

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_protos
**
**	This routine creates the MOM_PROTOTYPES.H include file.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_protos( mom )

MOM_BUILD *mom;

{
    momgen_status status;

    mom->temp = "build_protos";

    status = create_output_file(mom, file_prototypes_input, file_prototypes_output); 

    return status;
}    

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	build_mom
**
**	This routine is the main entry point for the MOM Generator.
**	It uses the user-supplied data to determine which output
**	file should be created, and calls the appropriate "build_"
**	routines above.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	SS$_BADPARAM	Input file had invalid data.
**--
*/
momgen_status build_mom( mom )

MOM_BUILD *mom;

{
    momgen_status status;

    mom->current_class = mom->class;

    if ((mom->build_get) || (mom->build_all))
    {
    if (mom->log)    
      fprintf(mom->log_file,"Building GET...\t\t\t\t\t");

	status = build_get(mom);
	if (ERROR_CONDITION(status))
	    return status;
    if (mom->log)
      fprintf(mom->log_file,"Done!\n");
    }
/*
    if (mom->build_cancel_get)
    {
    if (mom->log)    
      fprintf(mom->log_file,"Building CANCEL_GET...\t\t\t\t");

	status = build_cancel_get(mom);
	if (ERROR_CONDITION(status))
	    return status;
    if (mom->log)
      fprintf(mom->log_file,"Done!\n");
    }
*/
    if ((mom->build_set) || (mom->build_all))
    {
    if (mom->log)    
      fprintf(mom->log_file,"Building SET...\t\t\t\t\t");

	status = build_set(mom);
	if (ERROR_CONDITION(status))
	    return status;
    if (mom->log)
      fprintf(mom->log_file,"Done!\n");
    }

    if ((mom->build_delete) || (mom->build_all))
    {
    if (mom->log)    
      fprintf(mom->log_file,"Building DELETE...\t\t\t\t");

	status = build_delete(mom);
	if (ERROR_CONDITION(status))
	    return status;
    if (mom->log)
      fprintf(mom->log_file,"Done!\n");
    }

    if ((mom->build_delete || mom->build_all) && mom->support_delete) 
    {
    if (mom->log)    
      fprintf(mom->log_file,"Building PERFORM_DELETE...\t\t\t");

	status = build_perform_delete(mom);
	if (ERROR_CONDITION(status))
	    return status;
    if (mom->log)
      fprintf(mom->log_file,"Done!\n");
    }

    if ((mom->build_action) || (mom->build_all))
    {
    if (mom->log)    
      fprintf(mom->log_file,"Building ACTION...\t\t\t\t");

	status = build_action(mom);
	if (ERROR_CONDITION(status))
	    return status;
    if (mom->log)
      fprintf(mom->log_file,"Done!\n");
    }

    if ((mom->build_create) || (mom->build_all))
    {
    if (mom->log)    
      fprintf(mom->log_file,"Building CREATE...\t\t\t\t");

	status = build_create(mom);
	if (ERROR_CONDITION(status))
	    return status;
    if (mom->log)
      fprintf(mom->log_file,"Done!\n");
    }

    if ((mom->build_create || mom->build_all) && mom->support_create )
    {
    if (mom->log)    
      fprintf(mom->log_file,"Building PERFORM_CREATE...\t\t\t");

	status = build_perform_create(mom);
	if (ERROR_CONDITION(status))
	    return status;
    if (mom->log)
      fprintf(mom->log_file,"Done!\n");
    }

    if ((mom->build_access) || (mom->build_all))
    {
    if (mom->log)    
      fprintf(mom->log_file,"Building ACCESS...\t\t\t\t");

	status = build_access(mom);
	if (ERROR_CONDITION(status))
	    return status;
    if (mom->log)
      fprintf(mom->log_file,"Done!\n");
    }

    if ((mom->build_directive) || (mom->build_all))
    {
    if (mom->log)    
      fprintf(mom->log_file,"Building DIRECTIVE...\t\t\t\t");

	status = build_directive(mom);
	if (ERROR_CONDITION(status))
	    return status;
    if (mom->log)
      fprintf(mom->log_file,"Done!\n");
    }

    if ((mom->build_init) || (mom->build_all))
    {
    if (mom->log)    
      fprintf(mom->log_file,"Building INIT...\t\t\t\t");

	status = build_init(mom);
	if (ERROR_CONDITION(status))
	    return status;
    if (mom->log)
      fprintf(mom->log_file,"Done!\n");
    }

#ifdef UNIXPLATFORM
    if (1)
    {
    if (mom->log)    
      fprintf(mom->log_file,"Building TRAP...\t\t\t\t");

	status = build_trap(mom);
	if (ERROR_CONDITION(status))
	    return status;
    if (mom->log)
      fprintf(mom->log_file,"Done!\n");
    }
#endif

    if ((mom->build_oids) || (mom->build_all))
    {
    if (mom->log)    
      fprintf(mom->log_file,"Building OIDs...\t\t\t\t");

	status = build_oids(mom);
	if (ERROR_CONDITION(status))
	    return status;
    if (mom->log)
      fprintf(mom->log_file,"Done!\n");
    }

    if ((mom->build_get) || (mom->build_all))
    {
    if (mom->log)    
      fprintf(mom->log_file,"Building GET_INSTANCE....\t\t\t");

	status = build_get_candidate(mom);
	if (ERROR_CONDITION(status))
	    return status;
    if (mom->log)
      fprintf(mom->log_file,"Done!\n");
    }

    if ((mom->build_get || mom->build_all) && mom->support_get) 
    {
    if (mom->log)    
      fprintf(mom->log_file,"Building PERFORM_GET...\t\t\t\t");

	status = build_perform_get(mom);
	if (ERROR_CONDITION(status))
	    return status;
    if (mom->log)
      fprintf(mom->log_file,"Done!\n");
    }

    if ((mom->build_set || mom->build_all) && mom->support_set) 
    {
    if (mom->log)    
      fprintf(mom->log_file,"Building PERFORM_SET...\t\t\t\t");

	status = build_perform_set(mom);
	if (ERROR_CONDITION(status))
	    return status;
    if (mom->log)
      fprintf(mom->log_file,"Done!\n");
    }

    if ((mom->build_init) || (mom->build_all))
    {
    if (mom->log)    
      fprintf(mom->log_file,"Building PERFORM_INIT...\t\t\t");

	status = build_perform_init(mom);
	if (ERROR_CONDITION(status))
	    return status;
    if (mom->log)
      fprintf(mom->log_file,"Done!\n");
    }

    if (mom->build_all || mom->build_find)
    {
    if (mom->log)    
      fprintf(mom->log_file,"Building FIND...\t\t\t\t");

	status = build_find(mom);
	if (ERROR_CONDITION(status))
	    return status;
    if (mom->log)
      fprintf(mom->log_file,"Done!\n");
    }

    if ((mom->build_descrip) || (mom->build_all))
    {
    if (mom->log)    
      fprintf(mom->log_file,"Building DESCRIP...\t\t\t\t");

	status = build_descrip(mom);
	if (ERROR_CONDITION(status))
	    return status;
    if (mom->log)
      fprintf(mom->log_file,"Done!\n");
    }

#ifdef UNIXPLATFORM
    if ((mom->build_makefile) || (mom->build_all))
    {
    if (mom->log)    
      fprintf(mom->log_file,"Building MAKEFILE...\t\t\t\t");

	status = build_makefile(mom);
	if (ERROR_CONDITION(status))
	    return status;
    if (mom->log)
      fprintf(mom->log_file,"Done!\n");
    }
#endif /* UNIXPLATFORM */

    if ((mom->build_command_file) || (mom->build_all))
    {
    if (mom->log)    
      fprintf(mom->log_file,"Building COMMAND_FILE...\t\t\t");

	status = build_command_file(mom);
	if (ERROR_CONDITION(status))
	    return status;
    if (mom->log)
      fprintf(mom->log_file,"Done!\n");
    }

    if ((mom->build_util) || (mom->build_all))
    {
    if (mom->log)    
      fprintf(mom->log_file,"Building UTILITY_ROUTINES...\t\t\t");

	status = build_util(mom);
	if (ERROR_CONDITION(status))
	    return status;
    if (mom->log)
      fprintf(mom->log_file,"Done!\n");
    }

    if ((mom->build_prototypes) || (mom->build_all))
    {
    if (mom->log)    
      fprintf(mom->log_file,"Building MOM_PROTOTYPES.H...\t\t\t");
             
	status = build_protos(mom);
	if (ERROR_CONDITION(status))
	    return status;
    if (mom->log)
      fprintf(mom->log_file,"Done!\n");
    }

    if ((mom->build_common) || (mom->build_all))
    {
    if (mom->log)    
      fprintf(mom->log_file,"Building COMMON.H...\t\t\t\t");
             
	status = build_common(mom);
	if (ERROR_CONDITION(status))
	    return status;
    if (mom->log)
      fprintf(mom->log_file,"Done!\n");
    }

    if ((mom->build_getnext) || (mom->build_all))
    {
	if (mom->log)
	    fprintf(mom->log_file,"Building GETNEXT.C...\t\t\t\t");
	status = build_getnext(mom);
	if (ERROR_CONDITION(status))
	    return status;
	if (mom->log)
	    fprintf(mom->log_file,"Done!\n");
    }

    if ((mom->build_getnext || mom->build_all) && mom->support_getnext)
    {
	if (mom->log)
	    fprintf(mom->log_file,"Building PERFORM_GETNEXT...\t\t\t\t");
	status = build_perform_getnext(mom);
	if (ERROR_CONDITION(status))
	    return status;
	if (mom->log)
	    fprintf(mom->log_file,"Done!\n");
    }

    return SS$_NORMAL;
}
