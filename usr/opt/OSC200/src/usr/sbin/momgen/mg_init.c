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
static char *rcsid = "@(#)$RCSfile: mg_init.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/06/15 11:55:12 $";
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
**	This module contains the routines to used to generate init routines
**	and the OID definitions.
**
**  AUTHORS:
**
**      Gary Allison
**
**	Some routines were extracted from BUILD.C (Rich Bouchard, Jr.) to 
**	form this module.
**
**  CREATION DATE:  23-Sep-1991
**
**  MODIFICATION HISTORY:
**
**	F-19	RJB1238	    Richard J. Bouchard Jr. 05-Aug-1992
**	Create object_id's as 'unsigned' integers
**
**  	F-18	    	    Marc Nozell	    	    15-Jun-1992
**	Remove references to module.
**	Fixed arrays generated to export_oids.h to match the definition of
**      object_id, `long int' instead of `long signed int'. (insert_init_exports) 
**      
**	F-3		    Mike Densmore	    20-May-1992
**
**	Modified for use on U**X platforms:
**	    - define module as ident
**	    - cast mallocs for CPTR
**	    - commented out unreachable code in regenerate_common_directives()
**
**	F-2		    Marc Nozell		    24-May-1991
**                            
**	Support multiple classes.
**
**	25-Sep-1992	Mike Densmore	Modified for ANSI Standard C
**--
*/

#ifdef VMS
#include "vaxcshr.h"		/* translate from VAX C to DEC C RTL names */
#endif 
#include "mom_defs.h"
#include <string.h>
#include "mg_prototypes.h"

#if defined(sun) || defined(sparc)
int	check_parent();
void	insert_init_args_event_defs();
void	insert_init_args_event();
void	insert_init_args_event_imports();
void	insert_init_args_create_defs();
void	insert_init_args_create();
void	insert_init_args_create_imports();
void	insert_init_args_resp_defs();
void	insert_init_args_resp();
void	insert_init_args_resp_imports();
void	insert_init_args_exc_defs();
void	insert_init_args_exc();
void	insert_init_args_exc_imports();
void	insert_init_args_req_defs();
void	insert_init_args_req();
void	insert_init_args_req_imports();
#else
int	check_parent(CLASS_DEF *, MOM_BUILD *);
void	insert_init_args_event_defs(EVENT_DEF *, CLASS_DEF *, MOM_BUILD *, FILE *);
void	insert_init_args_event(EVENT_DEF *, CLASS_DEF *, MOM_BUILD *, FILE *);
void	insert_init_args_event_imports(EVENT_DEF *, CLASS_DEF *, MOM_BUILD *, FILE *);
void	insert_init_args_create_defs(CLASS_DEF *, MOM_BUILD *, FILE *);
void	insert_init_args_create(CLASS_DEF *, MOM_BUILD *, FILE *);
void	insert_init_args_create_imports(CLASS_DEF *, MOM_BUILD *, FILE *);
void	insert_init_args_resp_defs(RESPONSE_DEF *, DIRECTIVE_DEF *, CLASS_DEF *, MOM_BUILD *, FILE *);
void	insert_init_args_resp(RESPONSE_DEF *, DIRECTIVE_DEF *, CLASS_DEF *, MOM_BUILD *, FILE *);
void	insert_init_args_resp_imports(RESPONSE_DEF *, DIRECTIVE_DEF *, CLASS_DEF *, MOM_BUILD *, FILE *);
void	insert_init_args_exc_defs(EXCEPTION_DEF *, DIRECTIVE_DEF *, CLASS_DEF *, MOM_BUILD *, FILE *);
void	insert_init_args_exc(EXCEPTION_DEF *, DIRECTIVE_DEF *, CLASS_DEF *, MOM_BUILD *, FILE *);
void	insert_init_args_exc_imports(EXCEPTION_DEF *, DIRECTIVE_DEF *, CLASS_DEF *, MOM_BUILD *, FILE *);
void	insert_init_args_req_defs(REQUEST_DEF *, DIRECTIVE_DEF *, CLASS_DEF *, MOM_BUILD *, FILE *);
void	insert_init_args_req(REQUEST_DEF *, DIRECTIVE_DEF *, CLASS_DEF *, MOM_BUILD *, FILE *);
void	insert_init_args_req_imports(REQUEST_DEF *request, DIRECTIVE_DEF *, CLASS_DEF *, MOM_BUILD *, FILE *);
#endif


int insert_iso_dna( mom,
		    outfile)

MOM_BUILD *mom;
FILE *outfile;

