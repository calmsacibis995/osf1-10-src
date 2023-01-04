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
static char *rcsid = "@(#)$RCSfile: mg_struct.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/06/15 11:55:43 $";
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
**	This module contains the routines to used to generate the
**	[[class]]_DEF structure.
**
**  AUTHORS:
**
**      Gary Allison
**
**  CREATION DATE:  23-Sep-1991
**
**  MODIFICATION HISTORY:
**
**  	Marc Nozell 	15-Jun-1992 	Remove reference to module.
**	Mike Densmore	21-May-1992	Modified for use on U**X platforms.
**
**	Mike Densmore	25-Sep-1992	Modified for ANSI Standard C
**
**--
*/

#ifdef VMS
#include "vaxcshr.h"		/* translate from VAX C to DEC C RTL names */
#endif 
#define NAME_SIZE 5

#include "mom_defs.h"
#include "mg_prototypes.h"
#include <string.h>

#if defined(sun) || defined(sparc)
void	insert_definitions();
#else
void	insert_definitions( MOM_BUILD *, FILE *);
#endif


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_define_version
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file.
**
**	This routine generates the version for each class.
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
int insert_define_version( mom,
	    	           outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    CLASS_DEF	    *class;
    int		    class_num;
    char	    buffer[256];
    int		    status;

    class = mom->class;

    fputs("#ifdef VMS\n",outfile);
    for (class_num=0; class_num < mom->num_classes; class_num++)
    {
	sprintf(buffer,
		"#define %s%s_version \"X\\001\\000\\000\"\n", 
			class->prefix_k, class->class_name );
	fputs(buffer, outfile);

	class = class->next;
    }
    fputs("#endif /* VMS */\n",outfile);

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_define_classes
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file.
**
**	This routine generates the "define" statements to assign each
**	class the MOM supports a simple scalar value for later use.
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
int insert_define_classes( mom,
	    	           outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    CLASS_DEF	    *class;
    int		    class_num;
    char	    buffer[256];
    int		    status;

    class = mom->class;

    for (class_num=0; class_num < mom->num_classes; class_num++)
    {
	sprintf(buffer,
		"#define %s%s_ID %d\n", class->prefix_k, class->class_name, class_num+1);
	fputs(buffer, outfile);

	class = class->next;
    }

    insert_definitions( mom, outfile );

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_definitions
**
**	This routine generates define statements for all of the symbol
**	definitions for both MIR and MCC dictionaries.. This is generated 
**	for the CA MIR since the MIR does not generate a .H file. It is 
**	also useful for the MCC users since now they will not have to make 
** 	the "symbol = " changes (and they will not have to be documented.
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
void	insert_definitions( mom,	
		            outfile )

MOM_BUILD *mom;
FILE *outfile;

{
    char 		buffer[256];
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
    /* Make sure that no prefix K's start at 0 */
    int		   	unique_number=1;

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
    {
        fputs("\n", outfile);
	sprintf(buffer, "#define %s%s %d\n", class->prefix_k, 
		class->class_name, unique_number++);
	fputs(buffer, outfile);
	
	if (class->dna_cmip_int_len)
	    {
	    sprintf(buffer, "#define %s%s %d\n", class->prefix_dna, 
		class->class_name, class->dna_cmip_int);
	    fputs(buffer, outfile);
	    }	

	attribute = class->attribute;
	for (attribute_num=0; attribute_num < class->num_attributes; attribute_num++)
	    {
	    sprintf(buffer, "#define %s%s %d", class->prefix_k, 
		attribute->attribute_name, unique_number++);
	    fputs(buffer, outfile);
	    if (attribute->original_name != NULL)
		{
	        sprintf(buffer, "	/* %s */\n", attribute->original_name);
	        fputs(buffer, outfile);
		}
	    else
		fputs("\n",outfile);
		
	    if (attribute->dna_cmip_int_len)
	        {
		sprintf(buffer, "#define %s%s %d\n", class->prefix_dna, 
		    attribute->attribute_name, attribute->dna_cmip_int);
	    	fputs(buffer, outfile);
		}
	    fputs("\n",outfile);
	    attribute = attribute->next;
	    }

	event = class->event;
	for (event_num=0; event_num < class->num_events; event_num++)
	    {
	    sprintf(buffer, "#define %s%s %d", class->prefix_k, 
		    event->event_name, unique_number++);
	    fputs(buffer, outfile);

	    if (event->original_name != NULL)
		{
	 	sprintf(buffer, "	/* %s */\n", event->original_name);
	        fputs(buffer, outfile);
		}
	    else
		fputs("\n",outfile);
		
	    if (event->dna_cmip_int_len)
		{
	    	sprintf(buffer, "#define %s%s %d\n", class->prefix_dna, 
		    event->event_name, event->dna_cmip_int);
	    	fputs(buffer, outfile);
		}
	    fputs("\n",outfile);

	    if (event->num_event_args)
	      {
	      argument = event->event_arg;
	      for (argument_num=0; argument_num < event->num_event_args; argument_num++)
    		{
	        sprintf(buffer, "#define %s%s %d", class->prefix_k, 
		    argument->argument_name, unique_number++);
		fputs(buffer, outfile);
	    	if (argument->original_name != NULL)
		    {
	            sprintf(buffer,"	/* %s */\n", argument->original_name);
	            fputs(buffer, outfile);
		    }
		else
		    fputs("\n",outfile);

		if (argument->dna_cmip_int_len)
		    {
	            sprintf(buffer, "#define %s%s %d\n", class->prefix_dna, 
		        argument->argument_name,argument->dna_cmip_int);
		    fputs(buffer, outfile);
		    }		
	        fputs("\n",outfile);
		argument = argument->next;
		}
              }
	    event = event->next;
	    }

	directive = class->directive;
	for (directive_num=0; directive_num < class->num_directives; directive_num++)
	{
	    sprintf(buffer, "#define %s%s %d\n", class->prefix_k, 
		        directive->directive_name, unique_number++ );
	    fputs(buffer, outfile);

	    if (directive->dna_cmip_int_len)
		{
	    	sprintf(buffer, "#define %s%s %d\n", class->prefix_dna, 
		        directive->directive_name, directive->dna_cmip_int);
	    	fputs(buffer, outfile);
		}

            if (directive->num_responses)
		{
		response = directive->resp;
		for (response_num=0; response_num < directive->num_responses; response_num++)
		    {
	            sprintf(buffer, "#define %s%s %d", class->prefix_k, 
			     response->response_name , unique_number++);
		    fputs(buffer, outfile);

	    	    if (response->original_name != NULL)
		     	 {
	 	         sprintf(buffer, "	/* %s */\n", response->original_name);
	            	 fputs(buffer, outfile);
		    	 }
		    else
			fputs("\n",outfile);

		    if (response->dna_cmip_int_len)
			{
	            	sprintf(buffer, "#define %s%s %d\n", class->prefix_dna, 
			     response->response_name , response->dna_cmip_int);
		    	fputs(buffer, outfile);
			}
		
		    fputs("\n",outfile);

		    if (response->num_arguments)
		        {
	      		argument = response->arg;
	      		for (argument_num=0; argument_num < response->num_arguments; argument_num++)
    		 	    {
	        	    sprintf(buffer, "#define %s%s %d", class->prefix_k, 
		    	    argument->argument_name, unique_number++);
		            fputs(buffer, outfile);
	    	            if (argument->original_name != NULL)
		    		{
	            		sprintf(buffer,"	/* %s */\n", argument->original_name);
	            		fputs(buffer, outfile);
		    		}
			    else
				fputs("\n",outfile);

			    if (argument->dna_cmip_int_len)
		    		{
	            		sprintf(buffer, "#define %s%s %d\n", class->prefix_dna, 
		        		argument->argument_name,argument->dna_cmip_int);
		    		fputs(buffer, outfile);
		    		}		
			    fputs("\n",outfile);
			    argument = argument->next;
			    }
			}
		    response = response->next;
		    }
	  	}
            if (directive->num_exceptions)
		{
		exception = directive->exc;
		for (exception_num=0; exception_num < directive->num_exceptions; exception_num++)
		    {
	            sprintf(buffer, "#define %s%s %d", class->prefix_k, 
			     exception->exception_name, unique_number++);
		    fputs(buffer, outfile);
	    	    if (exception->original_name != NULL)
		     	 {
	 	         sprintf(buffer, "	/* %s */\n", exception->original_name);
	            	 fputs(buffer, outfile);
		    	 }
		    else
			fputs("\n",outfile);
		
		    if (exception->dna_cmip_int_len)
			{
	            	sprintf(buffer, "#define %s%s %d\n", class->prefix_dna, 
			     exception->exception_name, exception->dna_cmip_int);
		    	fputs(buffer, outfile);
			}
		    fputs("\n",outfile);

		    if (exception->num_arguments)
		        {
	      		argument = exception->arg;
	      		for (argument_num=0; argument_num < exception->num_arguments; argument_num++)
    		 	    {
	        	    sprintf(buffer, "#define %s%s %d", class->prefix_k, 
		    	    argument->argument_name, unique_number++);
		            fputs(buffer, outfile);
	    	            if (argument->original_name != NULL)
		    		{
	            		sprintf(buffer,"	/* %s */\n", argument->original_name);
	            		fputs(buffer, outfile);
		    		}
			    else
				fputs("\n",outfile);

			    if (argument->dna_cmip_int_len)
		    		{
	            		sprintf(buffer, "#define %s%s %d\n", class->prefix_dna, 
		        		argument->argument_name,argument->dna_cmip_int);
		    		fputs(buffer, outfile);
		    		}		
			    fputs("\n",outfile);
			    argument = argument->next;
			    }
			}

		    exception = exception->next;
		    }
	  	}
            if (directive->num_requests)
		{
		request = directive->req;
		for (request_num=0; request_num < directive->num_requests; request_num++)
		    {
		    if (request->num_arguments)
		        {
	      		argument = request->arg;
	      		for (argument_num=0; argument_num < request->num_arguments; argument_num++)
    		 	    {
	        	    sprintf(buffer, "#define %s%s %d", class->prefix_k, 
		    	    argument->argument_name, unique_number++);
		            fputs(buffer, outfile);
	    	            if (argument->original_name != NULL)
		    		{
	            		sprintf(buffer,"	/* %s */\n", argument->original_name);
	            		fputs(buffer, outfile);
		    		}
			    else
				fputs("\n",outfile);

			    if (argument->dna_cmip_int_len)
		    		{
	            		sprintf(buffer, "#define %s%s %d\n", class->prefix_dna, 
		        		argument->argument_name,argument->dna_cmip_int);
		    		fputs(buffer, outfile);
		    		}		
			    fputs("\n",outfile);
			    argument = argument->next;
			    }
			}
		    request = request->next;
		    }
		}		
	  directive = directive->next;
	  }	

    	if (class->num_create_args)
            {
            argument = class->create_arg;
            for (argument_num=0; argument_num < class->num_create_args; argument_num++)
	        {
	        sprintf(buffer, "#define %s%s %d", class->prefix_k, 
		argument->argument_name, unique_number++);
		fputs(buffer, outfile);
	    	if (argument->original_name != NULL)
		    {
	            sprintf(buffer,"	/* %s */\n", argument->original_name);
	            fputs(buffer, outfile);
		    }
		else
		    fputs("\n",outfile);

		if (argument->dna_cmip_int_len)
		    {
	            sprintf(buffer, "#define %s%s %d\n", class->prefix_dna, 
		        argument->argument_name,argument->dna_cmip_int);
		    fputs(buffer, outfile);
		    }		
	        fputs("\n",outfile);
	        argument = argument->next;
	        }
	    }
    class = class->next;
    }
}

/*
*++
**  FUNCTIONAL DESCRIPTION:
**
**      create_arg_not_attribute
**
**	This function returns true if an argument is not found in the
**	attribute list.
**
**  FORMAL PARAMETERS:
**
**      class		Pointer to current class.
**	argument	Pointer to argument.
**
**  RETURN VALUE:
**
**      1 if not found
**	0 if found 
**
**--
*/
int create_arg_not_attribute( class,
		       	      argument )

CLASS_DEF *class;
ARGUMENT_DEF *argument;

{
ATTRIBUTE_DEF *attribute;

attribute = class->attribute;
while (attribute != NULL)
    {
    if ((argument->original_name != NULL) && (attribute->original_name != NULL))
	{
        if (strcmp((attribute->original_name),(argument->original_name)) == 0)
		return 0;
	}
    else      
        if (strcmp((attribute->attribute_name+NAME_SIZE),(argument->argument_name+NAME_SIZE)) == 0)
		return 0;

   attribute = attribute->next;
   }

return 1;
}

/*
*++
**  FUNCTIONAL DESCRIPTION:
**
**      arg_create_not_attribute
**
**	This function returns true if an argument is not found in the
**	attribute list.
**
**  FORMAL PARAMETERS:
**
**      class		Pointer to current class.
**	argument	Pointer to argument.
**	attribute	Address of pointer of attribute to return.
**
**  RETURN VALUE:
**
**      1 if not found
**	0 if found 
**
**--
*/
int arg_create_not_attribute( class,
		       	      argument,
			      attr )

CLASS_DEF *class;
ARGUMENT_DEF *argument;
ATTRIBUTE_DEF **attr;

{
ATTRIBUTE_DEF *attribute;

attribute = class->attribute;
while (attribute != NULL)
    {
    *attr = attribute;
    if (argument->original_name != NULL) 
      {
      if (attribute->original_name != NULL)
	{
        if (strcmp((attribute->original_name),(argument->original_name)) == 0)
		return 0;
	}
      else 
	{
        if (strcmp((attribute->attribute_name+NAME_SIZE),(argument->original_name)) == 0)
		return 0;
	}
      }
    else      
        if (strcmp((attribute->attribute_name+NAME_SIZE),(argument->argument_name+NAME_SIZE)) == 0)
		return 0;

   attribute = attribute->next;
   }

return 1;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_defs
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on insert-code-defs tag.
**
**	This routine generates the [[class_def]] structure in the common.h
**	include file. It includes a definition for each attribute with the
**	name and UID hardcoded in (to make sure that they're always there).
**	For UNIXPLATFORM, generates the typedef for the global trap varbind
**	list structures.
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
int insert_defs( mom,
		 outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    ARGUMENT_DEF    *argument;
    int		    argument_num;
    ATTRIBUTE_DEF   *attribute;
    int 	    class_num;
    int		    attribute_num;
    CLASS_DEF	    *class;
    char	    buffer[256];
    int		    chars = 0;
    int		    status;
    EVENT_DEF       *event;
    int             event_num;
    int             arg_name_offset = strlen("ARG_EVT_");

#ifdef UNIXPLATFORM

    /*
     * Generate the typedef of global trap varbind list structures.
     * Note that event_name starts with "EVT_".
     */

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
        {
        event = class->event;

        for (event_num=0; event_num < class->num_events; event_num++)
            {
            fputs("typedef struct _", outfile);
            fputs((char *)&event->event_name[strlen("EVT_")], outfile);
            fputs("_TDEF {\n", outfile);

            argument = event->event_arg;

            for (argument_num=0; argument_num < event->num_event_args;
                                                            argument_num++)
                {
                if (argument->argument_number > 5)
                    {
                    switch (argument->mg_type)
                      {
                      case MG_C_BOOLEAN :
                      case MG_C_INTEGER :
                          if (argument->sign == MG_C_UNSIGNED)
                          {
                              if (argument->original_name == NULL)
                              {
                                  sprintf(buffer, "    unsigned int %s;\n",
                                    argument->argument_name+arg_name_offset);
                              }
                              else
                              {
                                  sprintf(buffer, "    unsigned int %s;\n",
                                    argument->original_name);
                              }
                          }
                          else
                          {
                              if (argument->original_name == NULL)
                              {
                                  sprintf(buffer, "    int %s;\n",
                                    argument->argument_name+arg_name_offset);
                              }
                              else
                              {
                                  sprintf(buffer, "    int %s;\n",
                                    argument->original_name);
                              }
                          }
                          fputs(buffer, outfile);
                          break;

                      case MG_C_STRING :
                          if (argument->original_name == NULL)
                          {
                              sprintf(buffer, "    char *%s;\n",
                                argument->argument_name+arg_name_offset);
                              fputs(buffer, outfile);
                              sprintf(buffer, "    unsigned int %s_len;\n",
                                argument->argument_name+arg_name_offset);
                              fputs(buffer, outfile);
                          }
                          else
                          {
                              sprintf(buffer, "    char *%s;\n",
                                argument->original_name);
                              fputs(buffer, outfile);
                              sprintf(buffer, "    unsigned int %s_len;\n",
                                argument->original_name);
                              fputs(buffer, outfile);
                          }
                          break;

                      case MG_C_OID :
                          if (argument->original_name == NULL)
                          {
                              sprintf(buffer, "    object_id *%s;\n",
                                argument->argument_name+arg_name_offset);
                          }
                          else
                          {
                              sprintf(buffer, "    object_id *%s;\n",
                                argument->original_name);
                          }
                          fputs(buffer, outfile);
                          break;

                      case MG_C_VERSION :
                      case MG_C_NULL :
                          if (argument->original_name == NULL)
                          {
                              sprintf(buffer, "    char *%s;\n",
                                argument->argument_name+arg_name_offset);
                          }
                          else
                          {
                              sprintf(buffer, "    char *%s;\n",
                                argument->original_name);
                          }
                          fputs(buffer, outfile);
                          break;
	    
                      case MG_C_ACL :
                      case MG_C_AVL :
                          if (argument->original_name == NULL)
                          {
                              sprintf(buffer, "    avl *%s;\n",
                                argument->argument_name+arg_name_offset);
                          }
                          else
                          {
                              sprintf(buffer, "    avl *%s;\n",
                                argument->original_name);
                          }
                          fputs(buffer, outfile);
                          break;

                      case MG_C_TIME :
                          if (argument->original_name == NULL)
                          {
                              sprintf(buffer, "    mo_time %s;\n",
                                argument->argument_name+arg_name_offset);
                          }
                          else
                          {
                              sprintf(buffer, "    mo_time %s;\n",
                                argument->original_name);
                          }
                          fputs(buffer, outfile);
                          break;
	
                      case MG_C_UID :
                          if (argument->original_name == NULL)
                          {
                              sprintf(buffer, "    uid  %s;\n",
                                argument->argument_name+arg_name_offset);
                          }
                          else
                          {
                              sprintf(buffer, "    uid  %s;\n",
                                argument->original_name);
                          }
                          fputs(buffer, outfile);
                          break;

                      case MG_C_BIT :
                          if (argument->original_name == NULL)
                          {
                              sprintf(buffer, "    unsigned int %s;",
                                argument->argument_name+arg_name_offset);
                          }
                          else
                          {
                              sprintf(buffer, "    unsigned int %s;",
                                argument->original_name);
                          }
                          fputs(buffer, outfile);
                          fputs(" /** May need to modify this BIT STRING definition **/\n",outfile); 	
                          break;

                      default :
                          /*  UNKNOWN ARGUMENT -- define it anyway with a comment.. */
                          if (argument->original_name == NULL)
                          {
                              sprintf(buffer, "    unsigned int %s; /** warning: argument %s - unknown datatype **/\n", 
                                argument->argument_name+arg_name_offset,
                                argument->argument_name+arg_name_offset);
                          }
                          else
                          {
                              sprintf(buffer, "    unsigned int %s; /** warning: argument %s - unknown datatype **/\n", 
                                argument->original_name,
                                argument->original_name);
                          }
                          fputs(buffer, outfile);
                          break;
                      }	
                    }

                argument = argument->next;
                }

            fputs("    char *enterprise;\n", outfile);
            fputs("    unsigned int enterprise_length;\n", outfile);
            fputs("    char *agent_addr;\n", outfile);
            fputs("    unsigned int agent_addr_length;\n", outfile);
            fputs("    unsigned int time_stamp;\n", outfile);
            fputs("    int time_stamp_valid;\n", outfile);
            fputs("    unsigned int instance_name_length;\n", outfile);
            fputs("    char *instance_name;\n", outfile);
            fputs("    } ", outfile);
            fputs((char *)&event->event_name[strlen("EVT_")], outfile);
            fputs("_TDEF;\n\n", outfile);

            event = event->next;
            }

        class = class->next;
        }

#endif

class = mom->class;

for (class_num=0; class_num < mom->num_classes; class_num++)
  {
   
    sprintf(buffer,
    "typedef struct _%s_DEF {\n", class->class_name);
    fputs(buffer, outfile);

    sprintf(buffer,
    "    struct _%s_DEF *next;\n", class->class_name);
    fputs(buffer, outfile);

    sprintf(buffer,
    "    struct _%s_DEF *prev;\n", class->class_name);
    fputs(buffer, outfile);

    attribute = class->attribute;

    /*
     * NOTE: Make sure any new data types are added for BOTH loops (attributes
     *	     and create arguments.
     */
    for (attribute_num=0; attribute_num < class->num_attributes; attribute_num++)
    {
      switch (attribute->mg_type)
        {
	case MG_C_BOOLEAN :
	case MG_C_INTEGER :
	    if (attribute->sign == MG_C_UNSIGNED)
	        sprintf(buffer, "    unsigned int %s;\n", attribute->attribute_name+NAME_SIZE);
	    else
	        sprintf(buffer, "    int %s;\n", attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
            break;

        case MG_C_STRING :
	    sprintf(buffer, 
		"    char *%s;\n", attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    sprintf(buffer, 
		"    unsigned int %s_len;\n", attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
            break;

	case MG_C_OID :
	    sprintf(buffer,
		"    object_id *%s;\n", attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
            break;

	case MG_C_VERSION :
	case MG_C_NULL :
	    sprintf(buffer,
	        "    char *%s;\n", attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
            break;
	    
	case MG_C_ACL :
	case MG_C_AVL :
	    sprintf(buffer,
	        "    avl *%s;\n", attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
            break;

	case MG_C_TIME :
            sprintf(buffer,
		"    mo_time %s;\n", attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
            break;
	
        case MG_C_UID :
	    sprintf(buffer,
		"    uid  %s;\n", attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
            break;

	case MG_C_BIT :
	    sprintf(buffer,
		"    unsigned int %s;", attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
            fputs(" /** May need to modify this BIT STRING definition **/\n",outfile); 	
            break;

        default :
	    /*  UNKNOWN ATTRIBUTE -- define it anyway with a comment.. */
	    sprintf(buffer,
		"    unsigned int %s; /** warning: attribute %s - unknown datatype **/\n", 
		attribute->attribute_name+NAME_SIZE,attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
            break;
	}	

      attribute = attribute->next;
      }

    argument = class->create_arg;
    for (argument_num=0; argument_num < class->num_create_args; argument_num++)
     {
     if (create_arg_not_attribute( class, argument ))
      {
/**
      fprintf(mom->log_file,"\n*** Warning -- no attribute associated with argument %s ***\n\n",
		argument->argument_name+NAME_SIZE);	
**/
      switch (argument->mg_type)
        {
	case MG_C_BOOLEAN :
	case MG_C_INTEGER :
	    sprintf(buffer, 
		"    unsigned int %s;\n", argument->argument_name+NAME_SIZE);
	    fputs(buffer, outfile);
            break;

        case MG_C_STRING :
	    sprintf(buffer, 
		"    char *%s;\n", argument->argument_name+NAME_SIZE);
	    fputs(buffer, outfile);
            break;

	case MG_C_OID :
	    sprintf(buffer,
		"    object_id *%s;\n", argument->argument_name+NAME_SIZE);
	    fputs(buffer, outfile);
            break;

	case MG_C_ACL :
	case MG_C_AVL :
	    sprintf(buffer,
	        "    avl *%s;\n", argument->argument_name+NAME_SIZE);
	    fputs(buffer, outfile);
            break;

	case MG_C_TIME :
            sprintf(buffer,
		"    mo_time %s;\n", argument->argument_name+NAME_SIZE);
	    fputs(buffer, outfile);
            break;
	
        case MG_C_UID :
	    sprintf(buffer,
		"    uid  %s;\n", argument->argument_name+NAME_SIZE);
	    fputs(buffer, outfile);
            break;

	case MG_C_BIT :
	    sprintf(buffer,
		"    unsigned int %s;", argument->argument_name+NAME_SIZE);
	    fputs(buffer, outfile);
            fputs(" /** Need to modify this BIT STRING definition **/\n",outfile); 	
            break;

        default :
	    /*  UNKNOWN ARGUMENT -- define it anyway with a comment.. */
	    sprintf(buffer,
		"    unsigned int %s; /** warning: argument %s - unknown datatype **/\n", 
		argument->argument_name+NAME_SIZE,argument->argument_name+NAME_SIZE);
	    fputs(buffer, outfile);
            break;
	}	
      }
    argument = argument->next;
    }

   fputs("    unsigned int instance_name_length;\n", outfile);
   fputs("    char *instance_name;\n", outfile);
   fputs("    uid   instance_uid;\n", outfile);
   fputs("    avl  *object_instance;\n",outfile);
   sprintf(buffer,
    "    } %s_DEF;\n", class->class_name);
   fputs(buffer, outfile);

   class = class->next;
   }

fputs("typedef struct _GET_CONTEXT_STRUCT {\n",outfile);
fputs("    char		*search_name;\n",outfile);
fputs("    int		 search_name_length;\n",outfile);
fputs("    int		 first_time;\n",outfile);
fputs("	   int 		 more_instances;\n",outfile);
class = mom->class;
for (class_num=0; class_num < mom->num_classes; class_num++)
   {
   sprintf(buffer,"    %s_DEF	*%s;\n",class->class_name,class->class_name_ptr);
   fputs(buffer, outfile);
   class = class->next;
   }
fputs("/** Context structure.  Add any required user-defined fields. **/\n",outfile);
fputs("} GET_CONTEXT_STRUCT;\n",outfile);

return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_extern_common
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on tag insert-code-extern-common.
**
**	This routine generates the code needed to IMPORT the instance header
**	and for UNIXPLATFORM, to IMPORT the global trap varbind list
**	structures.
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

int insert_extern_common( mom,
		          outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    int		    class_num;
    CLASS_DEF	    *class;
    char	    buffer[256];
    int		    status;
    EVENT_DEF       *event;
    int             event_num;

#ifdef UNIXPLATFORM

    /*
     * Generate the IMPORT of global trap varbind list structures.
     * Note that event_name starts with "EVT_".
     */

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
        {
        event = class->event;

        for (event_num=0; event_num < class->num_events; event_num++)
            {
            fputs("  IMPORT ", outfile);
            fputs((char *)&event->event_name[strlen("EVT_")], outfile);
            fputs("_TDEF ", outfile);
            fputs((char *)&event->event_name[strlen("EVT_")], outfile);
            fputs("_traplist;\n", outfile);

            event = event->next;
        }

        class = class->next;
        }

    fputs("\n", outfile);

#endif

    class = mom->class;

    for (class_num=0; class_num < mom->num_classes; class_num++)
    {
	sprintf(buffer,"  IMPORT %s_DEF *new_%s_header;\n",
			class->class_name, class->class_name );
	fputs(buffer, outfile);
        class = class->next;
    }
    return SS$_NORMAL;

} /* End of insert_extern_common */
