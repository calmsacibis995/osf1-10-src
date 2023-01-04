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
static char *rcsid = "@(#)$RCSfile: mg_create.c,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/07/13 22:25:56 $";
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
**	This module contains the routines to used to generate CREATE code.
**
**  AUTHOR:
**
**      Gary Allison
**
**	Some routines were extracted from BUILD.C (Rich Bouchard, Jr.) to 
**	form this module.
**
**  CREATION DATE:  30-JUL-1991
**
**  MODIFICATION HISTORY:
**
**	Marc Nozell	15-Jun-1992	Improve portability
**	    - removed references to module. 
**
**      Mike Densmore	21-May-1992	Modified for use on U**X platforms.
**
**	Mike Densmore	25-Sep-1992	Modified for ANSI Standard C
**
**	M. Ashraf	19-May-1993	Modified internal_select_routine to
**					add func. protos "extern man_status xx"
**
**--
*/

#ifdef VMS
#include "vaxcshr.h"		/* translate from VAX C to DEC C RTL names */
#endif 
#define NAME_SIZE 5

#include "mom_defs.h"
#include <string.h>
#include "mg_prototypes.h"

#if defined(sun) || defined(sparc)
int reply_get_response();
int insert_instance_oid();
int get_create_response_arg();
#else
int reply_get_response( CLASS_DEF *, RESPONSE_DEF **);
int insert_instance_oid( MOM_BUILD *, FILE *);
int get_create_response_arg( CLASS_DEF *, ARGUMENT_DEF *, ARGUMENT_DEF **);
#endif


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      insert_identifier_attr
**
**	This routine generates the code to setup the indentifier attribute.
**
**  FORMAL PARAMETERS:
**
**	mom		Pointer to the MOM description block.
**	class		Pointer to current class block.
**
**  RETURN VALUE:
**
**      None		
**
**--
*/
insert_identifier_attr( mom,
			outfile )

MOM_BUILD *mom;
FILE *outfile;