{
fputs("#include \"iso_defs.h\"\n", outfile);

return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_init_exports
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on tag "insert-code-init-exports".
**
**	This routine generates the EXPORT statements to define globals
**	for the Object IDs (OIDs) of the classes, attributes and arguments.
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
int insert_init_exports( mom,
		         outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    char		buffer[256];
    int			length;
    int			class_num;
    CLASS_DEF		*class;
    int			attribute_num;
    ATTRIBUTE_DEF	*attribute;
    int			directive_num;
    DIRECTIVE_DEF	*directive;
    int			argument_num;
    ARGUMENT_DEF	*argument;
    int			request_num;
    REQUEST_DEF		*request;
    int			response_num;
    RESPONSE_DEF	*response;
    int			exception_num;
    EXCEPTION_DEF	*exception;
    int			event_num;
    EVENT_DEF		*event;
    int			first = 1;
    
    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
      {
      if ((class->orig_parent_class_string != NULL) && (check_parent( class, mom)))
	{

        /* 
         * Generate the parent OID  -- generated from the OID_TYPE= in the parameter file
	 */

	if (mom->oid_type == DNA_CMIP_OID)
	    {
	    class->parent_generated = 1;
            sprintf(buffer, "static unsigned int %s_DA[] = {ENTITIES_SEQ, %s_DS};\n", 
		class->parent, class->parent);
            fputs(buffer, outfile); 
            sprintf(buffer,  "EXPORT struct object_id %s_DNA = {ENTITIES_LENGTH + %s_DL, %s_DA};\n", 
		class->parent, class->parent, class->parent);
            fputs(buffer, outfile); 
	    }
	if (mom->oid_type == OSI_CMIP_OID)
	    {
	    class->parent_generated = 1;
            sprintf(buffer, "static unsigned int %s_OA[] = {%s_OS};\n", 
		class->parent, class->parent);
            fputs(buffer, outfile); 
            sprintf(buffer,  "EXPORT struct object_id %s_OSI = {%s_OL, %s_OA};\n", 
		class->parent, class->parent, class->parent);
            fputs(buffer, outfile); 
	    }
	if (mom->oid_type == OID)
	    {
	    class->parent_generated = 1;
            sprintf(buffer, "static unsigned int %s_NA[] = {%s_NS};\n", 
		class->parent, class->parent);
            fputs(buffer, outfile); 
            sprintf(buffer,  "EXPORT struct object_id %s_OID = {%s_NL, %s_NA};\n", 
		class->parent, class->parent, class->parent);
            fputs(buffer, outfile); 
	    }
	if (mom->oid_type == SNMP_OID)
	    {
	    class->parent_generated = 1;
            sprintf(buffer, "static unsigned int %s_SA[] = {%s_SS};\n", 
		class->parent, class->parent);
            fputs(buffer, outfile); 
            sprintf(buffer,  "EXPORT struct object_id %s_SNP = {%s_SL, %s_SA};\n", 
		class->parent, class->parent, class->parent);
            fputs(buffer, outfile); 
	    }

        }

      /* 
       * Generate the class OID -- generated if the OID,DNA_CMIP_INT is specified
       * in the MSL file.
       */

      if (class->dna_cmip_int_len)
	{
        sprintf(buffer, "static unsigned int %sDA[] = {ENTITIES_SEQ, %sDS};\n", 
		class->prefix, class->prefix);
        fputs(buffer, outfile); 
        sprintf(buffer,  "EXPORT struct object_id %s_DNA = {ENTITIES_LENGTH + %sDL, %sDA};\n", 
		class->class_name, class->prefix, class->prefix);
        fputs(buffer, outfile); 
	}      

      if (class->oid_len)
        {
	sprintf(buffer, "static unsigned int %sNA[] = {%sNS};\n", 
		class->prefix, class->prefix);
        fputs(buffer, outfile); 
	sprintf(buffer, "EXPORT struct object_id %s_OID = {%sNL, %sNA};\n", 
		class->class_name, class->prefix, class->prefix);
	fputs(buffer, outfile);
	}
      if (class->snmp_oid_len)                  /*   ********** FIX THESE ************* */
        {
	sprintf(buffer, "static unsigned int %sSA[] = {%sSS};\n", 
		class->prefix, class->prefix);
        fputs(buffer, outfile); 
	sprintf(buffer, "EXPORT struct object_id %s_SNP = {%sSL, %sSA};\n", 
		class->class_name, class->prefix, class->prefix);
	fputs(buffer, outfile);
	}
      if (class->osi_oid_len)
        {
	sprintf(buffer, "static unsigned int %sOA[] = {%sOS};\n", 
		class->prefix, class->prefix);
        fputs(buffer, outfile); 
	sprintf(buffer, "EXPORT struct object_id %s_OSI = {%sOL, %sOA};\n", 
		class->class_name, class->prefix, class->prefix);
	fputs(buffer, outfile);
	}

      if (class->dna_cmip_int_len)
	{
	sprintf(buffer, "static unsigned int %sattr_prefix_DA[] = {ATTRIBUTES_SEQ, %sDS, EOECCS };\n", 
		    class->prefix_m, class->prefix );
	fputs(buffer, outfile);

	sprintf(buffer, "EXPORT struct object_id %sattr_prefix_DNA = {ATTRIBUTES_LENGTH + %sDL + 1, %sattr_prefix_DA};\n", 
		class->prefix_m, class->prefix, class->prefix_m); 
	fputs(buffer, outfile);
	}

	if (class->num_attributes)
	    {
            fputs("\n/*", outfile);
	    sprintf(buffer, " Attribute data for %s class", class->class_name);
	    fputs(buffer, outfile);
	    fputs(" */\n\n", outfile);
	    }

	attribute = class->attribute;
	for (attribute_num=0; attribute_num < class->num_attributes; attribute_num++)
	{
	  if (attribute->dna_cmip_int_len)
	    {
	    sprintf(buffer, "static unsigned int %s%s_DA[] = {%s%s_DS};\n",
		    class->prefix_m, attribute->attribute_name,
		    class->prefix_m, attribute->attribute_name);
	    fputs(buffer, outfile);
	    sprintf(buffer, "EXPORT struct object_id %s%s_DNA = {%s%s_DL, %s%s_DA};\n",
		    class->prefix_m, attribute->attribute_name, 
		    class->prefix_m, attribute->attribute_name,
		    class->prefix_m, attribute->attribute_name);
	    fputs(buffer, outfile);
	    }

	  if (attribute->oid_len)
	    {
	    sprintf(buffer, "static unsigned int %s%s_NA[] = {%s%s_NS};\n",
		    class->prefix_m, attribute->attribute_name,
		    class->prefix_m, attribute->attribute_name);
	    fputs(buffer, outfile);
	    sprintf(buffer, "EXPORT struct object_id %s%s_OID = {%s%s_NL, %s%s_NA};\n",
		    class->prefix_m, attribute->attribute_name, 
		    class->prefix_m, attribute->attribute_name,
		    class->prefix_m, attribute->attribute_name);
	    fputs(buffer, outfile);
	    }

	  if (attribute->snmp_oid_len)
	    {
	    sprintf(buffer, "static unsigned int %s%s_SA[] = {%s%s_SS};\n",
		    class->prefix_m, attribute->attribute_name,
		    class->prefix_m, attribute->attribute_name);
	    fputs(buffer, outfile);
	    sprintf(buffer, "EXPORT struct object_id %s%s_SNP = {%s%s_SL, %s%s_SA};\n",
		    class->prefix_m, attribute->attribute_name, 
		    class->prefix_m, attribute->attribute_name,
		    class->prefix_m, attribute->attribute_name);
	    fputs(buffer, outfile);
	    }

	  if (attribute->osi_oid_len)
	    {
	    sprintf(buffer, "static unsigned int %s%s_OA[] = {%s%s_OS};\n",
		    class->prefix_m, attribute->attribute_name,
		    class->prefix_m, attribute->attribute_name);
	    fputs(buffer, outfile);
	    sprintf(buffer, "EXPORT struct object_id %s%s_OSI = {%s%s_OL, %s%s_OA};\n",
		    class->prefix_m, attribute->attribute_name, 
		    class->prefix_m, attribute->attribute_name,
		    class->prefix_m, attribute->attribute_name);
	    fputs(buffer, outfile);
	    }

	  attribute = attribute->next;
	  }

	if (class->num_events)
	    {
            fputs("\n/*", outfile);
	    sprintf(buffer, " Event data for %s class", class->class_name);
	    fputs(buffer, outfile);
	    fputs(" */\n\n", outfile);
	    }

	event = class->event;
	for (event_num=0; event_num < class->num_events; event_num++)
	{
	  if (event->dna_cmip_int_len)
	    {
	    sprintf(buffer, "static unsigned int %s%s_DA[] = {%s%s_DS};\n",
		    class->prefix_m, event->event_name,
		    class->prefix_m, event->event_name);
	    fputs(buffer, outfile);
	    sprintf(buffer, "EXPORT struct object_id %s%s_DNA = {%s%s_DL, %s%s_DA};\n",
		    class->prefix_m, event->event_name,
		    class->prefix_m, event->event_name,
		    class->prefix_m, event->event_name);
	    fputs(buffer, outfile);
	    }

	  if (event->oid_len)
	    {
	    sprintf(buffer, "static unsigned int %s%s_NA[] = {%s%s_NS};\n",
		    class->prefix_m, event->event_name,
		    class->prefix_m, event->event_name);
	    fputs(buffer, outfile);
	    sprintf(buffer, "EXPORT struct object_id %s%s_OID = {%s%s_NL, %s%s_NA};\n",
		    class->prefix_m, event->event_name,
		    class->prefix_m, event->event_name,
		    class->prefix_m, event->event_name);
	    fputs(buffer, outfile);
	    }

	  if (event->snmp_oid_len)
	    {
	    sprintf(buffer, "static unsigned int %s%s_SA[] = {%s%s_SS};\n",
		    class->prefix_m, event->event_name,
		    class->prefix_m, event->event_name);
	    fputs(buffer, outfile);
	    sprintf(buffer, "EXPORT struct object_id %s%s_SNP = {%s%s_SL, %s%s_SA};\n",
		    class->prefix_m, event->event_name,
		    class->prefix_m, event->event_name,
		    class->prefix_m, event->event_name);
	    fputs(buffer, outfile);
	    }

	  if (event->osi_oid_len)
	    {
	    sprintf(buffer, "static unsigned int %s%s_OA[] = {%s%s_OS};\n",
		    class->prefix_m, event->event_name,
		    class->prefix_m, event->event_name);
	    fputs(buffer, outfile);
	    sprintf(buffer, "EXPORT struct object_id %s%s_OSI = {%s%s_OL, %s%s_OA};\n",
		    class->prefix_m, event->event_name,
		    class->prefix_m, event->event_name,
		    class->prefix_m, event->event_name);
	    fputs(buffer, outfile);
	    }

	  insert_init_args_event( event, class, mom, outfile );

	  event = event->next;
	}

	if (class->num_create_args)
	    insert_init_args_create( class, mom, outfile );

	directive = class->directive;
	for (directive_num=0; directive_num < class->num_directives; directive_num++)
	{
          if (directive->type == MOMGEN_K_DIR_ACTION)
            {
   	    fputs("\n/*", outfile);
	    sprintf(buffer, " Directive data for class %s action directive %s",
		class->class_name, directive->directive);
	    fputs(buffer, outfile);
	    fputs(" */\n\n", outfile);

	   if (directive->dna_cmip_int_len)
	    {
	    sprintf(buffer, "static unsigned int %s%s_DA[] = {%s%s_DS};\n",
		    class->prefix_m, directive->directive_name,
		    class->prefix_m, directive->directive_name);
	    fputs(buffer, outfile);
	    sprintf(buffer, "EXPORT struct object_id %s%s_DNA = {%s%s_DL, %s%s_DA};\n",
		    class->prefix_m, directive->directive_name,
		    class->prefix_m, directive->directive_name, 
		    class->prefix_m, directive->directive_name);
	    fputs(buffer, outfile);
	    }

	   if (directive->oid_len)
	    {
	    sprintf(buffer, "static unsigned int %s%s_NA[] = {%s%s_NS};\n",
		    class->prefix_m, directive->directive_name,
		    class->prefix_m, directive->directive_name);
	    fputs(buffer, outfile);
	    sprintf(buffer, "EXPORT struct object_id %s%s_OID = {%s%s_NL, %s%s_NA};\n",
		    class->prefix_m, directive->directive_name,
		    class->prefix_m, directive->directive_name, 
		    class->prefix_m, directive->directive_name);
	    fputs(buffer, outfile);
	    }

	   if (directive->snmp_oid_len)
	    {
	    sprintf(buffer, "static unsigned int %s%s_SA[] = {%s%s_SS};\n",
		    class->prefix_m, directive->directive_name,
		    class->prefix_m, directive->directive_name);
	    fputs(buffer, outfile);
	    sprintf(buffer, "EXPORT struct object_id %s%s_SNP = {%s%s_SL, %s%s_SA};\n",
		    class->prefix_m, directive->directive_name,
		    class->prefix_m, directive->directive_name, 
		    class->prefix_m, directive->directive_name);
	    fputs(buffer, outfile);
	    }
	   if (directive->osi_oid_len)
	    {
	    sprintf(buffer, "static unsigned int %s%s_OA[] = {%s%s_OS};\n",
		    class->prefix_m, directive->directive_name,
		    class->prefix_m, directive->directive_name);
	    fputs(buffer, outfile);
	    sprintf(buffer, "EXPORT struct object_id %s%s_OSI = {%s%s_OL, %s%s_OA};\n",
		    class->prefix_m, directive->directive_name,      
		    class->prefix_m, directive->directive_name, 
		    class->prefix_m, directive->directive_name);
	    fputs(buffer, outfile);
	    }
	
	    /* Write out all directive exceptions, responses, requests */

            if (directive->num_responses)
		{
	        fputs("\n/*", outfile);
		sprintf(buffer, " Response data for class %s action directive %s",
			class->class_name, directive->directive);
		fputs(buffer, outfile);
		fputs(" */\n\n", outfile);
	
		response = directive->resp;
		for (response_num=0; response_num < directive->num_responses; response_num++)
		{
		  if (response->dna_cmip_int_len)
		    {
		    sprintf(buffer, "static unsigned int %s%s_DA[] = {%s%s_DS};\n",
			    class->prefix_m, response->response_name,
			    class->prefix_m, response->response_name);
		    fputs(buffer, outfile);
		    sprintf(buffer, "EXPORT struct object_id %s%s_DNA = {%s%s_DL, %s%s_DA};\n",
			    class->prefix_m, response->response_name,
			    class->prefix_m, response->response_name,
			    class->prefix_m, response->response_name);
		    fputs(buffer, outfile);
		    }
		  if (response->oid_len)
		    {
		    sprintf(buffer, "static unsigned int %s%s_NA[] = {%s%s_NS};\n",
			    class->prefix_m, response->response_name,
			    class->prefix_m, response->response_name);
		    fputs(buffer, outfile);
		    sprintf(buffer, "EXPORT struct object_id %s%s_OID = {%s%s_NL, %s%s_NA};\n",
			    class->prefix_m, response->response_name,
			    class->prefix_m, response->response_name,
			    class->prefix_m, response->response_name);
		    fputs(buffer, outfile);
		    }

		  if (response->snmp_oid_len)
		    {
		    sprintf(buffer, "static unsigned int %s%s_SA[] = {%s%s_SS};\n",
			    class->prefix_m, response->response_name,
			    class->prefix_m, response->response_name);
		    fputs(buffer, outfile);
		    sprintf(buffer, "EXPORT struct object_id %s%s_SNP = {%s%s_SL, %s%s_SA};\n",
			    class->prefix_m, response->response_name,
			    class->prefix_m, response->response_name,
			    class->prefix_m, response->response_name);
		    fputs(buffer, outfile);
		    }

		  if (response->osi_oid_len)
		    {
		    sprintf(buffer, "static unsigned int %s%s_OA[] = {%s%s_OS};\n",
			    class->prefix_m, response->response_name,
			    class->prefix_m, response->response_name);
		    fputs(buffer, outfile);
		    sprintf(buffer, "EXPORT struct object_id %s%s_OSI = {%s%s_OL, %s%s_OA};\n",
			    class->prefix_m, response->response_name,
			    class->prefix_m, response->response_name,
			    class->prefix_m, response->response_name);
		    fputs(buffer, outfile);
		    }

		  if (response->num_arguments)
			insert_init_args_resp( response, directive, class, mom, outfile );

		    response = response->next;
		    }

	  	}
            if (directive->num_exceptions)
		{
	
	        fputs("\n/*", outfile);
		sprintf(buffer, " Exception data for class %s action directive %s",
			class->class_name, directive->directive);
		fputs(buffer, outfile);
		fputs(" */\n\n", outfile);
	
		exception = directive->exc;
		for (exception_num=0; exception_num < directive->num_exceptions; exception_num++)
		  {
		  if (exception->dna_cmip_int_len)
		    { 
		    sprintf(buffer,
			    "static unsigned int %s%s_DA[] = {%s%s_DS};\n",
			    class->prefix_m, exception->exception_name,
			    class->prefix_m, exception->exception_name);
		    fputs(buffer, outfile);
		    sprintf(buffer, "EXPORT struct object_id %s%s_DNA = {%s%s_DL, %s%s_DA};\n",
			    class->prefix_m, exception->exception_name,
			    class->prefix_m, exception->exception_name,
			    class->prefix_m, exception->exception_name);
		    fputs(buffer, outfile);
		    }
		  if (exception->oid_len)
		    { 
		    sprintf(buffer,
			    "static unsigned int %s%s_NA[] = {%s%s_NS};\n",
			    class->prefix_m, exception->exception_name,
			    class->prefix_m, exception->exception_name);
		    fputs(buffer, outfile);
		    sprintf(buffer, "EXPORT struct object_id %s%s_OID = {%s%s_NL, %s%s_NA};\n",
			    class->prefix_m, exception->exception_name,
			    class->prefix_m, exception->exception_name,
			    class->prefix_m, exception->exception_name);
		    fputs(buffer, outfile);
		    }

		  if (exception->snmp_oid_len)
		    { 
		    sprintf(buffer,
			    "static unsigned int %s%s_SA[] = {%s%s_SS};\n",
			    class->prefix_m, exception->exception_name,
			    class->prefix_m, exception->exception_name);
		    fputs(buffer, outfile);
		    sprintf(buffer, "EXPORT struct object_id %s%s_SNP = {%s%s_SL, %s%s_SA};\n",
			    class->prefix_m, exception->exception_name,
			    class->prefix_m, exception->exception_name,
			    class->prefix_m, exception->exception_name);
		    fputs(buffer, outfile);
		    }
		  if (exception->osi_oid_len)
		    { 
		    sprintf(buffer,
			    "static unsigned int %s%s_OA[] = {%s%s_OS};\n",
			    class->prefix_m, exception->exception_name,
			    class->prefix_m, exception->exception_name);
		    fputs(buffer, outfile);
		    sprintf(buffer, "EXPORT struct object_id %s%s_OSI = {%s%s_OL, %s%s_OA};\n",
			    class->prefix_m, exception->exception_name,
			    class->prefix_m, exception->exception_name,
			    class->prefix_m, exception->exception_name);
		    fputs(buffer, outfile);
		    }

		  if (exception->num_arguments)
			insert_init_args_exc( exception, directive, class, mom, outfile );

		    exception = exception->next;
		    }
	  	}

            if (directive->num_requests)
		{
		request = directive->req;
		for (request_num=0; request_num < directive->num_requests; request_num++)
		    {
		    if (request->num_arguments)
			insert_init_args_req( request, directive, class, mom, outfile );

		    request = request->next;
		    }
		}
	    }
          directive = directive->next;
	  }	
    class = class->next;
    }

    /* Generate the EMA style OIDs unless doing an SNMP MOM */

    if (!mom->OID_SNMP) {
        fputs("/*\n", outfile);
        fputs(" *	EMA Universal Attribute Group OIDs\n", outfile);
        fputs(" */\n\n", outfile);
        fputs("static unsigned int univ_attr_array [] = {UNIV_ATTR_GROUP_SEQ};\n", outfile);
        fputs("EXPORT struct object_id  UNIV_ATTR_DNA = {UNIV_ATTR_GROUP_LENGTH, univ_attr_array};\n", outfile);
        fputs("static unsigned int univ_all_attr_array [] = {UNIV_ALL_GROUP_SEQ};\n", outfile);
        fputs("EXPORT struct object_id  UNIV_ALL_ATTR_DNA = {UNIV_ALL_GROUP_LENGTH, univ_all_attr_array};\n", outfile);
        fputs("static unsigned int univ_all_id_attr_array [] = {UNIV_ID_GROUP_SEQ};\n", outfile);
        fputs("EXPORT struct object_id  UNIV_ALL_ID_ATTR_DNA = {UNIV_ID_GROUP_LENGTH, univ_all_id_attr_array};\n", outfile);
        fputs("static unsigned int univ_all_status_attr_array [] = {UNIV_STATUS_GROUP_SEQ};\n", outfile);
        fputs("EXPORT struct object_id  UNIV_ALL_STATUS_ATTR_DNA = {UNIV_STATUS_GROUP_LENGTH, univ_all_status_attr_array};\n", outfile);
        fputs("static unsigned int univ_all_counter_attr_array [] = {UNIV_COUNTER_GROUP_SEQ};\n", outfile);
        fputs("EXPORT struct object_id  UNIV_ALL_COUNTER_ATTR_DNA = {UNIV_COUNTER_GROUP_LENGTH, univ_all_counter_attr_array};\n", outfile);
        fputs("static unsigned int univ_all_char_attr_array [] = {UNIV_CHAR_GROUP_SEQ};\n", outfile);
        fputs("EXPORT struct object_id  UNIV_ALL_CHAR_ATTR_DNA = {UNIV_CHAR_GROUP_LENGTH, univ_all_char_attr_array};\n", outfile);
	fputs("\nstatic unsigned int NODE_ATTR_NAME_DA[] = {ATTRIBUTES_SEQ, NODE, EOECCS, 1};\n", outfile);
	fputs("EXPORT struct object_id NODE_ATTR_NAME_DNA = {ATTRIBUTES_LENGTH + NODE_CLASS_LENGTH + 2, NODE_ATTR_NAME_DA};\n", outfile);

    }
    else {  /* generate some required OIDs for SNMP */
	fputs("\n#define SNMP_NULL_NS	1,3,12,2,1011,2,13,1,126\n", outfile);
	fputs("#define SNMP_NULL_NL	9\n", outfile);
	fputs("static unsigned int SNMP_NULL_NA[] = {SNMP_NULL_NS};\n", outfile);
	fputs("EXPORT struct object_id SNMP_NULL_OID = {SNMP_NULL_NL, SNMP_NULL_NA};\n", outfile);
    }

    /* Generate the NULL OID for instanceless classes */

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
	{
	if (!class->support_instances)
	    {
            if (first && !mom->OID_SNMP)
		{
	        fputs("\n#define NULL_ID_CODE	13\n", outfile);	
	        fputs("#define NULL_ID_SEQ	EMA_SEQ, NULL_ID_CODE\n", outfile);
	        fputs("#define NULL_ID_LENGTH  EMA_LENGTH + 1\n\n", outfile);
	        fputs("static unsigned int NULL_ID_ATTRIBUTE_array[] = 			{NULL_ID_SEQ};\n", outfile);
	        fputs("EXPORT struct object_id  NULL_ID_ATTRIBUTE_DNA =   	{NULL_ID_LENGTH, NULL_ID_ATTRIBUTE_array};\n\n", outfile);
		first = 0;
		}	
       	    if (class->dna_cmip_int_len)
		{
        	sprintf(buffer, "\nstatic unsigned int %sNULL_DA[] = {NULL_ID_SEQ, %sDS, EOECCS};\n", 
			class->prefix, class->prefix);
        	fputs(buffer, outfile); 
        	sprintf(buffer,  "EXPORT struct object_id %sNULL_DNA = {NULL_ID_LENGTH + %sDL + 1, %sNULL_DA};\n", 
			class->prefix, class->prefix, class->prefix);
        	fputs(buffer, outfile); 
		}      
	    if (class->snmp_oid_len)
		{
        	sprintf(buffer,"\n#define %sNULL_SS   1,3,12,2,1011,2,13,1,%d,126\n",class->prefix,class->class_number);
        	fputs(buffer, outfile); 
        	sprintf(buffer,"#define %sNULL_SL    10 \n\n",class->prefix);
        	fputs(buffer, outfile); 
        	sprintf(buffer, "static unsigned int %sNULL_SA[] = {%sNULL_SS};\n", 
			class->prefix, class->prefix);
        	fputs(buffer, outfile); 
        	sprintf(buffer,  "EXPORT struct object_id %sNULL_SNP = {%sNULL_SL, %sNULL_SA};\n", 
			class->prefix, class->prefix, class->prefix);
        	fputs(buffer, outfile); 
		}      
	    if (class->oid_len)
		{
        	sprintf(buffer,"\n#define %sNULL_NS    1,3,12,2,1011,2,13,1,%d,126\n",class->prefix,class->class_number);
        	fputs(buffer, outfile); 
        	sprintf(buffer,"#define %sNULL_NL    10 \n\n",class->prefix);
        	fputs(buffer, outfile); 
        	sprintf(buffer, "static unsigned int %sNULL_NA[] = {%sNULL_NS};\n", 
			class->prefix, class->prefix);
        	fputs(buffer, outfile); 
        	sprintf(buffer,  "EXPORT struct object_id %sNULL_OID = {%sNULL_NL, %sNULL_NA};\n", 
			class->prefix, class->prefix, class->prefix);
        	fputs(buffer, outfile); 
		}      

	    /*** IS THERE A NULL OID FOR OSI??  ***/

	    }
        class = class->next;
        }

return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_init_defines
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on tag "insert-code-init-defines".
**
**	This routine generates define statements for the Object IDs (OIDs) 
**	of the classes, attributes and arguments.
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
int insert_init_defines( mom,
		         outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    char		buffer[256];
    int			length;
    int			class_num;
    CLASS_DEF		*class;
    int			attribute_num;
    ATTRIBUTE_DEF	*attribute;
    int			directive_num;
    DIRECTIVE_DEF	*directive;
    int			argument_num;
    ARGUMENT_DEF	*argument;
    int			request_num;
    REQUEST_DEF		*request;
    int			response_num;
    RESPONSE_DEF	*response;
    int			exception_num;
    EXCEPTION_DEF	*exception;
    int			event_num;
    EVENT_DEF		*event;
    
    fputs("/*\n",outfile);
    fputs(" * 			OID DEFINITIONS\n *\n",outfile);
    fputs(" *   The following key describes the suffixes used for OIDs.\n",outfile);
    fputs(" *   _DS - DNA OID sequence,    _DL - DNA OID length,    _DA DNA OID array.    OIDs generated from DNA_CMIP_INT.\n",outfile);
    fputs(" *   _NS - Normal OID sequence, _NL - Normal OID length, _NA Normal OID array. OIDs generated from OID=.\n",outfile);
    fputs(" *   _SS - SNMP OID sequence,   _SL - SNMP OID length,   _SA SNMP OID array.   OIDs generated from SNMP_OID=.\n",outfile);
    fputs(" *   _OS - OSI OID sequence,    _OL - OSI OID length,    _OA OSI OID array.    OIDs generated from OSI_OID=.\n",outfile);
    fputs(" */\n\n\n",outfile);

    fputs("#define NODE_ENTITY ENTITIES_SEQ, 1\n",outfile);
    fputs("#define NODE_ENTITY_LENGTH ENTITIES_LENGTH + 1\n\n",outfile);

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
      {
      if ((class->orig_parent_class_string != NULL) && (check_parent( class, mom)))
	{
	if (mom->oid_type == DNA_CMIP_OID)
	    {
	    class->parent_generated = 1;
	    sprintf(buffer, "#define %s_DS %s\n", class->parent, class->parent_class_string);
            fputs(buffer, outfile);
            sprintf(buffer, "#define %s_DL %d\n\n", class->parent, class->steps_below_node );
            fputs(buffer, outfile);
	    }
	if (mom->oid_type == OSI_CMIP_OID)
	    {
	    class->parent_generated = 1;
	    sprintf(buffer, "#define %s_OS %s\n", class->parent, class->parent_class_string);
            fputs(buffer, outfile);
            sprintf(buffer, "#define %s_OL %d\n\n", class->parent, class->parent_class_string_len);
            fputs(buffer, outfile);
	    }
	if (mom->oid_type == SNMP_OID)
	    {
	    class->parent_generated = 1;
	    sprintf(buffer, "#define %s_SS %s\n", class->parent, class->parent_class_string);
            fputs(buffer, outfile);
            sprintf(buffer, "#define %s_SL %d\n\n", class->parent, class->parent_class_string_len);
            fputs(buffer, outfile);
	    }
	if (mom->oid_type == OID)
	    {
	    class->parent_generated = 1;
	    sprintf(buffer, "#define %s_NS %s\n", class->parent, class->parent_class_string);
            fputs(buffer, outfile);
            sprintf(buffer, "#define %s_NL %d\n\n", class->parent, class->parent_class_string_len);
            fputs(buffer, outfile);
	    }
	    
        }

      if (class->dna_cmip_int_len)
	{
        sprintf(buffer, "\n#define %sDS %s\n", class->prefix,class->class_string );
        fputs(buffer, outfile);
        length = class->steps_below_node;
        length++;
        sprintf(buffer, "#define %sDL %d\n", class->prefix,length);
        fputs(buffer, outfile);
	}
      
      if (class->oid_len)
	{
        sprintf(buffer, "#define %sNS %s\n", class->prefix, class->oid_text );
        fputs(buffer, outfile);
        sprintf(buffer, "#define %sNL %d\n", class->prefix,class->oid_len);
        fputs(buffer, outfile);
	}

      if (class->snmp_oid_len)
	{
        sprintf(buffer, "#define %sSS %s\n", class->prefix, class->snmp_oid_text );
        fputs(buffer, outfile);
        sprintf(buffer, "#define %sSL %d\n", class->prefix,class->snmp_oid_len);
        fputs(buffer, outfile);
	}

      if (class->osi_oid_len)
	{
        sprintf(buffer, "#define %sOS %s\n", class->prefix, class->osi_oid_text );
        fputs(buffer, outfile);
        sprintf(buffer, "#define %sOL %d\n", class->prefix,class->osi_oid_len);
        fputs(buffer, outfile);
	}

      if (class->num_attributes)
	    {
            fputs("\n/*", outfile);
	    sprintf(buffer,  " Attribute definitions for %s class", class->class_name);
	    fputs(buffer, outfile);
	    fputs(" */\n\n", outfile);
	    }

	attribute = class->attribute;
	for (attribute_num=0; attribute_num < class->num_attributes; attribute_num++)
	{
	  if (attribute->dna_cmip_int_len)
	    {
	    sprintf(buffer,  
		"#define %s%s_DS ATTRIBUTES_SEQ, %sDS, EOECCS, %s%s\n",
		    class->prefix_m, attribute->attribute_name,  class->prefix, 
		    class->prefix_dna, attribute->attribute_name);
	    fputs(buffer, outfile);
	    sprintf(buffer, 
		"#define %s%s_DL ATTRIBUTES_LENGTH + %sDL + 2\n",
		    class->prefix_m, attribute->attribute_name, class->prefix);
	    fputs(buffer, outfile);
	    }

	  if (attribute->oid_len)
	    {
	    sprintf(buffer, "#define %s%s_NS %s\n", class->prefix_m,
		    attribute->attribute_name, attribute->oid_text);
	    fputs(buffer, outfile);
	    sprintf(buffer, "#define %s%s_NL %d\n", class->prefix_m,
		    attribute->attribute_name, attribute->oid_len);
	    fputs(buffer, outfile);
	    }

	  if (attribute->snmp_oid_len)
	    {
	    sprintf(buffer, "#define %s%s_SS %s\n", class->prefix_m,
		    attribute->attribute_name, attribute->snmp_oid_text);
	    fputs(buffer, outfile);
	    sprintf(buffer, "#define %s%s_SL %d\n", class->prefix_m,
		    attribute->attribute_name, attribute->snmp_oid_len);
	    fputs(buffer, outfile);
	    }

	  if (attribute->osi_oid_len)
	    {
	    sprintf(buffer, "#define %s%s_OS %s\n", class->prefix_m,
		    attribute->attribute_name, attribute->osi_oid_text);
	    fputs(buffer, outfile);
	    sprintf(buffer, "#define %s%s_OL %d\n", class->prefix_m,
		    attribute->attribute_name, attribute->osi_oid_len);
	    fputs(buffer, outfile);
	    }

	  attribute = attribute->next;
	  }

	if (class->num_events)
	    {
            fputs("\n/*", outfile);
	    sprintf(buffer,  " Event definitions for %s class", class->class_name);
	    fputs(buffer, outfile);
	    fputs(" */\n\n", outfile);
	    }

	event = class->event;
	for (event_num=0; event_num < class->num_events; event_num++)
	{
	if (event->dna_cmip_int_len)
	    {
	    sprintf(buffer,
		    "#define %s%s_DS EVENT_SEQ, %sDS, EOECCS, %s%s\n",
 		    class->prefix_m, event->event_name, class->prefix,
		    class->prefix_dna, event->event_name);
	    fputs(buffer, outfile);
	    sprintf(buffer,
		    "#define %s%s_DL EVENT_LENGTH + %sDL + 2\n",
		    class->prefix_m, event->event_name, class->prefix);
	    fputs(buffer, outfile);
       	    }
	  if (event->oid_len)
	    {
	    sprintf(buffer, "#define %s%s_NS %s\n",
		    class->prefix_m, event->event_name, event->oid_text);
	    fputs(buffer, outfile);
	    sprintf(buffer, "#define %s%s_NL %d\n", 
		    class->prefix_m, event->event_name, event->oid_len);
	    fputs(buffer, outfile);
	    }

	  if (event->snmp_oid_len)
	    {
	    sprintf(buffer, "#define %s%s_SS %s\n",
		    class->prefix_m, event->event_name, event->snmp_oid_text);
	    fputs(buffer, outfile);
	    sprintf(buffer,  "#define %s%s_SL %d\n", 
		    class->prefix_m, event->event_name, event->snmp_oid_len);
	    fputs(buffer, outfile);
	    }

	  if (event->osi_oid_len)
	    {
	    sprintf(buffer, "#define %s%s_OS %s\n",
		    class->prefix_m, event->event_name, event->osi_oid_text);
	    fputs(buffer, outfile);
	    sprintf(buffer,  "#define %s%s_OL %d\n", 
		    class->prefix_m, event->event_name, event->osi_oid_len);
	    fputs(buffer, outfile);
	    }

	  insert_init_args_event_defs( event, class, mom, outfile );

	  event = event->next;
	}

	if (class->num_create_args)
	    insert_init_args_create_defs( class, mom, outfile );

	directive = class->directive;
	for (directive_num=0; directive_num < class->num_directives; directive_num++)
	{
          if (directive->type == MOMGEN_K_DIR_ACTION)
            {
   	    fputs("\n/*", outfile);
	    sprintf(buffer,  " Directive definitions for class %s action directive %s",
		class->class_name, directive->directive);
	    fputs(buffer, outfile);
	    fputs(" */\n\n", outfile);

           if (directive->dna_cmip_int_len)
	    {
	    sprintf(buffer, "#define %s%s_DS ACTIONS_SEQ, %sDS, EOECCS, %s%s\n",
		        class->prefix_m, directive->directive_name, class->prefix, 
			class->prefix_dna, directive->directive_name);
	    fputs(buffer, outfile);

	    sprintf(buffer, "#define %s%s_DL ACTIONS_LENGTH + %sDL + 2\n",
		    class->prefix_m, directive->directive_name, class->prefix);
	    fputs(buffer, outfile);	    
	    }
	   if (directive->oid_len)
	    {
	    sprintf(buffer, "#define %s%s_NS %s\n",
		    class->prefix_m, directive->directive_name, directive->oid_text);
	    fputs(buffer, outfile);
	    sprintf(buffer, "#define %s%s_NL %d\n",
		    class->prefix_m, directive->directive_name, directive->oid_len);
	    fputs(buffer, outfile);
	    }

	   if (directive->snmp_oid_len)
	    {
	    sprintf(buffer, "#define %s%s_SS %s\n",
		    class->prefix_m, directive->directive_name, directive->snmp_oid_text);
	    fputs(buffer, outfile);
	    sprintf(buffer, "#define %s%s_SL %d\n",
		    class->prefix_m, directive->directive_name, directive->snmp_oid_len);
	    fputs(buffer, outfile);
	    }

	   if (directive->osi_oid_len)
	    {
	    sprintf(buffer, "#define %s%s_OS %s\n",
		    class->prefix_m, directive->directive_name, directive->osi_oid_text);
	    fputs(buffer, outfile);
	    sprintf(buffer, "#define %s%s_OL %d\n",
		    class->prefix_m, directive->directive_name, directive->osi_oid_len);
	    fputs(buffer, outfile);
	    }

	    /* Write out all directive exceptions, responses, requests */

            if (directive->num_responses)
		{
	        fputs("\n/*", outfile);
		sprintf(buffer,  " Response definitions for class %s action directive %s",
			class->class_name, directive->directive);
		fputs(buffer, outfile);
		fputs(" */\n\n", outfile);
	
		response = directive->resp;
		for (response_num=0; response_num < directive->num_responses; response_num++)
		  {
          	  if (response->dna_cmip_int_len)
	    	    {
	    	    sprintf(buffer,
			    "#define %s%s_DS ACTION_RESP_SEQ, %sDS, EOECCS, %s%s, %s%s \n",
		    	     class->prefix_m,  response->response_name , class->prefix,
			     class->prefix_dna, directive->directive_name, 
			     class->prefix_dna, response->response_name);
		    fputs(buffer, outfile);
	    	    sprintf(buffer,
		    	    "#define %s%s_DL ACTION_RESP_LENGTH + %sDL + 3\n",
		            class->prefix_m,  response->response_name, class->prefix);
	    	    fputs(buffer, outfile);	    
		    }
	   	  if (response->oid_len)
		    {
		    sprintf(buffer, "#define %s%s_NS %s\n", class->prefix_m, 
			response->response_name, response->oid_text);
		    fputs(buffer, outfile);
		    sprintf(buffer, "#define %s%s_NL %d\n", class->prefix_m,
			response->response_name,  response->oid_len);
		    fputs(buffer, outfile);
		    }
	   	  if (response->snmp_oid_len)
		    {
		    sprintf(buffer, "#define %s%s_SS %s\n", class->prefix_m, 
			response->response_name, response->snmp_oid_text);
		    fputs(buffer, outfile);
		    sprintf(buffer, "#define %s%s_SL %d\n", class->prefix_m,
			response->response_name,  response->snmp_oid_len);
		    fputs(buffer, outfile);
		    }
	   	  if (response->osi_oid_len)
		    {
		    sprintf(buffer, "#define %s%s_OS %s\n", class->prefix_m, 
			response->response_name, response->osi_oid_text);
		    fputs(buffer, outfile);
		    sprintf(buffer, "#define %s%s_OL %d\n", class->prefix_m,
			response->response_name,  response->osi_oid_len);
		    fputs(buffer, outfile);
		    }

		  if (response->num_arguments)
			insert_init_args_resp_defs( response, directive, class, mom, outfile );

		    response = response->next;
		    }
	  	}
            if (directive->num_exceptions)
		{
	        fputs("\n/*", outfile);
		sprintf(buffer,  " Exception definitions for class %s action directive %s", 
			class->class_name, directive->directive);
		fputs(buffer, outfile);
		fputs(" */\n\n", outfile);
	
		exception = directive->exc;
		for (exception_num=0; exception_num < directive->num_exceptions; exception_num++)
		  {
          	  if (exception->dna_cmip_int_len)
	    	    {
	    	    sprintf(buffer, "#define %s%s_DS ACTION_EXCEP_SEQ, %sDS, EOECCS, %s%s, %s%s \n",
		    	     class->prefix_m, exception->exception_name , class->prefix,
			     class->prefix_dna, directive->directive_name, 
			     class->prefix_dna, exception->exception_name);
		    fputs(buffer, outfile);
	    	    sprintf(buffer,  "#define %s%s_DL ACTION_EXCEP_LENGTH + %sDL + 3\n",
		            class->prefix_m, exception->exception_name, class->prefix);
	    	    fputs(buffer, outfile);	    
		    }
	   	  if (exception->oid_len)
		    {
		    sprintf(buffer, "#define %s%s_NS %s\n", class->prefix_m,
			    exception->exception_name, exception->oid_text);
		    fputs(buffer, outfile);
		    sprintf(buffer, "#define %s%s_NL %d\n", class->prefix_m,
			    exception->exception_name, exception->oid_len);
		    fputs(buffer, outfile);
		    }
	   	  if (exception->snmp_oid_len)
		    {
		    sprintf(buffer, "#define %s%s_SS %s\n", class->prefix_m,
			    exception->exception_name, exception->snmp_oid_text);
		    fputs(buffer, outfile);
		    sprintf(buffer, "#define %s%s_SL %d\n", class->prefix_m,
			    exception->exception_name, exception->snmp_oid_len);
		    fputs(buffer, outfile);
		    }
	   	  if (exception->osi_oid_len)
		    {
		    sprintf(buffer, "#define %s%s_OS %s\n", class->prefix_m,
			    exception->exception_name, exception->osi_oid_text);
		    fputs(buffer, outfile);
		    sprintf(buffer, "#define %s%s_OL %d\n", class->prefix_m,
			    exception->exception_name, exception->osi_oid_len);
		    fputs(buffer, outfile);
		    }

		  if (exception->num_arguments)
			insert_init_args_exc_defs( exception, directive, class, mom, outfile );

		  exception = exception->next;
		  }
	  	}

            if (directive->num_requests)
		{
		request = directive->req;
		for (request_num=0; request_num < directive->num_requests; request_num++)
		    {
		    if (request->num_arguments)
			insert_init_args_req_defs( request, directive, class, mom, outfile );

		    request = request->next;
		    }
		}
	    }
          directive = directive->next;
	  }	
    class = class->next;
    }

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	check_parent
**
**	This routine checks to see if a parent class has already been
**	generated since multiple classes can have the same parent (and
**	a generated class can also be a parent.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**	outfile		File handle of the current output file.
**
**  RETURN VALUE:
**
**      1 - not generated yet.
**	0 - already generated (or will generate since it may be a class as
**	    well as a parent).
**
**--
*/
int check_parent( chk_class, mom)

CLASS_DEF *chk_class;
MOM_BUILD *mom;

{
int class_num;
CLASS_DEF *class;

/*
 * If the parent is 0 (and the class is a global entity) don't generate 
 * the parent.
 */

if (chk_class->parent_class_string_len == 0)
    return 0;

/* 
 * Check all of the classes.
 */
 
class = mom->class;
for (class_num=0; class_num < mom->num_classes; class_num++)
    {
    if (chk_class != class)
        if (strcmp( chk_class->parent, class->class_name ) == 0)
	    return 0;
    class = class->next;
    }
/*
 * Now check all of the parents.
 */
class = mom->class;
for (class_num=0; class_num < mom->num_classes; class_num++)
    {
    if (chk_class != class)
	if (strcmp( chk_class->parent, class->parent ) == 0)
           if (class->parent_generated)
		return 0;		
	   else
		return 1;
    class = class->next;
    }
return 1;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_init_imports
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on tag "insert-code-init-imports".
**
**	This routine generates the IMPORT statements for the Object IDs (OIDs) 
**	of the classes, attributes and arguments.
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
int insert_init_imports( mom,
		         outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    char		buffer[256];
    int			length;
    int			class_num;
    CLASS_DEF		*class;
    int			attribute_num;
    ATTRIBUTE_DEF	*attribute;
    int			directive_num;
    DIRECTIVE_DEF	*directive;
    int			argument_num;
    ARGUMENT_DEF	*argument;
    int			request_num;
    REQUEST_DEF		*request;
    int			response_num;
    RESPONSE_DEF	*response;
    int			exception_num;
    EXCEPTION_DEF	*exception;
    int			event_num;
    int			first = 1;
    EVENT_DEF		*event;
    
    if (!mom->OID_SNMP) {
	fputs("IMPORT object_id NODE_ATTR_NAME_DNA;\n", outfile);
        fputs("IMPORT object_id UNIV_ATTR_DNA;\n", outfile);
        fputs("IMPORT object_id UNIV_ALL_ATTR_DNA;\n", outfile);
        fputs("IMPORT object_id UNIV_ALL_ID_ATTR_DNA;\n", outfile);
        fputs("IMPORT object_id UNIV_ALL_STATUS_ATTR_DNA;\n", outfile);
        fputs("IMPORT object_id UNIV_ALL_CHAR_ATTR_DNA;\n", outfile);
        fputs("IMPORT object_id UNIV_ALL_COUNTER_ATTR_DNA;\n", outfile);
    }
    else 
        fputs("IMPORT object_id SNMP_NULL_OID;\n", outfile);

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
      {
      fputs("\n/*", outfile);
      sprintf(buffer, " OID imports for %s class", class->class_name);
      fputs(buffer, outfile);
      fputs(" */\n\n", outfile);

      if (class->dna_cmip_int_len)
	{
        sprintf(buffer, "IMPORT object_id %s_DNA; \n", class->class_name);
        fputs(buffer, outfile);
	}
      if (class->oid_len)
	{
        sprintf(buffer, "IMPORT object_id %s_OID; \n", class->class_name);
        fputs(buffer, outfile);
	}
      if (class->snmp_oid_len)
	{
        sprintf(buffer, "IMPORT object_id %s_SNP; \n", class->class_name);
        fputs(buffer, outfile);
	}
      if (class->osi_oid_len)
	{
        sprintf(buffer, "IMPORT object_id %s_OSI; \n", class->class_name);
        fputs(buffer, outfile);
	}

      sprintf(buffer, "IMPORT object_id %sattr_prefix_DNA; \n", class->prefix_m );
      fputs(buffer, outfile);

	attribute = class->attribute;
	for (attribute_num=0; attribute_num < class->num_attributes; attribute_num++)
	  {
	  if (attribute->dna_cmip_int_len)
	    {
	    sprintf(buffer, "IMPORT object_id %s%s_DNA; \n", class->prefix_m, attribute->attribute_name);
	    fputs(buffer, outfile);
	    }
	  if (attribute->oid_len)
	    {
	    sprintf(buffer, "IMPORT object_id %s%s_OID; \n", class->prefix_m, attribute->attribute_name);
	    fputs(buffer, outfile);
	    }
	  if (attribute->snmp_oid_len)
	    {
	    sprintf(buffer, "IMPORT object_id %s%s_SNP; \n", class->prefix_m, attribute->attribute_name);
	    fputs(buffer, outfile);
	    }
	  if (attribute->osi_oid_len)
	    {
	    sprintf(buffer, "IMPORT object_id %s%s_OSI; \n", class->prefix_m, attribute->attribute_name);
	    fputs(buffer, outfile);
	    }
	
	  attribute = attribute->next;
	  }

	event = class->event;
	for (event_num=0; event_num < class->num_events; event_num++)
	{
	  if (event->dna_cmip_int_len)
	    {
	    sprintf(buffer, "IMPORT object_id %s%s_DNA; \n", class->prefix_m, event->event_name);
	    fputs(buffer, outfile);
	    }
	  if (event->oid_len)
	    {
	    sprintf(buffer, "IMPORT object_id %s%s_OID; \n", class->prefix_m, event->event_name);
	    fputs(buffer, outfile);
	    }
	  if (event->snmp_oid_len)
	    {
	    sprintf(buffer, "IMPORT object_id %s%s_SNP; \n", class->prefix_m, event->event_name);
	    fputs(buffer, outfile);
	    }
	  if (event->osi_oid_len)
	    {
	    sprintf(buffer, "IMPORT object_id %s%s_OSI; \n", class->prefix_m, event->event_name);
	    fputs(buffer, outfile);
	    }

	  insert_init_args_event_imports( event, class, mom, outfile );

	  event = event->next;
	}

	if (class->num_create_args)
	    insert_init_args_create_imports( class, mom, outfile );

	directive = class->directive;
	for (directive_num=0; directive_num < class->num_directives; directive_num++)
	{
          if (directive->type == MOMGEN_K_DIR_ACTION)
            {
	    if (directive->dna_cmip_int_len)
	    	{
		sprintf(buffer, "IMPORT object_id %s%s_DNA; \n",
		    class->prefix_m, directive->directive_name);
	    	fputs(buffer, outfile);
            	}
	    if (directive->oid_len)
	    	{
		sprintf(buffer, "IMPORT object_id %s%s_OID; \n",
		    class->prefix_m, directive->directive_name);
	    	fputs(buffer, outfile);
            	}
	    if (directive->snmp_oid_len)
	    	{
		sprintf(buffer, "IMPORT object_id %s%s_SNP; \n",
		    class->prefix_m, directive->directive_name);
	    	fputs(buffer, outfile);
            	}
	    if (directive->osi_oid_len)
	    	{
		sprintf(buffer, "IMPORT object_id %s%s_OSI; \n",
		    class->prefix_m, directive->directive_name);
	    	fputs(buffer, outfile);
            	}

	    /* Write out all directive exceptions, responses, requests */

            if (directive->num_responses)
		{
		response = directive->resp;
		for (response_num=0; response_num < directive->num_responses; response_num++)
		    {
		    if (response->dna_cmip_int_len)
		    	{
			sprintf(buffer, "IMPORT object_id %s%s_DNA; \n",
			    	class->prefix_m, response->response_name);
		    	fputs(buffer, outfile);
			}
		    if (response->oid_len)
		    	{
			sprintf(buffer, "IMPORT object_id %s%s_OID; \n",
			    	class->prefix_m, response->response_name);
		    	fputs(buffer, outfile);
			}
		    if (response->snmp_oid_len)
		    	{
			sprintf(buffer, "IMPORT object_id %s%s_SNP; \n",
			    	class->prefix_m, response->response_name);
		    	fputs(buffer, outfile);
			}
		    if (response->osi_oid_len)
		    	{
			sprintf(buffer, "IMPORT object_id %s%s_OSI; \n",
			    	class->prefix_m, response->response_name);
		    	fputs(buffer, outfile);
			}

                    if (response->num_arguments)
			insert_init_args_resp_imports( response, directive, class, mom, outfile );

		    response = response->next;
		    }

	  	}
            if (directive->num_exceptions)
		{
		exception = directive->exc;
		for (exception_num=0; exception_num < directive->num_exceptions; exception_num++)
		    {
		    if (exception->dna_cmip_int_len)
		    	{
			sprintf(buffer, "IMPORT object_id %s%s_DNA; \n",
			    	class->prefix_m, exception->exception_name);
		    	fputs(buffer, outfile);
			}
		    if (exception->oid_len)
		    	{
			sprintf(buffer, "IMPORT object_id %s%s_OID; \n",
			    	class->prefix_m, exception->exception_name);
		    	fputs(buffer, outfile);
			}
		    if (exception->snmp_oid_len)
		    	{
			sprintf(buffer, "IMPORT object_id %s%s_SNP; \n",
			    	class->prefix_m, exception->exception_name);
		    	fputs(buffer, outfile);
			}
		    if (exception->osi_oid_len)
		    	{
			sprintf(buffer, "IMPORT object_id %s%s_OSI; \n",
			    	class->prefix_m, exception->exception_name);
		    	fputs(buffer, outfile);
			}

		    if (exception->num_arguments)
			insert_init_args_exc_imports( exception, directive, class, mom, outfile );

		    exception = exception->next;
		    }
	  	}

            if (directive->num_requests)
		{
		request = directive->req;
		for (request_num=0; request_num < directive->num_requests; request_num++)
		    {
		    if (request->num_arguments)
			insert_init_args_req_imports( request, directive, class, mom, outfile );

		    request = request->next;
		    }
		}
	    }
          directive = directive->next;
	  }	
    class = class->next;
    }

    /* IMPORT the NULL OID for instanceless classes */

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
	{
	if (!class->support_instances)
	    {
            if (first && !mom->OID_SNMP)
		{
    		fputs("IMPORT object_id NULL_ID_ATTRIBUTE_DNA;\n", outfile);
		first = 0;
		}	
       	    if (class->dna_cmip_int_len)
		{
        	sprintf(buffer,  "IMPORT struct object_id %sNULL_DNA;\n", 
			class->prefix, class->prefix, class->prefix);
        	fputs(buffer, outfile); 
		}      
	    if (class->snmp_oid_len)
		{
        	sprintf(buffer,  "IMPORT struct object_id %sNULL_SNP;\n", 
			class->prefix, class->prefix, class->prefix);
        	fputs(buffer, outfile); 
		}      
	    if (class->oid_len)
		{
        	sprintf(buffer,  "IMPORT struct object_id %sNULL_OID;\n", 
			class->prefix, class->prefix, class->prefix);
        	fputs(buffer, outfile); 
		}      

	    /*** IS THERE A NULL OID FOR OSI??  ***/

	    }
        class = class->next;
        }


    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_init_args_event_defs
**
**	This routine generates the event arguments for a specific class.
**
**  FORMAL PARAMETERS:
**
**	event 		Pointer to event to generate.
**	class		Pointer to current class.
**      mom		Pointer to the MOM description block.
**	outfile		File handle of the current output file.
**
**  RETURN VALUE:
**
**      None
**
**--
*/
void	insert_init_args_event_defs( event,
		                     class, 
		                     mom, 
		                     outfile )

EVENT_DEF *event;
CLASS_DEF *class;
MOM_BUILD *mom;
FILE *outfile;

{
char		buffer[256];
int argument_num;
ARGUMENT_DEF *argument;

if (event->num_event_args)
    {
    fputs("\n/*", outfile);
    sprintf(buffer,
   	" Argument definitions for class %s event %s",
	class->class_name,
	event->event_name);
    fputs(buffer, outfile);
    fputs(" */\n\n", outfile);
    }

argument = event->event_arg;
for (argument_num=0; argument_num < event->num_event_args; argument_num++)
    {
    if (argument->dna_cmip_int_len)
	{
   	sprintf(buffer, "#define %s%s_DS %s, %sDS, EOECCS, \\\n", 
		class->prefix_m, argument->argument_name,
	        argument->argument_type, class->prefix);
        fputs(buffer, outfile);
   	sprintf(buffer, "            %s%s, %s%s \n",
		class->prefix_dna, event->event_name, 
		class->prefix_dna, argument->argument_name);
        fputs(buffer, outfile);

	sprintf(buffer, "#define %s%s_DL %s + %sDL + 3\n",
		class->prefix_m, argument->argument_name, 
	        argument->argument_length, class->prefix);
	fputs(buffer, outfile);	    
	}
    if (argument->oid_len)
	{
	sprintf(buffer, "#define %s%s_NS %s\n",
		class->prefix_m, argument->argument_name, 
		argument->oid_text);
	fputs(buffer, outfile);
	sprintf(buffer, "#define %s%s_NL %d\n",
		class->prefix_m, argument->argument_name,
		argument->oid_len);
	fputs(buffer, outfile);
        }
    if (argument->snmp_oid_len)
	{
	sprintf(buffer, "#define %s%s_SS %s\n",
		class->prefix_m, argument->argument_name, 
		argument->snmp_oid_text);
	fputs(buffer, outfile);
	sprintf(buffer, "#define %s%s_SL %d\n",
		class->prefix_m, argument->argument_name,
		argument->snmp_oid_len);
	fputs(buffer, outfile);
        }
    if (argument->osi_oid_len)
	{
	sprintf(buffer, "#define %s%s_OS %s\n",
		class->prefix_m, argument->argument_name, 
		argument->osi_oid_text);
	fputs(buffer, outfile);
	sprintf(buffer, "#define %s%s_OL %d\n",
		class->prefix_m, argument->argument_name,
		argument->osi_oid_len);
	fputs(buffer, outfile);
        }

    argument = argument->next;
   }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_init_args_req_defs
**
**	This routine generates the request arguments for a specific directive.
**
**  FORMAL PARAMETERS:
**
**	request		Pointer to request to generate.
**	directive	Pointer to directive to generate.
**	class		Pointer to current class.
**      mom		Pointer to the MOM description block.
**	outfile		File handle of the current output file.
**
**  RETURN VALUE:
**
**      None
**
**--
*/
void	insert_init_args_req_defs( request,
		                   directive, 
		                   class, 
   		                   mom, 
		                   outfile )

REQUEST_DEF *request;
DIRECTIVE_DEF *directive;
CLASS_DEF *class;
MOM_BUILD *mom;
FILE *outfile;

{
char		buffer[256];
int argument_num;
ARGUMENT_DEF *argument;

if (request->num_arguments)
    {
    fputs("\n/*", outfile);
    sprintf(buffer,
   	" Argument definitions for class %s directive %s request",
	class->class_name,
	directive->directive);
    fputs(buffer, outfile);
    fputs(" */\n\n", outfile);
    }

argument = request->arg;
for (argument_num=0; argument_num < request->num_arguments; argument_num++)
    {
    if (argument->dna_cmip_int_len)
	{
   	sprintf(buffer, "#define %s%s_DS %s, %sDS, EOECCS, %s%s, %s%s\n", 
		class->prefix_m, argument->argument_name, 
		argument->argument_type, class->prefix, 
		class->prefix_dna, directive->directive_name,  
		class->prefix_dna, argument->argument_name);
        fputs(buffer, outfile);

	sprintf(buffer, "#define %s%s_DL %s + %sDL + 3\n", 
		class->prefix_m, argument->argument_name,
	        argument->argument_length, class->prefix);
	fputs(buffer, outfile);	    
	}

    if (argument->oid_len)
	{
	sprintf(buffer, "#define %s%s_NS %s\n",
		class->prefix_m, argument->argument_name, 
		argument->oid_text);
	fputs(buffer, outfile);
	sprintf(buffer, "#define %s%s_NL %d\n",
		class->prefix_m, argument->argument_name,
		argument->oid_len);
	fputs(buffer, outfile);
        }
    if (argument->snmp_oid_len)
	{
	sprintf(buffer, "#define %s%s_SS %s\n",
		class->prefix_m, argument->argument_name, 
		argument->snmp_oid_text);
	fputs(buffer, outfile);
	sprintf(buffer, "#define %s%s_SL %d\n",
		class->prefix_m, argument->argument_name,
		argument->snmp_oid_len);
	fputs(buffer, outfile);
        }
    if (argument->osi_oid_len)
	{
	sprintf(buffer, "#define %s%s_OS %s\n",
		class->prefix_m, argument->argument_name, 
		argument->osi_oid_text);
	fputs(buffer, outfile);
	sprintf(buffer, "#define %s%s_OL %d\n",
		class->prefix_m, argument->argument_name,
		argument->osi_oid_len);
	fputs(buffer, outfile);
        }

    argument = argument->next;
   }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_init_args_resp_defs
**
**	This routine generates the response arguments for a specific directive.
**
**  FORMAL PARAMETERS:
**
**	response	Pointer to current response.
**	directive	Pointer to directive to generate.
**	class		Pointer to current class.
**      mom		Pointer to the MOM description block.
**	outfile		File handle of the current output file.
**
**  RETURN VALUE:
**
**      None
**
**--
*/
void	insert_init_args_resp_defs( response,
		                    directive, 
		                    class, 
		                    mom, 
		                    outfile )

RESPONSE_DEF *response;
DIRECTIVE_DEF *directive;
CLASS_DEF *class;
MOM_BUILD *mom;
FILE *outfile;

{
char		buffer[256];
int argument_num;
ARGUMENT_DEF *argument;

if (response->num_arguments)
    {
    fputs("\n/*", outfile);
    sprintf(buffer,
        " Argument definitions for class %s directive %s response %s",
    	class->class_name,
	directive->directive,
	response->name);
    fputs(buffer, outfile);
    fputs(" */\n\n", outfile);
    }

argument = response->arg;
for (argument_num=0; argument_num < response->num_arguments; argument_num++)
   {
    if (argument->dna_cmip_int_len)
	{
   	sprintf(buffer, "#define %s%s_DS %s, %sDS, EOECCS, %s%s, %s%s, %s%s\n", 
		class->prefix_m, argument->argument_name, 
		argument->argument_type, class->prefix, 
		class->prefix_dna, directive->directive_name, 
		class->prefix_dna, response->response_name, 
		class->prefix_dna, argument->argument_name);
        fputs(buffer, outfile);

	sprintf(buffer, "#define %s%s_DL %s + %sDL + 4\n",
		class->prefix_m, argument->argument_name,
	        argument->argument_length, class->prefix);
	fputs(buffer, outfile);	    
	}

    if (argument->oid_len)
	{
	sprintf(buffer, "#define %s%s_NS %s\n",
		class->prefix_m, argument->argument_name, 
		argument->oid_text);
	fputs(buffer, outfile);
	sprintf(buffer, "#define %s%s_NL %d\n",
		class->prefix_m, argument->argument_name,
		argument->oid_len);
	fputs(buffer, outfile);
        }
    if (argument->snmp_oid_len)
	{
	sprintf(buffer, "#define %s%s_SS %s\n",
		class->prefix_m, argument->argument_name, 
		argument->snmp_oid_text);
	fputs(buffer, outfile);
	sprintf(buffer, "#define %s%s_SL %d\n",
		class->prefix_m, argument->argument_name,
		argument->snmp_oid_len);
	fputs(buffer, outfile);
        }
    if (argument->osi_oid_len)
	{
	sprintf(buffer, "#define %s%s_OS %s\n",
		class->prefix_m, argument->argument_name, 
		argument->osi_oid_text);
	fputs(buffer, outfile);
	sprintf(buffer, "#define %s%s_OL %d\n",
		class->prefix_m, argument->argument_name,
		argument->osi_oid_len);
	fputs(buffer, outfile);
        }

    argument = argument->next;
    }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_init_args_create_defs
**
**	This routine generates the response arguments for a specific directive.
**
**  FORMAL PARAMETERS:
**
**	class		Pointer to current class.
**      mom		Pointer to the MOM description block.
**	outfile		File handle of the current output file.
**
**  RETURN VALUE:
**
**      None
**
**--
*/
void	insert_init_args_create_defs( class, 
		                      mom, 
		                      outfile )

CLASS_DEF *class;
MOM_BUILD *mom;
FILE *outfile;

{
char		buffer[256];
int argument_num;
ARGUMENT_DEF *argument;

if (class->num_create_args)
    {
    fputs("\n/*", outfile);
    sprintf(buffer,
    	" Argument definitions for class %s directive CREATE", class->class_name );
    fputs(buffer, outfile);
    fputs(" */\n\n", outfile);
    }

argument = class->create_arg;
for (argument_num=0; argument_num < class->num_create_args; argument_num++)
   {
    if (argument->dna_cmip_int_len)
	{
   	sprintf(buffer, "#define %s%s_DS ACTION_REQ_ARG_SEQ, %sDS, EOECCS, %sDIR_CREATE, %s%s\n", 
		class->prefix_m, argument->argument_name, class->prefix,
		class->prefix_dna, 
		class->prefix_dna, argument->argument_name);
        fputs(buffer, outfile);

	sprintf(buffer, "#define %s%s_DL %s + %sDL + 3\n",
		class->prefix_m, argument->argument_name,
	        argument->argument_length, class->prefix);
	fputs(buffer, outfile);	    
	}
    if (argument->oid_len)
	{
	sprintf(buffer, "#define %s%s_NS %s\n",
		class->prefix_m, argument->argument_name, 
		argument->oid_text);
	fputs(buffer, outfile);
	sprintf(buffer, "#define %s%s_NL %d\n",
		class->prefix_m, argument->argument_name,
		argument->oid_len);
	fputs(buffer, outfile);
        }
    if (argument->snmp_oid_len)
	{
	sprintf(buffer, "#define %s%s_SS %s\n",
		class->prefix_m, argument->argument_name, 
		argument->snmp_oid_text);
	fputs(buffer, outfile);
	sprintf(buffer, "#define %s%s_SL %d\n",
		class->prefix_m, argument->argument_name,
		argument->snmp_oid_len);
	fputs(buffer, outfile);
        }
    if (argument->osi_oid_len)
	{
	sprintf(buffer, "#define %s%s_OS %s\n",
		class->prefix_m, argument->argument_name, 
		argument->osi_oid_text);
	fputs(buffer, outfile);
	sprintf(buffer, "#define %s%s_OL %d\n",
		class->prefix_m, argument->argument_name,
		argument->osi_oid_len);
	fputs(buffer, outfile);
        }

    argument = argument->next;
    }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_init_args_exc_defs
**
**	This routine generates the exception arguments for a specific directive.
**
**  FORMAL PARAMETERS:
**
**	exception  	Pointer to current exception.
**	directive	Pointer to directive to generate.
**	class		Pointer to current class.
**      mom		Pointer to the MOM description block.
**	outfile		File handle of the current output file.
**
**  RETURN VALUE:
**
**      None
**
**--
*/
void	insert_init_args_exc_defs( exception, 
		                   directive, 
		                   class, 
		                   mom, 
		                   outfile )

EXCEPTION_DEF *exception;
DIRECTIVE_DEF *directive;
CLASS_DEF *class;
MOM_BUILD *mom;
FILE *outfile;

{
char		buffer[256];
int argument_num;
ARGUMENT_DEF *argument;

if (exception->num_arguments)
    {
    fputs("\n/*", outfile);
    sprintf(buffer,
    	" Argument definitions for class %s directive %s exception %s",
	class->class_name,
	directive->directive,
	exception->name);
    fputs(buffer, outfile);
    fputs(" */\n\n", outfile);
    }

argument = exception->arg;
for (argument_num=0; argument_num < exception->num_arguments; argument_num++)
   {
    if (argument->dna_cmip_int_len)
	{
   	sprintf(buffer, "#define %s%s_DS %s, %sDS, EOECCS, %s%s, %s%s, %s%s\n", 
		class->prefix_m, argument->argument_name, argument->argument_type,
		class->prefix, 
		class->prefix_dna, directive->directive_name, 
		class->prefix_dna, exception->exception_name, 
		class->prefix_dna, argument->argument_name);
        fputs(buffer, outfile);

	sprintf(buffer, "#define %s%s_DL %s + %sDL + 4\n",
		class->prefix_m, argument->argument_name,
		argument->argument_length, class->prefix);
	fputs(buffer, outfile);	    
	}

    if (argument->oid_len)
	{
	sprintf(buffer, "#define %s%s_NS %s\n",
		class->prefix_m, argument->argument_name, 
		argument->oid_text);
	fputs(buffer, outfile);
	sprintf(buffer, "#define %s%s_NL %d\n",
		class->prefix_m, argument->argument_name,
		argument->oid_len);
	fputs(buffer, outfile);
        }
    if (argument->snmp_oid_len)
	{
	sprintf(buffer, "#define %s%s_SS %s\n",
		class->prefix_m, argument->argument_name, 
		argument->snmp_oid_text);
	fputs(buffer, outfile);
	sprintf(buffer, "#define %s%s_SL %d\n",
		class->prefix_m, argument->argument_name,
		argument->snmp_oid_len);
	fputs(buffer, outfile);
        }
    if (argument->osi_oid_len)
	{
	sprintf(buffer, "#define %s%s_OS %s\n",
		class->prefix_m, argument->argument_name, 
		argument->osi_oid_text);
	fputs(buffer, outfile);
	sprintf(buffer, "#define %s%s_OL %d\n",
		class->prefix_m, argument->argument_name,
		argument->osi_oid_len);
	fputs(buffer, outfile);
        }

    argument = argument->next;
    }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_init_args_event
**
**	This routine generates the event arguments for a specific class.
**
**  FORMAL PARAMETERS:
**
**	event 		Pointer to event to generate.
**	class		Pointer to current class.
**      mom		Pointer to the MOM description block.
**	outfile		File handle of the current output file.
**
**  RETURN VALUE:
**
**      None
**
**--
*/
void	insert_init_args_event( event,
		                class, 
		                mom, 
		                outfile )

EVENT_DEF *event;
CLASS_DEF *class;
MOM_BUILD *mom;
FILE *outfile;

{
char		buffer[256];
int argument_num;
ARGUMENT_DEF *argument;

if (event->num_event_args)
    {
    fputs("\n/*", outfile);
    sprintf(buffer, " Argument data for class %s event %s",
	class->class_name, event->event_name);
    fputs(buffer, outfile);
    fputs(" */\n\n", outfile);
    }

argument = event->event_arg;
for (argument_num=0; argument_num < event->num_event_args; argument_num++)
  {
  if (argument->dna_cmip_int_len)
    {
    sprintf(buffer,  "static unsigned int %s%s_DA[] = {%s%s_DS};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    sprintf(buffer, "EXPORT struct object_id %s%s_DNA = {%s%s_DL, %s%s_DA};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    }
  if (argument->oid_len)
    {
    sprintf(buffer,  "static unsigned int %s%s_NA[] = {%s%s_NS};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    sprintf(buffer, "EXPORT struct object_id %s%s_OID = {%s%s_NL, %s%s_NA};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    }
  if (argument->snmp_oid_len)
    {
    sprintf(buffer,  "static unsigned int %s%s_SA[] = {%s%s_SS};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    sprintf(buffer, "EXPORT struct object_id %s%s_SNP = {%s%s_SL, %s%s_SA};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    }
  if (argument->osi_oid_len)
    {
    sprintf(buffer,  "static unsigned int %s%s_OA[] = {%s%s_OS};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    sprintf(buffer, "EXPORT struct object_id %s%s_OSI = {%s%s_OL, %s%s_OA};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    }

   argument = argument->next;
   }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_init_args_req
**
**	This routine generates the request arguments for a specific directive.
**
**  FORMAL PARAMETERS:
**
**	request		Pointer to request to generate.
**	directive	Pointer to directive to generate.
**	class		Pointer to current class.
**      mom		Pointer to the MOM description block.
**	outfile		File handle of the current output file.
**
**  RETURN VALUE:
**
**      None
**
**--
*/
void	insert_init_args_req( request,
		              directive, 
		              class, 
		              mom, 
		              outfile )

REQUEST_DEF *request;
DIRECTIVE_DEF *directive;
CLASS_DEF *class;
MOM_BUILD *mom;
FILE *outfile;

{
char		buffer[256];
int argument_num;
ARGUMENT_DEF *argument;

fputs("\n/*", outfile);
sprintf(buffer, " Argument data for class %s directive %s request",
	class->class_name, directive->directive);
fputs(buffer, outfile);
fputs(" */\n\n", outfile);

argument = request->arg;
for (argument_num=0; argument_num < request->num_arguments; argument_num++)
    {
  if (argument->dna_cmip_int_len)
    {
    sprintf(buffer,  "static unsigned int %s%s_DA[] = {%s%s_DS};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    sprintf(buffer, "EXPORT struct object_id %s%s_DNA = {%s%s_DL, %s%s_DA};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    }

  if (argument->oid_len)
    {
    sprintf(buffer,  "static unsigned int %s%s_NA[] = {%s%s_NS};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    sprintf(buffer, "EXPORT struct object_id %s%s_OID = {%s%s_NL, %s%s_NA};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    }
  if (argument->snmp_oid_len)
    {
    sprintf(buffer,  "static unsigned int %s%s_SA[] = {%s%s_SS};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    sprintf(buffer, "EXPORT struct object_id %s%s_SNP = {%s%s_SL, %s%s_SA};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    }
  if (argument->osi_oid_len)
    {
    sprintf(buffer,  "static unsigned int %s%s_OA[] = {%s%s_OS};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    sprintf(buffer, "EXPORT struct object_id %s%s_OSI = {%s%s_OL, %s%s_OA};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    }

   argument = argument->next;
   }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_init_args_resp
**
**	This routine generates the response arguments for a specific directive.
**
**  FORMAL PARAMETERS:
**
**	response	Pointer to current response.
**	directive	Pointer to directive to generate.
**	class		Pointer to current class.
**      mom		Pointer to the MOM description block.
**	outfile		File handle of the current output file.
**
**  RETURN VALUE:
**
**      None
**
**--
*/
void	insert_init_args_resp( response,
		               directive, 
		               class, 
		               mom, 
		               outfile )

RESPONSE_DEF *response;
DIRECTIVE_DEF *directive;
CLASS_DEF *class;
MOM_BUILD *mom;
FILE *outfile;

{
char		buffer[256];
int argument_num;
ARGUMENT_DEF *argument;

fputs("\n/*", outfile);
sprintf(buffer,
    	" Argument data for class %s directive %s response %s",
	class->class_name,
	directive->directive,
	response->name);
fputs(buffer, outfile);
fputs(" */\n\n", outfile);

argument = response->arg;
for (argument_num=0; argument_num < response->num_arguments; argument_num++)
  {
  if (argument->dna_cmip_int_len)
    {
    sprintf(buffer,  "static unsigned int %s%s_DA[] = {%s%s_DS};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    sprintf(buffer, "EXPORT struct object_id %s%s_DNA = {%s%s_DL, %s%s_DA};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    }

  if (argument->oid_len)
    {
    sprintf(buffer,  "static unsigned int %s%s_NA[] = {%s%s_NS};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    sprintf(buffer, "EXPORT struct object_id %s%s_OID = {%s%s_NL, %s%s_NA};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    }
  if (argument->snmp_oid_len)
    {
    sprintf(buffer,  "static unsigned int %s%s_SA[] = {%s%s_SS};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    sprintf(buffer, "EXPORT struct object_id %s%s_SNP = {%s%s_SL, %s%s_SA};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    }
  if (argument->osi_oid_len)
    {
    sprintf(buffer,  "static unsigned int %s%s_OA[] = {%s%s_OS};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    sprintf(buffer, "EXPORT struct object_id %s%s_OSI = {%s%s_OL, %s%s_OA};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    }

   argument = argument->next;
   }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_init_args_create
**
**	This routine generates the response arguments for a specific directive.
**
**  FORMAL PARAMETERS:
**
**	class		Pointer to current class.
**      mom		Pointer to the MOM description block.
**	outfile		File handle of the current output file.
**
**  RETURN VALUE:
**
**      None
**
**--
*/
void	insert_init_args_create( class, 
		                 mom, 
		                 outfile )

CLASS_DEF *class;
MOM_BUILD *mom;
FILE *outfile;

{
char		buffer[256];
int argument_num;
ARGUMENT_DEF *argument;

fputs("\n/*", outfile);
sprintf(buffer,
    	" Argument data for class %s directive CREATE", class->class_name );
fputs(buffer, outfile);
fputs(" */\n\n", outfile);

argument = class->create_arg;
for (argument_num=0; argument_num < class->num_create_args; argument_num++)
  {
  if (argument->dna_cmip_int_len)
    {
    sprintf(buffer,  "static unsigned int %s%s_DA[] = {%s%s_DS};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    sprintf(buffer, "EXPORT struct object_id %s%s_DNA = {%s%s_DL, %s%s_DA};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    }

  if (argument->oid_len)
    {
    sprintf(buffer,  "static unsigned int %s%s_NA[] = {%s%s_NS};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    sprintf(buffer, "EXPORT struct object_id %s%s_OID = {%s%s_NL, %s%s_NA};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    }
  if (argument->snmp_oid_len)
    {
    sprintf(buffer,  "static unsigned int %s%s_SA[] = {%s%s_SS};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    sprintf(buffer, "EXPORT struct object_id %s%s_SNP = {%s%s_SL, %s%s_SA};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    }
  if (argument->osi_oid_len)
    {
    sprintf(buffer,  "static unsigned int %s%s_OA[] = {%s%s_OS};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    sprintf(buffer, "EXPORT struct object_id %s%s_OSI = {%s%s_OL, %s%s_OA};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    }

   argument = argument->next;
   }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_init_args_exc
**
**	This routine generates the exception arguments for a specific directive.
**
**  FORMAL PARAMETERS:
**
**	exception  	Pointer to current exception.
**	directive	Pointer to directive to generate.
**	class		Pointer to current class.
**      mom		Pointer to the MOM description block.
**	outfile		File handle of the current output file.
**
**  RETURN VALUE:
**
**      None
**
**--
*/
void	insert_init_args_exc( exception, 
		              directive, 
		              class, 
		              mom, 
		              outfile )

EXCEPTION_DEF *exception;
DIRECTIVE_DEF *directive;
CLASS_DEF *class;
MOM_BUILD *mom;
FILE *outfile;

{
char		buffer[256];
int argument_num;
ARGUMENT_DEF *argument;

fputs("\n/*", outfile);
sprintf(buffer,
    	" Argument data for class %s directive %s exception %s",
	class->class_name,
	directive->directive,
	exception->name);
fputs(buffer, outfile);
fputs(" */\n\n", outfile);

argument = exception->arg;
for (argument_num=0; argument_num < exception->num_arguments; argument_num++)
  {
  if (argument->dna_cmip_int_len)
    {
    sprintf(buffer,  "static unsigned int %s%s_DA[] = {%s%s_DS};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    sprintf(buffer, "EXPORT struct object_id %s%s_DNA = {%s%s_DL, %s%s_DA};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    }

  if (argument->oid_len)
    {
    sprintf(buffer,  "static unsigned int %s%s_NA[] = {%s%s_NS};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    sprintf(buffer, "EXPORT struct object_id %s%s_OID = {%s%s_NL, %s%s_NA};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    }
  if (argument->snmp_oid_len)
    {
    sprintf(buffer,  "static unsigned int %s%s_SA[] = {%s%s_SS};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    sprintf(buffer, "EXPORT struct object_id %s%s_SNP = {%s%s_SL, %s%s_SA};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    }
  if (argument->osi_oid_len)
    {
    sprintf(buffer,  "static unsigned int %s%s_OA[] = {%s%s_OS};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    sprintf(buffer, "EXPORT struct object_id %s%s_OSI = {%s%s_OL, %s%s_OA};\n",
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name,
	  class->prefix_m, argument->argument_name);
    fputs(buffer, outfile);
    }

   argument = argument->next;
   }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_init_args_event_imports
**
**	This routine generates the event arguments for a specific class.
**
**  FORMAL PARAMETERS:
**
**	event 		Pointer to event to generate.
**	class		Pointer to current class.
**      mom		Pointer to the MOM description block.
**	outfile		File handle of the current output file.
**
**  RETURN VALUE:
**
**      None
**
**--
*/
void	insert_init_args_event_imports( event,
		         	        class, 
		        	        mom, 
		        	        outfile )

EVENT_DEF *event;
CLASS_DEF *class;
MOM_BUILD *mom;
FILE *outfile;

{
char		buffer[256];
int argument_num;
ARGUMENT_DEF *argument;

argument = event->event_arg;
for (argument_num=0; argument_num < event->num_event_args; argument_num++)
    {
    if (argument->dna_cmip_int_len)
	{
	sprintf(buffer, "IMPORT object_id %s%s_DNA; \n",
	    class->prefix_m, argument->argument_name);
	fputs(buffer, outfile);
	}
    if (argument->oid_len)
	{
	sprintf(buffer, "IMPORT object_id %s%s_OID; \n",
	    class->prefix_m, argument->argument_name);
	fputs(buffer, outfile);
	}
    if (argument->snmp_oid_len)
	{
	sprintf(buffer, "IMPORT object_id %s%s_SNP; \n",
	    class->prefix_m, argument->argument_name);
	fputs(buffer, outfile);
	}
    if (argument->osi_oid_len)
	{
	sprintf(buffer, "IMPORT object_id %s%s_OSI; \n",
	    class->prefix_m, argument->argument_name);
	fputs(buffer, outfile);
	}

   argument = argument->next;
   }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_init_args_req_imports
**
**	This routine generates the request arguments for a specific directive.
**
**  FORMAL PARAMETERS:
**
**	request		Pointer to request to generate.
**	directive	Pointer to directive to generate.
**	class		Pointer to current class.
**      mom		Pointer to the MOM description block.
**	outfile		File handle of the current output file.
**
**  RETURN VALUE:
**
**      None
**
**--
*/
void	insert_init_args_req_imports( request,
		      	              directive, 
		      	              class, 
		      	              mom, 
		      	              outfile )

REQUEST_DEF *request;
DIRECTIVE_DEF *directive;
CLASS_DEF *class;
MOM_BUILD *mom;
FILE *outfile;

{
char		buffer[256];
int argument_num;
ARGUMENT_DEF *argument;

argument = request->arg;
for (argument_num=0; argument_num < request->num_arguments; argument_num++)
    {
    if (argument->dna_cmip_int_len)
	{
	sprintf(buffer, "IMPORT object_id %s%s_DNA; \n",
	    class->prefix_m, argument->argument_name);
	fputs(buffer, outfile);
	}
    if (argument->oid_len)
	{
	sprintf(buffer, "IMPORT object_id %s%s_OID; \n",
	    class->prefix_m, argument->argument_name);
	fputs(buffer, outfile);
	}
    if (argument->snmp_oid_len)
	{
	sprintf(buffer, "IMPORT object_id %s%s_SNP; \n",
	    class->prefix_m, argument->argument_name);
	fputs(buffer, outfile);
	}
    if (argument->osi_oid_len)
	{
	sprintf(buffer, "IMPORT object_id %s%s_OSI; \n",
	    class->prefix_m, argument->argument_name);
	fputs(buffer, outfile);
	}

   argument = argument->next;
   }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_init_args_resp_imports
**
**	This routine generates the response arguments for a specific directive.
**
**  FORMAL PARAMETERS:
**
**	response	Pointer to current response.
**	directive	Pointer to directive to generate.
**	class		Pointer to current class.
**      mom		Pointer to the MOM description block.
**	outfile		File handle of the current output file.
**
**  RETURN VALUE:
**
**      None
**
**--
*/
void	insert_init_args_resp_imports( response,
			               directive, 
			               class, 
			               mom, 
			               outfile )

RESPONSE_DEF *response;
DIRECTIVE_DEF *directive;
CLASS_DEF *class;
MOM_BUILD *mom;
FILE *outfile;

{
char		buffer[256];
int argument_num;
ARGUMENT_DEF *argument;

argument = response->arg;
for (argument_num=0; argument_num < response->num_arguments; argument_num++)
   {
    if (argument->dna_cmip_int_len)
	{
	sprintf(buffer, "IMPORT object_id %s%s_DNA; \n",
	    class->prefix_m, argument->argument_name);
	fputs(buffer, outfile);
	}
    if (argument->oid_len)
	{
	sprintf(buffer, "IMPORT object_id %s%s_OID; \n",
	    class->prefix_m, argument->argument_name);
	fputs(buffer, outfile);
	}
    if (argument->snmp_oid_len)
	{
	sprintf(buffer, "IMPORT object_id %s%s_SNP; \n",
	    class->prefix_m, argument->argument_name);
	fputs(buffer, outfile);
	}
    if (argument->osi_oid_len)
	{
	sprintf(buffer, "IMPORT object_id %s%s_OSI; \n",
	    class->prefix_m, argument->argument_name);
	fputs(buffer, outfile);
	}

    argument = argument->next;
    }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_init_args_create_imports
**
**	This routine generates the response arguments for a specific directive.
**
**  FORMAL PARAMETERS:
**
**	class		Pointer to current class.
**      mom		Pointer to the MOM description block.
**	outfile		File handle of the current output file.
**
**  RETURN VALUE:
**
**      None
**
**--
*/
void	insert_init_args_create_imports( class, 
			                 mom, 
			                 outfile )

CLASS_DEF *class;
MOM_BUILD *mom;
FILE *outfile;

{
char		buffer[256];
int argument_num;
ARGUMENT_DEF *argument;

argument = class->create_arg;
for (argument_num=0; argument_num < class->num_create_args; argument_num++)
    {
    if (argument->dna_cmip_int_len)
	{
	sprintf(buffer, "IMPORT object_id %s%s_DNA; \n",
	    class->prefix_m, argument->argument_name);
	fputs(buffer, outfile);
	}
    if (argument->oid_len)
	{
	sprintf(buffer, "IMPORT object_id %s%s_OID; \n",
	    class->prefix_m, argument->argument_name);
	fputs(buffer, outfile);
	}
    if (argument->snmp_oid_len)
	{
	sprintf(buffer, "IMPORT object_id %s%s_SNP; \n",
	    class->prefix_m, argument->argument_name);
	fputs(buffer, outfile);
	}
    if (argument->osi_oid_len)
	{
	sprintf(buffer, "IMPORT object_id %s%s_OSI; \n",
	    class->prefix_m, argument->argument_name);
	fputs(buffer, outfile);
	}

    argument = argument->next;
    }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_init_args_exc_imports
**
**	This routine generates the exception arguments for a specific directive.
**
**  FORMAL PARAMETERS:
**
**	exception  	Pointer to current exception.
**	directive	Pointer to directive to generate.
**	class		Pointer to current class.
**      mom		Pointer to the MOM description block.
**	outfile		File handle of the current output file.
**
**  RETURN VALUE:
**
**      None
**
**--
*/
void	insert_init_args_exc_imports( exception, 
			              directive, 
			              class, 
			              mom, 
			              outfile )

EXCEPTION_DEF *exception;
DIRECTIVE_DEF *directive;
CLASS_DEF *class;
MOM_BUILD *mom;
FILE *outfile;

{
char		buffer[256];
int argument_num;
ARGUMENT_DEF *argument;

argument = exception->arg;
for (argument_num=0; argument_num < exception->num_arguments; argument_num++)
   {
    if (argument->dna_cmip_int_len)
	{
	sprintf(buffer, "IMPORT object_id %s%s_DNA; \n",
	    class->prefix_m, argument->argument_name);
	fputs(buffer, outfile);
	}
    if (argument->oid_len)
	{
	sprintf(buffer, "IMPORT object_id %s%s_OID; \n",
	    class->prefix_m, argument->argument_name);
	fputs(buffer, outfile);
	}
    if (argument->snmp_oid_len)
	{
	sprintf(buffer, "IMPORT object_id %s%s_SNP; \n",
	    class->prefix_m, argument->argument_name);
	fputs(buffer, outfile);
	}
    if (argument->osi_oid_len)
	{
	sprintf(buffer, "IMPORT object_id %s%s_OSI; \n",
	    class->prefix_m, argument->argument_name);
	fputs(buffer, outfile);
	}

    argument = argument->next;
    }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_init_register
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file.
**
**	This routine generates the code to register all of the MOM's
**	entity classes.
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
int insert_init_register( mom,
		          outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    char		buffer[256];
    int			class_num;
    CLASS_DEF		*class;
    int			status;

    class = mom->class;

    fputs("    status = deregister_all( mold_handle, mom_handle );\n",outfile);
    fputs("    if ERROR_CONDITION( status ) \n", outfile);
    fputs("        return status;\n",outfile);
    
    for (class_num=0; class_num < mom->num_classes; class_num++)
    {
	fputs("    /*\n", outfile);
	sprintf(buffer,
		"     * Register %s class\n",
		class->class_name);
	fputs(buffer, outfile);
	fputs("     */\n", outfile);

        if (class->dna_cmip_int_len)
	 {
	 if (class->multiple_oids)
	    fputs("#ifdef DNA_CMIP_OID\n",outfile);

	 sprintf(buffer,
		"    status = moss_register(mold_handle, mom_handle, &%s_DNA, &%s_DNA);\n",
		class->parent,
		class->class_name);
	 fputs(buffer, outfile);
	 fputs("    if ERROR_CONDITION(status)\n", outfile);
	 fputs("        return status;\n\n", outfile);
	 if (class->multiple_oids)
	     fputs("#endif /* DNA_CMIP_OID */ \n",outfile);
	 }

	if (class->snmp_oid_len)
	 {
	 fputs("#ifdef SNMP_OID\n",outfile);
	 sprintf(buffer,
		"    status = moss_register(mold_handle, mom_handle, &%s_SNP, &%s_SNP);\n",
		class->parent,
		class->class_name);
 	 fputs(buffer, outfile);
	 fputs("    if ERROR_CONDITION(status)\n", outfile);
 	 fputs("        return status;\n\n", outfile);
	 fputs("#endif /* SNMP_OID */ \n",outfile);
	 }
	if (class->oid_len)
	 {
	 fputs("#ifdef OID\n",outfile);
	 sprintf(buffer,
		"    status = moss_register(mold_handle, mom_handle, &%s_OID, &%s_OID);\n",
		class->parent,
		class->class_name);
	 fputs(buffer, outfile);
	 fputs("    if ERROR_CONDITION(status)\n", outfile);
	 fputs("        return status;\n\n", outfile);
	 fputs("#endif /* OID */ \n",outfile);
	 }

	if (class->osi_oid_len)
	 {
	 fputs("#ifdef OSI_OID\n",outfile);
	 sprintf(buffer,
		"    status = moss_register(mold_handle, mom_handle, &%s_OSI, &%s_OSI);\n",
		class->parent,
		class->class_name);
	 fputs(buffer, outfile);
	 fputs("    if ERROR_CONDITION(status)\n", outfile);
	 fputs("        return status;\n\n", outfile);
	 fputs("#endif /* OSI_OID */ \n",outfile);
	 }
	class = class->next;
    }

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_dereg_all
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on tag insert-code-dereg-all.
**
**	This routine deregisters all of the subclasses and then the parent
**	classes before registering. Errors from the deregister are ignored 
**	so that testing can be performed multiple times.
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
int insert_dereg_all( mom,
		      outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    typedef struct _CPTR {
	struct _CPTR *next;
	CLASS_DEF *class;
    } CPTR;
    CPTR		*ptr,*tmp;    
    char		buffer[256];
    int			class_num;
    CLASS_DEF		*class;
    int			status;

    ptr = (CPTR *)malloc (sizeof(CPTR));
    ptr->next = ptr;

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
    {
    	tmp = (CPTR *)malloc (sizeof(CPTR));
        tmp->class = class;

	tmp->next = ptr->next;
	ptr->next = tmp;

        class = class->next;
    }

    tmp = ptr->next;
    for (class_num=0; class_num < mom->num_classes; class_num++)
    {
	class = tmp->class;       
	fputs("    /*\n", outfile);
	sprintf(buffer,
		"     * Deregister %s class\n",
		class->class_name);
	fputs(buffer, outfile);
	fputs("     */\n", outfile);

	if (class->dna_cmip_int_len)
	{
	  if (class->multiple_oids)
	      fputs("#ifdef DNA_CMIP_OID\n",outfile);

	  sprintf(buffer,
		"    status = moss_deregister(mold_handle, mom_handle, &%s_DNA);  \n",
		class->class_name);
	  fputs(buffer, outfile);
	  fputs("    if ERROR_CONDITION( status ) \n", outfile);
	  fputs("        if (status != MAN_C_NO_SUCH_CLASS)\n",outfile);
	  fputs("            return status;\n",outfile);
	  if (class->multiple_oids)
	      fputs("#endif /* DNA_CMIP_OID */\n",outfile);
	}

	if (class->snmp_oid_len)
	{
	  fputs("#ifdef SNMP_OID\n",outfile);
	  sprintf(buffer,
		"    status = moss_deregister(mold_handle, mom_handle, &%s_SNP);  \n",
		class->class_name);
	  fputs(buffer, outfile);
	  fputs("    if ERROR_CONDITION( status ) \n", outfile);
	  fputs("        if (status != MAN_C_NO_SUCH_CLASS)\n",outfile);
	  fputs("            return status;\n",outfile);
	  fputs("#endif /* SNMP_OID */\n",outfile);
	}

	if (class->oid_len)
	{
	  fputs("#ifdef OID\n",outfile);
	  sprintf(buffer,
		"    status = moss_deregister(mold_handle, mom_handle, &%s_OID);  \n",
		class->class_name);
	  fputs(buffer, outfile);
	  fputs("    if ERROR_CONDITION( status ) \n", outfile);
	  fputs("        if (status != MAN_C_NO_SUCH_CLASS)\n",outfile);
	  fputs("            return status;\n",outfile);
	  fputs("#endif /* OID */\n",outfile);
	}

	if (class->osi_oid_len)
	{
	  fputs("#ifdef OSI_OID\n",outfile);
	  sprintf(buffer,
		"    status = moss_deregister(mold_handle, mom_handle, &%s_OSI);  \n",
		class->class_name);
	  fputs(buffer, outfile);
	  fputs("    if ERROR_CONDITION( status ) \n", outfile);
	  fputs("        if (status != MAN_C_NO_SUCH_CLASS)\n",outfile);
	  fputs("            return status;\n",outfile);
	  fputs("#endif /* OSI_OID */\n",outfile);
	}
	
	tmp = tmp->next;
    }

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_perform_inits
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the tag insert-code-perform-init.
**
**	This routine generates the code to call the initialization routine
**	for each entity class.
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
int insert_perform_inits( mom,
		  	  outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    CLASS_DEF	    *class;
    int		    class_num;
    char	    buffer[256];
    int		    status;

    if (!mom->multiclass)
    {
	fputs("    status = (man_status) perform_init();\n",		outfile);
	fputs("    if ERROR_CONDITION(status)\n",			outfile);
	fputs("        return status;\n",				outfile);
	return SS$_NORMAL;
    }

    fputs("    /*\n",							outfile);
    fputs("     * Call initialization routine for each class.\n",	outfile);
    fputs("     */\n",							outfile);

    class = mom->class;

    for (class_num=0; class_num < mom->num_classes; class_num++)
    {
	sprintf(buffer,
		"    status = (man_status) %sperform_init();\n",
		class->prefix);
	fputs(buffer, outfile);

	fputs("    if ERROR_CONDITION(status)\n",			outfile);
	fputs("        return status;\n",				outfile);

	class = class->next;
    }

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_init_trap
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the tag insert-code-init-trap.
**
**	This routine generates the code to call the trap initialization
**	routine init_trap().
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
int insert_init_trap( mom,
		      outfile)

MOM_BUILD *mom;
FILE *outfile;

{

#ifdef UNIXPLATFORM

    fputs("    /*\n", outfile);
    fputs("     * Initialize structure for each trap if any.\n", outfile);
    fputs("     */\n\n", outfile);
    fputs("    status = (man_status) init_trap();\n", outfile);
    fputs("    if ERROR_CONDITION(status)\n", outfile);
    fputs("        return status;\n", outfile);

#endif

    return SS$_NORMAL;
}
