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
static char *rcsid = "@(#)$RCSfile: mg_traps.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/07/13 22:56:32 $";
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
**  FACILITY:  MOM Generator
**
**  MODULE DESCRIPTION:
**
**	This module contains the routines to used to generate the
**	trap structure initialization and trap processing code.
**
**  AUTHORS:
**
**     Muhammad  I.  Ashraf 
**
**  CREATION DATE:   9-Mar-1993
**
**  MODIFICATION HISTORY:
**
**
**--
*/

#include "mom_defs.h"
#include <string.h>
#include "mg_prototypes.h"

#define  trap_suffix	"_traplist"


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  insert_trap_export_defs
**
**  Generates a codes section for inclusion in the current MOM Generator
**  output file based on insert-trap-export-defs tag.
**  This routine generates the EXPORT definitions for all the events
**  defined for this mom for the UNIXPLATFORM.
**
*/
int insert_trap_export_defs (mom ,
                 outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    ARGUMENT_DEF    *argument;
    int             argument_num;
    ATTRIBUTE_DEF   *attribute;
    int             class_num;
    int             attribute_num;
    CLASS_DEF       *class;
    char            buffer[256];
    int             chars = 0;
    int             status;
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
            fputs("EXPORT ", outfile);
            fputs((char *)&event->event_name[strlen("EVT_")], outfile);
            fputs("_TDEF ", outfile);
            fputs((char *)&event->event_name[strlen("EVT_")], outfile);
            fputs("_traplist;\n", outfile);

            event = event->next;
	}

        class = class->next;
    }

