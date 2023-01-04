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
static char *rcsid = "@(#)$RCSfile: mg_action.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/06/15 11:54:46 $";
#endif
/*
**+
**  Copyright (c) Digital Equipment Corporation, 1991
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United Statres.
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
**	This module contains the routines to build action code.  Generated
**	files include action.c and [[class]]_perform_[[action]].c.
**
**  AUTHOR:
**
**      Gary Allison
**
**  CREATION DATE:  September 23, 1991
**
**  MODIFICATION HISTORY:
**
**	Marc Nozell	15-Jun-1992	Improve portability
**	    - removed references to module. 
**
**	Mike Densmore	20-May-1992	Modified for U**X platforms
**	    - defined "module"
**	    - fixed array pointer warning in sprintf call
**
**
**	Mike Densmore	25-Sep-1992	modified for ANSI Standard C
**
**	M. Ashraf	 3-Mar-1993	updated for IPC changes
**
**--        
*/

#ifdef VMS
#include "vaxcshr.h"		/* translate from VAX C to DEC C RTL names */
#endif 
#ifndef UNIXPLATFORM
#include <ssdef>
#endif

#include "mom_defs.h"
#include <string.h>
#include "file_defs.h"
#include "mg_prototypes.h"

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_perform_action
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-perform-action tag.
**
**	This routine generates a complete routine prefix_perform_action 
**	that compares action OIDs and calls the appropriate directive
**	perform_action routine.
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
int insert_perform_action( mom,
		           outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    ACTION_DEF      *action;
    int		    action_num;
    DIRECTIVE_DEF   *directive;
    int		    one=0;
    CLASS_DEF	    *class;
    int		    class_num;
    char	    buffer[256];
    char	    outfile_name[256];
    int		    status;

 class = mom->class;
 for (class_num=0; class_num < mom->num_classes; class_num++)
    {
    fputs("/*\n",outfile);
    fputs("**++\n",outfile);
    fputs("**  FUNCTIONAL DESCRIPTION:\n",outfile);
    fputs("**\n",outfile);
    sprintf(buffer,"**      %sperform_action\n",class->prefix_m);
    fputs(buffer,outfile);
    fputs("**\n",outfile);
    fputs("**	This routine calls the appropriate action directive routine.\n",outfile);
    fputs("**\n",outfile);
    fputs("**  FORMAL PARAMETERS:\n",outfile);
    fputs("**\n",outfile);
    fputs("**	mom_handle        - Man binding handle.\n",outfile);
    fputs("**	object_class      - OID of class to create.\n",outfile);
    fputs("**	object_instance   - Instance name to create \n",outfile);
    fputs("**	iso_scope         - Scoping value. Not currently used.\n",outfile);
    fputs("**	filter            - AVL determining which instance(s) the operation should be\n",outfile);
    fputs("**	    		    performed on.\n",outfile);
    fputs("**      access_control    - Access control information.\n",outfile);
    fputs("**	synchronization   - Not currently used.\n",outfile);
    fputs("**	action_type       - OID of operation to be performed.	\n",outfile);
    fputs("**	action_information- AVL containing class-specific arguments.\n",outfile);
    fputs("**	invoke_id         - Invocation ID.\n",outfile);
    fputs("**	return_routine    - Handle for reply processing.\n",outfile);
    fputs("**\n",outfile);
    fputs("**  RETURN VALUES:       \n",outfile);
    fputs("** \n",outfile);
    fputs("**      MAN_C_SUCCESS\n",outfile);
    fputs("**	MAN_C_INSUFFICIENT_RESOURCES\n",outfile);
    fputs("**	MAN_C_PE_TIMEOUT\n",outfile);
    fputs("**\n",outfile);
    fputs("**  SIDE EFFECTS:\n",outfile);
    fputs("**\n",outfile);
    fputs("**      None\n",outfile);
    fputs("**\n",outfile);
    fputs("**--\n",outfile);
    fputs("*/\n",outfile);
    sprintf(buffer,"static man_status %sperform_action(   man_binding_handle  mom_handle,\n", class->prefix_m);
    fputs(buffer,outfile);
    fputs("				object_id	    *object_class, \n",outfile);
    fputs("	    			avl		    *object_instance,\n",outfile);
    fputs("				scope		    iso_scope,\n",outfile);
    fputs("				avl		    *filter,\n",outfile);
    fputs("				avl		    *access_control,\n",outfile);
    fputs("				int		    synchronization,\n",outfile);
    fputs("				object_id	    *action_type,\n",outfile);
    fputs("				avl		    *action_information,\n",outfile);
    fputs("				int		    invoke_id,\n",outfile);
    fputs("				management_handle   *return_routine)\n",outfile);
    fputs("{\n",outfile);
    fputs("    man_status		status;\n",outfile);
    fputs("    int			class_code;\n",outfile);
    fputs("    unsigned int		action_num;\n",outfile);
    fputs("    man_status 		(*perform_routine)();\n\n",outfile);
    
    mom->current_class = class;

	one = 0;
    action = class->action;
    for (action_num=0; action_num < class->num_actions; action_num++)
    {
        class->current_action = action;
	directive = action->directive;

        if (action_num && ((directive->dna_cmip_int_len) || (directive->oid_len) ||
	    (directive->osi_oid_len) || (directive->snmp_oid_len)))
            	fputs(     "    else",outfile);      

        if (directive->dna_cmip_int_len)
	    {
	    sprintf(buffer,"  if (((status = moss_compare_oid( action_type, &%s%s_DNA)) == MAN_C_EQUAL)",
			class->prefix_m, directive->directive_name);
   	    fputs(buffer, outfile);
	    one = 1;
	    }

	if (directive->oid_len)
	    {
	    if (one)
		fputs("   ||\n ", outfile);
	    else 
		fputs("  if",outfile);

	    sprintf(buffer," ((status = moss_compare_oid( action_type, &%s%s_OID)) == MAN_C_EQUAL)",
			class->prefix_m, directive->directive_name);
   	    fputs(buffer, outfile);
	    one = 1;
            }

	if (directive->snmp_oid_len)
	    {	
	    if (one)
		fputs("   ||\n ", outfile);
	    else 
		fputs("  if",outfile);

	    sprintf(buffer," ((status = moss_compare_oid( action_type, &%s%s_SNP)) == MAN_C_EQUAL)",
			class->prefix_m, directive->directive_name);
   	    fputs(buffer, outfile);
	    one = 1;
	    }

	if (directive->osi_oid_len)
	    {	
	    if (one)
		fputs("   ||\n ", outfile);
	    else 
		fputs("  if",outfile);

	    sprintf(buffer," ((status = moss_compare_oid( action_type, &%s%s_OSI)) == MAN_C_EQUAL)",
			class->prefix_m, directive->directive_name);
   	    fputs(buffer, outfile);

	    one = 1;
	    }

	if (one)
	    {
	    fputs(")\n ", outfile);
	    
	    if (strcmp(action->action_name,"CREATE") == 0) 
		{
                fputs("	    status = create_instance(mom_handle,\n",outfile);
                fputs("				     object_class,\n",outfile);
                fputs("				     object_instance,\n",outfile);
                fputs("				     NULL,			\n",outfile);
                fputs("				     access_control,\n",outfile);
                fputs("				     NULL,		        \n",outfile);
                fputs("				     action_information,	\n",outfile);
                fputs("				     invoke_id,\n",outfile);
                fputs("		    		     return_routine,\n",outfile);
                fputs("				     action_type,\n",outfile);
                fputs("				     ACTION);\n\n",outfile);
	        }
	    else if (strcmp(action->action_name,"DELETE") == 0)
		{
                fputs("	    status = delete_instance(mom_handle,\n",outfile);
                fputs("				     object_class,\n",outfile);
                fputs("				     object_instance,\n",outfile);
                fputs("				     iso_scope,\n",outfile);
                fputs("				     filter,\n",outfile);
                fputs("				     access_control,\n",outfile);
                fputs("			     	     synchronization,\n",outfile);
                fputs("				     invoke_id,\n",outfile);
                fputs("				     return_routine,\n",outfile);
                fputs("				     action_type,\n",outfile);
                fputs("				     action_information,\n",outfile);
                fputs("				     ACTION);\n\n",outfile);
		}
	    else 
		{
                fputs("	    status =  moi_directive(mom_handle,\n",outfile);
                fputs("                             object_class,\n",outfile);
                fputs("                             object_instance,\n",outfile);
                fputs("                             iso_scope,		\n",outfile);
                fputs("                             filter,\n",outfile);
                fputs("                             access_control,\n",outfile);
                fputs("                             NULL,\n",outfile);
                fputs("                             action_information,  \n",outfile);
                sprintf(buffer,"                             %sperform_%s,\n",class->prefix_m,action->action_name);
		fputs(buffer,outfile);
                fputs("                             moss_send_action_reply,\n",outfile);
                fputs("                             invoke_id,\n",outfile);
                fputs("                             return_routine,\n",outfile);
                fputs("                             action_type);\n\n",outfile);

		/*
		 * Create the action output file 
	    	 */
		sprintf(outfile_name, file_class_perform_action, class->prefix_m, action->action_name);
                status = create_output_file( mom, file_perform_action, outfile_name );

		}

	    one = 0;

	    }

	action = action->next;
    }

 if (action_num == 0)
    {
    fputs("	status = send_error_reply( return_routine,\n",outfile);
    fputs("			    	 moss_send_action_reply,\n",outfile);
    fputs("			    	 invoke_id,\n",outfile);
    fputs("			    	 MAN_C_NO_SUCH_ACTION,\n",outfile);
    fputs("			    	 object_class,\n",outfile);
    fputs("			    	 object_instance, \n",outfile);
    fputs("			    	 NULL, \n",outfile);
    fputs("				 NULL, \n",outfile);
    fputs("				 action_type);\n",outfile);
    fputs("\n",outfile);
    }

 fputs("    return status;\n}\n\f\n",outfile);

 class = class->next;
 }

return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_action_switch
**
**	This routine generates code for action switch statement in action.c
**	for each action directive other than create or delete. Code is 
**	generated set action_routine to the [[class]]_perform_[[action]].
**
**	This routine also creates the [[class]]_perform_[[action]] routine by
**	setting up a temporary MOM which has the current class and action.
**
**	This routine is called for the insert-code-action-switch tag. 
**	Note that the tag is actually found in the template file 
**	build_action_action.c.
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
int insert_action_switch( mom,
			  outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    CLASS_DEF   *class;
    int		class_num;
    char	buffer[256];

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
        {    
	sprintf(buffer,"            case %s%s_ID:\n", class->prefix_k, class->class_name );
	fputs(buffer, outfile);
	sprintf(buffer,"                perform_routine = %sperform_action;\n", class->prefix);
	fputs(buffer, outfile);
	fputs(         "                break;\n\n",outfile);
	class = class->next;
	}	
  return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_action_exception
**
**	This routine generates the code for an action exception in 
**	the perform_action routines.  If none exists, a comment is produced.
**
**	This routine is called for tag insert-code-action-exception.
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
int insert_action_exception( mom,
			     outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    int 	found=0;
    DIRECTIVE_DEF *dir;
    EXCEPTION_DEF *exc;
    int		exception_num;
    CLASS_DEF   *class;
    int		class_num;
    char	buffer[256];
    int		action_num,actions=0;
    int 	exception_found = 0;
    ACTION_DEF	*action;
    momgen_status status;

    /* First, find the exceptions */

    class = mom->current_class;
    dir = class->directive;
    action = class->current_action;
    if (action == NULL)
        return 1;


    while (!found && (dir != NULL))
	if (action->action_number == dir->directive_number)
	    found = 1;
	else
           dir = dir->next;
	    
    if (!found || (dir->num_exceptions == 0))
	{
        fputs("/** No exceptions found for this action **/\n",outfile);
        return 1;
	}

    /* At this point, we have some exceptions */

    /* 
     * Print out the exceptions looking for the success OID. If not found 
     * print a message saying success OID not there...
     */

    exc = dir->exc;
    for (exception_num=0; exception_num < dir->num_exceptions; exception_num++)
	{
	/* Look for "SUCCESS" string, "SUCC" should be sufficient. */

        if (strstr(exc->exception_name,"SUCC") == NULL)
	    {
	    if (exception_found)
		fputs("/** Multiple exception OIDs found **/\n",outfile);
	
            if (exc->dna_cmip_int_len)
		{
		fputs("#ifdef DNA_CMIP_OID\n",outfile);
                sprintf(buffer,"    *action_response_type_oid = &%s%s_DNA;\n", 
			class->prefix_m,exc->exception_name);
            	fputs(buffer, outfile);	    
		fputs("#endif /* DNA_CMIP_INT */\n",outfile);		
		}
            if (exc->osi_oid_len)
		{
		fputs("#ifdef OSI_CMIP_INT\n",outfile);
                sprintf(buffer,"    *action_response_type_oid = &%s%s_OSI;\n", 
			class->prefix_m,exc->exception_name);
            	fputs(buffer, outfile);	    
		fputs("#endif /* OSI_CMIP_INT */\n",outfile);		
		}
            if (exc->snmp_oid_len)
		{
		fputs("#ifdef SNMP_OID\n",outfile);
                sprintf(buffer,"    *action_response_type_oid = &%s%s_SNP;\n", 
			class->prefix_m,exc->exception_name);
            	fputs(buffer, outfile);	    
		fputs("#endif /* SNMP_OID */\n",outfile);		
		}
            if (exc->oid_len)
		{
		fputs("#ifdef OID\n",outfile);
                sprintf(buffer,"    *action_response_type_oid = &%s%s_OID;\n", 
			class->prefix_m,exc->exception_name);
            	fputs(buffer, outfile);	    
		fputs("#endif /* OID */\n",outfile);		
		}

	    exception_found = 1;
	    }

        exc = exc->next;
        }

    if (!exception_found)
        fputs("/** No exception OID found **/\n",outfile);

    return 1;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_action_success_oid
**
**	This routine generates the code for the action success OID in
**	the perform_action routines.
**
**	This routine is called for tag insert-code-action-success-oid.
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
int insert_action_success_oid( mom,
			       outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    int 	found=0;
    DIRECTIVE_DEF *dir;
    EXCEPTION_DEF *exc;
    RESPONSE_DEF  *resp;
    int 	response_num;
    int		exception_num;
    CLASS_DEF   *class;
    int		class_num;
    char	buffer[256];
    int		action_num,actions=0;
    int 	success_found = 0;
    ACTION_DEF	*action;
    momgen_status status;

    /* First, find the exceptions */

    class = mom->current_class;
    dir = class->directive;
    action = class->current_action;
    if (action == NULL)
        return 1;

    while (!found && (dir != NULL))
	if (action->action_number == dir->directive_number)
	    found = 1;
	else
           dir = dir->next;
	    
    if ((!found) || ((dir->num_exceptions == 0) && (dir->num_responses == 0)))
	{
        fputs("/** No exceptions or responses found for this action **/\n",outfile);
        return 1;
	}

    /* 
     * At this point, we have some exceptions and/or responses.
     * Print out the exceptions looking for the success OID. If not found 
     * print a message saying success OID not there...
     */

    exc = dir->exc;
    for (exception_num=0; exception_num < dir->num_exceptions; exception_num++)
	{
        if (strstr(exc->exception_name,"SUCC") != NULL)
	    {
            if (exc->dna_cmip_int_len)
		{
		fputs("#ifdef DNA_CMIP_OID\n",outfile);
                sprintf(buffer,"  *action_response_type_oid = &%s%s_DNA;\n", 
			class->prefix_m,exc->exception_name);
            	fputs(buffer, outfile);	    
	    	success_found = 1;
		fputs("#endif /* DNA_CMIP_INT */\n",outfile);		
		}
            if (exc->osi_oid_len)
		{
		fputs("#ifdef OSI_CMIP_INT\n",outfile);
                sprintf(buffer,"  *action_response_type_oid = &%s%s_OSI;\n", 
			class->prefix_m,exc->exception_name);
            	fputs(buffer, outfile);	    
	    	success_found = 1;
		fputs("#endif /* OSI_CMIP_INT */\n",outfile);		
		}
            if (exc->snmp_oid_len)
		{
		fputs("#ifdef SNMP_OID\n",outfile);
                sprintf(buffer,"  *action_response_type_oid = &%s%s_SNP;\n", 
			class->prefix_m,exc->exception_name);
            	fputs(buffer, outfile);	    
	    	success_found = 1;
		fputs("#endif /* SNMP_OID */\n",outfile);		
		}
            if (exc->oid_len)
		{
		fputs("#ifdef OID\n",outfile);
                sprintf(buffer,"  *action_response_type_oid = &%s%s_OID;\n", 
			class->prefix_m,exc->exception_name);
            	fputs(buffer, outfile);	    
	    	success_found = 1;
		fputs("#endif /* OID */\n",outfile);		
		}
	    }

        exc = exc->next;
        }

    resp = dir->resp;
    for (response_num=0; response_num < dir->num_responses; response_num++)
	{
        if (strstr(resp->response_name,"SUCC") != NULL)
	    {
	    if (success_found)
                fputs("/** Multiple action exceptions and responses found **/\n",outfile);
            if (resp->dna_cmip_int_len)
		{
		fputs("#ifdef DNA_CMIP_OID \n",outfile);
                sprintf(buffer,"  *action_response_type_oid = &%s%s_DNA;\n", 
			class->prefix_m,resp->response_name);
            	fputs(buffer, outfile);	    
	    	success_found = 1;
		fputs("#endif /* DNA_CMIP_INT */\n",outfile);		
		}
            if (resp->osi_oid_len)
		{
		fputs("#ifdef OSI_CMIP_INT\n",outfile);
                sprintf(buffer,"  *action_response_type_oid = &%s%s_OSI;\n", 
			class->prefix_m,resp->response_name);
            	fputs(buffer, outfile);	    
	    	success_found = 1;
		fputs("#endif /* OSI_CMIP_INT */\n",outfile);		
		}
            if (resp->snmp_oid_len)
		{
		fputs("#ifdef SNMP_OID\n",outfile);
                sprintf(buffer,"  *action_response_type_oid = &%s%s_SNP;\n", 
			class->prefix_m,resp->response_name);
            	fputs(buffer, outfile);	    
	    	success_found = 1;
		fputs("#endif /* SNMP_OID */\n",outfile);		
		}
            if (resp->oid_len)
		{
		fputs("#ifdef OID\n",outfile);
                sprintf(buffer,"  *action_response_type_oid = &%s%s_OID;\n", 
			class->prefix_m,resp->response_name);
            	fputs(buffer, outfile);	    
	    	success_found = 1;
		fputs("#endif /* OID */\n",outfile);		
		}
	    success_found = 1;
	    }

        resp = resp->next;
        }

    if (!success_found)
        fputs("/** No action success OID found for action_response_type_oid **/\n",outfile);

    return 1;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_delete_success_oid
**
**	This routine generates the code for the delete success OID in
**	the perform_delete routines.
**
**	This routine is called for tag insert-code-delete-success-oid.
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
int insert_delete_success_oid( mom,
			       outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    int 	found=0;
    DIRECTIVE_DEF *dir;
    EXCEPTION_DEF *exc;
    RESPONSE_DEF  *resp;
    int 	response_num;
    int		exception_num;
    CLASS_DEF   *class;
    int		class_num;
    char	buffer[256];
    int		action_num,actions=0;
    int 	success_found = 0;
    ACTION_DEF	*action;
    momgen_status status;

    /* First, find the exceptions */

    class = mom->current_class;
    dir = class->directive;

    while (!found && (dir != NULL))
	if (strcmp(dir->directive,"DELETE") == 0)   
	    found = 1;
	else
           dir = dir->next;
	    
    if ((!found) || ((dir->num_exceptions == 0) && (dir->num_responses == 0)))
	{
        fputs("/** No exceptions or responses found for this delete **/\n",outfile);
        return 1;
	}

    /* 
     * At this point, we have some exceptions and/or responses. 
     * Print out the exceptions looking for the success OID. If not found 
     * print a message saying success OID not there...
     */

    exc = dir->exc;
    for (exception_num=0; exception_num < dir->num_exceptions; exception_num++)
	{
        if (strstr(exc->exception_name,"SUCC") != NULL)
	    {
            if (exc->dna_cmip_int_len)
		{
		fputs("#ifdef DNA_CMIP_OID\n",outfile);
                sprintf(buffer,"  *action_response_type_oid = &%s%s_DNA;\n", 
			class->prefix_m,exc->exception_name);
            	fputs(buffer, outfile);	    
	    	success_found = 1;
		fputs("#endif /* DNA_CMIP_INT */\n",outfile);		
		}
            if (exc->osi_oid_len)
		{
		fputs("#ifdef OSI_CMIP_INT\n",outfile);
                sprintf(buffer,"  *action_response_type_oid = &%s%s_OSI;\n", 
			class->prefix_m,exc->exception_name);
            	fputs(buffer, outfile);	    
	    	success_found = 1;
		fputs("#endif /* OSI_CMIP_INT */\n",outfile);		
		}
            if (exc->snmp_oid_len)
		{
		fputs("#ifdef SNMP_OID\n",outfile);
                sprintf(buffer,"  *action_response_type_oid = &%s%s_SNP;\n", 
			class->prefix_m,exc->exception_name);
            	fputs(buffer, outfile);	    
	    	success_found = 1;
		fputs("#endif /* SNMP_OID */\n",outfile);		
		}
            if (exc->oid_len)
		{
		fputs("#ifdef OID\n",outfile);
                sprintf(buffer,"  *action_response_type_oid = &%s%s_OID;\n", 
			class->prefix_m,exc->exception_name);
            	fputs(buffer, outfile);	    
	    	success_found = 1;
		fputs("#endif /* OID */\n",outfile);		
		}
	    success_found = 1;
	    }

        exc = exc->next;
        }

    resp = dir->resp;
    for (response_num=0; response_num < dir->num_responses; response_num++)
	{
        if (strstr(resp->response_name,"SUCC") != NULL)
	    {
	    if (success_found)
                fputs("/** Multiple delete exceptions and responses found **/\n",outfile);
            if (resp->dna_cmip_int_len)
		{
		fputs("#ifdef DNA_CMIP_OID\n",outfile);
                sprintf(buffer,"  *action_response_type_oid = &%s%s_DNA;\n", 
			class->prefix_m,resp->response_name);
            	fputs(buffer, outfile);	    
	    	success_found = 1;
		fputs("#endif /* DNA_CMIP_INT */\n",outfile);		
		}
            if (resp->osi_oid_len)
		{
		fputs("#ifdef OSI_CMIP_INT\n",outfile);
                sprintf(buffer,"  *action_response_type_oid = &%s%s_OSI;\n", 
			class->prefix_m,resp->response_name);
            	fputs(buffer, outfile);	    
	    	success_found = 1;
		fputs("#endif /* OSI_CMIP_INT */\n",outfile);		
		}
            if (resp->snmp_oid_len)
		{
		fputs("#ifdef SNMP_OID\n",outfile);
                sprintf(buffer,"  *action_response_type_oid = &%s%s_SNP;\n", 
			class->prefix_m,resp->response_name);
            	fputs(buffer, outfile);	    
	    	success_found = 1;
		fputs("#endif /* SNMP_OID */\n",outfile);		
		}
            if (resp->oid_len)
		{
		fputs("#ifdef OID\n",outfile);
                sprintf(buffer,"  *action_response_type_oid = &%s%s_OID;\n", 
			class->prefix_m,resp->response_name);
            	fputs(buffer, outfile);	    
	    	success_found = 1;
		fputs("#endif /* OID */\n",outfile);		
		}
	    success_found = 1;
	    }
        resp = resp->next;
        }

    if (!success_found)
        fputs("/** No delete success OID found for action_response_type_oid **/\n",outfile);

    return 1;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_action_success_reply
**
**	This routine generates the code for the action success reply in
**	directive.c. Each action (other than create) returns a success
**	reply oid to moss_send_action_reply.  It also generates the
**	appropriate imports and externs.  
**
**	This routine is called for tag insert-code-action-success-reply.
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
int insert_action_success_reply( mom,
			 	 outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    CLASS_DEF   *class;
    int		class_num;
    char	buffer[256];
    int		action_num,actions=0;
    ACTION_DEF	*action;
    momgen_status status;

    /* First, print the imports */

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
        {
    	action = class->action;
    	for (action_num=0; action_num < class->num_actions; action_num++)
    	    {
	    if (action->action_number)
		{
		if (actions == 0)
		    {
                    fputs("                        else {\n\n",outfile);
		    }
		sprintf(buffer,"                         IMPORT object_id %sRESP_%s_SUCCESS_oid;\n",
			class->prefix_m,action->action_name);
		fputs(buffer, outfile);	    
		actions++;
		}
            action = action->next;
            }
        class = class->next;
        }

    if (actions == 0)
        return 1;

    fputs("                         struct object_id *action_response_type;\n",outfile);
    fputs("                         int action_num = NULL;\n\n",outfile);
    fputs("                         status = (man_status) _moss_get_oid_last( action_type, &action_num);\n\n",outfile);
    fputs("                         switch (class_code)\n                           {\n",outfile);

    actions = 0;

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
        {
        sprintf(buffer,"                           case %s%s_ID:\n",class->prefix_k,class->class_name);
        fputs(buffer,outfile);
	fputs( 		"                             switch (action_num) { \n",outfile);
	
    	action = class->action;
    	for (action_num=0; action_num < class->num_actions; action_num++)
    	    {
	    if (action->action_number)
		{
		sprintf(buffer,"                             case %d:\n",action->action_number);
        	fputs(buffer,outfile);
		sprintf(buffer,"                               action_response_type = &%sRESP_%s_SUCCESS_oid;\n",
				class->prefix_m,action->action_name);
        	fputs(buffer,outfile);
		fputs(		"                                break;\n",outfile);
		}
            action = action->next;
            }
	fputs(		"                             }\n                             break;\n\n",outfile);

        class = class->next;
        }

    fputs("                           }\n\n",outfile);
    fputs("      	            	status = (reply_routine)(invoke_identifier,\n",outfile);
    fputs("  						 handle,\n",outfile);
    fputs(" 					     	 return_status,\n",outfile);
    fputs(" 					     	 object_class,\n",outfile);
    fputs(" 					     	 object_instance,\n",outfile);
    fputs("                                                  object_uid,\n",outfile);
    fputs(" 					     	 *operation_time,\n",outfile);
    fputs("                                                  action_type,\n",outfile);
    fputs("                                                  action_response_type,\n",outfile);
    fputs(" 					     	 return_avl,\n",outfile);
    fputs(" 					     	 more_instances);\n",outfile);
    fputs("                          }\n", outfile);
        	    
    return SS$_NORMAL;
}