{			
    CLASS_DEF 	     *class;
    int 	     found_one = 0;
    char	     buffer[256];
    ATTRIBUTE_DEF    *attribute;
    int 	     attribute_num;

    class = mom->current_class;
    attribute = class->attribute;
    for (attribute_num=0; attribute_num < class->num_attributes; attribute_num++)
    {
        if (attribute->group == GROUP_IDENTIFIERS)
	    if (attribute->mg_type == MG_C_STRING)
	        {
		found_one = 1;
		sprintf(buffer,"    new_%s_instance->instance_name = (char *) malloc( instance_length + sizeof( char ));\n",
					class->class_name);
		fputs(buffer,outfile);
		sprintf(buffer,"    if (new_%s_instance->instance_name == NULL)\n", 
					class->class_name);
		fputs(buffer,outfile);
		fputs("        return MAN_C_INSUFFICIENT_RESOURCES;\n",outfile);
	
		sprintf(buffer,"    strncpy( new_%s_instance->instance_name, instance, instance_length );\n",
					class->class_name);
		fputs(buffer,outfile);
		sprintf(buffer,"    new_%s_instance->instance_name[ instance_length ] = (char) NULL;\n",
					class->class_name);
		fputs(buffer,outfile);
		sprintf(buffer,"    new_%s_instance->instance_name_length  = instance_length;\n",
					class->class_name);
		fputs(buffer,outfile);

		sprintf(buffer,"    new_%s_instance->%s = (char *) malloc( instance_length + sizeof( char ));\n",
			class->class_name,attribute->attribute_name+NAME_SIZE);
		fputs(buffer,outfile);

		sprintf(buffer,"    if (new_%s_instance->%s == NULL)\n",
			class->class_name,attribute->attribute_name+NAME_SIZE);
		fputs(buffer,outfile);
                fputs("        return MAN_C_INSUFFICIENT_RESOURCES;\n\n",outfile);
		sprintf(buffer,"    strncpy( new_%s_instance->%s, instance, instance_length );\n",
			class->class_name,attribute->attribute_name+NAME_SIZE);
		fputs(buffer,outfile);
		sprintf(buffer,"    new_%s_instance->%s[ instance_length ] = (char) NULL;\n",
			class->class_name,attribute->attribute_name+NAME_SIZE);
		fputs(buffer,outfile);
		sprintf(buffer,"    new_%s_instance->%s_len  = instance_length;\n",
			class->class_name,attribute->attribute_name+NAME_SIZE);
		fputs(buffer,outfile);
		return SS$_NORMAL;
	        }
		else if (attribute->mg_type == MG_C_INTEGER )
		{
		found_one = 1;
		if (attribute->sign == MG_C_UNSIGNED)
		    sprintf(buffer,"    status = copy_octet_to_unsigned_int( &new_%s_instance->%s, instance_octet );\n",
			class->class_name,attribute->attribute_name+NAME_SIZE);
		else
		    sprintf(buffer,"    status = copy_octet_to_signed_int( &new_%s_instance->%s, instance_octet );\n",
			class->class_name,attribute->attribute_name+NAME_SIZE);
		fputs(buffer,outfile);
	    	fputs("    if ERROR_CONDITION( status )\n",outfile);
	    	fputs("        return status;\n",outfile);
		
		
		return SS$_NORMAL;
		}

		else 
		{
		fputs("/** Unsupported identifier attribute datatype. Setup up instance_name field **/\n",outfile);
		return SS$_NORMAL;
		}
        attribute = attribute->next;
	}


return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      find_exception
**
**	This routine finds the exception string and returns the pointer
**	to that exception.
**
**  FORMAL PARAMETERS:
**
**	mom		Pointer to the MOM description block.
**	class		Pointer to current class block.
**	exc_str		Address of exception string to match.
**	exc_code	Integer containing code number.
**	exception	Address of pointer to return.
**	directive	Pointer to directive (optional).
**
**  RETURN VALUE:
**
**      None		
**
**--
*/
find_exception( mom,
		class,
		exc_str,
		exc_code,
		exception,
		directive )

MOM_BUILD *mom;
CLASS_DEF *class;
char *exc_str;
int exc_code;
EXCEPTION_DEF **exception;
DIRECTIVE_DEF *directive;

{
    DIRECTIVE_DEF *dir;
    EXCEPTION_DEF *exc;

    *exception = NULL;
    dir = class->directive;

    if (directive != NULL)
	dir = directive;

    while (dir != NULL)
	{
        if (dir->type == MOMGEN_K_DIR_ACTION)
            {
    	    exc = dir->exc;
    	    while (exc != NULL)
		{
		if (exc->dna_cmip_int == exc_code)
	    	    {
	    	    *exception = exc;
	    	    return;
                    }
		if (exc->original_name != NULL)
		    {
		    if (strcmp(exc->original_name, exc_str) == 0)
	    	    	{
	    	    	*exception = exc;
	    	    	return;
                    	}
		    }			
       		else if (strcmp(exc->name, exc_str) == 0)
	    	    {
	    	    *exception = exc;
	    	    return;
                    }
		exc = exc->next;
		}
	    }

	/* Only look at current directive (if specified) */

	if (directive != NULL)
	    return;
	else        
	    dir = dir->next;
	}
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      insert_create_action_exceptions
**
**	This routine gets called to insert a specific exception for the
**	DNA CMIP create action directive.
**	
**
**  FORMAL PARAMETERS:
**
**	mom		Pointer to the MOM description block.
**	outfile		Pointer to output file.
**	exc_str		Address of exception string to match.
**	exc_code	Integer containing code number.
**
**  RETURN VALUE:
**
**      None		
**
**--
*/
insert_create_action_exceptions( mom, 
		                 outfile,
		                 exc_str,
		                 exc_code )

MOM_BUILD *mom;
FILE *outfile;
char *exc_str;
int exc_code;

{
    EXCEPTION_DEF *exc = NULL;
    char	     buffer[256];
    CLASS_DEF *class;
    int	     status;
    int      support=0;
    int	     class_num;

    fputs("        if (type == ACTION)\n",outfile);
    fputs("            {\n",outfile);
    fputs("            *reply = (reply_type) MAN_C_PROCESSING_FAILURE;\n",outfile);

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
    {
      if (class->support_create)
	{
	support = 1;
	break;
	}
      class = class->next;
    }
    
    if (!support)
	{
    	fputs("            }\n",outfile);
	return;
	}

    fputs("        switch (class_code)\n",					outfile);
    fputs("            {\n",							outfile);

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
      {
      status = find_exception( mom, class, exc_str, exc_code,  &exc, (DIRECTIVE_DEF *) NULL  );
      if (exc != NULL)
        {
	sprintf(buffer, "            case %s%s_ID:\n", 
		class->prefix_k, class->class_name);
 	fputs(buffer, outfile);
	if (exc->dna_cmip_int_len)
	{
	    if (class->multiple_oids)
	    	fputs("#ifdef DNA_CMIP_OID\n",outfile);
            sprintf(buffer,"                    *oid = &%s%s_DNA;\n", 
			class->prefix_m, exc->exception_name);
            fputs(buffer, outfile);	    
	    if (class->multiple_oids)
		    fputs("#endif /* DNA_CMIP_INT */\n",outfile);		
	    }
        if (exc->osi_oid_len)
	    {
	    fputs("#ifdef OSI_CMIP_INT\n",outfile);
            sprintf(buffer,"                    *oid = &%s%s_OSI;\n", 
			class->prefix_m,exc->exception_name);
            fputs(buffer, outfile);	    
	    fputs("#endif /* OSI_CMIP_INT */\n",outfile);		
	    }
        if (exc->snmp_oid_len)
	    {
	    fputs("#ifdef SNMP_OID\n",outfile);
            sprintf(buffer,"                    *oid = &%s%s_SNP;\n", 
			class->prefix_m,exc->exception_name);
            fputs(buffer, outfile);	    
	    fputs("#endif /* SNMP_OID */\n",outfile);		
	    }
        if (exc->oid_len)
	    {
	    fputs("#ifdef OID\n",outfile);
            sprintf(buffer,"                    *oid = &%s%s_OID;\n", 
	    	class->prefix_m,exc->exception_name);
            fputs(buffer, outfile);	    
	    fputs("#endif /* OID */\n",outfile);		
	    }
        fputs("                    break;\n", outfile);
	}
      class = class->next;
    }
   return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      insert_default_error_reply
**
**	This routine adds the code to setup the default error reply. The
**	exception UNEXPECTED ERROR is generated as a reply with an argument
**	containing the unexpected error.
**
**	The tag insert-code-default-error-reply is used.
**
**  FORMAL PARAMETERS:
**
**	mom		Pointer to the MOM description block.
**	outfile		Pointer to file output file.
**
**  RETURN VALUE:
**
**      SS$_NORMAL		- Normal successful completion.
**
**--
*/
momgen_status insert_default_error_reply( mom, 
		     			  outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    int 	found=0;
    ARGUMENT_DEF *arg=NULL;
    DIRECTIVE_DEF *dir;
    EXCEPTION_DEF *exc=NULL;
    int	     exception_num;
    char	     buffer[256];
    int      support=0;
    int	     class_num;
    int	     status,found_one=0;
    CLASS_DEF *class;

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
    {
      if (class->support_create)
	{
	support = 1;
	break;
	}
      class = class->next;
    }
    if (!support)
	return;

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
      {
      dir = class->directive;

      while (dir != NULL)
	{
        if (dir->type == MOMGEN_K_DIR_ACTION)
	    {
	    find_exception( mom, class, "UNEXPECTEDERROR", mom->unexpected_exc_code, &exc, dir );
	    if (exc != NULL)	    
	      {                                        
	      arg = exc->arg;
	      if (arg != NULL)
	        {
		if (!found_one)
		    {
                    fputs("            switch (class_code)\n",					outfile);
               	    fputs("                {\n",							outfile);
                                                             
     	            sprintf(buffer,"                case %s%s_ID:\n", 
				class->prefix_k, class->class_name);
      		    fputs(buffer, outfile);
		    }		
	        found_one = 1;
	        sprintf(buffer,"                    if ((stat = moss_compare_oid( action_type, &%s%s_DNA)) == MAN_C_EQUAL)\n",
			class->prefix_m, dir->directive_name);
   	        fputs(buffer, outfile);
		fputs(         "                        {\n",outfile);

		if (exc->dna_cmip_int_len)
		    {
		    if (class->multiple_oids)
		    	fputs("#ifdef DNA_CMIP_OID\n",outfile);
 		    sprintf(buffer,"                        *oid = &%s%s_DNA;\n", 
				class->prefix_m, exc->exception_name);
	            fputs(buffer, outfile);	    
	            sprintf(buffer,"                        arg_oid = &%s%s_DNA;\n", 
		    	class->prefix_m,arg->argument_name);
	            fputs(buffer, outfile);	    
		    if (class->multiple_oids)
			    fputs("#endif /* DNA_CMIP_INT */\n",outfile);		
		    }
	        if (exc->osi_oid_len)
		    {
		    fputs("#ifdef OSI_CMIP_INT\n",outfile);
	            sprintf(buffer,"                        *oid = &%s%s_OSI;\n", 
				class->prefix_m,exc->exception_name);
	            fputs(buffer, outfile);	    
	            sprintf(buffer,"                        arg_oid = &%s%s_OSI;\n", 
		    	class->prefix_m,arg->argument_name);
	            fputs(buffer, outfile);	    
		    fputs("#endif /* OSI_CMIP_INT */\n",outfile);		
		    }
	        if (exc->snmp_oid_len)
		    {
		    fputs("#ifdef SNMP_OID\n",outfile);
	            sprintf(buffer,"                        *oid = &%s%s_SNP;\n", 
				class->prefix_m,exc->exception_name);
	            fputs(buffer, outfile);	    
	            sprintf(buffer,"                        arg_oid = &%s%s_SNP;\n", 
		    	class->prefix_m,arg->argument_name);
	            fputs(buffer, outfile);	    
		    fputs("#endif /* SNMP_OID */\n",outfile);		
		    }
	        if (exc->oid_len)
		    {
		    fputs("#ifdef OID\n",outfile);
	            sprintf(buffer,"                        *oid = &%s%s_OID;\n", 
		    	class->prefix_m,exc->exception_name);
	            fputs(buffer, outfile);	    
	            sprintf(buffer,"                        arg_oid = &%s%s_OID;\n", 
		    	class->prefix_m,arg->argument_name);
	            fputs(buffer, outfile);	    
		    fputs("#endif /* OID */\n",outfile);		
		    }
                
		fputs(             "                        }\n",outfile);
		}  /* ARG != NULL */
              }   /* EXC != NULL */
	    }    /* DIR == ACTION */

          dir = dir->next;
	  }    /* WHILE DIR */

        if (found_one)
            fputs("                    break;\n", outfile);

        class = class->next;
        }     /* WHILE CLASS */


        if (found_one)
	    {
            fputs("                }\n", outfile);
	    fputs("        octet.data_type = ASN1_C_INTEGER;\n",outfile);
	    fputs("        octet.length = 4;\n",outfile);
	    fputs("        octet.string = (char *) &status;\n",outfile);
	    fputs("        status = moss_avl_add( attr, \n",outfile);
	    fputs("                               arg_oid,\n",outfile);
	    fputs("                               (unsigned int) MAN_C_SUCCESS,\n",outfile);
	    fputs("                               ASN1_C_INTEGER,\n",outfile);
	    fputs("                               &octet);\n\n",outfile);
	    }

   return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      get_identifier_attr
**
**	This routine returns the first (primary) identifier attribute.
**
**  FORMAL PARAMETERS:
**
**
**  RETURN VALUE:
**
**      SS$_NORMAL		- Normal successful completion.
**
**--
*/
void	get_identifier_attr( class,
			     attr )

CLASS_DEF *class;
ATTRIBUTE_DEF **attr;

{
    int attribute_num;
    ATTRIBUTE_DEF *attribute;

    attribute = class->attribute;
    for (attribute_num=0; attribute_num < class->num_attributes; attribute_num++)
    {
        if (attribute->group == GROUP_IDENTIFIERS)
	    *attr = attribute;
	else
            attribute = attribute->next;
    }	
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      insert_duplicate_error_reply
**
**	This routine adds the code to setup the duplicate error reply.
**
**	The tag insert-code-duplicate-error-reply is used.
**
**  FORMAL PARAMETERS:
**
**	mom		Pointer to the MOM description block.
**	outfile		Pointer to file output file.
**
**  RETURN VALUE:
**
**      SS$_NORMAL		- Normal successful completion.
**
**--
*/
momgen_status insert_duplicate_error_reply( mom, 
		     			    outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    int	      support=0;
    DIRECTIVE_DEF *dir;
    EXCEPTION_DEF *exc;
    ARGUMENT_DEF *arg;
    ATTRIBUTE_DEF *attr;
    int	      exception_num;
    char      buffer[256];
    int	      status;
    int       class_num;
    CLASS_DEF *class;

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
    {
      if (class->support_create)
	{
	support = 1;
	break;
	}
      class = class->next;
    }
    if (!support)
	return;

    fputs("              object_id *arg_oid;\n",outfile);
    fputs("              int identifier_datatype;\n\n",outfile);
                         
    fputs("              switch (class_code)\n", outfile);
    fputs("                {\n",	       outfile);
                                                             
    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
      {
      dir = class->directive;
      while (dir != NULL)
	{
        if (strcmp( dir->directive, "CREATE") == 0)
	    {
	    find_exception( mom, class, "DUPLICATEMANAGEDOBJECTINSTANCE", mom->duplicate_exc_code, &exc, dir );
	    if (exc != NULL)	    
	      {                                        
	      arg = exc->arg;
	      if (arg != NULL)
	        {
                sprintf(buffer,"                case %s%s_ID:\n", 
		     class->prefix_k, class->class_name);
                fputs(buffer, outfile);
		fputs( "                    {\n",outfile);

		get_identifier_attr( class, &attr );
		
		if (exc->dna_cmip_int_len)
		{
	    	    if (class->multiple_oids)
	    		fputs("#ifdef DNA_CMIP_OID\n",outfile);
            	    sprintf(buffer,"                    action_response_type_oid = &%s%s_DNA;\n", 
				class->prefix_m, exc->exception_name);
            	    fputs(buffer, outfile);	    
        	    sprintf(buffer,"                    arg_oid = &%s%s_DNA;\n", class->prefix_m, arg->argument_name);
                    fputs(buffer, outfile);	    
		    sprintf(buffer,"                    identifier_datatype = %s;\n", attr->dna_data_type );
                    fputs(buffer, outfile);	    
	    	    if (class->multiple_oids)
		    	fputs("#endif /* DNA_CMIP_INT */\n",outfile);		
	    	}
        	if (exc->osi_oid_len)
	    	    {
	    	    fputs("#ifdef OSI_CMIP_INT\n",outfile);
                    sprintf(buffer,"                    action_response_type_oid = &%s%s_OSI;\n", 
			class->prefix_m,exc->exception_name);
                    fputs(buffer, outfile);	    
        	    sprintf(buffer,"                    arg_oid = &%s%s_OSI;\n", class->prefix_m, arg->argument_name);
                    fputs(buffer, outfile);	    
		    sprintf(buffer,"                    identifier_datatype = %s;\n", attr->dna_data_type );
                    fputs(buffer, outfile);	    
	            fputs("#endif /* OSI_CMIP_INT */\n",outfile);		
	            }
                if (exc->snmp_oid_len)
	            {
	            fputs("#ifdef SNMP_OID\n",outfile);
                    sprintf(buffer,"                    action_response_type_oid = &%s%s_SNP;\n", 
			        class->prefix_m,exc->exception_name);
                    fputs(buffer, outfile);	    
        	    sprintf(buffer,"                    arg_oid = &%s%s_SNP;\n", class->prefix_m, arg->argument_name);
                    fputs(buffer, outfile);	    
		    sprintf(buffer,"                    identifier_datatype = %s;\n", attr->dna_data_type );
                    fputs(buffer, outfile);	    
	            fputs("#endif /* SNMP_OID */\n",outfile);		
	            }
                if (exc->oid_len)
	            {
	            fputs("#ifdef OID\n",outfile);
                    sprintf(buffer,"                    action_response_type_oid = &%s%s_OID;\n", 
	    	        class->prefix_m,exc->exception_name);
                    fputs(buffer, outfile);	    
        	    sprintf(buffer,"                    arg_oid = &%s%s_OID;\n", class->prefix_m, arg->argument_name);
                    fputs(buffer, outfile);	    
		    sprintf(buffer,"                    identifier_datatype = %s;\n", attr->dna_data_type );
                    fputs(buffer, outfile);	    
	            fputs("#endif /* OID */\n",outfile);		
	            }
		fputs( "                    break;\n",outfile);
		fputs( "                    }\n",outfile);
		}  /* ARG != NULL */
	      }   /* EXC != NULL */
	    } 	 /* if CREATE */
	    dir = dir->next; 
	  }	/* While DIR != NULL */
	class = class->next;
	}	/* Class loop */

        fputs("                }\n\n",	       outfile);
	fputs("              status = moss_avl_add(reply_avl, \n",outfile);
	fputs("                                     arg_oid,\n",outfile);
        fputs("                                     (unsigned int) MAN_C_SUCCESS,\n",outfile);
        fputs("                                     identifier_datatype,\n",outfile);
        fputs("                                     instance_octet);\n\n",outfile);
        
   return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      insert_add_new_instances_call
**
**	This routine adds the call to add_new_instances() to create.c.
**
**	The tag insert-code-add-new-instances-call tag is used.
**
**  FORMAL PARAMETERS:
**
**	mom		Pointer to the MOM description block.
**	outfile		Pointer to file output file.
**
**  RETURN VALUE:
**
**      SS$_NORMAL		- Normal successful completion.
**
**--
*/
momgen_status insert_add_new_instances_call( mom, 
		     			     outfile)

MOM_BUILD *mom;
FILE *outfile;

{
   char	     buffer[256];
   int	     status;
   CLASS_DEF *class;

   class = mom->current_class;

   fputs(         "    if (status == MAN_C_SUCCESS)\n",outfile);
   sprintf(buffer,"      %sadd_new_instance( new_%s_header, new_%s_instance );\n", 
		class->prefix_m,class->class_name,class->class_name);
   fputs(buffer, outfile);                          
   
   return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      insert_add_new_instances
**
**	This routine adds the routine add_new_instances() to create.c.
**	Add_new_instances inserts an element q after element p. If the
**      MOM does not support instances, no code is produced.
**
**	The tag insert-code-add-new-instances tag is used for this routine.
**
**  FORMAL PARAMETERS:
**
**	mom		Pointer to the MOM description block.
**	outfile		Pointer to file output file.
**
**  RETURN VALUE:
**
**      SS$_NORMAL		- Normal successful completion.
**
**--
*/
momgen_status insert_add_new_instances( mom, 
		     			outfile)

MOM_BUILD *mom;
FILE *outfile;

{
   char	    buffer[256];
   int	    status;
   CLASS_DEF *class;

   class = mom->current_class;

   if (!mom->support_instances)
       	return SS$_NORMAL;

   sprintf(buffer,"man_status %sadd_new_instance( p, q )\n\n", 
		class->prefix_m);
   fputs(buffer, outfile);                           

   sprintf(buffer,"%s_DEF *p;\n%s_DEF *q;\n\n{\n", 
		class->class_name,class->class_name);
   fputs(buffer, outfile);                           

   fputs("    q->next = p->next;\n",outfile);
   fputs("    q->prev = p;\n",outfile);
   fputs("    p->next->prev = q;\n",outfile);
   fputs("    p->next = q;\n",outfile);

   fputs("return MAN_C_SUCCESS;\n}\n",outfile);

   return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      insert_delete_instance
**
**	This routine adds code to remove instances from the instance queue.
** 	If instances are not supported, no code is produced.
**
**	The tag insert-code-delete-instance tag is used for this routine.
**
**  FORMAL PARAMETERS:
**
**	mom		Pointer to the MOM description block.
**	outfile		Pointer to file output file.
**
**  RETURN VALUE:
**
**      SS$_NORMAL		- Normal successful completion.
**
**--
*/
momgen_status insert_delete_instance( mom, 
		     		      outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    ATTRIBUTE_DEF   *attribute;
    int		    attribute_num;
    CLASS_DEF	    *class;
    char	    buffer[256];

    fputs("    instance->prev->next = instance->next;\n",outfile);
    fputs("    instance->next->prev = instance->prev;\n\n",outfile);
    fputs("    if (instance->instance_name != NULL)\n",outfile);
    fputs("        free( instance->instance_name );\n",outfile);
    fputs("    if (instance->object_instance != NULL)\n",outfile);
    fputs("	   moss_avl_free( &instance->object_instance, TRUE );\n",outfile);
    fputs("    if (memcmp( &instance->instance_uid, nil_uid_p, sizeof( uid )) != 0)\n",outfile);
    fputs("        instance->instance_uid = *nil_uid_p;\n\n",outfile);


    class = mom->current_class;					      
    attribute = class->attribute;

    for (attribute_num=0; attribute_num < class->num_attributes; attribute_num++)
    {
	switch (attribute->mg_type)
	{

	case MG_C_STRING :

	    sprintf(buffer,"    if (instance->%s_len != 0)\n",
		   	attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    sprintf(buffer,"        free( instance->%s );\n",
			attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    break;
    
        case MG_C_AVL :
	    sprintf(buffer,"    if (instance->%s != NULL)\n",
		   	attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    sprintf(buffer,"        moss_avl_free( &instance->%s, TRUE );\n",
		   attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    break;
	
        case MG_C_VERSION :
	    sprintf(buffer,"    if (instance->%s != NULL)\n",
		   	attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    sprintf(buffer,"        free( instance->%s );\n",
			attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    break;
	}			
        attribute = attribute->next;
    }	
           
    fputs("\n",outfile);

    fputs("    free( instance );\n\n",outfile);
    return SS$_NORMAL;
}
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_create_args
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the tag insert-code-create-args.
**
**	This routine generates the "switch" statement to store the
**	arguments supplied with a CREATE directive.
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
int insert_create_args( mom,
		        outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    ARGUMENT_DEF    *argument;
    int		    argument_num;
    CLASS_DEF	    *class;
    char	    buffer[256];
    int		    status;

    class = mom->class;
    argument = class->create_arg;

    for (argument_num=0; argument_num < class->num_create_args; argument_num++)
    {
	sprintf(buffer,
		"                case %s_ARG_%s:\n",
		class->prefix_k,
		argument->argument_name);
	fputs(buffer, outfile);

	sprintf(buffer,
		"                    /** Store the %s argument **/\n",
		argument->argument_name);
	fputs(buffer, outfile);
	fputs("                    break;\n", outfile);

	argument++;
    }
    
    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_select_routine
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the the tag insert-code-select-routine.
**
**	This routine generates the code to reduce an Object ID for a
**	class to a scalar constant internal to the MOM, so that
**	future code can perform a simple "switch" statement to
**	determine what class is being operated upon.
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
int insert_select_routine( mom,
			   outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    CLASS_DEF	    *class;
    int		    class_num;
    char	    buffer[256];
    int		    status;

    if (!mom->multiclass)
	return SS$_NORMAL;


    /*  Define the function declarations  */
    fputs(      "              /* Function declarations */\n\n", outfile);

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
    {
	sprintf(buffer,
		"              extern man_status %sperform_%s();\n",
		                        class->prefix_m, mom->temp);
	fputs(buffer, outfile);

	class = class->next;
    }
    fputs("\n", outfile);

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
    {
	sprintf(buffer,
		"                case %s%s_ID:\n", class->prefix_k, class->class_name);
	fputs(buffer, outfile);
	sprintf(buffer,
		"                    perform_%s = %sperform_%s;\n",	      
		mom->temp, class->prefix_m, mom->temp);
	fputs(buffer, outfile);
     	fputs("                    break;\n", outfile);

	class = class->next;
    }

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_reply_select_routine
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the the tag insert-code-reply-select-routine.
**
**	This routine generates a switch statement on the ID of the class 
**	(typically used for multi-class MOMs.
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
int insert_reply_select_routine( mom,
			         outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    RESPONSE_DEF    *resp;
    CLASS_DEF	    *class;
    int		    class_num;
    int		    support = 0;
    char	    buffer[256];
    int		    status;

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
    {
      if (class->support_create)
	{
	support = 1;
	break;
	}
      class = class->next;
    }
    
    if (support)
    {
      fputs("    switch (class_code)\n",					outfile);
      fputs("    {\n",							outfile);

      class = mom->class;
      for (class_num=0; class_num < mom->num_classes; class_num++)
      {
        if ((class->support_instances) && (class->support_create))
	  {
	  sprintf(buffer,
		"        case %s%s_ID:\n", class->prefix_k, class->class_name);
 	  fputs(buffer, outfile);

	  if (reply_get_response( class, &resp ))
            {
            fputs("              if (type == ACTION )\n",outfile);
	    if (class->dna_cmip_int_len)
	      {
	      if (class->multiple_oids)
	          fputs("#ifdef DNA_CMIP_OID\n",outfile);
	      sprintf(buffer,
		    "                *oid = &%s%s_DNA;\n",
			 class->prefix_m,resp->response_name);
	      fputs(buffer, outfile);
	      if (class->multiple_oids)
	          fputs("#endif /* DNA_CMIP_OID */\n",outfile);
	      }
	    if (class->osi_oid_len)
	      {
	      fputs("#ifdef OSI_CMIP_OID\n",outfile);
	      sprintf(buffer,
		    "                *oid = &%s%s_OSI;\n", 
			 class->prefix_m,resp->response_name);
	      fputs(buffer, outfile);
	      fputs("#endif /* OSI_CMIP_OID */\n",outfile);
	      }
	    if (class->snmp_oid_len)
	      {
	      fputs("#ifdef SNMP_OID\n",outfile);
	      sprintf(buffer,
		    "                *oid = &%s%s_OSI;\n", 
			 class->prefix_m,resp->response_name);
	      fputs(buffer, outfile);
	      fputs("#endif /* DNA_CMIP_OID */\n",outfile);
	      }
	    if (class->oid_len)
	      {
	      fputs("#ifdef OID\n",outfile);
	      sprintf(buffer,
		    "                *oid = &%s%s_OSI;\n", 
			 class->prefix_m,resp->response_name);
	      fputs(buffer, outfile);
	      fputs("#endif /* OID */\n",outfile);
	      }
	    }
	  else
	    {
            sprintf(buffer,
	        "\n            /** Insert action response success oids for class %s **/\n", class->class_name);
	    fputs(buffer, outfile);
	    }
	  fputs("            break;\n", outfile);
	  }
        else
	  {
          sprintf(buffer,
	        "\n            /** Insert action response success oids for class %s **/\n", class->class_name);
	  fputs(buffer, outfile);
	  }	
        class = class->next;
      }

      fputs("        default:\n",					outfile);
      fputs("            *reply = (reply_type) MAN_C_PROCESSING_FAILURE;\n",	outfile);
      fputs("    }\n",						outfile);
    }
    else
    {
          sprintf(buffer,
	        "\n            /** Insert action response success oids for MOM %s **/\n", mom->mom_name);
	  fputs(buffer, outfile);
    }     
    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	reply_get_response
**
**	This routine returns a response that matches "SUCC" under the CREATE
**	directive.
**
**  FORMAL PARAMETERS:
**
**      class		- Pointer to current class
**	response	- Address of pointer to return response
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**
**--
*/
int reply_get_response( class,
		        response)

CLASS_DEF *class;
RESPONSE_DEF **response;

{
    DIRECTIVE_DEF *dir;
    RESPONSE_DEF  *resp;
    int 	response_num;
    int		class_num;
    int		action_num,actions=0;
    int 	found = 0;
    momgen_status status;

    dir = class->directive;

    while (!found && (dir != NULL))
	if (strcmp(dir->directive, "CREATE") == 0)
	    found = 1;
	else
           dir = dir->next;
	    
    if ((!found) || (dir->num_responses == 0))
        return 0;

    resp = dir->resp;
    for (response_num=0; response_num < dir->num_responses; response_num++)
	{
        if (strcmp(resp->response_name,"SUCC") != NULL)
	    {
	    *response = resp;
	    return 1;
	    }
        resp = resp->next;
        }
   
   return 0;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_create_select_routine
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the the tag insert-code-create-select-routine.
**
**	This routine generates a switch statement on the ID of the class 
**	(typically used for multi-class MOMs.
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
int insert_create_select_routine( mom,
			          outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    CLASS_DEF	    *class;
    int		    class_num;
    char	    buffer[256];
    int		    status;

    fputs("    status = _get_class_code(object_class, &class_code);\n", outfile);
    fputs("    if (status == MAN_C_SUCCESS)\n",		        outfile);
    fputs("      switch (class_code)\n",					outfile);
    fputs("        {\n",							outfile);

    class = mom->class;

    for (class_num=0; class_num < mom->num_classes; class_num++)
    {
	sprintf(buffer,
		"        case %s%s_ID:\n", class->prefix_k, class->class_name);
	fputs(buffer, outfile);

	sprintf(buffer,
		"            perform_routine = %sperform_%s;\n",	      
		class->prefix_m, mom->temp);
	fputs(buffer, outfile);

	fputs("            break;\n", outfile);

	class = class->next;
    }

    fputs("        }\n",					outfile);

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_create_select_locate
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the the tag insert-code-create-select-locate tag.
**
**	This routine generates a switch statement on the ID of the class 
**	(typically used for multi-class MOMs.
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
int insert_create_select_locate( mom,
			         outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    CLASS_DEF	    *class;
    int		    class_num;
    char	    buffer[256];
    int		    status;

    fputs("    switch (class_code)\n",					outfile);
    fputs("    {\n",							outfile);

    class = mom->class;

    for (class_num=0; class_num < mom->num_classes; class_num++)
    {
	mom->current_class = class;
	sprintf(buffer,
		"        case %s%s_ID:\n", class->prefix_k, class->class_name);
	fputs(buffer, outfile);

	sprintf(buffer,
		"            perform_locate = %slocate_instance;\n",	      
		class->prefix_m);
	fputs(buffer, outfile);
	insert_instance_oid (mom,outfile);
	fputs(  "            break;\n",outfile);

	class = class->next;
    }

    fputs("    }\n",						outfile);

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_map_oid_list
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-map-oid-list tag.
**
** 	This routine generates code to obtain the value of the attribute
**	and set it into the current instance being created.  This routine 
**	is called only if instances are supported.
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
int insert_map_oid_list( mom,
		         outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    ATTRIBUTE_DEF   *attribute;
    ARGUMENT_DEF    *resp_argument;
    ARGUMENT_DEF    *argument;
    int		    argument_num;
    CLASS_DEF	    *class;
    char	    buffer[256];
    int		    status;

    class = mom->current_class;					      
    argument = class->create_arg;

    for (argument_num=0; argument_num < class->num_create_args; argument_num++)
    {
	sprintf(buffer,      
		"        case %s%s:",
		class->prefix_k, argument->argument_name);
	fputs(buffer, outfile);

      switch (argument->mg_type)
	{	
        case MG_C_STRING :

	    fputs("	/* OCTET_STRING */\n", outfile );        
	    sprintf(buffer,
		  "            new_%s_instance->%s = (char *)malloc(octet->length + sizeof(char));\n",
		   class->class_name,argument->argument_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    sprintf(buffer,
		  "            memcpy(new_%s_instance->%s,\n",
		   class->class_name,argument->argument_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs("                   octet->string,\n                   octet->length);\n", outfile);
	    sprintf(buffer,
		  "            new_%s_instance->%s_len = (unsigned int) octet->length;\n",
		   class->class_name,argument->argument_name+NAME_SIZE);
	    fputs(buffer, outfile);
    	    fputs("#ifdef MOMGENDEBUG\n",outfile);
	    sprintf(buffer,"            printf(\" %s:\\n\" );\n", 
			argument->argument_name+NAME_SIZE );
	    fputs(buffer, outfile);
    	    fputs("#endif /* MOMGENDEBUG */\n",outfile);
	    if (get_create_response_arg( class, argument, &resp_argument ))
		{
		fputs(  "            add_status = moss_avl_add( *return_avl,\n",outfile);
		sprintf(buffer,
			"                                       &%sARG_CREATE_%s_DNA,\n",
                                     	class->prefix_m, resp_argument->argument_name+11 );
	        fputs(buffer, outfile);
		fputs(  "                                       (unsigned int) MAN_C_SUCCESS,\n",outfile);
	        sprintf(buffer,
			"                                       %s,\n", resp_argument->dna_data_type);
	        fputs(buffer, outfile);
		fputs(  "                                       octet);\n",outfile);
                fputs(  "            if ERROR_CONDITION( add_status )\n", outfile);
		fputs(  "                return add_status;\n\n", outfile);
		}	        
	    fputs("            break;\n\n", outfile);
	    break;

        case MG_C_INTEGER :

	    fputs("	/* INTEGER  */\n", outfile );        
	    if (argument->sign == MG_C_UNSIGNED)
		sprintf(buffer,"            status = copy_octet_to_unsigned_int( &new_%s_instance->%s, octet );\n",
		        class->class_name,argument->argument_name+NAME_SIZE);
	    else
		sprintf(buffer,"            status = copy_octet_to_signed_int( &new_%s_instance->%s, octet );\n",
		        class->class_name,argument->argument_name+NAME_SIZE);
	    fputs(buffer, outfile);
    	    fputs("#ifdef MOMGENDEBUG\n",outfile);
	    sprintf(buffer,
		  "            printf(\" %s:\t%%d\\n\", new_%s_instance->%s);\n", 
		   argument->argument_name+NAME_SIZE,class->class_name,argument->argument_name+NAME_SIZE);
	    fputs(buffer, outfile);
    	    fputs("#endif /* MOMGENDEBUG */\n",outfile);
	    if (get_create_response_arg( class, argument, &resp_argument ))
		{
		fputs(  "            add_status = moss_avl_add( *return_avl,\n",outfile);
		sprintf(buffer,
			"                                       &%sARG_CREATE_%s_DNA,\n",
                                     	class->prefix_m, resp_argument->argument_name+11 );
	        fputs(buffer, outfile);
		fputs(  "                                       (unsigned int) MAN_C_SUCCESS,\n",outfile);
	        sprintf(buffer,
			"                                       %s,\n", resp_argument->dna_data_type);
	        fputs(buffer, outfile);
		fputs(  "                                       octet);\n",outfile);
                fputs(  "            if ERROR_CONDITION( add_status )\n", outfile);
		fputs(  "                return add_status;\n\n", outfile);
		}	        
	    fputs("            break;\n\n", outfile);
	    break;

        case MG_C_BOOLEAN :

	    fputs("	/* BOOLEAN  */\n", outfile );        
	    sprintf(buffer,
		  "            new_%s_instance->%s = (unsigned int) *octet->string;\n"  ,
		   class->class_name,argument->argument_name+NAME_SIZE);
	    fputs(buffer, outfile);
    	    fputs("#ifdef MOMGENDEBUG\n",outfile);
	    sprintf(buffer,
		  "            printf(\" %s:\t%%d\\n\", new_%s_instance->%s);\n", 
		   argument->argument_name+NAME_SIZE,class->class_name,argument->argument_name+NAME_SIZE);
	    fputs(buffer, outfile);
    	    fputs("#endif /* MOMGENDEBUG */\n",outfile);
	    if (get_create_response_arg( class, argument, &resp_argument ))
		{
		fputs(  "            add_status = moss_avl_add( *return_avl,\n",outfile);
		sprintf(buffer,
			"                                       &%sARG_CREATE_%s_DNA,\n",
                                     	class->prefix_m, resp_argument->argument_name+11 );
	        fputs(buffer, outfile);
		fputs(  "                                       (unsigned int) MAN_C_SUCCESS,\n",outfile);
	        sprintf(buffer,
			"                                       %s,\n", resp_argument->dna_data_type);
	        fputs(buffer, outfile);
		fputs(  "                                       octet);\n",outfile);
                fputs(  "            if ERROR_CONDITION( add_status )\n", outfile);
		fputs(  "                return add_status;\n\n", outfile);
		}	        
	    fputs("            break;\n\n", outfile);
	    break;

          case MG_C_OID :

	    sprintf(buffer,"            status = moss_octet_to_oid( octet, &instance->%s, );\n",
		   argument->argument_name+NAME_SIZE );
	    fputs(buffer, outfile);
    	    fputs("#ifdef MOMGENDEBUG\n",outfile);
	    sprintf(buffer,
		  "            printf(\" %s:\\n\" );\n", 
		   argument->argument_name+NAME_SIZE );
	    fputs(buffer, outfile);
    	    fputs("#endif /* MOMGENDEBUG */\n",outfile);
	    if (get_create_response_arg( class, argument, &resp_argument ))
		{
		fputs(  "            add_status = moss_avl_add( *return_avl,\n",outfile);
		sprintf(buffer,
			"                                       &%sARG_CREATE_%s_DNA,\n",
                                     	class->prefix_m, resp_argument->argument_name+11 );
	        fputs(buffer, outfile);
		fputs(  "                                       (unsigned int) MAN_C_SUCCESS,\n",outfile);
	        sprintf(buffer,
			"                                       %s,\n", resp_argument->dna_data_type);
	        fputs(buffer, outfile);
		fputs(  "                                       octet);\n",outfile);
                fputs(  "            if ERROR_CONDITION( add_status )\n", outfile);
		fputs(  "                return add_status;\n\n", outfile);
		}	        
	    fputs("            break;\n\n", outfile);
	    break;
	
          case MG_C_VERSION :
	    fputs("	/* VERSION */\n", outfile );        
	    sprintf(buffer,
		  "            new_%s_instance->%s = (char *)malloc(octet->length + 1);\n",
		   class->class_name,argument->argument_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    sprintf(buffer,
		  "            memcpy(new_%s_instance->%s,\n",
		   class->class_name,argument->argument_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs("                   octet->string,\n                   octet->length);\n", outfile);
	    sprintf(buffer,
		  "            new_%s_instance->%s[octet->length] = \'\\0\';\n",
		   class->class_name,argument->argument_name+NAME_SIZE);
	    fputs(buffer, outfile);
    	    fputs("#ifdef MOMGENDEBUG\n",outfile);
	    sprintf(buffer,
		  "            printf(\" %s:\t%%s\\n\", new_%s_instance->%s);\n",
		   argument->argument_name+NAME_SIZE,class->class_name,argument->argument_name+NAME_SIZE);
	    fputs(buffer, outfile);
    	    fputs("#endif /* MOMGENDEBUG */\n",outfile);
	    if (get_create_response_arg( class, argument, &resp_argument ))
		{
		fputs(  "            add_status = moss_avl_add( *return_avl,\n",outfile);
		sprintf(buffer,
			"                                       &%sARG_CREATE_%s_DNA,\n",
                                     	class->prefix_m, resp_argument->argument_name+11 );
	        fputs(buffer, outfile);
		fputs(  "                                       (unsigned int) MAN_C_SUCCESS,\n",outfile);
	        sprintf(buffer,
			"                                       %s,\n", resp_argument->dna_data_type);
	        fputs(buffer, outfile);
		fputs(  "                                       octet);\n",outfile);
                fputs(  "            if ERROR_CONDITION( add_status )\n", outfile);
		fputs(  "                return add_status;\n\n", outfile);
		}	        
	    fputs("            break;\n\n", outfile);

	    break;
	
	case MG_C_ACL :

	  fputs("	/* ACL */\n", outfile );        
	  if (argument->dna_cmip_int_len)
	    {    
	    fputs("            status = validate_acl( attribute_list, \n", outfile);
	    sprintf(buffer,
		      "                               &%sARGC_%s_DNA,\n",
		      class->prefix_m,argument->argument_name+NAME_SIZE );
	    fputs(buffer, outfile);
	      sprintf(buffer,
		  "                                   &new_%s_instance->%s,\n",
		   class->class_name,argument->argument_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    sprintf(buffer,
		      "                               &%sATTR_%s_DNA );\n",
		      class->prefix_m,argument->argument_name+NAME_SIZE );
	    fputs(buffer, outfile);
    	    fputs("#ifdef MOMGENDEBUG\n",outfile);
	    sprintf(buffer,
		  "            printf(\" %s:\\n\");\n",
		   argument->argument_name+NAME_SIZE);
	    fputs(buffer, outfile);

	    if (get_create_response_arg( class, argument, &resp_argument ))
		{
	    	sprintf(buffer,
		       "\n            moss_avl_reset( new_%s_instance->%s );\n",
			   class->class_name,argument->argument_name+NAME_SIZE);
		fputs(buffer, outfile);
	    	fputs( "            moss_avl_copy( *return_avl, \n", outfile);
	    	sprintf(buffer,
		       "                           new_%s_instance->%s,\n",
			   class->class_name,argument->argument_name+NAME_SIZE);
	    	fputs(buffer, outfile);
	    	fputs( "                           FALSE, \n", outfile);
	    	sprintf(buffer,
		       "                           &%sARG_CREATE_%s_DNA,\n",
                                     	class->prefix_m, resp_argument->argument_name+11 );
	    	fputs(buffer, outfile);
	    	fputs( "                           NULL, \n", outfile);
	    	fputs( "                           &last );\n", outfile);
		}
    	    fputs("#endif /* MOMGENDEBUG */\n",outfile);
	    }
	  else 
	    fputs("            /** validate ACL **/\n",outfile);

	  fputs("            break;\n\n", outfile);
	  break;

	case MG_C_AVL :

	    fputs("	/* AVL */\n", outfile ); 
	    if (argument->dna_cmip_int_len)
	        {
	        if (class->multiple_oids)
	            fputs("#ifdef DNA_CMIP_OID\n",outfile);
      	        if (arg_create_not_attribute( class, argument, &attribute ))
	           sprintf(buffer, 
		      "            status = copy_avl_attribute( &%sARGC_%s_DNA, attribute_list, &new_%s_instance->%s );\n",
		      class->prefix_m,argument->argument_name+NAME_SIZE,class->class_name,argument->argument_name+NAME_SIZE);
		else
	           sprintf(buffer, 
		      "            status = copy_avl_attribute( &%sATTR_%s_DNA, attribute_list, &new_%s_instance->%s );\n",
		      class->prefix_m,attribute->attribute_name+NAME_SIZE,class->class_name,attribute->attribute_name+NAME_SIZE);
	        fputs(buffer, outfile);
	    	if (get_create_response_arg( class, argument, &resp_argument ))
		    {
		    sprintf(buffer,
		           "\n            moss_avl_reset( new_%s_instance->%s );\n",
			       class->class_name,argument->argument_name+NAME_SIZE);
	            fputs(buffer, outfile);
		    fputs( "            moss_avl_copy( *return_avl, \n", outfile);
		    sprintf(buffer,
		           "                           new_%s_instance->%s,\n",
			       class->class_name,argument->argument_name+NAME_SIZE);
	            fputs(buffer, outfile);
		    fputs( "                           FALSE, \n", outfile);
		    sprintf(buffer,
		           "                           &%sARG_CREATE_%s_DNA,\n",
                                     	    class->prefix_m, resp_argument->argument_name+11 );
	            fputs(buffer, outfile);
		    fputs( "                           NULL, \n", outfile);
		    fputs( "                           &last );\n", outfile);
		    }	
	        if (class->multiple_oids)
		    fputs("#endif /* DNA_CMIP_OID */\n",outfile);
		}
	    if (argument->osi_oid_len)
	        {
		fputs("#ifdef OSI_CMIP_OID\n",outfile);
      	        if (arg_create_not_attribute( class, argument, &attribute ))
	           sprintf(buffer, 
		      "            status = copy_avl_attribute( &%sARGC_%s_OSI, attribute_list, &new_%s_instance->%s );\n",
		      class->prefix_m,argument->argument_name+NAME_SIZE,class->class_name,argument->argument_name+NAME_SIZE);
		else
	          sprintf(buffer,
		      "            status = copy_avl_attribute( &%sATTR_%s_OSI, attribute_list, &new_%s_instance->%s );\n",
		      class->prefix_m,attribute->attribute_name+NAME_SIZE,class->class_name,attribute->attribute_name+NAME_SIZE);
	        fputs(buffer, outfile);
	        if (get_create_response_arg( class, argument, &resp_argument ))
		    {
		    sprintf(buffer,
		           "\n            moss_avl_reset( new_%s_instance->%s );\n",
			       class->class_name,argument->argument_name+NAME_SIZE);
	            fputs(buffer, outfile);
		    fputs( "            moss_avl_copy( *return_avl, \n", outfile);
		    sprintf(buffer,
		           "                           new_%s_instance->%s,\n",
			       class->class_name,argument->argument_name+NAME_SIZE);
	            fputs(buffer, outfile);
		    fputs( "                           FALSE, \n", outfile);
		    sprintf(buffer,
		           "                           &%sARG_CREATE_%s_OSI,\n",
                                     	    class->prefix_m, resp_argument->argument_name+11 );
	            fputs(buffer, outfile);
		    fputs( "                           NULL, \n", outfile);
		    fputs( "                           &last );\n", outfile);
		    }
		fputs("#endif /* OSI_CMIP_OID */\n",outfile);
		}
	    if (argument->snmp_oid_len)
	        {
		fputs("#ifdef SNMP_OID\n",outfile);
      	        if (arg_create_not_attribute( class, argument, &attribute ))
	           sprintf(buffer, 
		      "            status = copy_avl_attribute( &%sARGC_%s_SNP, attribute_list, &new_%s_instance->%s );\n",
		      class->prefix_m,argument->argument_name+NAME_SIZE,class->class_name,argument->argument_name+NAME_SIZE);
		else
	           sprintf(buffer,
		      "            status = copy_avl_attribute( &%sATTR_%s_SNP, attribute_list, &new_%s_instance->%s );\n",
		      class->prefix_m,attribute->attribute_name+NAME_SIZE,class->class_name,attribute->attribute_name+NAME_SIZE);
	        fputs(buffer, outfile);
	        if (get_create_response_arg( class, argument, &resp_argument ))
		    {
		    sprintf(buffer,
		           "\n            moss_avl_reset( new_%s_instance->%s );\n",
			       class->class_name,argument->argument_name+NAME_SIZE);
	            fputs(buffer, outfile);
		    fputs( "            moss_avl_copy( *return_avl, \n", outfile);
		    sprintf(buffer,
		           "                           new_%s_instance->%s,\n",
			       class->class_name,argument->argument_name+NAME_SIZE);
	            fputs(buffer, outfile);
		    fputs( "                           FALSE, \n", outfile);
		    sprintf(buffer,
		           "                           &%sARG_CREATE_%s_SNP,\n",
                                     	    class->prefix_m, resp_argument->argument_name+11 );
	            fputs(buffer, outfile);
		    fputs( "                           NULL, \n", outfile);
		    fputs( "                           &last );\n", outfile);
		    }
		fputs("#endif /* SNMP_OID */\n",outfile);
		}
	    if (argument->oid_len)
	        {
		fputs("#ifdef OID\n",outfile);

      	        if (arg_create_not_attribute( class, argument, &attribute ))
	           sprintf(buffer, 
		      "            status = copy_avl_attribute( &%sARGC_%s_OID, attribute_list, &new_%s_instance->%s );\n",
		      class->prefix_m,argument->argument_name+NAME_SIZE,class->class_name,argument->argument_name+NAME_SIZE);
		else
	           sprintf(buffer,
		      "            status = copy_avl_attribute( &%sATTR_%s_OID, attribute_list, &new_%s_instance->%s );\n",
		      class->prefix_m,attribute->attribute_name+NAME_SIZE,class->class_name,attribute->attribute_name+NAME_SIZE);
	        fputs(buffer, outfile);
	        if (get_create_response_arg( class, argument, &resp_argument ))
		    {
		    sprintf(buffer,
		           "\n            moss_avl_reset( new_%s_instance->%s );\n",
			       class->class_name,argument->argument_name+NAME_SIZE);
	            fputs(buffer, outfile);
		    fputs( "            moss_avl_copy( *return_avl, \n", outfile);
		    sprintf(buffer,
		           "                           new_%s_instance->%s,\n",
			       class->class_name,argument->argument_name+NAME_SIZE);
	            fputs(buffer, outfile);
		    fputs( "                           FALSE, \n", outfile);
		    sprintf(buffer,
		           "                           &%sARG_CREATE_%s_OID,\n",
                                     	    class->prefix_m, resp_argument->argument_name+11 );
	            fputs(buffer, outfile);
		    fputs( "                           NULL, \n", outfile);
		    fputs( "                           &last );\n", outfile);
		    }
		fputs("#endif /* OID */\n",outfile);
		}
    	    fputs("#ifdef MOMGENDEBUG\n",outfile);
	    sprintf(buffer,"            printf(\" %s:\\n\" );\n", 
			argument->argument_name+NAME_SIZE );
	    fputs(buffer, outfile);
    	    fputs("#endif /* MOMGENDEBUG */\n",outfile);
	    fputs("            break;\n\n", outfile);
	    break;

	case MG_C_TIME :

	    fputs("	/* TIME */\n", outfile ); 
            sprintf(buffer, "            memcpy( &new_%s_instance->%s, octet->string, 16 );\n",
		      class->class_name,argument->argument_name+NAME_SIZE);
	    fputs(buffer, outfile);
    	    fputs("#ifdef MOMGENDEBUG\n",outfile);
	    sprintf(buffer,"            printf(\" %s:\\n\" );\n", 
			argument->argument_name+NAME_SIZE );
	    fputs(buffer, outfile);
    	    fputs("#endif /* MOMGENDEBUG */\n",outfile);
	    if (get_create_response_arg( class, argument, &resp_argument ))
		{
		fputs(  "            add_status = moss_avl_add( *return_avl,\n",outfile);
		sprintf(buffer,
			"                                       &%sARG_CREATE_%s_DNA,\n",
                                     	class->prefix_m, resp_argument->argument_name+11 );
	        fputs(buffer, outfile);
		fputs(  "                                       (unsigned int) MAN_C_SUCCESS,\n",outfile);
	        sprintf(buffer,
			"                                       %s,\n", resp_argument->dna_data_type);
	        fputs(buffer, outfile);
		fputs(  "                                       octet);\n",outfile);
                fputs(  "            if ERROR_CONDITION( add_status )\n", outfile);
		fputs(  "                return add_status;\n\n", outfile);
		}	        
	    fputs("            break;\n\n", outfile);
	    break;

	case MG_C_UID :

	    fputs("	/* UID */\n", outfile ); 
            sprintf(buffer, "            temp_uid = &new_%s_instance->%s;\n",
		      class->class_name,argument->argument_name+NAME_SIZE);
	    fputs(buffer, outfile);
            fputs(          "            status = moss_get_uid( &temp_uid );\n",outfile);
    	    fputs("#ifdef MOMGENDEBUG\n",outfile);
	    sprintf(buffer,"            printf(\" %s:\\n\" );\n", 
			argument->argument_name+NAME_SIZE );
	    fputs(buffer, outfile);
    	    fputs("#endif /* MOMGENDEBUG */\n",outfile);
	    if (get_create_response_arg( class, argument, &resp_argument ))
		{
		fputs(  "            add_status = moss_avl_add( *return_avl,\n",outfile);
		sprintf(buffer,
			"                                       &%sARG_CREATE_%s_DNA,\n",
                                     	class->prefix_m, resp_argument->argument_name+11 );
	        fputs(buffer, outfile);
		fputs(  "                                       (unsigned int) MAN_C_SUCCESS,\n",outfile);
	        sprintf(buffer,
			"                                       %s,\n", resp_argument->dna_data_type);
	        fputs(buffer, outfile);
		fputs(  "                                       octet);\n",outfile);
                fputs(  "            if ERROR_CONDITION( add_status )\n", outfile);
		fputs(  "                return add_status;\n\n", outfile);
		}	        
	    fputs("            break;\n\n", outfile);
	    break;

	case MG_C_BIT :
	    fputs("    /** BIT SET data type **/\n", outfile );        
	    sprintf(buffer,
		  "            new_%s_instance->%s = (unsigned int) *octet->string;\n"  ,
		   class->class_name,argument->argument_name+NAME_SIZE);
	    fputs(buffer, outfile);
    	    fputs("#ifdef MOMGENDEBUG\n",outfile);
	    sprintf(buffer,"            printf(\" %s:\\n\" );\n", 
			argument->argument_name+NAME_SIZE );
	    fputs(buffer, outfile);
    	    fputs("#endif /* MOMGENDEBUG */\n",outfile);

	    if (get_create_response_arg( class, argument, &resp_argument ))
		{
		fputs(  "            add_status = moss_avl_add( *return_avl,\n",outfile);
		sprintf(buffer,
			"                                       &%sARG_CREATE_%s_DNA,\n",
                                     	class->prefix_m, resp_argument->argument_name+11 );
	        fputs(buffer, outfile);
		fputs(  "                                       (unsigned int) MAN_C_SUCCESS,\n",outfile);
	        sprintf(buffer,
			"                                       %s,\n", resp_argument->dna_data_type);
	        fputs(buffer, outfile);
		fputs(  "                                       octet);\n",outfile);
                fputs(  "            if ERROR_CONDITION( add_status )\n", outfile);
		fputs(  "                return add_status;\n\n", outfile);
		}	        
	    fputs("            break;\n\n", outfile);
	    break;	

	default :
	    fputs("    /** Unknown data type **/\n", outfile );        
	    sprintf(buffer,
		  "            status = copy_octet_to_unsigned_int( &new_%s_instance->%s, octet );\n"  ,
		   class->class_name,argument->argument_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs("    /** Verify data type **/\n\n",outfile);
	    if (get_create_response_arg( class, argument, &resp_argument ))
		{
		fputs(  "            add_status = moss_avl_add( *return_avl,\n",outfile);
		sprintf(buffer,
			"                                       &%sARG_CREATE_%s_DNA,\n",
                                     	class->prefix_m, resp_argument->argument_name+11);
	        fputs(buffer, outfile);
		fputs(  "                                       (unsigned int) MAN_C_SUCCESS,\n",outfile);
	        sprintf(buffer,
			"                                       %s,\n", resp_argument->dna_data_type);
	        fputs(buffer, outfile);
		fputs(  "                                       octet);\n",outfile);
                fputs(  "            if ERROR_CONDITION( add_status )\n", outfile);
		fputs(  "                return add_status;\n\n", outfile);
		}	        
	    fputs("            break;\n\n", outfile);
	}

    argument = argument->next;
    }

    return SS$_NORMAL;
}

int get_create_response_arg( class,
	       	    	     check_arg,
	       	    	     resp_arg )

CLASS_DEF *class;
ARGUMENT_DEF *check_arg;
ARGUMENT_DEF **resp_arg;


{
DIRECTIVE_DEF *dir;
RESPONSE_DEF  *response;
ARGUMENT_DEF *argument;

dir = class->directive;
while (dir != NULL)
    if (strcmp( dir->directive,"CREATE") == 0)
        break;
    else 
	dir = dir->next;

if (dir == NULL)
    return 0;

/*
 * Found the CREATE directive, now look at all of the response arguments.
 */

response = dir->resp;

while (response != NULL)
  {
  argument = response->arg;

  while (argument != NULL)
    {
    if (argument->original_name != NULL) 
      {
      if (check_arg->original_name != NULL)
	{
        if (strcmp((check_arg->original_name),(argument->original_name)) == 0)
	    {
            *resp_arg = argument;
            return 1;
	    }
	}
      else 
	if (strcmp((check_arg->argument_name+NAME_SIZE),(argument->original_name)) == 0)
	    {
            *resp_arg = argument;
            return 1;
	    }
      }
    else      
	/*
	 * NOTE: If the argument original name is not NULL then the check_arg
	 * should never be null since the prefix for check-arg is argc_ and
	 * prefix for argument is arg_create_
	 */
        if (strcmp((check_arg->argument_name+NAME_SIZE),(argument->argument_name+11)) == 0)
	    {
            *resp_arg = argument;
            return 1;
	    }

   argument = argument->next;
   }
  response = response->next;
  }

return 0;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_name_oid
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-name-oid tag.
**
** 	This routine generates the identifier name OID.
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
int insert_name_oid( mom,
		     outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    ATTRIBUTE_DEF   *attribute;
    int		    attribute_num;
    CLASS_DEF	    *class;
    char	    buffer[256];
    int		    status;
    int 	    found_one = 0;

    class = mom->current_class;					      
    attribute = class->attribute;

    /* 
     * Generate NULL CLASS OID if class does not suport instances
     */

    if (!class->support_instances)  
	{
        if (class->dna_cmip_int_len)
	    {
	    if (class->multiple_oids)
		    fputs("#ifdef DNA_CMIP_OID \n",outfile);
            sprintf(buffer,"                               &%sNULL_DNA,\n",
		   class->prefix);
            fputs(buffer, outfile);	    
	    if (class->multiple_oids)
		    fputs("#endif /* DNA_CMIP_INT */\n",outfile);		
	    }

	if (class->snmp_oid_len)
	    {
	    fputs("#ifdef SNMP_OID\n",outfile);
            sprintf(buffer,"                               &%sNULL_SNP,\n",
		   class->prefix);
            fputs(buffer, outfile);	    
	    fputs("#endif /* SNMP_OID */\n",outfile);		
	    }

	if (class->osi_oid_len)
	    {
	    fputs("#ifdef OSI_CMIP_OID\n",outfile);
            sprintf(buffer,"                               &%sNULL_OSI,\n",
		   class->prefix);
            fputs(buffer, outfile);	    
	    fputs("#endif /* OSI_CMIP_OID */\n",outfile);		
	    }

	if (class->oid_len)
	    {
	    fputs("#ifdef OID\n",outfile);
            sprintf(buffer,"                               &%sNULL_OID,\n",
		   class->prefix);
            fputs(buffer, outfile);	    
	    fputs("#endif /* OID */\n",outfile);		
	    }

	return SS$_NORMAL;
	}	

    for (attribute_num=0; attribute_num < class->num_attributes; attribute_num++)
    {
	if (attribute->group == GROUP_IDENTIFIERS)
	    {
	    if (found_one)
		{
		fputs("/** Multiple identifier OIDs found. Verify the correct OID **/\n",outfile);
		return SS$_NORMAL;
		}
            if (attribute->dna_cmip_int_len)
		{
	        if (class->multiple_oids)
		    fputs("#ifdef DNA_CMIP_OID \n",outfile);
                sprintf(buffer,"                               &%s%s_DNA,\n",
		   class->prefix_m,attribute->attribute_name);
            	fputs(buffer, outfile);	    
	        if (class->multiple_oids)
		    fputs("#endif /* DNA_CMIP_OID */\n",outfile);		
		}
            if (attribute->osi_oid_len)
		{
		fputs("#ifdef OSI_CMIP_OID\n",outfile);
                sprintf(buffer,"                               &%s%s_OSI,\n",
		   class->prefix_m,attribute->attribute_name);
            	fputs(buffer, outfile);	    
		fputs("#endif /* OSI_CMIP_OID */\n",outfile);		
		}
            if (attribute->snmp_oid_len)
		{
		fputs("#ifdef SNMP_OID\n",outfile);
                sprintf(buffer,"                               &%s%s_SNP,\n",
		   class->prefix_m,attribute->attribute_name);
            	fputs(buffer, outfile);	    
		fputs("#endif /* SNMP_OID */\n",outfile);		
		}
            if (attribute->oid_len)
		{
		fputs("#ifdef OID\n",outfile);
                sprintf(buffer,"                               &%s%s_OID,\n",
		   class->prefix_m,attribute->attribute_name);
            	fputs(buffer, outfile);	    
		fputs("#endif /* OID */\n",outfile);		
		}
	    found_one = 1;
	    }	 
	attribute = attribute->next;
    }
    
    /*
     * Not found; print a comment...
     */

     fputs("				/** Need to supply identifier attribute OID **/\n",outfile);
     fputs("                                          /** &ATTR_NAME_DNA **/\n", outfile );

    return( SS$_NORMAL );
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_instance_oid
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file in support of the create_select_locate routine.
**
** 	This routine generates the identifier name OID.
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
int insert_instance_oid( mom,
		         outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    ATTRIBUTE_DEF   *attribute;
    int		    attribute_num;
    CLASS_DEF	    *class;
    char	    buffer[256];
    int		    status;
    int 	    found_one = 0;

    class = mom->current_class;					      
    attribute = class->attribute;

    /* 
     * Generate NULL CLASS OID if class does not suport instances
     */

    if (!class->support_instances)  
	{
        if (class->dna_cmip_int_len)
	    {
	    if (class->multiple_oids)
		    fputs("#ifdef DNA_CMIP_OID \n",outfile);
            sprintf(buffer,"            instance_oid = &%sNULL_DNA;\n",
		   class->prefix);
            fputs(buffer, outfile);	    
	    if (class->multiple_oids)
		    fputs("#endif /* DNA_CMIP_INT */\n",outfile);		
	    }

	if (class->snmp_oid_len)
	    {
	    fputs("#ifdef SNMP_OID\n",outfile);
            sprintf(buffer,"            instance_oid = &%sNULL_SNP;\n",
		   class->prefix);
            fputs(buffer, outfile);	    
	    fputs("#endif /* SNMP_OID */\n",outfile);		
	    }

	if (class->osi_oid_len)
	    {
	    fputs("#ifdef OSI_CMIP_OID\n",outfile);
            sprintf(buffer,"            instance_oid = &%sNULL_OSI;\n",
		   class->prefix);
            fputs(buffer, outfile);	    
	    fputs("#endif /* OSI_CMIP_OID */\n",outfile);		
	    }

	if (class->oid_len)
	    {
	    fputs("#ifdef OID\n",outfile);
            sprintf(buffer,"            instance_oid = &%sNULL_OID;\n",
		   class->prefix);
            fputs(buffer, outfile);	    
	    fputs("#endif /* OID */\n",outfile);		
	    }

	return SS$_NORMAL;
	}	

    for (attribute_num=0; attribute_num < class->num_attributes; attribute_num++)
    {
	if (attribute->group == GROUP_IDENTIFIERS)
	    {
	    if (found_one)
		{
		fputs("/** Multiple identifier OIDs found. Verify the correct OID **/\n",outfile);
		return SS$_NORMAL;
		}
            if (attribute->dna_cmip_int_len)
		{
	        if (class->multiple_oids)
		    fputs("#ifdef DNA_CMIP_OID \n",outfile);
                sprintf(buffer,"            instance_oid = &%s%s_DNA;\n",
		   class->prefix_m,attribute->attribute_name);
            	fputs(buffer, outfile);	    
	        if (class->multiple_oids)
		    fputs("#endif /* DNA_CMIP_OID */\n",outfile);		
		}
            if (attribute->osi_oid_len)
		{
		fputs("#ifdef OSI_CMIP_OID\n",outfile);
                sprintf(buffer,"            instance_oid = &%s%s_OSI;\n",
		   class->prefix_m,attribute->attribute_name);
            	fputs(buffer, outfile);	    
		fputs("#endif /* OSI_CMIP_OID */\n",outfile);		
		}
            if (attribute->snmp_oid_len)
		{
		fputs("#ifdef SNMP_OID\n",outfile);
                sprintf(buffer,"            instance_oid = &%s%s_SNP;\n",
		   class->prefix_m,attribute->attribute_name);
            	fputs(buffer, outfile);	    
		fputs("#endif /* SNMP_OID */\n",outfile);		
		}
            if (attribute->oid_len)
		{
		fputs("#ifdef OID\n",outfile);
                sprintf(buffer,"            instance_oid = &%s%s_OID;\n",
		   class->prefix_m,attribute->attribute_name);
            	fputs(buffer, outfile);	    
		fputs("#endif /* OID */\n",outfile);		
		}
	    found_one = 1;
	    }	 
	attribute = attribute->next;
    }
    
    /*
     * Not found; print a comment...
     */

     fputs("				/** Need to supply identifier attribute OID **/\n",outfile);
     fputs("                                          /** &ATTR_NAME_DNA **/\n", outfile );

    return( SS$_NORMAL );
}