#endif

    return SS$_NORMAL;
} /* End of insert_trap_export_defs */ 

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_trap_arg_inits
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on insert-code-trap-arg-inits tag.
**
**	This routine initializes the trap structure, <trap>_TDEF arguments
**	to all 0's or NULL values. 
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
int insert_trap_arg_inits( mom,
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
     * Generate the code for each class and event to initialize the
     * respective arguments.  Note that event_name starts with "EVT_".
     */

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
        {
        event = class->event;

        for (event_num=0; event_num < class->num_events; event_num++)
            {
            fputs("    /*\n", outfile);
	    fputs("     * Initialize ", outfile); 
            fputs((char *)&event->event_name[strlen("EVT_")], outfile);
            fputs(trap_suffix, outfile);
            fputs(" of type\n", outfile);
	    fputs("     * ", outfile);
            fputs((char *)&event->event_name[strlen("EVT_")], outfile);
            fputs("_TDEF for this trap.\n", outfile);
	    fputs("     */\n", outfile);

            argument = event->event_arg;

            for (argument_num=0; argument_num < event->num_event_args;
                                                            argument_num++)
                {
                if (argument->argument_number > 5)
                    {
                    switch (argument->mg_type)
                      {
                      case MG_C_BOOLEAN :
                              if (argument->original_name == NULL)
                              {
                                  sprintf(buffer, "    %s%s.%s = FALSE;\n",
			            (char *)&event->event_name[strlen("EVT_")],
				    trap_suffix,
                                    argument->argument_name+arg_name_offset);
                              }
                              else
                              {
                                  sprintf(buffer, "    %s%s.%s = FALSE;\n",
                                    (char *)&event->event_name[strlen("EVT_")],
                                    trap_suffix,
                                    argument->original_name);
                              }
                          fputs(buffer, outfile);
                          break;

                      case MG_C_INTEGER :
                              if (argument->original_name == NULL)
                              {
                                  sprintf(buffer, "    %s%s.%s = 0;\n",
			            (char *)&event->event_name[strlen("EVT_")],
				    trap_suffix,
                                    argument->argument_name+arg_name_offset);
                              }
                              else
                              {
                                  sprintf(buffer, "    %s%s.%s = 0;\n",
                                    (char *)&event->event_name[strlen("EVT_")],
                                    trap_suffix,
                                    argument->original_name);
                              }
                          fputs(buffer, outfile);
                          break;

                      case MG_C_STRING :
                          if (argument->original_name == NULL)
                          {
                              sprintf(buffer, "    %s%s.%s = NULL;\n",
                                (char *)&event->event_name[strlen("EVT_")],
                                trap_suffix,
                                argument->argument_name+arg_name_offset);
                              fputs(buffer, outfile);
                              sprintf(buffer, "    %s%s.%s_len = 0;\n",
                                (char *)&event->event_name[strlen("EVT_")],
                                trap_suffix,
                                argument->argument_name+arg_name_offset);
                          }
                          else
                          {
                              sprintf(buffer, "    %s%s.%s = NULL;\n",
                                (char *)&event->event_name[strlen("EVT_")],
                                trap_suffix,
                                argument->original_name);
                              fputs(buffer, outfile);
                              sprintf(buffer, "    %s%s.%s_len = 0;\n",
                                (char *)&event->event_name[strlen("EVT_")],
                                trap_suffix,
                                argument->original_name);
                          }
                          fputs(buffer, outfile);
                          break;

                      case MG_C_OID :
                          if (argument->original_name == NULL)
                          {
                              sprintf(buffer, "    %s%s.%s = NULL;\n",
                                (char *)&event->event_name[strlen("EVT_")],
                                trap_suffix,
                                argument->argument_name+arg_name_offset);
                          }
                          else
                          {
                              sprintf(buffer, "    %s%s.%s = NULL;\n",
                                (char *)&event->event_name[strlen("EVT_")],
                                trap_suffix,
                                argument->original_name);
                          }
                          fputs(buffer, outfile);
                          break;

                      default :
                          /*  UNKNOWN ARGUMENT -- define it anyway with a comment.. */
                          if (argument->original_name == NULL)
                          {
                              sprintf(buffer, "    %s%s.%s = 0; /** warning: argument %s - unknown datatype **/\n",
			        (char *)&event->event_name[strlen("EVT_")],
			        trap_suffix,
                                argument->argument_name+arg_name_offset,
                                argument->argument_name+arg_name_offset);
                          }
                          else
                          {
                              sprintf(buffer, "    %s%s.%s = 0; /** warning: argument %s - unknown datatype **/\n",
			        (char *)&event->event_name[strlen("EVT_")],
			        trap_suffix,
                                argument->original_name,
                                argument->original_name);
                          }
                          fputs(buffer, outfile);
                          break;
                      }	
                    }

                argument = argument->next;
                }

	/*
         * Write out initialization for fixed elements of *_TDEF
         */
            sprintf(buffer, "    %s%s.enterprise = NULL;\n",
              (char *)&event->event_name[strlen("EVT_")],
              trap_suffix);
            fputs(buffer, outfile);
            sprintf(buffer, "    %s%s.enterprise_length = 0;\n",
              (char *)&event->event_name[strlen("EVT_")],
              trap_suffix);
            fputs(buffer, outfile);

            sprintf(buffer, "    %s%s.agent_addr = NULL;\n",
              (char *)&event->event_name[strlen("EVT_")],
              trap_suffix);
            fputs(buffer, outfile);

            sprintf(buffer, "    %s%s.agent_addr_length = 0;\n",
              (char *)&event->event_name[strlen("EVT_")],
              trap_suffix);
            fputs(buffer, outfile);

            sprintf(buffer, "    %s%s.time_stamp = 0;\n",
              (char *)&event->event_name[strlen("EVT_")],
              trap_suffix);
            fputs(buffer, outfile);

            sprintf(buffer, "    %s%s.time_stamp_valid = FALSE;\n",
              (char *)&event->event_name[strlen("EVT_")],
              trap_suffix);
            fputs(buffer, outfile);

            sprintf(buffer, "    %s%s.instance_name_length = 0;\n",
              (char *)&event->event_name[strlen("EVT_")],
              trap_suffix);
            fputs(buffer, outfile);

            sprintf(buffer, "    %s%s.instance_name = NULL;\n",
              (char *)&event->event_name[strlen("EVT_")],
              trap_suffix);
            fputs(buffer, outfile);

            event = event->next;
            }

        class = class->next;
        }

#endif

    return SS$_NORMAL;

} /* End of insert_trap_arg_inits */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_trap_arg_code
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file. 
**
**	This routine generates code for building a trap argument.  It 
**	builds trap-specific arguments required by send_trap() routine in 
**	MOM. 
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
int insert_trap_arg_code( mom,
                          outfile)
