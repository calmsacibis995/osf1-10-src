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
static char *rcsid = "@(#)$RCSfile: mg_set.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/06/15 11:55:39 $";
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
**	This module contains the routines to used to support the set entry point.
**
**  AUTHORS:
**
**      Rich Bouchard, Jr
**
**  CREATION DATE:  4-Apr-1991
**
**  MODIFICATION HISTORY:
**
**	F-29		    Marc Nozell	    	    15-Jun-1992
**  	Remove references to module.
**
**	F-4		    Mike Densmore	    21-May-1992
**
**	Modified for use on U**X platforms.
**
**	F-3		    Gary Allison	    23-Sep-1991
**
**	Add new routines, restructure module, support instance data.
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
#define NAME_SIZE 5

#include "mom_defs.h"
#include "mg_prototypes.h"
#include <string.h>


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_set_char
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-set-char tag.
**
**	This routine generates a "switch" statement with an entry
**	for every characteristic attribute.
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
int insert_set_char( mom,
		     outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    ATTRIBUTE_DEF   *attribute;
    int		    attribute_num;
    CLASS_DEF	    *class;
    char	    buffer[256];
    int		    chars = 0;
    int		    status;

    class = mom->current_class;					      
    attribute = class->attribute;

    for (attribute_num=0; attribute_num < class->num_attributes; attribute_num++)
    {
        if (attribute->group == GROUP_CHARACTERISTICS)
          {
          sprintf(buffer,
		    "        case %s%s  :  \n",
		    class->prefix_k, attribute->attribute_name);
	  fputs(buffer, outfile);
          chars++;
	  }
	attribute = attribute->next;
    }

    if (chars)
	{
        fputs("		return MAN_C_SUCCESS;\n",outfile);
	fputs("		break;\n",outfile);
	}

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_mom_check_attributes
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-mom-check-attributes tag.
**
**	This code generates the entire mom_check_attributes routine.
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
int insert_mom_check_attributes( mom,
			  	 outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    CLASS_DEF	    *class;
    int		    class_num;
    char	    buffer[256];
    int		    status;

    fputs("static man_status mom_check_attributes( avl *attribute_list, \n",outfile);
    fputs("                                        avl *reply_avl,\n",outfile);
    fputs("                                        int class_code)\n", outfile);
    fputs("{                               \n",				outfile);
    fputs("    man_status (*perform_check)();\n",			outfile);
    fputs("    man_status status;\n\n",					outfile);
    fputs("    switch (class_code)\n",					outfile);
    fputs("    {\n",							outfile);

    class = mom->class;

    for (class_num=0; class_num < mom->num_classes; class_num++)
    {
	sprintf(buffer,
		"        case %s%s_ID:\n", class->prefix_k, class->class_name);
	fputs(buffer, outfile);

	sprintf(buffer,
		"            perform_check = %scheck_all_attrs;\n",	      
		class->prefix_m);
	fputs(buffer, outfile);

	fputs("            break;\n", outfile);

	class = class->next;
    }

    fputs("    }\n\n",						outfile);
    fputs("    moss_avl_reset( attribute_list );\n\n", 		outfile);
    fputs("    status = perform_check( attribute_list, \n",outfile);
    fputs("                                reply_avl);\n\n",outfile);
    fputs("    return status;\n    }\n",			outfile);

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_add_check_attributes
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-add-check-attributes tag.
**
**	This routine generates the entire [[class]]check_all_attrs routine.
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
int insert_add_check_attributes( mom,
			  	 outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    EXCEPTION_DEF   *exc=NULL;
    ARGUMENT_DEF    *arg=NULL;
    CLASS_DEF	    *class;
    int		    class_num;
    char	    buffer[256];
    int		    status;

    class = mom->current_class;

    sprintf(buffer, 
	  "man_status %scheck_all_attrs( avl *modification_list, \n", class->prefix_m);
    fputs(buffer, outfile);
    fputs("                                   avl *reply_attribute_list ) \n",outfile);
    fputs("{                               \n",				outfile);

    fputs("    object_id *oid;\n",                   		outfile);
    fputs("    unsigned int modifier;\n",			outfile);
    fputs("    unsigned int tag;\n",   			        outfile);
    fputs("    octet_string *octet;\n",	    			outfile);
    fputs("    int last_one = 0;\n",        			outfile);
    fputs("    man_status status;\n",				outfile);
    fputs("    man_status check_status = MAN_C_SUCCESS;\n\n",	outfile);
    fputs("    do {\n",			    			outfile);
    fputs("    /*\n",outfile);
    fputs("     * Point to each element.  We are interested in the \n", outfile);
    fputs("     * object identifier, the modifier value, and the \n", outfile);
    fputs("     * new value(s) of the attribute.\n", outfile);
    fputs("     */\n", outfile);
    fputs("    status = moss_avl_point( modification_list,\n", outfile);
    fputs("                             &oid,\n", outfile);
    fputs("                             &modifier,\n", outfile);
    fputs("                             &tag,\n", outfile);
    fputs("                             &octet,\n", outfile);
    fputs("                             &last_one);\n", outfile);
    fputs("\n",outfile);
    fputs("    if (status == MAN_C_SUCCESS)\n    {\n",outfile);
    sprintf(buffer, 
	  "        status = %scheck_attr_value( oid, modifier, tag, octet, modification_list );\n", class->prefix_m);
    fputs(buffer, outfile);
    fputs("        if (status != MAN_C_SUCCESS) {\n",outfile);
    fputs("#ifdef MOMGENDEBUG\n",outfile);
    fputs("            printf(\"\\n*** Bad attribute value ***\\n\");\n",outfile);
    fputs("#endif /* MOMGENDEBUG */\n",outfile);
    fputs("            check_status = status;\n",outfile);
    fputs("           }\n",outfile);
    fputs("        if (IS_CONSTRUCTED( tag ))   /* If construction, go on to the next AVL element */ \n",outfile);
    fputs("           status = moss_avl_exit_construction( modification_list,  TRUE, 0, &last_one);\n",outfile);
    fputs("        }\n",outfile);
    fputs("    else \n", outfile);
    fputs("        if (status == MAN_C_NO_ELEMENT)\n", outfile);
    fputs("             return MAN_C_SUCCESS;\n", outfile);
    fputs("        else \n", outfile);
    fputs("             return status;\n\n", outfile);
    fputs("    } while (!last_one);\n\n",outfile);    
    fputs("    return check_status;\n  }\n",outfile);
    return SS$_NORMAL;
}

/*
*++
**  FUNCTIONAL DESCRIPTION:
**
**      insert_check_types
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-check-datatypes tag.
**
**	This routine generates the code to verify that a set of user-supplied
**	attributes are of the proper size and datatypes.
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
int insert_check_types( mom,
		        outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    int		    argument_num;
    ATTRIBUTE_DEF   *attribute;
    int		    attribute_num;
    CLASS_DEF	    *class;
    char	    buffer[256];
    int		    status;
    ARGUMENT_DEF    *argument;

    class = mom->current_class;					      
    attribute = class->attribute;

    if (class->num_attributes == 0) {   /* some SNMP classes have no attributes */
        fputs("                case 0:\n", outfile);
        fputs("                    break;\n", outfile);
        return SS$_NORMAL;
    }

    for (attribute_num=0; attribute_num < class->num_attributes; attribute_num++)
    {
	/*
   	 * The routine check_attr_value can be called from perform_create
	 * and perform_set. Therefore, it is necessary to have the ID Code for
	 * each of the attributes (for set) and create arguments (for create).
	 * If they are the same, then use the attribute value, if different,
	 * then they both need to be part of the switch statement.
	 */
	
	  if (get_create_arg( class, attribute, &argument ))
	    {
	    sprintf(buffer,
		    "        case %s%s  :  case %s%s :\n",
		    class->prefix_k, attribute->attribute_name,
		    class->prefix_k, argument->argument_name);
	    fputs(buffer, outfile);
	    }	
	  else
	    {
	    sprintf(buffer,
		    "        case %s%s:\n",
		    class->prefix_k, attribute->attribute_name);
	    fputs(buffer, outfile);
	    }

	/*
	 * Generate the length check. NOTE that MCC may use a length or 1,2, or 4
	 * based on the size of the integer specified EVEN THOUGH the attribute
	 * was declared with an integer32.
	 */

	switch (attribute->mg_type)
	{
	case MG_C_INTEGER :
	    sprintf(buffer,
		    "            if ( !((octet->length > 0 ) && (octet->length <= %s)) || (octet->data_type != ASN1_C_%s))\n",
		    attribute->length, attribute->data_type);
            fputs(buffer, outfile);
	    fputs("                return MAN_C_INVALID_ATTRIBUTE_VALUE;\n", outfile);
	    break;

	case MG_C_ACL :
	case MG_C_AVL :
	    break;

	case MG_C_UNKNOWN :
	    sprintf(buffer,
		    "            /** Unknown data type %s **/\n", attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    break;

	default :
	    sprintf(buffer,
		    "            if (octet->data_type != ASN1_C_%s)\n",
		    attribute->data_type);
	    fputs(buffer, outfile);
	    fputs("                return MAN_C_INVALID_ATTRIBUTE_VALUE;\n", outfile);
	    break;
	}	

	fputs("            break;\n", outfile);

	attribute = attribute->next;
    }

    argument = class->create_arg;
    for (argument_num=0; argument_num < class->num_create_args; argument_num++)
     {
      if (create_arg_not_attribute( class, argument ))
      {
	sprintf(buffer,
		    "        case %s%s:\n",
		    class->prefix_k, argument->argument_name);
	fputs(buffer, outfile);

	/*
	 * Generate the length check. NOTE that MCC may use a length or 1,2, or 4
	 * based on the size of the integer specified EVEN THOUGH the argument
	 * was declared with an integer32.
	 */

	switch (argument->mg_type)
	{
	case MG_C_INTEGER:
	    sprintf(buffer,
		    "            if ( !((octet->length > 0 ) && (octet->length <= %s)) || (octet->data_type != ASN1_C_%s))\n",
		    argument->argument_length, argument->data_type);
            fputs(buffer, outfile);
	    fputs("                return MAN_C_INVALID_ATTRIBUTE_VALUE;\n", outfile);
	    break;

	case MG_C_UNKNOWN:
	    sprintf(buffer,
		    "            /** Unknown data type %s **/\n", argument->argument_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    break;

	case MG_C_ACL:
	case MG_C_AVL:
	    break;

	default:
	    sprintf(buffer,
		    "            if (octet->data_type != ASN1_C_%s)\n",
		    argument->data_type);
	    fputs(buffer, outfile);
	    fputs("                return MAN_C_INVALID_ATTRIBUTE_VALUE;\n", outfile);
	}

	fputs("            break;\n", outfile);
      }
      argument = argument->next;
    }

    return SS$_NORMAL;
}

/*
*++
**  FUNCTIONAL DESCRIPTION:
**
**      get_create_arg
**
**  FORMAL PARAMETERS:
**
**      class		Pointer to current class.
**	attribute	Pointer to current attribute.
**	argument	Address of pointer to argument if matched.
**
**  RETURN VALUE:
**
**
**--
*/
int get_create_arg( class,
	       	    attribute,
	       	    arg )

CLASS_DEF *class;
ATTRIBUTE_DEF *attribute;
ARGUMENT_DEF **arg;


{
ARGUMENT_DEF *argument;

argument = class->create_arg;
while (argument != NULL)
    {
    if ((argument->original_name != NULL) && (attribute->original_name != NULL))
	{
        if (strcmp((attribute->original_name),(argument->original_name)) == 0)
	    {
            *arg = argument;
            return 1;
	    }
	}
    else      
        if (strcmp((attribute->attribute_name+NAME_SIZE),(argument->argument_name+NAME_SIZE)) == 0)
	    {
            *arg = argument;
            return 1;
	    }

   argument = argument->next;
   }

return 0;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_check_values
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-check-values tag.
**
**	This routine generates code to stub code to check the value of the
**	attribute for the user to modify.
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
int insert_check_values( mom,
		         outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    int		    argument_num;
    ARGUMENT_DEF    *argument;
    ATTRIBUTE_DEF   *attribute;
    int		    attribute_num;
    CLASS_DEF	    *class;
    char	    buffer[256];
    int		    status;

    class = mom->current_class;					      
    attribute = class->attribute;

    if (class->num_attributes == 0) {   /* some SNMP classes have no attributes */
        fputs("                case 0:\n", outfile);
        fputs("                    break;\n", outfile);
        return SS$_NORMAL;
    }

    for (attribute_num=0; attribute_num < class->num_attributes; attribute_num++)
    {
	/*
   	 * The routine check_attr_value can be called from perform_create
	 * and perform_set. Therefore, it is necessary to have the ID Code for
	 * each of the attributes (for set) and create arguments (for create).
	 * If they are the same, then use the attribute value, if different,
	 * then they both need to be part of the switch statement.
	 *
	 * Always use the create argument since the constant is prefix_k which
	 * is unique and will NOT generate a duplicate case error.
	 */
	
	  if (get_create_arg( class, attribute, &argument ))
	    {
	    sprintf(buffer,
		    "        case %s%s  :  case %s%s :\n",
		    class->prefix_k, attribute->attribute_name,
		    class->prefix_k, argument->argument_name);
	    fputs(buffer, outfile);
	    }	
	  else
	    {
	    sprintf(buffer,
 		"        case %s%s:\n",
		class->prefix_k,
		attribute->attribute_name);
  	    fputs(buffer, outfile);
	    }
	sprintf(buffer,
		"            if (FALSE) /** If %s attribute has invalid value **/\n",
		attribute->attribute_name);
	fputs(buffer, outfile);

	fputs("                return MAN_C_INVALID_ATTRIBUTE_VALUE;\n", outfile);

	fputs("            break;\n", outfile);

	attribute = attribute->next;
    }

    argument = class->create_arg;
    for (argument_num=0; argument_num < class->num_create_args; argument_num++)
     {
      if (create_arg_not_attribute( class, argument ))
      {
	sprintf(buffer, "        case %s%s:\n", class->prefix_k,
		argument->argument_name);
  	fputs(buffer, outfile);

	sprintf(buffer,
		"            if (FALSE) /** If %s argument has invalid value **/\n",
		argument->argument_name);
	fputs(buffer, outfile);

	fputs("                return MAN_C_INVALID_ATTRIBUTE_VALUE;\n", outfile);

	fputs("            break;\n", outfile);
      }
      argument = argument->next;
    }

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_set_attributes
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file.
**
**	This routine generates the "switch" statement to perform
**	a "set" function on each attribute.
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
int insert_set_attributes( mom,
		           outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    ATTRIBUTE_DEF   *attribute;
    int		    attribute_num;
    CLASS_DEF	    *class;
    char	    buffer[256];
    int		    status;

    class = mom->current_class;					      
    attribute = class->attribute;

    if (class->num_attributes == 0) {   /* some SNMP classes have no attributes */
        fputs("                case 0:\n", outfile);
        fputs("                    break;\n", outfile);
        return SS$_NORMAL;
    }

    for (attribute_num=0; attribute_num < class->num_attributes; attribute_num++)
    {
	sprintf(buffer,
		"        case %s%s:",
		class->prefix_k,
		attribute->attribute_name);
	fputs(buffer, outfile);

	/* This switch sets the instance structure from the AVL */
	
	switch (attribute->mg_type)
	{

	case MG_C_STRING:
	    fputs("    /* OCTET_STRING */\n", outfile );        
	    sprintf(buffer,
		  "            if (instance->%s != NULL)\n",
		   attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    sprintf(buffer,
		  "                free( instance->%s );\n",
		   attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    sprintf(buffer,
		  "            instance->%s = (char *) malloc( octet->length );\n",
		   attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    sprintf(buffer,
		  "            memcpy(instance->%s,\n",
		   attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs("                   octet->string,\n                   octet->length);\n", outfile);
	    sprintf(buffer,
		  "            instance->%s_len = (unsigned int) octet->length;\n",
		   attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs("#ifdef MOMGENDEBUG\n",outfile);
	    sprintf(buffer,"            printf(\" %s:\\n\" );\n", 
		   attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs("#endif /* MOMGENDEBUG */\n",outfile);
	    fputs("            break;\n\n", outfile);
	    break;

	case MG_C_BOOLEAN:
	    fputs("    /* BOOLEAN  */\n", outfile );        
	    sprintf(buffer,
		  "            instance->%s = (unsigned int) *octet->string;\n"  ,
		   attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs("#ifdef MOMGENDEBUG\n",outfile);
	    sprintf(buffer,
		  "            printf(\"Setting %s:\t%%d\\n\", instance->%s);\n", 
		   attribute->attribute_name+NAME_SIZE,attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs("#endif /* MOMGENDEBUG */\n",outfile);
	    fputs("            break;\n\n", outfile);
	    break;
		
	case MG_C_INTEGER:
	    fputs("    /* INTEGER  */\n", outfile );        
	    if (attribute->sign == MG_C_UNSIGNED)
		sprintf(buffer,"            status = copy_octet_to_unsigned_int( &instance->%s, octet );\n",
			attribute->attribute_name+NAME_SIZE);
	    else
		sprintf(buffer,"            status = copy_octet_to_signed_int( &instance->%s, octet );\n",
			attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs("            if ERROR_CONDITION( status )\n",outfile);
	    fputs("                return status;\n",outfile);
	    fputs("#ifdef MOMGENDEBUG\n",outfile);
	    sprintf(buffer,
		  "            printf(\"Setting %s:\t%%d\\n\", instance->%s);\n", 
		   attribute->attribute_name+NAME_SIZE,attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs("#endif /* MOMGENDEBUG */\n",outfile);
	    fputs("            break;\n\n", outfile);
	    break;

	case MG_C_OID:
	    sprintf(buffer,"            status = moss_octet_to_oid( octet, &instance->%s );\n",
		    attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs("            if ERROR_CONDITION( status )\n",outfile);
	    fputs("                return status;\n",outfile);
	    fputs("            break;\n\n", outfile);
	    break;

	case MG_C_NULL:
	    sprintf(buffer,
		"            instance->%s = (char *)NULL;\n",
		attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs("            break;\n\n", outfile);
	    break;

	case MG_C_UID:
	    fputs("    /* UID */\n", outfile );        
	    sprintf(buffer,
		  "            memcpy(&instance->%s, octet->string, 16 );\n",
		   attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs("#ifdef MOMGENDEBUG\n",outfile);
	    sprintf(buffer,
		  "            printf(\"Setting %s:\\n\");\n",
		   attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs("#endif /* MOMGENDEBUG */\n",outfile);
	    fputs("            break;\n\n", outfile);
	    break;
	
	case MG_C_ACL:
	  fputs("	/* ACL */\n", outfile );        
	  if (attribute->dna_cmip_int_len)
	    {    
	    fputs("            status = validate_acl( attribute_list, \n", outfile);
	    sprintf(buffer,
		      "                               &%sATTR_%s_DNA,\n",
		      class->prefix_m,attribute->attribute_name+NAME_SIZE );
	    fputs(buffer, outfile);
	      sprintf(buffer,
		  "                                   &instance->%s,\n",
		   attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    sprintf(buffer,
		      "                               &%sATTR_%s_DNA );\n",
		      class->prefix_m,attribute->attribute_name+NAME_SIZE );
	    fputs(buffer, outfile);
    	    fputs("#ifdef MOMGENDEBUG\n",outfile);
	    sprintf(buffer,
		  "            printf(\"Setting %s:\\n\" );\n", attribute->attribute_name+NAME_SIZE );
	    fputs(buffer, outfile);
    	    fputs("#endif /* MOMGENDEBUG */\n",outfile);
	    }
	  else 
	    fputs("            /** validate ACL **/\n",outfile);

	  fputs("            break;\n\n", outfile);
	  break;

	case MG_C_AVL:
	    fputs("    /* AVL */\n", outfile );        
	    fputs("	    if (modifier == MAN_C_SET_MODIFY)\n", outfile ); 
	    if (attribute->dna_cmip_int_len)
	        {
	        if (class->multiple_oids)
		    fputs("#ifdef DNA_CMIP_OID\n",outfile);
	        sprintf(buffer,
		  "                status = copy_avl_attribute( &%sATTR_%s_DNA, attribute_list, &instance->%s );\n",
		  class->prefix_m,attribute->attribute_name+NAME_SIZE,attribute->attribute_name+NAME_SIZE);
	        fputs(buffer, outfile);
	        fputs("            if ERROR_CONDITION( status )\n",outfile);
	        fputs("                return status;\n",outfile);
	        if (class->multiple_oids)
		    fputs("#endif /* DNA_CMIP_OID */\n",outfile);
		}
	    if (attribute->osi_oid_len)
	        {
		fputs("#ifdef OSI_CMIP_OID\n",outfile);
	        sprintf(buffer,
		  "                status = copy_avl_attribute( &%sATTR_%s_OSI, attribute_list, &instance->%s );\n",
		  class->prefix_m,attribute->attribute_name+NAME_SIZE,attribute->attribute_name+NAME_SIZE);
	        fputs(buffer, outfile);
	        fputs("            if ERROR_CONDITION( status )\n",outfile);
	        fputs("                return status;\n",outfile);
		fputs("#endif /* OSI_CMIP_OID */\n",outfile);
		}
	    if (attribute->snmp_oid_len)
	        {
		fputs("#ifdef SNMP_OID\n",outfile);
	        sprintf(buffer,
		  "                status = copy_avl_attribute( &%sATTR_%s_SNP, attribute_list, &instance->%s );\n",
		  class->prefix_m,attribute->attribute_name+NAME_SIZE,attribute->attribute_name+NAME_SIZE);
	        fputs(buffer, outfile);
	        fputs("            if ERROR_CONDITION( status )\n",outfile);
	        fputs("                return status;\n",outfile);
		fputs("#endif /* SNMP_OID */\n",outfile);
		}
	    if (attribute->oid_len)
	        {
		fputs("#ifdef OID\n",outfile);
	        sprintf(buffer,
		  "                status = copy_avl_attribute( &%sATTR_%s_OID, attribute_list, &instance->%s );\n",
		  class->prefix_m,attribute->attribute_name+NAME_SIZE,attribute->attribute_name+NAME_SIZE);
	        fputs(buffer, outfile);
	        fputs("            if ERROR_CONDITION( status )\n",outfile);
	        fputs("                return status;\n",outfile);
		fputs("#endif /* OID */\n",outfile);
		}
	    fputs("	    if (modifier == MAN_C_SET_ADD)\n", outfile );   
	    if (attribute->dna_cmip_int_len)
	        {
	        if (class->multiple_oids)
		    fputs("#ifdef DNA_CMIP_OID\n",outfile);
	        sprintf(buffer,
		      "                status = add_avl_attribute( &%sATTR_%s_DNA, attribute_list, &instance->%s );\n",
		      class->prefix_m,attribute->attribute_name+NAME_SIZE,attribute->attribute_name+NAME_SIZE);
	        fputs(buffer, outfile);
	        fputs("            if ERROR_CONDITION( status )\n",outfile);
	        fputs("                return status;\n",outfile);
	        if (class->multiple_oids)
		    fputs("#endif /* DNA_CMIP_OID */\n",outfile);
		}
	    if (attribute->osi_oid_len)
	        {
		fputs("#ifdef OSI_CMIP_OID\n",outfile);
	        sprintf(buffer,
		      "                status = add_avl_attribute( &%sATTR_%s_OSI, attribute_list, &instance->%s );\n",
		      class->prefix_m,attribute->attribute_name+NAME_SIZE,attribute->attribute_name+NAME_SIZE);
	        fputs(buffer, outfile);
	        fputs("            if ERROR_CONDITION( status )\n",outfile);
	        fputs("                return status;\n",outfile);
		fputs("#endif /* OSI_CMIP_OID */\n",outfile);
		}
	    if (attribute->snmp_oid_len)
	        {
		fputs("#ifdef SNMP_OID\n",outfile);
	        sprintf(buffer,
		     "                status = add_avl_attribute( &%sATTR_%s_SNP, attribute_list, &instance->%s );\n",
		     class->prefix_m,attribute->attribute_name+NAME_SIZE,attribute->attribute_name+NAME_SIZE);
	        fputs(buffer, outfile);
	        fputs("            if ERROR_CONDITION( status )\n",outfile);
	        fputs("                return status;\n",outfile);
		fputs("#endif /* SNMP_OID */\n",outfile);
		}
	    if (attribute->oid_len)
	        {
		fputs("#ifdef OID\n",outfile);
	    	sprintf(buffer,
		     "                status = add_avl_attribute( &%sATTR_%s_OID, attribute_list, &instance->%s );\n",
		     class->prefix_m,attribute->attribute_name+NAME_SIZE,attribute->attribute_name+NAME_SIZE);
	        fputs(buffer, outfile);
	        fputs("            if ERROR_CONDITION( status )\n",outfile);
	        fputs("                return status;\n",outfile);
		fputs("#endif /* OID */\n",outfile);
		}
	    fputs("	    if (modifier == MAN_C_SET_REMOVE)\n", outfile );
	    if (attribute->dna_cmip_int_len)
	        {
	        if (class->multiple_oids)
		    fputs("#ifdef DNA_CMIP_OID\n",outfile);
	        sprintf(buffer,
		    "                status = remove_avl_attribute( &%sATTR_%s_DNA, attribute_list, &instance->%s );\n",
		    class->prefix_m,attribute->attribute_name+NAME_SIZE,attribute->attribute_name+NAME_SIZE);
	        fputs(buffer, outfile);
	        fputs("            if ERROR_CONDITION( status )\n",outfile);
	        fputs("                return status;\n",outfile);
	        if (class->multiple_oids)
		    fputs("#endif /* DNA_CMIP_OID */\n",outfile);
		}
	    if (attribute->osi_oid_len)
	        {
		fputs("#ifdef OSI_CMIP_OID\n",outfile);
	        sprintf(buffer,
		    "                status = remove_avl_attribute( &%sATTR_%s_OSI, attribute_list, &instance->%s );\n",
		    class->prefix_m,attribute->attribute_name+NAME_SIZE,attribute->attribute_name+NAME_SIZE);
	        fputs(buffer, outfile);
	        fputs("            if ERROR_CONDITION( status )\n",outfile);
	        fputs("                return status;\n",outfile);
		fputs("#endif /* OSI_CMIP_OID */\n",outfile);
		}
	    if (attribute->snmp_oid_len)
	        {
		fputs("#ifdef SNMP_OID\n",outfile);
	        sprintf(buffer,
		    "                status = remove_avl_attribute( &%sATTR_%s_SNP, attribute_list, &instance->%s );\n",
		    class->prefix_m,attribute->attribute_name+NAME_SIZE,attribute->attribute_name+NAME_SIZE);
	        fputs(buffer, outfile);
	        fputs("            if ERROR_CONDITION( status )\n",outfile);
	        fputs("                return status;\n",outfile);
		fputs("#endif /* SNMP_OID */\n",outfile);
		}
	    if (attribute->oid_len)
	        {
		fputs("#ifdef OID\n",outfile);
	        sprintf(buffer,
		    "                status = remove_avl_attribute( &%sATTR_%s_OID, attribute_list, &instance->%s );\n",
		    class->prefix_m,attribute->attribute_name+NAME_SIZE,attribute->attribute_name+NAME_SIZE);
	        fputs(buffer, outfile);
	        fputs("            if ERROR_CONDITION( status )\n",outfile);
	        fputs("                return status;\n",outfile);
		fputs("#endif /* OID */\n",outfile);
		}
	    fputs("#ifdef MOMGENDEBUG\n",outfile);
	    sprintf(buffer,
		  "            printf(\"Setting %s:\\n\" );\n", attribute->attribute_name+NAME_SIZE );
	    fputs(buffer, outfile);
	    fputs("#endif /* MOMGENDEBUG */\n",outfile);
	    fputs("            break;\n\n", outfile);
	    break;

	case MG_C_BIT :
	    fputs("    /** BIT SET data type **/\n", outfile );        
	    sprintf(buffer,
		  "            instance->%s = (unsigned int) *octet->string;\n"  ,
		   attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
    	    fputs("#ifdef MOMGENDEBUG\n",outfile);
	    sprintf(buffer,"            printf(\" %s:\\n\" );\n", 
		   attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
    	    fputs("#endif /* MOMGENDEBUG */\n",outfile);
	    fputs("            break;\n\n", outfile);
	    break;
	
	case MG_C_TIME:
	    fputs("	/* TIME */\n", outfile ); 
            sprintf(buffer, "            memcpy( instance->%s, octet->string, 16 );\n", attribute->attribute_name+NAME_SIZE);
    	    fputs("#ifdef MOMGENDEBUG\n",outfile);
	    sprintf(buffer,"            printf(\" Setting %s:\\n\" );\n", 
			attribute->attribute_name+NAME_SIZE );
	    fputs(buffer, outfile);
    	    fputs("#endif /* MOMGENDEBUG */\n",outfile);
	    fputs("            break;\n\n", outfile);
	    break;

	case MG_C_VERSION :
	    fputs("    /* VERSION */\n", outfile );        
	    sprintf(buffer,
		  "            instance->%s = (char *) malloc( octet->length + 1);\n",
		   attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    sprintf(buffer,
		  "            memcpy(instance->%s,\n",
		   attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs("                   octet->string,\n                   octet->length);\n", outfile);
	    sprintf(buffer,
		  "            instance->%s[octet->length] = \'\\0\';\n",
		   attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs("#ifdef MOMGENDEBUG\n",outfile);
	    sprintf(buffer,
		  "            printf(\"Setting %s:\t%%s\\n\", instance->%s);\n",
		   attribute->attribute_name+NAME_SIZE,attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs("#endif /* MOMGENDEBUG */\n",outfile);
	    fputs("            break;\n\n", outfile);
	    break;
	
	default:
	    fputs("     /** Unknown data type **/\n", outfile );        
	    fputs("     /** Verify data type and set instance value **/\n",outfile);
	    sprintf(buffer,
		  "            status = copy_octet_to_unsigned_int( &instance->%s, octet );\n"  ,
		   attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs("            if ERROR_CONDITION( status )\n",outfile);
	    fputs("                return status;\n",outfile);
	    fputs("#ifdef MOMGENDEBUG\n",outfile);
	    sprintf(buffer,
		  "            printf(\"Setting %s:\t%%s\\n\", instance->%s);\n", 
		   attribute->attribute_name+NAME_SIZE,attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs("#endif /* MOMGENDEBUG */\n",outfile);
	    fputs("            break;\n\n", outfile);
	    break;
	}
	
    attribute = attribute->next;
    }

    return SS$_NORMAL;
}