MOM_BUILD *mom;
FILE *outfile;
{
    ARGUMENT_DEF    *argument;
    int             argument_num;
    ATTRIBUTE_DEF   *attribute;
    int             class_num;
    int             attribute_num;
    CLASS_DEF       *class;
    char            buffer[256];
    int             chars = 0;
    int             status;
    EVENT_DEF       *event;
    int             event_num;
    int             arg_name_offset = strlen("ARG_EVT_");
    char            full_arg_name[256], full_ARG_EVT_name[256];
    char            *event_str, *arg_str;
    char            data_type_str[40];

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
        event_str = (char *)&event->event_name[strlen("EVT_")];
        fputs("\f\n", outfile);
        fputs("/*\n", outfile);
        fputs("**++\n", outfile);
        fputs("**  FUNCTIONAL DESCRIPTION:\n", outfile);
        fputs("**\n", outfile);
        fputs("**      send_", outfile);
        fputs(event_str, outfile);
        fputs("_trap\n", outfile);
        fputs("**\n", outfile);
        fputs("**      This routine sends a ", outfile);
        fputs(event_str, outfile);
        fputs(" trap. It builds the arguments\n", outfile);
        fputs("**      required by send_trap() routine using the global trap variable\n", outfile);
        fputs("**      list structure called ", outfile);
        fputs(event_str, outfile);
        fputs("_traplist defined in this module.\n", outfile);
        fputs("**\n", outfile);
        fputs("**  RETURN VALUES:\n", outfile);
        fputs("**\n", outfile);
        fputs("**      MAN_C_SUCCESS\n", outfile);
        fputs("**\n", outfile);
        fputs("**  SIDE EFFECTS:\n", outfile);
        fputs("**\n", outfile);
        fputs("**      None\n", outfile);
        fputs("**\n", outfile);
        fputs("**--\n", outfile);
        fputs("*/\n", outfile);
	sprintf( buffer, "man_status send_%s_trap()\n",
		 event_str);
	fputs(buffer, outfile);
        fputs("{\n", outfile);
        fputs("    man_status status = MAN_C_SUCCESS;\n", outfile);
        fputs("    avl *instance_name_avl = NULL;\n", outfile);
        fputs("    avl *event_parameters_avl = NULL;\n", outfile);
        fputs("    octet_string octet;\n", outfile);
        fputs("    int generic_trap = enterprise_specific_trap;\n", outfile);
        fputs(" \n", outfile);
        fputs("#ifdef MOMGENDEBUG\n", outfile);
        fputs("    printf(\"entering send_", outfile);
        fputs(event_str, outfile);
        fputs("_trap()\"\\n\\n);\n", outfile);
        fputs("#endif\n\n", outfile);

	sprintf(buffer, "        if ((%s%s.instance_name_length == 0) ||\n",
	        event_str,
		trap_suffix);
	fputs(buffer, outfile);
	sprintf(buffer, "            (%s%s.instance_name == NULL))\n",
	        event_str,
		trap_suffix);
	fputs(buffer, outfile);

        fputs("    {\n", outfile);
        fputs("        instance_name_avl = nil_avl;\n", outfile);
        fputs("    }\n", outfile);
        fputs("    else\n", outfile);
        fputs("    {\n", outfile);
        fputs("        status = moss_avl_init(&instance_name_avl);\n", outfile);
        fputs("        if ERROR_CONDITION(status)\n", outfile);
        fputs("            return status;\n", outfile);
        fputs("\n", outfile);
	sprintf(buffer, "        octet.length = %s%s.instance_name_length;\n",
	        event_str,
		trap_suffix);
	fputs(buffer, outfile);
	sprintf(buffer, "        octet.string = (char *) %s%s.instance_name;\n",
	        event_str,
		trap_suffix);
	fputs(buffer, outfile);
        fputs("        octet.data_type = ASN1_C_INTEGER;\n\n", outfile);
        fputs("        status = moss_avl_add(instance_name_avl,\n", outfile);
        sprintf(buffer, "                              &%s_SNP,      /* ? */\n",
		class->class_name);
        fputs(buffer, outfile);
        fputs("                              MAN_C_SUCCESS,\n", outfile);
        fputs("                              ASN1_C_INTEGER,\n", outfile);
        fputs("                              &octet);\n", outfile);
        fputs("    }\n\n", outfile);
        fputs("    if (status == MAN_C_SUCCESS)\n    {\n", outfile);
        fputs("        status = moss_avl_init(&event_parameters_avl);\n", outfile);
        fputs("    }\n\n", outfile);
        fputs("        /* Build event argument */\n\n", outfile);
        fputs("    if (status == MAN_C_SUCCESS)\n    {\n", outfile);
        fputs("        octet.length = sizeof(int);\n", outfile);
        fputs("        octet.data_type = ASN1_C_INTEGER;\n", outfile);
        fputs("        octet.string = (char *)\n", outfile);
        sprintf(buffer, "        &%s_%s_SNP.value[%s_%s_SNP.count-1];\n\n",
	    class->class_name,
	    (char *)event->event_name,
            class->class_name,
	    (char *)event->event_name);
        fputs(buffer, outfile);

        fputs("        status = moss_avl_add(event_parameters_avl,\n", outfile);
        sprintf(buffer, "                              &%s_%s_SNP,\n",
	    class->class_name,
	    (char *)event->event_name);
        fputs(buffer, outfile);
        fputs("                              MAN_C_SUCCESS,\n", outfile);
        fputs("                              ASN1_C_INTEGER,\n", outfile);
        fputs("                              &octet);\n", outfile);
        fputs("    }\n\n", outfile);

	/*
	 * Build the first 5 arguments from the argument 
	 * structure for this event 
	 */
        argument = event->event_arg;

        for (argument_num=0; argument_num < 5; argument_num++)
        { 
          if (argument->original_name != NULL)
              arg_str = (char *)&argument->original_name[0];
          else
              arg_str = (char *)&argument->argument_name[strlen("ARG_EVT_")];

          sprintf(full_arg_name, "%s%s.%s",
		    event_str, trap_suffix, arg_str);
          sprintf(full_ARG_EVT_name, "%s_%s",
		    class->class_name, (char *)argument->argument_name);

          sprintf(buffer, "        /* Build %s's %s argument */\n\n",
		    event_str, arg_str);
          fputs(buffer, outfile);
          fputs("    if (status == MAN_C_SUCCESS)\n", outfile);
          fputs("    {\n", outfile);

          switch (argument->argument_number)
          {
	  case 1:        /* Build enterprise argument */
             sprintf(buffer, "        if ((%s_length == 0) ||\n",  full_arg_name);
             fputs(buffer, outfile);
             sprintf(buffer, "            (%s == NULL))\n",   full_arg_name);
             fputs(buffer, outfile);
             fputs("        {\n", outfile);
             sprintf(buffer, "            octet.length = %s_SNP.count * sizeof(unsigned int);\n",
             class->class_name);
             fputs(buffer, outfile);
             fputs("            octet.data_type = ASN1_C_OCTET_STRING;\n", outfile);
             sprintf(buffer,"            octet.string = (char *) %s_SNP.value;\n",
                class->class_name);
             fputs(buffer, outfile);
             fputs("        }\n", outfile);
             fputs("        else\n", outfile);
             fputs("        {\n", outfile);
             sprintf(buffer, "            octet.length = %s_length;\n",
                full_arg_name);
             fputs(buffer, outfile);
             fputs("            octet.data_type = ASN1_C_OCTET_STRING;\n", outfile);
             sprintf(buffer, "            octet.string = (char *) %s;\n", full_arg_name);
             fputs(buffer, outfile);
             fputs("        }\n\n", outfile);
	          strcpy(data_type_str, "ASN1_C_OCTET_STRING");
	     break;

	  case 2:        /* Build agent_addr argument */
             sprintf(buffer, "        if ((%s_length == 0) ||\n",  full_arg_name);
             fputs(buffer, outfile);
             sprintf(buffer, "            (%s == NULL))\n",   full_arg_name);
             fputs(buffer, outfile);
             fputs("        {\n", outfile);
             fputs("            octet.length = 0;\n", outfile);
             fputs("            octet.data_type = ASN1_C_OCTET_STRING;\n", outfile);
             fputs("            octet.string = NULL;\n", outfile);
             fputs("        }\n", outfile);
             fputs("        else\n", outfile);
             fputs("        {\n", outfile);
             sprintf(buffer, "            octet.length = %s_length;\n",
                full_arg_name);
             fputs(buffer, outfile);
             fputs("            octet.data_type = ASN1_C_OCTET_STRING;\n", outfile);
             sprintf(buffer, "            octet.string = (char *) %s;\n", 
		full_arg_name);
             fputs(buffer, outfile);
             fputs("        }\n\n", outfile);
	          strcpy(data_type_str, "ASN1_C_OCTET_STRING");
	     break;

	  case 3:        /* Build generic_trap argument */
             fputs("        octet.length = sizeof(int);\n", outfile);
             fputs("        octet.data_type = ASN1_C_INTEGER;\n", outfile);
             fputs("        octet.string = (char *) &generic_trap;\n", outfile);
	          strcpy(data_type_str, "ASN1_C_INTEGER");
	     break;

	  case 4:        /* Build specific_trap argument */
             fputs("        octet.length = sizeof(int);\n", outfile);
             fputs("        octet.data_type = ASN1_C_INTEGER;\n", outfile);
             fputs("        octet.string = (char *)\n", outfile);
             sprintf(buffer,"        &%s_%s_SNP.value[%s_%s_SNP.count-1];\n",
                     class->class_name,
	             (char *)event->event_name,
                     class->class_name,
	             (char *)event->event_name);
             fputs(buffer, outfile);
	          strcpy(data_type_str, "ASN1_C_INTEGER");
	     break;

	  case 5:        /* Build time_stamp argument */
             sprintf(buffer, "        if (%s_valid == TRUE)\n",  full_arg_name);
             fputs(buffer, outfile);
             fputs("        {\n", outfile);
             fputs("            octet.length = sizeof (unsigned int);\n", outfile);
             fputs("            octet.data_type = ASN1_C_INTEGER;\n", outfile);
             sprintf(buffer,"            octet.string = (char *) %s;\n",
                full_arg_name);
             fputs(buffer, outfile);
             fputs("        }\n", outfile);
             fputs("        else\n", outfile);
             fputs("        {\n", outfile);
             fputs("            octet.length = 0;\n", outfile);
             fputs("            octet.data_type = ASN1_C_INTEGER;\n", outfile);
             fputs("            octet.string = NULL;\n", outfile);
             fputs("        }\n\n", outfile);
	          strcpy(data_type_str, "ASN1_C_INTEGER");
	     break;

          } /* end of switch */

          fputs("            status = moss_avl_add(event_parameters_avl,\n", outfile);
          sprintf(buffer, "                              &%s_SNP,\n",
		full_ARG_EVT_name);
          fputs(buffer, outfile);
          fputs("                              MAN_C_SUCCESS,\n", outfile);
          sprintf(buffer, "                              %s,\n", 
	        data_type_str);
          fputs(buffer, outfile);
          fputs("                              &octet);\n", outfile);
          fputs("    }\n\n", outfile);
        
          argument = argument->next;
        } 
	/*
         * Build code for the varbind arguments in the event
         */

        for (argument_num=5; argument_num < event->num_event_args;
							argument_num++)
        { 
          if (argument->argument_number > 5)
          {
          if (argument->original_name != NULL)
              arg_str = (char *)&argument->original_name[0];
          else
              arg_str = (char *)&argument->argument_name[strlen("ARG_EVT_")];

          sprintf(full_arg_name, "%s%s.%s",
		    event_str, trap_suffix, arg_str);
          sprintf(full_ARG_EVT_name, "%s_%s",
		    class->class_name, (char *)argument->argument_name);

           sprintf(buffer, "        /* Build %s's varbind argument, %s */\n\n",
		    event_str, arg_str);
           fputs(buffer, outfile);
           fputs("    if (status == MAN_C_SUCCESS)\n", outfile);
           fputs("    {\n", outfile);

           switch (argument->mg_type)
           {
             case MG_C_INTEGER :
                 if (argument->sign == MG_C_UNSIGNED)
                 {
	           fputs("        status = copy_unsigned_int_to_octet (\n", outfile);
		   fputs("                               &octet,\n", outfile);
                   sprintf(buffer, "                               &%s);\n\n",
		     full_arg_name);
                   fputs(buffer, outfile);
                 }
	         else
                 {
                   fputs("        status = copy_signed_int_to_octet (\n", outfile);
                   fputs("                               &octet,\n",
outfile);
                   sprintf(buffer, "                               &%s);\n\n",
                     full_arg_name);
                   fputs(buffer, outfile);
                   fputs("        octet.length = sizeof (int);\n", outfile);
                 }
                 fputs("        octet.data_type = ASN1_C_INTEGER;\n\n", outfile);
                 fputs("        status = moss_avl_add(event_parameters_avl,\n", outfile);
                 sprintf(buffer, "                              &%s_SNP,\n",
			 full_ARG_EVT_name);
                 fputs(buffer, outfile);
                 fputs("                              MAN_C_SUCCESS,\n", outfile);

                 if (argument->dna_data_type != NULL)
                 {
                   if (strncmp(argument->dna_data_type,
                               "INET_C_SMI",
                               strlen("INET_C_SMI")) == 0)
                   {
                     fputs("                              ", outfile);
                     fputs(argument->dna_data_type, outfile);
                     fputs(",\n", outfile);
                   }
                   else
                   {
                     fputs("                              ASN1_C_INTEGER,\n", outfile);
                   }
                 }
                 else
                 {
                   fputs("                              ASN1_C_INTEGER,\n", outfile);
                 }

                 fputs("                              &octet);\n", outfile);
                 fputs("    }\n\n", outfile);

                 break;

             case MG_C_STRING  :
	         sprintf(buffer, "        if ((%s_len != 0) &&\n",
		   full_arg_name);
                 fputs(buffer, outfile);
	         sprintf(buffer, "            (%s != NULL))\n",
		   full_arg_name);
                 fputs(buffer, outfile);
                 fputs("          {\n", outfile);
	         sprintf(buffer, "            octet.length = %s_len;\n",
                     full_arg_name);
                 fputs(buffer, outfile);
	         sprintf(buffer, "            octet.string = %s;\n",
                     full_arg_name);
                 fputs(buffer, outfile);
	         fputs("            octet.data_type = ASN1_C_OCTET_STRING;\n", outfile);
                 fputs("        status = moss_avl_add(event_parameters_avl,\n", outfile);
                 sprintf(buffer, "                              &%s_SNP,\n",
			 full_ARG_EVT_name);
                 fputs(buffer, outfile);
                 fputs("                              MAN_C_SUCCESS,\n", outfile);

                 if (argument->dna_data_type != NULL)
                 {
                   if (strncmp(argument->dna_data_type,
                               "INET_C_SMI",
                               strlen("INET_C_SMI")) == 0)
                   {
                     fputs("                              ", outfile);
                     fputs(argument->dna_data_type, outfile);
                     fputs(",\n", outfile);
                   }
                   else
                   {
                     fputs("                              ASN1_C_OCTET_STRING,\n", outfile);
                   }
                 }
                 else
                 {
                   fputs("                              ASN1_C_OCTET_STRING,\n", outfile);
                 }

                 fputs("                              &octet);\n", outfile);
                 fputs("        }\n\n", outfile);
                 fputs("    }\n\n", outfile);
                 break;
 
             case MG_C_BOOLEAN :
                 fputs("            octet.length = sizeof (int);\n", outfile);
	         sprintf(buffer, "            octet.string = (char *) &%s%s.%s;\n",
                         event_str,  trap_suffix,  arg_str);
                 fputs(buffer, outfile);
	         sprintf("            octet.data_type = ASN1_C_%s;\n",
		         argument->data_type);
                 fputs(buffer, outfile);
                 fputs("        status = moss_avl_add(event_parameters_avl,\n", outfile);
                 sprintf(buffer, "                              &%s_SNP,\n",
			 full_ARG_EVT_name);
                 fputs(buffer, outfile);
                 fputs("                              MAN_C_SUCCESS,\n", outfile);
                 sprintf(buffer, "                              ASN1_C_%s;\n",
                         argument->data_type);
                 fputs(buffer, outfile);
                 fputs("                              &octet);\n", outfile);
                 fputs("    }\n\n", outfile);
                 break;
 
             case MG_C_OID :
	         sprintf(buffer, "        if (%s != NULL)\n",
		   full_arg_name);
                 fputs(buffer, outfile);
                 fputs("        status = moss_oid_to_octet (\n", outfile);
                 sprintf(buffer, "        %s, \n",
                   full_arg_name);
                 fputs(buffer, outfile);
                 fputs("                   &octet);\n", outfile);
	         fputs("            octet.data_type = ASN1_C_OBJECT_ID;\n", outfile);
                 fputs("        status = moss_avl_add(event_parameters_avl,\n", outfile);
                 sprintf(buffer, "                              &%s_SNP,\n",
			 full_ARG_EVT_name);
                 fputs(buffer, outfile);
                 fputs("                              MAN_C_SUCCESS,\n", outfile);
                 fputs("                              ASN1_C_OBJECT_ID,\n", outfile);
                 fputs("                              &octet);\n", outfile);
                 fputs("    }\n\n", outfile);
                 break;
 
           } /* End of switch */

          }

          argument = argument->next;
        }

	/*  Generate code to send this trap */
 
        fputs("    if (status == MAN_C_SUCCESS)\n", outfile);
        fputs("    {\n", outfile);
	sprintf(buffer, "        status = send_trap(&%s_SNP,\n",
		class->class_name);
        fputs(buffer, outfile);
        fputs("                           instance_name_avl,\n", outfile);
        fputs("                           &event_type_oid,\n", outfile);
        fputs("                           event_parameters_avl\n", outfile);
        fputs("                          );\n", outfile);
        fputs("    }\n\n", outfile);
        fputs("    if ((instance_name_avl != nil_avl) && (instance_name_avl != NULL))\n", outfile);
        fputs("        moss_avl_free(&instance_name_avl, TRUE);\n\n", outfile);
        fputs("    if (event_parameters_avl != NULL)\n", outfile);
        fputs("        moss_avl_free(&event_parameters_avl, TRUE);\n\n", outfile);
        fputs("    return status;\n\n", outfile);
        sprintf( buffer, "} /* End of send_%s_trap */\n\n",
		event_str);
        fputs(buffer, outfile);

        event = event->next;
      }

    class = class->next;
    }

#endif
    return SS$_NORMAL;

} /* End of insert_trap_arg_code */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  insert_trap_cond
**
**  Generates a codes section for inclusion in the current MOM Generator
**  output file based on insert-code-trap-cond tag.
**  This routine generates the polling conditions for the events into
**  the routine poll_for_trap_conditions().
**
*/
int insert_trap_cond (mom ,
                 outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    CLASS_DEF       *class;
    int             class_num;
    EVENT_DEF       *event;
    int             event_num;
    char            buffer[256];

#ifdef UNIXPLATFORM

    /*
     * Generate the polling condition of the form trap_condition_x
     * where x is the event number defined in the mib.
     */

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
        {
        event = class->event;

        for (event_num=0; event_num < class->num_events; event_num++)
            {
            fputs("    int trap_condition_", outfile);
            sprintf(buffer, "%d", event->event_number);
            fputs(buffer, outfile);
            fputs(" = FALSE;\n", outfile);

            event = event->next;
	}

        class = class->next;
    }

#endif

    return SS$_NORMAL;
} /* End of insert_trap_cond */ 

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_trap_polling
**
**  Generates a codes section for inclusion in the current MOM Generator
**  output file based on insert-code-trap-polling tag.
**  This routine generates the polling and sending for the events into
**  the routine poll_for_trap_conditions().
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
int insert_trap_polling( mom,
		 outfile)
MOM_BUILD *mom;
FILE *outfile;

{
    ARGUMENT_DEF    *argument;
    int		    argument_num;
    CLASS_DEF	    *class;
    int 	    class_num;
    char	    buffer[256];
    EVENT_DEF       *event;
    int             event_num;
    int             arg_name_offset = strlen("ARG_EVT_");

#ifdef UNIXPLATFORM

    /*
     * Generate the polling and sending of trap for each class and event
     * and the setting for each argument.
     * Note that event_name starts with "EVT_".
     */

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
        {
        event = class->event;

        for (event_num=0; event_num < class->num_events; event_num++)
            {
            fputs("        /*\n", outfile);
            fputs("         * If trap condition for ", outfile);
            fputs((char *)&event->event_name[strlen("EVT_")], outfile);
            fputs(" trap is TRUE\n", outfile);
            fputs("         * initialize the structure ", outfile);
            fputs((char *)&event->event_name[strlen("EVT_")], outfile);
            fputs(trap_suffix, outfile);
            fputs(" of type\n", outfile);
            fputs("         * ", outfile);
            fputs((char *)&event->event_name[strlen("EVT_")], outfile);
            fputs("_TDEF for this trap.\n", outfile);
            fputs("         *\n", outfile);
            fputs("         * Call send_", outfile);
            fputs((char *)&event->event_name[strlen("EVT_")], outfile);
            fputs("_trap() routine to send the trap.\n", outfile);
            fputs("         */\n\n", outfile);

            fputs("        if ( trap_condition_", outfile);
            sprintf(buffer, "%d", event->event_number);
            fputs(buffer, outfile);
            fputs(" == TRUE )\n", outfile);
            fputs("        {\n", outfile);

            argument = event->event_arg;

            for (argument_num=0; argument_num < event->num_event_args;
                                                            argument_num++)
                {
                if (argument->argument_number > 5)
                    {
                    switch (argument->mg_type)
                      {
                      case MG_C_BOOLEAN :
                              if (argument->original_name == NULL)
                              {
                                  sprintf(buffer, "            %s%s.%s = FALSE;\n",
			            (char *)&event->event_name[strlen("EVT_")],
				    trap_suffix,
                                    argument->argument_name+arg_name_offset);
                              }
                              else
                              {
                                  sprintf(buffer, "            %s%s.%s = FALSE;\n",
                                    (char *)&event->event_name[strlen("EVT_")],
                                    trap_suffix,
                                    argument->original_name);
                              }
                          fputs(buffer, outfile);
                          break;

                      case MG_C_INTEGER :
                              if (argument->original_name == NULL)
                              {
                                  sprintf(buffer, "            %s%s.%s = 0;\n",
			            (char *)&event->event_name[strlen("EVT_")],
				    trap_suffix,
                                    argument->argument_name+arg_name_offset);
                              }
                              else
                              {
                                  sprintf(buffer, "            %s%s.%s = 0;\n",
                                    (char *)&event->event_name[strlen("EVT_")],
                                    trap_suffix,
                                    argument->original_name);
                              }
                          fputs(buffer, outfile);
                          break;

                      case MG_C_STRING :
                          if (argument->original_name == NULL)
                          {
                              sprintf(buffer, "            %s%s.%s = NULL;\n",
                                (char *)&event->event_name[strlen("EVT_")],
                                trap_suffix,
                                argument->argument_name+arg_name_offset);
                              fputs(buffer, outfile);
                              sprintf(buffer, "            %s%s.%s_len = 0;\n",
                                (char *)&event->event_name[strlen("EVT_")],
                                trap_suffix,
                                argument->argument_name+arg_name_offset);
                          }
                          else
                          {
                              sprintf(buffer, "            %s%s.%s = NULL;\n",
                                (char *)&event->event_name[strlen("EVT_")],
                                trap_suffix,
                                argument->original_name);
                              fputs(buffer, outfile);
                              sprintf(buffer, "            %s%s.%s_len = 0;\n",
                                (char *)&event->event_name[strlen("EVT_")],
                                trap_suffix,
                                argument->original_name);
                          }
                          fputs(buffer, outfile);
                          break;

                      case MG_C_OID :
                          if (argument->original_name == NULL)
                          {
                              sprintf(buffer, "            %s%s.%s = NULL;\n",
                                (char *)&event->event_name[strlen("EVT_")],
                                trap_suffix,
                                argument->argument_name+arg_name_offset);
                          }
                          else
                          {
                              sprintf(buffer, "            %s%s.%s = NULL;\n",
                                (char *)&event->event_name[strlen("EVT_")],
                                trap_suffix,
                                argument->original_name);
                          }
                          fputs(buffer, outfile);
                          break;

                      default :
                          /*  UNKNOWN ARGUMENT -- define it anyway with a comment.. */
                          if (argument->original_name == NULL)
                          {
                              sprintf(buffer, "            %s%s.%s = 0; /** warning: argument %s - unknown datatype **/\n",
			        (char *)&event->event_name[strlen("EVT_")],
			        trap_suffix,
                                argument->argument_name+arg_name_offset,
                                argument->argument_name+arg_name_offset);
                          }
                          else
                          {
                              sprintf(buffer, "            %s%s.%s = 0; /** warning: argument %s - unknown datatype **/\n",
			        (char *)&event->event_name[strlen("EVT_")],
			        trap_suffix,
                                argument->original_name,
                                argument->original_name);
                          }
                          fputs(buffer, outfile);
                          break;
                      }	
                    }

                argument = argument->next;
                }

	/*
         * Write out initialization for fixed elements of *_TDEF
         */
            sprintf(buffer, "            %s%s.enterprise = NULL;\n",
              (char *)&event->event_name[strlen("EVT_")],
              trap_suffix);
            fputs(buffer, outfile);
            sprintf(buffer, "            %s%s.enterprise_length = 0;\n",
              (char *)&event->event_name[strlen("EVT_")],
              trap_suffix);
            fputs(buffer, outfile);

            sprintf(buffer, "            %s%s.agent_addr = NULL;\n",
              (char *)&event->event_name[strlen("EVT_")],
              trap_suffix);
            fputs(buffer, outfile);

            sprintf(buffer, "            %s%s.agent_addr_length = 0;\n",
              (char *)&event->event_name[strlen("EVT_")],
              trap_suffix);
            fputs(buffer, outfile);

            sprintf(buffer, "            %s%s.time_stamp = 0;\n",
              (char *)&event->event_name[strlen("EVT_")],
              trap_suffix);
            fputs(buffer, outfile);

            sprintf(buffer, "            %s%s.time_stamp_valid = FALSE;\n",
              (char *)&event->event_name[strlen("EVT_")],
              trap_suffix);
            fputs(buffer, outfile);

            sprintf(buffer, "            %s%s.instance_name_length = 0;\n",
              (char *)&event->event_name[strlen("EVT_")],
              trap_suffix);
            fputs(buffer, outfile);

            sprintf(buffer, "            %s%s.instance_name = NULL;\n",
              (char *)&event->event_name[strlen("EVT_")],
              trap_suffix);
            fputs(buffer, outfile);

            fputs("\n", outfile);
            fputs("            send_", outfile);
            fputs((char *)&event->event_name[strlen("EVT_")], outfile);
            fputs("_trap();\n\n", outfile);
            fputs("        }\n\n", outfile);

            event = event->next;
            }

        class = class->next;
        }

#endif

    return SS$_NORMAL;

} /* End of insert_trap_arg_inits */
